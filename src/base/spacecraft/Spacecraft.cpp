//$Id: Spacecraft.cpp 9766 2011-08-15 20:38:01Z wendys-dev $
//------------------------------------------------------------------------------
//                                  Spacecraft
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
// Author:  Joey Gurganus, Reworked by D. Conway
// Created: 2003/10/22
//
/**
 * Implements the Spacecraft base class.
 *    Spacecraft internal state is in EarthMJ2000Eq Cartesian.
 *    If state output is in Keplerian, the anomaly type is True Anomaly.
 *    Internal time is in A1ModJulian.
 *
 *    It converts to proper format using epochType, stateType, anomalyType
 *    before generating scripts from the internal data.
 */
//------------------------------------------------------------------------------

#include <sstream>
#include "Spacecraft.hpp"
#include "MessageInterface.hpp"
#include "SpaceObjectException.hpp"
#include "StringUtil.hpp"
#include "TimeTypes.hpp"
#include "CSFixed.hpp"               // for default attitude creation
#include "FileManager.hpp"           // for GetFullPathname()
#ifdef __USE_SPICE__
#include "SpiceAttitude.hpp"         // for SpiceAttitude - to set object name and ID
#endif

// Do we want to write anomaly type?
//#define __WRITE_ANOMALY_TYPE__


//#define DEBUG_SPACECRAFT
//#define DEBUG_SPACECRAFT_SET
//#define DEBUG_SPACECRAFT_SET_ELEMENT
//#define DEBUG_LOOK_UP_LABEL
//#define DEBUG_SPACECRAFT_CS
//#define DEBUG_RENAME
//#define DEBUG_DATE_FORMAT
//#define DEBUG_STATE_INTERFACE
//#define DEBUG_SC_ATTITUDE
//#define DEBUG_OBJ_CLONE
//#define DEBUG_GET_REAL
//#define DEBUG_GET_STATE
//#define DEBUG_SC_PARAMETER_TEXT
//#define DEBUG_SC_REF_OBJECT
//#define DEBUG_SC_EPOCHSTR
//#define DEBUG_SC_SET_STRING
//#define DEBUG_WRITE_PARAMETERS
//#define DEBUG_OWNED_OBJECT_STRINGS
//#define DEBUG_SC_OWNED_OBJECT
//#define DEBUG_MASS_FLOW
//#define DEBUG_SPICE_KERNELS
//#define DEBUG_HARDWARE


#ifdef DEBUG_SPACECRAFT
#include <iostream>
#include <sstream>
#endif

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

const int NO_MODEL = -1;

// Spacecraft parameter types
const Gmat::ParameterType
Spacecraft::PARAMETER_TYPE[SpacecraftParamCount - SpaceObjectParamCount] =
   {
      Gmat::STRING_TYPE,      // Epoch
      Gmat::REAL_TYPE,        // Element1
      Gmat::REAL_TYPE,        // Element2
      Gmat::REAL_TYPE,        // Element3
      Gmat::REAL_TYPE,        // Element4
      Gmat::REAL_TYPE,        // Element5
      Gmat::REAL_TYPE,        // Element6
      Gmat::STRING_TYPE,      // Element1Units
      Gmat::STRING_TYPE,      // Element2Units
      Gmat::STRING_TYPE,      // Element3Units
      Gmat::STRING_TYPE,      // Element4Units
      Gmat::STRING_TYPE,      // Element5Units
      Gmat::STRING_TYPE,      // Element6Units
      Gmat::ENUMERATION_TYPE, // StateType
      Gmat::ENUMERATION_TYPE, // DisplayStateType
      Gmat::ENUMERATION_TYPE, // AnomalyType
      Gmat::OBJECT_TYPE,      // CoordinateSystem
      Gmat::REAL_TYPE,        // DryMass
      Gmat::ENUMERATION_TYPE, // DateFormat
      Gmat::REAL_TYPE,        // Cd
      Gmat::REAL_TYPE,        // Cr
      Gmat::REAL_TYPE,        // DragArea
      Gmat::REAL_TYPE,        // SRPArea
      Gmat::OBJECTARRAY_TYPE, // Tanks
      Gmat::OBJECTARRAY_TYPE, // Thrusters
      Gmat::REAL_TYPE,        // TotalMass
      Gmat::STRING_TYPE,      // Id
      Gmat::OBJECT_TYPE,      // Attitude
//      Gmat::STRINGARRAY_TYPE, // OrbitSpiceKernelName
//      Gmat::STRINGARRAY_TYPE, // AttitudeSpiceKernelName
//      Gmat::STRINGARRAY_TYPE, // SCClockSpiceKernelName
//      Gmat::STRINGARRAY_TYPE, // FrameSpiceKernelName
      Gmat::RMATRIX_TYPE,     // OrbitSTM
      Gmat::RMATRIX_TYPE,     // OrbitAMatrix
      Gmat::STRING_TYPE,      // UTCGregorian
      Gmat::REAL_TYPE,        // CartesianX
      Gmat::REAL_TYPE,        // CartesianY
      Gmat::REAL_TYPE,        // CartesianZ
      Gmat::REAL_TYPE,        // CartesianVX
      Gmat::REAL_TYPE,        // CartesianVY
      Gmat::REAL_TYPE,        // CartesianVZ
      Gmat::REAL_TYPE,        // Mass Flow
      Gmat::OBJECTARRAY_TYPE, // AddHardware    // made changes by Tuan Nguyen
      Gmat::STRING_TYPE,      // Model File
      Gmat::REAL_TYPE,        // Model Offset X
      Gmat::REAL_TYPE,        // Model Offset Y
      Gmat::REAL_TYPE,        // Model Offset Z
      Gmat::REAL_TYPE,        // Model Rotation X
      Gmat::REAL_TYPE,        // Model Rotation Y
      Gmat::REAL_TYPE,        // Model Rotation Z
      Gmat::REAL_TYPE,        // Model Scale Factor
   };

const wxString
Spacecraft::PARAMETER_LABEL[SpacecraftParamCount - SpaceObjectParamCount] =
   {
      wxT("Epoch"),
      wxT("Element1"),
      wxT("Element2"),
      wxT("Element3"),
      wxT("Element4"),
      wxT("Element5"),
      wxT("Element6"),
      wxT("Element1Units"),
      wxT("Element2Units"),
      wxT("Element3Units"),
      wxT("Element4Units"),
      wxT("Element5Units"),
      wxT("Element6Units"),
      wxT("StateType"),
      wxT("DisplayStateType"),
      wxT("AnomalyType"),
      wxT("CoordinateSystem"),
      wxT("DryMass"),
      wxT("DateFormat"),
      wxT("Cd"),
      wxT("Cr"),
      wxT("DragArea"),
      wxT("SRPArea"),
      wxT("Tanks"),
      wxT("Thrusters"),
      wxT("TotalMass"),
      wxT("Id"),
      wxT("Attitude"),
//      wxT("OrbitSpiceKernelName"),
//      wxT("AttitudeSpiceKernelName"),
//      wxT("SCClockSpiceKernelName"),
//      wxT("FrameSpiceKernelName"),
      wxT("OrbitSTM"),
      wxT("OrbitAMatrix"),
      wxT("UTCGregorian"),
      wxT("CartesianX"),
      wxT("CartesianY"),
      wxT("CartesianZ"),
      wxT("CartesianVX"),
      wxT("CartesianVY"),
      wxT("CartesianVZ"),
      wxT("MassFlow"),
      wxT("AddHardware"),                            // made changes by Tuan Nguyen
      wxT("ModelFile"),
      wxT("ModelOffsetX"),
      wxT("ModelOffsetY"),
      wxT("ModelOffsetZ"),
      wxT("ModelRotationX"),
      wxT("ModelRotationY"),
      wxT("ModelRotationZ"),
      wxT("ModelScale"),
};

const wxString Spacecraft::MULT_REP_STRINGS[EndMultipleReps - CART_X] =
{
   // Cartesian
   wxT("X"),
   wxT("Y"),
   wxT("Z"),
   wxT("VX"),
   wxT("VY"),
   wxT("VZ"),
   // Keplerian
   wxT("SMA"),
   wxT("ECC"),
   wxT("INC"),
   wxT("RAAN"),
   wxT("AOP"),
   wxT("TA"),
   wxT("EA"),
   wxT("MA"),
   wxT("HA"),
   // Modified Keplerian
   wxT("RadPer"),
   wxT("RadApo"),
   // Speherical AZFPA
   wxT("RMAG"),
   wxT("RA"),
   wxT("DEC"),
   wxT("VMAG"),
   wxT("AZI"),
   wxT("FPA"),
   // Spherical RADEC
   wxT("RAV"),
   wxT("DECV"),
   // Equinoctial
//   wxT("PEY"),
//   wxT("PEX"),
//   wxT("PNY"),
//   wxT("PNX"),
   wxT("EquinoctialH"),
   wxT("EquinoctialK"),
   wxT("EquinoctialP"),
   wxT("EquinoctialQ"),
   wxT("MLONG"),
};

const Integer Spacecraft::ATTITUDE_ID_OFFSET = 20000;

//-------------------------------------
// public methods
//-------------------------------------

//---------------------------------------------------------------------------
//  Spacecraft(const wxString &name)
//---------------------------------------------------------------------------
/**
 * Creates constructors with parameters.
 *
 * @param <name> Optional name for the object.  Defaults to wxT("").
 *
 */
//---------------------------------------------------------------------------
Spacecraft::Spacecraft(const wxString &name, const wxString &typeStr) :
   SpaceObject          (Gmat::SPACECRAFT, typeStr, name),
   modelFile            (wxT("")),
   modelID              (NO_MODEL),
   dryMass              (850.0),
   coeffDrag            (2.2),
   dragArea             (15.0),
   srpArea              (1.0),
   reflectCoeff         (1.8),
   epochSystem          (wxT("TAI")),
   epochFormat          (wxT("ModJulian")),
   epochType            (wxT("TAIModJulian")),  // Should be A1ModJulian?
   stateType            (wxT("Cartesian")),
   displayStateType     (wxT("Cartesian")),
   anomalyType          (wxT("TA")),
   modelOffsetX         (0),
   modelOffsetY         (0),
   modelOffsetZ         (0),
   modelRotationX       (0),
   modelRotationY       (0),
   modelRotationZ       (0),
   modelScale           (1),
   solarSystem          (NULL),
   internalCoordSystem  (NULL),
   coordinateSystem     (NULL),
   coordSysName         (wxT("EarthMJ2000Eq")),
   spacecraftId         (wxT("SatId")),
   attitude             (NULL),
   totalMass            (850.0),
   initialDisplay       (false),
   csSet                (false),
   isThrusterSettingMode(false),
   orbitSTM             (6,6),
   orbitAMatrix         (6,6),
   includeCartesianState(0)
{
   #ifdef DEBUG_SPACECRAFT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::Spacecraft() <%p>'%s' entered\n"), this, name.c_str());
   #endif

   objectTypes.push_back(Gmat::SPACECRAFT);
   objectTypeNames.push_back(wxT("Spacecraft"));
   ownedObjectCount = 0;

   wxString ss(wxT(""));
   ss << GmatTimeConstants::MJD_OF_J2000;
   scEpochStr = ss;

   Real a1mjd = -999.999;
   wxString outStr;
   Real taimjd = GmatTimeConstants::MJD_OF_J2000;

   // Internal epoch is in A1ModJulian, so convert
   TimeConverterUtil::Convert(wxT("TAIModJulian"), taimjd, wxT(""),
                              wxT("A1ModJulian"), a1mjd, outStr);

   //state.SetEpoch(GmatTimeConstants::MJD_OF_J2000);
   state.SetEpoch(a1mjd);

   state[0] = 7100.0;
   state[1] = 0.0;
   state[2] = 1300.0;
   state[3] = 0.0;
   state[4] = 7.35;
   state[5] = 1.0;

   stateElementLabel.push_back(wxT("X"));
   stateElementLabel.push_back(wxT("Y"));
   stateElementLabel.push_back(wxT("Z"));
   stateElementLabel.push_back(wxT("VX"));
   stateElementLabel.push_back(wxT("VY"));
   stateElementLabel.push_back(wxT("VZ"));

   stateElementUnits.push_back(wxT("km"));
   stateElementUnits.push_back(wxT("km"));
   stateElementUnits.push_back(wxT("km"));
   stateElementUnits.push_back(wxT("km/s"));
   stateElementUnits.push_back(wxT("km/s"));
   stateElementUnits.push_back(wxT("km/s"));

   representations.push_back(wxT("Cartesian"));
   representations.push_back(wxT("Keplerian"));
   representations.push_back(wxT("ModifiedKeplerian"));
   representations.push_back(wxT("SphericalAZFPA"));
   representations.push_back(wxT("SphericalRADEC"));
   representations.push_back(wxT("Equinoctial"));

   parameterCount = SpacecraftParamCount;

   // Create a default unnamed attitude (LOJ: 2009.03.10)
   attitude = new CSFixed(wxT(""));
   attitude->SetEpoch(state.GetEpoch());
   ownedObjectCount++;

   #ifdef DEBUG_SC_OWNED_OBJECT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::Spacecraft() <%p>'%s' ownedObjectCount=%d\n"),
       this, GetName().c_str(), ownedObjectCount);
   #endif

   #ifdef DEBUG_MEMORY
   MemoryTracker::Instance()->Add
      (attitude, wxT("new attitude"), wxT("Spacecraft constructor()"),
       wxT("attitude = new CSFixed(")wxT(")"), this);
   #endif

   BuildElementLabelMap();

   // Initialize the STM to the identity matrix
   orbitSTM(0,0) = orbitSTM(1,1) = orbitSTM(2,2) =
   orbitSTM(3,3) = orbitSTM(4,4) = orbitSTM(5,5) = 1.0;

   orbitAMatrix(0,0) = orbitAMatrix(1,1) = orbitAMatrix(2,2) =
   orbitAMatrix(3,3) = orbitAMatrix(4,4) = orbitAMatrix(5,5) = 1.0;
   // Initialize the covariance matrix
   covariance.AddCovarianceElement(wxT("CartesianState"), this);
   covariance.ConstructLHS();
   
   covariance(0,0) = covariance(1,1) = covariance(2,2) = 1.0e10;
   covariance(3,3) = covariance(4,4) = covariance(5,5) = 1.0e6;
   
   // Load default model file
   modelFile = FileManager::Instance()->GetFullPathname(wxT("SPACECRAFT_MODEL_FILE"));
   modelScale = 3.0;
   modelID = NO_MODEL;
   
   #ifdef DEBUG_SPACECRAFT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::Spacecraft() <%p>'%s' exiting\n"), this, name.c_str());
   #endif
}


//---------------------------------------------------------------------------
//  ~Spacecraft()
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
//---------------------------------------------------------------------------
Spacecraft::~Spacecraft()
{
   #ifdef DEBUG_SPACECRAFT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::~Spacecraft() <%p>'%s' entered, attitude=<%p>\n"),
       this, GetName().c_str(), attitude);
   #endif

   // Delete the attached hardware (it was set as clones in the ObjectInitializer)
   // It is not anymore setting the clone (LOJ: 2009.07.24)
   //@see ObjectInitializer::BuildAssociations()
   DeleteOwnedObjects(true, true, true, true);

   #ifdef DEBUG_SPACECRAFT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::~Spacecraft() <%p>'%s' exiting\n"), this, GetName().c_str());
   #endif
}


//---------------------------------------------------------------------------
//  Spacecraft(const Spacecraft &a)
//---------------------------------------------------------------------------
/**
 * Copy Constructor for base Spacecraft structures.
 *
 * @param <a> The original that is being copied.
 *
 * @notes We need to copy internal and display coordinate systems to work
 * properly in the mission sequence for object copy.
 */
//---------------------------------------------------------------------------
Spacecraft::Spacecraft(const Spacecraft &a) :
   SpaceObject          (a),
   modelFile            (a.modelFile),
   modelID              (a.modelID),
   scEpochStr           (a.scEpochStr),
   dryMass              (a.dryMass),
   coeffDrag            (a.coeffDrag),
   dragArea             (a.dragArea),
   srpArea              (a.srpArea),
   reflectCoeff         (a.reflectCoeff),
   epochSystem          (a.epochSystem),
   epochFormat          (a.epochFormat),
   epochType            (a.epochType),
   stateType            (a.stateType),
   displayStateType     (a.displayStateType),
   anomalyType          (a.anomalyType),
   modelOffsetX         (a.modelOffsetX),
   modelOffsetY         (a.modelOffsetY),
   modelOffsetZ         (a.modelOffsetZ),
   modelRotationX       (a.modelRotationX),
   modelRotationY       (a.modelRotationY),
   modelRotationZ       (a.modelRotationZ),
   modelScale           (a.modelScale),
   solarSystem          (a.solarSystem),           // need to copy
   internalCoordSystem  (a.internalCoordSystem),   // need to copy
   coordinateSystem     (a.coordinateSystem),      // need to copy
   coordSysName         (a.coordSysName),
   coordSysMap          (a.coordSysMap),
   spacecraftId         (a.spacecraftId),
//   orbitSpiceKernelNames(a.orbitSpiceKernelNames),
   stateConverter       (a.stateConverter),
   coordConverter       (a.coordConverter),
   totalMass            (a.totalMass),
   initialDisplay       (false),
   csSet                (a.csSet),
   isThrusterSettingMode(a.isThrusterSettingMode),
   orbitSTM             (a.orbitSTM),
   orbitAMatrix         (a.orbitAMatrix),
   includeCartesianState(a.includeCartesianState)
{
   #ifdef DEBUG_SPACECRAFT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::Spacecraft(copy) <%p>'%s' entered\n"), this, GetName().c_str());
   #endif

   objectTypes.push_back(Gmat::SPACECRAFT);
   objectTypeNames.push_back(wxT("Spacecraft"));
   parameterCount = a.parameterCount;
   ownedObjectCount = 0;

   state.SetEpoch(a.state.GetEpoch());
   state[0] = a.state[0];
   state[1] = a.state[1];
   state[2] = a.state[2];
   state[3] = a.state[3];
   state[4] = a.state[4];
   state[5] = a.state[5];
   trueAnomaly = a.trueAnomaly;

   stateElementLabel = a.stateElementLabel;
   stateElementUnits = a.stateElementUnits;
   representations   = a.representations;
   tankNames         = a.tankNames;
   thrusterNames     = a.thrusterNames;

   hardwareNames     = a.hardwareNames; // made changes by Tuan Nguyen
//   hardwareList      = a.hardwareList; // made changes by Tuan Nguyen

   // set cloned hardware
   CloneOwnedObjects(a.attitude, a.tanks, a.thrusters);

   BuildElementLabelMap();

   #ifdef DEBUG_SPACECRAFT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::Spacecraft(copy) <%p>'%s' exiting\n"), this, GetName().c_str());
   #endif
}


//---------------------------------------------------------------------------
//  Spacecraft& operator=(const Spacecraft &a)
//---------------------------------------------------------------------------
/**
 * Assignment operator for Spacecraft structures.
 *
 * @note: Coordinate systems are not copied here.
 *
 * @param <a> The original that is being copied.
 *
 * @return Reference to this object
 */
//---------------------------------------------------------------------------
Spacecraft& Spacecraft::operator=(const Spacecraft &a)
{
   // Don't do anything if copying self
   if (&a == this)
      return *this;

   #ifdef DEBUG_SPACECRAFT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::Spacecraft(=) <%p>'%s' entered\n"), this, GetName().c_str());
   #endif

   SpaceObject::operator=(a);

   ownedObjectCount     = a.ownedObjectCount;

   scEpochStr           = a.scEpochStr;
   dryMass              = a.dryMass;
   coeffDrag            = a.coeffDrag;
   dragArea             = a.dragArea;
   srpArea              = a.srpArea;
   reflectCoeff         = a.reflectCoeff;
   epochSystem          = a.epochSystem;
   epochFormat          = a.epochFormat;
   epochType            = a.epochType;
   stateType            = a.stateType;
   displayStateType     = a.displayStateType;
   anomalyType          = a.anomalyType;
   coordSysName         = a.coordSysName;
   coordSysMap          = a.coordSysMap;
   spacecraftId         = a.spacecraftId;
   solarSystem          = a.solarSystem;         // need to copy
   internalCoordSystem  = a.internalCoordSystem; // need to copy
   coordinateSystem     = a.coordinateSystem;    // need to copy
   stateConverter       = a.stateConverter;
   coordConverter       = a.coordConverter;
   totalMass            = a.totalMass;
   initialDisplay       = false;
   csSet                = a.csSet;
   isThrusterSettingMode= a.isThrusterSettingMode;
   trueAnomaly          = a.trueAnomaly;
   modelID              = a.modelID;
   modelFile            = a.modelFile;
   modelOffsetX         = a.modelOffsetX;
   modelOffsetY         = a.modelOffsetY;
   modelOffsetZ         = a.modelOffsetZ;
   modelRotationX       = a.modelRotationX;
   modelRotationY       = a.modelRotationY;
   modelRotationZ       = a.modelRotationZ;
   modelScale           = a.modelScale;

   #ifdef DEBUG_SPACECRAFT
   MessageInterface::ShowMessage
      (wxT("Anomaly has type %s, copied from %s\n"), trueAnomaly.GetTypeString().c_str(),
       a.trueAnomaly.GetTypeString().c_str());
   #endif

   state.SetEpoch(a.state.GetEpoch());
   state[0] = a.state[0];
   state[1] = a.state[1];
   state[2] = a.state[2];
   state[3] = a.state[3];
   state[4] = a.state[4];
   state[5] = a.state[5];

   stateElementLabel = a.stateElementLabel;
   stateElementUnits = a.stateElementUnits;
   representations   = a.representations;
   tankNames         = a.tankNames;
   thrusterNames     = a.thrusterNames;

   hardwareNames     = a.hardwareNames; // made changes by Tuan Nguyen
//   hardwareList      = a.hardwareList; // made changes by Tuan Nguyen

   // delete attached hardware, such as tanks and thrusters
   #ifdef DEBUG_SPACECRAFT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::Spacecraft(=) about to delete all owned objects\n"));
      #endif
   DeleteOwnedObjects(true, true, true, true);

   // then cloned owned objects
   #ifdef DEBUG_SPACECRAFT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::Spacecraft(=) about to clone all owned objects\n"));
      #endif
   CloneOwnedObjects(a.attitude, a.tanks, a.thrusters);

   BuildElementLabelMap();

   orbitSTM = a.orbitSTM;
   orbitAMatrix = a.orbitAMatrix;
//   orbitSpiceKernelNames = a.orbitSpiceKernelNames;
   includeCartesianState = a.includeCartesianState;


   #ifdef DEBUG_SPACECRAFT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::Spacecraft(=) <%p>'%s' exiting\n"), this, GetName().c_str());
   #endif

   return *this;
}


//---------------------------------------------------------------------------
// virtual void SetSolarSystem(SolarSystem *ss)
//---------------------------------------------------------------------------
void Spacecraft::SetSolarSystem(SolarSystem *ss)
{
   #ifdef DEBUG_SET_SOLAR_SYSTEM
   MessageInterface::ShowMessage
      (wxT("Spacecraft::SetSolarSystem() this=<%p>'%s' entered, ss=<%p>\n"), this,
       GetName().c_str(), ss);
   #endif
   solarSystem = ss;
}


//---------------------------------------------------------------------------
// void SetInternalCoordSystem(CoordinateSystem *cs)
//---------------------------------------------------------------------------
void Spacecraft::SetInternalCoordSystem(CoordinateSystem *cs)
{
   #ifdef DEBUG_SPACECRAFT_CS
   MessageInterface::ShowMessage
      (wxT("Spacecraft::SetInternalCoordSystem() this=<%p> '%s', setting %s <%p>\n"),
       this, GetName().c_str(), cs->GetName().c_str(), cs);
   #endif

   if (internalCoordSystem != cs)
   {
      internalCoordSystem = cs;
      if (coordinateSystem == NULL)
         coordinateSystem = cs;
   }
}


