//$Id: ConsoleMessageReceiver.cpp 9517 2011-04-30 21:57:41Z djcinsb $
//------------------------------------------------------------------------------
//                           ConsolMessageReceiver
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
// Created: 2008/05/28
//
/**
 * Implements operations on messages for the Console app.
 */
//------------------------------------------------------------------------------
#include "ConsoleMessageReceiver.hpp"

#include <stdarg.h>                // for va_start(), va_end()
#include <stdio.h>                 // for vsprintf()
#include <string.h>                // for strlen()
#include <iostream>                // for cout, endl
#include <fstream>
//#include <sstream>
#include <queue>                   // for queue
#include "MessageInterface.hpp"    // for MessageInterface functions
#include "BaseException.hpp"
#include "FileManager.hpp"         // for GetFullPathname()
#include "GmatGlobal.hpp"          // for RunBachMode()

#include <cstdlib>                  // For GCC4.4 support

//---------------------------------
//  static data
//---------------------------------
ConsoleMessageReceiver* ConsoleMessageReceiver::theInstance = NULL;

//---------------------------------
//  public methods
//---------------------------------


//------------------------------------------------------------------------------
// ConsoleMessageReceiver* Instance()
//------------------------------------------------------------------------------
/**
 * Singleton accessor method
 * 
 * This method creates the ConsoleMessageReceiver singleton if it has not been
 * constructed, and returns the singleton instance.
 * 
 * @return The ConsoleMessageReceiver instance.
 */
//------------------------------------------------------------------------------
ConsoleMessageReceiver* ConsoleMessageReceiver::Instance()
{
   if (theInstance == NULL)
      theInstance = new ConsoleMessageReceiver;
   
   return theInstance;
}

//------------------------------------------------------------------------------
//  void ShowMessage(const wxString &msgString)
//------------------------------------------------------------------------------
/**
 * Displays a message passed in as an wxString.
 * 
 * This method sends the message to the user's console and to the log file by 
 * calling the variable argument method, ShowMessage(const char *msg, ...).
 * 
 * @param msgString The message that is displayed.
 */
//------------------------------------------------------------------------------
void ConsoleMessageReceiver::ShowMessage(const wxString &msg)
{
   ShowMessage(msg.c_str());
}

//------------------------------------------------------------------------------
//  void ShowMessage(const char *msg, ...)
//------------------------------------------------------------------------------
/**
 * Displays a message passed in as a char* and a variable argument list.
 * 
 * @param msg The message, possibly including markers for variable argument 
 *            substitution.
 * @param ... The optional list of parameters that are inserted into the msg 
 *            string.
 */
//------------------------------------------------------------------------------
void ConsoleMessageReceiver::ShowMessage(const char *msg, ...)
{
   short    ret;
   short    size;
   va_list  marker;
   char     *msgBuffer;
   
   // msg is vsprintf format
   // actual max message length is MAX_MESSAGE_LENGTH
   size = strlen(msg) + MAX_MESSAGE_LENGTH;
   
   // Note: 'new' throws an exception of type std::bad_alloc on failure.
   // (Note that if an exception is thrown, no memory will have been
   // allocated, so there will be no memory leak.)
   msgBuffer = new char[size];

   // For older C++ compilers, duplicate that behavior by hand.
   if (!msgBuffer)
      throw std::bad_alloc();

   // Process the message
   va_start(marker, msg);
   ret = vsprintf(msgBuffer, msg, marker);
   va_end(marker);
   
   LogMessage(wxString(msgBuffer));
   delete [] msgBuffer;
}

//------------------------------------------------------------------------------
//  void PopupMessage(Gmat::MessageType msgType, const wxString &msg)
//------------------------------------------------------------------------------
/**
 * Pops up a message in a message box.
 * 
 * This method logs informational messages directed at pop-up message boxes.  
 * The Console application does not support pop-ups, so the message cannot be 
 * shown as a pop-up.
 * 
 * This method calls the variable argument version of the method to perform the 
 * actual logging.
 * 
 * @param msgType The type of message that is displayed, selected from the set
 *                {ERROR_, WARNING_, INFO_} enumerated in the Gmat namespace.
 * @param msg The message.
 */
//------------------------------------------------------------------------------
void ConsoleMessageReceiver::PopupMessage(Gmat::MessageType msgType, 
      const wxString &msg)
{
   popupMessage = msg;
   messageType = msgType;
   
   PopupMessage(msgType, msg.c_str());
}

//------------------------------------------------------------------------------
//  void PopupMessage(Gmat::MessageType msgType, const char *msg, ...)
//------------------------------------------------------------------------------
/**
 * Pops up a message in a message box.
 * 
 * This method logs informational messages directed at pop-up message boxes.  
 * The Console application does not support pop-ups, so the message cannot be 
 * shown as a pop-up.
 * 
 * @param msgType The type of message that is displayed, selected from the set
 *                {ERROR_, WARNING_, INFO_} enumerated in the Gmat namespace.
 * @param msg The message, possibly including markers for variable argument 
 *            substitution.
 * @param ... The optional list of parameters that are inserted into the msg 
 *            string.
 */
