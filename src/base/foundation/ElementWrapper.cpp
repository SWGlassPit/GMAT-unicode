//$Id: ElementWrapper.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  ElementWrapper
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P
//
// Author: Wendy C. Shoan
// Created: 2007.01.17
//
/**
 * Implementation of the ElementWrapper class.  This is the base class for wrappers 
 * for various element types.
 *
 */
//------------------------------------------------------------------------------

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "GmatBaseException.hpp"
#include "ElementWrapper.hpp"
#include "StringUtil.hpp"          // for ReplaceName()
#include "TextParser.hpp"          // for SeparateBrackets()
#include "SolarSystem.hpp"         // for GetBody()
#include "MessageInterface.hpp"

//#define DEBUG_RENAME
//#define DEBUG_EW_SET_VALUE
//#define DEBUG_FIND_OBJECT

//---------------------------------
// static data
//---------------------------------
const Real ElementWrapper::UNDEFINED_REAL = -999.99;

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  ElementWrapper();
//---------------------------------------------------------------------------
/**
 * Constructs base ElementWrapper structures used in derived classes
 * (default constructor).
 *
 */
//---------------------------------------------------------------------------
ElementWrapper::ElementWrapper() :
   description  (wxT("")),
   wrapperType  (Gmat::NUMBER_WT)
{
}

//---------------------------------------------------------------------------
//  ElementWrapper(const ElementWrapper &er);
//---------------------------------------------------------------------------
/**
 * Constructs base ElementWrapper structures used in derived classes, by copying 
 * the input instance (copy constructor).
 *
 * @param <er>  ElementWrapper instance to copy to create wxT("this") instance.
 */
//---------------------------------------------------------------------------
ElementWrapper::ElementWrapper(const ElementWrapper &er) :
   description    (er.description),
   refObjectNames (er.refObjectNames),
   wrapperType    (er.wrapperType)
{
}

//---------------------------------------------------------------------------
//  ElementWrapper& operator=(const ElementWrapper &er)
//---------------------------------------------------------------------------
/**
 * Assignment operator for ElementWrapper structures.
 *
 * @param <er> The original that is being copied.
 *
 * @return Reference to this object
 */
//---------------------------------------------------------------------------
const ElementWrapper& ElementWrapper::operator=(const ElementWrapper &er)
{
   if (&er == this)
      return *this;

   description    = er.description;
   refObjectNames = er.refObjectNames;
   wrapperType    = er.wrapperType;

   return *this;
}

//---------------------------------------------------------------------------
//  ~ElementWrapper()
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
//---------------------------------------------------------------------------
ElementWrapper::~ElementWrapper()
{
   refObjectNames.clear();
}

//------------------------------------------------------------------------------
// wxString ToString()
//------------------------------------------------------------------------------
/**
 * @return ElementWrapper value converted to wxString.
 *
 * @exception <GmatBaseException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
wxString ElementWrapper::ToString()
{
   GmatBaseException be;
   be.SetDetails
      (wxT("ElementWrapper::ToString() has not been implemented for wrapper ")
       wxT("type %d, description of \"%s\""), wrapperType, description.c_str());
   throw be;
}

//------------------------------------------------------------------------------
// virtual ElementWrapper* Clone() const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
ElementWrapper* ElementWrapper::Clone() const
{
   GmatBaseException be;
   be.SetDetails
      (wxT("ElementWrapper::Clone() has not been implemented for wrapper ")
       wxT("type %d, description of \"%s\""), wrapperType, description.c_str());
   throw be;
}

//------------------------------------------------------------------------------
//  void SetDescription(const wxString &str)
//------------------------------------------------------------------------------
/**
 * Sets the description for the ElementWrapper object.
 */
//------------------------------------------------------------------------------
void ElementWrapper::SetDescription(const wxString &str)
{
   description = str;
   SetupWrapper();
}


//------------------------------------------------------------------------------
//  wxString  GetDescription() const
//------------------------------------------------------------------------------
/**
 * This method returns the description for the ElementWrapper object.
 *
 * @return description string for the object.
 *
 */
//------------------------------------------------------------------------------
wxString ElementWrapper::GetDescription() const
{
   return description;
}

