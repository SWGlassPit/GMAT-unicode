//$Id$
//------------------------------------------------------------------------------
//                                  BallisticMassReal
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

#include "BallisticMassReal.hpp"
#include "ParameterException.hpp"
#include "MessageInterface.hpp"


//------------------------------------------------------------------------------
// BallisticMassReal(const wxString &name, const wxString &typeStr, 
//              GmatBase *obj, const wxString &desc, const wxString &unit)
//------------------------------------------------------------------------------
BallisticMassReal::BallisticMassReal(const wxString &name, const wxString &typeStr, 
                                     GmatBase *obj, const wxString &desc,
                                     const wxString &unit)
   : RealVar(name, wxT(""), typeStr, GmatParam::SYSTEM_PARAM, obj, desc, unit,
             GmatParam::NO_DEP, Gmat::SPACECRAFT),
     SpacecraftData(name)
{
   AddRefObject(obj);
}


//------------------------------------------------------------------------------
// BallisticMassReal(const BallisticMassReal &copy)
//------------------------------------------------------------------------------
BallisticMassReal::BallisticMassReal(const BallisticMassReal &copy)
   : RealVar(copy), SpacecraftData(copy)
{
}


//------------------------------------------------------------------------------
// BallisticMassReal& operator=(const BallisticMassReal &right)
//------------------------------------------------------------------------------
BallisticMassReal& BallisticMassReal::operator=(const BallisticMassReal &right)
{
   if (this != &right)
   {
      RealVar::operator=(right);
      SpacecraftData::operator=(right);
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// ~BallisticMassReal()
//------------------------------------------------------------------------------
BallisticMassReal::~BallisticMassReal()
{
}


//------------------------------------------------------------------------------
// Real EvaluateReal()
//------------------------------------------------------------------------------
Real BallisticMassReal::EvaluateReal()
{
   Evaluate();
   return mRealValue;
}


//------------------------------------------------------------------------------
// Integer GetNumRefObjects() const
//------------------------------------------------------------------------------
Integer BallisticMassReal::GetNumRefObjects() const
{
   return SpacecraftData::GetNumRefObjects();
}


//------------------------------------------------------------------------------
// bool addRefObject(GmatBase *obj, bool replaceName)
//------------------------------------------------------------------------------
bool BallisticMassReal::AddRefObject(GmatBase *obj, bool replaceName)
{
   if (obj != NULL)
   {
      #if DEBUG_ATTITUDEREAL
      MessageInterface::ShowMessage
         (wxT("BallisticMassReal::AddRefObject() obj->GetName()=%s, type=%d\n"),
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
bool BallisticMassReal::Validate()
{
   return ValidateRefObjects(this);
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
bool BallisticMassReal::Initialize()
{
   try
   {
      InitializeRefObjects();
   }
   catch(BaseException &e)
   {
      throw GmatBaseException
         (wxT("BallisticMassReal::Initialize() Fail to initialize Parameter:") +
          this->GetTypeName() + wxT("\n") + e.GetFullMessage());
   }
   
   return true;
}


//------------------------------------------------------------------------------
// bool RenameRefObject(const Gmat::ObjectType type, const wxString &oldName,
//                      const wxString &newName)
//------------------------------------------------------------------------------
bool BallisticMassReal::RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName)
{
   return SpacecraftData::RenameRefObject(type, oldName, newName);
}


//------------------------------------------------------------------------------
// wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
wxString BallisticMassReal::GetRefObjectName(const Gmat::ObjectType type) const
{
   wxString objName = SpacecraftData::GetRefObjectName(type);
   
   if (objName == wxT("INVALID_OBJECT_TYPE"))
   {
      throw ParameterException
         (wxT("BallisticMassReal::GetRefObjectName() ") + GmatBase::GetObjectTypeString(type) +
          wxT(" is not valid object type of ") + this->GetTypeName() + wxT("\n"));
   }
   
   return objName;
}


//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
const StringArray& BallisticMassReal::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   return SpacecraftData::GetRefObjectNameArray(type);
}


//------------------------------------------------------------------------------
// bool SetRefObjectName(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
bool BallisticMassReal::SetRefObjectName(const Gmat::ObjectType type,
                                         const wxString &name)
{
   bool ret = SpacecraftData::SetRefObjectName(type, name);
   
   if (!ret)
      MessageInterface::ShowMessage
         (wxT("*** Warning *** BallisticMassReal::SetRefObjectName() RefObjType:%s is not valid ")
          wxT("for ParameterName:%s\n"), GmatBase::GetObjectTypeString(type).c_str(),
          this->GetName().c_str());
   
   return ret;
}


//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
GmatBase* BallisticMassReal::GetRefObject(const Gmat::ObjectType type,
                                          const wxString &name)
{
   GmatBase *obj = SpacecraftData::GetRefObject(type, name);
   
   if (obj == NULL)
   {
      throw ParameterException
         (wxT("BallisticMassReal::GetRefObject() Cannot find ref. object of type:") +
          GmatBase::GetObjectTypeString(type) + wxT(", name:") + name + wxT(" in ") +
          this->GetName());
   }
   
   return obj;
}


//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name)
//------------------------------------------------------------------------------
bool BallisticMassReal::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name)
{
   #if DEBUG_ATTITUDEREAL
   MessageInterface::ShowMessage
      (wxT("BallisticMassReal::SetRefObject() setting type=%d, name=%s to %s\n"),
       type, name.c_str(), this->GetName().c_str());
   #endif
   
   return SpacecraftData::SetRefObject(obj, type, name);
}

