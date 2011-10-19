//$Id: Parameter.cpp 9741 2011-08-02 13:25:46Z wendys-dev $
//------------------------------------------------------------------------------
//                                  Parameter
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
// Created: 2003/09/16
//
/**
 * Implements base class of parameters.
 */
//------------------------------------------------------------------------------
#include "gmatdefs.hpp"
#include "Parameter.hpp"
#include "ParameterException.hpp"
#include "ParameterInfo.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_PARAMETER
//#define DEBUG_RENAME
//#define DEBUG_COMMENT

//---------------------------------
// static data
//---------------------------------
const wxString
Parameter::PARAMETER_KEY_STRING[GmatParam::KeyCount] =
{
   wxT("SystemParam"),
   wxT("UserParam")
};

const wxString
Parameter::PARAMETER_TEXT[ParameterParamCount - GmatBaseParamCount] =
{
   wxT("Object"),
   wxT("InitialValue"),
   wxT("Expression"),
   wxT("Description"),
   wxT("Unit"),
   wxT("DepObject"),
   wxT("Color"),
};

const Gmat::ParameterType
Parameter::PARAMETER_TYPE[ParameterParamCount - GmatBaseParamCount] =
{
   Gmat::OBJECT_TYPE,          //wxT("Object"),
   Gmat::STRING_TYPE,          //wxT("InitialValue")
   Gmat::STRING_TYPE,          //wxT("Expression"),
   Gmat::STRING_TYPE,          //wxT("Description"),
   Gmat::STRING_TYPE,          //wxT("Unit"),
   Gmat::STRING_TYPE,          //wxT("DepObject"),
   Gmat::UNSIGNED_INT_TYPE,    //wxT("Color"),
};

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// Parameter(const wxString &name, const wxString &typeStr,
//           GmatParam::ParameterKey key, GmatBase *obj, const wxString &desc,
//           const wxString &unit, GmatParam::DepObject depObj,
//           Gmat::ObjectType ownerType, bool isTimeParam, bool isSettable,
//           bool isPlottable, bool isReportable)
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
 * @param <ownerType> object type who owns this parameter as property
 * @param <depObj> object which parameter is dependent on (COORD_SYS, ORIGIN, NONE)
 * @param <isTimeParam> true if parameter is time related, false otherwise
 * @param <isSettable> true if parameter is settable, false otherwise
 * @param <isPlottable> true if parameter is plottable (Real), false otherwise
 * @param <isReportable> true if parameter is reportable (Real, String), false otherwise
 *
 * @exception <ParameterException> thrown if parameter name has blank spaces
 */
//------------------------------------------------------------------------------
Parameter::Parameter(const wxString &name, const wxString &typeStr,
                     GmatParam::ParameterKey key, GmatBase *obj,
                     const wxString &desc, const wxString &unit,
                     GmatParam::DepObject depObj, Gmat::ObjectType ownerType,
                     bool isTimeParam, bool isSettable, bool isPlottable,
                     bool isReportable)
   : GmatBase(Gmat::PARAMETER, typeStr, name)
{  
   objectTypes.push_back(Gmat::PARAMETER);
   objectTypeNames.push_back(wxT("Parameter"));
   
   if (key == GmatParam::SYSTEM_PARAM)
      objectTypeNames.push_back(wxT("SystemParameter"));
   
   mKey = key;
   
   //if ((name != wxT("") && name != wxT(" ")))
   if (name != wxT(""))
   {
      //if constructor throws an exception, it isn't caught in the caller code.
      //so replace blank space with underscore wxT("_")
      wxString tempName = name;
      wxString replaceStr = wxT("_");
      for (unsigned int i=0; i<tempName.size(); i++)
         if (tempName[i] == ' ')
            tempName.replace(i, 1, replaceStr);
      
      instanceName = tempName;
      
      //if (name.find(' ') < name.npos)
      //     throw ParameterException
      //         (wxT("Parameter: parameter name cannot have blank space: ") + name);
   }
   
   if (desc == wxT(""))
      mDesc = instanceName;
   else
      mDesc = desc;
   
   mExpr = wxT("");
   mUnit = unit;
   mOwnerName = wxT("");
   mDepObjectName = wxT("");
   mCommentLine2 = wxT("");
   mInitialValue = wxT("");
   mIsCommentFromCreate = true;
   mOwnerType = ownerType;
   mDepObj = depObj;
   mCycleType = GmatParam::NOT_CYCLIC;
   mColor = 0; // black
   mNeedCoordSystem = false;
   mIsCoordSysDependent = false;
   mIsOriginDependent = false;
   
   if (depObj == GmatParam::COORD_SYS)
      mIsCoordSysDependent = true;
   else if (depObj == GmatParam::ORIGIN)
      mIsOriginDependent = true;
   
   mIsAngleParam = false;
   mIsTimeParam = isTimeParam;
   mIsSettable = isSettable;
   mIsPlottable = isPlottable;
   mIsReportable = isReportable;
   
   // register parameter names with info
   ParameterInfo::Instance()->Add(typeName, mOwnerType, instanceName, mDepObj,
                                  isPlottable, isReportable, isSettable);
   
   // set parameter count
   parameterCount = ParameterParamCount;

}


