//$Id: Sandbox.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                 Sandbox
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
// Author: Darrel J. Conway
// Created: 2003/10/08
//
/**
 * Implementation for the GMAT Sandbox class
 */
//------------------------------------------------------------------------------

#include "Sandbox.hpp"
#include "Moderator.hpp"
#include "SandboxException.hpp"
#include "Parameter.hpp"
#include "FiniteThrust.hpp"
#include "GmatFunction.hpp"
#include "CallFunction.hpp"
#include "Assignment.hpp"
#include "BranchCommand.hpp"
#include "SubscriberException.hpp"
#include "CommandUtil.hpp"         // for GetCommandSeqString()
#include "MessageInterface.hpp"

#include <algorithm>       // for find

//#define DISABLE_SOLAR_SYSTEM_CLONING

//#define DISALLOW_NESTED_GMAT_FUNCTIONS

//#define DEBUG_SANDBOX_INIT
//#define DEBUG_MODERATOR_CALLBACK
//#define DEBUG_SANDBOX_GMATFUNCTION
//#define DEBUG_SANDBOX_OBJ_INIT
//#define DEBUG_SANDBOX_OBJ_ADD
//#define DEBUG_SANDBOX_OBJECT_MAPS
//#define DBGLVL_SANDBOX_RUN 1
//#define DEBUG_SANDBOX_CLEAR
//#define DEBUG_SANDBOX_CLONING
//#define DEBUG_SS_CLONING

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

#ifdef DEBUG_SANDBOX_INIT
   std::map<wxString, GmatBase *>::iterator omIter;
#endif


//------------------------------------------------------------------------------
// Sandbox::Sandbox()
//------------------------------------------------------------------------------
/**
 *  Default constructor.
 */
//------------------------------------------------------------------------------
Sandbox::Sandbox() :
   solarSys          (NULL),
   internalCoordSys  (NULL),
   publisher         (NULL),
   sequence          (NULL),
   current           (NULL),
   moderator         (NULL),
   state             (IDLE),
   interruptCount    (45),
   pollFrequency     (50),
   objInit           (NULL)
{
}


//------------------------------------------------------------------------------
// ~Sandbox()
//------------------------------------------------------------------------------
/**
 *  Destructor.
 */
//------------------------------------------------------------------------------
Sandbox::~Sandbox()
{
   #ifndef DISABLE_SOLAR_SYSTEM_CLONING   
      if (solarSys)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (solarSys, solarSys->GetName(), wxT("Sandbox::~Sandbox()"),
             wxT(" deleting cloned solarSys"));
         #endif
         delete solarSys;
      }
   #endif
   
   for (UnsignedInt i = 0; i < triggerManagers.size(); ++i)
      delete triggerManagers[i];

   if (sequence)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (sequence, wxT("sequence"), wxT("Sandbox::~Sandbox()"),
          wxT(" deleting mission sequence"));
      #endif
      delete sequence;
   }
   
   if (objInit)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (objInit, wxT("objInit"), wxT("Sandbox::~Sandbox()"), wxT(" deleting objInit"));
      #endif
      delete objInit;
   }
   
   // Delete the local objects
   Clear();
}


//------------------------------------------------------------------------------
// Setup methods
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// GmatBase* AddObject(GmatBase *obj)
//------------------------------------------------------------------------------
/**
 *  Adds an object to the Sandbox's object container.
 *
 *  Objects are added to the Sandbox by cloning the objects.  That way local
 *  copies can be manipulated without affecting the objects managed by the
 *  ConfigurationManager.
 *
 *  @param <obj> The object that needs to be included in the Sandbox.
 *
 *  @return Cloned object pointer if the object was added to the Sandbox's
 *          container, NULL if it was not.
 */
//------------------------------------------------------------------------------
//Changed to return GmatBase* (loj: 2008.11.06)
GmatBase* Sandbox::AddObject(GmatBase *obj)
{
   if (obj == NULL)
      return NULL;
   
   #ifdef DEBUG_SANDBOX_OBJ_ADD
      MessageInterface::ShowMessage
         (wxT("Sandbox::AddObject() objTypeName=%s, objName=%s\n"),
          obj->GetTypeName().c_str(), obj->GetName().c_str());
   #endif
      
   if ((state != INITIALIZED) && (state != STOPPED) && (state != IDLE))
          MessageInterface::ShowMessage(
             wxT("Unexpected state transition in the Sandbox\n"));

   state = IDLE;
   
   wxString name = obj->GetName();
   if (name == wxT(""))
      return NULL;  // No unnamed objects in the Sandbox tables
   
   GmatBase *cloned = obj;
   
   // Check to see if the object is already in the map
   //if (objectMap.find(name) == objectMap.end())
   if (FindObject(name) == NULL)
   {
      // If not, store the new object pointer
      #ifdef DEBUG_SANDBOX_CLONING
         MessageInterface::ShowMessage(wxT("Cloning %s <%p> -> "),
               obj->GetName().c_str(), obj);
      #endif
         #ifdef DEBUG_SANDBOX_OBJECT_MAPS
         MessageInterface::ShowMessage(
            wxT("Cloning object %s of type %s\n"), obj->GetName().c_str(),
            obj->GetTypeName().c_str());
         #endif
         
         cloned = obj->Clone();
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Add
            (cloned, obj->GetName(), wxT("Sandbox::AddObject()"),
             wxT("*cloned = obj->Clone()"));
         #endif
         SetObjectByNameInMap(name, cloned);
      #ifdef DEBUG_SANDBOX_CLONING
         MessageInterface::ShowMessage(wxT("<%p>\n"), cloned);

         if (cloned->IsOfType(Gmat::PROP_SETUP))
            MessageInterface::ShowMessage(wxT("   PropSetup propagator <%p> -> ")
                  wxT("<%p>\n"), ((PropSetup*)(obj))->GetPropagator(),
                  ((PropSetup*)(cloned))->GetPropagator());
      #endif
   }
   else
   {
      MessageInterface::ShowMessage
         (wxT("in Sandbox::AddObject() %s is already in the map\n"), name.c_str());
   }
   
   return cloned;
}


