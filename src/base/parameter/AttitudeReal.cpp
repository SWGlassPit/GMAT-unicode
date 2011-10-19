//$Id: AttitudeReal.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  AttitudeReal
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
// Author: Daniel Hunter
// Created: 2006/6/26
//
/**
 * Declares Attitude real data class.
 */
//------------------------------------------------------------------------------
#include "AttitudeReal.hpp"
#include "ParameterException.hpp"
#include "MessageInterface.hpp"

//------------------------------------------------------------------------------
// AttitudeReal(const wxString &name, const wxString &typeStr, 
//              GmatBase *obj, const wxString &desc, const wxString &unit)
//------------------------------------------------------------------------------
AttitudeReal::AttitudeReal(const wxString &name, const wxString &typeStr, 
                           GmatBase *obj, const wxString &desc,
                           const wxString &unit)
   : RealVar(name, wxT(""), typeStr, GmatParam::SYSTEM_PARAM, obj, desc, unit,
             GmatParam::NO_DEP, Gmat::SPACECRAFT)
{
   AddRefObject(obj);
}


//------------------------------------------------------------------------------
// AttitudeReal(const AttitudeReal &copy)
//------------------------------------------------------------------------------
AttitudeReal::AttitudeReal(const AttitudeReal &copy)
   : RealVar(copy), AttitudeData(copy)
{
}


//------------------------------------------------------------------------------
// AttitudeReal& operator=(const AttitudeReal &right)
//------------------------------------------------------------------------------
AttitudeReal& AttitudeReal::operator=(const AttitudeReal &right)
{
   if (this != &right)
   {
      RealVar::operator=(right);
      AttitudeData::operator=(right);
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// ~AttitudeReal()
//------------------------------------------------------------------------------
AttitudeReal::~AttitudeReal()
{
}


//------------------------------------------------------------------------------
// Real EvaluateReal()
//------------------------------------------------------------------------------
Real AttitudeReal::EvaluateReal()
{
   Evaluate();
   return mRealValue;
}


//------------------------------------------------------------------------------
// Integer GetNumRefObjects() const
//------------------------------------------------------------------------------
Integer AttitudeReal::GetNumRefObjects() const
{
   return AttitudeData::GetNumRefObjects();
}


//------------------------------------------------------------------------------
// bool addRefObject(GmatBase *obj, bool replaceName)
//------------------------------------------------------------------------------
bool AttitudeReal::AddRefObject(GmatBase *obj, bool replaceName)
{
   if (obj != NULL)
   {
      #if DEBUG_ATTITUDEREAL
      MessageInterface::ShowMessage
         (wxT("AttitudeReal::AddRefObject() obj->GetName()=%s, type=%d\n"),
          obj->GetName().c_str(), obj->GetType());
      #endif
      
      return AttitudeData::AddRefObject(obj->GetType(), obj->GetName(), obj,
                                        replaceName);
      
   }
   
   return false;
}


//------------------------------------------------------------------------------
// bool Validate()
//------------------------------------------------------------------------------
bool AttitudeReal::Validate()
{
   return ValidateRefObjects(this);
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
bool AttitudeReal::Initialize()
{
   try
   {
      InitializeRefObjects();
   }
   catch(BaseException &e)
   {
      throw GmatBaseException
         (wxT("AttitudeReal::Initialize() Fail to initialize Parameter:") +
          this->GetTypeName() + wxT("\n") + e.GetFullMessage());
   }
   
   return true;
}


//------------------------------------------------------------------------------
// bool RenameRefObject(const Gmat::ObjectType type, const wxString &oldName,
//                      const wxString &newName)
//------------------------------------------------------------------------------
bool AttitudeReal::RenameRefObject(const Gmat::ObjectType type,
                                   const wxString &oldName,
                                   const wxString &newName)
{
   return AttitudeData::RenameRefObject(type, oldName, newName);
}


//------------------------------------------------------------------------------
// wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
wxString AttitudeReal::GetRefObjectName(const Gmat::ObjectType type) const
{
   wxString objName = AttitudeData::GetRefObjectName(type);
   
   if (objName == wxT("INVALID_OBJECT_TYPE"))
   {
      throw ParameterException
         (wxT("AttitudeReal::GetRefObjectName() ") + GmatBase::GetObjectTypeString(type) +
          wxT(" is not valid object type of ") + this->GetTypeName() + wxT("\n"));
   }
   
   return objName;
}


//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
const StringArray& AttitudeReal::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   return AttitudeData::GetRefObjectNameArray(type);
}


//------------------------------------------------------------------------------
// bool SetRefObjectName(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
bool AttitudeReal::SetRefObjectName(const Gmat::ObjectType type,
                                    const wxString &name)
{
   bool ret = AttitudeData::SetRefObjectName(type, name);
   
   if (!ret)
      MessageInterface::ShowMessage
         (wxT("*** Warning *** AttitudeReal::SetRefObjectName() RefObjType:%s is not valid ")
          wxT("for ParameterName:%s\n"), GmatBase::GetObjectTypeString(type).c_str(),
          this->GetName().c_str());
   
   return ret;
}


//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
GmatBase* AttitudeReal::GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name)
{
   GmatBase *obj = AttitudeData::GetRefObject(type, name);
   
   if (obj == NULL)
   {
      throw ParameterException
         (wxT("AttitudeReal::GetRefObject() Cannot find ref. object of type:") +
          GmatBase::GetObjectTypeString(type) + wxT(", name:") + name + wxT(" in ") +
          this->GetName());
   }
   
   return obj;
}


//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name)
//------------------------------------------------------------------------------
bool AttitudeReal::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                const wxString &name)
{
   #if DEBUG_ATTITUDEREAL
   MessageInterface::ShowMessage
      (wxT("AttitudeReal::SetRefObject() setting type=%d, name=%s to %s\n"),
       type, name.c_str(), this->GetName().c_str());
   #endif
   
   return AttitudeData::SetRefObject(obj, type, name);
}

