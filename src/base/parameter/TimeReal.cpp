//$Id: TimeReal.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                TimeReal
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
// Created: 2004/04/28
//
/**
 * Implements TimeReal class which provides base class for time realated Real
 * Parameters
 */
//------------------------------------------------------------------------------

#include "TimeReal.hpp"
#include "ParameterException.hpp"

//#define DEBUG_TIMEREAL 1

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// TimeReal(const wxString &name, const wxString &typeStr, 
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
TimeReal::TimeReal(const wxString &name, const wxString &typeStr, 
                   GmatBase *obj, const wxString &desc,
                   const wxString &unit)
   : RealVar(name, wxT(""), typeStr, GmatParam::SYSTEM_PARAM, obj, desc, unit,
             GmatParam::NO_DEP, Gmat::SPACECRAFT, true),
     TimeData(name)
{
   AddRefObject(obj);
}


//------------------------------------------------------------------------------
// TimeReal(const TimeReal &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TimeReal::TimeReal(const TimeReal &copy)
   : RealVar(copy), TimeData(copy)
{
}


//------------------------------------------------------------------------------
// TimeReal& operator=(const TimeReal &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TimeReal& TimeReal::operator=(const TimeReal &right)
{
   if (this != &right)
   {
      RealVar::operator=(right);
      TimeData::operator=(right);
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// ~TimeReal()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
TimeReal::~TimeReal()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual Real EvaluateReal()
//------------------------------------------------------------------------------
/**
 * @return newly evaluated value of parameter
 */
//------------------------------------------------------------------------------
Real TimeReal::EvaluateReal()
{
   Evaluate();
   return mRealValue;
}


//------------------------------------------------------------------------------
// virtual Integer GetNumRefObjects() const
//------------------------------------------------------------------------------
/**
 * @return number of reference objects set.
 */
//------------------------------------------------------------------------------
Integer TimeReal::GetNumRefObjects() const
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
bool TimeReal::AddRefObject(GmatBase *obj, bool replaceName)
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
bool TimeReal::Validate()
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
bool TimeReal::Initialize()
{
   mInitialEpoch = 0.0;
   mIsInitialEpochSet = false;

   try
   {
      InitializeRefObjects();
   }
   catch (BaseException &e)
   {
      #if DEBUG_TIMEREAL
      MessageInterface::ShowMessage
         (wxT("TimeReal::Initialize() Fail to initialize Parameter:%s\n"),
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
bool TimeReal::RenameRefObject(const Gmat::ObjectType type,
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
wxString TimeReal::GetRefObjectName(const Gmat::ObjectType type) const
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
const StringArray& TimeReal::GetRefObjectNameArray(const Gmat::ObjectType type)
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
bool TimeReal::SetRefObjectName(const Gmat::ObjectType type,
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
GmatBase* TimeReal::GetRefObject(const Gmat::ObjectType type,
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
bool TimeReal::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                             const wxString &name)
{
   return TimeData::SetRefObject(obj, type, name);
}

