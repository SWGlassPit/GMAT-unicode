//$Id: SPKPropagator.cpp 9513 2011-04-30 21:23:06Z djcinsb $
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


#include "SPKPropagator.hpp"
#include "MessageInterface.hpp"
#include "FileManager.hpp"

//#define DEBUG_INITIALIZATION
//#define DEBUG_PROPAGATION
//#define TEST_TDB_ROUND_TRIP

//---------------------------------
// static data
//---------------------------------

/// SPKPropagator parameter labels
const wxString SPKPropagator::PARAMETER_TEXT[
                 SPKPropagatorParamCount - EphemerisPropagatorParamCount] =
{
      wxT("SPKFiles")                    //SPKFILENAMES
};

/// SPKPropagator parameter types
const Gmat::ParameterType SPKPropagator::PARAMETER_TYPE[
                 SPKPropagatorParamCount - EphemerisPropagatorParamCount] =
{
      Gmat::STRINGARRAY_TYPE        //SPKFILENAMES
};


//---------------------------------
// public
//---------------------------------

//------------------------------------------------------------------------------
// SPKPropagator(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Default constructor
 *
 * @param name The name of the object that gets constructed
 */
//------------------------------------------------------------------------------
SPKPropagator::SPKPropagator(const wxString &name) :
   EphemerisPropagator        (wxT("SPK"), name),
   skr                        (NULL)
{
   // GmatBase data
  objectTypeNames.push_back(wxT("SPK"));
  parameterCount = SPKPropagatorParamCount;

  spkCentralBody = centralBody;
}


//------------------------------------------------------------------------------
// ~SPKPropagator()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
SPKPropagator::~SPKPropagator()
{
   if (skr)
   {
      // unload the SPK kernels so they will not be retained in the kernel
      // pool
      skr->UnloadKernels(spkFileNames);
      delete skr;
   }
}


//------------------------------------------------------------------------------
// SPKPropagator(const SPKPropagator & spk)
//------------------------------------------------------------------------------
/**
 * Copy constructor
 *
 * @param spk The object that is copied into this new one
 */
//------------------------------------------------------------------------------
SPKPropagator::SPKPropagator(const SPKPropagator & spk) :
   EphemerisPropagator        (spk),
   spkCentralBody             (spk.spkCentralBody),
   skr                        (NULL)
{
}


//------------------------------------------------------------------------------
// SPKPropagator & SPKPropagator::operator =(const SPKPropagator & spk)
//------------------------------------------------------------------------------
/**
 * Assignment operator
 *
 * @param spk The object that is provides data for into this one
 *
 * @return This propagator, configured to match spk.
 */
//------------------------------------------------------------------------------
SPKPropagator & SPKPropagator::operator =(const SPKPropagator & spk)
{
   if (this != &spk)
   {
      EphemerisPropagator::operator=(spk);

      skr = NULL;
      spkCentralBody = spk.spkCentralBody;
   }

   return *this;
}


//------------------------------------------------------------------------------
// GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Generates a new object that matches this one
 *
 * @return The new object
 */
//------------------------------------------------------------------------------
GmatBase* SPKPropagator::Clone() const
{
   return new SPKPropagator(*this);
}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieves the script string for a parameter
 *
 * @param id The index of the parameter in the parameter tables
 *
 * @return The string
 */