//------------------------------------------------------------------------------
// bool AddCommand(GmatCommand *cmd)
//------------------------------------------------------------------------------
/**
 *  Adds a command to the Sandbox's command sequence.
 *
 *  Command are added to the command srquence by appending them ti the command
 *  list, using the GmatCommand::Append() method.
 *
 *  @param <cmd> The command that needs to be added to this Sandbox's sequence.
 *
 *  @return true if the command was added to the sequence, false if not.
 */
//------------------------------------------------------------------------------
bool Sandbox::AddCommand(GmatCommand *cmd)
{

   if ((state != INITIALIZED) && (state != STOPPED) && (state != IDLE))
          MessageInterface::ShowMessage(
             wxT("Unexpected state transition in the Sandbox\n"));

  state = IDLE;


   if (!cmd)
      return false;

   if (cmd == sequence)
      return true;

   if (sequence)
      return sequence->Append(cmd);


   sequence = cmd;
   return true;
}


//------------------------------------------------------------------------------
// bool AddSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 *  Sets the SolarSystem for this Sandbox by cloning the input solar system.
 *
 *  @param <ss> The SolarSystem this Sandbox's will use.
 *
 *  @return true if the solar system was added to the Sandbox, false if not.
 */
//------------------------------------------------------------------------------
bool Sandbox::AddSolarSystem(SolarSystem *ss)
{
   if ((state != INITIALIZED) && (state != STOPPED) && (state != IDLE))
          MessageInterface::ShowMessage(
             wxT("Unexpected state transition in the Sandbox\n"));
   state = IDLE;
   
   if (!ss)
      return false;
   
#ifdef DISABLE_SOLAR_SYSTEM_CLONING
   solarSys = ss;
#else
   if (GmatGlobal::Instance()->GetRunMode() == GmatGlobal::TESTING)
      MessageInterface::LogMessage(wxT("Cloning the solar system in the Sandbox\n"));
   
   solarSys = (SolarSystem*)(ss->Clone());
   #ifdef DEBUG_MEMORY
   MemoryTracker::Instance()->Add
      (solarSys, solarSys->GetName(), wxT("Sandbox::AddSolarSystem()"),
       wxT("solarSys = (SolarSystem*)(ss->Clone())"));
   #endif
   
   #ifdef DEBUG_SS_CLONING
   MessageInterface::ShowMessage(wxT("Sandbox cloned the solar system: %p\n"), solarSys);
   #endif
#endif
   return true;
}


//------------------------------------------------------------------------------
// bool AddTriggerManagers(const std::vector<TriggerManager*> *trigs)
//------------------------------------------------------------------------------
/**
 * This method...
 *
 * @param
 *
 * @return
 */
//------------------------------------------------------------------------------
bool Sandbox::AddTriggerManagers(const std::vector<TriggerManager*> *trigs)
{
   bool retval = true;

   for (UnsignedInt i = 0; i < triggerManagers.size(); ++i)
      delete triggerManagers[i];
   triggerManagers.clear();

   #ifdef DEBUG_INITIALIZATION
      MessageInterface::ShowMessage(wxT("Sandbox received %d trigger managers\n"),
            trigs->size());
   #endif
   
   for (UnsignedInt i = 0; i < trigs->size(); ++i)
   {
      TriggerManager *trigMan = (*trigs)[i]->Clone();
      if (trigMan != NULL)
         triggerManagers.push_back(trigMan);
      else
      {
         MessageInterface::ShowMessage(wxT("Unable to clone a TriggerManager -- ")
               wxT("please check the copy constructor and assignment operator"));
         retval = false;
      }
   }

   return retval;
}


//------------------------------------------------------------------------------
// bool SetInternalCoordSystem(CoordinateSystem *cs)
//------------------------------------------------------------------------------
/**
 *  Sets the internal coordinate system used by the Sandbox.
 *
 *  @param <cs> The internal coordinate system.
 *
 *  @return true if the command was added to the sequence, false if not.
 */
//------------------------------------------------------------------------------
bool Sandbox::SetInternalCoordSystem(CoordinateSystem *cs)
{
   if ((state != INITIALIZED) && (state != STOPPED) && (state != IDLE))
          MessageInterface::ShowMessage(
             wxT("Unexpected state transition in the Sandbox\n"));

   state = IDLE;

   if (!cs)
      return false;

   /// @todo Check initialization and cloning for the internal CoordinateSystem.
   //internalCoordSys = (CoordinateSystem*)(cs->Clone());
   internalCoordSys = cs;
   return true;
}


//------------------------------------------------------------------------------
// bool SetPublisher(Publisher *pub)
//------------------------------------------------------------------------------
/**
 *  Sets the Publisher so the Sandbox can pipe data to the rest of GMAT.
 *
 *  @param <pub> The GMAT Publisher.
 *
 *  @return true if the command was added to the sequence, false if not.
 */
//------------------------------------------------------------------------------
bool Sandbox::SetPublisher(Publisher *pub)
{
   if ((state != INITIALIZED) && (state != STOPPED) && (state != IDLE))
          MessageInterface::ShowMessage(
             wxT("Unexpected state transition in the Sandbox\n"));
   state = IDLE;
   
   if (pub)
   {
      publisher = pub;
      // Now publisher needs internal coordinate system
      publisher->SetInternalCoordSystem(internalCoordSys);
      return true;
   }
   
   if (!publisher)
      return false;
   
   return true;
}


//------------------------------------------------------------------------------
// GmatBase* GetInternalObject(wxString name, Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 *  Accesses objects managed by this Sandbox.
 *
 *  @param <name> The name of the object.
 *  @param <name> type of object requested.
 *
 *  @return The pointer to the object.
 */
