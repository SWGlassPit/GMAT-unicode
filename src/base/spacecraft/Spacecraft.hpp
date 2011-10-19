//$Id: Spacecraft.hpp 9568 2011-06-06 18:27:29Z djcinsb $
//------------------------------------------------------------------------------
//                                 Spacecraft
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
 * Definition of the Spacecraft class base
 */
//------------------------------------------------------------------------------

#ifndef Spacecraft_hpp
#define Spacecraft_hpp

#include <valarray>
#include "SpaceObject.hpp"
#include "Rvector6.hpp"
#include "GmatState.hpp"
#include "FuelTank.hpp"
#include "Thruster.hpp"
#include "Anomaly.hpp"
#include "CoordinateSystem.hpp"
#include "CoordinateConverter.hpp"
#include "TimeSystemConverter.hpp"
#include "StateConverter.hpp"
#include "Attitude.hpp"

#include <map>

class GMAT_API Spacecraft : public SpaceObject
{
public:
   Spacecraft(const wxString &name,
      const wxString &typeStr = wxT("Spacecraft"));
   Spacecraft(const Spacecraft &a);
   Spacecraft&          operator=(const Spacecraft &a);

   // Destructor
   virtual              ~Spacecraft();

   virtual void         SetSolarSystem(SolarSystem *ss);
   void                 SetInternalCoordSystem(CoordinateSystem *cs);
   CoordinateSystem*    GetInternalCoordSystem();

   void                 SetState(const Rvector6 &cartState);
   void                 SetState(const wxString &elementType, Real *instate);
   void                 SetState(const Real s1, const Real s2, const Real s3,
                                 const Real s4, const Real s5, const Real s6);

   virtual GmatState&   GetState();
   virtual Rvector6     GetState(wxString rep);
   virtual Rvector6     GetState(Integer rep);
   Rvector6             GetCartesianState();
   Rvector6             GetKeplerianState();
   Rvector6             GetModifiedKeplerianState();

   Anomaly              GetAnomaly() const;
   
   virtual bool         HasAttitude();
   virtual const Rmatrix33&
                        GetAttitude(Real a1mjdTime);
   const Rvector3&      GetAngularVelocity(Real a1mjdTime) const;
   const UnsignedIntArray&
                        GetEulerAngleSequence() const;
   
