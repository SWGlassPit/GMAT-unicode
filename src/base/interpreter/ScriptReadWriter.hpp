//$Id: ScriptReadWriter.hpp 9513 2011-04-30 21:23:06Z djcinsb $
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

#ifndef SCRIPTREADWRITER_HPP_
#define SCRIPTREADWRITER_HPP_

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include <iostream>

class GMAT_API ScriptReadWriter 
{
public:
   static ScriptReadWriter* Instance();
   ~ScriptReadWriter();
   
   void SetInStream(wxInputStream *is);
   void SetOutStream(wxOutputStream *os) { outStream = os; }
   
   Integer GetLineWidth();
   void SetLineWidth(Integer width);
   
   Integer GetLineNumber();
   wxString GetCurrentLine() { return currentLine; }
   
   void ReadFirstBlock(wxString &header, wxString &firstBlock,
                       bool skipHeader = false);
   wxString ReadLogicalBlock();
   bool WriteText(const wxString &textToWrite);
   
protected:

private:

   // These data are not created here
//   std::istream *inStream;
//   std::ostream *outStream;
   wxInputStream *inStream;
   wxOutputStream *outStream;
   
   wxString currentLine;
   
   Integer lineWidth;
   Integer currentLineNumber;
   bool writeGmatKeyword;
   bool reachedEndOfFile;
   bool readFirstBlock;
   
   bool Initialize();
   wxString CrossPlatformGetLine();
   bool IsComment(const wxString &text);
   bool IsBlank(const wxString &text);
   bool HasEllipse(const wxString &text);
   wxString HandleEllipsis(const wxString &text);
   wxString HandleComments(const wxString &text);
   
   static ScriptReadWriter *instance;   
   static const wxString sectionDelimiter;
   static const wxString ellipsis;
};

#endif /*SCRIPTREADWRITER_HPP_*/
