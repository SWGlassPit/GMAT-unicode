//$Id: RefData.cpp 9752 2011-08-10 12:39:44Z wendys-dev $
//------------------------------------------------------------------------------
//                                  RefData
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
// Created: 2004/01/09
//
/**
 * Implements base class of reference data.
 */
//------------------------------------------------------------------------------
#include "gmatdefs.hpp"
#include "RefData.hpp"
#include "ParameterException.hpp"
#include "StringUtil.hpp"          // for ParseParameter()
#include "MessageInterface.hpp"

//#define DEBUG_REFDATA_OBJECT 2
//#define DEBUG_REFDATA_OBJECT_GET 2
//#define DEBUG_REFDATA_OBJECT_SET 2
//#define DEBUG_REFDATA_FIND 1
//#define DEBUG_REFDATA_ADD 1
//#define DEBUG_RENAME 1
//#define DEBUG_CLONE 1

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// RefData(const wxString &name = wxT(""))
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
RefData::RefData(const wxString &name)
{
   mName = name;
   mRefObjList.clear();
   mNumRefObjects = 0;
}


//------------------------------------------------------------------------------
// RefData(const RefData &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the RefData object being copied.
 */
//------------------------------------------------------------------------------
RefData::RefData(const RefData &copy)
{
   mName = copy.mName;
   mObjectTypeNames = copy.mObjectTypeNames;
   mAllRefObjectNames = copy.mAllRefObjectNames;
   mNumRefObjects = copy.mNumRefObjects;
   mRefObjList = copy.mRefObjList;
}


//------------------------------------------------------------------------------
// RefData& operator= (const RefData &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the object being copied.
 *
 * @return reference to this object
 */
//------------------------------------------------------------------------------
RefData& RefData::operator= (const RefData& right)
{
   mName = right.mName;
   mObjectTypeNames = right.mObjectTypeNames;
   mAllRefObjectNames = right.mAllRefObjectNames;
   mNumRefObjects = right.mNumRefObjects;
   mRefObjList = right.mRefObjList;
   return *this;
}


//------------------------------------------------------------------------------
// ~RefData()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
RefData::~RefData()
{
   //MessageInterface::ShowMessage(wxT("==> RefData::~RefData()\n"));
   mRefObjList.clear();
}


//------------------------------------------------------------------------------
// virtual Integer GetNumRefObjects() const
//------------------------------------------------------------------------------
Integer RefData::GetNumRefObjects() const
{
   return mNumRefObjects;
}


//------------------------------------------------------------------------------
// GmatBase* GetSpacecraft()
//------------------------------------------------------------------------------
/*
 * Returns first spacecraft object from the list
 */
//------------------------------------------------------------------------------
GmatBase* RefData::GetSpacecraft()
{
   for (int i=0; i<mNumRefObjects; i++)
   {
      if (mRefObjList[i].objType == Gmat::SPACECRAFT)
         return mRefObjList[i].obj;
   }
   
   return NULL;
}


//------------------------------------------------------------------------------
// wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
wxString RefData::GetRefObjectName(const Gmat::ObjectType type) const
{
   for (int i=0; i<mNumRefObjects; i++)
   {
      #if DEBUG_REFDATA_OBJECT > 1
      MessageInterface::ShowMessage
         (wxT("RefData::GetRefObjectName() mRefObjList[i].objType=%d\n"),
          mRefObjList[i].objType);
      #endif
      
      if (mRefObjList[i].objType == type)
      {
         //Notes: will return first object name.
         #if DEBUG_REFDATA_OBJECT > 1
         MessageInterface::ShowMessage
            (wxT("RefData::GetRefObjectName() type=%d returning: %s\n"), type,
             mRefObjList[i].objName.c_str());
         #endif
         
         return mRefObjList[i].objName; 
      }
   }
   
   #if DEBUG_REFDATA_OBJECT
   MessageInterface::ShowMessage
      (wxT("RefData::GetRefObjectName() '%s', type=%d, throwing exception ")
       wxT("INVALID_OBJECT_TYPE\n"), mName.c_str(), type);
   #endif
   
   //return wxT("RefData::GetRefObjectName(): INVALID_OBJECT_TYPE");
   throw ParameterException(wxT("RefData::GetRefObjectName(): INVALID_OBJECT_TYPE"));
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
const StringArray& RefData::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   mAllRefObjectNames.clear();

   #if DEBUG_REFDATA_OBJECT_GET
   MessageInterface::ShowMessage
      (wxT("RefData::GetRefObjectNameArray() '%s', type=%d\n   there are %d ref ")
       wxT("objects\n"), mName.c_str(), type, mNumRefObjects);
   for (int i=0; i<mNumRefObjects; i++)
   {
      MessageInterface::ShowMessage
         (wxT("   objType=%d, name='%s'\n"), mRefObjList[i].objType,
          mRefObjList[i].objName.c_str());
   }
   #endif
   
   if (type == Gmat::UNKNOWN_OBJECT)
   {
      for (int i=0; i<mNumRefObjects; i++)
      {
         mAllRefObjectNames.push_back(mRefObjList[i].objName);
      }
   }
   else
   {
      for (int i=0; i<mNumRefObjects; i++)
      {
         if (mRefObjList[i].objType == type)
            mAllRefObjectNames.push_back(mRefObjList[i].objName);
      }
   }
   
   return mAllRefObjectNames;
}


