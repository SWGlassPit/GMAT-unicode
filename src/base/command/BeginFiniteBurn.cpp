//$Id: BeginFiniteBurn.cpp 9791 2011-08-24 16:17:06Z wendys-dev $
//------------------------------------------------------------------------------
//                            BeginFiniteBurn
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
 *  Implementation code for the BeginFiniteBurn command.
 */
//------------------------------------------------------------------------------

#include "BeginFiniteBurn.hpp"


//#define DEBUG_BEGIN_MANEUVER
//#define DEBUG_BEGIN_MANEUVER_EXE
//#define DEBUG_TRANSIENT_FORCES
//#define DEBUG_BFB_THRUSTER

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

#include "MessageInterface.hpp"


//------------------------------------------------------------------------------
// BeginFiniteBurn()
//------------------------------------------------------------------------------
/**
 * Default constructor.
 */
//------------------------------------------------------------------------------
BeginFiniteBurn::BeginFiniteBurn() :
   GmatCommand        (wxT("BeginFiniteBurn")),
   burnName           (wxT("")),
   maneuver           (NULL),
   burnForce          (NULL),
   transientForces    (NULL),
   firstTimeExecution (true)
{
   if (instanceName == wxT(""))
      instanceName = wxT("BeginFiniteBurn");
   physicsBasedCommand = true;
}


//------------------------------------------------------------------------------
// ~BeginFiniteBurn()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
BeginFiniteBurn::~BeginFiniteBurn()
{
   #ifdef DEBUG_TRANSIENT_FORCES
   MessageInterface::ShowMessage
      (wxT("BeginFiniteBurn destructor entered, this=<%p>, burnForce=<%p>'%s'\n"),
       this, burnForce, burnForce ? burnForce->GetName().c_str() : wxT("NULL"));
   #endif
   
   if (burnForce)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (burnForce, burnForce->GetName(), wxT("BeginFiniteBurn::~BeginFiniteBurn()"),
          wxT("deleting burn force"));
      #endif
      delete burnForce;
   }
}


//------------------------------------------------------------------------------
// BeginFiniteBurn(const BeginFiniteBurn& begman)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 * 
 * @param begman The command that gets copied.
 */
//------------------------------------------------------------------------------
BeginFiniteBurn::BeginFiniteBurn(const BeginFiniteBurn& begman) :
   GmatCommand        (begman),
   burnName           (begman.burnName),
   maneuver           (NULL),
   burnForce          (NULL),
   transientForces    (NULL),
   satNames           (begman.satNames),
   firstTimeExecution (true)
{
   sats.clear();
   thrusters.clear();
}


//------------------------------------------------------------------------------
// BeginFiniteBurn& operator=(const BeginFiniteBurn& begman)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 * 
 * @param begman The command that gets copied.
 * 
 * @return this instance, with internal data structures set to match the input
 *         instance.
 */
//------------------------------------------------------------------------------
BeginFiniteBurn& BeginFiniteBurn::operator=(const BeginFiniteBurn& begman)
{
   if (&begman == this)
      return *this;
      
   GmatCommand::operator=(begman);
   burnName = begman.burnName;
   maneuver = NULL;
   burnForce = NULL;
   transientForces = NULL;
   satNames = begman.satNames;
   firstTimeExecution = true;
   
   sats.clear();
   thrusters.clear();
   
   return *this;
}


