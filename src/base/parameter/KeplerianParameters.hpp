//$Id: KeplerianParameters.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                             File: KeplerianParameters.hpp
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
 * Declares Keplerian related parameter classes.
 *   KepSMA, KepEcc, KepInc, KepAOP, KepRAAN, KepTA, KepMA, KepMM, KepElem,
 *   ModKepElem
 */
//------------------------------------------------------------------------------
#ifndef KeplerianParameters_hpp
#define KeplerianParameters_hpp

#include "gmatdefs.hpp"
#include "OrbitReal.hpp"
#include "OrbitRvec6.hpp"

//==============================================================================
//                              KepSMA
//==============================================================================
/**
 * Declares Keplerian Semimajor Axis class.
 */
//------------------------------------------------------------------------------

class GMAT_API KepSMA : public OrbitReal
{
public:

   KepSMA(const wxString &name = wxT(""), GmatBase *obj = NULL);
   KepSMA(const KepSMA &copy);
   const KepSMA& operator=(const KepSMA &right);
   virtual ~KepSMA();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;

protected:

};

//==============================================================================
//                              KepEcc
//==============================================================================
/**
 * Declares Keplerian Eccentricity class.
 */
//------------------------------------------------------------------------------

class GMAT_API KepEcc : public OrbitReal
{
public:

   KepEcc(const wxString &name = wxT(""), GmatBase *obj = NULL);
   KepEcc(const KepEcc &param);
   const KepEcc& operator=(const KepEcc &right);
   virtual ~KepEcc();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;

protected:

};


//==============================================================================
//                              KepInc
//==============================================================================
/**
 * Declares Keplerian Inclinatin class.
 */
//------------------------------------------------------------------------------

class GMAT_API KepInc : public OrbitReal
{
public:

   KepInc(const wxString &name = wxT(""), GmatBase *obj = NULL);
   KepInc(const KepInc &copy);
   const KepInc& operator=(const KepInc &right);
   virtual ~KepInc();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;

protected:

};

//==============================================================================
//                              KepAOP
//==============================================================================
/**
 * Declares Keplerian Argument of Periapsis class.
 */
//------------------------------------------------------------------------------

class GMAT_API KepAOP : public OrbitReal
{
public:

   KepAOP(const wxString &name = wxT(""), GmatBase *obj = NULL);
   KepAOP(const KepAOP &copy);
   const KepAOP& operator=(const KepAOP &right);
   virtual ~KepAOP();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;

protected:

};

//==============================================================================
//                              KepRAAN
//==============================================================================
/**
 * Declares Keplerian Right Ascention of Ascending Node class.
 */
//------------------------------------------------------------------------------

class GMAT_API KepRAAN : public OrbitReal
{
public:

   KepRAAN(const wxString &name = wxT(""), GmatBase *obj = NULL);
   KepRAAN(const KepRAAN &copy);
   const KepRAAN& operator=(const KepRAAN &right);
   virtual ~KepRAAN();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;

protected:

};


//==============================================================================
//                              KepRADN
//==============================================================================
/**
 * Declares Keplerian Right Ascention of Ascending Node class.
 */
//------------------------------------------------------------------------------

class GMAT_API KepRADN : public OrbitReal
{
public:

   KepRADN(const wxString &name = wxT(""), GmatBase *obj = NULL);
   KepRADN(const KepRADN &copy);
   const KepRADN& operator=(const KepRADN &right);
   virtual ~KepRADN();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;

protected:

};


//==============================================================================
//                              KepTA
//==============================================================================
/**
 * Declares Keplerian True Anomaly class.
 */
//------------------------------------------------------------------------------

class GMAT_API KepTA : public OrbitReal
{
public:

   KepTA(const wxString &name = wxT(""), GmatBase *obj = NULL);
   KepTA(const KepTA &copy);
   const KepTA& operator=(const KepTA &right);
   virtual ~KepTA();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;

protected:

};


//==============================================================================
//                              KepMA
//==============================================================================
/**
 * Declares Keplerian Mean Anomaly class.
 */
//------------------------------------------------------------------------------

class GMAT_API KepMA : public OrbitReal
{
public:

   KepMA(const wxString &name = wxT(""), GmatBase *obj = NULL);
   KepMA(const KepMA &copy);
   const KepMA& operator=(const KepMA &right);
   virtual ~KepMA();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;

protected:

};


//==============================================================================
//                              KepEA
//==============================================================================
/**
 * Declares Keplerian Mean Anomaly class.
 */
//------------------------------------------------------------------------------

class GMAT_API KepEA : public OrbitReal
{
public:

   KepEA(const wxString &name = wxT(""), GmatBase *obj = NULL);
   KepEA(const KepEA &copy);
   const KepEA& operator=(const KepEA &right);
   virtual ~KepEA();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;

protected:

};


//==============================================================================
//                              KepHA
//==============================================================================
/**
 * Declares Keplerian Mean Anomaly class.
 */
//------------------------------------------------------------------------------

class GMAT_API KepHA : public OrbitReal
{
public:

   KepHA(const wxString &name = wxT(""), GmatBase *obj = NULL);
   KepHA(const KepHA &copy);
   const KepHA& operator=(const KepHA &right);
   virtual ~KepHA();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;

protected:

};


//==============================================================================
//                              KepMM
//==============================================================================
/**
 * Declares Keplerian Mean Motion class.
 */
//------------------------------------------------------------------------------

class GMAT_API KepMM : public OrbitReal
{
public:

   KepMM(const wxString &name = wxT(""), GmatBase *obj = NULL);
   KepMM(const KepMM &copy);
   const KepMM& operator=(const KepMM &right);
   virtual ~KepMM();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   
protected:

};


//==============================================================================
//                              KepElem
//==============================================================================
/**
 * Declares Keplerian Elements class.
 *   6 elements: KepSMA, KepEcc, KepInc, KepRAAN, KepAOP, KepTA
 */
//------------------------------------------------------------------------------

class GMAT_API KepElem : public OrbitRvec6
{
public:

   KepElem(const wxString &name = wxT(""), GmatBase *obj = NULL);
   KepElem(const KepElem &copy);
   const KepElem& operator=(const KepElem &right);
   virtual ~KepElem();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;

protected:
    
};


//loj: 2/16/05 Added
//==============================================================================
//                              ModKepElem
//==============================================================================
/**
 * Declares Keplerian Elements class.
 *   6 elements: RadPeriapais, RadApoapsis, KepInc, KepRAAN, KepAOP, KepTA
 */
//------------------------------------------------------------------------------

class GMAT_API ModKepElem : public OrbitRvec6
{
public:

   ModKepElem(const wxString &name = wxT(""), GmatBase *obj = NULL);
   ModKepElem(const ModKepElem &copy);
   const ModKepElem& operator=(const ModKepElem &right);
   virtual ~ModKepElem();

   // methods inherited from Parameter
   virtual bool Evaluate();

   // methods inherited from GmatBase
   virtual GmatBase* Clone(void) const;

protected:
    
};

#endif // KeplerianParameters_hpp
