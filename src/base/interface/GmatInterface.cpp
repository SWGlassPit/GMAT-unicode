//$Id: GmatInterface.cpp 9513 2011-04-30 21:23:06Z djcinsb $
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
 * Implements class to provide sending scripts from MATLAB to GMAT.
 */
//------------------------------------------------------------------------------

#include "GmatInterface.hpp"
#include "Moderator.hpp"         // for Instance()
#include "MessageInterface.hpp"
#include "InterfaceException.hpp"
#include <wx/sstream.h>
#include <wx/txtstrm.h>

GmatInterface* GmatInterface::instance = NULL;
bool GmatInterface::mPassedInterpreter = false;
wxString GmatInterface::dataString = wxT("");
wxString GmatInterface::mStringStream = wxT("");

//#define DEBUG_GMAT_INTERFACE
//#define DEBUG_TEST_CALLBACK

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// static Instance()
//------------------------------------------------------------------------------
GmatInterface* GmatInterface::Instance()
{
   if (instance == NULL)
      instance = new GmatInterface();
   return instance;
}


//------------------------------------------------------------------------------
// void OpenScript()
//------------------------------------------------------------------------------
void GmatInterface::OpenScript()
{
  // if (!mInStringStream)
    //  mInStringStream = new std::istringstream;

}


//------------------------------------------------------------------------------
// void ClearScrip()
//------------------------------------------------------------------------------
void GmatInterface::ClearScript()
{
   mStringStream = wxT("");
   Moderator::GetUiInterpreter()->CloseCurrentProject();
}


//------------------------------------------------------------------------------
// void PutScript(char *str)
//------------------------------------------------------------------------------
/*
 * Appends script to a string stream.
 *
 * @param <str> string to append
 */
//------------------------------------------------------------------------------
void GmatInterface::PutScript(wxString &str)
{
   mStringStream << str << wxT("\n");
   #ifdef DEBUG_GMAT_INTERFACE
   MessageInterface::ShowMessage(wxT("GmatInterface::PutScript() str=%s\n"), str);
   #endif
}


//------------------------------------------------------------------------------
// void BuildObject()
//------------------------------------------------------------------------------
/*
 * Clears resource and build new objects from a internal string stream.
 */
//------------------------------------------------------------------------------
void GmatInterface::BuildObject()
{
  
   Moderator *moderator = Moderator::Instance();
   mPassedInterpreter = false;
   //std::streambuf *streamBuf = mStringStream.rdbuf();

   // redirect mInStringStream into mStringStream
   //RedirectBuffer(mInStringStream, streamBuf);
   wxStringInputStream mInStringStream (mStringStream);
//   wxTextInputStream mInStringStream( mInStringStreamInput);
   
   #ifdef DEBUG_GMAT_INTERFACE
   //loj: 8/31/04 Why this causes problem for long scripts? buffer overflow?
   //MessageInterface::ShowMessage
   //   (wxT("GmatInterface::BuildObject() mStringStream.str=\n%s"), mStringStream.str().c_str());
   //MessageInterface::ShowMessage
   //   (wxT("GmatInterface::BuildObject() mInStringStream.str=\n%s\n"), mInStringStream->str().c_str());
   #endif
   
   // flag to clear objects and mission sequence
   mPassedInterpreter = moderator->InterpretScript(&mInStringStream, true);
   Moderator::GetUiInterpreter()->UpdateView(3);
   
   // empty the buffer, once objects are created
   mStringStream = wxT("");
}


//------------------------------------------------------------------------------
// void UpdateObject()
//------------------------------------------------------------------------------
/*
 * Build and updates objects from a internal string stream without clearing the
 * resource.
 */
//------------------------------------------------------------------------------
void GmatInterface::UpdateObject()
{
  
   Moderator *moderator = Moderator::Instance();
   

   // redirect mInStringStream into mStringStream
   // RedirectBuffer(mInStringStream, streamBuf);
   wxStringInputStream mInStringStream (mStringStream);
  // wxTextInputStream mInStringStream( mInStringStreamInput);
   
   #ifdef DEBUG_GMAT_INTERFACE
   //loj: 8/31/04 Why this causes problem for long scripts? buffer overflow?
   //MessageInterface::ShowMessage
   //   (wxT("GmatInterface::UpdateObject() mStringStream.str=\n%s"), mStringStream.str().c_str());
   //MessageInterface::ShowMessage
   //   (wxT("GmatInterface::UpdateObject() mInStringStream.str=\n%s\n"), mInStringStream->str().c_str());
   #endif
   
   // flag not to clear objects and mission sequence
   moderator->InterpretScript(&mInStringStream, false);
   Moderator::GetUiInterpreter()->UpdateView(3);
   
   // empty the buffer, once objects are created
   mStringStream = wxT("");
}


//------------------------------------------------------------------------------
// void RunScript()
//------------------------------------------------------------------------------
/*
 * Executues commands from existing objects.
 */
