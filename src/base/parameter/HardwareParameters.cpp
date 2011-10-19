//$Id: HardwareParameters.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                            File: HardwareParameters
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc.
//
// Author: Linda Jun
// Created: 2009.03.20
//
/**
 * Implements Hardware related parameter classes.
 *    FuelTak : FuelMass, Pressure, Temperature, FuelVolume, FuelDensity
 *    Thruster: DutyCycle, ThrustScaleFactor, GravitationalAccel
 */
//------------------------------------------------------------------------------

#include "HardwareParameters.hpp"
#include "ColorTypes.hpp"
#include "ParameterInfo.hpp"


// To use preset colors, uncomment this line:
//#define USE_PREDEFINED_COLORS

//==============================================================================
//                              FuelMass
//==============================================================================
/**
 * Implements FuelMass class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// FuelMass(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
FuelMass::FuelMass(const wxString &name, GmatBase *obj)
   : HardwareReal(name, wxT("FuelMass"), obj, wxT("Fuel Mass"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::RED32;
   #endif
}


//------------------------------------------------------------------------------
// FuelMass(const FuelMass &copy)
//------------------------------------------------------------------------------
FuelMass::FuelMass(const FuelMass &copy)
   : HardwareReal(copy)
{
}


//------------------------------------------------------------------------------
// FuelMass& operator=(const FuelMass &right)
//------------------------------------------------------------------------------
FuelMass& FuelMass::operator=(const FuelMass &right)
{
   if (this != &right)
      HardwareReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~FuelMass()
//------------------------------------------------------------------------------
FuelMass::~FuelMass()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool FuelMass::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(FUEL_MASS);
   
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets value to the owner of the parameter.
 */
//------------------------------------------------------------------------------
void FuelMass::SetReal(Real val)
{
   SpacecraftData::SetReal(FUEL_MASS, val);
   RealVar::SetReal(val);
}


//------------------------------------------------------------------------------
// GmatBase* FuelMass::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* FuelMass::Clone(void) const
{
   return new FuelMass(*this);
}


//==============================================================================
//                              Pressure
//==============================================================================
/**
 * Implements Pressure class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Pressure(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
Pressure::Pressure(const wxString &name, GmatBase *obj)
   : HardwareReal(name, wxT("Pressure"), obj, wxT("Pressure"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::YELLOW32;
   #endif
}


//------------------------------------------------------------------------------
// Pressure(const Pressure &copy)
//------------------------------------------------------------------------------
Pressure::Pressure(const Pressure &copy)
   : HardwareReal(copy)
{
}


//------------------------------------------------------------------------------
// Pressure& operator=(const Pressure &right)
//------------------------------------------------------------------------------
Pressure& Pressure::operator=(const Pressure &right)
{
   if (this != &right)
      HardwareReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~Pressure()
//------------------------------------------------------------------------------
Pressure::~Pressure()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool Pressure::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(PRESSURE);
    
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets value to the owner of the parameter.
 */
//------------------------------------------------------------------------------
void Pressure::SetReal(Real val)
{
   SpacecraftData::SetReal(PRESSURE, val);
   RealVar::SetReal(val);
}


//------------------------------------------------------------------------------
// GmatBase* Pressure::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* Pressure::Clone(void) const
{
   return new Pressure(*this);
}


//==============================================================================
//                              Temperature
//==============================================================================
/**
 * Implements Temperature class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Temperature(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
Temperature::Temperature(const wxString &name, GmatBase *obj)
   : HardwareReal(name, wxT("Temperature"), obj, wxT("Temperature"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::BLUE32;
   #endif
}


//------------------------------------------------------------------------------
// Temperature(const Temperature &copy)
//------------------------------------------------------------------------------
Temperature::Temperature(const Temperature &copy)
   : HardwareReal(copy)
{
}


//------------------------------------------------------------------------------
// Temperature& operator=(const Temperature &right)
//------------------------------------------------------------------------------
Temperature& Temperature::operator=(const Temperature &right)
{
   if (this != &right)
      HardwareReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~Temperature()
//------------------------------------------------------------------------------
Temperature::~Temperature()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool Temperature::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(TEMPERATURE);
    
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets value to the owner of the parameter.
 */
