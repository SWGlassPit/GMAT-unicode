//$Id: $
//------------------------------------------------------------------------------
//                              BodyFixedStateConverter
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
// Author:  Wendy Shoan/NASA/GSFC/583
// Created: 2010.08.25
//
/**
 * Implements the BodyFixedStateConverter namespace
 */
//------------------------------------------------------------------------------
#include "BodyFixedStateConverter.hpp"
#include "StringUtil.hpp"
#include "GmatConstants.hpp"
#include "MessageInterface.hpp"
#include "RealUtilities.hpp"


//#define DEBUG_BF_STATE_CONVERTER

//------------------------------------------------------------------------------
//  Rvector3 BodyFixedStateConverterUtil::Convert(const Rvector3    &origValue,
//                                                const wxString &fromType,
//                                                const wxString &toType,
//                                                const Real        flattening,
//                                                const Real        meanRadius)
//------------------------------------------------------------------------------
/**
 * Conversion method.
 *
 * @param <origValue>  data in given representation
 * @param <fromType>   representation from which to convert
 * @param <toType>     representation to which to convert
 * @param <flattening> flattening coefficient for the body
 * @param <meanRadius> mean radius of the body
 *
 * @return Converted state from the specified wxT("to") representation to the
 *         specified wxT("from") representation
 */
//---------------------------------------------------------------------------
Rvector3 BodyFixedStateConverterUtil::Convert(const Rvector3    &origValue,
                                              const wxString &fromType,
                                              const wxString &toType,
                                              const Real        flattening,
                                              const Real        meanRadius)
{
   Rvector3 outState;
   bool     unknownRep = false;
   #ifdef DEBUG_BF_STATE_CONVERTER
      MessageInterface::ShowMessage(
         wxT("      BodyFixedStateConverterUtil::Converting %.18lf %.18lf %.18lf in %s to %s\n"),
         origValue[0], origValue[1], origValue[2],
         fromType.c_str(), toType.c_str());
   #endif

   // Call the appropriate method, depending on the from and to types
   if (fromType == wxT("Cartesian"))
   {
      if (toType == wxT("Spherical"))
         outState = CartesianToSpherical(origValue, flattening, meanRadius);
      else if (toType == wxT("Ellipsoid"))
         outState = CartesianToSphericalEllipsoid(origValue, flattening, meanRadius);
      else
         unknownRep = true;
   }
   else if (fromType == wxT("Spherical"))
   {
      if (toType == wxT("Cartesian"))
         outState = SphericalToCartesian(origValue, flattening, meanRadius);
      else if (toType == wxT("Ellipsoid"))
         outState = SphericalToSphericalEllipsoid(origValue, flattening, meanRadius);
      else
         unknownRep = true;
   }
   else if (fromType == wxT("Ellipsoid"))
   {
      if (toType == wxT("Cartesian"))
         outState = SphericalEllipsoidToCartesian(origValue, flattening, meanRadius);
      else if (toType == wxT("Spherical"))
         outState = SphericalEllipsoidToSpherical(origValue, flattening, meanRadius);
      else
         unknownRep = true;
   }
   else
      unknownRep = true;

   // If one or both of the types are invalid, throw an exception
   if (unknownRep)
   {
      wxString errmsg = wxT("representation requested is from ");
      errmsg += fromType + wxT(" to ");
      errmsg += toType + wxT(".\n");
      throw InvalidStateRepresentationException(errmsg);
   }

   #ifdef DEBUG_BF_STATE_CONVERTER
      MessageInterface::ShowMessage(wxT("      state in %s  =  %.18lf %.18lf %.18lf\n"),
         toType.c_str(), outState[0], outState[1], outState[2]);
   #endif

   return outState;
}

