//$Id: GmatInterface.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              GmatInterface
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
// Created: 2004/8/30
//
/**
 * Declares class to provide sending scripts from MATLAB to GMAT or
 * requesting GMAT data from MATLAB.
 */
//------------------------------------------------------------------------------
#ifndef GmatInterface_hpp
#define GmatInterface_hpp

#include <sstream>       // for stringstream, istringstream
#include "gmatdefs.hpp"
#include "GmatCommand.hpp"

class GMAT_API GmatInterface
{
public:
   static GmatInterface* Instance();
   ~GmatInterface();
   
   void OpenScript();
   void ClearScript();
   void PutScript(wxString &str);
   void BuildObject();
   void UpdateObject();
   void RunScript();
   
   // methods to manage execution of GMAT callback
   bool  ExecuteCallback();
   bool  RegisterCallbackServer(GmatBase *callbackObject);
   wxChar * GetCallbackStatus();
   void  PutCallbackData(wxString &data);
   wxChar * GetCallbackResults();
   
   wxChar * GetRunState();
   wxChar * GetGmatObject(const wxString &name);
   wxChar * GetParameter(const wxString &name);
   
   void CheckUserInterrupt();
   
private:
   static const int MAX_PARAM_VAL_STRING = 512;
   static const int MAX_OBJECT_VAL_STRING = 4096;
   static const int MAX_CALLBACK_DATA_VAL_STRING = 1024;
   
   void RedirectBuffer(std::ios *stream, std::streambuf* newBuff) 
      { stream->rdbuf(newBuff); }
   
   GmatInterface();
   
   static wxString dataString;
   static wxString mStringStream;
   static GmatInterface *instance;
   static bool mPassedInterpreter;
   
   /// class containing callback function
   GmatBase *callbackObj;  
};
#endif
