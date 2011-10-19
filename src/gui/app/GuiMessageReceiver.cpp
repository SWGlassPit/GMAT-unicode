//$Id: GuiMessageReceiver.cpp 9585 2011-06-10 19:54:28Z lindajun $
//------------------------------------------------------------------------------
//                             GuiMessageReceiver
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
// Author: Darrel Conway, based on code by Linda Jun
// Created: 2008/04/28
//
/**
 * Defines operations on messages.
 */
//------------------------------------------------------------------------------
#include "gmatwxdefs.hpp"
#include "GmatAppData.hpp"
#include "ViewTextFrame.hpp"
#include <stdarg.h>                // for va_start(), va_end()
#include <stdio.h>                 // for vsprintf()
#include <string.h>                // for strlen()

#ifdef __WXMSW__                   // for malloc.h if MS Windows only
#include <malloc.h>                
#endif

#include <iostream>                // for cout, endl
#include <fstream>
#include <sstream>
#include <queue>                   // for queue
#include "GuiMessageReceiver.hpp"  // for GuiMessageReceiver functions
#include "BaseException.hpp"
#include "FileManager.hpp"         // for GetFullPathname()
#include "GmatGlobal.hpp"          // for RunBachMode()
#include "FileUtil.hpp"            // for ParsePathName()
//#include "StringUtil.hpp"          // for GmatStringUtil::Replace()

//---------------------------------
//  static data
//---------------------------------
GuiMessageReceiver*  GuiMessageReceiver::instance = NULL;

//---------------------------------
//  private functions
//---------------------------------

//------------------------------------------------------------------------------
//  GuiMessageReceiver()
//------------------------------------------------------------------------------
/**
 * Constructor, called from the Instance method to create the singleton.
 */
//------------------------------------------------------------------------------
GuiMessageReceiver::GuiMessageReceiver() :
   MAX_MESSAGE_LENGTH      (10000),
   logFile                 (NULL),
   logEnabled              (false),
   logFileSet              (false)
{
}

//------------------------------------------------------------------------------
//  GuiMessageReceiver()
//------------------------------------------------------------------------------
/**
 * Class destructor.
 */
//------------------------------------------------------------------------------
GuiMessageReceiver::~GuiMessageReceiver()
{
}

//---------------------------------
//  public functions
//---------------------------------

//------------------------------------------------------------------------------
// GuiMessageReceiver* Instance()
//------------------------------------------------------------------------------
/**
 * Singleton accessor method
 * 
 * This method creates the GuiMessageReceiver singleton if it has not been
 * constructed, and returns the singleton instance.
 * 
 * @return The GuiMessageReceiver instance.
 */
//------------------------------------------------------------------------------
GuiMessageReceiver* GuiMessageReceiver::Instance()
{
   if (instance == NULL)
      instance = new GuiMessageReceiver;
   
   return instance;   
}


//------------------------------------------------------------------------------
//  void ClearMessage()
//------------------------------------------------------------------------------
/**
 * Clears the message window.
 */  
//------------------------------------------------------------------------------
void GuiMessageReceiver::ClearMessage()
{
   GmatAppData *appData = GmatAppData::Instance();
   if (appData->GetMessageTextCtrl() != NULL)
      appData->GetMessageTextCtrl()->Clear();
}


//------------------------------------------------------------------------------
//  int GetNumberOfMessageLines()
//------------------------------------------------------------------------------
/**
 * Returns the number of lines of text in the message window.
 * 
 * @return The number of lines used.
 */
//------------------------------------------------------------------------------
int GuiMessageReceiver::GetNumberOfMessageLines()
{
   GmatAppData *appData = GmatAppData::Instance();
   ViewTextFrame* theMessageWindow = appData->GetMessageWindow();
   if (theMessageWindow != NULL)
   {
      return theMessageWindow->GetNumberOfLines();
   }
   else
   {
      wxLogError(wxT("GuiMessageReceiver::GetNumberOfMessageLines(): ")
                 wxT("MessageWindow was not created."));
      wxLog::FlushActive();
      return 0;
   }

   return 0;
}




//------------------------------------------------------------------------------
//  void ShowMessage(const wxString &msg, ...)
//------------------------------------------------------------------------------
/**
 * Displays a message passed in as a char* and a variable argument
 * list.  Throws std::bad_alloc on memory exhaustion.
 * 
 * @param msg The message, possibly including markers for variable argument 
 *            substitution.
 * @param ... The optional list of parameters that are inserted into the msg 
 *            string.
 */
