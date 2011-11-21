//$Id: ScriptInterpreter.cpp 9840 2011-09-07 00:20:57Z djcinsb $
//------------------------------------------------------------------------------
//                               ScriptInterpreter
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Waka Waktola
// Created: 2006/08/25
// Copyright: (c) 2006 NASA/GSFC. All rights reserved.
//
//------------------------------------------------------------------------------
/**
 * Class implementation for the ScriptInterpreter
 */
//------------------------------------------------------------------------------

#include "ScriptInterpreter.hpp"
#include "MessageInterface.hpp"
#include "Moderator.hpp"
#include "MathParser.hpp"
#include "NoOp.hpp"
#include "CommandUtil.hpp"     // for GmatCommandUtil::GetCommandSeqString()
#include "StringUtil.hpp"      // for GmatStringUtil::
#include "TimeTypes.hpp"       // for GmatTimeUtil::FormatCurrentTime()
#include <wx/wfstream.h>
#include <wx/txtstrm.h>

// to allow object creation in command mode, such as inside ScriptEvent
//#define __ALLOW_OBJECT_CREATION_IN_COMMAND_MODE__


//#define DEBUG_READ_FIRST_PASS
//#define DEBUG_DELAYED_BLOCK
//#define DEBUG_PARSE
//#define DEBUG_PARSE_FOOTER
//#define DEBUG_SET_COMMENTS
//#define DEBUG_SCRIPT_WRITING
//#define DEBUG_SCRIPT_WRITING_PARAMETER
//#define DEBUG_SECTION_DELIMITER
//#define DEBUG_SCRIPT_WRITING_COMMANDS
//#define DBGLVL_SCRIPT_READING 1
//#define DBGLVL_GMAT_FUNCTION 1

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

ScriptInterpreter *ScriptInterpreter::instance = NULL;

//------------------------------------------------------------------------------
// ScriptInterpreter* Instance()
//------------------------------------------------------------------------------
/**
 * Accessor for the ScriptInterpreter singleton.
 * 
 * @return Pointer to the singleton.
 */
//------------------------------------------------------------------------------
ScriptInterpreter* ScriptInterpreter::Instance()
{
   if (!instance)
      instance = new ScriptInterpreter();
   return instance;
}


//------------------------------------------------------------------------------
// ScriptInterpreter()
//------------------------------------------------------------------------------
/**
 * Default constructor.
 */
//------------------------------------------------------------------------------
ScriptInterpreter::ScriptInterpreter() : Interpreter()
{
   logicalBlockCount = 0;
   functionDefined = false;
   ignoreRest = false;
   
   functionDef      = wxT("");
   functionFilename = wxT("");
   scriptFilename   = wxT("");
   currentBlock     = wxT("");
   headerComment    = wxT("");
   footerComment    = wxT("");
   
   inCommandMode = false;
   inRealCommandMode = false;
   
   // Initialize the section delimiter comment
   sectionDelimiterString.clear();
   userParameterLines.clear();
   sectionDelimiterString.push_back(wxT("\n%----------------------------------------\n"));
   sectionDelimiterString.push_back(wxT("%---------- "));
   sectionDelimiterString.push_back(wxT("\n%----------------------------------------\n"));
   
   Initialize();
}


//------------------------------------------------------------------------------
// ~ScriptInterpreter()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
ScriptInterpreter::~ScriptInterpreter()
{
}


//------------------------------------------------------------------------------
// bool Interpret()
//------------------------------------------------------------------------------
/**
 * Parses the input stream, line by line, into GMAT objects.
 * 
 * @return true if the stream parses successfully, false on failure.
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::Interpret()
{
   #if DBGLVL_SCRIPT_READING
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::Interpret() entered, Calling Initialize()\n"));
   #endif
   
   Initialize();
   
   inCommandMode = false;
   inRealCommandMode = false;
   userParameterLines.clear();
   
   // Before parsing script, check for unmatching control logic
   #if DBGLVL_SCRIPT_READING
   MessageInterface::ShowMessage(wxT("   Calling ReadFirstPass()\n"));
   #endif
   
   bool retval0 = ReadFirstPass();
   bool retval1 = false;
   bool retval2 = false;
   
   if (retval0)
   {
      retval1 = ReadScript();
      retval2 = FinalPass();
   }
   
   // Write any error messages collected
   for (UnsignedInt i=0; i<errorList.size(); i++)
      MessageInterface::ShowMessage(wxT("%d: %s\n"), i+1, errorList[i].c_str());
   
   #if DBGLVL_SCRIPT_READING
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::Interpret() Leaving retval1=%d, retval2=%d\n"),
       retval1, retval2);
   #endif
   
   return (retval1 && retval2);
}


//------------------------------------------------------------------------------
// bool Interpret(GmatCommand *inCmd, bool skipHeader, bool functionMode)
//------------------------------------------------------------------------------
/**
 * Parses and creates commands from input stream and append to input command.
 *
 * @param  inCmd  Command which appended to
 * @param  skipHeader Flag indicating first comment block is not a header (false)
 * @param  functionMode Flag indicating function mode interpretation (false)
 * @return true if the stream parses successfully, false on failure.
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::Interpret(GmatCommand *inCmd, bool skipHeader,
                                  bool functionMode)
{
   Initialize();
   
   #if DBGLVL_SCRIPT_READING
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::Interpret(%p) Entered inCmd=%s, skipHeader=%d, ")
       wxT("functionMode=%d\n"), inCmd, inCmd->GetTypeName().c_str(), skipHeader,
       functionMode);
   #endif
   
   // Since this method is called from ScriptEvent or InterpretGmatFunction,
   // set command mode to true
   inFunctionMode = functionMode;
   inCommandMode = true;
   inRealCommandMode = true;
   functionDefined = false;
   ignoreRest = false;
   
   // Before parsing script, check for unmatching control logic
   bool retval0 = ReadFirstPass();
   bool retval1 = false;
   bool retval2 = false;
   
   if (retval0)
   {
      retval1 = ReadScript(inCmd, skipHeader);

      // Call FinalPass() if not in function mode or creating command inside a ScriptEvent
      // Added to check for inCmd != NULL for Bug 2436 fix (LOJ: 2011.07.05)
      if (inFunctionMode || inCmd != NULL)
         retval2 = true;
      else if (inCmd == NULL)
      {
         #if DBGLVL_SCRIPT_READING
         MessageInterface::ShowMessage(wxT("   Callilng FinalPass()\n"));
         #endif
         retval2 = FinalPass();
      }
   }
   
   // Write any error messages collected
   for (UnsignedInt i=0; i<errorList.size(); i++)
      MessageInterface::ShowMessage(wxT("%d: %s\n"), i+1, errorList[i].c_str());
   
   #if DBGLVL_SCRIPT_READING
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::Interpret(GmatCommand) Leaving retval1=%d, retval2=%d\n"),
       retval1, retval2);
   #endif
   
   return (retval1 && retval2);
}


//------------------------------------------------------------------------------
// bool Interpret(const wxString &scriptfile)
//------------------------------------------------------------------------------
/**
 * Parses the input stream from a file into GMAT objects.
 * 
 * @param scriptfile The name of the input file.
 * 
 * @return true if the file parses successfully, false on failure.
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::Interpret(const wxString &scriptfile)
{
   bool retval = false;
   
   scriptFilename = scriptfile;   
   wxFileInputStream inFile(scriptFilename);
   wxTextInputStream inFileStream(inFile);
   inStream = &inFile;
   
   theReadWriter->SetInStream(inStream);
   retval = Interpret();
   
   inStream = NULL;
   
   return retval;
}


//------------------------------------------------------------------------------
// GmatCommand* InterpretGmatFunction(const wxString fileName)
//------------------------------------------------------------------------------
/**
 * Builds function cmmand sequence by parsing the function file.
 *
 * @param <fileName>  Full path and name of the GmatFunction file.
 *
 * @return A command list that is executed to run the function.
 */
//------------------------------------------------------------------------------
GmatCommand* ScriptInterpreter::InterpretGmatFunction(const wxString &fileName)
{
   #if DBGLVL_GMAT_FUNCTION
   MessageInterface::ShowMessage
      (wxT("======================================================================\n"),
       wxT("ScriptInterpreter::InterpretGmatFunction()\n   filename = %s\n"),
       fileName.c_str());
   #endif
   
   // Check if ObjectMap and SolarSystem is set
   if (theObjectMap == NULL)
      throw InterpreterException(wxT("The Object Map is not set in the Interpreter.\n"));
   
   if (theSolarSystem == NULL)
      throw InterpreterException(wxT("The Solar System is not set in the Interpreter.\n"));
   
   wxString msg;
   if (fileName == wxT(""))
      msg = wxT("The GMATFunction file name is empty.\n");
   
   if (currentFunction == NULL)
      msg = wxT("The GMATFunction pointer is NULL.\n");
   
   // We don't want to continue if error found in the function file,
   // so set continueOnError to false
   continueOnError = false;
   if (!CheckFunctionDefinition(fileName, currentFunction))
   {
      #if DBGLVL_GMAT_FUNCTION
      MessageInterface::ShowMessage
         (wxT("ScriptInterpreter::InterpretGmatFunction() returning NULL, failed to ")
          wxT("CheckFunctionDefinition()\n"));
      #endif
      return NULL;
   }
   
   // Now function file is ready to parse
   functionFilename = fileName;
   continueOnError = true;
   bool retval = false;
   wxFileInputStream funcFile(fileName);
   wxTextInputStream funcFileStream(funcFile);
   SetInStream(&funcFile);
   GmatCommand *noOp = new NoOp;
   #ifdef DEBUG_MEMORY
   MemoryTracker::Instance()->Add
      (noOp, wxT("NoOp"), wxT("ScriptInterpreter::InterpretGmatFunction()"), wxT("*noOp = new NoOp"));
   #endif
   #if DBGLVL_GMAT_FUNCTION
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::InterpretGmatFunction() Create <%p>NoOp\n"), noOp);
   #endif
   
   // Set build function definition flag
   hasFunctionDefinition = true;
   currentFunction->SetScriptErrorFound(false);
   
   // Clear temporary object names which currently holding MatlabFunction names
   ClearTempObjectNames();
   
   // We don't want parse first comment as header, so set skipHeader to true.
   // Set function mode to true
   retval = Interpret(noOp, true, true);
   
   // Set error found to function (loj: 2008.09.09)
   // Sandbox should check error flag before interpreting Function.
   if (retval)
      currentFunction->SetScriptErrorFound(false);
   else
      currentFunction->SetScriptErrorFound(true);
   
   
   // Reset function mode and current function
   inFunctionMode = false;
   hasFunctionDefinition = false;
   currentFunction = NULL;
   
   #if DBGLVL_GMAT_FUNCTION > 0
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::InterpretGmatFunction() retval=%d\n"), retval);
   #endif

   // Just return noOP for now
   if (retval)
   {
      #if DBGLVL_GMAT_FUNCTION > 0
      MessageInterface::ShowMessage
         (wxT("ScriptInterpreter::InterpretGmatFunction() returning <%p><NoOp>\n"), noOp);
      #endif
      
      #if DBGLVL_GMAT_FUNCTION > 1
      wxString fcsStr = GmatCommandUtil::GetCommandSeqString(noOp, true, true);
      MessageInterface::ShowMessage(wxT("---------- FCS of '%s'\n"), fileName.c_str());
      MessageInterface::ShowMessage(fcsStr); //Notes: Do not use %s for command string
      #endif
      
      return noOp;
   }
   else
   {
      #if DBGLVL_GMAT_FUNCTION > 0
      MessageInterface::ShowMessage
         (wxT("ScriptInterpreter::InterpretGmatFunction() returning NULL\n"));
      #endif
      delete noOp;
      return NULL;
   }
}


