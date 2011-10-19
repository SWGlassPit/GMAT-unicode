//$Id: OrbitReal.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                OrbitReal
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
// Created: 2004/03/29
//
/**
 * Implements OrbitReal class which provides base class for orbit realated Real
 * Parameters
 */
//------------------------------------------------------------------------------

#include "OrbitReal.hpp"
#include "ParameterException.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_ORBITREAL 1

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// OrbitReal(const wxString &name, const wxString &typeStr, 
//           GmatBase *obj, const wxString &desc,
//           const wxString &unit, GmatParam::DepObject depObj,
//           bool isSettable)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <typeStr> type of the parameter
 * @param <obj> reference object pointer
 * @param <desc> description of the parameter
 * @param <unit> unit of the parameter
 * @param <depObj> object which parameter is dependent on (COORD_SYS, ORIGIN, NO_DEP)
 * @param <isSettable> true if parameter is settable, false otherwise
 */
//------------------------------------------------------------------------------
OrbitReal::OrbitReal(const wxString &name, const wxString &typeStr, 
                     GmatBase *obj, const wxString &desc,
                     const wxString &unit, GmatParam::DepObject depObj,
                     bool isSettable)
   : RealVar(name, wxT(""), typeStr, GmatParam::SYSTEM_PARAM, obj, desc, unit,
             depObj, Gmat::SPACECRAFT, false, isSettable),
     OrbitData(name)
{
   mNeedCoordSystem = true;
   AddRefObject(obj);
}


//------------------------------------------------------------------------------
// OrbitReal(const OrbitReal &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
OrbitReal::OrbitReal(const OrbitReal &copy)
   : RealVar(copy), OrbitData(copy)
{
   #if DEBUG_ORBITREAL
   MessageInterface::ShowMessage
      (wxT("===> OrbitReal::OrbitReal() copy constructor called on %s\n"),
       instanceName.c_str());
   #endif
}