//------------------------------------------------------------------------------
void GuiMessageReceiver::ShowMessage(const wxString &msg, ...)
{
   short    ret;
   short    size;
   va_list  marker;
   wxString msgBuffer;
   
   // msg is vsprintf format
   // actual max message length is MAX_MESSAGE_LENGTH
   //LogMessage("strlen(msg)=%d, size=%d\n", strlen(msg), size);
   
//   if( (msgBuffer = (char *)malloc(size)) != NULL )
//   {
//      va_start(marker, msg);
//      ret = vsprintf(msgBuffer, msg, marker);
//      va_end(marker);
//      //LogMessage("ret from vsprintf()=%d\n", ret);
//   }
//   else
//   {
//      msgBuffer =
//         "*** WARNING *** Cannot allocate enough memory to show the message.\n";
//   }


   // For older C++ compilers, duplicate that behavior by hand.

   // Process the message
   va_start(marker, msg);
   ret = msgBuffer.PrintfV( msg, marker);
   va_end(marker);
   //LogMessage("ret from vsprintf()=%d\n", ret);
   
   GmatAppData *appData = GmatAppData::Instance();
   if (appData->GetMessageTextCtrl() != NULL)
   {
      appData->GetMessageTextCtrl()->AppendText(wxString(msgBuffer));
      appData->GetMessageTextCtrl()->PageDown();
      appData->GetMessageTextCtrl()->Update();
   }
   LogMessage(wxString(msgBuffer));

//   free(msgBuffer);
} // end ShowMessage()


//------------------------------------------------------------------------------
//  void PopupAbortContinue(const wxString abortMsg, ...)
//------------------------------------------------------------------------------
/**
 * Pops up Abort or Continue message box.
 * 
 * @param abortMsg    The abort message
 * @param continueMsg The continue message
 * @param msg         The message
 */
//------------------------------------------------------------------------------
void GuiMessageReceiver::PopupAbortContinue(const wxString &abortMsg,
                                          const wxString &continueMsg,
                                          const wxString &msg)
{  
   popupMessage = msg;
   abortMessage = abortMsg;
   continueMessage = continueMsg;
} // end PopupAbortContinue()



//------------------------------------------------------------------------------
//  static void PopupMessage(Gmat::MessageType msgType, const wxString &msg, ...)
//------------------------------------------------------------------------------
/**
 * Pops up a message in a message box.
 * 
 * This method logs informational messages directed at pop-up message boxes and
 * shows the message as a pop-up.
 *
 * Throws std::bad_alloc in memory exhaustion.
 * 
 * @param msgType The type of message that is displayed, selected from the set
 *                {ERROR_, WARNING_, INFO_} enumerated in the Gmat namespace.
 * @param msg The message, possibly including markers for variable argument 
 *            substitution.
 * @param ... The optional list of parameters that are inserted into the msg 
 *            string.
 */
//------------------------------------------------------------------------------
void GuiMessageReceiver::PopupMessage(Gmat::MessageType msgType, const wxString &msg, ...)
{
   short    ret;
   short    size;
   va_list  marker;
   wxString msgBuffer;
   
   // msg is vsprintf format
   // actual max message length is MAX_MESSAGE_LENGTH
   
   // Process the message
   va_start(marker, msg);
   ret = msgBuffer.PrintfV( msg, marker);
   va_end(marker);

   // if no EOL then append it
   if (msgBuffer.EndsWith(wxT("\n")))
      msgBuffer += wxT("\n");

//   if ( (msgBuffer = (char *)malloc(size)) != NULL )
//   {
//      va_start(marker, msg);
//      ret = vsprintf(msgBuffer, msg, marker);
//      va_end(marker);
//
//      // if no EOL then append it
//      if (msgBuffer[strlen(msgBuffer)-1] != '\n')
//         msgBuffer[strlen(msgBuffer)] = '\n';
//   }
//   else
//   {
//      msgBuffer = "*** WARNING *** Cannot allocate enough memory to show the message.\n";
//   }
   
   // always show message
   ShowMessage(msgBuffer);
   
   if (GmatGlobal::Instance()->IsBatchMode() != true)
   {
      switch (msgType)
      {
      case Gmat::ERROR_:
         (void)wxMessageBox(msgBuffer,
                            wxT("GMAT Error"));
         break;
      case Gmat::WARNING_:
         (void)wxMessageBox(msgBuffer,
                            wxT("GMAT Warning"));
         break;
      case Gmat::INFO_:
         (void)wxMessageBox(msgBuffer,
                            wxT("Information"));
         break;
      default:
         break;
      };
   }
   
//   free(msgBuffer);
} // end PopupMessage()


