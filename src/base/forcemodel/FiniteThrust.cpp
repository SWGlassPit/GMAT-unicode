//$Id: FiniteThrust.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              FiniteThrust
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under MOMS Task
// Order 124.
//
// Author: Darrel J. Conway, Thinking Systems, Inc.
// Created: 2004/12/20
//
/**
 * Implements the FiniteThrust class used to model the acceleration during a 
 * finite burn. 
 */
//------------------------------------------------------------------------------


#include "FiniteThrust.hpp"

//#define DEBUG_FINITETHRUST_INIT
//#define DEBUG_REGISTRATION
//#define DEBUG_FINITETHRUST_EXE
//#define DEBUG_MASS_FLOW


#include "MessageInterface.hpp"
#include "TimeTypes.hpp"
#include <sstream>               // for stringstream


//------------------------------------------------------------------------------
// FiniteThrust(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Constructor for forces from finite burns.
 * 
 * @param name Name of the constructed instance.
 */
//------------------------------------------------------------------------------
FiniteThrust::FiniteThrust(const wxString &name) :
   PhysicalModel        (Gmat::PHYSICAL_MODEL, wxT("FiniteThrust"), name),
   satCount             (0),
   cartIndex            (-1),
   fillCartesian        (false),
   satThrustCount       (0),
   mDotIndex            (-1),
   depleteMass          (true)
{
   derivativeIds.push_back(Gmat::CARTESIAN_STATE);
}


//------------------------------------------------------------------------------
// ~FiniteThrust()
//------------------------------------------------------------------------------
/**
 * Destructor for forces from finite burns.
 */
//------------------------------------------------------------------------------
FiniteThrust::~FiniteThrust()
{
}


//------------------------------------------------------------------------------
// FiniteThrust(const FiniteThrust& ft)
//------------------------------------------------------------------------------
/**
 * Copy constructor for forces from finite burns.
 * 
 * @param ft The oroginal that gets copied.
 */
//------------------------------------------------------------------------------
FiniteThrust::FiniteThrust(const FiniteThrust& ft) :
   PhysicalModel           (ft),
   burnNames               (ft.burnNames),
   mySpacecraft            (ft.mySpacecraft),
   spacecraft              (ft.spacecraft),
   satCount                (ft.satCount),
   cartIndex               (ft.cartIndex),
   fillCartesian           (ft.fillCartesian),
   satThrustCount          (ft.satThrustCount),
   mDotIndex               (ft.mDotIndex),
   depleteMass             (ft.depleteMass)
{
   burns.clear();
   scIndices.clear();
}


//------------------------------------------------------------------------------
// FiniteThrust& operator=(const FiniteThrust& ft)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 * 
 * @param ft Instance that provides the parameter values.
 * 
 * @return This instance, set to match the input instance.
 */
//------------------------------------------------------------------------------
FiniteThrust& FiniteThrust::operator=(const FiniteThrust& ft)
{
   if (this != &ft)
   {
      PhysicalModel::operator=(ft);

      burnNames    = ft.burnNames;
      spacecraft   = ft.spacecraft;
      mySpacecraft = ft.mySpacecraft;
      burns.clear();
      scIndices.clear();

      satCount       = ft.satCount;
      cartIndex      = ft.cartIndex;
      fillCartesian  = ft.fillCartesian;
      satThrustCount = ft.satThrustCount;
      mDotIndex      = ft.mDotIndex;
      depleteMass    = ft.depleteMass;
   }
   return *this;
}


//------------------------------------------------------------------------------
// GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * AOverridden Clone method used to make copies from a GmatBase pointer.
 * 
 * @return New instance, set to look like this one.
 */
//------------------------------------------------------------------------------
GmatBase* FiniteThrust::Clone() const
{
   return (new FiniteThrust(*this));
}


//------------------------------------------------------------------------------
// void Clear(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Clears the arrays of elements that get set by the Propagate commands
 * 
 * @param type The type of element that gets cleared.  Set to 
 *             Gmat::UNKNOWN_OBJECT to clear all of the configrable arrays.
 */
