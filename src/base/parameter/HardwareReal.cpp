//$Id: HardwareReal.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  HardwareReal
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc.
//
// Author: Linda Jun
// Created: 2009.03.20
//
/**
 * Declares BallisticMass real data class.
 */
//------------------------------------------------------------------------------

#include "HardwareReal.hpp"
#include "ParameterException.hpp"
#include "MessageInterface.hpp"


//------------------------------------------------------------------------------
// HardwareReal(const wxString &name, const wxString &typeStr, 
//              GmatBase *obj, const wxString &desc, const wxString &unit)
//------------------------------------------------------------------------------
HardwareReal::HardwareReal(const wxString &name, const wxString &typeStr, 
                           GmatBase *obj, const wxString &desc,
                           const wxString &unit)
   : RealVar(name, wxT(""), typeStr, GmatParam::SYSTEM_PARAM, obj, desc, unit,
             GmatParam::OWNED_OBJ, Gmat::SPACECRAFT, false, true),
     SpacecraftData(name)
{
   AddRefObject(obj);
}


//------------------------------------------------------------------------------
// HardwareReal(const HardwareReal &copy)
//------------------------------------------------------------------------------
HardwareReal::HardwareReal(const HardwareReal &copy)
   : RealVar(copy), SpacecraftData(copy)
{
}


//------------------------------------------------------------------------------
// HardwareReal& operator=(const HardwareReal &right)
//------------------------------------------------------------------------------
HardwareReal& HardwareReal::operator=(const HardwareReal &right)
{
   if (this != &right)
   {
      RealVar::operator=(right);
      SpacecraftData::operator=(right);
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// ~HardwareReal()
//------------------------------------------------------------------------------
HardwareReal::~HardwareReal()
{
}


//------------------------------------------------------------------------------
// Real EvaluateReal()
//------------------------------------------------------------------------------
Real HardwareReal::EvaluateReal()
{
   Evaluate();
   return mRealValue;
}


//------------------------------------------------------------------------------
// Integer GetNumRefObjects() const
//------------------------------------------------------------------------------
Integer HardwareReal::GetNumRefObjects() const
{
   return SpacecraftData::GetNumRefObjects();
}


//------------------------------------------------------------------------------
// bool addRefObject(GmatBase *obj, bool replaceName)
//------------------------------------------------------------------------------
bool HardwareReal::AddRefObject(GmatBase *obj, bool replaceName)
{
   if (obj != NULL)
   {
      #if DEBUG_ATTITUDEREAL
      MessageInterface::ShowMessage
         (wxT("HardwareReal::AddRefObject() obj->GetName()=%s, type=%d\n"),
          obj->GetName().c_str(), obj->GetType());
      #endif
      
      return SpacecraftData::AddRefObject(obj->GetType(), obj->GetName(), obj,
                                        replaceName);
      
   }
   
   return false;
}


//------------------------------------------------------------------------------
// bool Validate()
//------------------------------------------------------------------------------
bool HardwareReal::Validate()
{
   return ValidateRefObjects(this);
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
bool HardwareReal::Initialize()
{
   try
   {
      InitializeRefObjects();
   }
   catch(BaseException &e)
   {
      throw GmatBaseException
         (wxT("HardwareReal::Initialize() Fail to initialize Parameter:") +
          this->GetTypeName() + wxT("\n") + e.GetFullMessage());
   }
   
   return true;
}


//------------------------------------------------------------------------------
// bool RenameRefObject(const Gmat::ObjectType type, const wxString &oldName,
//                      const wxString &newName)
//------------------------------------------------------------------------------
bool HardwareReal::RenameRefObject(const Gmat::ObjectType type,
                                   const wxString &oldName,
                                   const wxString &newName)
{
   return SpacecraftData::RenameRefObject(type, oldName, newName);
}


//------------------------------------------------------------------------------
// wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
wxString HardwareReal::GetRefObjectName(const Gmat::ObjectType type) const
{
   wxString objName = SpacecraftData::GetRefObjectName(type);
   
   if (objName == wxT("INVALID_OBJECT_TYPE"))
   {
      throw ParameterException
         (wxT("HardwareReal::GetRefObjectName() ") + GmatBase::GetObjectTypeString(type) +
          wxT(" is not valid object type of ") + this->GetTypeName() + wxT("\n"));
   }
   
   return objName;
}


//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
const StringArray& HardwareReal::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   return SpacecraftData::GetRefObjectNameArray(type);
}


//------------------------------------------------------------------------------
// bool SetRefObjectName(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
bool HardwareReal::SetRefObjectName(const Gmat::ObjectType type,
                                    const wxString &name)
{
   bool ret = SpacecraftData::SetRefObjectName(type, name);
   
   if (!ret)
      MessageInterface::ShowMessage
         (wxT("*** Warning *** HardwareReal::SetRefObjectName() RefObjType:%s is not valid ")
          wxT("for ParameterName:%s\n"), GmatBase::GetObjectTypeString(type).c_str(),
          this->GetName().c_str());
   
   return ret;
}


//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
GmatBase* HardwareReal::GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name)
{
   GmatBase *obj = SpacecraftData::GetRefObject(type, name);
   
   if (obj == NULL)
   {
      throw ParameterException
         (wxT("HardwareReal::GetRefObject() Cannot find ref. object of type:") +
          GmatBase::GetObjectTypeString(type) + wxT(", name:") + name + wxT(" in ") +
          this->GetName());
   }
   
   return obj;
}


//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name)
//------------------------------------------------------------------------------
bool HardwareReal::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                const wxString &name)
{
   #if DEBUG_ATTITUDEREAL
   MessageInterface::ShowMessage
      (wxT("HardwareReal::SetRefObject() setting type=%d, name=%s to %s\n"),
       type, name.c_str(), this->GetName().c_str());
   #endif
   
   return SpacecraftData::SetRefObject(obj, type, name);
}

