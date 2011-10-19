//$Id: ParameterFactory.cpp 9754 2011-08-10 12:42:26Z wendys-dev $
//------------------------------------------------------------------------------
//                            ParameterFactory
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
// Developed further jointly by NASA/GSFC, Thinking Systems, Inc., and 
// Schafer Corp., under AFRL NOVA Contract #FA945104D03990003
//
// Author: Darrel Conway
// Created: 2003/10/28
// Modified:  Dunn Idle (added MRPs)
// Date:      2010/08/24
//
/**
 *  Implementation code for the ParameterFactory class, responsible
 *  for creating Parameter objects.
 */
//------------------------------------------------------------------------------


#include "ParameterFactory.hpp"

#include "TimeParameters.hpp"
#include "CartesianParameters.hpp"
#include "KeplerianParameters.hpp"
#include "SphericalParameters.hpp"
#include "EquinoctialParameters.hpp"
#include "OrbitalParameters.hpp"
#include "AngularParameters.hpp"
#include "EnvParameters.hpp"
#include "PlanetParameters.hpp"
#include "Variable.hpp"
#include "StringVar.hpp"
#include "Array.hpp"
#include "BplaneParameters.hpp"
#include "BurnParameters.hpp"
#include "AttitudeParameters.hpp"
#include "BallisticMassParameters.hpp"
#include "OrbitStmParameters.hpp"
#include "HardwareParameters.hpp"
#include "MessageInterface.hpp"

//---------------------------------
//  public methods
//---------------------------------

//------------------------------------------------------------------------------
//  CreateParameter(const wxString &ofType, const wxString &withName)
//------------------------------------------------------------------------------
/**
 * This method creates and returns an object of the requested Parameter class
 *
 * @param <ofType> the Parameter object to create and return.
 * @param <withName> the name to give the newly-created Parameter object.
 */
