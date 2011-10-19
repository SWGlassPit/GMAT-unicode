//$Id: Array.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  Array
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
// Created: 2004/09/20
//
/**
 * Implements Array class which handles one or two dimentional array.
 */
//------------------------------------------------------------------------------

#include "gmatdefs.hpp"
#include "Array.hpp"
#include "ParameterException.hpp"
#include "StringUtil.hpp"          // for SeparateBy(), GetArrayIndexVar()
#include "MessageInterface.hpp"
#include <sstream>

//#define DEBUG_ARRAY 1
//#define DEBUG_ARRAY_GET
//#define DEBUG_ARRAY_SET
//#define DEBUG_INITIAL_VALUE

//---------------------------------
// static data
//---------------------------------
const wxString
Array::PARAMETER_TEXT[ArrayParamCount - ParameterParamCount] =
{
   wxT("NumRows"),
   wxT("NumCols"),
   wxT("RmatValue"),
   wxT("SingleValue"),
   wxT("RowValue"),
   wxT("ColValue"),
   wxT("InitialValue"),
   wxT("InitialValueType"),
}; 

const Gmat::ParameterType
Array::PARAMETER_TYPE[ArrayParamCount - ParameterParamCount] =
{
   Gmat::INTEGER_TYPE,
   Gmat::INTEGER_TYPE,
   Gmat::RMATRIX_TYPE,
   Gmat::REAL_ELEMENT_TYPE,  // was Gmat::REAL_TYPE,
   Gmat::RVECTOR_TYPE,
   Gmat::RVECTOR_TYPE,
   Gmat::STRING_TYPE,
   Gmat::INTEGER_TYPE,
};

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// Array(const wxString &name = wxT(""), const wxString &desc = wxT(""),
//       const wxString &unit = wxT(""))
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> parameter name
 * @param <desc> parameter description
 * @param <unit> parameter unit
 */
//------------------------------------------------------------------------------
Array::Array(const wxString &name, const wxString &desc,
             const wxString &unit)
   : Parameter(name, wxT("Array"), GmatParam::USER_PARAM, NULL, desc, unit,
               GmatParam::NO_DEP, Gmat::UNKNOWN_OBJECT, false, false, false, true)
{
   mNumRows = 0;
   mNumCols = 0;
   mInitialValueType = 1;
   mSizeSet = false;
   
   // GmatBase data
   objectTypes.push_back(Gmat::ARRAY);
   objectTypeNames.push_back(wxT("Array"));
   mReturnType = Gmat::RMATRIX_TYPE;
   parameterCount = ArrayParamCount;
}


//------------------------------------------------------------------------------
// Array(const Array &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the object being copied.
 */
//------------------------------------------------------------------------------
Array::Array(const Array &copy)
   : Parameter(copy)
{
   // We don't want to change the name when copy
   wxString thisName = GetName();
   
   mNumRows = copy.mNumRows;
   mNumCols = copy.mNumCols;
   mInitialValueType = copy.mInitialValueType;
   mSizeSet = copy.mSizeSet;
   mRmatValue = copy.mRmatValue;
   
   SetName(thisName);
}


//------------------------------------------------------------------------------
// Array& operator= (const Array& right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the object being copied.
 *
 * @return reference to this object
 */
