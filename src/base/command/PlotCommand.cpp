//$Id$
//------------------------------------------------------------------------------
//                                PlotCommand
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Linda Jun
// Created: 2011/09/08
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
/**
 * Class implementation for the PlotCommand
 */
//------------------------------------------------------------------------------


#include "PlotCommand.hpp"
#include "MessageInterface.hpp"
#include "StringUtil.hpp"

//#define DEBUG_PLOT_IA
//#define DEBUG_PLOT_INIT


//---------------------------------
// static data
//---------------------------------
const wxString
PlotCommand::PARAMETER_TEXT[PlotCommandParamCount - GmatCommandParamCount] =
{
   wxT("Subscriber"),
};

const Gmat::ParameterType
PlotCommand::PARAMETER_TYPE[PlotCommandParamCount - GmatCommandParamCount] =
{
   Gmat::OBJECTARRAY_TYPE,   // "Subscriber",
};


//------------------------------------------------------------------------------
// ~PlotCommand()
//------------------------------------------------------------------------------
/**
 * Constructor
 */
//------------------------------------------------------------------------------
PlotCommand::PlotCommand(const wxString &plotTypeName) :
   GmatCommand(plotTypeName)
{
   plotNameList.clear();
   thePlotList.clear();
}


//------------------------------------------------------------------------------
// ~PlotCommand()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
PlotCommand::~PlotCommand()
{
   plotNameList.clear();
   thePlotList.clear();
}


//------------------------------------------------------------------------------
// PlotCommand(const PlotCommand &c)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 * 
 * @param <c> The instance that gets copied.
 */
//------------------------------------------------------------------------------
PlotCommand::PlotCommand(const PlotCommand &c) :
   GmatCommand    (c),
   plotNameList   (c.plotNameList),
   thePlotList    (c.thePlotList)
{
}

//------------------------------------------------------------------------------
// PlotCommand& operator=(const PlotCommand &c)
//------------------------------------------------------------------------------
/**
 * Assignment operator
 * 
 * @param <c> The command that gets copied.
 * 
 * @return A reference to this instance.
 */
//------------------------------------------------------------------------------
PlotCommand& PlotCommand::operator=(const PlotCommand &c)
{
   if (&c != this)
   {
      GmatCommand::operator=(c);
      plotNameList = c.plotNameList;
      thePlotList.clear();
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by the Achieve.
 *
 * @return the list of object types.
 * 
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& PlotCommand::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::SUBSCRIBER);
   return refObjectTypes;
}


//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Accesses arrays of names for referenced objects.
 * 
 * @param type Type of object requested.
 * 
 * @return the StringArray containing the referenced object names.
 */
//------------------------------------------------------------------------------
const StringArray& PlotCommand::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   // There are only subscribers, so ignore object type
   return plotNameList;
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
bool PlotCommand::RenameRefObject(const Gmat::ObjectType type,
                                const wxString &oldName,
                                const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("PlotCommand::RenameConfiguredItem() type=%s, oldName=%s, newName=%s\n"),
       GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif
   
   if (type != Gmat::SUBSCRIBER)
      return true;
   
   for (unsigned int i=0; i<plotNameList.size(); i++)
   {
      if (plotNameList[i] == oldName)
         plotNameList[i] = newName;
   }

   return true;
}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
wxString PlotCommand::GetParameterText(const Integer id) const
{
   if (id >= GmatCommandParamCount && id < PlotCommandParamCount)
      return PARAMETER_TEXT[id - GmatCommandParamCount];
   else
      return GmatCommand::GetParameterText(id);
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
Integer PlotCommand::GetParameterID(const wxString &str) const
{
   for (Integer i = GmatCommandParamCount; i < PlotCommandParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatCommandParamCount])
         return i;
   }
   
   return GmatCommand::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
