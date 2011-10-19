//$Id: ParameterWrapper.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  ParameterWrapper
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
// Created: 2007.01.18
//
/**
 * Implementation of the ParameterWrapper class.
 *
 */
//------------------------------------------------------------------------------

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "ParameterWrapper.hpp"
#include "ParameterException.hpp"

#include "MessageInterface.hpp"

//#define DEBUG_PW
//#define DEBUG_PW_REFOBJ
//#define DEBUG_PARAMETER_WRAPPER

//---------------------------------
// static data
//---------------------------------
// none at this time

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  ParameterWrapper();
//---------------------------------------------------------------------------
/**
 * Constructs ParameterWrapper structures
 * (default constructor).
 *
 */
//---------------------------------------------------------------------------
ParameterWrapper::ParameterWrapper() :
   ElementWrapper(),
   param         (NULL)
{
   wrapperType = Gmat::PARAMETER_WT;
}

//---------------------------------------------------------------------------
//  ParameterWrapper(const ParameterWrapper &pw);
//---------------------------------------------------------------------------
/**
 * Constructs base ParameterWrapper structures used in derived classes, by 
 * copying the input instance (copy constructor).
 *
 * @param <pw>  ParameterWrapper instance to copy to create wxT("this") instance.
 */
//---------------------------------------------------------------------------
ParameterWrapper::ParameterWrapper(const ParameterWrapper &pw) :
   ElementWrapper(pw),
   param         (pw.param)
{
}

//---------------------------------------------------------------------------
//  ParameterWrapper& operator=(const ParameterWrapper &pw)
//---------------------------------------------------------------------------
/**
 * Assignment operator for ParameterWrapper structures.
 *
 * @param <pw> The original that is being copied.
 *
 * @return Reference to this object
 */
//---------------------------------------------------------------------------
const ParameterWrapper& ParameterWrapper::operator=(const ParameterWrapper &pw)
{
   if (&pw == this)
      return *this;
   
   ElementWrapper::operator=(pw);
   param = pw.param;
   
   return *this;
}

//---------------------------------------------------------------------------
//  ~ParameterWrapper()
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
//---------------------------------------------------------------------------
ParameterWrapper::~ParameterWrapper()
{
   #ifdef DEBUG_PW_REFOBJ
   MessageInterface::ShowMessage
      (wxT("ParameterWrapper::~ParameterWrapper() <%p>'%s' entered, param=<%p><%s>\n"),
       this, description.c_str(), param, param ? param->GetName().c_str() : wxT("NULL"));
   #endif
}

//------------------------------------------------------------------------------
// virtual ElementWrapper* Clone() const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
ElementWrapper* ParameterWrapper::Clone() const
{
   return new ParameterWrapper(*this);
}

//------------------------------------------------------------------------------
//  Gmat::ParameterType GetDataType() const
//------------------------------------------------------------------------------
/**
 * This method returns the data type for the ParameterWrapper object.
 *
 * @return data type for the object.
 *
 */