//------------------------------------------------------------------------------
void Temperature::SetReal(Real val)
{
   SpacecraftData::SetReal(TEMPERATURE, val);
   RealVar::SetReal(val);
}


//------------------------------------------------------------------------------
// GmatBase* Temperature::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* Temperature::Clone(void) const
{
   return new Temperature(*this);
}


//==============================================================================
//                              RefTemperature
//==============================================================================
/**
 * Implements RefTemperature class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// RefTemperature(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
RefTemperature::RefTemperature(const wxString &name, GmatBase *obj)
   : HardwareReal(name, wxT("RefTemperature"), obj, wxT("RefTemperature"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::BLUE32;
   #endif
}


//------------------------------------------------------------------------------
// RefTemperature(const RefTemperature &copy)
//------------------------------------------------------------------------------
RefTemperature::RefTemperature(const RefTemperature &copy)
   : HardwareReal(copy)
{
}


//------------------------------------------------------------------------------
// RefTemperature& operator=(const RefTemperature &right)
//------------------------------------------------------------------------------
RefTemperature& RefTemperature::operator=(const RefTemperature &right)
{
   if (this != &right)
      HardwareReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~RefTemperature()
//------------------------------------------------------------------------------
RefTemperature::~RefTemperature()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool RefTemperature::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(REF_TEMPERATURE);
    
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets value to the owner of the parameter.
 */
//------------------------------------------------------------------------------
void RefTemperature::SetReal(Real val)
{
   SpacecraftData::SetReal(REF_TEMPERATURE, val);
   RealVar::SetReal(val);
}


//------------------------------------------------------------------------------
// GmatBase* RefTemperature::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* RefTemperature::Clone(void) const
{
   return new RefTemperature(*this);
}


//==============================================================================
//                              Volume
//==============================================================================
/**
 * Implements Volume class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Volume(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
Volume::Volume(const wxString &name, GmatBase *obj)
   : HardwareReal(name, wxT("Volume"), obj, wxT("Volume"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::GREEN32;
   #endif
}


//------------------------------------------------------------------------------
// Volume(const Volume &copy)
//------------------------------------------------------------------------------
Volume::Volume(const Volume &copy)
   : HardwareReal(copy)
{
}


//------------------------------------------------------------------------------
// Volume& operator=(const Volume &right)
//------------------------------------------------------------------------------
Volume& Volume::operator=(const Volume &right)
{
   if (this != &right)
      HardwareReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~Volume()
//------------------------------------------------------------------------------
Volume::~Volume()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool Volume::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(VOLUME);
    
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets value to the owner of the parameter.
 */
//------------------------------------------------------------------------------
void Volume::SetReal(Real val)
{
   SpacecraftData::SetReal(VOLUME, val);
   RealVar::SetReal(val);
}


//------------------------------------------------------------------------------
// GmatBase* Volume::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* Volume::Clone(void) const
{
   return new Volume(*this);
}


//==============================================================================
//                              FuelDensity
//==============================================================================
/**
 * Implements FuelDensity class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// FuelDensity(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
FuelDensity::FuelDensity(const wxString &name, GmatBase *obj)
   : HardwareReal(name, wxT("FuelDensity"), obj, wxT("FuelDensity"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::ORANGE32;
   #endif
}


//------------------------------------------------------------------------------
// FuelDensity(const FuelDensity &copy)
//------------------------------------------------------------------------------
FuelDensity::FuelDensity(const FuelDensity &copy)
   : HardwareReal(copy)
{
}


//------------------------------------------------------------------------------
// FuelDensity& operator=(const FuelDensity &right)
//------------------------------------------------------------------------------
FuelDensity& FuelDensity::operator=(const FuelDensity &right)
{
   if (this != &right)
      HardwareReal::operator=(right);
   
   return *this;
}


//------------------------------------------------------------------------------
// ~FuelDensity()
//------------------------------------------------------------------------------
FuelDensity::~FuelDensity()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool FuelDensity::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(FUEL_DENSITY);
   
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets value to the owner of the parameter.
 */
