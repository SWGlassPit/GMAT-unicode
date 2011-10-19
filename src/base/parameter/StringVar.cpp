//$Id: StringVar.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  StringVar
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
// Created: 2005/1/7
//
/**
 * Implements StringVar class which handles wxString value. The string value
 * is stored in Parameter::mExpr.
 */
//------------------------------------------------------------------------------

#include "StringVar.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_STRINGVAR
//#define DEBUG_GEN_STRING

//---------------------------------
// static data
//---------------------------------

const wxString
StringVar::PARAMETER_TEXT[StringVarParamCount - ParameterParamCount] =
{
   wxT("Value"),
};

const Gmat::ParameterType
StringVar::PARAMETER_TYPE[StringVarParamCount - ParameterParamCount] =
{
   Gmat::STRING_TYPE,
};


//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// StringVar(const wxString &name, const wxString &typeStr, 
//         GmatParam::ParameterKey key, GmatBase *obj, const wxString &desc,
//         const wxString &unit, GmatParam::DepObject depObj, Gmat::ObjectType,
//         bool isTimeParam)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> parameter name
 * @param <typeStr>  parameter type string
 * @param <key>  parameter key (SYSTEM_PARAM, USER_PARAM, etc)
 * @param <obj>  reference object pointer
 * @param <desc> parameter description
 * @param <unit> parameter unit
 * @param <depObj> object which parameter is dependent on (COORD_SYS, ORIGIN, NONE)
 * @param <ownerType> object type who owns this parameter as property
 * @param <isTimeParam> true if parameter is time related, false otherwise
 */
//------------------------------------------------------------------------------
StringVar::StringVar(const wxString &name, const wxString &typeStr, 
                     GmatParam::ParameterKey key, GmatBase *obj, const wxString &desc,
                     const wxString &unit, GmatParam::DepObject depObj,
                     Gmat::ObjectType ownerType, bool isTimeParam)
   : Parameter(name, typeStr, key, obj, desc, unit, depObj, ownerType, isTimeParam,
               false, false, true)
{  
   objectTypes.push_back(Gmat::STRING);
   objectTypeNames.push_back(wxT("String"));
   mStringValue = STRING_PARAMETER_UNDEFINED;
   mReturnType = Gmat::STRING_TYPE;
   // Don't set name to expression, but leave it blank if not set (LOJ: 2010.11.29)
   //mExpr = name;
}


//------------------------------------------------------------------------------
// StringVar(const StringVar &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the object being copied.
 */
//------------------------------------------------------------------------------
StringVar::StringVar(const StringVar &copy)
   : Parameter(copy)
{
   mStringValue = copy.mStringValue;   
}


//------------------------------------------------------------------------------
// StringVar& operator= (const StringVar& right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the object being copied.
 *
 * @return reference to this object
 */