//------------------------------------------------------------------------------
GmatBase* Sandbox::GetInternalObject(wxString name, Gmat::ObjectType type)
{
   #ifdef DEBUG_INTERNAL_OBJ
   MessageInterface::ShowMessage
      (wxT("Sandbox::GetInternalObject() name=%s, type=%d\n"), name.c_str(), type);
   #endif
   
   GmatBase* obj = NULL;
   
   if ((obj = FindObject(name)) != NULL) 
   {
      if (type != Gmat::UNKNOWN_OBJECT)
      {
         if (obj->GetType() != type) 
         {
            wxString errorStr = wxT("GetInternalObject type mismatch for ");
            errorStr += name;
            throw SandboxException(errorStr);
         }
      }
   }
   else 
   {
      wxString errorStr = wxT("Sandbox::GetInternalObject(") + name +
                             wxT("...) Could not find \"");
      errorStr += name;
      errorStr += wxT("\" in the Sandbox.");
      
      #ifdef DEBUG_SANDBOX_OBJECT_MAPS
         MessageInterface::ShowMessage(wxT("Here is the current object map:\n"));
         for (std::map<wxString, GmatBase *>::iterator i = objectMap.begin();
              i != objectMap.end(); ++i)
            MessageInterface::ShowMessage(wxT("   %s\n"), i->first.c_str());
         MessageInterface::ShowMessage(wxT("Here is the current global object map:\n"));
         for (std::map<wxString, GmatBase *>::iterator i = globalObjectMap.begin();
              i != globalObjectMap.end(); ++i)
            MessageInterface::ShowMessage(wxT("   %s\n"), i->first.c_str());
      #endif
      
      throw SandboxException(errorStr);
   }

   return obj;
}


//------------------------------------------------------------------------------
// Execution methods
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 *  Established the internal linkages between objects needed prior to running a
 *  mission sequence.
 *
 *  @return true if everything was connected properly, false if not.
 */