//------------------------------------------------------------------------------
// wxString GetLogFileName()
//------------------------------------------------------------------------------
/**
 * Retrieves the fully qualified name of the log file.
 * 
 * @return The name of the log file, including path information.
 */
//------------------------------------------------------------------------------
wxString GuiMessageReceiver::GetLogFileName()
{
   FileManager *fm = FileManager::Instance();
   wxString fname;
   try
   {
      if (logFileName == wxT(""))
      {
         fname = fm->GetFullPathname(wxT("LOG_FILE"));
      }
      else
      {
         wxString outputPath = fm->GetPathname(FileManager::LOG_FILE);
         
         // add output path if there is no path
         if (logFileName.find(wxT("/")) == logFileName.npos &&
             logFileName.find(wxT("\\")) == logFileName.npos)
         {
            fname = outputPath + logFileName;
         }
      }
   }
   catch (BaseException &e)
   {
      GuiMessageReceiver::ShowMessage
         (wxT("**** ERROR **** ") + e.GetFullMessage() + 
          wxT("So setting log file name to GmatLog.txt"));
      
      fname = wxT("GmatLog.txt");
   }
   
   return fname;
}




//------------------------------------------------------------------------------
//  void LogMessage(const wxString &msg, ...)
//------------------------------------------------------------------------------
/**
 * Logs a variable argument formatted message to the log file.
 * 
 * This method calls the wxString vrrersion of LogMessage to do the actual
 * logging.
 *
 * Throws std::bad_alloc on memory exhaustion.
 * 
 * @param msg The message, possibly including markers for variable argument 
 *            substitution.
 * @param ... The optional list of parameters that are inserted into the msg 
 *            string.
 */
//------------------------------------------------------------------------------
void GuiMessageReceiver::LogMessage(const wxString &msg, ...)
{
   short    ret;
   short    size;
   va_list  marker;
   wxString msgBuffer;
   
   // msg is vsprintf format
   // actual max message length is MAX_MESSAGE_LENGTH
   
//   if ( (msgBuffer = (char *)malloc(size)) != NULL )
//   {
//      va_start(marker, msg);
//      ret = vsprintf(msgBuffer, msg, marker);
//      va_end(marker);
//   }
//   else
//   {
//      msgBuffer = "*** WARNING *** Cannot allocate enough memory to log the message.\n";
//   }
//

   // Note: 'new' throws an exception of type std::bad_alloc on failure.
   // (Note that if an exception is thrown, no memory will have been
   // allocated, so there will be no memory leak.)

   // For older C++ compilers, duplicate that behavior by hand.

   // Process the message
   va_start(marker, msg);
   ret = msgBuffer.PrintfV( msg, marker);
   va_end(marker);
   
   std::cout << msg;
   
   if (logEnabled)
   {
      if (logFile == NULL)
      {
         SetLogFile(GetLogFileName());
      }
   }
   else if (!logFileSet)
   {
      OpenLogFile(logFileName);
   }
   
   if (logFile)
   {
      //wxString tempStr = GmatStringUtil::Replace(msg, "%", "%%");
      //fprintf(logFile, "%s", tempStr.c_str());
      fprintf(logFile, "%s", (char *) msg.char_str());
      fflush(logFile);
   }
   
//   free(msgBuffer);
} // end LogMessage()


//------------------------------------------------------------------------------
// void SetLogEnable(bool flag)
//------------------------------------------------------------------------------
/**
 * Turns logging on or off.
 * 
 * @param flag The new loggign state -- true enables logging, and false disables 
 *             it.  The logging state is idempotent.
 */
//------------------------------------------------------------------------------
void GuiMessageReceiver::SetLogEnable(bool flag)
{
   logEnabled = flag;
}


//------------------------------------------------------------------------------
// void SetLogPath(const wxString &pathname, bool append = false)
//------------------------------------------------------------------------------
/*
 * Sets log file path with keeping log file name as is.
 *
 * @param  pathname  log file path name, such as "/newpath/test1/"
 * @param  append  true if appending log message (false)
 */