//---------------------------------------------------------------------------
// CoordinateSystem* GetInternalCoordSystem()
//---------------------------------------------------------------------------
CoordinateSystem* Spacecraft::GetInternalCoordSystem()
{
   return internalCoordSystem;
}


//---------------------------------------------------------------------------
//  void SetState(const Rvector6 &cartState)
//---------------------------------------------------------------------------
/**
 * Set the elements to Cartesian states.
 *
 * @param <cartState> cartesian state
 *
 */
//---------------------------------------------------------------------------
void Spacecraft::SetState(const Rvector6 &cartState)
{
   #ifdef DEBUG_SPACECRAFT_SET
      MessageInterface::ShowMessage(wxT("Spacecraft::SetState(Rvector6)\n"));
      MessageInterface::ShowMessage(
      wxT("Spacecraft::SetState(Rvector6) cartesianState=%s\n"),
       cartState.ToString().c_str());
   #endif

   SetState(cartState[0], cartState[1], cartState[2],
            cartState[3], cartState[4], cartState[5]);
}


//---------------------------------------------------------------------------
//  void SetState(const wxString elementType, Real *instate)
//---------------------------------------------------------------------------
/**
 * Set the elements to Cartesian states.
 *
 * @param <elementType>  Element Type
 * @param <instate>      element states
 *
 */
//---------------------------------------------------------------------------
void Spacecraft::SetState(const wxString &elementType, Real *instate)
{
   #ifdef DEBUG_SPACECRAFT_SET
      MessageInterface::ShowMessage(
         wxT("Spacecraft::SetState() elementType = %s, instate =\n")
         wxT("   %.9lf, %.9lf, %.9lf, %.14lf, %.14lf, %.14lf\n"),
         elementType.c_str(), instate[0], instate[1], instate[2], instate[3],
         instate[4], instate[5]);
   #endif

   Rvector6 newState;

   newState.Set(instate[0],instate[1],instate[2],
                instate[3],instate[4],instate[5]);

   if (elementType != wxT("Cartesian"))
   {
      stateType = wxT("Cartesian");  // why not use SetStateFromRepresentation here?? wcs
      newState = stateConverter.Convert(instate, elementType,
         stateType, trueAnomaly);
   }

   SetState(newState.Get(0),newState.Get(1),newState.Get(2),
            newState.Get(3),newState.Get(4),newState.Get(5));
}


//------------------------------------------------------------------------------
//  void SetState(const Real s1, const Real s2, const Real s3,
//                const Real s4, const Real s5, const Real s6)
//------------------------------------------------------------------------------
/**
 * Set the elements of a Cartesian state.
 *
 * @param <s1>  First element
 * @param <s2>  Second element
 * @param <s3>  Third element
 * @param <s4>  Fourth element
 * @param <s5>  Fifth element
 * @param <s6>  Sixth element
 */
//------------------------------------------------------------------------------
void Spacecraft::SetState(const Real s1, const Real s2, const Real s3,
                          const Real s4, const Real s5, const Real s6)
{
    state[0] = s1;
    state[1] = s2;
    state[2] = s3;
    state[3] = s4;
    state[4] = s5;
    state[5] = s6;
}


//------------------------------------------------------------------------------
//  GmatState& GetState()
//------------------------------------------------------------------------------
/**
 * wxT("Unhide") the SpaceObject method.
 *
 * @return the core GmatState.
 */
//------------------------------------------------------------------------------
GmatState& Spacecraft::GetState()
{
   #ifdef DEBUG_GET_STATE
   Rvector6 stateTmp;
   stateTmp.Set(SpaceObject::GetState().GetState());
   MessageInterface::ShowMessage
      (wxT("Spacecraft::GetState() '%s' returning\n   %s\n"), GetName().c_str(),
       stateTmp.ToString().c_str());
   #endif
   return SpaceObject::GetState();
}

//------------------------------------------------------------------------------
//  Rvector6 GetState(wxString &rep)
//------------------------------------------------------------------------------
/**
 * Get the converted Cartesian states from states in different coordinate type.
 *
 * @return converted Cartesian states
 */
//------------------------------------------------------------------------------
Rvector6 Spacecraft::GetState(wxString rep)
{
   #ifdef DEBUG_STATE_INTERFACE
      MessageInterface::ShowMessage(wxT("Getting state in representation %s"),
         rep.c_str());
   #endif
   rvState = GetStateInRepresentation(rep);
   return rvState;
}


//------------------------------------------------------------------------------
//  Rvector6 GetState(wxString &rep)
//------------------------------------------------------------------------------
/**
 * Get the converted Cartesian states from states in different coordinate type.
 *
 * @return converted Cartesian states
 */
//------------------------------------------------------------------------------
Rvector6 Spacecraft::GetState(Integer rep)
{
   rvState = GetStateInRepresentation(rep);
   return rvState;
}


//------------------------------------------------------------------------------
//  Rvector6 GetCartesianState()
//------------------------------------------------------------------------------
/**
 * Get the converted Cartesian states from states in different coordinate type.
 *
 * @return converted Cartesian states
 */
//------------------------------------------------------------------------------
Rvector6 Spacecraft::GetCartesianState()
{
//   Real *tempState = state.GetState();
//
//   for (int i=0; i<6; i++)
//      rvState[i] = tempState[i];
//
   MessageInterface::ShowMessage(wxT("GetCartesianState() is obsolete; ")
      wxT("use GetState(\"Cartesian\") or GetState(%d) instead.\n"), CARTESIAN_ID);
   return GetState(wxT("Cartesian"));//rvState;
}

//------------------------------------------------------------------------------
//  Rvector6 GetKeplerianState()
//------------------------------------------------------------------------------
/**
 * Get the converted Keplerian states from states in different coordinate type.
 *
 * @return converted Keplerain states
 */
//------------------------------------------------------------------------------
Rvector6 Spacecraft::GetKeplerianState()
{
//   rvState = stateConverter.Convert(state.GetState(), stateType,
//                wxT("Keplerian"),trueAnomaly);
//
//   return rvState;
   MessageInterface::ShowMessage(wxT("GetKeplerianState() is obsolete; ")
      wxT("use GetState(\"Keplerian\") or GetState(%d) instead.\n"), KEPLERIAN_ID);
   return GetState(wxT("Keplerian"));
}

//------------------------------------------------------------------------------
//  Rvector6 GetModifiedKeplerianState()
//------------------------------------------------------------------------------
/**
 * Get the converted Modified Keplerian states from states in different
 * coordinate type.
 *
 * @return converted Modified Keplerain states
 */
//------------------------------------------------------------------------------
Rvector6 Spacecraft::GetModifiedKeplerianState()
{
//   rvState = stateConverter.Convert(state.GetState(),stateType,
//                wxT("ModifiedKeplerian"),trueAnomaly);
//   return (rvState);
   MessageInterface::ShowMessage(wxT("GetModifiedKeplerianState() is obsolete; ")
      wxT("use GetState(\"ModifiedKeplerian\") or GetState(%d) instead.\n"),
      MODIFIED_KEPLERIAN_ID);
   return GetState(wxT("ModifiedKeplerian"));
}


//------------------------------------------------------------------------------
// Anomaly GetAnomaly() const
//------------------------------------------------------------------------------
Anomaly Spacecraft::GetAnomaly() const
{
   return trueAnomaly;
}

//------------------------------------------------------------------------------
// virtual bool HasAttitude()
//------------------------------------------------------------------------------
bool Spacecraft::HasAttitude()
{
   return true;
}

//------------------------------------------------------------------------------
// const Rmatrix33& GetAttitude(Real a1mjdTime) const
//------------------------------------------------------------------------------
const Rmatrix33& Spacecraft::GetAttitude(Real a1mjdTime)
{
   #ifdef DEBUG_SC_ATTITUDE
      MessageInterface::ShowMessage(wxT("Entering SC::GetAttitude ...\n"));
   #endif
   if (attitude) return attitude->GetCosineMatrix(a1mjdTime);
   else
   {
      wxString errmsg =
         wxT("Error attempting to retrieve Attitude Matrix for spacecraft \"");
      errmsg += instanceName + wxT("\", for which no attitude has been set.\n");
      throw SpaceObjectException(errmsg);
   }
}


//------------------------------------------------------------------------------
// const Rvector3& GetAngularVelocity(Real a1mjdTime) const
//------------------------------------------------------------------------------
const Rvector3&  Spacecraft::GetAngularVelocity(Real a1mjdTime) const
{
   if (attitude) return attitude->GetAngularVelocity(a1mjdTime);
   else
   {
      wxString errmsg =
         wxT("Error attempting to retrieve Angular Velocity for spacecraft \"");
      errmsg += instanceName + wxT("\", for which no attitude has been set.\n");
      throw SpaceObjectException(errmsg);
   }
}

//------------------------------------------------------------------------------
// const UnsignedIntArray& GetEulerAngleSequence() const
//------------------------------------------------------------------------------
const UnsignedIntArray& Spacecraft::GetEulerAngleSequence() const
{
   if (attitude)
      return attitude->GetUnsignedIntArrayParameter(wxT("EulerSequenceArray"));
   else
   {
      wxString errmsg =
         wxT("Error attempting to retrieve Euler Angle Sequence for spacecraft \"");
      errmsg += instanceName + wxT("\", for which no attitude has been set.\n");
      throw SpaceObjectException(errmsg);
   }
}

//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the Spacecraft.
 *
 * @return clone of the Spacecraft.
 *
 */
//------------------------------------------------------------------------------
GmatBase* Spacecraft::Clone() const
{
   Spacecraft *clone = new Spacecraft(*this);

   #ifdef DEBUG_SPACECRAFT
      MessageInterface::ShowMessage
         (wxT("Spacecraft::Clone() cloned <%p>'%s' to <%p>'%s'\n"), this,
         instanceName.c_str(), clone, clone->GetName().c_str());
   #endif
   #ifdef DEBUG_OBJ_CLONE
      Spacecraft *tmp = (const_cast<Spacecraft*>(this));
      GmatState vec6Orig = tmp->GetState();
      GmatState vec6     = clone->GetState();
      MessageInterface::ShowMessage(wxT("ORIGINAL spacecraft has state of %12.10f %12.10f %12.10f %12.10f %12.10f %12.10f\n"),
            vec6Orig[0], vec6Orig[1], vec6Orig[2], vec6Orig[3], vec6Orig[4], vec6Orig[5]);
      MessageInterface::ShowMessage(wxT("Cloned spacecraft has state of %12.10f %12.10f %12.10f %12.10f %12.10f %12.10f\n"),
            vec6[0], vec6[1], vec6[2], vec6[3], vec6[4], vec6[5]);
   #endif

   return (clone);
}


//---------------------------------------------------------------------------
//  void Copy(const GmatBase* orig)
//---------------------------------------------------------------------------
/**
 * Sets this object to match another one.
 *
 * @param orig The original that is being copied.
 */
//---------------------------------------------------------------------------
void Spacecraft::Copy(const GmatBase* orig)
{
   operator=(*((Spacecraft *)(orig)));
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
bool Spacecraft::RenameRefObject(const Gmat::ObjectType type,
                                 const wxString &oldName,
                                 const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("Spacecraft::RenameRefObject() type=%s, oldName=%s, newName=%s\n"),
       GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif
   
   if (type != Gmat::HARDWARE && type != Gmat::COORDINATE_SYSTEM)
      return true;
   
   if (type == Gmat::COORDINATE_SYSTEM)
   {
      if (coordSysName == oldName)
         coordSysName = newName;
   }
   
   // made changes by Tuan Nguyen
   if (type == Gmat::HARDWARE)
   {
      for (UnsignedInt i=0; i<hardwareNames.size(); i++)
      {
         if (hardwareNames[i] == oldName)
         {
            hardwareNames[i] = newName;
            break;
         }
      }
      
      for (UnsignedInt i=0; i<thrusterNames.size(); i++)
      {
         if (thrusterNames[i] == oldName)
         {
            thrusterNames[i] = newName;
            break;
         }
      }
      
      for (UnsignedInt i=0; i<tankNames.size(); i++)
      {
         if (tankNames[i] == oldName)
         {
            tankNames[i] = newName;
            break;
         }
      }
   }
   
   return true;
}


//------------------------------------------------------------------------------
// wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
/**
 * This method returns a name of the referenced objects.
 *
 * @return a name of objects of the requested type.
 */
//------------------------------------------------------------------------------
wxString Spacecraft::GetRefObjectName(const Gmat::ObjectType type) const
{
   if (type == Gmat::COORDINATE_SYSTEM)
   {
      return coordSysName;
   }
   if (type == Gmat::ATTITUDE)   return wxT("");   // Attitude objects don't have names
   return SpaceObject::GetRefObjectName(type);
}


//------------------------------------------------------------------------------
// virtual bool HasRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
bool Spacecraft::HasRefObjectTypeArray()
{
   return true;
}


//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by this class.
 *
 * @return the list of object types.
 *
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& Spacecraft::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::COORDINATE_SYSTEM);
   refObjectTypes.push_back(Gmat::HARDWARE);
   // Now Attitude is local object it will be created all the time (LOJ:2009.09.24)
   //refObjectTypes.push_back(Gmat::ATTITUDE);
   return refObjectTypes;
}


//------------------------------------------------------------------------------
//  const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * This method returns an array with the names of the referenced objects.
 *
 * @return a vector with the names of objects of the requested type.
 */
//------------------------------------------------------------------------------
const StringArray&
Spacecraft::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   #ifdef DEBUG_SC_REF_OBJECT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::GetRefObjectNameArray() <%p>'%s' entered, type='%s'\n"),
       this, GetName().c_str(), GmatBase::GetObjectTypeString(type).c_str());
   #endif
   static StringArray fullList;  // Maintain scope if the full list is requested
   fullList.clear();

   // If type is UNKNOWN_OBJECT, add only coordinate system and attitude.
   // Other objects are handled separately in the ObjectInitializer
   if (type == Gmat::UNKNOWN_OBJECT)
   {
      // Put in the SpaceObject origin
      fullList.push_back(originName);

      // Add Spacecraft CS name
      fullList.push_back(coordSysName);

      // Add Tank names
      fullList.insert(fullList.end(), tankNames.begin(), tankNames.end());

      // Add Thruster names and it's ref. object names
      for (ObjectArray::iterator i = thrusters.begin(); i < thrusters.end(); ++i)
      {
         // Add Thruster name
         if ((*i)->GetName() != wxT(""))
            fullList.push_back((*i)->GetName());

         // Add Thruster's ref. object name
         StringArray refObjNames = (*i)->GetRefObjectNameArray(type);
         for (StringArray::iterator j = refObjNames.begin(); j != refObjNames.end(); ++j)
         {
            if (find(fullList.begin(), fullList.end(), (*j)) == fullList.end())
               fullList.push_back(*j);
         }
      }

      // Add other hardware names and it's ref. object names
      fullList.insert(fullList.end(), hardwareNames.begin(), hardwareNames.end());

      // Add Attitude's ref. object names
      wxString attRefObjName = attitude->GetRefObjectName(type);
      if (find(fullList.begin(), fullList.end(), attRefObjName) == fullList.end())
         fullList.push_back(attRefObjName);

      #ifdef DEBUG_SC_REF_OBJECT
      MessageInterface::ShowMessage
         (wxT("Spacecraft::GetRefObjectNameArray() ALL, thrusters.size()=%d, ")
          wxT("fullList.size()=%d, returning\n"), thrusters.size(), fullList.size());
      for (UnsignedInt i=0; i<fullList.size(); i++)
         MessageInterface::ShowMessage(wxT("   '%s'\n"), fullList[i].c_str());
      #endif
      return fullList;
   }
   else
   {
      if (type == Gmat::ATTITUDE)
      {
         fullList.push_back(attitude->GetRefObjectName(type));
         return fullList;
      }

      if (type == Gmat::FUEL_TANK)
         return tankNames;
      if (type == Gmat::THRUSTER)
         return thrusterNames;

      if (type == Gmat::HARDWARE)
      {
         fullList = tankNames;
         fullList.insert(fullList.end(), thrusterNames.begin(), thrusterNames.end());
         fullList.insert(fullList.end(), hardwareNames.begin(), hardwareNames.end());   // made changes by Tuan Nguyen
         return fullList;
      }

      if (type == Gmat::COORDINATE_SYSTEM)
      {
         // Add Spacecraft's CoordinateSystem name
         fullList.push_back(coordSysName);

         // Add Thruster's CoordinateSystem name
         for (ObjectArray::iterator i = thrusters.begin(); i < thrusters.end(); ++i)
         {
            StringArray refObjNames = (*i)->GetRefObjectNameArray(type);
            for (StringArray::iterator j = refObjNames.begin(); j != refObjNames.end(); ++j)
            {
               if (find(fullList.begin(), fullList.end(), (*j)) == fullList.end())
                  fullList.push_back(*j);
            }
         }

         // Add Attitude's CoordinateSystem name
         wxString attRefObjName = attitude->GetRefObjectName(type);

         if (find(fullList.begin(), fullList.end(), attRefObjName) == fullList.end())
            fullList.push_back(attRefObjName);

         #ifdef DEBUG_SC_REF_OBJECT
         MessageInterface::ShowMessage
            (wxT("Spacecraft::GetRefObjectNameArray() CS, thrusters.size()=%d, ")
             wxT("fullList.size()=%d, returning\n"), thrusters.size(), fullList.size());
         for (UnsignedInt i=0; i<fullList.size(); i++)
            MessageInterface::ShowMessage(wxT("   '%s'\n"), fullList[i].c_str());
         #endif

         return fullList;
      }
   }

   return SpaceObject::GetRefObjectNameArray(type);
}


// DJC: Not sure if we need this yet...
//------------------------------------------------------------------------------
// bool SetRefObjectName(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
bool Spacecraft::SetRefObjectName(const Gmat::ObjectType type, const wxString &name)
{
   #ifdef DEBUG_SC_REF_OBJECT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::SetRefObjectName() this=<%p>'%s' entered, type=%d, name='%s'\n"),
       this, GetName().c_str(), type, name.c_str());
   #endif

   if (type == Gmat::COORDINATE_SYSTEM)
   {
      #ifdef DEBUG_SPACECRAFT_CS
      MessageInterface::ShowMessage
         (wxT("Spacecraft::SetRefObjectName() About to change CoordSysName ")
          wxT("'%s' to '%s'\n"), coordSysName.c_str(), name.c_str());
      #endif
      coordSysName = name;
      return true;
   }

   return SpaceObject::SetRefObjectName(type, name);
}


//---------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name)
//---------------------------------------------------------------------------
/**
 * Returns the reference object pointer.
 *
 * @param type type of the reference object.
 * @param name name of the reference object.
 *
 * @return reference object pointer.
 */
//---------------------------------------------------------------------------
GmatBase* Spacecraft::GetRefObject(const Gmat::ObjectType type,
                                   const wxString &name)
{
   #ifdef DEBUG_GET_REF_OBJECT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::GetRefObject() <%p>'%s' entered, type=%d, name='%s'\n")
       wxT("tanks.size()=%d, thrusters.size()=%d\n"), this, GetName().c_str(), type,
       name.c_str(), tanks.size(), thrusters.size());
   #endif

   // This switch statement intentionally drops through without breaks, so that
   // the search in the tank and thruster name lists only need to be coded once.
   switch (type)
   {
      case Gmat::COORDINATE_SYSTEM:
         return coordinateSystem;

      case Gmat::ATTITUDE:
         #ifdef DEBUG_SC_ATTITUDE
         MessageInterface::ShowMessage(
         wxT("In SC::GetRefObject - returning Attitude pointer <%p>\n"), attitude);
         #endif
         return attitude;

      case Gmat::HARDWARE:                      // made changes by Tuan Nguyen
          for (ObjectArray::iterator i = hardwareList.begin();
               i < hardwareList.end(); ++i) {
             if ((*i)->GetName() == name)
                return *i;
          }

      case Gmat::FUEL_TANK:
         for (ObjectArray::iterator i = tanks.begin();
              i < tanks.end(); ++i) {
            if ((*i)->GetName() == name)
               return *i;
         }

      case Gmat::THRUSTER:
         for (ObjectArray::iterator i = thrusters.begin();
              i < thrusters.end(); ++i) {
            if ((*i)->GetName() == name)
            {
               #ifdef DEBUG_GET_REF_OBJECT
               MessageInterface::ShowMessage
                  (wxT("Spacecraft::GetRefObject() Found Thruster named '%s', so ")
                   wxT("returning <%p>\n"), name.c_str(), (*i));
               #endif
               return *i;
            }
         }

         // Other Hardware cases go here...

         return NULL;      // Hardware requested, but not in the hardware lists

      default:
         break;
   }

   return SpaceObject::GetRefObject(type, name);
}


