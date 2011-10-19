//$Id: ScriptReadWriter.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                           ScriptReadWriter
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Allison Greene
// Created: 2006/07/10
//
/**
 * Implements reading and writing a script file.
 */
//------------------------------------------------------------------------------

#include "ScriptReadWriter.hpp"
#include "InterpreterException.hpp"
#include "StringUtil.hpp"
#include "MessageInterface.hpp"
#include <wx/txtstrm.h>
#include <sstream>

ScriptReadWriter* ScriptReadWriter::instance = NULL;
const wxString ScriptReadWriter::sectionDelimiter = wxT("%--------");
const wxString ScriptReadWriter::ellipsis = wxT("...");

//#define DEBUG_SCRIPT_READ
//#define DEBUG_FIRST_BLOCK

//---------------------------------
// public
//---------------------------------

//------------------------------------------------------------------------------
// static Instance()
//------------------------------------------------------------------------------
ScriptReadWriter* ScriptReadWriter::Instance()
{
   if (instance == NULL)
   {
      instance = new ScriptReadWriter();
      instance->Initialize();
   }
   
   return instance;
}


//------------------------------------------------------------------------------
// ~ScriptReadWriter()
//------------------------------------------------------------------------------
ScriptReadWriter::~ScriptReadWriter()
{
}


//------------------------------------------------------------------------------
// void SetInStream(wxInputStream *is)
//------------------------------------------------------------------------------
void ScriptReadWriter::SetInStream(wxInputStream *is)
{
   inStream = is; 
   reachedEndOfFile = false;
   readFirstBlock = false;
   currentLineNumber = 0;
}


//------------------------------------------------------------------------------
// Integer GetLineWidth()
//------------------------------------------------------------------------------
Integer ScriptReadWriter::GetLineWidth()
{
   return lineWidth;
}


//------------------------------------------------------------------------------
// void SetLineWidth(Integer lineWidth)
//------------------------------------------------------------------------------
void ScriptReadWriter::SetLineWidth(Integer width)
{
   if ((width < 20) && (width != 0))
      throw InterpreterException
         (wxT("Line width must either be unlimited (denoted by 0) or greater ")
          wxT("than 19 characters.\n"));
   
   lineWidth = width;
}

//------------------------------------------------------------------------------
// Integer GetLineNumber()
//------------------------------------------------------------------------------
Integer ScriptReadWriter::GetLineNumber()
{
   return currentLineNumber;
}


//------------------------------------------------------------------------------
// void ReadFirstBlock(wxString &header, wxString &firstBlock,
//                     bool skipHeader = false)
//------------------------------------------------------------------------------
/*
 * Reads header and first preface comment and script from the script file.
 * The header block ends when first blank line is read.
 * The first block ends when first non-blank and non-comment line is read.
 * When skipHeader is true, it will read as first block. Usually skipping header
 * will be needed when interpreting ScriptEvent from the GUI.
 *
 * @param  header  Header comment lines read
 * @param  firstBlock  First preface comment and script read
 * @param  skipHeader Flag indicating first comment block is not a header(false)
 */