   // The ID of the model that the spacecraft uses, and the filename as well
   wxString          modelFile;
   int                  modelID;

   
   // inherited from GmatBase
   virtual GmatBase*    Clone(void) const;
   virtual void         Copy(const GmatBase* orig);
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);

   virtual wxString  GetRefObjectName(const Gmat::ObjectType type) const;

   virtual bool         HasRefObjectTypeArray();
   virtual const        ObjectTypeArray& GetRefObjectTypeArray();
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool         SetRefObjectName(const Gmat::ObjectType type,
                                         const wxString &name);
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name = wxT(""));

   virtual ObjectArray& GetRefObjectArray(const Gmat::ObjectType type);
   virtual ObjectArray& GetRefObjectArray(const wxString& typeString);

   // Parameter accessor methods -- overridden from GmatBase
   virtual Integer      GetParameterID(const wxString &str) const;

   virtual bool         IsParameterReadOnly(const Integer id) const;
   virtual bool         IsParameterReadOnly(const wxString &label) const;
   virtual bool         ParameterAffectsDynamics(const Integer id) const;
   virtual bool         ParameterDvInitializesNonzero(const Integer id,
                              const Integer r = 0, const Integer c = 0) const;
   virtual Real         ParameterDvInitialValue(const Integer id,
                              const Integer r = 0, const Integer c = 0) const;

   virtual Real         GetRealParameter(const Integer id) const;
   virtual Real         GetRealParameter(const wxString &label) const;
   virtual Real         SetRealParameter(const Integer id, const Real value);
   virtual Real         SetRealParameter(const wxString &label, const Real value);
   virtual Real         SetRealParameter(const Integer id,
                                         const Real value,
                                         const Integer index);

   virtual wxString  GetStringParameter(const Integer id) const;
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const Integer id, const wxString &value);
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value);
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value,
                                           const Integer index);
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value,
                                           const Integer index);

   virtual const Rmatrix&
                        GetRmatrixParameter(const Integer id) const;
   virtual const Rmatrix&
                        SetRmatrixParameter(const Integer id,
                                            const Rmatrix &value);
   virtual const Rmatrix&
                        GetRmatrixParameter(const wxString &label) const;
   virtual const Rmatrix&
                        SetRmatrixParameter(const wxString &label,
                                            const Rmatrix &value);
   virtual Real         GetRealParameter(const Integer id, const Integer row,
                                         const Integer col) const;
   virtual Real         GetRealParameter(const wxString &label,
                                         const Integer row,
                                         const Integer col) const;
   virtual Real         SetRealParameter(const Integer id, const Real value,
                                         const Integer row, const Integer col);
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value, const Integer row,
                                         const Integer col);

   const StringArray&   GetStringArrayParameter(const Integer id) const;
   virtual const StringArray&
                        GetStringArrayParameter(const wxString &label) const;
   virtual wxString  GetStringParameter(const Integer id, const Integer index) const;           // made changes by Tuan Nguyen
   virtual wxString  GetStringParameter(const wxString & label, const Integer index) const;  // made changes by Tuan Nguyen

   virtual wxString  GetParameterText(const Integer id) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;

   virtual bool         Initialize();

   virtual bool         TakeAction(const wxString &action,
                                   const wxString &actionData = wxT(""));
   virtual bool         IsOwnedObject(Integer id) const;
   virtual GmatBase*    GetOwnedObject(Integer whichOne);


   virtual const wxString&
                        GetGeneratingString(Gmat::WriteMode mode = Gmat::SCRIPTING,
                                            const wxString &prefix = wxT(""),
                                            const wxString &useName = wxT(""));

   wxString GetEpochString();
   void SetDateFormat(const wxString &dateType);
   void SetEpoch(const wxString &ep);
   void SetEpoch(const wxString &type, const wxString &ep, Real a1mjd);
   void SetState(const wxString &type, const Rvector6 &cartState);
   void SetAnomaly(const wxString &type, const Anomaly &ta);

   virtual Integer         GetPropItemID(const wxString &whichItem);
   virtual Integer         SetPropItem(const wxString &propItem);
   virtual StringArray     GetDefaultPropItems();
   virtual Real*           GetPropItem(const Integer item);
   virtual Integer         GetPropItemSize(const Integer item);
   virtual bool            PropItemNeedsFinalUpdate(const Integer item);

   virtual bool            IsEstimationParameterValid(const Integer id);
   virtual Integer         GetEstimationParameterSize(const Integer id);
   virtual Real*           GetEstimationParameterValue(const Integer id);

   virtual bool            HasDynamicParameterSTM(Integer parameterId);
   virtual Rmatrix*        GetParameterSTM(Integer parameterId);
   virtual Integer         HasParameterCovariances(Integer parameterId);
//   virtual Rmatrix*        GetParameterCovariances(Integer parameterId);