//------------------------------------------------------------------------------
//  Rvector3 BodyFixedStateConverterUtil::Convert(const Rvector3    &origValue,
//                                                const wxString &fromType,
//                                                const wxString &fromHorizon,
//                                                const wxString &toType,
//                                                const wxString &toHorizon,
//                                                const Real        flattening,
//                                                const Real        meanRadius)
//------------------------------------------------------------------------------
/**
 * Conversion method.
 *
 * @param <origValue>   data in given representation
 * @param <fromType>    state type from which to convert
 * @param <fromHorizon> horizon reference value from which to convert
 * @param <toType>      state type to which to convert
 * @param <toHorizon>   horizon reference to which to convert
 * @param <flattening>  flattening coefficient for the body
 * @param <meanRadius>  mean radius of the body
 *
 * @return Converted state from the specified wxT("to") representation to the
 *         specified wxT("from") representation
 */
//---------------------------------------------------------------------------
Rvector3 BodyFixedStateConverterUtil::Convert(const Rvector3    &origValue,
                                              const wxString &fromType,  const wxString &fromHorizon,
                                              const wxString &toType,    const wxString &toHorizon,
                                              const Real        flattening, const Real        meanRadius)
{
   Rvector3 outState;
   #ifdef DEBUG_BF_STATE_CONVERTER
      MessageInterface::ShowMessage(
         wxT("      BodyFixedStateConverterUtil::Converting %.18lf %.18lf %.18lf in %s (%s) to %s (%s)\n"),
         origValue[0], origValue[1], origValue[2],
         fromType.c_str(), fromHorizon.c_str(), toType.c_str(), toHorizon.c_str());
   #endif

   // If one or both of the types are invalid, throw an exception
   if (((fromType    != wxT("Cartesian")) && (fromType    != wxT("Spherical"))) ||
       ((toType      != wxT("Cartesian")) && (toType      != wxT("Spherical"))) ||
       ((fromHorizon != wxT("Sphere"))    && (fromHorizon != wxT("Ellipsoid"))) ||
       ((toHorizon   != wxT("Sphere"))    && (toHorizon   != wxT("Ellipsoid"))) )
   {
      wxString errmsg = wxT("representation requested is from ");
      errmsg += fromType + wxT("(");
      errmsg += fromHorizon + wxT(") to ");
      errmsg += toType + wxT("(");
      errmsg += toHorizon + wxT(").\n");
      throw InvalidStateRepresentationException(errmsg);
   }

   // Call the appropriate method, depending on the from and to types
   if (fromType == wxT("Cartesian"))
   {
      if (toType == wxT("Cartesian"))
         outState = origValue;
      else  // toType == Spherical
      {
         if (toHorizon == wxT("Sphere"))
            outState = CartesianToSpherical(origValue, flattening, meanRadius);
         else // toHorizon == Ellipsoid
            outState = CartesianToSphericalEllipsoid(origValue, flattening, meanRadius);
     }
   }
   else // fromType == Spherical
   {
      if (toType == wxT("Spherical"))
      {
         if (fromHorizon == wxT("Sphere"))
         {
            if (toHorizon == wxT("Sphere"))
               outState = origValue;
            else // toHorizon == Ellipsoid
               outState = SphericalToSphericalEllipsoid(origValue, flattening, meanRadius);
         }
         else  // fromHorizon == Ellipsoid
         {
            if (toHorizon == wxT("Ellipsoid"))
               outState = origValue;
            else // toHorizon == Sphere
               outState = SphericalEllipsoidToSpherical(origValue, flattening, meanRadius);
         }
      }
      else if (toType == wxT("Cartesian"))
      {
         if (fromHorizon == wxT("Sphere"))
            outState = SphericalToCartesian(origValue, flattening, meanRadius);
         else // fromHorizon == Ellipsoid
            outState = SphericalEllipsoidToCartesian(origValue, flattening, meanRadius);
      }
   }

   #ifdef DEBUG_BF_STATE_CONVERTER
      MessageInterface::ShowMessage(wxT("      state in %s (%s)  =  %.18lf %.18lf %.18lf\n"),
         toType.c_str(), toHorizon.c_str(), outState[0], outState[1], outState[2]);
   #endif

   return outState;
}


