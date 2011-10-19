//$Id$
//------------------------------------------------------------------------------
//                                  OrbitRmat33
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. 
//
// Author: Linda Jun (GSFC/NASA)
// Created: 2009.03.30
//
/**
 * Declares OrbitRmat33 class.
 */
//------------------------------------------------------------------------------

#include "OrbitRmat33.hpp"
#include "ParameterException.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_ORBITRMAT33_REFOBJ
//#define DEBUG_ORBITRMAT33_SET

//------------------------------------------------------------------------------
// OrbitRmat33(const wxString &name, const wxString &typeStr, 
//             GmatBase *obj, const wxString &desc, const wxString &unit,
//             GmatParam::DepObject depObj, bool isSettable)
//------------------------------------------------------------------------------
OrbitRmat33::OrbitRmat33(const wxString &name, const wxString &typeStr, 
                         GmatBase *obj, const wxString &desc,
                         const wxString &unit, GmatParam::DepObject depObj,
                         bool isSettable)
   : Rmat33Var(name, typeStr, GmatParam::SYSTEM_PARAM, obj, desc, unit,
               depObj, Gmat::SPACECRAFT, isSettable)
{
   mNeedCoordSystem = true;
   AddRefObject(obj);
}


//------------------------------------------------------------------------------
// OrbitRmat33(const OrbitRmat33 &copy)
//------------------------------------------------------------------------------
OrbitRmat33::OrbitRmat33(const OrbitRmat33 &copy)
   : Rmat33Var(copy), OrbitData(copy)
{
}


//------------------------------------------------------------------------------
// OrbitRmat33& operator=(const OrbitRmat33 &right)
//------------------------------------------------------------------------------
OrbitRmat33& OrbitRmat33::operator=(const OrbitRmat33 &right)
{
   if (this != &right)
   {
      Rmat33Var::operator=(right);
      OrbitData::operator=(right);
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// ~OrbitRmat33()
//------------------------------------------------------------------------------
OrbitRmat33::~OrbitRmat33()
{
}


//------------------------------------------------------------------------------
// const Rmatrix66& EvaluateRmatrix()
//------------------------------------------------------------------------------
const Rmatrix& OrbitRmat33::EvaluateRmatrix()
{
   Evaluate();
   return mRmat33Value;
}


//------------------------------------------------------------------------------
// virtual CoordinateSystem* GetInternalCoordSystem()
//------------------------------------------------------------------------------
CoordinateSystem* OrbitRmat33::GetInternalCoordSystem()
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
void OrbitRmat33::SetSolarSystem(SolarSystem *ss)
{
   #ifdef DEBUG_ORBITRMAT33_SET
   MessageInterface::ShowMessage
      (wxT("OrbitRmat33::SetSolarSystem() '%s' entered, ss=<%p>'%s'\n"),
       GetName().c_str(), ss, ss->GetTypeName().c_str());
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
void OrbitRmat33::SetInternalCoordSystem(CoordinateSystem *cs)
{
   #ifdef DEBUG_ORBITRMAT33_SET
   MessageInterface::ShowMessage
      (wxT("OrbitRmat33::SetInternalCoordSystem() '%s' entered, cs=<%p>'%s'\n"),
       GetName().c_str(), cs, cs->GetTypeName().c_str());
   #endif
   
   OrbitData::SetInternalCoordSys(cs);
}


//------------------------------------------------------------------------------
// Integer GetNumRefObjects() const
//------------------------------------------------------------------------------
Integer OrbitRmat33::GetNumRefObjects() const
{
   return OrbitData::GetNumRefObjects();
}


//------------------------------------------------------------------------------
// bool addRefObject(GmatBase *obj, bool replaceName)
//------------------------------------------------------------------------------
bool OrbitRmat33::AddRefObject(GmatBase *obj, bool replaceName)
{
   if (obj != NULL)
   {
      #ifdef DEBUG_ORBITRMAT33_REFOBJ
      MessageInterface::ShowMessage
         (wxT("OrbitRmat33::AddRefObject() obj->GetName()=%s, type=%d\n"),
          obj->GetName().c_str(), obj->GetType());
      #endif
      
      return OrbitData::AddRefObject(obj->GetType(), obj->GetName(), obj,
                                     replaceName);
      
   }
   
   return false;
}


//------------------------------------------------------------------------------
// bool Validate()
//------------------------------------------------------------------------------
bool OrbitRmat33::Validate()
{
   return ValidateRefObjects(this);
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
bool OrbitRmat33::Initialize()
{
   try
   {
      InitializeRefObjects();
   }
   catch(BaseException &e)
   {
      throw GmatBaseException
         (wxT("OrbitRmat33::Initialize() Fail to initialize Parameter:") +
          this->GetTypeName() + wxT("\n") + e.GetFullMessage());
   }
   
   return true;
}


//------------------------------------------------------------------------------
// bool RenameRefObject(const Gmat::ObjectType type, const wxString &oldName,
//                      const wxString &newName)
//------------------------------------------------------------------------------
bool OrbitRmat33::RenameRefObject(const Gmat::ObjectType type,
                                  const wxString &oldName,
                                  const wxString &newName)
{
   return OrbitData::RenameRefObject(type, oldName, newName);
}


//------------------------------------------------------------------------------
// wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
wxString OrbitRmat33::GetRefObjectName(const Gmat::ObjectType type) const
{
   wxString objName = OrbitData::GetRefObjectName(type);
   
   if (objName == wxT("INVALID_OBJECT_TYPE"))
   {
      throw ParameterException
         (wxT("OrbitRmat33::GetRefObjectName() ") + GmatBase::GetObjectTypeString(type) +
          wxT(" is not valid object type of ") + this->GetTypeName() + wxT("\n"));
   }
   
   return objName;
}


//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
const StringArray& OrbitRmat33::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   return OrbitData::GetRefObjectNameArray(type);
}


//------------------------------------------------------------------------------
// bool SetRefObjectName(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
bool OrbitRmat33::SetRefObjectName(const Gmat::ObjectType type,
                                   const wxString &name)
{
   bool ret = OrbitData::SetRefObjectName(type, name);
   
   if (!ret)
      MessageInterface::ShowMessage
         (wxT("*** Warning *** OrbitRmat33::SetRefObjectName() RefObjType:%s is not valid ")
          wxT("for ParameterName:%s\n"), GmatBase::GetObjectTypeString(type).c_str(),
          this->GetName().c_str());
   
   return ret;
}


//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
GmatBase* OrbitRmat33::GetRefObject(const Gmat::ObjectType type,
                                    const wxString &name)
{
   GmatBase *obj = OrbitData::GetRefObject(type, name);
   
   if (obj == NULL)
   {
      throw ParameterException
         (wxT("OrbitRmat33::GetRefObject() Cannot find ref. object of type:") +
          GmatBase::GetObjectTypeString(type) + wxT(", name:") + name + wxT(" in ") +
          this->GetName());
   }
   
   return obj;
}


//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name)
//------------------------------------------------------------------------------
bool OrbitRmat33::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                               const wxString &name)
{
   #ifdef DEBUG_ORBITRMAT33_REFOBJ
   MessageInterface::ShowMessage
      (wxT("OrbitRmat33::SetRefObject() setting type=%d, name=%s to %s\n"),
       type, name.c_str(), this->GetName().c_str());
   #endif
   
   return OrbitData::SetRefObject(obj, type, name);
}