//------------------------------------------------------------------------------
//  Gmat::WrapperDataType  GetWrapperType() const
//------------------------------------------------------------------------------
/**
 * This method returns the wrapper type for the ElementWrapper object.
 *
 * @return wrapper type for the object.
 *
 */
//------------------------------------------------------------------------------
Gmat::WrapperDataType ElementWrapper::GetWrapperType() const
{
   return wrapperType;
}

//------------------------------------------------------------------------------
// virtual void ClearRefObjectNames()
//------------------------------------------------------------------------------
void ElementWrapper::ClearRefObjectNames()
{
   #ifdef DEBUG_EW_SET_VALUE
      MessageInterface::ShowMessage(wxT("Clearing reference object names\n"));
   #endif
   refObjectNames.clear();
}


//------------------------------------------------------------------------------
//  const StringArray&  GetRefObjectNames() 
//------------------------------------------------------------------------------
/**
 * This method returns the list of reference object names for the ElementWrapper 
 * object.
 *
 * @return list of reference object names for the object.
 *
 */
//------------------------------------------------------------------------------
const StringArray& ElementWrapper::GetRefObjectNames()
{
   return refObjectNames;
}


//------------------------------------------------------------------------------
// virtual bool SetRefObjectName(const wxString &name, Integer index)
//------------------------------------------------------------------------------
/**
 * This method sets a reference object name for the ElementWrapper 
 * object.
 * 
 * @param <name> name of the ref object to set
 * @param <index> index of ref object name to set (0)
 *
 * @return true for success; false for failure.
 *
 */
//------------------------------------------------------------------------------
bool ElementWrapper::SetRefObjectName(const wxString &name, Integer index)
{
   return false;
}


//------------------------------------------------------------------------------
//  GmatBase* GetRefObject(const wxString &name = wxT(""))
//------------------------------------------------------------------------------
/**
 * This method retrives a reference object for the wrapper name
 * 
 * @param <name> name of the wrapper
 *
 * @return reference for success; NULL if name not found
 *
 */
//------------------------------------------------------------------------------
GmatBase* ElementWrapper::GetRefObject(const wxString &name)
{
   return NULL;
}


//------------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * This method sets a reference object for the ElementWrapper 
 * object.
 * 
 * @param <obj> pointer to the object.
 *
 * @return true for success; false for failure.
 *
 */
//------------------------------------------------------------------------------
bool ElementWrapper::SetRefObject(GmatBase *obj)
{
   return false;
}


//---------------------------------------------------------------------------
//  bool RenameObject(const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
/**
 * Method to rename a reference object for the wrapper.
 *
 * @return true if successful; false otherwise.
 */
//---------------------------------------------------------------------------
bool ElementWrapper::RenameObject(const wxString &oldName, 
                                  const wxString &newName)
{
   #ifdef DEBUG_RENAME
      MessageInterface::ShowMessage(
      wxT("Now entering EW::RenameObject with oldName = %s\n and newName = %s\n"),
      oldName.c_str(), newName.c_str());
   #endif
   // replace the old name with the new name in the list of ref objects
   Integer sz = refObjectNames.size();
   for (Integer j=0; j < sz; j++)
   {
      if (refObjectNames[j].find(oldName) != oldName.npos)
      {
         #ifdef DEBUG_RENAME
         MessageInterface::ShowMessage
            (wxT("   old refObjectNames[%d]=<%s>\n"), j, refObjectNames[j].c_str());
         #endif
         
         refObjectNames[j] =
            GmatStringUtil::ReplaceName(refObjectNames[j], oldName, newName);
         
         #ifdef DEBUG_RENAME
         MessageInterface::ShowMessage
            (wxT("   new refObjectNames[%d]=<%s>\n"), j, refObjectNames[j].c_str());
         #endif
      }
   }
   return true;
}


bool ElementWrapper::TakeRequiredAction() const
{
   return true;
}

//---------------------------------------------------------------------------
// const Rmatrix& EvaluateArray() const
//---------------------------------------------------------------------------
const Rmatrix& ElementWrapper::EvaluateArray() const
{
   throw GmatBaseException(
      wxT("In ElementWrapper, EvaluateArray() method not valid for wrapper of non-Array type.\n"));
}

