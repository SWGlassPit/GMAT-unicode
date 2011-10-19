//$Id: FiniteBurn.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              FiniteBurn
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
 * Implements the FiniteBurn class used for maneuvers. 
 */
//------------------------------------------------------------------------------


#include "FiniteBurn.hpp"
#include "BurnException.hpp"
#include "StringUtil.hpp"          // for ToString()
#include "MessageInterface.hpp"

//#define DEBUG_RENAME
//#define DEBUG_BURN_ORIGIN
//#define DEBUG_FINITE_BURN
//#define DEBUG_FINITEBURN_FIRE
//#define DEBUG_FINITEBURN_SET
//#define DEBUG_FINITEBURN_INIT
//#define DEBUG_FINITEBURN_OBJECT

//---------------------------------
// static data
//---------------------------------

/// Labels used for the finite burn parameters.
const wxString
FiniteBurn::PARAMETER_TEXT[FiniteBurnParamCount - BurnParamCount] =
{
   wxT("Thrusters"),
   wxT("Tanks"),
   wxT("BurnScaleFactor")
};

/// Types of the parameters used by finite burns.
const Gmat::ParameterType
FiniteBurn::PARAMETER_TYPE[FiniteBurnParamCount - BurnParamCount] =
{
   Gmat::OBJECTARRAY_TYPE,
   Gmat::OBJECTARRAY_TYPE,
   Gmat::REAL_TYPE,
};


//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
//  FiniteBurn(const wxString &nomme)
//------------------------------------------------------------------------------
/**
 * FiniteBurn constructor (default constructor).
 * 
 * @param <nomme> Name of the constructed object.
 */
//------------------------------------------------------------------------------
FiniteBurn::FiniteBurn(const wxString &nomme) :
   Burn (Gmat::FINITE_BURN, wxT("FiniteBurn"), nomme)
{
   objectTypes.push_back(Gmat::FINITE_BURN);
   objectTypeNames.push_back(wxT("FiniteBurn"));
   parameterCount = FiniteBurnParamCount;
   
   thrusterNames.clear();
}


//------------------------------------------------------------------------------
//  ~FiniteBurn()
//------------------------------------------------------------------------------
/**
 * FiniteBurn destructor.
 */
//------------------------------------------------------------------------------
FiniteBurn::~FiniteBurn()
{
}


//------------------------------------------------------------------------------
//  FiniteBurn(const FiniteBurn& fb)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 * 
 * @param <fb> The FiniteBurn that is copied.
 */
//------------------------------------------------------------------------------
FiniteBurn::FiniteBurn(const FiniteBurn& fb) :
   Burn              (fb),
   thrusterNames     (fb.thrusterNames)
{
   parameterCount = fb.parameterCount;
}


//------------------------------------------------------------------------------
//  FiniteBurn& operator=(const FiniteBurn& fb)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 * 
 * @param <fb> The FiniteBurn that is copied.
 * 
 * @return this instance, with parameters set to match fb.
 */
//------------------------------------------------------------------------------
FiniteBurn& FiniteBurn::operator=(const FiniteBurn& fb)
{
   if (&fb == this)
      return *this;
      
   Burn::operator=(fb);
   
   thrusterNames       = fb.thrusterNames;
   
   return *this;
}


//------------------------------------------------------------------------------
//  void SetSpacecraftToManeuver(Spacecraft *sat)
//------------------------------------------------------------------------------
/**
 * Accessor method used by Maneuver to pass in the spacecraft pointer
 * 
 * @param <sat> the Spacecraft
 */
//------------------------------------------------------------------------------
void FiniteBurn::SetSpacecraftToManeuver(Spacecraft *sat)
{
   #ifdef DEBUG_FINITEBURN_SET
   MessageInterface::ShowMessage
      (wxT("FiniteBurn::SetSpacecraftToManeuver() sat=<%p>'%s'\n"), sat,
       sat->GetName().c_str());
   Real *state = (sat->GetState()).GetState();
   MessageInterface::ShowMessage
      (wxT("   state = [%.13f %.13f %.13f %.13f %.13f %.13f]\n"), state[0],
       state[1], state[2], state[3], state[4], state[5]);
   #endif
   
   if (sat == NULL)
      return;
   
   // FiniteBurn does not require CoordinateSystem conversion
   // so we don't need Burn::SetSpacecraftToManeuver(sat);
   // The thruster will handle CoordinateSystem conversion
   
   // If spacecraft changed, re-associate tank of the spacecraft
   if (spacecraft != sat)
   {
      spacecraft = sat;
      SetThrustersFromSpacecraft();
   }
   
   #ifdef DEBUG_FINITEBURN_SET
   MessageInterface::ShowMessage
      (wxT("FiniteBurn::SetSpacecraftToManeuver() returning\n"));
   #endif
}