//------------------------------------------------------------------------------
// bool SetRefObjectName(Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Adds type and name to reference object list.
 *
 * @param <type> reference object type
 * @param <name> reference object name
 *
 * @return true if type and name has successfully added to the list
 *
 */
//------------------------------------------------------------------------------
bool RefData::SetRefObjectName(Gmat::ObjectType type, const wxString &name)
{
   #if DEBUG_REFDATA_OBJECT
   MessageInterface::ShowMessage
      (wxT("RefData::SetRefObjectName() '%s' entered, type=%d(%s), name=%s\n"),
       mName.c_str(), type,  GmatBase::OBJECT_TYPE_STRING[type - Gmat::SPACECRAFT].c_str(), name.c_str());
   #endif
   
   if (FindFirstObjectName(type) != wxT(""))
   {
      for (int i=0; i<mNumRefObjects; i++)
      {
         if (mRefObjList[i].objType == type)
         {
            mRefObjList[i].objName = name;
            return true;
         }
      }
   }
   
   return AddRefObject(type, name);
   
}


//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type,
//                        const wxString &name = wxT(""));
//------------------------------------------------------------------------------
GmatBase* RefData::GetRefObject(const Gmat::ObjectType type,
                                const wxString &name)
{
   for (int i=0; i<mNumRefObjects; i++)
   {
      if (mRefObjList[i].objType == type)
      {
         if (name == wxT("")) //if name is wxT(""), return first object
            return mRefObjList[i].obj;
         
         if (mRefObjList[i].objName == name)
         {
            return mRefObjList[i].obj;
         }
      }
   }
   
   return NULL;
}


//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name = wxT(""))
//------------------------------------------------------------------------------
/**
 * Sets object which is used in evaluation.
 *
 * @return true if the object has been added.
 */
//------------------------------------------------------------------------------
bool RefData::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                           const wxString &name)
{
   bool status = false;
   #if DEBUG_REFDATA_OBJECT_SET
   MessageInterface::ShowMessage
      (wxT("RefData::SetRefObject() <%p>'%s' entered\n   numRefObjects=%d, type=%d, ")
       wxT("obj=<%p>'%s'\n"), this, mName.c_str(), mNumRefObjects, type, obj, name.c_str());
   #endif
   
   // Since Sandbox calls SetRefObject() with obj->GetType(), I need to
   // set type to SPACE_POINT if CELESTIAL_BODY
   
   Gmat::ObjectType actualType = type;
   if (type == Gmat::CELESTIAL_BODY)
      actualType = Gmat::SPACE_POINT;
   
   for (int i=0; i<mNumRefObjects; i++)
   {
      if (mRefObjList[i].objType == actualType)
      {
         if (mRefObjList[i].objName == name)
         {
            mRefObjList[i].obj = obj;
            #if DEBUG_REFDATA_OBJECT_SET > 1
            MessageInterface::ShowMessage
               (wxT("   The object pointer <%p> set to '%s'\n"), obj, name.c_str());
            #endif
            status = true;
            break;
         }
      }
   }
   
   #if DEBUG_REFDATA_OBJECT_SET > 1
   for (int i=0; i<mNumRefObjects; i++)
   {
      MessageInterface::ShowMessage
         (wxT("   type=%d, obj=<%p>, name='%s'\n"), mRefObjList[i].objType,
          mRefObjList[i].obj, mRefObjList[i].objName.c_str());
   }   
   #endif
   
   if (!status)
   {
      #if DEBUG_REFDATA_OBJECT_SET
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** RefData::SetRefObject() Cannot find '%s' of type '%s'\n"),
          name.c_str(), GmatBase::GetObjectTypeString(actualType).c_str());
      #endif
   }
   
   #if DEBUG_REFDATA_OBJECT_SET
   MessageInterface::ShowMessage
      (wxT("RefData::SetRefObject() <%p>'%s' returning %d\n"), this, mName.c_str(), status);
   #endif
   
   return status;
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
/*
 * This method renames Parameter object used in the Parameter, such as Sat in
 * Sat.EarthMJ2000Eq.X MyBurn in MyBurn.Element1. This also renames Parameter
 * object owned object name such as Sat.Thruster1.DutyCycle.
 */