//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name)
//------------------------------------------------------------------------------
bool Spacecraft::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                              const wxString &name)
{
   #ifdef DEBUG_SC_REF_OBJECT
   MessageInterface::ShowMessage
      (wxT("Entering SC::SetRefObject <%p>'%s', obj=<%p><%s>'%s'\n"), this, GetName().c_str(),
       obj, obj ? obj->GetTypeName().c_str() : wxT("NULL"),
       obj ? obj->GetName().c_str() : wxT("NULL"));
   #endif

   if (obj == NULL)
      return false;

   wxString objType = obj->GetTypeName();
   wxString objName = obj->GetName();

   if (objName == originName)
   {
      if (obj->IsOfType(Gmat::SPACE_POINT))
      {
         origin = (SpacePoint*)obj;
      }
   }

   // now work on hardware
   if (type == Gmat::HARDWARE || type == Gmat::FUEL_TANK || type == Gmat::THRUSTER)
   {
      #ifdef DEBUG_SC_REF_OBJECT
      MessageInterface::ShowMessage
         (wxT("Spacecraft::SetRefObject() tanks.size()=%d, thrusters.size()=%d\n"),
          tanks.size(), thrusters.size());
      #endif

      // set fueltank
      if (objType == wxT("FuelTank"))
         return SetHardware(obj, tankNames, tanks);

      // set thruster
      if (objType == wxT("Thruster"))
         return SetHardware(obj, thrusterNames, thrusters);

      // set on hardware                // made changes by Tuan Nguyen
      if (obj->GetType() == Gmat::HARDWARE)             //(objType == wxT("Hardware"))
      {
         return SetHardware(obj, hardwareNames, hardwareList);
      }

      return false;
   }
   else if (type == Gmat::COORDINATE_SYSTEM)
   {
      CoordinateSystem *cs = (CoordinateSystem*)obj;

      #ifdef DEBUG_SPACECRAFT_CS
      MessageInterface::ShowMessage
         (wxT("Spacecraft::SetRefObject() '%s', coordinateSystem=%s<%p>, cs=%s<%p>\n"),
          instanceName.c_str(), coordinateSystem->GetName().c_str(), coordinateSystem,
          cs->GetName().c_str(), cs);
      #endif

      // Assign CoordinateSystem to map, so that spacecraft can set
      // CoordinateSystem pointer to cloned thruster in SetHardware()(LOJ: 2009.08.25)
      coordSysMap[objName] = cs;
      #ifdef DEBUG_SC_REF_OBJECT
      MessageInterface::ShowMessage
         (wxT("Spacecraft::SetRefObject() Assigned <%p>'%s' to coordSysMap, ")
          wxT("coordSysMap.size()=%d, isThrusterSettingMode=%d\n"), obj, objName.c_str(),
          coordSysMap.size(), isThrusterSettingMode);
      #endif

      // first, try setting it on the attitude (owned object)
      if (attitude)
      {
         try
         {
            #ifdef DEBUG_SC_ATTITUDE
            MessageInterface::ShowMessage
               (wxT("   Setting <%p><%s>'%s' to attitude <%p>\n"), obj,
                objType.c_str(), objName.c_str(), attitude);
            #endif
            // Pass objName as name since name can be blank.
            // Attitude::SetRefObject() checks names before setting
            attitude->SetRefObject(obj, type, objName);
         }
         catch (BaseException &)
         {
            #ifdef DEBUG_SC_ATTITUDE
            MessageInterface::ShowMessage(
               wxT("------ error setting ref object %s on attitude\n"),
               name.c_str());
            #endif
         }
      }

      // Set Thruster's CoordinateSystem
      for (ObjectArray::iterator i = thrusters.begin(); i != thrusters.end(); ++i)
      {
         GmatBase *thr = *i;
         wxString thrCsName = thr->GetRefObjectName(Gmat::COORDINATE_SYSTEM);

         if (thrCsName == name)
         {
            #ifdef DEBUG_SC_REF_OBJECT
            MessageInterface::ShowMessage
               (wxT("   Setting CoordinateSystem <%p>'%s' to thruster<%p>'%s'\n"),
                cs, name.c_str(), thr, thr->GetName().c_str());
            #endif
            thr->SetRefObject(cs, Gmat::COORDINATE_SYSTEM, thrCsName);
         }
      }

      // If thruster setting mode, we are done.
      if (isThrusterSettingMode)
         return true;

      // If CS name is not the spacecraft CS name, we are done.
      if (objName != coordSysName)
         return true;

      // Otherwise, convert initial state to to new CS
      if (coordinateSystem == cs)
      {
         #ifdef DEBUG_SPACECRAFT_CS
         MessageInterface::ShowMessage
            (wxT("   Input coordinateSystem is the same as current one, so ignoring\n"));
         #endif
      }
      else
      {
         #ifdef DEBUG_SPACECRAFT_CS
         MessageInterface::ShowMessage
            (wxT("   About to convert to new CS '%s'\n"), coordSysName.c_str());
         #endif

         // saved the old CS and added try/catch block to set to old CS
         // in case of exception thrown (loj: 2008.10.23)
         CoordinateSystem *oldCS = coordinateSystem;
         coordinateSystem = cs;

         originName = coordinateSystem->GetOriginName();
         origin     = coordinateSystem->GetOrigin();

         try
         {
            TakeAction(wxT("ApplyCoordinateSystem"));

            #ifdef DEBUG_SPACECRAFT_CS
            MessageInterface::ShowMessage
               (wxT("Spacecraft::SetRefObject() coordinateSystem applied ----------\n"));
            Rvector6 vec6(state.GetState());
            MessageInterface::ShowMessage(wxT("   %s\n"), vec6.ToString().c_str());
            #endif
         }
         catch (BaseException &)
         {
            #ifdef DEBUG_SPACECRAFT_CS
            MessageInterface::ShowMessage
               (wxT("Exception thrown: '%s', so setting back to old CS\n"), e.GetFullMessage().c_str());
            #endif
            coordinateSystem = oldCS;
            throw;
         }
      }

      return true;
   }
   else if (type == Gmat::ATTITUDE)
   {
      #ifdef DEBUG_SC_ATTITUDE
         MessageInterface::ShowMessage(wxT("Setting attitude object on spacecraft %s\n"),
         instanceName.c_str());
      #endif
      if ((attitude != NULL) && (attitude != (Attitude*) obj))
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (attitude, wxT("attitude"), wxT("Spacecraft::SetRefObject()"),
             wxT("deleting attitude of ") + GetName(), this);
         #endif
         delete attitude;
         ownedObjectCount--;
         #ifdef DEBUG_SC_OWNED_OBJECT
         MessageInterface::ShowMessage
            (wxT("Spacecraft::SetRefObject() <%p>'%s' ownedObjectCount=%d\n"),
             this, GetName().c_str(), ownedObjectCount);
         #endif
      }
      attitude = (Attitude*) obj;
      ownedObjectCount++;
      // set epoch ...
      #ifdef DEBUG_SC_ATTITUDE
         MessageInterface::ShowMessage(wxT("Setting attitude object on spacecraft %s\n"),
         instanceName.c_str());
         MessageInterface::ShowMessage(
         wxT("Setting epoch on attitude object for spacecraft %s\n"),
         instanceName.c_str());
      #endif
      attitude->SetEpoch(state.GetEpoch());
      #ifdef __USE_SPICE__
         if (attitude->IsOfType(wxT("SpiceAttitude")))
            ((SpiceAttitude*) attitude)->SetObjectID(instanceName, naifId, naifIdRefFrame);
      #endif
      return true;
   }

   #ifdef DEBUG_SC_REF_OBJECT
   MessageInterface::ShowMessage
      (wxT("Exiting SC::SetRefObject, Calling SpaceObject::SetRefObject()\n"));
   #endif

   return SpaceObject::SetRefObject(obj, type, name);
}


//---------------------------------------------------------------------------
//  ObjectArray& GetRefObjectArray(const Gmat::ObjectType type)
//---------------------------------------------------------------------------
/**
 * Obtains an array of GmatBase pointers by type.
 *
 * @param type The type of objects requested
 *
 * @return Reference to the array.
 */
//---------------------------------------------------------------------------
ObjectArray& Spacecraft::GetRefObjectArray(const Gmat::ObjectType type)
{
   if (type == Gmat::HARDWARE)          // made changes by Tuan Nguyen
      return hardwareList;
   if (type == Gmat::FUEL_TANK)
      return tanks;
   if (type == Gmat::THRUSTER)
      return thrusters;
   return SpaceObject::GetRefObjectArray(type);
}


//---------------------------------------------------------------------------
//  ObjectArray& GetRefObjectArray(const Gmat::ObjectType type)
//---------------------------------------------------------------------------
/**
 * Obtains an array of GmatBase pointers based on a string (e.g. the typename).
 *
 * @param typeString The string used to find the objects requested.
 *
 * @return Reference to the array.
 */
//---------------------------------------------------------------------------
ObjectArray& Spacecraft::GetRefObjectArray(const wxString& typeString)
{
   if (typeString == wxT("Hardware"))        // made changes by Tuan Nguyen
      return hardwareList;
   if ((typeString == wxT("FuelTank")) || (typeString == wxT("Tanks")))
      return tanks;
   if ((typeString == wxT("Thruster")) || (typeString == wxT("Thrusters")))
      return thrusters;
   return SpaceObject::GetRefObjectArray(typeString);
}


//---------------------------------------------------------------------------
//  Integer GetParameterID(const wxString &str) const
//---------------------------------------------------------------------------
/**
 * Retrieve the ID for the parameter given its description.
 *
 * @param <str> Description for the parameter.
 *
 * @return the parameter ID, or -1 if there is no associated ID.
 */