//------------------------------------------------------------------------------
Array& Array::operator= (const Array& right)
{
   if (this != &right)
   {
      // We don't want to change the name when copy
      wxString thisName = GetName();
      
      Parameter::operator=(right);
      mNumRows = right.mNumRows;
      mNumCols = right.mNumCols;
      mInitialValueType = right.mInitialValueType;
      mSizeSet = right.mSizeSet;
      mRmatValue = right.mRmatValue;
      
      SetName(thisName);
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// ~Array()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
Array::~Array()
{
}


//------------------------------------------------------------------------------
// bool operator==(const Array &right) const
//------------------------------------------------------------------------------
/**
 * @return true if input object's type and name are the same as this object.
 */
//------------------------------------------------------------------------------
bool Array::operator==(const Array &right) const
{
   return Parameter::operator==(right);
}


//------------------------------------------------------------------------------
// bool operator!=(const Array &right) const
//------------------------------------------------------------------------------
/**
 * @return true if input object's type and name are not the same as this object.
 */
//------------------------------------------------------------------------------
bool Array::operator!=(const Array &right) const
{
   return Parameter::operator!=(right);
}


//------------------------------------------------------------------------------
// bool SetSize(const Integer row, const Integer col)
//------------------------------------------------------------------------------
bool Array::SetSize(const Integer row, const Integer col)
{
   #if DEBUG_ARRAY
   MessageInterface::ShowMessage
      (wxT("Array::SetSize() row=%d, col=%d\n"), row, col);
   #endif
   
   mNumRows = row;
   mNumCols = col;
   mRmatValue.SetSize(mNumRows, mNumCols);
   mSizeSet = true;
   return true;
}


//------------------------------------
// methods inherited from Parameter
//------------------------------------

//------------------------------------------------------------------------------
// void SetRmatrix(const Rmatrix &mat)
//------------------------------------------------------------------------------
/**
 * Sets Rmatrix value of parameter.
 *
 * @exception <ParameterException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
void Array::SetRmatrix(const Rmatrix &mat)
{
   if (mSizeSet)
   {
      mRmatValue = mat;
   }
   else
   {
      throw ParameterException(wxT("The size has not been set for ") + GetName());
   }
}


//------------------------------------------------------------------------------
// wxString ToString()
//------------------------------------------------------------------------------
/**
 * @return parameter value converted to wxString.
 */
//------------------------------------------------------------------------------
wxString Array::ToString()
{
   // use default global precision to convert to string (loj: 2008.03.05)
   return mRmatValue.ToString(false, false, false, GmatGlobal::DATA_PRECISION, 1,
                              true, 1, wxT(""), false);
   
   //wxString ss(wxT(""));
   //ss.precision(10);
   //ss << mRmatValue;
   //return wxString(ss.str());
}


//------------------------------------------------------------------------------
// virtual const wxString* GetParameterList() const
//------------------------------------------------------------------------------
const wxString* Array::GetParameterList() const
{
   return PARAMETER_TEXT;
}


//------------------------------------
// methods inherited from GmatBase
//------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* Array::Clone() const
{
   return new Array(*this);
}


//------------------------------------------------------------------------------
// void Copy(const GmatBase* orig)
//------------------------------------------------------------------------------
void Array::Copy(const GmatBase* orig)
{
   operator=(*((Array *)(orig)));
}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
wxString Array::GetParameterText(const Integer id) const
{
   if (id >= ParameterParamCount && id < ArrayParamCount)
      return PARAMETER_TEXT[id - ParameterParamCount];
   else
      return Parameter::GetParameterText(id);
}

//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
Integer Array::GetParameterID(const wxString &str) const
{
   for (int i=ParameterParamCount; i<ArrayParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - ParameterParamCount])
         return i;
   }
   
   return Parameter::GetParameterID(str);
}

//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
Gmat::ParameterType Array::GetParameterType(const Integer id) const
{
   if (id >= ParameterParamCount && id < ArrayParamCount)
      return PARAMETER_TYPE[id - ParameterParamCount];
   else
      return Parameter::GetParameterType(id);
}