//------------------------------------------------------------------------------
void FuelDensity::SetReal(Real val)
{
   SpacecraftData::SetReal(FUEL_DENSITY, val);
   RealVar::SetReal(val);
}


//------------------------------------------------------------------------------
// GmatBase* FuelDensity::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* FuelDensity::Clone(void) const
{
   return new FuelDensity(*this);
}


//==============================================================================
//                              DutyCycle
//==============================================================================
/**
 * Implements DutyCycle class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// DutyCycle(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
DutyCycle::DutyCycle(const wxString &name, GmatBase *obj)
   : HardwareReal(name, wxT("DutyCycle"), obj, wxT("DutyCycle"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::CHESTNUT;
   #endif
}


//------------------------------------------------------------------------------
// DutyCycle(const DutyCycle &copy)
//------------------------------------------------------------------------------
DutyCycle::DutyCycle(const DutyCycle &copy)
   : HardwareReal(copy)
{
}


//------------------------------------------------------------------------------
// DutyCycle& operator=(const DutyCycle &right)
//------------------------------------------------------------------------------
DutyCycle& DutyCycle::operator=(const DutyCycle &right)
{
   if (this != &right)
      HardwareReal::operator=(right);
   
   return *this;
}


//------------------------------------------------------------------------------
// ~DutyCycle()
//------------------------------------------------------------------------------
DutyCycle::~DutyCycle()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool DutyCycle::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(DUTY_CYCLE);
   
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets value to the owner of the parameter.
 */
//------------------------------------------------------------------------------
void DutyCycle::SetReal(Real val)
{
   SpacecraftData::SetReal(DUTY_CYCLE, val);
   RealVar::SetReal(val);
}


//------------------------------------------------------------------------------
// GmatBase* DutyCycle::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* DutyCycle::Clone(void) const
{
   return new DutyCycle(*this);
}


//==============================================================================
//                              ThrustScaleFactor
//==============================================================================
/**
 * Implements ThrustScaleFactor class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// ThrustScaleFactor(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
ThrustScaleFactor::ThrustScaleFactor(const wxString &name, GmatBase *obj)
   : HardwareReal(name, wxT("ThrustScaleFactor"), obj, wxT("ThrustScaleFactor"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::CHESTNUT;
   #endif
}


//------------------------------------------------------------------------------
// ThrustScaleFactor(const ThrustScaleFactor &copy)
//------------------------------------------------------------------------------
ThrustScaleFactor::ThrustScaleFactor(const ThrustScaleFactor &copy)
   : HardwareReal(copy)
{
}


//------------------------------------------------------------------------------
// ThrustScaleFactor& operator=(const ThrustScaleFactor &right)
//------------------------------------------------------------------------------
ThrustScaleFactor& ThrustScaleFactor::operator=(const ThrustScaleFactor &right)
{
   if (this != &right)
      HardwareReal::operator=(right);
   
   return *this;
}


//------------------------------------------------------------------------------
// ~ThrustScaleFactor()
//------------------------------------------------------------------------------
ThrustScaleFactor::~ThrustScaleFactor()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool ThrustScaleFactor::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(THRUSTER_SCALE_FACTOR);
   
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets value to the owner of the parameter.
 */
//------------------------------------------------------------------------------
void ThrustScaleFactor::SetReal(Real val)
{
   SpacecraftData::SetReal(THRUSTER_SCALE_FACTOR, val);
   RealVar::SetReal(val);
}


