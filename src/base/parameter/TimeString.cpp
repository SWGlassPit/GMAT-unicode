//$Id: TimeString.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                TimeString
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
// Created: 2006/03/09
//
/**
 * Implements TimeString class which provides base class for time realated String
 * Parameters
 */
//------------------------------------------------------------------------------

#include "TimeString.hpp"
#include "ParameterException.hpp"
#include "StringUtil.hpp"               // for ParseParameter()
#include "MessageInterface.hpp"

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// TimeString(const wxString &name, const wxString &typeStr, 
//          GmatBase *obj, const wxString &desc, const wxString &unit)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <typeStr> type of the parameter
 * @param <obj> reference object pointer
 * @param <desc> description of the parameter
 * @param <unit> unit of the parameter
 */
//------------------------------------------------------------------------------
TimeString::TimeString(const wxString &name, const wxString &typeStr, 
                       GmatBase *obj, const wxString &desc,
                       const wxString &unit)
   : StringVar(name, typeStr, GmatParam::SYSTEM_PARAM, obj, desc, unit,
               GmatParam::NO_DEP, Gmat::SPACECRAFT, true),
     TimeData(name)
{
   wxString type, ownerName, depObj;
   GmatStringUtil::ParseParameter(name, type, ownerName, depObj);
   mOwnerName = ownerName;
   mExpr = name;
   AddRefObject(obj);
}


//------------------------------------------------------------------------------
// TimeString(const TimeString &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TimeString::TimeString(const TimeString &copy)
   : StringVar(copy), TimeData(copy)
{
}


//------------------------------------------------------------------------------
// TimeString& operator=(const TimeString &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TimeString& TimeString::operator=(const TimeString &right)
{
   if (this != &right)
   {
      StringVar::operator=(right);
      TimeData::operator=(right);
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// ~TimeString()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
TimeString::~TimeString()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// const virtual wxString& EvaluateString()
//------------------------------------------------------------------------------
/**
 * @return newly evaluated value of parameter
 */
//------------------------------------------------------------------------------
const wxString& TimeString::EvaluateString()
{
   Evaluate();
   return mStringValue;
}


//------------------------------------------------------------------------------
// virtual Integer GetNumRefObjects() const
//------------------------------------------------------------------------------
/**
 * @return number of reference objects set.
 */
//------------------------------------------------------------------------------
Integer TimeString::GetNumRefObjects() const
{
   return TimeData::GetNumRefObjects();
}


//------------------------------------------------------------------------------
// virtual bool AddRefObject(GmatBase *obj, bool replaceName = false)
//------------------------------------------------------------------------------
/**
 * Adds reference object.
 *
 * @param <obj> object pointer
 *
 * @return true if the object has been added.
 */
//------------------------------------------------------------------------------
bool TimeString::AddRefObject(GmatBase *obj, bool replaceName)
{
   if (obj != NULL)
      return TimeData::AddRefObject(obj->GetType(), obj->GetName(), obj,
                                    replaceName);
   else
      return false;
}


//------------------------------------------------------------------------------
// virtual bool Validate()
//------------------------------------------------------------------------------
/**
 * Validates reference objects.
 *
 * @return true if all objects are set; false otherwise
 */
//------------------------------------------------------------------------------
bool TimeString::Validate()
{
   return ValidateRefObjects(this);
}


//------------------------------------------------------------------------------
// virtual bool Initialize()
//------------------------------------------------------------------------------
/**
 * Initializes reference objects.
 *
 * @return true if all objects are set; false otherwise
 */
//------------------------------------------------------------------------------
bool TimeString::Initialize()
{
   mInitialEpoch = 0.0;
   mIsInitialEpochSet = false;

   try
   {
      InitializeRefObjects();
   }
   catch (BaseException &e)
   {
      #if DEBUG_TIMESTRING
      MessageInterface::ShowMessage
         (wxT("TimeString::Initialize() Fail to initialize Parameter '%s'\n"),
          this->GetName().c_str());
      #endif
      
      throw ParameterException
         (wxT("WARNING:  ") + e.GetFullMessage() + wxT(" in ") + GetName() + wxT("\n"));
   }
   
   return true;
}


//-------------------------------------
// Methods inherited from GmatBase
//-------------------------------------

//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
bool TimeString::RenameRefObject(const Gmat::ObjectType type,
                                 const wxString &oldName,
                                 const wxString &newName)
{
   return TimeData::RenameRefObject(type, oldName, newName);
}


//------------------------------------------------------------------------------
// virtual wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
/**
 * Calls TimeData to get reference object name for given type.
 *
 * @return reference object name.
 */
//------------------------------------------------------------------------------
wxString TimeString::GetRefObjectName(const Gmat::ObjectType type) const
{
   return TimeData::GetRefObjectName(type);
}


//------------------------------------------------------------------------------
// virtual const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Retrieves reference object name array for given type. It will return all
 * object names if type is Gmat::UNKNOWN_NAME.
 *
 * @param <type> object type
 * @return reference object name.
 */
//------------------------------------------------------------------------------
const StringArray& TimeString::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   return TimeData::GetRefObjectNameArray(type);
}


//------------------------------------------------------------------------------
// virtual bool SetRefObjectName(const Gmat::ObjectType type,
//                               const wxString &name)
//------------------------------------------------------------------------------
/**
 * Sets reference object name to given object type.
 *
 * @param <type> object type
 * @param <name> object name
 *
 */
//------------------------------------------------------------------------------
bool TimeString::SetRefObjectName(const Gmat::ObjectType type,
                                 const wxString &name)
{
   return TimeData::SetRefObjectName(type, name);
}


//------------------------------------------------------------------------------
// virtual GmatBase* GetRefObject(const Gmat::ObjectType type,
//                                const wxString &name)
//------------------------------------------------------------------------------
/**
 * Calls TimeData to get object pointer of given type and name
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return reference object pointer for given object type and name
 */
//------------------------------------------------------------------------------
GmatBase* TimeString::GetRefObject(const Gmat::ObjectType type,
                                  const wxString &name)
{
   return TimeData::GetRefObject(type, name);
}


//------------------------------------------------------------------------------
// virtual bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                           const wxString &name = wxT(""))
//------------------------------------------------------------------------------
/**
 * Calls TimeData to set reference object pointer to given type and name.
 *
 * @param <obj>  reference object pointer
 * @param <type> object type
 * @param <name> object name
 *
 * @return true if object pointer is successfully set.
 *
 */
//------------------------------------------------------------------------------
bool TimeString::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                              const wxString &name)
{
   return TimeData::SetRefObject(obj, type, name);
}


//------------------------------------------------------------------------------
// const wxString& GetGeneratingString(...)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
const wxString& TimeString::GetGeneratingString(Gmat::WriteMode mode,
                                                   const wxString &prefix,
                                                   const wxString &useName)
{
   #ifdef DEBUG_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("TimeString::GetGeneratingString() this=<%p>'%s' entered, mode=%d, prefix='%s', ")
       wxT("useName='%s'\n"), this, GetName().c_str(), mode, prefix.c_str(), useName.c_str());
   MessageInterface::ShowMessage
      (wxT("   mExpr='%s', mDepObjectName='%s'\n"), mExpr.c_str(), mDepObjectName.c_str());
   #endif

   // We want to skip StringVar::GetGeneratingString() since it is handled specially
   // for String
   return Parameter::GetGeneratingString(mode, prefix, useName);
}