protected:
   enum SC_Param_ID
   {
      SC_EPOCH_ID = SpaceObjectParamCount,
      ELEMENT1_ID,
      ELEMENT2_ID,
      ELEMENT3_ID,
      ELEMENT4_ID,
      ELEMENT5_ID,
      ELEMENT6_ID,
      ELEMENT1UNIT_ID,
      ELEMENT2UNIT_ID,
      ELEMENT3UNIT_ID,
      ELEMENT4UNIT_ID,
      ELEMENT5UNIT_ID,
      ELEMENT6UNIT_ID,
      STATE_TYPE_ID,           // deprecated
      DISPLAY_STATE_TYPE_ID,
      ANOMALY_ID,
      COORD_SYS_ID,
      DRY_MASS_ID,
      DATE_FORMAT_ID,
      CD_ID,
      CR_ID,
      DRAG_AREA_ID,
      SRP_AREA_ID,
      FUEL_TANK_ID,
      THRUSTER_ID,
      TOTAL_MASS_ID,
      SPACECRAFT_ID,
      ATTITUDE,
//      ORBIT_SPICE_KERNEL_NAME,
//      ATTITUDE_SPICE_KERNEL_NAME,
//      SC_CLOCK_SPICE_KERNEL_NAME,
//      FRAME_SPICE_KERNEL_NAME,
      ORBIT_STM,
      ORBIT_A_MATRIX,
//      ORBIT_COVARIANCE,

      // special parameter to handle in GmatFunction
      UTC_GREGORIAN,

      // Hidden parameters used by the PSM
      CARTESIAN_X,
      CARTESIAN_Y,
      CARTESIAN_Z,
      CARTESIAN_VX,
      CARTESIAN_VY,
      CARTESIAN_VZ,
      MASS_FLOW,

      // Hardware for spacecraft
      ADD_HARDWARE,                                             // made changes by Tuan Nguyen
      // The filename used for the spacecraft's model 
      MODEL_FILE,

      // The Offset, rotation, and scale values for the spacecraft's model
      MODEL_OFFSET_X,
      MODEL_OFFSET_Y,
      MODEL_OFFSET_Z,
      MODEL_ROTATION_X,
      MODEL_ROTATION_Y,
      MODEL_ROTATION_Z,
      MODEL_SCALE,
      MODEL_MAX,

      SpacecraftParamCount = MODEL_MAX  // Assumes model params at the end
   };

   enum MultipleReps  // these are IDs for the different representations
   {
      CART_X = 10000,      // Cartesian
      CART_Y,
      CART_Z,
      CART_VX,
      CART_VY,
      CART_VZ,
      KEPL_SMA,            // Keplerian
      KEPL_ECC,
      KEPL_INC,
      KEPL_RAAN,
      KEPL_AOP,
      KEPL_TA,
      KEPL_EA,
      KEPL_MA,
      KEPL_HA,
      MOD_KEPL_RADPER,     // Modified Keplerian
      MOD_KEPL_RADAPO,
      AZFPA_RMAG,          // SphericalAZFPA
      AZFPA_RA,
      AZFPA_DEC,
      AZFPA_VMAG,
      AZFPA_AZI,
      AZFPA_FPA,
      RADEC_RAV,           // SphericalRADEC
      RADEC_DECV,
      EQ_PEY,              // Equinoctial
      EQ_PEX,
      EQ_PNY,
      EQ_PNX,
      EQ_MLONG,
      EndMultipleReps
   };
   // these are the corresponding strings
   static const wxString MULT_REP_STRINGS[EndMultipleReps - CART_X];

   /// Spacecraft parameter types
   static const Gmat::ParameterType
                  PARAMETER_TYPE[SpacecraftParamCount - SpaceObjectParamCount];
   /// Spacecraft parameter labels
   static const wxString
                  PARAMETER_LABEL[SpacecraftParamCount - SpaceObjectParamCount];

   enum STATE_REPS
   {
      CARTESIAN_ID = 0,
      KEPLERIAN_ID,
      MODIFIED_KEPLERIAN_ID,
      SPHERICAL_AZFPA_ID,
      SPHERICAL_RADEC_ID
   };

   static const Integer ATTITUDE_ID_OFFSET;

   std::map <wxString, wxString> elementLabelMap;

   /// State element labels
   StringArray       stateElementLabel;
   /// State element units
   StringArray       stateElementUnits;
   /// Possible state representations
   StringArray       representations;

   /// Epoch string, specifying the text form of the epoch
   wxString       scEpochStr;
   Real              dryMass;
   Real              coeffDrag;
   Real              dragArea;
   Real              srpArea;
   Real              reflectCoeff;
   /// String specifying the epoch time system (A1, TAI, UTC, or TT)
   wxString       epochSystem;
   /// String specifying the epoch time format (Gregorian or ModJulian)
   wxString       epochFormat;
   /// String specifying the epoch system and format used for scEpochStr (TAIModJulian, etc)
   wxString       epochType;
   wxString       stateType;
   wxString       displayStateType;
   wxString       anomalyType;
   Anomaly           trueAnomaly;

   
   // The Offset, Rotation, and Scale values for the model
   Real                 modelOffsetX;
   Real                 modelOffsetY;
   Real                 modelOffsetZ;
   Real                 modelRotationX;
   Real                 modelRotationY;
   Real                 modelRotationZ;
   Real                 modelScale;
   
   /// Solar system now needed to set to cloned Thruster
   SolarSystem       *solarSystem;
   /// Base coordinate system for the Spacecraft
   CoordinateSystem  *internalCoordSystem;
   /// Coordinate system used for the input and output to the GUI
   CoordinateSystem  *coordinateSystem;

   wxString       coordSysName;

   /// coordinate system map to be used for Thrusters for now
   std::map<wxString, CoordinateSystem*> coordSysMap;

   /// Spacecraft ID Used in estimation, measuremetn data files, etc
   wxString       spacecraftId;

   /// Pointer to the object that manages the attitude of the spacecraft
   Attitude          *attitude;