//------------------------------------------------------------------------------
Parameter* ParameterFactory::CreateParameter(const wxString &ofType,
                                             const wxString &withName)
{
   // User defined parameters
   if (ofType == wxT("Variable"))
      return new Variable(withName);
   if (ofType == wxT("String"))
      return new StringVar(withName);
   if (ofType == wxT("Array"))
      return new Array(withName);
   
   // Time parameters
   if (ofType == wxT("ElapsedDays"))
      return new ElapsedDays(withName);
   if (ofType == wxT("ElapsedSecs"))
      return new ElapsedSecs(withName);
   if (ofType == wxT("CurrA1MJD"))
      return new CurrA1MJD(withName);
   if (ofType == wxT("A1ModJulian"))
      return new A1ModJulian(withName);
   if (ofType == wxT("A1Gregorian"))
      return new A1Gregorian(withName);
   if (ofType == wxT("TAIModJulian"))
      return new TAIModJulian(withName);
   if (ofType == wxT("TAIGregorian"))
      return new TAIGregorian(withName);
   if (ofType == wxT("TTModJulian"))
      return new TTModJulian(withName);
   if (ofType == wxT("TTGregorian"))
      return new TTGregorian(withName);
   if (ofType == wxT("TDBModJulian"))
      return new TDBModJulian(withName);
   if (ofType == wxT("TDBGregorian"))
      return new TDBGregorian(withName);
   if (ofType == wxT("TCBModJulian"))
      return new TCBModJulian(withName);
   if (ofType == wxT("TCBGregorian"))
      return new TCBGregorian(withName);
   if (ofType == wxT("UTCModJulian"))
      return new UTCModJulian(withName);
   if (ofType == wxT("UTCGregorian"))
      return new UTCGregorian(withName);

   // Cartesian parameters
   if (ofType == wxT("X"))
      return new CartX(withName);
   if (ofType == wxT("Y"))
      return new CartY(withName);
   if (ofType == wxT("Z"))
      return new CartZ(withName);
   if (ofType == wxT("VX"))
      return new CartVx(withName);
   if (ofType == wxT("VY"))
      return new CartVy(withName);
   if (ofType == wxT("VZ"))
      return new CartVz(withName);
   if (ofType == wxT("Cartesian"))
      return new CartState(withName);

    // Keplerian parameters
   if (ofType == wxT("SMA"))
      return new KepSMA(withName);
   if (ofType == wxT("ECC"))
      return new KepEcc(withName);
   if (ofType == wxT("INC"))
      return new KepInc(withName);
   if (ofType == wxT("RAAN"))
      return new KepRAAN(withName);
   if (ofType == wxT("RADN"))
      return new KepRADN(withName);
   if (ofType == wxT("AOP"))
      return new KepAOP(withName);
   if (ofType == wxT("TA"))
      return new KepTA(withName);
   if (ofType == wxT("MA"))
      return new KepMA(withName);
   if (ofType == wxT("EA"))
      return new KepEA(withName);
   if (ofType == wxT("HA"))
      return new KepHA(withName);
   if (ofType == wxT("MM"))
      return new KepMM(withName);
   if (ofType == wxT("Keplerian"))
      return new KepElem(withName);
   if (ofType == wxT("ModKeplerian"))
      return new ModKepElem(withName);

   // Spherical parameters
   if (ofType == wxT("RMAG"))
      return new SphRMag(withName);
   if (ofType == wxT("RA"))
      return new SphRA(withName);
   if (ofType == wxT("DEC"))
      return new SphDec(withName);
   if (ofType == wxT("VMAG"))
      return new SphVMag(withName);
   if (ofType == wxT("RAV"))
      return new SphRAV(withName);
   if (ofType == wxT("DECV"))
      return new SphDecV(withName);
   if (ofType == wxT("AZI"))
      return new SphAzi(withName);
   if (ofType == wxT("FPA"))
      return new SphFPA(withName);
   if (ofType == wxT("SphericalRADEC")) 
      return new SphRaDecElem(withName);
   if (ofType == wxT("SphericalAZFPA")) 
      return new SphAzFpaElem(withName);
   if (ofType == wxT("Altitude"))
      return new Altitude(withName);

   // Equinoctial parameters
   if (ofType == wxT("EquinoctialH"))
      return new EquinEy(withName);
   if (ofType == wxT("EquinoctialK"))
      return new EquinEx(withName);
   if (ofType == wxT("EquinoctialP"))
      return new EquinNy(withName);
   if (ofType == wxT("EquinoctialQ"))
      return new EquinNx(withName);
   if (ofType == wxT("MLONG"))
      return new EquinMlong(withName);
   if (ofType == wxT("Equinoctial"))
      return new EquinState(withName);

   // Orbital parameters
   if (ofType == wxT("VelApoapsis"))
      return new VelApoapsis(withName);
   if (ofType == wxT("VelPeriapsis"))
      return new VelPeriapsis(withName);
   if (ofType == wxT("Apoapsis"))
      return new Apoapsis(withName);
   if (ofType == wxT("Periapsis"))
      return new Periapsis(withName);
   if (ofType == wxT("OrbitPeriod"))
      return new OrbitPeriod(withName);
   if (ofType == wxT("RadApo"))
      return new RadApoapsis(withName);
   if (ofType == wxT("RadPer"))
      return new RadPeriapsis(withName);
   if (ofType == wxT("C3Energy"))
      return new C3Energy(withName);
   if (ofType == wxT("Energy"))
      return new Energy(withName);

   // Angular parameters
   if (ofType == wxT("SemilatusRectum"))
      return new SemilatusRectum(withName);
   if (ofType == wxT("HMAG"))
      return new AngularMomentumMag(withName);
   if (ofType == wxT("HX"))
      return new AngularMomentumX(withName);
   if (ofType == wxT("HY"))
      return new AngularMomentumY(withName);
   if (ofType == wxT("HZ"))
      return new AngularMomentumZ(withName);
   if (ofType == wxT("DLA"))
      return new DLA(withName);
   if (ofType == wxT("RLA"))
      return new RLA(withName);

   // Environmental parameters
   if (ofType == wxT("AtmosDensity"))
      return new AtmosDensity(withName);
   
   // Planet parameters
   if (ofType == wxT("MHA"))
      return new MHA(withName);
   if (ofType == wxT("Longitude"))
      return new Longitude(withName);
   if (ofType == wxT("Latitude"))
      return new Latitude(withName);
   if (ofType == wxT("LST"))
      return new LST(withName);
   if (ofType == wxT("BetaAngle"))
      return new BetaAngle(withName);
   
   // B-Plane parameters
   if (ofType == wxT("BdotT"))
      return new BdotT(withName);
   if (ofType == wxT("BdotR"))
      return new BdotR(withName);
   if (ofType == wxT("BVectorMag"))
      return new BVectorMag(withName);
   if (ofType == wxT("BVectorAngle"))
      return new BVectorAngle(withName);
   
   // ImpulsiveBurn parameters
   if (ofType == wxT("Element1") || ofType == wxT("Element2") || ofType == wxT("Element3"))
      return new ImpBurnElements(ofType, withName);   
   if (ofType == wxT("V") || ofType == wxT("N") || ofType == wxT("B"))
      return new ImpBurnElements(ofType, withName);
   
   // Attitude parameters
   if (ofType == wxT("DCM11") || ofType == wxT("DirectionCosineMatrix11"))
      return new DCM11(withName);
   if (ofType == wxT("DCM12") || ofType == wxT("DirectionCosineMatrix12"))
      return new DCM12(withName);
   if (ofType == wxT("DCM13") || ofType == wxT("DirectionCosineMatrix13"))
      return new DCM13(withName);
   if (ofType == wxT("DCM21") || ofType == wxT("DirectionCosineMatrix21"))
      return new DCM21(withName);
   if (ofType == wxT("DCM22") || ofType == wxT("DirectionCosineMatrix22"))
      return new DCM22(withName);
   if (ofType == wxT("DCM23") || ofType == wxT("DirectionCosineMatrix23"))
      return new DCM23(withName);
   if (ofType == wxT("DCM31") || ofType == wxT("DirectionCosineMatrix31"))
      return new DCM31(withName);
   if (ofType == wxT("DCM32") || ofType == wxT("DirectionCosineMatrix32"))
      return new DCM32(withName);
   if (ofType == wxT("DCM33") || ofType == wxT("DirectionCosineMatrix33"))
      return new DCM33(withName);
   if (ofType == wxT("EulerAngle1"))
      return new EulerAngle1(withName);
   if (ofType == wxT("EulerAngle2"))
      return new EulerAngle2(withName);
   if (ofType == wxT("EulerAngle3"))
      return new EulerAngle3(withName);
   if (ofType == wxT("MRP1"))  // Dunn Added
      return new MRP1(withName);
   if (ofType == wxT("MRP2"))  // Dunn Added
      return new MRP2(withName);
   if (ofType == wxT("MRP3"))  // Dunn Added
      return new MRP3(withName);
   if (ofType == wxT("Q1") || ofType == wxT("q1"))
      return new Quat1(withName);
   if (ofType == wxT("Q2") || ofType == wxT("q2"))
      return new Quat2(withName);
   if (ofType == wxT("Q3") || ofType == wxT("q3"))
      return new Quat3(withName);
   if (ofType == wxT("Q4") || ofType == wxT("q4"))
      return new Quat4(withName);
      
   if (ofType == wxT("AngularVelocityX") || ofType == wxT("AngVelX"))
      return new AngVelX(withName);
   if (ofType == wxT("AngularVelocityY") || ofType == wxT("AngVelY"))
      return new AngVelY(withName);
   if (ofType == wxT("AngularVelocityZ") || ofType == wxT("AngVelZ"))
      return new AngVelZ(withName);
   if (ofType == wxT("EulerAngleRate1"))
      return new EulerAngleRate1(withName);
   if (ofType == wxT("EulerAngleRate2"))
      return new EulerAngleRate2(withName);
   if (ofType == wxT("EulerAngleRate3"))
      return new EulerAngleRate3(withName);
   
   // Ballistic/Mass parameters
   if (ofType == wxT("DryMass"))
      return new DryMass(withName);
   if (ofType == wxT("Cd"))
      return new DragCoeff(withName);
   if (ofType == wxT("Cr"))
      return new ReflectCoeff(withName);
   if (ofType == wxT("DragArea"))
      return new DragArea(withName);
   if (ofType == wxT("SRPArea"))
      return new SRPArea(withName);
   if (ofType == wxT("TotalMass"))
      return new TotalMass(withName);
   
   // orbit STM parameters
   if (ofType == wxT("OrbitSTM"))
      return new OrbitStm(withName);
   if (ofType == wxT("OrbitSTMA"))
      return new OrbitStmA(withName);
   if (ofType == wxT("OrbitSTMB"))
      return new OrbitStmB(withName);
   if (ofType == wxT("OrbitSTMC"))
      return new OrbitStmC(withName);
   if (ofType == wxT("OrbitSTMD"))
      return new OrbitStmD(withName);
   
   // FuelTank parameters
   if (ofType == wxT("FuelMass"))
      return new FuelMass(withName);
   if (ofType == wxT("Pressure"))
      return new Pressure(withName);
   if (ofType == wxT("Temperature"))
      return new Temperature(withName);
   if (ofType == wxT("RefTemperature"))
      return new RefTemperature(withName);
   if (ofType == wxT("Volume"))
      return new Volume(withName);
   if (ofType == wxT("FuelDensity"))
      return new FuelDensity(withName);
   
   // Thruster parameters
   if (ofType == wxT("DutyCycle"))
      return new DutyCycle(withName);
   if (ofType == wxT("ThrustScaleFactor"))
      return new ThrustScaleFactor(withName);
   if (ofType == wxT("GravitationalAccel"))
      return new GravitationalAccel(withName);
   
   if (ofType == wxT("C1")  || ofType == wxT("C2")  || ofType == wxT("C3")  || ofType == wxT("C4")  ||
       ofType == wxT("C5")  || ofType == wxT("C6")  || ofType == wxT("C7")  || ofType == wxT("C8")  ||
       ofType == wxT("C9")  || ofType == wxT("C10") || ofType == wxT("C11") || ofType == wxT("C12") ||
       ofType == wxT("C13") || ofType == wxT("C14") || ofType == wxT("C15") || ofType == wxT("C16"))
      return new ThrustCoefficients(ofType, withName);
   
   if (ofType == wxT("K1")  || ofType == wxT("K2")  || ofType == wxT("K3")  || ofType == wxT("K4")  ||
       ofType == wxT("K5")  || ofType == wxT("K6")  || ofType == wxT("K7")  || ofType == wxT("K8")  ||
       ofType == wxT("K9")  || ofType == wxT("K10") || ofType == wxT("K11") || ofType == wxT("K12") ||
       ofType == wxT("K13") || ofType == wxT("K14") || ofType == wxT("K15") || ofType == wxT("K16"))
      return new ImpulseCoefficients(ofType, withName);
   
   if (ofType == wxT("ThrustDirection1") || ofType == wxT("ThrustDirection2") ||
       ofType == wxT("ThrustDirection3"))
      return new ThrustDirections(ofType, withName);
   
   // add others here
   
   MessageInterface::ShowMessage
      (wxT("**** ERROR **** Cannot create a parameter with unknown type \"%s\"\n"),
       ofType.c_str());
   
   return NULL;
}