//---------------------------------------------------------------------------
bool RefData::RenameRefObject(const Gmat::ObjectType type,
                              const wxString &oldName,
                              const wxString &newName)
{
   #if DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("RefData::RenameRefObject() '%s' entered, type=%d, oldName='%s', ")
       wxT("newName='%s'\n"), mName.c_str(), type, oldName.c_str(), newName.c_str());
   MessageInterface::ShowMessage(wxT("   mNumRefObjects=%d\n"), mNumRefObjects);
   #endif
   
   // Check for allowed object types for rename
   if (type != Gmat::SPACECRAFT && type != Gmat::COORDINATE_SYSTEM &&
       type != Gmat::CALCULATED_POINT && type != Gmat::BURN &&
       type != Gmat::IMPULSIVE_BURN && type != Gmat::HARDWARE &&
       type != Gmat::THRUSTER && type != Gmat::FUEL_TANK)
   {
      #if DEBUG_RENAME
      MessageInterface::ShowMessage
         (wxT("RefData::RenameRefObject() '%s' returning true, there are no allowed types\n"),
          mName.c_str());
      #endif
      return true;
   }
   
   // Change instance name
   wxString ownerStr, typeStr, depStr;
   GmatStringUtil::ParseParameter(mName, typeStr, ownerStr, depStr);
   #if DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("   mName='%s', owner='%s', dep='%s', type='%s'\n"),
       mName.c_str(), ownerStr.c_str(), depStr.c_str(), typeStr.c_str());
   #endif
   // Check for depStr for hardware parameter such as Sat.Thruster1.DutyCycle
   if (ownerStr == oldName || depStr == oldName)
   {
      mName = GmatStringUtil::ReplaceName(mName, oldName, newName);
      #if DEBUG_RENAME
      MessageInterface::ShowMessage
         (wxT("   instance name changed to '%s'\n"), mName.c_str());
      #endif
   }
   
   Integer numRenamed = 0;
   for (int i=0; i<mNumRefObjects; i++)
   {
      if (mRefObjList[i].objType == type)
      {
         if (mRefObjList[i].objName == oldName)
         {
            mRefObjList[i].objName = newName;
            numRenamed++;
            #if DEBUG_RENAME
            MessageInterface::ShowMessage
               (wxT("   '%s' renamed to '%s'\n"), oldName.c_str(),
                mRefObjList[i].objName.c_str());
            #endif
         }
      }
   }
   
   #if DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("RefData::RenameRefObject() '%s' returning true, %d ref objects renamed!\n"),
       mName.c_str(), numRenamed);
   #endif
   return true;
}


//------------------------------------------------------------------------------
// virtual const wxString* GetValidObjectList() const
//------------------------------------------------------------------------------
const wxString* RefData::GetValidObjectList() const
{
   return NULL;
}


//---------------------------------
// protected methods
//---------------------------------

//------------------------------------------------------------------------------
// bool AddRefObject(const Gmat::ObjectType type, const wxString &name,
//                   GmatBase *obj = NULL, bool replaceName = false)
//------------------------------------------------------------------------------
/**
 * Adds object which is used in evaluation.
 *
 * @return true if the object has been added.
 */
//------------------------------------------------------------------------------
bool RefData::AddRefObject(const Gmat::ObjectType type, const wxString &name,
                           GmatBase *obj, bool replaceName)
{
   #if DEBUG_REFDATA_ADD
   MessageInterface::ShowMessage
      (wxT("==> RefData::AddRefObject() '%s' entered, mNumRefObjects=%d, type=%d, ")
       wxT("name=%s, obj=%p, replaceName=%d\n"), mName.c_str(), mNumRefObjects, type,
       name.c_str(), obj, replaceName);
   #endif
   
   Gmat::ObjectType actualType = type;
   if (type == Gmat::CELESTIAL_BODY)
      actualType = Gmat::SPACE_POINT;

   if (IsValidObjectType(actualType))
   {
      if (FindFirstObjectName(actualType) == wxT(""))
      {
         RefObjType newObj(actualType, name, obj);
         mRefObjList.push_back(newObj);
         mNumRefObjects = mRefObjList.size();
         return true;        
      }
      else
      {
         if (replaceName)
            SetRefObjectWithNewName(obj, actualType, name);
         else
            SetRefObject(obj, actualType, name);
         
         return true;
      }
   }
   
   #if DEBUG_REFDATA_ADD
   MessageInterface::ShowMessage
      (wxT("RefData::AddRefObject() '%s' does NOT have a valid object type so returning ")
       wxT("false\n"), name.c_str());
   #endif
   
   return false;
}


//------------------------------------------------------------------------------
// bool SetRefObjectWithNewName(GmatBase *obj, const Gmat::ObjectType type,
//                              const wxString &name = wxT(""))
//------------------------------------------------------------------------------
/**
 * Sets object pointer with new name which is used in evaluation.
 *
 * @return true if the object has been added.
 */
