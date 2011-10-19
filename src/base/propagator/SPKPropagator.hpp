//$Id: SPKPropagator.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                             SPKPropagator
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
 * Implementation for the SPKPropagator class
 */
//------------------------------------------------------------------------------


#ifndef SPKPropagator_hpp
#define SPKPropagator_hpp

/**
 * SPKPropagator ...
 */
#include "EphemerisPropagator.hpp"
#include "SpiceOrbitKernelReader.hpp"


class SPKPropagator : public EphemerisPropagator
{
public:
   SPKPropagator(const wxString &name = wxT(""));
   virtual ~SPKPropagator();
   SPKPropagator(const SPKPropagator& spk);
   SPKPropagator& operator=(const SPKPropagator& spk);

   // Access methods for the scriptable parameters
   virtual wxString  GetParameterText(const Integer id) const;
   virtual wxString  GetParameterUnit(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;

   virtual bool         IsParameterReadOnly(const Integer id) const;
   virtual bool         IsParameterReadOnly(const wxString &label) const;

   virtual wxString  GetStringParameter(const Integer id) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value);
   virtual wxString  GetStringParameter(const Integer id,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value,
                                           const Integer index);
   virtual const StringArray&
                        GetStringArrayParameter(const Integer id) const;
   virtual const StringArray&
                        GetStringArrayParameter(const Integer id,
                                                const Integer index) const;
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value);
   virtual wxString  GetStringParameter(const wxString &label,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value,
                                           const Integer index);
   virtual const StringArray&
                        GetStringArrayParameter(const wxString &label) const;
   virtual const StringArray&
                        GetStringArrayParameter(const wxString &label,
                                                const Integer index) const;

   virtual bool Initialize();
   // virtual bool Step(Real dt);
   // virtual void SetAsFinalStep(bool fs);

   virtual GmatBase* Clone() const;
   virtual bool Step();
   virtual bool RawStep();
   virtual Real GetStepTaken();

protected:
   /// List of the SPICE files that are needed by this propagator
   StringArray spkFileNames;
   /// ID's used to lookup data for the prop objects
   IntegerArray naifIds;
   /// J2000 Epoch from the SPICE library
   SpiceDouble j2ET;

   /// Central body string used by SPK (needed because of Luna/Moon names)
   wxString spkCentralBody;

   SpiceOrbitKernelReader *skr;


   /// Parameter IDs
   enum
   {
      SPKFILENAMES = EphemerisPropagatorParamCount,
      SPKPropagatorParamCount,
   };

   /// SPKPropagator parameter types
   static const Gmat::ParameterType PARAMETER_TYPE[SPKPropagatorParamCount -
                                                   EphemerisPropagatorParamCount];
   /// SPKPropagator parameter labels
   static const wxString PARAMETER_TEXT[SPKPropagatorParamCount -
                                           EphemerisPropagatorParamCount];

   virtual void         UpdateState();

   virtual void         SetEphemSpan(Integer whichOne = 0);
};

#endif /* SPKPropagator_hpp */
