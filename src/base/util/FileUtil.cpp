//$Id: FileUtil.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                 FileUtil
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
// Author: Linda Jun
// Created: 2005/12/12
//
/**
 * This file provides methods to compare two output files. The compare summary
 * is written to the log file.
 */
//------------------------------------------------------------------------------

#include "FileUtil.hpp"
#include "StringTokenizer.hpp"
#include "MessageInterface.hpp"
#include "RealUtilities.hpp"       // for Abs()
#include "StringUtil.hpp"          // for ToString()
#include "FileTypes.hpp"           // for GmatFile::MAX_PATH_LEN
#include <algorithm>               // for set_difference()
#include <iterator>                // For back_inserter() with VC++ 2010
#include <wx/wfstream.h>
#include <wx/txtstrm.h>

#ifndef _MSC_VER  // if not Microsoft Visual C++
#include <dirent.h>
#endif

#ifdef __WIN32__
#include <windows.h>
#endif


using namespace GmatStringUtil;

//#define DBGLVL_COMPARE_REPORT 1
//#define DBGLVL_FUNCTION_OUTPUT 2

//------------------------------------------------------------------------------
// wxString GetPathSeparator()
//------------------------------------------------------------------------------
/**
 * @return path separator; wxT("/") or "\\" dependends on the platform
 */
//------------------------------------------------------------------------------
wxString GmatFileUtil::GetPathSeparator()
{
   wxString sep = wxT("/");
   
   char *buffer;
   buffer = getenv("OS");
   if (buffer != NULL)
   {
      #ifdef DEBUG_FILE_UTIL
      MessageInterface::ShowMessage
         (wxT("GmatFileUtil::GetPathSeparator() Current OS is %s\n"), buffer);
      #endif
      
      wxString osStr(wxString::FromAscii(buffer));
      
      if (osStr.find(wxT("Windows")) != osStr.npos)
         sep = wxT("\\");
   }
   
   return sep;
}


//------------------------------------------------------------------------------
// wxString GetCurrentPath()
//------------------------------------------------------------------------------
/*
 * Note: This function calls getcwd() which is defined in <dirent>. There is a
 *       problem compling with VC++ compiler, so until it is resolved, it will
 *       always return blank if it is compiled with VC++ compiler.
 *
 * @return  The current working directory, generally the application path.
 *
 */
//------------------------------------------------------------------------------
wxString GmatFileUtil::GetCurrentPath()
{
   wxString currPath;
   
#ifndef _MSC_VER  // if not Microsoft Visual C++
   char buffer[GmatFile::MAX_PATH_LEN];
   // Intentionally get the return and then ignore it to move warning from
   // system libraries to GMAT code base.  The wxT("unused variable") warning
   // here can be safely ignored.
//   char *ch = getcwd(buffer, GmatFile::MAX_PATH_LEN);
   // This clears a warning message
   if (getcwd(buffer, GmatFile::MAX_PATH_LEN) != buffer)
      ;
   currPath = wxString::FromAscii(buffer);
#else
   MessageInterface::ShowMessage
      (wxT("*** WARNING *** GmatFileUtil::GetCurrentPath() \n")
       wxT("Cannot compile getcwd() with MSVC, so jsut returning empty path\n"));
#endif
   
   return currPath;
   
}


//------------------------------------------------------------------------------
// wxString ParseFirstPathName(const wxString &fullPath, bool appendSep = true)
//------------------------------------------------------------------------------
/*
 * This function parses first path name from given full path name.
 *
 * @param  fullPath  input full path name
 * @param  appendSep appends path separator if true
 * @return  The file name from the full path
 *
 */
//------------------------------------------------------------------------------
wxString GmatFileUtil::ParseFirstPathName(const wxString &fullPath,
                                             bool appendSep)
{
   #ifdef DEBUG_PARSE_FILENAME
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::ParseFirstPathName() fullPath=<%s>\n"), fullPath.c_str());
   #endif
   
   wxString filePath;
   wxString::size_type firstSlash = fullPath.find_first_of(wxT("/\\"));
   
   if (firstSlash != filePath.npos)
   {
      if (appendSep)
         filePath = fullPath.substr(0, firstSlash + 1);
      else
         filePath = fullPath.substr(0, firstSlash);
   }
   
   #ifdef DEBUG_PARSE_FILENAME
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::ParseFirstPathName() returning <%s>\n"), filePath.c_str());
   #endif
   
   return filePath;
}


//------------------------------------------------------------------------------
// wxString ParsePathName(const wxString &fullPath, bool appendSep = true)
//------------------------------------------------------------------------------
/*
 * This function parses whole path name from given full path name.
 *
 * @param  fullPath  input full path name
 * @param  appendSep appends path separator if true
 * @return  The file name from the full path
 *
 */
//------------------------------------------------------------------------------
wxString GmatFileUtil::ParsePathName(const wxString &fullPath,
                                        bool appendSep)
{
   #ifdef DEBUG_PARSE_FILENAME
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::ParsePathName() fullPath=<%s>\n"), fullPath.c_str());
   #endif
   
   wxString filePath;
   wxString::size_type lastSlash = fullPath.find_last_of(wxT("/\\"));
   
   if (lastSlash != filePath.npos)
   {
      if (appendSep)
         filePath = fullPath.substr(0, lastSlash + 1);
      else
         filePath = fullPath.substr(0, lastSlash);
   }
   
   #ifdef DEBUG_PARSE_FILENAME
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::ParsePathName() returning <%s>\n"), filePath.c_str());
   #endif
   
   return filePath;
}


//------------------------------------------------------------------------------
// wxString ParseFileName(const wxString &fullPath, bool removeExt = false)
//------------------------------------------------------------------------------
/*
 * This function parses file name from given full path name.
 *
 * @param  fullPath  input full path name
 * @param  removeExt  Set this flag to true if file extension to be removed [false]
 * @return  The file name from the full path
 *
 */
//------------------------------------------------------------------------------
wxString GmatFileUtil::ParseFileName(const wxString &fullPath, bool removeExt)
{
   #ifdef DEBUG_PARSE_FILENAME
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::ParseFileName() fullPath=<%s>\n"), fullPath.c_str());
   #endif
   
   wxString fileName = fullPath;
   
   wxString::size_type lastSlash = fileName.find_last_of(wxT("/\\"));
   if (lastSlash != fileName.npos)
      fileName = fileName.substr(lastSlash+1);
   
   #ifdef DEBUG_PARSE_FILENAME
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::ParseFileName() returning <%s>\n"), fileName.c_str());
   #endif
   
   if (removeExt)
   {
      wxString::size_type index1 = fileName.find_first_of(wxT("."));
      if (index1 != fileName.npos)
         fileName = fileName.substr(0, index1);
   }
   
   return fileName;
}


//------------------------------------------------------------------------------
// wxString ParseFileExtension(const wxString &fullPath, bool prependDot)
//------------------------------------------------------------------------------
/*
 * This function parses file extension (string after .) from given full path name.
 *
 * @param  fullPath  input full path name
 * @param  prependDot  prepends dot(.) if this is true [false]
 * @return  The file extension from the full path
 *
 */
//------------------------------------------------------------------------------
wxString GmatFileUtil::ParseFileExtension(const wxString &fullPath,
                                             bool prependDot)
{
   #ifdef DEBUG_PARSE_FILENAME
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::ParseFileExtension() fullPath=<%s>\n"), fullPath.c_str());
   #endif
   
   wxString fileExt;
   
   wxString::size_type lastDot = fullPath.find_last_of(wxT("."));
   if (lastDot != fullPath.npos)
   {
      if (lastDot < fullPath.size())
      {
         fileExt = fullPath.substr(lastDot+1);
         
         #ifdef DEBUG_PARSE_FILENAME
         MessageInterface::ShowMessage(wxT("   fileExt='%s'\n"), fileExt.c_str());
         #endif
         
         // Check for path separator
         if (fileExt[0] == wxT('/') || fileExt[0] == wxT('\\'))
            fileExt = wxT("");
      }
   }
   
   if (fileExt != wxT("") && prependDot)
      fileExt = wxT(".") + fileExt;
   
   #ifdef DEBUG_PARSE_FILENAME
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::ParseFileExtension() returning <%s>\n"), fileExt.c_str());
   #endif
   
   return fileExt;
}


