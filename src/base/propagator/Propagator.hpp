//$Id: Propagator.hpp 9923 2011-09-29 15:55:33Z djcinsb $
//------------------------------------------------------------------------------
//                              Propagator
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// *** File Name : Propagator.hpp
// *** Created   : October 1, 2002
// **************************************************************************
// ***  Developed By  :  Thinking Systems, Inc. (www.thinksysinc.com)     ***
// ***  For:  Flight Dynamics Analysis Branch (Code 572)                  ***
// ***  Under Contract:  P.O.  GSFC S-66617-G                             ***
// ***                                                                    ***
// ***  This software is subject to the Sofware Usage Agreement described ***
// ***  by NASA Case Number GSC-14735-1.  The Softare Usage Agreement     ***
// ***  must be included in any distribution.  Removal of this header is  ***
// ***  strictly prohibited.                                              ***
// ***                                                                    ***
// ***                                                                    ***
// ***  Header Version: July 12, 2002                                     ***
// **************************************************************************
// Module Type               : ANSI C++ Source
// Development Environment   : Visual C++ 7.0
// Modification History      : 11/26/2002 - D. Conway, Thinking Systems, Inc.
//                             Original delivery
//
//                           : 12/18/2002 - D. Conway, Thinking Systems, Inc.
//                             Updated interfaces based on GSFC feedback
//
//                           : 09/23/2003 - W. Waktola, Missions Applications Branch
//                              Changes:
//                                - Updated style using GMAT cpp style guide
//
//                           : 10/09/2003 - W. Waktola, Missions Applications Branch
//                              Changes:
//                                - virtual char* GetParameterName(const int parm) const to
//                                  virtual wxString GetParameterName(const int parm) const
//                              Additions:
//                                - GetParameterType()
//                                - GetParameterText()
//                                - GetParameterTypeString()
//                                - GetRealParameter()
//                                - SetRealParameter()
//                              Removals:
//                                - GetParameter()
//                                - SetParameter()
//                                - ParameterCount()
//
//                           : 10/15/2003 - W. Waktola, Missions Applications Branch
//                              Changes:
//                                - All double types to Real types
//                                - All primitive int types to Integer types
//                - STEP_SIZE_PARAMETER to stepSizeParameter
//                              Removals:
//                                - static Real parameterUndefined
//                                - SetUndefinedValue()
//                                - GetParameterName(), replaced by GetParameterText()
//                              Additions:
//                                - PARAMTER_TEXT[]
//                                - PARAMETER_TYPE[]
//
// **************************************************************************

#ifndef Propagator_hpp
#define Propagator_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "PhysicalModel.hpp"
#include "PropagatorException.hpp"      // Propagator Exceptions
#include "PropagationStateManager.hpp"


class GMAT_API Propagator : public GmatBase
{
public:
   Propagator(const wxString &typeStr,
              const wxString &nomme = wxT(""));
   virtual ~Propagator(void);

   Propagator(const Propagator&);
   Propagator& operator=(const Propagator&);

   virtual bool RenameRefObject(const Gmat::ObjectType type,
                                const wxString &oldName,
                                const wxString &newName);

   // Parameter accessor methods -- overridden from GmatBase
   virtual wxString GetParameterText(const Integer id) const;
   virtual Integer GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType GetParameterType(const Integer id) const;
   virtual wxString GetParameterTypeString(const Integer id) const;
   virtual bool IsParameterReadOnly(const Integer id) const;
   virtual bool IsParameterReadOnly(const wxString &label) const;

   virtual Real GetRealParameter(const Integer id) const;
   virtual Real GetRealParameter(const wxString &label) const;
   virtual Real SetRealParameter(const Integer id, const Real value);
   virtual Real SetRealParameter(const wxString &label, const Real value);

   virtual Real GetRealParameter(const Integer id, const Integer index) const;
   virtual Real GetRealParameter(const Integer id, const Integer row,
         const Integer col) const;
   virtual Real SetRealParameter(const Integer id, const Real value,
         const Integer index);
   virtual Real SetRealParameter(const Integer id, const Real value,
         const Integer row, const Integer col);

