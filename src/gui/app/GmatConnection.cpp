//$Id: GmatConnection.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                               GmatConnection
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// ** Legal **
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Linda Jun
// Created: 2004/08/27
//
/**
 * Implements GmatConnection which provides service to client.
 */
//------------------------------------------------------------------------------

#include "GmatConnection.hpp"
#include "GmatInterface.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_CONNECTION
//#define DEBUG_CONNECTION_EXECUTE
//#define DEBUG_CONNECTION_POKE
//#define DEBUG_CONNECTION_REQUEST
//#define DEBUG_CONNECTION_ADVISE


//------------------------------------------------------------------------------
// GmatConnection()
//------------------------------------------------------------------------------
GmatConnection::GmatConnection()
   : wxConnection()
{   
   #ifdef DEBUG_CONNECTION
   MessageInterface::ShowMessage
      ("GmatConnection() constructor entered, this=%p\n", this);
   #endif
}


//------------------------------------------------------------------------------
// ~GmatConnection()
//------------------------------------------------------------------------------
GmatConnection::~GmatConnection()
{
   #ifdef DEBUG_CONNECTION
   MessageInterface::ShowMessage
      ("~GmatConnection() destructor entered, this=%p\n", this);
   #endif   
}


//------------------------------------------------------------------------------
// wxChar* OnRequest(const wxString& WXUNUSED(topic), const wxString& item,
//                   int * WXUNUSED(size), wxIPCFormat WXUNUSED(format))
//------------------------------------------------------------------------------
/*
 * This method responds to the client application to request data from the server.
 *
 * @param <topic>  Unused
 * @param <item>   Object or parameter name to retrive value from
 * @param <size>   Unused
 * @param <format> Unused
 *
 * @return Object or parameter value string.
 */
//------------------------------------------------------------------------------
wxChar* GmatConnection::OnRequest(const wxString& WXUNUSED(topic),
                                  const wxString& item,
                                  int * WXUNUSED(size),
                                  wxIPCFormat WXUNUSED(format))
{
   #ifdef DEBUG_CONNECTION_REQUEST
   MessageInterface::ShowMessage
      ("GmatConnection::OnRequest() %s\n", item.c_str());
   #endif
   
   // Check for user interrupt first (loj: 2007.05.11 Added)
   GmatInterface::Instance()->CheckUserInterrupt();
   
   // How can I tell whether item is an object or a parameter?
   // For now GetGMATObject.m appends '.' for object name.
   
   wxChar *data;
   if (item.Last() == '.')
   {
      wxString tempItem = item;
      tempItem.RemoveLast();
      data = GmatInterface::Instance()->GetGmatObject(tempItem);
   }
   else if (item == wxT("RunState"))
   {
      data = GmatInterface::Instance()->GetRunState();
      
      #ifdef DEBUG_CONNECTION_REQUEST
      MessageInterface::ShowMessage
         (wxT("GmatConnection::OnRequest() data=%s\n"), data);
      #endif
   }
   else if (item == wxT("CallbackStatus"))
   {
      data = GmatInterface::Instance()->GetCallbackStatus();
      
      #ifdef DEBUG_CONNECTION_REQUEST
      MessageInterface::ShowMessage
         (wxT("GmatConnection::OnRequest() data=%s\n"), data);
      #endif
   }
   else if (item == wxT("CallbackResults"))
   {
      data = GmatInterface::Instance()->GetCallbackResults();
      
      #ifdef DEBUG_CONNECTION_REQUEST
      MessageInterface::ShowMessage
         (wxT("GmatConnection::OnRequest() data=%s\n"), data);
      #endif
   }
   else
   {
      data = GmatInterface::Instance()->GetParameter(item);
   }
   
   return data;
}


//------------------------------------------------------------------------------
// bool OnExecute(const wxString& WXUNUSED(topic),
//------------------------------------------------------------------------------
bool GmatConnection::OnExecute(const wxString& WXUNUSED(topic),
                               wxChar *data,
                               int WXUNUSED(size),
                               wxIPCFormat WXUNUSED(format))
{
   #ifdef DEBUG_CONNECTION_EXECUTE
   MessageInterface::ShowMessage
      ("GmatConnection::OnExecute() command: %s\n", data);
   #endif
   
   return TRUE;
}


//------------------------------------------------------------------------------
// bool OnPoke(const wxString& WXUNUSED(topic),
//------------------------------------------------------------------------------
bool GmatConnection::OnPoke(const wxString& WXUNUSED(topic),
                            const wxString& item,
                            wxChar *data,
                            int WXUNUSED(size),
                            wxIPCFormat WXUNUSED(format))
{
   #ifdef DEBUG_CONNECTION_POKE
   MessageInterface::ShowMessage
      ("GmatConnection::OnPoke() %s = %s\n", item.c_str(), data);
   #endif
   
   //------------------------------
   // save data to string stream
   //------------------------------
   wxString theString(data);
   if (theString == wxT("Open;"))
   {
      GmatInterface::Instance()->OpenScript();
   }
   else if (theString == wxT("Clear;"))
   {
      GmatInterface::Instance()->ClearScript();
   }
   else if (theString == wxT("Build;"))
   {
      GmatInterface::Instance()->BuildObject();
   }
   else if (theString == wxT("Update;"))
   {
      GmatInterface::Instance()->UpdateObject();
   }
   else if (theString == wxT("Build+Run;"))
   {
      GmatInterface::Instance()->BuildObject();
      GmatInterface::Instance()->RunScript();
   }
   else if (theString == wxT("Run;"))
   {
      GmatInterface::Instance()->RunScript();
   }
   else if (theString == wxT("Callback;"))
   {
      GmatInterface::Instance()->ExecuteCallback();
   }
   else if (theString.StartsWith( wxT("CallbackData")))
   {
      wxString callbackData = theString.substr(13,1024);
      #ifdef DEBUG_CONNECTION_POKE
         MessageInterface::ShowMessage
            ("GmatConnection::callbackData = %s\n", callbackData.c_str());
      #endif
      GmatInterface::Instance()->PutCallbackData(callbackData);
   }
   else
   {
      GmatInterface::Instance()->PutScript(theString);
   }
   
   return TRUE;
}


//------------------------------------------------------------------------------
// bool OnStartAdvise(const wxString& WXUNUSED(topic),
//------------------------------------------------------------------------------
bool GmatConnection::OnStartAdvise(const wxString& WXUNUSED(topic),
                                   const wxString& item)
{
   #ifdef DEBUG_CONNECTION_ADVISE
   MessageInterface::ShowMessage
      ("GmatConnection::OnStartAdvise() %s\n", item.c_str());
   #endif
   
   //#ifdef DEBUG_CONNECTION_ADVISE
   //char* data = GmatInterface::Instance()->GetRunState();
   //MessageInterface::ShowMessage
   //   ("GmatConnection::OnStartAdvise() data=%s\n", data);
   //#endif
   
   return TRUE;
}


//------------------------------------------------------------------------------
// bool OnDisconnect()
//------------------------------------------------------------------------------
bool GmatConnection::OnDisconnect()
{
   #ifdef DEBUG_CONNECTION
   MessageInterface::ShowMessage
      ("GmatConnection::OnDisconnect() entered, this=%p\n", this);
   #endif
   return true;
}