//---------------------------------------------------------------------------
// bool SetArray(const Rmatrix &toValue)
//---------------------------------------------------------------------------
bool ElementWrapper::SetArray(const Rmatrix &toValue)
{
   throw GmatBaseException(
      wxT("In ElementWrapper, SetArray() method not valid for wrapper of non-Array type.\n"));
}

//---------------------------------------------------------------------------
// wxString EvaluateString() const
//---------------------------------------------------------------------------
wxString ElementWrapper::EvaluateString() const
{
   throw GmatBaseException(
      wxT("In ElementWrapper, EvaluateString() method not valid for wrapper of non-String type.\n"));
}

//---------------------------------------------------------------------------
// bool SetString(const wxString &toValue)
//---------------------------------------------------------------------------
bool ElementWrapper::SetString(const wxString &toValue)
{
   throw GmatBaseException(
      wxT("In ElementWrapper, SetString() method not valid for wrapper of non-String type.\n"));
}

//---------------------------------------------------------------------------
// wxString EvaluateOnOff() const
//---------------------------------------------------------------------------
wxString ElementWrapper::EvaluateOnOff() const
{
   throw GmatBaseException(
      wxT("In ElementWrapper, EvaluateOnOff() method not valid for wrapper of non-OnOff type.\n"));
}

//---------------------------------------------------------------------------
// bool SetOnOff(const wxString &toValue)
//---------------------------------------------------------------------------
bool ElementWrapper::SetOnOff(const wxString &toValue)
{
   throw GmatBaseException(
      wxT("In ElementWrapper, SetOnOff() method not valid for wrapper of non-OnOff type.\n"));
}

//---------------------------------------------------------------------------
// bool EvaluateBoolean() const
//---------------------------------------------------------------------------
bool ElementWrapper::EvaluateBoolean() const
{
   throw GmatBaseException(
      wxT("In ElementWrapper, EvaluateBoolean() method not valid for wrapper of non-Boolean type.\n"));
}

//---------------------------------------------------------------------------
// bool SetBoolean(const bool toValue)
//---------------------------------------------------------------------------
bool ElementWrapper::SetBoolean(const bool toValue)
{
   throw GmatBaseException(
      wxT("In ElementWrapper, SetBoolean() method not valid for wrapper of non-Boolean type.\n"));
}

//---------------------------------------------------------------------------
// Integer EvaluateInteger() const
//---------------------------------------------------------------------------
Integer ElementWrapper::EvaluateInteger() const
{
   throw GmatBaseException(
      wxT("In ElementWrapper, EvaluateInteger() method not valid for wrapper of non-Integer type.\n"));
}

//---------------------------------------------------------------------------
// bool SetInteger(const Integer toValue)
//---------------------------------------------------------------------------
bool ElementWrapper::SetInteger(const Integer toValue)
{
   throw GmatBaseException(
      wxT("In ElementWrapper, SetInteger() method not valid for wrapper of non-Integer type.\n"));
}

//---------------------------------------------------------------------------
// GmatBase* EvaluateObject() const
//---------------------------------------------------------------------------
GmatBase* ElementWrapper::EvaluateObject() const
{
   throw GmatBaseException(
      wxT("In ElementWrapper, EvaluateObject() method not valid for wrapper of non-Object type.\n"));
}


//---------------------------------------------------------------------------
// bool SetObject(GmatBase *obj)
//---------------------------------------------------------------------------
bool ElementWrapper::SetObject(GmatBase *obj)
{
   throw GmatBaseException(
      wxT("In ElementWrapper, SetObject() method not valid for wrapper of non-Object type.\n"));
}


//---------------------------------------------------------------------------
// static bool SetValue(ElementWrapper *lhsWrapper, ElementWrapper *rhsWrapper,
//                      SolarSystem *solarSys, ObjectMap *objMap,
//                      ObjectMap *globalObjMap, bool setRefObj = true)
//---------------------------------------------------------------------------
/*
 * static function to set value from rhs wrapper to lhs wrapper.
 */