//------------------------------------------------------------------------------
// wxString GetInvalidFileNameMessage(Integer option = 1)
//------------------------------------------------------------------------------
/**
 * Returns invalid file name message.
 */
//------------------------------------------------------------------------------
wxString GmatFileUtil::GetInvalidFileNameMessage(Integer option)
{
   wxString msg;
   
   if (option == 1)
      msg = wxT("Maximum of 232 chars of non-blank name without containing any of ")
         wxT("the following characters: \\/:*?\"<>| ");
   else if (option == 2)
      msg = wxT("A file name cannot be blank or contain any of the following characters:\n")
      wxT("   \\/:*?\"<>|");
   
   return msg;
}


//------------------------------------------------------------------------------
// bool GmatFileUtil::IsValidFileName(const wxString &fname, bool blankIsOk = true)
//------------------------------------------------------------------------------
bool GmatFileUtil::IsValidFileName(const wxString &fname, bool blankIsOk)
{
   if (fname == wxT(""))
   {
      if (blankIsOk)
         return true;
      else
         return false;
   }
   
   wxString filename = ParseFileName(fname);
   bool retval = false;
   
   // Check for invalid characters
   wxString invalidChars = wxT("\\/:*?\"<>|");
   if (filename.find_first_of(invalidChars) == filename.npos)
      retval = true;
   else
      retval = false;
   
   // Check for name too long
   if (retval)
   {
      if ((Integer) filename.size() > GmatFile::MAX_FILE_LEN)
         retval = false;
   }
   
   return retval;
}


//------------------------------------------------------------------------------
// bool GmatFileUtil::IsSameFileName(const wxString &fname1, const wxString &fname2)
//------------------------------------------------------------------------------
/*
 * @return  true  If two file names are same, false otherwise
 */
//------------------------------------------------------------------------------
bool GmatFileUtil::IsSameFileName(const wxString &fname1, const wxString &fname2)
{
   if (fname1 == wxT("") || fname2 == wxT(""))
      return false;

   wxString name1 = fname1;
   wxString name2 = fname2;
   
   // Replace \ with /
   name1 = GmatStringUtil::Replace(name1, wxT("\\"), wxT("/"));
   name2 = GmatStringUtil::Replace(name2, wxT("\\"), wxT("/"));
   if (name1 == name2)
      return true;
   else
      return false;
}


//------------------------------------------------------------------------------
// bool DoesDirectoryExist(const wxString &fullPath, bool blankIsOk = true)
//------------------------------------------------------------------------------
/*
 * @return  true  If directory exist, false otherwise
 */
//------------------------------------------------------------------------------
bool GmatFileUtil::DoesDirectoryExist(const wxString &fullPath, bool blankIsOk)
{
   #ifdef DEBUG_DIR_EXIST
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::DoesDirectoryExist() entered, fullPath='%s'\n"), fullPath.c_str());
   #endif
   
   if (fullPath == wxT(""))
   {
      if (blankIsOk)
         return true;
      else
         return false;
   }
   
   bool dirExist = false;
   wxString dirName = ParsePathName(fullPath, true);
   
   // empty dir name is OK.
   if (dirName == wxT(""))
      return true;
   
   #ifdef DEBUG_DIR_EXIST
   MessageInterface::ShowMessage(wxT("   ==> dirName='%s'\n"), dirName.c_str());
   #endif
   
#ifndef _MSC_VER  // if not Microsoft Visual C++
   DIR *dir = NULL;
   dir = opendir(dirName.char_str());
   
   if (dir != NULL)
   {
      dirExist = true; 
      closedir(dir);
   }
#else
   TCHAR currDir[BUFFER_SIZE];
   
   // Save current directory
   DWORD ret = GetCurrentDirectory(BUFFER_SIZE, currDir);
   // Try setting to requested direcotry
   dirExist = (SetCurrentDirectory(dirName.c_str()) == 0 ? false : true);
   // Set back to current directory
   SetCurrentDirectory(currDir);
#endif
   
   #ifdef DEBUG_DIR_EXIST
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::DoesDirectoryExist() returning %d\n"), dirExist);
   #endif
   
   return dirExist;
}


//------------------------------------------------------------------------------
// bool DoesFileExist(const wxString &filename)
//------------------------------------------------------------------------------
bool GmatFileUtil::DoesFileExist(const wxString &filename)
{
   #ifdef DEBUG_FILE_CHECK
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::DoesFileExist() filename=<%s>\n"),
       filename.c_str());
   #endif
   
   FILE * pFile;
   pFile = fopen (filename.char_str(), "rt+");
   bool fileExist = false;
   if (pFile!=NULL)
   {
      fclose (pFile);
      fileExist = true;
   }
   
   #ifdef DEBUG_FILE_CHECK
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::DoesFileExist() returning %d\n"), fileExist);
   #endif
   
   return fileExist;
}


//------------------------------------------------------------------------------
// bool GetLine(std::istream *is, wxString &line)
//------------------------------------------------------------------------------
/*
 * Reads a platform independent line from the input stream.
 *
 * @param  is    The input stream pointer
 * @param  line  The line read from the input stream
 *
 * @return  true if a line from the input stream was read successfully.
 */
//------------------------------------------------------------------------------
bool GmatFileUtil::GetLine(std::istream *is, wxString &line)
{
   if (is == NULL)
      return false;
   
   char ch;
   wxString result;
   
   while (is->get(ch) && ch != wxT('\r') && ch != wxT('\n') && ch != wxT('\0') &&
          !is->eof()) 
      result += ch;
   
   line = result;
   return true;
}

//#define DEBUG_APP_INSTALLATION
//------------------------------------------------------------------------------
// bool IsAppInstalled(const wxString &appName)
//------------------------------------------------------------------------------
/**
 * Asks system if requested application is installed
 *
 * @param  appName  Name of the application, such as MATLAB
 * @return  true requested application is installed on the system
 * @note GMAT currently checks for only MATLAB installation
 */