//------------------------------------------------------------------------------
void ConsoleMessageReceiver::PopupMessage(Gmat::MessageType msgType, 
      const char *msg, ...)
{
   short    ret;
   short    size;
   va_list  marker;
   char     *msgBuffer;
   
   // msg is vsprintf format
   // actual max message length is MAX_MESSAGE_LENGTH
   size = strlen(msg) + MAX_MESSAGE_LENGTH;
   
   // Note: 'new' throws an exception of type std::bad_alloc on failure.
   // (Note that if an exception is thrown, no memory will have been
   // allocated, so there will be no memory leak.)
   msgBuffer = new char[size];

   // For older C++ compilers, duplicate that behavior by hand.
   if (!msgBuffer)
      throw std::bad_alloc();

   // Process the message
   va_start(marker, msg);
   ret = vsprintf(msgBuffer, msg, marker);
   va_end(marker);

   // if no EOL then append it
   if (msgBuffer[strlen(msgBuffer)-1] != '\n')
       msgBuffer[strlen(msgBuffer)] = '\n';

   LogMessage(wxString(msgBuffer) + _T("\n"));
   delete [] msgBuffer;
}

//------------------------------------------------------------------------------
// wxString GetLogFileName()
//------------------------------------------------------------------------------
/**
 * Retrieves the fully qualified name of the log file.
 * 
 * @return The name of the log file, including path information.
 */
//------------------------------------------------------------------------------
wxString ConsoleMessageReceiver::GetLogFileName()
{
   FileManager *fm = FileManager::Instance();
   wxString fname;
   try
   {
      if (logFileName == _T(""))
      {
         fname = fm->GetFullPathname(_T("LOG_FILE"));
      }
      else
      {
         wxString outputPath = fm->GetPathname(FileManager::LOG_FILE);
         
         // add output path if there is no path
         if (logFileName.find(_T("/")) == logFileName.npos &&
             logFileName.find("\\") == logFileName.npos)
         {
            fname = outputPath + logFileName;
         }
      }
   }
   catch (BaseException &e)
   {
      ShowMessage
         (_T("**** ERROR **** ") + e.GetFullMessage() + 
          _T("So setting log file name to GmatLog.txt"));
      
      fname = _T("GmatLog.txt");
   }
   
   return fname;
}

//------------------------------------------------------------------------------
// void SetLogEnable(bool flag)
//------------------------------------------------------------------------------
/**
 * Turns logging on or off.
 * 
 * @param flag The new logging state -- true enables logging, and false disables 
 *             it.  The logging state is idempotent.
 */
//------------------------------------------------------------------------------
void ConsoleMessageReceiver::SetLogEnable(bool flag)
{
   logEnabled = flag;
}

//------------------------------------------------------------------------------
// void SetLogPath(const wxString &pathname, bool append = false)
//------------------------------------------------------------------------------
/*
 * Sets log file path with keeping log file name as is.
 *
 * @param  pathname  log file path name, such as _T("/newpath/test1/")
 * @param  append  true if appending log message (false)
 */
//------------------------------------------------------------------------------
void ConsoleMessageReceiver::SetLogPath(const wxString &pathname, 
      bool append)
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
      ShowMessage
         (_T("**** ERROR **** ") + e.GetFullMessage() + 
          _T("So setting log file name to GmatLog.txt"));
      
      fname = _T("GmatLog.txt");
   }
   
   OpenLogFile(fname, append);
}

//------------------------------------------------------------------------------
// void SetLogFile(const wxString &filename)
//------------------------------------------------------------------------------
/*
 * Calls OpenLogFile() to set the log file path and name and then open the log.
 *
 * @param  filename  log file name, such as _T("/newpath/test1/GmatLog.txt")
 */
//------------------------------------------------------------------------------
void ConsoleMessageReceiver::SetLogFile(const wxString &filename)
{
   OpenLogFile(filename);
}

//------------------------------------------------------------------------------
// void OpenLogFile(const wxString &filename, bool append)
//------------------------------------------------------------------------------
/*
 * Sets the log file name and opens the log file.
 *
 * @param filename  log file name, such as _T("/newpath/test1/GmatLog.txt")
 * @param append  true if appending log message
 */