//---------------------------------------------------------------------------
bool ElementWrapper::SetValue(ElementWrapper *lhsWrapper, ElementWrapper *rhsWrapper,
                              SolarSystem *solarSys, ObjectMap *objMap,
                              ObjectMap *globalObjMap, bool setRefObj)
{
   #ifdef DEBUG_EW_SET_VALUE
   MessageInterface::ShowMessage
      (wxT("ElemementWrapper::SetValue() entered, lhsWrapper=<%p>, rhsWrapper=<%p>\n   ")
       wxT("solarSys=<%p> objMap=<%p>, globalObjMap=<%p>, setRefObj=%d\n"), lhsWrapper,
       rhsWrapper, solarSys, objMap, globalObjMap, setRefObj);
   #endif
   
   if (lhsWrapper == NULL || rhsWrapper == NULL)
      return false;
   
   wxString lhs = lhsWrapper->GetDescription();
   wxString rhs = rhsWrapper->GetDescription();
   
   #ifdef DEBUG_EW_SET_VALUE
   MessageInterface::ShowMessage(wxT("   lhs=\"%s\"\n   rhs=\"%s\"\n"), lhs.c_str(), rhs.c_str());
   #endif
   
   Real rval = -99999.999;
   Integer ival = -99999;
   bool bval = false;
   wxString sval = wxT("UnknownValue");
   Rmatrix rmat;
   GmatBase *rhsObj = NULL;
   
   Gmat::ParameterType lhsDataType = lhsWrapper->GetDataType();
   Gmat::ParameterType rhsDataType = Gmat::UNKNOWN_PARAMETER_TYPE;
   wxString lhsTypeStr = wxT("UnknownDataType");
   if (lhsDataType != Gmat::UNKNOWN_PARAMETER_TYPE)
      lhsTypeStr = GmatBase::PARAM_TYPE_STRING[lhsDataType];
   wxString rhsTypeStr = wxT("UnknownDataType");
   Gmat::WrapperDataType lhsWrapperType = lhsWrapper->GetWrapperType();
   Gmat::WrapperDataType rhsWrapperType = Gmat::UNKNOWN_WRAPPER_TYPE;
   
   #ifdef DEBUG_EW_SET_VALUE
   MessageInterface::ShowMessage
      (wxT("   lhsWrapperType=%2d, lhsDataType=%s\n"), lhsWrapperType, lhsTypeStr.c_str());
   #endif
   
   try
   {
      rhsDataType = rhsWrapper->GetDataType();
      rhsTypeStr = GmatBase::PARAM_TYPE_STRING[rhsDataType];
      rhsWrapperType = rhsWrapper->GetWrapperType();
      
      lhsWrapper->TakeRequiredAction();

      #ifdef DEBUG_EW_SET_VALUE
      MessageInterface::ShowMessage
         (wxT("   rhsWrapperType=%2d, rhsDataType=%s\n"), rhsWrapperType, rhsTypeStr.c_str());
      #endif
      
      // If lhs is a String, it must be String Object and STRING_OBJECT_WT,
      // so check it first for error condition. ex) UnknownObj1 = str1
      if (lhsDataType == Gmat::STRING_TYPE && lhsWrapperType == Gmat::STRING_WT)
      {
         GmatBaseException ex;
         //ex.SetDetails(wxT("ElementWrapper::SetValue() Cannot set \"%s\" to unknown ")
         //              wxT("object \"%s\""), rhs.c_str(), lhs.c_str());
         ex.SetDetails(wxT("ElementWrapper::SetValue() Cannot set unknown object \"%s\" to ")
                       wxT("\"%s\""), lhs.c_str(), rhs.c_str());
         throw ex;
      }
      
      switch (rhsDataType)
      {
      case Gmat::BOOLEAN_TYPE:
         bval = rhsWrapper->EvaluateBoolean();
         break;
      case Gmat::INTEGER_TYPE:
         ival = rhsWrapper->EvaluateInteger();
         break;
      case Gmat::REAL_TYPE:
         rval = rhsWrapper->EvaluateReal();
         #ifdef DEBUG_EW_SET_VALUE
         MessageInterface::ShowMessage(wxT("   REAL_TYPE rhs rval=%f\n"), rval);
         #endif
         break;
      case Gmat::RMATRIX_TYPE:
         rmat = rhsWrapper->EvaluateArray();
         break;
      case Gmat::STRING_TYPE:
      case Gmat::ENUMERATION_TYPE:
      case Gmat::FILENAME_TYPE:
         sval = rhsWrapper->EvaluateString();
         sval = GmatStringUtil::RemoveEnclosingString(sval, wxT("'"));
         break;
      case Gmat::ON_OFF_TYPE:
         sval = rhsWrapper->EvaluateOnOff();
         break;
      case Gmat::OBJECT_TYPE:
         rhsObj = rhsWrapper->EvaluateObject();
         break;
      default:
         throw GmatBaseException
            (wxT("ElementWrapper::SetValue() RHS type is unknown for \"") + rhs + wxT("\""));
      }
      
      #ifdef DEBUG_EW_SET_VALUE
      MessageInterface::ShowMessage
         (wxT("   ==> Now assign \"%s\" to \"%s\", rhsObj=<%p>, sval='%s'\n"),
          rhs.c_str(), lhs.c_str(), rhsObj, sval.c_str());
      #endif
      
      // Now assign to LHS
      switch (lhsDataType)
      {
      case Gmat::BOOLEAN_TYPE:
         {
            lhsWrapper->SetBoolean(bval);
            break;
         }
      case Gmat::BOOLEANARRAY_TYPE:
         {
            if (rhsDataType == Gmat::STRING_TYPE)
               lhsWrapper->SetString(rhs);
            else
               throw GmatBaseException
                  (wxT("ElementWrapper::SetValue() Cannot set \"") + lhs + wxT("\" to ") + rhs + wxT("\""));
            break;
         }
      case Gmat::INTEGER_TYPE:
         {
            // Since it always creates NumberWrapper for numbers,
            // check both Integer and Real types
            if (rhsDataType == Gmat::INTEGER_TYPE)
            {
               lhsWrapper->SetInteger(ival);
            }
            else if (rhsDataType == Gmat::REAL_TYPE)
            {
               Integer itempval;
               wxString desc = rhs;
               if (GmatStringUtil::ToInteger(desc, itempval))
                  lhsWrapper->SetInteger(itempval);
               else
                  throw GmatBaseException
                     (wxT("ElementWrapper::SetValue() Cannot set \"") + lhs + wxT("\" to ") + rhs + wxT("\""));
            }
            break;
         }
      case Gmat::UNSIGNED_INTARRAY_TYPE:
         {
            if (rhsDataType == Gmat::STRING_TYPE)
               lhsWrapper->SetString(rhs);
            else
               throw GmatBaseException
                  (wxT("ElementWrapper::SetValue() Cannot set \"") + lhs + wxT("\" to ") + rhs + wxT("\""));
            break;
         }
      case Gmat::RVECTOR_TYPE:
         {
            if (rhsDataType == Gmat::STRING_TYPE)
               lhsWrapper->SetString(rhs);
            else
               throw GmatBaseException
                  (wxT("ElementWrapper::SetValue() Cannot set \"") + lhs + wxT("\" to ") + rhs + wxT("\""));
            break;
         }
      case Gmat::REAL_TYPE:
         {
            #ifdef DEBUG_EW_SET_VALUE
            MessageInterface::ShowMessage(wxT("   setting rhs rval=%f to lhs\n"), rval);
            #endif
            bool valueSet = false;
            if (rval != -99999.999)
            {
               lhsWrapper->SetReal(rval);
               valueSet = true;
            }
            else if (rhsDataType == Gmat::RMATRIX_TYPE)
            {
               if (rmat.GetNumRows() == 1 && rmat.GetNumColumns() == 1)
               {
                  Real val = rmat.GetElement(0, 0);
                  lhsWrapper->SetReal(val);
                  valueSet = true;
               }
            }
            
            if (!valueSet)
            {
               throw GmatBaseException
                  (wxT("ElementWrapper::SetValue() Cannot set \"") + lhs + wxT("\" to ") + rhs + wxT("\""));
            }
            break;
         }
      case Gmat::RMATRIX_TYPE:
         lhsWrapper->SetArray(rmat);
         break;
      case Gmat::STRING_TYPE:
      case Gmat::ENUMERATION_TYPE:
      case Gmat::FILENAME_TYPE:
         // Object to String is needed for Remove for Formation
         if (rhsObj != NULL)
         {
            lhsWrapper->SetString(rhsObj->GetName());
         }
         else if ((rhsDataType == Gmat::STRING_TYPE ||
                   rhsDataType == Gmat::ENUMERATION_TYPE ||
                   rhsDataType == Gmat::FILENAME_TYPE ||
                   rhsDataType == Gmat::ON_OFF_TYPE))
         {
            lhsWrapper->SetString(sval);
         }
         // We don't want to allow Variable or Array element to STRING assignment
         else if (rhsDataType == Gmat::REAL_TYPE &&
                  rhsWrapperType != Gmat::VARIABLE_WT &&
                  rhsWrapperType != Gmat::ARRAY_ELEMENT_WT)
         {
            lhsWrapper->SetString(rhs);
         }
         else
         {
            // Handle setting real value to string here
            // This fixes Bug 1340 (LOJ: 2009.10.19)
            if (rhsDataType == Gmat::REAL_TYPE)
            {
               sval = GmatStringUtil::ToString(rval, 16);
               #ifdef DEBUG_EW_SET_VALUE
               MessageInterface::ShowMessage
                  (wxT("   %f converted to string '%s'\n"), rval, sval.c_str());
               #endif
               lhsWrapper->SetString(sval);
               break;
            }
            else if (rhsDataType == Gmat::INTEGER_TYPE)
            {
               sval = GmatStringUtil::ToString(ival);
               #ifdef DEBUG_EW_SET_VALUE
               MessageInterface::ShowMessage
                  (wxT("   %d converted to string '%s'\n"), ival, sval.c_str());
               #endif
               lhsWrapper->SetString(sval);
               break;
            }
            else if (rhsDataType == Gmat::BOOLEAN_TYPE)
            {
               sval = GmatStringUtil::ToString(bval);
               #ifdef DEBUG_EW_SET_VALUE
               MessageInterface::ShowMessage
                  (wxT("   %s converted to string '%s'\n"), bval ? wxT("true") : wxT("false"),
                   sval.c_str());
               #endif
               lhsWrapper->SetString(sval);
               break;
            }
            else
            {
               GmatBaseException ex;
               if (rhsObj != NULL)
                  ex.SetDetails(wxT("ElementWrapper::SetValue() Cannot set undefined object ")
                                wxT("\"%s\" to object \"%s\""), lhs.c_str(),
                                rhsObj->GetTypeName().c_str());
               else if (lhsWrapperType == Gmat::STRING_OBJECT_WT &&
                        rhsWrapperType == Gmat::VARIABLE_WT)
                  ex.SetDetails(wxT("ElementWrapper::SetValue() Cannot set String \"%s\" to ")
                                wxT("Variable \"%s\""), lhs.c_str(), rhs.c_str());
               else
                  ex.SetDetails(wxT("ElementWrapper::SetValue() Cannot set undefined object ")
                                wxT("\"%s\" to \"%s\""), lhs.c_str(), rhs.c_str());
               throw ex;
            }
         }
         break;
      case Gmat::ON_OFF_TYPE:
         lhsWrapper->SetOnOff(sval);
         break;
      case Gmat::OBJECT_TYPE:
         if (rhsObj == NULL)
         {
            // Handle special case for wxT("DefaultFM.Drag = None;")
            if (rhsDataType == Gmat::STRING_TYPE)
            {
               #ifdef DEBUG_EW_SET_VALUE
               MessageInterface::ShowMessage(wxT("   calling lhsWrapper->SetString(rhs)\n"));
               #endif
               
               // Show more meaningful message
               try
               {
                  lhsWrapper->SetString(rhs);
               }
               catch (BaseException &)
               {
                  // Show more meaningful message from the wrapper ref object (LOJ: 2011.02.17)
                  throw;
               }
            }
            // Handle case like wxT("XYPlot1.IndVar = sat.A1ModJulian;")
            else if (rhsWrapperType == Gmat::PARAMETER_WT)
            {
               lhsWrapper->SetObject(rhsWrapper->GetRefObject());
            }
            else
               throw GmatBaseException
                  (wxT("ElementWrapper::SetValue() Cannot set object \"") + lhs +
                   wxT("\" to non-object type \"") + rhs + wxT("\""));
         }
         else
         {            
            // check if ref object can be set to lhs
            if (setRefObj)
            {
               #ifdef DEBUG_EW_SET_VALUE
               MessageInterface::ShowMessage(wxT("   calling lhsWrapper->SetObject(rhsObj)\n"));
               #endif
               lhsWrapper->SetObject(rhsObj);
            }
            else
            {
               #ifdef DEBUG_EW_SET_VALUE
               MessageInterface::ShowMessage(wxT("   calling lhsWrapper->SetString(rhsObjName)\n"));
               #endif
               lhsWrapper->SetString(rhsObj->GetName());
            }
         }
         break;
      case Gmat::STRINGARRAY_TYPE:                  
         if (rhsObj != NULL)
         {
            #ifdef DEBUG_EW_SET_VALUE
            MessageInterface::ShowMessage(wxT("   calling lhsWrapper->SetString(rhsObj->GetName())\n"));
            #endif
            lhsWrapper->SetString(rhsObj->GetName());
         }
         else
         {
            #ifdef DEBUG_EW_SET_VALUE
            MessageInterface::ShowMessage(wxT("   calling lhsWrapper->SetString(rhs or sval)\n"));
            #endif
            if (sval == wxT("UnknownValue"))
               lhsWrapper->SetString(rhs);
            else
               lhsWrapper->SetString(sval);
            
            // Commented out to handle SolarSystem.Ephemeris = {SLP} (loj: 2008.07.16)            
            //GmatBaseException ex;
            //ex.SetDetails(wxT("ElementWrapper::SetValue() Cannot set \"%s\" to \"%s\""),
            //              rhs.c_str(), lhs.c_str());
            //throw ex;
         }
         break;
      case Gmat::OBJECTARRAY_TYPE:
         // Object to String is needed for Add for Subscribers/Formation
         if (rhsObj != NULL)
            lhsWrapper->SetObject(rhsObj);
         else
         {
            bool errorCond = true;
            
            // Handle case like wxT("GMAT XYPlot1.Add = {sat.X sat.Y};")
            // Set individually if RHS has more than one object
            StringArray rhsValues;
            TextParser tp;
            rhsValues = tp.SeparateBrackets(rhs, wxT("{}"), wxT(" ,"), false);
            
            #ifdef DEBUG_EW_SET_VALUE
            MessageInterface::ShowMessage(wxT("   rhs {} has %d items\n"), rhsValues.size());
            #endif
            
            wxString firstTypeStr, currTypeStr;
            for (UnsignedInt i=0; i<rhsValues.size(); i++)
            {
               #ifdef DEBUG_EW_SET_VALUE
               MessageInterface::ShowMessage(wxT("   rhsValues[%d]=<%s>\n"), i, rhsValues[i].c_str());
               #endif
               
               // Remove enclosing single quotes (LOJ: 2009.10.09)
               rhsValues[i] = GmatStringUtil::RemoveEnclosingString(rhsValues[i], wxT("'"));
               
               GmatBase *obj = FindObject(rhsValues[i], solarSys, objMap, globalObjMap);
               
               #ifdef DEBUG_EW_SET_VALUE
               MessageInterface::ShowMessage
                  (wxT("   obj=<%p><%s>'%s'\n"), obj, obj->GetTypeName().c_str(), obj->GetName().c_str());
               #endif
               
               if (obj == NULL)
               {
                  errorCond = true;
                  break;
               }
               
               #ifdef DEBUG_EW_SET_VALUE
               MessageInterface::ShowMessage
                  (wxT("   calling lhsWrapper->SetObject(obj), wrapperType=%d\n"),
                   lhsWrapper->GetWrapperType());
               #endif
               
               lhsWrapper->SetObject(obj);
               errorCond = false;
            }
            
            // To handle Earth2Body.PointMasses = {}, check for empty items
            if (errorCond && rhsValues.size() > 0)
            {
               GmatBaseException ex;
               ex.SetDetails
                  (wxT("ElementWrapper::SetValue() Cannot set \"%s\" to \"%s\""),
                   lhs.c_str(), rhs.c_str());
               throw ex;
            }
         }
         break;
      default:
         throw GmatBaseException
            (wxT("ElementWrapper::SetValue() LHS type is unknown for \"") + lhs + wxT("\""));
      }
      
      #ifdef DEBUG_EW_SET_VALUE
      MessageInterface::ShowMessage(wxT("ElemementWrapper::SetValue() (1)returning true\n"));
      #endif
      return true;
   }
   catch (BaseException &)
   {
      // anyting to add here?
      throw;
   }
   
   #ifdef DEBUG_EW_SET_VALUE
   MessageInterface::ShowMessage(wxT("ElemementWrapper::SetValue() (2)returning true\n"));
   #endif
   
   return true;
}