//------------------------------------------------------------------------------
// Rvector3 CartesianToSpherical(const Rvector3 &cart, const Real flattening,
//                               const Real meanRadius);
//------------------------------------------------------------------------------
/**
 * Method to convert from Cartesian to Spherical.
 *
 * @param <cart>        data in cartesian
 * @param <flattening>  flattening coefficient for the body
 * @param <meanRadius>  mean radius of the body
 *
 * @return Converted state from the cartesian representation to the
 *         spherical representation
 */
//---------------------------------------------------------------------------
Rvector3 BodyFixedStateConverterUtil::CartesianToSpherical(const Rvector3 &cart,
                                                           const Real     flattening,
                                                           const Real     meanRadius)
{
   // Calculate the longitude
   Real longitude = GmatMathUtil::ATan2(cart[1], cart[0]);
   // constrain it to be 0 - 360 degrees
   while (longitude < 0.0)  longitude += GmatMathConstants::TWO_PI;

   // Calculate the latitude
   Real rMag      = cart.GetMagnitude();
   Real latitude  = GmatMathUtil::ASin(cart[2] / rMag);

   // Calculate the height
   Real height    = rMag - meanRadius;
   return Rvector3(latitude, longitude, height);
}

//------------------------------------------------------------------------------
// Rvector3 SphericalToCartesian(const Rvector3 &spherical, const Real flattening,
//                               const Real meanRadius)
//------------------------------------------------------------------------------
/**
 * Method to convert from Spherical to Cartesian.
 *
 * @param <spherical>   data in spherical
 * @param <flattening>  flattening coefficient for the body
 * @param <meanRadius>  mean radius of the body
 *
 * @return Converted state from the spherical representation to the
 *         cartesian representation
 */
//---------------------------------------------------------------------------
Rvector3 BodyFixedStateConverterUtil::SphericalToCartesian(const Rvector3 &spherical,
                                                           const Real     flattening,
                                                           const Real     meanRadius)
{
   Real latitude   = spherical[0];
   Real longitude  = spherical[1];
   Real height     = spherical[2];

   // Calculate the cartesian position
   Rvector3 tmp((GmatMathUtil::Cos(latitude) * GmatMathUtil::Cos(longitude)),
                (GmatMathUtil::Cos(latitude) * GmatMathUtil::Sin(longitude)),
                 GmatMathUtil::Sin(latitude));

   Rvector3 r = (height + meanRadius) * tmp;
   return r;
}

//------------------------------------------------------------------------------
// Rvector3 SphericalEllipsoidToCartesian(const Rvector3 &sphEll, const Real flattening,
//                                        const Real meanRadius);
//------------------------------------------------------------------------------
/**
 * Method to convert from Spherical-Ellipsoid to Cartesian.
 *
 * @param <sphEll>      data in spherical-ellipsoid
 * @param <flattening>  flattening coefficient for the body
 * @param <meanRadius>  mean radius of the body
 *
 * @return Converted state from the spherical-ellipsoid representation to the
 *         cartesian representation
 */
//---------------------------------------------------------------------------
Rvector3 BodyFixedStateConverterUtil::SphericalEllipsoidToCartesian(const Rvector3 &sphEll,
                                                                    const Real     flattening,
                                                                    const Real     meanRadius)
{
   Real latitude    = sphEll[0]; // input is in radians
   Real longitude   = sphEll[1]; // input is in radians
   Real height      = sphEll[2];
   Real sinLatitude = GmatMathUtil::Sin(latitude);

   // Calculate intermediate variables
   Real ee        = 2.0 * flattening - (flattening * flattening);
   Real C         = meanRadius /
                    GmatMathUtil::Sqrt(1.0 - (ee * sinLatitude * sinLatitude));
   Real S         = C * (1.0 - ee);

   // Calculate the square of the x and y components, and the z component
   Real rxy       = (C + height) * GmatMathUtil::Cos(latitude);
   Real rz        = (S + height) * sinLatitude;

   // Calculate and return the cartesian position
   return Rvector3((rxy * GmatMathUtil::Cos(longitude)),
                   (rxy * GmatMathUtil::Sin(longitude)),
                    rz);
}