//------------------------------------------------------------------------------
bool Sandbox::Initialize()
{
   #ifdef DEBUG_SANDBOX_INIT
      MessageInterface::ShowMessage(wxT("Initializing the Sandbox\n"));
      MessageInterface::ShowMessage(wxT("At the start, the Sandbox Object Map contains:\n"));
      for (omIter = objectMap.begin(); omIter != objectMap.end(); ++omIter)
         MessageInterface::ShowMessage(wxT("   %s of type %s\n"),
               (omIter->first).c_str(), ((omIter->second)->GetTypeName()).c_str());
      MessageInterface::ShowMessage(wxT("At the start, the Global Object Map contains:\n"));
      for (omIter = globalObjectMap.begin(); omIter != globalObjectMap.end(); ++omIter)
         MessageInterface::ShowMessage(wxT("   %s of type %s\n"),
               (omIter->first).c_str(), ((omIter->second)->GetTypeName()).c_str());
      MessageInterface::ShowMessage(wxT(" ........ \n"));
   #endif

   bool rv = false;


   if (moderator == NULL)
      moderator = Moderator::Instance();
   
   // this should be clear() (loj: 2008.11.03)
   //transientForces.empty();
   transientForces.clear();
   
   
   // Already initialized
   if (state == INITIALIZED)
      return true;


   current = sequence;
   if (!current)
      throw SandboxException(wxT("No mission sequence defined in the Sandbox!"));


   if (!internalCoordSys)
      throw SandboxException(
         wxT("No reference (internal) coordinate system defined in the Sandbox!"));


   std::map<wxString, GmatBase *>::iterator omi;
   GmatBase *obj = NULL;
   wxString oName;
   wxString j2kName;


   // Set the solar system references
   if (solarSys == NULL)
      throw SandboxException(wxT("No solar system defined in the Sandbox!"));
   
   // Initialize the solar system, internal coord system, etc.

   // Set J2000 Body for all SpacePoint derivatives before anything else
   // NOTE - at this point, everything should be in the SandboxObjectMap,
   // and the GlobalObjectMap should be empty
   #ifdef DEBUG_SANDBOX_OBJ_INIT
      MessageInterface::ShowMessage(wxT("About to create the ObjectInitializer ... \n"));
      MessageInterface::ShowMessage(wxT(" and the objInit pointer is %s\n"),
            (objInit? wxT("NOT NULL") : wxT("NULL!!!")));
   #endif

   if (objInit)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (objInit, wxT("objInit", "Sandbox::Initialize()", " deleting objInit"));
      #endif
      delete objInit;  // if Initialize is called more than once, delete 'old' objInit
   }
   
   objInit = new ObjectInitializer(solarSys, &objectMap, &globalObjectMap, internalCoordSys);
   
   #ifdef DEBUG_MEMORY
   MemoryTracker::Instance()->Add
      (objInit, wxT("objInit"), wxT("Sandbox::Initialize()"), wxT("objInit = new ObjectInitializer"));
   #endif
   try
   {
      #ifdef DEBUG_SANDBOX_OBJ_INIT
         MessageInterface::ShowMessage(
               wxT("About to call the ObjectInitializer::InitializeObjects ... \n"));
      #endif
      objInit->InitializeObjects();
   }
   catch (BaseException &be)
   {
      SandboxException se(wxT(""));
      se.SetDetails(wxT("Error initializing objects in Sandbox.\n%s\n"),
                    be.GetFullMessage().c_str());
      throw se;
      //throw SandboxException("Error initializing objects in Sandbox");
   }
   
   // Move global objects to the Global Object Store
   combinedObjectMap = objectMap;
   StringArray movedObjects;
   for (omi = objectMap.begin(); omi != objectMap.end(); ++omi)
   {
      obj = omi->second;
      #ifdef DEBUG_SANDBOX_INIT
         MessageInterface::ShowMessage(
            wxT("Sandbox::checking object %s (of type %s) \n"),
            (omi->first).c_str(), (obj->GetTypeName()).c_str());
      #endif
      // Check the isGlobal flag
      if (obj->GetIsGlobal())
      {
         #ifdef DEBUG_SANDBOX_INIT
            MessageInterface::ShowMessage(
               wxT("Sandbox::moving object <%p>'%s' to the Global Object Store\n"),
               omi->second, (omi->first).c_str());
         #endif
         globalObjectMap.insert(*omi);
         movedObjects.push_back(omi->first);
      }
   }
   for (unsigned int ii = 0; ii < movedObjects.size(); ii++)
      objectMap.erase(movedObjects.at(ii));
   movedObjects.clear();  
   
   #ifdef DEBUG_SANDBOX_INIT
      MessageInterface::ShowMessage(wxT("--- Right AFTER moving things to the GOS --- \n"));
      MessageInterface::ShowMessage(wxT("The Sandbox Object Map contains:\n"));
      for (omIter = objectMap.begin(); omIter != objectMap.end(); ++omIter)
         MessageInterface::ShowMessage(wxT("   %s of type %s\n"),
               (omIter->first).c_str(), ((omIter->second)->GetTypeName()).c_str());
      MessageInterface::ShowMessage(wxT("The Global Object Map contains:\n"));
      for (omIter = globalObjectMap.begin(); omIter != globalObjectMap.end(); ++omIter)
         MessageInterface::ShowMessage(wxT("   %s of type %s\n"),
               (omIter->first).c_str(), ((omIter->second)->GetTypeName()).c_str());
   #endif
   
   #ifdef DEBUG_SANDBOX_INIT
      MessageInterface::ShowMessage(
         wxT("Sandbox::Initialize() Initializing Commands...\n"));
   #endif
   
   StringArray exceptions;
   IntegerArray exceptionTypes;
   UnsignedInt exceptionCount = 0;

   // Initialize commands
   while (current)
   {
      try
      {
         #ifdef DEBUG_SANDBOX_INIT
         MessageInterface::ShowMessage
            (wxT("Initializing %s command\n   \"%s\"\n"),
             current->GetTypeName().c_str(),
             current->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
         #endif

         current->SetTriggerManagers(&triggerManagers);

         #ifdef DEBUG_SANDBOX_GMATFUNCTION
            MessageInterface::ShowMessage(wxT("Sandbox Initializing %s command\n"),
               current->GetTypeName().c_str());
         #endif

         current->SetObjectMap(&objectMap);
         current->SetGlobalObjectMap(&globalObjectMap);
         SetGlobalRefObject(current);

         // Handle GmatFunctions
         if ((current->IsOfType(wxT("CallFunction"))) ||
             (current->IsOfType(wxT("Assignment"))))
         {
            #ifdef DEBUG_SANDBOX_GMATFUNCTION
               MessageInterface::ShowMessage(
                  wxT("CallFunction or Assignment found in MCS: calling HandleGmatFunction \n"));
            #endif
            HandleGmatFunction(current, &combinedObjectMap);
         }
         if (current->IsOfType(wxT("BranchCommand")))
         {
            std::vector<GmatCommand*> cmdList = ((BranchCommand*) current)->GetCommandsWithGmatFunctions();
            Integer sz = (Integer) cmdList.size();
            #ifdef DEBUG_SANDBOX_GMATFUNCTION
               MessageInterface::ShowMessage(wxT("... returning %d functions with GmatFunctions\n"), sz);
            #endif
            for (Integer jj = 0; jj < sz; jj++)
            {
               HandleGmatFunction(cmdList.at(jj), &combinedObjectMap);
               (cmdList.at(jj))->SetInternalCoordSystem(internalCoordSys);
            }
         }

         try
         {
            rv = current->Initialize();
            if (!rv)
               throw SandboxException(wxT("The Mission Control Sequence command\n\n") +
                     current->GetGeneratingString(Gmat::SCRIPTING, wxT("   ")) +
                     wxT("\n\nfailed to initialize correctly.  Please correct the error ")
                     wxT("and try again."));
         }
         catch (BaseException &)
         {
            // Call ValidateCommand to create wrappers and Initialize.(LOJ: 2010.08.24)
            // This will fix bug 1918 for the following scenario in ScriptEvent.
            // In ScriptEvent, x = 1 where x is undefined, save it.
            // Add x from the ResourceTree and run the mission.
            moderator->ValidateCommand(current);
            rv = current->Initialize();
         }

         // Check to see if the command needs a server startup
         if (current->NeedsServerStartup())
            if (moderator->StartMatlabServer() == false)
               throw SandboxException(wxT("Unable to start the server needed by the ") +
                        (current->GetTypeName()) + wxT(" command"));
      }
      catch (BaseException &be)
      {
         ++exceptionCount;
         exceptionTypes.push_back(be.GetMessageType());
         exceptions.push_back(be.GetFullMessage());
      }
      current = current->GetNext();
   }
   
   if (exceptionCount > 0)
   {
      for (UnsignedInt i = 0; i < exceptionCount; ++i)
      {
         // Add error count only if message type is Gmat::ERROR_ (Bug 2272 fix)
         if (exceptionTypes[i] == Gmat::ERROR_)
             MessageInterface::ShowMessage(wxT("%d: %s\n"), i+1, exceptions[i].c_str());
         else
            MessageInterface::ShowMessage(wxT("%s\n"), exceptions[i].c_str());
      }
      throw SandboxException(wxT("Errors were found in the mission control ")
            wxT("sequence; please correct the errors listed in the message window"));
   }

   #ifdef DEBUG_SANDBOX_INIT
      MessageInterface::ShowMessage(
         wxT("Sandbox::Initialize() Successfully initialized\n"));
   #endif

   state = INITIALIZED;
   
   //MessageInterface::ShowMessage(wxT("=====> Initialize successful\n"));
   return rv;
}


//------------------------------------------------------------------------------
// bool Execute()
//------------------------------------------------------------------------------
/**
 *  Runs the mission sequence.
 *
 *  This method walks through the command linked list, firing each GmatCommand
 *  as it is encountered by calling Execute() on the commands.  Between command
 *  executions, the method check with the Moderator to see if the user has
 *  requested that the sequence be paused or halted.
 *
 *  @return true if the mission sequence was executed, false if not.
 */