//------------------------------------------------------------------------------
//  bool Fire(Real *burnData, Real epoch)
//------------------------------------------------------------------------------
/**
 * Fire does not currently perform any action for FiniteBurn objects.  The 
 * BeginManeuver/EndManeuver commands replace the actions that fire would 
 * perform.
 * 
 * @param <burnData> Pointer to an array that will be filled with the
 *                   acceleration and mass flow data.  The data returned in 
 *                   burnData has the format
 *                     burnData[0]  dVx/dt
 *                     burnData[1]  dVy/dt
 *                     burnData[2]  dVz/dt
 *                     burnData[3]  dM/dt
 * @param <epoch>    Epoch of the burn fire
 *
 * @return true on success; throws on failure.
 */
//------------------------------------------------------------------------------
bool FiniteBurn::Fire(Real *burnData, Real epoch)
{
   #ifdef DEBUG_FINITEBURN_FIRE
      MessageInterface::ShowMessage
         (wxT("FiniteBurn::Fire() this<%p>'%s' entered, epoch=%f, spacecraft=<%p>'%s'\n"),
          this, instanceName.c_str(), epoch, spacecraft,
          spacecraft ? spacecraft->GetName().c_str() : wxT("NULL"));
   #endif
   
   if (initialized == false)
      Initialize();
   
   if (!spacecraft)
      throw BurnException(wxT("Maneuver initial state undefined (No spacecraft?)"));
   
   // Accumulate the individual accelerations from the thrusters
   Real dm = 0.0, tMass, tOverM, *dir, norm;
   deltaV[0] = deltaV[1] = deltaV[2] = 0.0;
   Thruster *current;
   
   tMass = spacecraft->GetRealParameter(wxT("TotalMass"));
   
   #ifdef DEBUG_BURN_ORIGIN
   Real *satState = spacecraft->GetState().GetState();
   MessageInterface::ShowMessage
      (wxT("FiniteBurn Vectors:\n   ")
       wxT("Sat   = [%.15f %.15f %.15f %.15f %.15f %.15f]\n   ")
       wxT("Frame = [%.15f %.15f %.15f\n   ") 
       wxT("         %.15f %.15f %.15f\n   ")
       wxT("         %.15f %.15f %.15f]\n\n"),
       satState[0], satState[1], satState[2], satState[3], satState[4], satState[5], 
       frameBasis[0][0], frameBasis[0][1], frameBasis[0][2],
       frameBasis[1][0], frameBasis[1][1], frameBasis[1][2],
       frameBasis[2][0], frameBasis[2][1], frameBasis[2][2]);
   #endif
   
   for (StringArray::iterator i = thrusterNames.begin(); 
        i != thrusterNames.end(); ++i)
   {
      #ifdef DEBUG_FINITE_BURN
         MessageInterface::ShowMessage
            (wxT("   Accessing thruster '%s' from spacecraft <%p>'%s'\n"), (*i).c_str(),
             spacecraft, spacecraft->GetName().c_str());
      #endif
      
      current = (Thruster *)spacecraft->GetRefObject(Gmat::THRUSTER, *i);
      if (!current)
         throw BurnException(wxT("FiniteBurn::Fire requires thruster named \"") +
            (*i) + wxT("\" on spacecraft ") + spacecraft->GetName());
      
      // Save current thruster so that GetRefObject() can return it (LOJ: 2009.08.28)
      thrusterMap[current->GetName()] = current;
      
      // FiniteBurn class is friend of Thruster class, so we can access
      // member data directly
      current->ComputeInertialDirection(epoch);
      dir = current->inertialDirection;
      norm = sqrt(dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2]);
      
      #ifdef DEBUG_FINITE_BURN
         MessageInterface::ShowMessage
         (wxT("   Thruster Direction: %.15f  %.15f  %.15f\n")
          wxT("                 norm: %.15f\n"), dir[0], dir[1], dir[2], norm);
      #endif
         
      if (norm == 0.0)
         throw BurnException(wxT("FiniteBurn::Fire thruster ") + (*i) +
                             wxT(" on spacecraft ") + spacecraft->GetName() +
                             wxT(" has no direction."));
      
      dm += current->CalculateMassFlow();
      //tOverM = current->thrust / (tMass * norm * 1000.0); //old code
      tOverM = current->thrust * current->thrustScaleFactor *
               current->dutyCycle / (tMass * norm * 1000.0);
      
      deltaV[0] += dir[0] * tOverM;
      deltaV[1] += dir[1] * tOverM;
      deltaV[2] += dir[2] * tOverM;
      
      #ifdef DEBUG_FINITE_BURN
         MessageInterface::ShowMessage(wxT("   Thruster %s = %s details:\n"), 
            (*i).c_str(), current->GetName().c_str());
         MessageInterface::ShowMessage(
            wxT("   thrust   = %.15f\n")
            wxT("       dM   = %.15e\n      Mass  = %.15f\n")
            wxT("      TSF   = %.15f\n      |Acc| = %.15e\n      ")
            wxT("Acc   = [%.15e   %.15e   %.15e]\n"), current->thrust,
            dm, tMass, current->thrustScaleFactor, tOverM,
            deltaV[0], deltaV[1], deltaV[2]);
      #endif
   }
   
   // Build the acceleration
   burnData[0] = deltaV[0]*frameBasis[0][0] +
                 deltaV[1]*frameBasis[0][1] +
                 deltaV[2]*frameBasis[0][2];
   burnData[1] = deltaV[0]*frameBasis[1][0] +
                 deltaV[1]*frameBasis[1][1] +
                 deltaV[2]*frameBasis[1][2];
   burnData[2] = deltaV[0]*frameBasis[2][0] +
                 deltaV[1]*frameBasis[2][1] +
                 deltaV[2]*frameBasis[2][2];
   burnData[3] = dm;
   
   #ifdef DEBUG_FINITEBURN_FIRE
      MessageInterface::ShowMessage(
          wxT("FiniteBurn::Fire() this<%p>'%s' returning\n")
          wxT("   Acceleration:  %.15e  %.15e  %.15e  dm: %.15e\n"), this,
          GetName().c_str(), burnData[0], burnData[1], burnData[2], dm);
   #endif

   return true;
}


