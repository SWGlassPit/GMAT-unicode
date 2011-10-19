//$Id: TimeParameters.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              File: TimeParameters.hpp
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
// Created: 2004/03/12
//
/**
 * Declares Time related parameter classes.
 *   CurrA1MJD, A1ModJulian, A1Gregorian, TAIModJulian, TAIGregorian, TTModJulian,
 *   TTGregorian, TDBModJulian, TDBGregorian, TCBModJulian, TCBGregorian,
 *   UTCModJulian, UGCGregorian, ElapsedDays, ElapsedDaysFromStart, ElapsedSecs,
 *   ElapsedSecsFromStart
 */
//------------------------------------------------------------------------------
#ifndef TimeParameters_hpp
#define TimeParameters_hpp

#include "TimeReal.hpp"
#include "TimeString.hpp"


//==============================================================================
//                              CurrA1MJD
//==============================================================================

class GMAT_API CurrA1MJD : public TimeReal
{
public:

   CurrA1MJD(const wxString &name = wxT(""), GmatBase *obj = NULL);
   CurrA1MJD(const CurrA1MJD &copy);
   CurrA1MJD& operator= (const CurrA1MJD &right); 
   virtual ~CurrA1MJD();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:
private:

};


//==============================================================================
//                              A1ModJulian
//==============================================================================

class GMAT_API A1ModJulian : public TimeReal
{
public:

   A1ModJulian(const wxString &name = wxT(""), GmatBase *obj = NULL);
   A1ModJulian(const A1ModJulian &copy);
   A1ModJulian& operator= (const A1ModJulian &right); 
   virtual ~A1ModJulian();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:
private:

};


//==============================================================================
//                              A1Gregorian
//==============================================================================

class GMAT_API A1Gregorian : public TimeString
{
public:

   A1Gregorian(const wxString &name = wxT(""), GmatBase *obj = NULL);
   A1Gregorian(const A1Gregorian &copy);
   A1Gregorian& operator= (const A1Gregorian &right); 
   virtual ~A1Gregorian();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:
private:

};


//==============================================================================
//                              TAIModJulian
//==============================================================================

class GMAT_API TAIModJulian : public TimeReal
{
public:

   TAIModJulian(const wxString &name = wxT(""), GmatBase *obj = NULL);
   TAIModJulian(const TAIModJulian &copy);
   TAIModJulian& operator= (const TAIModJulian &right); 
   virtual ~TAIModJulian();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:
private:

};


//==============================================================================
//                              TAIGregorian
//==============================================================================

class GMAT_API TAIGregorian : public TimeString
{
public:

   TAIGregorian(const wxString &name = wxT(""), GmatBase *obj = NULL);
   TAIGregorian(const TAIGregorian &copy);
   TAIGregorian& operator= (const TAIGregorian &right); 
   virtual ~TAIGregorian();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:
private:

};


//==============================================================================
//                              TTModJulian
//==============================================================================

class GMAT_API TTModJulian : public TimeReal
{
public:

   TTModJulian(const wxString &name = wxT(""), GmatBase *obj = NULL);
   TTModJulian(const TTModJulian &copy);
   TTModJulian& operator= (const TTModJulian &right); 
   virtual ~TTModJulian();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:
private:

};


//==============================================================================
//                              TTGregorian
//==============================================================================

class GMAT_API TTGregorian : public TimeString
{
public:

   TTGregorian(const wxString &name = wxT(""), GmatBase *obj = NULL);
   TTGregorian(const TTGregorian &copy);
   TTGregorian& operator= (const TTGregorian &right); 
   virtual ~TTGregorian();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:
private:

};


//==============================================================================
//                              TDBModJulian
//==============================================================================

class GMAT_API TDBModJulian : public TimeReal
{
public:

   TDBModJulian(const wxString &name = wxT(""), GmatBase *obj = NULL);
   TDBModJulian(const TDBModJulian &copy);
   TDBModJulian& operator= (const TDBModJulian &right); 
   virtual ~TDBModJulian();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:
private:

};


//==============================================================================
//                              TDBGregorian
//==============================================================================

class GMAT_API TDBGregorian : public TimeString
{
public:

   TDBGregorian(const wxString &name = wxT(""), GmatBase *obj = NULL);
   TDBGregorian(const TDBGregorian &copy);
   TDBGregorian& operator= (const TDBGregorian &right); 
   virtual ~TDBGregorian();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:
private:

};


//==============================================================================
//                              TCBModJulian
//==============================================================================

class GMAT_API TCBModJulian : public TimeReal
{
public:

   TCBModJulian(const wxString &name = wxT(""), GmatBase *obj = NULL);
   TCBModJulian(const TCBModJulian &copy);
   TCBModJulian& operator= (const TCBModJulian &right); 
   virtual ~TCBModJulian();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:
private:

};


//==============================================================================
//                              TCBGregorian
//==============================================================================

class GMAT_API TCBGregorian : public TimeString
{
public:

   TCBGregorian(const wxString &name = wxT(""), GmatBase *obj = NULL);
   TCBGregorian(const TCBGregorian &copy);
   TCBGregorian& operator= (const TCBGregorian &right); 
   virtual ~TCBGregorian();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:
private:

};


//==============================================================================
//                              UTCModJulian
//==============================================================================

class GMAT_API UTCModJulian : public TimeReal
{
public:

   UTCModJulian(const wxString &name = wxT(""), GmatBase *obj = NULL);
   UTCModJulian(const UTCModJulian &copy);
   UTCModJulian& operator= (const UTCModJulian &right); 
   virtual ~UTCModJulian();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:
private:

};


//==============================================================================
//                              UTCGregorian
//==============================================================================

class GMAT_API UTCGregorian : public TimeString
{
public:

