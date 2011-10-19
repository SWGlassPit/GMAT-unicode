//$Id: FileUtil.hpp 9513 2011-04-30 21:23:06Z djcinsb $
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
 * This file provides methods to query file information and methods to compare
 * two output files. The compare summary is written to the log file.
 */
//------------------------------------------------------------------------------
#ifndef FileUtil_hpp
#define FileUtil_hpp

#include "gmatdefs.hpp"
#include <fstream>

namespace GmatFileUtil
{
   const Integer BUFFER_SIZE = 4096;
   static Real CompareAbsTol = 1.0e-4;
   
   wxString GMAT_API GetPathSeparator();
   wxString GMAT_API GetCurrentPath();
   wxString GMAT_API ParseFirstPathName(const wxString &fullPath, bool appendSep = true);
   wxString GMAT_API ParsePathName(const wxString &fullPath, bool appendSep = true);
   wxString GMAT_API ParseFileName(const wxString &fullPath, bool removeExt = false);
   wxString GMAT_API ParseFileExtension(const wxString &fullPath, bool prependDot = false);
   wxString GMAT_API GetInvalidFileNameMessage(Integer option = 1);
   
   bool GMAT_API IsValidFileName(const wxString &fname, bool blankIsOk = true);
   bool GMAT_API IsSameFileName(const wxString &fname1, const wxString &fname2);
   bool GMAT_API DoesDirectoryExist(const wxString &dirPath, bool blankIsOk = true);
   bool GMAT_API DoesFileExist(const wxString &filename);
   bool GMAT_API GetLine(std::istream *inStream, wxString &line);
   bool GMAT_API IsAppInstalled(const wxString &appName, wxString &appLoc);
   
   WrapperTypeArray GMAT_API 
      GetFunctionOutputTypes(std::istream *is, const StringArray &inputs,
                             const StringArray &outputs, wxString &errMsg,
                             IntegerArray &outputRows, IntegerArray &outputCols);
   
   StringArray GMAT_API GetFileListFromDirectory(const wxString &dirName,
                                        bool addPath = false);
   StringArray GMAT_API GetTextLines(const wxString &fileName);
   
   StringArray GMAT_API &Compare(const wxString &filename1,
                        const wxString &filename2,
                        const StringArray &colTitles,
                        Real tol = CompareAbsTol);
   
   StringArray GMAT_API &Compare(Integer numDirsToCompare,
                        const wxString &basefilename,
                        const wxString &filename1,
                        const wxString &filename2,
                        const wxString &filename3,
                        const StringArray &colTitles,
                        Real tol = CompareAbsTol);
   
   StringArray GMAT_API &CompareLines(Integer numDirsToCompare,
                             const wxString &basefilename,
                             const wxString &filename1,
                             const wxString &filename2,
                             const wxString &filename3,
                             int &file1DiffCount, int &file2DiffCount,
                             int &file3DiffCount);
   
   bool GMAT_API SkipHeaderLines(std::ifstream &in, StringArray &tokens);
   
   static StringArray textBuffer;
}

#endif // FileUtil_hpp
