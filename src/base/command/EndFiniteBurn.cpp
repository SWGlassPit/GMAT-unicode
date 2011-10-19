//$Id: EndFiniteBurn.cpp 9791 2011-08-24 16:17:06Z wendys-dev $
//------------------------------------------------------------------------------
//                            EndFiniteBurn
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under MOMS task
// order 124.
//
// Author: Darrel J. Conway, Thinking Systems, Inc.
// Created: 2005/01/04
//
/**
 *  Implementation code for the EndFiniteBurn command.
 */
//------------------------------------------------------------------------------

#include "EndFiniteBurn.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_END_MANEUVER
//#define DEBUG_END_MANEUVER_EXE


//------------------------------------------------------------------------------
// EndFiniteBurn()
//------------------------------------------------------------------------------
/**
 * Default constructor.
 */
//------------------------------------------------------------------------------
EndFiniteBurn::EndFiniteBurn() :
   GmatCommand        (wxT("EndFiniteBurn")),
   thrustName         (wxT("")),
   burnForce          (NULL),
   burnName           (wxT("")),
   maneuver           (NULL),
   transientForces    (NULL),
   firstTimeExecution (true)
{
   if (instanceName == wxT(""))
      instanceName = wxT("EndFiniteBurn");
   physicsBasedCommand = true;
}


//------------------------------------------------------------------------------
// ~EndFiniteBurn()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
EndFiniteBurn::~EndFiniteBurn()
{
}


//------------------------------------------------------------------------------
// EndFiniteBurn(const BeginManeuver& endman)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 * 
 * @param <endman> The command that gets copied.
 */
//------------------------------------------------------------------------------
EndFiniteBurn::EndFiniteBurn(const EndFiniteBurn& endman) :
   GmatCommand        (endman),
   thrustName         (endman.thrustName),
   burnForce          (NULL),
   burnName           (endman.burnName),
   maneuver           (NULL),
   transientForces    (NULL),
   satNames           (endman.satNames),
   firstTimeExecution (true)
{
   sats.clear();
   thrusters.clear();
}


//------------------------------------------------------------------------------
// EndFiniteBurn& operator=(const EndFiniteBurn& endman)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 * 
 * @param <endman> The command that gets copied.
 * 
 * @return this instance, with internal data structures set to match the input
 *         instance.
 */
//------------------------------------------------------------------------------
EndFiniteBurn& EndFiniteBurn::operator=(const EndFiniteBurn& endman)
{
   if (&endman == this)
      return *this;
      
   GmatCommand::operator=(endman);
   thrustName = endman.thrustName;
   burnForce = NULL;
   burnName = endman.burnName;
   maneuver = NULL;
   transientForces = NULL;
   satNames = endman.satNames;
   firstTimeExecution = true;
   
   sats.clear();
   thrusters.clear();
   
   return *this;
}


//------------------------------------------------------------------------------
// virtual bool TakeAction(const wxString &action,  
//                         const wxString &actionData = wxT(""));
//------------------------------------------------------------------------------
/**
 * This method performs action.
 *
 * @param <action> action to perform
 * @param <actionData> action data associated with action
 * @return true if action successfully performed
 *
 */
//------------------------------------------------------------------------------
bool EndFiniteBurn::TakeAction(const wxString &action,
                                 const wxString &actionData)
{
   #if DEBUG_TAKE_ACTION
   MessageInterface::ShowMessage
      (wxT("EndFiniteBurn::TakeAction() action=%s, actionData=%s\n"),
       action.c_str(), actionData.c_str());
   #endif
   
   if (action == wxT("Clear"))
   {
      satNames.clear();
      return true;
   }

   return false;
}


//------------------------------------------------------------------------------
// wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
/**
 * Accesses names for referenced objects.
 * 
 * @param <type> Type of object requested.
 * 
 * @return the referenced object's name.
 */
//------------------------------------------------------------------------------
wxString EndFiniteBurn::GetRefObjectName(const Gmat::ObjectType type) const
{
   switch (type)
   {
      case Gmat::FINITE_BURN:
         #ifdef DEBUG_END_MANEUVER
            MessageInterface::ShowMessage
               (wxT("Getting EndFiniteBurn reference burn names\n"));
         #endif
         return burnName;
         
      default:
         ;
   }
   
   return GmatCommand::GetRefObjectName(type);
}

//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by the BeginFiniteBurn.
 *
 * @return the list of object types.
 * 
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& EndFiniteBurn::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::FINITE_BURN);
   refObjectTypes.push_back(Gmat::SPACECRAFT);
   return refObjectTypes;
}