//------------------------------------------------------------------------------
// Parameter(const Parameter &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the object being copied.
 */
//------------------------------------------------------------------------------
Parameter::Parameter(const Parameter &copy)
   : GmatBase(copy)
{
   mKey  = copy.mKey;
   mExpr = copy.mExpr;
   mDesc = copy.mDesc;
   mUnit = copy.mUnit;
   mOwnerName = copy.mOwnerName;
   mDepObjectName = copy.mDepObjectName;
   mCommentLine2 = copy.mCommentLine2;
   mInitialValue = copy.mInitialValue;
   mIsCommentFromCreate = copy.mIsCommentFromCreate;
   mOwnerType = copy.mOwnerType;
   mReturnType = copy.mReturnType;
   mDepObj = copy.mDepObj;
   mCycleType = copy.mCycleType;
   mColor = copy.mColor;
   mIsAngleParam = copy.mIsAngleParam;
   mIsTimeParam = copy.mIsTimeParam;
   mIsSettable = copy.mIsSettable;
   mIsPlottable = copy.mIsPlottable;
   mIsReportable = copy.mIsReportable;
   mIsCoordSysDependent = copy.mIsCoordSysDependent;
   mIsOriginDependent = copy.mIsOriginDependent;
   mNeedCoordSystem = copy.mNeedCoordSystem;
   
}


//------------------------------------------------------------------------------
// Parameter& operator= (const Parameter& right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the object being copied.
 *
 * @return reference to this object
 */
//------------------------------------------------------------------------------
Parameter& Parameter::operator= (const Parameter& right)
{
   if (this == &right)
      return *this;

   GmatBase::operator=(right);
   
   mKey = right.mKey;
   mExpr = right.mExpr;
   mDesc = right.mDesc;
   mUnit = right.mUnit;
   mDepObjectName = right.mDepObjectName;
   mCommentLine2 = right.mCommentLine2;
   mInitialValue = right.mInitialValue;
   mIsCommentFromCreate = right.mIsCommentFromCreate;
   mOwnerType = right.mOwnerType;
   mReturnType = right.mReturnType;
   mDepObj = right.mDepObj;
   mCycleType = right.mCycleType;
   mColor = right.mColor;
   mIsAngleParam = right.mIsAngleParam;
   mIsTimeParam = right.mIsTimeParam;
   mIsSettable = right.mIsSettable;
   mIsPlottable = right.mIsPlottable;
   mIsReportable = right.mIsReportable;
   mIsCoordSysDependent = right.mIsCoordSysDependent;
   mIsOriginDependent = right.mIsOriginDependent;
   mNeedCoordSystem = right.mNeedCoordSystem;

   return *this;
}

//------------------------------------------------------------------------------
// ~Parameter()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
Parameter::~Parameter()
{
}

//------------------------------------------------------------------------------
// GmatParam::ParameterKey GetKey() const
//------------------------------------------------------------------------------
/**
 * @return enumeration value of parameter key.
 */
//------------------------------------------------------------------------------
GmatParam::ParameterKey Parameter::GetKey() const
{
   return mKey;
}


//------------------------------------------------------------------------------
// Gmat::ObjectType GetOwnerType() const
//------------------------------------------------------------------------------
/**
 * @return enumeration value of object type.
 */