//------------------------------------------------------------------------------
void ConsoleMessageReceiver::OpenLogFile(const wxString &filename, 
      bool append)
{
   logFileName = filename;
   
   if (logFile)
      fclose(logFile);
   
   if (append)
      logFile = fopen(logFileName.c_str(), _T("a"));
   else
      logFile = fopen(logFileName.c_str(), _T("w"));
   
   if (!logFile)
   {
      std::cout << _T("**** ERROR **** Error setting the log file to ") 
                << logFileName
                << _T("\nSo setting it to \"GmatLog.txt\" in the ")
                << _T("executable directory\n");
      
      logFileName = _T("GmatLog.txt");
      
      if (append)
         logFile = fopen(logFileName.c_str(), _T("a"));
      else
         logFile = fopen(logFileName.c_str(), _T("w"));
   }
   
   if (logFile)
   {
      fprintf(logFile, _T("GMAT Build Date: %s %s\n\n"),  __DATE__, __TIME__);
      fprintf(logFile, _T("GuiMessageReceiver::SetLogFile() Log file set to %s\n"),
              logFileName.c_str());
      
      if (append)
         fprintf(logFile, _T("The log file mode is append\n"));
      else
         fprintf(logFile, _T("The log file mode is create\n"));
      
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
void ConsoleMessageReceiver::CloseLogFile()
{
   if (logFile)
      fclose(logFile);
   
   logFile = NULL;
   logFileSet = false;
}


//------------------------------------------------------------------------------
//  void LogMessage(const wxString &msg)
//------------------------------------------------------------------------------
/**
 * Logs the message to the log file.
 * 
 * This method displays the input message on the console and writes it to the 
 * log file.
 * 
 * @param msg The message.
 */
//------------------------------------------------------------------------------
void ConsoleMessageReceiver::LogMessage(const wxString &msg)
{
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
      //wxString tempStr = GmatStringUtil::Replace(msg, _T("%"), _T("%%"));
      //fprintf(logFile, _T("%s"), tempStr.c_str());
      fprintf(logFile, _T("%s"), msg.c_str());
      fflush(logFile);
   }
}

//------------------------------------------------------------------------------
//  void LogMessage(const char *msg, ...)
//------------------------------------------------------------------------------
/**
 * Logs a variable argument formatted message to the log file.
 * 
 * This method displays the input message on the console and writes it to the 
 * log file.
 * 
 * @param msg The message, possibly including markers for variable argument 
 *            substitution.
 * @param ... The optional list of parameters that are inserted into the msg 
 *            string.
 */
//------------------------------------------------------------------------------
void ConsoleMessageReceiver::LogMessage(const char *msg, ...)
{
   short    ret;
   short    size;
   va_list  marker;
   char     *msgBuffer;
   
   // msg is vsprintf format
   // actual max message length is MAX_MESSAGE_LENGTH
   size = strlen(msg) + MAX_MESSAGE_LENGTH;
   
   // Note: 'new' throws an exception of type std::bad_alloc on failure.
   // (Note that if an exception is thrown, no memory will have been
   // allocated, so there will be no memory leak.)
   msgBuffer = new char[size];

   // For older C++ compilers, duplicate that behavior by hand.
   if (!msgBuffer)
      throw std::bad_alloc();

   // Process the message
   va_start(marker, msg);
   ret = vsprintf(msgBuffer, msg, marker);
   va_end(marker);

   LogMessage(wxString(msgBuffer));
   delete [] msgBuffer;
}

//------------------------------------------------------------------------------
//  void ClearMessage()
//------------------------------------------------------------------------------
/**
 * Clears the message window.  This console version does nothing.
 */  
//------------------------------------------------------------------------------
void ConsoleMessageReceiver::ClearMessage()
{
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
wxString ConsoleMessageReceiver::GetMessage()
{
   return _T("");
}

//------------------------------------------------------------------------------
// void PutMessage(const wxString &msg)
//------------------------------------------------------------------------------
/**
 * Push the message into queue
 */
//------------------------------------------------------------------------------
void ConsoleMessageReceiver::PutMessage(const wxString &msg)
{
   ; // do nothing here
}

//------------------------------------------------------------------------------
// void ClearMessageQueue()
//------------------------------------------------------------------------------
/**
 * Tells the MessageReceiver to clear the message queue.
 */
//------------------------------------------------------------------------------
void ConsoleMessageReceiver::ClearMessageQueue()
{
   ; // do nothing here
}


//---------------------------------
//  private methods
//---------------------------------

//------------------------------------------------------------------------------
// ConsoleMessageReceiver()
//------------------------------------------------------------------------------
/**
 * Constructor, called from the Instance method to create the singleton.
 */
//------------------------------------------------------------------------------
ConsoleMessageReceiver::ConsoleMessageReceiver() :
   MAX_MESSAGE_LENGTH      (10000),
   logFile                 (NULL),
   logEnabled              (false),
   logFileSet              (false)
{
   messageQueue.push(_T("ConsoleMessageReceiver: Starting GMAT ..."));
}

//------------------------------------------------------------------------------
// ~ConsoleMessageReceiver()
//------------------------------------------------------------------------------
/**
 * Class destructor.
 */
//------------------------------------------------------------------------------
ConsoleMessageReceiver::~ConsoleMessageReceiver()
{
}
