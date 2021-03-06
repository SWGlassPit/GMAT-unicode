//$Id:  $
//------------------------------------------------------------------------------
//                              CalculationUtilities
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under NASA Prime
// Contract NNG10CP02C, Task Order 28
//
// Created: 2011.08.12
//
/**
 * This namespace provides Calculation Utilities that can be used to compute the
 * GMAT Calculation Objects.
 */
//------------------------------------------------------------------------------
#include <math.h>
#include "CalculationUtilities.hpp"
#include "GmatConstants.hpp"
#include "Linear.hpp"
#include "Keplerian.hpp"
#include "Rvector3.hpp"
#include "RealUtilities.hpp"
#include "GmatConstants.hpp"
#include "AngleUtil.hpp"
#include "UtilityException.hpp"
#include "MessageInterface.hpp"

//#define __COMPUTE_LONGITUDE_OLDWAY__

//#define DEBUG_CALC_UTIL

#ifdef DEBUG_CALC_UTIL
#include "MessageInterface.hpp"
#endif

using namespace GmatMathUtil;
using namespace GmatMathConstants;


//------------------------------------------------------------------------------
//  Real  CalculateBPlaneData(const wxString &item, const Rvector6 &state,
//                            const Real originMu)
//------------------------------------------------------------------------------
/**
 * This method calculates the BPlane Calculation Objects.
 *
 * @param item       string indicating which item to compute.
 * @param state      input state in the desired coordinate system
 * @param originMu   gravitational constant for the origin of the
 *                   coordinate system (assumes origin is a Celestial Body)
 *
 * @return           Real value requested
 *
 */
//------------------------------------------------------------------------------
Real GmatCalcUtil::CalculateBPlaneData(const wxString &item, const Rvector6 &state,
                                       const Real originMu)
{
   Rvector3 pos(state[0], state[1], state[2]);
   Rvector3 vel(state[3], state[4], state[5]);

   Real rMag       = pos.GetMagnitude();
   Real vMag       = vel.GetMagnitude();

   // Compute eccentricity related information
   Rvector3 eVec   = ((vMag*vMag - originMu/rMag) * pos - (pos*vel)*vel) / originMu;

   Real eMag        = eVec.GetMagnitude();

   // if eMag <= 1, then the method fails, orbit should be hyperbolic
   if (eMag <= 1.0)
      return GmatMathConstants::QUIET_NAN;
//      throw UtilityException
//         (wxT("CalculationUtilities::CalculateBPlaneData() : ERROR - eccentricity magnitude is <= 1.0. eMag: ") +
//          GmatRealUtil::ToString(eMag));

   eVec.Normalize();

   // Compute the angular momentum and orbit normal vectors
   Rvector3 hVec    = Cross(pos, vel);
   Real hMag        = hVec.GetMagnitude();
   hVec.Normalize();
   Rvector3 nVec    = Cross(hVec, eVec);

   // Compute semiminor axis, b
   Real b           = (hMag*hMag) / (originMu * Sqrt(eMag*eMag - 1.0));

   // Compute incoming asymptote
   Real oneOverEmag = 1.0/eMag;
   Real temp        = Sqrt(1.0 - oneOverEmag*oneOverEmag);
   Rvector3 sVec    = (eVec/eMag) + (temp*nVec);

   // Compute the B-vector
   Rvector3 bVec    = b * (temp * eVec - oneOverEmag*nVec);

   // Compute T and R vector
   Rvector3 sVec1(sVec[1], -sVec[0], 0.0);
   Rvector3 tVec = sVec1 / Sqrt(sVec[0]*sVec[0] + sVec[1]*sVec[1]);
   Rvector3 rVec = Cross(sVec, tVec);

   Real bDotT = bVec * tVec;
   Real bDotR = bVec * rVec;

   if (item == wxT("BDotR"))
   {
      return bDotR;
   }
   else if (item == wxT("BDotT"))
   {
      return bDotT;
   }
   else if (item == wxT("BVectorMag"))
   {
      return Sqrt(bDotT*bDotT + bDotR*bDotR);
   }
   else if (item == wxT("BVectorAngle"))
   {
      return ATan(bDotR, bDotT) * GmatMathConstants::DEG_PER_RAD;
   }
   else
   {
      throw UtilityException
         (wxT("CalculationUtilities::CalculateBPlaneData() Unknown item: ") + item);
   }

}


//------------------------------------------------------------------------------
//  Real  CalculateAngularData(const wxString &item, const Rvector6 &state,
//                             const Real originMu, const Rvector3 &originToSunUnit)
//------------------------------------------------------------------------------
/**
 * This method calculates the Angular (Orbit) Calculation Objects.
 *
 * @param item       string indicating which item to compute.
 * @param state      input state in the desired coordinate system
 * @param originMu   gravitational constant for the origin of the
 *                   coordinate system (assumes origin is a Celestial Body)
 * @param originToSunUnit origin-to-sun unit vector
 *
 * @return           Real value requested
 *
 */