//------------------------------------------------------------------------------
//  ParameterFactory()
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class ParameterFactory
 * (default constructor).
 *
 *
 */
//------------------------------------------------------------------------------
ParameterFactory::ParameterFactory()
   : Factory(Gmat::PARAMETER)
{
   if (creatables.empty())
   {
      // User defined parameters
      creatables.push_back(wxT("Variable"));
      creatables.push_back(wxT("String"));
      creatables.push_back(wxT("Array"));
      
      // Time parameters
      creatables.push_back(wxT("ElapsedDays"));
      creatables.push_back(wxT("ElapsedSecs"));
      creatables.push_back(wxT("CurrA1MJD"));
      creatables.push_back(wxT("A1ModJulian"));
      creatables.push_back(wxT("A1Gregorian"));
      creatables.push_back(wxT("TAIModJulian"));
      creatables.push_back(wxT("TAIGregorian"));
      creatables.push_back(wxT("TTModJulian"));
      creatables.push_back(wxT("TTGregorian"));
      creatables.push_back(wxT("TDBModJulian"));
      creatables.push_back(wxT("TDBGregorian"));
      creatables.push_back(wxT("TCBModJulian"));
      creatables.push_back(wxT("TCBGregorian"));
      creatables.push_back(wxT("UTCModJulian"));
      creatables.push_back(wxT("UTCGregorian"));

      // Cartesian parameters
      creatables.push_back(wxT("X"));
      creatables.push_back(wxT("Y"));
      creatables.push_back(wxT("Z"));
      creatables.push_back(wxT("VX"));
      creatables.push_back(wxT("VY"));
      creatables.push_back(wxT("VZ"));
      creatables.push_back(wxT("Cartesian"));

      // Keplerian parameters
      creatables.push_back(wxT("SMA"));
      creatables.push_back(wxT("ECC"));
      creatables.push_back(wxT("INC"));
      creatables.push_back(wxT("RAAN"));
      creatables.push_back(wxT("RADN"));
      creatables.push_back(wxT("AOP"));
      creatables.push_back(wxT("TA"));
      creatables.push_back(wxT("MA"));
      creatables.push_back(wxT("EA"));
      creatables.push_back(wxT("HA"));
      creatables.push_back(wxT("MM"));
      creatables.push_back(wxT("Keplerian"));
      creatables.push_back(wxT("ModKeplerian"));

      // Spherical parameters
      creatables.push_back(wxT("RMAG"));
      creatables.push_back(wxT("RA"));
      creatables.push_back(wxT("DEC"));
      creatables.push_back(wxT("VMAG"));
      creatables.push_back(wxT("RAV"));
      creatables.push_back(wxT("DECV"));
      creatables.push_back(wxT("AZI"));
      creatables.push_back(wxT("FPA"));
      creatables.push_back(wxT("SphericalRADEC"));
      creatables.push_back(wxT("SphericalAZFPA"));
      creatables.push_back(wxT("Altitude"));

      // Equinoctial parameters
//      creatables.push_back(wxT("h"));
//      creatables.push_back(wxT("k"));
//      creatables.push_back(wxT("p"));
//      creatables.push_back(wxT("q"));
      creatables.push_back(wxT("EquinoctialH"));
      creatables.push_back(wxT("EquinoctialK"));
      creatables.push_back(wxT("EquinoctialP"));
      creatables.push_back(wxT("EquinoctialQ"));
      creatables.push_back(wxT("MLONG"));
      creatables.push_back(wxT("Equinoctial"));

      // Orbital parameters
      creatables.push_back(wxT("VelApoapsis"));
      creatables.push_back(wxT("VelPeriapsis"));
      creatables.push_back(wxT("Apoapsis"));
      creatables.push_back(wxT("Periapsis"));
      creatables.push_back(wxT("OrbitPeriod"));
      creatables.push_back(wxT("RadApo"));
      creatables.push_back(wxT("RadPer"));
      creatables.push_back(wxT("C3Energy"));
      creatables.push_back(wxT("Energy"));

      // Angular parameters
      creatables.push_back(wxT("SemilatusRectum"));
      creatables.push_back(wxT("HMAG"));
      creatables.push_back(wxT("HX"));
      creatables.push_back(wxT("HY"));
      creatables.push_back(wxT("HZ"));
      creatables.push_back(wxT("DLA"));
      creatables.push_back(wxT("RLA"));
      
      // Environmental parameters
      #ifdef __ENABLE_ATMOS_DENSITY__
      creatables.push_back(wxT("AtmosDensity"));
      #endif
      
      // Planet parameters
      creatables.push_back(wxT("MHA"));
      creatables.push_back(wxT("Longitude"));
      creatables.push_back(wxT("Latitude"));
      creatables.push_back(wxT("LST"));
      creatables.push_back(wxT("BetaAngle"));
      
      // B-Plane parameters
      creatables.push_back(wxT("BdotT"));
      creatables.push_back(wxT("BdotR"));
      creatables.push_back(wxT("BVectorMag"));
      creatables.push_back(wxT("BVectorAngle"));
      
      // Burn parameters
      creatables.push_back(wxT("Element1"));
      creatables.push_back(wxT("Element2"));
      creatables.push_back(wxT("Element3"));
      creatables.push_back(wxT("V"));
      creatables.push_back(wxT("N"));
      creatables.push_back(wxT("B"));
      
      // Attitude parameters
      creatables.push_back(wxT("DCM11"));
      creatables.push_back(wxT("DCM12"));
      creatables.push_back(wxT("DCM13"));
      creatables.push_back(wxT("DCM21"));
      creatables.push_back(wxT("DCM22"));
      creatables.push_back(wxT("DCM23"));
      creatables.push_back(wxT("DCM31"));
      creatables.push_back(wxT("DCM32"));
      creatables.push_back(wxT("DCM33"));
      creatables.push_back(wxT("EulerAngle1"));
      creatables.push_back(wxT("EulerAngle2"));
      creatables.push_back(wxT("EulerAngle3"));
      creatables.push_back(wxT("MRP1"));  // Dunn Added
      creatables.push_back(wxT("MRP2"));  // Dunn Added
      creatables.push_back(wxT("MRP3"));  // Dunn Added
      creatables.push_back(wxT("Q1"));
      creatables.push_back(wxT("Q2"));
      creatables.push_back(wxT("Q3"));
      creatables.push_back(wxT("Q4"));
      creatables.push_back(wxT("AngularVelocityX"));
      creatables.push_back(wxT("AngularVelocityY"));
      creatables.push_back(wxT("AngularVelocityZ"));
      creatables.push_back(wxT("EulerAngleRate1"));
      creatables.push_back(wxT("EulerAngleRate2"));
      creatables.push_back(wxT("EulerAngleRate3"));
      
      // Ballistic/Mass parameters
      creatables.push_back(wxT("DryMass"));
      creatables.push_back(wxT("Cd"));
      creatables.push_back(wxT("Cr"));
      creatables.push_back(wxT("DragArea"));
      creatables.push_back(wxT("SRPArea"));
      creatables.push_back(wxT("TotalMass"));
      
      // Orbit STM parameters
      creatables.push_back(wxT("OrbitSTM"));
      creatables.push_back(wxT("OrbitSTMA"));
      creatables.push_back(wxT("OrbitSTMB"));
      creatables.push_back(wxT("OrbitSTMC"));
      creatables.push_back(wxT("OrbitSTMD"));
      
      // FuelTank parameters
      creatables.push_back(wxT("FuelMass"));
      creatables.push_back(wxT("Pressure"));
      creatables.push_back(wxT("Temperature"));
      creatables.push_back(wxT("RefTemperature"));
      creatables.push_back(wxT("Volume"));
      creatables.push_back(wxT("FuelDensity"));
      
      // Thruster parameters
      creatables.push_back(wxT("DutyCycle"));
      creatables.push_back(wxT("ThrustScaleFactor"));
      creatables.push_back(wxT("GravitationalAccel"));
      
      creatables.push_back(wxT("C1"));
      creatables.push_back(wxT("C2"));
      creatables.push_back(wxT("C3"));
      creatables.push_back(wxT("C4"));
      creatables.push_back(wxT("C5"));
      creatables.push_back(wxT("C6"));
      creatables.push_back(wxT("C7"));
      creatables.push_back(wxT("C8"));
      creatables.push_back(wxT("C9"));
      creatables.push_back(wxT("C10"));
      creatables.push_back(wxT("C11"));
      creatables.push_back(wxT("C12"));
      creatables.push_back(wxT("C13"));
      creatables.push_back(wxT("C14"));
      creatables.push_back(wxT("C15"));
      creatables.push_back(wxT("C16"));
      
      creatables.push_back(wxT("K1"));
      creatables.push_back(wxT("K2"));
      creatables.push_back(wxT("K3"));
      creatables.push_back(wxT("K4"));
      creatables.push_back(wxT("K5"));
      creatables.push_back(wxT("K6"));
      creatables.push_back(wxT("K7"));
      creatables.push_back(wxT("K8"));
      creatables.push_back(wxT("K9"));
      creatables.push_back(wxT("K10"));
      creatables.push_back(wxT("K11"));
      creatables.push_back(wxT("K12"));
      creatables.push_back(wxT("K13"));
      creatables.push_back(wxT("K14"));
      creatables.push_back(wxT("K15"));
      creatables.push_back(wxT("K16"));
      
      creatables.push_back(wxT("ThrustDirection1"));
      creatables.push_back(wxT("ThrustDirection2"));
      creatables.push_back(wxT("ThrustDirection3"));
   }
}