//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Accesses arrays of names for referenced objects.
 * 
 * @param <type> Type of object requested.
 * 
 * @return the StringArray containing the referenced object names.
 */
//------------------------------------------------------------------------------
const StringArray& EndFiniteBurn::GetRefObjectNameArray(
                             const Gmat::ObjectType type)
{
   refObjectNames.clear();
   
   if (type == Gmat::UNKNOWN_OBJECT ||
       type == Gmat::SPACECRAFT)
   {
      refObjectNames.insert(refObjectNames.end(), satNames.begin(), satNames.end());
   }
   
   if (type == Gmat::UNKNOWN_OBJECT ||
       type == Gmat::FINITE_BURN)
   {
      refObjectNames.push_back(burnName);
   }

   return refObjectNames;
/*   switch (type)
   {
      case Gmat::SPACECRAFT:
         #ifdef DEBUG_END_MANEUVER
            MessageInterface::ShowMessage
               (wxT("Getting EndFiniteBurn reference spacecraft list\n"));
         #endif
         return satNames;
      
      default:
         ;
   }
   
   return GmatCommand::GetRefObjectNameArray(type);
   */
}


//------------------------------------------------------------------------------
// bool SetRefObjectName(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Sets names for referenced objects.
 * 
 * @param <type> Type of the object.
 * @param <name> Name of the object.
 * 
 * @return true if the name was set, false if not.
 */
//------------------------------------------------------------------------------
bool EndFiniteBurn::SetRefObjectName(const Gmat::ObjectType type, 
                                     const wxString &name)
{
   switch (type)
   {
      case Gmat::SPACECRAFT:
         #ifdef DEBUG_END_MANEUVER
            MessageInterface::ShowMessage
               (wxT("Setting EndFiniteBurn reference spacecraft \"%s\"\n"), 
                name.c_str());
         #endif
         satNames.push_back(name);
         return true;
      
      case Gmat::FINITE_BURN:
         #ifdef DEBUG_END_MANEUVER
            MessageInterface::ShowMessage
               (wxT("Setting EndFiniteBurn reference burn \"%s\"\n"), name.c_str());
         #endif
         burnName = name;
         return true;
         
      default:
         #ifdef DEBUG_END_MANEUVER
            MessageInterface::ShowMessage
               (wxT("EndFiniteBurn reference object \"%s\" not set!\n"), 
                name.c_str());
         #endif
         ;
   }
   
   return GmatCommand::SetRefObjectName(type, name);
}


//------------------------------------------------------------------------------
//  GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the EndFiniteBurn command.
 *
 * @return clone of the Propagate.
 */
//------------------------------------------------------------------------------
GmatBase* EndFiniteBurn::Clone() const
{
   return new EndFiniteBurn(*this);
}


//------------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//------------------------------------------------------------------------------
/**
 * Renames referenced objects.
 *
 * @param type Type of the object that is renamed.
 * @param oldName The current name for the object.
 * @param newName The name the object has when this operation is complete.
 *
 * @return true on success.
 */
//------------------------------------------------------------------------------
bool EndFiniteBurn::RenameRefObject(const Gmat::ObjectType type,
                                    const wxString &oldName,
                                    const wxString &newName)
{
   // EndFiniteBurn needs to know about Burn and Spacecraft only
   if (type != Gmat::FINITE_BURN && type != Gmat::SPACECRAFT)
      return true;
   
   if (burnName == oldName)
      burnName = newName;
   
   for (UnsignedInt i=0; i<satNames.size(); i++)
      if (satNames[i] == oldName)
         satNames[i] = newName;
   
   return true;
}


//------------------------------------------------------------------------------
//  const wxString GetGeneratingString()
//------------------------------------------------------------------------------
/**
 * Method used to retrieve the string that was parsed to build this GmatCommand.
 *
 * This method is used to retrieve the GmatCommand string from the script that
 * was parsed to build the GmatCommand.  It is used to save the script line, so
 * that the script can be written to a file without inverting the steps taken to
 * set up the internal object data.  As a side benefit, the script line is
 * available in the GmatCommand structure for debugging purposes.
 *
 * @param <mode>    Specifies the type of serialization requested.
 * @param <prefix>  Optional prefix appended to the object's name. (Used for
 *                  indentation)
 * @param <useName> Name that replaces the object's name (Not yet used
 *                  in commands).
 *
 * @return The script line that defines this GmatCommand.
 */
