//$Id: EphemerisPropagator.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                             EphemerisPropagator
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Darrel J. Conway
// Created: Mar 26, 2010 by Darrel Conway (Thinking Systems)
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under the FDSS 
// contract, Task 28
//
/**
 * Implementation for the EphemerisPropagator class
 */
//------------------------------------------------------------------------------


#ifndef EphemerisPropagator_hpp
#define EphemerisPropagator_hpp

#include "Propagator.hpp"

/**
 * EphemerisPropagator is the base class for objects that model orbit evolution
 * through interpolation of data in an ephemeris file.
 */
class GMAT_API EphemerisPropagator : public Propagator
{
public:
   EphemerisPropagator(const wxString &typeStr, const wxString &name = wxT(""));
   virtual ~EphemerisPropagator();
   EphemerisPropagator(const EphemerisPropagator& ep);
   EphemerisPropagator& operator=(const EphemerisPropagator& ep);

   // Access methods for the scriptable parameters
   virtual wxString  GetParameterText(const Integer id) const;
   virtual wxString  GetParameterUnit(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;

   // Access methods for the scriptable parameters
   virtual bool         IsParameterReadOnly(const Integer id) const;
   virtual bool         IsParameterReadOnly(const wxString &label) const;

   virtual Real         GetRealParameter(const Integer id) const;
   virtual Real         SetRealParameter(const Integer id,
                                         const Real value);
   virtual Real         GetRealParameter(const Integer id,
                                         const Integer index) const;
   virtual Real         GetRealParameter(const Integer id, const Integer row,
                                         const Integer col) const;
   virtual Real         SetRealParameter(const Integer id,
                                         const Real value,
                                         const Integer index);
   virtual Real         SetRealParameter(const Integer id, const Real value,
                                         const Integer row, const Integer col);
   virtual Real         GetRealParameter(const wxString &label) const;
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value);
   virtual Real         GetRealParameter(const wxString &label,
                                         const Integer index) const;
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value,
                                         const Integer index);
   virtual Real         GetRealParameter(const wxString &label,
                                         const Integer row,
                                         const Integer col) const;
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value, const Integer row,
                                         const Integer col);


   virtual wxString  GetStringParameter(const Integer id) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value);
   virtual wxString  GetStringParameter(const Integer id,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value,
                                           const Integer index);
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value);
   virtual wxString  GetStringParameter(const wxString &label,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value,
                                           const Integer index);

   // Access methods derived classes can override on reference objects
   virtual wxString  GetRefObjectName(const Gmat::ObjectType type) const;
//   virtual const ObjectTypeArray&
//                        GetRefObjectTypeArray();
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool         SetRefObjectName(const Gmat::ObjectType type,
                                         const wxString &name);
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
//   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
//                                     const wxString &name);
//   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
//                                     const wxString &name,
//                                     const Integer index);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name = wxT(""));
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name,
                                     const Integer index);
//   virtual ObjectArray& GetRefObjectArray(const Gmat::ObjectType type);
//   virtual ObjectArray& GetRefObjectArray(const wxString& typeString);

   bool                 UsesODEModel();
   virtual void         SetPropStateManager(PropagationStateManager *sm);
   virtual bool         Initialize();
   virtual bool         Step(Real dt);
   virtual bool         Step() = 0;

   virtual Integer      GetDimension();
   virtual Real*        GetState();
   virtual Real*        GetJ2KState();
   virtual void         UpdateSpaceObject(Real newEpoch = -1.0);
   virtual void         UpdateFromSpaceObject();
   virtual void         RevertSpaceObject();
   virtual void         BufferState();

   virtual Real         GetTime();
   virtual void         SetTime(Real t);
   virtual void         SetSolarSystem(SolarSystem *ss);

   /// Types of initial epoch data supported
   enum StartEpochSource
   {
      FROM_SCRIPT,
      FROM_EPHEM,
      FROM_SPACECRAFT
   };

protected:
   /// Step used to propagate through the ephemeris
   Real                       ephemStep;
   /// Format used for the start epoch data
   wxString                epochFormat;
   /// Start epoch
   wxString                startEpoch;
   /// Initial epoch
   Real                       initialEpoch;
   /// Current epoch
   Real                       currentEpoch;
   /// Current epoch - initial epoch (used to minimize accumulated error)
   Real                       timeFromEpoch;

   /// Start time on the ephem
   GmatEpoch                  ephemStart;
   /// End time on the ephem
   GmatEpoch                  ephemEnd;

   /// Names of the objects that are propagated
   StringArray                propObjectNames;
   /// The propagated objects
   ObjectArray                propObjects;
   /// The (current) ephemeris file names, one per prop object
   StringArray                theEphems;

   /// The propagation state manager, used to manage the state
   PropagationStateManager    *psm;

   /// State vector for the latest propagated vector
   Real                       *state;
   /// Second state vector, in the J2000 frame
   Real                       *j2kState;
   /// Size of the most recent prop step
   Real                       stepTaken;
   /// Type of source used to obtain start epoch
   StartEpochSource           startEpochSource;
   /// Buffer that allows quick reversion to the previous state
   GmatState                  previousState;
   /// Direction of propagation
   Real                       stepDirection;

   ///The solar system that supplies the j2kBody and propOrigin
   SolarSystem                *solarSystem;
   /// Parameter IDs
   enum
   {
      EPHEM_STEP_SIZE = PropagatorParamCount,
      EPHEM_CENTRAL_BODY,
      EPHEM_EPOCH_FORMAT,
      EPHEM_START_EPOCH,
      EphemerisPropagatorParamCount
   };

   /// EphemerisPropagator parameter types
   static const Gmat::ParameterType
         PARAMETER_TYPE[EphemerisPropagatorParamCount - PropagatorParamCount];
   /// EphemerisPropagator parameter labels
   static const wxString
         PARAMETER_TEXT[EphemerisPropagatorParamCount - PropagatorParamCount];

   GmatEpoch ConvertToRealEpoch(const wxString &theEpoch,
         const wxString &theFormat);
   virtual void UpdateState() = 0;

   virtual void SetEphemSpan(const GmatEpoch start, const GmatEpoch);
   virtual void SetEphemSpan(Integer whichOne = 0);
   virtual bool IsValidEpoch(GmatEpoch time);

   virtual void MoveToOrigin(Real newEpoch = -1.0);
   virtual void ReturnFromOrigin(Real newEpoch = -1.0);

};

#endif /* EphemerisPropagator_hpp */