   UTCGregorian(const wxString &name = wxT(""), GmatBase *obj = NULL);
   UTCGregorian(const UTCGregorian &copy);
   UTCGregorian& operator= (const UTCGregorian &right); 
   virtual ~UTCGregorian();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:
private:

};


//==============================================================================
//                              ElapsedDays
//==============================================================================

class GMAT_API ElapsedDays : public TimeReal
{
public:

   ElapsedDays(const wxString &name = wxT(""), GmatBase *obj = NULL);
   ElapsedDays(const ElapsedDays &copy);
   const ElapsedDays& operator= (const ElapsedDays &right); 
   virtual ~ElapsedDays();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
   virtual Gmat::ParameterType GetParameterType(const Integer id) const;
   virtual wxString GetParameterTypeString(const Integer id) const;
   virtual wxString GetParameterText(const Integer id) const;
   virtual bool IsParameterReadOnly(const Integer id) const;
   virtual Integer GetParameterID(const wxString &str) const;
   
   virtual Real GetRealParameter(const Integer id) const;
   virtual Real GetRealParameter(const wxString &label) const;
   virtual Real SetRealParameter(const Integer id, const Real value);
   virtual Real SetRealParameter(const wxString &label, const Real value);
    
protected:

   enum
   {
      INITIAL_EPOCH = ParameterParamCount,
      ElapsedDaysParamCount
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[ElapsedDaysParamCount - ParameterParamCount];
   static const wxString
      PARAMETER_TEXT[ElapsedDaysParamCount - ParameterParamCount];
    
private:
};


//==============================================================================
//                              ElapsedDaysFromStart
//==============================================================================

class GMAT_API ElapsedDaysFromStart : public TimeReal
{
public:

   ElapsedDaysFromStart(const wxString &name = wxT(""), GmatBase *obj = NULL);
   ElapsedDaysFromStart(const ElapsedDaysFromStart &copy);
   const ElapsedDaysFromStart& operator= (const ElapsedDaysFromStart &right); 
   virtual ~ElapsedDaysFromStart();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
   virtual Gmat::ParameterType GetParameterType(const Integer id) const;
   virtual wxString GetParameterTypeString(const Integer id) const;
   virtual wxString GetParameterText(const Integer id) const;
   virtual bool IsParameterReadOnly(const Integer id) const;
   virtual Integer GetParameterID(const wxString &str) const;
   
   virtual Real GetRealParameter(const Integer id) const;
   virtual Real GetRealParameter(const wxString &label) const;
   virtual Real SetRealParameter(const Integer id, const Real value);
   virtual Real SetRealParameter(const wxString &label, const Real value);
       
protected:    
   enum
   {
      INITIAL_EPOCH = ParameterParamCount,
      ElapsedDaysFromStartParamCount
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[ElapsedDaysFromStartParamCount - ParameterParamCount];
   static const wxString
      PARAMETER_TEXT[ElapsedDaysFromStartParamCount - ParameterParamCount];
    
private:
};


//==============================================================================
//                              ElapsedSecs
//==============================================================================

class GMAT_API ElapsedSecs : public TimeReal
{
public:

   ElapsedSecs(const wxString &name = wxT(""), GmatBase *obj = NULL);
   ElapsedSecs(const ElapsedSecs &copy);
   const ElapsedSecs& operator= (const ElapsedSecs &right); 
   virtual ~ElapsedSecs();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
   virtual Gmat::ParameterType GetParameterType(const Integer id) const;
   virtual wxString GetParameterTypeString(const Integer id) const;
   virtual wxString GetParameterText(const Integer id) const;
   virtual bool IsParameterReadOnly(const Integer id) const;
   virtual Integer GetParameterID(const wxString &str) const;
   
   virtual Real GetRealParameter(const Integer id) const;
   virtual Real GetRealParameter(const wxString &label) const;
   virtual Real SetRealParameter(const Integer id, const Real value);
   virtual Real SetRealParameter(const wxString &label, const Real value);
   
protected:
   
   enum
   {
      INITIAL_EPOCH = ParameterParamCount,
      ElapsedSecsParamCount
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[ElapsedSecsParamCount - ParameterParamCount];
   static const wxString
      PARAMETER_TEXT[ElapsedSecsParamCount - ParameterParamCount];
   
private:
};


//==============================================================================
//                              ElapsedSecsFromStart
//==============================================================================

class GMAT_API ElapsedSecsFromStart : public TimeReal
{
public:

   ElapsedSecsFromStart(const wxString &name = wxT(""), GmatBase *obj = NULL);
   ElapsedSecsFromStart(const ElapsedSecsFromStart &copy);
   const ElapsedSecsFromStart& operator= (const ElapsedSecsFromStart &right); 
   virtual ~ElapsedSecsFromStart();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
      
   virtual Gmat::ParameterType GetParameterType(const Integer id) const;
   virtual wxString GetParameterTypeString(const Integer id) const;
   virtual wxString GetParameterText(const Integer id) const;
   virtual bool IsParameterReadOnly(const Integer id) const;
   virtual Integer GetParameterID(const wxString &str) const;
   
   virtual Real GetRealParameter(const Integer id) const;
   virtual Real GetRealParameter(const wxString &label) const;
   virtual Real SetRealParameter(const Integer id, const Real value);
   virtual Real SetRealParameter(const wxString &label, const Real value);
   
protected:   
   enum
   {
      INITIAL_EPOCH = ParameterParamCount,
      ElapsedSecsFromStartParamCount
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[ElapsedSecsFromStartParamCount - ParameterParamCount];
   static const wxString
      PARAMETER_TEXT[ElapsedSecsFromStartParamCount - ParameterParamCount];
    
private:
};


#endif // TimeParameters_hpp