//   /// Orbit SPICE kernel name(s)
//   StringArray       orbitSpiceKernelNames;

   // for non-internal spacecraft information
   StateConverter      stateConverter;
   CoordinateConverter coordConverter;

   // Lists of hardware elements added 11/12/04, djc
   /// Fuel tank names
   StringArray       tankNames;
   /// Thruster names
   StringArray       thrusterNames;
   /// Pointers to the fuel tanks
   ObjectArray       tanks;
   /// Pointers to the spacecraft thrusters
   ObjectArray       thrusters;
   /// Dry mass plus fuel masses, a calculated parameter
   Real              totalMass;

   /// New constructs needed to preserve interfaces
   Rvector6          rvState;

   bool              initialDisplay;
   bool              csSet;
   bool              isThrusterSettingMode;

   /// The orbit State Transition Matrix
   Rmatrix           orbitSTM;
   /// The orbit State A Matrix
   Rmatrix           orbitAMatrix;

   /// Toggle to making Cart state dynamic; Integer to handle multiple includes
   Integer           includeCartesianState;

   // Hardware
   /// List of hardware names used in the spacecraft
   StringArray           hardwareNames;                                 // made changes by Tuan Nguyen
   /// List of hardware objects used in the spacecraft
   ObjectArray           hardwareList;                                  // made changes by Tuan Nguyen

   Real              UpdateTotalMass();
   Real              UpdateTotalMass() const;
   bool              ApplyTotalMass(Real newMass);
   void              DeleteOwnedObjects(bool deleteAttitude, bool deleteTanks,
                                        bool deleteThrusters, bool otherHardware);
   void              CloneOwnedObjects(Attitude *att, const ObjectArray &tnks,
                                       const ObjectArray &thrs);
   void              AttachTanksToThrusters();
   bool              SetHardware(GmatBase *obj, StringArray &hwNames,
                                 ObjectArray &hwArray);
   virtual void      WriteParameters(Gmat::WriteMode mode, wxString &prefix,
                        wxString &stream);

   virtual void      UpdateElementLabels();
   Rvector6          GetStateInRepresentation(wxString rep = wxT(""));
   Rvector6          GetStateInRepresentation(Integer rep = CARTESIAN_ID);
   void              SetStateFromRepresentation(wxString rep, Rvector6 &st);

   Real              GetElement(const wxString &label);
   bool              SetElement(const wxString &label, const Real &value);
   Integer           LookUpLabel(const wxString &label, wxString &rep);
   Integer           LookUpID(const Integer id, wxString &label, wxString &rep);
   void              BuildElementLabelMap();
   void              RecomputeStateAtEpoch(const GmatEpoch &toEpoch);

private:
   bool                          VerifyAddHardware();                   // made changes by Tuan Nguyen

};

#endif // Spacecraft_hpp