//---------------------------------------------------------------------------
// static GmatBase* FindObject(const wxString &name, SolarSystem *solarSys,
//                             ObjectMap *objMap, ObjectMap *globalObjMap)
//---------------------------------------------------------------------------
GmatBase* ElementWrapper::FindObject(const wxString &name, SolarSystem *solarSys,
                                     ObjectMap *objMap, ObjectMap *globalObjMap)
{
   wxString newName = name;
   
   // Ignore array indexing of Array
   wxString::size_type index = name.find(wxT('('));
   if (index != name.npos)
      newName = name.substr(0, index);
   
   #ifdef DEBUG_FIND_OBJECT
   MessageInterface::ShowMessage
      (wxT("ElementWrapper::FindObject() entered, name='%s', newName='%s'\n"), name.c_str(),
       newName.c_str());
   #endif
   
   #ifdef DEBUG_FIND_OBJECT
   ShowObjectMaps(objMap, globalObjMap);
   #endif
   
   // Check for the object in the Local Object Store (LOS) first
   if (objMap && objMap->find(newName) != objMap->end())
      return (*objMap)[newName];
   
   // If not found in the LOS, check the Global Object Store (GOS)
   if (globalObjMap && globalObjMap->find(newName) != globalObjMap->end())
      return (*globalObjMap)[newName];
   
   // Let's try SolarSystem
   if (solarSys && solarSys->GetBody(newName))
      return (GmatBase*)(solarSys->GetBody(newName));
   
   return NULL;
}