//------------------------------------------------------------------------------
// GmatBase* ThrustScaleFactor::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* ThrustScaleFactor::Clone(void) const
{
   return new ThrustScaleFactor(*this);
}


//==============================================================================
//                              GravitationalAccel
//==============================================================================
/**
 * Implements GravitationalAccel class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// GravitationalAccel(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
GravitationalAccel::GravitationalAccel(const wxString &name, GmatBase *obj)
   : HardwareReal(name, wxT("GravitationalAccel"), obj, wxT("GravitationalAccel"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::CHESTNUT;
   #endif
}


//------------------------------------------------------------------------------
// GravitationalAccel(const GravitationalAccel &copy)
//------------------------------------------------------------------------------
GravitationalAccel::GravitationalAccel(const GravitationalAccel &copy)
   : HardwareReal(copy)
{
}


//------------------------------------------------------------------------------
// GravitationalAccel& operator=(const GravitationalAccel &right)
//------------------------------------------------------------------------------
GravitationalAccel& GravitationalAccel::operator=(const GravitationalAccel &right)
{
   if (this != &right)
      HardwareReal::operator=(right);
   
   return *this;
}


//------------------------------------------------------------------------------
// ~GravitationalAccel()
//------------------------------------------------------------------------------
GravitationalAccel::~GravitationalAccel()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool GravitationalAccel::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(GRAVITATIONAL_ACCEL);
   
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets value to the owner of the parameter.
 */
//------------------------------------------------------------------------------
void GravitationalAccel::SetReal(Real val)
{
   SpacecraftData::SetReal(GRAVITATIONAL_ACCEL, val);
   RealVar::SetReal(val);
}


//------------------------------------------------------------------------------
// GmatBase* GravitationalAccel::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* GravitationalAccel::Clone(void) const
{
   return new GravitationalAccel(*this);
}


//==============================================================================
//                              ThrustCoefficients
//==============================================================================
/**
 * Implements ThrustCoefficients class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// ThrustCoefficients(const wxString &type, const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
ThrustCoefficients::ThrustCoefficients(const wxString &type,
                                       const wxString &name, GmatBase *obj)
   : HardwareReal(name, wxT("ThrustCoefficients"), obj, wxT("ThrustCoefficients"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::CHESTNUT;
   #endif

   // Add type to ParameterInfo so that we can get information without an instance
   ParameterInfo::Instance()->Add(type, mOwnerType, instanceName, mDepObj,
                                  mIsPlottable, mIsReportable, mIsSettable);
   
   if      (type == wxT("C1") )  mThrustCoeffId = C1;
   else if (type == wxT("C2") )  mThrustCoeffId = C2;
   else if (type == wxT("C3") )  mThrustCoeffId = C3;
   else if (type == wxT("C4") )  mThrustCoeffId = C4;
   else if (type == wxT("C5") )  mThrustCoeffId = C5;
   else if (type == wxT("C6") )  mThrustCoeffId = C6;
   else if (type == wxT("C7") )  mThrustCoeffId = C7;
   else if (type == wxT("C8") )  mThrustCoeffId = C8;
   else if (type == wxT("C9") )  mThrustCoeffId = C9;
   else if (type == wxT("C10"))  mThrustCoeffId = C10;
   else if (type == wxT("C11"))  mThrustCoeffId = C11;
   else if (type == wxT("C12"))  mThrustCoeffId = C12;
   else if (type == wxT("C13"))  mThrustCoeffId = C13;
   else if (type == wxT("C14"))  mThrustCoeffId = C14;
   else if (type == wxT("C15"))  mThrustCoeffId = C15;
   else if (type == wxT("C16"))  mThrustCoeffId = C16;
   else
      mThrustCoeffId = -1;
   
   #ifdef DEBUG_THRUST_COEFF
   MessageInterface::ShowMessage
      (wxT("ThrustCoefficients::ThrustCoefficients() type='%s', name='%s', ")
       wxT("mThrustCoeffId=%d\n"), type.c_str(), name.c_str(), mThrustCoeffId);
   #endif
}


//------------------------------------------------------------------------------
// ThrustCoefficients(const ThrustCoefficients &copy)
//------------------------------------------------------------------------------
ThrustCoefficients::ThrustCoefficients(const ThrustCoefficients &copy)
   : HardwareReal(copy)
{
   mThrustCoeffId = copy.mThrustCoeffId;
}


//------------------------------------------------------------------------------
// ThrustCoefficients& operator=(const ThrustCoefficients &right)
//------------------------------------------------------------------------------
ThrustCoefficients& ThrustCoefficients::operator=(const ThrustCoefficients &right)
{
   if (this != &right)
   {
      HardwareReal::operator=(right);
      mThrustCoeffId = right.mThrustCoeffId;
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// ~ThrustCoefficients()
//------------------------------------------------------------------------------
ThrustCoefficients::~ThrustCoefficients()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool ThrustCoefficients::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(mThrustCoeffId);
   
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets value to the owner of the parameter.
 */