//------------------------------------------------------------------------------
// GmatCommand* InterpretGmatFunction(Function *funct)
//------------------------------------------------------------------------------
/**
 * Reads a GMATFunction file and builds the corresponding command stream.
 * 
 * @param <funct> The GmatFunction pointer
 *
 * @return The head of the generated command list.
 */
//------------------------------------------------------------------------------
GmatCommand* ScriptInterpreter::InterpretGmatFunction(Function *funct)
{
   if (funct == NULL)
      return NULL;
   
   wxString fileName = funct->GetStringParameter(wxT("FunctionPath"));
   
   #if DBGLVL_GMAT_FUNCTION
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::InterpretGmatFunction() function=%p\n   ")
       wxT("filename = %s\n"), funct, fileName.c_str());
   #endif
   
   // Set current function
   SetFunction(funct);
   
   #if DBGLVL_GMAT_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   currentFunction set to <%p>\n"), currentFunction);
   #endif
   
   return InterpretGmatFunction(fileName);
   
}


//------------------------------------------------------------------------------
// bool Build()
//------------------------------------------------------------------------------
/**
 * Writes the currently configured data to an output stream.
 * 
 * @return true if the file parses successfully, false on failure.
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::Build(Gmat::WriteMode mode)
{
   if (!initialized)
      Initialize();

   // set configured object map first
   SetConfiguredObjectMap();
   return WriteScript(mode);
}


//------------------------------------------------------------------------------
// bool Build(const wxString &scriptfile)
//------------------------------------------------------------------------------
/**
 * Writes the currently configured data to a file.
 * 
 * @param scriptfile Name of the output file.
 * 
 * @return true if the file parses successfully, false on failure.
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::Build(const wxString &scriptfile, Gmat::WriteMode mode)
{
   bool retval = false;
   
   if (scriptfile != wxT(""))
      scriptFilename = scriptfile;
   
   wxFileOutputStream outFile(scriptFilename);
   wxTextOutputStream outFileStream(outFile);
   outStream = &outFile;
   
   theReadWriter->SetOutStream(outStream);
   retval = Build(mode);
   
   outStream = NULL;
   
   return retval;
}


//------------------------------------------------------------------------------
// bool SetInStream(wxInputStream *str)
//------------------------------------------------------------------------------
/**
 * Defines the input stream that gets interpreted.
 * 
 * @param str The input stream.
 * 
 * @return true on success, false on failure.  (Currently always returns true.)
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::SetInStream(wxInputStream *str)
{
   #if DBGLVL_SCRIPT_READING
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::SetInStream() entered str=<%p>\n"), str);
   #endif
   
   inStream = str;
   theReadWriter->SetInStream(inStream);
   return true;
}


//------------------------------------------------------------------------------
// bool SetOutStream(wxOutputStream *str)
//------------------------------------------------------------------------------
/**
 * Defines the output stream for writing serialized output.
 * 
 * @param str The output stream.
 * 
 * @return true on success, false on failure.  (Currently always returns true.)
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::SetOutStream(wxOutputStream *str)
{
   outStream = str;
   theReadWriter->SetOutStream(outStream);
   return true;
}


//------------------------------------------------------------------------------
// bool ReadFirstPass()
//------------------------------------------------------------------------------
/**
 * Reads only contol logic command lines from the input stream and checks for
 * unmatching End
 * 
 * @return true if the file passes checking, false on failure.
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::ReadFirstPass()
{
   #ifdef DEBUG_READ_FIRST_PASS
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::ReadFirstPass() entered, inStream=<%p>\n"), inStream);
   #endif
   
   // Make sure inStream is set
   if (inStream == NULL)
   {
      MessageInterface::ShowMessage
         (wxT("**** ERROR **** ScriptInterpreter::ReadFirstPass() input stream is NULL"));
      return false;
   }
   
   wxChar ch;
   bool reachedEndOfFile = false;
   wxString line, newLine, type;
   StringArray controlLines;
   IntegerArray lineNumbers;
   Integer charCounter = -1;
   Integer lineCounter = 1;
   
   while (!reachedEndOfFile)
   {
      line = wxT("");
      
      charCounter++;
      inStream->SeekI(charCounter, wxFromStart);
      
      while ((ch = inStream->Peek()) != wxT('\r') && ch != wxT('\n') && inStream->LastRead() != 0)
      {
         line += ch;
         charCounter++;
         inStream->SeekI(charCounter, wxFromStart);
      }
      
      newLine = GmatStringUtil::Trim(line, GmatStringUtil::BOTH, true);
      
      // Skip blank or comment line
      if (newLine != wxT("") && newLine[0] != wxT('%'))
      {         
         // Remove ending % or ;
         wxString::size_type index;
         index = newLine.find_first_of(wxT("%;"));
         if (index != newLine.npos)
         {
            newLine = newLine.substr(0, index);
         }
         
         #ifdef DEBUG_READ_FIRST_PASS
         MessageInterface::ShowMessage(wxT("newLine=%s\n"), newLine.c_str());
         #endif
         
         type = newLine;
         // Grap only control command part from the line
         // ex) While var1 == var2, If var1 > 5
         index = newLine.find_first_of(wxT(" \t"));
         if (index != newLine.npos)
         {
            type = newLine.substr(0, index);
            if (type[index-1] == wxT(';'))
               type = type.substr(0, index-1);         
         }
         
         if (type != wxT("") && IsBranchCommand(type))
         {
            lineNumbers.push_back(lineCounter);
            controlLines.push_back(type);
         }
      }
      
      if (inStream->LastRead() == 0)
         break;
      
      if (ch == wxT('\r') || ch == wxT('\n'))
      {
         lineCounter++;
         inStream->SeekI(charCounter+1, wxFromStart);
         // Why is line number incorrect for some script files?
         if (inStream->Peek() == wxT('\n'))
            charCounter++;
      }
   }
   
   // Clear staus flags first and then move pointer to beginning
//   inStream->Clear();
   inStream->SeekI(wxFromStart);
   
   #ifdef DEBUG_READ_FIRST_PASS
   for (UnsignedInt i=0; i<lineNumbers.size(); i++)
      MessageInterface::ShowMessage
         (wxT("     %d: %s\n"), lineNumbers[i], controlLines[i].c_str());
   #endif
   
   // Check for unbalaced branch command Begin/End
   bool retval = CheckBranchCommands(lineNumbers, controlLines);
   
   #ifdef DEBUG_READ_FIRST_PASS
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::ReadFirstPass() returning %d\n"), retval);
   #endif
   
   return retval;
   
}


//------------------------------------------------------------------------------
// bool ReadScript(GmatCommand *inCmd, bool skipHeader = false)
//------------------------------------------------------------------------------
/**
 * Reads a script from the input stream line by line and parses it.
 *
 * @param *inCmd The input command to append new commands to
 * @param  skipHeader Flag indicating first comment block is not a header(false)
 * @return true if the file parses successfully, false on failure.
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::ReadScript(GmatCommand *inCmd, bool skipHeader)
{
   bool retval1 = true;
   
   if (inStream->CanRead() == false || inStream->Eof())
   {
      MessageInterface::ShowMessage
         (wxT("==> ScriptInterpreter::ReadScript() inStream failed or eof reached, ")
          wxT("so returning false\n"));
      return false;
   }
   
   // Empty header & footer comment data members
   headerComment = wxT("");
   footerComment = wxT("");
   
   currentBlock = wxT("");
   
   logicalBlockCount = 0;
   theTextParser.Reset();
   
   initialized = false;
   Initialize();
   
   if (inFunctionMode)
      inCommandMode = true;
   
   // Read header comment and first logical block.
   // If input command is NULL, this method is called from GUI to interpret
   // BeginScript block. We want to ignore header comment if parsing script event.
   wxString tempHeader;
   theReadWriter->ReadFirstBlock(tempHeader, currentBlock, skipHeader);
   if (inCmd == NULL)
      headerComment = tempHeader;
   
   #if DBGLVL_SCRIPT_READING
   MessageInterface::ShowMessage
      (wxT("===> currentBlock:\n<<<%s>>>\n"), currentBlock.c_str());
   MessageInterface::ShowMessage
      (wxT("===> headerComment:\n<<<%s>>>\n"), headerComment.c_str());
   #endif
   
   while (currentBlock != wxT(""))
   {
      try
      {
         #if DBGLVL_SCRIPT_READING
         MessageInterface::ShowMessage(wxT("==========> Calling EvaluateBlock()\n"));
         #endif
         
         currentBlockType = theTextParser.EvaluateBlock(currentBlock);
         
         #if DBGLVL_SCRIPT_READING > 1
         MessageInterface::ShowMessage
            (wxT("===> after EvaluateBlock() currentBlock:\n<<<%s>>>\n"), currentBlock.c_str());
         #endif
         
         #if DBGLVL_SCRIPT_READING
         MessageInterface::ShowMessage
            (wxT("==========> Calling Parse() currentBlockType=%d\n"), currentBlockType);
         #endif
         
         // Keep previous retval1 value
         retval1 = Parse(inCmd) && retval1;
         
         #if DBGLVL_SCRIPT_READING > 1
         MessageInterface::ShowMessage
            (wxT("===> after Parse() currentBlock:\n<<<%s>>>\n"), currentBlock.c_str());
         MessageInterface::ShowMessage
            (wxT("===> currentBlockType:%d, retval1=%d\n"), currentBlockType, retval1);
         #endif
         
      }
      catch (BaseException &e)
      {
         // Catch exception thrown from the Command::InterpretAction()
         HandleError(e);
         retval1 = false;
      }
      
      if (!retval1 && !continueOnError)
      {
         #if DBGLVL_SCRIPT_READING
         MessageInterface::ShowMessage
            (wxT("ScriptInterpreter::ReadScript() Leaving retval1=%d, ")
             wxT("continueOnError=%d\n"), retval1, continueOnError);
         #endif
         
         return false;
      }
      
      if (ignoreRest)
         break;
      
      #if DBGLVL_SCRIPT_READING
      MessageInterface::ShowMessage(wxT("===> Read next logical block\n"));
      #endif
      
      currentBlock = theReadWriter->ReadLogicalBlock();
      
      #if DBGLVL_SCRIPT_READING
      MessageInterface::ShowMessage
         (wxT("===> currentBlock:\n<<<%s>>>\n"), currentBlock.c_str());
      #endif
   }
   
   // Parse delayed blocks here
   Integer delayedCount = delayedBlocks.size();
   bool retval2 = true;
   inCommandMode = false;
   
   #ifdef DEBUG_DELAYED_BLOCK
   MessageInterface::ShowMessage
      (wxT("===> ScriptInterpreter::ReadScript() Start parsing delayed blocks. count=%d\n"),
       delayedBlocks.size());
   #endif
   
   parsingDelayedBlock = true;
   
   for (Integer i = 0; i < delayedCount; i++)
   {
      #ifdef DEBUG_DELAYED_BLOCK
      MessageInterface::ShowMessage
         (wxT("===> delayedBlocks[%d]=%s\n"), i, delayedBlocks[i].c_str());
      #endif
      
      currentLine = delayedBlocks[i];
      lineNumber = delayedBlockLineNumbers[i];
      currentBlock = delayedBlocks[i];
      currentBlockType = theTextParser.EvaluateBlock(currentBlock);
      
      #ifdef DEBUG_DELAYED_BLOCK
      MessageInterface::ShowMessage
         (wxT("==========> Calling Parse() currentBlockType=%d\n"), currentBlockType);
      #endif
      
      // Keep previous retval1 value
      retval2 = Parse(inCmd) && retval2;
      
      #ifdef DEBUG_DELAYED_BLOCK
      MessageInterface::ShowMessage(wxT("===> delayedCount:%d, retval2=%d\n"), i, retval2);
      #endif
      
      if (!retval2 && !continueOnError)
      {
         #if DBGLVL_SCRIPT_READING
         MessageInterface::ShowMessage
            (wxT("In delayed block: Leaving retval1=%d, ")
             wxT("continueOnError=%d\n"), retval1, continueOnError);
         #endif
         
         return false;
      }
   }
   
   #if DBGLVL_SCRIPT_READING
   MessageInterface::ShowMessage
      (wxT("Leaving ReadScript() retval1=%d, retval2=%d\n"), retval1, retval2);
   #endif
   
   return (retval1 && retval2);
}


//------------------------------------------------------------------------------
// bool Parse(GmatCommand *inCmd)
//------------------------------------------------------------------------------
/**
 * Builds or configures GMAT objects based on the current line of script.
 * 
 * @return true if the file parses successfully, false on failure.
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::Parse(GmatCommand *inCmd)
{
   #ifdef DEBUG_PARSE
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::Parse() inCmd=<%p>, logicalBlock = \n<<<%s>>>\n"),
       inCmd, currentBlock.c_str());
   #endif
   
   bool retval = true;
   
   StringArray sarray = theTextParser.GetChunks();
   Integer count = sarray.size();
   
   #ifdef DEBUG_PARSE
   MessageInterface::ShowMessage
      (wxT("   currentBlockType=%d, inCommandMode=%d, inRealCommandMode=%d\n"),
       currentBlockType, inCommandMode, inRealCommandMode);
   for (UnsignedInt i=0; i<sarray.size(); i++)
      MessageInterface::ShowMessage(wxT("   sarray[%d]=%s\n"), i, sarray[i].c_str());
   #endif
   
   // check for empty chunks
   Integer emptyChunks = 0;
   for (Integer i = 0; i < count; i++)
      if (sarray[i] == wxT(""))
         emptyChunks++;
   #ifdef DEBUG_PARSE
   MessageInterface::ShowMessage
      (wxT("   emptyChunks=%d, count=%d\n"),
       emptyChunks, count);
   #endif
   
   if (emptyChunks == count)
      return true;   // ignore lines with just a semicolon
 //     return false;
   
   // actual script line
   wxString actualScript = sarray[count-1];
   
   // check for function definition line
   if (currentBlockType == Gmat::FUNCTION_BLOCK)
   {
      // Check if function already defined
      // GMAT function test criteria states:
      // 2.11 The system must only allow one function to be defined inside of a function file. 
      // 2.12 If more than one function is present in a file, a warning shall be thrown
      //      and only the first function in the file shall be used.
      if (functionDefined)
      {
         MessageInterface::PopupMessage
            (Gmat::WARNING_, wxT("*** WARNING *** There are more than one function ")
             wxT("defined in the function file \"%s\". \nOnly the first function \"%s\" ")
             wxT("will be used and \"%s\" and the rest of the file will be ignored.\n"),
             functionFilename.c_str(), functionDef.c_str(), sarray[2].c_str());
         ignoreRest = true;
         return true;
      }
      else
      {
         functionDef = sarray[2];
         
         if (BuildFunctionDefinition(sarray[count-1]))
         {
            functionDefined = true;
            return true;
         }
         else
            throw InterpreterException(wxT("Failed to interpret function definition"));
      }
   }
   
   // Decompose by block type
   StringArray chunks;
   try
   {
      chunks = theTextParser.ChunkLine();
   }
   catch (BaseException &e)
   {
      // Use exception to remove Visual C++ warning
      e.GetMessageType();

      // if in function mode, throw better message 
      if (inFunctionMode && currentFunction != NULL)
      {
         wxString funcPath = currentFunction->GetStringParameter(wxT("FunctionPath"));
         InterpreterException ex
            (wxT("In function file \"") + funcPath + wxT("\": ")
             wxT("Invalid function definition found "));
         HandleError(ex, true, false);
         return false;
      }
      else
      {
         throw;
      }
   }
   
   count = chunks.size();
   GmatBase *obj = NULL;
   
   #ifdef DEBUG_PARSE
   for (int i=0; i<count; i++)
      MessageInterface::ShowMessage(wxT("   chunks[%d]=%s\n"), i, chunks[i].c_str());
   #endif
   
   // Now go through each block type
   switch (currentBlockType)
   {
   case Gmat::COMMENT_BLOCK:
      {
         footerComment = currentBlock;
         
         #ifdef DEBUG_PARSE_FOOTER
         MessageInterface::ShowMessage(wxT("footerComment=<<<%s>>>\n"), footerComment.c_str());
         #endif
         
         // More to do here for a block of comments (See page 35)
         break;
      }
   case Gmat::DEFINITION_BLOCK:
      {
         retval = ParseDefinitionBlock(chunks, inCmd, obj);
         logicalBlockCount++;
         break;
      }
   case Gmat::COMMAND_BLOCK:
      {
         // if TextParser detected as function call
         if (theTextParser.IsFunctionCall())
         {
            #ifdef DEBUG_PARSE
            MessageInterface::ShowMessage
               (wxT("   TextParser detected as CallFunction\n"));
            #endif
            
            wxString::size_type index = actualScript.find_first_of(wxT("( "));
            wxString substr = actualScript.substr(0, index);
            
            #ifdef DEBUG_PARSE
            MessageInterface::ShowMessage
               (wxT("   Checking if '%s' is predefined non-Function object, or not ")
                wxT("yet supported ElseIf/Switch\n"),
                substr.c_str());
            #endif
            
            if (substr.find(wxT("ElseIf")) != substr.npos ||
                substr.find(wxT("Switch")) != substr.npos)
            {
               InterpreterException ex(wxT("\"") + substr + wxT("\" is not yet supported"));
               HandleError(ex);
               return false;
            }
            
            obj = FindObject(substr);
            if (obj != NULL && !(obj->IsOfType(wxT("Function"))))
            {
               InterpreterException ex;
               if (actualScript.find_first_of(wxT("(")) != actualScript.npos)
               {
                  ex.SetDetails(wxT("The object named \"") + substr + wxT("\" of type \"") +
                                obj->GetTypeName() + wxT("\" cannot be a Function name"));
               }
               else
               {
                  ex.SetDetails(wxT("The object named \"") + substr + wxT("\" of type \"") +
                                obj->GetTypeName() + wxT("\" is not a valid Command"));
               }
               HandleError(ex);
               return false;
            }
            
            #ifdef DEBUG_PARSE
            MessageInterface::ShowMessage
               (wxT("   About to create CallFunction of '%s'\n"), actualScript.c_str());
            #endif
            obj = (GmatBase*)CreateCommand(wxT("CallFunction"), actualScript, retval, inCmd);
            
            if (obj && retval)
            {
               // Setting comments was missing (loj: 2008.08.08)
               // Get comments and set to object
               wxString preStr = theTextParser.GetPrefaceComment();
               wxString inStr = theTextParser.GetInlineComment();
               SetComments(obj, preStr, inStr);
            }
            else
            {
               #ifdef DEBUG_PARSE
               if (obj == NULL)
                  MessageInterface::ShowMessage
                     (wxT("   *** CreateCommand() returned NULL command\n"));
               if (!retval)
                  MessageInterface::ShowMessage
                     (wxT("   *** CreateCommand() returned false\n"));
               #endif
            }
            
            break;
         }
         
         retval = ParseCommandBlock(chunks, inCmd, obj);
         logicalBlockCount++;
         break;
      }
   case Gmat::ASSIGNMENT_BLOCK:
      {
         retval = ParseAssignmentBlock(chunks, inCmd, obj);
         logicalBlockCount++;
         break;
      }
   default:
      break;
   }
   
   #ifdef DEBUG_PARSE
   MessageInterface::ShowMessage(wxT("ScriptInterpreter::Parse() retval=%d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool WriteScript()
//------------------------------------------------------------------------------
/**
 * Writes a script -- including all configured objects -- to the output stream.
 * 
 * @return true if the file parses successfully, false on failure.
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::WriteScript(Gmat::WriteMode mode)
{
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::WriteScript() entered, headerComment='%s'\n"),
       headerComment.c_str());
   #endif

   if (outStream == NULL)
      return false;
   
   //-----------------------------------
   // Header Comment
   //-----------------------------------
   //if (headerComment == wxT(""))
   if (GmatStringUtil::IsBlank(headerComment, true))
      theReadWriter->WriteText
         (wxT("%General Mission Analysis Tool(GMAT) Script\n%Created: ") +
          GmatTimeUtil::FormatCurrentTime(3) + wxT("\n\n"));
   else
      theReadWriter->WriteText(headerComment);
   
   StringArray::iterator current;
   StringArray objs;
   wxString objName;
   GmatBase *object =  NULL;
   
   //-----------------------------------
   // The Solar System
   //-----------------------------------
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found Solar Systems In Use\n"));
   #endif
   // Write if not modified by user
   if (!theSolarSystem->IsObjectCloaked())
   {
      objs.clear();
      objs.push_back(wxT("SolarSystem"));
      WriteObjects(objs, wxT("Solar System User-Modified Values"), mode);
   }
   
   //-----------------------------------
   // Celestial Bodies (for now, only user-defined or modified ones)
   //-----------------------------------
   objs = theModerator->GetListOfObjects(Gmat::CELESTIAL_BODY);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d CelestialBodys\n"), objs.size());
   #endif
   if (objs.size() > 0)
   {
      bool        foundUserDefinedBodies = false;
      bool        foundModifiedBodies    = false;
      StringArray userDefinedBodies;
      StringArray modifiedBodies;
      for (current = objs.begin(); current != objs.end(); ++current)
      {
         #ifdef DEBUG_SCRIPT_WRITING
         MessageInterface::ShowMessage(wxT("      body name = '%s'\n"), (*current).c_str());
         #endif
         
         object = FindObject(*current);
         if (object == NULL)
            throw InterpreterException(wxT("Cannot write NULL object \"") + (*current) + wxT("\""));
         
         if (!(object->IsOfType(wxT("CelestialBody"))))
            throw InterpreterException(wxT("Error writing invalid celestial body \"") + (*current) + wxT("\""));
         CelestialBody *theBody = (CelestialBody*) object;
         if (theBody->IsUserDefined())
         {
            foundUserDefinedBodies = true;
            userDefinedBodies.push_back(*current);
         }
         else if (!theBody->IsObjectCloaked())
         {
            foundModifiedBodies = true;
            modifiedBodies.push_back(*current);
         }
      }
      if (foundModifiedBodies)  
         WriteObjects(modifiedBodies, wxT("User-Modified Default Celestial Bodies"), mode);
      if (foundUserDefinedBodies) 
         WriteObjects(userDefinedBodies, wxT("User-Defined Celestial Bodies"), mode);
   }
   
   //-----------------------------------
   // Libration Points and Barycenters
   //-----------------------------------
   #ifdef DEBUG_SCRIPT_WRITING
      MessageInterface::ShowMessage(wxT("   About to ask Moderator for list of Calculated Points\n"));
   #endif
   objs = theModerator->GetListOfObjects(Gmat::CALCULATED_POINT, true);
   #ifdef DEBUG_SCRIPT_WRITING
      MessageInterface::ShowMessage(wxT("   Found %d Calculated Points\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteObjects(objs, wxT("Calculated Points"), mode);
   
   
   //-----------------------------------
   // Spacecraft
   //-----------------------------------
   objs = theModerator->GetListOfObjects(Gmat::SPACECRAFT);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Spacecraft\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteSpacecrafts(objs, mode);
   
   //-----------------------------------
   // Hardware
   //-----------------------------------
   objs = theModerator->GetListOfObjects(Gmat::HARDWARE);
   #ifdef DEBUG_SCRIPT_WRITING 
   MessageInterface::ShowMessage(wxT("   Found %d Hardware Components\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteHardwares(objs, mode);
   
   //-----------------------------------
   // Formation
   //-----------------------------------
   objs = theModerator->GetListOfObjects(Gmat::FORMATION);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Formation\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteObjects(objs, wxT("Formation"), mode);
   
   //-----------------------------------
   // Ground stations
   //-----------------------------------
   objs = theModerator->GetListOfObjects(Gmat::GROUND_STATION);
   #ifdef DEBUG_SCRIPT_WRITING
     MessageInterface::ShowMessage(wxT("   Found %d GroundStations\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteObjects(objs, wxT("GroundStations"), mode);
   
   //-----------------------------------
   // Force Model
   //-----------------------------------
   StringArray odeObjs = theModerator->GetListOfObjects(Gmat::ODE_MODEL);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Force Models\n"), objs.size());
   #endif
   WriteODEModels(odeObjs, mode);
   
   //-----------------------------------
   // Propagator
   //-----------------------------------
   objs = theModerator->GetListOfObjects(Gmat::PROP_SETUP);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Propagators\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WritePropagators(objs, wxT("Propagators"), mode, odeObjs);
   
   //-----------------------------------
   // Burn
   //-----------------------------------
   objs = theModerator->GetListOfObjects(Gmat::BURN);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Burns\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteObjects(objs, wxT("Burns"), mode);
   
   //-----------------------------------
   // Array, Variable and String
   //-----------------------------------
   objs = theModerator->GetListOfObjects(Gmat::PARAMETER);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Parameters\n"), objs.size());
   #endif
   bool foundVarsAndArrays = false;
   bool foundOtherParameter = false; // such as Create X pos; where X is Parameter name
   if (objs.size() > 0)
   {
      for (current = objs.begin(); current != objs.end(); ++current)
      {
         #ifdef DEBUG_SCRIPT_WRITING
         MessageInterface::ShowMessage(wxT("      name = '%s'\n"), (*current).c_str());
         #endif
         
         object = FindObject(*current);
         if (object == NULL)
            throw InterpreterException(wxT("Cannot write NULL object \"") + (*current) + wxT("\""));
         
         if ((object->GetTypeName() == wxT("Array")) ||
             (object->GetTypeName() == wxT("Variable")) ||
             (object->GetTypeName() == wxT("String")))
            foundVarsAndArrays = true;
         else
            foundOtherParameter = true;
      }
   }
   
   if (foundVarsAndArrays)
      WriteVariablesAndArrays(objs, mode);
   
   if (foundOtherParameter)
      WriteOtherParameters(objs, mode);
   
   //-----------------------------------
   // Coordinate System
   //-----------------------------------
   // Don't write default coordinate systems since they are automatically created
   objs = theModerator->GetListOfObjects(Gmat::COORDINATE_SYSTEM, true);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Coordinate Systems\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteObjects(objs, wxT("Coordinate Systems"), mode);
   
   //-----------------------------------
   // Measurement Data Files
   //-----------------------------------
   objs = theModerator->GetListOfObjects(Gmat::DATASTREAM);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Datastreams\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteObjects(objs, wxT("DataStreams"), mode);

   objs = theModerator->GetListOfObjects(Gmat::DATA_FILE);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Datafiles\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteObjects(objs, wxT("DataFiles"), mode);

   //---------------------------------------------
   // Measurement Models and Tracking Data/Systems
   //---------------------------------------------
   objs = theModerator->GetListOfObjects(Gmat::MEASUREMENT_MODEL);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Measurement Models\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteObjects(objs, wxT("MeasurementModels"), mode);

   objs = theModerator->GetListOfObjects(Gmat::TRACKING_DATA);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d TrackingData Objects\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteObjects(objs, wxT("TrackingData"), mode);

   objs = theModerator->GetListOfObjects(Gmat::TRACKING_SYSTEM);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Tracking Systems\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteObjects(objs, wxT("TrackingSystems"), mode);

   //-----------------------------------
   // Solver
   //-----------------------------------
   objs = theModerator->GetListOfObjects(Gmat::SOLVER);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Solvers\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteObjects(objs, wxT("Solvers"), mode);
      
   //-----------------------------------
   // Subscriber
   //-----------------------------------
   objs = theModerator->GetListOfObjects(Gmat::SUBSCRIBER);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Subscribers\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteSubscribers(objs, mode);
      
   //-----------------------------------
   // Function
   //-----------------------------------
   objs = theModerator->GetListOfObjects(Gmat::FUNCTION);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Functions\n"), objs.size());
   #endif
   if (objs.size() > 0)
      WriteObjects(objs, wxT("Functions"), mode);
   
   //-----------------------------------
   // Command sequence
   //-----------------------------------
   WriteCommandSequence(mode);
   
   //-----------------------------------
   // Footer Comment
   //-----------------------------------
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   footerComment=\n<%s>\n"), footerComment.c_str());
   #endif
   
   if (footerComment != wxT(""))
      theReadWriter->WriteText(footerComment);
   //else
   //   theReadWriter->WriteText(wxT("\n"));
   
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::WriteScript() leaving\n"));
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// bool ParseDefinitionBlock(const StringArray &chunks, GmatCommand *inCmd,
//                           GmatBase *obj)
//------------------------------------------------------------------------------
/*
 * Parses the definition block.
 *
 * @param  chunks  Input string array to be used in the parsing
 * @param  inCmd   Input command pointer to be used to append the new command
 * @param  obj     Ouput object pointer if created
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::ParseDefinitionBlock(const StringArray &chunks,
                                             GmatCommand *inCmd, GmatBase *obj)
{
   #ifdef DEBUG_PARSE
   WriteStringArray(wxT("ParseDefinitionBlock()"), wxT(""), chunks);
   #endif
   
   // Get comments
   wxString preStr = theTextParser.GetPrefaceComment();
   wxString inStr = theTextParser.GetInlineComment();
   
   Integer count = chunks.size();
   bool retval = true;
   
   // If object creation is not allowed in command mode
   #ifndef __ALLOW_OBJECT_CREATION_IN_COMMAND_MODE__
   if (inRealCommandMode)
   {
      if (!inFunctionMode)
      {
         InterpreterException ex
            (wxT("GMAT currently requires that all object are created before the ")
             wxT("mission sequence begins"));
         HandleError(ex, true, true);
         return true; // just a warning, so return true
      }
   }
   #endif
   
   if (count < 3)
   {
      InterpreterException ex(wxT("Missing parameter creating object for"));
      HandleError(ex);
      return false;
   }
   
   wxString type = chunks[1];
   StringArray names;
   if (type == wxT("Array"))
   {
      if (chunks[2].find(wxT('[')) == chunks[2].npos)
         throw InterpreterException(wxT("Opening bracket \"[\" not found"));
      
      names = theTextParser.Decompose(chunks[2], wxT("[]"));
   }
   else
   {
      names = theTextParser.Decompose(chunks[2], wxT("()"));
   }
   
   count = names.size();
   
   // Special case for Propagator
   if (type == wxT("Propagator"))
      type = wxT("PropSetup");
   
   // Handle creating objects in function mode
   if (inFunctionMode)
   {
      wxString desc = chunks[1] + wxT(" ") + chunks[2];
      obj = (GmatBase*)CreateCommand(chunks[0], desc, retval, inCmd);
   }
   else
   {
      Integer objCounter = 0;
      for (Integer i = 0; i < count; i++)
      {
         obj = CreateObject(type, names[i]);
            
         if (obj == NULL)
         {
            InterpreterException ex
               (wxT("Cannot create an object \"") + names[i] + wxT("\". The \"") +
                type + wxT("\" is unknown object type"));
            HandleError(ex);
            return false;
         }
         
         objCounter++;     
         obj->FinalizeCreation();
         
         SetComments(obj, preStr, inStr);
      }
      
      // if not all objectes are created, return false
      if (objCounter < count)
      {
         InterpreterException ex(wxT("All objects are not created"));
         HandleError(ex);
         return false;
      }
   }

   return retval;
}


//------------------------------------------------------------------------------
// bool ParseCommandBlock(const StringArray &chunks, GmatCommand *inCmd,
//                        GmatBase *obj)
//------------------------------------------------------------------------------
/*
 * Parses the command block.
 *
 * @param  chunks  Input string array to be used in the parsing
 * @param  inCmd   Input command pointer to be used to append the new command
 * @param  obj     Ouput object pointer if created
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::ParseCommandBlock(const StringArray &chunks,
                                          GmatCommand *inCmd, GmatBase *obj)
{
   #ifdef DEBUG_PARSE
   WriteStringArray(wxT("ParseCommandBlock()"), wxT(""), chunks);
   #endif
   
   // Get comments
   wxString preStr = theTextParser.GetPrefaceComment();
   wxString inStr = theTextParser.GetInlineComment();
   
   Integer count = chunks.size();
   bool retval = true;
   inCommandMode = true;
   inRealCommandMode = true;
   bool isFunction = false;
   
   // A call function doesn't have to have arguments so this code gets a list
   // of functions and checks to see if chunks[0] is a function name.
   // Only Matlab function is required to create before the use in the call function.
   StringArray functionNames = GetListOfObjects(Gmat::FUNCTION);
   
   for (Integer i=0; i<(Integer)functionNames.size(); i++)
   {
      if (functionNames[i] == chunks[0])
      {
         isFunction = true;
         break;
      }
   }
   
   if (count < 2)
   {
      // check for one-word commands
      if (IsOneWordCommand(chunks[0]))
      {
         obj = (GmatBase*)CreateCommand(chunks[0], wxT(""), retval, inCmd);
      }
      else if (isFunction)
      {
         #ifdef DEBUG_PARSE
         MessageInterface::ShowMessage(wxT("   Creating CallFunction\n"));
         #endif
         
         obj = (GmatBase*)CreateCommand(wxT("CallFunction"), chunks[0], retval, inCmd);
      }
      else
      {
         InterpreterException ex
            (wxT("Missing parameter with \"") + chunks[0] + wxT("\" command"));
         HandleError(ex);
         return false;
      }
   }
   else
   {
      // check for extra text at the end of one-word commands
      if (IsOneWordCommand(chunks[0]))
      {
         // If second item is not a command name then throw a exception
         if (!GmatStringUtil::IsEnclosedWith(chunks[1], wxT("'")))
         {
            InterpreterException ex
               (wxT("Unexpected text after \"") + chunks[0] + wxT("\" command"));
            HandleError(ex);
            return false;
         }
      }
      
      // check for .. in the command block
      if (chunks[1].find(wxT("..")) != currentBlock.npos)
      {
         // allow relative path using ..
         if (chunks[1].find(wxT("../")) == currentBlock.npos &&
             chunks[1].find(wxT("..\\")) == currentBlock.npos)
         {
            InterpreterException ex(wxT("Found invalid syntax \"..\""));
            HandleError(ex);
            return false;
         }
      }
      
      obj = (GmatBase*)CreateCommand(chunks[0], chunks[1], retval, inCmd);
   }
   
   // if in function mode just check for retval, since function definition
   // line will not create a command
   if (inFunctionMode && retval)
   {
      return true;
   }
   else
   {
      if (obj == NULL)
         return false;
   }
   
   SetComments(obj, preStr, inStr);
   return retval;
}


//------------------------------------------------------------------------------
// bool ParseAssignmentBlock(const StringArray &chunks, GmatCommand *inCmd,
//                           GmatBase *obj)
//------------------------------------------------------------------------------
/*
 * Parses the assignment block. The assignment block has equal sign, so it
 * can be either assignment or function call.
 *
 * @param  chunks  Input string array to be used in the parsing
 * @param  inCmd   Input command pointer to be used to append the new command
 * @param  obj     Ouput object pointer if created
 */