//------------------------------------------------------------------------------
StringVar& StringVar::operator= (const StringVar& right)
{
   if (this != &right)
   {
      // We don't want to change the name when copy
      wxString thisName = instanceName;
      
      Parameter::operator=(right);
      mStringValue = right.mStringValue;
      // Set expression so that we can preserve string value when we write (loj: 2008.08.13)
      // Set expression to name of right side since expression is used for
      // writnig in GetGeneratingString() (loj: 2008.08.13)
      // For example:
      // str1 = 'this is str1'
      // str2 = str1;
      // We want to write wxT("str2 = str1") instead of "str2 = 'this is str1'
      mExpr = right.GetName();
      // Set depObjectName so that we can check whether to add quotes when we write (loj: 2008.08.13)
      mDepObjectName = right.GetName();
      SetName(thisName);
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// ~StringVar()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
StringVar::~StringVar()
{
}


//------------------------------------
// methods inherited from Parameter
//------------------------------------

//------------------------------------------------------------------------------
// bool operator==(const StringVar &right) const
//------------------------------------------------------------------------------
/**
 * Equal operator.
 *
 * @return true if input object's type and name are the same as this object.
 */
//------------------------------------------------------------------------------
bool StringVar::operator==(const StringVar &right) const
{
   return Parameter::operator==(right);
}


//------------------------------------------------------------------------------
// bool operator!=(const StringVar &right) const
//------------------------------------------------------------------------------
/**
 * Not equal operator.
 *
 * @return true if input object's type and name are not the same as this object.
 */
//------------------------------------------------------------------------------
bool StringVar::operator!=(const StringVar &right) const
{
   return Parameter::operator!=(right);
}


//------------------------------------------------------------------------------
// wxString ToString()
//------------------------------------------------------------------------------
/**
 * Retrieves string value of parameter.
 *
 * @return string value of parameter.
 */
//------------------------------------------------------------------------------
wxString StringVar::ToString()
{
   return mStringValue;
}


//------------------------------------------------------------------------------
// const wxString& GetString() const
//------------------------------------------------------------------------------
/**
 * Retrieves string value of parameter.
 *
 * @return string value.
 */
//------------------------------------------------------------------------------
const wxString& StringVar::GetString() const
{
   return mStringValue;
}


//------------------------------------------------------------------------------
// const wxString& EvaluateString()
//------------------------------------------------------------------------------
/**
 * Retrieves string value of parameter.
 *
 * @return string value.
 */
//------------------------------------------------------------------------------
const wxString& StringVar::EvaluateString()
{
   return mStringValue;
}


//------------------------------------
// methods inherited from GmatBase
//------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object.
 *
 * @return cloned object pointer.
 */
//------------------------------------------------------------------------------
GmatBase* StringVar::Clone() const
{
   return new StringVar(*this);
}


//------------------------------------------------------------------------------
// void Copy(const GmatBase* orig)
//------------------------------------------------------------------------------
void StringVar::Copy(const GmatBase* orig)
{
   operator=(*((StringVar *)(orig)));
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
Integer StringVar::GetParameterID(const wxString &str) const
{
   for (int i=ParameterParamCount; i<StringVarParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - ParameterParamCount])
         return i;
   }
   
   return Parameter::GetParameterID(str);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
wxString StringVar::GetStringParameter(const Integer id) const
{
   switch (id)
   {
   case VALUE:
      #ifdef DEBUG_STRINGVAR
      MessageInterface::ShowMessage
         (wxT("StringVar::GetStringParameter(%d) returning '%s'\n"), id,
          mStringValue.c_str());
      #endif
      return mStringValue;
   default:
      return Parameter::GetStringParameter(id);
   }
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString StringVar::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
bool StringVar::SetStringParameter(const Integer id, const wxString &value)
{
   #ifdef DEBUG_STRINGVAR
   MessageInterface::ShowMessage
      (wxT("StringVar::SetStringParameter() this=<%p>, id=%d, value='%s'\n"), this,
       id, value.c_str());
   #endif
   
   switch (id)
   {
   case EXPRESSION:
      mExpr = value;
      mStringValue = value; // set as initial value
      #ifdef DEBUG_STRINGVAR
      MessageInterface::ShowMessage
         (wxT("StringVar::SetStringParameter() returning true, ")
          wxT("both mExpr and mStringValue are set to '%s'\n"), value.c_str());
      #endif
      return true;
   case VALUE:
      mStringValue = value;
      #ifdef DEBUG_STRINGVAR
      MessageInterface::ShowMessage
         (wxT("StringVar::SetStringParameter() returning true, ")
          wxT("mStringValue is set to '%s'\n"), value.c_str());
      #endif
      return true;
   default:
      return Parameter::SetStringParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label,
//                         const wxString &value)
//------------------------------------------------------------------------------
bool StringVar::SetStringParameter(const wxString &label,
                                   const wxString &value)
{
   #ifdef DEBUG_STRINGVAR
   MessageInterface::ShowMessage
      (wxT("StringVar::SetStringParameter() label=%s value='%s'\n"),
       label.c_str(), value.c_str());
   #endif
   
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// const wxString& GetGeneratingString(...)
//------------------------------------------------------------------------------
/**
 * Produces a string, possibly multi-line, containing the text that produces an
 * object.
 * 
 * @param mode Specifies the type of serialization requested.
 * @param prefix Optional prefix appended to the object's name
 * @param useName Name that replaces the object's name.
 * 
 * @return A string containing the text.
 */
//------------------------------------------------------------------------------
const wxString& StringVar::GetGeneratingString(Gmat::WriteMode mode,
                                                  const wxString &prefix,
                                                  const wxString &useName)
{
   #ifdef DEBUG_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("StringVar::GetGeneratingString() this=<%p>'%s' entered, mode=%d, prefix='%s', ")
       wxT("useName='%s'\n"), this, GetName().c_str(), mode, prefix.c_str(), useName.c_str());
   MessageInterface::ShowMessage
      (wxT("   mExpr='%s', mDepObjectName='%s'\n"), mExpr.c_str(), mDepObjectName.c_str());
   #endif
   
   // @note
   // Do not write wxT("Create name") since multiple Strings per line will be written from
   // the ScriptInterpreter
   
   // Write value if it is not blank or blank and SHOW_SCRIPT mode
   if ( mExpr != wxT("") ||
       (mExpr == wxT("") && mode == Gmat::SHOW_SCRIPT))
   {
      // if value is other StringVar object, do not put quotes
      if (mExpr != wxT("") && mExpr == mDepObjectName)
         generatingString = wxT("GMAT ") + GetName() + wxT(" = ") + mExpr;
      else
         generatingString = wxT("GMAT ") + GetName() + wxT(" = '") + mExpr + wxT("'");
      
      if (mode == Gmat::NO_COMMENTS)
         generatingString = generatingString + wxT(";\n");
      else
         generatingString = generatingString + wxT(";") + inlineComment + wxT("\n");
   }
   else
   {
      generatingString = wxT("");
   }
   
   #ifdef DEBUG_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("StringVar::GetGeneratingString() returning\n   <%s>\n"), generatingString.c_str());
   #endif
   
   return generatingString;
}