//------------------------------------------------------------------------------
Gmat::ObjectType Parameter::GetOwnerType() const
{
   return mOwnerType;
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetReturnType() const
//------------------------------------------------------------------------------
/**
 * @return enumeration value of return parameter type.
 */
//------------------------------------------------------------------------------
Gmat::ParameterType Parameter::GetReturnType() const
{
   return mReturnType;
}


//------------------------------------------------------------------------------
// GmatParam::CycleType GetCycleType() const
//------------------------------------------------------------------------------
/**
 * @return enumeration value of return parameter type.
 */
//------------------------------------------------------------------------------
GmatParam::CycleType  Parameter::GetCycleType() const
{
   return mCycleType;
}


//------------------------------------------------------------------------------
// bool IsAngleParameter() const
//------------------------------------------------------------------------------
/**
 * @return true if parameter outputs angle value
 */
//------------------------------------------------------------------------------
bool Parameter::IsAngleParameter() const
{
   return mIsAngleParam;
}


//------------------------------------------------------------------------------
// bool IsTimeParameter() const
//------------------------------------------------------------------------------
/**
 * @return true if parameter is time related.
 */
//------------------------------------------------------------------------------
bool Parameter::IsTimeParameter() const
{
   return mIsTimeParam;
}


//------------------------------------------------------------------------------
// bool IsPlottable() const
//------------------------------------------------------------------------------
/**
 * @return true if parameter is plottble.
 */
//------------------------------------------------------------------------------
bool Parameter::IsPlottable() const
{
   return mIsPlottable;
}


//------------------------------------------------------------------------------
// bool IsReportable() const
//------------------------------------------------------------------------------
/**
 * @return true if parameter is plottble.
 */
//------------------------------------------------------------------------------
bool Parameter::IsReportable() const
{
   return mIsReportable;
}


//------------------------------------------------------------------------------
// bool IsSettable() const
//------------------------------------------------------------------------------
/**
 * @return true if parameter is plottble.
 */
//------------------------------------------------------------------------------
bool Parameter::IsSettable() const
{
   return mIsSettable;
}


//------------------------------------------------------------------------------
// bool IsCoordSysDependent() const
//------------------------------------------------------------------------------
/**
 * @return true if parameter is CoordinateSystem dependent.
 */
//------------------------------------------------------------------------------
bool Parameter::IsCoordSysDependent() const
{
   return mIsCoordSysDependent;
}


//------------------------------------------------------------------------------
// bool IsOriginDependent() const
//------------------------------------------------------------------------------
/**
 * @return true if parameter is origin dependent.
 */
//------------------------------------------------------------------------------
bool Parameter::IsOriginDependent() const
{
   return mIsOriginDependent;
}

//------------------------------------------------------------------------------
// bool NeedCoordSystem() const
//------------------------------------------------------------------------------
/**
 * @return true if parameter is needs CoordinateSystem.
 */
//------------------------------------------------------------------------------
bool Parameter::NeedCoordSystem() const
{
   return mNeedCoordSystem;
}

//------------------------------------------------------------------------------
// void SetKey(const GmatParam::ParameterKey &key)
//------------------------------------------------------------------------------
/**
 * Sets parameter key.
 *
 * @param <key> key of parameter.
 */
//------------------------------------------------------------------------------
void Parameter::SetKey(const GmatParam::ParameterKey &key)
{
   mKey = key;
}

//------------------------------------------------------------------------------
// bool operator==(const Parameter &right) const
//------------------------------------------------------------------------------
/**
 * @return true if input object's type and name are the same as this object.
 */
//------------------------------------------------------------------------------
bool Parameter::operator==(const Parameter &right) const
{
   if (typeName != right.typeName)
      return false;

   if (instanceName.compare(right.instanceName) != 0)
      return false;

   return true;
}

//------------------------------------------------------------------------------
// bool operator!=(const Parameter &right) const
//------------------------------------------------------------------------------
/**
 * @return true if input object's type and name are not the same as this object.
 */
//------------------------------------------------------------------------------
bool Parameter::operator!=(const Parameter &right) const
{
   return !(*this == right);
}

//------------------------------------------------------------------------------
// wxString ToString()
//------------------------------------------------------------------------------
/**
 * @return parameter value converted to wxString.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
wxString Parameter::ToString()
{
   throw ParameterException
      (wxT("Parameter: ToString(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of ToString().\n"));
}

//------------------------------------------------------------------------------
// Real GetReal() const
//------------------------------------------------------------------------------
/**
 * @return parameter value without evaluating.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
Real Parameter::GetReal() const
{
   throw ParameterException
      (wxT("Parameter: GetReal(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of GetReal().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// const Rvector6& GetRvector6() const
//------------------------------------------------------------------------------
/**
 * @return parameter value without evaluating.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
const Rvector6& Parameter::GetRvector6() const
{
   throw ParameterException
      (wxT("Parameter: GetRvector6(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of GetRvector6().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// const Rmatrix66& GetRmatrix66() const
//------------------------------------------------------------------------------
/**
 * @return parameter value without evaluating.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
const Rmatrix66& Parameter::GetRmatrix66() const
{
   throw ParameterException
      (wxT("Parameter: GetRmatrix66(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of GetRmatrix66().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// const Rmatrix33& GetRmatrix33() const
//------------------------------------------------------------------------------
/**
 * @return parameter value without evaluating.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
const Rmatrix33& Parameter::GetRmatrix33() const
{
   throw ParameterException
      (wxT("Parameter: GetRmatrix33(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of GetRmatrix33().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// const Rmatrix& GetRmatrix() const
//------------------------------------------------------------------------------
/**
 * @return parameter value without evaluating.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
const Rmatrix& Parameter::GetRmatrix() const
{
   throw ParameterException
      (wxT("Parameter: GetRmatrix(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of GetRmatrix().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// const wxString& GetString() const
//------------------------------------------------------------------------------
/**
 * @return parameter value without evaluating.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
const wxString& Parameter::GetString() const
{
   throw ParameterException
      (wxT("Parameter: GetString(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of GetString().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets Real value of parameter.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
void Parameter::SetReal(Real val)
{
   throw ParameterException
      (wxT("Parameter: SetReal(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of SetReal().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// void SetRvector6(const Rvector6 &val)
//------------------------------------------------------------------------------
/**
 * Sets Rvector6 value of parameter.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
void Parameter::SetRvector6(const Rvector6 &val)
{
   throw ParameterException
      (wxT("Parameter: SetRvector6(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of SetRvector6().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// void SetRmatrix66(const Rmatrix66 &mat)
//------------------------------------------------------------------------------
/**
 * Sets Rmatrix66 value of parameter.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
void Parameter::SetRmatrix66(const Rmatrix66 &mat)
{
   throw ParameterException
      (wxT("Parameter: SetRmatrix66(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of SetRmatrix66().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// void SetRmatrix33(const Rmatrix33 &mat)
//------------------------------------------------------------------------------
/**
 * Sets Rmatrix33 value of parameter.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
void Parameter::SetRmatrix33(const Rmatrix33 &mat)
{
   throw ParameterException
      (wxT("Parameter: SetRmatrix33(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of SetRmatrix33().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// void SetRmatrix(const Rmatrix &mat)
//------------------------------------------------------------------------------
/**
 * Sets Rmatrix value of parameter.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
void Parameter::SetRmatrix(const Rmatrix &mat)
{
   throw ParameterException
      (wxT("Parameter: SetRmatrix(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of SetRmatrix().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// void SetString(const wxString &val)
//------------------------------------------------------------------------------
/**
 * Sets string value of parameter.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
void Parameter::SetString(const wxString &val)
{
   throw ParameterException
      (wxT("Parameter: SetString(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of SetString().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// Real EvaluateReal()
//------------------------------------------------------------------------------
/**
 * @return newly evaluated parameter value.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
Real Parameter::EvaluateReal()
{
   throw ParameterException
      (wxT("Parameter: EvaluateReal(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of EvaluateReal().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// const Rvector6& EvaluateRvector6()
//------------------------------------------------------------------------------
/**
 * @return newly evaluated parameter value.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
const Rvector6& Parameter::EvaluateRvector6()
{
   throw ParameterException
      (wxT("Parameter: EvaluateRvector6(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of EvaluateRvector6().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// const Rmatrix66& EvaluateRmatrix66()
//------------------------------------------------------------------------------
/**
 * @return newly evaluated parameter value.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
const Rmatrix66& Parameter::EvaluateRmatrix66()
{
   throw ParameterException
      (wxT("Parameter: EvaluateRmatrix66(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of EvaluateRmatrix66().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// const Rmatrix33& EvaluateRmatrix33()
//------------------------------------------------------------------------------
/**
 * @return newly evaluated parameter value.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
const Rmatrix33& Parameter::EvaluateRmatrix33()
{
   throw ParameterException
      (wxT("Parameter: EvaluateRmatrix33(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of EvaluateRmatrix33().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// const Rmatrix& EvaluateRmatrix()
//------------------------------------------------------------------------------
/**
 * @return newly evaluated parameter value.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
const Rmatrix& Parameter::EvaluateRmatrix()
{
   throw ParameterException
      (wxT("Parameter: EvaluateRmatrix(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of EvaluateRmatrix().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// const wxString& EvaluateString()
//------------------------------------------------------------------------------
/**
 * @return newly evaluated parameter value.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
const wxString& Parameter::EvaluateString()
{
   throw ParameterException
      (wxT("Parameter: EvaluateString(): ") + this->GetTypeName() + wxT(" has no ")
       wxT("implementation of EvaluateString().\nMay be an invalid call to this ")
       wxT("function.\n"));
}


//------------------------------------------------------------------------------
// virtual const wxString* GetParameterList() const
//------------------------------------------------------------------------------
const wxString* Parameter::GetParameterList() const
{
   return NULL;
}

//------------------------------------------------------------------------------
// virtual CoordinateSystem* GetInternalCoordSystem()
//------------------------------------------------------------------------------
CoordinateSystem* Parameter::GetInternalCoordSystem()
{
   return NULL;
}

//------------------------------------------------------------------------------
// void SetSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
void Parameter::SetSolarSystem(SolarSystem *ss)
{
   ; // do nothing here
}

//------------------------------------------------------------------------------
// void SetInternalCoordSystem(CoordinateSystem *cs)
//------------------------------------------------------------------------------
void Parameter::SetInternalCoordSystem(CoordinateSystem *cs)
{
   ; // do nothing here
}

//------------------------------------------------------------------------------
// virtual void Initialize()
//------------------------------------------------------------------------------
bool Parameter::Initialize()
{
   return true; // do nothing here
}

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
bool Parameter::Evaluate()
{
   if (mKey == GmatParam::SYSTEM_PARAM)
      throw ParameterException(wxT("Parameter: Evaluate() should be implemented ")
                               wxT("for Parameter Type: ") + GetTypeName() + wxT("\n"));

   return false;
}

//------------------------------------------
// methods All SYSTEM_PARAM should implement
//------------------------------------------

//------------------------------------------------------------------------------
// virtual bool AddRefObject(GmatBase *object, bool replaceName = false)
//------------------------------------------------------------------------------
bool Parameter::AddRefObject(GmatBase *object, bool replaceName)
{
   if (mKey == GmatParam::SYSTEM_PARAM)
      throw ParameterException(wxT("Parameter: AddRefObject() should be implemented ")
                               wxT("for Parameter Type:") + GetTypeName() + wxT("\n"));

   return false;
}

//------------------------------------------------------------------------------
// virtual Integer GetNumRefObjects() const
//------------------------------------------------------------------------------
Integer Parameter::GetNumRefObjects() const
{
   if (mKey == GmatParam::SYSTEM_PARAM)
      throw ParameterException(wxT("Parameter: GetNumRefObjects() should be implemented")
                               wxT("for Parameter Type: ") + GetTypeName() + wxT("\n"));

   return 0;
}

//------------------------------------------------------------------------------
// virtual bool Validate()
//------------------------------------------------------------------------------
bool Parameter::Validate()
{
   if (mKey == GmatParam::SYSTEM_PARAM)
      throw ParameterException(wxT("Parameter: Validate() should be implemented ")
                               wxT("for Parameter Type: ") + GetTypeName() + wxT("\n"));

   return true; // loj: 9/23/04 There is nothing to validate for USER_PARAM
}


//---------------------------------
// methods inherited from GmatBase
//---------------------------------

// required method for all subclasses that can be copied in a script
//------------------------------------------------------------------------------
// void Copy(const GmatBase* orig)
//------------------------------------------------------------------------------
void Parameter::Copy(const GmatBase* orig)
{
   operator=(*((Parameter *)(orig)));
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
bool Parameter::RenameRefObject(const Gmat::ObjectType type,
                                const wxString &oldName,
                                const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("Parameter::RenameRefObject() type=%s, oldName=%s, newName=%s\n"),
       GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif

   // loj: Should I add SPACE_POINT?
   if (type != Gmat::SPACECRAFT && type != Gmat::COORDINATE_SYSTEM)
      return true;
   
   wxString oldExpr = mExpr;
   wxString::size_type pos;
   
   pos = mExpr.find(oldName);
   
   // change expression, if oldName found
   if (pos != mExpr.npos)
   {
      mExpr.replace(pos, oldName.size(), newName);
   }
   
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("oldExpr=%s, mExpr=%s\n"), oldExpr.c_str(), mExpr.c_str());
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
wxString Parameter::GetParameterText(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < ParameterParamCount)
      return PARAMETER_TEXT[id - GmatBaseParamCount];
   else
      return GmatBase::GetParameterText(id);
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
Integer Parameter::GetParameterID(const wxString &str) const
{
   for (int i=GmatBaseParamCount; i<ParameterParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatBaseParamCount])
         return i;
   }
   
   return GmatBase::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
Gmat::ParameterType Parameter::GetParameterType(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < ParameterParamCount)
      return PARAMETER_TYPE[id - GmatBaseParamCount];
   else
      return GmatBase::GetParameterType(id);
}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
wxString Parameter::GetParameterTypeString(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < ParameterParamCount)
      return GmatBase::PARAM_TYPE_STRING[GetParameterType(id)];
   else
      return GmatBase::GetParameterTypeString(id);
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <id> Description for the parameter.
 *
 * @return true if the parameter is read only, false (the default) if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool Parameter::IsParameterReadOnly(const Integer id) const
{
   if ((id == DESCRIPTION) || (id == UNIT) || (id == DEP_OBJECT) ||
       (id == COLOR) || (id == EXPRESSION) || id == INITIAL_VALUE)
      return true;
   
   return GmatBase::IsParameterReadOnly(id);
}


//----- UnsignedInt parameters

//------------------------------------------------------------------------------
// UnsignedInt GetUnsignedIntParameter(const Integer id) const
//------------------------------------------------------------------------------
UnsignedInt Parameter::GetUnsignedIntParameter(const Integer id) const
{
   #ifdef DEBUG_PARAMETER
   MessageInterface::ShowMessage(wxT("Parameter::GetUnsignedIntParameter() ")
                                 wxT("id=%d\n"), id);
   #endif
   
   switch (id)
   {
   case COLOR:
      return mColor;
   default:
      return GmatBase::GetUnsignedIntParameter(id);
   }
}

//------------------------------------------------------------------------------
// UnsignedInt GetUnsignedIntParameter(const wxString &label) const
//------------------------------------------------------------------------------
UnsignedInt Parameter::GetUnsignedIntParameter(const wxString &label) const
{
   return GetUnsignedIntParameter(GetParameterID(label));
}

//------------------------------------------------------------------------------
// UnsignedInt SetUnsignedIntParameter(const Integer id, const UnsignedInt value)
//------------------------------------------------------------------------------
UnsignedInt Parameter::SetUnsignedIntParameter(const Integer id,
                                               const UnsignedInt value)
{
   #ifdef DEBUG_PARAMETER
   MessageInterface::ShowMessage(wxT("Parameter::SetUnsignedIntParameter() ")
                                 wxT("id=%d value=%d\n"), id, value);
   #endif
   switch (id)
   {
   case COLOR: 
      mColor = value;
      return mColor;
   default:
      return GmatBase::SetUnsignedIntParameter(id, value);
   }
}

//------------------------------------------------------------------------------
// UnsignedInt SetUnsignedIntParameter(const wxString &label,
//                                     const UnsignedInt &value)
//------------------------------------------------------------------------------
UnsignedInt Parameter::SetUnsignedIntParameter(const wxString &label,
                                               const UnsignedInt value)
{
   return SetUnsignedIntParameter(GetParameterID(label), value);
}

//----- wxString parameters

//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
wxString Parameter::GetStringParameter(const Integer id) const
{
   switch (id)
   {
   case OBJECT:
      if (GetNumRefObjects() > 0)
         return GetRefObjectName(mOwnerType);
      else
         return wxT("");
   case INITIAL_VALUE:
      return mInitialValue;
   case EXPRESSION:
      return mExpr;
   case DESCRIPTION:
      return mDesc;
   case UNIT:
      return mUnit;
   case DEP_OBJECT:
      return mDepObjectName;
   default:
      return GmatBase::GetStringParameter(id);
   }
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString Parameter::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
bool Parameter::SetStringParameter(const Integer id, const wxString &value)
{
   #ifdef DEBUG_SET_STRING
   MessageInterface::ShowMessage(wxT("Parameter::SetStringParameter() id=%d, value=%s\n"),
                                 id, value.c_str());
   #endif
   
   switch (id)
   {
   case OBJECT:
      return SetRefObjectName(mOwnerType, value);
   case INITIAL_VALUE:
      #ifdef DEBUG_SET_STRING
      MessageInterface::ShowMessage(wxT("   InitialValue = '%s'\n"), value.c_str());
      #endif
      mInitialValue = value;
      return true;
   case EXPRESSION:
      mExpr = value;
      return true;
   case DESCRIPTION:
      mDesc = value;
      return true;
   case UNIT:
      mUnit = value;
      return true;
   case DEP_OBJECT:
      mDepObjectName = value;
      if (mIsCoordSysDependent)
         return SetRefObjectName(Gmat::COORDINATE_SYSTEM, value);
      else if (mIsOriginDependent)
         return SetRefObjectName(Gmat::SPACE_POINT, value);
      return true;
   default:
      return GmatBase::SetStringParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label,
//                         const wxString &value)
//------------------------------------------------------------------------------
bool Parameter::SetStringParameter(const wxString &label,
                                   const wxString &value)
{
   #ifdef DEBUG_PARAMETER
   MessageInterface::ShowMessage(wxT("Parameter::SetStringParameter() label=%s value=%s\n"),
                                 label.c_str(), value.c_str());
   #endif
   
   return SetStringParameter(GetParameterID(label), value);
}


//---------------------------------------------------------------------------
//  const wxString GetCommentLine(Integer which)
//---------------------------------------------------------------------------
/*
 * This method retrives preface comments for Variable and Array.
 *
 * @param which Indicates which comment should be returned
 *              1 = Preface comment from Create line
 *              2 = Preface comment from Initialization line
 * @return Preface comments
 */
//---------------------------------------------------------------------------
const wxString Parameter::GetCommentLine(Integer which)
{
   #ifdef DEBUG_COMMENT
   MessageInterface::ShowMessage
      (wxT("Parameter::GetCommentLine() <%s> which=%d\ncommentLine=<%s>\n")
       wxT("mCommentLine2=<%s>\n"), GetName().c_str(), which, commentLine.c_str(),
       mCommentLine2.c_str());
   #endif
   
   if (which == 2)
      return mCommentLine2;
   else
      return commentLine;
}


//---------------------------------------------------------------------------
//  void SetCommentLine(const wxString &comment, Integer which = 0)
//---------------------------------------------------------------------------
/*
 * This method sets preface comments for Variable, Array, and String.
 * Since preface comments from initialization line overrides comments from
 * Create line, there are two comments used in the Parameter class.
 *
 * @param which Indicates which comment should be saved
 *              0 = When this method is called first time,
 *                     it assumes comment is set for Create line.
 *              1 = Preface comment from Create line
 *              2 = Preface comment from Initialization line
 *
 */
//---------------------------------------------------------------------------
void Parameter::SetCommentLine(const wxString &comment, Integer which)
{
   if (which == 0)
   {
      // When this method is called first time, it assumes comment from Create line.
      if (mIsCommentFromCreate)
      {
         #ifdef DEBUG_COMMENT
         MessageInterface::ShowMessage
            (wxT("Parameter::SetCommentLine() <%s> commentLine is set to <%s>\n"),
             GetName().c_str(), comment.c_str());
         #endif
         
         commentLine = comment;
         mIsCommentFromCreate = false;
      }
      else
      {
         mCommentLine2 = comment;
         
         #ifdef DEBUG_COMMENT
         MessageInterface::ShowMessage
            (wxT("Parameter::SetCommentLine() <%s> commentLine2 is set to <%s>\n"),
             GetName().c_str(), comment.c_str());
         #endif
      }
   }
   else if (which == 1)
   {
      commentLine = comment;
   }
   else if (which == 2)
   {
      mCommentLine2 = comment;
   }
}

