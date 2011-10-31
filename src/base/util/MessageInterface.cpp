//$Id: MessageInterface.cpp 9577 2011-06-08 15:09:03Z djcinsb $
//------------------------------------------------------------------------------
//                             MessageInterface
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
/**
 * Implements operations on messages.
 */
//------------------------------------------------------------------------------
#include "MessageInterface.hpp"
#include <stdarg.h>              // for va_start() and va_end()
#include <cstdlib>

#include <cstdlib>                      // Required for GCC 4.3

//---------------------------------
//  static data
//---------------------------------
MessageReceiver* MessageInterface::theMessageReceiver = NULL;
const int MessageInterface::MAX_MESSAGE_LENGTH = 10000;


//---------------------------------
//  private methods
//---------------------------------


//------------------------------------------------------------------------------
// MessageInterface()
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
MessageInterface::MessageInterface()
{
}

//------------------------------------------------------------------------------
// ~MessageInterface()
//------------------------------------------------------------------------------
/**
 * Class destructor.
 */
//------------------------------------------------------------------------------
MessageInterface::~MessageInterface()
{
}


//---------------------------------
//  public static methods
//---------------------------------


//------------------------------------------------------------------------------
// bool SetMessageReceiver(MessageReceiver *mr)
//------------------------------------------------------------------------------
bool MessageInterface::SetMessageReceiver(MessageReceiver *mr)
{
   theMessageReceiver = mr;
   return true;
}


//------------------------------------------------------------------------------
// MessageReceiver* GetMessageReceiver()
//------------------------------------------------------------------------------
MessageReceiver* MessageInterface::GetMessageReceiver()
{
   return theMessageReceiver;
}



//------------------------------------------------------------------------------
//  void ShowMessage(const wxString &format, ...)
//------------------------------------------------------------------------------
/**
 * Passes a variable argument delimited message to the MessageReceiver.
 *
 * @param format The format, possibly including markers for variable argument
 *                  substitution.
 * @param ...    The optional list of parameters that are inserted into the format
 *                  string.
 */
//------------------------------------------------------------------------------
void MessageInterface::ShowMessage(const wxString &format, ...)
{
   if (theMessageReceiver != NULL)
   {
      short    ret;
      va_list  marker;
      wxString msgBuffer = wxT("");
      wxString msgStr = wxT("*** WARNING *** Cannot allocate enough memory to show the message.\n");
      
      // format is vsprintf format
      // actual max message length is MAX_MESSAGE_LENGTH
      //LogMessage(wxT("strlen(format)=%d, size=%d\n"), strlen(format), size);
      
      va_start(marker, format);
      ret = msgBuffer.PrintfV(format, marker);
      if (ret < 0)
         theMessageReceiver->ShowMessage(wxT("Unable to complete messaging"));
      else
      {
         theMessageReceiver->ShowMessage(msgBuffer);
      }
      va_end(marker);

//      theMessageReceiver->ShowMessage(wxString(msgBuffer));
   }
} // end ShowMessage()



//------------------------------------------------------------------------------
//  static void PopupMessage(Gmat::MessageType msgType, const wxString &format, ...)
//------------------------------------------------------------------------------
/**
 * Passes a variable argument delimited popup message to the MessageReceiver.
 *
 * @param msgType The type of message that is displayed, selected from the set
 *                   {ERROR_, WARNING_, INFO_} enumerated in the Gmat namespace.
 * @param format  The format, possibly including markers for variable argument
 *                    substitution.
 * @param ...     The optional list of parameters that are inserted into the format
 *                   string.
 */
//------------------------------------------------------------------------------
void MessageInterface::PopupMessage(Gmat::MessageType msgType, const wxString &format,
      ...)
{
   if (theMessageReceiver != NULL)
   {
      short    ret;
      short    size;
      va_list  marker;
      wxString msgBuffer = wxT("");
      wxString msgStr(wxT("*** WARNING *** Cannot allocate enough memory to show the message.\n"));

      // format is vsprintf format
      // actual max message length is MAX_MESSAGE_LENGTH
      
      va_start(marker, format);
      ret = msgBuffer.PrintfV( format, marker);
      if (ret < 0)
         theMessageReceiver->PopupMessage(msgType,
               wxT("Unable to complete messaging"));
      else
      {

         // if no EOL then append it
         if (msgBuffer.at(msgBuffer.length()-1) != wxT('\n'))
            msgBuffer.append(1,wxT('\n'));
         theMessageReceiver->PopupMessage(msgType, wxString(msgBuffer));
      }
      va_end(marker);
      
//      theMessageReceiver->PopupMessage(msgType, wxString(msgBuffer));
      
   }
} // end PopupMessage()

