//$Id: SphericalParameters.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              SphericalParameters
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
 * Implements Spacecraft Spehrical parameter classes.
 *   SphRMag, SphRA, SphDec, SphVMag, SphRAV, SphDecV, SphAzi, SphElem
 */
//------------------------------------------------------------------------------

#include "SphericalParameters.hpp"

//==============================================================================
//                              SphRMag
//==============================================================================
/**
 * Implements Magnitude of Position.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SphRMag(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
SphRMag::SphRMag(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("RMAG"), obj, wxT("Spherical R mag"), wxT("Km"),
               GmatParam::ORIGIN, true)
{
   mDepObjectName = wxT("Earth");
   SetRefObjectName(Gmat::SPACE_POINT, wxT("Earth")); //loj: 4/7/05 Added
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, wxT("EarthMJ2000Eq"));
}


//------------------------------------------------------------------------------
// SphRMag(const SphRMag &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphRMag::SphRMag(const SphRMag &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// SphRMag& operator=(const SphRMag &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphRMag& SphRMag::operator=(const SphRMag &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~SphRMag()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
SphRMag::~SphRMag()
{
}


//--------------------------------------
// Inherited methods from Parameter
//--------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool SphRMag::Evaluate()
{
   mRealValue = OrbitData::GetSphRaDecReal(RD_RMAG);    
    
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* SphRMag::Clone(void) const
{
   return new SphRMag(*this);
}


//==============================================================================
//                              SphRA
//==============================================================================
/**
 * Implements Spherical Declination parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SphRA(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
SphRA::SphRA(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("RA"), obj, wxT("Sph. Right Ascension"), wxT("Deg"),
               GmatParam::COORD_SYS, true)
{
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
   mIsAngleParam = true;
   mCycleType = GmatParam::PLUS_MINUS_180;
}


//------------------------------------------------------------------------------
// SphRA(const SphRA &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphRA::SphRA(const SphRA &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// SphRA& operator=(const SphRA &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphRA& SphRA::operator=(const SphRA &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~SphRA()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
SphRA::~SphRA()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool SphRA::Evaluate()
{
   mRealValue = OrbitData::GetSphRaDecReal(RD_RRA);    
    
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* SphRA::Clone(void) const
{
   return new SphRA(*this);
}


//==============================================================================
//                              SphDec
//==============================================================================
/**
 * Implements Spherical Declination parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SphDec(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
SphDec::SphDec(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("DEC"), obj, wxT("Sph. Declination"), wxT("Deg"),
               GmatParam::COORD_SYS, true)
{
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
   mIsAngleParam = true;
   mCycleType = GmatParam::PLUS_MINUS_90;
}


//------------------------------------------------------------------------------
// SphDec(const SphDec &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphDec::SphDec(const SphDec &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// SphDec& operator=(const SphDec &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphDec& SphDec::operator=(const SphDec &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~SphDec()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
SphDec::~SphDec()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool SphDec::Evaluate()
{
   mRealValue = OrbitData::GetSphRaDecReal(RD_RDEC);    
    
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* SphDec::Clone(void) const
{
   return new SphDec(*this);
}


//==============================================================================
//                              SphVMag
//==============================================================================
/**
 * Implements Magnitude of Velocity.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SphVMag(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
SphVMag::SphVMag(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("VMAG"), obj, wxT("Sph. Mag of Velocity"), wxT("Km/s"),
               GmatParam::COORD_SYS, true)
{
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
}


//------------------------------------------------------------------------------
// SphVMag(const SphVMag &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphVMag::SphVMag(const SphVMag &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// SphVMag& operator=(const SphVMag &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphVMag& SphVMag::operator=(const SphVMag &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~SphVMag()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
SphVMag::~SphVMag()
{
}


//--------------------------------------
// Inherited methods from Parameter
//--------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool SphVMag::Evaluate()
{
   mRealValue = OrbitData::GetSphRaDecReal(RD_VMAG);    
    
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* SphVMag::Clone(void) const
{
   return new SphVMag(*this);
}


//==============================================================================
//                              SphRAV
//==============================================================================
/**
 * Implements Spherical Right Ascension of Velocity parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SphRAV(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
SphRAV::SphRAV(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("RAV"), obj, wxT("Sph. RA of Velocity"), wxT("Deg"),
               GmatParam::COORD_SYS, true)
{
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
   mIsAngleParam = true;
   mCycleType = GmatParam::PLUS_MINUS_180;
}


//------------------------------------------------------------------------------
// SphRAV(const SphRAV &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphRAV::SphRAV(const SphRAV &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// SphRAV& operator=(const SphRAV &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphRAV& SphRAV::operator=(const SphRAV &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~SphRAV()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
SphRAV::~SphRAV()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool SphRAV::Evaluate()
{
   mRealValue = OrbitData::GetSphRaDecReal(RD_RAV);    
   
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* SphRAV::Clone(void) const
{
   return new SphRAV(*this);
}


//==============================================================================
//                              SphDecV
//==============================================================================
/**
 * Implements Spherical Declination of Velocity parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SphDecV(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
SphDecV::SphDecV(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("DECV"), obj, wxT("Sph. Dec of Velocity"), wxT("Deg"),
               GmatParam::COORD_SYS, true)
{
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
   mIsAngleParam = true;
   mCycleType = GmatParam::PLUS_MINUS_90;
}


//------------------------------------------------------------------------------
// SphDecV(const SphDecV &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphDecV::SphDecV(const SphDecV &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// SphDecV& operator=(const SphDecV &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphDecV& SphDecV::operator=(const SphDecV &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}

//------------------------------------------------------------------------------
// ~SphDecV()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
SphDecV::~SphDecV()
{
}

//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool SphDecV::Evaluate()
{
   mRealValue = OrbitData::GetSphRaDecReal(RD_DECV);    
    
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* SphDecV::Clone(void) const
{
   return new SphDecV(*this);
}


//==============================================================================
//                              SphAzi
//==============================================================================
/**
 * Implements Spherical Right Ascension of Velocity parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SphAzi(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
SphAzi::SphAzi(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("AZI"), obj, wxT("Sph. RA of Velocity"), wxT("Deg"),
               GmatParam::COORD_SYS, true)
{
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
   mIsAngleParam = true;
   mCycleType = GmatParam::PLUS_MINUS_180;
}


//------------------------------------------------------------------------------
// SphAzi(const SphAzi &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphAzi::SphAzi(const SphAzi &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// SphAzi& operator=(const SphAzi &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphAzi& SphAzi::operator=(const SphAzi &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~SphAzi()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
SphAzi::~SphAzi()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool SphAzi::Evaluate()
{
   mRealValue = OrbitData::GetSphAzFpaReal(AF_AZI);    
    
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* SphAzi::Clone(void) const
{
   return new SphAzi(*this);
}


//==============================================================================
//                              SphFPA
//==============================================================================
/**
 * Implements Spherical Declination of Velocity parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SphFPA(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
SphFPA::SphFPA(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("FPA"), obj, wxT("Sph. Dec of Velocity"), wxT("Deg"),
               GmatParam::COORD_SYS, true)
{
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
}


//------------------------------------------------------------------------------
// SphFPA(const SphFPA &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphFPA::SphFPA(const SphFPA &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// SphFPA& operator=(const SphFPA &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphFPA& SphFPA::operator=(const SphFPA &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~SphFPA()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
SphFPA::~SphFPA()
{
}

//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool SphFPA::Evaluate()
{
   mRealValue = OrbitData::GetSphAzFpaReal(AF_FPA);    
    
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* SphFPA::Clone(void) const
{
   return new SphFPA(*this);
}


//==============================================================================
//                              SphRaDecElem
//==============================================================================
/**
 * Implements Spherical RA/DEC type Elements class.
 *   Elements are SphRMag, SphRA, SphDec, SphVMag, SphRAV, SphDecV
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SphRaDecElem(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
SphRaDecElem::SphRaDecElem(const wxString &name, GmatBase *obj)
   : OrbitRvec6(name, wxT("SphericalRADEC"), obj, wxT("Spherical Elements"), wxT(" "),
                GmatParam::COORD_SYS)
{
   // Parameter member data
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
   mIsPlottable = false;
}


//------------------------------------------------------------------------------
// SphRaDecElem(const SphRaDecElem &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphRaDecElem::SphRaDecElem(const SphRaDecElem &copy)
   : OrbitRvec6(copy)
{
}


//------------------------------------------------------------------------------
// SphRaDecElem& operator=(const SphRaDecElem &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphRaDecElem& SphRaDecElem::operator=(const SphRaDecElem &right)
{
   if (this != &right)
      OrbitRvec6::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~SphRaDecElem()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
SphRaDecElem::~SphRaDecElem()
{
}


//--------------------------------------
// Inherited methods from Parameter
//--------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool SphRaDecElem::Evaluate()
{
   mRvec6Value = GetSphRaDecState();

   return mRvec6Value.IsValid(GmatOrbitConstants::ORBIT_REAL_UNDEFINED);
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* SphRaDecElem::Clone(void) const
{
   return new SphRaDecElem(*this);
}


//==============================================================================
//                              SphAzFpaElem
//==============================================================================
/**
 * Implements Spherical AZI/FPA type Elements class.
 *   Elements are SphRMag, SphRA, SphDec, SphVMag, SphAzi, SphFPA
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SphAzFpaElem(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
SphAzFpaElem::SphAzFpaElem(const wxString &name, GmatBase *obj)
   : OrbitRvec6(name, wxT("SphericalAZFPA"), obj, wxT("Spherical Elements"), wxT(" "),
                GmatParam::COORD_SYS)
{
   // Parameter member data
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
   mIsPlottable = false;
}


//------------------------------------------------------------------------------
// SphAzFpaElem(const SphAzFpaElem &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphAzFpaElem::SphAzFpaElem(const SphAzFpaElem &copy)
   : OrbitRvec6(copy)
{
}


//------------------------------------------------------------------------------
// SphAzFpaElem& operator=(const SphAzFpaElem &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SphAzFpaElem& SphAzFpaElem::operator=(const SphAzFpaElem &right)
{
   if (this != &right)
      OrbitRvec6::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~SphAzFpaElem()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
SphAzFpaElem::~SphAzFpaElem()
{
}


//--------------------------------------
// Inherited methods from Parameter
//--------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool SphAzFpaElem::Evaluate()
{
   mRvec6Value = GetSphAzFpaState();

   return mRvec6Value.IsValid(GmatOrbitConstants::ORBIT_REAL_UNDEFINED);
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* SphAzFpaElem::Clone(void) const
{
   return new SphAzFpaElem(*this);
}
