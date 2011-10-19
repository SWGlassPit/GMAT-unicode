//$Id: FileManager.hpp 9518 2011-04-30 22:32:04Z djcinsb $
//------------------------------------------------------------------------------
//                                  FileManager
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
// Created: 2004/04/02
/**
 * Declares FileManager class. This is singleton class which manages list of
 * file paths and names.
 *
 * The textures files and non-Earth gravity potential files not appear in the
 * predefined enum FileType list can be retrieved by using file naming
 * convention. The texture files should have PLANETNAME_TEXTURE_FILE.
 * e.g. wxT("EARTH_TEXTURE_FILE"), wxT("LUNA_TEXTURE_FILE"), etc. The potential files
 * should have PLANETNAME_POT_FILE.
 */
//------------------------------------------------------------------------------
#ifndef FileManager_hpp
#define FileManager_hpp

#include "gmatdefs.hpp"
#include <map>
#include <list>
#include <fstream>

class GMAT_API FileManager
{
public:
   
   // The following is predefined file paths/types.
   enum FileType
   {
      // file path
      BEGIN_OF_PATH = 0,
      OUTPUT_PATH,
      DE_PATH,
      SPK_PATH,
      EARTH_POT_PATH,
      LUNA_POT_PATH,
      VENUS_POT_PATH,
      MARS_POT_PATH,
      PLANETARY_COEFF_PATH,
      TIME_PATH,
      TEXTURE_PATH, //Notes: TEXTURE_PATH is used in SetPathname()
      MEASUREMENT_PATH,
      EPHEM_PATH,
      GUI_CONFIG_PATH,
      SPLASH_PATH,
      ICON_PATH,
      STAR_PATH,
      MODEL_PATH,
      END_OF_PATH,
      
      // general file name
      LOG_FILE,
      REPORT_FILE,
      SPLASH_FILE,
      TIME_COEFF_FILE,
      
      // specific file name
      //    Notes: Don't add general planet potential files here. They are handled
      //    when gmat_startup_file are read by following naming convention.
      DE405_FILE,
      PLANETARY_SPK_FILE,
      JGM2_FILE,
      JGM3_FILE,
      EGM96_FILE,
      LP165P_FILE,
      MGNP180U_FILE,
      MARS50C_FILE,
      EOP_FILE,
      PLANETARY_COEFF_FILE,
      NUTATION_COEFF_FILE,
      LEAP_SECS_FILE,
      LSK_FILE,
      PERSONALIZATION_FILE,
      MAIN_ICON_FILE,
      STAR_FILE,
      CONSTELLATION_FILE,
      SPACECRAFT_MODEL_FILE,
      HELP_FILE,
      FileTypeCount,
   };
   
   static FileManager* Instance();
   ~FileManager();
   
   wxString GetPathSeparator();
   wxString GetCurrentPath();
   bool DoesDirectoryExist(const wxString &dirPath);
   bool DoesFileExist(const wxString &filename);
   bool RenameFile(const wxString &oldName, const wxString &newName,
                   Integer &retCode, bool overwriteIfExists = false);
   
   wxString GetStartupFileDir();
   wxString GetStartupFileName();
   wxString GetFullStartupFilePath();
   void ReadStartupFile(const wxString &fileName = wxT(""));
   void WriteStartupFile(const wxString &fileName = wxT(""));
   
   wxString GetRootPath();
   
   // Methods returning path
   wxString GetPathname(const FileType type);
   wxString GetPathname(const wxString &typeName);
   
   // Methods returning filename
   wxString GetFilename(const FileType type);
   wxString GetFilename(const wxString &typeName);
   
   // Methods returning full path and filename
   //loj: Why the name wxT("GetFullPathName()") doesn't work? Reserved word?
   // I'm getting unresolved ref on GetFullPathNameA()
   wxString GetFullPathname(const FileType type);
   wxString GetFullPathname(const wxString &typeName);
   wxString GetAbsPathname(const FileType type);
   wxString GetAbsPathname(const wxString &typeName);
   
   wxString ConvertToAbsPath(const wxString &relPath);
   
   void SetAbsPathname(const FileType type, const wxString &newpath);
   void SetAbsPathname(const wxString &type, const wxString &newpath);
   
   void ClearGmatFunctionPath();
   void AddGmatFunctionPath(const wxString &path, bool addFront = true);
   wxString GetGmatFunctionPath(const wxString &name);
   const StringArray& GetAllGmatFunctionPaths();
   
   void ClearMatlabFunctionPath();
   void AddMatlabFunctionPath(const wxString &path, bool addFront = true);
   wxString GetMatlabFunctionPath(const wxString &name);
   const StringArray& GetAllMatlabFunctionPaths();
   
   // Plug-in code
   const StringArray& GetPluginList();
   
private:
   
   enum FunctionType
   {
      GMAT_FUNCTION = 101,
      MATLAB_FUNCTION,
   };
   
   struct FileInfo
   {
      wxString mPath;
      wxString mFile;
      
      FileInfo(const wxString &path, const wxString &file)
         { mPath = path; mFile = file; }
   };
   
   wxString mPathSeparator;
   wxString mStartupFileDir;
   wxString mStartupFileName;
   wxString mRunMode;
   wxString mMatlabMode;
   wxString mDebugMatlab;
   std::ifstream mInStream;
   std::map<wxString, wxString> mPathMap;
   std::map<wxString, FileInfo*> mFileMap;
   std::list<wxString> mGmatFunctionPaths;
   std::list<wxString> mMatlabFunctionPaths;
   StringArray mGmatFunctionFullPaths;
   StringArray mMatlabFunctionFullPaths;
   StringArray mSavedComments;
   StringArray mPathWrittenOuts;
   StringArray mFileWrittenOuts;
   
   StringArray mPluginList;
   
   wxString GetFunctionPath(FunctionType type, std::list<wxString> &pathList,
                               const wxString &funcName);
   void AddFileType(const wxString &type, const wxString &name);
   void AddAvailablePotentialFiles();
   void WriteHeader(std::ofstream &outStream);
   void WriteFiles(std::ofstream &outStream, const wxString &type);
   void RefreshFiles();
   
   // For debugging
   void ShowMaps(const wxString &msg);
   
   static FileManager *theInstance;
   static const wxString FILE_TYPE_STRING[FileTypeCount];
   
   FileManager();
   
};
#endif // FileManager_hpp
