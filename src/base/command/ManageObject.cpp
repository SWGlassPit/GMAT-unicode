//$Id: ManageObject.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                 ManageObject
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Wendy C. Shoan
// Created: 2008.03.12
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CCA54C
//
/**
 * Class implementation for the ManageObject command
 */
//------------------------------------------------------------------------------


#include "ManageObject.hpp"
#include "MessageInterface.hpp"
#include "CommandException.hpp"
#include "Array.hpp"

//#define DEBUG_MANAGE_OBJECT

//---------------------------------
// static data
//---------------------------------
const wxString
ManageObject::PARAMETER_TEXT[ManageObjectParamCount - GmatCommandParamCount] =
{
   wxT("ObjectNames"),
};

const Gmat::ParameterType
ManageObject::PARAMETER_TYPE[ManageObjectParamCount - GmatCommandParamCount] =
{
   Gmat::STRINGARRAY_TYPE,   //"ObjectNames",
};

//------------------------------------------------------------------------------
// ManageObject()
//------------------------------------------------------------------------------
/**
 * Constructor
 */
//------------------------------------------------------------------------------
ManageObject::ManageObject(const wxString &typeStr) :
   GmatCommand(typeStr)
{
}


//------------------------------------------------------------------------------
// ~ManageObject()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
ManageObject::~ManageObject()
{
}


//------------------------------------------------------------------------------
// ManageObject(const ManageObject &mo)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 * 
 * @param <mo> The instance that gets copied.
 */
//------------------------------------------------------------------------------
ManageObject::ManageObject(const ManageObject &mo) :
   GmatCommand(mo),
   objectNames   (mo.objectNames)      
{
}


//------------------------------------------------------------------------------
// ManageObject& operator=(const ManageObject &mo)
//------------------------------------------------------------------------------
/**
 * Assignment operator
 * 
 * @param <mo> The command that gets copied.
 * 
 * @return A reference to this instance.
 */
//------------------------------------------------------------------------------
ManageObject& ManageObject::operator=(const ManageObject &mo)
{
   if (&mo != this)
   {
      GmatCommand::operator=(mo);
      objectNames = mo.objectNames;
   }
   
   return *this;
}

// Parameter access methods - overridden from GmatBase
wxString ManageObject::GetParameterText(const Integer id) const
{
   if (id >= GmatCommandParamCount && id < ManageObjectParamCount)
      return PARAMETER_TEXT[id - GmatCommandParamCount];
   return GmatCommand::GetParameterText(id);
}

Integer ManageObject::GetParameterID(const wxString &str) const
{
   for (Integer i = GmatCommandParamCount; i < ManageObjectParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatCommandParamCount])
         return i;
   }
   
   return GmatCommand::GetParameterID(str);
}

Gmat::ParameterType ManageObject::GetParameterType(const Integer id) const
{
   if (id >= GmatCommandParamCount && id < ManageObjectParamCount)
      return PARAMETER_TYPE[id - GmatCommandParamCount];
      
   return GmatCommand::GetParameterType(id);
}

wxString ManageObject::GetParameterTypeString(const Integer id) const
{
   return GmatCommand::PARAM_TYPE_STRING[GetParameterType(id)];
}


wxString ManageObject::GetStringParameter(const Integer id) const
{
   return GmatCommand::GetStringParameter(id);
}

//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const Integer id, const Integer index)
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter ID and the index into the array.
 *
 * @param <id> ID for the requested parameter.
 * @param <index> index into the StringArray parameter.
 *
 * @exception <CommandException> thrown if value is out of range
 *
 * @return  string value at index 'index'.
 *
 */
//------------------------------------------------------------------------------
wxString ManageObject::GetStringParameter(const Integer id,
                                             const Integer index) const
{
   if (id == OBJECT_NAMES)
   {
      if ((index < 0) || (index >= ((Integer) objectNames.size())))
         throw CommandException(
               wxT("Index out of bounds when attempting to return object name\n"));
      return objectNames.at(index);
    }
   return GmatCommand::GetStringParameter(id, index);
}

//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const wxString &label, const Integer index)
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * string label and the index into the array.
 *
 * @param <label> label for the requested parameter.
 * @param <index> index into the StringArray parameter.
 *
 * @return  string value at index 'index'.
 *
 */
//------------------------------------------------------------------------------
wxString ManageObject::GetStringParameter(const wxString &label,
                                             const Integer index) const
{
   return GetStringParameter(GetParameterID(label), index);
}

//------------------------------------------------------------------------------
//  bool  SetStringParameter(const Integer id, const wxString value)
//------------------------------------------------------------------------------
/**
 * This method sets the string parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 * @param <value> string value for the requested parameter.
 *
 * @exception <CommandException> thrown if value is already in the list.
 *
 * @return  success flag.
 *
 */
//------------------------------------------------------------------------------
bool ManageObject::SetStringParameter(const Integer id, 
                                      const wxString &value)
{
   if (id == OBJECT_NAMES)
   {
      Integer sz = objectNames.size();
      for (Integer ii = 0; ii < sz; ii++)
         if (objectNames[ii] == value)
         {
            wxString ex = wxT("Attempting to add """);
            ex += value + wxT(" more than once to list of objects.\n");
            throw CommandException(ex);
         }
      objectNames.push_back(value);
      return true;
    }
   return GmatCommand::SetStringParameter(id, value);
}