//------------------------------------------------------------------------------
void FiniteThrust::Clear(const Gmat::ObjectType type)
{
   if ((type == Gmat::UNKNOWN_OBJECT) || (type == Gmat::SPACECRAFT)) 
   {
      mySpacecraft.clear();
      spacecraft.clear();
   }
   
   if ((type == Gmat::UNKNOWN_OBJECT) || (type == Gmat::THRUSTER)) 
   {
      burnNames.clear();
   }
}


//------------------------------------------------------------------------------
// bool SetRefObjectName(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Sets the names for referenced objects.
 * 
 * FiniteThrust instances use Spacecraft and FiniteThrust objects.  This method
 * sets the names for those objects.
 * 
 * @param type The type of object that has the name.
 * @param name The object's name
 * 
 * @return true on success, false otherwise.
 */
//------------------------------------------------------------------------------
bool FiniteThrust::SetRefObjectName(const Gmat::ObjectType type,
                                  const wxString &name)
{
   if (type == Gmat::SPACECRAFT) 
   {
      if (find(mySpacecraft.begin(), mySpacecraft.end(), name) == mySpacecraft.end())
         mySpacecraft.push_back(name);
      return true;
   }
   
   if (type == Gmat::FINITE_BURN) 
   {
      if (find(burnNames.begin(), burnNames.end(), name) == burnNames.end())
         burnNames.push_back(name);
      return true;
   }
   
   return PhysicalModel::SetRefObjectName(type, name);
}


//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Accesses the names for referenced objects.
 * 
 * @param type The type of object requested.
 * 
 * @return The vector of names.
 */
//------------------------------------------------------------------------------
const StringArray& FiniteThrust::GetRefObjectNameArray(
                                                   const Gmat::ObjectType type)
{
   if (type == Gmat::SPACECRAFT)
      return mySpacecraft;
      
   return PhysicalModel::GetRefObjectNameArray(type);
}


//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type, 
//                   const wxString &name)
//------------------------------------------------------------------------------
/**
 * Sets referenced object pointers.
 * 
 * @param obj The object.
 * @param type The type of the object.
 * @param name The object's name.
 * 
 * @return true if the object is set, false if not.
 */
//------------------------------------------------------------------------------
bool FiniteThrust::SetRefObject(GmatBase *obj, const Gmat::ObjectType type, 
                                const wxString &name)
{
   if (type == Gmat::FINITE_BURN) 
   {
      if (obj->GetTypeName() != wxT("FiniteBurn"))
         throw ODEModelException(
            wxT("FiniteThrust::SetRefObject cannot use objects of type ") + 
            obj->GetTypeName());
      if (find(burns.begin(), burns.end(), obj) == burns.end())
         burns.push_back((FiniteBurn*)obj);
      return true;
   }
   
   return PhysicalModel::SetRefObject(obj, type, name);
}

//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//       const wxString &name, const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets referenced object pointers in an object array.
 *
 * @param obj The object.
 * @param type The type of the object.
 * @param name The object's name.
 * @param index The index of the object in the array
 *
 * @return true if the object is set, false if not.
 */
//------------------------------------------------------------------------------
bool FiniteThrust::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
      const wxString &name, const Integer index)
{
   return PhysicalModel::SetRefObject(obj, type, name, index);
}


//------------------------------------------------------------------------------
// bool RenameRefObject(const Gmat::ObjectType type, const wxString &oldName,
//       const wxString &newName)
//------------------------------------------------------------------------------
/**
 * Changes the name for reference objects
 *
 * @param type The type of the object.
 * @param oldName The object's name before the change.
 * @param newName The object's proposed name after the change.
 *
 * @return true if a change was made, false if not
 */