//------------------------------------------------------------------------------
const wxString& EndFiniteBurn::GetGeneratingString(Gmat::WriteMode mode,
                                                    const wxString &prefix,
                                                    const wxString &useName)
{
   generatingString = prefix + wxT("EndFiniteBurn ") + burnName + wxT("(");
   for (StringArray::iterator satIt = satNames.begin();
        satIt != satNames.end(); ++satIt)
   {
      if (satIt != satNames.begin())
         generatingString += wxT(", ");
      generatingString += *satIt;
   }

   generatingString += wxT(");");

   return GmatCommand::GetGeneratingString(mode, prefix, useName);
}


//------------------------------------------------------------------------------
//  void SetTransientForces(std::vector<PhysicalModel*> *tf)
//------------------------------------------------------------------------------
/**
 * Sets the array of transient forces for the command.
 *
 * @param <tf> The transient force vector.
 */
//------------------------------------------------------------------------------
void EndFiniteBurn::SetTransientForces(std::vector<PhysicalModel*> *tf)
{
   transientForces = tf;
}


//------------------------------------------------------------------------------
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * Initializes the BeginManeuver structures at the start of a run.
 *
 * @return true if the GmatCommand is initialized, false if an error occurs.
 */
//------------------------------------------------------------------------------
bool EndFiniteBurn::Initialize()
{
   bool retval = GmatCommand::Initialize();
   firstTimeExecution = true;
   
   GmatBase *mapObj;
   if (retval)
   {
      // Look up the maneuver object
      if ((mapObj = FindObject(burnName)) == NULL) 
         throw CommandException(wxT("EndFiniteBurn: Unknown finite burn \"") + burnName + wxT("\""));
      if (mapObj->GetTypeName() != wxT("FiniteBurn"))
         throw CommandException(wxT("EndFiniteBurn: ") + (burnName) + wxT(" is not a FiniteBurn"));
      maneuver = (FiniteBurn*)mapObj;
      
      // find all of the spacecraft
      StringArray::iterator scName;
      Spacecraft *sc;
      
      sats.clear();
      for (scName = satNames.begin(); scName != satNames.end(); ++scName)
      {
         if ((mapObj = FindObject(*scName)) == NULL) 
            throw CommandException(wxT("EndFiniteBurn: Unknown SpaceObject \"") + (*scName) + wxT("\""));
         
         if (mapObj->GetType() != Gmat::SPACECRAFT)
            throw CommandException(wxT("EndFiniteBurn: ") + (*scName) + wxT(" is not a Spacecraft"));
         sc = (Spacecraft*)mapObj;
         sats.push_back(sc);
      }
      
      // We cannot validate thrusters until execution time(LOJ: 2010.08.13)
      // A script can have sc2 = sc1 before running BeginFiniteBurn
      //ValidateThrusters();
   }
   
   thrustName = burnName + wxT("_FiniteThrust");
   #ifdef DEBUG_END_MANEUVER
      MessageInterface::ShowMessage
         (wxT("EndFiniteBurn initialized with thrust force named \"%s\"\n"), 
          thrustName.c_str());
   #endif
   
   return initialized;
}


//------------------------------------------------------------------------------
// bool Execute()
//------------------------------------------------------------------------------
/** 
 * The method that is fired to turn off thrusters.
 *
 * @return true if the GmatCommand runs to completion, false if an error 
 *         occurs. 
 */