//---------------------------------------------------------------------------
Integer Spacecraft::GetParameterID(const wxString &str) const
{
   #ifdef DEBUG_PARM_PERFORMANCE
      MessageInterface::ShowMessage(wxT("Spacecraft::GetParameterID(%s)\n"), str.c_str());
   #endif
   #ifdef DEBUG_GET_REAL
   MessageInterface::ShowMessage(wxT("In SC::GetParameterID, str = %s\n "),
   str.c_str());
   #endif

   try
   {
      // handle AddHardware parameter:
      if (str == wxT("AddHardware")) // made changes by Tuan Nguyen
         return ADD_HARDWARE;

      // handle special parameter to work in GmatFunction (loj: 2008.06.27)
      if (str == wxT("UTCGregorian"))
         return UTC_GREGORIAN;

      // first check the multiple reps
      Integer sz = EndMultipleReps - CART_X;
      for (Integer ii = 0; ii < sz; ii++)
         if (str == MULT_REP_STRINGS[ii])
         {
            #ifdef DEBUG_GET_REAL
            MessageInterface::ShowMessage(
            wxT("In SC::GetParameterID, multiple reps found!! - str = %s and id = %d\n "),
            str.c_str(), (ii + CART_X));
            #endif
            return ii + CART_X;
         }

      Integer retval = -1;
      if (str == wxT("Element1") || str == wxT("X") || str == wxT("SMA") || str == wxT("RadPer") ||
          str == wxT("RMAG"))
         retval =  ELEMENT1_ID;
         //return ELEMENT1_ID;

      else if (str == wxT("Element2") || str == wxT("Y") || str == wxT("ECC") || str == wxT("RadApo") ||
               str == wxT("RA") || str == wxT("PEY") || str == wxT("EquinoctialH"))
//            str == wxT("RA") || str == wxT("PECCY"))
         retval =  ELEMENT2_ID;
         //return ELEMENT2_ID;

      else if (str == wxT("Element3") || str == wxT("Z") || str == wxT("INC") || str == wxT("DEC") ||
               str == wxT("PEX") || str == wxT("EquinoctialK"))
//            str == wxT("PECCX"))
         retval =  ELEMENT3_ID;
         //return ELEMENT3_ID;

      else if (str == wxT("Element4") || str == wxT("VX") || str == wxT("RAAN") || str == wxT("VMAG") ||
          str == wxT("PNY") || str == wxT("EquinoctialP"))
         retval =  ELEMENT4_ID;
         //return ELEMENT4_ID;

      else if (str == wxT("Element5") || str == wxT("VY") || str == wxT("AOP") || str == wxT("AZI") ||
          str == wxT("RAV") || str == wxT("PNX") || str == wxT("EquinoctialQ"))
         retval =  ELEMENT5_ID;
         //return ELEMENT5_ID;

      else if (str == wxT("Element6") || str == wxT("VZ") || str == wxT("TA") || str == wxT("MA") ||
          str == wxT("EA") || str == wxT("HA") || str == wxT("FPA") || str == wxT("DECV") || str == wxT("MLONG"))
         retval =  ELEMENT6_ID;
         //return ELEMENT6_ID;

      #ifdef DEBUG_GET_REAL
      MessageInterface::ShowMessage(
      wxT("In SC::GetParameterID, after checking for elements, id = %d\n "),
      retval);
      #endif
      if (retval != -1) return retval;

      for (Integer i = SpaceObjectParamCount; i < SpacecraftParamCount; ++i)
      {
         if (str == PARAMETER_LABEL[i - SpaceObjectParamCount])
         {
            #ifdef DEBUG_SPACECRAFT_SET
            MessageInterface::ShowMessage(
            wxT("In SC::GetParameterID, setting id to %d for str = %s\n "),
            i, str.c_str());
            #endif
            return i;
         }
      }
      if (str == wxT("STM"))
         return ORBIT_STM;

      if (str == wxT("AMatrix"))
         return ORBIT_A_MATRIX;
//      if (str == wxT("OrbitSpiceKernelName"))
//         return ORBIT_SPICE_KERNEL_NAME;
//
//      if (str == wxT("AttitudeSpiceKernelName"))
//         return ATTITUDE_SPICE_KERNEL_NAME;
//
//      if (str == wxT("SCClockSpiceKernelName"))
//         return SC_CLOCK_SPICE_KERNEL_NAME;
//
//      if (str == wxT("FrameSpiceKernelName"))
//         return FRAME_SPICE_KERNEL_NAME;


      if ((str == wxT("CartesianState")) || (str == wxT("CartesianX"))) return CARTESIAN_X;
      if (str == wxT("CartesianY") )  return CARTESIAN_Y;
      if (str == wxT("CartesianZ") )  return CARTESIAN_Z;
      if (str == wxT("CartesianVX"))  return CARTESIAN_VX;
      if (str == wxT("CartesianVY"))  return CARTESIAN_VY;
      if (str == wxT("CartesianVZ"))  return CARTESIAN_VZ;

      return SpaceObject::GetParameterID(str);
   }
   catch (BaseException&)
   {
      // continue - could be an attitude parameter
      if (attitude)
      {
            Integer attId = attitude->GetParameterID(str);
            #ifdef DEBUG_SC_ATTITUDE
            MessageInterface::ShowMessage(
               wxT("------ Now calling attitude to get id for label %s\n"),
               str.c_str());
            MessageInterface::ShowMessage(wxT(" ------ and the id = %d\n"), attId);
            MessageInterface::ShowMessage(wxT(" ------ and the id with offset  = %d\n"),
                                          attId + ATTITUDE_ID_OFFSET);
            #endif
            return attId + ATTITUDE_ID_OFFSET;

      }
      // Add other owned objects here

      // Rethrow the exception
      else
         throw;
   }

   return -1;
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <id> Description for the parameter.
 *
 * @return true if the parameter is read only, false if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool Spacecraft::IsParameterReadOnly(const Integer id) const
{
   if (id >= ATTITUDE_ID_OFFSET)
   {
      if (attitude)
         return attitude->IsParameterReadOnly(id - ATTITUDE_ID_OFFSET);
   }
   // We are currently not allowing users to set anomaly other than the
   // True Anomaly ****** to be modified in the future ******
   if ((id == ELEMENT6_ID) &&
       ((stateElementLabel[5] == wxT("MA")) || (stateElementLabel[5] == wxT("EA")) ||
        (stateElementLabel[5] == wxT("HA"))))
   {
      return true;
   }
   if ((id >= ELEMENT1UNIT_ID) && (id <= ELEMENT6UNIT_ID))
   {
      return true;
   }

   if ((id >= CARTESIAN_X) && (id <= CARTESIAN_VZ))
   {
      return true;
   }

   if (id == TOTAL_MASS_ID)
   {
      return true;
   }

   // Hide SpaceObject epoch so spacecraft can treat it as a string
   if (id == EPOCH_PARAM)
   {
      return true;
   }

   // This is fix for using Epoch.UTCGregorian in GmatFunction
   if (id == UTC_GREGORIAN)
   {
      return true;
   }

   if (id == ORBIT_STM)
   {
      return true;
   }

   if (id == ORBIT_A_MATRIX)
   {
      return true;
   }

   if (id == MASS_FLOW)
   {
      return true;
   }

   if ((id > MODEL_FILE) && (id < MODEL_MAX))
   {
      if (modelFile == wxT(""))
         return true;
      else
         return false;
   }

   // NAIF ID is not read-only for spacecraft
   if (id == NAIF_ID)  return false;

   // NAIF ID for the spacecraft reference frame is not read-only for spacecraft
   if (id == NAIF_ID_REFERENCE_FRAME)  return false;

   // if (id == STATE_TYPE) return true;   when deprecated stuff goes away

   return SpaceObject::IsParameterReadOnly(id);
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <label> Description for the parameter.
 *
 * @return true if the parameter is read only, false (the default) if not.
 */
//---------------------------------------------------------------------------
bool Spacecraft::IsParameterReadOnly(const wxString &label) const
{
   return IsParameterReadOnly(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool ParameterAffectsDynamics(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Determines if a parameter update affects propagation, and therfore forces a
 * reload of parameters used in propagation
 *
 * @param id The ID of the parameter
 *
 * @return true if the parameter affects propagation, false otherwise
 */
//------------------------------------------------------------------------------
bool Spacecraft::ParameterAffectsDynamics(const Integer id) const
{
   if (id == MASS_FLOW)
      return true;

   //if (includeCartesianState > 0)
   if (isManeuvering)
   {
      if (id == CARTESIAN_X)
         return true;
      if (id == CARTESIAN_Y)
         return true;
      if (id == CARTESIAN_Z)
         return true;
      if (id == CARTESIAN_VX)
         return true;
      if (id == CARTESIAN_VY)
         return true;
      if (id == CARTESIAN_VZ)
         return true;
   }

   if (id == SRP_AREA_ID)
      return true;

   if (id == DRAG_AREA_ID)
      return true;

   return SpaceObject::ParameterAffectsDynamics(id);
}

//------------------------------------------------------------------------------
// bool ParameterDvInitializesNonzero(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Describe the method here
 *
 * @param
 *
 * @return
 */
//------------------------------------------------------------------------------
bool Spacecraft::ParameterDvInitializesNonzero(const Integer id,
      const Integer r, const Integer c) const
{
   if (id == ORBIT_STM)
   {
      if (r == c-3)
         return true;
      return false;
   }

   if (id == ORBIT_A_MATRIX)
   {
      if (r == c-3)
         return true;
      return false;
   }

   return SpaceObject::ParameterDvInitializesNonzero(id);
}

Real Spacecraft::ParameterDvInitialValue(const Integer id, const Integer r,
      const Integer c) const
{
   if (r == c-3)
      return 1.0;
   return 0.0;
}

//---------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//---------------------------------------------------------------------------
/*
 * @see GmatBase
 */
//---------------------------------------------------------------------------
wxString Spacecraft::GetParameterText(const Integer id) const
{
   #ifdef DEBUG_SC_PARAMETER_TEXT
   MessageInterface::ShowMessage(wxT("SC::GetParameterText - called with id = %d\n"),
   id);
   #endif

   // handle special parameter to work in GmatFunction (loj: 2008.06.27)
   if (id == UTC_GREGORIAN)
      return PARAMETER_LABEL[id - SpaceObjectParamCount];

   if ((id >= CART_X) && (id < EndMultipleReps))
   {
      #ifdef DEBUG_SC_PARAMETER_TEXT
      MessageInterface::ShowMessage(wxT("SC::GetParameterText - returning text = %s\n"),
      (MULT_REP_STRINGS[id-CART_X]).c_str());
      #endif
      return MULT_REP_STRINGS[id - CART_X];
   }
   // Handle the dynamic labels for the elements first
   if (id == ELEMENT1_ID || id == ELEMENT2_ID || id == ELEMENT3_ID
       || id == ELEMENT4_ID || id == ELEMENT5_ID || id == ELEMENT6_ID)
      return stateElementLabel[id - ELEMENT1_ID];

   if ((id >= SpaceObjectParamCount) && (id < SpacecraftParamCount))
      return PARAMETER_LABEL[id - SpaceObjectParamCount];

   if (id >= ATTITUDE_ID_OFFSET)
   {
      if (attitude)
         return attitude->GetParameterText(id - ATTITUDE_ID_OFFSET);
   }

   #ifdef DEBUG_SC_PARAMETER_TEXT
   MessageInterface::ShowMessage(
   wxT("SC::GetParameterText - calling through to base class .....\n"));
   #endif
   return SpaceObject::GetParameterText(id);
}

//------------------------------------------------------------------------------
//  Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Gets the type of a parameter.
 *
 * @param <id> Integer ID of the parameter.
 *
 * @return The type of the parameter.
 */
//------------------------------------------------------------------------------
Gmat::ParameterType Spacecraft::GetParameterType(const Integer id) const
{
   if ((id >= CART_X) && (id < EndMultipleReps))
      return Gmat::REAL_TYPE;
   if ((id >= SpaceObjectParamCount) && (id < SpacecraftParamCount))
      return PARAMETER_TYPE[id - SpaceObjectParamCount];
   if (id >= ATTITUDE_ID_OFFSET)
      if (attitude)
      {
         #ifdef DEBUG_SC_ATTITUDE
            MessageInterface::ShowMessage
               (wxT("Calling attitude to get parameter type ( for %d) - it is %d (%s)\n"),
               id, attitude->GetParameterType(id - ATTITUDE_ID_OFFSET),
               (GmatBase::PARAM_TYPE_STRING[(Integer)(attitude->GetParameterType(id - ATTITUDE_ID_OFFSET))]).c_str());
         #endif
         return attitude->GetParameterType(id - ATTITUDE_ID_OFFSET);
      }

    return SpaceObject::GetParameterType(id);
}

//------------------------------------------------------------------------------
//  wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Gets the text description for the type of a parameter.
 *
 * @param <id> Integer ID of the parameter.
 *
 * @return The text description of the type of the parameter.
 */
//------------------------------------------------------------------------------
wxString Spacecraft::GetParameterTypeString(const Integer id) const
{
//    return SpaceObject::PARAM_TYPE_STRING[GetParameterType(id)];
    return GmatBase::PARAM_TYPE_STRING[GetParameterType(id)];
}

//---------------------------------------------------------------------------
//  Real GetRealParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for a Real parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The parameter's value.
 */
Real Spacecraft::GetRealParameter(const Integer id) const
{
   #ifdef DEBUG_GET_REAL
      MessageInterface::ShowMessage(
      wxT("In SC::GetReal, asking for parameter %d, whose string is \"%s\"\n"),
      id, (GetParameterText(id)).c_str());
      //for (Integer i=0; i<6;i++)
      //   MessageInterface::ShowMessage(wxT("   state(%d) = %.12f\n"),
      //   i, state[i]);
      //MessageInterface::ShowMessage(wxT("    and stateType = %s\n"),
      //   stateType.c_str());
   #endif

   if ((id >= ELEMENT1_ID && id <= ELEMENT6_ID) ||
      (id >= CART_X      && id < EndMultipleReps))
   {
      #ifdef DEBUG_GET_REAL
         MessageInterface::ShowMessage(
         wxT("In SC::GetReal, calling GetElement ....... \n"));
      #endif
      return (const_cast<Spacecraft*>(this))->GetElement(GetParameterText(id));
   }

   if (id == DRY_MASS_ID) return dryMass;
   if (id == CD_ID) return coeffDrag;
   if (id == CR_ID) return reflectCoeff;
   if (id == DRAG_AREA_ID) return dragArea;
   if (id == SRP_AREA_ID) return srpArea;
   if (id == TOTAL_MASS_ID)  return UpdateTotalMass();

   if (id == CARTESIAN_X )  return state[0];
   if (id == CARTESIAN_Y )  return state[1];
   if (id == CARTESIAN_Z )  return state[2];
   if (id == CARTESIAN_VX)  return state[3];
   if (id == CARTESIAN_VY)  return state[4];
   if (id == CARTESIAN_VZ)  return state[5];

   if (id == MASS_FLOW)  return UpdateTotalMass();

   if (id >= ATTITUDE_ID_OFFSET)
      if (attitude)
      {
         #ifdef DEBUG_SC_ATTITUDE
         MessageInterface::ShowMessage(
            wxT("------ Now calling attitude to get real parameter for id =  %d\n"),
            id);
         #endif
         return attitude->GetRealParameter(id - ATTITUDE_ID_OFFSET);
      }

   if (id == MODEL_OFFSET_X)     return modelOffsetX;
   if (id == MODEL_OFFSET_Y)     return modelOffsetY;
   if (id == MODEL_OFFSET_Z)     return modelOffsetZ;
   if (id == MODEL_ROTATION_X)   return modelRotationX;
   if (id == MODEL_ROTATION_Y)   return modelRotationY;
   if (id == MODEL_ROTATION_Z)   return modelRotationZ;
   if (id == MODEL_SCALE)        return modelScale;

   return SpaceObject::GetRealParameter(id);
}

//---------------------------------------------------------------------------
//  Real GetRealParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for a Real parameter.
 *
 * @param <label> The label of the parameter.
 *
 * @return The parameter's value.
 */
Real Spacecraft::GetRealParameter(const wxString &label) const
{
   // Performance!
    if (label == wxT("A1Epoch"))
       return state.GetEpoch();

    // First check with anomaly
    /* OLD CODE
    if (label == wxT("TA") || label == wxT("MA") || label == wxT("EA") || label == wxT("HA"))
    {
       //return trueAnomaly.GetValue();
       return trueAnomaly.GetValue(label);
    }
    */

    return GetRealParameter(GetParameterID(label));
}

//---------------------------------------------------------------------------
//  Real SetRealParameter(const Integer id, const Real value)
//---------------------------------------------------------------------------
/**
 * Set the value for a Real parameter.
 *
 * @param <id> The integer ID for the parameter.
 * @param <value> The new parameter value.
 *
 * @return the parameter value at the end of this call, or
 *         REAL_PARAMETER_UNDEFINED if the parameter id is invalid or the
 *         parameter type is not Real.
 */
Real Spacecraft::SetRealParameter(const Integer id, const Real value)
{
   #ifdef DEBUG_SPACECRAFT_SET
   MessageInterface::ShowMessage(wxT("In SC::SetRealParameter (%s), id = %d (%s) and value = %.12f\n"),
   instanceName.c_str(), id, GetParameterText(id).c_str(), value);
   #endif
   if (id >= CART_X && id < EndMultipleReps)
   {
      wxString idString = MULT_REP_STRINGS[id - CART_X];
      return SetRealParameter(idString,value);
   }
   if (id == ELEMENT1_ID) return SetRealParameter(stateElementLabel[0],value);
   if (id == ELEMENT2_ID) return SetRealParameter(stateElementLabel[1],value);
   if (id == ELEMENT3_ID) return SetRealParameter(stateElementLabel[2],value);
   if (id == ELEMENT4_ID) return SetRealParameter(stateElementLabel[3],value);
   if (id == ELEMENT5_ID) return SetRealParameter(stateElementLabel[4],value);
   if (id == ELEMENT6_ID) return SetRealParameter(stateElementLabel[5],value);

   if (id == DRY_MASS_ID)
   {
      parmsChanged = true;
      return SetRealParameter(wxT("DryMass"), value);
   }

   if (id == CD_ID)
   {
      parmsChanged = true;
      return SetRealParameter(wxT("Cd"),value);
   }
   if (id == CR_ID)
   {
      parmsChanged = true;
      return SetRealParameter(wxT("Cr"),value);
   }
   if (id == DRAG_AREA_ID)
   {
      parmsChanged = true;
      return SetRealParameter(wxT("DragArea"),value);
   }
   if (id == SRP_AREA_ID)
   {
      parmsChanged = true;
      return SetRealParameter(wxT("SRPArea"),value);
   }

   // We should not allow users to set this one -- it's a calculated parameter
   if (id == TOTAL_MASS_ID) return SetRealParameter(wxT("TotalMass"),value);

   if (id >= ATTITUDE_ID_OFFSET)
      if (attitude)
         return attitude->SetRealParameter(id - ATTITUDE_ID_OFFSET,value);

   if (id == CARTESIAN_X )
   {
      state[0] = value;
      return state[0];
   }

   if (id == CARTESIAN_Y )
   {
      state[1] = value;
      return state[1];
   }

   if (id == CARTESIAN_Z )
   {
      state[2] = value;
      return state[2];
   }

   if (id == CARTESIAN_VX)
   {
      state[3] = value;
      return state[3];
   }

   if (id == CARTESIAN_VY)
   {
      state[4] = value;
      return state[4];
   }

   if (id == CARTESIAN_VZ)
   {
      state[5] = value;
      return state[5];
   }


   if (id == MASS_FLOW)
   {
      return ApplyTotalMass(value);
   }

   if (id == MODEL_OFFSET_X)
   {
      modelOffsetX = value;
      return modelOffsetX;
   }

   if (id == MODEL_OFFSET_Y)
   {
      modelOffsetY = value;
      return modelOffsetY;
   }

   if (id == MODEL_OFFSET_Z)
   {
      modelOffsetZ = value;
      return modelOffsetZ;
   }

   if (id == MODEL_ROTATION_X)
   {
      modelRotationX = value;
      return modelRotationX;
   }

   if (id == MODEL_ROTATION_Y)
   {
      modelRotationY = value;
      return modelRotationY;
   }

   if (id == MODEL_ROTATION_Z)
   {
      modelRotationZ = value;
      return modelRotationZ;
   }

   if (id == MODEL_SCALE)
   {
      modelScale = value;
      return modelScale;
   }


   return SpaceObject::SetRealParameter(id, value);
}

//------------------------------------------------------------------------------
//  Real SetRealParameter(const wxString &label, const Real value)
//------------------------------------------------------------------------------
/**
 * Set the value for a Real parameter.
 *
 * @param <label> The label of the parameter.
 * @param <value> The new parameter value.
 *
 * @return the parameter value at the end of this call, or
 *         REAL_PARAMETER_UNDEFINED if the parameter id is invalid or the
 *         parameter type is not Real.
 */
//------------------------------------------------------------------------------
Real Spacecraft::SetRealParameter(const wxString &label, const Real value)
{
   #ifdef DEBUG_SPACECRAFT_SET
   MessageInterface::ShowMessage
      (wxT("In SC::SetRealParameter(label)(%s), label = %s and value = %.12f\n"),
       instanceName.c_str(), label.c_str(), value);
   #endif
   // first (really) see if it's a parameter for an owned object (i.e. attitude)
   if (GetParameterID(label) >= ATTITUDE_ID_OFFSET)
      if (attitude)
         return attitude->SetRealParameter(label, value);

   // We are currently not allowing users to set anomaly other than the True
   // Anomaly ****** to be modified in the future ******
   if ((label == wxT("MA")) || (label == wxT("EA")) || (label == wxT("HA")))
      throw SpaceObjectException
         (wxT("ERROR - setting of anomaly of type other than True Anomaly not ")
          wxT("currently allowed."));

   // First try to set as a state element
   if (SetElement(label, value))
      return value;

   if (label == wxT("A1Epoch"))
   {
      state.SetEpoch(value);
      return value;
   }

   if (label == wxT("DryMass"))
   {
      if (value >= 0.0)
         dryMass = value;
      else
      {
         SpaceObjectException soe(wxT(""));
         soe.SetDetails(errorMessageFormat.c_str(),
                        GmatStringUtil::ToString(value, 16).c_str(),
                        wxT("DryMass"), wxT("Real Number >= 0.0"));
         throw soe;
      }
      parmsChanged = true;
      return dryMass;
   }

   if (label == wxT("Cd"))
   {
      if (value >= 0.0)
         coeffDrag = value;
      else
      {
         SpaceObjectException soe(wxT(""));
         soe.SetDetails(errorMessageFormat.c_str(),
                        GmatStringUtil::ToString(value, 16).c_str(),
                        wxT("Cd"), wxT("Real Number >= 0.0"));
         throw soe;
      }
      parmsChanged = true;
      return coeffDrag;
   }
   if (label == wxT("DragArea"))
   {
      if (value >= 0.0)
         dragArea = value;
      else
      {
         SpaceObjectException soe(wxT(""));
         soe.SetDetails(errorMessageFormat.c_str(),
                        GmatStringUtil::ToString(value, 16).c_str(),
                        wxT("DragArea"), wxT("Real Number >= 0.0"));
         throw soe;
      }
      parmsChanged = true;
      return dragArea;
   }
   if (label == wxT("SRPArea"))
   {
      if (value >= 0.0)
         srpArea = value;
      else
      {
         SpaceObjectException soe(wxT(""));
         soe.SetDetails(errorMessageFormat.c_str(),
                        GmatStringUtil::ToString(value, 16).c_str(),
                        wxT("SRPArea"), wxT("Real Number >= 0.0"));
         throw soe;
      }
      parmsChanged = true;
      return srpArea;
   }
   if (label == wxT("Cr"))
   {
      if ((value >= 0.0) && (value <= 2.0))
         reflectCoeff = value;
      else
      {
         SpaceObjectException soe(wxT(""));
         soe.SetDetails(errorMessageFormat.c_str(),
                        GmatStringUtil::ToString(value, 16).c_str(),
                        wxT("Cr"), wxT("0.0 <= Real Number <= 2.0"));
         throw soe;
      }
      parmsChanged = true;
      return reflectCoeff;
   }

   if (label == wxT("TotalMass"))// return totalMass;    // Don't change the total mass
      throw SpaceObjectException(wxT("The parameter \"TotalMass\" is a calculated ")
            wxT("parameter and cannot be set on the spacecraft ") + instanceName);

   return SpaceObject::SetRealParameter(label, value);
}


//------------------------------------------------------------------------------
//  wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieve a string parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The string stored for this parameter, or the empty string if there
 *         is no string association.
 */
//------------------------------------------------------------------------------
wxString Spacecraft::GetStringParameter(const Integer id) const
{
    if (id == SC_EPOCH_ID)
    {
       return (const_cast<Spacecraft*>(this))->GetEpochString();
//       return scEpochStr;
    }

    if (id == DATE_FORMAT_ID)
       return epochType;

    if (id == STATE_TYPE_ID)
    {
       MessageInterface::ShowMessage( wxT("\"StateType\" is deprecated as the ")
          wxT("string specifying the state type for display, and will be ")
          wxT("removed from a future build; please use \"DisplayStateType\" ")
          wxT("instead.\n") );
       return displayStateType;
       //return stateType;
    }

    if (id == DISPLAY_STATE_TYPE_ID)
    {
       return displayStateType;
    }

    if (id == ANOMALY_ID)
       return trueAnomaly.GetTypeString();

    if (id == COORD_SYS_ID)
       return coordSysName;

    if ((id >= ELEMENT1UNIT_ID) && (id <= ELEMENT6UNIT_ID))
       return stateElementUnits[id - ELEMENT1UNIT_ID];

    if (id == SPACECRAFT_ID)
       return spacecraftId;

    if (id >= ATTITUDE_ID_OFFSET)
       if (attitude)
       {
         #ifdef DEBUG_SC_ATTITUDE
         MessageInterface::ShowMessage(
            wxT("------ Now calling attitude to get string parameter for id =  %d\n"),
            id);
         #endif
          return attitude->GetStringParameter(id - ATTITUDE_ID_OFFSET);
       }

    if (id == MODEL_FILE)
       return modelFile;

    return SpaceObject::GetStringParameter(id);
}

//---------------------------------------------------------------------------
//  wxString GetStringParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Retrieve a string parameter.
 *
 * @param <label> The label for the parameter.
 *
 * @return The string stored for this parameter, or the empty string if there
 *         is no string association.
 */
//---------------------------------------------------------------------------
wxString Spacecraft::GetStringParameter(const wxString &label) const
{
//   if (label == wxT("StateType"))
//      return stateType;
//
   return GetStringParameter(GetParameterID(label));
}
//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id,
//       const Integer index) const
//------------------------------------------------------------------------------
/**
 * This method retrieves a string parameter from a StringArray
 *
 * @param id The ID of the parameter
 *
 * @return The parameter
 */
//------------------------------------------------------------------------------
wxString Spacecraft::GetStringParameter(const Integer id,
                                           const Integer index) const
{
   // made changes by Tuan Nguyen
   switch (id)
   {
      case ADD_HARDWARE:
         {
            if ((0 <= index)&(index < (Integer)(hardwareNames.size())))
               return hardwareNames[index];
            else
               return wxT("");
         }

      default:
         break;
   }
   return SpaceObject::GetStringParameter(id, index);
}

//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString & label,
//       const Integer index) const
//------------------------------------------------------------------------------
/**
 * This method retrieves a string parameter from a StringArray
 *
 * @param label The string label for the parameter
 *
 * @return The parameter
 */
//------------------------------------------------------------------------------
wxString Spacecraft::GetStringParameter(const wxString & label,
      const Integer index) const
{
   return GetStringParameter(GetParameterID(label), index);
}


//---------------------------------------------------------------------------
//  const StringArray& GetStringArrayParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Accesses lists of tank and thruster (and, eventually, other hardware) names,
 * and other StringArray parameters.
 *
 * @param id The integer ID for the parameter.
 *
 * @return The requested StringArray; throws if the parameter is not a
 *         StringArray.
 */
//---------------------------------------------------------------------------
const StringArray& Spacecraft::GetStringArrayParameter(const Integer id) const
{
   if (id == ADD_HARDWARE) // make changes by Tuan Nguyen
      return hardwareNames;
   if (id == FUEL_TANK_ID)
      return tankNames;
   if (id == THRUSTER_ID)
      return thrusterNames;
   if (id >= ATTITUDE_ID_OFFSET)
      if (attitude)
      {
         #ifdef DEBUG_SC_ATTITUDE
         MessageInterface::ShowMessage(
            wxT("------ Now calling attitude to SET string parameter for id =  %d\n"),
            id);
         #endif
         return attitude->GetStringArrayParameter(id - ATTITUDE_ID_OFFSET);
      }
//   if (id == ORBIT_SPICE_KERNEL_NAME)
//      return orbitSpiceKernelNames;
   return SpaceObject::GetStringArrayParameter(id);
}


//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Accesses lists of tank and thruster (and, eventually, other hardware) names,
 * and other StringArray parameters.
 *
 * @param label The script string for the parameter.
 *
 * @return The requested StringArray; throws if the parameter is not a
 *         StringArray.
 */
//------------------------------------------------------------------------------
const StringArray& Spacecraft::GetStringArrayParameter(
      const wxString &label) const
{
   return GetStringArrayParameter(GetParameterID(label));
}

//---------------------------------------------------------------------------
//  bool SetStringParameter(const Integer id, const wxString &value)
//---------------------------------------------------------------------------
/**
 * Change the value of a string parameter.
 *
 * @param <id> The integer ID for the parameter.
 * @param <value> The new string for this parameter.
 *
 * @return true if the string is stored, false if not.
 */
//---------------------------------------------------------------------------
bool Spacecraft::SetStringParameter(const Integer id, const wxString &value)
{
   #ifdef DEBUG_SC_SET_STRING
      MessageInterface::ShowMessage
         (wxT("Spacecraft::SetStringParameter() string parameter %d (%s) to %s\n"),
         id, GetParameterText(id).c_str(), value.c_str());
   #endif

   // made changes by Tuan Nguyen
   if (id == ADD_HARDWARE)
   {
      // Only add the hardware if it is not in the list already
      if (find(hardwareNames.begin(), hardwareNames.end(), value) ==
          hardwareNames.end())
      {
         hardwareNames.push_back(value);
      }
      return true;
   }

   if (id >= ATTITUDE_ID_OFFSET)
      if (attitude)
      {
         #ifdef DEBUG_SC_ATTITUDE
         MessageInterface::ShowMessage(
            wxT("------ Now calling attitude to SET string parameter for id =  %d")
            wxT(" and value = %s\n"), id, value.c_str());
         #endif
         return attitude->SetStringParameter(id - ATTITUDE_ID_OFFSET, value);
      }

   if ((id < SpaceObjectParamCount) || (id >= SpacecraftParamCount))
      return SpaceObject::SetStringParameter(id, value);

   if (id == SC_EPOCH_ID)
   {
      // Validate first...

      // and set the epoch value in the state
      SetEpoch(value);
   }
   else if (id == DATE_FORMAT_ID)
   {
      SetDateFormat(value);
   }
   // To handle Epoch.UTCGregorian in GmatFunction Assignment wrapper (loj: 2008.06.27)
   else if (id == UTC_GREGORIAN)
   {
      SetDateFormat(wxT("UTCGregorian"));
      SetEpoch(value);
   }
   else if ((id == STATE_TYPE_ID) || (id == DISPLAY_STATE_TYPE_ID))
   {
      if (id == STATE_TYPE_ID)
          MessageInterface::ShowMessage( wxT("\"StateType\" is deprecated as the ")
          wxT("string specifying the state type for display, and will be ")
          wxT("removed from a future build; please use \"DisplayStateType\" ")
          wxT("instead.\n") );

      // Check for invalid input then return unknown value from GmatBase
      if (value != wxT("Cartesian") && value != wxT("Keplerian") &&
          value != wxT("ModifiedKeplerian") && value != wxT("SphericalAZFPA") &&
          value != wxT("SphericalRADEC") && value != wxT("Equinoctial"))
      {
         throw SpaceObjectException(wxT("Unknown state element representation: ") +
            value);
      }
      #ifdef DEBUG_SC_SET_STRING
      MessageInterface::ShowMessage(wxT("SC::SetString - setting display state type to %s\n"),
      value.c_str());
      #endif

      if ((value == wxT("Keplerian")) || (value == wxT("ModifiedKeplerian")))
      {
         // Load trueAnomaly with the state data
         Rvector6 kep = GetStateInRepresentation(wxT("Keplerian"));
         trueAnomaly.SetSMA(kep[0]);
         trueAnomaly.SetECC(kep[1]);
         trueAnomaly.SetValue(kep[5]);
      }

      //stateType = value;
      displayStateType = value;
      UpdateElementLabels();
   }
   else if (id == ANOMALY_ID)
   {
      // Check for invalid input then return unknown value from GmatBase
      if (trueAnomaly.IsInvalid(value))
      {
         return GmatBase::SetStringParameter(id, value);
      }
      #ifdef DEBUG_SC_SET_STRING
          MessageInterface::ShowMessage(wxT("\nSpacecraft::SetStringParamter()...")
             wxT("\n   Before change, Anomaly info -> a: %f, e: %f, %s: %f\n"),
             trueAnomaly.GetSMA(),trueAnomaly.GetECC(),trueAnomaly.GetTypeString().c_str(),
             trueAnomaly.GetValue());
      #endif

      //trueAnomaly.SetType(value);
      anomalyType = value;
      UpdateElementLabels();

      #ifdef DEBUG_SC_SET_STRING
          MessageInterface::ShowMessage(
             wxT("\n   After change, Anomaly info -> a: %lf, e: %lf, %s: %lf\n"),
             trueAnomaly.GetSMA(), trueAnomaly.GetECC(), trueAnomaly.GetTypeString().c_str(),
             trueAnomaly.GetValue());
      #endif
      if ((stateType == wxT("Keplerian")) ||
          (stateType == wxT("ModifiedKeplerian")))
         rvState[5] = trueAnomaly.GetValue();   // @todo: add state[5]?
   }
   else if (id == COORD_SYS_ID)
   {
      #ifdef DEBUG_SPACECRAFT_CS
      MessageInterface::ShowMessage
         (wxT("Spacecraft::SetStringParameter() About to change CoordSysName ")
          wxT("'%s' to '%s'\n"), coordSysName.c_str(), value.c_str());
      #endif
      parmsChanged = true;
      coordSysName = value;
   }
   else if (id == SPACECRAFT_ID)
   {
      spacecraftId = value;
   }
   else if (id == FUEL_TANK_ID)
   {
      // Only add the tank if it is not in the list already
      if (find(tankNames.begin(), tankNames.end(), value) == tankNames.end())
      {
          tankNames.push_back(value);
      }
   }
   else if (id == THRUSTER_ID)
   {
      // Only add the thruster if it is not in the list already
      if (find(thrusterNames.begin(), thrusterNames.end(), value) ==
          thrusterNames.end())
      {
         thrusterNames.push_back(value);
      }
   }
// else if (id == ORBIT_SPICE_KERNEL_NAME)
//   {
//      // Only add the thruster if it is not in the list already
//      if (find(orbitSpiceKernelNames.begin(), orbitSpiceKernelNames.end(),
//            value) == orbitSpiceKernelNames.end())
//      {
//         orbitSpiceKernelNames.push_back(value);
//      }
//   }
        else if (id == MODEL_FILE)
   {
        modelFile = value;
   }
   else if (id == ORBIT_SPICE_KERNEL_NAME)
   {
      // Only add the thruster if it is not in the list already
      if (find(orbitSpiceKernelNames.begin(), orbitSpiceKernelNames.end(),
            value) == orbitSpiceKernelNames.end())
      {
         orbitSpiceKernelNames.push_back(value);
      }
   }

   #ifdef DEBUG_SC_SET_STRING
   MessageInterface::ShowMessage
      (wxT("Spacecraft::SetStringParameter() returning true\n"));
   #endif

   return true;
}

//---------------------------------------------------------------------------
//  bool SetStringParameter(const wxString &label, const wxString &value)
//---------------------------------------------------------------------------
/**
 * Change the value of a string parameter.
 *
 * @param <label> The label for the parameter.
 * @param <value> The new string for this parameter.
 *
 * @return true if the string is stored, false if not.
 */
//---------------------------------------------------------------------------
bool Spacecraft::SetStringParameter(const wxString &label,
                                    const wxString &value)
{
   #ifdef DEBUG_SPACECRAFT_SET
       MessageInterface::ShowMessage
          (wxT("\nSpacecraft::SetStringParameter(\"%s\", \"%s\") enters\n"),
           label.c_str(), value.c_str() );
       Integer id = GetParameterID(label);
       MessageInterface::ShowMessage
          (wxT("GetParameterText: %s\n"), GetParameterText(id).c_str());
       MessageInterface::ShowMessage
          (wxT("Spacecraft::SetStringParameter exits sooner\n\n"));
   #endif

   return SetStringParameter(GetParameterID(label),value);
}


//---------------------------------------------------------------------------
//  bool SetStringParameter(const Integer id, const wxString &value,
//                          const Integer index)
//---------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//---------------------------------------------------------------------------
bool Spacecraft::SetStringParameter(const Integer id, const wxString &value,
                                    const Integer index)
{
   #ifdef DEBUG_SC_SET_STRING
      MessageInterface::ShowMessage(wxT("In SC::SetStringParameter, id = %d, value = %s, index = %d\n"),
            id, value.c_str(), index);
   #endif
   if (index < 0)
   {
      SpaceObjectException ex;
      ex.SetDetails(wxT("The index %d is out-of-range for field \"%s\""), index,
                    GetParameterText(id).c_str());
      throw ex;
   }
   // check for owned object IDs first
   if (id >= ATTITUDE_ID_OFFSET)
   {
      if (attitude)
      {
         #ifdef DEBUG_SC_ATTITUDE
         MessageInterface::ShowMessage(
            wxT("------ Now calling attitude to SET string parameter for id =  %d")
            wxT(", index = %d,  and value = %s\n"), id, index, value.c_str());
         #endif
         return attitude->SetStringParameter(id - ATTITUDE_ID_OFFSET, value, index);
      }
   }

   switch (id)
   {
   case ADD_HARDWARE: // made changes by Tuan nguyen
      {
         if (index < (Integer)hardwareNames.size())
            hardwareNames[index] = value;
         else
            // Only add the hardware if it is not in the list already
            if (find(hardwareNames.begin(), hardwareNames.end(), value) == hardwareNames.end())
               hardwareNames.push_back(value);

         return true;
      }
   case FUEL_TANK_ID:
      {
         if (index < (Integer)tankNames.size())
            tankNames[index] = value;
         else
            // Only add the tank if it is not in the list already
            if (find(tankNames.begin(), tankNames.end(), value) == tankNames.end())
               tankNames.push_back(value);

         return true;
      }
   case THRUSTER_ID:
      {
         if (index < (Integer)thrusterNames.size())
            thrusterNames[index] = value;
         else
            // Only add the tank if it is not in the list already
            if (find(thrusterNames.begin(), thrusterNames.end(), value) ==
                thrusterNames.end())
               thrusterNames.push_back(value);

         return true;
      }

//   case ORBIT_SPICE_KERNEL_NAME:
//      if (index < (Integer)orbitSpiceKernelNames.size())
//         orbitSpiceKernelNames[index] = value;
//      // Only add the orbit spice kernel name if it is not in the list already
//      else if (find(orbitSpiceKernelNames.begin(), orbitSpiceKernelNames.end(),
//            value) == orbitSpiceKernelNames.end())
//         orbitSpiceKernelNames.push_back(value);
//      return true;

   default:
      return SpaceObject::SetStringParameter(id, value, index);
   }
}


//---------------------------------------------------------------------------
//  bool SetStringParameter(const wxString &label, const wxString &value,
//                          const Integer index)
//---------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//---------------------------------------------------------------------------
bool Spacecraft::SetStringParameter(const wxString &label,
                                    const wxString &value,
                                    const Integer index)
{
   return SetStringParameter(GetParameterID(label), value, index);
}

// todo: Comment these methods
//---------------------------------------------------------------------------
// const Rmatrix& GetRmatrixParameter(const Integer id) const
//---------------------------------------------------------------------------
const Rmatrix& Spacecraft::GetRmatrixParameter(const Integer id) const
{
   if (id == ORBIT_STM)
      return orbitSTM;

   if (id == ORBIT_A_MATRIX)
      return orbitAMatrix;

//   if (id == ORBIT_COVARIANCE)
//      return covariance;

   return SpaceObject::GetRmatrixParameter(id);
}

//---------------------------------------------------------------------------
// const Rmatrix& SetRmatrixParameter(const Integer id, const Rmatrix &value)
//---------------------------------------------------------------------------
const Rmatrix& Spacecraft::SetRmatrixParameter(const Integer id,
                                         const Rmatrix &value)
{
   if (id == ORBIT_STM)
   {
      orbitSTM = value;
      return orbitSTM;
   }

   if (id == ORBIT_A_MATRIX)
   {
      orbitAMatrix = value;
      return orbitAMatrix;
   }

//   if (id == ORBIT_COVARIANCE)
//   {
//      covariance = value;
//      return covariance;
//   }

   return SpaceObject::SetRmatrixParameter(id, value);
}

//---------------------------------------------------------------------------
// const Rmatrix& GetRmatrixParameter(const wxString &label) const
//---------------------------------------------------------------------------
const Rmatrix& Spacecraft::GetRmatrixParameter(const wxString &label) const
{
   return GetRmatrixParameter(GetParameterID(label));
}

//---------------------------------------------------------------------------
// const Rmatrix& SetRmatrixParameter(const wxString &label,
//                                    const Rmatrix &value)
//---------------------------------------------------------------------------
const Rmatrix& Spacecraft::SetRmatrixParameter(const wxString &label,
                                               const Rmatrix &value)
{
   return SetRmatrixParameter(GetParameterID(label), value);
}

//---------------------------------------------------------------------------
// Real GetRealParameter(const Integer id, const Integer row,
//                       const Integer col) const
//---------------------------------------------------------------------------
Real Spacecraft::GetRealParameter(const Integer id, const Integer row,
                                  const Integer col) const
{
   if (id == ORBIT_STM)
      return orbitSTM(row, col);

   if (id == ORBIT_A_MATRIX)
      return orbitAMatrix(row, col);

   return SpaceObject::GetRealParameter(id, row, col);
}

//---------------------------------------------------------------------------
// Real GetRealParameter(const wxString &label, const Integer row,
//                       const Integer col) const
//---------------------------------------------------------------------------
Real Spacecraft::GetRealParameter(const wxString &label,
                                  const Integer row,
                                  const Integer col) const
{
   return GetRealParameter(GetParameterID(label), row, col);
}

//---------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value,
//                       const Integer row, const Integer col)
//---------------------------------------------------------------------------
Real Spacecraft::SetRealParameter(const Integer id, const Real value,
                                  const Integer row, const Integer col)
{
   if (id == ORBIT_STM)
   {
      orbitSTM(row, col) = value;
      return orbitSTM(row, col);
   }

   if (id == ORBIT_A_MATRIX)
   {
      orbitAMatrix(row, col) = value;
      return orbitAMatrix(row, col);
   }

   return SpaceObject::SetRealParameter(id, value, row, col);
}

//---------------------------------------------------------------------------
// Real SetRealParameter(const wxString &label, const Real value, const Integer row,
//                       const Integer col)
//---------------------------------------------------------------------------
Real Spacecraft::SetRealParameter(const wxString &label,
                                      const Real value, const Integer row,
                                      const Integer col)
{
   return SetRealParameter(GetParameterID(label), value, row, col);
}

//---------------------------------------------------------------------------
//  Real SetRealParameter(const Integer id, const Real value, Integer index)
//---------------------------------------------------------------------------
/**
 * Set the value for a Real parameter.
 *
 * @param id The integer ID for the parameter.
 * @param value The new parameter value.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return the parameter value at the end of this call, or
 *         REAL_PARAMETER_UNDEFINED if the parameter id is invalid or the
 *         parameter type is not Real.
 */
//------------------------------------------------------------------------------
Real Spacecraft::SetRealParameter(const Integer id, const Real value,
                                  const Integer index)
{
   try
   {
      if (id >= ATTITUDE_ID_OFFSET)
      {
         if (attitude)
         {
            #ifdef DEBUG_SC_ATTITUDE
            MessageInterface::ShowMessage(
               wxT("------ Now calling attitude to SET real parameter for id =  %d")
               wxT(", index = %d,  and value = %12.10f\n"), id, index, value);
            #endif
            return attitude->SetRealParameter(id - ATTITUDE_ID_OFFSET, value, index);
         }
      }
   }
   catch (BaseException &)
   {
      return SpaceObject::SetRealParameter(id, value, index);
   }
   return SpaceObject::SetRealParameter(id, value, index);
}

//---------------------------------------------------------------------------
//  bool TakeAction(const wxString &action, const wxString &actionData)
//---------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//---------------------------------------------------------------------------
bool Spacecraft::TakeAction(const wxString &action,
                            const wxString &actionData)
{
   #ifdef DEBUG_SC_REF_OBJECT
   MessageInterface::ShowMessage
      (wxT("Entering SC<%p>'%s'::TakeAction with action = '%s', and actionData = ")
       wxT("'%s'\n"), this, GetName().c_str(), action.c_str(), actionData.c_str());
   #endif
   if (action == wxT("SetupHardware"))
   {
      AttachTanksToThrusters();

      #ifdef DEBUG_SC_REF_OBJECT
      MessageInterface::ShowMessage(wxT("Leaviong SC::TakeAction with true\n"));
      #endif
      return true;
   }

   if (action == wxT("RequireCartesianStateDynamics"))
   {
      ++includeCartesianState;
      return true;
   }

   if (action == wxT("ReleaseCartesianStateDynamics"))
   {
      --includeCartesianState;
      if (includeCartesianState < 0)
         includeCartesianState = 0;
      return true;
   }

   if ((action == wxT("RemoveHardware")) || (action == wxT("RemoveTank")) ||
       (action == wxT("RemoveThruster")))
   {
      bool removeTank = true, removeThruster = true, removeAll = false;
      if (action == wxT("RemoveTank"))
         removeThruster = false;
      if (action == wxT("RemoveThruster"))
         removeTank = false;
      if (actionData == wxT(""))
         removeAll = true;

      if (removeThruster)
      {
         if (removeAll)
         {
            DeleteOwnedObjects(false, false, true, false);
            thrusters.clear();
            thrusterNames.clear();
         }
         else
         {
            for (StringArray::iterator i = thrusterNames.begin();
                 i != thrusterNames.end(); ++i)
               if (*i == actionData)
                  thrusterNames.erase(i);
            for (ObjectArray::iterator i = thrusters.begin();
                 i != thrusters.end(); ++i)
               if ((*i)->GetName() == actionData)
               {
                  GmatBase *thr = (*i);
                  thrusters.erase(i);
                  #ifdef DEBUG_MEMORY
                  MemoryTracker::Instance()->Remove
                     (*i, (*i)->GetName(), wxT("Spacecraft::TakeAction()"),
                      wxT("deleting cloned Thruster"), this);
                  #endif
                  delete thr;
               }
         }
      }

      if (removeTank)
      {
         if (removeAll)
         {
            DeleteOwnedObjects(false, true, true, false);
            tanks.clear();
            tankNames.clear();
         }
         else
         {
            for (StringArray::iterator i = tankNames.begin();
                 i != tankNames.end(); ++i)
               if (*i == actionData)
                  tankNames.erase(i);
            for (ObjectArray::iterator i = tanks.begin();
                 i != tanks.end(); ++i)
               if ((*i)->GetName() == actionData)
               {
                  GmatBase *tnk = (*i);
                  tanks.erase(i);
                  #ifdef DEBUG_MEMORY
                  MemoryTracker::Instance()->Remove
                     (*i, (*i)->GetName(), wxT("Spacecraft::TakeAction()"),
                      wxT("deleting cloned Tanks"), this);
                  #endif
                  delete tnk;
               }
         }
      }

      #ifdef DEBUG_SC_REF_OBJECT
      MessageInterface::ShowMessage(wxT("Leaviong SC::TakeAction with true\n"));
      #endif
      return true;
   }

   if (action == wxT("ApplyCoordinateSystem"))
   {
      #ifdef DEBUG_SPACECRAFT_CS
      MessageInterface::ShowMessage
         (wxT("Spacecraft::TakeAction(ApplyCoordinateSystem) Calling StateConverter::SetMu(%p)\n"),
          coordinateSystem);
      #endif

      if (!stateConverter.SetMu(coordinateSystem))
      {
         throw SpaceObjectException(
            wxT("\nError:  Spacecraft has empty coordinate system\n"));
      }

      if (csSet == false)
      {
         Rvector6 st(state.GetState());
         #ifdef DEBUG_SPACECRAFT_CS
         MessageInterface::ShowMessage
            (wxT("Spacecraft::TakeAction() Calling SetStateFromRepresentation(%s, cartesianstate), ")
             wxT("since CS was not set()\n"), stateType.c_str());
         MessageInterface::ShowMessage
            (wxT(" ... at this point, the state = %s\n"), (st.ToString()).c_str());
         #endif
         try
         {
            SetStateFromRepresentation(stateType, st);
         }
         catch (BaseException &be)
         {
            wxString errmsg = wxT("Error applying coordinate system due to errors in spacecraft state. ");
            errmsg += be.GetFullMessage() + wxT("\n");
            throw SpaceObjectException(errmsg);
         }

         #ifdef DEBUG_SPACECRAFT_CS
         MessageInterface::ShowMessage
            (wxT("Spacecraft::TakeAction() setting csSet to true ........\n"));
         #endif
         csSet = true;
      }

      #ifdef DEBUG_SPACECRAFT_CS
      MessageInterface::ShowMessage(wxT("Spacecraft::TakeAction() returning true\n"));
      #endif
      return true;
   }

   // 6/12/06 - arg: reset scEpochStr to epoch from prop state
   if (action == wxT("UpdateEpoch"))
   {
      Real currEpoch = state.GetEpoch();

      if (epochSystem != wxT(""))
      {
         if (epochSystem != wxT("A1"))
            currEpoch = TimeConverterUtil::Convert(currEpoch,
                          TimeConverterUtil::A1,
                          TimeConverterUtil::GetTimeTypeID(epochSystem),
                          GmatTimeConstants::JD_JAN_5_1941);
      }

      if (epochFormat != wxT(""))
      {
         if (epochFormat == wxT("Gregorian"))
            scEpochStr = TimeConverterUtil::ConvertMjdToGregorian(currEpoch);
         else
         {
            wxString timestream;
            timestream << currEpoch;
            scEpochStr = timestream;
         }
      }
      #ifdef DEBUG_SC_EPOCHSTR
      MessageInterface::ShowMessage(wxT("In TakeAction, epochSystem = %s\n"),
      epochSystem.c_str());
      MessageInterface::ShowMessage(wxT("In TakeAction, epochFormat = %s\n"),
      epochFormat.c_str());
      MessageInterface::ShowMessage(wxT("In TakeAction, scEpochStr being set to %s\n"),
      scEpochStr.c_str());
      #endif

      return true;
   }

   if (action == wxT("ThrusterSettingMode"))
   {
      if (actionData == wxT("On"))
         isThrusterSettingMode = true;
      else
         isThrusterSettingMode = false;

      return true;
   }

   if (action == wxT("ResetSTM"))
   {
      orbitSTM(0,0) = orbitSTM(1,1) = orbitSTM(2,2) =
      orbitSTM(3,3) = orbitSTM(4,4) = orbitSTM(5,5) = 1.0;

      orbitSTM(0,1)=orbitSTM(0,2)=orbitSTM(0,3)=orbitSTM(0,4)=orbitSTM(0,5)=
      orbitSTM(1,0)=orbitSTM(1,2)=orbitSTM(1,3)=orbitSTM(1,4)=orbitSTM(1,5)=
      orbitSTM(2,0)=orbitSTM(2,1)=orbitSTM(2,3)=orbitSTM(2,4)=orbitSTM(2,5)=
      orbitSTM(3,0)=orbitSTM(3,1)=orbitSTM(3,2)=orbitSTM(3,4)=orbitSTM(3,5)=
      orbitSTM(4,0)=orbitSTM(4,1)=orbitSTM(4,2)=orbitSTM(4,3)=orbitSTM(4,5)=
      orbitSTM(5,0)=orbitSTM(5,1)=orbitSTM(5,2)=orbitSTM(5,3)=orbitSTM(5,4)
            = 0.0;
   }

   if (action == wxT("ResetAMatrix"))
   {
      orbitAMatrix(0,0) = orbitAMatrix(1,1) = orbitAMatrix(2,2) =
      orbitAMatrix(3,3) = orbitAMatrix(4,4) = orbitAMatrix(5,5) = 1.0;

      orbitAMatrix(0,1) = orbitAMatrix(0,2) = orbitAMatrix(0,3) =
      orbitAMatrix(0,4) = orbitAMatrix(0,5) = orbitAMatrix(1,0) =
      orbitAMatrix(1,2) = orbitAMatrix(1,3) = orbitAMatrix(1,4) =
      orbitAMatrix(1,5) = orbitAMatrix(2,0) = orbitAMatrix(2,1) =
      orbitAMatrix(2,3) = orbitAMatrix(2,4) = orbitAMatrix(2,5) =
      orbitAMatrix(3,0) = orbitAMatrix(3,1) = orbitAMatrix(3,2) =
      orbitAMatrix(3,4) = orbitAMatrix(3,5) = orbitAMatrix(4,0) =
      orbitAMatrix(4,1) = orbitAMatrix(4,2) = orbitAMatrix(4,3) =
      orbitAMatrix(4,5) = orbitAMatrix(5,0) = orbitAMatrix(5,1) =
      orbitAMatrix(5,2) = orbitAMatrix(5,3) = orbitAMatrix(5,4) = 0.0;
   }

   return SpaceObject::TakeAction(action, actionData);
}


//---------------------------------------------------------------------------
// bool IsOwnedObject(Integer id) const;
//---------------------------------------------------------------------------
bool Spacecraft::IsOwnedObject(Integer id) const
{
   if (id == ATTITUDE)
      return true;
   else
      return false;
}


//---------------------------------------------------------------------------
// GmatBase* GetOwnedObject(Integer whichOne)
//---------------------------------------------------------------------------
GmatBase* Spacecraft::GetOwnedObject(Integer whichOne)
{
   // only one owned object at the moment
   if (attitude)
      return attitude;

   return NULL;
}


//---------------------------------------------------------------------------
//  bool Initialize()
//---------------------------------------------------------------------------
/**
 * Initialize the default values of spacecraft information.
 *
 * @return always success unless the coordinate system is empty
 */
//---------------------------------------------------------------------------
bool Spacecraft::Initialize()
{
   #ifdef DEBUG_SPACECRAFT_CS
   MessageInterface::ShowMessage
      (wxT("Spacecraft::Initialize() entered ---------- this=<%p> '%s'\n   ")
       wxT("internalCoordSystem=<%p> '%s', coordinateSystem=<%p> '%s'\n"), this,
       GetName().c_str(), internalCoordSystem,
       internalCoordSystem ? internalCoordSystem->GetName().c_str() : wxT("NULL"),
       coordinateSystem, coordinateSystem ? coordinateSystem->GetName().c_str() : wxT("NULL"));
   MessageInterface::ShowMessage
      (wxT("   stateType=%s, state=\n   %.9f, %.9f, %.9f, %.14f, %.14f, %f.14\n"),
       stateType.c_str(), state[0], state[1], state[2], state[3],
       state[4], state[5]);
   #endif

   // Set the mu if CelestialBody is there through coordinate system's origin;
   // Otherwise, discontinue process and send the error message
   if (!stateConverter.SetMu(coordinateSystem))
   {
      throw SpaceObjectException(wxT("Spacecraft has empty coordinate system"));
   }
   if (!attitude)
   {
      #ifdef DEBUG_SC_ATTITUDE
      MessageInterface::ShowMessage(wxT("Spacecraft %s has no defined attitude object.\n"),
                     instanceName.c_str());
      #endif
      throw SpaceObjectException(wxT("Spacecraft has no attitude set."));
   }
   else
   {
      #ifdef DEBUG_SC_ATTITUDE
         MessageInterface::ShowMessage(
         wxT("Initializing attitude object for spacecraft %s\n"),
         instanceName.c_str());
      #endif

#ifdef __USE_SPICE__
      if (attitude->IsOfType(wxT("SpiceAttitude")))
      {
         #ifdef DEBUG_SPICE_KERNELS
         MessageInterface::ShowMessage(wxT("About to set %d CK kernels on spiceAttitude\n"),
               attitudeSpiceKernelNames.size());
         MessageInterface::ShowMessage(wxT("About to set %d SCLK kernels on spiceAttitude\n"),
               scClockSpiceKernelNames.size());
         MessageInterface::ShowMessage(wxT("About to set %d FK kernels on spiceAttitude\n"),
               frameSpiceKernelNames.size());
         #endif
         SpiceAttitude *spiceAttitude = (SpiceAttitude*) attitude;
         spiceAttitude->SetObjectID(instanceName, naifId, naifIdRefFrame);
         for (Integer ii = 0; ii < (Integer) attitudeSpiceKernelNames.size(); ii++)
            spiceAttitude->SetStringParameter(wxT("AttitudeKernelName"), attitudeSpiceKernelNames[ii], ii);
         for (Integer ii = 0; ii < (Integer) scClockSpiceKernelNames.size(); ii++)
            spiceAttitude->SetStringParameter(wxT("SCClockKernelName"), scClockSpiceKernelNames[ii], ii);
         for (Integer ii = 0; ii < (Integer) frameSpiceKernelNames.size(); ii++)
            spiceAttitude->SetStringParameter(wxT("FrameKernelName"), frameSpiceKernelNames[ii], ii);
      }
#endif
      attitude->Initialize();
      #ifdef DEBUG_SC_ATTITUDE
         MessageInterface::ShowMessage(
         wxT("***Finished initializing attitude object for spacecraft %s\n"),
         instanceName.c_str());
      #endif
   }

   #ifdef DEBUG_HARDWARE
      MessageInterface::ShowMessage(wxT("Hardware list names:\n"));
      for (UnsignedInt i = 0; i < hardwareNames.size(); ++i)
      {
         MessageInterface::ShowMessage(wxT("   %s\n"), hardwareNames[i].c_str());
      }

      MessageInterface::ShowMessage(wxT("Hardware list objects:\n"));
      for (UnsignedInt i = 0; i < hardwareList.size(); ++i)
      {
         MessageInterface::ShowMessage(wxT("   %s\n"), hardwareList[i]->GetName().c_str());
      }
   #endif

   // Set the hardware interconnections
   for (ObjectArray::iterator i=hardwareList.begin(); i!=hardwareList.end(); ++i)
   {
      if ((*i)->IsOfType(Gmat::HARDWARE))
      {
         Hardware *current = (Hardware*)(*i);

         // Get the hardware reference list
         StringArray refs = current->GetRefObjectNameArray(Gmat::UNKNOWN_OBJECT);
         for (UnsignedInt j = 0; j < refs.size(); ++j)
         {
            #ifdef DEBUG_HARDWARE
               MessageInterface::ShowMessage(wxT("Connecting up %s for %s\n"),
                     refs[j].c_str(), current->GetName().c_str());
            #endif

            for (UnsignedInt k = 0; k < hardwareList.size(); ++k)
            {
               if (hardwareList[k]->GetName() == refs[j])
                  current->SetRefObject(hardwareList[k],
                        hardwareList[k]->GetType(), hardwareList[k]->GetName());
            }
         }
      }
   }


   // made changes by Tuan Nguyen
   // Verify all Spacecarft's referenced objects:
   if (VerifyAddHardware() == false)            // verify added hardware
           return false;

   #ifdef DEBUG_SPACECRAFT_CS
      MessageInterface::ShowMessage(wxT("Spacecraft::Initialize() exiting ----------\n"));
   #endif

   return true;
}


//------------------------------------------------------------------------------
// wxString GetEpochString()
//------------------------------------------------------------------------------
wxString Spacecraft::GetEpochString()
{
   Real outMjd = -999.999;
   wxString outStr;

   TimeConverterUtil::Convert(wxT("A1ModJulian"), GetEpoch(), wxT(""),
                              epochType, outMjd, outStr);

   return outStr;
}


//------------------------------------------------------------------------------
// void SetDateFormat(const wxString &dateType)
//------------------------------------------------------------------------------
/**
 * Sets the output date format of epoch.
 *
 * @param <dateType> date type given.
 */
//------------------------------------------------------------------------------
void Spacecraft::SetDateFormat(const wxString &dateType)
{
   #ifdef DEBUG_DATE_FORMAT
      MessageInterface::ShowMessage(wxT("Spacecraft::SetDateFormat() ")
         wxT("Setting date format to %s; initial epoch is %s\n"),
         dateType.c_str(), scEpochStr.c_str());
   #endif

   epochType = dateType;
   scEpochStr = GetEpochString();

}


//------------------------------------------------------------------------------
//  void SetEpoch(wxString ep)
//------------------------------------------------------------------------------
/**
 * Set the epoch.
 *
 * @param <ep> The new epoch.
 */
//------------------------------------------------------------------------------
void Spacecraft::SetEpoch(const wxString &ep)
{
   #ifdef DEBUG_DATE_FORMAT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::SetEpoch() Setting epoch  for spacecraft %s to %s\n"),
       instanceName.c_str(), ep.c_str());
   #endif

   scEpochStr = ep;

   Real fromMjd = -999.999;
   Real outMjd = -999.999;
   wxString outStr;

   TimeConverterUtil::Convert(epochType, fromMjd, ep, wxT("A1ModJulian"), outMjd,
                              outStr);

   if (outMjd != -999.999)
   {
      RecomputeStateAtEpoch(outMjd);
      state.SetEpoch(outMjd);
      if (attitude) attitude->SetEpoch(outMjd);
   }
   else
   {
      #ifdef DEBUG_DATE_FORMAT
      MessageInterface::ShowMessage(wxT("Spacecraft::SetEpoch() oops!  outMjd = -999.999!!\n"));
      #endif
   }

}


//------------------------------------------------------------------------------
// void SetEpoch(const wxString &type, const wxString &ep, Real a1mjd)
//------------------------------------------------------------------------------
/*
 * Sets output epoch type, system, format, and epoch.  No conversion is done here.
 *
 * @param  type   epoch type to be used for output (TAIModJulian, TTGregorian, etc)
 * @param  epoch  epoch string
 * @param  a1mjd  epoch in TAIModJulian format
 */
//------------------------------------------------------------------------------
void Spacecraft::SetEpoch(const wxString &type, const wxString &ep, Real a1mjd)
{
   #ifdef DEBUG_SC_EPOCHSTR
   MessageInterface::ShowMessage(wxT("In SC::SetEpoch, type = %s, ep = %s, a1mjd = %.12f\n"),
   type.c_str(), ep.c_str(), a1mjd);
   #endif
   TimeConverterUtil::GetTimeSystemAndFormat(type, epochSystem, epochFormat);
   epochType = type;
   scEpochStr = ep;
   RecomputeStateAtEpoch(a1mjd);
   state.SetEpoch(a1mjd);
   if (attitude) attitude->SetEpoch(a1mjd);
   #ifdef DEBUG_SC_EPOCHSTR
   MessageInterface::ShowMessage(wxT("and in SC::SetEpoch, epochSystem = %s, epochFormat = %s\n"),
   epochSystem.c_str(), epochFormat.c_str());
   #endif
}


//------------------------------------------------------------------------------
// void  SetState(const wxString &type, const Rvector6 &cartState)
//------------------------------------------------------------------------------
/*
 * Sets output state type and state in cartesian representation. Just set to
 * internal cartesian state. No conversion is done here.
 *
 * @param  type       state type to be used for output
 * @param  cartState  cartesian state to set as internal state
 */
//------------------------------------------------------------------------------
void Spacecraft::SetState(const wxString &type, const Rvector6 &cartState)
{
   #ifdef DEBUG_SPACECRAFT_SET
   MessageInterface::ShowMessage
      (wxT("Spacecraft::SetState() type=%s, cartState=\n   %s\n"), type.c_str(),
       cartState.ToString().c_str());
   #endif

   //stateType = type;
   displayStateType = type;
   SetState(cartState[0], cartState[1], cartState[2],
            cartState[3], cartState[4], cartState[5]);
   UpdateElementLabels();
}


//------------------------------------------------------------------------------
// void Spacecraft::SetAnomaly(const wxString &type, const Anomaly &ta)
//------------------------------------------------------------------------------
/*
 * Sets anomaly type and input true anomaly to internal true anomaly
 *
 * @param  type  Anomaly type (wxT("TA"), wxT("MA"), wxT("EA, ")HAwxT(" or full name like ")True Anomaly")
 * @param  ta    True anomaly to set as internal true anomaly
 */
//------------------------------------------------------------------------------
void Spacecraft::SetAnomaly(const wxString &type, const Anomaly &ta)
{
   trueAnomaly = ta;
   anomalyType = Anomaly::GetTypeString(type); // why call a static here?? - wcs
   // wcs 2007.05.18 - don't assume - only set the label if it's appropriate
   if (displayStateType == wxT("Keplerian") || displayStateType == wxT("ModifiedKeplerian"))
   stateElementLabel[5] = anomalyType;     // this assumes current display type is Keplerian/ModKep??

   #ifdef DEBUG_SPACECRAFT_SET
   MessageInterface::ShowMessage
      (wxT("Spacecraft::SetAnomaly() anomalyType=%s, value=%f\n"), anomalyType.c_str(),
       trueAnomaly.GetValue());
   MessageInterface::ShowMessage
      (wxT("Spacecraft::SetAnomaly() stateElementLabel[5] = %s\n"),
      stateElementLabel[5].c_str());
   #endif
}

// todo: comment methods
//------------------------------------------------------------------------------
// Integer Spacecraft::GetPropItemID(const wxString &whichItem)
//------------------------------------------------------------------------------
Integer Spacecraft::GetPropItemID(const wxString &whichItem)
{
   if (whichItem == wxT("CartesianState"))
      return Gmat::CARTESIAN_STATE;
   if (whichItem == wxT("STM"))
      return Gmat::ORBIT_STATE_TRANSITION_MATRIX;
   if (whichItem == wxT("AMatrix"))
      return Gmat::ORBIT_A_MATRIX;

   return SpaceObject::GetPropItemID(whichItem);
}

//------------------------------------------------------------------------------
// Integer SetPropItem(const wxString &propItem)
//------------------------------------------------------------------------------
Integer Spacecraft::SetPropItem(const wxString &propItem)
{
   if (propItem == wxT("CartesianState"))
      return Gmat::CARTESIAN_STATE;
   if (propItem == wxT("STM"))
      return Gmat::ORBIT_STATE_TRANSITION_MATRIX;
   if (propItem == wxT("AMatrix"))
      return Gmat::ORBIT_A_MATRIX;
   if (propItem == wxT("MassFlow"))
      if (tanks.size() > 0)
         return Gmat::MASS_FLOW;

   return SpaceObject::SetPropItem(propItem);
}


//------------------------------------------------------------------------------
// StringArray GetDefaultPropItems()
//------------------------------------------------------------------------------
StringArray Spacecraft::GetDefaultPropItems()
{
   StringArray defaults = SpaceObject::GetDefaultPropItems();
   defaults.push_back(wxT("CartesianState"));
   return defaults;
}


//------------------------------------------------------------------------------
// Real* GetPropItem(const Integer item)
//------------------------------------------------------------------------------
Real* Spacecraft::GetPropItem(const Integer item)
{
   Real* retval = NULL;
   switch (item)
   {
      case Gmat::CARTESIAN_STATE:
         retval = state.GetState();
         break;

      case Gmat::ORBIT_STATE_TRANSITION_MATRIX:
//         retval = stm;
         break;

      case Gmat::ORBIT_A_MATRIX:
         break;

      case Gmat::MASS_FLOW:
         // todo: Access tanks for mass information to handle mass flow
         break;

      // All other values call up the class heirarchy
      default:
         retval = SpaceObject::GetPropItem(item);
   }

   return retval;
}

//------------------------------------------------------------------------------
// Integer GetPropItemSize(const Integer item)
//------------------------------------------------------------------------------
Integer Spacecraft::GetPropItemSize(const Integer item)
{
   Integer retval = -1;
   switch (item)
   {
      case Gmat::CARTESIAN_STATE:
         retval = state.GetSize();
         break;

      case Gmat::ORBIT_STATE_TRANSITION_MATRIX:
         retval = 36;
         break;

      case Gmat::ORBIT_A_MATRIX:
         retval = 36;
         break;
      case Gmat::MASS_FLOW:
         // todo: Access tanks for mass information to handle mass flow

         // For now, only allow one tank
         retval = 1;
         break;

      // All other values call up the hierarchy
      default:
         retval = SpaceObject::GetPropItemSize(item);
   }

   return retval;
}

bool Spacecraft::PropItemNeedsFinalUpdate(const Integer item)
{
   switch (item)
   {
      case Gmat::ORBIT_STATE_TRANSITION_MATRIX:
      case Gmat::ORBIT_A_MATRIX:
         return true;

      case Gmat::CARTESIAN_STATE:
      case Gmat::MASS_FLOW:
         return false;

      // All other values call up the hierarchy
      default:
         ;        // Intentional drop through
   }

   return SpaceObject::PropItemNeedsFinalUpdate(item);
}


//------------------------------------------------------------------------------
// bool IsEstimationParameterValid(const Integer item)
//------------------------------------------------------------------------------
bool Spacecraft::IsEstimationParameterValid(const Integer item)
{
   bool retval = false;

   Integer id = item - type * ESTIMATION_TYPE_ALLOCATION;

   switch (id)
   {
      case Gmat::CARTESIAN_STATE:
         retval = true;
         break;

      case Gmat::MASS_FLOW:
         // todo: Access tanks for mass information to handle mass flow
         break;

      // All other values call up the hierarchy
      default:
         retval = SpaceObject::IsEstimationParameterValid(item);
   }

   return retval;
}

//------------------------------------------------------------------------------
// Integer GetEstimationParameterSize(const Integer item)
//------------------------------------------------------------------------------
Integer Spacecraft::GetEstimationParameterSize(const Integer item)
{
   Integer retval = 1;


   Integer id = item - type * ESTIMATION_TYPE_ALLOCATION;

   #ifdef DEBUG_ESTIMATION
      MessageInterface::ShowMessage(wxT("Spacecraft::GetEstimationParameterSize(%d)")
            wxT(" called; parameter ID is %d\n"), item, id);
   #endif


   switch (id)
   {
      case CARTESIAN_X:
         retval = 6;
         break;

      case Gmat::MASS_FLOW:
         // todo: Access tanks for mass information to handle mass flow
         break;

      // All other values call up the hierarchy
      default:
         retval = SpaceObject::GetEstimationParameterSize(item);
   }

   return retval;
}

//------------------------------------------------------------------------------
// Real* GetEstimationParameterValue(const Integer item)
//------------------------------------------------------------------------------
Real* Spacecraft::GetEstimationParameterValue(const Integer item)
{
   Real* retval = NULL;

   Integer id = item - type * ESTIMATION_TYPE_ALLOCATION;

   switch (id)
   {
      case CARTESIAN_X:
         retval = state.GetState();
         break;

//      case Gmat::MASS_FLOW:
//         // todo: Access tanks for mass information to handle mass flow
//         break;

      // All other values call up the class heirarchy
      default:
         retval = SpaceObject::GetEstimationParameterValue(item);
   }

   return retval;
}


//-------------------------------------
// protected methods
//-------------------------------------


//------------------------------------------------------------------------------
//  void UpdateTotalMass()
//------------------------------------------------------------------------------
/**
 * Updates the total mass by adding all hardware masses to the dry mass.
 */
//------------------------------------------------------------------------------
Real Spacecraft::UpdateTotalMass()
{
   #ifdef DEBUG_UPDATE_TOTAL_MASS
   MessageInterface::ShowMessage
      (wxT("Spacecraft::UpdateTotalMass() <%p>'%s' entered, dryMass=%f\n"),
       this, GetName().c_str(), dryMass);
   #endif

   totalMass = dryMass;
   for (ObjectArray::iterator i = tanks.begin(); i < tanks.end(); ++i)
   {
      totalMass += (*i)->GetRealParameter(wxT("FuelMass"));
   }

   #ifdef DEBUG_UPDATE_TOTAL_MASS
   MessageInterface::ShowMessage
      (wxT("Spacecraft::UpdateTotalMass() <%p>'%s' returning %f\n"), this,
       GetName().c_str(), totalMass);
   #endif

   return totalMass;
}


//------------------------------------------------------------------------------
//  Real UpdateTotalMass() const
//------------------------------------------------------------------------------
/**
 * Calculates the total mass by adding all hardware masses to the dry mass.
 *
 * This method is const (so const methods can obtain the value), and therefore
 * does not update the internal data member.
 *
 * @return The mass of the spacecraft plus the mass of the fuel in the tanks.
 */
//------------------------------------------------------------------------------
Real Spacecraft::UpdateTotalMass() const
{
   #ifdef DEBUG_UPDATE_TOTAL_MASS
   MessageInterface::ShowMessage
      (wxT("Spacecraft::UpdateTotalMass() const <%p>'%s' entered, dryMass=%f, ")
       wxT("attached tank count = %d\n"), this, GetName().c_str(), dryMass,
       tanks.size());
   #endif

   Real tmass = dryMass;
   for (ObjectArray::const_iterator i = tanks.begin(); i < tanks.end(); ++i)
   {
      tmass += (*i)->GetRealParameter(wxT("FuelMass"));
   }

   #ifdef DEBUG_UPDATE_TOTAL_MASS
   MessageInterface::ShowMessage
      (wxT("Spacecraft::UpdateTotalMass() const <%p>'%s' returning %f\n"), this,
       GetName().c_str(), tmass);
   #endif

   return tmass;
}


//------------------------------------------------------------------------------
// bool ApplyTotalMass(Real newMass)
//------------------------------------------------------------------------------
/**
 * Adjusts the mass in the fuel tanks, based on the active thrusters, to a new
 * value
 *
 * @param newMass The new total mass
 *
 * @return true if the mass was adjusted to the new value
 *
 * @note This method applies a new total mass after propagating through a
 * maneuver.  This aspect of the design will need to change once multiple tanks
 * are used in maneuvers so that hte proportionate draw on the tanks is modeled
 * correctly.
 */
//------------------------------------------------------------------------------
bool Spacecraft::ApplyTotalMass(Real newMass)
{
   bool retval = true;
   Real massChange = newMass - UpdateTotalMass();

   #ifdef DEBUG_MASS_FLOW
      MessageInterface::ShowMessage(wxT("Mass change = %.12le; depeting "), massChange);
   #endif

   // Find the active thruster(s)
   ObjectArray active;
   RealArray   flowrate;
   Real        totalFlow = 0.0, rate;
   for (ObjectArray::iterator i = thrusters.begin(); i != thrusters.end(); ++i)
   {
      if ((*i)->GetBooleanParameter(wxT("IsFiring")))
      {
         active.push_back(*i);
         rate = ((Thruster*)(*i))->CalculateMassFlow();
         flowrate.push_back(rate);
         totalFlow += rate;
      }
   }

   // Divide the mass flow evenly between the tanks on each active thruster
   Real numberFiring = active.size();
   if ((numberFiring <= 0) && (massChange != 0.0))
   {
      wxString errmsg;
      errmsg << wxT("Mass update ") << massChange
             << wxT(" requested but there are no active thrusters");
      throw SpaceObjectException(errmsg);
   }

   Real dm;  // = massChange / numberFiring;
   for (UnsignedInt i = 0; i < active.size(); ++i)
   {
      // Change the mass in each attached tank
      ObjectArray usedTanks = active[i]->GetRefObjectArray(Gmat::HARDWARE);
      dm = massChange * flowrate[i] / totalFlow;

      #ifdef DEBUG_MASS_FLOW
         MessageInterface::ShowMessage(wxT("%.12le from %s = [ "), dm, active[i]->GetName().c_str());
      #endif

      Real dmt = dm / usedTanks.size();
      for (ObjectArray::iterator j = usedTanks.begin();
            j != usedTanks.end(); ++j)
      {
         #ifdef DEBUG_MASS_FLOW
            MessageInterface::ShowMessage(wxT(" %.12le "), dmt);
         #endif
         (*j)->SetRealParameter(wxT("FuelMass"),
               (*j)->GetRealParameter(wxT("FuelMass")) + dmt);
      }
      #ifdef DEBUG_MASS_FLOW
               MessageInterface::ShowMessage(wxT(" ] "));
      #endif
   }
   #ifdef DEBUG_MASS_FLOW
      MessageInterface::ShowMessage(wxT("\n"));
   #endif

   return retval;
}


//------------------------------------------------------------------------------
// void DeleteOwnedObjects(bool deleteAttitude, bool deleteTanks, bool deleteThrusters)
//------------------------------------------------------------------------------
/*
 * Deletes owned objects, such as attitude, tanks, and thrusters
 */
//------------------------------------------------------------------------------
void Spacecraft::DeleteOwnedObjects(bool deleteAttitude, bool deleteTanks,
                                    bool deleteThrusters, bool otherHardware)
{
   #ifdef DEBUG_DELETE_OWNED_OBJ
   MessageInterface::ShowMessage
      (wxT("Spacecraft::DeleteOwnedObjects() <%p>'%s' entered, deleteAttitude=%d, ")
       wxT("deleteTanks=%d, deleteThrusters=%d, otherHardware=%d\n"), this,
       GetName().c_str(), deleteAttitude, deleteTanks, deleteThrusters,
       otherHardware);
   #endif

   // delete attitude
   if (deleteAttitude)
   {
      if (attitude)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (attitude, wxT("attitude"), wxT("Spacecraft::DeleteOwnedObjects()"),
             wxT("deleting attitude of ") + GetName(), this);
         #endif
         delete attitude;
         ownedObjectCount--;
         #ifdef DEBUG_SC_OWNED_OBJECT
         MessageInterface::ShowMessage
            (wxT("Spacecraft::DeleteOwnedObjects() <%p>'%s' ownedObjectCount=%d\n"),
             this, GetName().c_str(), ownedObjectCount);
         #endif
      }
   }

   // delete tanks
   if (deleteTanks)
   {
      for (ObjectArray::iterator i = tanks.begin(); i < tanks.end(); ++i)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (*i, (*i)->GetName(), wxT("Spacecraft::DeleteOwnedObjects()"),
             wxT("deleting cloned Tank"), this);
         #endif
         delete *i;
      }
      tanks.clear();
   }

   // delete thrusters
   if (deleteThrusters)
   {
      for (ObjectArray::iterator i = thrusters.begin(); i < thrusters.end(); ++i)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (*i, (*i)->GetName(), wxT("Spacecraft::DeleteOwnedObjects()"),
             wxT("deleting cloned Thruster"), this);
         #endif
         delete *i;
      }
      thrusters.clear();
   }

   // Delete other hardware
   if (otherHardware)
   {
      for (ObjectArray::iterator i = hardwareList.begin();
            i < hardwareList.end(); ++i)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (*i, (*i)->GetName(), wxT("Spacecraft::DeleteOwnedObjects()"),
             wxT("deleting cloned Hardware"), this);
         #endif
         delete *i;
      }
      hardwareList.clear();
   }

   #ifdef DEBUG_DELETE_OWNED_OBJ
   MessageInterface::ShowMessage
      (wxT("Spacecraft::DeleteOwnedObjects() <%p>'%s' leaving, tanks.size()=%d, ")
       wxT("thrusters.size()=%d, hardwareLise.size()=%d\n"), this, GetName().c_str(),
       tanks.size(), thrusters.size(), hardwareList.size());
   #endif
}