//------------------------------------------------------------------------------
void GuiMessageReceiver::SetLogPath(const wxString &pathname, bool append)
{
   FileManager *fm = FileManager::Instance();
   wxString fname;
   try
   {
      wxString filename = fm->GetFilename(FileManager::LOG_FILE);
      fname = pathname + filename;
   }
   catch (BaseException &e)
   {
      GuiMessageReceiver::ShowMessage
         (wxT("**** ERROR **** ") + e.GetFullMessage() + 
          wxT("So setting log file name to GmatLog.txt"));
      
      fname = wxT("GmatLog.txt");
   }
   
   OpenLogFile(fname, append);
   
}


//------------------------------------------------------------------------------
// void SetLogFile(const wxString &filename)
//------------------------------------------------------------------------------
/*
 * Calls OpenLogFile() to set the log file path and name and then open the log.
 *
 * @param  filename  log file name, such as "/newpath/test1/GmatLog.txt"
 */
//------------------------------------------------------------------------------
void GuiMessageReceiver::SetLogFile(const wxString &filename)
{
   wxString fname = filename;
   
   if (GmatFileUtil::ParsePathName(fname) == wxT(""))
   {
      FileManager *fm = FileManager::Instance();
      wxString outPath = fm->GetFullPathname(FileManager::OUTPUT_PATH);
      fname = outPath + fname;
   }
   
   OpenLogFile(fname);
}


//------------------------------------------------------------------------------
// void OpenLogFile(const wxString &filename, bool append = false)
//------------------------------------------------------------------------------
/*
 * Sets the log file name and opens the log file.
 *
 * @param filename  log file name, such as "/newpath/test1/GmatLog.txt"
 * @param append  true if appending log message
 */
//------------------------------------------------------------------------------
void GuiMessageReceiver::OpenLogFile(const wxString &filename, bool append)
{
   logFileName = filename;
   
   if (logFile)
      fclose(logFile);
   
   if (append)
      logFile = fopen(logFileName.char_str(), "a");
   else
      logFile = fopen(logFileName.char_str(), "w");
   
   if (!logFile)
   {
      std::cout << wxT("**** ERROR **** Error setting the log file to ") << logFileName
                << wxT("\nSo setting it to \"GmatLog.txt\" in the executable directory\n");
      
      logFileName = wxT("GmatLog.txt");
      
      if (append)
         logFile = fopen(logFileName.char_str(), "a");
      else
         logFile = fopen(logFileName.char_str(), "w");
   }
   
   if (logFile)
   {
      fprintf(logFile, "GMAT Build Date: %s %s\n\n",  __DATE__, __TIME__);
      fprintf(logFile, "GMAT Log file set to %s\n", (char *) logFileName.char_str());
      
      logFileSet = true;
   }
}


//------------------------------------------------------------------------------
// void CloseLogFile()
//------------------------------------------------------------------------------
/**
 * Closes the log file.
 */
//------------------------------------------------------------------------------
void GuiMessageReceiver::CloseLogFile()
{
   if (logFile)
      fclose(logFile);
   
   logFile = NULL;
   logFileSet = false;
}


//------------------------------------------------------------------------------
//  wxString GetMessage()
//------------------------------------------------------------------------------
/**
 * Pops the messages off the message queue and concatenates them together.
 * 
 * @return The concatenated messages.
 */
//------------------------------------------------------------------------------
wxString GuiMessageReceiver::GetMessage()
{
   wxString msg;
   
   while (!GuiMessageReceiver::messageQueue.empty())
   {
      msg = msg + messageQueue.front().c_str();
      messageQueue.pop();
   }
   GuiMessageReceiver::messageExist = 0;
   
   return msg;
}

//------------------------------------------------------------------------------
// void PutMessage(const wxString &msg)
//------------------------------------------------------------------------------
/**
 * Push the message into queue
 */
//------------------------------------------------------------------------------
void GuiMessageReceiver::PutMessage(const wxString &msg)
{
   messageQueue.push(msg);
}

//------------------------------------------------------------------------------
// void ClearMessageQueue()
//------------------------------------------------------------------------------
/**
 * Tells the MessageReceiver to clear the message queue.
 */
//------------------------------------------------------------------------------
void GuiMessageReceiver::ClearMessageQueue()
{
   while (!GuiMessageReceiver::messageQueue.empty())
      messageQueue.pop();
   
   GuiMessageReceiver::messageExist = 0;
}