//------------------------------------------------------------------------------
//  ParameterFactory(StringArray createList)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class ParameterFactory
 * (constructor).
 *
 * @param <createList> initial list of creatable objects
 *
 */
//------------------------------------------------------------------------------
ParameterFactory::ParameterFactory(StringArray createList) :
   Factory(createList, Gmat::PARAMETER)
{
}


//------------------------------------------------------------------------------
//  ParameterFactory(const ParameterFactory &fact)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class ParameterFactory
 * (copy constructor).
 *
 * @param <fact> the factory object to copy to wxT("this") factory.
 */
//------------------------------------------------------------------------------
ParameterFactory::ParameterFactory(const ParameterFactory &fact) :
   Factory(fact)
{
}


//------------------------------------------------------------------------------
//  ParameterFactory& operator= (const ParameterFactory &fact)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the ParameterFactory base class.
 *
 * @param <fact> the ParameterFactory object whose data to assign to wxT("this")
 *  factory.
 *
 * @return wxT("this") ParameterFactory with data of input factory fact.
 */
//------------------------------------------------------------------------------
ParameterFactory& ParameterFactory::operator= (const ParameterFactory &fact)
{
   Factory::operator=(fact);
   return *this;
}


//------------------------------------------------------------------------------
// ~ParameterFactory()
//------------------------------------------------------------------------------
/**
 * Destructor for the ParameterFactory base class.
 */
//------------------------------------------------------------------------------
ParameterFactory::~ParameterFactory()
{
   // deletes handled by Factory destructor
}

