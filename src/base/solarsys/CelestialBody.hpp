//$Id: CelestialBody.hpp 9723 2011-07-20 19:11:32Z wendys-dev $
//------------------------------------------------------------------------------
//                                  CelestialBody
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
// Author: Wendy C. Shoan
// Created: 2004/01/28
//
/**
 * This is the base class for celestial bodies.
 *
 * @note This is an abstract class.
 * @note Class (initial shell only) automatically generated by Dev-C++ New
 *       Class wizard (heavily modified after that)
 */
//------------------------------------------------------------------------------

#ifndef CelestialBody_hpp
#define CelestialBody_hpp

#include <math.h>
#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "SpacePoint.hpp"
#include "A1Mjd.hpp"
#include "PlanetaryEphem.hpp"
#include "AtmosphereModel.hpp"
#include "Rmatrix.hpp"
#include "Rvector6.hpp"
#include "TimeTypes.hpp"
#ifdef __USE_SPICE__
#include "SpiceOrbitKernelReader.hpp"
#endif
// forward reference for SolarSystem
class SolarSystem;

// Add needed things to Gmat namespace
namespace Gmat
{
   // possible sources of position and velocity data for celestial bodies
   enum PosVelSource
   {
      TWO_BODY_PROPAGATION,
      DE405,
      SPICE,
      PosVelSourceCount
   };

   const wxString POS_VEL_SOURCE_STRINGS[PosVelSourceCount] =
   {
      wxT("TwoBodyPropagation"),
      wxT("DE405"),
      wxT("SPICE")
   };

   // possible types of celestial bodies
   enum BodyType
   {
      STAR = 0,
      PLANET,
      MOON,
      ASTEROID,
      COMET,
      SPECIAL_CELESTIAL_POINT,
      KUIPER_BELT_OBJECT,   // KBOs not yet implemented
      BodyTypeCount
   };
   
   const wxString BODY_TYPE_STRINGS[BodyTypeCount] = 
   {
      wxT("Star"),
      wxT("Planet"),
      wxT("Moon"),
      wxT("Asteroid"),
      wxT("Comet"),
      wxT("SpecialCelestialPoint"),
      wxT("KuiperBeltObject"),   // KBOs not yet implemented
   };
   
   // types of environment models for a body
   enum ModelType
   {
      ATMOSPHERE_MODEL = 0,
      GRAVITY_FIELD,
      MAGNETIC_FIELD,
      SHAPE_MODEL,     // reserved for future use
      ModelTypeCount
   };
   
   const wxString MODEL_TYPE_STRINGS[ModelTypeCount] =
   {
      wxT("AtmosphereModel"),
      wxT("GravityField"),
      wxT("MagneticField"),
      wxT("ShapeModel"),    // reserved for future use
   };
   
   enum RotationDataSource
   {
      DE_405_FILE = 0,
      IAU_2002,
//      IAU_FILE,   // TBD
      FK5_IAU_1980,
      IAU_SIMPLIFIED,
      RotationDataSrcCount
   };
   
   const wxString ROTATION_DATA_SOURCE_STRINGS[RotationDataSrcCount] = 
   {
      wxT("DE405File"),
      wxT("IAU2002"),
//      wxT("IAUFile"),  // TBD
      wxT("FK5IAU1980"),
      wxT("IAUSimplified"),
   };
};

/**
 * CelestialBody base class, from which all types of celestial bodies will derive.
 *
 * The CelestialBody class is primarily an intermediate base class, from which
 * all types of celestial bodies will derive.  CelestialBody itself derives from
 * SpacePoint.
 */
class GMAT_API CelestialBody : public SpacePoint
{
public:
   
   // additional constructor, specifying body type (as string) and name
   CelestialBody(wxString itsBodyType, wxString name);
   // additional constructor, specifying type (as Gmat::BodyType) and name
   CelestialBody(Gmat::BodyType itsBodyType, wxString name);
   // copy constructor
   CelestialBody(const CelestialBody &cBody);
   // operator=
   CelestialBody& operator=(const CelestialBody &cBody);
   // destructor
   virtual ~CelestialBody();

   virtual bool Initialize();
   virtual void SetUpBody();
   
   // method to return the state (position and velocity) of the body at
   // the specified time, using the specified method
   virtual const Rvector6&      GetState(A1Mjd atTime);
   virtual const Rvector6&      GetState(Real atTime); 
   virtual void                 GetState(const A1Mjd &atTime, Real *outState);
   // Moved to SpacePoint (LOJ: 2010.11.19)
   //virtual void                 SetSolarSystem(SolarSystem *ss);
#ifdef __USE_SPICE__
   virtual void                 SetSpiceOrbitKernelReader(SpiceOrbitKernelReader *skr);
#endif
   