//------------------------------------------------------------------------------
void ThrustCoefficients::SetReal(Real val)
{
   SpacecraftData::SetReal(mThrustCoeffId, val);
   RealVar::SetReal(val);
}


//------------------------------------------------------------------------------
// GmatBase* ThrustCoefficients::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* ThrustCoefficients::Clone(void) const
{
   return new ThrustCoefficients(*this);
}


//==============================================================================
//                              ImpulseCoefficients
//==============================================================================
/**
 * Implements ImpulseCoefficients class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// ImpulseCoefficients(const wxString &type, const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
ImpulseCoefficients::ImpulseCoefficients(const wxString &type,
                                         const wxString &name, GmatBase *obj)
   : HardwareReal(name, wxT("ImpulseCoefficients"), obj, wxT("ImpulseCoefficients"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::CHESTNUT;
   #endif

   // Add type to ParameterInfo so that we can get information without an instance
   ParameterInfo::Instance()->Add(type, mOwnerType, instanceName, mDepObj,
                                  mIsPlottable, mIsReportable, mIsSettable);
   
   if      (type == wxT("K1") )  mImpulseCoeffId = K1;
   else if (type == wxT("K2") )  mImpulseCoeffId = K2;
   else if (type == wxT("K3") )  mImpulseCoeffId = K3;
   else if (type == wxT("K4") )  mImpulseCoeffId = K4;
   else if (type == wxT("K5") )  mImpulseCoeffId = K5;
   else if (type == wxT("K6") )  mImpulseCoeffId = K6;
   else if (type == wxT("K7") )  mImpulseCoeffId = K7;
   else if (type == wxT("K8") )  mImpulseCoeffId = K8;
   else if (type == wxT("K9") )  mImpulseCoeffId = K9;
   else if (type == wxT("K10"))  mImpulseCoeffId = K10;
   else if (type == wxT("K11"))  mImpulseCoeffId = K11;
   else if (type == wxT("K12"))  mImpulseCoeffId = K12;
   else if (type == wxT("K13"))  mImpulseCoeffId = K13;
   else if (type == wxT("K14"))  mImpulseCoeffId = K14;
   else if (type == wxT("K15"))  mImpulseCoeffId = K15;
   else if (type == wxT("K16"))  mImpulseCoeffId = K16;
   else
      mImpulseCoeffId = -1;
   
   #ifdef DEBUG_IMPULSE_COEFF
   MessageInterface::ShowMessage
      (wxT("ImpulseCoefficients::ImpulseCoefficients() type='%s', name='%s', ")
       wxT("mImpulseCoeffId=%d\n"), type.c_str(), name.c_str(), mImpulseCoeffId);
   #endif
}


//------------------------------------------------------------------------------
// ImpulseCoefficients(const ImpulseCoefficients &copy)
//------------------------------------------------------------------------------
ImpulseCoefficients::ImpulseCoefficients(const ImpulseCoefficients &copy)
   : HardwareReal(copy)
{
   mImpulseCoeffId = copy.mImpulseCoeffId;
}


//------------------------------------------------------------------------------
// ImpulseCoefficients& operator=(const ImpulseCoefficients &right)
//------------------------------------------------------------------------------
ImpulseCoefficients& ImpulseCoefficients::operator=(const ImpulseCoefficients &right)
{
   if (this != &right)
   {
      HardwareReal::operator=(right);
      mImpulseCoeffId = right.mImpulseCoeffId;
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// ~ImpulseCoefficients()
//------------------------------------------------------------------------------
ImpulseCoefficients::~ImpulseCoefficients()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool ImpulseCoefficients::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(mImpulseCoeffId);
   
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets value to the owner of the parameter.
 */