//------------------------------------------------------------------------------
Real GmatCalcUtil::CalculateAngularData(const wxString &item, const Rvector6 &state,
                                        const Real &originMu, const Rvector3 &originToSunUnit)
{
   Rvector3 pos(state[0], state[1], state[2]);
   Rvector3 vel(state[3], state[4], state[5]);

   Rvector3 hVec3 = Cross(pos, vel);
   Real     h     = Sqrt(hVec3 * hVec3);

   if (item == wxT("SemilatusRectum"))
   {
      if (h < GmatOrbitConstants::KEP_TOL)
         return 0.0;
      else
         return (h / originMu) * h;
   }
   else if (item == wxT("HMag"))
   {
      return h;
   }
   else if (item == wxT("HX"))
   {
      return hVec3[0];
   }
   else if (item == wxT("HY"))
   {
      return hVec3[1];
   }
   else if (item == wxT("HZ"))
   {
      return hVec3[2];
   }
   else if (item == wxT("BetaAngle"))
   {
      hVec3.Normalize();
      Real betaAngle = ASin(hVec3*originToSunUnit) * GmatMathConstants::DEG_PER_RAD;
      return betaAngle;
   }
   else if ((item == wxT("RLA")) || (item == wxT("DLA")))
   {
      // Compute the eccentricity vector
      Real     r     = pos.GetMagnitude();
      Real     v     = vel.GetMagnitude();
      Rvector3 e     = ((((v * v) - originMu / r) * pos) - (pos * vel) *vel) / originMu;
      Real     ecc   = e.GetMagnitude();
      if (Abs(ecc) < 1.0 + GmatOrbitConstants::KEP_ECC_TOL)
         return GmatMathConstants::QUIET_NAN;

      // Compute orbit normal unit vector
      Rvector3 hVec3 = Cross(pos, vel);
      Real     h     = hVec3.GetMagnitude();

      // Compute C3
      Real     C3    = v * v - (2.0 * originMu) / r;
      Real     s_1   = 1.0 / (1.0 + C3 * (h / originMu) * (h / originMu));
      Rvector3 s     = s_1 * ((Sqrt(C3) / originMu) * Cross(hVec3, e) - e);
      if (item == wxT("RLA"))
         return ATan2(s[1], s[0]) * GmatMathConstants::DEG_PER_RAD;
      else // DLA
         return ASin(s[2]) * GmatMathConstants::DEG_PER_RAD;
   }
   else
   {
      throw UtilityException
         (wxT("CalculationUtilities::CalculateAngularData() Unknown item: ") + item);
   }

}

//------------------------------------------------------------------------------
//  Real  CalculateKeplerianData(const wxString &item, const Rvector6 &state,
//                               const Real originMu)
//------------------------------------------------------------------------------
/**
 * This method calculates the Keplerian Calculation Objects.
 *
 * @param item       string indicating which item to compute.
 * @param state      input state in the desired coordinate system
 * @param originMu   gravitational constant for the origin of the
 *                   coordinate system (assumes origin is a Celestial Body)
 *
 * @return           Real value requested
 *
 */
