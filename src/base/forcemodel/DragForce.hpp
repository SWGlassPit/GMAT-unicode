//$Id: DragForce.hpp 9757 2011-08-10 22:47:51Z djcinsb $
//------------------------------------------------------------------------------
//                                DragForce
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P
//
// Author: Darrel J. Conway
// Created: 2004/02/29
//
/**
 * Drag force modeling
 */
//------------------------------------------------------------------------------


#ifndef DragForce_hpp
#define DragForce_hpp

#include "PhysicalModel.hpp"
#include "AtmosphereModel.hpp"
#include "TimeTypes.hpp"
#include <vector>

// Forward reference
class CoordinateSystem;


/**
 * Class used to model accelerations due to drag.
 */
class GMAT_API DragForce : public PhysicalModel
{
public:
   DragForce(const wxString &name = wxT(""));
   virtual ~DragForce();
   
   DragForce(const DragForce& df); 
   DragForce&           operator=(const DragForce& df); 
   
   virtual bool         GetComponentMap(Integer * map, Integer order = 1) const;
   virtual void         SetSatelliteParameter(const Integer i,
                                              const wxString parmName, 
                                              const Real parm,
                                              const Integer parmID = -1);
   virtual void         SetSatelliteParameter(const Integer i,
                                              Integer parmID,
                                              const Real parm);
   virtual void         SetSatelliteParameter(const Integer i,
                                              const wxString parmName, 
                                              const wxString parm);
   virtual void         ClearSatelliteParameters(const wxString parmName = wxT(""));
   
   bool                 Initialize();
   virtual bool         GetDerivatives(Real * state, Real dt = 0.0, 
                                       Integer order = 1, 
                                       const Integer id = -1);
   
   // inherited from GmatBase
   virtual GmatBase*    Clone(void) const;
   
   // Parameter accessor methods -- overridden from GmatBase
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   virtual bool         IsParameterReadOnly(const Integer id) const;
   virtual bool         IsParameterReadOnly(const wxString &label) const;
   
   virtual Real         GetRealParameter(const Integer id) const;
   virtual Real         GetRealParameter(const wxString &label) const;
   virtual Real         SetRealParameter(const Integer id, const Real value);
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value);
   
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value);
   virtual bool         SetStringParameter(const wxString &label, 
                                           const wxString &value);
   virtual Integer      GetIntegerParameter(const Integer id) const;
   virtual Integer      SetIntegerParameter(const Integer id,
                                            const Integer value);
   virtual Integer      GetIntegerParameter(const Integer id,
                                            const Integer index) const;
   virtual Integer      SetIntegerParameter(const Integer id,
                                            const Integer value,
                                            const Integer index);
   virtual Integer      GetIntegerParameter(const wxString &label) const;
   virtual Integer      SetIntegerParameter(const wxString &label,
                                            const Integer value);
   virtual Integer      GetIntegerParameter(const wxString &label,
                                            const Integer index) const;
   virtual Integer      SetIntegerParameter(const wxString &label,
                                            const Integer value,
                                            const Integer index);

   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name = wxT(""));
   
   virtual Integer      GetOwnedObjectCount();
   virtual GmatBase*    GetOwnedObject(Integer whichOne);
   
   // Special access methods used by drag forces
   bool                 SetInternalAtmosphereModel(AtmosphereModel* atm);
   AtmosphereModel*     GetInternalAtmosphereModel();
   
   // Methods used by the ODEModel to set the state indexes, etc
   virtual bool SupportsDerivative(Gmat::StateElementId id);
   virtual bool SetStart(Gmat::StateElementId id, Integer index, 
                         Integer quantity);

   
protected:
   /// Sun pointer for bulge calculations
   CelestialBody        *sun;
   /// Position of the Sun
   Real                 sunLoc[3];
   /// Central body pointer for bulge calculations
   CelestialBody        *centralBody;
   /// Position of the body with the atmosphere
   Real                 cbLoc[3];
   /// Angular velocity of the central body
   Real                 *angVel;
   /// Flag to indicate if the atmosphere model is externally owned or internal
   bool                 useExternalAtmosphere;
   /// Name of the atmosphere model we want to use
   wxString          atmosphereType;
   /// Pointer to the atmosphere model used
   AtmosphereModel      *atmos;
   /// Pointer to Internal atmosphere model
   AtmosphereModel      *internalAtmos;
   /// Array of densities
   Real                 *density;
   /// Array of products of spacecraft properties
   Real                 *prefactor;
   /// Flag used to determine if data has changed for the prefactors
   bool                 firedOnce;
   /// Flag used to indicate that local wind calcs are used
   bool                 hasWindModel;
   /// Number of spacecraft in the state vector that use CartesianState
   Integer              satCount;
   /// Central bodies used for atmosphere source
   StringArray          dragBody;
   /// Spacecraft drag areas
   std::vector <Real>   area;
   /// Spacecraft masses
   std::vector <Real>   mass;
   /// Spacecraft coefficients of drag
   std::vector <Real>   dragCoeff;
   /// Size of the CartesianState data -- in other words, 6 * satCount
   Integer              orbitDimension;
   /// State vector translated from force model origin to body with atmosphere
   Real                 *dragState;
   /// Interval of angular momentum vector updates, in days
   Real                 wUpdateInterval;
   /// Epoch of last angular momentum update
   Real                 wUpdateEpoch;
   
   /// ID used to retrieve mass data
   Integer massID;
   /// ID used to retrieve coefficient of drag
   Integer cdID;
   /// ID used to retrieve drag area
   Integer areaID;
   /// ID used to set F10.7
   Integer F107ID;
   /// ID used to set F10.7A
   Integer F107AID;
   /// ID used to set Geomagnetic index
   Integer KPID;


   // Optional input parameters used by atmospheric models
   /// Type of input data -- wxT("File") or wxT("Constant")
   wxString          dataType;
   /// Solar flux file name
   wxString          fluxFile;
   /// wxT("Current") value of F10.7
   Real                 fluxF107;
   /// Running average of the F10.7
   Real                 fluxF107A;
   /// Magnetic field index, Ap (a calculated value)
   Real                 ap;
   /// Magnetic field index, Kp (user specified)
   Real                 kp;

   /// Start index for the Cartesian state
   Integer              cartIndex;
   /// Flag indicating if the Cartesian state should be populated
   bool                 fillCartesian;
   /// CS used to get the w cross r piece
   CoordinateSystem     *cbFixed;
   /// CS used to for conversions
   CoordinateSystem     *internalCoordSystem;
   /// Index used to select Kp/Ap conversion method.  Default is a table lookup
   Integer              kpApConversion;


   
   void                 BuildPrefactors();
   void                 TranslateOrigin(const Real *state, const Real now);
   void                 GetDensity(Real *state, Real when = GmatTimeConstants::MJD_OF_J2000);
      
   Real                 CalculateAp(Real kp);
   
   
   /// Parameter IDs
   enum
   {
      ATMOSPHERE_MODEL = PhysicalModelParamCount, 
      ATMOSPHERE_BODY,
      SOURCE_TYPE,
      FLUX_FILE,
      FLUX,
      AVERAGE_FLUX,
      MAGNETIC_INDEX,
      FIXED_COORD_SYSTEM,
      W_UPDATE_INTERVAL,
      KP2AP_METHOD,
      DragForceParamCount
   };
   
   static const wxString 
      PARAMETER_TEXT[DragForceParamCount - PhysicalModelParamCount];
   static const Gmat::ParameterType 
      PARAMETER_TYPE[DragForceParamCount - PhysicalModelParamCount];
};

#endif // DragForce_hpp