//------------------------------------------------------------------------------
void ImpulseCoefficients::SetReal(Real val)
{
   SpacecraftData::SetReal(mImpulseCoeffId, val);
   RealVar::SetReal(val);
}


//------------------------------------------------------------------------------
// GmatBase* ImpulseCoefficients::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* ImpulseCoefficients::Clone(void) const
{
   return new ImpulseCoefficients(*this);
}


//==============================================================================
//                              ThrustDirections
//==============================================================================
/**
 * Implements ThrustDirections class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// ThrustDirections(const wxString &type, const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
ThrustDirections::ThrustDirections(const wxString &type,
                                   const wxString &name, GmatBase *obj)
   : HardwareReal(name, wxT("ThrustDirection"), obj, wxT("ThrustDirection"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::CHESTNUT;
   #endif

   // Add type to ParameterInfo so that we can get information without an instance
   ParameterInfo::Instance()->Add(type, mOwnerType, instanceName, mDepObj,
                                  mIsPlottable, mIsReportable, mIsSettable);
   
   if (type == wxT("ThrustDirection1") )
      mThrustDirectionId = THRUST_DIRECTION1;
   else if (type == wxT("ThrustDirection2") )
      mThrustDirectionId = THRUST_DIRECTION2;
   else if (type == wxT("ThrustDirection3") )
      mThrustDirectionId = THRUST_DIRECTION3;
   else
      mThrustDirectionId = -1;
   
   #ifdef DEBUG_IMPULSE_COEFF
   MessageInterface::ShowMessage
      (wxT("ThrustDirections::ThrustDirections() type='%s', name='%s', ")
       wxT("mThrustDirectionId=%d\n"), type.c_str(), name.c_str(), mThrustDirectionId);
   #endif
}


//------------------------------------------------------------------------------
// ThrustDirections(const ThrustDirections &copy)
//------------------------------------------------------------------------------
ThrustDirections::ThrustDirections(const ThrustDirections &copy)
   : HardwareReal(copy)
{
   mThrustDirectionId = copy.mThrustDirectionId;
}


//------------------------------------------------------------------------------
// ThrustDirections& operator=(const ThrustDirections &right)
//------------------------------------------------------------------------------
ThrustDirections& ThrustDirections::operator=(const ThrustDirections &right)
{
   if (this != &right)
   {
      HardwareReal::operator=(right);
      mThrustDirectionId = right.mThrustDirectionId;
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// ~ThrustDirections()
//------------------------------------------------------------------------------
ThrustDirections::~ThrustDirections()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool ThrustDirections::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(mThrustDirectionId);
   
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets value to the owner of the parameter.
 */
//------------------------------------------------------------------------------
void ThrustDirections::SetReal(Real val)
{
   SpacecraftData::SetReal(mThrustDirectionId, val);
   RealVar::SetReal(val);
}


//------------------------------------------------------------------------------
// GmatBase* ThrustDirections::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* ThrustDirections::Clone(void) const
{
   return new ThrustDirections(*this);
}