//------------------------------------------------------------------------------
bool RefData::SetRefObjectWithNewName(GmatBase *obj, const Gmat::ObjectType type,
                                      const wxString &name)
{
   #if DEBUG_REFDATA_OBJECT
   MessageInterface::ShowMessage
      (wxT("RefData::SetRefObjectWithNewName() numRefObjects=%d, type=%d, name=%s ")
       wxT("obj addr=%p\n"), mNumRefObjects, type, name.c_str(), obj);
   #endif
   
   #if DEBUG_REFDATA_OBJECT > 1
   for (int i=0; i<mNumRefObjects; i++)
   {
      MessageInterface::ShowMessage
         (wxT("type=%d, name=%s, obj=%d\n"), mRefObjList[i].objType,
          mRefObjList[i].objName.c_str(), mRefObjList[i].obj);
   }   
   #endif
   
   for (int i=0; i<mNumRefObjects; i++)
   {
      if (mRefObjList[i].objType == type)
      {
         mRefObjList[i].objName = name;
         mRefObjList[i].obj = obj;
         
         #if DEBUG_REFDATA_OBJECT > 1
         MessageInterface::ShowMessage
            (wxT("RefData::SetRefObjectWithName() set %s to %p\n"),
             mRefObjList[i].objName.c_str(), obj);
         #endif
         return true;
      }
   }

   #if DEBUG_REFDATA_OBJECT
   MessageInterface::ShowMessage
      (wxT("*** Warning *** RefData::SetRefObjectWithName() Cannot find type=%s\n"),
       GmatBase::GetObjectTypeString(type).c_str());
   #endif
   
   return false;
}


//------------------------------------------------------------------------------
// virtual void InitializeRefObjects()
//------------------------------------------------------------------------------
void RefData::InitializeRefObjects()
{
   //do nothing here
}


//------------------------------------------------------------------------------
// bool HasObjectType(const wxString &type) const
//------------------------------------------------------------------------------
/**
 * @return true if it has the given object type, false otherwise
 */
//------------------------------------------------------------------------------
bool RefData::HasObjectType(const wxString &type) const
{

   for (int i=0; i<mNumRefObjects; i++)
   {
      if (GmatBase::GetObjectTypeString(mRefObjList[i].objType) == type)
         return true;
   }

   return false;
}


//------------------------------------------------------------------------------
// GmatBase* FindFirstObject(const wxString &typeName) const
//------------------------------------------------------------------------------
/**
 * @return first object found for given object type name.
 */
//------------------------------------------------------------------------------
GmatBase* RefData::FindFirstObject(const wxString &typeName) const
{   
   return FindFirstObject(GmatBase::GetObjectType(typeName));
}


//------------------------------------------------------------------------------
// GmatBase* FindFirstObject(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
/**
 * @return first object found for given object type.
 */
//------------------------------------------------------------------------------
GmatBase* RefData::FindFirstObject(const Gmat::ObjectType type) const
{
   #if DEBUG_REFDATA_FIND
   MessageInterface::ShowMessage
      (wxT("RefData::FindFirstObject() type=%d mNumRefObjects=%d\n"),
       type, mNumRefObjects);
   #endif
   
   for (int i=0; i<mNumRefObjects; i++)
   {
      #if DEBUG_REFDATA_FIND
      MessageInterface::ShowMessage
         (wxT("RefData::FindFirstObject() i=%d, type=%d, name=%s, obj=%p\n"), i,
          mRefObjList[i].objType, mRefObjList[i].objName.c_str(), mRefObjList[i].obj);
      #endif
      
      if (mRefObjList[i].objType == type)
      {
         #if DEBUG_REFDATA_FIND
         MessageInterface::ShowMessage
            (wxT("RefData::FindFirstObject() returning %p\n"), mRefObjList[i].obj);
         #endif
         
         return mRefObjList[i].obj;
      }
   }
   
   #if DEBUG_REFDATA_FIND
   MessageInterface::ShowMessage
      (wxT("RefData::FindFirstObject() returning NULL\n"));
   #endif
   
   return NULL;
}


//------------------------------------------------------------------------------
// wxString FindFirstObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
/**
 * @return first object name found for given object type.
 */
//------------------------------------------------------------------------------
wxString RefData::FindFirstObjectName(const Gmat::ObjectType type) const
{
   for (int i=0; i<mNumRefObjects; i++)
   {
      #if DEBUG_REFDATA_OBJECT > 1
      MessageInterface::ShowMessage
         (wxT("RefData::FindFirstObjectName() mRefObjList[%d].objType=%d, objName=%s\n"),
          i, mRefObjList[i].objType, mRefObjList[i].objName.c_str());
      #endif
      
      if (mRefObjList[i].objType == type)
         return mRefObjList[i].objName;
   }
   
   return wxT("");
}