   // methods to return the body type, central body, gravitational constant,
   // radius, mass, posvel source, analytic method, and userDefined flag
   virtual Gmat::BodyType       GetBodyType() const;
   virtual const wxString&   GetCentralBody() const;
   virtual Real                 GetGravitationalConstant();
   virtual Real                 GetEquatorialRadius();
   virtual Real                 GetFlattening() const;
   virtual Real                 GetPolarRadius();
   virtual Real                 GetMass();
   virtual Gmat::PosVelSource   GetPosVelSource() const;
   virtual wxString          GetSourceFileName() const;
   virtual PlanetaryEphem*      GetSourceFile() const;
   virtual bool                 GetUsePotentialFile() const;
   virtual bool                 GetOverrideTimeSystem() const;
   virtual Real                 GetEphemUpdateInterval() const;
   virtual StringArray          GetValidModelList(Gmat::ModelType m) const;
   virtual const Rvector3&      GetAngularVelocity();             // rad/sec

   virtual Real                 GetHourAngle(A1Mjd atTime); 
   
//   virtual const Rmatrix&       GetHarmonicCoefficientsSij();
//   virtual const Rmatrix&       GetHarmonicCoefficientsCij();
   virtual Integer              GetDegree();
   virtual Integer              GetOrder();
   virtual wxString          GetAtmosphereModelType();
   virtual AtmosphereModel*     GetAtmosphereModel();
   virtual bool                 GetDensity(Real *position, Real *density,
                                        Real epoch = GmatTimeConstants::MJD_OF_J2000,
                                        Integer count = 1);
   // methods to get the initial epoch and keplerian elements 
   virtual A1Mjd                GetTwoBodyEpoch() const;
   virtual Rvector6             GetTwoBodyElements() const; 
   virtual Gmat::RotationDataSource 
                                GetRotationDataSource() const;
   virtual StringArray          GetRotationDataSourceList() const;
   virtual bool                 IsUserDefined() const;
   virtual StringArray          GetEphemSourceList() const;
   virtual Rvector6             GetOrientationParameters() const;
   

   // methods to set data for the body
   virtual bool           SetBodyType(Gmat::BodyType bType);
   virtual bool           SetCentralBody(const wxString &cBody);
   virtual bool           SetGravitationalConstant(Real newMu);
   virtual bool           SetEquatorialRadius(Real newEqRadius);
   virtual bool           SetFlattening(Real flat);
   virtual bool           SetSource(Gmat::PosVelSource pvSrc);
   virtual bool           SetSourceFile(PlanetaryEphem *src);
   virtual bool           SetAllowSpice(const bool allow);
   virtual bool           SetUsePotentialFile(bool useIt);
   
   virtual bool           SetOverrideTimeSystem(bool overrideIt);
   virtual bool           SetEphemUpdateInterval(Real intvl);
   virtual bool           AddValidModelName(Gmat::ModelType m, 
                                            const wxString &newModel);
   virtual bool           RemoveValidModelName(Gmat::ModelType m, 
                                               const wxString &modelName);
   virtual bool           SetValidModelList(Gmat::ModelType m, const StringArray &toList);
   virtual bool           SetOrder(Integer toOrder);
   virtual bool           SetDegree(Integer toDegree);
//   virtual bool           SetHarmonicCoefficientsSij(const Rmatrix &coeffSij);
//   virtual bool           SetHarmonicCoefficientsCij(const Rmatrix &coeffCij);

   
   virtual bool           SetAtmosphereModelType(wxString toAtmModelType);
   virtual bool           SetAtmosphereModel(AtmosphereModel *toAtmModel);
   virtual bool           SetPotentialFilename(const wxString &fn);
   virtual bool           SetTwoBodyEpoch(const A1Mjd &toTime);
   virtual bool           SetTwoBodyElements(const Rvector6 &kepl);
   virtual bool           SetSMA(Real value);
   virtual bool           SetECC(Real value);   
   virtual bool           SetRotationDataSource(Gmat::RotationDataSource src);
   virtual bool           SetUserDefined(bool userDefinedBody);
   
   // methods inherited from SpacePoint, that must be implemented here (and/or
   // in the derived classes
   virtual const Rvector6 GetMJ2000State(const A1Mjd &atTime);
   virtual const Rvector3 GetMJ2000Position(const A1Mjd &atTime);
   virtual const Rvector3 GetMJ2000Velocity(const A1Mjd &atTime);