//------------------------------------------------------------------------------
// wxString GetLogFileName()
//------------------------------------------------------------------------------
/**
 * Retrieves the fully qualified name of the log file from the MessageReceiver.
 *
 * @return The name of the log file, including path information.
 */
//------------------------------------------------------------------------------
wxString MessageInterface::GetLogFileName()
{
   if (theMessageReceiver == NULL)
      return wxT("");
   return theMessageReceiver->GetLogFileName();
}

//------------------------------------------------------------------------------
// void SetLogEnable(bool flag)
//------------------------------------------------------------------------------
/**
 * Tells the MessageReceiver to turn logging on or off.
 *
 * @param flag The new logging state -- true enables logging, and false disables
 *             it.  The logging state is idempotent.
 */
//------------------------------------------------------------------------------
void MessageInterface::SetLogEnable(bool flag)
{
   if (theMessageReceiver != NULL)
      theMessageReceiver->SetLogEnable(flag);
}

//------------------------------------------------------------------------------
// void SetLogPath(const wxString &pathname, bool append = false)
//------------------------------------------------------------------------------
/*
 * Sends log file path and append state to the MessageReceiver.
 *
 * @param  pathname  log file path name, such as wxT("/newpath/test1/")
 * @param  append  true if appending log message (false)
 */
//------------------------------------------------------------------------------
void MessageInterface::SetLogPath(const wxString &pathname, bool append)
{
   if (theMessageReceiver != NULL)
      theMessageReceiver->SetLogPath(pathname, append);
}

//------------------------------------------------------------------------------
// void SetLogFile(const wxString &filename)
//------------------------------------------------------------------------------
/*
 * Sends the log file path and name to the MessageReceiver.
 *
 * @param  filename  log file name, such as wxT("/newpath/test1/GmatLog.txt")
 */
//------------------------------------------------------------------------------
void MessageInterface::SetLogFile(const wxString &filename)
{
   if (theMessageReceiver != NULL)
      theMessageReceiver->SetLogFile(filename);
}

//------------------------------------------------------------------------------
//  void LogMessage(const wxString &msg, ...)
//------------------------------------------------------------------------------
/**
 * Sends a variable argument message to the MessageReceiver for logging.
 *
 * @param msg The message, possibly including markers for variable argument
 *            substitution.
 * @param ... The optional list of parameters that are inserted into the msg
 *            string.
 */
//------------------------------------------------------------------------------
void MessageInterface::LogMessage(const wxString &msg, ...)
{
   if (theMessageReceiver != NULL)
   {
      short    ret;
      short    size;
      va_list  marker;
      wxString msgBuffer = wxT("");
      
      // msg is vsprintf format
      // actual max message length is MAX_MESSAGE_LENGTH
      //LogMessage(wxT("strlen(msg)=%d, size=%d\n"), strlen(msg), size);
      
      va_start(marker, msg);
      ret = msgBuffer.PrintfV(msg, marker);
      if (ret < 0) // vsprintf failed
         theMessageReceiver->LogMessage(wxT("Unable to complete messaging\n"));
      else
      {
         theMessageReceiver->LogMessage(wxString(msgBuffer));
      }
      va_end(marker);
      
//      theMessageReceiver->LogMessage(wxString(msgBuffer));
   }
}

//------------------------------------------------------------------------------
//  void ClearMessage()
//------------------------------------------------------------------------------
/**
 * Tells the MessageReceiver to clear the message window.
 */
//------------------------------------------------------------------------------
void MessageInterface::ClearMessage()
{
   if (theMessageReceiver != NULL)
      theMessageReceiver->ClearMessage();
}

//------------------------------------------------------------------------------
// wxString GetQueuedMessage()
//------------------------------------------------------------------------------
/**
 * Tells the MessageReceiver to retrieve all message from the queue.
 */
//------------------------------------------------------------------------------
wxString MessageInterface::GetQueuedMessage()
{
   if (theMessageReceiver != NULL)
      return theMessageReceiver->GetMessage();
   else
      return wxT("");
}

//------------------------------------------------------------------------------
// void PutMessage(const wxString &msg)
//------------------------------------------------------------------------------
/**
 * Tells the MessageReceiver to push the message into queue
 */
//------------------------------------------------------------------------------
void MessageInterface::PutMessage(const wxString &msg)
{
   if (theMessageReceiver != NULL)
      theMessageReceiver->PutMessage(msg);
}

//------------------------------------------------------------------------------
// void ClearMessageQueue()
//------------------------------------------------------------------------------
/**
 * Tells the MessageReceiver to clear the message queue.
 */
//------------------------------------------------------------------------------
void MessageInterface::ClearMessageQueue()
{
   if (theMessageReceiver != NULL)
      theMessageReceiver->ClearMessageQueue();
}