//------------------------------------------------------------------------------
bool FiniteThrust::RenameRefObject(const Gmat::ObjectType type,
      const wxString &oldName, const wxString &newName)
{
   if (type == Gmat::FINITE_BURN)
   {
      for (UnsignedInt i = 0; i < burnNames.size(); ++i)
      {
         if (burnNames[i] == oldName)
            burnNames[i] = newName;
      }
      for (UnsignedInt i = 0; i < burns.size(); ++i)
      {
         if (burns[i] != NULL)
            if (burns[i]->GetName() == oldName)
               burns[i]->SetName(newName);
      }
      return true;
   }

   return PhysicalModel::RenameRefObject(type, oldName, newName);
}


//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a pointer to a reference object
 *
 * @param type The type of the object
 * @param name The object's name
 *
 * @return The object pointer
 */
//------------------------------------------------------------------------------
GmatBase* FiniteThrust::GetRefObject(const Gmat::ObjectType type,
      const wxString &name)
{
   if (type == Gmat::FINITE_BURN)
   {
      if ((name == wxT("")) && (burns.size() > 0))
         return burns[0];

      for (UnsignedInt i = 0; i < burns.size(); ++i)
      {
         if (burns[i] != NULL)
            if (burns[i]->GetName() == name)
               return burns[i];
      }

      return NULL;
   }

   if (type == Gmat::SPACECRAFT)
   {
      MessageInterface::ShowMessage(wxT("Returning spacecrft\n"));

      if (name == wxT(""))
         if (spacecraft.size() > 0)
            return spacecraft[0];
      for (UnsignedInt i = 0; i < spacecraft.size(); ++i)
         if (spacecraft[i]->GetName() == name)
            return spacecraft[i];
      return NULL;
   }

   return PhysicalModel::GetRefObject(type, name);
}


//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name,
//       const Integer index)
//------------------------------------------------------------------------------
/**
 * Retrieves a pointer to a reference object from an array
 *
 * @param type The type of the object
 * @param name The object's name
 * @param index The index of the object in the array
 *
 * @return The object pointer
 */
//------------------------------------------------------------------------------
GmatBase* FiniteThrust::GetRefObject(const Gmat::ObjectType type,
      const wxString &name, const Integer index)
{
   return PhysicalModel::GetRefObject(type, name, index);
}


//------------------------------------------------------------------------------
// bool IsTransient()
//------------------------------------------------------------------------------
/**
 * Specifies whether the force is transient.
 * 
 * @return true for all FiniteThrust forces.
 */
//------------------------------------------------------------------------------
bool FiniteThrust::IsTransient()
{
   return true;
}

//------------------------------------------------------------------------------
// bool DepletesMass()
//------------------------------------------------------------------------------
/**
 * Detects mass depletion from a PhysicalModel
 *
 * @return true if the model depletes mass, false if it does not
 */
//------------------------------------------------------------------------------
bool FiniteThrust::DepletesMass()
{
   return depleteMass;
}


//------------------------------------------------------------------------------
// bool IsTransient()
//------------------------------------------------------------------------------
/**
 * Sets the list of propagated space objects for transient forces.
 */
