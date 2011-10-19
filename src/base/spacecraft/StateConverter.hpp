//$Id: StateConverter.hpp 9513 2011-04-30 21:23:06Z djcinsb $ 
//------------------------------------------------------------------------------
//                                 StateConverter
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author:  Joey Gurganus
// Created: 2004/03/22
//
/**
 * Definition of the StateConverter class
 */
//------------------------------------------------------------------------------

#ifndef StateConverter_hpp
#define StateConverter_hpp

#include "SolarSystem.hpp" 
#include "CoordinateSystem.hpp"
#include "Anomaly.hpp"
#include "Rvector6.hpp"

class GMAT_API StateConverter 
{
public:

   enum StateType
   {
      CARTESIAN,
      KEPLERIAN,
      MOD_KEPLERIAN,
      SPH_AZFPA,
      SPH_RADEC,
      EQUINOCTIAL,
      StateTypeCount
   };
   
   // Default constructor
   StateConverter();
   StateConverter(const wxString &newType);
   StateConverter(const wxString &newType, const Real newMu);
   
   // Copy constructor
   StateConverter(const StateConverter &state);
   // Assignment operator
   StateConverter& operator=(const StateConverter &state);
   
   // Destructor
   virtual ~StateConverter();
   
   // public method 
   Real GetMu() const { return mMu; }
   void SetMu(Real mu) { mMu = mu; }
   bool SetMu(const CoordinateSystem *cs);
   
   Rvector6 FromCartesian(const Rvector6 &state, const wxString &toType,
                          const wxString &anomalyType = wxT("TA"));
   
   Rvector6 FromKeplerian(const Rvector6 &state, const wxString &toType,
                          const wxString &anomalyType = wxT("TA"));
   
   Rvector6 FromModKeplerian(const Rvector6 &state, const wxString &toType,
                             const wxString &anomalyType = wxT("TA"));
   
   Rvector6 FromSphericalAZFPA(const Rvector6 &state, const wxString &toType,
                               const wxString &anomalyType = wxT("TA"));
   
   Rvector6 FromSphericalRADEC(const Rvector6 &state, const wxString &toType,
                               const wxString &anomalyType = wxT("TA"));
   
   Rvector6 FromEquinoctial(const Rvector6 &state, const wxString &toType,
                            const wxString &anomalyType = wxT("TA"));
   
   Rvector6 Convert(const Rvector6 &state, const wxString &fromType,
                    const wxString &toType,
                    const wxString &anomalyType = wxT("TA"));
   
   Rvector6 Convert(const Rvector6 &state, const wxString &fromType,
                    const wxString &toType, Anomaly &anomaly);
   
   Rvector6 Convert(const Real *state, const wxString &fromType,
                    const wxString &toType, Anomaly &anomaly);
   
   // Constant variable
   static const Real DEFAULT_MU;  // km^3/s^2
   
   static wxString        GetTypeString(const wxString &type);
   static Integer            GetTypeCount() { return StateTypeCount; }
   static const wxString* GetStateTypeList();
   static bool               RequiresCelestialBodyOrigin(const wxString &type);
   
protected:

private:
   Real         mMu;
   
   static const wxString STATE_TYPE_TEXT[StateTypeCount];
   static const bool        REQUIRES_CB_ORIGIN[StateTypeCount];

   Rvector6 CartesianToEquinoctial(const Rvector6& cartesian, const Real& mu);
   Rvector6 EquinoctialToCartesian(const Rvector6& equinoctial, const Real& mu);

   Rvector6 CartesianToSphericalAZFPA(const Rvector6& cartesian);
   Rvector6 SphericalAZFPAToCartesian(const Rvector6& spherical);

   Rvector6 CartesianToSphericalRADEC(const Rvector6& cartesian);
   Rvector6 SphericalRADECToCartesian(const Rvector6& spherical);

};

#endif // StateConverter_hpp