//------------------------------------------------------------------------------
bool GmatFileUtil::IsAppInstalled(const wxString &appName, wxString &appLoc)
{
#ifdef __WIN32__
   #ifdef DEBUG_APP_INSTALLATION
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::IsAppInstalled() entered, appName='%s'\n"), appName.c_str());
   #endif
   
   if (appName != wxT("MATLAB"))
   {
      MessageInterface::ShowMessage
         (wxT("GMAT currently checks for only MATLAB installation\n"));
      return false;
   }
   
   long lRet;
   HKEY hKey;
   char temp[150];
   DWORD dwBufLen;
   HKEY tree = HKEY_LOCAL_MACHINE;

   // @todo
   // Should we check other versions by querying sub keys?
   // See RegEnumKeyEx(), RegEnumValue() example in
   // http://msdn.microsoft.com/en-us/library/ms724256%28VS.85%29.aspx
   //wxString ver75 = wxT("7.5"); // 2007b
   wxString ver79 = wxT("7.9"); // 2009b
   
   wxString matlabFolder = "Software\\MathWorks\\MATLAB\\";

   wxString folder = matlabFolder + ver79;
   wxString key = wxT("MATLABROOT");
   
   #ifdef DEBUG_APP_INSTALLATION
   MessageInterface::ShowMessage(wxT("   About to open '%s'\n"), folder.c_str());
   #endif
   
   // Open location
   lRet = RegOpenKeyEx(tree, folder.c_str(), 0, KEY_QUERY_VALUE, &hKey);
   if (lRet != ERROR_SUCCESS)
   {
      #ifdef DEBUG_APP_INSTALLATION
      MessageInterface::ShowMessage
         (wxT("   Failed on RegOpenKeyEx(), return code is %ld\n"), lRet);
      #endif
      
      return false;
   }
   else
   {
      #ifdef DEBUG_APP_INSTALLATION
      MessageInterface::ShowMessage(wxT("   hKey is %ld\n"), hKey);
      #endif
   }
   
   // Get key
   dwBufLen = sizeof(temp);
   lRet = RegQueryValueEx( hKey, key.c_str(), NULL, NULL, (BYTE*)&temp, &dwBufLen );
   if (lRet != ERROR_SUCCESS)
   {
      #ifdef DEBUG_APP_INSTALLATION
      MessageInterface::ShowMessage
         (wxT("   Failed on RegQueryValueEx(), rReturn code is %ld\n"), lRet);
      #endif
      return false;
   }
   
   #ifdef DEBUG_APP_INSTALLATION
   MessageInterface::ShowMessage(wxT("   Key value: %s\n"), temp);
   #endif
   
   // Close key
   lRet = RegCloseKey( hKey );
   if (lRet != ERROR_SUCCESS)
   {
      #ifdef DEBUG_APP_INSTALLATION
      MessageInterface::ShowMessage
         (wxT("   Failed on RegCloseKey(), return code is %ld\n"), lRet);
      #endif
      return false;
   }
   
   appLoc = temp;
   
   // Got this far, then key exists
   #ifdef DEBUG_APP_INSTALLATION
   MessageInterface::ShowMessage(wxT("GmatFileUtil::IsAppInstalled() returning true\n"));
   #endif
   return true;
   
#else // other operating system
   return true;
#endif
}

//------------------------------------------------------------------------------
// WrapperTypeArray GetFunctionOutputTypes(std::istream *inStream,
//                     const StringArray &outputs, wxString &errMsg,
//                     IntegerArray &outputRows, IntegerArray &outputCols)
//------------------------------------------------------------------------------
/*
 * Retrives function output information from the input stream, keeping the
 * the order of outputs.
 *
 * @param  inStream  the input function stream
 * @param  inputs    the input name list
 * @param  outputs   the output name list
 * @param  errMsg    the error message to be set if any
 * @param  outputRows  the array of row count to be set
 * @param  outputRows  the array of column count to be set
 *
 * @return  return  the wrapper type array of outputs
 */
//------------------------------------------------------------------------------
WrapperTypeArray
GmatFileUtil::GetFunctionOutputTypes(std::istream *inStream, const StringArray &inputs,
                                     const StringArray &outputs, wxString &errMsg,
                                     IntegerArray &outputRows, IntegerArray &outputCols)
{
   UnsignedInt outputSize = outputs.size();
   
   #if DBGLVL_FUNCTION_OUTPUT
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::GetFunctionOutputTypes() inputSize = %d, outputSize = %d\n"),
       inputs.size(), outputSize);
   for (UnsignedInt i=0; i<inputs.size(); i++)
      MessageInterface::ShowMessage(wxT("   inputs[%d]='%s'\n"), i, inputs[i].c_str());
   for (UnsignedInt i=0; i<outputs.size(); i++)
      MessageInterface::ShowMessage(wxT("   outputs[%d]='%s'\n"), i, outputs[i].c_str());
   #endif
   
   WrapperTypeArray outputWrapperTypes;
   wxString line;
   StringArray outputTypes, outputNames, outputDefs, multiples, globals;
   errMsg = wxT("");
   wxString errMsg1, errMsg2;
   wxString name;
   Integer row, col;
   
   // if no output, just return
   if (outputSize == 0)
      return outputWrapperTypes;
   
   // check for duplicate output names
   for (UnsignedInt i=0; i<outputSize; i++)
   {
      for (UnsignedInt j=0; j<outputSize; j++)
      {
         if (i == j)
            continue;
         
         if (outputs[i] == outputs[j])
            if (find(multiples.begin(), multiples.end(), outputs[i]) == multiples.end())
               multiples.push_back(outputs[i]);
      }
   }
   
   if (multiples.size() > 0)
   {
      errMsg = wxT("Duplicate output of");
      
      for (UnsignedInt i=0; i<multiples.size(); i++)
         errMsg = errMsg + wxT(" \"") + multiples[i] + wxT("\"");
      return outputWrapperTypes;
   }
   
   
   // Initialize arrays to be used
   for (UnsignedInt i=0; i<outputSize; i++)
   {
      outputTypes.push_back(wxT(""));;
      outputNames.push_back(wxT(""));;
      outputDefs.push_back(wxT(""));;
   }
   
   // Go through each line in the function file, ignoring after % inline comment
   while (!inStream->eof())
   {
      if (GetLine(inStream, line))
      {
         // remove inline comments and trim
         line = GmatStringUtil::RemoveInlineComment(line, wxT("%"));         
         line = GmatStringUtil::Trim(line, GmatStringUtil::BOTH, true, true);
         
         // Skip empty line or comment line
         if (line[0] == wxT('\0') || line[0] == wxT('%'))
            continue;
         
         StringArray parts = GmatStringUtil::SeparateBy(line, wxT(" ,"), true);
         
         if (parts[0] == wxT("Global"))
         {
            #if DBGLVL_FUNCTION_OUTPUT > 1
            for (UnsignedInt i=0; i<parts.size(); i++)
               MessageInterface::ShowMessage(wxT("   parts[%d]='%s'\n"), i, parts[i].c_str());
            #endif
            
            for (UnsignedInt j=1; j<parts.size(); j++)
               globals.push_back(parts[j]);
            
         }
         else if (parts[0] == wxT("Create"))
         {
            #if DBGLVL_FUNCTION_OUTPUT > 1
            for (UnsignedInt i=0; i<parts.size(); i++)
               MessageInterface::ShowMessage(wxT("   parts[%d]='%s'\n"), i, parts[i].c_str());
            #endif
         
            for (UnsignedInt i=0; i<outputSize; i++)
            {
               for (UnsignedInt j=2; j<parts.size(); j++)
               {
                  GmatStringUtil::GetArrayIndex(parts[j], row, col, name, wxT("[]"));
                  
                  if (name == outputs[i])
                  {
                     // add multiple output defs
                     if (find(outputNames.begin(), outputNames.end(), name) != outputNames.end())
                        multiples.push_back(name);
                     
                     outputNames[i] = name;
                     outputTypes[i] = parts[1];
                     outputDefs[i] = parts[j];
                     
                     #if DBGLVL_FUNCTION_OUTPUT > 1
                     MessageInterface::ShowMessage
                        (wxT("   i=%d, type='%s', name='%s', def='%s'\n"), i, parts[1].c_str(),
                         name.c_str(), parts[j].c_str());
                     #endif                  
                  }
               }
            }
         }
      }
      else
      {
         errMsg = wxT("Encountered an error reading a file");
         return outputWrapperTypes;
      }
   }
   
   // find missing output definition
   StringArray missing;
   set_difference(outputs.begin(), outputs.end(),
                  outputNames.begin(), outputNames.end(), back_inserter(missing));
   
   #if DBGLVL_FUNCTION_OUTPUT
   MessageInterface::ShowMessage
      (wxT("   missing.size()=%d, multiples.size()=%d, globals.size()=%d\n"),
       missing.size(), multiples.size(), globals.size());
   for (UnsignedInt i=0; i<globals.size(); i++)
      MessageInterface::ShowMessage(wxT("   globals[%d] = '%s'\n"), i, globals[i].c_str());
   #endif
   
   if (missing.size() == 0 && multiples.size() == 0)
   {
      // if all output found, figure out the output wrapper types
      for (UnsignedInt i=0; i<outputSize; i++)
      {
         if (outputTypes[i] == wxT("Variable"))
         {
            outputWrapperTypes.push_back(Gmat::VARIABLE_WT);
            outputRows.push_back(-1);
            outputCols.push_back(-1);
         }
         else if (outputTypes[i] == wxT("Array"))
         {
            GmatStringUtil::GetArrayIndex(outputDefs[i], row, col, name, wxT("[]"));
            
            #if DBGLVL_FUNCTION_OUTPUT > 1
            MessageInterface::ShowMessage
               (wxT("   name='%s', row=%d, col=%d\n"), name.c_str(), row, col);
            #endif
            
            outputWrapperTypes.push_back(Gmat::ARRAY_WT);
            outputRows.push_back(row);
            outputCols.push_back(col);
         }
      }
   }
   else
   {
      if (missing.size() > 0)
      {
         StringArray reallyMissing;
         for (UnsignedInt i=0; i<missing.size(); i++)
         {
            // Check if missing output declarations are in the input names or
            // globals. If output names are not in the inputs or globals, it is an
            // error condition as in the GMAT Function requirements 1.6, 1.7, 1.8
            if (find(inputs.begin(), inputs.end(), missing[i]) == inputs.end() &&
                find(globals.begin(), globals.end(), missing[i]) == globals.end())
               reallyMissing.push_back(missing[i]);
         }
         
         if (reallyMissing.size() > 0)
         {
            errMsg1 = wxT("Missing output declaration of");
            for (UnsignedInt i=0; i<reallyMissing.size(); i++)
               errMsg1 = errMsg1 + wxT(" \"") + reallyMissing[i] + wxT("\"");
         }
      }
      
      if (multiples.size() > 0)
      {
         for (UnsignedInt i=0; i<multiples.size(); i++)
            if (multiples[i] != wxT(""))
               errMsg2 = errMsg2 + wxT(" \"") + multiples[i] + wxT("\"");
         
         if (errMsg2 != wxT(""))
         {
            if (errMsg1 == wxT(""))
               errMsg2 = wxT("Multiple declaration of") + errMsg2;
            else
               errMsg2 = wxT(" and multiple declaration of") + errMsg2;
         }
      }
      
      errMsg = errMsg1 + errMsg2;
   }
   
   
   #if DBGLVL_FUNCTION_OUTPUT
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::GetFunctionOutputTypes() returning %d outputWrapperTypes\n"),
       outputWrapperTypes.size());
   for (UnsignedInt i=0; i<outputWrapperTypes.size(); i++)
      MessageInterface::ShowMessage
         (wxT("   i=%d, outputWrapperTypes=%d, outputRows=%d, outputCols=%d\n"), i,
          outputWrapperTypes[i], outputRows[i], outputCols[i]);
   #endif
   
   return outputWrapperTypes;
}