//------------------------------------------------------------------------------
//  wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Gets the name of the parameter with the input id.
 * 
 * @param <id> Integer id for the parameter.
 * 
 * @return The string name of the parameter.
 */
//------------------------------------------------------------------------------
wxString FiniteBurn::GetParameterText(const Integer id) const
{
   if (id >= BurnParamCount && id < FiniteBurnParamCount)
      return PARAMETER_TEXT[id - BurnParamCount];
      
   return Burn::GetParameterText(id);
}


//------------------------------------------------------------------------------
//  Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * Gets the id corresponding to a named parameter.
 * 
 * @param <str> Name of the parameter.
 * 
 * @return The ID.
 */
//------------------------------------------------------------------------------
Integer FiniteBurn::GetParameterID(const wxString &str) const
{
   // Check for deprecated parameters
   if (str == wxT("Tanks"))
   {
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** \"Tanks\" field of FiniteBurn ")
          wxT("is deprecated and will be removed from a future build.\n"));
      return FUEL_TANK;
   }
   
   if (str == wxT("BurnScaleFactor"))
   {
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** \"BurnScaleFactor\" field of FiniteBurn ")
          wxT("is deprecated and will be removed from a future build.\n"));
      return BURN_SCALE_FACTOR;
   }
   
   if (str == wxT("CoordinateSystem"))
   {
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** \"CoordinateSystem\" field of FiniteBurn ")
          wxT("is deprecated and will be removed from a future build.\n"));
      return COORDINATESYSTEM;
   }
   
   if (str == wxT("Origin"))
   {
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** \"Origin\" field of FiniteBurn ")
          wxT("is deprecated and will be removed from a future build.\n"));
      return BURNORIGIN;
   }
   
   if (str == wxT("Axes"))
   {
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** \"Axes\" field of FiniteBurn ")
          wxT("is deprecated and will be removed from a future build.\n"));
      return BURNAXES;
   }
   
   for (Integer i = BurnParamCount; i < FiniteBurnParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - BurnParamCount])
         return i;
   }
   
   return Burn::GetParameterID(str);
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
Gmat::ParameterType FiniteBurn::GetParameterType(const Integer id) const
{
   if (id >= BurnParamCount && id < FiniteBurnParamCount)
      return PARAMETER_TYPE[id - BurnParamCount];
      
   return Burn::GetParameterType(id);
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
wxString FiniteBurn::GetParameterTypeString(const Integer id) const
{
   return Burn::PARAM_TYPE_STRING[GetParameterType(id)];
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <id> Description for the parameter.
 *
 * @return true if the parameter is read only, false (the default) if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool FiniteBurn::IsParameterReadOnly(const Integer id) const
{
   if ((id == FUEL_TANK) || (id == BURN_SCALE_FACTOR) ||
       (id == COORDINATESYSTEM) || (id == BURNORIGIN) || (id == BURNAXES) || 
       (id == DELTAV1) || (id == DELTAV2) || (id == DELTAV3))
      return true;
   
   return Burn::IsParameterReadOnly(id);
}


//------------------------------------------------------------------------------
//  wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
wxString FiniteBurn::GetStringParameter(const Integer id) const
{
   // CoordinateSystem, Origin, Axes are not valid FiniteBurn parameters,
   // so handle here.
   switch (id)
   {
   case COORDINATESYSTEM:
   case BURNORIGIN:
   case BURNAXES:
      return wxT("Deprecated"); // just to ignore
   default:
      break;
   }
   
   return Burn::GetStringParameter(id);
}


//------------------------------------------------------------------------------
//  bool SetStringParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * Sets the value for a wxString parameter.
 * 
 * @param <id>    Integer ID of the parameter.
 * @param <value> New value for the parameter.
 * 
 * @return The value of the parameter.
 */
//------------------------------------------------------------------------------
bool FiniteBurn::SetStringParameter(const Integer id, const wxString &value)
{
   // CoordinateSystem, Origin, Axes are not valid FiniteBurn parameters,
   // so handle here.
   switch (id)
   {
   case FUEL_TANK:
   case COORDINATESYSTEM:
   case BURNORIGIN:
   case BURNAXES:
      return true; // just to ignore
   default:
      break;
   }
   
   if (id == THRUSTER)
   {
      if (find(thrusterNames.begin(), thrusterNames.end(), value) == thrusterNames.end())
         thrusterNames.push_back(value);
      initialized = false;
      return true;
   }
   
   return Burn::SetStringParameter(id, value);
}


//------------------------------------------------------------------------------
//  Real SetStringParameter(const Integer id, const Real value,
//                          const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets the value for a specific wxString element in an array.
 *
 * @param <id>    Integer ID of the parameter.
 * @param <value> New value for the parameter.
 * @param <index> Index for the element
 *
 * @return true on success
 */
//------------------------------------------------------------------------------
bool FiniteBurn::SetStringParameter(const Integer id, const wxString &value,
                                    const Integer index)
{
   if (id == FUEL_TANK)
      return true;     // just to ignore
   
   Integer count;
   
   if (id == THRUSTER)
   {
      count = thrusterNames.size();
      if (index > count)
         throw BurnException(wxT("Attempting to write thruster ") + value +
                  wxT(" past the allowed range for FiniteBurn ") + instanceName);
      if (find(thrusterNames.begin(), thrusterNames.end(), value) != thrusterNames.end())
      {
         if (thrusterNames[index] == value)
            return true;
         throw BurnException(wxT("Thruster ") + value +
                  wxT(" already set for FiniteBurn ") + instanceName);
      }
      if (index == count)
         thrusterNames.push_back(value);
      else
         thrusterNames[index] = value;

      initialized = false;
      return true;
   }
   
   return Burn::SetStringParameter(id, value, index);
}


//---------------------------------------------------------------------------
//  const StringArray& GetStringArrayParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Access an array of string data.
 * 
 * For the Burn classes, calls to this method get passed to the maneuver frame
 * manager when the user requests the frames that are available for the system.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The requested StringArray; throws if the parameter is not a 
 *         StringArray.
 */
//---------------------------------------------------------------------------
const StringArray& FiniteBurn::GetStringArrayParameter(const Integer id) const
{
   if (id == FUEL_TANK)
      return tankNames;  // deprecated
   
   if (id == THRUSTER)
      return thrusterNames;
   
   return Burn::GetStringArrayParameter(id);
}


//---------------------------------------------------------------------------
//  Real GetRealParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Access the Real data associated with this burn.
 * 
 * @param <id> The integer ID for the parameter.
 *
 * @return The requested Real.
 */
//---------------------------------------------------------------------------
Real FiniteBurn::GetRealParameter(const Integer id) const
{
   if (id == BURN_SCALE_FACTOR)  // deprecated
      return REAL_PARAMETER_UNDEFINED;
   
   return Burn::GetRealParameter(id);
}


//---------------------------------------------------------------------------
//  Real setRealParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Change the Real data associated with this burn.
 * 
 * @param <id>    Integer ID of the parameter.
 * @param <value> New value for the parameter.
 *
 * @exception <BurnException> thrown if value is out of range
 * 
 * @return The value of the parameter at the end of the call.
 */
//---------------------------------------------------------------------------
Real FiniteBurn::SetRealParameter(const Integer id, const Real value)
{
   if (id == BURN_SCALE_FACTOR)  // deprecated
      return value;
   
   return Burn::SetRealParameter(id, value);
}


//------------------------------------------------------------------------------
// virtual bool HasRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
bool FiniteBurn::HasRefObjectTypeArray()
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
const ObjectTypeArray& FiniteBurn::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   
   // Get ref. object types from the parent class
   refObjectTypes = Burn::GetRefObjectTypeArray();
   
   // Add ref. object types from this class if not already added
   if (find(refObjectTypes.begin(), refObjectTypes.end(), Gmat::THRUSTER) ==
       refObjectTypes.end())
      refObjectTypes.push_back(Gmat::THRUSTER);
   
   return refObjectTypes;
}


//------------------------------------------------------------------------------
// virtual const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
const StringArray& FiniteBurn::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   #ifdef DEBUG_FINITEBURN_OBJECT
   MessageInterface::ShowMessage
      (wxT("FiniteBurn::GetRefObjectNameArray() this=<%p>'%s' entered, type=%d\n"),
       this, GetName().c_str(), type);
   #endif
   
   refObjectNames.clear();
   
   if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::HARDWARE)
   {
      // Get ref. objects for requesting type from the parent class
      Burn::GetRefObjectNameArray(type);
      
      // Add ref. objects for requesting type from this class
      refObjectNames.insert(refObjectNames.end(), thrusterNames.begin(),
                            thrusterNames.end());
      
      #ifdef DEBUG_FINITEBURN_OBJECT
      MessageInterface::ShowMessage
         (wxT("FiniteBurn::GetRefObjectNameArray() this=<%p>'%s' returning %d ")
          wxT("ref. object names\n"), this, GetName().c_str(), refObjectNames.size());
      for (UnsignedInt i=0; i<refObjectNames.size(); i++)
         MessageInterface::ShowMessage(wxT("   '%s'\n"), refObjectNames[i].c_str());
      #endif
      
      return refObjectNames;
   }
   
   #ifdef DEBUG_FINITEBURN_OBJECT
   MessageInterface::ShowMessage
      (wxT("FiniteBurn::GetRefObjectNameArray() this=<%p>'%s' returning ")
       wxT("Burn::GetRefObjectNameArray()\n"), this, GetName().c_str());
   #endif
   
   return Burn::GetRefObjectNameArray(type);
}


