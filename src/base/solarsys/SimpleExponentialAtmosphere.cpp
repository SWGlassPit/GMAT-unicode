//$Id: SimpleExponentialAtmosphere.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                        SimpleExponentialAtmosphere
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under FDSS contract
// task 28
//
// Author: Darrel J. Conway
// Created: 2011/02/01
//
/**
 * A simple exponentially modeled atmosphere based on input parameters in the
 * STK GUI.
 */
//------------------------------------------------------------------------------


#include "SimpleExponentialAtmosphere.hpp"
#include <cmath>
#include "MessageInterface.hpp"

//#define DEBUG_DENSITY

//------------------------------------------------------------------------------
// SimpleExponentialAtmosphere(const wxString &name = wxT(""))
//------------------------------------------------------------------------------
/**
 * Default constructor.
 */
//------------------------------------------------------------------------------
SimpleExponentialAtmosphere::SimpleExponentialAtmosphere(const wxString &name) :
   AtmosphereModel      (wxT("SimpleExponential"), name),
   scaleHeight          (8.5),
   refHeight            (0.0),
   refDensity           (1.217),
   geocentricAltitude   (false)
{
}


//------------------------------------------------------------------------------
// ~SimpleExponentialAtmosphere()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
SimpleExponentialAtmosphere::~SimpleExponentialAtmosphere()
{
}


//------------------------------------------------------------------------------
// SimpleExponentialAtmosphere(const SimpleExponentialAtmosphere& atm)
//------------------------------------------------------------------------------
/**
 * Copy constructor. (private implementation)
 *
 * @param atm SimpleExponentialAtmosphere object to copy into the new one.
 */
//------------------------------------------------------------------------------
SimpleExponentialAtmosphere::SimpleExponentialAtmosphere(
      const SimpleExponentialAtmosphere& atm) :
   AtmosphereModel      (atm),
   scaleHeight          (atm.scaleHeight),
   refHeight            (atm.refHeight),
   refDensity           (atm.refDensity),
   geocentricAltitude   (atm.geocentricAltitude)
{
}

//------------------------------------------------------------------------------
// SimpleExponentialAtmosphere& operator=(
//       const SimpleExponentialAtmosphere& bary)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the SimpleExponentialAtmosphere class.
 *
 * @param bary the SimpleExponentialAtmosphere object whose data to assign to
 *             wxT("this") calculated point.
 *
 * @return wxT("this") SimpleExponentialAtmosphere with data of input
 *                SimpleExponentialAtmosphere ea.
 */
//------------------------------------------------------------------------------
SimpleExponentialAtmosphere& SimpleExponentialAtmosphere::operator=(
      const SimpleExponentialAtmosphere &atm)
{
   if (&atm != this)
   {
      AtmosphereModel::operator=(atm);

      scaleHeight        = atm.scaleHeight;
      refHeight          = atm.refHeight;
      refDensity         = atm.refDensity;
      geocentricAltitude = atm.geocentricAltitude;
   }
   return *this;
}


//------------------------------------------------------------------------------
// bool Density(Real *position, Real *density, Real epoch, Integer count)
//------------------------------------------------------------------------------
/**
 * Calculates the density at each of the states in the input vector using
 * Vallado's method to interpolate the densities.
 * 
 * @param pos      The input vector of spacecraft states
 * @param density  The array of output densities
 * @param epoch    The current TAIJulian epoch (unused here)
 * @param count    The number of spacecraft contained in pos
 *
 * @return true on success, throws on failure.
 */
//------------------------------------------------------------------------------
bool SimpleExponentialAtmosphere::Density(Real *position, Real *density,
      Real epoch, Integer count)
{
   #ifdef DEBUG_DENSITY
      MessageInterface::ShowMessage(wxT("SimpleExponentialAtmosphere::Density called\n"));
   #endif

   if (centralBodyLocation == NULL)
      throw AtmosphereException(wxT("Exponential atmosphere: Central body vector ")
            wxT("was not initialized"));
        
   Real loc[3], height;
   Integer i;
    
   for (i = 0; i < count; ++i)
   {
      loc[0] = position[ i*6 ] - centralBodyLocation[0];
      loc[1] = position[i*6+1] - centralBodyLocation[1];
      loc[2] = position[i*6+2] - centralBodyLocation[2];
        
      height = CalculateGeodetics(loc, epoch);
      if (height < 0.0)
         throw AtmosphereException(wxT("Exponential atmosphere: Position vector is ")
               wxT("inside central body"));

      density[i] = refDensity * exp(-(height - refHeight) / scaleHeight);
      #ifdef DEBUG_DENSITY
         MessageInterface::ShowMessage(wxT("SEAtmos: [%lf %lf %lf] -> ht: %lf -> ")
               wxT("density: %.12le\n"), loc[0], loc[1], loc[2], height, density[i]);
      #endif
   }
    
   return true;
}


//------------------------------------------------------------------------------
// GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Clone the object (inherited from GmatBase).
 *
 * @return a clone of wxT("this") object.
 */
//------------------------------------------------------------------------------
GmatBase* SimpleExponentialAtmosphere::Clone() const
{
   return (new SimpleExponentialAtmosphere(*this));
}
