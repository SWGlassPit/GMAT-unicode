//$Id: Array.hpp 9513 2011-04-30 21:23:06Z djcinsb $
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
 * Declares Array class which handles one or two dimentional array.
 */
//------------------------------------------------------------------------------
#ifndef Array_hpp
#define Array_hpp

#include "gmatdefs.hpp"
#include "Parameter.hpp"
#include "Rmatrix.hpp"
#include "Rvector.hpp"

class GMAT_API Array : public Parameter
{
public:
   
   Array(const wxString &name = wxT(""), const wxString &desc = wxT(""),
         const wxString &unit = wxT(""));
   Array(const Array &copy);
   Array& operator= (const Array& right);
   virtual ~Array();
   
   bool operator==(const Array &right) const;
   bool operator!=(const Array &right) const;
   
   bool IsSized() { return mSizeSet; }
   bool SetSize(const Integer row, const Integer col);
   void GetSize(Integer &row, Integer &col) { row = mNumRows; col = mNumCols; }
   Integer GetRowCount() { return mNumRows; }
   Integer GetColCount() { return mNumCols; }
   
   // methods inherited from Parameter
   virtual void SetRmatrix(const Rmatrix &mat);
   virtual const Rmatrix& GetRmatrix() const { return mRmatValue; }
   virtual const Rmatrix& EvaluateRmatrix() { return mRmatValue; } /// assumes it has only numbers
   virtual wxString ToString();
   virtual const wxString* GetParameterList() const;
   
   // methods inherited from GmatBase
   virtual GmatBase* Clone() const;
   virtual void Copy(const GmatBase*);
   virtual wxString GetParameterText(const Integer id) const;
   virtual Integer GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType GetParameterType(const Integer id) const;
   virtual wxString GetParameterTypeString(const Integer id) const;
   virtual bool IsParameterReadOnly(const Integer id) const;
   
   virtual Integer GetIntegerParameter(const Integer id) const;
   virtual Integer GetIntegerParameter(const wxString &label) const;
   virtual Integer SetIntegerParameter(const Integer id, const Integer value);
   virtual Integer SetIntegerParameter(const wxString &label,
                                       const Integer value);
   
   virtual const Rmatrix& GetRmatrixParameter(const Integer id) const;
   virtual const Rmatrix& GetRmatrixParameter(const wxString &label) const;
   virtual const Rmatrix& SetRmatrixParameter(const Integer id,
                                              const Rmatrix &value);
   virtual const Rmatrix& SetRmatrixParameter(const wxString &label,
                                              const Rmatrix &value);
   
   virtual Real GetRealParameter(const Integer id, const Integer index) const;
   virtual Real GetRealParameter(const wxString &label,
                                 const Integer index) const;
   
   virtual Real GetRealParameter(const Integer id, const Integer row,
                                 const Integer col) const;
   virtual Real GetRealParameter(const wxString &label, const Integer row,
                                 const Integer col) const;
   
   virtual Real SetRealParameter(const Integer id, const Real value,
                                 const Integer row, const Integer col);
   virtual Real SetRealParameter(const wxString &label,
                                 const Real value, const Integer row,
                                 const Integer col);
   
   virtual Rvector GetRvectorParameter(const Integer id,
                                       const Integer index) const;
   virtual Rvector GetRvectorParameter(const wxString &label,
                                       const Integer index) const;
   
   virtual const Rvector& SetRvectorParameter(const Integer id,
                                              const Rvector &value,
                                              const Integer index);   
   virtual const Rvector& SetRvectorParameter(const wxString &label,
                                              const Rvector &value,
                                              const Integer index);
   
   virtual wxString GetStringParameter(const Integer id) const;
   virtual wxString GetStringParameter(const wxString &label) const;
   virtual bool SetStringParameter(const Integer id, const wxString &value);
   virtual bool SetStringParameter(const wxString &label,
                                   const wxString &value);
   
   virtual const wxString&
                        GetGeneratingString(
                           Gmat::WriteMode mode = Gmat::SCRIPTING,
                           const wxString &prefix = wxT(""),
                           const wxString &useName = wxT(""));
protected:

   Integer mNumRows;
   Integer mNumCols;
   Integer mInitialValueType; // 1 = number, 2 = Variable or other Array
   bool mSizeSet;
   Rmatrix mRmatValue;
   std::map<wxString, wxString> initialValueMap;
   
   wxString GetArrayDefString() const;
   wxString GetInitialValueString(const wxString &prefix = wxT(""));
   
   enum
   {
      NUM_ROWS = ParameterParamCount,
      NUM_COLS,
      RMAT_VALUE,
      SINGLE_VALUE,
      ROW_VALUE,
      COL_VALUE,
      INITIAL_VALUE,
      INITIAL_VALUE_TYPE,
      ArrayParamCount
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[ArrayParamCount - ParameterParamCount];
   static const wxString
      PARAMETER_TEXT[ArrayParamCount - ParameterParamCount];
    
private:

};
#endif // Array_hpp