//------------------------------------------------------------------------------
void GmatInterface::RunScript()
{
   #ifdef DEBUG_GMAT_INTERFACE
   MessageInterface::ShowMessage
      (wxT("GmatInterface::RunScript() entered. mPassedInterpreter=%d\n"),
       mPassedInterpreter);
   #endif

   if (mPassedInterpreter)
      Moderator::Instance()->RunScript();
   
   Moderator::GetUiInterpreter()->UpdateView(4);
}


//------------------------------------------------------------------------------
// bool ExecuteCallback()
//------------------------------------------------------------------------------
bool GmatInterface::ExecuteCallback()
{
   #ifdef DEBUG_TEST_CALLBACK
      MessageInterface::ShowMessage(wxT("GmatInterface::ExecuteCallback being called ...\n"));
   #endif
   if (callbackObj)
   {
      callbackObj->ExecuteCallback();
      return true;
   }
   else
   {
      //*************** TEMPORARY tuff to test MATLAB->GMAT part ******************
      MessageInterface::ShowMessage(wxT("call back object is NULL, so returning false\n"));
      //*************** TEMPORARY tuff to test MATLAB->GMAT part ******************
      return false;
   }
}


//------------------------------------------------------------------------------
// bool RegisterCallbackServer(GmatBase *callbackObject)
//------------------------------------------------------------------------------
bool GmatInterface::RegisterCallbackServer(GmatBase *callbackObject)
{
   #ifdef DEBUG_TEST_CALLBACK
      MessageInterface::ShowMessage(
      wxT("GmatInterface::RegisterCallbackServer being called with object %s \"%s\"...\n"),
      callbackObject->GetTypeName().c_str(), callbackObject->GetName().c_str());
   #endif
   callbackObj = callbackObject;
   return true;
}


//------------------------------------------------------------------------------
// wxChar * GetCallbackStatus()
//------------------------------------------------------------------------------
/*
 * @return the status of the callback execution (wxT("Executing"), wxT("Completed")).
 */
//------------------------------------------------------------------------------
wxChar * GmatInterface::GetCallbackStatus()
{
   #ifdef DEBUG_TEST_CALLBACK
      MessageInterface::ShowMessage(
      wxT("GmatInterface::GetCallbackStatus being called ...\n"));
   #endif
   if (!callbackObj) // not running a callback - why are you asking?
   {
      dataString = wxT("Completed");
   }
   else
   {
      if (callbackObj->IsCallbackExecuting())
         dataString = wxT("Executing");
      else
         dataString = wxT("Completed");
   }
   #ifdef DEBUG_TEST_CALLBACK
   MessageInterface::ShowMessage
      (wxT("GmatInterface::GetCallbackStatus() dataString=<%s>\n"), dataString);
   #endif
   return (wxChar *) dataString.c_str();
}

//------------------------------------------------------------------------------
// void PutCallbackData(wxString &data)
//------------------------------------------------------------------------------
/*
 */
//------------------------------------------------------------------------------
void  GmatInterface::PutCallbackData(wxString &data)
{
   #ifdef DEBUG_TEST_CALLBACK
      MessageInterface::ShowMessage(
      wxT("GmatInterface::PutCallbackData being called with data = %s\n"), data.c_str());
   #endif
   if (callbackObj)
   {
      if (!(callbackObj->PutCallbackData(data)))
         throw InterfaceException(
         wxT("GmatInterface::Error setting callback data on callback server"));
   }
}

//------------------------------------------------------------------------------
// wxChar * GetCallbackResults()
//------------------------------------------------------------------------------
/*
 * @return the status of the callback execution (wxT("Executing"), wxT("Completed")).
 */
//------------------------------------------------------------------------------
wxChar * GmatInterface::GetCallbackResults()
{
   #ifdef DEBUG_TEST_CALLBACK
      MessageInterface::ShowMessage(
      wxT("GmatInterface::GetCallbackResults being called ...\n"));
   #endif
   if (!callbackObj) // not running a callback - why are you asking?
   {
      dataString = wxT("ERROR!!");
   }
   else
   {
      dataString = callbackObj->GetCallbackResults();
   }
   #ifdef DEBUG_TEST_CALLBACK
   MessageInterface::ShowMessage
      (wxT("GmatInterface::GetCallbackData() dataString=<%s>\n"), dataString);
   #endif
   return (wxChar *) dataString.c_str();
}

//------------------------------------------------------------------------------
// wxChar * GetRunState()
//------------------------------------------------------------------------------
/*
 * @return the state of system (wxT("RUNNING"), wxT("PAUSED"), wxT("IDLE")).
 */
//------------------------------------------------------------------------------
wxChar * GmatInterface::GetRunState()
{
   Gmat::RunState state = Moderator::Instance()->GetRunState();
   
   if (state == Gmat::RUNNING)
      dataString = wxT("Running");
   else if (state == Gmat::PAUSED)
      dataString = wxT("Paused");
   else if (state == Gmat::IDLE)
      dataString = wxT("Idle");
   else
      dataString = wxT("Unknown");
   
   #ifdef DEBUG_GMAT_INTERFACE
   MessageInterface::ShowMessage
      (wxT("GmatInterface::GetRunState() state=%d, dataString=<%s>\n"), state,
       dataString);
   #endif
   
   return (wxChar *) dataString.c_str();
}