//------------------------------------------------------------------------------
// Rvector3 CartesianToSphericalEllipsoid(const Rvector3 &cart, const Real flattening,
//                                        const Real meanRadius);
//------------------------------------------------------------------------------
/**
 * Method to convert from Cartesian to Spherical-Ellipsoid.
 *
 * @param <cart>        data in cartesian
 * @param <flattening>  flattening coefficient for the body
 * @param <meanRadius>  mean radius of the body
 *
 * @return Converted state from the cartesian representation to the
 *         spherical-ellipsoid representation
 */
//---------------------------------------------------------------------------
Rvector3 BodyFixedStateConverterUtil::CartesianToSphericalEllipsoid(const Rvector3 &cart,
                                                                    const Real     flattening,
                                                                    const Real     meanRadius)
{
   #ifdef DEBUG_BF_STATE_CONVERTER
      MessageInterface::ShowMessage(wxT("Entering BFSC::Cart2SphEll, cart = %12.10f  %12.10f  %12.10f\n"),
            cart[0], cart[1], cart[2]);
      MessageInterface::ShowMessage(wxT("                             flattening = %12.10f,  radius = %12.10f\n"),
            flattening, meanRadius);
   #endif
   // Calculate the longitude
   Real longitude = GmatMathUtil::ATan2(cart[1], cart[0]);
   // constrain it to be 0 - 360 degrees
   while (longitude < 0.0)  longitude += GmatMathConstants::TWO_PI;

   // Calculate intermediate variables
//   Real rMag      = cart.GetMagnitude();
   Real rxy         = GmatMathUtil::Sqrt((cart[0] * cart[0]) + (cart[1] * cart[1]));
   Real ee          = 2.0 * flattening - (flattening * flattening);
   Real delta       = 1.0;
   Real lat2        = 0.0;
   Real C           = 0.0;
   Real sinLatitude = 0.0;

   // Calculate the spherical latitude to use as an initial guess for latitude
   Real latitude  = GmatMathUtil::ATan2(cart[2], rxy);

   // Iterate until the difference is less than the tolerance
   while (delta > 1.0e-13)
   {
      lat2        = latitude;
      sinLatitude = GmatMathUtil::Sin(latitude);
      C           = meanRadius /
                    GmatMathUtil::Sqrt(1.0 - (ee * sinLatitude * sinLatitude));
      latitude    = atan((cart[2] + (C * ee * GmatMathUtil::Sin(latitude))) /  rxy);
      delta       = GmatMathUtil::Abs(latitude - lat2);
   }
   #ifdef DEBUG_BF_STATE_CONVERTER
      MessageInterface::ShowMessage(wxT("   after lat/long computation, latitude = %12.10f (%12.10f), longitude = %12.10f (%12.10f)\n"),
            latitude, latitude * GmatMathUtil::DEG_PER_RAD, longitude, longitude * GmatMathUtil::DEG_PER_RAD);
   #endif

   // Calculate the height above the reference spherical ellipsoid
    C           = meanRadius / GmatMathUtil::Sqrt(1.0 - (ee *
                  GmatMathUtil::Sin(latitude) * GmatMathUtil::Sin(latitude)));
    Real S      = C * (1.0 - ee);
    Real height = 0.0;
    // if near a pole ...
    if ((GmatMathConstants::PI_OVER_TWO - GmatMathUtil::Abs(latitude)) > .02)
       height = (rxy / GmatMathUtil::Cos(latitude)) - C;
    else // not near a pole
       height = (cart[2] / GmatMathUtil::Sin(latitude)) - S;

   #ifdef DEBUG_BF_STATE_CONVERTER
      MessageInterface::ShowMessage(wxT("   Exiting BFSC::Cart2SphEll, latitude = %12.10f, longitude = %12.10f,  height = %12.10f\n"),
            latitude, longitude, height);
   #endif
   return Rvector3(latitude, longitude, height);
}