//------------------------------------------------------------------------------
// void CloneOwnedObjects(Attitude *att, const ObjectArray &tnks, const ObjectArray &thrs)
//------------------------------------------------------------------------------
/*
 * Clones input tanks and thrusters set as attached hardware.
 */
//------------------------------------------------------------------------------
void Spacecraft::CloneOwnedObjects(Attitude *att, const ObjectArray &tnks,
                                   const ObjectArray &thrs)
{
   #ifdef DEBUG_OBJ_CLONE
   MessageInterface::ShowMessage
      (wxT("Spacecraft::CloneOwnedObjects() <%p>'%s' entered, att=<%p>, tank count = %d,")
       wxT(" thruster count = %d\n"), this, GetName().c_str(), att, tnks.size(), thrs.size());
   #endif

   attitude = NULL;

   // clone the attitude
   if (att)
   {
      attitude = (Attitude*) att->Clone();
      attitude->SetEpoch(state.GetEpoch());
      ownedObjectCount++;
      #ifdef DEBUG_SC_OWNED_OBJECT
      MessageInterface::ShowMessage
         (wxT("Spacecraft::CloneOwnedObjects() <%p>'%s' ownedObjectCount=%d\n"),
          this, GetName().c_str(), ownedObjectCount);
      #endif
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Add
         (attitude, wxT("cloned attitude"), wxT("Spacecraft::CloneOwnedObjects()"),
          wxT("attitude = (Attitude*) att->Clone()"), this);
      #endif
   }

   // handle tanks
   if (tnks.size() > 0)
   {
      for (UnsignedInt i=0; i<tnks.size(); i++)
      {
         // clone the tanks here
         GmatBase *clonedTank = (tnks[i])->Clone();
         tanks.push_back(clonedTank);
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Add
            (clonedTank, clonedTank->GetName(), wxT("Spacecraft::CloneOwnedObjects()"),
             wxT("clonedTank = (tnks[i])->Clone()"), this);
         #endif
      }
   }

   // handle thrusters
   if (thrs.size() > 0)
   {
      for (UnsignedInt i=0; i<thrs.size(); i++)
      {
         // clone the thrusters here
         GmatBase *clonedObj = (thrs[i])->Clone();
         thrusters.push_back(clonedObj);
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Add
            (clonedObj, clonedObj->GetName(), wxT("Spacecraft::CloneOwnedObjects()"),
             wxT("clonedObj = (thrs[i])->Clone()"), this);
         #endif

         #ifdef DEBUG_OBJ_CLONE
         MessageInterface::ShowMessage
            (wxT("Spacecraft::CloneOwnedObjects() Setting ref objects to ")
             wxT("thruster<%p>'%s'\n"), clonedObj, clonedObj->GetName().c_str());
         #endif

         // Set ref. objects to cloned Thruster
         clonedObj->SetSolarSystem(solarSystem);
         clonedObj->SetRefObject(this, Gmat::SPACECRAFT, GetName());

         // Set Thruster's CoordinateSystem
         wxString thrCsName = clonedObj->GetRefObjectName(Gmat::COORDINATE_SYSTEM);
         if (coordSysMap.find(thrCsName) != coordSysMap.end())
            clonedObj->SetRefObject(coordSysMap[thrCsName], Gmat::COORDINATE_SYSTEM, thrCsName);
      }
   }

   if (tnks.size() > 0 && thrs.size() > 0)
   {
      #ifdef DEBUG_OBJ_CLONE
      MessageInterface::ShowMessage
         (wxT("Spacecraft::CloneOwnedObjects() calling AttachTanksToThrusters()\n"));
      #endif
      AttachTanksToThrusters();
   }
}


