//$Id: FileManager.cpp 9518 2011-04-30 22:32:04Z djcinsb $
//------------------------------------------------------------------------------
//                            FileManager
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
// Author: Linda Jun, NASA/GSFC
// Created: 2004/04/02
/**
 * Implements FileManager class. This is singleton class which manages
 * list of file paths and names.
 */
//------------------------------------------------------------------------------

#include "FileManager.hpp"
#include "MessageInterface.hpp"
#include "UtilityException.hpp"
#include "StringUtil.hpp"
#include "FileTypes.hpp"          // for GmatFile::MAX_PATH_LEN
#include "FileUtil.hpp"           // for GmatFileUtil::
#include "StringTokenizer.hpp"    // for StringTokenizer()
#include "GmatGlobal.hpp"         // for SetTestingMode()
#include <fstream>
#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <iomanip>
#include <wx/filename.h>

#include <algorithm>                    // Required for GCC 4.3

#ifndef _MSC_VER  // if not Microsoft Visual C++
#include <dirent.h>
#endif

// For adding default input path and files
#define __FM_ADD_DEFAULT_INPUT__

//#define DEBUG_FILE_MANAGER
//#define DEBUG_FUNCTION_PATH
//#define DEBUG_ADD_FILETYPE
//#define DEBUG_FILE_PATH
//#define DEBUG_SET_PATH
//#define DEBUG_READ_STARTUP_FILE
//#define DEBUG_WRITE_STARTUP_FILE
//#define DEBUG_PLUGIN_DETECTION
//#define DEBUG_FILE_RENAME
//#define DEBUG_MAPPING

//---------------------------------
// static data
//---------------------------------
const wxString
FileManager::FILE_TYPE_STRING[FileTypeCount] =
{
   // file path
   wxT("BEGIN_OF_PATH"),
   wxT("OUTPUT_PATH"),
   wxT("DE_PATH"),
   wxT("SPK_PATH"),
   wxT("EARTH_POT_PATH"),
   wxT("LUNA_POT_PATH"),
   wxT("VENUS_POT_PATH"),
   wxT("MARS_POT_PATH"),
   wxT("PLANETARY_COEFF_PATH"),
   wxT("TIME_PATH"),
   wxT("TEXTURE_PATH"),
   wxT("MEASUREMENT_PATH"),
   wxT("EPHEM_PATH"),
   wxT("GUI_CONFIG_PATH"),
   wxT("SPLASH_PATH"),
   wxT("ICON_PATH"),
   wxT("STAR_PATH"),
   wxT("MODEL_PATH"),
   wxT("END_OF_PATH"),
   // file name
   wxT("LOG_FILE"),
   wxT("REPORT_FILE"),
   wxT("SPLASH_FILE"),
   wxT("TIME_COEFF_FILE"),
   // specific file name
   wxT("DE405_FILE"),
   wxT("PLANETARY_SPK_FILE"),
   wxT("JGM2_FILE"),
   wxT("JGM3_FILE"),
   wxT("EGM96_FILE"),
   wxT("LP165P_FILE"),
   wxT("MGNP180U_FILE"),
   wxT("MARS50C_FILE"),
   wxT("EOP_FILE"),
   wxT("PLANETARY_COEFF_FILE"),
   wxT("NUTATION_COEFF_FILE"),
   wxT("LEAP_SECS_FILE"),
   wxT("LSK_FILE"),
   wxT("PERSONALIZATION_FILE"),
   wxT("MAIN_ICON_FILE"),
   wxT("STAR_FILE"),
   wxT("CONSTELLATION_FILE"),
   wxT("SPACECRAFT_MODEL_FILE"),
   wxT("HELP_FILE"),
};

FileManager* FileManager::theInstance = NULL;


//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// FileManager* Instance()
//------------------------------------------------------------------------------
FileManager* FileManager::Instance()
{
   if (theInstance == NULL)
      theInstance = new FileManager;
   return theInstance;
}


//------------------------------------------------------------------------------
// ~FileManager()
//------------------------------------------------------------------------------
FileManager::~FileManager()
{
   for (std::map<wxString, FileInfo*>::iterator pos = mFileMap.begin();
        pos != mFileMap.end(); ++pos)
   {
      if (pos->second)
      {
         #ifdef DEBUG_FILE_MANAGER
         MessageInterface::ShowMessage
            (wxT("FileManager::~FileManager deleting %s\n"), pos->first.c_str());
         #endif

         delete pos->second;
      }
   }
}


//------------------------------------------------------------------------------
// wxString GetPathSeparator()
//------------------------------------------------------------------------------
/**
 * @return path separator; wxT("/") or "\\" dependends on the platform
 */
//------------------------------------------------------------------------------
wxString FileManager::GetPathSeparator()
{
   wxString sep = wxT("/");

   // Just return wxT("/") for all operating system for consistency (LOJ: 2011.03.18)
   #if 0
   char *buffer;
   buffer = getenv(wxT("OS"));
   if (buffer != NULL)
   {
      //MessageInterface::ShowMessage(wxT("Current OS is %s\n"), buffer);
      wxString osStr(buffer);

      if (osStr.find(wxT("Windows")) != osStr.npos)
         sep = "\\";
   }
   #endif
   
   return sep;
}


//------------------------------------------------------------------------------
// wxString GetCurrentPath()
//------------------------------------------------------------------------------
/*
 * Note: This function calls getcwd() which is defiend in <dirent>. There is a
 *       problem compling with VC++ compiler, so until it is resolved, it will
 *       always return blank if it is compiled with VC++ compiler.
 *
 * @return  The current working directory, generally the application path.
 *
 */
//------------------------------------------------------------------------------
wxString FileManager::GetCurrentPath()
{
   wxString currPath;

#ifdef _MSC_VER  // if Microsoft Visual C++
   currPath = wxT(".");
#else
   currPath = wxFileName::GetCwd();
#endif

   return currPath;
}


//------------------------------------------------------------------------------
// bool DoesDirectoryExist(const wxString &dirPath)
//------------------------------------------------------------------------------
/*
 * @return  true  If directory exist, false otherwise
 */
//------------------------------------------------------------------------------
bool FileManager::DoesDirectoryExist(const wxString &dirPath)
{
   return GmatFileUtil::DoesDirectoryExist(dirPath);
}


//------------------------------------------------------------------------------
// bool DoesFileExist(const wxString &filename)
//------------------------------------------------------------------------------
bool FileManager::DoesFileExist(const wxString &filename)
{
   FILE * pFile;
   pFile = fopen (filename.char_str(), "rt+");

   if (pFile!=NULL)
   {
      fclose (pFile);
      return true;
   }
   else
   {
      return false;
   }
}