//------------------------------------------------------------------------------
Gmat::ParameterType ParameterWrapper::GetDataType() const
{
   // Changed to return Parameter return type (loj: 2008.06.18)
   if (param)
      return param->GetReturnType();
   
   // What type should we return here?
   return Gmat::REAL_TYPE;
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
GmatBase* ParameterWrapper::GetRefObject(const wxString &name)
{
   // We don't need to check for the name since only one Parameter
   return (GmatBase*)param;
}

//---------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj)
//---------------------------------------------------------------------------
/**
 * Method to set the reference object (Parameter) pointer on the wrapped 
 * object.
 *
 * @return true if successful; false otherwise.
 */
//---------------------------------------------------------------------------
bool ParameterWrapper::SetRefObject(GmatBase *obj)
{
   #ifdef DEBUG_PW_REFOBJ
   MessageInterface::ShowMessage
      (wxT("ParameterWrapper::SetRefObject() obj=<%p><%s>\n"), obj,
       obj ? obj->GetName().c_str() : wxT("NULL"));
   #endif
   
//   if ( (obj->GetName() != refObjectNames[0]) ||
//        (!obj->IsOfType(wxT("Parameter"))) )
//   {
//      wxString errmsg = wxT("Referenced object \"");
//      errmsg += obj->GetName();
//      errmsg += wxT("\" was passed into the parameter wrapper \"");
//      errmsg += description;
//      errmsg += wxT("\", but an object named \"");
//      errmsg += refObjectNames[0];
//      errmsg += wxT("\" was expected.\n");
//      throw ParameterException(errmsg);
//   }
   if ( (obj->GetName() == refObjectNames[0]) || (obj->IsOfType(wxT("Parameter"))) )
   {
      param = (Parameter*) obj;
      return true;
   }
   else return false;
}

//---------------------------------------------------------------------------
//  Real EvaluateReal() const
//---------------------------------------------------------------------------
/**
 * Method to return the Real value of the wrapped object.
 *
 * @return Real value of the wrapped number object.
 * 
 */
//---------------------------------------------------------------------------
Real ParameterWrapper::EvaluateReal() const
{
   if (param == NULL)
      throw ParameterException(
      wxT("Cannot return value of Parameter - pointer is NULL\n"));
   #ifdef DEBUG_PW
      MessageInterface::ShowMessage(
      wxT("In ParameterWrapper::EvalReal, value is %.12f\n"), param->EvaluateReal());
   #endif
   return param->EvaluateReal();
}

//---------------------------------------------------------------------------
//  bool SetReal(const Real toValue)
//---------------------------------------------------------------------------
/**
 * Method to set the Real value of the wrapped object.
 *
 * @return true if successful; false otherwise.
 */
//---------------------------------------------------------------------------
bool ParameterWrapper::SetReal(const Real toValue)
{
   if (param == NULL)
      throw ParameterException(
      wxT("Cannot set value of Parameter - pointer is NULL\n"));
   param->SetReal(toValue);
   return true;
}


//---------------------------------------------------------------------------
//  const Rmatrix& EvaluateArray() const
//---------------------------------------------------------------------------
/**
 * Method to return the Rmatrix value of the wrapped object.
 *
 * @return Rmatrix value of the wrapped number object.
 * 
 */
//---------------------------------------------------------------------------
const Rmatrix& ParameterWrapper::EvaluateArray() const
{
   if (param == NULL)
      throw ParameterException(
      wxT("Cannot return value of Parameter - pointer is NULL\n"));
   #ifdef DEBUG_PW
      MessageInterface::ShowMessage(
      wxT("In ParameterWrapper::EvaluateArray, value is %.12f\n"), param->EvaluateRmatrix());
   #endif
   return param->EvaluateRmatrix();
}


//---------------------------------------------------------------------------
//  bool SetArray(const Rmatrix &toValue)
//---------------------------------------------------------------------------
/**
 * Method to set the Real value of the wrapped object.
 *
 * @return true if successful; false otherwise.
 */
//---------------------------------------------------------------------------
bool ParameterWrapper::SetArray(const Rmatrix &toValue)
{
   if (param == NULL)
      throw ParameterException(
      wxT("Cannot set value of Parameter - pointer is NULL\n"));
   param->SetRmatrix(toValue);
   return true;
}


//------------------------------------------------------------------------------
// GmatBase* EvaluateObject() const
//------------------------------------------------------------------------------
/**
 * Method to return the Object pointer of the ParameterWrapper object.
 *
 * @return value of the ParameterWrapper object.
 * 
 */
//------------------------------------------------------------------------------
GmatBase* ParameterWrapper::EvaluateObject() const
{
   return param;
}


//------------------------------------------------------------------------------
// bool SetObject(const GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Method to set the object of the wrapped object.
 *
 * @param <obj> The object pointer to set
 * @return true if successful; false otherwise.
 */
//------------------------------------------------------------------------------
bool ParameterWrapper::SetObject(const GmatBase *obj)
{
   if (obj == NULL)
   {
      if (param == NULL)
         throw ParameterException(wxT("Cannot set undefined object to undefined object"));
      else
         throw ParameterException
            (wxT("Cannot set undefined object to object of type \"") +
             param->GetTypeName() +  wxT("\""));
   }
   
   if (param != NULL)
   {
      // Let's check the object type
      if (param->GetTypeName() == obj->GetTypeName())
      {
         #ifdef DEBUG_PARAMETER_WRAPPER
         MessageInterface::ShowMessage
            (wxT("ParameterWrapper::SetObject() fromType=%s, toType=%s\n"),
             obj->GetTypeName().c_str(), param->GetTypeName().c_str());
         #endif
         
         param->Copy(obj);
      }
      else
      {
         ParameterException pe;
         pe.SetDetails(wxT("Cannot set object of type \"%s\" to object of type \"%s\""),
                       obj->GetTypeName().c_str(), param->GetTypeName().c_str());
         throw pe;
      }
   }
   else
   {
      throw ParameterException(wxT("Cannot set Parameter \"") + obj->GetName() +
                               wxT("\" to an undefined object"));
   }
   
   return true;
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
bool ParameterWrapper::RenameObject(const wxString &oldName, 
                                    const wxString &newName)
{
   ElementWrapper::RenameObject(oldName, newName);
   description = refObjectNames[0];  
   return true;
}


//---------------------------------------------------------------------------
//  void SetupWrapper()
//---------------------------------------------------------------------------
/**
 * Method to set up the Parameter Wrapper.
 *
 */
//---------------------------------------------------------------------------
void ParameterWrapper::SetupWrapper()
{
   refObjectNames.push_back(description);
}

