//$Id: BaseException.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                             BaseException
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P.
//
// Author: Linda Jun (NASA/GSFC)
// Created: 2007/1/18
//
/**
 * Exception class used by the GmatBase base class.
 */
//------------------------------------------------------------------------------

#include "BaseException.hpp"
#include <stdarg.h>                // for va_start(), va_end()

#include <cstdlib>                 // Required for GCC 4.3
#include <string.h>                // Required for GCC 4.3
#include <stdio.h>                 // Fix for header rearrangement in gcc 4.4


//------------------------------------------------------------------------------
// wxString GetFullMessage() const 
//------------------------------------------------------------------------------
wxString BaseException::GetFullMessage() const 
{
   wxString preface = wxT("");

   if (msgType == Gmat::ERROR_)
      preface = wxT("**** ERROR **** ");
   if (msgType == Gmat::WARNING_)
      preface = wxT("**** WARNING **** ");
   return preface + theMessage + theDetails;
}

//------------------------------------------------------------------------------
// wxString GetDetails() const 
//------------------------------------------------------------------------------
wxString BaseException::GetDetails() const 
{
   return theDetails;
}

//------------------------------------------------------------------------------
// bool IsFatal() const
//------------------------------------------------------------------------------
bool BaseException::IsFatal() const
{
   return isFatal;
}

//------------------------------------------------------------------------------
// void BaseException::SetMessage(const wxString &message)  
//------------------------------------------------------------------------------
void BaseException::SetMessage(const wxString &message)  
{
   theMessage = message;
}

//------------------------------------------------------------------------------
// void SetDetails(const wxString &details)  
//------------------------------------------------------------------------------
//void BaseException::SetDetails(const wxString &details)  
//{
//   theDetails = details;
//}

//------------------------------------------------------------------------------
// void SetFatal(bool fatal)
//------------------------------------------------------------------------------
void BaseException::SetFatal(bool fatal)
{
   isFatal = fatal;
}

//------------------------------------------------------------------------------
// Gmat::MessageType GetMessageType()
//------------------------------------------------------------------------------
Gmat::MessageType BaseException::GetMessageType()
{
   return msgType;
}

//------------------------------------------------------------------------------
// void SetMessageType(Gmat::MessageType mt)
//------------------------------------------------------------------------------
void BaseException::SetMessageType(Gmat::MessageType mt)
{
   msgType = mt;
}

//------------------------------------------------------------------------------
// const BaseException& operator=(const wxString &newMessage) 
//------------------------------------------------------------------------------
const BaseException& BaseException::operator=(const wxString &newMessage) 
{
   theMessage = newMessage;
   return *this;
}

//------------------------------------------------------------------------------
// void SetDetails(const wxString &details, ...)
//------------------------------------------------------------------------------
/**
 * constructor taking variable arguments
 */
//------------------------------------------------------------------------------
void BaseException::SetDetails(const wxString &details, ...)
{
   short    ret;
   short    size;
   va_list  marker;
   wxString msgBuffer;


   va_start(marker, details);

//      Compiler quirks should be implementation details for wxString library
//      #ifdef _MSC_VER  // Microsoft Visual C++
//      // _vscprintf doesn't count terminating '\0'
//      int len = _vscprintf( details, marker ) + 1;
//     ret = vsprintf_s(msgBuffer, len, details, marker);
//      #else
   ret = msgBuffer.Printf(details, marker);
//      #endif

   va_end(marker);

   theDetails = wxString(msgBuffer);
}

//---------------------------------
// protected
//---------------------------------

//------------------------------------------------------------------------------
// BaseException(const wxString& message = wxT(""), const wxString &details = wxT("")) 
//------------------------------------------------------------------------------
BaseException::BaseException(const wxString& message,
      const wxString &details, Gmat::MessageType mt)
{
   theMessage = message;
   theDetails = details;
   msgType    = mt;
   isFatal = false;
}

//------------------------------------------------------------------------------
// BaseException(const BaseException& be) 
//------------------------------------------------------------------------------
BaseException::BaseException(const BaseException& be) 
{
   theMessage = be.theMessage;
   theDetails = be.theDetails;
   isFatal = be.isFatal;
}

//------------------------------------------------------------------------------
// virtual ~BaseException() 
//------------------------------------------------------------------------------
BaseException::~BaseException() 
{
}

//------------------------------------------------------------------------------
// const BaseException& operator=(const BaseException& be) 
//------------------------------------------------------------------------------
const BaseException& BaseException::operator=(const BaseException& be) 
{
   theMessage = be.theMessage;
   theDetails = be.theDetails;
   isFatal = be.isFatal;
   return *this;
}