//------------------------------------------------------------------------------
wxString SPKPropagator::GetParameterText(const Integer id) const
{
   if (id >= EphemerisPropagatorParamCount && id < SPKPropagatorParamCount)
      return PARAMETER_TEXT[id - EphemerisPropagatorParamCount];
   return EphemerisPropagator::GetParameterText(id);
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * Retrieves the ID of a parameter
 *
 * @param The script string for the parameter
 *
 * @return The parameter's ID
 */
//------------------------------------------------------------------------------
Integer SPKPropagator::GetParameterID(const wxString &str) const
{
   for (Integer i = EphemerisPropagatorParamCount;
         i < SPKPropagatorParamCount; ++i)
   {
       if (str == PARAMETER_TEXT[i - EphemerisPropagatorParamCount])
           return i;
   }

   return EphemerisPropagator::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieves the type for a parameter
 *
 * @param id The ID of the parameter
 *
 * @return The parameter's type
 */
//------------------------------------------------------------------------------
Gmat::ParameterType SPKPropagator::GetParameterType(const Integer id) const
{
   if (id >= EphemerisPropagatorParamCount && id < SPKPropagatorParamCount)
      return PARAMETER_TYPE[id - EphemerisPropagatorParamCount];
   return EphemerisPropagator::GetParameterType(id);
}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieves a string description of a parameter's type
 *
 * @param id The ID of the parameter
 *
 * @return The type of the parameter
 */
//------------------------------------------------------------------------------
wxString SPKPropagator::GetParameterTypeString(const Integer id) const
{
   if (id >= EphemerisPropagatorParamCount && id < SPKPropagatorParamCount)
      return EphemerisPropagator::PARAM_TYPE_STRING[GetParameterType(id)];
   return EphemerisPropagator::GetParameterTypeString(id);
}


//------------------------------------------------------------------------------
// wxString GetParameterUnit(const Integer id) const
//------------------------------------------------------------------------------
/**
 * retrieves the dimensional units for a parameter
 *
 * @param id The ID of the parameter
 *
 * @return The unit label
 */
//------------------------------------------------------------------------------
wxString SPKPropagator::GetParameterUnit(const Integer id) const
{
   return EphemerisPropagator::GetParameterUnit(id);
}


//------------------------------------------------------------------------------
// bool IsParameterReadOnly(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Reports if a parameter should be hidden from the users
 *
 * @param id The ID of the parameter
 *
 * @return true if the parameter should be hidden, false if not
 */
//------------------------------------------------------------------------------
bool SPKPropagator::IsParameterReadOnly(const Integer id) const
{
   if (id == SPKFILENAMES)
      return true;
   return EphemerisPropagator::IsParameterReadOnly(id);
}


//------------------------------------------------------------------------------
// bool IsParameterReadOnly(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Reports if a parameter should be hidden from the users
 *
 * @param label The scripted string of the parameter
 *
 * @return true if the paameter should be hidden, false if not
 */
//------------------------------------------------------------------------------
bool SPKPropagator::IsParameterReadOnly(const wxString &label) const
{
   return IsParameterReadOnly(GetParameterID(label));
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieves a string parameter
 *
 * @param id The ID of the parameter
 *
 * @return The parameter's value
 */
//------------------------------------------------------------------------------
wxString SPKPropagator::GetStringParameter(const Integer id) const
{
   return EphemerisPropagator::GetStringParameter(id);
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
/**
 * Sets a string parameter
 *
 * @param id The ID of the parameter
 * @param value The new value
 *
 * @return true on success, false on failure
 */
//------------------------------------------------------------------------------
bool SPKPropagator::SetStringParameter(const Integer id,
      const wxString &value)
{

   if (id == SPKFILENAMES)
   {
      if (value != wxT(""))
         if (find(spkFileNames.begin(), spkFileNames.end(), value) ==
               spkFileNames.end())
            spkFileNames.push_back(value);
      return true;         // Idempotent, so return true
   }

   bool retval = EphemerisPropagator::SetStringParameter(id, value);

   if ((retval = true) && (id == EPHEM_CENTRAL_BODY))
   {
      // Special case code that we may want to remove later
      if (value == wxT("Moon"))
         throw PropagatorException(wxT("\"Moon\" is not an allowed central body; ")
               wxT("try \"Luna\""), Gmat::ERROR_);
      if (centralBody == wxT("Luna"))
         spkCentralBody = wxT("Moon");
      else
         spkCentralBody = centralBody;
   }

   return retval;
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieves a string parameter from an array
 *
 * @param id The ID of the parameter
 * @param index The array index
 *
 * @return The parameter's value
 */
//------------------------------------------------------------------------------
wxString SPKPropagator::GetStringParameter(const Integer id,
      const Integer index) const
{
   if (id == SPKFILENAMES)
   {
      if ((index >= 0) && (index < (Integer)spkFileNames.size()))
         return spkFileNames[index];
      return wxT("");
   }

   return EphemerisPropagator::GetStringParameter(id, index);
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value,
//       const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets a string parameter in an array
 *
 * @param id The ID of the parameter
 * @param value The new value
 * @param index The index of the parameter in the array
 *
 * @return True on success, false on failure
 */
//------------------------------------------------------------------------------
bool SPKPropagator::SetStringParameter(const Integer id,
      const wxString &value, const Integer index)
{
   if (id == SPKFILENAMES)
   {
      if ((index >= 0) && (index < (Integer)spkFileNames.size()))
      {
         spkFileNames[index] = value;
         return true;
      }
      return false;
   }

   return EphemerisPropagator::SetStringParameter(id, value, index);
}


//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieves a StringArray parameter
 *
 * @param id The ID of the parameter
 *
 * @return The StringArray
 */
//------------------------------------------------------------------------------
const StringArray& SPKPropagator::GetStringArrayParameter(
      const Integer id) const
{
   if (id == SPKFILENAMES)
      return spkFileNames;
   return EphemerisPropagator::GetStringArrayParameter(id);
}


//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const Integer id,
//       const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieves a StringArray parameter from an array of StringArrays
 *
 * @param id The ID of the parameter
 * @param index The index of the StringArray
 *
 * @return The StringArray
 */
//------------------------------------------------------------------------------
const StringArray& SPKPropagator::GetStringArrayParameter(const Integer id,
      const Integer index) const
{
   return EphemerisPropagator::GetStringArrayParameter(id, index);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Retrieves a StringArray parameter
 *
 * @param label The script label of the parameter
 *
 * @return The StringArray
 */
//------------------------------------------------------------------------------
wxString SPKPropagator::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
/**
 * Sets a string parameter
 *
 * @param label The script label of the parameter
 * @param value The new value
 *
 * @return true on success, false on failure
 */
//------------------------------------------------------------------------------
bool SPKPropagator::SetStringParameter(const wxString &label,
      const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label,
//       const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieves a string parameter from an array
 *
 * @param label The script label of the parameter
 * @param index The array index
 *
 * @return The parameter's value
 */
//------------------------------------------------------------------------------
wxString SPKPropagator::GetStringParameter(const wxString &label,
      const Integer index) const
{
   return GetStringParameter(GetParameterID(label), index);
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value,
//       const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets a string parameter in an array
 *
 * @param label The script label of the parameter
 * @param value The new value
 * @param index The index of the parameter in the array
 *
 * @return True on success, false on failure
 */
//------------------------------------------------------------------------------
bool SPKPropagator::SetStringParameter(const wxString &label,
      const wxString &value, const Integer index)
{
   return SetStringParameter(GetParameterID(label), value, index);
}


//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Retrieves a StringArray parameter
 *
 * @param label The script label of the parameter
 *
 * @return The StringArray
 */
//------------------------------------------------------------------------------
const StringArray& SPKPropagator::GetStringArrayParameter(
      const wxString &label) const
{
   return GetStringArrayParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const wxString &label,
//       const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieves a StringArray parameter from an array of StringArrays
 *
 * @param label The script label of the parameter
 * @param index The index of the StringArray
 *
 * @return The StringArray
 */
//------------------------------------------------------------------------------
const StringArray& SPKPropagator::GetStringArrayParameter(
      const wxString &label, const Integer index) const
{
   return GetStringArrayParameter(GetParameterID(label), index);
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 * Prepares the SPKPropagator for use in a run
 *
 * @return true on success, false on failure
 */
//------------------------------------------------------------------------------
bool SPKPropagator::Initialize()
{
   #ifdef DEBUG_INITIALIZATION
      MessageInterface::ShowMessage(wxT("SPKPropagator::Initialize() entered\n"));
      MessageInterface::ShowMessage(wxT("spkCentralBody is %s\n"), spkCentralBody.c_str());
   #endif

   bool retval = false;

   if (EphemerisPropagator::Initialize())
   {
      // If skr already set, just keep it
      if (skr == NULL)
         skr = new SpiceOrbitKernelReader;

      stepTaken = 0.0;
      j2ET = j2000_c();   // CSPICE method to return Julian date of J2000 (TDB)

      FileManager *fm = FileManager::Instance();
      wxString fullPath = fm->GetFullPathname(FileManager::PLANETARY_SPK_FILE);

      if (skr->IsLoaded(fullPath) == false)
         skr->LoadKernel(fullPath);

      if (propObjects.size() != 1)
         throw PropagatorException(wxT("SPICE propagators (i.e. \"SPK\" ")
               wxT("propagators) require exactly one SpaceObject."));
      #ifdef DEBUG_INITIALIZATION
         MessageInterface::ShowMessage(wxT("Clearing %d naifIds\n"), naifIds.size());
      #endif
      naifIds.clear();
      for (UnsignedInt i = 0; i < propObjects.size(); ++i)
      {
         Integer id = propObjects[i]->GetIntegerParameter(wxT("NAIFId"));
         naifIds.push_back(id);

         // Load the SPICE files for each propObject
         StringArray spices;
         if (propObjects[i]->IsOfType(Gmat::SPACECRAFT))
            spices = propObjects[i]->GetStringArrayParameter(
                  wxT("OrbitSpiceKernelName"));
         else
            throw PropagatorException(wxT("Spice (SPK) propagators only work for ")
                  wxT("Spacecraft right now."));

         if (spices.size() == 0)
            throw PropagatorException(wxT("Spice (SPK) propagator requires at ")
                  wxT("least one orbit SPICE kernel,"));

         wxString ephemPath = fm->GetPathname(FileManager::EPHEM_PATH);
         for (UnsignedInt j = 0; j < spices.size(); ++j)
         {
            fullPath = spices[j];

            // Check to see if this name includes path information
            // If no path designation slash character is found, add the default path
            if ((fullPath.find('/') == wxString::npos) &&
                (fullPath.find('\\') == wxString::npos))
            {
               fullPath = ephemPath + fullPath;
            }
            #ifdef DEBUG_INITIALIZATION
               MessageInterface::ShowMessage(wxT("Checking for kernel %s\n"),
                     fullPath.c_str());
            #endif
            if (skr->IsLoaded(fullPath) == false)
               skr->LoadKernel(fullPath);

            if (find(spkFileNames.begin(), spkFileNames.end(), fullPath) ==
                  spkFileNames.end())
               spkFileNames.push_back(fullPath);
         }

         // Load the initial data point
         if (skr)
         {
            try
            {
               Rvector6  outState;

               for (UnsignedInt i = 0; i < propObjects.size(); ++i)
               {
                  wxString scName = propObjectNames[i];
                  Integer id = naifIds[i];

                  currentEpoch = initialEpoch + timeFromEpoch /
                        GmatTimeConstants::SECS_PER_DAY;

                  // Allow for slop in the last few bits
                  if ((currentEpoch < ephemStart - 1e-10) ||
                      (currentEpoch > ephemEnd + 1e-10))
                  {
                     wxString errmsg;
                     errmsg << wxT("The SPKPropagator ")
                            << instanceName
                            << wxT(" is attempting to initialize outside of the ")
                               wxT("timespan  of the ephemeris data; halting.  ");
                     errmsg << wxT("The current SPICE ephemeris covers the A.1 ")
                               wxT("modified Julian span ");
                     errmsg << ephemStart << wxT(" to ") << ephemEnd << wxT(" and the ")
                              wxT("requested epoch is ") << currentEpoch << wxT(".\n");
                     throw PropagatorException(errmsg);
                  }
                  #ifdef DEBUG_INITIALIZATION
                     MessageInterface::ShowMessage(wxT("Getting target state in %p ")
                           wxT("for %s (ID = %ld) at epoch %lf and CB %s\n"), this,
                           scName.c_str(), id, currentEpoch,
                           spkCentralBody.c_str());
                  #endif
                  outState = skr->GetTargetState(scName, id, currentEpoch,
                        spkCentralBody);

                  std::memcpy(state, outState.GetDataVector(),
                        dimension*sizeof(Real));
               }

               UpdateSpaceObject(currentEpoch);

               retval = true;
            }
            catch (BaseException &e)
            {
               MessageInterface::ShowMessage(e.GetFullMessage());
               retval = false;
               throw;
            }
         }
      }

      SetEphemSpan();
   }

   #ifdef DEBUG_INITIALIZATION
      MessageInterface::ShowMessage(wxT("SPKPropagator::Initialize(): Start state ")
            wxT("at epoch %.12lf is ["), currentEpoch);
      for (Integer i = 0; i < dimension; ++i)
      {
         MessageInterface::ShowMessage(wxT("%.12lf"), state[i]);
         if (i < dimension-1)
            MessageInterface::ShowMessage(wxT("   "));
         else
            MessageInterface::ShowMessage(wxT("]\n"));
      }
      MessageInterface::ShowMessage(wxT("SPKPropagator::Initialize() finished\n"));
   #endif

   return retval;
}


//------------------------------------------------------------------------------
// bool Step()
//------------------------------------------------------------------------------
/**
 * Advances the state vector by the ephem step
 *
 * @return true on success, false on failure
 */
//------------------------------------------------------------------------------
bool SPKPropagator::Step()
{
   #ifdef DEBUG_PROPAGATION
      MessageInterface::ShowMessage(wxT("SPKPropagator::Step() entered: ")
            wxT("initialEpoch = %.12lf; stepsize = %.12lf; ")
            wxT("timeFromEpoch = %.12lf\n"), initialEpoch, ephemStep, timeFromEpoch);
   #endif

   bool retval = false;

   if (skr)
   {
      try
      {
         Rvector6  outState;

         for (UnsignedInt i = 0; i < propObjects.size(); ++i)
         {
            wxString scName = propObjectNames[i];
            Integer id = naifIds[i];

            timeFromEpoch += ephemStep;
            stepTaken = ephemStep;
            currentEpoch = initialEpoch + timeFromEpoch /
                  GmatTimeConstants::SECS_PER_DAY;

            // Allow for slop in the last few bits
            if ((currentEpoch < ephemStart - 1e-10) ||
                (currentEpoch > ephemEnd + 1e-10))
            {
               wxString errmsg;
               errmsg << wxT("The SPKPropagator ")
                      << instanceName
                      << wxT(" is attempting to step outside of the span of the ")
                         wxT("ephemeris data; halting.  ");
               errmsg << wxT("The current SPICE ephemeris covers the A.1 modified ")
                         wxT("Julian span ");
               errmsg << ephemStart << wxT(" to ") << ephemEnd << wxT(" and the ")
                     wxT("requested epoch is ") << currentEpoch << wxT(".\n");
               throw PropagatorException(errmsg);
            }

            outState = skr->GetTargetState(scName, id, currentEpoch,
                  spkCentralBody);

            /**
             *  @todo: When SPKProp can evolve more than one spacecraft, these
             *  memcpy lines need revision
             */
//            std::memcpy(state, outState.GetDataVector(),
//                  dimension*sizeof(Real));
//            ReturnFromOrigin(currentEpoch);
//            std::memcpy(j2kState, outState.GetDataVector(),
            std::memcpy(state, outState.GetDataVector(),
                  dimension*sizeof(Real));
            //MoveToOrigin(currentEpoch);
            UpdateSpaceObject(currentEpoch);

            #ifdef DEBUG_PROPAGATION
               MessageInterface::ShowMessage(wxT("(Step for %p) State at epoch ")
                     wxT("%.12lf is ["), this, currentEpoch);
               for (Integer i = 0; i < dimension; ++i)
               {
                  MessageInterface::ShowMessage(wxT("%.12lf"), state[i]);
                  if (i < 5)
                     MessageInterface::ShowMessage(wxT("   "));
                  else
                     MessageInterface::ShowMessage(wxT("]\n"));
               }
            #endif
         }

         retval = true;
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(e.GetFullMessage());
         retval = false;
         throw;
      }
   }

   #ifdef DEBUG_PROPAGATION
      else
         MessageInterface::ShowMessage(wxT("skr was not initialized]\n"));
   #endif

   return retval;
}


//------------------------------------------------------------------------------
// bool RawStep()
//------------------------------------------------------------------------------
/**
 * Performs a propagation step without error control
 *
 * @note: RawStep is not used with the SPKPropagator
 *
 * @return false always
 */
//------------------------------------------------------------------------------
bool SPKPropagator::RawStep()
{
   bool retval = false;
   return retval;
}


//------------------------------------------------------------------------------
// Real GetStepTaken()
//------------------------------------------------------------------------------
/**
 * Retrieves the size of the most recent SPKPropagator step
 *
 * @return The most recent step (0.0 if no step was taken with this instance).
 */
//------------------------------------------------------------------------------
Real SPKPropagator::GetStepTaken()
{
   return stepTaken;
}


//------------------------------------------------------------------------------
// void UpdateState()
//------------------------------------------------------------------------------
/**
 * Updates the propagation state vector with data from the
 * PropagationStateManager
 */
//------------------------------------------------------------------------------
void SPKPropagator::UpdateState()
{
   #ifdef DEBUG_EXECUTION
      MessageInterface::ShowMessage(wxT("Updating state to epoch %.12lf\n"),
            currentEpoch);
   #endif

   if (skr)
   {
      try
      {
         Rvector6  outState;

         for (UnsignedInt i = 0; i < propObjects.size(); ++i)
         {
            wxString scName = propObjectNames[i];
            Integer id = naifIds[i];

            // Allow for slop in the last few bits
            if ((currentEpoch < ephemStart - 1e-10) ||
                (currentEpoch > ephemEnd + 1e-10))
            {
               wxString errmsg;
               errmsg << wxT("The SPKPropagator ")
                      << instanceName
                      << wxT(" is attempting to access state data outside of the ")
                         wxT("span of the ephemeris data; halting.  ");
               errmsg << wxT("The current SPICE ephemeris covers the A.1 modified ")
                         wxT("Julian span ")
                      << ephemStart << wxT(" to ") << ephemEnd << wxT(" and the ")
                         wxT("requested epoch is ") << currentEpoch << wxT(".\n");
               throw PropagatorException(errmsg);
            }

            outState = skr->GetTargetState(scName, id, currentEpoch,
                  spkCentralBody);

            /**
             *  @todo: When SPKProp can evolve more than one spacecraft, this
             *  memcpy line needs revision
             */
//            std::memcpy(state, outState.GetDataVector(),
//                  dimension*sizeof(Real));
//            std::memcpy(j2kState, outState.GetDataVector(),
            std::memcpy(state, outState.GetDataVector(),
                  dimension*sizeof(Real));
//            MoveToOrigin(currentEpoch);
//            UpdateSpaceObject(currentEpoch);

            #ifdef DEBUG_PROPAGATION
               MessageInterface::ShowMessage(wxT("(UpdateState for %p) State at ")
                     wxT("epoch %.12lf is ["), this, currentEpoch);
               for (Integer i = 0; i < dimension; ++i)
               {
                  MessageInterface::ShowMessage(wxT("%.12lf"), state[i]);
                  if (i < 5)
                     MessageInterface::ShowMessage(wxT("   "));
                  else
                     MessageInterface::ShowMessage(wxT("]\n"));
               }
            #endif
         }
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(e.GetFullMessage());
         throw;
      }
   }
}


//------------------------------------------------------------------------------
// void SetEphemSpan(Integer whichOne)
//------------------------------------------------------------------------------
/**
 * Determines the start and end epoch for the SPICE ephemerides associated with
 * the propagated spacecraft
 *
 * @param whichOne Not currrently used.
 */
//------------------------------------------------------------------------------
void SPKPropagator::SetEphemSpan(Integer whichOne)
{
   if (whichOne < 0)
      throw PropagatorException(wxT("SPKPropagator::SetEphemSpan(Integer whichOne):")
            wxT(" Invalid index"));

   if (skr)
   {
      // @todo: When the SPKPropagator supports more than one spacecraft, the
      // ephem span needs to be modified to track spans for each spacecraft
      for (UnsignedInt i = 0; i < naifIds.size(); ++i)
         skr->GetCoverageStartAndEnd(spkFileNames, naifIds[i], ephemStart,
               ephemEnd);

      #ifdef DEBUG_INITIALIZATION
         MessageInterface::ShowMessage(wxT("EphemSpan is [%.12lf %.12lf]\n"),
               ephemStart, ephemEnd);
      #endif
   }
}