//------------------------------------------------------------------------------
// StringArray GetFileListFromDirectory(const wxString &dirName, bool addPath)
//------------------------------------------------------------------------------
/*
 * Get list of files from a directory.
 * The input directory should include directory namd and file spec.
 * For example, c:\MyDir\*.txt, c:\MyFunctions\*.gmf
 *
 * @param  dirName  Directory name with file specification
 * @param  addPath  If true, it prepends path to file name (false)
 * @return  String array containing file names in the directory
 */
//------------------------------------------------------------------------------
StringArray GmatFileUtil::GetFileListFromDirectory(const wxString &dirName,
                                                   bool addPath)
{
   #ifdef DEBUG_FILELIST
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::GetFileListFromDirectory() dirName=<%s>\n"),
       dirName.c_str());
   #endif
   
   wxString pathName = ParsePathName(dirName);
   wxString fileExt = ParseFileExtension(dirName);
   wxString outFile;
   StringArray fileList;
   
   MessageInterface::ShowMessage
      (wxT("==> pathName=<%s>, fileExt=<%s>\n"), pathName.c_str(), fileExt.c_str());
   
   //-----------------------------------------------------------------
   // for windows
   //-----------------------------------------------------------------
   #ifdef __WIN32__
   
   HANDLE hFind;
   WIN32_FIND_DATA findData;
   int errorCode;
   bool hasError = false;
   
   hFind = FindFirstFile(dirName.c_str(), &findData);
   
   if(hFind == INVALID_HANDLE_VALUE)
   {
      errorCode = GetLastError();
      if (errorCode == ERROR_FILE_NOT_FOUND)
      {
         MessageInterface::ShowMessage
            (wxT("**** ERROR **** GmatFileUtil::GetFileListFromDirectory() \n")
             wxT("There are no directory matching \"%s\"\n"), dirName.c_str());
      }
      else
      {
         MessageInterface::ShowMessage
            (wxT("**** ERROR **** GmatFileUtil::GetFileListFromDirectory() \n")
             wxT("FindFirstFile() returned error code %d\n"), errorCode);
      }
      hasError = true;
   }
   else
   {
      outFile = findData.cFileName;

      // add if the file matches exact file extension (i.e, no backup files allowed)
      if (ParseFileExtension(outFile) == fileExt)
      {
         if (addPath)
            outFile = pathName + findData.cFileName;
         fileList.push_back(outFile);
         
         #ifdef DEBUG_FILELIST
         MessageInterface::ShowMessage
            (wxT("   > added %s to file list\n"), findData.cFileName);
         #endif
      }
   }
   
   if (!hasError)
   {
      while (FindNextFile(hFind, &findData))
      {
         outFile = findData.cFileName;
         if (ParseFileExtension(outFile) == fileExt)
         {
            if (addPath)
               outFile = pathName + findData.cFileName;
            fileList.push_back(outFile);
            
            #ifdef DEBUG_FILELIST
            MessageInterface::ShowMessage
               (wxT("   > added %s to file list\n"), findData.cFileName);
            #endif
         }
      }
      
      errorCode = GetLastError();
      
      if (errorCode != ERROR_NO_MORE_FILES)
      {
         MessageInterface::ShowMessage
            (wxT("**** ERROR **** GmatFileUtil::GetFileListFromDirectory() \n")
             wxT("FindNextFile() returned error code %d\n"), errorCode);
      }
      
      if (!FindClose(hFind))
      {
         errorCode = GetLastError();
         MessageInterface::ShowMessage
            (wxT("**** ERROR **** GmatFileUtil::GetFileListFromDirectory() \n")
             wxT("FindClose() returned error code %d\n"), errorCode);
      }
   }
   #else
   // add other operating system here
   #endif
   
   #ifdef DEBUG_FILELIST
   MessageInterface::ShowMessage
      (wxT("GmatFileUtil::GetFileListFromDirectory() returning %d files\n"),
       fileList.size());
   #endif
   
   return fileList;
}


//------------------------------------------------------------------------------
// StringArray GetTextLines(const wxString &fileName)
//------------------------------------------------------------------------------
/*
 * Reads a text file and returns an array of string.
 *
 * @param  dirName  Directory name
 * @return  String array containing file names in the directory
 */
//------------------------------------------------------------------------------
StringArray GmatFileUtil::GetTextLines(const wxString &fileName)
{
   StringArray lines;
   
   // check if file exist
   wxFileInputStream inFileStream(fileName);
   wxTextInputStream inFile(inFileStream);
   
   if (!(inFileStream.IsOk()))
   {
      MessageInterface::ShowMessage
         (wxT("**** ERROR **** GmatFileUtil::GetTextLines() \n")
          wxT("The file \"") + fileName + wxT("\" does not exist\n"));
      return lines;
   }
   
   wxString oneLine;
   
   while (!inFileStream.Eof())
   {
      inFile >> oneLine;
      lines.push_back(oneLine);
   }
   
   return lines;
}