//------------------------------------------------------------------------------
// OrbitReal& operator=(const OrbitReal &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
OrbitReal& OrbitReal::operator=(const OrbitReal &right)
{
   if (this != &right)
   {
      RealVar::operator=(right);
      OrbitData::operator=(right);
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// ~OrbitReal()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
OrbitReal::~OrbitReal()
{
   //MessageInterface::ShowMessage(wxT("==> OrbitReal::~OrbitReal()\n"));
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
Real OrbitReal::EvaluateReal()
{
//    try
//    {
      Evaluate();
//    }
//    catch (BaseException &e)
//    {
//       throw ParameterException
//          (wxT("OrbitReal::EvaluateReal() for parameter ") +  this->GetTypeName() + wxT(":") +
//           instanceName + wxT("\n") + e.GetDetails());
//    }
   
   return mRealValue;
}


//------------------------------------------------------------------------------
// virtual CoordinateSystem* GetInternalCoordSystem()
//------------------------------------------------------------------------------
CoordinateSystem* OrbitReal::GetInternalCoordSystem()
{
   return OrbitData::GetInternalCoordSys();
}


//------------------------------------------------------------------------------
// virtual void SetSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 * Sets SolarSystem pointer.
 */
//------------------------------------------------------------------------------
void OrbitReal::SetSolarSystem(SolarSystem *ss)
{
   #if DEBUG_ORBITREAL
      MessageInterface::ShowMessage
         (wxT("OrbitReal::SetSolarSystem() ss=%s to %s\n"), ss->GetTypeName().c_str(),
          this->GetName().c_str());
   #endif
   
   if (OrbitData::GetRefObject(Gmat::SOLAR_SYSTEM, ss->GetName()) == NULL)
      OrbitData::AddRefObject(ss->GetType(), ss->GetName(), ss);
   else
      OrbitData::SetRefObject(ss, Gmat::SOLAR_SYSTEM, ss->GetName());
   
}


//------------------------------------------------------------------------------
// virtual void SetInternalCoordSystem(CoordinateSystem *cs)
//------------------------------------------------------------------------------
/**
 * Sets internal CoordinateSystem pointer. Assumes parameter data is in
 * this internal CoordinateSystem.
 */
//------------------------------------------------------------------------------
void OrbitReal::SetInternalCoordSystem(CoordinateSystem *cs)
{
   #if DEBUG_ORBITREAL
      MessageInterface::ShowMessage
         (wxT("OrbitReal::SetInternalCoordSystem() cs=%s to %s\n"), cs->GetTypeName().c_str(),
          this->GetName().c_str());
   #endif
   
   OrbitData::SetInternalCoordSys(cs);
}


//------------------------------------------------------------------------------
// virtual Integer GetNumRefObjects() const
//------------------------------------------------------------------------------
/**
 * @return number of reference objects set.
 */
//------------------------------------------------------------------------------
Integer OrbitReal::GetNumRefObjects() const
{
   return OrbitData::GetNumRefObjects();
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
bool OrbitReal::AddRefObject(GmatBase *obj, bool replaceName)
{
   if (obj != NULL)
   {
      #if DEBUG_ORBITREAL
      MessageInterface::ShowMessage
         (wxT("OrbitReal::AddRefObject() obj->GetName()=%s, type=%d\n"),
          obj->GetName().c_str(), obj->GetType());
      #endif
      
      if (obj->IsOfType(Gmat::CELESTIAL_BODY))
         return OrbitData::AddRefObject(Gmat::SPACE_POINT, obj->GetName(), obj,
                                        replaceName);
      else
         return OrbitData::AddRefObject(obj->GetType(), obj->GetName(), obj,
                                        replaceName);
   }
   
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
bool OrbitReal::Validate()
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
bool OrbitReal::Initialize()
{
   RealVar::Initialize();
   
   try
   {
      #if DEBUG_ORBITREAL
      MessageInterface::ShowMessage
         (wxT("===> OrbitReal::Initialize() calling InitializeRefObjects() on %s\n"),
          instanceName.c_str());
      #endif
      
      InitializeRefObjects();
   }
   catch(InvalidDependencyException &e)
   {
      #if DEBUG_ORBITREAL
      MessageInterface::ShowMessage
         (wxT("OrbitReal::Initialize() Fail to initialize Parameter:%s\n"),
          this->GetName().c_str());
      #endif
      
      throw ParameterException
         (wxT("Incorrect parameter dependency: ") + GetName() + wxT(".\n") +
          this->GetTypeName() + e.GetFullMessage() + wxT("\n"));
   }
   catch(BaseException &e)
   {
      #if DEBUG_ORBITREAL
      MessageInterface::ShowMessage
         (wxT("OrbitReal::Initialize() Fail to initialize Parameter:%s\n"),
          this->GetName().c_str());
      #endif
      
      throw ParameterException
         (e.GetFullMessage() + wxT(" in ") + GetName() + wxT("\n"));
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
bool OrbitReal::RenameRefObject(const Gmat::ObjectType type,
                                const wxString &oldName,
                                const wxString &newName)
{
   OrbitData::RenameRefObject(type, oldName, newName);
   RealVar::RenameRefObject(type, oldName, newName);
   
   return true;
}


//------------------------------------------------------------------------------
// virtual wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
/**
 * Calls OrbitData to get reference object name for given type.
 *
 * @return reference object name.
 */
//------------------------------------------------------------------------------
wxString OrbitReal::GetRefObjectName(const Gmat::ObjectType type) const
{
   wxString objName = OrbitData::GetRefObjectName(type);
   
   if (objName == wxT("INVALID_OBJECT_TYPE"))
   {
      throw ParameterException
         (wxT("OrbitReal::GetRefObjectName() ") + GmatBase::GetObjectTypeString(type) +
          wxT(" is not valid object type of ") + this->GetTypeName() + wxT("\n"));
   }
   
   return objName;
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
const StringArray& OrbitReal::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   return OrbitData::GetRefObjectNameArray(type);
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
bool OrbitReal::SetRefObjectName(const Gmat::ObjectType type,
                                 const wxString &name)
{
   return OrbitData::SetRefObjectName(type, name);
}


//------------------------------------------------------------------------------
// virtual GmatBase* GetRefObject(const Gmat::ObjectType type,
//                                const wxString &name)
//------------------------------------------------------------------------------
/**
 * Calls OrbitData to get object pointer of given type and name
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return reference object pointer for given object type and name
 */
//------------------------------------------------------------------------------
GmatBase* OrbitReal::GetRefObject(const Gmat::ObjectType type,
                                  const wxString &name)
{
   #ifdef DEBUG_REF_OBJECT
   MessageInterface::ShowMessage
      (wxT("OrbitReal::GetRefObject() this='%s', type=%d, name='%s'\n"),
       this->GetName().c_str(), type, name.c_str());
   #endif
   
   GmatBase *obj = OrbitData::GetRefObject(type, name);
   
   if (obj == NULL)
   {
      throw ParameterException
         (wxT("OrbitReal::GetRefObject() Cannot find ref. object of type:") +
          GmatBase::GetObjectTypeString(type) + wxT(", name:") + name + wxT(" in ") +
          this->GetName());
   }
   
   return obj;
}


//------------------------------------------------------------------------------
// virtual bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                           const wxString &name = wxT(""))
//------------------------------------------------------------------------------
/**
 * Calls OrbitData to set reference object pointer to given type and name.
 *
 * @param <obj>  reference object pointer
 * @param <type> object type
 * @param <name> object name
 *
 * @return true if object pointer is successfully set.
 *
 */
//------------------------------------------------------------------------------
bool OrbitReal::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                             const wxString &name)
{
   #if DEBUG_ORBITREAL
   MessageInterface::ShowMessage
      (wxT("OrbitReal::SetRefObject() setting type=%d, name=%s to %s\n"),
       type, name.c_str(), this->GetName().c_str());
   #endif
   
   return OrbitData::SetRefObject(obj, type, name);
}