   // Inputs to SetOrientationParameters are in the order:
   // SpinAxisRAConstant
   // SpinAxisRARate
   // SpinAxisDECConstant
   // SpinAxisDECRate
   // RotationConstant
   // RotationRate
   //
   // Currently, for all non-Earth bodies (except for the Moon and Neptune), 
   // cartographic coordinates are computed as follows:
   //   alpha = SpinAxisRAConstant + SpinAxisRARate * T
   //   delta = SpinAxisDECConstant + SpinAxisDECRate * T
   //   W     = RotationConstant + RotationRate * d;
   //   Wdot  = RotationRate;
   // where T = Julian Days from TCB epoch
   //       d = Julian centuries from TCB epoch
   //
   // @todo - we will need to use more terms for moons in the future
   //
   virtual bool          SetOrientationParameters(const Rvector6 &orient);
   // method to return the alpha, delta, W, and Wdot for the body (specifying
   // the direction of the north pole of rotation (right ascension and
   // declination) and the prime meridian.  
   // Units are degrees (alpha), degrees (delta), and degrees (W); also,
   // the time derivative of W (deg/day)
   // For more information, see
   // "Report of the IAU/IAG Working Group on Cartographic Coordinates and
   // Rotational Elements of the Planets and Satellites: 2000"
   virtual Rvector       GetBodyCartographicCoordinates(const A1Mjd &forTime) const;
   
   
   // Parameter access methods - overridden from GmatBase
   virtual wxString    GetParameterText(const Integer id) const;     // const?
   virtual Integer        GetParameterID(const wxString &str) const; // const?
   virtual Gmat::ParameterType
                          GetParameterType(const Integer id) const;
   virtual wxString    GetParameterTypeString(const Integer id) const;

   virtual Real           GetRealParameter(const Integer id) const;
   virtual Real           SetRealParameter(const Integer id,
                                           const Real value);
   virtual Integer        GetIntegerParameter(const Integer id) const; // const?
   virtual Integer        SetIntegerParameter(const Integer id,
                                              const Integer value); // const?
   virtual wxString    GetStringParameter(const Integer id) const; // const?
   virtual wxString    GetStringParameter(const Integer id,
                                             const Integer index) const;
   virtual bool           SetStringParameter(const Integer id, 
                                             const wxString &value); // const?
   virtual bool           SetStringParameter(const Integer id, const wxString &value,
                                             const Integer index);
   virtual bool           GetBooleanParameter(const Integer id) const; // const?
   virtual bool           SetBooleanParameter(const Integer id,
                                              const bool value); // const?
   virtual const Rvector&    GetRvectorParameter(const Integer id) const;
   virtual const Rvector&    SetRvectorParameter(const Integer id,
                                                 const Rvector &value);
   virtual const Rvector&    GetRvectorParameter(const wxString &label) const;
   virtual const Rvector&    SetRvectorParameter(const wxString &label,
                                                 const Rvector &value);
   virtual const StringArray& GetStringArrayParameter(const Integer id) const;

   virtual GmatBase*   GetRefObject(const Gmat::ObjectType type,
                                    const wxString &name);
   const StringArray&  GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool        SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                    const wxString &name = wxT(""));
   
   virtual bool        IsParameterReadOnly(const Integer id) const;
   virtual bool        IsParameterCloaked(const Integer id) const;
   virtual bool        IsParameterEqualToDefault(const Integer id) const;
   virtual bool        SaveAllAsDefault();
   virtual bool        SaveParameterAsDefault(const Integer id);

   // need methods to get/set stateTime (a1MJD type)?

   //virtual const Rmatrix&       GetCoefDriftS();
   //virtual const Rmatrix&       GetCoefDriftC();
   //virtual bool           SetPhysicalParameters(Real bodyMass, Real bodyEqRad,
   //                                             Real bodyPolarRad, Real bodyMu,
   //                                             Integer coeffSize, Rmatrix& bodySij,
   //                                             Rmatrix& bodyCij);
   
   
   
   // strings representing the possible celestial body types
   //static const wxString BODY_TYPE_STRINGS[Gmat::BodyTypeCount];
   //static const wxString POS_VEL_STRINGS[Gmat::PosVelSourceCount];

   // local constants
   static const Integer BUFSIZE               = 256;
   
   //static const Integer MAX_DEGREE            = 360;
   //static const Integer MAX_ORDER             = 360;
   //static const Integer GRAV_MAX_DRIFT_DEGREE = 2;