//------------------------------------------------------------------------------
// bool RenameFile(const wxString &oldName, const wxString &newName,
//                 Integer &retCode, bool overwriteIfExists = false)
//------------------------------------------------------------------------------
bool FileManager::RenameFile(const wxString &oldName,
                             const wxString &newName,
                             Integer &retCode, bool overwriteIfExists)
{
   retCode = 0;
   bool oldExists = DoesFileExist(oldName);
   bool newExists = DoesFileExist(newName);
   #ifdef DEBUG_FILE_RENAME
      MessageInterface::ShowMessage(wxT("FM::Rename, old file (%s) exists = %s\n"),
            oldName.c_str(), (oldExists? wxT("true") : wxT("false")));
      MessageInterface::ShowMessage(wxT("FM::Rename, new file (%s) exists = %s\n"),
            newName.c_str(), (newExists? wxT("true") : wxT("false")));
   #endif
   // if a file with the old name does not exist, we cannot do anything
   if (!oldExists)
   {
      wxString errmsg = wxT("Error renaming file \"");
      errmsg += oldName + wxT("\" to \"");
      errmsg += newName + wxT("\": file \"");
      errmsg += oldName + wxT("\" does not exist.\n");
      throw UtilityException(errmsg);
   }

   // if a file with the new name does not exist, or exists but we are
   // supposed to overwrite it, try to do the rename
   if ((!newExists) || (newExists && overwriteIfExists))
   {
      #ifdef DEBUG_FILE_RENAME
         MessageInterface::ShowMessage(wxT("FM::Rename, attempting to rename %s to %s\n"),
               oldName.c_str(), newName.c_str());
      #endif
      retCode = rename(oldName.char_str(), newName.char_str()); // overwriting is platform-dependent!!!!
      if (retCode == 0)
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   else // it exists but we are not to overwrite it
      return false;
}

//------------------------------------------------------------------------------
// wxString GetStartupFileDir()
//------------------------------------------------------------------------------
/*
 * Returns startup file name without directory.
 */
//------------------------------------------------------------------------------
wxString FileManager::GetStartupFileDir()
{
   return mStartupFileDir;
}


//------------------------------------------------------------------------------
// wxString GetStartupFileName()
//------------------------------------------------------------------------------
/*
 * Returns startup file name without directory.
 */
//------------------------------------------------------------------------------
wxString FileManager::GetStartupFileName()
{
   return mStartupFileName;
}


//------------------------------------------------------------------------------
// wxString GetFullStartupFilePath()
//------------------------------------------------------------------------------
/*
 * Returns startup file directory and name
 */
//------------------------------------------------------------------------------
wxString FileManager::GetFullStartupFilePath()
{
   #ifdef DEBUG_STARTUP_FILE
   MessageInterface::ShowMessage
      (wxT("FileManager::GetFullStartupFilePath() mStartupFileDir='%s', ")
       wxT("mStartupFileName='%s'\n"), mStartupFileDir.c_str(), mStartupFileName.c_str());
   #endif

   if (mStartupFileDir == wxT(""))
      return mStartupFileName;
   else
      return mStartupFileDir + mStartupFileName;
}


//------------------------------------------------------------------------------
// void ReadStartupFile(const wxString &fileName = wxT(""))
//------------------------------------------------------------------------------
/**
 * Reads GMAT startup file.
 *
 * @param <fileName> startup file name.
 *
 */
//------------------------------------------------------------------------------
void FileManager::ReadStartupFile(const wxString &fileName)
{
   #ifdef DEBUG_READ_STARTUP_FILE
   MessageInterface::ShowMessage
      (wxT("FileManager::ReadStartupFile() entered, fileName='%s'\n"), fileName.c_str());
   #endif

   RefreshFiles();

   wxString line;
   mSavedComments.clear();

   wxString tmpStartupDir;
   wxString tmpStartupFile;
   wxString tmpStartupFilePath;

   if (fileName == wxT(""))
   {
      tmpStartupDir = wxT("");
      tmpStartupFile = mStartupFileName;
      tmpStartupFilePath = mStartupFileName;
   }
   else
   {
      tmpStartupDir = GmatFileUtil::ParsePathName(fileName);
      tmpStartupFile = GmatFileUtil::ParseFileName(fileName);

      if (tmpStartupDir == wxT(""))
         tmpStartupFilePath = tmpStartupFile;
      else
         tmpStartupFilePath = tmpStartupDir + mPathSeparator + tmpStartupFile;
   }

   #ifdef DEBUG_READ_STARTUP_FILE
   MessageInterface::ShowMessage
      (wxT("FileManager::ReadStartupFile() reading '%s'\n"), tmpStartupFilePath.c_str());
   #endif

   std::ifstream mInStream(tmpStartupFilePath.char_str());

   if (!mInStream)
      throw UtilityException
         (wxT("FileManager::ReadStartupFile() cannot open:") + tmpStartupFilePath);

   while (!mInStream.eof())
   {
      // Use cross-platform GetLine
      GmatFileUtil::GetLine(&mInStream, line);
      
      #ifdef DEBUG_READ_STARTUP_FILE
      MessageInterface::ShowMessage(wxT("line=%s\n"), line.c_str());
      #endif

      // Skip empty line or comment line
      if (line.length() > 0)     // Crashes in VS 2010 debugger without this
      {
         if (line[0] == wxT('\0') || line[0] == wxT('#'))
         {
            // save line with ## in the first col
            if (line.size() > 1 && line[1] == wxT('#'))
               mSavedComments.push_back(line);
            continue;
         }
      }
      else
         continue;

      wxString type, equal, name;
      wxString ss;

      ss << line;
      wxStringInputStream ssStringInputStream(ss);
      wxTextInputStream ssTextInputStream(ssStringInputStream);
      ssTextInputStream >> type >> equal;

      if (equal != wxT("="))
      {
         mInStream.close();
         throw UtilityException
            (wxT("FileManager::ReadStartupFile() expecting '=' at line:\n") +
             line + wxT("\n"));
      }

      // To fix bug 1916 (LOJ: 2010.10.08)
      // Since >> uses space as deliminter, we cannot use it.
      // So use GmatStringUtil::DecomposeBy() instead.
      //ss >> name;
      
      StringArray parts = GmatStringUtil::DecomposeBy(line, wxT("="));
      name = parts[1];
      name = GmatStringUtil::Trim(name);
      
      #ifdef DEBUG_READ_STARTUP_FILE
      MessageInterface::ShowMessage(wxT("type=%s, name=%s\n"), type.c_str(), name.c_str());
      #endif
      
      
      if (type == wxT("RUN_MODE"))
      {
         mRunMode = name;
         if (name == wxT("TESTING"))
            GmatGlobal::Instance()->SetRunMode(GmatGlobal::TESTING);
         else if (name == wxT("TESTING_NO_PLOTS"))
            GmatGlobal::Instance()->SetRunMode(GmatGlobal::TESTING_NO_PLOTS);
         else if (name == wxT("EXIT_AFTER_RUN"))
            GmatGlobal::Instance()->SetRunMode(GmatGlobal::EXIT_AFTER_RUN);
      }
      else if (type == wxT("MATLAB_MODE"))
      {
         mMatlabMode = name;
         if (name == wxT("SINGLE"))
            GmatGlobal::Instance()->SetMatlabMode(GmatGlobal::SINGLE_USE);
         else if (name == wxT("SHARED"))
            GmatGlobal::Instance()->SetMatlabMode(GmatGlobal::SHARED);
         else if (name == wxT("NO_MATLAB"))
            GmatGlobal::Instance()->SetMatlabMode(GmatGlobal::NO_MATLAB);
      }
      else if (type == wxT("DEBUG_MATLAB"))
      {
         if (name == wxT("ON"))
         {
            mDebugMatlab = name;
            GmatGlobal::Instance()->SetMatlabDebug(true);
         }
      }
      else
      {
         // Ignore old VERSION specification (2011.03.18)
         if (type != wxT("VERSION"))
            AddFileType(type, name);
      }
   } // end While()
   
   // Since we set all output to ./ as default, we don't need this (LOJ: 2011.03.17)
   // Set EPHEM_PATH to OUTPUT_PATH from the startup file if not set
   // so that ./output directory is not required when writing the ephemeris file.
//    if (mPathMap[wxT("EPHEM_PATH")] == wxT("./output/") &&
//        mPathMap[wxT("OUTPUT_PATH")] != wxT("./files/output/"))
//    {
//       mPathMap[wxT("EPHEM_PATH")] = mPathMap[wxT("OUTPUT_PATH")];
//       #ifdef DEBUG_READ_STARTUP_FILE
//       MessageInterface::ShowMessage
//          (wxT("==> EPHEM_PATH set to '%s'\n"), mPathMap[wxT("EPHEM_PATH")].c_str());
//       #endif
//    }
   
   // add potential files by type names
   AddAvailablePotentialFiles();
   
   // save good startup file
   mStartupFileDir = tmpStartupDir;
   mStartupFileName = tmpStartupFile;
   
   // now use log file from the startup file
   MessageInterface::SetLogFile(GetAbsPathname(wxT("LOG_FILE")));
   MessageInterface::SetLogEnable(true);
   mInStream.close();
   
   #ifdef DEBUG_MAPPING
   ShowMaps(wxT("In ReadStartupFile()"));
   #endif
}


//------------------------------------------------------------------------------
// void WriteStartupFile(const wxString &fileName = wxT(""))
//------------------------------------------------------------------------------
/**
 * Reads GMAT startup file.
 *
 * @param <fileName> startup file name.
 *
 * @exception UtilityException thrown if file cannot be opened
 */
//------------------------------------------------------------------------------
void FileManager::WriteStartupFile(const wxString &fileName)
{
   wxString outFileName = wxT("gmat_startup_file.new.txt");
   mPathWrittenOuts.clear();
   mFileWrittenOuts.clear();
   
   if (fileName != wxT(""))
      outFileName = fileName;
   
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage
      (wxT("FileManager::WriteStartupFile() entered, outFileName = %s\n"),
       outFileName.c_str());
   #endif

   std::ofstream outStream(outFileName.char_str());

   if (!outStream)
      throw UtilityException
         (wxT("FileManager::WriteStartupFile() cannot open:") + fileName);

   //---------------------------------------------
   // write header
   //---------------------------------------------
   WriteHeader(outStream);
   
   // set left justified
   outStream.setf(std::ios::left);
   
   // don't write CURRENT_PATH
   mPathWrittenOuts.push_back(wxT("CURRENT_PATH"));
   
   //---------------------------------------------
   // write RUN_MODE if not blank
   //---------------------------------------------
   if (mRunMode != wxT(""))
   {
      #ifdef DEBUG_WRITE_STARTUP_FILE
      MessageInterface::ShowMessage(wxT("   .....Writing RUN_MODE\n"));
      #endif
      outStream << std::setw(22) << wxT("RUN_MODE") << wxT(" = ") << mRunMode << wxT("\n");
   }
   
   // Write other option as comments
   outStream << std::setw(22) << wxT("#RUN_MODE") << wxT(" = TESTING\n");
   outStream << std::setw(22) << wxT("#RUN_MODE") << wxT(" = TESTING_NO_PLOTS\n");
   outStream << std::setw(22) << wxT("#RUN_MODE") << wxT(" = EXIT_AFTER_RUN\n");
   
   //---------------------------------------------
   // write MATLAB_MODE if not blank
   //---------------------------------------------
   if (mMatlabMode != wxT(""))
   {
      #ifdef DEBUG_WRITE_STARTUP_FILE
      MessageInterface::ShowMessage(wxT("   .....Writing RUN_MODE\n"));
      #endif
      outStream << std::setw(22) << wxT("MATLAB_MODE") << wxT(" = ") << mMatlabMode << wxT("\n");
   }
   
   // Write other option as comments
   outStream << std::setw(22) << wxT("#MATLAB_MODE") << wxT(" = SINGLE\n");
   outStream << std::setw(22) << wxT("#MATLAB_MODE") << wxT(" = SHARED\n");
   outStream << std::setw(22) << wxT("#MATLAB_MODE") << wxT(" = NO_MATLAB\n");
   
   //---------------------------------------------
   // write DEBUG_MATLAB if not blank
   //---------------------------------------------
   if (mDebugMatlab != wxT(""))
   {
      #ifdef DEBUG_WRITE_STARTUP_FILE
      MessageInterface::ShowMessage(wxT("   .....Writing RUN_MODE\n"));
      #endif
      outStream << std::setw(22) << wxT("DEBUG_MATLAB") << wxT(" = ") << mDebugMatlab << wxT("\n");
   }
   
   if (mRunMode != wxT("") || mMatlabMode != wxT("") || mDebugMatlab != wxT(""))
      outStream << wxT("#-----------------------------------------------------------\n");
   
   //---------------------------------------------
   // write ROOT_PATH next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing ROOT_PATH path\n"));
   #endif
   outStream << std::setw(22) << wxT("ROOT_PATH") << wxT(" = ") << mPathMap[wxT("ROOT_PATH")]
             << wxT("\n");
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("ROOT_PATH"));
   
   //---------------------------------------------
   // write PLUGIN next 
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing PLUGIN\n"));
   #endif
   if (mPluginList.size() > 0)
   {
      for (UnsignedInt i = 0; i < mPluginList.size(); ++i)
      {
         outStream << std::setw(22) << wxT("PLUGIN") << wxT(" = ") << mPluginList[i]
                   << wxT("\n");
      }
      outStream << wxT("#-----------------------------------------------------------\n");
   }
   
   //---------------------------------------------
   // write OUTPUT_PATH and output files next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing OUTPUT_PATH paths\n"));
   #endif
   outStream << std::setw(22) << wxT("OUTPUT_PATH") << wxT(" = ")
             << mPathMap[wxT("OUTPUT_PATH")] << wxT("\n");
   WriteFiles(outStream, wxT("LOG_"));
   WriteFiles(outStream, wxT("REPORT_"));
   WriteFiles(outStream, wxT("SCREENSHOT_"));
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("OUTPUT_PATH"));
   
   //---------------------------------------------
   // write MEASUREMENT_PATH next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing MEASUREMENT_PATH paths\n"));
   #endif
   outStream << std::setw(22) << wxT("MEASUREMENT_PATH") << wxT(" = ")
             << mPathMap[wxT("MEASUREMENT_PATH")] << wxT("\n");
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("MEASUREMENT_PATH"));
   
   //---------------------------------------------
   // write the EPHEM_PATH next if set
   //---------------------------------------------
   if (mPathMap[wxT("EPHEM_PATH")] != wxT("./output/"))
   {
      #ifdef DEBUG_WRITE_STARTUP_FILE
      MessageInterface::ShowMessage(wxT("   .....Writing EPHEM_PATH path\n"));
      #endif
      outStream << std::setw(22) << wxT("EPHEM_PATH") << wxT(" = ")
                << mPathMap[wxT("EPHEM_PATH")];
      outStream << wxT("\n#---------------------------------------------")
            wxT("--------------\n");
      mPathWrittenOuts.push_back(wxT("EPHEM_PATH"));
   }
   
   //---------------------------------------------
   // write GMAT_FUNCTION_PATH next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing GMAT_FUNCTION_PATH paths\n"));
   #endif
   bool isEmptyPath = true;
   for (std::map<wxString, wxString>::iterator pos = mPathMap.begin();
        pos != mPathMap.end(); ++pos)
   {
      if (pos->first == wxT("GMAT_FUNCTION_PATH"))
      {
         // Write all GmatFunction paths
         std::list<wxString>::iterator listpos = mGmatFunctionPaths.begin();
         while (listpos != mGmatFunctionPaths.end())
         {
            outStream << std::setw(22) << pos->first << wxT(" = ")
                      << *listpos << wxT("\n");
            ++listpos;
         }
         isEmptyPath = false;
         break;
      }
   }
   if (isEmptyPath)
      outStream << std::setw(22) << wxT("#GMAT_FUNCTION_PATH ") << wxT(" = ") << wxT("\n");
   
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("GMAT_FUNCTION_PATH"));
   
   //---------------------------------------------
   // write MATLAB_FUNCTION_PATH next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing MATLAB_FUNCTION_PATH paths\n"));
   #endif
   isEmptyPath = true;
   for (std::map<wxString, wxString>::iterator pos = mPathMap.begin();
        pos != mPathMap.end(); ++pos)
   {
      if (pos->first == wxT("MATLAB_FUNCTION_PATH"))
      {
         // Write all GmatFunction paths
         std::list<wxString>::iterator listpos = mMatlabFunctionPaths.begin();
         while (listpos != mMatlabFunctionPaths.end())
         {
            outStream << std::setw(22) << pos->first << wxT(" = ") << *listpos << wxT("\n");
            ++listpos;
         }
         break;
      }
   }
   if (isEmptyPath)
      outStream << std::setw(22) << wxT("#MATLAB_FUNCTION_PATH ") << wxT(" = ") << wxT("\n");
   
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("MATLAB_FUNCTION_PATH"));
   
   //---------------------------------------------
   // write DATA_PATH next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing DATA_PATH path\n"));
   #endif
   outStream << std::setw(22) << wxT("DATA_PATH") << wxT(" = ") << mPathMap[wxT("DATA_PATH")]
             << wxT("\n");
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("DATA_PATH"));
   
   //---------------------------------------------
   // write any relative path used in SPK_PATH
   //---------------------------------------------
   wxString spkPath = mPathMap[wxT("SPK_PATH")];
   if (spkPath.find(wxT("_PATH")) != spkPath.npos)
   {
      wxString relPath = GmatFileUtil::ParseFirstPathName(spkPath, false);
      if (find(mPathWrittenOuts.begin(), mPathWrittenOuts.end(), relPath) ==
          mPathWrittenOuts.end())
      {
         #ifdef DEBUG_WRITE_STARTUP_FILE
         MessageInterface::ShowMessage(wxT("   .....Writing %s\n"), relPath.c_str());
         #endif
         outStream << std::setw(22) << relPath << wxT(" = ") << mPathMap[relPath] << wxT("\n");
         outStream << wxT("#-----------------------------------------------------------\n");
         mPathWrittenOuts.push_back(relPath);
      }
   }
   
   //---------------------------------------------
   // write the SPK_PATH and SPK file next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing SPK path\n"));
   #endif
   outStream << std::setw(22) << wxT("SPK_PATH") << wxT(" = ")
             << mPathMap[wxT("SPK_PATH")] << wxT("\n");
   WriteFiles(outStream, wxT("SPK"));
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("SPK_PATH"));
   
   //---------------------------------------------
   // write the DE_PATH and DE file next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing DE path\n"));
   #endif
   outStream << std::setw(22) << wxT("DE_PATH") << wxT(" = ")
             << mPathMap[wxT("DE_PATH")] << wxT("\n");
   WriteFiles(outStream, wxT("DE405"));
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("DE_PATH"));
   
   //---------------------------------------------
   // write the PLANETARY_COEFF_PATH and files next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing PLANETARY_COEFF_PATH path\n"));
   #endif
   outStream << std::setw(22) << wxT("PLANETARY_COEFF_PATH") << wxT(" = ")
             << mPathMap[wxT("PLANETARY_COEFF_PATH")] << wxT("\n");
   WriteFiles(outStream, wxT("EOP_FILE"));
   WriteFiles(outStream, wxT("PLANETARY_COEFF_FILE"));
   WriteFiles(outStream, wxT("NUTATION_COEFF_FILE"));
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("PLANETARY_COEFF_PATH"));

   //---------------------------------------------
   // write the TIME_PATH and TIME file next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing TIME path\n"));
   #endif
   outStream << std::setw(22) << wxT("TIME_PATH") << wxT(" = ") << mPathMap[wxT("TIME_PATH")] << wxT("\n");
   WriteFiles(outStream, wxT("LEAP_"));
   WriteFiles(outStream, wxT("LSK_"));
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("TIME_PATH"));

   //---------------------------------------------
   // write *_POT_PATH and files next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing *_POT_PATH paths\n"));
   #endif
   for (std::map<wxString, wxString>::iterator pos = mPathMap.begin();
        pos != mPathMap.end(); ++pos)
   {
      if (pos->first.find(wxT("_POT_")) != wxString::npos)
      {
         outStream << std::setw(22) << pos->first << wxT(" = ")
                   << pos->second << wxT("\n");
         mPathWrittenOuts.push_back(pos->first);
      }
   }
   outStream << wxT("#-----------------------------------------------------------\n");
   WriteFiles(outStream, wxT("POT_FILE"));
   WriteFiles(outStream, wxT("EGM96"));
   WriteFiles(outStream, wxT("JGM"));
   WriteFiles(outStream, wxT("MARS50C"));
   WriteFiles(outStream, wxT("MGNP180U"));
   WriteFiles(outStream, wxT("LP165P"));
   
   outStream << wxT("#-----------------------------------------------------------\n");

   //---------------------------------------------
   // write the GUI_CONFIG_PATH and files next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing GUI_CONFIG_PATH path\n"));
   #endif
   outStream << std::setw(22) << wxT("GUI_CONFIG_PATH") << wxT(" = ")
             << mPathMap[wxT("GUI_CONFIG_PATH")] << wxT("\n");
   WriteFiles(outStream, wxT("PERSONALIZATION_FILE"));
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("GUI_CONFIG_PATH"));
   
   //---------------------------------------------
   // write the ICON_PATH and files next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing ICON_PATH path\n"));
   #endif
   outStream << std::setw(22) << wxT("ICON_PATH") << wxT(" = ")
             << mPathMap[wxT("ICON_PATH")] << wxT("\n");
   WriteFiles(outStream, wxT("ICON_FILE"));
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("ICON_PATH"));

   //---------------------------------------------
   // write the SPLASH_PATH and files next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing SPLASH_PATH path\n"));
   #endif
   outStream << std::setw(22) << wxT("SPLASH_PATH") << wxT(" = ")
             << mPathMap[wxT("SPLASH_PATH")] << wxT("\n");
   WriteFiles(outStream, wxT("SPLASH_FILE"));
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("SPLASH_PATH"));
   
   //---------------------------------------------
   // write the TEXTURE_PATH and files next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing TEXTURE_PATH path\n"));
   #endif
   outStream << std::setw(22) << wxT("TEXTURE_PATH") << wxT(" = ")
             << mPathMap[wxT("TEXTURE_PATH")] << wxT("\n");
   WriteFiles(outStream, wxT("TEXTURE_FILE"));
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("TEXTURE_PATH"));

   //---------------------------------------------
   // write the STAR_PATH and files next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing STAR_PATH path\n"));
   #endif
   outStream << std::setw(22) << wxT("STAR_PATH") << wxT(" = ")
             << mPathMap[wxT("STAR_PATH")] << wxT("\n");
   WriteFiles(outStream, wxT("STAR_FILE"));
   WriteFiles(outStream, wxT("CONSTELLATION_FILE"));
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("STAR_PATH"));

   //---------------------------------------------
   // write the MODEL_PATH and files next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing MODEL_PATH path\n"));
   #endif
   outStream << std::setw(22) << wxT("MODEL_PATH") << wxT(" = ")
             << mPathMap[wxT("MODEL_PATH")] << wxT("\n");
   WriteFiles(outStream, wxT("SPACECRAFT_MODEL_FILE"));
   outStream << wxT("#-----------------------------------------------------------\n");
   mPathWrittenOuts.push_back(wxT("MODEL_PATH"));
   
   //---------------------------------------------
   // write the HELP_FILE next
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing HELP_FILE\n"));
   #endif
   
   if (GetFilename(wxT("HELP_FILE")) == wxT(""))
      outStream << std::setw(22) << wxT("#HELP_FILE ") << wxT(" = ") << wxT("\n");
   else
      WriteFiles(outStream, wxT("HELP_FILE"));
   outStream << wxT("#-----------------------------------------------------------\n");
   mFileWrittenOuts.push_back(wxT("HELP_FILE"));
   
   //---------------------------------------------
   // write rest of paths and files
   //---------------------------------------------
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing rest of paths and files\n"));
   #endif
   WriteFiles(outStream, wxT("-OTHER-PATH-"));
   WriteFiles(outStream, wxT("-OTHER-"));
   outStream << wxT("#-----------------------------------------------------------\n");
   
   //---------------------------------------------
   // write saved comments
   //---------------------------------------------
   if (!mSavedComments.empty())
   {
      #ifdef DEBUG_WRITE_STARTUP_FILE
      MessageInterface::ShowMessage(wxT("   .....Writing saved comments\n"));
      #endif
      outStream << wxT("# Saved Comments\n");
      outStream << wxT("#-----------------------------------------------------------\n");
      for (UnsignedInt i=0; i<mSavedComments.size(); i++)
         outStream << mSavedComments[i] << wxT("\n");
      outStream << wxT("#-----------------------------------------------------------\n");
   }

   outStream << wxT("\n");
   outStream.close();

   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("FileManager::WriteStartupFile() exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
// wxString GetRootPath()
//------------------------------------------------------------------------------
/**
 * Retrives root pathname.
 *
 * @return file pathname if path type found.
 */
//------------------------------------------------------------------------------
wxString FileManager::GetRootPath()
{
   return mPathMap[wxT("ROOT_PATH")];
}


//------------------------------------------------------------------------------
// wxString GetPathname(const FileType type)
//------------------------------------------------------------------------------
/**
 * Retrives absolute path for the type without filename.
 *
 * @param <type> enum file type of which path to be returned.
 *
 * @return file pathname if path type found.
 * @exception thrown if enum type is out of bounds.
 */
//------------------------------------------------------------------------------
wxString FileManager::GetPathname(const FileType type)
{
   if (type >=0 && type < FileTypeCount)
      return GetPathname(FILE_TYPE_STRING[type]);
   
   wxString ss(wxT(""));
   ss << wxT("FileManager::GetPathname() enum type: ") << type
      << wxT(" is out of bounds\n");
   
   throw UtilityException(ss);
}


//------------------------------------------------------------------------------
// wxString GetPathname(const wxString &typeName)
//------------------------------------------------------------------------------
/**
 * Retrives absolute pathname for the type name without filename.
 *
 * @param <typeName> file type name of which pathname to be returned.
 *
 * @return pathname if type found.
 * @exception thrown if type cannot be found.
 */
//------------------------------------------------------------------------------
wxString FileManager::GetPathname(const wxString &typeName)
{
   wxString fileType = GmatStringUtil::ToUpper(typeName);
   
   #ifdef DEBUG_FILE_PATH
   MessageInterface::ShowMessage
      (wxT("FileManager::GetPathname() entered, flleType='%s'\n"), fileType.c_str());
   #endif

   wxString pathname;
   bool nameFound = false;
   
   // if typeName contains _PATH
   if (fileType.find(wxT("_PATH")) != fileType.npos)
   {
      if (mPathMap.find(fileType) != mPathMap.end())
      {
         pathname = mPathMap[fileType];
         nameFound = true;
      }
   }
   else
   {
      // typeName contains _FILE      
      if (mFileMap.find(fileType) != mFileMap.end())
      {
         pathname = mFileMap[fileType]->mPath;
         nameFound = true;
      }
   }
   
   if (nameFound)
   {
      // Replace relative path with absolute path
      wxString abspath = ConvertToAbsPath(pathname);
      
      #ifdef DEBUG_FILE_PATH
      MessageInterface::ShowMessage
         (wxT("FileManager::GetPathname() returning '%s'\n"), abspath.c_str());
      #endif
      
      return abspath;
   }
   else
   {
      throw UtilityException(wxT("FileManager::GetPathname() file type: ") + typeName +
                             wxT(" is unknown\n"));
   }
}


//------------------------------------------------------------------------------
// wxString GetFilename(const FileType type)
//------------------------------------------------------------------------------
/**
 * Retrives filename for the type without path.
 *
 * @param <type> enum file type of which filename to be returned.
 *
 * @return file filename if file type found
 * @exception thrown if enum type is out of bounds
 */
//------------------------------------------------------------------------------
wxString FileManager::GetFilename(const FileType type)
{
   bool nameFound = false;
   wxString name;
   if (type >=0 && type < FileTypeCount)
   {
      name = GetFilename(FILE_TYPE_STRING[type]);
      nameFound = true;
   }
   
   if (nameFound)
   {
      name = GmatFileUtil::ParseFileName(name);
      return name;
   }
   
   wxString ss(wxT(""));
   ss << wxT("FileManager::GetFilename() enum type: ") << type
      << wxT(" is out of bounds\n");

   throw UtilityException(ss);
}


//------------------------------------------------------------------------------
// wxString GetFilename(const wxString &typeName)
//------------------------------------------------------------------------------
/**
 * Retrives filename for the type name without path.
 *
 * @param <type> file type name of which filename to be returned.
 *
 * @return file filename if file type found
 * @exception thrown if type cannot be found.
 */
//------------------------------------------------------------------------------
wxString FileManager::GetFilename(const wxString &typeName)
{
   bool nameFound = false;
   wxString name;
   if (mFileMap.find(typeName) != mFileMap.end())
   {
      name = mFileMap[typeName]->mFile;
      nameFound = true;
   }
   
   if (nameFound)
   {
      name = GmatFileUtil::ParseFileName(name);
      return name;
   }
   
   throw UtilityException(wxT("FileManager::GetFilename() file type: ") + typeName +
                           wxT(" is unknown\n"));
}


//------------------------------------------------------------------------------
// wxString GetFullPathname(const FileType type)
//------------------------------------------------------------------------------
/**
 * Retrieves full pathname for the type.
 *
 * @param <type> file type of which filename to be returned.
 *
 * @return file pathname if file type found
 * @exception thrown if enum type is out of bounds
 */
//------------------------------------------------------------------------------
wxString FileManager::GetFullPathname(const FileType type)
{
   return GetAbsPathname(type);
}


//------------------------------------------------------------------------------
// wxString GetFullPathname(const wxString &typeName)
//------------------------------------------------------------------------------
/**
 * Retrives full pathname for the type name.
 *
 * @param <type> file type name of which filename to be returned.
 *
 * @return file pathname if file type name found
 * @exception thrown if type cannot be found.
 */
//------------------------------------------------------------------------------
wxString FileManager::GetFullPathname(const wxString &typeName)
{
   return GetAbsPathname(typeName);
}


//------------------------------------------------------------------------------
// wxString GetAbsPathname(const FileType type)
//------------------------------------------------------------------------------
/**
 * Retrieves full pathname for the type.
 *
 * @param <type> file type of which filename to be returned.
 *
 * @return file pathname if file type found
 * @exception thrown if enum type is out of bounds
 */
//------------------------------------------------------------------------------
wxString FileManager::GetAbsPathname(const FileType type)
{

   if (type >=0 && type < FileTypeCount)
      return GetAbsPathname(FILE_TYPE_STRING[type]);

   wxString ss(wxT(""));
   ss << wxT("FileManager::GetAbsPathname() enum type: ") << type <<
      wxT(" is out of bounds\n");

   throw UtilityException(ss);
}


//------------------------------------------------------------------------------
// wxString GetAbsPathname(const wxString &typeName)
//------------------------------------------------------------------------------
/**
 * Retrives full pathname for the type name.
 *
 * @param <type> file type name of which filename to be returned.
 *
 * @return file pathname if file type name found
 * @exception thrown if type cannot be found.
 */
//------------------------------------------------------------------------------
wxString FileManager::GetAbsPathname(const wxString &typeName)
{
   wxString fileType = GmatStringUtil::ToUpper(typeName);
   wxString absPath;

   #ifdef DEBUG_FILE_MANAGER
   MessageInterface::ShowMessage
      (wxT("FileManager::GetAbsPathname() typeName='%s', fileType='%s'\n"),
       typeName.c_str(), fileType.c_str());
   #endif
   
   // typeName contains _PATH
   if (fileType.find(wxT("_PATH")) != fileType.npos)
   {
      if (mPathMap.find(fileType) != mPathMap.end())
      {
         absPath = ConvertToAbsPath(fileType);
                  
         #ifdef DEBUG_FILE_MANAGER
         MessageInterface::ShowMessage
            (wxT("FileManager::GetAbsPathname() with _PATH returning '%s'\n"), absPath.c_str());
         #endif
         return absPath;
      }
   }
   else
   {
      if (mFileMap.find(fileType) != mFileMap.end())
      {
         wxString path = GetPathname(fileType);
         absPath = path + mFileMap[fileType]->mFile;
      }
      else if (mFileMap.find(fileType + wxT("_ABS")) != mFileMap.end())
      {
         absPath = mFileMap[typeName]->mFile;
      }

      #ifdef DEBUG_FILE_MANAGER
      MessageInterface::ShowMessage
         (wxT("FileManager::GetAbsPathname() without _PATH returning '%s'\n"), absPath.c_str());
      #endif
      return absPath;
   }

   throw UtilityException
      (GmatStringUtil::ToUpper(typeName) + wxT(" not in the gmat_startup_file\n"));

}


//------------------------------------------------------------------------------
// wxString ConvertToAbsPath(const wxString &relPath)
//------------------------------------------------------------------------------
/**
 * Converts relative path to absolute path
 */
//------------------------------------------------------------------------------
wxString FileManager::ConvertToAbsPath(const wxString &relPath)
{
   #ifdef DEBUG_FILE_PATH
   MessageInterface::ShowMessage
      (wxT("FileManager::ConvertToAbsPath() relPath='%s'\n"), relPath.c_str());
   #endif
   
   //wxString absPath = relPath;
   wxString absPath;
   StringTokenizer st(relPath, wxT("/\\"));
   StringArray allNames = st.GetAllTokens();
   StringArray pathNames;
   
   for (UnsignedInt i = 0; i < allNames.size(); i++)
   {
      wxString name = allNames[i];
      
      #ifdef DEBUG_FILE_PATH
      MessageInterface::ShowMessage(wxT("   name = '%s'\n"), name.c_str());
      #endif
      
      absPath = name;
      if (GmatStringUtil::EndsWith(name, wxT("_PATH")))
      {
         #ifdef DEBUG_FILE_PATH
         MessageInterface::ShowMessage(wxT("   _PATH found\n"));
         #endif
         
         if (mPathMap.find(name) != mPathMap.end())
            absPath = mPathMap[name];
         
         if (absPath.find(wxT("_PATH")) != absPath.npos)
            absPath = ConvertToAbsPath(absPath);
         
         #ifdef DEBUG_FILE_PATH
         MessageInterface::ShowMessage(wxT("   absPath = '%s'\n"), absPath.c_str());
         #endif
         
         pathNames.push_back(absPath);
      }
      else
      {
         #ifdef DEBUG_FILE_PATH
         MessageInterface::ShowMessage(wxT("   _PATH not found\n"));
         MessageInterface::ShowMessage(wxT("   absPath = '%s'\n"), absPath.c_str());
         #endif
         
         pathNames.push_back(absPath);
      }
   }
   
   absPath = wxT("");
   for (UnsignedInt i = 0; i < pathNames.size(); i++)
   {
      if (GmatStringUtil::EndsWithPathSeparator(pathNames[i]))
         absPath = absPath + pathNames[i];
      else
         absPath = absPath + pathNames[i] + wxT("/");
   }
      
   #ifdef DEBUG_FILE_PATH
   MessageInterface::ShowMessage
      (wxT("FileManager::ConvertToAbsPath() returning '%s'\n"), absPath.c_str());
   #endif
   
   return absPath;
}


//------------------------------------------------------------------------------
// void SetAbsPathname(const FileType type, const wxString &newpath)
//------------------------------------------------------------------------------
/**
 * Sets absoulute pathname for the type.
 *
 * @param <type> file type of which path to be set.
 * @param <newpath> new pathname.
 *
 * @exception thrown if enum type is out of bounds
 */
//------------------------------------------------------------------------------
void FileManager::SetAbsPathname(const FileType type, const wxString &newpath)
{
   if (type >= BEGIN_OF_PATH && type <= END_OF_PATH)
   {
      SetAbsPathname(FILE_TYPE_STRING[type], newpath);
   }
   else
   {
      wxString ss(wxT(""));
      ss << wxT("FileManager::SetAbsPathname() enum type: ") << type <<
         wxT(" is out of bounds of file path\n");

      throw UtilityException(ss);
   }
}


//------------------------------------------------------------------------------
// void SetAbsPathname(const wxString &type, const wxString &newpath)
//------------------------------------------------------------------------------
/**
 * Sets absolute pathname for the type.
 *
 * @param <type> type name of which path to be set.
 * @param <newpath> new pathname.
 *
 * @exception thrown if enum type is out of bounds
 */
//------------------------------------------------------------------------------
void FileManager::SetAbsPathname(const wxString &type, const wxString &newpath)
{
   if (mPathMap.find(type) != mPathMap.end())
   {
      if (type.find(wxT("_PATH")) != type.npos)
      {
         wxString str2 = newpath;

         // append '/' if not there
         wxString::size_type index = str2.find_last_of(wxT("/\\"));
         if (index != str2.length() - 1)
         {
            str2 = str2 + mPathSeparator;
         }
         else
         {
            index = str2.find_last_not_of(wxT("/\\"));
            str2 = str2.substr(0, index+1) + mPathSeparator;
         }

         mPathMap[type] = str2;

         #ifdef DEBUG_SET_PATH
         MessageInterface::ShowMessage
            (wxT("FileManager::SetAbsPathname() %s set to %s\n"), type.c_str(),
             str2.c_str());
         #endif
      }
      else
      {
         throw UtilityException
            (wxT("FileManager::SetAbsPathname() type doesn't contain _PATH"));
      }
   }
}


//------------------------------------------------------------------------------
// void ClearGmatFunctionPath()
//------------------------------------------------------------------------------
void FileManager::ClearGmatFunctionPath()
{
   mGmatFunctionPaths.clear();
}


//------------------------------------------------------------------------------
// void  AddGmatFunctionPath(const wxString &path, bool addFront, bool addFront)
//------------------------------------------------------------------------------
/*
 * If new path it adds to the GmatFunction path list.
 * If path already exist, it moves to the front or back of the list, depends on
 * addFront flag.
 *
 * @param  path  path name to be added
 * @param  addFront  if set to true, it adds to the front, else adds to the back (true)
 */
//------------------------------------------------------------------------------
void FileManager::AddGmatFunctionPath(const wxString &path, bool addFront)
{
   #ifdef DEBUG_FUNCTION_PATH
   MessageInterface::ShowMessage
      (wxT("FileManager::AddGmatFunctionPath() Adding %s to GmatFunctionPath\n   ")
       wxT("addFront=%d\n"), path.c_str(), addFront);
   #endif

   wxString pathname = path;

   // if path has full pathname (directory and filename), remove filename first
   if (path.find(wxT(".")) != path.npos)
      pathname = GmatFileUtil::ParsePathName(path);

   std::list<wxString>::iterator pos =
      find(mGmatFunctionPaths.begin(), mGmatFunctionPaths.end(), pathname);

   if (pos == mGmatFunctionPaths.end())
   {
      #ifdef DEBUG_FUNCTION_PATH
      MessageInterface::ShowMessage
         (wxT("   the pathname <%s> is new, so adding to %s\n"), pathname.c_str(),
          addFront ? wxT("front") : wxT("back"));
      #endif

      // if new pathname, add to front or back of the list
      if (addFront)
         mGmatFunctionPaths.push_front(pathname);
      else
         mGmatFunctionPaths.push_back(pathname);
   }
   else
   {
      // if existing pathname remove and add front or back of the list
      #ifdef DEBUG_FUNCTION_PATH
      MessageInterface::ShowMessage
         (wxT("   the pathname <%s> already exists, so moving to %s\n"), pathname.c_str(),
          addFront ? wxT("front") : wxT("back"));
      #endif

      wxString oldPath = *pos;
      mGmatFunctionPaths.erase(pos);
      if (addFront)
         mGmatFunctionPaths.push_front(oldPath);
      else
         mGmatFunctionPaths.push_back(oldPath);
   }

   #ifdef DEBUG_FUNCTION_PATH
   pos = mGmatFunctionPaths.begin();
   while (pos != mGmatFunctionPaths.end())
   {
      MessageInterface::ShowMessage
         (wxT("   mGmatFunctionPaths = %s\n"), (*pos).c_str());
      ++pos;
   }
   #endif
}


//------------------------------------------------------------------------------
// wxString GetGmatFunctionPath(const wxString &funcName)
//------------------------------------------------------------------------------
/*
 * Returns the absolute path that has GmatFunction name.
 * It searches in the most recently added path first which is at the top of
 * the list.
 *
 * @param   funcName  Name of the GmatFunction to be located
 * @return  Path that has GmatFunction name
 */
//------------------------------------------------------------------------------
wxString FileManager::GetGmatFunctionPath(const wxString &funcName)
{
   return GetFunctionPath(GMAT_FUNCTION, mGmatFunctionPaths, funcName);
}


//------------------------------------------------------------------------------
// const StringArray& GetAllGmatFunctionPaths()
//------------------------------------------------------------------------------
const StringArray& FileManager::GetAllGmatFunctionPaths()
{
   mGmatFunctionFullPaths.clear();

   std::list<wxString>::iterator listpos = mGmatFunctionPaths.begin();
   while (listpos != mGmatFunctionPaths.end())
   {
      mGmatFunctionFullPaths.push_back(ConvertToAbsPath(*listpos));
      ++listpos;
   }

   return mGmatFunctionFullPaths;
}


//------------------------------------------------------------------------------
// void ClearMatlabFunctionPath()
//------------------------------------------------------------------------------
void FileManager::ClearMatlabFunctionPath()
{
   mMatlabFunctionPaths.clear();
}


//------------------------------------------------------------------------------
// void  AddMatlabFunctionPath(const wxString &pat, bool addFront)
//------------------------------------------------------------------------------
/*
 * If new path it adds to the MatlabFunction path list.
 * If path already exist, it moves to the front or back of the list, depends on
 * addFront flag.
 *
 * @param  path  path name to be added
 * @param  addFront  if set to true, it adds to the front, else adds to the back (true)
 */
//------------------------------------------------------------------------------
void FileManager::AddMatlabFunctionPath(const wxString &path, bool addFront)
{
   #ifdef DEBUG_FUNCTION_PATH
   MessageInterface::ShowMessage
      (wxT("FileManager::AddMatlabFunctionPath() Adding %s to MatlabFunctionPath\n"),
       path.c_str());
   #endif

   std::list<wxString>::iterator pos =
      find(mMatlabFunctionPaths.begin(), mMatlabFunctionPaths.end(), path);

   if (pos == mMatlabFunctionPaths.end())
   {
      // if new path, add to front or back of the list
      if (addFront)
         mMatlabFunctionPaths.push_front(path);
      else
         mMatlabFunctionPaths.push_back(path);
   }
   else
   {
      // if existing path remove and add front or back of the list
      wxString oldPath = *pos;
      mMatlabFunctionPaths.erase(pos);
      if (addFront)
         mMatlabFunctionPaths.push_front(oldPath);
      else
         mMatlabFunctionPaths.push_back(oldPath);
   }

   #ifdef DEBUG_FUNCTION_PATH
   pos = mMatlabFunctionPaths.begin();
   while (pos != mMatlabFunctionPaths.end())
   {
      MessageInterface::ShowMessage
         (wxT("   mMatlabFunctionPaths=%s\n"),(*pos).c_str());
      ++pos;
   }
   #endif
}


//------------------------------------------------------------------------------
// wxString GetMatlabFunctionPath(const wxString &name)
//------------------------------------------------------------------------------
/*
 * Returns the absolute path that has MatlabFunction name.
 * It searches in the most recently added path first which is at the top of
 * the list.
 *
 * @param   funcName  Name of the MatlabFunction to be located
 * @return  Path that has MatlabFunction name
 */
//------------------------------------------------------------------------------
wxString FileManager::GetMatlabFunctionPath(const wxString &name)
{
   return GetFunctionPath(MATLAB_FUNCTION, mMatlabFunctionPaths, name);
}


//------------------------------------------------------------------------------
// const StringArray& GetAllMatlabFunctionPaths()
//------------------------------------------------------------------------------
const StringArray& FileManager::GetAllMatlabFunctionPaths()
{
   mMatlabFunctionFullPaths.clear();

   std::list<wxString>::iterator listpos = mMatlabFunctionPaths.begin();
   while (listpos != mMatlabFunctionPaths.end())
   {
      mMatlabFunctionFullPaths.push_back(ConvertToAbsPath(*listpos));
      ++listpos;
   }

   return mMatlabFunctionFullPaths;
}


//------------------------------------------------------------------------------
// const StringArray& GetPluginList()
//------------------------------------------------------------------------------
/**
 * Accesses the list of plug-in libraries parsed from the startup file.
 *
 * @return The list of plug-in libraries
 */
//------------------------------------------------------------------------------
const StringArray& FileManager::GetPluginList()
{
   return mPluginList;
}

//---------------------------------
// private methods
//---------------------------------

//------------------------------------------------------------------------------
// wxString GetFunctionPath(FunctionType type, const std::list<wxString> &pathList
//                             const wxString &funcName)
//------------------------------------------------------------------------------
/*
 * Searches proper function path list from the top and return first path found.
 *
 * @param  type  type of function (MATLAB_FUNCTION, GMAT_FUNCTION)
 * @param  pathList  function path list to use in search
 * @param  funcName  name of the function to search
 */
//------------------------------------------------------------------------------
wxString FileManager::GetFunctionPath(FunctionType type,
                                         std::list<wxString> &pathList,
                                         const wxString &funcName)
{
   #ifdef DEBUG_FUNCTION_PATH
   MessageInterface::ShowMessage
      (wxT("FileManager::GetFunctionPath(%s) with type %d entered\n"),
       funcName.c_str(), type);
   #endif

   wxString funcName1 = funcName;
   if (type == GMAT_FUNCTION)
   {
      if (funcName.find(wxT(".gmf")) == funcName.npos)
         funcName1 = funcName1 + wxT(".gmf");
   }
   else
   {
      if (funcName.find(wxT(".m")) == funcName.npos)
         funcName1 = funcName1 + wxT(".m");
   }

   // Search through pathList
   // The most recent path added to the last, so search backwards
   wxString pathName, fullPath;
   bool fileFound = false;

   // Search from the top of the list, which is the most recently added path
   // The search order goes from top to bottom. (loj: 2008.10.02)
   std::list<wxString>::iterator pos = pathList.begin();
   while (pos != pathList.end())
   {
      pathName = *pos;
      fullPath = ConvertToAbsPath(pathName) + funcName1;

      #ifdef DEBUG_FUNCTION_PATH
      MessageInterface::ShowMessage(wxT("   fullPath='%s'\n"), fullPath.c_str());
      #endif

      if (GmatFileUtil::DoesFileExist(fullPath))
      {
         fileFound = true;
         break;
      }

      pos++;
   }

   if (fileFound)
      fullPath = GmatFileUtil::ParsePathName(fullPath);
   else
      fullPath = wxT("");

   #ifdef DEBUG_FUNCTION_PATH
   MessageInterface::ShowMessage
      (wxT("FileManager::GetFunctionPath(%s) returning '%s'\n"), funcName.c_str(),
       fullPath.c_str());
   #endif

   return fullPath;

}


//------------------------------------------------------------------------------
// void AddFileType(const wxString &type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Adds file type, path, name to the list. If typeName ends with _PATH, it is
 * added to path map. If typeName ends with _FILE, it is added to file map, an
 * exception is throw otherwise.
 *
 * @param <type> file type
 * @param <name> file path or name
 *
 * @excepton thrown if typeName does not end with _PATH or _FILE
 */
//------------------------------------------------------------------------------
void FileManager::AddFileType(const wxString &type, const wxString &name)
{
   #ifdef DEBUG_ADD_FILETYPE
   MessageInterface::ShowMessage
      (wxT("FileManager::AddFileType() entered, type=%s, name=%s\n"), type.c_str(), name.c_str());
   #endif

   if (type.find(wxT("_PATH")) != type.npos)
   {
      wxString str2 = name;
      
      // append '/' if '\\' or '/' not there
      if (!GmatStringUtil::EndsWithPathSeparator(str2))
         str2 = str2 + mPathSeparator;
      
      mPathMap[type] = str2;

      #ifdef DEBUG_ADD_FILETYPE
      MessageInterface::ShowMessage
         (wxT("   Adding %s = %s to mPathMap\n"), type.c_str(), str2.c_str());
      #endif
      
      // Handle Gmat and Matlab Function path
      if (type == wxT("GMAT_FUNCTION_PATH"))
         AddGmatFunctionPath(str2, false);
      else if (type == wxT("MATLAB_FUNCTION_PATH"))
         AddMatlabFunctionPath(str2, false);

   }
   else if (type.find(wxT("_FILE_ABS")) != type.npos)
   {
      mFileMap[type] = new FileInfo(wxT(""), name);
   }
   else if (type.find(wxT("_FILE")) != type.npos)
   {
      wxString pathName;
      wxString fileName;
      
      // file name
      wxString::size_type pos = name.find_last_of(wxT("/"));
      if (pos == name.npos)
         pos = name.find_last_of(wxT("\\"));
      
      if (pos != name.npos)
      {
         wxString pathName = name.substr(0, pos);
         wxString fileName = name.substr(pos+1, name.npos);
         mFileMap[type] = new FileInfo(pathName, fileName);
         
         #ifdef DEBUG_ADD_FILETYPE
         MessageInterface::ShowMessage
            (wxT("   Adding %s and %s to mFileMap\n"), pathName.c_str(), fileName.c_str());
         #endif
      }
      else
      {
         //loj: Should we add current path?
         wxString pathName = wxT("CURRENT_PATH");
         mPathMap[pathName] = wxT("./");
         wxString fileName = name;
         mFileMap[type] = new FileInfo(pathName, fileName);
         
         #ifdef DEBUG_ADD_FILETYPE
         MessageInterface::ShowMessage
            (wxT("   Adding %s and %s to mFileMap\n"), pathName.c_str(), fileName.c_str());
         MessageInterface::ShowMessage
            (wxT("   'PATH/' not found in line:\n   %s = %s\n   So adding CURRENT_PATH = ./\n"),
             type.c_str(), name.c_str());
         #endif
         
         //loj: Should we just throw an exception?
         //mInStream.close();
         //throw UtilityException
         //   (wxT("FileManager::AddFileType() expecting 'PATH/' in line:\n") +
         //    type + wxT(" = ") + name);
      }
   }
   else if (type == wxT("PLUGIN"))
   {
      #ifdef DEBUG_PLUGIN_DETECTION
         MessageInterface::ShowMessage(wxT("Adding plug-in %s to plugin list\n"),
               name.c_str());
      #endif
      mPluginList.push_back(name);
   }
   else
   {
      throw UtilityException
         (wxT("FileManager::AddFileType() file type should have '_PATH' or '_FILE'")
          wxT(" in:\n") + type);
   }
   
   #ifdef DEBUG_ADD_FILETYPE
   MessageInterface::ShowMessage(wxT("FileManager::AddFileType() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void AddAvailablePotentialFiles()
//------------------------------------------------------------------------------
void FileManager::AddAvailablePotentialFiles()
{
   // add available potential files

   // earth gravity files
   if (mFileMap.find(wxT("JGM2_FILE")) == mFileMap.end())
      AddFileType(wxT("JGM2_FILE"), wxT("EARTH_POT_PATH/JGM2.cof"));

   if (mFileMap.find(wxT("JGM3_FILE")) == mFileMap.end())
      AddFileType(wxT("JGM3_FILE"), wxT("EARTH_POT_PATH/JGM3.cof"));

   if (mFileMap.find(wxT("EGM96_FILE")) == mFileMap.end())
      AddFileType(wxT("EGM96_FILE"), wxT("EARTH_POT_PATH/EGM96low.cof"));

   // luna gravity files
   if (mFileMap.find(wxT("LP165P_FILE")) == mFileMap.end())
      AddFileType(wxT("LP165P_FILE"), wxT("LUNA_POT_PATH/LP165P.cof"));

   // venus gravity files
   if (mFileMap.find(wxT("MGNP180U_FILE")) == mFileMap.end())
      AddFileType(wxT("MGNP180U_FILE"), wxT("VENUS_POT_PATH/MGNP180U.cof"));

   // mars gravity files
   if (mFileMap.find(wxT("MARS50C_FILE")) == mFileMap.end())
      AddFileType(wxT("MARS50C_FILE"), wxT("MARS_POT_PATH/Mars50c.cof"));

}


//------------------------------------------------------------------------------
// void WriteHeader(std::ofstream &outStream)
//------------------------------------------------------------------------------
void FileManager::WriteHeader(std::ofstream &outStream)
{
   outStream << wxT("#-------------------------------------------------------------------------------\n");
   outStream << wxT("# General Mission Analysis Tool (GMAT) startup file\n");
   outStream << wxT("#-------------------------------------------------------------------------------\n");
   outStream << wxT("# Comment line starts with #\n");
   outStream << wxT("# Comment line starting with ## will be saved when saving startup file.\n");
   outStream << wxT("#\n");
   outStream << wxT("# Path/File naming convention:\n");
   outStream << wxT("#   - Path name should end with _PATH\n");
   outStream << wxT("#   - File name should end with _FILE\n");
   outStream << wxT("#   - Path/File names are case sensative\n");
   outStream << wxT("#\n");
   outStream << wxT("# You can add potential and texture files by following the naming convention.\n");
   outStream << wxT("#   - Potential file should begin with planet name and end with _POT_FILE\n");
   outStream << wxT("#   - Texture file should begin with planet name and end with _TEXTURE_FILE\n");
   outStream << wxT("#\n");
   outStream << wxT("# If same _FILE is specified multiple times, it will use the last one.\n");
   outStream << wxT("#\n");
   outStream << wxT("# You can have more than one line containing GMAT_FUNCTION_PATH. GMAT will store \n");
   outStream << wxT("# the multiple paths you specify and scan for GMAT Functions using the paths \n");
   outStream << wxT("# in top to bottom order and use the first function found from the search paths.\n");
   outStream << wxT("#\n");
   outStream << wxT("# In order for an object plugin to work inside GMAT, the plugin dynamic link libraries; \n");
   outStream << wxT("# Windows(.dll), Linux(.so) and Mac(.dylib), must be placed in the folder containing\n");
   outStream << wxT("# the GMAT executable or application. Once placed in the correct folder \n");
   outStream << wxT("# the PLUGIN line below must be set equal to the plugin name without the dynamic link \n");
   outStream << wxT("# library extension with the comment (#) removed from the front of the line.\n");
   outStream << wxT("#\n");
   outStream << wxT("# Some available PLUGINs are:\n");
   outStream << wxT("# PLUGIN = libMatlabInterface\n");
   outStream << wxT("# PLUGIN = libFminconOptimizer\n");
   outStream << wxT("# PLUGIN = libVF13Optimizer\n");
   outStream << wxT("# PLUGIN = libDataFile\n");
   outStream << wxT("# PLUGIN = libCcsdsEphemerisFile\n");
   outStream << wxT("# PLUGIN = libGmatEstimation\n");
   outStream << wxT("#\n");
   outStream << wxT("#===============================================================================\n");
}

//------------------------------------------------------------------------------
// void WriteFiles(std::ofstream &outStream, const wxString &type)
//------------------------------------------------------------------------------
void FileManager::WriteFiles(std::ofstream &outStream, const wxString &type)
{
   #ifdef DEBUG_WRITE_STARTUP_FILE
   MessageInterface::ShowMessage(wxT("   .....Writing %s file\n"), type.c_str());
   #endif
   
   wxString realPath;
   
   // Write remainder of path
   if (type == wxT("-OTHER-PATH-"))
   {
      for (std::map<wxString, wxString>::iterator pos = mPathMap.begin();
           pos != mPathMap.end(); ++pos)
      {
         // if name not found in already written out list, then write
         if (find(mPathWrittenOuts.begin(), mPathWrittenOuts.end(), pos->first) ==
             mPathWrittenOuts.end())
         {
            if (pos->second != wxT(""))
            {
               realPath = pos->second;
               mPathWrittenOuts.push_back(pos->first);
               outStream << std::setw(22) << pos->first << wxT(" = ")
                         << realPath << wxT("\n");
            }
         }
      }
      return;
   }
   
   // Write remainder of files
   if (type == wxT("-OTHER-"))
   {
      for (std::map<wxString, FileInfo*>::iterator pos = mFileMap.begin();
           pos != mFileMap.end(); ++pos)
      {
         // if name not found in already written out list, then write
         if (find(mFileWrittenOuts.begin(), mFileWrittenOuts.end(), pos->first) ==
             mFileWrittenOuts.end())
         {
            if (pos->second)
            {
               realPath = pos->second->mPath;
               if (realPath == wxT("CURRENT_PATH"))
                  realPath = wxT("");
               else
                  realPath = realPath + mPathSeparator;
               
               mFileWrittenOuts.push_back(pos->first);
               outStream << std::setw(22) << pos->first << wxT(" = ")
                         << realPath << pos->second->mFile << wxT("\n");
            }
         }
      }
      return;
   }
   
   for (std::map<wxString, FileInfo*>::iterator pos = mFileMap.begin();
        pos != mFileMap.end(); ++pos)
   {
      if (pos->first.find(type) != wxString::npos)
      {
         if (pos->second)
         {
            realPath = pos->second->mPath;
            if (realPath == wxT("CURRENT_PATH"))
               realPath = wxT("");
            else
               realPath = realPath + mPathSeparator;
            
            mFileWrittenOuts.push_back(pos->first);
            outStream << std::setw(22) << pos->first << wxT(" = ")
                      << realPath << pos->second->mFile << wxT("\n");
         }
      }
   }
}


//------------------------------------------------------------------------------
// void RefreshFiles()
//------------------------------------------------------------------------------
void FileManager::RefreshFiles()
{
   mRunMode = wxT("");
   mMatlabMode = wxT("");
   mDebugMatlab = wxT("");
   mPathMap.clear();
   mGmatFunctionPaths.clear();
   mMatlabFunctionPaths.clear();
   mGmatFunctionFullPaths.clear();
   mSavedComments.clear();
   mPluginList.clear();

   for (std::map<wxString, FileInfo*>::iterator iter = mFileMap.begin();
        iter != mFileMap.begin(); ++iter)
      delete iter->second;

   mFileMap.clear();
   
   //-------------------------------------------------------
   // add root and data path
   //-------------------------------------------------------
   AddFileType(wxT("ROOT_PATH"), wxT("../"));
   AddFileType(wxT("DATA_PATH"), wxT("ROOT_PATH/data"));
   
   //-------------------------------------------------------
   // add default output paths and files
   //-------------------------------------------------------
   wxString defOutPath = wxT("../output");
   if (!DoesDirectoryExist(defOutPath))
      defOutPath = wxT("./");
   
   AddFileType(wxT("OUTPUT_PATH"), defOutPath);
   AddFileType(wxT("LOG_FILE"), wxT("OUTPUT_PATH/GmatLog.txt"));
   AddFileType(wxT("REPORT_FILE"), wxT("OUTPUT_PATH/GmatReport.txt"));
   AddFileType(wxT("MEASUREMENT_PATH"), wxT("OUTPUT_PATH"));
   AddFileType(wxT("EPHEM_PATH"), wxT("OUTPUT_PATH"));
   AddFileType(wxT("SCREENSHOT_FILE"), wxT("OUTPUT_PATH"));
   AddFileType(wxT("EPHEM_PATH"), wxT("OUTPUT_PATH"));
   
   
   // Should we add default input paths and files?
   // Yes, for now in case of startup file doesn't specify all the required
   // input path and files (LOJ: 2011.03.21)
#ifdef __FM_ADD_DEFAULT_INPUT__

   //-------------------------------------------------------
   // create default input paths and files
   //-------------------------------------------------------
   
   // de files
   AddFileType(wxT("DE_PATH"), wxT("DATA_PATH/planetary_ephem/de/"));
   AddFileType(wxT("DE405_FILE"), wxT("DE_PATH/leDE1941.405"));

   // spk files
   AddFileType(wxT("SPK_PATH"), wxT("DATA_PATH/planetary_ephem/spk/"));
   AddFileType(wxT("PLANETARY_SPK_FILE"), wxT("SPK_PATH/de421.bsp"));

   // earth gravity files
   AddFileType(wxT("EARTH_POT_PATH"), wxT("DATA_PATH/gravity/earth/"));
   AddFileType(wxT("JGM2_FILE"), wxT("EARTH_POT_PATH/JGM2.cof"));
   AddFileType(wxT("JGM3_FILE"), wxT("EARTH_POT_PATH/JGM3.cof"));
   AddFileType(wxT("EGM96_FILE"), wxT("EARTH_POT_PATH/EGM96.cof"));

   // luna gravity files
   AddFileType(wxT("LUNA_POT_PATH"), wxT("DATA_PATH/gravity/luna/"));
   AddFileType(wxT("LP165P_FILE"), wxT("LUNA_POT_PATH/lp165p.cof"));

   // venus gravity files
   AddFileType(wxT("VENUS_POT_PATH"), wxT("DATA_PATH/gravity/venus/"));
   AddFileType(wxT("MGNP180U_FILE"), wxT("VENUS_POT_PATH/MGNP180U.cof"));

   // mars gravity files
   AddFileType(wxT("MARS_POT_PATH"), wxT("DATA_PATH/gravity/mars/"));
   AddFileType(wxT("MARS50C_FILE"), wxT("MARS_POT_PATH/Mars50c.cof"));

   // planetary coeff. fiels
   AddFileType(wxT("PLANETARY_COEFF_PATH"), wxT("DATA_PATH/planetary_coeff/"));
   AddFileType(wxT("EOP_FILE"), wxT("PLANETARY_COEFF_PATH/eopc04.62-now"));
   AddFileType(wxT("PLANETARY_COEFF_FILE"), wxT("PLANETARY_COEFF_PATH/NUT85.DAT"));
   AddFileType(wxT("NUTATION_COEFF_FILE"), wxT("PLANETARY_COEFF_PATH/NUTATION.DAT"));

   // time path and files
   AddFileType(wxT("TIME_PATH"), wxT("DATA_PATH/time/"));
   AddFileType(wxT("LEAP_SECS_FILE"), wxT("TIME_PATH/tai-utc.dat"));
   AddFileType(wxT("LSK_FILE"), wxT("TIME_PATH/naif0009.tls"));
   
   // gui config file path
   AddFileType(wxT("GUI_CONFIG_PATH"), wxT("DATA_PATH/gui_config/"));
   
   // personalization file
   AddFileType(wxT("PERSONALIZATION_FILE"), wxT("DATA_PATH/gui_config/MyGmat.ini"));
   
   // icon path and main icon file
   AddFileType(wxT("ICON_PATH"), wxT("DATA_PATH/graphics/icons/"));
   
   #if defined __WXMSW__
   AddFileType(wxT("MAIN_ICON_FILE"), wxT("ICON_PATH/GMATWin32.ico"));
   #elif defined __WXGTK__
   AddFileType(wxT("MAIN_ICON_FILE"), wxT("ICON_PATH/GMATLinux48.xpm"));
   #elif defined __WXMAC__
   AddFileType(wxT("MAIN_ICON_FILE"), wxT("ICON_PATH/GMATIcon.icns"));
   #endif

   // splash file path
   AddFileType(wxT("SPLASH_PATH"), wxT("DATA_PATH/graphics/splash/"));
   AddFileType(wxT("SPLASH_FILE"), wxT("SPLASH_PATH/GMATSplashScreen.tif"));
   
   // texture file path
   AddFileType(wxT("TEXTURE_PATH"), wxT("DATA_PATH/graphics/texture/"));
   AddFileType(wxT("SUN_TEXTURE_FILE"), wxT("TEXTURE_PATH/Sun.jpg"));
   AddFileType(wxT("MERCURY_TEXTURE_FILE"), wxT("TEXTURE_PATH/Mercury_JPLCaltech.jpg"));
   AddFileType(wxT("EARTH_TEXTURE_FILE"), wxT("TEXTURE_PATH/ModifiedBlueMarble.jpg"));
   AddFileType(wxT("MARS_TEXTURE_FILE"), wxT("TEXTURE_PATH/Mars_JPLCaltechUSGS.jpg"));
   AddFileType(wxT("JUPITER_TEXTURE_FILE"), wxT("TEXTURE_PATH/Jupiter_HermesCelestiaMotherlode.jpg"));
   AddFileType(wxT("SATRUN_TEXTURE_FILE"), wxT("TEXTURE_PATH/Saturn_gradiusCelestiaMotherlode.jpg"));
   AddFileType(wxT("URANUS_TEXTURE_FILE"), wxT("TEXTURE_PATH/Uranus_JPLCaltech.jpg"));
   AddFileType(wxT("NEPTUNE_TEXTURE_FILE"), wxT("TEXTURE_PATH/Neptune_BjornJonsson.jpg"));
   AddFileType(wxT("PLUTO_TEXTURE_FILE"), wxT("TEXTURE_PATH/Pluto_JPLCaltech.jpg"));
   AddFileType(wxT("LUNA_TEXTURE_FILE"), wxT("TEXTURE_PATH/Moon_HermesCelestiaMotherlode.jpg"));
   
   // star path and files
   AddFileType(wxT("STAR_PATH"), wxT("DATA_PATH/graphics/stars/"));
   AddFileType(wxT("STAR_FILE"), wxT("STAR_PATH/inp_StarCatalog.txt"));
   AddFileType(wxT("CONSTELLATION_FILE"), wxT("STAR_PATH/inp_Constellation.txt"));
   
   // models
   AddFileType(wxT("MODEL_PATH"), wxT("DATA_PATH/vehicle/models/"));
   AddFileType(wxT("SPACECRAFT_MODEL_FILE"), wxT("MODEL_PATH/aura.3ds"));
   
   // help file
   AddFileType(wxT("HELP_FILE"), wxT(""));
   
#endif

}


//------------------------------------------------------------------------------
// void ShowMaps(const wxString &msg)
//------------------------------------------------------------------------------
void FileManager::ShowMaps(const wxString &msg)
{
   MessageInterface::ShowMessage(wxT("%s\n"), msg.c_str());
   MessageInterface::ShowMessage(wxT("Here is path map, there are %d items\n"), mPathMap.size());
   for (std::map<wxString, wxString>::iterator pos = mPathMap.begin();
        pos != mPathMap.end(); ++pos)
   {
      MessageInterface::ShowMessage(wxT("%20s: %s\n"), (pos->first).c_str(), (pos->second).c_str());
   }
   
   MessageInterface::ShowMessage(wxT("Here is file map, there are %d items\n"), mFileMap.size());
   for (std::map<wxString, FileInfo*>::iterator pos = mFileMap.begin();
        pos != mFileMap.end(); ++pos)
   {
      MessageInterface::ShowMessage
         (wxT("%20s: %20s  %s\n"), (pos->first).c_str(), (pos->second)->mPath.c_str(), (pos->second)->mFile.c_str());
   }
}


//------------------------------------------------------------------------------
// FileManager()
//------------------------------------------------------------------------------
/*
 * Constructor
 */
//------------------------------------------------------------------------------
FileManager::FileManager()
{
   MessageInterface::SetLogEnable(false); // so that debug can be written from here

   #ifdef DEBUG_FILE_MANAGER
   MessageInterface::ShowMessage(wxT("FileManager::FileManager() entered\n"));
   #endif

   mPathSeparator = GetPathSeparator();
   mStartupFileDir = GetCurrentPath() + mPathSeparator;
   mStartupFileName = wxT("gmat_startup_file.txt");

   #ifdef DEBUG_STARTUP_FILE
   MessageInterface::ShowMessage
      (wxT("FileManager::FileManager() entered, mPathSeparator='%s', ")
       wxT("mStartupFileDir='%s', mStartupFileName='%s'\n"), mPathSeparator.c_str(),
       mStartupFileDir.c_str(), mStartupFileName.c_str());
   #endif

   RefreshFiles();

}