//------------------------------------------------------------------------------
// Rvector3 SphericalToSphericalEllipsoid(const Rvector3 &spherical, const Real flattening,
//                                        const Real meanRadius);
//------------------------------------------------------------------------------
/**
 * Method to convert from Spherical to Spherical-Ellipsoid.
 *
 * @param <spherical>   data in spherical
 * @param <flattening>  flattening coefficient for the body
 * @param <meanRadius>  mean radius of the body
 *
 * @return Converted state from the spherical representation to the
 *         spherical-ellipsoid representation
 */
//---------------------------------------------------------------------------
Rvector3 BodyFixedStateConverterUtil::SphericalToSphericalEllipsoid(const Rvector3 &spherical,
                                                                    const Real     flattening,
                                                                    const Real     meanRadius)
{
   #ifdef DEBUG_BF_STATE_CONVERTER
      MessageInterface::ShowMessage(wxT("Entering BFSC::SphericalToSphericalEllipsoid: input = %12.10 f  %12.10f  %12.10f\n"),
                                   spherical[0], spherical[1], spherical[2]);
   #endif
   Rvector3 sph2cart = SphericalToCartesian(spherical, flattening, meanRadius);
   #ifdef DEBUG_BF_STATE_CONVERTER
      MessageInterface::ShowMessage(wxT("   intermediate sph2cart = %12.10 f  %12.10f  %12.10f\n"),
            sph2cart[0], sph2cart[1], sph2cart[2]);
   #endif
   return CartesianToSphericalEllipsoid(sph2cart, flattening, meanRadius);
}

//------------------------------------------------------------------------------
// Rvector3 SphericalEllipsoidToSpherical(const Rvector3 &sphEll, const Real flattening,
//                                        const Real meanRadius);
//------------------------------------------------------------------------------
/**
 * Method to convert from Spherical-Ellipsoid to Spherical.
 *
 * @param <sphEll>      data in spherical-ellipsoid
 * @param <flattening>  flattening coefficient for the body
 * @param <meanRadius>  mean radius of the body
 *
 * @return Converted state from the spherical-ellipsoid representation to the
 *         spherical representation
 */
//---------------------------------------------------------------------------
Rvector3 BodyFixedStateConverterUtil::SphericalEllipsoidToSpherical(const Rvector3 &sphEll,
                                                                    const Real     flattening,
                                                                    const Real     meanRadius)
{
   Rvector3 sphel2cart = SphericalEllipsoidToCartesian(sphEll, flattening, meanRadius);
   return CartesianToSpherical(sphel2cart, flattening, meanRadius);
}


//---------------------------------------------------------------------------
// bool BodyFixedStateConverterUtil::IsValidStateRepresentation(const wxString &rep)
//---------------------------------------------------------------------------
bool BodyFixedStateConverterUtil::IsValidStateRepresentation(const wxString &rep)
{
   for (Integer ii = 0; ii < NUM_STATE_REPRESENTATIONS; ii++)
      if (rep == BODY_FIXED_STATE_REPRESENTATION_TEXT[ii]) return true;
   return false;
}


//---------------------------------------------------------------------------
// StringArray BodyFixedStateConverterUtil::GetValidRepresentations()
//---------------------------------------------------------------------------
StringArray BodyFixedStateConverterUtil::GetValidRepresentations()
{
   StringArray reps;
   for (Integer ii = 0; ii < NUM_STATE_REPRESENTATIONS; ii++)
      reps.push_back(BODY_FIXED_STATE_REPRESENTATION_TEXT[ii]);
   return reps;
}