//------------------------------------------------------------------------------
bool EndFiniteBurn::Execute()
{
   if (firstTimeExecution)
   {
      // Get updated thruster pointers from the spacecraft since spacecraft
      // clones them
      //ValidateThrusters(); // commented out (LOJ: 2010.01.25)
      firstTimeExecution = false;
   }
   
   // Let's validate thrusters all the time (LOJ: 2010.01.25)
   ValidateThrusters();
   
   // Turn off all of the referenced thrusters
   for (std::vector<Thruster*>::iterator i = thrusters.begin(); 
        i != thrusters.end(); ++i)
   {
      Thruster *th = *i;
      #ifdef DEBUG_END_MANEUVER_EXE
         MessageInterface::ShowMessage
            (wxT("EndFiniteBurn::Execute() Deactivating engine <%p>'%s'\n"), th,
             th->GetName().c_str());
      #endif
      th->SetBooleanParameter(th->GetParameterID(wxT("IsFiring")), false);

      #ifdef DEBUG_END_MANEUVER_EXE
         MessageInterface::ShowMessage
            (wxT("Checking to see if engine is inactive: returned %s\n"), 
             (th->GetBooleanParameter(th->GetParameterID(wxT("IsFiring"))) ? 
              wxT("true") : wxT("false")));
      #endif      
   }
   
   
   // Tell active spacecraft that they are no longer firing
   for (std::vector<Spacecraft*>::iterator s=sats.begin(); s!=sats.end(); ++s)
   {
      /// todo: Be sure that no other maneuver has the spacecraft maneuvering
      (*s)->IsManeuvering(false);
//      (*current)->TakeAction(wxT("ReleaseCartesianStateDynamics"));
   }
   
   // Remove the force from the list of transient forces
   for (std::vector<PhysicalModel*>::iterator j = transientForces->begin();
        j != transientForces->end(); ++j)
   {
      if (((*j)->GetName()) == thrustName)
      {
         #ifdef DEBUG_TRANSIENT_FORCES
         MessageInterface::ShowMessage
            (wxT("EndFiniteBurn::Execute() Removing burnForce<%p>'%s' from transientForces\n"),
             burnForce, burnForce->GetName().c_str());
         #endif
         transientForces->erase(j);
         break;
      }
   }
   
   // Reset maneuvering to Publisher so that any subscriber can do its own action
   if (!sats.empty())
   {
      Real epoch = sats[0]->GetEpoch();
      publisher->SetManeuvering(this, false, epoch, satNames, wxT("end of finite maneuver"));
   }
   
   #ifdef DEBUG_END_MANEUVER_EXE
      MessageInterface::ShowMessage(wxT("EndFiniteBurn::Execute() Current TransientForces list:\n"));
      for (std::vector<PhysicalModel*>::iterator j = transientForces->begin();
           j != transientForces->end(); ++j)
         MessageInterface::ShowMessage(wxT("   %s\n"), (*j)->GetName().c_str());
   #endif
   
   BuildCommandSummary(true);
   return true;
}


//------------------------------------------------------------------------------
// void ValidateThrusters()
//------------------------------------------------------------------------------
/**
 * Validate that the spacecrafts have the thrusters they need.
 */
//------------------------------------------------------------------------------
void EndFiniteBurn::ValidateThrusters()
{
   thrusters.clear();
   for (std::vector<Spacecraft*>::iterator current = sats.begin(); 
        current != sats.end(); ++current)
   {
      #ifdef DEBUG_EFB_THRUSTER
      MessageInterface::ShowMessage
         (wxT("EndFiniteBurn::ValidateThrusters() entered, checking Spacecraft ")
          wxT("<%p>'%s' for Thrusters\n"), *current, (*current)->GetName().c_str());
      #endif
      
      StringArray thrusterNames = (*current)->GetStringArrayParameter(
                                  (*current)->GetParameterID(wxT("Thrusters")));
      StringArray engines = (maneuver)->GetStringArrayParameter(
                            (maneuver)->GetParameterID(wxT("Thrusters")));
      
      #ifdef DEBUG_EFB_THRUSTER
      MessageInterface::ShowMessage
         (wxT("   Spacecraft has %d Thrusters and FiniteBurn has %d thrusters\n"),
          thrusterNames.size(), engines.size());
      #endif
      
      for (StringArray::iterator i = engines.begin(); i != engines.end(); ++i)
      {
         if (find(thrusterNames.begin(), thrusterNames.end(), *i) == 
             thrusterNames.end())
         {
            thrusters.clear();
            throw CommandException(wxT("EndFiniteBurn: Spacecraft ") + (*current)->GetName() +
                                   wxT(" does not have a thruster named \"") +
                                   (*i) + wxT("\""));
         }
         
         Thruster* th = 
            (Thruster*)((*current)->GetRefObject(Gmat::THRUSTER, *i));
         
         if (th)
         {
            #ifdef DEBUG_EFB_THRUSTER
            MessageInterface::ShowMessage
               (wxT("EndFiniteBurn::ValidateThrusters() addding the Thruster <%p>'%s' ")
                wxT("to thrusters\n"), th, th->GetName().c_str());
            #endif
            thrusters.push_back(th);
         }
         else
         {
            thrusters.clear();
            throw CommandException(wxT("EndFiniteBurn: Thruster object \"") + (*i) +
                                   wxT("\" was not set on Spacecraft \"") 
                                   + (*current)->GetName() + wxT("\""));
         }
      }
   }
   
   #ifdef DEBUG_EFB_THRUSTER
   MessageInterface::ShowMessage
      (wxT("EndFiniteBurn::ValidateThrusters() leaving\n"));
   #endif
}