//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
wxString Array::GetParameterTypeString(const Integer id) const
{
   if (id >= ParameterParamCount && id < ArrayParamCount)
      return PARAM_TYPE_STRING[GetParameterType(id - ParameterParamCount)];
   else
      return Parameter::GetParameterTypeString(id);
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
bool Array::IsParameterReadOnly(const Integer id) const
{
   //loj: 11/22/06 read only parameters were wrong
   //if ((id == NUM_ROWS) || (id == NUM_COLS) || (id == SINGLE_VALUE))
   //   return true;
   
   if ((id == NUM_ROWS) || (id == NUM_COLS) || (id == RMAT_VALUE) ||
       (id == ROW_VALUE) || (id == COL_VALUE) || (id == INITIAL_VALUE) ||
       (id == INITIAL_VALUE_TYPE))
      return true;
   
   return Parameter::IsParameterReadOnly(id);
}



//----- Integer parameter

//------------------------------------------------------------------------------
// virtual Integer GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
Integer Array::GetIntegerParameter(const Integer id) const
{
   switch (id)
   {
   case NUM_ROWS:
      return mNumRows;
   case NUM_COLS:
      return mNumCols;
   case INITIAL_VALUE_TYPE:
      return mInitialValueType;
   default:
      return Parameter::GetIntegerParameter(id);
   }
}


//------------------------------------------------------------------------------
// virtual Integer GetIntegerParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Integer Array::GetIntegerParameter(const wxString &label) const
{
   return GetIntegerParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual Integer SetIntegerParameter(const Integer id, const Integer value)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 *
 * @exception <ParameterException> thrown if Row or Column already has been set.
 */
//------------------------------------------------------------------------------
Integer Array::SetIntegerParameter(const Integer id, const Integer value)
{
   switch (id)
   {
   case NUM_ROWS:
      if (mNumRows == 0)
         mNumRows = value;
      else
         throw ParameterException(wxT("Row alrealy has been set for ") + GetName());

      if (mNumCols > 0 && !mSizeSet)
      {
         mRmatValue.SetSize(mNumRows, mNumCols);
         mSizeSet = true;
      }
      return value;
   case NUM_COLS:
      if (mNumCols == 0)
         mNumCols = value;
      else
         throw ParameterException(wxT("Column alrealy has been set for ") + GetName());
      
      if (mNumRows > 0 && !mSizeSet)
      {
         mRmatValue.SetSize(mNumRows, mNumCols);
         mSizeSet = true;
      }
      return value;
   case INITIAL_VALUE_TYPE:
      mInitialValueType = value;
      return value;
   default:
      return Parameter::SetIntegerParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// virtual Integer SetIntegerParameter(wxString &label, const Integer value)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Integer Array::SetIntegerParameter(const wxString &label, const Integer value)
{
   return SetIntegerParameter(GetParameterID(label), value);
}

//----- Rvector parameter

//------------------------------------------------------------------------------
// Rvector GetRvectorParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
Rvector Array::GetRvectorParameter(const Integer id, const Integer index) const
{
   switch (id)
   {
   case ROW_VALUE:
      {
         Rvector rvec(mNumCols);
      
         for (int i=0; i<mNumCols; i++)
            rvec.SetElement(i, mRmatValue.GetElement(index, i));

         return rvec;
      }
   case COL_VALUE:
      {
         Rvector rvec(mNumRows);
      
         for (int i=0; i<mNumRows; i++)
            rvec.SetElement(i, mRmatValue.GetElement(i, index));

         return rvec;
      }
   default:
      throw ParameterException
         (wxT("Array::GetRvectorParameter() Unknown Parameter Name") + PARAMETER_TEXT[id]);
      //return Parameter::GetRvectorParameter(id, index);
   }
}

//------------------------------------------------------------------------------
// Rvector GetRvectorParameter(const wxString &label,
//                                    const Integer index) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Rvector Array::GetRvectorParameter(const wxString &label,
                                   const Integer index) const
{
   return GetRvectorParameter(GetParameterID(label), index);
}

//------------------------------------------------------------------------------
// const Rvector& SetRvectorParameter(const Integer id, const Rvector &value,
//                                    const Integer index)
//------------------------------------------------------------------------------
const Rvector& Array::SetRvectorParameter(const Integer id, const Rvector &value,
                                          const Integer index)
{
   
   #if DEBUG_ARRAY
   MessageInterface::ShowMessage
      (wxT("Array::SetRvectorParameter() index=%d, mNumRows=%d, mNumCols=%d\n"),
       index, mNumRows, mNumCols);
   #endif
   
   switch (id)
   {
   case ROW_VALUE:
      for (int i=0; i<mNumCols; i++)
         mRmatValue.SetElement(index, i, value(i));
      
      return value;
   case COL_VALUE:
      for (int i=0; i<mNumRows; i++)
         mRmatValue.SetElement(i, index, value(i));
      
      return value;
   default:
      throw ParameterException
         (wxT("Array::GetRvectorParameter() Unknown Parameter Name") + PARAMETER_TEXT[id]);
      //return Parameter::SetRvectorParameter(id, value, index);
   }
}

//------------------------------------------------------------------------------
// const Rvector& SetRvectorParameter(const std:string &label,
//                                    const Rvector &value, const Integer index)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
const Rvector& Array::SetRvectorParameter(const wxString &label,
                                          const Rvector &value,
                                          const Integer index)
{
   return SetRvectorParameter(GetParameterID(label), value, index);
}

//----- Rmatrix parameter

//------------------------------------------------------------------------------
// const Rmatrix& GetRmatrixParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 *
 * @exception <ParameterException> thrown if matrix size has not been set.
 */
//------------------------------------------------------------------------------
const Rmatrix& Array::GetRmatrixParameter(const Integer id) const
{
   if (mSizeSet)
   {
      switch (id)
      {
      case RMAT_VALUE:
         return mRmatValue;
      default:
         return Parameter::GetRmatrixParameter(id);
      }
   }
   else
   {
      throw ParameterException(wxT("The size has not been set for ") + GetName());
   }
}


//------------------------------------------------------------------------------
// const Rvector& GetRvectorParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
const Rmatrix& Array::GetRmatrixParameter(const wxString &label) const
{
   return GetRmatrixParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// const Rmatrix& SetRmatrixParameter(const Integer id,
//                                    const Rmatrix &value)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 *
 * @exception <ParameterException> thrown if matrix size has not been set.
 */
//------------------------------------------------------------------------------
const Rmatrix& Array::SetRmatrixParameter(const Integer id,
                                          const Rmatrix &value)
{
   switch (id)
   {
   case RMAT_VALUE:
      SetRmatrix(value);
      return value;
   default:
      return Parameter::SetRmatrixParameter(id, value);
   }
   
//    if (mSizeSet)
//    {
//       switch (id)
//       {
//       case RMAT_VALUE:
//          mRmatValue = value;
//          return value;
//       default:
//          return Parameter::SetRmatrixParameter(id, value);
//       }
//    }
//    else
//    {
//       throw ParameterException(wxT("The size has not been set for ") + GetName());
//    }
}


//------------------------------------------------------------------------------
// const Rmatrix& SetRmatrixParameter(const wxString &label,
//                                    const Rmatrix &value)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
const Rmatrix& Array::SetRmatrixParameter(const wxString &label,
                                          const Rmatrix &value)
{
   return SetRmatrixParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const Integer id, const Integer index)
//------------------------------------------------------------------------------
Real Array::GetRealParameter(const Integer id, const Integer index) const
{
   switch (id)
   {
   case SINGLE_VALUE:
      return mRmatValue.GetElement(0, index);
   default:
      throw ParameterException
         (wxT("Array::GetRealParameter() Unknown Parameter Name") + PARAMETER_TEXT[id]);
   }
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const wxString &label, const Integer index) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real Array::GetRealParameter(const wxString &label, const Integer index) const
{
   return GetRealParameter(GetParameterID(label), index);
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const Integer id, const Integer row,
//                       const Integer col) const
//------------------------------------------------------------------------------
Real Array::GetRealParameter(const Integer id, const Integer row,
                             const Integer col) const
{
   #if DEBUG_ARRAY_GET
   MessageInterface::ShowMessage
      (wxT("Array::GetRealParameter() entered, id=%d, row=%d, col=%d\n"), id,
       row, col);
   #endif
   
   switch (id)
   {
   case SINGLE_VALUE:
      return mRmatValue.GetElement(row, col);
   default:
      throw ParameterException
         (wxT("Array::GetRealParameter() Unknown Parameter Name") + PARAMETER_TEXT[id]);
      //return Parameter::GetRealParameter(id, row, col);
   }
}

//------------------------------------------------------------------------------
// Real GetRealParameter(const wxString &label, const Integer row,
//                       const Integer col) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real Array::GetRealParameter(const wxString &label, const Integer row,
                             const Integer col) const
{
   return GetRealParameter(GetParameterID(label), row, col);
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value, const Integer row,
//                       const Integer col)
//------------------------------------------------------------------------------
/**
 * Sets value of array element row and col.
 *
 * @param id     The Id of the parameter
 * @param value  The real value of the array element
 * @param row    The row of the array element (starts from 0)
 * @param col    The column of the array element (starts form 0)
 */
//------------------------------------------------------------------------------
Real Array::SetRealParameter(const Integer id, const Real value,
                             const Integer row, const Integer col)
{
   switch (id)
   {
   case SINGLE_VALUE:
      #ifdef DEBUG_ARRAY_SET
         MessageInterface::ShowMessage(
         wxT("In Array::SetRealParameter, row = %d, col = %d, value = %.12f\n"),
         row, col, value);
      #endif
      mRmatValue.SetElement(row, col, value);
      return value;
   default:
      throw ParameterException
         (wxT("Array::SetRealParameter() Unknown Parameter Name") + PARAMETER_TEXT[id]);
      //return Parameter::SetRealParameter(id, value, row, col);
   }
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const std:string &label, const Real value,
//                       const Integer row, const Integer col)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real Array::SetRealParameter(const wxString &label, const Real value,
                             const Integer row, const Integer col)
{
   return SetRealParameter(GetParameterID(label), value, row, col);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/*
 * Override this method to return array declaration string.
 * Such as Arr1[2]
 */
//------------------------------------------------------------------------------
wxString Array::GetStringParameter(const Integer id) const
{
   switch (id)
   {
   case DESCRIPTION:
      return GetArrayDefString();
   case INITIAL_VALUE:
      // Use const_cast to remove the const,
      // since GetInitialValueString() is no longer a const method
      return (const_cast<Array*>(this))->GetInitialValueString();
   default:
      return Parameter::GetStringParameter(id);
   }
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString Array::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
bool Array::SetStringParameter(const Integer id, const wxString &value)
{
   switch (id)
   {
   case INITIAL_VALUE:
      {
         // Sets initial value of array elements (LOJ: 2010.09.21)
         // value should be in format of Arr(I,J)=Value, so it can be parsed
         StringArray parts = GmatStringUtil::SeparateBy(value, wxT("="), true);
         if (parts.size() == 2)
         {
            wxString name, rowStr, colStr;
            // parse array name and index
            GmatStringUtil::GetArrayIndexVar(parts[0], rowStr, colStr, name);
            wxString mapstr = rowStr + wxT(",") + colStr;
            #ifdef DEBUG_ARRAY_SET
            MessageInterface::ShowMessage
               (wxT("Array::SetStringParameter() mapstr='%s', value=%s\n"),
                mapstr.c_str(), parts[1].c_str());
            #endif
            initialValueMap[mapstr] = parts[1];
         }
         return true;
      }
   default:
      return Parameter::SetStringParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
bool Array::SetStringParameter(const wxString &label, const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// StringArray GetGeneratingString(Gmat::WriteMode mode,
//                const wxString &prefix, const wxString &useName)
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
const wxString& Array::GetGeneratingString(Gmat::WriteMode mode,
                                              const wxString &prefix,
                                              const wxString &useName)
{
   #ifdef DEBUG_GEN_STRING
   MessageInterface::ShowMessage(wxT("Array::GetGeneratingString() entered\n"));
   #endif
   
   wxString data;
   
   // Crank up data precision so we don't lose anything
   wxString preface = wxT(""), nomme;
   
   if ((mode == Gmat::SCRIPTING) || (mode == Gmat::OWNED_OBJECT) ||
       (mode == Gmat::SHOW_SCRIPT))
      inMatlabMode = false;
   if (mode == Gmat::MATLAB_STRUCT)
      inMatlabMode = true;
   
   if (useName != wxT(""))
      nomme = useName;
   else
      nomme = instanceName;
   
   if ((mode == Gmat::SCRIPTING) || (mode == Gmat::SHOW_SCRIPT))
   {
      wxString tname = typeName;

      // Add comment line (loj: 03/27/07)
      if (GetCommentLine() != wxT(""))
         data << GetCommentLine();
      
      data << wxT("Create ") << tname << wxT(" ") << nomme 
           << wxT("[") << mNumRows << wxT(",") << mNumCols << wxT("];\n");
      
      preface = wxT("GMAT ");
   }
   
   nomme += wxT(".");
   
   if (mode == Gmat::OWNED_OBJECT)
   {
      preface = prefix;
      nomme = wxT("");
   }
   
   preface += nomme;
   
   WriteParameters(mode, preface, data);
   generatingString = data;
   
   #ifdef DEBUG_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("Array::GetGeneratingString() return\n%s\n"), generatingString.c_str());
   #endif
   
   return generatingString;
}


//------------------------------------------------------------------------------
// wxString GetArrayDefString() const
//------------------------------------------------------------------------------
/*
 * Returns array declaration string.
 * Such as Arr1[2]
 */
//------------------------------------------------------------------------------
wxString Array::GetArrayDefString() const
{
   wxString data;
   data << instanceName << wxT("[") << mNumRows << wxT(",") << mNumCols << wxT("]");
   return data;
}


//------------------------------------------------------------------------------
// wxString GetInitialValueString(const wxString &prefix)
//------------------------------------------------------------------------------
/*
 * Returns array initial value string including inline comments.
 *
 * Such as GMAT Arr1[1,1] = 13.34; %% initialize
 */
//------------------------------------------------------------------------------
wxString Array::GetInitialValueString(const wxString &prefix)
{
   #ifdef DEBUG_INITIAL_VALUE
   MessageInterface::ShowMessage
      (wxT("Array::GetInitialValueString() '%s' entered\n"), GetName().c_str());
   #endif
   
   wxString data;
   
   Integer row = GetIntegerParameter(wxT("NumRows"));
   Integer col = GetIntegerParameter(wxT("NumCols"));
   
   for (Integer i = 0; i < row; ++i)
   {
      //loj: Do not write if value is zero since default is zero(03/27/07)
      for (Integer j = 0; j < col; ++j)
      {
         Real realVal = GetRealParameter(SINGLE_VALUE, i, j);
         #ifdef DEBUG_INITIAL_VALUE
         MessageInterface::ShowMessage(wxT("   value = %f\n"), realVal);
         #endif
         
         //if (GetRealParameter(SINGLE_VALUE, i, j) != 0.0)
         if (realVal != 0.0)
         {
            //========================================================
            #ifndef __WRITE_INITIAL_VALUE_STRING__
            //========================================================
            
            // This writes out actual value
            data << wxT("GMAT ") << instanceName << wxT("(") << i+1 << wxT(", ") << j+1 <<
               wxT(") = ") << GetRealParameter(SINGLE_VALUE, i, j) << wxT(";");
            data << GetInlineComment() + wxT("\n");
            
            //========================================================
            #else
            //========================================================
            
            // This writes out initial value string (LOJ: 2010.09.21)
            wxString mapstr = GmatStringUtil::ToString(i+1, 1) + wxT(",") +
               GmatStringUtil::ToString(j+1, 1);
            
            #ifdef DEBUG_INITIAL_VALUE
            MessageInterface::ShowMessage
               (wxT("Array::GetInitialValueString() mapstr='%s'\n"), mapstr.c_str());
            #endif
            
            wxString initialVal = wxT("No Initial Value");
            bool writeData = false;
            
            if (initialValueMap.find(mapstr) != initialValueMap.end())
               initialVal = initialValueMap[mapstr];
            
            #ifdef DEBUG_INITIAL_VALUE
            MessageInterface::ShowMessage(wxT("   initialVal='%s'\n"), initialVal.c_str());
            #endif
            
            if (GmatStringUtil::IsNumber(initialVal))
               if (mInitialValueType == 1)
                  writeData = true;
               else
                  writeData = false;
            else
               if (mInitialValueType == 1)
                  writeData = false;
               else
                  writeData = true;
            
            if (writeData)
            {
               data << prefix << wxT("GMAT ") << instanceName << wxT("(") << i+1 << wxT(", ") << j+1 << wxT(") = ")
                    << initialVal;
               data << GetInlineComment() + wxT("\n");
            }
            
            //========================================================
            #endif
            //========================================================
         }
      }
   }
   
   return data;
}