protected:

   enum
   {
      BODY_TYPE = SpacePointParamCount, 
      MASS,
      EQUATORIAL_RADIUS,
      FLATTENING,
      POLAR_RADIUS,
      MU,
      POS_VEL_SOURCE,
      STATE,
      STATE_TIME,
      CENTRAL_BODY,
      BODY_NUMBER,
      REF_BODY_NUMBER,
      SOURCE_FILENAME,
      SOURCE_FILE,
      USE_POTENTIAL_FILE_FLAG,
      POTENTIAL_FILE_NAME,
      ANGULAR_VELOCITY,
      HOUR_ANGLE,
      ATMOS_MODEL_NAME,
      ORDER,  // may need to access these through general methods at some point
      DEGREE,
      ROTATION_DATA_SRC,
      TWO_BODY_DATE_FORMAT,
      TWO_BODY_STATE_TYPE,
      TWO_BODY_INITIAL_EPOCH,
      TWO_BODY_SMA,
      TWO_BODY_ECC,
      TWO_BODY_INC,
      TWO_BODY_RAAN,
      TWO_BODY_AOP,
      TWO_BODY_TA,
      //
      ORIENTATION_DATE_FORMAT,
      ORIENTATION_EPOCH,
      SPIN_AXIS_RA_CONSTANT,
      SPIN_AXIS_RA_RATE,
      SPIN_AXIS_DEC_CONSTANT,
      SPIN_AXIS_DEC_RATE,
      ROTATION_CONSTANT,
      ROTATION_RATE,
      //
      TEXTURE_MAP_FILE_NAME,
      // @todo - add Shape Models, etc.
      CelestialBodyParamCount
   };
   static const wxString PARAMETER_TEXT[CelestialBodyParamCount - SpacePointParamCount];

   static const Gmat::ParameterType PARAMETER_TYPE[CelestialBodyParamCount - SpacePointParamCount];
   static const Real    JD_EPOCH_2000_TCB;
   static const Real    JD_EPOCH_2000_TT;
   static const Real    dDot;
   static const Real    TDot;
   static const Real    KEPLER_TOL;
   static const Integer KEPLER_MAX_ITERATIONS;
   
   // body type of the body
   Gmat::BodyType           bodyType;
   // mass
   Real                     mass;
   /// radius of the body at the equator
   Real                     equatorialRadius;
   /// flattening - used to compute polar radius
   Real                     flattening;
   /// radius of the body from center to the pole
   Real                     polarRadius;
   /// gravitational constant in km^3/s^2
   Real                     mu;
   /// source for position and velocity
   Gmat::PosVelSource       posVelSrc;
   /// state of the body 0-2 position 3-5 velocity
   Rvector6                 state;
   // time of the state
   A1Mjd                    stateTime;
   
   /// name of central body around which this body revolves
   wxString              theCentralBodyName;
   /// central body around which this body revolves
   CelestialBody            *theCentralBody;
   /// flag indicating whether or not the central body has been set
   bool                     centralBodySet;
   /// body number for the SLP file
   Integer                  bodyNumber;
   /// body number of origin of coordinate system for file
   Integer                  referenceBodyNumber;
   /// name of file that is the source of position and velocity for this body (DE)
   wxString              sourceFilename;
   /// the source file (DE)
   PlanetaryEphem           *theSourceFile;
   #ifdef __USE_SPICE__
      /// the SPICE file (kernel) reader
      SpiceOrbitKernelReader      *kernelReader;
   #endif
   
   /// flag indicating whether or not to get data from potential file
   bool                     usePotentialFile;
   /// file name of the potential file to use
   wxString              potentialFileName;
   /// angular velocity
   Rvector3                 angularVelocity;
   /// the hour angle 
   Real                     hourAngle;
   /// pointer to the atmosphere model to use for the body
   AtmosphereModel*         atmModel;
   /// the type of the atmosphere model (e.g. wxT("Exponential"))
   wxString              atmModelType;

   /// has the potential file been read already?
   bool                     potentialFileRead;
   // default values for the (non-read-only) celestial body parameters
   /// default eauatorial radius to use if potential file is not used
   Real                     default_equatorialRadius;
   /// default flattening coefficient
   Real                     default_flattening;
   /// default mu to use if potential file is not used
   Real                     default_mu;
   /// default value for the ephem source
   wxString              default_posVelSrc;
   /// default value for the central body
   wxString              default_centralBodyName;
   /// default value for the ephemeris file
   wxString              default_sourceFilename;
   /// default values for the SPICe kernel names
   StringArray              default_orbitSpiceKernelNames;
   /// default value for rotation data source
   Gmat::RotationDataSource default_rotationSrc; 
   /// default value for two-body intial epoch
   A1Mjd                    default_twoBodyEpoch;
   /// default value for initial two-body state
   Rvector6                 default_twoBodyKepler;
   /// default value for orientation epoch
   A1Mjd                    default_orientationEpoch;
   /// default value for orientation values in the order:
   /// SpinAxisRAConstant, SpinAxisRARate, SpinAxisDECConstant, 
   /// SpinAxisDECRate, RotationConstant, RotationRate
   Rvector6                 default_orientation;
   /// default value for texture map file name
   wxString              default_textureMapFileName;

   
   /// order of the gravity model
   Integer                order;    
   /// degree of the gravity model
   Integer                degree;  