//------------------------------------------------------------------------------
void FiniteThrust::SetPropList(ObjectArray *soList)
{
   spacecraft.clear();
   for (ObjectArray::iterator i = soList->begin(); i != soList->end(); ++i)
      spacecraft.push_back(*i);
   
   #ifdef DEBUG_FINITETHRUST_INIT
      MessageInterface::ShowMessage(wxT("Spacecraft list contains %d objects\n"),
            spacecraft.size());
   #endif
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 * Set up data structures to manage finite burns
 * 
 * @return              true if the call succeeds, false on failure.
 */
//------------------------------------------------------------------------------
bool FiniteThrust::Initialize()
{
   #ifdef DEBUG_FINITETHRUST_INIT
      MessageInterface::ShowMessage(wxT("FiniteThrust::Initialize entered\n"));
   #endif
      
   if (!PhysicalModel::Initialize())
      throw ODEModelException(wxT("Unable to initialize FiniteThrust base"));
      
   // set up the indices into the state vector that match spacecraft with active 
   // thrusters
   Integer satIndex, stateIndex;
   #ifdef DEBUG_FINITETHRUST_INIT
      MessageInterface::ShowMessage(
            wxT("   Looping over satNames\n"));
   #endif
   for (StringArray::iterator satName = mySpacecraft.begin(); 
        satName != mySpacecraft.end(); ++satName)
   {
      #ifdef DEBUG_FINITETHRUST_INIT
         MessageInterface::ShowMessage(
               wxT("      satName = %s\n"), satName->c_str());
      #endif
      satIndex = 0;
      stateIndex = 0;
      
      for (ObjectArray::iterator propSat = spacecraft.begin();
           propSat != spacecraft.end(); ++propSat) 
      {
         #ifdef DEBUG_FINITETHRUST_INIT
            MessageInterface::ShowMessage(
                  wxT("      Looking at satIndex %d; stateIndex = %d\n"), satIndex, 
                  stateIndex);
         #endif
         if ((*propSat)->GetName() == *satName) 
         {
            #ifdef DEBUG_FINITETHRUST_INIT
               MessageInterface::ShowMessage(
                  wxT("      FiniteThrust::Initialize Matched up %s\n"), 
                  satName->c_str());
            #endif
         }
         #ifdef DEBUG_FINITETHRUST_INIT
            MessageInterface::ShowMessage(
                  wxT("      PropSat is \"%s\"\n"), (*satName).c_str());
         #endif
         if ((*propSat)->IsOfType(Gmat::SPACEOBJECT))
         {
            stateIndex += ((SpaceObject*)(*propSat))->GetState().GetSize();
            ++satIndex;
         }
      }
   }
   
   #ifdef DEBUG_FINITETHRUST_INIT
      MessageInterface::ShowMessage(
            wxT("FiniteThrust::Initialize finished\n"));
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// bool GetDerivatives(Real * state, Real dt, Integer order)
//------------------------------------------------------------------------------
/**
 * Method invoked to calculate derivatives
 * 
 * This method is invoked to fill the deriv array with derivative information 
 * for the system that is being integrated.  It uses the state and elapsedTime 
 * parameters, along with the time interval dt passed in as a parameter, to 
 * calculate the derivative information at time \f$t=t_0+t_{elapsed}+dt\f$.
 *
 * @param dt            Additional time increment for the derivatitive 
 *                      calculation; defaults to 0.
 * @param state         Pointer to the current state data.  This can differ
 *                      from the PhysicalModel state if the subscribing
 *                      integrator samples other state values during 
 *                      propagation.  (For example, the Runge-Kutta integrators 
 *                      do this during the stage calculations.)
 * @param order         The order of the derivative to be taken (first 
 *                      derivative, second derivative, etc)
 *
 * @return              true if the call succeeds, false on failure.
 */
//------------------------------------------------------------------------------
bool FiniteThrust::GetDerivatives(Real * state, Real dt, Integer order, 
      const Integer id)
{
   #ifdef DEBUG_FINITETHRUST_EXE
      MessageInterface::ShowMessage(
            wxT("FiniteThrust::GetDerivatives entered; fillCartesian is %s, ")
            wxT("maneuvering %d spacecraft\n"), (fillCartesian ? wxT("true") : wxT("false")),
            spacecraft.size());
   #endif

   Real accel[3], mDot, burnData[4];
   Integer i6, mloc = -1;
   Integer i = 0, j = 0;
   SpaceObject *sat;
   
   if (fillCartesian)
   {
      // Loop through the spacecraft list, building accels for active sats
      for (ObjectArray::iterator sc = spacecraft.begin();
           sc != spacecraft.end(); ++sc) 
      {
         i6 = cartIndex + i * 6;
         
         if (1)  // <-- NEED A TEST HERE!
         {
            if (j < satThrustCount)
            {
               mloc = mDotIndex + j;
               ++j;
            }
         }
         else
            mloc = -1;

         if ((*sc)->IsOfType(Gmat::SPACEOBJECT) == false)
            continue;
         
         // Just a convenience
         sat = (SpaceObject*)(*sc);

         #ifdef DEBUG_FINITETHRUST_EXE
            MessageInterface::ShowMessage(wxT("   Checking spacecraft %s\n"), 
               sat->GetName().c_str());
         #endif

         if (find(mySpacecraft.begin(), mySpacecraft.end(), sat->GetName()) != 
             mySpacecraft.end()) 
         {

            #ifdef DEBUG_FINITETHRUST_EXE
               MessageInterface::ShowMessage(wxT("   Maneuvering\n"));
            #endif
   
            if (sat->GetType() != Gmat::SPACECRAFT)
               throw ODEModelException(wxT("FiniteThrust::GetDerivatives Finite ")
               wxT("burns cannot maneuver ") + sat->GetTypeName() + wxT(" objects"));
            // Start with zero thrust
            mDot = accel[0] = accel[1] = accel[2] = 0.0;
   
            // Accumulate thrust and mass flow for each active thruster
            for (std::vector <FiniteBurn*>::iterator fb = burns.begin();
                 fb != burns.end(); ++fb)
            {
               (*fb)->SetSpacecraftToManeuver((Spacecraft*)sat);
               Real now = epoch + dt / GmatTimeConstants::SECS_PER_DAY;
               if ((*fb)->Fire(burnData, now)) 
               {
                  #ifdef DEBUG_FINITETHRUST_EXE
                     MessageInterface::ShowMessage(wxT("%s "),(*fb)->GetName().c_str());
                  #endif
                  accel[0] += burnData[0];
                  accel[1] += burnData[1];
                  accel[2] += burnData[2];
                  #ifdef DEBUG_MASS_FLOW
                     MessageInterface::ShowMessage(
                           wxT("   --> direction = [%.12le %.12le %.12le]"),
                           accel[0], accel[1], accel[2]);
                  #endif
                  if ((*fb)->DepletesMass())
                  {
                     if (order != 1)
                        throw ODEModelException(wxT("Mass depletion cannot be ")
                              wxT("performed with the selected propagator."));
                     mDot += burnData[3];
                     #ifdef DEBUG_MASS_FLOW
                        MessageInterface::ShowMessage(wxT("  mDot =  %.12lf\n"),
                              mDot);
                     #endif
                  }
                  #ifdef DEBUG_MASS_FLOW
                  else
                     MessageInterface::ShowMessage(wxT("  mDot =  0.0\n"));
                  #endif

               }
               #ifdef DEBUG_FINITETHRUST_EXE
                  MessageInterface::ShowMessage(
                     wxT("\n   Acceleration = [%18le  %18le  %18le]; ")
                     wxT("dm/dt = %.18le"), accel[0], accel[1], accel[2], mDot);
               #endif
            }
      
            #ifdef DEBUG_FINITETHRUST_EXE
               MessageInterface::ShowMessage(wxT("\n"));
            #endif
            
            // Apply the burns to the state vector
            if (order == 1) 
            {
               // dr/dt = v
               deriv[i6]     = 
               deriv[1 + i6] = 
               deriv[2 + i6] = 0.0;
               deriv[3 + i6] = accel[0];
               deriv[4 + i6] = accel[1];
               deriv[5 + i6] = accel[2];

               if (mloc >= 0)
               {
                  deriv[mloc+i]   = mDot;  // Is this right???
               }
            } 
            else  
            {
               deriv[ i6 ] = accel[0]; 
               deriv[i6+1] = accel[1]; 
               deriv[i6+2] = accel[2]; 
               deriv[i6+3] = 0.0; 
               deriv[i6+4] = 0.0; 
               deriv[i6+5] = 0.0; 
            }
         }
         else
         {
            // Zero any burn contribution that is not made for this spacecraft
            if (order == 1) 
            {
               // dr/dt = v
                  deriv[i6]     = 
                  deriv[1 + i6] = 
                  deriv[2 + i6] = 
                  deriv[3 + i6] = 
                  deriv[4 + i6] = 
               deriv[5 + i6] = 0.0;
            } 
            else  
            {
               deriv[ i6 ] = 0.0; 
               deriv[i6+1] = 0.0; 
               deriv[i6+2] = 0.0; 
               deriv[i6+3] = 0.0; 
               deriv[i6+4] = 0.0; 
               deriv[i6+5] = 0.0; 
            }
         }
         ++i;
      }
   }
   
   #ifdef DEBUG_FINITETHRUST_EXE
      //ShowDerivative(wxT("FiniteThrust::GetDerivatives() AFTER compute"), state,
      //               satCount);
      MessageInterface::ShowMessage
         (wxT("     deriv[1:3] = [%18le %18le %18le]\n     deriv[4:6] = [%18le %18le %18le]\n"),
          deriv[0], deriv[1], deriv[2], deriv[3], deriv[4], deriv[5]);
      MessageInterface::ShowMessage(wxT("FiniteThrust::GetDerivatives finished\n"));
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// bool SupportsDerivative(Gmat::StateElementId id)
//------------------------------------------------------------------------------
/**
 * Function used to determine if the physical model supports derivative 
 * information for a specified type.
 * 
 * @param id State Element ID for the derivative type
 * 
 * @return true if the type is supported, false otherwise. 
 */
//------------------------------------------------------------------------------
bool FiniteThrust::SupportsDerivative(Gmat::StateElementId id)
{
   #ifdef DEBUG_REGISTRATION
      MessageInterface::ShowMessage(
            wxT("FiniteThrust checking for support for id %d\n"), id);
   #endif
   
   if (id == Gmat::CARTESIAN_STATE)
      return true;
   
//   if (id == Gmat::ORBIT_STATE_TRANSITION_MATRIX)
//      return true;
   
   if (id == Gmat::MASS_FLOW)
   {
      #ifdef DEBUG_REGISTRATION
         MessageInterface::ShowMessage(wxT("I support mass flow!\n"));
      #endif
      return true;
   }
   
   return PhysicalModel::SupportsDerivative(id);
}


//------------------------------------------------------------------------------
// bool SetStart(Gmat::StateElementId id, Integer index, Integer quantity)
//------------------------------------------------------------------------------
/**
 * Function used to set the start point and size information for the state 
 * vector, so that the derivative information can be placed in the correct place 
 * in the derivative vector.
 * 
 * @param id State Element ID for the derivative type
 * @param index Starting index in the state vector for this type of derivative
 * @param quantity Number of objects that supply this type of data
 * 
 * @return true if the type is supported, false otherwise. 
 */
//------------------------------------------------------------------------------
bool FiniteThrust::SetStart(Gmat::StateElementId id, Integer index, 
                      Integer quantity)
{
   #ifdef DEBUG_REGISTRATION
      MessageInterface::ShowMessage(wxT("FiniteThrust setting start data for id = ")
            wxT("%d to index %d; %d objects identified\n"), id, index, quantity);
   #endif
   
   bool retval = false;
   
   switch (id)
   {
      case Gmat::CARTESIAN_STATE:
         satCount = quantity;
         cartIndex = index;
         fillCartesian = true;
         retval = true;
         break;
         
//      case Gmat::ORBIT_STATE_TRANSITION_MATRIX:
//         stmCount = quantity;
//         stmIndex = index;
//         fillSTM = true;
//         retval = true;
//         break;
         
      case Gmat::MASS_FLOW:
         // todo: add mass flow bits here
         satThrustCount = quantity;
         mDotIndex = index;
         depleteMass = true;
         retval = true;
         break;
         
      default:
         break;
   }
   
   return retval;
}