//------------------------------------------------------------------------------
Real GmatCalcUtil::CalculateKeplerianData(const wxString &item, const Rvector6 &state,
                                          const Real originMu)
{
   Rvector3 pos(state[0], state[1], state[2]);
   Rvector3 vel(state[3], state[4], state[5]);

   Real sma = Keplerian::CartesianToSMA(originMu, pos, vel);
   Real ecc = Keplerian::CartesianToECC(originMu, pos, vel);

   if (GmatMathUtil::Abs(1.0 - ecc) <= GmatOrbitConstants::KEP_ECC_TOL)
   {
      throw UtilityException
         (wxT("In CalculateKeplerianData, Error in conversion to Keplerian state: ")
          wxT("The state results in an orbit that is nearly parabolic.\n"));
   }

  if (sma*(1 - ecc) < .001)
   {
      throw UtilityException
         (wxT("In CalculateKeplerianData, Error in conversion to Keplerian state: ")
          wxT("The state results in a singular conic section with radius of periapsis less than 1 m.\n"));
   }

  if (item == wxT("MeanMotion"))
  {
     if (ecc < (1.0 - GmatOrbitConstants::KEP_ECC_TOL))      // Ellipse
        return Sqrt(originMu / (sma*sma*sma));
     else if (ecc > (1.0 + GmatOrbitConstants::KEP_ECC_TOL)) // Hyperbola
        return Sqrt(-(originMu / (sma*sma*sma)));
     else
        return 2.0 * Sqrt(originMu); // Parabola
  }
  else if (item == wxT("VelApoapsis"))
  {
     if ( (ecc < 1.0 - GmatOrbitConstants::KEP_ECC_TOL) || (ecc > 1.0 + GmatOrbitConstants::KEP_ECC_TOL))  //Ellipse and Hyperbola
        return Sqrt( (originMu/sma)*((1-ecc)/(1+ecc)) );
     else
        return 0.0; // Parabola
  }
  else if (item == wxT("VelPeriapsis"))
  {
     return Sqrt( (originMu/sma)*((1+ecc)/(1-ecc)) );
  }
  else if (item == wxT("OrbitPeriod"))
  {
     if (sma < 0.0)
        return 0.0;
     else
        return GmatMathConstants::TWO_PI * Sqrt((sma * sma * sma)/ originMu);
  }
  else if (item == wxT("RadApoapsis"))
  {
      if ( (ecc < 1.0 - GmatOrbitConstants::KEP_ECC_TOL) || (ecc > 1.0 + GmatOrbitConstants::KEP_ECC_TOL)) //Ellipse and Hyperbola
          return sma * (1.0 + ecc);
      else
          return 0.0;   // Parabola
  }
  else if (item == wxT("RadPeriapsis"))
  {
     return sma * (1.0 - ecc);
  }
  else if (item == wxT("C3Energy"))
  {
     return -originMu / sma;
  }
  else if (item == wxT("Energy"))
  {
     return -originMu / (2.0 * sma);
  }
  else
  {
     throw UtilityException
        (wxT("CalculationUtilities::CalculateKeplerianData() Unknown item: ") + item);
  }
}

Real GmatCalcUtil::CalculatePlanetData(const wxString &item, const Rvector6 &state,
                                       const Real originRadius, const Real originFlattening, const Real originHourAngle)
{
   if (item == wxT("MHA"))
   {
      return originHourAngle;   // is this call even really necessary???
   }
   else if (item == wxT("Longitude"))
   {
      // The input state is in the origin-centered BodyFixed Coordinate System
      Real longitude = ATan(state[1], state[0]) * GmatMathConstants::DEG_PER_RAD;
      longitude = AngleUtil::PutAngleInDegRange(longitude, -180.0, 180.0);

      return longitude;
   }
   else if ((item == wxT("Latitude")) || (item == wxT("Altitude")))
   {

      // Reworked to match Vallado algorithm 12 (Vallado, 2nd ed, p. 177)

      // Note -- using cmath here because I know it better -- may want to change
      // to GmatMath

      Real rxy               = sqrt(state[0]*state[0] + state[1]*state[1]);
      Real geolat            = atan2(state[2], rxy);
      Real delta             = 1.0;
      Real geodeticTolerance = 1.0e-7;    // Better than 0.0001 degrees
      Real ecc2              = 2.0 * originFlattening - originFlattening*originFlattening;

      Real cFactor, oldlat, sinlat;
      while (delta > geodeticTolerance)
      {
         oldlat  = geolat;
         sinlat  = sin(oldlat);
         cFactor = originRadius / sqrt(1.0 - ecc2 * sinlat * sinlat);
         geolat  = atan2(state[2] + cFactor*ecc2*sinlat, rxy);
         delta   = fabs(geolat - oldlat);
      }

      if (item == wxT("Latitude"))
      {
         //return geolat * 180.0 / PI;
         // put latitude between -90 and 90
         geolat = geolat * 180.0 / GmatMathConstants::PI;
         geolat = AngleUtil::PutAngleInDegRange(geolat, -90.0, 90.0);
         return geolat;
      }
      else  // item == wxT("Altitude")
      {
         sinlat = sin(geolat);
         cFactor = originRadius / sqrt(1.0 - ecc2 * sinlat * sinlat);
         return rxy / cos(geolat) - cFactor;
      }
   }
   else if (item == wxT("LST"))
   {
      // compute Local Sidereal Time (LST = GMST + Longitude)
      // according to Vallado Eq. 3-41
      // The input state is in the origin-centered BodyFixed Coordinate System
      Real longitude = ATan(state[1], state[0]) * GmatMathConstants::DEG_PER_RAD;
      longitude      = AngleUtil::PutAngleInDegRange(longitude, -180.0, 180.0);
      Real lst       = originHourAngle + longitude;
      lst            = AngleUtil::PutAngleInDegRange(lst, 0.0, 360.0);

      return lst;
   }
   else
   {
      throw UtilityException
         (wxT("CalculationUtilities::CalculatePlanetData() Unknown item: ") + item);
   }
}