//------------------------------------------------------------------------------
bool Sandbox::Execute()
{

   #if DBGLVL_SANDBOX_RUN > 1
   MessageInterface::ShowMessage(wxT("Sandbox::Execute() Here is the current object map:\n"));
   for (std::map<wxString, GmatBase *>::iterator i = objectMap.begin();
        i != objectMap.end(); ++i)
      MessageInterface::ShowMessage(wxT("   (%p) %s\n"), i->second, i->first.c_str());
   MessageInterface::ShowMessage(wxT("Sandbox::Execute() Here is the current global object map:\n"));
   for (std::map<wxString, GmatBase *>::iterator i = globalObjectMap.begin();
        i != globalObjectMap.end(); ++i)
      MessageInterface::ShowMessage(wxT("   (%p) %s\n"), i->second, i->first.c_str());

   MessageInterface::ShowMessage(wxT("Sandbox::Execute() Here is the mission sequence:\n"));
   wxString seq = GmatCommandUtil::GetCommandSeqString(sequence);
   MessageInterface::ShowMessage(seq);
   #endif
   
   bool rv = true;

   state = RUNNING;
   Gmat::RunState runState = Gmat::IDLE, currentState = Gmat::RUNNING;
   GmatCommand *prev = NULL;
   
   current = sequence;
   if (!current)
      return false;

   try
   {
      while (current)
      {
         // First check to see if the run should be interrupted
         if (Interrupt())
         {
            #ifdef DEBUG_MODERATOR_CALLBACK
            MessageInterface::ShowMessage(wxT("   Interrupted in %s command\n"),
                                          current->GetTypeName().c_str());
            #endif
            
            
            if (state == PAUSED)
            {
               continue;
            }
            else
            {
               //MessageInterface::ShowMessage(wxT("Sandbox::Execution interrupted.\n"));
               sequence->RunComplete();
               
               // notify subscribers end of run
               currentState = Gmat::IDLE;
               publisher->SetRunState(currentState);
               publisher->NotifyEndOfRun();
               
               throw SandboxException(wxT("Execution interrupted"));
               //return rv;
            }
         }
         
         #if DBGLVL_SANDBOX_RUN
         if (current != prev)
         {
            MessageInterface::ShowMessage
               (wxT("Sandbox::Execution running %s\n"), current->GetTypeName().c_str());
            
            #if DBGLVL_SANDBOX_RUN > 1
            MessageInterface::ShowMessage
               (wxT("command = \n<%s>\n"), current->GetGeneratingString().c_str());
            #endif
         }
         #endif
         
         if (currentState != runState)
         {
            publisher->SetRunState(currentState);
            runState = currentState;
         }
         
         rv = current->Execute();
         
         if (!rv)
         {
            wxString str = wxT("\"") + current->GetTypeName() +
               wxT("\" Command failed to run to completion\n");
            
            #if DBGLVL_SANDBOX_RUN > 1
            MessageInterface::ShowMessage
               (wxT("%sCommand Text is\n\"%s\n"), str.c_str(),
                current->GetGeneratingString().c_str());
            #endif
            
            throw SandboxException(str);
         }
         
         prev = current;
         current = current->GetNext();
      }
   }
   catch (BaseException &e)
   {
      // Use exception to remove Visual C++ warning
      e.GetMessageType();
      sequence->RunComplete();
      state = STOPPED;
      
      #if DBGLVL_SANDBOX_RUN
      MessageInterface::ShowMessage
         (wxT("   Sandbox rethrowing %s\n"), e.GetFullMessage().c_str());
      #endif
      
      throw;
   }
   
   sequence->RunComplete();
   state = STOPPED;
   
   // notify subscribers end of run
   currentState = Gmat::IDLE;
   publisher->SetRunState(currentState);
   publisher->NotifyEndOfRun();
   
   return rv;
}



//------------------------------------------------------------------------------
// bool Interrupt()
//------------------------------------------------------------------------------
/**
 *  Tests to see if the mission sequence should be interrupted.
 *
 *  @return true if the Moderator wants to interrupt execution, false if not.
 */
//------------------------------------------------------------------------------
bool Sandbox::Interrupt()
{
   // Ask the moderator for the current RunState; only check at fixed frequency
   if (++interruptCount == pollFrequency)
   {
      Gmat::RunState interruptType =  moderator->GetUserInterrupt();
   
      switch (interruptType)
      {
         case Gmat::PAUSED:   // Pause
            state = PAUSED;
            break;
   
         case Gmat::IDLE:     // Stop puts GMAT into the Idle state
            state = STOPPED;
            break;
   
         case Gmat::RUNNING:   // MCS is running
            state = RUNNING;
            break;
   
         default:
            break;
      }
      interruptCount = 0;
   }
   
   if ((state == PAUSED) || (state == STOPPED))
      return true;

   return false;
}


//------------------------------------------------------------------------------
// void Clear()
//------------------------------------------------------------------------------
/**
 *  Cleans up the local object store.
 */