//--------------------------------------------------------------------
// void AttachTanksToThrusters()
//--------------------------------------------------------------------
void Spacecraft::AttachTanksToThrusters()
{
   #ifdef DEBUG_SC_REF_OBJECT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::AttachTanksToThrusters() this=<%p>'%s' entered\n"),
       this, GetName().c_str());
   #endif

   // Attach tanks to thrusters
   StringArray tankNommes;
   GmatBase *tank;

   #ifdef DEBUG_SC_REF_OBJECT
   MessageInterface::ShowMessage
      (wxT("   tanks.size()=%d, thrusters.size()=%d\n"), tanks.size(), thrusters.size());
   #endif

   for (ObjectArray::iterator i = thrusters.begin(); i < thrusters.end(); ++i)
   {
      tankNommes = (*i)->GetStringArrayParameter(wxT("Tank"));

      #ifdef DEBUG_SC_REF_OBJECT
      MessageInterface::ShowMessage
         (wxT("   go through %d tank(s) in the thruster <%p>'%s'\n"), tankNommes.size(),
          (*i), (*i)->GetName().c_str());
      #endif

      for (StringArray::iterator j = tankNommes.begin();
           j < tankNommes.end(); ++j)
      {
         // Look up the tank in the hardware list
         #ifdef DEBUG_SC_REF_OBJECT
         MessageInterface::ShowMessage
            (wxT("   Checking to see if '%s' is in the spacecraft tank list\n"),
             (*j).c_str());
         #endif
         tank = NULL;
         for (ObjectArray::iterator k = tanks.begin(); k < tanks.end(); ++k)
            if ((*k)->GetName() == *j)
            {
               tank = *k;
               break;
            }

         if (tank)
         {
            #ifdef DEBUG_SC_REF_OBJECT
            MessageInterface::ShowMessage
               (wxT("   Setting the tank <%p>'%s' to the thruster <%p>'%s'\n"),
                tank, tank->GetName().c_str(), (*i), (*i)->GetName().c_str());
            #endif
            (*i)->SetRefObject(tank, tank->GetType(), tank->GetName());
         }
         else
            throw SpaceObjectException
               (wxT("Cannot find tank \"") + (*j) + wxT("\" in spacecraft \"") +
                instanceName + wxT("\"\n"));
      }
   }

   #ifdef DEBUG_SC_REF_OBJECT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::AttachTanksToThrusters() exiting\n"));
   #endif
}