//------------------------------------------------------------------------------
// static void ShowObjectMaps(ObjectMap *objMap, ObjectMap *globalObjMap)
//------------------------------------------------------------------------------
void ElementWrapper::ShowObjectMaps(ObjectMap *objMap, ObjectMap *globalObjMap)
{
   MessageInterface::ShowMessage
      (wxT("ElementWrapper::ShowObjectMaps() objMap=<%p>, globalObjMap=<%p>\n"),
       objMap, globalObjMap);
   
   if (objMap)
   {
      MessageInterface::ShowMessage(wxT("Here is the local object map:\n"));
      for (std::map<wxString, GmatBase *>::iterator i = objMap->begin();
           i != objMap->end(); ++i)
         MessageInterface::ShowMessage
            (wxT("   %30s  <%p><%s>\n"), i->first.c_str(), i->second,
             i->second == NULL ? wxT("NULL") : (i->second)->GetTypeName().c_str());
   }
   if (globalObjMap)
   {
      MessageInterface::ShowMessage(wxT("Here is the global object map:\n"));
      for (std::map<wxString, GmatBase *>::iterator i = globalObjMap->begin();
           i != globalObjMap->end(); ++i)
         MessageInterface::ShowMessage
            (wxT("   %30s  <%p><%s>\n"), i->first.c_str(), i->second,
             i->second == NULL ? wxT("NULL") : (i->second)->GetTypeName().c_str());
   }
}