//------------------------------------------------------------------------------
// virtual bool TakeAction(const wxString &action,  
//                         const wxString &actionData = "");
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
bool BeginFiniteBurn::TakeAction(const wxString &action,
                                 const wxString &actionData)
{
   #if DEBUG_TAKE_ACTION
   MessageInterface::ShowMessage
      (wxT("BeginFiniteBurn::TakeAction() action=%s, actionData=%s\n"),
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
 * @param type Type of object requested.
 * 
 * @return the referenced object's name.
 */
//------------------------------------------------------------------------------
wxString BeginFiniteBurn::GetRefObjectName(const Gmat::ObjectType type) const
{
   switch (type)
   {
      case Gmat::FINITE_BURN:
         #ifdef DEBUG_BEGIN_MANEUVER
            MessageInterface::ShowMessage
               (wxT("Getting BeginFiniteBurn reference burn names\n"));
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
const ObjectTypeArray& BeginFiniteBurn::GetRefObjectTypeArray()
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
 * @param type Type of object requested.
 * 
 * @return the StringArray containing the referenced object names.
 */
//------------------------------------------------------------------------------
const StringArray& BeginFiniteBurn::GetRefObjectNameArray(
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
   
//    switch (type)
//    {
//       case Gmat::SPACECRAFT:
//          #ifdef DEBUG_BEGIN_MANEUVER
//             MessageInterface::ShowMessage
//                ("Getting BeginFiniteBurn reference spacecraft list\n");
//          #endif
//          return satNames;
//       default:
//          ;
//    }
   
//    return GmatCommand::GetRefObjectNameArray(type);
}


//------------------------------------------------------------------------------
// bool SetRefObjectName(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Sets names for referenced objects.
 * 
 * @param type Type of the object.
 * @param name Name of the object.
 * 
 * @return true if the name was set, false if not.
 */
//------------------------------------------------------------------------------
bool BeginFiniteBurn::SetRefObjectName(const Gmat::ObjectType type, 
                                       const wxString &name)
{
   switch (type)
   {
      case Gmat::SPACECRAFT:
         #ifdef DEBUG_BEGIN_MANEUVER
            MessageInterface::ShowMessage
               (wxT("Setting BeginFiniteBurn reference spacecraft \"%s\"\n"), 
                name.c_str());
         #endif
         satNames.push_back(name);
         return true;
      
      case Gmat::FINITE_BURN:
         #ifdef DEBUG_BEGIN_MANEUVER
            MessageInterface::ShowMessage
               (wxT("Setting BeginFiniteBurn reference burn \"%s\"\n"), name.c_str());
         #endif
         burnName = name;
         return true;
         
      default:
         #ifdef DEBUG_BEGIN_MANEUVER
            MessageInterface::ShowMessage
               (wxT("BeginFiniteBurn reference object \"%s\" not set!\n"), 
                name.c_str());
         #endif
         ;
   }
   
   return GmatCommand::SetRefObjectName(type, name);
}


//------------------------------------------------------------------------------
// GmatBase* GetObject(const Gmat::ObjectType type, const wxString objName)
//------------------------------------------------------------------------------
/**
 * Access an object used by this instance.
 * 
 * This command constructs a transient force that can be accessed using this 
 * method.
 * 
 * @param type Type of the object.
 * @param objName Name of the object.
 *
 * @return The object.
 */
//------------------------------------------------------------------------------
GmatBase* BeginFiniteBurn::GetGmatObject(const Gmat::ObjectType type, 
                                   const wxString objName)
{
   if (type == Gmat::TRANSIENT_FORCE)
      return burnForce;
   return GmatCommand::GetGmatObject(type, objName);
}


//------------------------------------------------------------------------------
//  void BeginFiniteBurn::SetTransientForces(std::vector<PhysicalModel*> *tf)
//------------------------------------------------------------------------------
/**
 * Sets the array of transient forces for the command.
 *
 * @param tf The transient force vector.
 */
//------------------------------------------------------------------------------
void BeginFiniteBurn::SetTransientForces(std::vector<PhysicalModel*> *tf)
{
   #ifdef DEBUG_TRANSIENT_FORCES
   MessageInterface::ShowMessage
      (wxT("BeginFiniteBurn::SetTransientForces() tf=<%p>\n"), tf);
   #endif
   transientForces = tf;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the BeginFiniteBurn command.
 *
 * @return clone of the BeginFiniteBurn.
 */
//------------------------------------------------------------------------------
GmatBase* BeginFiniteBurn::Clone() const
{
   return new BeginFiniteBurn(*this);
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
bool BeginFiniteBurn::RenameRefObject(const Gmat::ObjectType type,
                                      const wxString &oldName,
                                      const wxString &newName)
{
   // BeginFiniteBurn needs to know about Burn and Spacecraft only
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
const wxString& BeginFiniteBurn::GetGeneratingString(Gmat::WriteMode mode,
                                                  const wxString &prefix,
                                                  const wxString &useName)
{
   generatingString = prefix + wxT("BeginFiniteBurn ") + burnName + wxT("(");
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
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * Initializes the BeginFiniteBurn structures at the start of a run.
 *
 * @return true if the GmatCommand is initialized, false if an error occurs.
 */
//------------------------------------------------------------------------------
bool BeginFiniteBurn::Initialize()
{
   bool retval = GmatCommand::Initialize();
   firstTimeExecution = true;
   
   #ifdef DEBUG_BEGIN_MANEUVER
      MessageInterface::ShowMessage
         (wxT("BeginFiniteBurn::Initialize() entered. burnName=%s\n"), burnName.c_str());
   #endif
   
   GmatBase *mapObj = NULL;
   if (retval)
   {
      // Look up the maneuver object
      if ((mapObj = FindObject(burnName)) == NULL) 
         throw CommandException(wxT("Unknown finite burn \"") + burnName + wxT("\"\n"));
      if (mapObj->GetTypeName() != wxT("FiniteBurn"))
         throw CommandException((burnName) + wxT(" is not a FiniteBurn\n"));

      maneuver = (FiniteBurn*)mapObj;

      #ifdef DEBUG_BEGIN_MANEUVER
         MessageInterface::ShowMessage(
            wxT("BeginFiniteBurn::Initialize() found %s with type %s\n"), 
            maneuver->GetName().c_str(), maneuver->GetTypeName().c_str());
      #endif      

      // find all of the spacecraft
      StringArray::iterator scName;
      Spacecraft *sc;
      sats.clear();
      for (scName = satNames.begin(); scName != satNames.end(); ++scName)
      {
         if ((mapObj = FindObject(*scName)) == NULL) 
            throw CommandException(wxT("Unknown SpaceObject \"") + (*scName) + wxT("\""));

         if (mapObj->GetType() != Gmat::SPACECRAFT)
            throw CommandException((*scName) + wxT(" is not a Spacecraft"));
         sc = (Spacecraft*)mapObj;

         #ifdef DEBUG_BEGIN_MANEUVER
            MessageInterface::ShowMessage(
               wxT("BeginFiniteBurn::Initialize() found %s with type %s\n"), 
               scName->c_str(), sc->GetTypeName().c_str());
         #endif
         
         sats.push_back(sc);
      }
      
      // Validate that the spacecraft have the thrusters they need
      // We cannot validate thrusters until execution time(LOJ: 2010.08.13)
      // A script can have sc2 = sc1 before running BeginFiniteBurn
      //ValidateThrusters();
      
      // Delete old burnForce
      if (burnForce != NULL)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (burnForce, burnForce->GetName(), wxT("BeginFiniteBurn::Initialize()"),
             wxT("deleting burn force"));
         #endif
         delete burnForce;
      }
      
      // If all is okay, create the FiniteThrust object and configure it.
      wxString thrustName = burnName + wxT("_FiniteThrust");
      burnForce = new FiniteThrust(thrustName);
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Add
         (burnForce, thrustName, wxT("BeginFiniteBurn::Initialize()"),
          wxT("burnForce = new FiniteThrust()"));
      #endif
      
      burnForce->SetRefObject(maneuver, maneuver->GetType(),
                              maneuver->GetName());
      Gmat::ObjectType type = Gmat::SPACECRAFT;
      StringArray::iterator iter;
      
      // load up the spacecraft name list
      for (iter = satNames.begin(); iter != satNames.end(); ++iter)
      {
         #ifdef DEBUG_BEGIN_MANEUVER
            MessageInterface::ShowMessage(
               wxT("BeginFiniteBurn::Initialize() setting %s on %s\n"), 
               iter->c_str(), thrustName.c_str());
         #endif
         burnForce->SetRefObjectName(type, *iter);
      }
   }
   
   #ifdef DEBUG_BEGIN_MANEUVER
   MessageInterface::ShowMessage
      (wxT("BeginFiniteBurn::Initialize() returning %d\n"), initialized);
   #endif
   
   return initialized;
}


//------------------------------------------------------------------------------
// bool Execute()
//------------------------------------------------------------------------------
/** 
 * The method that is fired to turn on thrusters.
 *
 * @return true if the GmatCommand runs to completion, false if an error 
 *         occurs. 
 */
//------------------------------------------------------------------------------
bool BeginFiniteBurn::Execute()
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
   
   // Turn on all of the referenced thrusters
   #ifdef DEBUG_BEGIN_MANEUVER_EXE
   MessageInterface::ShowMessage
      (wxT("BeginFiniteBurn::Execute() <%p>'%s' entered, firstTimeExecution=%d, \n   ")
       wxT("There are %d thruster(s) in use\n"), this, GetGeneratingString(Gmat::NO_COMMENTS).c_str(),
       firstTimeExecution, thrusters.size());
   #endif
   for (std::vector<Thruster*>::iterator i = thrusters.begin(); 
        i != thrusters.end(); ++i)
   {
      Thruster *th = *i;
      #ifdef DEBUG_BEGIN_MANEUVER_EXE
         MessageInterface::ShowMessage
            (wxT("Activating engine <%p>'%s'\n"), th, th->GetName().c_str());
      #endif
      th->SetBooleanParameter(th->GetParameterID(wxT("IsFiring")), true);
      
      #ifdef DEBUG_BEGIN_MANEUVER_EXE
         MessageInterface::ShowMessage
            (wxT("Checking to see if engine is active: returned %s\n"), 
             (th->GetBooleanParameter(th->GetParameterID(wxT("IsFiring"))) ? 
              wxT("true") : _("false")));
      #endif
   }
   
   // Tell active spacecraft that they are now firing
   for (std::vector<Spacecraft*>::iterator s=sats.begin(); s!=sats.end(); ++s)
   {
      (*s)->IsManeuvering(true);
//      (*s)->TakeAction("RequireCartesianStateDynamics");
   }

   if (transientForces == NULL)
      throw CommandException(wxT("Transient force list was NOT initialized; ABORTING RUN!!!\n\n"));
   
   // Insert the force into the list of transient forces if not found
   if (find(transientForces->begin(), transientForces->end(), burnForce) ==
       transientForces->end())
   {
      #ifdef DEBUG_TRANSIENT_FORCES
      MessageInterface::ShowMessage
         (wxT("BeginFiniteBurn::Execute() Adding burnForce<%p>'%s' to transientForces\n"),
          burnForce, burnForce->GetName().c_str());
      #endif
      transientForces->push_back(burnForce);
   }
   
   // Set maneuvering to Publisher so that any subscriber can do its own action
   if (!sats.empty())
   {
      Real epoch = sats[0]->GetEpoch();
      publisher->SetManeuvering(this, true, epoch, satNames, wxT("begin of finite maneuver"));
   }
   
   #ifdef DEBUG_BEGIN_MANEUVER_EXE
      MessageInterface::ShowMessage
         (wxT("There are %d transient force(s)\n"), transientForces->size());
      MessageInterface::ShowMessage(wxT("Current TransientForces list:\n"));
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
void BeginFiniteBurn::ValidateThrusters()
{
   thrusters.clear();
   for (std::vector<Spacecraft*>::iterator current = sats.begin(); 
        current != sats.end(); ++current)
   {
      #ifdef DEBUG_BFB_THRUSTER
      MessageInterface::ShowMessage
         (wxT("BeginFiniteBurn::ValidateThrusters() entered, checking Spacecraft ")
          wxT("<%p>'%s' for Thrusters\n"), *current, (*current)->GetName().c_str());
      #endif
      
      StringArray thrusterNames = (*current)->GetStringArrayParameter(
                                  (*current)->GetParameterID(wxT("Thrusters")));
      StringArray engines = (maneuver)->GetStringArrayParameter(
                            (maneuver)->GetParameterID(wxT("Thrusters")));
      
      #ifdef DEBUG_BFB_THRUSTER
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
            throw CommandException(wxT("BeginFiniteBurn: Spacecraft ") + (*current)->GetName() +
                                   wxT(" does not have a thruster named \"") +
                                   (*i) + wxT("\""));
         }
         
         Thruster* th = 
            (Thruster*)((*current)->GetRefObject(Gmat::THRUSTER, *i));
         
         if (th)
         {
            #ifdef DEBUG_BFB_THRUSTER
            MessageInterface::ShowMessage
               (wxT("BeginFiniteBurn::ValidateThrusters() addding the Thruster <%p>'%s' ")
                wxT("to thrusters\n"), th, th->GetName().c_str());
            #endif
            thrusters.push_back(th);
         }
         else
         {
            thrusters.clear();
            throw CommandException(wxT("Thruster object \"") + (*i) +
                                   wxT("\" was not set on Spacecraft \"") 
                                   + (*current)->GetName() + wxT("\""));
         }
      }
   }

   #ifdef DEBUG_BFB_THRUSTER
   MessageInterface::ShowMessage
      (wxT("BeginFiniteBurn::ValidateThrusters() leaving\n"));
   #endif
}

