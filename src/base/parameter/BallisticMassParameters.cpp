//$Id: BallisticMassParameters.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                            File: BallisticMassParameters
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
 * Implements BallisticMass related parameter classes.
 *    DryMass, DragCoeff, ReflectCoeff, DragArea, SRPArea, TotalMass
 */
//------------------------------------------------------------------------------

#include "BallisticMassParameters.hpp"
#include "ColorTypes.hpp"


// To use preset colors, uncomment this line:
//#define USE_PREDEFINED_COLORS

//==============================================================================
//                              DryMass
//==============================================================================
/**
 * Implements DryMass class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// DryMass(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
DryMass::DryMass(const wxString &name, GmatBase *obj)
   : BallisticMassReal(name, wxT("DryMass"), obj, wxT("Dry Mass"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::RED32;
   #endif
}


//------------------------------------------------------------------------------
// DryMass(const DryMass &copy)
//------------------------------------------------------------------------------
DryMass::DryMass(const DryMass &copy)
   : BallisticMassReal(copy)
{
}


//------------------------------------------------------------------------------
// DryMass& operator=(const DryMass &right)
//------------------------------------------------------------------------------
DryMass& DryMass::operator=(const DryMass &right)
{
   if (this != &right)
      BallisticMassReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~DryMass()
//------------------------------------------------------------------------------
DryMass::~DryMass()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool DryMass::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(DRY_MASS);
   
    
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// GmatBase* DryMass::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* DryMass::Clone(void) const
{
   return new DryMass(*this);
}


//==============================================================================
//                              DragCoeff
//==============================================================================
/**
 * Implements DragCoeff class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// DragCoeff(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
DragCoeff::DragCoeff(const wxString &name, GmatBase *obj)
   : BallisticMassReal(name, wxT("Cd"), obj, wxT("Drag Coefficient"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::YELLOW32;
   #endif
}


//------------------------------------------------------------------------------
// DragCoeff(const DragCoeff &copy)
//------------------------------------------------------------------------------
DragCoeff::DragCoeff(const DragCoeff &copy)
   : BallisticMassReal(copy)
{
}


//------------------------------------------------------------------------------
// DragCoeff& operator=(const DragCoeff &right)
//------------------------------------------------------------------------------
DragCoeff& DragCoeff::operator=(const DragCoeff &right)
{
   if (this != &right)
      BallisticMassReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~DragCoeff()
//------------------------------------------------------------------------------
DragCoeff::~DragCoeff()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool DragCoeff::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(DRAG_COEFF);
    
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// GmatBase* DragCoeff::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* DragCoeff::Clone(void) const
{
   return new DragCoeff(*this);
}


//==============================================================================
//                              ReflectCoeff
//==============================================================================
/**
 * Implements ReflectCoeff class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// ReflectCoeff(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
ReflectCoeff::ReflectCoeff(const wxString &name, GmatBase *obj)
   : BallisticMassReal(name, wxT("Cr"), obj, wxT("Reflectivity Coefficient"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::BLUE32;
   #endif
}


//------------------------------------------------------------------------------
// ReflectCoeff(const ReflectCoeff &copy)
//------------------------------------------------------------------------------
ReflectCoeff::ReflectCoeff(const ReflectCoeff &copy)
   : BallisticMassReal(copy)
{
}


//------------------------------------------------------------------------------
// ReflectCoeff& operator=(const ReflectCoeff &right)
//------------------------------------------------------------------------------
ReflectCoeff& ReflectCoeff::operator=(const ReflectCoeff &right)
{
   if (this != &right)
      BallisticMassReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~ReflectCoeff()
//------------------------------------------------------------------------------
ReflectCoeff::~ReflectCoeff()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool ReflectCoeff::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(REFLECT_COEFF);
    
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// GmatBase* ReflectCoeff::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* ReflectCoeff::Clone(void) const
{
   return new ReflectCoeff(*this);
}


//==============================================================================
//                              DragArea
//==============================================================================
/**
 * Implements DragArea class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// DragArea(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
DragArea::DragArea(const wxString &name, GmatBase *obj)
   : BallisticMassReal(name, wxT("DragArea"), obj, wxT("Drag Area"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::GREEN32;
   #endif
}


//------------------------------------------------------------------------------
// DragArea(const DragArea &copy)
//------------------------------------------------------------------------------
DragArea::DragArea(const DragArea &copy)
   : BallisticMassReal(copy)
{
}


//------------------------------------------------------------------------------
// DragArea& operator=(const DragArea &right)
//------------------------------------------------------------------------------
DragArea& DragArea::operator=(const DragArea &right)
{
   if (this != &right)
      BallisticMassReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~DragArea()
//------------------------------------------------------------------------------
DragArea::~DragArea()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool DragArea::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(DRAG_AREA);
    
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// GmatBase* DragArea::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* DragArea::Clone(void) const
{
   return new DragArea(*this);
}


//==============================================================================
//                              SRPArea
//==============================================================================
/**
 * Implements SRPArea class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SRPArea(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
SRPArea::SRPArea(const wxString &name, GmatBase *obj)
   : BallisticMassReal(name, wxT("SRPArea"), obj, wxT("SRP Area"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::ORANGE32;
   #endif
}


//------------------------------------------------------------------------------
// SRPArea(const SRPArea &copy)
//------------------------------------------------------------------------------
SRPArea::SRPArea(const SRPArea &copy)
   : BallisticMassReal(copy)
{
}


//------------------------------------------------------------------------------
// SRPArea& operator=(const SRPArea &right)
//------------------------------------------------------------------------------
SRPArea& SRPArea::operator=(const SRPArea &right)
{
   if (this != &right)
      BallisticMassReal::operator=(right);
   
   return *this;
}


//------------------------------------------------------------------------------
// ~SRPArea()
//------------------------------------------------------------------------------
SRPArea::~SRPArea()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool SRPArea::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(SRP_AREA);
   
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// GmatBase* SRPArea::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* SRPArea::Clone(void) const
{
   return new SRPArea(*this);
}


//==============================================================================
//                              TotalMass
//==============================================================================
/**
 * Implements TotalMass class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// TotalMass(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
TotalMass::TotalMass(const wxString &name, GmatBase *obj)
   : BallisticMassReal(name, wxT("TotalMass"), obj, wxT("Total Mass"), wxT(""))
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::CHESTNUT;
   #endif
}


//------------------------------------------------------------------------------
// TotalMass(const TotalMass &copy)
//------------------------------------------------------------------------------
TotalMass::TotalMass(const TotalMass &copy)
   : BallisticMassReal(copy)
{
}


//------------------------------------------------------------------------------
// TotalMass& operator=(const TotalMass &right)
//------------------------------------------------------------------------------
TotalMass& TotalMass::operator=(const TotalMass &right)
{
   if (this != &right)
      BallisticMassReal::operator=(right);
   
   return *this;
}


//------------------------------------------------------------------------------
// ~TotalMass()
//------------------------------------------------------------------------------
TotalMass::~TotalMass()
{
}


//------------------------------------------------------------------------------
// bool Evaluate()
//------------------------------------------------------------------------------
bool TotalMass::Evaluate()
{
   mRealValue = SpacecraftData::GetReal(TOTAL_MASS);
   
   if (mRealValue == GmatBase::REAL_PARAMETER_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// GmatBase* TotalMass::Clone(void) const
//------------------------------------------------------------------------------
GmatBase* TotalMass::Clone(void) const
{
   return new TotalMass(*this);
}