//---------------------------------------------------------------------------
// bool SetHardware(GmatBase *obj, StringArray &hwNames, ObjectArray &hwArray)
//---------------------------------------------------------------------------
bool Spacecraft::SetHardware(GmatBase *obj, StringArray &hwNames,
                             ObjectArray &hwArray)
{
   wxString objType = obj->GetTypeName();
   wxString objName = obj->GetName();

   #ifdef DEBUG_SC_REF_OBJECT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::SetHardware() <%p>'%s' entered, obj=<%p>'%s'\n"), this,
       GetName().c_str(), obj, objType.c_str(), objName.c_str());
   MessageInterface::ShowMessage
      (wxT("   There are %d hw name(s) and %d hw pointer(s)\n"), hwNames.size(), hwArray.size());
   for (UnsignedInt i=0; i<hwNames.size(); i++)
      MessageInterface::ShowMessage(wxT("   hwNames[%d] = '%s'\n"), i, hwNames[i].c_str());
   for (UnsignedInt i=0; i<hwArray.size(); i++)
      MessageInterface::ShowMessage(wxT("   hwArray[%d] = <%p>\n"), i, hwArray[i]);
   #endif

   // if not adding the same hardware
   if (find(hwArray.begin(), hwArray.end(), obj) == hwArray.end())
   {
      #ifdef DEBUG_SC_REF_OBJECT
      MessageInterface::ShowMessage
         (wxT("   Now checking for the hardware name '%s'\n"), objName.c_str());
      #endif

      // if hardware name found
      if (find(hwNames.begin(), hwNames.end(), objName) != hwNames.end())
      {
         #ifdef DEBUG_SC_REF_OBJECT
         MessageInterface::ShowMessage
            (wxT("      The hardware name '%s' found\n"), objName.c_str());
         #endif

         for (ObjectArray::iterator i = hwArray.begin(); i != hwArray.end(); ++i)
         {
            #ifdef DEBUG_SC_REF_OBJECT
            MessageInterface::ShowMessage
               (wxT("      Now Checking if '%s' == '%s'\n"), (*i)->GetName().c_str(), objName.c_str());
            #endif

            // check if same name found, delete it first since it was cloned
            if ((*i)->GetName() == objName)
            {
               // delete the old one
               GmatBase *old = (*i);
               hwArray.erase(i);
               #ifdef DEBUG_MEMORY
               MemoryTracker::Instance()->Remove
                  (old, old->GetName(), wxT("Spacecraft::SetHardware()"),
                   wxT("deleting old cloned ") + objType, this);
               #endif
               delete old;
               old = NULL;
               break;
            }
         }

         // clone and push the hardware to the list
         GmatBase *clonedObj = obj->Clone();
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Add
            (clonedObj, clonedObj->GetName(), wxT("Spacecraft::SetHardware()"),
             wxT("GmatBase *cloneObj = obj->Clone()"), this);
         #endif

         hwArray.push_back(clonedObj);

         #ifdef DEBUG_SC_REF_OBJECT
         MessageInterface::ShowMessage
            (wxT("      Added cloned hardware <%p><%s>'%s' to list, ")
             wxT("hwArray.size()=%d\n"), clonedObj, clonedObj->GetTypeName().c_str(),
             clonedObj->GetName().c_str(), hwArray.size());
         #endif

         if (clonedObj->IsOfType(wxT("Thruster")))
         {
            #ifdef DEBUG_SC_REF_OBJECT
            MessageInterface::ShowMessage
               (wxT("   Setting ref objects to thruster<%p>'%s'\n"),
                clonedObj, clonedObj->GetName().c_str());
            #endif

            // Set SolarSystem and Spacecraft
            clonedObj->SetSolarSystem(solarSystem);
            clonedObj->SetRefObject(this, Gmat::SPACECRAFT, GetName());
            // Set CoordinateSystem
            wxString csName;
            csName = clonedObj->GetRefObjectName(Gmat::COORDINATE_SYSTEM);
            if (csName != wxT("") && coordSysMap.find(csName) != coordSysMap.end())
            {
               #ifdef DEBUG_SC_REF_OBJECT
               MessageInterface::ShowMessage
                  (wxT("   Setting the CoordinateSystem <%p>'%s' to cloned thruster\n"),
                  coordSysMap[csName], csName.c_str() );
               #endif
               clonedObj->SetRefObject(coordSysMap[csName], Gmat::COORDINATE_SYSTEM, csName);
            }
         }
      }
      else
      {
         #ifdef DEBUG_SC_REF_OBJECT
         MessageInterface::ShowMessage
            (wxT("   The name '%s' not found in %s list\n"), objName.c_str(),
             objType.c_str());
         #endif
      }
   }
   else
   {
      #ifdef DEBUG_SC_REF_OBJECT
      MessageInterface::ShowMessage
         (wxT("   The same %s <%p> found in %s pointer list\n"), objType.c_str(),
          obj, objType.c_str());
      #endif
   }

   #ifdef DEBUG_SC_REF_OBJECT
   MessageInterface::ShowMessage
      (wxT("Spacecraft::SetHardware() <%p>'%s' returning true, total %s count = %d\n"),
       this, GetName().c_str(), objType.c_str(), hwArray.size());
   #endif

   return true;
}


//------------------------------------------------------------------------------
// const wxString&  GetGeneratingString(Gmat::WriteMode mode,
//                const wxString &prefix, const wxString &useName)
//------------------------------------------------------------------------------
/**
 * Produces a string, possibly multi-line, containing the text that produces an
 * object.
 *
 * @param mode Specifies the type of serialization requested.
 * @param prefix Optional prefix appended to the object's name
 * @param useName Name that replaces the object's name.
 *
 * @return A string containing the text.
 */
//------------------------------------------------------------------------------
const wxString& Spacecraft::GetGeneratingString(Gmat::WriteMode mode,
                                                   const wxString &prefix,
                                                   const wxString &useName)
{
   #ifdef DEBUG_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("Spacecraft::GetGeneratingString() <%p><%s>'%s' entered, mode=%d, ")
       wxT("prefix='%s', useName='%s', \n"), this, GetTypeName().c_str(),
       GetName().c_str(), mode, prefix.c_str(), useName.c_str());
   MessageInterface::ShowMessage
      (wxT("   showPrefaceComment=%d, commentLine=<%s>\n   showInlineComment=%d ")
       wxT("inlineComment=<%s>\n"),  showPrefaceComment, commentLine.c_str(),
       showInlineComment, inlineComment.c_str());
   #endif

   wxString data;

   wxString preface = wxT(""), nomme;

   if ((mode == Gmat::SCRIPTING) || (mode == Gmat::OWNED_OBJECT) ||
       (mode == Gmat::SHOW_SCRIPT))
      inMatlabMode = false;
   if (mode == Gmat::MATLAB_STRUCT || mode == Gmat::EPHEM_HEADER)
      inMatlabMode = true;

   if (useName != wxT(""))
      nomme = useName;
   else
      nomme = instanceName;

   if ((mode == Gmat::SCRIPTING) || (mode == Gmat::SHOW_SCRIPT))
   {
      wxString tname = typeName;
      data << wxT("Create ") << tname << wxT(" ") << nomme << wxT(";\n");
      preface = wxT("GMAT ");
   }
   else if (mode == Gmat::EPHEM_HEADER)
   {
      data << typeName << wxT(" = ") << wxT("'") << nomme << wxT("';\n");
      preface = wxT("");
   }

   nomme += wxT(".");

   if (mode == Gmat::OWNED_OBJECT)
   {
      preface = prefix;
      nomme = wxT("");
   }

   preface += nomme;
   WriteParameters(mode, preface, data);

   generatingString = data;

   #ifdef DEBUG_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("==========> <%p>'%s'\n%s\n"), this, GetName().c_str(), generatingString.c_str());
   #endif

   #ifdef DEBUG_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("Spacecraft::GetGeneratingString() <%p><%s>'%s' leaving\n"),
       this, GetTypeName().c_str(), GetName().c_str());
   #endif

//   return generatingString;

   // Commented out all parameter writings are handled here (LOJ: 2009.11.23)
   // Then call the parent class method for preface and inline comments
   // Added back in to fix issue with no comment header being written before
   // Spacecraft section
   return SpaceObject::GetGeneratingString(mode, prefix, useName);
}


//------------------------------------------------------------------------------
// void WriteParameters(wxString &prefix, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Code that writes the parameter details for an object.
 *
 * @param prefix Starting portion of the script string used for the parameter.
 * @param obj The object that is written.
 */
//------------------------------------------------------------------------------
void Spacecraft::WriteParameters(Gmat::WriteMode mode, wxString &prefix,
                                 wxString &stream)
{
   #ifdef DEBUG_WRITE_PARAMETERS
   MessageInterface::ShowMessage(wxT("--- Entering SC::WriteParameters ...\n"));
   MessageInterface::ShowMessage(wxT("--- mode = %d, prefix = %s\n"),
   (Integer) mode, prefix.c_str());
   MessageInterface::ShowMessage(wxT(" --- stateType = %s, displayStateType = %s\n"),
   stateType.c_str(), displayStateType.c_str());
   #endif
   Integer i;
   Gmat::ParameterType parmType;
   wxString value;

   bool showAnomaly = false;
   if (stateType == wxT("Keplerian") || stateType == wxT("ModKeplerian"))
      showAnomaly = true;

   Integer *parmOrder = new Integer[parameterCount];
   Integer parmIndex = 0;

   parmOrder[parmIndex++] = DATE_FORMAT_ID;
   parmOrder[parmIndex++] = SC_EPOCH_ID;
   parmOrder[parmIndex++] = COORD_SYS_ID;
   parmOrder[parmIndex++] = DISPLAY_STATE_TYPE_ID;
   parmOrder[parmIndex++] = ANOMALY_ID;
   parmOrder[parmIndex++] = ELEMENT1_ID;
   parmOrder[parmIndex++] = ELEMENT2_ID;
   parmOrder[parmIndex++] = ELEMENT3_ID;
   parmOrder[parmIndex++] = ELEMENT4_ID;
   parmOrder[parmIndex++] = ELEMENT5_ID;
   parmOrder[parmIndex++] = ELEMENT6_ID;
   parmOrder[parmIndex++] = DRY_MASS_ID;
   parmOrder[parmIndex++] = CD_ID;
   parmOrder[parmIndex++] = CR_ID;
   parmOrder[parmIndex++] = DRAG_AREA_ID;
   parmOrder[parmIndex++] = SRP_AREA_ID;
   parmOrder[parmIndex++] = FUEL_TANK_ID;
   parmOrder[parmIndex++] = THRUSTER_ID;
   parmOrder[parmIndex++] = ORBIT_STM;
   parmOrder[parmIndex++] = ORBIT_A_MATRIX;
   parmOrder[parmIndex++] = ELEMENT1UNIT_ID;
   parmOrder[parmIndex++] = ELEMENT2UNIT_ID;
   parmOrder[parmIndex++] = ELEMENT3UNIT_ID;
   parmOrder[parmIndex++] = ELEMENT4UNIT_ID;
   parmOrder[parmIndex++] = ELEMENT5UNIT_ID;
   parmOrder[parmIndex++] = ELEMENT6UNIT_ID;


   bool registered;
   for (i = 0; i < parameterCount; ++i)
   {
      registered = false;
      for (Integer j = 0; j < parmIndex; ++j)
      {
         if (parmOrder[j] == i)
         {
            registered = true;
            break;
         }
      }
      if (!registered)
         parmOrder[parmIndex++] = i;
   }

   Rvector6 repState = GetStateInRepresentation(displayStateType);

   #ifdef DEBUG_WRITE_PARAMETERS
   MessageInterface::ShowMessage
      (wxT("   trueAnomaly=%s\n"), trueAnomaly.ToString().c_str());
   MessageInterface::ShowMessage
      (wxT("   stateType=%s, repState=%s\n"), stateType.c_str(),
       repState.ToString().c_str());
   MessageInterface::ShowMessage
      (wxT("   displayStateType=%s, repState=%s\n"), displayStateType.c_str(),
       repState.ToString().c_str());
   #endif

   for (i = 0; i < parameterCount; ++i)
   {
      if ((IsParameterReadOnly(parmOrder[i]) == false) &&
          (parmOrder[i] != J2000_BODY_NAME) &&
          (parmOrder[i] != TOTAL_MASS_ID)   &&
          (parmOrder[i] != STATE_TYPE_ID)   &&         // deprecated
          (parmOrder[i] != ATTITUDE))
      {
         parmType = GetParameterType(parmOrder[i]);

         // Handle StringArray parameters separately
         if (parmType != Gmat::STRINGARRAY_TYPE &&
             parmType != Gmat::OBJECTARRAY_TYPE)
         {
            // Skip unhandled types
            if (
                (parmType != Gmat::UNSIGNED_INTARRAY_TYPE) &&
                (parmType != Gmat::RVECTOR_TYPE) &&
//                (parmType != Gmat::RMATRIX_TYPE) &&
                (parmType != Gmat::UNKNOWN_PARAMETER_TYPE)
               )
            {
               // Fill in the l.h.s.
               value = wxT("");
               if ((parmOrder[i] >= ELEMENT1_ID) &&
                   (parmOrder[i] <= ELEMENT6_ID))
               {
                  #ifdef DEBUG_WRITE_PARAMETERS
                  MessageInterface::ShowMessage(wxT("--- parmOrder[i] = %d,"),
                  (Integer) parmOrder[i]);
                  MessageInterface::ShowMessage(wxT(" --- and that is for element %s\n"),
                  (GetParameterText(parmOrder[i])).c_str());
                  #endif
                  value << repState[parmOrder[i] - ELEMENT1_ID];
               }
               else if (parmOrder[i] == DISPLAY_STATE_TYPE_ID)
               {
                  if (mode != Gmat::MATLAB_STRUCT)
                     value << displayStateType;
                  else
                     value << wxT("'") << displayStateType << wxT("'");
               }
               else if (parmOrder[i] == ANOMALY_ID)
               {
                  #ifdef __WRITE_ANOMALY_TYPE__
                  if (showAnomaly)
                  {
                     if (mode != Gmat::MATLAB_STRUCT)
                        value << anomalyType;
                     else
                        value << wxT("'") << anomalyType << wxT("'");
                  }
                  #endif
               }
               else
               {
                  #ifdef DEBUG_WRITE_PARAMETERS
                  MessageInterface::ShowMessage(
                  wxT("--- about to call WriteParameterValue with parmOrder[i] = %d\n"),
                  (Integer) parmOrder[i]);
                  MessageInterface::ShowMessage(wxT("--- and the string associated with it is %s\n"),
                  GetParameterText(parmOrder[i]).c_str());
                  #endif
                  WriteParameterValue(parmOrder[i], value);
               }

               if (value != wxT(""))
               {
                  stream << prefix << GetParameterText(parmOrder[i])
                         << wxT(" = ") << value << wxT(";\n");
               }
            }
         }
         else
         {
            bool writeQuotes = inMatlabMode || parmType == Gmat::STRINGARRAY_TYPE;

            // Handle StringArrays
            StringArray sar = GetStringArrayParameter(parmOrder[i]);
            if (sar.size() > 0)
            {
               stream << prefix << GetParameterText(parmOrder[i]) << wxT(" = {");
               for (StringArray::iterator n = sar.begin(); n != sar.end(); ++n)
               {
                  if (n != sar.begin())
                     stream << wxT(", ");
                  if (writeQuotes)
                     stream << wxT("'");
                  stream << (*n);
                  if (writeQuotes)
                     stream << wxT("'");
               }
               stream << wxT("};\n");
            }
         }
      }
      // handle ATTITUDE differently
      else if (parmOrder[i] == ATTITUDE)
      {
         if (attitude)
         {
            if (inMatlabMode)
               stream << prefix << wxT("Attitude = '") << attitude->GetAttitudeModelName() << wxT("';\n");
            else
               stream << prefix << wxT("Attitude = ") << attitude->GetAttitudeModelName() << wxT(";\n");
         }
         else
         {
            MessageInterface::ShowMessage
               (wxT("*** INTERNAL ERROR *** attitude is NULL\n"));
         }
      }

   }

   // Prep in case spacecraft wxT("own") the attached hardware
   GmatBase *ownedObject;
   wxString nomme, newprefix;

   #ifdef DEBUG_OWNED_OBJECT_STRINGS
      MessageInterface::ShowMessage(wxT("\"%s\" has %d owned objects\n"),
         instanceName.c_str(), GetOwnedObjectCount());
   #endif

   // @note Currently only attitude is considered owned object.
   // The properties of hardware such as tanks and thrusters are
   // not written out (LOJ: 2009.09.14)
   for (i = 0; i < GetOwnedObjectCount(); ++i)
   {
      newprefix = prefix;
      ownedObject = GetOwnedObject(i);
      nomme = ownedObject->GetName();

      #ifdef DEBUG_OWNED_OBJECT_STRINGS
          MessageInterface::ShowMessage(
             wxT("   index %d <%p> has type %s and name \"%s\"\n"),
             i, ownedObject, ownedObject->GetTypeName().c_str(),
             ownedObject->GetName().c_str());
      #endif

      if (nomme != wxT(""))
         newprefix += nomme + wxT(".");
      //else if (GetType() == Gmat::FORCE_MODEL)  wcs - why is this here? GetType on s/c?
      //   newprefix += ownedObject->GetTypeName();
      stream << ownedObject->GetGeneratingString(Gmat::OWNED_OBJECT, newprefix);
   }

   delete [] parmOrder;
}


//------------------------------------------------------------------------------
// void UpdateElementLabels()
//------------------------------------------------------------------------------
/**
 * Code used to set the element labels.
 */
//------------------------------------------------------------------------------
void Spacecraft::UpdateElementLabels()
{
   //if (stateType == wxT("Cartesian"))
   if (displayStateType == wxT("Cartesian"))
   {
      stateElementLabel[0] = wxT("X");
      stateElementLabel[1] = wxT("Y");
      stateElementLabel[2] = wxT("Z");
      stateElementLabel[3] = wxT("VX");
      stateElementLabel[4] = wxT("VY");
      stateElementLabel[5] = wxT("VZ");

      stateElementUnits[0] = wxT("km");
      stateElementUnits[1] = wxT("km");
      stateElementUnits[2] = wxT("km");
      stateElementUnits[3] = wxT("km/s");
      stateElementUnits[4] = wxT("km/s");
      stateElementUnits[5] = wxT("km/s");

      return;
   }

   //if (stateType == wxT("Keplerian"))
   if (displayStateType == wxT("Keplerian"))
   {
      stateElementLabel[0] = wxT("SMA");
      stateElementLabel[1] = wxT("ECC");
      stateElementLabel[2] = wxT("INC");
      stateElementLabel[3] = wxT("RAAN");
      stateElementLabel[4] = wxT("AOP");
      stateElementLabel[5] = anomalyType;

      stateElementUnits[0] = wxT("km");
      stateElementUnits[1] = wxT("");
      stateElementUnits[2] = wxT("deg");
      stateElementUnits[3] = wxT("deg");
      stateElementUnits[4] = wxT("deg");
      stateElementUnits[5] = wxT("deg");

      return;
   }

   //if (stateType == wxT("ModifiedKeplerian"))
   if (displayStateType == wxT("ModifiedKeplerian"))
   {
      stateElementLabel[0] = wxT("RadPer");
      stateElementLabel[1] = wxT("RadApo");
      stateElementLabel[2] = wxT("INC");
      stateElementLabel[3] = wxT("RAAN");
      stateElementLabel[4] = wxT("AOP");
      stateElementLabel[5] = anomalyType;

      stateElementUnits[0] = wxT("km");
      stateElementUnits[1] = wxT("km");
      stateElementUnits[2] = wxT("deg");
      stateElementUnits[3] = wxT("deg");
      stateElementUnits[4] = wxT("deg");
      stateElementUnits[5] = wxT("deg");

      return;
   }

   //if (stateType == wxT("SphericalAZFPA"))
   if (displayStateType == wxT("SphericalAZFPA"))
   {
      stateElementLabel[0] = wxT("RMAG");
      stateElementLabel[1] = wxT("RA");
      stateElementLabel[2] = wxT("DEC");
      stateElementLabel[3] = wxT("VMAG");
      stateElementLabel[4] = wxT("AZI");
      stateElementLabel[5] = wxT("FPA");

      stateElementUnits[0] = wxT("km");
      stateElementUnits[1] = wxT("deg");
      stateElementUnits[2] = wxT("deg");
      stateElementUnits[3] = wxT("km/s");
      stateElementUnits[4] = wxT("deg");
      stateElementUnits[5] = wxT("deg");

      return;
   }

   //if (stateType == wxT("SphericalRADEC"))
   if (displayStateType == wxT("SphericalRADEC"))
   {
      stateElementLabel[0] = wxT("RMAG");
      stateElementLabel[1] = wxT("RA");
      stateElementLabel[2] = wxT("DEC");
      stateElementLabel[3] = wxT("VMAG");
      stateElementLabel[4] = wxT("RAV");
      stateElementLabel[5] = wxT("DECV");

      stateElementUnits[0] = wxT("km");
      stateElementUnits[1] = wxT("deg");
      stateElementUnits[2] = wxT("deg");
      stateElementUnits[3] = wxT("km/s");
      stateElementUnits[4] = wxT("deg");
      stateElementUnits[5] = wxT("deg");

      return;
   }

   //if (stateType == wxT("Equinoctial"))
   if (displayStateType == wxT("Equinoctial"))
   {
      stateElementLabel[0] = wxT("SMA");
      stateElementLabel[1] = wxT("EquinoctialH");
      stateElementLabel[2] = wxT("EquinoctialK");
      stateElementLabel[3] = wxT("EquinoctialP");
      stateElementLabel[4] = wxT("EquinoctialQ");
//      stateElementLabel[1] = wxT("h");
//      stateElementLabel[2] = wxT("k");
//      stateElementLabel[3] = wxT("p");
//      stateElementLabel[4] = wxT("q");
      stateElementLabel[5] = wxT("MLONG");

      stateElementUnits[0] = wxT("km");
      stateElementUnits[1] = wxT("");
      stateElementUnits[2] = wxT("");
      stateElementUnits[3] = wxT("");
      stateElementUnits[4] = wxT("");
      stateElementUnits[5] = wxT("deg");

      return;
   }


}


//------------------------------------------------------------------------------
// Rvector6 GetStateInRepresentation(wxString rep)
//------------------------------------------------------------------------------
/**
 * Code used to obtain a state in a non-Cartesian representation.
 */
//------------------------------------------------------------------------------
Rvector6 Spacecraft::GetStateInRepresentation(wxString rep)
{
   #ifdef DEBUG_STATE_INTERFACE
      MessageInterface::ShowMessage(
         wxT("Spacecraft::GetStateInRepresentation(string): Constructing %s state\n"),
         rep.c_str());
   #endif

   Rvector6 csState;
   Rvector6 finalState;

   // First convert from the internal CS to the state CS
   if (internalCoordSystem != coordinateSystem)
   {
      Rvector6 inState(state.GetState());
      coordConverter.Convert(GetEpoch(), inState, internalCoordSystem, csState,
         coordinateSystem);
   }
   else
   {
      csState.Set(state.GetState());
   }

   // Then convert to the desired representation
   if (rep == wxT(""))
      rep = stateType;   // do I want displayStateType here?

   if (rep == wxT("Cartesian"))
   {
      finalState = csState;
      #ifdef DEBUG_STATE_INTERFACE
         MessageInterface::ShowMessage(
            wxT("Spacecraft::GetStateInRepresentation(string): type is Cartesian, so no conversion done\n"));
      #endif
   }
   else
   {
      #ifdef DEBUG_STATE_INTERFACE
         MessageInterface::ShowMessage(
            wxT("Spacecraft::GetStateInRepresentation(string): type is %s, so calling stateConverter to convert\n"),
            rep.c_str());
      #endif
      //finalState = stateConverter.Convert(csState, wxT("Cartesian"), rep, trueAnomaly);
      finalState = stateConverter.Convert(csState, wxT("Cartesian"), rep, anomalyType);
   }

   #ifdef DEBUG_STATE_INTERFACE
      MessageInterface::ShowMessage(
         wxT("Spacecraft::GetStateInRepresentation(string): %s state is ")
         wxT("[%.9lf %.9lf %.9lf %.14lf %.14lf %.14lf]\n"),
         rep.c_str(), finalState[0], finalState[1],
         finalState[2], finalState[3], finalState[4],
         finalState[5]);
   #endif

   return finalState;
}


//------------------------------------------------------------------------------
// Rvector6 GetStateInRepresentation(Integer rep)
//------------------------------------------------------------------------------
/**
 * Code used to obtain a state in a non-Cartesian representation.
 */
//------------------------------------------------------------------------------
Rvector6 Spacecraft::GetStateInRepresentation(Integer rep)
{
   #ifdef DEBUG_STATE_INTERFACE
      MessageInterface::ShowMessage(
         wxT("Spacecraft::GetStateInRepresentation(int): Constructing %s state\n"),
         representations[rep].c_str());
   #endif
   Rvector6 csState;
   Rvector6 finalState;

   // First convert from the internal CS to the state CS
   if (internalCoordSystem != coordinateSystem)
   {
      Rvector6 inState(state.GetState());
      coordConverter.Convert(GetEpoch(), inState, internalCoordSystem, csState,
         coordinateSystem);
   }
   else
   {
      csState.Set(state.GetState());
   }

   // Then convert to the desired representation
   if (rep == CARTESIAN_ID)
      finalState = csState;
   else
   {
      finalState = stateConverter.Convert(csState, wxT("Cartesian"),
                   representations[rep], anomalyType); //trueAnomaly);
   }

   #ifdef DEBUG_STATE_INTERFACE
      MessageInterface::ShowMessage(
         wxT("Spacecraft::GetStateInRepresentation(int): %s state is ")
         wxT("[%.9lf %.9lf %.9lf %.14lf %.14lf %.14lf]\n"),
         representations[rep].c_str(), finalState[0], finalState[1],
         finalState[2], finalState[3], finalState[4],
         finalState[5]);
   #endif

   return finalState;
}


