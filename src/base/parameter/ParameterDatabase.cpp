//$Id: ParameterDatabase.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                ParameterDatabase
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
// Created: 2003/09/18
//
/**
 * Implements parameter database class.
 */
//------------------------------------------------------------------------------

#include "ParameterDatabase.hpp"
#include "ParameterDatabaseException.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_RENAME
//#define DEBUG_PARAM_DB
//#define DEBUG_PARAMDB_ADD

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//---------------------------------
// public
//---------------------------------

//------------------------------------------------------------------------------
// ParameterDatabase()
//------------------------------------------------------------------------------
/**
 * Default constructor.
 */
//------------------------------------------------------------------------------
ParameterDatabase::ParameterDatabase()
{
   mNumParams = 0;
   mStringParamPtrMap = new StringParamPtrMap;
   #ifdef DEBUG_MEMORY
   MemoryTracker::Instance()->Add
      (mStringParamPtrMap, wxT("mStringParamPtrMap"), wxT("ParameterDatabase::ParameterDatabase()"),
       wxT("mStringParamPtrMap = new StringParamPtrMap"));
   #endif
   
   #ifdef DEBUG_PARAMDB
   MessageInterface::ShowMessage
      (wxT("ParameterDatabase(default) mStringParamPtrMap.size()=%d, ")
       wxT("mNumParams=%d\n"),  mStringParamPtrMap->size(), mNumParams);
   #endif
}


//------------------------------------------------------------------------------
// ParameterDatabase(const ParameterDatabase &copy)
//------------------------------------------------------------------------------
ParameterDatabase::ParameterDatabase(const ParameterDatabase &copy)
{
   mNumParams = copy.mNumParams;
   mStringParamPtrMap = new StringParamPtrMap;
   #ifdef DEBUG_MEMORY
   MemoryTracker::Instance()->Add
      (mStringParamPtrMap, wxT("mStringParamPtrMap"), wxT("ParameterDatabase copy constructor"),
       wxT("mStringParamPtrMap = new StringParamPtrMap"));
   #endif
   
   StringParamPtrMap::iterator pos;
   
   for (pos = copy.mStringParamPtrMap->begin();
        pos != copy.mStringParamPtrMap->end(); ++pos)
   {
      Add(pos->first, pos->second);
   }
   
   #ifdef DEBUG_PARAMDB
   MessageInterface::ShowMessage
      (wxT("==> ParameterDatabase(copy) mStringParamPtrMap.size()=%d, ")
       wxT("mNumParams=%d\n"),  mStringParamPtrMap->size(), mNumParams);
   #endif
}


//------------------------------------------------------------------------------
// ParameterDatabase& operator=(const ParameterDatabase &right)
//------------------------------------------------------------------------------
ParameterDatabase& ParameterDatabase::operator=(const ParameterDatabase &right)
{
   if (this == &right)
      return *this;
   
   mNumParams = right.mNumParams;

   if (mStringParamPtrMap)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (mStringParamPtrMap, wxT("mStringParamPtrMap"), wxT("ParameterDatabase operator="),
          wxT("deleting old map"));
      #endif
      delete mStringParamPtrMap;
   }
   
   mStringParamPtrMap = new StringParamPtrMap;
   #ifdef DEBUG_MEMORY
   MemoryTracker::Instance()->Add
      (mStringParamPtrMap, wxT("mStringParamPtrMap"), wxT("ParameterDatabase operator="),
       wxT("mStringParamPtrMap = new StringParamPtrMap"));
   #endif
   
   StringParamPtrMap::iterator pos;
   
   for (pos = right.mStringParamPtrMap->begin();
        pos != right.mStringParamPtrMap->end(); ++pos)
   {
      Add(pos->first, pos->second);
   }
   
   #ifdef DEBUG_PARAMDB
   MessageInterface::ShowMessage
      (wxT("==> ParameterDatabase(=) mStringParamPtrMap.size()=%d, ")
       wxT("mNumParams=%d\n"),  mStringParamPtrMap->size(), mNumParams);
   #endif
   
   return *this;
}


//------------------------------------------------------------------------------
// virtual ~ParameterDatabase()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
ParameterDatabase::~ParameterDatabase()
{
   #ifdef DEBUG_MEMORY
   MemoryTracker::Instance()->Remove
      (mStringParamPtrMap, wxT("mStringParamPtrMap"), wxT("ParameterDatabase destructor"),
       wxT("deleting old map"));
   #endif
   delete mStringParamPtrMap;
   mStringParamPtrMap = NULL;
}