//------------------------------------------------------------------------------
//  bool  SetStringParameter(const wxString &label, const wxString value)
//------------------------------------------------------------------------------
/**
 * This method sets the string parameter value, given the input
 * parameter label.
 *
 * @param <label> label for the requested parameter.
 * @param <value> string value for the requested parameter.
 *
 * @exception <CommandException> thrown if value is already in the list.
 *
 * @return  success flag.
 *
 */
//------------------------------------------------------------------------------
bool ManageObject::SetStringParameter(const wxString &label, 
                                      const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}

bool ManageObject::SetStringParameter(const Integer id, 
                                      const wxString &value,
                                      const Integer index)
{
   if (id == OBJECT_NAMES)
   {
      Integer sz = objectNames.size();
      if (index < 0)
         throw CommandException(
               wxT("Index of object name array out of bounds for ManageObject command.\n"));
      if (index > sz)
         throw CommandException(
               wxT("Missing elements in Object Name list for ManageObject command.\n"));
      // push it onto the end of the list
      if (index == sz) objectNames.push_back(value);
      // or replace one of the already-existing names
      else
         objectNames.at(index) = value;
      return true;
   }
   return GmatCommand::SetStringParameter(id, value, index);
}

bool ManageObject::SetStringParameter(const wxString &label, 
                                      const wxString &value,
                                      const Integer index)
{
   return SetStringParameter(GetParameterID(label),value,index);
}

//------------------------------------------------------------------------------
//  const StringArray&  GetStringArrayParameter(const Integer id)
//------------------------------------------------------------------------------
/**
 * This method returns the string array value, given the input
 * parameter ID .
 *
 * @param <id> ID for the requested parameter.
 *
  * @return  string array.
 *
 */
//------------------------------------------------------------------------------
const StringArray& 
ManageObject::GetStringArrayParameter(const Integer id) const
{
   if (id == OBJECT_NAMES)
   {
      return objectNames;
   }
   return GmatCommand::GetStringArrayParameter(id);
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
bool ManageObject::Initialize()
{
   #ifdef DEBUG_MANAGE_OBJECT
      MessageInterface::ShowMessage(wxT("ManageObject::Initialize() entered\n"));
   #endif
      
   GmatCommand::Initialize();
   Integer numNames = (Integer) objectNames.size();
   if (numNames <= 0)
   {
      wxString ex = wxT("No objects listed for ManageObject command.\n");
      throw CommandException(ex);
   }
   return true;
}


bool ManageObject::InsertIntoGOS(GmatBase *obj, const wxString &withName)
{
   #ifdef DEBUG_MANAGE_OBJECT
      MessageInterface::ShowMessage(wxT("Entering InsertIntoGOS, with obj = <%p> and name = %s\n"),
            obj, withName.c_str());
   #endif
   if (!obj)
   {
      wxString errMsg = wxT("ManageObject::InsertIntoGOS() passed a NULL object\n");
      throw CommandException(errMsg);
   }
   GmatBase *mapObj;
   wxString ex;
   ////wxString objType = obj->GetTypeName();
   Gmat::ObjectType objType = obj->GetType();
   // if it is already in the GOS, make sure the types match
   if (globalObjectMap->find(withName) != globalObjectMap->end())
   {
      mapObj = (*globalObjectMap)[withName];
      if (!mapObj->IsOfType(objType))
      {
         ex = wxT("Object of name ") + withName;
         ex += wxT(", but of a different type, already exists in Global Object Store\n");
         throw CommandException(ex);
      }
      ////if (objType == "Array")
      if (objType == Gmat::PARAMETER && obj->GetTypeName() == wxT("Array"))
      {
         Integer r1, r2, c1, c2;
         ((Array*) mapObj)->GetSize(r1, c1);
         ((Array*) obj)->GetSize(r2, c2);
         if ((r1 != r2) || (c1 != c2))
         {
            ex = wxT("Array of name ") + withName;
            ex += wxT(", but with different dimensions already exists in Global Object Store\n");
            throw CommandException(ex);
         }
      }
      
      // This Warning is very annoying when GmatFunction is running in a loop,
      // so defined macro here (loj: 2008.10.08)
      #ifdef __SHOW_GOS_WARNING__
      ex = wxT("ManageObject::InsertIntoGOS() Cannot add more than ")
         wxT("one object with name \"");
      ex += withName + wxT("\" to the Global Object Store");
      MessageInterface::ShowMessage(wxT("*** WARNING *** ") + ex + wxT(", So ignored.\n"));
      // Let's just ignore for now to run TargetHohmannTransfer.gmf (loj: 2008.06.05) 
      //throw CommandException(ex);
      #endif
      
      // it is already in there, so we do not need to put this one in; clean it up
      #ifdef DEBUG_MANAGE_OBJECT
         MessageInterface::ShowMessage(wxT(" Create::object %s was already in object store ...\n"),
            withName.c_str());
         MessageInterface::ShowMessage(wxT("  pointer for obj = <%p> and pointer for mapObj = <%p>\n"),
            obj, mapObj);
      #endif
      if (mapObj != obj) 
      {
         #ifdef DEBUG_MANAGE_OBJECT
               MessageInterface::ShowMessage(wxT(" Create:: object is not the same, though\n"),
                     withName.c_str());
         #endif
         return false;
      }
   }
   else
      // put it into the GOS
      globalObjectMap->insert(std::make_pair(withName,obj));
   return true;
}
