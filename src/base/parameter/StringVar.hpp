//$Id: StringVar.hpp 9513 2011-04-30 21:23:06Z djcinsb $
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
 * Declares StringVar class which handles wxString value. The string value
 * is stored in Parameter::mExpr.
 */
//------------------------------------------------------------------------------
#ifndef StringVar_hpp
#define StringVar_hpp

#include "gmatdefs.hpp"
#include "Parameter.hpp"

class GMAT_API StringVar : public Parameter
{
public:

   StringVar(const wxString &name = wxT(""),
             const wxString &typeStr = wxT("String"),
             GmatParam::ParameterKey key = GmatParam::USER_PARAM,
             GmatBase *obj = NULL, const wxString &desc = wxT(""),
             const wxString &unit = wxT(""),
             GmatParam::DepObject depObj = GmatParam::NO_DEP,
             Gmat::ObjectType ownerType = Gmat::UNKNOWN_OBJECT,
             bool isTimeParam = false);
   StringVar(const StringVar &copy);
   StringVar& operator= (const StringVar& right);
   virtual ~StringVar();
   
   bool operator==(const StringVar &right) const;
   bool operator!=(const StringVar &right) const;

   // methods inherited from Parameter
   virtual wxString ToString();
   virtual const wxString& GetString() const;
   virtual const wxString& EvaluateString();
   
   // methods inherited from GmatBase
   virtual GmatBase* Clone() const;
   virtual void Copy(const GmatBase*);
   
   virtual Integer GetParameterID(const wxString &str) const;
   
   virtual wxString GetStringParameter(const Integer id) const;
   virtual wxString GetStringParameter(const wxString &label) const;
   virtual bool SetStringParameter(const Integer id, const wxString &value);
   virtual bool SetStringParameter(const wxString &label,
                                   const wxString &value);
   
   virtual const wxString& GetGeneratingString(Gmat::WriteMode mode,
                                                  const wxString &prefix,
                                                  const wxString &useName);
protected:
   
   enum
   {
      VALUE = ParameterParamCount,
      StringVarParamCount
   };
   
   static const wxString
      PARAMETER_TEXT[StringVarParamCount - ParameterParamCount];
   static const Gmat::ParameterType
      PARAMETER_TYPE[StringVarParamCount - ParameterParamCount];
   
   wxString mStringValue;
   
private:

};
#endif // StringVar_hpp