   virtual bool GetBooleanParameter(const Integer id) const;
   virtual bool SetBooleanParameter(const Integer id, const bool value);
   virtual bool GetBooleanParameter(const Integer id, const Integer index) const;
   virtual bool SetBooleanParameter(const Integer id, const bool value,
                                    const Integer index);
   virtual bool GetBooleanParameter(const wxString &label) const;
   virtual bool SetBooleanParameter(const wxString &label, const bool value);
   virtual bool GetBooleanParameter(const wxString &label,
                                    const Integer index) const;
   virtual bool SetBooleanParameter(const wxString &label, const bool value,
                                    const Integer index);

   virtual bool Initialize();
   virtual void SetPhysicalModel(PhysicalModel *pPhysicalModel);
   virtual bool Step(Real dt);
   virtual void SetAsFinalStep(bool fs);

   virtual void Update(bool forwards = true);
   virtual void ResetInitialData();
   const Real* AccessOutState();

   virtual Integer GetPropagatorOrder(void) const;
   virtual bool UsesODEModel();

   // Methods that control behavior for analytic or interpolation evolution
   // operators
   virtual void SetPropStateManager(PropagationStateManager *sm);
   virtual Integer GetDimension();
   virtual Real* GetState();
   virtual Real* GetJ2KState();
   virtual void UpdateSpaceObject(Real newEpoch = -1.0);
   virtual void UpdateFromSpaceObject();
   virtual void RevertSpaceObject();
   virtual void BufferState();

   virtual Real GetTime();
   virtual void SetTime(Real t);

   virtual bool PropagatesForward();
   virtual void SetForwardPropagation(bool tf);

   // Abstract methods

   //---------------------------------------------------------------------------
   // virtual bool Step(void)
   //---------------------------------------------------------------------------
   /**
    * Method to propagate by the default step
    * This pure virtual method provides the interface used to advance the state
    * by the amount specified in the stepSize parameter.  This is the core
    * propagation routine.
    * Adaptive step propagators will take the desired step if accuracy permits,
    * and then adjust the stepSize parameter to the estimated optimal size for
    * the next call.
    */
   //---------------------------------------------------------------------------
   virtual bool Step() = 0;

   //---------------------------------------------------------------------------
   // virtual bool RawStep(void)
   //---------------------------------------------------------------------------
   /**
    * Method to take a raw propagation step without error control
    */
   virtual bool RawStep() = 0;

   //---------------------------------------------------------------------------
   // virtual bool GetStepTaken()
   //---------------------------------------------------------------------------
   /**
    * Method to report the size of the most recent propagation step.
    */
   virtual Real                GetStepTaken() = 0;

   virtual bool RawStep(Real dt);

   enum
   {
      INITIAL_STEP_SIZE = GmatBaseParamCount, /// Stepsize for the propagation
      AlwaysUpdateStepsize,
      PropagatorParamCount                    /// Count of the parameters for this class
   };

protected:

   // Start with the parameter IDs and associated strings
   static const wxString
      PARAMETER_TEXT[PropagatorParamCount - GmatBaseParamCount];
   static const Gmat::ParameterType
      PARAMETER_TYPE[PropagatorParamCount - GmatBaseParamCount];

   static const Real STEP_SIZE_TOLERANCE;

   /// Size of the default time step
   Real stepSize;
   /// Buffer for the stepsize.
   Real stepSizeBuffer;
   /// Flag used to determine if the propagator has been initialized
   bool initialized;
   /// Flag used to determine if the propagator need to reset the initial step
   bool resetInitialData;
   /// Flag to force stepsize initialization whenever the propagator initializes
   bool alwaysUpdateStepsize;
   /// Pointer to the input state vector
   Real *inState;
   /// Pointer to the location for the propagated state
   Real *outState;
   /// Number of parameters being propagated
   Integer dimension;
   /// Pointer to the information related to the physics of the system
   PhysicalModel *physicalModel;
   /// Flag used to detect if the code is taking the last Propagate step
   bool finalStep;

   // Pieces for prop with origin code
   /// Name of the common J2000 body that the state providers all use
   wxString               j2kBodyName;
   /// Pointer to the J2000 body
   CelestialBody             *j2kBody;
   /// Name of the central body
   wxString                centralBody;
   /// Pointer to the central body
   SpacePoint                *propOrigin;

   virtual void MoveToOrigin(Real newEpoch = -1.0);
   virtual void ReturnFromOrigin(Real newEpoch = -1.0);

};

#endif // Propagator_hpp