//------------------------------------------------------------------------------
// void SetStateFromRepresentation(wxString rep, Rvector6 &st)
//------------------------------------------------------------------------------
/**
 * Code used to obtain a state in a non-Cartesian representation.
 */
//------------------------------------------------------------------------------
void Spacecraft::SetStateFromRepresentation(wxString rep, Rvector6 &st)
{
   #ifdef DEBUG_STATE_INTERFACE
      MessageInterface::ShowMessage(
         wxT("Spacecraft::SetStateFromRepresentation: Setting %s state to %s\n"),
         rep.c_str(), st.ToString(16).c_str());
   #endif

   // First convert from the representation to Cartesian
   static Rvector6 csState, finalState;

   if (rep == wxT("Cartesian"))
      csState = st;
   else
   {
      #ifdef DEBUG_STATE_INTERFACE
      MessageInterface::ShowMessage
         (wxT("   rep is not Cartesian, so calling stateConverter.Convert()\n"));
      #endif
      csState = stateConverter.Convert(st, rep, wxT("Cartesian"), anomalyType);
   }

   #ifdef DEBUG_STATE_INTERFACE
      MessageInterface::ShowMessage(
         wxT("Spacecraft::SetStateFromRepresentation: state has been converted\n"));
   #endif

   if (internalCoordSystem == NULL)
      throw SpaceObjectException(wxT(" The spacecraft internal coordinate system is not set"));
   if (coordinateSystem == NULL)
      throw SpaceObjectException(wxT(" The spacecraft coordinate system is not set"));

   #ifdef DEBUG_STATE_INTERFACE
   MessageInterface::ShowMessage
      (wxT("   Now convert to internal CS, internalCoordSystem=<%p>, coordinateSystem=<%p>\n"),
       internalCoordSystem, coordinateSystem);
   #endif

   // Then convert to the internal CS
   if (internalCoordSystem != coordinateSystem)
   {
      #ifdef DEBUG_STATE_INTERFACE
      MessageInterface::ShowMessage
         (wxT("   cs is not InteralCS, so calling coordConverter.Convert() at epoch %f\n"),
          GetEpoch());
      #endif
      coordConverter.Convert(GetEpoch(), csState, coordinateSystem, finalState,
         internalCoordSystem);
   }
   else
      finalState = csState;

   for (int i=0; i<6; i++)
      state[i] = finalState[i];

   #ifdef DEBUG_STATE_INTERFACE
      MessageInterface::ShowMessage(
         wxT("Spacecraft::SetStateFromRepresentation: Cartesian State is now\n   ")
         wxT("%.9lf %.9lf %.9lf %.14lf %.14lf %.14lf\n"), state[0], state[1],
         state[2], state[3], state[4], state[5]);
   #endif
}


//------------------------------------------------------------------------------
// Real Spacecraft::GetElement(const wxString &label)
//------------------------------------------------------------------------------
/**
 * Code used to obtain a state element.
 *
 * @param <label> The test label for the element.
 *
 * @return The element's value, or -9999999999.999999 on failure.
 */
//------------------------------------------------------------------------------
Real Spacecraft::GetElement(const wxString &label)
{
   #ifdef DEBUG_GET_REAL
      MessageInterface::ShowMessage(
      wxT("In SC::GetElement, asking for parameter %s\n"), label.c_str());
   #endif
   Integer baseID;
   wxString rep = wxT("");
   baseID = LookUpLabel(label,rep);
   #ifdef DEBUG_GET_REAL
   MessageInterface::ShowMessage(
   wxT("In SC::GetElement, after LookUpLabel, id = %d, its string = \"%s\",  and rep = \"%s\"\n"),
   baseID, (GetParameterText(baseID)).c_str(), rep.c_str());
   #endif
   Rvector6 stateInRep = GetStateInRepresentation(rep);
   #ifdef DEBUG_GET_REAL
      MessageInterface::ShowMessage(
      wxT("In SC::GetElement, stateInRep = \n"));
      for (Integer jj=0;jj<6;jj++)
         MessageInterface::ShowMessage(wxT("    %.12f\n"), stateInRep[jj]);
   #endif
   // check for Anomaly data first
   if (label == wxT("TA") || label == wxT("EA") ||
       label == wxT("MA") || label == wxT("HA"))
   {
      Anomaly tmpAnomaly;
      tmpAnomaly.SetSMA(stateInRep[0]);
      tmpAnomaly.SetECC(stateInRep[1]);
      tmpAnomaly.SetValue(stateInRep[5]);
      return tmpAnomaly.GetValue(label);
   }
   else
   {
      if (baseID == ELEMENT1_ID) return stateInRep[0];
      if (baseID == ELEMENT2_ID) return stateInRep[1];
      if (baseID == ELEMENT3_ID) return stateInRep[2];
      if (baseID == ELEMENT4_ID) return stateInRep[3];
      if (baseID == ELEMENT5_ID) return stateInRep[4];
      if (baseID == ELEMENT6_ID) return stateInRep[5];
   }

   return -9999999999.999999;  // some kind of error
}


//------------------------------------------------------------------------------
// bool SetElement(const wxString &label, const Real &value)
//------------------------------------------------------------------------------
/**
 * Set a state element.
 *
 * @param <label> Label for the element -- 'X', 'Y', 'SMA', etc.
 * @param <value> New value for the element.
 *
 * @return true on success, false on failure.
 */
//------------------------------------------------------------------------------
bool Spacecraft::SetElement(const wxString &label, const Real &value)
{
   #ifdef DEBUG_SPACECRAFT_SET_ELEMENT
   MessageInterface::ShowMessage
      (wxT("In SC::SetElement, <%p> '%s', label=%s, value=%.12f\n"), this,
       GetName().c_str(), label.c_str(), value);
   #endif
   wxString rep = wxT("");
   Integer id = LookUpLabel(label, rep) - ELEMENT1_ID;
   #ifdef DEBUG_SPACECRAFT_SET_ELEMENT
      MessageInterface::ShowMessage
         (wxT(" ************ In SC::SetElement, after LookUpLabel, ELEMENT1_ID = %d, id = %d, rep = %s\n"),
               ELEMENT1_ID, id, rep.c_str());
   #endif

   if ((rep != wxT("")) && (stateType != rep))
   {
      if ((rep == wxT("Keplerian")) || (rep == wxT("ModifiedKeplerian")))
      {
         // Load trueAnomaly with the state data
         Rvector6 kep = GetStateInRepresentation(wxT("Keplerian"));
         trueAnomaly.SetSMA(kep[0]);
         trueAnomaly.SetECC(kep[1]);
         trueAnomaly.SetValue(kep[5]);
      }
      // 2007.05.24 - wcs - Bug 875 - because some elements are the same for
      // Keplerian and ModifiedKeplerian, make sure it only changes when it should
      if ( (stateType == wxT("ModifiedKeplerian")) && (rep == wxT("Keplerian")) &&
         (label != wxT("SMA")) && (label != wxT("ECC")) )
      {
         #ifdef DEBUG_SPACECRAFT_SET_ELEMENT
            MessageInterface::ShowMessage
               (wxT(" ************ In SC::SetElement, leaving stateType as ModifiedKeplerian\n"));
         #endif
         // leave stateType as ModifiedKeplerian
      }
      else if ( (stateType == wxT("SphericalRADEC")) && (rep == wxT("SphericalAZFPA")) &&
               (label != wxT("AZI")) && (label != wxT("FPA")) )
      {
         #ifdef DEBUG_SPACECRAFT_SET_ELEMENT
            MessageInterface::ShowMessage
               (wxT(" ************ In SC::SetElement, leaving stateType as SphericalRADEC\n"));
         #endif
         // leave it as SphericalRADEC
      }
      /// 2010.03.22 - wcs - SMA could also be Equinoctial
      else if ( (stateType == wxT("Equinoctial")) && (rep == wxT("Keplerian")) &&
               (label == wxT("SMA")) )
//         (label != wxT("SMA")))
      {
         #ifdef DEBUG_SPACECRAFT_SET_ELEMENT
            MessageInterface::ShowMessage
               (wxT(" ************ In SC::SetElement, leaving stateType as Equinoctial\n"));
         #endif
         // leave state as Equinoctial
      }
      else
      {
         #ifdef DEBUG_SPACECRAFT_SET_ELEMENT
            MessageInterface::ShowMessage
               (wxT(" ************ In SC::SetElement, MODIFYING stateType from %s to %s\n"),
                 stateType.c_str(), rep.c_str());
         #endif
         stateType = rep;
      }
   }

   #ifdef DEBUG_SPACECRAFT_SET_ELEMENT
   if (id >= 0)
      MessageInterface::ShowMessage
         (wxT("In SC::SetElement, after LookUpLabel, id+ELEMENT1_ID = %d, its ")
          wxT("string = \"%s\",  and rep = \"%s\"\n"), id+ELEMENT1_ID,
          (GetParameterText(id+ELEMENT1_ID)).c_str(), rep.c_str());
      MessageInterface::ShowMessage
         (wxT("In SC::SetElement, after LookUpLabel, its label = \"%s\" and its value = %12.10f\n"),
               label.c_str(), value);

   #endif

   // parabolic and hyperbolic orbits not yet supported
   if ((label == wxT("ECC")) && value == 1.0)
   {
      SpaceObjectException se;
      se.SetDetails(errorMessageFormat.c_str(),
                    GmatStringUtil::ToString(value, GetDataPrecision()).c_str(),
                    wxT("Eccentricity"), wxT("Real Number != 1.0"));
      throw se;
   }
   // Equinoctial elements must be within bounds
   if (((label == wxT("EquinoctialH")) || (label == wxT("EquinoctialK"))) &&
       ((value < -1.0) || (value > 1.0)))
   {
      #ifdef DEBUG_SPACECRAFT_SET_ELEMENT
         MessageInterface::ShowMessage
            (wxT("In SC::SetElement, reached where the exception should be thrown!!!!\n"));
      #endif
      SpaceObjectException se;
      se.SetDetails(errorMessageFormat.c_str(),
                    GmatStringUtil::ToString(value, GetDataPrecision()).c_str(),
                    label.c_str(), wxT("-1.0 <= Real Number <= 1.0"));
      throw se;
   }

   if ((id == 5) && (!trueAnomaly.IsInvalid(label)))
      trueAnomaly.SetType(label);

   if (id >= 0)
   {
      if (csSet)
      {
         #ifdef DEBUG_SPACECRAFT_SET_ELEMENT
            MessageInterface::ShowMessage(wxT("In SC::SetElement, csSet = TRUE\n"));
         #endif
         Rvector6 tempState = GetStateInRepresentation(rep);
         tempState[id] = value;
         SetStateFromRepresentation(rep, tempState);
      }
      else
      {
         #ifdef DEBUG_SPACECRAFT_SET_ELEMENT
            MessageInterface::ShowMessage(wxT("In SC::SetElement, csSet = FALSE\n"));
         #endif
         Real *tempState = state.GetState();
         tempState[id] = value;
      }

      #ifdef DEBUG_SPACECRAFT_SET_ELEMENT
      Rvector6 vec6(state.GetState());
      MessageInterface::ShowMessage
         (wxT("   CS was %sset, state is now\n   %s \n"), (csSet ? wxT("") : wxT("NOT ")),
          vec6.ToString().c_str());
      MessageInterface::ShowMessage
         (wxT("In SC::SetElement, '%s', returning TRUE\n"), GetName().c_str());
      #endif
      return true;
   }

   #ifdef DEBUG_SPACECRAFT_SET_ELEMENT
   MessageInterface::ShowMessage(wxT("In SC::SetElement, returning FALSE\n"));
   #endif
   return false;
}


//------------------------------------------------------------------------------
// void LookUpLabel(wxString rep, Rvector6 &st)
//------------------------------------------------------------------------------
/**
 * Code used to obtain a state in a non-Cartesian representation.
 */
//------------------------------------------------------------------------------
Integer Spacecraft::LookUpLabel(const wxString &label, wxString &rep)
{
   #ifdef DEBUG_LOOK_UP_LABEL
      MessageInterface::ShowMessage(wxT("Spacecraft::LookUpLabel(%s)called\n"),
         label.c_str());
   #endif
   Integer retval = -1;

   if (label == wxT("Element1"))
   {
      rep = stateType;
      return ELEMENT1_ID;
   }
   if (label == wxT("Element2"))
   {
      rep = stateType;
      return ELEMENT2_ID;
   }
   if (label == wxT("Element3"))
   {
      rep = stateType;
      return ELEMENT3_ID;
   }
   if (label == wxT("Element4"))
   {
      rep = stateType;
      return ELEMENT4_ID;
   }
   if (label == wxT("Element5"))
   {
      rep = stateType;
      return ELEMENT5_ID;
   }
   if (label == wxT("Element6"))
   {
      rep = stateType;
      return ELEMENT6_ID;
   }

   if (label == wxT("X") || label == wxT("SMA") || label == wxT("RadPer") || label == wxT("RMAG"))
      retval = ELEMENT1_ID;

   else if (label == wxT("Y") || label == wxT("ECC") || label == wxT("RadApo") || label == wxT("RA") ||
            label == wxT("PEY") || label == wxT("EquinoctialH"))
      retval = ELEMENT2_ID;

   else if (label == wxT("Z") || label == wxT("INC") || label == wxT("DEC") || label == wxT("PEX") ||
            label == wxT("EquinoctialK"))
      retval = ELEMENT3_ID;

   else if (label == wxT("VX") || label == wxT("RAAN") || label == wxT("VMAG") || label == wxT("PNY") ||
            label == wxT("EquinoctialP"))
      retval = ELEMENT4_ID;

   else if (label == wxT("VY") || label == wxT("AOP") || label == wxT("AZI") || label == wxT("RAV") ||
            label == wxT("PNX") || label == wxT("EquinoctialQ"))
      retval = ELEMENT5_ID;

   else if (label == wxT("VZ") || !trueAnomaly.IsInvalid(label) ||
        label == wxT("FPA") || label == wxT("DECV") || label == wxT("MLONG"))
      retval = ELEMENT6_ID;

   rep = elementLabelMap[label];

   #ifdef DEBUG_LOOK_UP_LABEL
      MessageInterface::ShowMessage(wxT("Spacecraft::LookUpLabel(%s..) gives rep %s with retval = %d\n"),
         label.c_str(), rep.c_str(), retval);
   #endif

   return retval;
}

//------------------------------------------------------------------------------
// Integer LookUpID(const Integer id, wxString &label, wxString &rep)
//------------------------------------------------------------------------------
Integer Spacecraft::LookUpID(const Integer id, wxString &label, wxString &rep)
{
   label = GetParameterText(id);
   // if it's not one of the multiple reps IDs, just return the ID
   if (id < CART_X)
   {
      rep   = stateType;
      return id;
   }
   // otherwise, figure out the base ID to use for the state data
   return LookUpLabel(label, rep);
}

//------------------------------------------------------------------------------
// void BuildElementLabelMap()
//------------------------------------------------------------------------------
/**
 * Set the mapping between elements and representations.
 */
//------------------------------------------------------------------------------
void Spacecraft::BuildElementLabelMap()
{
   if (elementLabelMap.size() == 0)
   {
      elementLabelMap[wxT("X")] = wxT("Cartesian");
      elementLabelMap[wxT("Y")] = wxT("Cartesian");
      elementLabelMap[wxT("Z")] = wxT("Cartesian");
      elementLabelMap[wxT("VX")] = wxT("Cartesian");
      elementLabelMap[wxT("VY")] = wxT("Cartesian");
      elementLabelMap[wxT("VZ")] = wxT("Cartesian");

      elementLabelMap[wxT("SMA")]  = wxT("Keplerian");
      elementLabelMap[wxT("ECC")]  = wxT("Keplerian");
      elementLabelMap[wxT("INC")]  = wxT("Keplerian");
      elementLabelMap[wxT("RAAN")] = wxT("Keplerian");
      elementLabelMap[wxT("AOP")]  = wxT("Keplerian");
      elementLabelMap[wxT("TA")]   = wxT("Keplerian");
      elementLabelMap[wxT("EA")]   = wxT("Keplerian");
      elementLabelMap[wxT("MA")]   = wxT("Keplerian");
      elementLabelMap[wxT("HA")]   = wxT("Keplerian");

      elementLabelMap[wxT("RadPer")] = wxT("ModifiedKeplerian");
      elementLabelMap[wxT("RadApo")] = wxT("ModifiedKeplerian");

      elementLabelMap[wxT("RMAG")] = wxT("SphericalAZFPA");
      elementLabelMap[wxT("RA")]   = wxT("SphericalAZFPA");
      elementLabelMap[wxT("DEC")]  = wxT("SphericalAZFPA");
      elementLabelMap[wxT("VMAG")] = wxT("SphericalAZFPA");
      elementLabelMap[wxT("AZI")]  = wxT("SphericalAZFPA");
      elementLabelMap[wxT("FPA")]  = wxT("SphericalAZFPA");

      elementLabelMap[wxT("RAV")]  = wxT("SphericalRADEC");
      elementLabelMap[wxT("DECV")] = wxT("SphericalRADEC");

//      elementLabelMap[wxT("PEY")]    = wxT("Equinoctial");
//      elementLabelMap[wxT("PEX")]    = wxT("Equinoctial");
//      elementLabelMap[wxT("PNY")]    = wxT("Equinoctial");
//      elementLabelMap[wxT("PNX")]    = wxT("Equinoctial");
      elementLabelMap[wxT("EquinoctialH")]    = wxT("Equinoctial");
      elementLabelMap[wxT("EquinoctialK")]    = wxT("Equinoctial");
      elementLabelMap[wxT("EquinoctialP")]    = wxT("Equinoctial");
      elementLabelMap[wxT("EquinoctialQ")]    = wxT("Equinoctial");
      elementLabelMap[wxT("MLONG")]  = wxT("Equinoctial");
   }
}

//-------------------------------------------------------------------------
// bool HasDynamicParameterSTM(Integer parameterId)
//-------------------------------------------------------------------------
bool Spacecraft::HasDynamicParameterSTM(Integer parameterId)
{
   if (parameterId == CARTESIAN_X)
      return true;
   return SpaceObject::HasDynamicParameterSTM(parameterId);
}

//-------------------------------------------------------------------------
// Rmatrix* GetParameterSTM(Integer parameterId)
//-------------------------------------------------------------------------
Rmatrix* Spacecraft::GetParameterSTM(Integer parameterId)
{
   if (parameterId == CARTESIAN_X)
      return &orbitSTM;
   return SpaceObject::GetParameterSTM(parameterId);
}

//-------------------------------------------------------------------------
// Integer Spacecraft::HasParameterCovariances(Integer parameterId)
//-------------------------------------------------------------------------
Integer Spacecraft::HasParameterCovariances(Integer parameterId)
{
   if (parameterId == CARTESIAN_X)
      return 6;
   return SpaceObject::HasParameterCovariances(parameterId);
}

//Rmatrix* Spacecraft::GetParameterCovariances(Integer parameterId)
//{
//
//}

// Additions for the propagation rework

//-------------------------------------------------------------------------
// void Spacecraft::RecomputeStateAtEpoch(const GmatEpoch &toEpoch)
//-------------------------------------------------------------------------
void Spacecraft::RecomputeStateAtEpoch(const GmatEpoch &toEpoch)
{
   if (internalCoordSystem != coordinateSystem)
   {
      // First convert from the internal CS to the state CS at the old epoch
      Rvector6 inState(state.GetState());
      Rvector6 csState;
      Rvector6 finalState;
      coordConverter.Convert(GetEpoch(), inState, internalCoordSystem, csState,
         coordinateSystem);
      // Then convert back at the new epoch
      Real newEpoch = toEpoch;
      coordConverter.Convert(newEpoch, csState, coordinateSystem, finalState,
            internalCoordSystem);

      state[0] = finalState[0];
      state[1] = finalState[1];
      state[2] = finalState[2];
      state[3] = finalState[3];
      state[4] = finalState[4];
      state[5] = finalState[5];
   }
   // otherwise, state stays the same
}


//-------------------------------------------------------------------------
// This function is used to verify Spacecraft's added hardware.
//
// return true if there is no error, false otherwise.
//-------------------------------------------------------------------------
// made changes by Tuan Nguyen
bool Spacecraft::VerifyAddHardware()
{
   Gmat::ObjectType type;
   wxString subTypeName;
   GmatBase* obj;
   
   // 1. Verify all hardware in hardwareList are not NULL:
   for(ObjectArray::iterator i= hardwareList.begin(); i != hardwareList.end(); ++i)
   {
      obj = (*i);
      if (obj == NULL)
      {
         MessageInterface::ShowMessage(wxT("***Error***:One element of hardwareList = NULL\n"));
         return false;
      }
   }
   
   // 2. Verify primary antenna to be in hardwareList:
   // 2.1. Create antenna list from hardwareList for searching:
   // extract all antenna from hardwareList and store to antennaList
   ObjectArray antennaList;
   for(ObjectArray::iterator i= hardwareList.begin(); i != hardwareList.end(); ++i)
   {
      obj = (*i);
      subTypeName = obj->GetTypeName();
      if (subTypeName == wxT("Antenna"))
         antennaList.push_back(obj);
   }
   
   // 2.2. Verify primary antenna of Receiver, Transmitter, and Transponder:
   GmatBase* antenna;
   GmatBase* primaryAntenna;
   wxString primaryAntennaName;
   bool verify = true;
   for(ObjectArray::iterator i= hardwareList.begin(); i != hardwareList.end(); ++i)
   {
      obj = (*i);
      type = obj->GetType();
      if (type == Gmat::HARDWARE)
      {
         subTypeName = obj->GetTypeName();
         if ((subTypeName == wxT("Transmitter"))||
             (subTypeName == wxT("Receiver"))||
             (subTypeName == wxT("Transponder")))
         {
            // Get primary antenna:
            primaryAntennaName = obj->GetRefObjectName(Gmat::HARDWARE);
            primaryAntenna = obj->GetRefObject(Gmat::HARDWARE,primaryAntennaName);
            
            bool check;
            if (primaryAntenna == NULL)
            {
               MessageInterface::ShowMessage
                  (wxT("***Error***:primary antenna of %s in %s's AddHardware list is NULL \n"),
                   obj->GetName().c_str(), this->GetName().c_str());
               check = false;
            }
            else
            {
               // Check primary antenna of transmitter, receiver, or transponder is in antenna list:
               check = false;
               for(ObjectArray::iterator j= antennaList.begin(); j != antennaList.end(); ++j)
               {
                  antenna = (*j);
                  if (antenna == primaryAntenna)
                  {
                     check = true;
                     break;
                  }
                  else if (antenna->GetName() == primaryAntenna->GetName())
                  {
                     MessageInterface::ShowMessage
                        (wxT("Primary antenna %s of %s is a clone of an antenna in %s's AddHardware\n"),
                         primaryAntenna->GetName().c_str(), obj->GetName().c_str(),
                         this->GetName().c_str());
                  }
               }
               if (check == false)
               {
                  // Display error message:
                  MessageInterface::ShowMessage
                     (wxT("***Error***:primary antenna of %s is not in %s's AddHardware\n"),
                      obj->GetName().c_str(), this->GetName().c_str());
               }
            }
            
            verify = verify && check;
         }
      }
   }
   
   return verify;
}