//------------------------------------------------------------------------------
bool ScriptInterpreter::ParseAssignmentBlock(const StringArray &chunks,
                                             GmatCommand *inCmd, GmatBase *obj)
{
   #ifdef DEBUG_PARSE
   MessageInterface::ShowMessage
      (wxT("ParseAssignmentBlock() entered, inCmd=<%p>, obj=<%p>\n"), inCmd, obj);
   WriteStringArray(wxT("ParseAssignmentBlock()"), wxT(""), chunks);
   #endif
   
   Integer count = chunks.size();
   bool retval = true;
   
   // Get comments
   wxString preStr = theTextParser.GetPrefaceComment();
   wxString inStr = theTextParser.GetInlineComment();
   
   // check for .. in the command block
   if (chunks[0].find(wxT("..")) != chunks[0].npos ||
       chunks[1].find(wxT("..")) != chunks[1].npos)
   {
      // allow relative path using ..
      if (chunks[1].find(wxT("../")) == currentBlock.npos &&
          chunks[1].find(wxT("..\\")) == currentBlock.npos)
      {
         InterpreterException ex(wxT("Found invalid syntax \"..\""));
         HandleError(ex);
         return false;
      }
   }
   
   // check for missing RHS
   if (count < 2)
   {
      InterpreterException ex(wxT("Missing parameter assigning object for: "));
      HandleError(ex);
      return false;
   }
   
   wxString lhs = chunks[0];
   wxString rhs = chunks[1];
   
   // check for ElseIf, since it is not yet supported
   if (lhs.find(wxT("ElseIf ")) != lhs.npos ||
       rhs.find(wxT("ElseIf ")) != rhs.npos)
   {
      InterpreterException ex(wxT("\"ElseIf\" is not yet supported"));
      HandleError(ex);
      return false;
   }
   
   // if RHS is not enclosed with single quotes, check for unexpected symbols or space
   if (!GmatStringUtil::IsEnclosedWith(rhs, wxT("'")))
   {
      if (lhs.find_first_of(wxT("=~<>")) != lhs.npos ||
          rhs.find_first_of(wxT("=~<>")) != rhs.npos)
      {
         wxString cmd;
         InterpreterException ex;
         
         if (lhs == wxT(""))
         {
            cmd = rhs.substr(0, rhs.find_first_of(wxT(" ")));
            if (!IsCommandType(cmd))
               ex.SetDetails(wxT("\"") + cmd + wxT("\" is not a valid Command"));
         }
         else
         {
            wxString::size_type index = lhs.find_first_of(wxT(" "));
            if (index != lhs.npos)
            {
               cmd = lhs.substr(0, index);
               if (!IsCommandType(cmd))
                  ex.SetDetails(wxT("\"") + cmd + wxT("\" is not a valid Command"));
            }
            else
            {
               if (!IsCommandType(cmd) && lhs.find(wxT(".")) == lhs.npos)
                  ex.SetDetails(wxT("\"") + cmd + wxT("\" is not a valid Command"));
               else
                  ex.SetDetails(wxT("\"") + rhs + wxT("\" is not a valid RHS of Assignment"));
            }
         }
         
         HandleError(ex);
         return false;
         
      }
   }
   
   
   // Check for GmatGlobal setting
   if (lhs.find(wxT("GmatGlobal.")) != wxString::npos)
   {
      StringArray lhsParts = theTextParser.SeparateDots(lhs);
      if (lhsParts[1] == wxT("LogFile"))
      {
         #ifdef DEBUG_PARSE
         MessageInterface::ShowMessage
            (wxT("   Found Global.LogFile, so calling MI::SetLogFile(%s)\n"),
             rhs.c_str());
         #endif
         
         wxString fname = rhs;
         fname = GmatStringUtil::RemoveEnclosingString(fname, wxT("'"));
         MessageInterface::SetLogFile(fname);
         return true;
      }
   }
   
   GmatBase *owner = NULL;
   wxString attrStr = wxT(""); 
   wxString attrInLineStr = wxT(""); 
   Integer paramID = -1;
   Gmat::ParameterType paramType;
   
   #ifdef DEBUG_PARSE
   MessageInterface::ShowMessage(wxT("   before check, inCommandMode=%d\n"), inCommandMode);
   #endif
   
   if (!inCommandMode)
   {
      // check for math operators/functions
      MathParser mp = MathParser();
      
      try
      {
         if (mp.IsEquation(rhs, true))
         {
            #ifdef DEBUG_PARSE
            MessageInterface::ShowMessage(wxT("   It is a math equation\n"));
            #endif
            
            // check if LHS is object.property
            if (FindPropertyID(obj, lhs, &owner, paramID, paramType))
            {
               Gmat::ParameterType paramType = obj->GetParameterType(paramID);
               // Since string can have minus sign, check it first
               if (paramType != Gmat::STRING_TYPE && paramType != Gmat::ENUMERATION_TYPE &&
                   paramType != Gmat::FILENAME_TYPE)
                  inCommandMode = true;
            }
            else
            {
               // check if LHS is a parameter
               GmatBase *tempLhsObj = FindObject(lhs);
               if (tempLhsObj && tempLhsObj->GetType() == Gmat::PARAMETER)
               {
                  Parameter *tempLhsParam = (Parameter*)tempLhsObj;
                  #ifdef DEBUG_PARSE
                  MessageInterface::ShowMessage
                     (wxT("   LHS return type = %d, Gmat::RMATRIX_TYPE = %d, ")
                      wxT("inRealCommandMode = %d\n"), tempLhsParam->GetReturnType(),
                      Gmat::RMATRIX_TYPE, inRealCommandMode);
                  #endif
                  
                  if (tempLhsParam->GetReturnType() == Gmat::REAL_TYPE ||
                      tempLhsParam->GetReturnType() == Gmat::RMATRIX_TYPE)
                  {
                     if (inRealCommandMode)
                        inCommandMode = true;
                     // If LHS is Array, it is assignment, so commented out (LOJ: 2010.09.21)
                     //else if (tempLhsParam->GetTypeName() == wxT("Array"))
                     //   inCommandMode = true;
                     else
                        inCommandMode = false;
                  }
               }
            }
         }
      }
      catch (BaseException &e)
      {
         // Use exception to remove Visual C++ warning
         e.GetMessageType();
         #ifdef DEBUG_PARSE
         MessageInterface::ShowMessage(e.GetFullMessage());
         #endif
      }
   }
   
   
   #ifdef DEBUG_PARSE
   MessageInterface::ShowMessage
      (wxT("    after check, inCommandMode=%d, inFunctionMode=%d\n"), inCommandMode, inFunctionMode);
   #endif
   
   bool createAssignment = true;
   
   if (inCommandMode)
   {
      // If LHS is CoordinateSystem property or Subscriber Call MakeAssignment.
      // Some scripts mixed with definitions and commands
      StringArray parts = theTextParser.SeparateDots(lhs);
      
      // If in function mode, always create Assignment command
      if (!inFunctionMode)
      {
         if (parts.size() > 1)
         {
            GmatBase *tempObj = FindObject(parts[0]);
            if ((tempObj) &&
                (tempObj->GetType() == Gmat::COORDINATE_SYSTEM ||
                 (!inRealCommandMode && tempObj->GetType() == Gmat::SUBSCRIBER)))
               createAssignment = false;
         }
      }
   }
   else
   {
      // Check for the same Variable name on both LHS and RHS, (loj: 2008.08.06)
      // such as Var = Var + 1, it must be Assignment command
      
      GmatBase *lhsObj = FindObject(lhs);
      if (lhsObj != NULL && lhsObj->IsOfType(wxT("Variable")))
      {
         StringArray varNames = GmatStringUtil::GetVarNames(rhs);
         createAssignment = false; // Forgot to set to false (loj: 2008.08.08)
         for (UnsignedInt i=0; i<varNames.size(); i++)
         {
            if (varNames[i] == lhs)
            {
               createAssignment = true;
               break;
            }
         }
      }
      else
         createAssignment = false;
   }
   
   
   if (createAssignment)
      obj = (GmatBase*)CreateAssignmentCommand(lhs, rhs, retval, inCmd);
   else
   {
      obj = MakeAssignment(lhs, rhs);

      // Save script if lhs is Variable, Array, and String so those can be
      // written out as they are read
      GmatBase *lhsObj = FindObject(lhs);
      if (lhsObj != NULL && (lhsObj->IsOfType(wxT("Variable")) || lhsObj->IsOfType(wxT("Array")) ||
                             lhsObj->IsOfType(wxT("String"))))
         userParameterLines.push_back(preStr + lhs + wxT(" = ") + rhs + inStr);
   }
   
   if (obj == NULL)
   {
      #ifdef DEBUG_PARSE
      MessageInterface::ShowMessage
         (wxT("   obj is NULL, and %singoring the error, so returning %s\n"),
          ignoreError ? wxT("") : wxT("NOT "), ignoreError ? wxT("true") : wxT("false"));
      #endif
      if (ignoreError)
         return true;
      else
         return false;
   }
   
   // paramID will be assigned from call to Interpreter::FindPropertyID()
   if ( FindPropertyID(obj, lhs, &owner, paramID, paramType) )
   {
      attrStr = preStr;
      attrInLineStr = inStr;
      
      if (attrStr != wxT(""))
         owner->SetAttributeCommentLine(paramID, attrStr);
      
      if (attrInLineStr != wxT(""))
         owner->SetInlineAttributeComment(paramID, attrInLineStr);
      
      //Reset
      attrStr = wxT(""); 
      attrInLineStr = wxT(""); 
      paramID = -1;
   }
   else
   {
      SetComments(obj, preStr, inStr);
   }
   
   #ifdef DEBUG_PARSE
   MessageInterface::ShowMessage(wxT("ParseAssignmentBlock() returning %d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool IsOneWordCommand(const wxString &str)
//------------------------------------------------------------------------------
bool ScriptInterpreter::IsOneWordCommand(const wxString &str)
{
   // Note: The interpreter really should ask the command this!
   // but this information is needed before a command is created
   bool retval = false;
   
   if ((str.find(wxT("End"))                  != str.npos  &&
        str.find(wxT("EndFiniteBurn"))        == str.npos) ||
       (str.find(wxT("BeginScript"))          != str.npos) ||
       (str.find(wxT("NoOp"))                 != str.npos) ||
       (str.find(wxT("BeginMissionSequence")) != str.npos) ||
       (str.find(wxT("Else"))                 != str.npos  &&
        str.find(wxT("ElseIf"))               == str.npos) ||
       (str.find(wxT("Stop"))                 != str.npos))
   {
      retval = true;
   }

   if (theModerator->IsSequenceStarter(str))
      retval = true;
   
   #ifdef DEBUG_ONE_WORD_COMMAND
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::IsOneWordCommand() str='%s' returning %s\n"),
       str.c_str(), retval ? wxT("true") : wxT("false"));
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// void SetComments(GmatBase *obj, const wxString &preStr,
//                  const wxString &inStr, bool isAttributeComment)
//------------------------------------------------------------------------------
void ScriptInterpreter::SetComments(GmatBase *obj, const wxString &preStr,
                                    const wxString &inStr)
{
   #ifdef DEBUG_SET_COMMENTS
   MessageInterface::ShowMessage
      (wxT("ScriptInterpreter::SetComments() %s<%s>\n   preStr=%s\n    inStr=%s\n"),
       obj->GetTypeName().c_str(), obj->GetName().c_str(), preStr.c_str(),
       inStr.c_str());
   #endif
   
   // Preseve blank lines if command
   if (obj->GetType() == Gmat::COMMAND)
   {
      if (preStr != wxT(""))
         obj->SetCommentLine(preStr);
   }
   else
   {
      // If comment has only blank space or lines, ignore
      if (!GmatStringUtil::IsBlank(preStr, true))
      {
         // Handle preface comment for Parameters separately since there are
         // comments from Create line and Initialization line
         if (obj->GetType() == Gmat::PARAMETER)
            ((Parameter*)obj)->SetCommentLine(preStr);
         else
            obj->SetCommentLine(preStr);
      }
   }
   
   if (inStr != wxT(""))
      obj->SetInlineComment(inStr);
}


//------------------------------------------------------------------------------
// void WriteSectionDelimiter(const GmatBase *firstObj,
//                            const wxString &objDesc, bool forceWriting = false)
//------------------------------------------------------------------------------
void ScriptInterpreter::WriteSectionDelimiter(const GmatBase *firstObj,
                                              const wxString &objDesc,
                                              bool forceWriting)
{
   if (firstObj == NULL)
      return;
   
   wxString comment = firstObj->GetCommentLine();
   
   #ifdef DEBUG_SECTION_DELIMITER
   MessageInterface::ShowMessage
      (wxT("WriteSectionDelimiter() PrefaceComment of %s=<%s>\n"),
       firstObj->GetName().c_str(), comment.c_str());
   #endif
   
   // Write if section delimiter not found
   if (comment.find(sectionDelimiterString[0]) == comment.npos || forceWriting)
   {
      theReadWriter->WriteText(sectionDelimiterString[0]);
      theReadWriter->WriteText(sectionDelimiterString[1] + objDesc);
      theReadWriter->WriteText(sectionDelimiterString[2]);
   }
}


//------------------------------------------------------------------------------
// void WriteSectionDelimiter(const wxString &firstObj,
//                            const wxString &objDesc, bool forceWriting = false)
//------------------------------------------------------------------------------
void ScriptInterpreter::WriteSectionDelimiter(const wxString &firstObj,
                                              const wxString &objDesc,
                                              bool forceWriting)
{
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage
      (wxT("WriteSectionDelimiter() entered, firstObj='%s', objDesc='%s'\n"),
       firstObj.c_str(), objDesc.c_str());
   #endif
   
   GmatBase *object;
   object = FindObject(firstObj);
   if (object == NULL)
      return;
   
   WriteSectionDelimiter(object, objDesc, forceWriting);
}


//------------------------------------------------------------------------------
// void WriteObjects(StringArray &objs, const wxString &objDesc,
//                   Gmat::WriteMode mod)
//------------------------------------------------------------------------------
/*
 * This method writes given object.
 */
//------------------------------------------------------------------------------
void ScriptInterpreter::WriteObjects(StringArray &objs, const wxString &objDesc,
                                     Gmat::WriteMode mode)
{
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage
      (wxT("SI::WriteObjects() entered, objs.size=%d, objDesc='%s', mode=%d\n"),
       objs.size(), objDesc.c_str(), mode);
   #endif
   
   if (objs.empty())
      return;
   
   StringArray::iterator current;
   GmatBase *object =  NULL;
   
   WriteSectionDelimiter(objs[0], objDesc);
   
   for (current = objs.begin(); current != objs.end(); ++current)
   {
      object = FindObject(*current);
      if (object != NULL)
      {
         if (object->GetCommentLine() == wxT(""))
            theReadWriter->WriteText(wxT("\n"));
         
         theReadWriter->WriteText(object->GetGeneratingString(mode));
      }
   }
   
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage
      (wxT("SI::WriteObjects() returning for '%s'\n"), objDesc.c_str());
   #endif
}


//------------------------------------------------------------------------------
// void WriteODEModels(StringArray &objs, Gmat::WriteMode mod)
//------------------------------------------------------------------------------
/*
 * This method writes ODEModel objects.
 */
//------------------------------------------------------------------------------
void ScriptInterpreter::WriteODEModels(StringArray &objs, Gmat::WriteMode mode)
{
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage
      (wxT("SI::WriteODEModels() entered, objs.size=%d\n"), objs.size());
   #endif
   
   StringArray propOdes;
   ObjectArray props;
   
   // Since actual ODEModel used are written from the PropSetup, check for the
   // same name first to avoid duplicate writing.
   
   StringArray propNames = theModerator->GetListOfObjects(Gmat::PROP_SETUP);
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage(wxT("   Found %d Propagators\n"), propNames.size());
   #endif
   
   for (StringArray::iterator i = propNames.begin(); i != propNames.end(); ++i)
   {
      PropSetup *ps = (PropSetup*)FindObject(*i);
      if (ps != NULL)
      {
         ODEModel *ode = ps->GetODEModel();
         if (ode != NULL)
         {
            props.push_back(ps);
            propOdes.push_back(ode->GetName());
         }
      }
   }
   
   // Make a list of configured ODEs not in PropSetups
   StringArray odes;

   // set_difference requires SORTED sets, so this does not work:
   // set_difference(objs.begin(), objs.end(), propOdes.begin(),
   //               propOdes.end(), back_inserter(odes));

   // Instead, we'll difference wxT("by hand"):
   for (UnsignedInt i = 0; i < objs.size(); ++i)
   {
      bool matchFound = false;
      for (UnsignedInt j = 0; j < propOdes.size(); ++j)
         if (objs[i] == propOdes[j])
            matchFound = true;
      if (!matchFound)
         odes.push_back(objs[i]);
   }

   
   #ifdef DEBUG_SCRIPT_WRITING
   GmatStringUtil::WriteStringArray(objs, wxT("   Input objects"), wxT("      "));
   GmatStringUtil::WriteStringArray(propOdes, wxT("   Propagator ODEs"), wxT("      "));
   GmatStringUtil::WriteStringArray(odes, wxT("   Configured ODEs"), wxT("      "));
   #endif
   // Write configured ODEModels not in PropSetups
   if (odes.size() > 0)
      WriteObjects(odes, wxT("ForceModels"), mode);
   
   // Write ODEModel from PropSetup
   if (propOdes.size() > 0)
   {
      if (odes.empty())
      {
         ///WriteSectionDelimiter(props[0], wxT("ForceModels"));
         WriteSectionDelimiter(propOdes[0], wxT("ForceModels"));
      }
      
      for (ObjectArray::iterator i = props.begin(); i != props.end(); ++i)
      {
         ODEModel *ode = ((PropSetup*)(*i))->GetODEModel();
         if (ode != NULL)
         {
            theReadWriter->WriteText(wxT("\n"));
            theReadWriter->WriteText(ode->GetGeneratingString(mode));
         }
      }
   }
}


//------------------------------------------------------------------------------
// void WritePropagators(StringArray &objs, const wxString &objDesc,
//       Gmat::WriteMode mode, const StringArray &odes)
//------------------------------------------------------------------------------
/**
 * This method writes out PropSetup objects, including ODEModels that were not
 * previously written
 *
 * @param objs The list of prop setup objects
 * @param objDesc The section opener for the script file
 * @param mode The scripting mode
 * @param odes The ODEModels that have been written already
 */
//------------------------------------------------------------------------------
void ScriptInterpreter::WritePropagators(StringArray &objs,
      const wxString &objDesc, Gmat::WriteMode mode, const StringArray &odes)
{
   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage
      (wxT("SI::WritePropagators() entered, objs.size=%d, objDesc='%s', mode=%d\n"),
       objs.size(), objDesc.c_str(), mode);
   #endif

   if (objs.empty())
      return;

   StringArray::iterator current;
   GmatBase *object =  NULL;

   WriteSectionDelimiter(objs[0], objDesc);

   for (current = objs.begin(); current != objs.end(); ++current)
   {
      object = FindObject(*current);
      if (object != NULL)
      {
         if (object->GetCommentLine() == wxT(""))
            theReadWriter->WriteText(wxT("\n"));

         if (!object->IsOfType(Gmat::PROP_SETUP))
            throw InterpreterException(wxT("In ScriptInterpreter::WritePropagators,")
                  wxT(" the object ") + (*current) + wxT(" should be a PropSetup, but ")
                  wxT("it is a ") + object->GetTypeName());

         PropSetup *prop = (PropSetup*)object;
         prop->TakeAction(wxT("ExcludeODEModel"));  // WriteODEModels wrote them all
         theReadWriter->WriteText(object->GetGeneratingString(mode));
         prop->TakeAction(wxT("IncludeODEModel"));
      }
   }

   #ifdef DEBUG_SCRIPT_WRITING
   MessageInterface::ShowMessage
      (wxT("SI::WritePropagators() returning for '%s'\n"), objDesc.c_str());
   #endif
}


//------------------------------------------------------------------------------
// void WriteSpacecrafts(StringArray &objs, Gmat::WriteMode mod)
//------------------------------------------------------------------------------
/*
 * This method writes Spacecraft objects.
 */
//------------------------------------------------------------------------------
void ScriptInterpreter::WriteSpacecrafts(StringArray &objs, Gmat::WriteMode mode)
{
   StringArray::iterator current;
   GmatBase *object =  NULL;
   
   WriteSectionDelimiter(objs[0], wxT("Spacecraft"));
   
   for (current = objs.begin(); current != objs.end(); ++current)
   {
      // I cannot recall why we need to initialize spacecraft here.
      // We may not need this any more, so set to 0 for now (LOJ: 2009.09.24)
      //==============================================================
      #if 0
      //==============================================================
      // Setup the coordinate systems on Spacecraft so they can perform conversions
      CoordinateSystem *ics = theModerator->GetInternalCoordinateSystem();
      CoordinateSystem *sccs = NULL;
      object = FindObject(*current);
      if (object != NULL)
      {
         object->SetInternalCoordSystem(ics);
         sccs = (CoordinateSystem*)
            FindObject(object->GetRefObjectName(Gmat::COORDINATE_SYSTEM));
         
         if (sccs)
            object->SetRefObject(sccs, Gmat::COORDINATE_SYSTEM);
         
         object->Initialize();
      }
      //==============================================================
      #endif
      //==============================================================
      
      object = FindObject(*current);
      if (object != NULL)
      {
         if (object->GetCommentLine() == wxT(""))
            theReadWriter->WriteText(wxT("\n"));
         
         theReadWriter->WriteText(object->GetGeneratingString(mode));               
      }
   }
}


//------------------------------------------------------------------------------
// void WriteHardwares(StringArray &objs, Gmat::WriteMode mod)
//------------------------------------------------------------------------------
/*
 * This method writes Hardware objects.
 */
//------------------------------------------------------------------------------
void ScriptInterpreter::WriteHardwares(StringArray &objs, Gmat::WriteMode mode)
{
   StringArray::iterator current;
   GmatBase *object =  NULL;

   WriteSectionDelimiter(objs[0], wxT("Hardware Components"));
   
   // Hardware Tanks
   for (current = objs.begin(); current != objs.end(); ++current) 
   {
      object = FindObject(*current);
      if (object != NULL)
         if (object->GetTypeName() == wxT("FuelTank"))
         {
            if (object->GetCommentLine() == wxT(""))
               theReadWriter->WriteText(wxT("\n"));
            theReadWriter->WriteText(object->GetGeneratingString(mode));
         }
   }
   
   // Hardware Thrusters
   for (current = objs.begin(); current != objs.end(); ++current) 
   {
      object = FindObject(*current);
      if (object != NULL)
         if (object->GetTypeName() == wxT("Thruster"))
         {
            if (object->GetCommentLine() == wxT(""))
               theReadWriter->WriteText(wxT("\n"));
            theReadWriter->WriteText(object->GetGeneratingString(mode));
         }
   }

   // Other Hardware
   for (current = objs.begin(); current != objs.end(); ++current)
   {
      object = FindObject(*current);
      if (object != NULL)
         if ((object->GetTypeName() != wxT("FuelTank")) &&
                  (object->GetTypeName() != wxT("Thruster")))
         {
            if (object->GetCommentLine() == wxT(""))
               theReadWriter->WriteText(wxT("\n"));
            theReadWriter->WriteText(object->GetGeneratingString(mode));
         }
   }

}


//------------------------------------------------------------------------------
// void WriteSubscribers(StringArray &objs, Gmat::WriteMode mod)
//------------------------------------------------------------------------------
/*
 * This method writes Subscriber objects.
 */
//------------------------------------------------------------------------------
void ScriptInterpreter::WriteSubscribers(StringArray &objs, Gmat::WriteMode mode)
{
   StringArray::iterator current;
   GmatBase *object =  NULL;
   
   WriteSectionDelimiter(objs[0], wxT("Subscribers"));
   
   for (current = objs.begin(); current != objs.end(); ++current)
   {
      object = FindObject(*current);
      if (object != NULL)
      {
         if (object->GetTypeName() != wxT("TextEphemFile"))
         {
            if (object->GetCommentLine() == wxT(""))
               theReadWriter->WriteText(wxT("\n"));
            theReadWriter->WriteText(object->GetGeneratingString(mode));
         }
      }
   }
}


//------------------------------------------------------------------------------
// void WriteVariablesAndArrays(StringArray &objs, Gmat::WriteMode mod)
//------------------------------------------------------------------------------
/*
 * This method writes 10 variables and arrays per line.
 * If variable or array was initialzied (non zero), it writes after Create line.
 */
//------------------------------------------------------------------------------
void ScriptInterpreter::WriteVariablesAndArrays(StringArray &objs,
                                                Gmat::WriteMode mode)
{
   #ifdef DEBUG_SCRIPT_WRITING_PARAMETER
   MessageInterface::ShowMessage(wxT("WriteVariablesAndArrays() entered\n"));
   #endif
   
   // Updated to write Variable and Array as they appear in the script. (LOJ: 2010.09.23)
   // It uses userParameterLines which are saved during the parsing.
   
   StringArray::iterator current;
   ObjectArray arrList;
   ObjectArray varList;
   ObjectArray strList;
   ObjectArray arrWithValList;
   ObjectArray varWithValList;
   ObjectArray strWithValList;
   wxString genStr;
   GmatBase *object =  NULL;
   wxString sectionStr = wxT("Arrays, Variables, Strings");
   WriteSectionDelimiter(objs[0], sectionStr, true);
   
   //-----------------------------------------------------------------
   // Fill in proper arrays
   //-----------------------------------------------------------------
   for (current = objs.begin(); current != objs.end(); ++current)
   {
      object = FindObject(*current);
      if (object != NULL)
      {
         genStr = object->GetGeneratingString(Gmat::NO_COMMENTS);
         
         if (object->GetTypeName() == wxT("Array"))
         {
            arrList.push_back(object);
            
            // if initial value found
            if (genStr.find(wxT("=")) != genStr.npos)
               arrWithValList.push_back(object);            
         }
         else if (object->GetTypeName() == wxT("Variable"))
         {
            varList.push_back(object);
            
            // if initial value found
            if (genStr.find(wxT("=")) != genStr.npos)
            {
               wxString::size_type equalPos = genStr.find(wxT("="));
               wxString rhs = genStr.substr(equalPos + 1);
               wxString rhsComment = wxT("");
               if (rhs.find(wxT('%')) != wxString::npos)
               {
                  rhsComment = rhs.substr(rhs.find(wxT('%')));
                  rhs = rhs.substr(0, rhs.find(wxT('%')));
                  MessageInterface::ShowMessage(wxT("Variable with value and comment\n   Value: %s\n   Comment: %s\n"),
                        rhs.c_str(), rhsComment.c_str());
               }
               rhs = GmatStringUtil::Trim(rhs, GmatStringUtil::BOTH, true, true);
               Real rval;
               // check if initial value is Real number or other Variable object
               if (GmatStringUtil::ToReal(rhs, rval))
                  varWithValList.push_back(object);
            }
         }
         else if (object->GetTypeName() == wxT("String"))
         {
            strList.push_back(object);
            
            // if initial value found
            if (genStr.find(wxT("=")) != genStr.npos)
            {
               wxString::size_type equalPos = genStr.find(wxT("="));
               wxString rhs = genStr.substr(equalPos + 1);
               rhs = GmatStringUtil::Trim(rhs, GmatStringUtil::BOTH, true, true);
               // check if initial value is string literal or other String object
               if (GmatStringUtil::IsEnclosedWith(rhs, wxT("'")))
                  strWithValList.push_back(object);
            }
         }
      }
   }
   
   //-----------------------------------------------------------------
   // Write Create Array ...
   // Write 10 Arrays without initial values per line
   //-----------------------------------------------------------------
   Integer counter = 0;
   UnsignedInt size = arrList.size();
   
   #ifdef DEBUG_SCRIPT_WRITING_PARAMETER
   MessageInterface::ShowMessage
      (wxT("WriteVariablesAndArrays() Writing %d Arrays without initial values \n"), size);
   #endif
   
   for (UnsignedInt i = 0; i<size; i++)
   {
      counter++;
      
      // Write comment line
      if (i == 0)
      {
         if (((Parameter*)arrList[i])->GetCommentLine(1) == wxT(""))
            theReadWriter->WriteText(wxT("\n"));
         else
         {
            // Write comment line if non section delimiter
            wxString comment = ((Parameter*)arrList[i])->GetCommentLine(1);
            if (comment.find(sectionStr) == comment.npos)
               theReadWriter->WriteText(comment);
            else
               theReadWriter->WriteText(wxT("\n"));
         }
      }
      
      if (counter == 1)
         theReadWriter->WriteText(wxT("Create Array"));
      
      theReadWriter->WriteText(wxT(" ") + arrList[i]->GetStringParameter(wxT("Description")));
      
      if ((counter % 10) == 0 || (i == size-1))
      {
         counter = 0;
         theReadWriter->WriteText(wxT(";\n"));
      }
   }
   
   //-----------------------------------------------------------------
   // Write Create Variable ...
   // Write 10 Variables without initial values per line
   //-----------------------------------------------------------------
   counter = 0;
   size = varList.size();
   
   #ifdef DEBUG_SCRIPT_WRITING_PARAMETER
   MessageInterface::ShowMessage
      (wxT("WriteVariablesAndArrays() Writing %d Variables without initial values \n"), size);
   #endif
   
   for (UnsignedInt i = 0; i<size; i++)
   {
      counter++;
      
      // Write comment line if non section delimiter
      if (i == 0)
      {
         wxString comment = ((Parameter*)varList[i])->GetCommentLine(1);
         if (comment.find(sectionStr) == comment.npos)
            theReadWriter->WriteText(comment);
      }
      
      if (counter == 1)
         theReadWriter->WriteText(wxT("Create Variable"));
      
      theReadWriter->WriteText(wxT(" ") + varList[i]->GetName());
      
      if ((counter % 10) == 0 || (i == size-1))
      {
         counter = 0;
         theReadWriter->WriteText(wxT(";\n"));
      }
   }
   
   //-----------------------------------------------------------------
   // Write Create String ...
   // Write 10 Strings without initial values per line
   //-----------------------------------------------------------------
   counter = 0;   
   size = strList.size();
   
   #ifdef DEBUG_SCRIPT_WRITING_PARAMETER
   MessageInterface::ShowMessage
      (wxT("WriteVariablesAndArrays() Writing %d Strings without initial values \n"), size);
   #endif
   
   for (UnsignedInt i = 0; i<strList.size(); i++)
   {
      counter++;
      
      // Write comment line if non section delimiter
      if (i == 0)
      {
         wxString comment = ((Parameter*)strList[i])->GetCommentLine(1);
         if (comment.find(sectionStr) == comment.npos)
            theReadWriter->WriteText(comment);
      }
      
      if (counter == 1)
         theReadWriter->WriteText(wxT("Create String"));
      
      theReadWriter->WriteText(wxT(" ") + strList[i]->GetName());
      
      if ((counter % 10) == 0 || i == size-1)
      {
         counter = 0;
         theReadWriter->WriteText(wxT(";\n"));
      }
   }
   
   // Write initial values created or changed via the GUI
   WriteArrayInitialValues(arrWithValList, mode);
   WriteVariableInitialValues(varWithValList, mode);
   WriteStringInitialValues(strWithValList, mode);
   theReadWriter->WriteText(wxT("\n"));
     
   #ifdef DEBUG_SCRIPT_WRITING_PARAMETER
   MessageInterface::ShowMessage(wxT("WriteVariablesAndArrays() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void WriteArrayInitialValues(const ObjectArray &arrWithValList, Gmat::WriteMode mode)
//------------------------------------------------------------------------------
/*
 * This method writes initial value of Variables
 */
//------------------------------------------------------------------------------
void ScriptInterpreter::WriteArrayInitialValues(const ObjectArray &arrWithValList,
                                                Gmat::WriteMode mode)
{
   //-----------------------------------------------------------------
   // Write Arrays with initial values
   //-----------------------------------------------------------------
   
   UnsignedInt size = arrWithValList.size();
   
   #ifdef DEBUG_SCRIPT_WRITING_PARAMETER
   MessageInterface::ShowMessage
      (wxT("WriteArrayInitialValues() Writing %d Arrays with initial values \n"), size);
   #endif
   
   for (UnsignedInt i = 0; i < size; i++)
   {
      // Write comment line
      if (i == 0)
         theReadWriter->WriteText(((Parameter*)arrWithValList[0])->GetCommentLine(2));
      
      theReadWriter->WriteText(arrWithValList[i]->GetStringParameter(wxT("InitialValue")));
   }
}


//------------------------------------------------------------------------------
// void WriteVariableInitialValues(const ObjectArray &varWithValList, Gmat::WriteMode mode)
//------------------------------------------------------------------------------
/*
 * This method writes initial value of Variables
 */
//------------------------------------------------------------------------------
void ScriptInterpreter::WriteVariableInitialValues(const ObjectArray &varWithValList,
                                                   Gmat::WriteMode mode)
{
   //-----------------------------------------------------------------
   // Write Variables with initial values
   //-----------------------------------------------------------------
   UnsignedInt size = varWithValList.size();
   
   #ifdef DEBUG_SCRIPT_WRITING_PARAMETER
   MessageInterface::ShowMessage
      (wxT("WriteVariablesAndArrays() Writing %d Variables with initial Real values \n"), size);
   #endif
   
   for (UnsignedInt i = 0; i < size; i++)
   {
      if (i == 0)
         theReadWriter->WriteText(((Parameter*)varWithValList[i])->GetCommentLine(2));
      
      theReadWriter->WriteText(varWithValList[i]->GetGeneratingString(mode));
   }
}


//------------------------------------------------------------------------------
// void WriteStringInitialValues(const ObjectArray &strWithValList, Gmat::WriteMode mode)
//------------------------------------------------------------------------------
/*
 * This method writes initial value of Variables
 */
//------------------------------------------------------------------------------
void ScriptInterpreter::WriteStringInitialValues(const ObjectArray &strWithValList,
                                                 Gmat::WriteMode mode)
{
   //-----------------------------------------------------------------
   // Write Strings with initial values by string literal
   //-----------------------------------------------------------------
   UnsignedInt size = strWithValList.size();
   
   #ifdef DEBUG_SCRIPT_WRITING_PARAMETER
   MessageInterface::ShowMessage
      (wxT("WriteStringInitialValues() Writing %d Strings with initial values \n"), size);
   #endif
   
   for (UnsignedInt i = 0; i<size; i++)
   {
      // If no new value has been assigned, skip
      //if (strWithValList[i]->GetName() ==
      //    strWithValList[i]->GetStringParameter(wxT("Expression")))
      if (strWithValList[i]->GetName() == wxT(""))
         continue;
      
      // Write comment line
      if (i == 0)
         theReadWriter->WriteText(((Parameter*)strWithValList[i])->GetCommentLine(2));
      
      theReadWriter->WriteText(strWithValList[i]->GetGeneratingString(mode));
   }
}


//------------------------------------------------------------------------------
// void WriteOtherParameters(StringArray &objs, Gmat::WriteMode mode)
//------------------------------------------------------------------------------
/*
 * This method writes other Parameters, such as X in Create X pos;
 * where X is calculated (system) Parameter name.
 */
//------------------------------------------------------------------------------
void ScriptInterpreter::WriteOtherParameters(StringArray &objs,
                                             Gmat::WriteMode mode)
{
   StringArray::iterator current;
   GmatBase *object =  NULL;
   bool isFirstTime = true;
   
   //-----------------------------------------------------------------
   // Fill in proper arrays
   //-----------------------------------------------------------------
   for (current = objs.begin(); current != objs.end(); ++current)
   {
      object = FindObject(*current);
      if (object != NULL)
      {
         wxString typeName = object->GetTypeName();
         if (typeName != wxT("Array") && typeName != wxT("Variable") &&
             typeName != wxT("String"))
         {
            // write only user created calculated parameters with no dots
            if (object->GetName().find(wxT(".")) == wxString::npos)
            {
               if (isFirstTime)
               {
                  WriteSectionDelimiter(objs[0], wxT("Other Parameters"));
                  isFirstTime = false;
               }
               
               wxString genStr = object->GetGeneratingString(mode);
               
               #ifdef DEBUG_SCRIPT_WRITING
               MessageInterface::ShowMessage
                  (wxT("WriteOtherParameters() writing typeName=<%s>\n"), typeName.c_str());
               #endif
               
               if (object->GetCommentLine() == wxT(""))
                  theReadWriter->WriteText(wxT("\n"));
               theReadWriter->WriteText(genStr);
            }
         }
      }
   }
}


//------------------------------------------------------------------------------
// void WriteCommandSequence(Gmat::WriteMode mode)
//------------------------------------------------------------------------------
void ScriptInterpreter::WriteCommandSequence(Gmat::WriteMode mode)
{
   GmatCommand *cmd = theModerator->GetFirstCommand();
   bool inTextMode = false;
   Integer scriptEventCount = 0;
   
   if (cmd == NULL)
      return;
   
   // Write out the section delimiter comment if preface comment is blank
   // The first command is always NoOp, so get next command
   cmd = cmd->GetNext();
   
   // If there is no command after NoOp, return
   if (cmd == NULL)
      return;
   
   #ifdef DEBUG_SCRIPT_WRITING_COMMANDS
   MessageInterface::ShowMessage
      (wxT("WriteCommandSequence() Writing Command Sequence\nPrefaceComment of %s=%s\n"),
       cmd->GetTypeName().c_str(), cmd->GetCommentLine().c_str());
   #endif
   
   GmatCommand *nextCmd = cmd->GetNext();
   bool writeMissionSeqDelim = false;
   
   // Since second command should be BeginMissionSequence, check for the next one for comment
   if (nextCmd != NULL &&
       GmatStringUtil::IsBlank(cmd->GetCommentLine(), true) &&
       GmatStringUtil::IsBlank(nextCmd->GetCommentLine(), true))
   {
      theReadWriter->WriteText(wxT("\n"));
      writeMissionSeqDelim = true;
   }
   else
   {
      wxString comment1 = cmd->GetCommentLine();
      wxString comment2;
      
      #ifdef DEBUG_SCRIPT_WRITING_COMMANDS
      MessageInterface::ShowMessage(wxT("==> curr comment = '%s'\n"), comment1.c_str());
      #endif
      
      if (nextCmd != NULL)
      {
         comment2 = nextCmd->GetCommentLine();
         
         #ifdef DEBUG_SCRIPT_WRITING_COMMANDS
         MessageInterface::ShowMessage(wxT("==> next comment = '%s'\n"), comment2.c_str());
         #endif
         
         // Swap comments if second comment has Mission Sequence
         if (comment2.find(wxT("Mission Sequence")) != comment2.npos)
         {
            cmd->SetCommentLine(comment2);
            nextCmd->SetCommentLine(comment1);
         }
      }
      
      // We don't want to write section delimiter multiple times, so check for it
      if (comment1.find(wxT("Mission Sequence")) == comment1.npos &&
          comment2.find(wxT("Mission Sequence")) == comment2.npos)
      {
         writeMissionSeqDelim = true;
      }
   }
   
   // Write section delimiter
   if (writeMissionSeqDelim)
   {
      theReadWriter->WriteText(sectionDelimiterString[0]);
      theReadWriter->WriteText(sectionDelimiterString[1] + wxT("Mission Sequence"));
      theReadWriter->WriteText(sectionDelimiterString[2]);
      theReadWriter->WriteText(wxT("\n"));      
   }
   
   while (cmd != NULL) 
   {
      #ifdef DEBUG_SCRIPT_WRITING_COMMANDS
      MessageInterface::ShowMessage
         (wxT("ScriptInterpreter::WriteCommandSequence() before write cmd=%s, mode=%d, ")
          wxT("inTextMode=%d\n"), cmd->GetTypeName().c_str(), mode, inTextMode);
      #endif
      
      // EndScript is written from BeginScript
      if (!inTextMode && cmd->GetTypeName() != wxT("EndScript"))
      {
         theReadWriter->WriteText(cmd->GetGeneratingString());
         theReadWriter->WriteText(wxT("\n"));
      }
      
      if (cmd->GetTypeName() == wxT("BeginScript"))
         scriptEventCount++;
      
      if (cmd->GetTypeName() == wxT("EndScript"))
         scriptEventCount--;
      
      inTextMode = (scriptEventCount == 0) ? false : true;
      
      if (cmd == cmd->GetNext())
         throw InterpreterException
            (wxT("Self-reference found in command stream during write.\n"));
      
      cmd = cmd->GetNext();
   }
}