//------------------------------------------------------------------------------
void ScriptReadWriter::ReadFirstBlock(wxString &header, wxString &firstBlock,
                                      bool skipHeader)
{
   wxString newLine = wxT("");
   header = wxT("");
   firstBlock = wxT("");
   bool doneWithHeader = false;
   
   if (reachedEndOfFile)
      return;
   
   // get 1 line of text
   newLine = CrossPlatformGetLine();
   
   #ifdef DEBUG_FIRST_BLOCK
   MessageInterface::ShowMessage
      (wxT("ReadFirstBlock() firstLine=<<<%s>>>\n"), newLine.c_str());
   #endif
   
   if (reachedEndOfFile && IsBlank(newLine))
      return;
   
   //if line is not blank and is not comment line, return this line
   if (!IsBlank(newLine) && (!IsComment(newLine)))
   {
      firstBlock = newLine;
      return;
   }
   
   header = newLine + wxT("\n");
   
   if (IsBlank(newLine))
      doneWithHeader = true;
   
   //-----------------------------------------------------------------
   // Read header comments
   // keep looping and append till we find blank line or end of file
   //-----------------------------------------------------------------
   if (!doneWithHeader)
   {
      while (!reachedEndOfFile)
      {
         newLine = CrossPlatformGetLine();
         
         #ifdef DEBUG_FIRST_BLOCK
         MessageInterface::ShowMessage
            (wxT("   header newLine=<<<%s>>>\n"), newLine.c_str());
         #endif
         
         // If non-blank and non-comment line found, return
         if (!IsBlank(newLine) && (!IsComment(newLine)))
         {
            firstBlock = newLine + wxT("\n");
            
            if (skipHeader)
            {
               firstBlock = header + firstBlock;
               header = wxT("");
            }
            
            #ifdef DEBUG_FIRST_BLOCK
            MessageInterface::ShowMessage
               (wxT("ReadFirstBlock() non-blank and non-comment found\n")
                wxT("header=<<<%s>>>\nfirstBlock=<<<%s>>>\n"), header.c_str(),
                firstBlock.c_str());
            #endif
            
            return;
         }
         
         // If blank line found, break
         if (IsBlank(newLine))
         {
            header = header + newLine + wxT("\n");
            doneWithHeader = true;
            break;
         }
         
         header = header + newLine + wxT("\n");
      }
   }
   
   
   //-----------------------------------------------------------------
   // Read first script
   // Keep looping and append till we find non-blank/non-comment line
   // or end of file
   //-----------------------------------------------------------------
   while (!reachedEndOfFile)
   {
      newLine = CrossPlatformGetLine();
      
      #ifdef DEBUG_FIRST_BLOCK
      MessageInterface::ShowMessage(wxT("   1stblk newLine=<<<%s>>>\n"), newLine.c_str());
      #endif
      
      // If non-blank and non-comment line found, break
      if (!IsBlank(newLine) && (!IsComment(newLine)))
      {
         firstBlock = firstBlock + newLine + wxT("\n");
         break;
      }
      
      firstBlock = firstBlock + newLine + wxT("\n");
   }
   
   if (skipHeader)
   {
      firstBlock = header + firstBlock;
      header = wxT("");
   }
   
   #ifdef DEBUG_FIRST_BLOCK
   MessageInterface::ShowMessage
      (wxT("ReadFirstBlock() header=<<<%s>>>\nfirstBlock=<<<%s>>>\n"), header.c_str(),
       firstBlock.c_str());
   #endif
}


//------------------------------------------------------------------------------
// wxString ReadLogicalBlock()
//------------------------------------------------------------------------------
/*
 * Reads lines until non-blank and non-comment line from the input stream
 */
//------------------------------------------------------------------------------
wxString ScriptReadWriter::ReadLogicalBlock()
{
   wxString result = wxT("");
   wxString oneLine = wxT(""); 
   wxString block = wxT("");
   
   if (reachedEndOfFile)
      return wxT("\0");
   
   // get 1 line of text
   oneLine = CrossPlatformGetLine();
   
   if (reachedEndOfFile && IsBlank(oneLine))
      return wxT("\0");
   
   #ifdef DEBUG_SCRIPT_READ
   MessageInterface::ShowMessage
      (wxT("ReadLogicalBlock() oneLine=\n<<<%s>>>\n"), oneLine.c_str());
   #endif
   
   // keep looping till we find non-blank or non-comment line
   while ((!reachedEndOfFile) && (IsBlank(oneLine) || IsComment(oneLine)))
   {
      block = block + oneLine + wxT("\n");
      oneLine = CrossPlatformGetLine();
      
      #ifdef DEBUG_SCRIPT_READ
      MessageInterface::ShowMessage
         (wxT("ReadLogicalBlock() oneLine=\n<<<%s>>>\n"), oneLine.c_str());
      #endif
   }
   
   block = block + oneLine + wxT("\n");
   
   #ifdef DEBUG_SCRIPT_READ
   MessageInterface::ShowMessage
      (wxT("ReadLogicalBlock() block=\n<<<%s>>>\n"), block.c_str());
   #endif
   
   result = block;
   
   if (HasEllipse(oneLine))
   {
      // overwrite result with mulitple lines separated  by ellipsis
      result = block + HandleEllipsis(oneLine);
   }
   
   readFirstBlock = true;
   
   return result;
}