//---------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name)
//---------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
GmatBase* FiniteBurn::GetRefObject(const Gmat::ObjectType type,
                                   const wxString &name)
{
   if (type == Gmat::THRUSTER)
   {
      if (thrusterMap.find(name) != thrusterMap.end())
         return thrusterMap[name];
      else
         return NULL;
   }
   
   return Burn::GetRefObject(type, name);
}


//---------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name)
//---------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
bool FiniteBurn::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                              const wxString &name)
{
   if (type == Gmat::THRUSTER)
   {
      if (thrusterMap.find(name) != thrusterMap.end())
         thrusterMap[name] = obj;
      
      return true;
   }
   
   return Burn::SetRefObject(obj, type, name);
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the FiniteBurn.
 *
 * @return clone of the FiniteBurn.
 */
//------------------------------------------------------------------------------
GmatBase* FiniteBurn::Clone() const
{
   return (new FiniteBurn(*this));
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
void FiniteBurn::Copy(const GmatBase* orig)
{
   operator=(*((FiniteBurn *)(orig)));
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
/**
 * Renames reference object name used in this class.
 *
 * @param <type> reference object type.
 * @param <oldName> object name to be renamed.
 * @param <newName> new object name.
 * 
 * @return true if object name changed, false if not.
 */
//---------------------------------------------------------------------------
bool FiniteBurn::RenameRefObject(const Gmat::ObjectType type,
                                 const wxString &oldName,
                                 const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("FiniteBurn::RenameRefObject() type=%s, oldName=%s, newName=%s\n"),
       GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif
   
   if (type == Gmat::HARDWARE)
   {
      for (UnsignedInt i=0; i<thrusterNames.size(); i++)
         if (thrusterNames[i] == oldName)
            thrusterNames[i] = newName;
   }
   
   return Burn::RenameRefObject(type, oldName, newName);
}


bool FiniteBurn::DepletesMass()
{
   bool retval = false;

   ObjectArray thrusterArray = spacecraft->GetRefObjectArray(Gmat::THRUSTER);

   // Check each the thruster
   for (ObjectArray::iterator th = thrusterArray.begin();
        th != thrusterArray.end(); ++th)
   {
      if ((*th)->GetBooleanParameter(wxT("DecrementMass")))
      {
         retval = true;
         #ifdef DEBUG_MASS_FLOW
            MessageInterface::ShowMessage(wxT("Thruster %s decrements mass\n"),
                  (*th)->GetName().c_str());
         #endif
         // Save the mass depleting thruster pointer(s) for later access?
      }
   }

   return retval;
}


//------------------------------------------------------------------------------
// bool FiniteBurn::Initialize()
//------------------------------------------------------------------------------
/**
 * This method sets up the data structures and pointers for a finite burn.
 *
 * @return true on success.
 */
//------------------------------------------------------------------------------
bool FiniteBurn::Initialize()
{
   #ifdef DEBUG_FINITEBURN_INIT
   MessageInterface::ShowMessage
      (wxT("FiniteBurn::Initialize() <%p>'%s' entered, spacecraft=<%p>\n"), this,
       GetName().c_str(), spacecraft);
   #endif
   
   if (Burn::Initialize())
   {
      if (spacecraft == NULL)
         return false;
      
      SetThrustersFromSpacecraft();      
      initialized = true;
   }
   
   #ifdef DEBUG_FINITEBURN_INIT
   MessageInterface::ShowMessage
      (wxT("FiniteBurn::Initialize() <%p>'%s' returning %d\n"), this, GetName().c_str(),
       initialized);
   #endif
   
   return initialized;
}


//------------------------------------------------------------------------------
// bool SetThrustersFromSpacecraft()
//------------------------------------------------------------------------------
bool FiniteBurn::SetThrustersFromSpacecraft()
{
   #ifdef DEBUG_FINITEBURN_SET
   MessageInterface::ShowMessage
      (wxT("FiniteBurn::SetThrustersFromSpacecraft() entered, spacecraft=<%p>'%s'\n"),
       spacecraft, spacecraft ? spacecraft->GetName().c_str() : wxT("NULL"));
   MessageInterface::ShowMessage(wxT("   thrusterNames.size()=%d\n"), thrusterNames.size());
   #endif
   
   // Get thrusters and tanks associated to spacecraft
   ObjectArray thrusterArray = spacecraft->GetRefObjectArray(Gmat::THRUSTER);
   ObjectArray tankArray = spacecraft->GetRefObjectArray(Gmat::FUEL_TANK);
   
   // Look up the thruster(s)
   for (ObjectArray::iterator th = thrusterArray.begin(); 
        th != thrusterArray.end(); ++th)
   {
      for (StringArray::iterator thName = thrusterNames.begin();
           thName != thrusterNames.end(); ++thName)
      {
         // Only act on thrusters assigned to this burn
         if ((*th)->GetName() == *thName)
         {
            Integer paramId = (*th)->GetParameterID(wxT("Tank"));
            StringArray tankNames = (*th)->GetStringArrayParameter(paramId);
            // Setup the tankNames
            (*th)->TakeAction(wxT("ClearTankNames"));
            // Loop through each tank for the burn
            for (StringArray::iterator tankName = tankNames.begin();
                 tankName != tankNames.end(); ++tankName)
            {
               ObjectArray::iterator tnk = tankArray.begin();
               // Find the tank on the spacecraft
               while (tnk != tankArray.end())
               {
                  if ((*tnk)->GetName() == *tankName)
                  {
                     // Make the assignment
                     (*th)->SetStringParameter(wxT("Tank"), *tankName);
                     (*th)->SetRefObject(*tnk, (*tnk)->GetType(), 
                                         (*tnk)->GetName());
                     break;
                  }
                  // Not found; keep looking 
                  ++tnk;
                  if (tnk == tankArray.end())
                     throw BurnException
                        (wxT("FiniteBurn::Initialize() cannot find tank ") +
                         (*tankName) + wxT(" for burn ") + instanceName);
               }
            }
         }
      }
   }
   
   #ifdef DEBUG_FINITEBURN_SET
   MessageInterface::ShowMessage
      (wxT("FiniteBurn::SetThrustersFromSpacecraft() returning true\n"));
   #endif
   
   return true;
}