//------------------------------------------------------------------------------
// wxChar * GetParameter(const wxString &name)
//------------------------------------------------------------------------------
/*
 * It retrieves a Parameter pointer from the Sandbox, if it is not found in the
 * Sandbox, it retrieves it from the Configuration.
 *
 * @return string value of the parameter in the Sandbox or in the Configuration.
 */
//------------------------------------------------------------------------------
wxChar * GmatInterface::GetParameter(const wxString &name)
{
   #ifdef DEBUG_GMAT_INTERFACE
   MessageInterface::ShowMessage
      (wxT("GmatInterface::GetParameter() name=%s\n"), name.c_str());
   #endif
   
   dataString = wxT("-123456789.123456789\0");
   Parameter *param = NULL;
   GmatBase *obj = NULL;
   
   try
   {
      obj = Moderator::Instance()->GetInternalObject(name);
   }
   catch (BaseException &)
   {
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** Could not find \"%s\" in the Sandbox. ")
          wxT("Trying Configuration...\n"), name.c_str());
   }
   
   // if internal object not found, get configured object (loj: 2008.03.04)
   if (obj == NULL)
      param = Moderator::Instance()->GetParameter(name);
   else
      param = (Parameter*)obj;
   
   #ifdef DEBUG_GMAT_INTERFACE
   MessageInterface::ShowMessage(wxT("   internal obj=%p, param=%p\n"), obj, param);
   #endif
   
   if (param != NULL)
   {
      #ifdef DEBUG_GMAT_INTERFACE
      MessageInterface::ShowMessage
         (wxT("GmatInterface::GetParameter() evaluate the parameter:%s, type=%s\n"),
          param->GetName().c_str(), param->GetTypeName().c_str());
      #endif
      
      //loj: 2/16/05 param->Evaluate() causes system to crash!!
      // so just get the last value without evaluting
      //param->Evaluate(); 
      wxString str = param->ToString(); // returns last value
      dataString = wxT("[") + str + wxT("]");
      
      #ifdef DEBUG_GMAT_INTERFACE
      MessageInterface::ShowMessage
         (wxT("GmatInterface::GetParameter() str=%s\n"), str.c_str());
      #endif
      
   }
   else
   {
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** Could not find \"%s\" in the Configuration\n"), name.c_str());
   }
   
   return (wxChar *) dataString.c_str();
}


//------------------------------------------------------------------------------
// wxChar * GetGmatObject(const wxString &name)
//------------------------------------------------------------------------------
/*
 * It retrieves an object pointer from the Sandbox, if it is not found in the
 * Sandbox, it retrieves it from the Configuration.
 *
 * @return serialized string value of the internal object in the Sandbox or
 *         object in the Configuration
 */
//------------------------------------------------------------------------------
wxChar * GmatInterface::GetGmatObject(const wxString &name)
{
   #ifdef DEBUG_GMAT_INTERFACE
   MessageInterface::ShowMessage
      (wxT("GmatInterface::GetGmatObject() name=%s\n"), name.c_str());
   #endif
   
   dataString = wxT("-123456789.123456789\0");
   GmatBase *obj = NULL;
   
   try
   {
      obj = Moderator::Instance()->GetInternalObject(name);
   }
   catch (BaseException &)
   {
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** Could not find \"%s\" in the Sandbox. ")
          wxT("Trying Configuration...\n"), name.c_str());
      
      // if internal object not found, get configured object (loj: 2008.03.04)
      obj = Moderator::Instance()->GetConfiguredObject(name);
   }
   
   if (obj != NULL)
   {
      #ifdef DEBUG_GMAT_INTERFACE
      MessageInterface::ShowMessage
         (wxT("GmatInterface::GetGmatObject() get serialized string of object name:")
          wxT("%s, type=%s\n"), obj->GetName().c_str(), obj->GetTypeName().c_str());
      #endif
      
      dataString = obj->GetGeneratingString(Gmat::MATLAB_STRUCT);
      
      #ifdef DEBUG_GMAT_INTERFACE
      MessageInterface::ShowMessage(wxT("str=%s\n"), str.c_str());
      #endif
   }
   else
   {
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** Could not find \"%s\" in the Configuration\n"), name.c_str());
   }
   
   return (wxChar *) dataString.c_str();
}


//------------------------------------------------------------------------------
// void CheckUserInterrupt()
//------------------------------------------------------------------------------
/*
 * Calls Moderator::GetUserInterrupt() to check if user interrupted the
 * mission sequence.
 */
//------------------------------------------------------------------------------
void GmatInterface::CheckUserInterrupt()
{
   Moderator::Instance()->GetUserInterrupt();
}


//---------------------------------
// private methods
//---------------------------------

//------------------------------------------------------------------------------
// GmatInterface()
//------------------------------------------------------------------------------
GmatInterface::GmatInterface()
{
   //mInStringStream = NULL;
   callbackObj     = NULL;
}


//------------------------------------------------------------------------------
// GmatInterface()
//------------------------------------------------------------------------------
GmatInterface::~GmatInterface()
{
  // if (mInStringStream)
    //  delete mInStringStream;
}
  
