//$Id: MessageWindow.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  MessageWindow
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
// Created: 2003/11/6
//
/**
 * Defines operation of MessageWindow class.
 */
//------------------------------------------------------------------------------
#include <iomanip>
#include "MessageWindow.hpp"
#include "MessageInterface.hpp" // for ShowMessage()

//---------------------------------
// static data
//---------------------------------
const wxString
MessageWindow::PARAMETER_TEXT[MessageWindowParamCount - SubscriberParamCount] =
{
   wxT("Precision")
}; 

const Gmat::ParameterType
MessageWindow::PARAMETER_TYPE[MessageWindowParamCount - SubscriberParamCount] =
{
   Gmat::INTEGER_TYPE,
};

//------------------------------------------------------------------------------
// MessageWindow(const MessageWindow &mw)
//------------------------------------------------------------------------------
MessageWindow::MessageWindow(const MessageWindow &mw)
   : Subscriber (mw),
     precision  (mw.precision)
{
   // GmatBase data
   //parameterCount = MessageWindowParamCount;
}

//------------------------------------------------------------------------------
// MessageWindow(const wxString &name)
//------------------------------------------------------------------------------
MessageWindow::MessageWindow(const wxString &name)
   : Subscriber (wxT("MessageWindow"), name),
     precision  (10)
{
   // GmatBase data
   parameterCount = MessageWindowParamCount;
}

//------------------------------------------------------------------------------
// ~MessageWindow(void)
//------------------------------------------------------------------------------
MessageWindow::~MessageWindow(void)
{
}

//------------------------------------------------------------------------------
// bool MessageWindow::Distribute(int len)
//------------------------------------------------------------------------------
bool MessageWindow::Distribute(int len)
{
   dstream = wxT("");
   
   if (len == 0)
      //dstream << data;
      return false;
   else
      for (int i = 0; i < len; ++i)
         dstream << data[i];
   
   MessageInterface::ShowMessage(dstream);
   return true;
}

//------------------------------------------------------------------------------
// bool Distribute(const Real * dat, Integer len)
//------------------------------------------------------------------------------
bool MessageWindow::Distribute(const Real * dat, Integer len)
{
    dstream = wxT("");
    
    if (len == 0)
    {
        return false;
    }
    else
    {
        for (int i = 0; i < len-1; ++i)
            dstream << dat[i] << wxT("  ");
        
        dstream << dat[len-1] << wxT("\n");
    }

    MessageInterface::ShowMessage(dstream);
    //dstream.str(wxT(""));
    //dstream << wxT("line # ") << MessageInterface::GetNumberOfMessageLines() << wxT("\n");
    //MessageInterface::ShowMessage(dstream.str());
    return true;
}

//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the MessageWindow.
 *
 * @return clone of the MessageWindow.
 *
 */
//------------------------------------------------------------------------------
GmatBase* MessageWindow::Clone() const
{
   return (new MessageWindow(*this));
}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
wxString MessageWindow::GetParameterText(const Integer id) const
{
   if (id >= SubscriberParamCount && id < MessageWindowParamCount)
      return PARAMETER_TEXT[id - MessageWindowParamCount];
   else
      return Subscriber::GetParameterText(id);
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
Integer MessageWindow::GetParameterID(const wxString &str) const
{
   for (int i=SubscriberParamCount; i<MessageWindowParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - SubscriberParamCount])
         return i;
   }
   
   return Subscriber::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
Gmat::ParameterType MessageWindow::GetParameterType(const Integer id) const
{
   if (id >= SubscriberParamCount && id < MessageWindowParamCount)
      return PARAMETER_TYPE[id - SubscriberParamCount];
   else
      return Subscriber::GetParameterType(id);
}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
wxString MessageWindow::GetParameterTypeString(const Integer id) const
{
   if (id >= SubscriberParamCount && id < MessageWindowParamCount)
      return GmatBase::PARAM_TYPE_STRING[GetParameterType(id - SubscriberParamCount)];
   else
      return Subscriber::GetParameterTypeString(id);
}


//------------------------------------------------------------------------------
// Integer GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
Integer MessageWindow::GetIntegerParameter(const Integer id) const
{
    switch (id)
    {
    case PRECISION:
        return precision;
    default:
        return Subscriber::GetIntegerParameter(id);
    }
}


//------------------------------------------------------------------------------
// Integer SetIntegerParameter(const Integer id, const Integer value)
//------------------------------------------------------------------------------
Integer MessageWindow::SetIntegerParameter(const Integer id, const Integer value)
{
    switch (id)
    {
    case PRECISION:
        if (value > 0)
        {
            precision = value;
        }
        return precision;
    default:
        return Subscriber::GetIntegerParameter(id);
    }
}
