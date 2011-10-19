//$Id: RealVar.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  RealVar
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
// Created: 2004/01/08
//
/**
 * Declares base class of parameters returning Real.
 */
//------------------------------------------------------------------------------
#ifndef RealVar_hpp
#define RealVar_hpp

#include "gmatdefs.hpp"
#include "Parameter.hpp"

class GMAT_API RealVar : public Parameter
{
public:

   RealVar(const wxString &name = wxT(""), const wxString &valStr = wxT(""),
           const wxString &typeStr = wxT("RealVar"),
           GmatParam::ParameterKey key = GmatParam::USER_PARAM,
           GmatBase *obj = NULL, const wxString &desc = wxT(""),
           const wxString &unit = wxT(""),
           GmatParam::DepObject depObj = GmatParam::NO_DEP,
           Gmat::ObjectType ownerType = Gmat::UNKNOWN_OBJECT,
           bool isTimeParam = false, bool isSettable = false);
   RealVar(const RealVar &copy);
   RealVar& operator= (const RealVar& right);
   virtual ~RealVar();
   
   bool operator==(const RealVar &right) const;
   bool operator!=(const RealVar &right) const;
   
   // methods inherited from Parameter
   virtual bool Initialize();
   virtual wxString ToString();
   
   virtual Real GetReal() const;
   virtual void SetReal(Real val);
   
   virtual Integer GetParameterID(const wxString &str) const;
   
   virtual Real GetRealParameter(const Integer id) const;
   virtual Real GetRealParameter(const wxString &label) const;
   
   virtual Real SetRealParameter(const Integer id, const Real value);
   virtual Real SetRealParameter(const wxString &label, const Real value);
   
   virtual bool SetStringParameter(const Integer id, const wxString &value);
   virtual bool SetStringParameter(const wxString &label,
                                   const wxString &value);
protected:

   enum
   {
      VALUE = ParameterParamCount,
      RealVarParamCount
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[RealVarParamCount - ParameterParamCount];
   static const wxString
      PARAMETER_TEXT[RealVarParamCount - ParameterParamCount];
   
   bool mValueSet;
   bool mIsNumber;
   Real mRealValue;
   
private:

};
#endif // RealVar_hpp