Gmat::ParameterType PlotCommand::GetParameterType(const Integer id) const
{
    if (id >= GmatCommandParamCount && id < PlotCommandParamCount)
        return PARAMETER_TYPE[id - GmatCommandParamCount];
    else
       return GmatCommand::GetParameterType(id);
}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
wxString PlotCommand::GetParameterTypeString(const Integer id) const
{
   if (id >= GmatCommandParamCount && id < PlotCommandParamCount)
      return PlotCommand::PARAM_TYPE_STRING[GetParameterType(id)];
   else
      return GmatCommand::GetParameterTypeString(id);
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
/**
 * Adds value to the list if value not found.
 */
//------------------------------------------------------------------------------
bool PlotCommand::SetStringParameter(const Integer id, const wxString &value)
{
   #ifdef DEBUG_PLOT_SET
   MessageInterface::ShowMessage
      (wxT("PlotCommand::SetStringParameter() id=%d, value=%s\n"), id, value.c_str());
   #endif
   
   if (value == wxT(""))
      return false;
   
   if (id == SUBSCRIBER)
   {
      if (find(plotNameList.begin(), plotNameList.end(), value) == plotNameList.end())
         plotNameList.push_back(value);
      
      return true;
   }
   
   return GmatCommand::SetStringParameter(id, value);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
wxString PlotCommand::GetStringParameter(const Integer id,
                                            const Integer index) const
{
   if (id == SUBSCRIBER)
   {
      if (index < 0 || index >= (Integer) plotNameList.size())
         throw CommandException(
                  wxT("Index out-of-range for subscriber names list for PlotCommand command.\n"));
      return plotNameList.at(index); 
   }
   return GmatCommand::GetStringParameter(id, index);
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value,
//                         const Integer index)
//------------------------------------------------------------------------------
bool PlotCommand::SetStringParameter(const Integer id, 
                                     const wxString &value,
                                     const Integer index)
{
   if (value == wxT(""))
      return false;
   
   if (id == SUBSCRIBER)
   {
      if (index < 0 || index > (Integer) plotNameList.size())
         throw CommandException(
                  wxT("Index out-of-range for subscriber names list for PlotCommand command.\n"));
      else
      {
         if (index == (Integer) plotNameList.size())  plotNameList.push_back(value);
         else                   plotNameList.at(index) = value;
         
         //@todo Do we need this code? Commented out for now (LOJ: 2009.06.01)
         //if (publisher == NULL)
         //   publisher = Publisher::Instance();         
         //streamID = publisher->RegisterPublishedData(this, plotNameList, plotNameList);
      }
      
      return true;
   }
   return GmatCommand::SetStringParameter(id, value, index);
}


//------------------------------------------------------------------------------
// const wxString& GetGeneratingString(Gmat::WriteMode mode, ...)
//------------------------------------------------------------------------------
const wxString& PlotCommand::GetGeneratingString(Gmat::WriteMode mode,
                                                    const wxString &prefix,
                                                    const wxString &useName)
{
   #ifdef DEBUG_GEN_STR
   MessageInterface::ShowMessage(wxT("PlotCommand::GetGeneratingString() entered\n"));
   #endif
   
   generatingString = prefix + typeName + wxT(" ");
   int plotCount = plotNameList.size();
   for (int i = 0; i < plotCount; i++)
   {
      if (i == plotCount-1)
         generatingString += plotNameList[i] + wxT(";");
      else
         generatingString += plotNameList[i] + wxT(" ");
   }
   
   return GmatCommand::GetGeneratingString(mode, prefix, useName);
}


//------------------------------------------------------------------------------
// bool InterpretAction()
//------------------------------------------------------------------------------
bool PlotCommand::InterpretAction()
{
   #ifdef DEBUG_PLOT_IA
   MessageInterface::ShowMessage(wxT("PlotCommand::InterpretAction() entered\n"));
   #endif
   
   plotNameList.clear();
   thePlotList.clear();
   
   wxString genStr = generatingString;
   
   // Trim it first
   genStr = GmatStringUtil::Trim(genStr, GmatStringUtil::BOTH, true, true);
   #ifdef DEBUG_PLOT_IA
   MessageInterface::ShowMessage(wxT("   genStr = '%s'\n"), genStr.c_str());
   #endif
   
   Integer loc = genStr.find(typeName, 0) + typeName.size();
   const char *str = genStr.char_str();
   while (str[loc] == ' ')
      ++loc;

   // this command, for compatability with MATLAB, should not have
   // parentheses (except to indicate array elements), brackets, or braces
   if (!GmatStringUtil::HasNoBrackets(genStr, false))
   {
      wxString msg = 
         wxT("The PlotCommand command is not allowed to contain brackets, braces, or ")
         wxT("parentheses");
      throw CommandException(msg);
   }

   // Find the Subscriber list
   wxString sub = genStr.substr(loc, genStr.size()-loc);
   StringArray parts = GmatStringUtil::SeparateBy(sub,wxT(" "), false);
   Integer partsSz = (Integer) parts.size();
   #ifdef DEBUG_PLOT_IA
      MessageInterface::ShowMessage(wxT("In PlotCommand::InterpretAction, parts = \n"));
      for (Integer jj = 0; jj < partsSz; jj++)
         MessageInterface::ShowMessage(wxT("   %s\n"), parts.at(jj).c_str());
   #endif
   if (partsSz < 1) // 'PlotCommand' already found
      throw CommandException(wxT("Missing field in PlotCommand command"));
   for (Integer ii = 0; ii < partsSz; ii++)
      plotNameList.push_back(parts.at(ii));
   
   #ifdef DEBUG_PLOT_IA
      MessageInterface::ShowMessage(wxT("Plots to be %sed:\n"), typeName.c_str());
      for (unsigned int ii = 0; ii < plotNameList.size(); ii++)
         MessageInterface::ShowMessage(wxT("   %s\n"), (plotNameList.at(ii)).c_str());
   #endif
      
   #ifdef DEBUG_PLOT_IA
   MessageInterface::ShowMessage(wxT("PlotCommand::InterpretAction() returning true\n"));
   #endif
   return true;
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 * Method that initializes the internal data structures.
 * 
 * @return true if initialization succeeds.
 */
//------------------------------------------------------------------------------
bool PlotCommand::Initialize()
{
   #ifdef DEBUG_PLOT_INIT
   MessageInterface::ShowMessage
      (wxT("PlotCommand::Initialize() entered for %s\n"), typeName.c_str());
   #endif
   
   GmatCommand::Initialize();
   
   if (plotNameList.size() == 0)
   {
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** %s command nas no associated plots; command has no effect.\n"),
          typeName.c_str());
      return false;      
   }
   
   #ifdef DEBUG_PLOT_INIT
   MessageInterface::ShowMessage(wxT("PlotCommand::Initialize() returning true\n"));
   #endif
   return true;
}