//------------------------------------------------------------------------------
void Sandbox::Clear()
{
   #ifdef DEBUG_SANDBOX_CLEAR
   MessageInterface::ShowMessage(wxT("Sandbox::Clear() entered\n"));
   #endif
   
   sequence  = NULL;
   current   = NULL;
   
   // Delete the all cloned objects
   std::map<wxString, GmatBase *>::iterator omi;
   
   #ifdef DEBUG_SANDBOX_OBJECT_MAPS
   MessageInterface::ShowMessage(wxT("Sandbox OMI List\n"));
   for (omi = objectMap.begin(); omi != objectMap.end(); omi++)
   {
      MessageInterface::ShowMessage(wxT("   %s"), (omi->first).c_str());
      MessageInterface::ShowMessage(wxT(" of type %s\n"),
         (omi->second)->GetTypeName().c_str());
   }
   MessageInterface::ShowMessage(wxT("Sandbox GOMI List\n"));
   for (omi = globalObjectMap.begin(); omi != globalObjectMap.end(); omi++)
   {
      MessageInterface::ShowMessage(wxT("   %s"), (omi->first).c_str());
      MessageInterface::ShowMessage(wxT(" of type %s\n"),
         (omi->second)->GetTypeName().c_str());
   }
   #endif
   
   #ifdef DEBUG_SANDBOX_CLEAR
   ShowObjectMap(objectMap, wxT("Sandbox::Clear() clearing objectMap\n"));
   #endif
   
   for (omi = objectMap.begin(); omi != objectMap.end(); omi++)
   {
      #ifdef DEBUG_SANDBOX_OBJECT_MAPS
         MessageInterface::ShowMessage(wxT("Sandbox clearing <%p>'%s'\n"), omi->second,
            (omi->first).c_str());
      #endif

      // if object is a SUBSCRIBER, let's unsubscribe it first
      if ((omi->second != NULL) && (omi->second)->GetType() == Gmat::SUBSCRIBER)
         publisher->Unsubscribe((Subscriber*)(omi->second));
      
      #ifdef DEBUG_SANDBOX_OBJECT_MAPS
         MessageInterface::ShowMessage(wxT("   Deleting <%p>'%s'\n"), omi->second,
            (omi->second)->GetName().c_str());
      #endif
      #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (omi->second, omi->first, wxT("Sandbox::Clear()"),
             wxT(" deleting cloned obj from objectMap"));
      #endif
      delete omi->second;
      omi->second = NULL;
      //objectMap.erase(omi);
   }
   #ifdef DEBUG_SANDBOX_CLEAR
   MessageInterface::ShowMessage
      (wxT("--- Sandbox::Clear() deleting objects from objectMap done\n"));
   ShowObjectMap(globalObjectMap, wxT("Sandbox::Clear() clearing globalObjectMap\n"));
   #endif
   for (omi = globalObjectMap.begin(); omi != globalObjectMap.end(); omi++)
   {
      #ifdef DEBUG_SANDBOX_OBJECT_MAPS
         MessageInterface::ShowMessage(wxT("Sandbox clearing <%p>%s\n"), omi->second,
            (omi->first).c_str());
      #endif

      // if object is a SUBSCRIBER, let's unsubscribe it first
      if ((omi->second != NULL) && (omi->second)->GetType() == Gmat::SUBSCRIBER)
         publisher->Unsubscribe((Subscriber*)(omi->second));
      
      #ifdef DEBUG_SANDBOX_OBJECT_MAPS
         MessageInterface::ShowMessage(wxT("   Deleting <%p>'%s'\n"), omi->second,
            (omi->second)->GetName().c_str());
      #endif
      #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (omi->second, omi->first, wxT("Sandbox::Clear()"),
             wxT(" deleting cloned obj from globalObjectMap"));
      #endif
      delete omi->second;
      omi->second = NULL;
      //globalObjectMap.erase(omi);
   }
   
   #ifdef DEBUG_SANDBOX_CLEAR
   MessageInterface::ShowMessage
      (wxT("--- Sandbox::Clear() deleting objects from globalObjectMap done\n"));
   #endif
   
   // Clear published data
   if (publisher)
   {
      publisher->ClearPublishedData();
      #ifdef DEBUG_SANDBOX_CLEAR
      MessageInterface::ShowMessage
         (wxT("--- Sandbox::Clear() publisher cleared published data\n"));
      #endif
   }
   
   // Set publisher to NULL. The publisher is set before the run and this method
   // Sandbox::Clear() can be called multiple times from the Moderator
   publisher = NULL;
   
#ifndef DISABLE_SOLAR_SYSTEM_CLONING
   if (solarSys != NULL)
   {
      #ifdef DEBUG_SS_CLONING
      MessageInterface::ShowMessage
         (wxT("--- Sandbox::Clear() deleting the solar system clone: %p\n"), solarSys);
      #endif
      
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (solarSys, solarSys->GetName(), wxT("Sandbox::Clear()", " deleting solarSys"));
      #endif
      delete solarSys;
   }
   
   solarSys = NULL;
