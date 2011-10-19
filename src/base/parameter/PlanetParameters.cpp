//$Id: PlanetParameters.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                             File: PlanetParameters.cpp
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
// Created: 2004/12/13
//
/**
 * Implements planet related parameter classes.
 *   MHA, Longitude, Altitude(Geodetic), Latitude(Geodetic), LST
 */
//------------------------------------------------------------------------------

#include "PlanetParameters.hpp"


//==============================================================================
//                              MHA
//==============================================================================
/**
 * Implements Mean Hour Angle parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// MHA(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
MHA::MHA(const wxString &name, GmatBase *obj)
   : PlanetReal(name, wxT("MHA"), obj, wxT("Greenwich Hour Angle"), wxT("deg"),
                Gmat::SPACECRAFT, GmatParam::ORIGIN)
{
   mDepObjectName = wxT("Earth");
   PlanetData::mCentralBodyName = wxT("Earth");
   SetRefObjectName(Gmat::SPACE_POINT, wxT("Earth"));
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, wxT("EarthFixed"));
   mIsAngleParam = true;
   mCycleType = GmatParam::ZERO_360;
}


//------------------------------------------------------------------------------
// MHA(const MHA &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
MHA::MHA(const MHA &copy)
   : PlanetReal(copy)
{
}


//------------------------------------------------------------------------------
// const MHA& operator=(const MHA &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const MHA&
MHA::operator=(const MHA &right)
{
   if (this != &right)
      PlanetReal::operator=(right);
   
   return *this;
}


//------------------------------------------------------------------------------
// ~MHA()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
MHA::~MHA()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool MHA::Evaluate()
{
   //mRealValue = PlanetData::GetPlanetReal(wxT("MHA"));    
   mRealValue = PlanetData::GetPlanetReal(MHA_ID);
   
   if (mRealValue == PlanetData::PLANET_REAL_UNDEFINED)
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
GmatBase* MHA::Clone(void) const
{
   return new MHA(*this);
}


//==============================================================================
//                              Longitude
//==============================================================================
/**
 * Implements Longitude parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Longitude(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
Longitude::Longitude(const wxString &name, GmatBase *obj)
   : PlanetReal(name, wxT("Longitude"), obj, wxT("Longitude"), wxT("deg"),
                Gmat::SPACECRAFT, GmatParam::ORIGIN)
{
   mDepObjectName = wxT("Earth");
   PlanetData::mCentralBodyName = wxT("Earth");
   SetRefObjectName(Gmat::SPACE_POINT, wxT("Earth"));
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, wxT("EarthFixed"));
   mIsAngleParam = true;
   mCycleType = GmatParam::PLUS_MINUS_180;
}


//------------------------------------------------------------------------------
// Longitude(const Longitude &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
Longitude::Longitude(const Longitude &copy)
   : PlanetReal(copy)
{
}


//------------------------------------------------------------------------------
// const Longitude& operator=(const Longitude &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const Longitude&
Longitude::operator=(const Longitude &right)
{
   if (this != &right)
      PlanetReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~Longitude()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
Longitude::~Longitude()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool Longitude::Evaluate()
{
   //mRealValue = PlanetData::GetPlanetReal(wxT("Longitude"));    
   mRealValue = PlanetData::GetPlanetReal(LONGITUDE);    
   
   if (mRealValue == PlanetData::PLANET_REAL_UNDEFINED)
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
GmatBase* Longitude::Clone(void) const
{
   return new Longitude(*this);
}


//==============================================================================
//                              Altitude
//==============================================================================
/**
 * Implements spcecraft altitude.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Altitude(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
Altitude::Altitude(const wxString &name, GmatBase *obj)
   : PlanetReal(name, wxT("Altitude"), obj, wxT("Altitude"), wxT("Km"),
                Gmat::SPACECRAFT, GmatParam::ORIGIN)
{
   mDepObjectName = wxT("Earth");
   PlanetData::mCentralBodyName = wxT("Earth");
   SetRefObjectName(Gmat::SPACE_POINT, wxT("Earth"));
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, wxT("EarthFixed"));
}


//------------------------------------------------------------------------------
// Altitude(const Altitude &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
Altitude::Altitude(const Altitude &copy)
   : PlanetReal(copy)
{
}


//------------------------------------------------------------------------------
// const Altitude& operator=(const Altitude &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const Altitude&
Altitude::operator=(const Altitude &right)
{
   if (this != &right)
      PlanetReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~Altitude()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
Altitude::~Altitude()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool Altitude::Evaluate()
{
   //mRealValue = PlanetData::GetPlanetReal(wxT("Altitude"));
   mRealValue = PlanetData::GetPlanetReal(ALTITUDE);
   
   if (mRealValue == PlanetData::PLANET_REAL_UNDEFINED)
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
GmatBase* Altitude::Clone(void) const
{
   return new Altitude(*this);
}


//==============================================================================
//                              Latitude
//==============================================================================
/**
 * Implements Latitude parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Latitude(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
Latitude::Latitude(const wxString &name, GmatBase *obj)
   : PlanetReal(name, wxT("Latitude"), obj, wxT("Latitude"), wxT("deg"),
                Gmat::SPACECRAFT, GmatParam::ORIGIN)
{
   mDepObjectName = wxT("Earth");
   PlanetData::mCentralBodyName = wxT("Earth");
   SetRefObjectName(Gmat::SPACE_POINT, wxT("Earth"));
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, wxT("EarthFixed"));
   mIsAngleParam = true;
   mCycleType = GmatParam::ZERO_360;
}


//------------------------------------------------------------------------------
// Latitude(const Latitude &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
Latitude::Latitude(const Latitude &copy)
   : PlanetReal(copy)
{
}


//------------------------------------------------------------------------------
// const Latitude& operator=(const Latitude &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const Latitude&
Latitude::operator=(const Latitude &right)
{
   if (this != &right)
      PlanetReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~Latitude()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
Latitude::~Latitude()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool Latitude::Evaluate()
{
   //mRealValue = PlanetData::GetPlanetReal(wxT("Latitude"));    
   mRealValue = PlanetData::GetPlanetReal(LATITUDE);    
   
   if (mRealValue == PlanetData::PLANET_REAL_UNDEFINED)
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
GmatBase* Latitude::Clone(void) const
{
   return new Latitude(*this);
}


//==============================================================================
//                              LST
//==============================================================================
/**
 * Implements LST parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// LST(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
LST::LST(const wxString &name, GmatBase *obj)
   : PlanetReal(name, wxT("LST"), obj, wxT("Local Sidereal Time"), wxT("deg"),
                Gmat::SPACECRAFT, GmatParam::ORIGIN)
{
   mDepObjectName = wxT("Earth");
   PlanetData::mCentralBodyName = wxT("Earth");
   SetRefObjectName(Gmat::SPACE_POINT, wxT("Earth"));
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, wxT("EarthFixed"));
   mIsAngleParam = true;
   mCycleType = GmatParam::ZERO_360;
}


//------------------------------------------------------------------------------
// LST(const LST &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
LST::LST(const LST &copy)
   : PlanetReal(copy)
{
}


//------------------------------------------------------------------------------
// const LST& operator=(const LST &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const LST&
LST::operator=(const LST &right)
{
   if (this != &right)
      PlanetReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~LST()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
LST::~LST()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool LST::Evaluate()
{
   //mRealValue = PlanetData::GetPlanetReal(wxT("LST"));    
   mRealValue = PlanetData::GetPlanetReal(LST_ID);    
   
   if (mRealValue == PlanetData::PLANET_REAL_UNDEFINED)
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
GmatBase* LST::Clone(void) const
{
   return new LST(*this);
}