//---------------------------------
// Get methods
//---------------------------------

//------------------------------------------------------------------------------
// Integer GetNumParameters() const
//------------------------------------------------------------------------------
/**
 * Retrieves number of parameters in the database.
 *
 * @return number of parameters
 */
//------------------------------------------------------------------------------
Integer ParameterDatabase::GetNumParameters() const
{
   return mNumParams;
}


//------------------------------------------------------------------------------
// const StringArray& GetNamesOfParameters()
//------------------------------------------------------------------------------
/**
 * @return names of parameters
 */
//------------------------------------------------------------------------------
const StringArray& ParameterDatabase::GetNamesOfParameters()
{
   mParamNames.clear();
   StringParamPtrMap::iterator pos;

   if (mStringParamPtrMap == NULL)
      throw ParameterDatabaseException
         (wxT("ParameterDatabase::GetNamesOfParameters() mStringParamPtrMap is NULL\n"));

   #ifdef DEBUG_PARAMDB
   MessageInterface::ShowMessage
      (wxT("==> ParameterDatabase::GetNamesOfParameters() mStringParamPtrMap.size()=%d, ")
       wxT("mNumParams=%d\n"),  mStringParamPtrMap->size(), mNumParams);
   #endif
   
   for (pos = mStringParamPtrMap->begin(); pos != mStringParamPtrMap->end(); ++pos)
   {
      mParamNames.push_back(pos->first);
   }
   
   return mParamNames;
}


//------------------------------------------------------------------------------
// ParameterPtrArray GetParameters() const
//------------------------------------------------------------------------------
/**
 * @return array of parameters
 */
//------------------------------------------------------------------------------
ParameterPtrArray ParameterDatabase::GetParameters() const
{
   ParameterPtrArray parameters;
   StringParamPtrMap::iterator pos;
   
   for (pos = mStringParamPtrMap->begin(); pos != mStringParamPtrMap->end(); ++pos)
   {
      parameters.push_back(pos->second);
   }
   
   return parameters;
}


//------------------------------------------------------------------------------
// bool HasParameter(const wxString &name) const
//------------------------------------------------------------------------------
/**
 * @return true if database has the parameter name, false otherwise
 */
//------------------------------------------------------------------------------
bool ParameterDatabase::HasParameter(const wxString &name) const
{
   bool found = false;
   StringParamPtrMap::iterator pos;
   
   pos = mStringParamPtrMap->find(name);
   
   if (pos != mStringParamPtrMap->end())
      found = true;
   
   return found;
}


//------------------------------------------------------------------------------
// bool RenameParameter(const wxString &oldName, const wxString &newName)
//------------------------------------------------------------------------------
/**
 * @param <oldName> parameter name to be renamed
 * @param <newName> new prameter name
 *
 * @return true if parameter is renamed, false otherwise
 */
//------------------------------------------------------------------------------
bool ParameterDatabase::RenameParameter(const wxString &oldName,
                                        const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("ParameterDatabase::RenameParameter() oldName=%s, newName=%s\n"),
       oldName.c_str(), newName.c_str());
   #endif
   
   StringArray paramNames = GetNamesOfParameters();
   wxString::size_type pos;
   StringParamPtrMap::iterator pos1;
   wxString newParamName;
   
   for (UnsignedInt i=0; i<paramNames.size(); i++)
   {
      pos = paramNames[i].find(oldName);
      
      // if oldName found
      if (pos != paramNames[i].npos)
      {
         newParamName = paramNames[i];
         newParamName.replace(pos, oldName.size(), newName);
         
         pos1 = mStringParamPtrMap->find(paramNames[i]);
         
         if (pos1 != mStringParamPtrMap->end())
         {
            // add new parameter name key and delete old
            Add(newParamName, pos1->second);
            mStringParamPtrMap->erase(pos1);
         }
      }
   }
      
   return true;
}


//------------------------------------------------------------------------------
// Integer GetParameterCount(const wxString &name) const
//------------------------------------------------------------------------------
/**
 * @return number of data elements of given parameter name
 */