#endif
   
   #ifdef DEBUG_SANDBOX_CLEAR
   MessageInterface::ShowMessage
      (wxT("--- Sandbox::Clear() now about to delete triggerManagers\n"));
   #endif
   // Remove the TriggerManager clones
   for (UnsignedInt i = 0; i < triggerManagers.size(); ++i)
      delete triggerManagers[i];
   triggerManagers.clear();
   
   #ifdef DEBUG_SANDBOX_CLEAR
   MessageInterface::ShowMessage
      (wxT("--- Sandbox::Clear() triggerManagers are cleared\n"));
   #endif
   
   // who deletes objects?  ConfigManager::RemoveAllItems() deleletes them
   objectMap.clear();
   globalObjectMap.clear();
   
   #ifdef DEBUG_TRANSIENT_FORCES
   MessageInterface::ShowMessage
      (wxT("Sandbox::Clear() transientForces<%p> has %d transient forces\n"),
       &transientForces, transientForces.size());
   #endif
   // Who pushes forces to transientForces?
   //    BeginFiniteBurn::Execute() pushes burn force
   // Should we delete transient forces here? (loj: 2008.11.03)
   // @note transient forces are deleted in the BeginFiniteBurn destructor
   // so we don't need to delete here. (LOJ: 2009.05.08)
   #if 0
   for (std::vector<PhysicalModel*>::iterator tf = transientForces.begin();
        tf != transientForces.end(); ++tf)
   {
      #ifdef DEBUG_TRANSIENT_FORCES
      MessageInterface::ShowMessage(wxT("   tf=<%p>\n"), (*tf));
      #endif
      
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         ((*tf), (*tf)->GetName(), wxT("Sandbox::Clear()", "deleting transient force"));
      #endif
      delete (*tf);
   }
   #endif
   
   transientForces.clear();
   
   // Update the sandbox state
   if ((state != STOPPED) && (state != IDLE))
          MessageInterface::ShowMessage(
             wxT("Unexpected state transition in the Sandbox\n"));

   state     = IDLE;
   
   #ifdef DEBUG_SANDBOX_CLEAR
   MessageInterface::ShowMessage(wxT("Sandbox::Clear() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// bool AddSubscriber(Subscriber *sub)
//------------------------------------------------------------------------------
/**
 *  Add Subcribers to the Sandbox and registers them with the Publisher.
 *
 *  @param <sub> The subscriber.
 *
 *  @return true if the Subscriber was registered.
 */
//------------------------------------------------------------------------------
bool Sandbox::AddSubscriber(Subscriber *sub)
{
   // add subscriber to sandbox by AddObject() so that cloned subscribers
   // can be deleted when clear (loj: 2008.11.06)
   Subscriber *newSub = (Subscriber*)AddObject(sub);
   if (newSub != NULL)
   {
      publisher->Subscribe(newSub);
      return true;
   }
   
   return false;
}


//------------------------------------------------------------------------------
// GmatBase* Sandbox::FindObject(const wxString &name)
//------------------------------------------------------------------------------
/**
 *  Finds an object by name, searching through the SandboxObjectMap first,
 *  then the GlobalObjectMap
 *
 *  @param <spname> The name of the SpacePoint.
 *
 *  @return A pointer to the SpacePoint, or NULL if it does not exist in the
 *          Sandbox.
 */
//------------------------------------------------------------------------------
GmatBase* Sandbox::FindObject(const wxString &name)
{
   if (objectMap.find(name) == objectMap.end())
   {
     // If not found in the LOS, check the Global Object Store (GOS)
      if (globalObjectMap.find(name) == globalObjectMap.end())
         return NULL;
      else return globalObjectMap[name];
   }
   else
      return objectMap[name];
}

//------------------------------------------------------------------------------
// bool Sandbox::SetObjectByNameInMap(const wxString &name,
//                                    GmatBase *obj)
//------------------------------------------------------------------------------
/**
 *  Sets the object pointer for the given name in the object map(s).  NOTE that
 *  an object should only exist in one of the object maps, so both IFs should
 *  not evaluate to TRUE.
 *
 *  @param <name> The name of the object.
 *  @param <obj>  The object pointer.
 *
 *  @return true if successful; flase otherwise
 */
//------------------------------------------------------------------------------
bool Sandbox::SetObjectByNameInMap(const wxString &name,
                                   GmatBase *obj)
{
   #ifdef DEBUG_SANDBOX_OBJECT_MAPS
   MessageInterface::ShowMessage
      (wxT("Sandbox::SetObjectByNameInMap() name = %s\n"),
       name.c_str());
   #endif
   bool found = false;
   // if it's already in a map, set the object pointer for the name
   if (objectMap.find(name) != objectMap.end())
   {
      objectMap[name] = obj;
      #ifdef DEBUG_SANDBOX_OBJECT_MAPS
      MessageInterface::ShowMessage
         (wxT("Sandbox::SetObjectByNameInMap() set object name = %s in objectMap\n"),
          name.c_str());
      #endif
      found = true;
   }
   if (globalObjectMap.find(name) != globalObjectMap.end())
   {
      globalObjectMap[name] = obj;
      #ifdef DEBUG_SANDBOX_OBJECT_MAPS
      MessageInterface::ShowMessage
         (wxT("Sandbox::SetObjectByNameInMap() set object name = %s in globalObjectMap\n"),
          name.c_str());
      #endif      
      found = true;
   }
   // if not already in the map, add it to the objectMap
   // (globals are added to the globalObjectMap later)
   if (!found)
      objectMap.insert(std::make_pair(name,obj));
   
   #ifdef DEBUG_SANDBOX_OBJECT_MAPS
   MessageInterface::ShowMessage
      (wxT("Sandbox::SetObjectByNameInMap() returning found = %s\n"),
       (found? wxT("TRUE") : wxT("FALSE")));
   #endif   
   return found;
}

//------------------------------------------------------------------------------
// bool Sandbox::HandleGmatFunction(GmatCommand *cmd,
//                                  std::map<wxString, GmatBase *> *usingMap)
//------------------------------------------------------------------------------
/**
 *  Handles any GmatFunctions included in the sequence.  The input cmd is the 
 *  CallFunction or Assignment command to process - it may itself contain a nested
 *  GmatFunction.  If it does, this method willbe called recursively to process
 *  the nested GmatFunctions.
 *
 *  @param <name>     The cmd.
 *  @param <usingMap> The map to send to the Interpreter (via the Moderator).
 *
 *  @return true if successful; flase otherwise
 */
//------------------------------------------------------------------------------
bool Sandbox::HandleGmatFunction(GmatCommand *cmd, std::map<wxString,
                                 GmatBase *> *usingMap)
{
   #ifdef DEBUG_SANDBOX_GMATFUNCTION
      MessageInterface::ShowMessage(
         wxT("Now entering HandleGmatFunction with command of type %s, '%s'\n"),
         (cmd->GetTypeName()).c_str(), cmd->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
   #endif
      
   bool OK = false;
   GmatGlobal *global = GmatGlobal::Instance();
   wxString matlabExt = global->GetMatlabFuncNameExt();
   StringArray gfList;
   bool        isMatlabFunction = false;
   
   SetGlobalRefObject(cmd);
   
   if (cmd->IsOfType(wxT("CallFunction")))
   {
      wxString cfName = cmd->GetStringParameter(wxT("FunctionName"));
      gfList.push_back(cfName);
   }
   else if (cmd->IsOfType(wxT("Assignment")))  
   {
      gfList = ((Assignment*) cmd)->GetGmatFunctionNames();
   }
   
   #ifdef DEBUG_SANDBOX_GMATFUNCTION
   MessageInterface::ShowMessage(wxT("   Has %d GmatFunctions\n"), gfList.size());
   #endif
   
   for (unsigned int ii = 0; ii < gfList.size(); ii++)
   {
      wxString fName = gfList.at(ii);
      Function *f;
      isMatlabFunction = false;
      // if it's a Matlab Function, remove the extension from the name before looking in the GOS
      // (as Matlab function names are placed into the GOS without the extension)
      if (fName.find(matlabExt) != fName.npos)
      {
         fName = fName.substr(0, fName.find(matlabExt));
         #ifdef DEBUG_SANDBOX_GMATFUNCTION
         MessageInterface::ShowMessage
            (wxT("   actual matlab function name='%s'\n"), fName.c_str());
         #endif
         isMatlabFunction = true;
      }
      #ifdef DEBUG_SANDBOX_GMATFUNCTION
         MessageInterface::ShowMessage(wxT("Now searching GOS for object %s\n"),
            (gfList.at(ii)).c_str());
      #endif
      // if there is not already a function of that name, create it
      if (globalObjectMap.find(fName) == globalObjectMap.end())
      {
         if (isMatlabFunction)
            f = moderator->CreateFunction(wxT("MatlabFunction"),fName, 0);
         else
            f = moderator->CreateFunction(wxT("GmatFunction"),fName, 0);
         if (!f) 
            throw SandboxException(wxT("Sandbox::HandleGmatFunction - error creating new function\n"));
         #ifdef DEBUG_SANDBOX_GMATFUNCTION
         MessageInterface::ShowMessage
            (wxT("Adding function <%p>'%s' to the Global Object Store\n"), f, fName.c_str());
         #endif
         globalObjectMap.insert(std::make_pair(fName,f));
      }
      else // it's already in the GOS, so just grab it
         f = (Function*) globalObjectMap[fName];
      
      if (cmd->IsOfType(wxT("CallFunction")))
      {
         ((CallFunction*)cmd)->SetRefObject(f,Gmat::FUNCTION,fName);
         cmd->SetStringParameter(wxT("FunctionName"), fName);
      }
      else if (cmd->IsOfType(wxT("Assignment")))
         ((Assignment*) cmd)->SetFunction(f);
      
      #ifdef DEBUG_SANDBOX_GMATFUNCTION
      MessageInterface::ShowMessage(
         wxT("Now handling function \"%s\", whose fcs is %s set, "),
         (f->GetStringParameter(wxT("FunctionName"))).c_str(), 
         ((f->IsFunctionControlSequenceSet())? wxT("already") : wxT("NOT")));
      MessageInterface::ShowMessage
         (wxT("script errors were %sfound.\n", f->ScriptErrorFound() ? "" : "not "));
      #endif
      
      // if function is GmatFunction and no FCS has built and no script error found,
      // build FCS
      if ((f->GetTypeName() == wxT("GmatFunction")) &&
          (!(f->IsFunctionControlSequenceSet())) &&
          (!(f->ScriptErrorFound())))
      {
         #ifdef DEBUG_SANDBOX_GMATFUNCTION
         MessageInterface::ShowMessage(
            wxT("About to call InterpretGmatFunction for function %s\n"),
            (f->GetStringParameter(wxT("FunctionName"))).c_str());
         #endif
         GmatCommand* fcs = moderator->InterpretGmatFunction(f, usingMap, solarSys);

         // If FCS not created, throw an exception with Gmat::GENERAL_ so that it will not
         // write error count again for function in Initialize()(Bug 2272 fix)
         if (fcs == NULL)
            throw SandboxException(wxT("Sandbox::HandleGmatFunction - error creating FCS\n"),
                                   Gmat::GENERAL_);
         
         f->SetFunctionControlSequence(fcs);
         GmatCommand* fcsCmd = fcs;
         while (fcsCmd)
         {
            #ifdef DISALLOW_NESTED_GMAT_FUNCTIONS
            if (fcsCmd->HasAFunction())
            {
               wxString errMsg = wxT("Sandbox::HandleGmatFunction (");
               errMsg += fName + wxT(") - nested or recursive GmatFunctions not yet supported.\n");
               throw SandboxException(errMsg);
            }
            #endif
            
            if ((fcsCmd->IsOfType(wxT("CallFunction"))) ||
                (fcsCmd->IsOfType(wxT("Assignment"))))
            {
               #ifdef DEBUG_SANDBOX_GMATFUNCTION
               MessageInterface::ShowMessage(
                  wxT("CallFunction or Assignment (%s)'%s' detected in FCS... now processing\n"),
                  (fcsCmd->GetTypeName()).c_str(),
                  fcsCmd->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
               #endif
               
               //Let's handle GmatFunction first
               //compiler warning: '+=' : unsafe use of type 'bool' in operation
               //OK += HandleGmatFunction(fcsCmd, &combinedObjectMap);
               OK = HandleGmatFunction(fcsCmd, &combinedObjectMap) && OK;
               // do not set the non-global object map here; it will need to be
               // setup by the FunctionManager at execution
               fcsCmd->SetGlobalObjectMap(&globalObjectMap);
            }
            if (fcsCmd->IsOfType(wxT("BranchCommand")))
            {
               std::vector<GmatCommand*> cmdList =
                  ((BranchCommand*) fcsCmd)->GetCommandsWithGmatFunctions();
               Integer sz = (Integer) cmdList.size();
               for (Integer jj = 0; jj < sz; jj++)
               {
                  GmatCommand *childCmd = cmdList.at(jj);
                  HandleGmatFunction(childCmd, &combinedObjectMap);
               }
            }
            fcsCmd = fcsCmd->GetNext();
         }
      }
   }
   return OK;
}


//------------------------------------------------------------------------------
// void SetGlobalRefObject(GmatCommand *cmd)
//------------------------------------------------------------------------------
/*
 * Sets globally used object pointers to command
 *
 * @param cmd The command to set global object pointers to
 */
//------------------------------------------------------------------------------
void Sandbox::SetGlobalRefObject(GmatCommand *cmd)
{
   #ifdef DEBUG_SANDBOX_GLOBAL_REF_OBJ
   MessageInterface::ShowMessage
      (wxT("Sandbox::SetGlobalRefObject() Setting solarSystem <%p>, transientForces <%p>\n   ")
       wxT("internalCoordSystem <%p>, publisher <%p>, to <%p>'%s'\n"), solarSys,
       &transientForces, internalCoordSys, publisher, cmd, cmd->GetTypeName().c_str());
   #endif
   cmd->SetSolarSystem(solarSys);
   cmd->SetTransientForces(&transientForces);
   cmd->SetInternalCoordSystem(internalCoordSys);
   cmd->SetPublisher(publisher);
}


//------------------------------------------------------------------------------
// void ShowObjectMap(ObjectMap &om, const wxString &title)
//------------------------------------------------------------------------------
void Sandbox::ShowObjectMap(ObjectMap &om, const wxString &title)
{   
   MessageInterface::ShowMessage(title);
   MessageInterface::ShowMessage(wxT("object map = <%p>\n"), &om);
   if (om.size() > 0)
   {
      for (ObjectMap::iterator i = om.begin(); i != om.end(); ++i)
      MessageInterface::ShowMessage
         (wxT("   %30s  <%p><%s>\n"), i->first.c_str(), i->second,
          i->second == NULL ? wxT("NULL") : (i->second)->GetTypeName().c_str());
   }
   else
   {
      MessageInterface::ShowMessage(wxT("The object map is empty\n"));
   }
}