//   /// spherical harmonic coefficients (Sij) for the body
//   Rmatrix                sij;
//   /// spherical harmonic coefficients (Cij) for the body
//   Rmatrix                cij;
   /// date format for the twoBody method epoch
   wxString            twoBodyFormat;
   /// state type for twoBody inputs
   wxString            twoBodyStateType;
   /// Initial epoch for twoBody method
   A1Mjd                  twoBodyEpoch;
   /// initial Keplerian elements for twoBody method 
   /// (with respect to central body)
   Rvector6               twoBodyKepler; 
   /// most recent epoch for twoBody method
   A1Mjd                  prevTwoBodyEpoch;  
   /// most recent state (cartesian - wrt the Earth) for twoBody method
   Rvector6               prevTwoBodyState;
   /// flag indicating whether or not the twoBody method epoch and 
   /// state have been modified
   bool                   newTwoBody;
   /// flag indicating whether or not to override the TDB/TCB tiems with TT
   bool                   overrideTime;
   /// update interval for the ephemeris calculations (file-reading)
   Real                   ephemUpdateInterval;
   /// last time that the state was calculated
   A1Mjd                  lastEphemTime;
   /// last state value calculated
   Rvector6               lastState;
   
   Real                   prevState[6];
   
   /// lists of valid models
   StringArray            models[Gmat::ModelTypeCount];

   /// source to use for computing rotation data
   Gmat::RotationDataSource rotationSrc;   // 0 -> DE405,  1 -> IAU (see above)
   
   /// flag specifying whether or not the body was user-defined 
   /// (i.e. not a default solar system body)
   bool                   userDefined;
   /// flag indicating whether or not SPICE is allowed as position/velocity source
   /// for this (default) body
   bool                   allowSpice;
   
   /// date format for the orientation epoch
   wxString            orientationDateFormat;
   /// initial epoch for the orientation parameters
   A1Mjd                  orientationEpoch;  // A1Mjd?  Ew.
   /// orientation parameters for the body in the order: 
   /// SpinAxisRAConstant, SpinAxisRARate, SpinAxisDECConstant, SpinAxisDECRate, RotationConstant, RotationRate
   Rvector6               orientation;   
   // has the naifID been set (figured out from SPK file(s))
   bool                   naifIdSet;
   /// name to use when requesting data from an SPK kernel
   wxString            naifName;
   /// Name of the texture map file to use when plotting
   wxString            textureMapFileName;
   /// has message about possible needed SPKs been written
   bool                   msgWritten;
   /// date and time of start of source file
   //A1Mjd                  sourceStart;      // currently unused
   /// date and time of end of sourcce file
   //A1Mjd                  sourceEnd;        // currently unused
   //Integer                coefficientSize;      // n   // same as degree, order above?
   //Rmatrix                Cbar, Sbar;
   //Rmatrix                dCbar, dSbar; // from original GravityField
   //Integer                defaultCoefSize;
   //Rmatrix                defaultSij;
   //Rmatrix                defaultCij;
   
   // initialze the body
   void             InitializeBody(wxString withBodyType = wxT("Planet"));
   // methods to read the potential file, if requested
   virtual bool     DeterminePotentialFileNameFromStartup();
   bool             ReadPotentialFile();
   
   bool             IsBlank(wxString &aLine);
   virtual Real     GetJulianDaysFromTCBEpoch(const A1Mjd &forTime) const;
   virtual Rvector6 ComputeTwoBody(const A1Mjd &forTime);
   virtual Rvector6 KeplersProblem(const A1Mjd &forTime);
   virtual bool     SetUpSPICE();
   virtual bool     NeedsOnlyMainSPK();
   
private:

   bool isFirstTimeMu;
   bool isFirstTimeRadius;
};
#endif // CelestialBody_hpp