//------------------------------------------------------------------------------
Integer ParameterDatabase::GetParameterCount(const wxString &name) const
{
   StringParamPtrMap::iterator pos;
   
   pos = mStringParamPtrMap->find(name);
   if (pos == mStringParamPtrMap->end())
      throw ParameterDatabaseException
         (wxT("ParameterDatabase::GetParameterCount() Parameter name ") + name +
          wxT(" not found in the database"));
   else
      return pos->second->GetParameterCount();
}


//------------------------------------------------------------------------------
// Parameter* GetParameter(const wxString &name) const
//------------------------------------------------------------------------------
/**
 * @return parameter object of given parameter name
 */
//------------------------------------------------------------------------------
Parameter* ParameterDatabase::GetParameter(const wxString &name) const
{
   StringParamPtrMap::iterator pos;
   
   pos = mStringParamPtrMap->find(name);
   if (pos == mStringParamPtrMap->end())
      throw ParameterDatabaseException
         (wxT("ParameterDatabase::GetParameter() Cannot find Parameter name \"") + name +
          wxT("\" in the Database"));
   else
      return pos->second;
}


//------------------------------------------------------------------------------
// wxString GetFirstParameterName() const
//------------------------------------------------------------------------------
wxString ParameterDatabase::GetFirstParameterName() const
{
   StringParamPtrMap::iterator pos;
   pos = mStringParamPtrMap->begin();
   return pos->first;
}


//------------------------------------------------------------------------------
// bool SetParameter(const wxString &name, Parameter *param)
//------------------------------------------------------------------------------
bool ParameterDatabase::SetParameter(const wxString &name, Parameter *param)
{
   StringParamPtrMap::iterator pos;
   
   pos = mStringParamPtrMap->find(name);
   if (pos == mStringParamPtrMap->end())
      throw ParameterDatabaseException
         (wxT("ParameterDatabase::SetParameter() Parameter name ") + name +
          wxT(" not found in the database\n"));

   pos->second = param;
   return true;
}


//*********************************
// for Add, Remove 
//*********************************

//------------------------------------------------------------------------------
// void Add(Parameter *param)
//------------------------------------------------------------------------------
void ParameterDatabase::Add(Parameter *param)
{
   if (param != NULL)
   {
      wxString name = param->GetName();
      Add(name, param);
   }
   else
   {
      throw ParameterDatabaseException
         (wxT("ParameterDatabase::Add() Cannot add NULL Parameter\n"));
   }
}


//------------------------------------------------------------------------------
// void Add(const wxString &name, Parameter *param = NULL)
//------------------------------------------------------------------------------
void ParameterDatabase::Add(const wxString &name, Parameter *param)
{
   #ifdef DEBUG_PARAMDB_ADD
   MessageInterface::ShowMessage
      (wxT("ParameterDatabase::Add() <%p> entered, name='%p', param=<%p>'%s'\n"),
       this, name.c_str(), param);
   #endif
   
   StringParamPtrMap::iterator pos;
   
   pos = mStringParamPtrMap->find(name);
   
   //if name already in the database, just ignore
   if (pos == mStringParamPtrMap->end())
   {
      mStringParamPtrMap->insert(StringParamPtrPair(name, param));
      mNumParams = mStringParamPtrMap->size();
      #ifdef DEBUG_PARAMDB_ADD
      MessageInterface::ShowMessage(wxT("   '%s' added to the map\n"));
      #endif
   }
   else
   {
      #ifdef DEBUG_PARAMDB_ADD
      MessageInterface::ShowMessage(wxT("   '%s' already in the map, so ignored\n"));
      #endif
   }
}


//------------------------------------------------------------------------------
// void Remove(const Parameter *param)
//------------------------------------------------------------------------------
void ParameterDatabase::Remove(const Parameter *param)
{
   Remove(param->GetName());
}


//------------------------------------------------------------------------------
// void Remove(const wxString &name)
//------------------------------------------------------------------------------
void ParameterDatabase::Remove(const wxString &name)
{
   StringParamPtrMap::iterator pos;
   
   pos = mStringParamPtrMap->find(name);
   if (pos == mStringParamPtrMap->end())
      throw ParameterDatabaseException
         (wxT("ParameterDatabase::Remove() Parameter name: ") + name +
          wxT(" not found in the database\n"));
   
   mStringParamPtrMap->erase(name);
   mNumParams = mStringParamPtrMap->size();  
}