//------------------------------------------------------------------------------
// bool WriteText(const wxString &textToWrite)
//------------------------------------------------------------------------------
bool ScriptReadWriter::WriteText(const wxString &textToWrite)
{
   wxTextOutputStream theOutStream(*outStream);
   theOutStream << textToWrite;
//   outStream->flush();
   return true;
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
bool ScriptReadWriter::Initialize()
{
   lineWidth = 80;
   currentLineNumber = 0;
   writeGmatKeyword = true;
   reachedEndOfFile = false;
   readFirstBlock = false;
   
   return true;  // need to change so if something wasn't set to return false
}


//------------------------------------------------------------------------------
// wxString CrossPlatformGetLine()
//------------------------------------------------------------------------------
wxString ScriptReadWriter::CrossPlatformGetLine()
{
   char ch;
   wxString result;
   wxTextInputStream theInStream(*inStream);
   
 /*  while (inStream->get(ch) && ch != '\r' && ch != '\n' && ch != '\0' &&
          !inStream->eof()) 
   {
      if (result.length() < 3)
      {
         // Test 1st 3 bytes for non-ANSI encoding -- anything with the top bit set
         if (ch < 0)
         {
            throw InterpreterException(wxT("Non-standard characters were ")
                  wxT("encountered in the script file; please check the file to ")
                  wxT("be sure it is saved as an ASCII file, and not formatted ")
                  wxT("for Unicode or UTDF."));
         }
      }
      result += ch;
   }
   */
   result = theInStream.ReadLine();
   if  (inStream->Eof())
   {
      reachedEndOfFile = true;
   }
   
   ++currentLineNumber;
   currentLine = result;
   
   return result;
}


//------------------------------------------------------------------------------
// bool IsComment(const wxString &text)
//------------------------------------------------------------------------------
bool ScriptReadWriter::IsComment(const wxString &text)
{
   wxString str = GmatStringUtil::Trim(text, GmatStringUtil::BOTH);
   return GmatStringUtil::StartsWith(str, wxT("%"));
}


//------------------------------------------------------------------------------
// bool IsBlank(const wxString &text)
//------------------------------------------------------------------------------
bool ScriptReadWriter::IsBlank(const wxString &text)
{
   wxString str = GmatStringUtil::Trim(text, GmatStringUtil::BOTH);
   
   if (str == wxT(""))
      return true;
   else
      return false;
}


//------------------------------------------------------------------------------
// bool HasEllipse(const wxString &text)
//------------------------------------------------------------------------------
bool ScriptReadWriter::HasEllipse(const wxString &text)
{
   wxString ellipsis = wxT("...");

   int pos = text.find(ellipsis,0);
   
   if (pos < 0)
      return false;
   else
      return true;
}

//------------------------------------------------------------------------------
// wxString HandleEllipsis(const wxString &text)
//------------------------------------------------------------------------------
wxString ScriptReadWriter::HandleEllipsis(const wxString &text)
{
   wxString str = GmatStringUtil::Trim(text, GmatStringUtil::TRAILING);
   int pos = str.find(ellipsis,0);
   
   if (pos < 0)      // no ellipsis
      return str;
   
   // make sure ellipsis is at the end of the line
   if ((int)(str.size())-3 != pos)
   {
      wxString buffer;
      buffer << currentLineNumber;
      throw InterpreterException(wxT("Script Line ") + buffer +
                                 wxT("-->Ellipses must be at the end of the line\n") );
   }
   
   wxString result = wxT("");
   
   while (pos >= 0)
   {
      if (pos == 0)     // ellipsis were on a line by themselves
        result += wxT(" ");
      else
      {
         result += str.substr(0, pos);  // add substring to first set
         result += wxT(" ");
      }
      
      // reset str string and position
      str = wxT("");
      pos = -1;
      
      // read a line
      str = CrossPlatformGetLine();
      
      while (IsBlank(str) && !reachedEndOfFile)
         str = CrossPlatformGetLine();
      
      if (IsBlank(str) && reachedEndOfFile)
      {
         wxString buffer;
         buffer << currentLineNumber;
         throw InterpreterException(wxT("Script Line ") + buffer +
             wxT("-->Prematurely reached the end of file.\n"));
      }
      
      if (IsComment(str))
      {
         wxString buffer;
         buffer << currentLineNumber;
         throw InterpreterException(wxT("Script Line ") + buffer +
            wxT("-->Comments are not allowed in the middle of a block\n"));
      }
      
      str = GmatStringUtil::Trim(str, GmatStringUtil::TRAILING);      
      pos = str.find(ellipsis, 0);
   }
   
   // add the last line on to result
   if (IsComment(str))
   {
      wxString buffer;
      buffer << currentLineNumber;
      throw InterpreterException(wxT("Script Line ") + buffer +
         wxT("-->Comments are not allowed in the middle of a block\n"));
   }
   
   result += str;
   return result;
}


//------------------------------------------------------------------------------
// wxString HandleComments(const wxString &text)
//------------------------------------------------------------------------------
wxString ScriptReadWriter::HandleComments(const wxString &text)
{
   wxString result = text + wxT("\n");
   
   wxString newLine = CrossPlatformGetLine();

   // keep adding to comment if line is blank or comment
   while (((IsComment(newLine)) || (IsBlank(newLine))) && (!reachedEndOfFile))
   {
      result += (newLine + wxT("\n"));
      newLine = CrossPlatformGetLine();
   }
   
   if (HasEllipse(newLine))
      newLine = HandleEllipsis(newLine);
   
   result += newLine;
   
   return result;
}