//------------------------------------------------------------------------------
// StringArray& Compare(const wxString &filename1, const wxString &filename2,
//                      Real tol = 1.0e-4)
//------------------------------------------------------------------------------
StringArray& GmatFileUtil::Compare(const wxString &filename1,
                                   const wxString &filename2,
                                   const StringArray &colTitles,
                                   Real tol)
{
   textBuffer.clear();
   textBuffer.push_back(wxT("\n======================================== Compare Utility\n"));
   textBuffer.push_back(wxT("filename1=") + filename1 + wxT("\n"));
   textBuffer.push_back(wxT("filename2=") + filename2 + wxT("\n"));

   #if DBGLVL_COMPARE_REPORT
   MessageInterface::ShowMessage(wxT("\n======================================== Compare Utility\n"));
   MessageInterface::ShowMessage(wxT("filename1=%s\n"));
   MessageInterface::ShowMessage(wxT("filename2=%s\n"));
   #endif
   
   // open file
   std::ifstream in1(filename1.char_str());
   std::ifstream in2(filename2.char_str());
   
   if (!in1)
   {
      textBuffer.push_back(wxT("Cannot open first file: ") +  filename1 + wxT("\n\n"));
      return textBuffer;
   }
   
   if (!in2)
   {
      textBuffer.push_back(wxT("Cannot open second file: ") + filename2 + wxT("\n\n"));
      return textBuffer;
   }
   
   char buffer[BUFFER_SIZE];
   Real item1, item2, diff;
   wxString line;
   int count = 1;
   int file1Cols = 0, file2Cols = 0;
   StringTokenizer stk;
   StringArray tokens1, tokens2;
   
   //------------------------------------------
   // if files have header lines, skip
   //------------------------------------------
   if (!GmatFileUtil::SkipHeaderLines(in1, tokens1))
   {
      textBuffer.push_back(wxT("***Cannot compare files: Data record not found on file 1.\n"));
      return textBuffer;
   }
   
   if (!GmatFileUtil::SkipHeaderLines(in2, tokens2))
   {
      textBuffer.push_back(wxT("***Cannot compare files: Data record not found on file 2.\n"));
      return textBuffer;
   }
   
   //------------------------------------------
   // check number of columns
   //------------------------------------------
   file1Cols = tokens1.size();
   file2Cols = tokens2.size();
   Integer numCols = file1Cols<file2Cols ? file1Cols : file2Cols;
   
   if (file1Cols != file2Cols)
   {
      textBuffer.push_back
         (wxT("*** Number of colmuns are different. file1:") + ToString(file1Cols) +
          wxT(",  file2:") + ToString(file2Cols) + wxT("\n*** Will compare up to") +
          ToString(numCols) + wxT(" columns\n"));
   }

   // compare first data line
   RealArray minDiffs, maxDiffs;
   IntegerArray minLines, maxLines;
   
   for (int i=0; i<numCols; i++)
   {
      item1 = atof(tokens1[i].char_str());
      item2 = atof(tokens2[i].char_str());
      diff = GmatMathUtil::Abs(item1 - item2);
      minDiffs.push_back(diff);
      maxDiffs.push_back(diff);
      minLines.push_back(1);
      maxLines.push_back(1);
      
      #if DBGLVL_COMPARE_REPORT > 1
      MessageInterface::ShowMessage
         (wxT("column=%3d, item1=% e, item2=% e, diff=% e, minDiff=% e, maxDiff=% e\n"),
          i, item1, item1, diff, minDiffs[i], maxDiffs[i]);
      #endif
   }
   
   //------------------------------------------
   // now start compare
   //------------------------------------------
   #if DBGLVL_COMPARE_REPORT > 2
   for (int i=0; i<10; i++)
   {
      if (in1.eof() || in2.eof())
         break;
   #else
   while (!in1.eof() && !in2.eof())
   {
   #endif

      count++;
      
      #if DBGLVL_COMPARE_REPORT > 1
      MessageInterface::ShowMessage(wxT("============================== line # = %d\n"), count);
      #endif
      
      // file 1
      in1.getline(buffer, BUFFER_SIZE-1);
      line = wxString::FromAscii(buffer);
      
      #if DBGLVL_COMPARE_REPORT > 2
      MessageInterface::ShowMessage(wxT("===> file 1: buffer = %s\n"), buffer);
      #endif
      
      stk.Set(line, wxT(" "));
      tokens1 = stk.GetAllTokens();

      // check for blank lines in file1
      if ((Integer)(tokens1.size()) != file1Cols)
         break;
      
      #if DBGLVL_COMPARE_REPORT > 2
      for (int i=0; i<numCols; i++)
         MessageInterface::ShowMessage(wxT("tokens1[%d] = %s\n"), i, tokens1[i].c_str());
      #endif
      
      // file 2
      in2.getline(buffer, BUFFER_SIZE-1);
      line = wxString::FromAscii(buffer);
      
      #if DBGLVL_COMPARE_REPORT > 2
      MessageInterface::ShowMessage(wxT("===> file 2: buffer = %s\n"), buffer);
      #endif
      
      stk.Set(line, wxT(" "));
      tokens2 = stk.GetAllTokens();
      
      // check for blank lines in file1
      if ((Integer)(tokens2.size()) != file1Cols)
         break;
      
      #if DBGLVL_COMPARE_REPORT > 2
      for (int i=0; i<file1Cols; i++)
         MessageInterface::ShowMessage(wxT("tokens2[%d] = %s\n"), i, tokens2[i].c_str());
      #endif
      
      for (int i=0; i<numCols; i++)
      {
         item1 = atof(tokens1[i].char_str());
         item2 = atof(tokens2[i].char_str());
         diff = GmatMathUtil::Abs(item1 - item2);
            
         if (diff < minDiffs[i])
         {
            minDiffs[i] = diff;
            minLines[i] = count;
         }
            
         if (diff > maxDiffs[i])
         {
            maxDiffs[i] = diff;
            maxLines[i] = count;
         }
            
         #if DBGLVL_COMPARE_REPORT > 1
         MessageInterface::ShowMessage
            (wxT("column=%3d, item1=% e, item2=% e, diff=% e, minDiff=% e, maxDiff=% e\n"),
             i, item1, item2, diff, minDiffs[i], maxDiffs[i]);
         #endif
      }
         
   }
   
   // report the difference summary
   wxString outLine;
   outLine = wxT("Total lines compared: ") + ToString(count) + wxT(",   Tolerance: ") +
      ToString(tol, false, true, true, 7, 6) + wxT("\n\n");
   textBuffer.push_back(outLine);

   #if DBGLVL_COMPARE_REPORT
   MessageInterface::ShowMessage(wxT("%s"), outLine.c_str());
   #endif

   if (colTitles.size() == 0)
   {
      outLine =
         wxT("Column   Minimum Diff.   Line#   Maximum Diff.   Line#   Min>Tol   ")
         wxT("Max>Tol\n")
         wxT("------   -------------   -----   -------------   -----   -------   ")
         wxT("-------\n");
   }
   else
   {
      outLine =
         wxT("Column   Column Title                     Minimum Diff.   Line#   ")
         wxT("Maximum Diff.   Line#   Min>Tol   Max>Tol\n")
         wxT("------   ------------                     -------------   -----   ")
         wxT("-------------   -----   -------   -------\n");
   }
   textBuffer.push_back(outLine);
   
   #if DBGLVL_COMPARE_REPORT
   MessageInterface::ShowMessage(wxT("%s"), outLine.c_str());
   #endif
   
   wxChar minGtTol, maxGtTol;
   char title[30] = "";
   
   for (int i=0; i<numCols; i++)
   {
      minGtTol = wxT(' ');
      maxGtTol = wxT(' ');
      
      if (minDiffs[i] > tol)
         minGtTol = wxT('*');
      
      if (maxDiffs[i] > tol)
         maxGtTol = wxT('*');

      if (colTitles.size() == 0)
      {
         outLine = ToString(i+1) + wxT("     ") + ToString(minDiffs[i], false, true, true, 7, 6) +
            wxT("   ") + ToString(minLines[i]) + wxT("    ") +
            ToString(maxDiffs[i], false, true, true, 7, 6) + wxT("   ") + ToString(maxLines[i]) +
            wxT("       ") + minGtTol + wxT("         ") + maxGtTol + wxT("\n");
      }
      else
      {
         sprintf(title, "%-30.30s", (char *) (colTitles[i].char_str()));
         outLine = ToString(i+1) + wxT("     ") + wxString::FromAscii(title) + wxT("   ") +
            ToString(minDiffs[i], false, true, true, 7, 6) + wxT("   ") + ToString(minLines[i]) +
            wxT("    ") + ToString(maxDiffs[i], false, true, true, 7, 6) + wxT("   ") +
            ToString(maxLines[i]) + wxT("       ") + minGtTol + wxT("         ") +
            maxGtTol + wxT("\n");
      }
      
      textBuffer.push_back(outLine);
      
      #if DBGLVL_COMPARE_REPORT
      MessageInterface::ShowMessage(wxT("%s"), outLine.c_str());
      #endif
   }
   
   in1.close();
   in2.close();

   return textBuffer;
}
   
   
//------------------------------------------------------------------------------
// StringArray& Compare(Integer numDirsToCompare, const wxString &basefilename,
//                      const wxString &filename1, const wxString &filename2,
//                      const wxString &filename3,  const StringArray &colTitles,
//                      Real tol = CompareAbsTol);
//------------------------------------------------------------------------------
StringArray& GmatFileUtil::Compare(Integer numDirsToCompare, const wxString &basefilename,
                                   const wxString &filename1, const wxString &filename2,
                                   const wxString &filename3, const StringArray &colTitles,
                                   Real tol)
{
   textBuffer.clear();
   textBuffer.push_back(wxT("\n======================================== Compare Utility\n"));
   textBuffer.push_back(wxT("basefile =") + basefilename + wxT("\n"));

   textBuffer.push_back(wxT("filename1=") + filename1 + wxT("\n"));
   textBuffer.push_back(wxT("filename2=") + filename2 + wxT("\n"));
   
   if (numDirsToCompare == 3)
      textBuffer.push_back(wxT("filename3=") + filename3 + wxT("\n"));

   #if DBGLVL_COMPARE_REPORT
   MessageInterface::ShowMessage(wxT("\n======================================== Compare Utility\n"));
   MessageInterface::ShowMessage(wxT("numDirsToCompare=%3\n"), numDirsToCompare);
   MessageInterface::ShowMessage(wxT("basefile =%s\n"), basefilename.c_str());
   MessageInterface::ShowMessage(wxT("filename1=%s\nfilename2=%s\nfilename3=%s\n"),
                                 filename1.c_str(), filename2.c_str(), filename3.c_str());
   #endif
   
   // open base file
   std::ifstream baseIn(basefilename.char_str());

   // open compare files
   std::ifstream in1(filename1.char_str());
   std::ifstream in2(filename2.char_str());
   std::ifstream in3(filename3.char_str());
   
   if (!baseIn)
   {
      textBuffer.push_back(wxT("Cannot open base file: ") +  basefilename + wxT("\n\n"));
      return textBuffer;
   }
   
   if (!in1)
   {
      textBuffer.push_back(wxT("Cannot open first file: ") + filename1 + wxT("\n\n"));
      return textBuffer;
   }
   
   if (!in2)
   {
      textBuffer.push_back(wxT("Cannot open second file: ") + filename2 + wxT("\n\n"));
      return textBuffer;
   }

   if (numDirsToCompare == 3)
      if (!in3)
      {
         textBuffer.push_back(wxT("Cannot open third file: ") + filename3 + wxT("\n\n"));
         return textBuffer;
      }

   
   char buffer[BUFFER_SIZE];
   Real baseItem, item, diff;
   wxString line;
   int count = 1;
   int baseCols = 99, file1Cols = 99, file2Cols = 99, file3Cols = 99;
   StringTokenizer stk;
   StringArray baseTokens, tokens1, tokens2, tokens3;
   
   //------------------------------------------
   // if files have header lines, skip
   //------------------------------------------
   if (!GmatFileUtil::SkipHeaderLines(baseIn, baseTokens))
   {
      textBuffer.push_back(wxT("***Cannot compare files: Data record not found on base file.\n"));
      return textBuffer;
   }
   
   if (!GmatFileUtil::SkipHeaderLines(in1, tokens1))
   {
      textBuffer.push_back(wxT("***Cannot compare files: Data record not found on file 1.\n"));
      return textBuffer;
   }
   
   if (!GmatFileUtil::SkipHeaderLines(in2, tokens2))
   {
      textBuffer.push_back(wxT("***Cannot compare files: Data record not found on file 2.\n"));
      return textBuffer;
   }
   
   if (numDirsToCompare == 3)
      if (!GmatFileUtil::SkipHeaderLines(in3, tokens3))
      {
         textBuffer.push_back(wxT("***Cannot compare files: Data record not found on file 3.\n"));
         return textBuffer;
      }
   
   //------------------------------------------
   // check number of columns
   //------------------------------------------
   baseCols = baseTokens.size();
   file1Cols = tokens1.size();
   file2Cols = tokens2.size();
   
   if (numDirsToCompare == 3)
      file3Cols = tokens3.size();
   
   Integer numCols = baseCols<file1Cols ? baseCols : file1Cols;
   numCols = numCols<file2Cols ? numCols : file2Cols;
   //MessageInterface::ShowMessage(wxT("===> numCols=%d\n"), numCols);
   
   if (numDirsToCompare == 3)
      numCols = numCols<file3Cols ? numCols : file3Cols;
   
   if (baseCols != file1Cols)
   {
      textBuffer.push_back
         (wxT("*** Number of colmuns are different. file1:") + ToString(baseCols) +
          wxT(",  file2:") + ToString(file1Cols) + wxT("\n*** Will compare up to") +
          ToString(numCols) + wxT(" columns\n"));
   }
   
   // compare first data line
   RealArray maxDiffs1, maxDiffs2, maxDiffs3;
   
   for (int i=0; i<numCols; i++)
   {
      baseItem = atof(baseTokens[i].char_str());
      item = atof(tokens1[i].char_str());
      diff = GmatMathUtil::Abs(item - baseItem);
      maxDiffs1.push_back(diff);
      
      item = atof(tokens2[i].char_str());
      diff = GmatMathUtil::Abs(item - baseItem);
      maxDiffs2.push_back(diff);
      
      if (numDirsToCompare == 3)
      {
         item = atof(tokens3[i].char_str());
         diff = GmatMathUtil::Abs(item - baseItem);
         maxDiffs3.push_back(diff);
      }
      
      #if DBGLVL_COMPARE_REPORT > 1
      MessageInterface::ShowMessage
         (wxT("column=%3d, baseItem=% e, item=% e, diff=% e, maxDiff=% e\n"),
          i, baseItem, baseItem, diff, maxDiffs1[i]);
      #endif
   }
   
   //------------------------------------------
   // now start compare
   //------------------------------------------
   #if DBGLVL_COMPARE_REPORT > 2
   for (int i=0; i<10; i++)
   {
      if (baseIn.eof() || in1.eof())
         break;
   #else
      while (!baseIn.eof() && !in1.eof() && !in2.eof())  
   {
   #endif

      if (numDirsToCompare == 3)
         if (in3.eof())
            break;
      
      count++;
      
      #if DBGLVL_COMPARE_REPORT > 1
      MessageInterface::ShowMessage(wxT("============================== line # = %d\n"), count);
      #endif

      //----------------------------------------------------
      // base file
      //----------------------------------------------------
      baseIn.getline(buffer, BUFFER_SIZE-1);
      line = wxString::FromAscii(buffer);
      
      #if DBGLVL_COMPARE_REPORT > 2
      MessageInterface::ShowMessage(wxT("===> base file: buffer = %s\n"), buffer);
      #endif
      
      stk.Set(line, wxT(" "));
      baseTokens = stk.GetAllTokens();

      // check for blank lines in base file
      if ((Integer)(baseTokens.size()) != baseCols)
         break;
      
      #if DBGLVL_COMPARE_REPORT > 2
      for (int i=0; i<numCols; i++)
         MessageInterface::ShowMessage(wxT("baseTokens[%d] = %s\n"), i, baseTokens[i].c_str());
      #endif
      
      //----------------------------------------------------
      // file 1
      //----------------------------------------------------
      in1.getline(buffer, BUFFER_SIZE-1);
      line = wxString::FromAscii(buffer);
      
      #if DBGLVL_COMPARE_REPORT > 2
      MessageInterface::ShowMessage(wxT("===> file 1: buffer = %s\n"), buffer);
      #endif
      
      stk.Set(line, wxT(" "));
      tokens1 = stk.GetAllTokens();
      
      // check for blank lines in file1
      if ((Integer)(tokens1.size()) != baseCols)
         break;
      
      //----------------------------------------------------
      // file 2
      //----------------------------------------------------      
      in2.getline(buffer, BUFFER_SIZE-1);
      line = wxString::FromAscii(buffer);
      
      #if DBGLVL_COMPARE_REPORT > 2
      MessageInterface::ShowMessage(wxT("===> file 2: buffer = %s\n"), buffer);
      #endif
      
      stk.Set(line, wxT(" "));
      tokens2 = stk.GetAllTokens();
      
      // check for blank lines in file2
      if ((Integer)(tokens2.size()) != baseCols)
         break;
      
      //----------------------------------------------------
      // file 3
      //----------------------------------------------------      
      if (numDirsToCompare == 3)
      {
         in3.getline(buffer, BUFFER_SIZE-1);
         line = wxString::FromAscii(buffer);
      
         #if DBGLVL_COMPARE_REPORT > 2
         MessageInterface::ShowMessage(wxT("===> file 3: buffer = %s\n"), buffer);
         #endif
      
         stk.Set(line, wxT(" "));
         tokens3 = stk.GetAllTokens();
      
         // check for blank lines in file2
         if ((Integer)(tokens3.size()) != baseCols)
            break;
      }
      
      #if DBGLVL_COMPARE_REPORT > 2
      for (int i=0; i<baseCols; i++)
         MessageInterface::ShowMessage(wxT("tokens1[%d] = %s\n"), i, tokens1[i].c_str());
      #endif
      
      for (int i=0; i<numCols; i++)
      {
         baseItem = atof(baseTokens[i].char_str());
         item = atof(tokens1[i].char_str());
         diff = GmatMathUtil::Abs(item - baseItem);
         if (diff > maxDiffs1[i])
            maxDiffs1[i] = diff;
         
         item = atof(tokens2[i].char_str());
         diff = GmatMathUtil::Abs(item - baseItem);
         if (diff > maxDiffs2[i])
            maxDiffs2[i] = diff;
         
         if (numDirsToCompare == 3)
         {
            item = atof(tokens3[i].char_str());
            diff = GmatMathUtil::Abs(item - baseItem);
            if (diff > maxDiffs3[i])
               maxDiffs3[i] = diff;
         }
         
         #if DBGLVL_COMPARE_REPORT > 1
         MessageInterface::ShowMessage
            (wxT("column=%3d, baseItem=% e, item=% e, diff=% e, maxDiff1=% e\n"),
             i, baseItem, item, diff, maxDiffs1[i]);
         #endif
      }
      
   }
   
   // report the difference summary
   wxString outLine;
   outLine = wxT("Total lines compared: ") + ToString(count) + wxT(",   Tolerance: ") +
      ToString(tol, false, true, true, 7, 6) + wxT("\n\n");
   textBuffer.push_back(outLine);

   #if DBGLVL_COMPARE_REPORT
   MessageInterface::ShowMessage(wxT("%s"), outLine.c_str());
   #endif

   
   if (numDirsToCompare == 2)
   {
      outLine =
         wxT("Column   Maximum Diff1   Max1>Tol   Maximum Diff2   Max2>Tol\n")
         wxT("------   -------------   -------    -------------   --------\n");
   }
   else if (numDirsToCompare == 3)
   {
      outLine =
         wxT("Column   Maximum Diff1   Max1>Tol   Maximum Diff2   Max2>Tol   Maximum Diff3   Max3>Tol\n")
         wxT("------   -------------   -------    -------------   --------   -------------   --------\n");
   }
   
   textBuffer.push_back(outLine);
   
   #if DBGLVL_COMPARE_REPORT
   MessageInterface::ShowMessage(wxT("%s"), outLine.c_str());
   #endif
   
   wxChar maxGtTol1, maxGtTol2, maxGtTol3;
   
   for (int i=0; i<numCols; i++)
   {
      maxGtTol1 = wxT(' ');
      maxGtTol2 = wxT(' ');
      maxGtTol3 = wxT(' ');
      
      if (maxDiffs1[i] > tol)
         maxGtTol1 = wxT('*');

      if (maxDiffs2[i] > tol)
         maxGtTol2 = wxT('*');

      if (numDirsToCompare == 3)
         if (maxDiffs3[i] > tol)
            maxGtTol3 = wxT('*');
      
      if (numDirsToCompare == 2)
      {
         outLine = ToString(i+1) + wxT("     ") +
            ToString(maxDiffs1[i], false, true, true, 7, 6) + wxT("      ") + maxGtTol1 + wxT("       ") +
            ToString(maxDiffs2[i], false, true, true, 7, 6) + wxT("      ") + maxGtTol2 + wxT("\n");
      }
      else if (numDirsToCompare == 3)
      {
         outLine = ToString(i+1) + wxT("     ") +
            ToString(maxDiffs1[i], false, true, true, 7, 6) + wxT("      ") + maxGtTol1 + wxT("       ") +
            ToString(maxDiffs2[i], false, true, true, 7, 6) + wxT("      ") + maxGtTol2 + wxT("       ") +
            ToString(maxDiffs3[i], false, true, true, 7, 6) + wxT("      ") + maxGtTol3 + wxT("\n");
      }
      
      textBuffer.push_back(outLine);
      
      #if DBGLVL_COMPARE_REPORT
      MessageInterface::ShowMessage(wxT("%s"), outLine.c_str());
      #endif
   }
   
   baseIn.close();
   in1.close();
   in2.close();
   in3.close();

   return textBuffer;
}
   
   
//------------------------------------------------------------------------------
// StringArray& Compare(Integer numDirsToCompare, const wxString &basefilename,
//                      const wxString &filename1, const wxString &filename2,
//------------------------------------------------------------------------------
StringArray& GmatFileUtil::CompareLines(Integer numDirsToCompare,
                                        const wxString &basefilename,
                                        const wxString &filename1,
                                        const wxString &filename2,
                                        const wxString &filename3,
                                        int &file1DiffCount, int &file2DiffCount,
                                        int &file3DiffCount)
{
   textBuffer.clear();
   textBuffer.push_back(wxT("\n======================================== Compare Utility\n"));
   textBuffer.push_back(wxT("basefile =") + basefilename + wxT("\n"));
   
   textBuffer.push_back(wxT("filename1=") + filename1 + wxT("\n"));
   
   if (numDirsToCompare >= 2)
      textBuffer.push_back(wxT("filename2=") + filename2 + wxT("\n"));
   
   if (numDirsToCompare >= 3)
      textBuffer.push_back(wxT("filename3=") + filename3 + wxT("\n"));
   
   #if DBGLVL_COMPARE_REPORT
   MessageInterface::ShowMessage(wxT("\n======================================== Compare Utility\n"));
   MessageInterface::ShowMessage(wxT("numDirsToCompare=%3\n"), numDirsToCompare);
   MessageInterface::ShowMessage(wxT("basefile =%s\n"), basefilename.c_str());
   MessageInterface::ShowMessage(wxT("filename1=%s\nfilename2=%s\nfilename3=%s\n"),
                                 filename1.c_str(), filename2.c_str(), filename3.c_str());
   #endif
   
   // open base file
   std::ifstream baseIn(basefilename.char_str());

   // open compare files
   std::ifstream in1(filename1.char_str());
   std::ifstream in2(filename2.char_str());
   std::ifstream in3(filename3.char_str());
   
   if (!baseIn)
   {
      textBuffer.push_back(wxT("Cannot open base file: ") +  basefilename + wxT("\n"));
      return textBuffer;
   }
   
   if (!in1)
   {
      textBuffer.push_back(wxT("Cannot open first file: ") + filename1 + wxT("\n"));
      return textBuffer;
   }
   
   if (numDirsToCompare >= 2)
      if (!in2)
      {
         textBuffer.push_back(wxT("Cannot open second file: ") + filename2 + wxT("\n"));
         return textBuffer;
      }
   
   if (numDirsToCompare >= 3)
      if (!in3)
      {
         textBuffer.push_back(wxT("Cannot open third file: ") + filename3 + wxT("\n"));
         return textBuffer;
      }

   
   char buffer[BUFFER_SIZE];
   wxString line0, line1, line2, line3;
   file1DiffCount = 0;
   file2DiffCount = 0;
   file3DiffCount = 0;
   int count = 1;
   
   
   //------------------------------------------
   // now start compare
   //------------------------------------------
   while (!baseIn.eof() && !in1.eof())
   {
      if (numDirsToCompare >= 2)
         if (in2.eof())
            break;
      
      if (numDirsToCompare >= 3)
         if (in3.eof())
            break;
      
      count++;
      
      #if DBGLVL_COMPARE_REPORT > 1
      MessageInterface::ShowMessage(wxT("============================== line # = %d\n"), count);
      #endif
      
      //----------------------------------------------------
      // base file
      //----------------------------------------------------
      baseIn.getline(buffer, BUFFER_SIZE-1);
      line0 = wxString::FromAscii(buffer);
      
      #if DBGLVL_COMPARE_REPORT > 2
      MessageInterface::ShowMessage(wxT("===> base file: buffer = %s\n"), buffer);
      #endif
      
      //----------------------------------------------------
      // file 1
      //----------------------------------------------------
      in1.getline(buffer, BUFFER_SIZE-1);
      line1 = wxString::FromAscii(buffer);
      
      #if DBGLVL_COMPARE_REPORT > 2
      MessageInterface::ShowMessage(wxT("===> file 1: buffer = %s\n"), buffer);
      #endif

      if (line0 != line1)
         file1DiffCount++;
      
      //----------------------------------------------------
      // file 2
      //----------------------------------------------------      
      if (numDirsToCompare >= 2)
      {
         in2.getline(buffer, BUFFER_SIZE-1);
         line2 = wxString::FromAscii(buffer);
      
         #if DBGLVL_COMPARE_REPORT > 2
         MessageInterface::ShowMessage(wxT("===> file 2: buffer = %s\n"), buffer);
         #endif

         if (line0 != line2)
            file2DiffCount++;
      }
      
      //----------------------------------------------------
      // file 3
      //----------------------------------------------------      
      if (numDirsToCompare >= 3)
      {
         in3.getline(buffer, BUFFER_SIZE-1);
         line3 = wxString::FromAscii(buffer);
      
         #if DBGLVL_COMPARE_REPORT > 2
         MessageInterface::ShowMessage(wxT("===> file 3: buffer = %s\n"), buffer);
         #endif
         
         if (line0 != line3)
            file3DiffCount++;
      }
   }
   
   // report the difference summary
   wxString outLine;
   outLine = wxT("Total lines compared: ") + ToString(count) + wxT("\n\n");
   textBuffer.push_back(outLine);

   #if DBGLVL_COMPARE_REPORT
   MessageInterface::ShowMessage(wxT("%s"), outLine.c_str());
   #endif
   
   outLine = wxT("File1 - Number of Lines different: ") + ToString(file1DiffCount) + wxT("\n");
   textBuffer.push_back(outLine);
   
   #if DBGLVL_COMPARE_REPORT
   MessageInterface::ShowMessage(wxT("%s"), outLine.c_str());
   #endif
   
   if (numDirsToCompare >= 2)
   {
      outLine = wxT("File2 - Number of Lines different: ") + ToString(file2DiffCount) + wxT("\n");
      textBuffer.push_back(outLine);
      
      #if DBGLVL_COMPARE_REPORT
      MessageInterface::ShowMessage(wxT("%s"), outLine.c_str());
      #endif
   }
   
   if (numDirsToCompare >= 3)
   {
      outLine = wxT("File3 - Number of Lines different: ") + ToString(file3DiffCount) + wxT("\n");
      textBuffer.push_back(outLine);
      
      #if DBGLVL_COMPARE_REPORT
      MessageInterface::ShowMessage(wxT("%s"), outLine.c_str());
      #endif
   }
      
   textBuffer.push_back(wxT("\n"));
   
   baseIn.close();
   in1.close();
   in2.close();
   in3.close();
   
   return textBuffer;
}
   
   
//------------------------------------------------------------------------------
// bool SkipHeaderLines(ifstream &in, StringArray &tokens)
//------------------------------------------------------------------------------
bool GmatFileUtil::SkipHeaderLines(std::ifstream &in, StringArray &tokens)
{
   char buffer[BUFFER_SIZE];
   bool dataFound = false;
   bool alphaFound = false;
   Real rval;
   int colCount = 0, fileCols = 0;
   char ch;
   StringTokenizer stk;
   wxString line;
   
   while (!dataFound)
   {
      if (in.eof())
         break;
      
      in.getline(buffer, BUFFER_SIZE-1);
      line = wxString::FromAscii(buffer);
      
      #if DBGLVL_COMPARE_REPORT > 1
      MessageInterface::ShowMessage(wxT("file length=%d, line = %s\n"),
                                    line.length(), line.c_str());
      #endif
      
      if (line.length() == 0)
         continue;
      
      alphaFound = false;
      
      for (unsigned int i=0; i<line.length(); i++)
      {
         ch = line[i];
         
         #if DBGLVL_COMPARE_REPORT > 1
         MessageInterface::ShowMessage(wxT("%c"), ch);
         #endif
         
         if (!isdigit(ch) && ch != wxT('.') && ch != wxT('e') && ch != wxT('E') && ch != wxT('-') &&
             ch != wxT(' '))
         {
            alphaFound = true;
            break;
         }
      }
      
      #if DBGLVL_COMPARE_REPORT > 1
      MessageInterface::ShowMessage(wxT("\n"));
      #endif
      
      if (alphaFound)
         continue;

      if (line.find(wxT("--")) != line.npos)
         continue;
      
      stk.Set(line, wxT(" "));
      tokens = stk.GetAllTokens();
      fileCols = tokens.size();

      colCount = 0;
      for (int i=0; i<fileCols; i++)
      {
         rval = atof(tokens[i].char_str());
         colCount++;
         
         #if DBGLVL_COMPARE_REPORT > 1
         MessageInterface::ShowMessage(wxT("rval=%f\n"), rval);
         #endif
      }

      if (colCount == fileCols)
         dataFound = true;
   }

   return dataFound;
}
