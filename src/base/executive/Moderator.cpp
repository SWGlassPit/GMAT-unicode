//$Id: Moderator.cpp 10024 2011-11-23 19:51:49Z djcinsb $
//------------------------------------------------------------------------------
//                                 Moderator
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Developed further jointly by NASA/GSFC, Thinking Systems, Inc., and 
// Schafer Corp., under AFRL NOVA Contract #FA945104D03990003
//
// Author: Linda Jun
// Created: 2003/08/25
// Modified:  Dunn Idle (added MRPs)
// Date:      2010/08/24
//
/**
 * Implements operations of the GMAT executive.  It is a singleton class -
 * only one instance of this class can be created.
 */
//------------------------------------------------------------------------------

#include "Moderator.hpp"

// factories
#include "AssetFactory.hpp"
#include "AtmosphereFactory.hpp"
#include "AttitudeFactory.hpp"
#include "AxisSystemFactory.hpp"
#include "BurnFactory.hpp"
#include "CelestialBodyFactory.hpp"
#include "CommandFactory.hpp"
#include "CoordinateSystemFactory.hpp"
#include "ODEModelFactory.hpp"
#include "FunctionFactory.hpp"
#include "HardwareFactory.hpp"
#include "ParameterFactory.hpp"
#include "PhysicalModelFactory.hpp"
#include "PropagatorFactory.hpp"
#include "PropSetupFactory.hpp"
#include "SolverFactory.hpp"
#include "SpacecraftFactory.hpp"
#include "StopConditionFactory.hpp"
#include "SubscriberFactory.hpp"
#include "CalculatedPointFactory.hpp"
#include "MathFactory.hpp"
#include "Interface.hpp"
#include "XyPlot.hpp"
#include "OrbitPlot.hpp"
#include "GmatDefaults.hpp"

#include "NoOp.hpp"
#include "GravityField.hpp"
#include "RelativisticCorrection.hpp"
#include "CalculatedPoint.hpp"
#include "Barycenter.hpp"
#include "TimeSystemConverter.hpp"  // for SetLeapSecsFileReader(), SetEopFile()
#include "BodyFixedAxes.hpp"        // for SetEopFile(), SetCoefficientsFile()
#include "ObjectReferencedAxes.hpp"
#include "MessageInterface.hpp"
#include "CommandUtil.hpp"          // for GetCommandSeq()
#include "StringTokenizer.hpp"      // for StringTokenizer
#include "StringUtil.hpp"           // for GmatStringUtil::
#include "FileUtil.hpp"             // for GmatFileUtil::
#include <algorithm>                // for sort(), set_difference()
#include <wx/datetime.h>                    // for clock()
#include <wx/sstream.h>
#include <wx/txtstrm.h>



//#define DEBUG_INITIALIZE 1
//#define DEBUG_FINALIZE 1
//#define DEBUG_INTERPRET 2
//#define DEBUG_RUN 1
//#define DEBUG_CREATE_COORDSYS 1
//#define DEBUG_CREATE_RESOURCE 2
//#define DEBUG_CREATE_CALC_POINT
//#define DEBUG_CREATE_PARAMETER 1
//#define DEBUG_CREATE_EPHEMFILE 1
//#define DEBUG_PARAMETER_REF_OBJ 1
//#define DEBUG_DEFAULT_COMMAND 1
//#define DEBUG_COMMAND_APPEND 1
//#define DEBUG_COMMAND_DELETE 1
//#define DEBUG_RENAME 1
//#define DEBUG_REMOVE 1
//#define DEBUG_DEFAULT_MISSION 2
//#define DEBUG_MULTI_STOP 2
//#define DEBUG_USER_INTERRUPT 1
//#define DEBUG_LOOKUP_RESOURCE 1
//#define DEBUG_SEQUENCE_CLEARING 1
//#define DEBUG_CONFIG 1
//#define DEBUG_CREATE_VAR 1
//#define DEBUG_GMAT_FUNCTION 2
//#define DEBUG_OBJECT_MAP 1
//#define DEBUG_FIND_OBJECT 1
//#define DEBUG_ADD_OBJECT 1
//#define DEBUG_SOLAR_SYSTEM 1
//#define DEBUG_SOLAR_SYSTEM_IN_USE 1
//#define DEBUG_CREATE_BODY
//#define DEBUG_PLUGIN_REGISTRATION
//#define DEBUG_MATLAB
//#define DEBUG_CCSDS_EPHEMERIS
//#define DEBUG_CREATE_PHYSICAL_MODEL
//#define DEBUG_LIST_CALCULATED_POINT

// Currently we can't use DataFile for 2011a release so commented out
// Actually we want to put this flag in BuildEnv.mk but it is very close to
// release so added it here and EphemerisFile.cpp
//#define __USE_DATAFILE__

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

// @note If this is enabled, Bug1430-Func_GFuncInsideCFlow.script leaves
// no memory tracks but AssigningWholeObjects.script crashes when exiting
//#define __ENABLE_CLEAR_UNMANAGED_FUNCTIONS__
//#define __CREATE_DEFAULT_BC__
//#define __SHOW_FINAL_STATE__
//#define __DISABLE_SOLAR_SYSTEM_CLONING__

//---------------------------------
// static data
//---------------------------------
Moderator* Moderator::instance = NULL;
ScriptInterpreter* Moderator::theUiInterpreter = NULL;
ScriptInterpreter* Moderator::theScriptInterpreter = NULL;


//---------------------------------
// public
//---------------------------------

//------------------------------------------------------------------------------
// Moderator* Instance()
//------------------------------------------------------------------------------
Moderator* Moderator::Instance()
{
   if (instance == NULL)
      instance = new Moderator;
    
   return instance;
}


//------------------------------------------------------------------------------
// bool Initialize(const wxString &startupFile, bool fromGui = false)
//------------------------------------------------------------------------------
bool Moderator::Initialize(const wxString &startupFile, bool fromGui)
{
   #if DEBUG_INITIALIZE
   MessageInterface::ShowMessage
      (wxT("Moderator::Initialize() entered, startupFile='%s', fromGui=%d\n"),
       startupFile.c_str(), fromGui);
   #endif
   
   isFromGui = fromGui;
   
   try
   {
      // We don't want to write before startup file is read so commented out
      //MessageInterface::ShowMessage("Moderator is reading startup file...\n");
      
      // Read startup file, Set Log file
      theFileManager = FileManager::Instance();
      theFileManager->ReadStartupFile(startupFile);
      
      MessageInterface::ShowMessage(wxT("Moderator is creating core engine...\n"));
      
      // Set trace flag globally
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->SetShowTrace(false);
      #endif
      
      // Create Managers
      theFactoryManager = FactoryManager::Instance();
      
      #if DEBUG_INITIALIZE
      MessageInterface::ShowMessage
         (wxT(".....created  (%p)theFactoryManager\n"), theFactoryManager);
      #endif
      
      theConfigManager = ConfigManager::Instance();
      
      #if DEBUG_INITIALIZE
      MessageInterface::ShowMessage
         (wxT(".....created  (%p)theConfigManager\n"), theConfigManager);
      #endif
      
      // Register factories
      theFactoryManager->RegisterFactory(new AtmosphereFactory());
      theFactoryManager->RegisterFactory(new AttitudeFactory());
      theFactoryManager->RegisterFactory(new AxisSystemFactory());
      theFactoryManager->RegisterFactory(new BurnFactory());
      theFactoryManager->RegisterFactory(new CalculatedPointFactory());
      theFactoryManager->RegisterFactory(new CommandFactory());
      theFactoryManager->RegisterFactory(new CoordinateSystemFactory());
      theFactoryManager->RegisterFactory(new ODEModelFactory());
      theFactoryManager->RegisterFactory(new FunctionFactory());
      theFactoryManager->RegisterFactory(new HardwareFactory());
      theFactoryManager->RegisterFactory(new MathFactory());
      theFactoryManager->RegisterFactory(new ParameterFactory());
      theFactoryManager->RegisterFactory(new PhysicalModelFactory());
      theFactoryManager->RegisterFactory(new PropagatorFactory());
      theFactoryManager->RegisterFactory(new PropSetupFactory());
      theFactoryManager->RegisterFactory(new SolverFactory());
      theFactoryManager->RegisterFactory(new SpacecraftFactory());
      theFactoryManager->RegisterFactory(new StopConditionFactory());
      theFactoryManager->RegisterFactory(new SubscriberFactory());

      theFactoryManager->RegisterFactory(new CelestialBodyFactory());
      theFactoryManager->RegisterFactory(new AssetFactory());
            
      // Create publisher
      thePublisher = Publisher::Instance();
      
      #if DEBUG_INITIALIZE
      MessageInterface::ShowMessage
         (wxT(".....created  (%p)thePublisher...\n"), thePublisher);
      #endif
      
      // Create script interpreter
      theScriptInterpreter = ScriptInterpreter::Instance();      
      
      #if DEBUG_INITIALIZE
      MessageInterface::ShowMessage
         (wxT(".....created  (%p)theScriptInterpreter\n"), theScriptInterpreter);
      #endif
      
      LoadPlugins();
      
      // Create default SolarSystem
      theDefaultSolarSystem = CreateSolarSystem(wxT("DefaultSolarSystem"));
      
      #if DEBUG_INITIALIZE
      MessageInterface::ShowMessage
         (wxT(".....created  (%p)theDefaultSolarSystem\n"), theDefaultSolarSystem);
      #endif
      
      theConfigManager->SetDefaultSolarSystem(theDefaultSolarSystem);
      
      // Create solar system in use
      /// @note: If the solar system can be configured by name, add it to the
      ///        ConfigManager by calling CreateSolarSystem().
      ///        Until then, just use solar system name as "SolarSystem"
      
      CreateSolarSystemInUse();
      
      // Create other files in use
      CreatePlanetaryCoeffFile();
      CreateTimeFile();
      
      // Create at least 1 Sandbox and NoOp Command
      Sandbox *sandbox = new Sandbox();
      GmatCommand *noOp = new NoOp();
      
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Add(sandbox, wxT("Sandbox"), wxT("Moderator::Initialize()"), wxT(""));
      MemoryTracker::Instance()->Add(noOp, wxT("NoOP"), wxT("Moderator::Initialize()"), wxT(""));
      #endif
      
      sandboxes.push_back(sandbox);
      commands.push_back(noOp);
      
      #if DEBUG_INITIALIZE
      MessageInterface::ShowMessage
         (wxT(".....created  (%p)Sandbox 1\n"), sandboxes[0]);
      MessageInterface::ShowMessage
         (wxT(".....created  (%p)%s\n"), commands[0], commands[0]->GetTypeName().c_str());
      #endif
      
      // set objectMapInUse (loj: 2008.05.23)
      objectMapInUse = theConfigManager->GetObjectMap();
      
      #ifdef DEBUG_OBJECT_MAP
      MessageInterface::ShowMessage
         (wxT("Moderator::Initialize() objectMapInUse was set to the ")
          wxT("configuration map <%p>\n"), objectMapInUse);
      #endif
      
      if (isFromGui)
         CreateDefaultMission();
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("Error occurred during initialization: ") +
          e.GetFullMessage());
      return false;
   }
   catch (...)
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("Unknown Error occurred during initialization"));
      return false;
   }
   
   // Let's put current time out
   wxDateTime now = wxDateTime::Now();
   wxString timestr;
   timestr = now.Format( wxT("%Y-%m-%d %H:%M:%S"));
   
   MessageInterface::ShowMessage
      (wxT("%s GMAT Moderator successfully created core engine\n"), timestr.c_str());
   
   // Check to see if there are any event locator factories
   StringArray elList = theFactoryManager->GetListOfItems(Gmat::EVENT_LOCATOR);
   if (elList.size() > 0)
      GmatGlobal::Instance()->SetEventLocationAvailable(true);

   // Check if MatlabInterface is required
   if (GmatGlobal::Instance()->GetMatlabMode() == GmatGlobal::NO_MATLAB)
   {
      MessageInterface::ShowMessage
         (wxT("*** Use of MATLAB is disabled from the gmat_startup_file\n"));
   }
   else
   {
      try
      {
         theMatlabInterface = theFactoryManager->CreateInterface(wxT("MatlabInterface"), wxT("MI"));
         #ifdef DEBUG_MATLAB
         MessageInterface::ShowMessage
            (wxT("Moderator::Initialize() theMatlabInterface=<%p>\n"), theMatlabInterface);
         #endif
         // Check if MATLAB is installed
         // Do not override matlab setting in the startup file since
         // GmatFileUtil::IsAppInstalled() not implemented for all platforms
         wxString appLoc;
         bool hasMatlab = GmatFileUtil::IsAppInstalled(wxT("MATLAB"), appLoc);
         // Since GmatFileUtil::IsAppInstalled() is not complete for all platforms,
         // assume there is MATLAB for now. (LOJ: 2010.04.07)
         hasMatlab = true;
         if (hasMatlab)
         {
            #ifdef DEBUG_MATLAB
            MessageInterface::ShowMessage
               (wxT("*** MATLAB is installed in '%s'\n"), appLoc.c_str());
            #endif
            GmatGlobal::Instance()->SetMatlabAvailable(true);
         }
         else
         {
            #ifdef DEBUG_MATLAB
            MessageInterface::ShowMessage(wxT("*** MATLAB is not installed\n"));
            #endif
         }
      }
      catch (BaseException &be)
      {
         MessageInterface::ShowMessage(be.GetFullMessage());
      }
   }

   // Set MatlabInterface run mode, that is SINGLE_USE or SHARED MATLAB engine.
   if (theMatlabInterface == NULL)
      GmatGlobal::Instance()->SetMatlabMode(GmatGlobal::NO_MATLAB);
   else
      theMatlabInterface->
         SetIntegerParameter(wxT("MatlabMode"),
                             GmatGlobal::Instance()->GetMatlabMode());
   #if DEBUG_INITIALIZE
   MessageInterface::ShowMessage(wxT("Moderator::Initialize() returning true\n"));
   #endif
   
   return true;;
} // Initialize()



//------------------------------------------------------------------------------
// void Finalize()
//------------------------------------------------------------------------------
/*
 * Finalizes the system by closing all opened files by deleting objects.
 */
//------------------------------------------------------------------------------
void Moderator::Finalize()
{
   MessageInterface::ShowMessage(wxT("Moderator is deleting core engine...\n"));
   
   #if DEBUG_FINALIZE > 0
   MessageInterface::ShowMessage(wxT("Moderator::Finalize() entered\n"));
   #endif
   
   #if DEBUG_FINALIZE > 1
   //Notes: Do not use %s for command string, it may crash when it encounters
   // comment with % in the scripts
   GmatCommand *cmd = GetFirstCommand();
   MessageInterface::ShowMessage(GmatCommandUtil::GetCommandSeqString(cmd));
   MessageInterface::ShowMessage(GetScript());
   #endif
   
   #if DEBUG_FINALIZE > 0
   MessageInterface::ShowMessage
      (wxT(".....Moderator::Finalize() deleting (%p)theFileManager\n"), theFileManager);
   MessageInterface::ShowMessage
      (wxT(".....Moderator::Finalize() deleting (%p)theEopFile\n"), theEopFile);
   MessageInterface::ShowMessage
      (wxT(".....Moderator::Finalize() deleting (%p)theItrfFile\n"), theItrfFile);
   MessageInterface::ShowMessage
      (wxT(".....Moderator::Finalize() deleting (%p)theLeapSecsFile\n"), theLeapSecsFile);
   #endif
   
   delete theFileManager;
   delete theEopFile;
   delete theItrfFile;
   delete theLeapSecsFile;
   if (theMatlabInterface != NULL)
      delete theMatlabInterface;
   
   theFileManager = NULL;
   theEopFile = NULL;
   theItrfFile = NULL;
   theLeapSecsFile = NULL;
   
   //clear resource and command sequence
   try
   {
      #if DEBUG_FINALIZE > 0
      MessageInterface::ShowMessage(wxT(".....clearing resource\n"));
      MessageInterface::ShowMessage(wxT(".....clearing command sequence\n"));
      #endif
      
      // Clear command sequence before resource (loj: 2008.07.10)
      // only 1 sandbox for now
      ClearCommandSeq(false, false);
      ClearResource();
      
      // Delete the plugin resource data
      for (UnsignedInt i = 0; i < userResources.size(); ++i)
         delete userResources[i];
      userResources.clear();

      // Close out the plug-in libraries
      std::map<wxString, DynamicLibrary*>::iterator i;
      for (i = userLibraries.begin(); i != userLibraries.end(); ++i)
      {
         delete i->second;
         i->second = NULL;
         #ifdef DEBUG_PLUGINS
            MessageInterface::ShowMessage(wxT("Closed %s.\n"), i->first.c_str());
         #endif
      }

      
//       #if DEBUG_FINALIZE > 0
//       MessageInterface::ShowMessage
//          (".....deleting (%p)theGuiInterpreter\n", theGuiInterpreter);
//       #endif
//       delete theGuiInterpreter;
//       theGuiInterpreter = NULL;
      
      //delete theFactoryManager; (private destructor)
      //delete theConfigManager; (private destructor)
      //delete theScriptInterpreter; (private destructor)
      //delete theGuiInterpreter; (private destructor)
      #if DEBUG_FINALIZE > 0
      MessageInterface::ShowMessage
         (wxT(".....Moderator::Finalize() deleting (%p)thePublisher\n"),
          thePublisher);
      #endif      
      delete thePublisher;
      
      // delete solar systems
      #if DEBUG_FINALIZE > 0
      MessageInterface::ShowMessage
         (wxT(".....Moderator::Finalize() deleting (%p)theDefaultSolarSystem\n"),
          theDefaultSolarSystem);
      #endif      
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (theDefaultSolarSystem, theDefaultSolarSystem->GetName(),
          wxT("Moderator::Finalize()"));
      #endif
      delete theDefaultSolarSystem;
      theDefaultSolarSystem = NULL;
      
      if (theSolarSystemInUse != NULL)
      {
         #if DEBUG_FINALIZE > 0
         MessageInterface::ShowMessage
            (wxT(".....Moderator::Finalize() deleting (%p)theSolarSystemInUse\n"),
             theSolarSystemInUse);
         #endif
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (theSolarSystemInUse, theSolarSystemInUse->GetName(),
             wxT("Moderator::Finalize()"));
         #endif
         delete theSolarSystemInUse;
         theSolarSystemInUse = NULL;
      }
      
      // delete internal coordinate system
      if (theInternalCoordSystem != NULL)
      {
         #if DEBUG_FINALIZE > 0
         MessageInterface::ShowMessage
            (wxT(".....Moderator::Finalize() deleting (%p)theInternalCoordSystem\n"),
             theInternalCoordSystem);
         #endif
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (theInternalCoordSystem, theInternalCoordSystem->GetName(),
             wxT("Moderator::Finalize()"));
         #endif
         delete theInternalCoordSystem;
         theInternalCoordSystem = NULL;
      }
      
      // Delete unmanaged functions (LOJ: 2009.03.24)
      // This causes crash on Func_MatlabObjectPassingCheck.script
      // so disable until it is fully tested (LOJ: 2009.04.08)
      #ifdef __ENABLE_CLEAR_UNMANAGED_FUNCTIONS__
      #if DEBUG_FINALIZE > 0
      MessageInterface::ShowMessage
         (wxT(".....Moderator::Finalize() deleting %d unmanaged functions\n"),
          unmanagedFunctions.size());
      #endif
      for (UnsignedInt i=0; i<unmanagedFunctions.size(); i++)
      {
         GmatBase *func = (GmatBase*)(unmanagedFunctions[i]);
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (func, func->GetName(), wxT("Moderator::Finalize()"),
             wxT("deleting unmanaged function"));
         #endif
         delete func;
         func = NULL;
      }
      unmanagedFunctions.clear();
      #endif
      
      // delete Sandbox (only 1 Sandbox for now)
      #if DEBUG_FINALIZE > 0
      MessageInterface::ShowMessage
         (wxT(".....Moderator::Finalize() deleting (%p)sandbox 1\n"), sandboxes[0]);
      #endif
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (sandboxes[0], wxT("Sandbox"), wxT("Moderator::Finalize()"));
      #endif
      delete sandboxes[0];
      commands[0] = NULL;
      sandboxes[0] = NULL;
      commands.clear();
      sandboxes.clear();
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
   
   #ifdef DEBUG_MEMORY
   StringArray tracks = MemoryTracker::Instance()->GetTracks(true, false);
   MessageInterface::ShowMessage
      (wxT("===> There are %d memory tracks after Finalize\n"), tracks.size());
   for (UnsignedInt i=0; i<tracks.size(); i++)
      MessageInterface::ShowMessage(wxT("%s\n"), tracks[i].c_str());
   #endif
   
   #if DEBUG_FINALIZE > 0
   MessageInterface::ShowMessage(wxT("Moderator::Finalize() exiting\n"));
   #endif
} // Finalize()


//------------------------------------------------------------------------------
// void SetRunReady(bool flag = false)
//------------------------------------------------------------------------------
void Moderator::SetRunReady(bool flag)
{
   isRunReady = flag;
}


//------------------------------------------------------------------------------
// Interface* GetMatlabInterface()
//------------------------------------------------------------------------------
/**
 * Returns MatlabInterface pointer.
 */ 
//------------------------------------------------------------------------------
Interface* Moderator::GetMatlabInterface()
{
   return theMatlabInterface;
}


//------------------------------------------------------------------------------
// bool OpenMatlabEngine()
//------------------------------------------------------------------------------
bool Moderator::OpenMatlabEngine()
{
   #ifdef DEBUG_MATLAB
   MessageInterface::ShowMessage
      (wxT("Moderator::OpenMatlabEngine() theMatlabInterface=<%p>\n"),
       theMatlabInterface);
   #endif
   
   if (theMatlabInterface != NULL)
   {
      #ifdef DEBUG_MATLAB
      MessageInterface::ShowMessage
         (wxT("Moderator::OpenMatlabEngine() calling theMatlabInterface->Open()\n"));
      #endif
      
      if (theMatlabInterface->Open() == 1)
         return true;
      else
         return false;
   }
   else
   {
      #ifdef DEBUG_MATLAB
      MessageInterface::ShowMessage
         (wxT("Moderator::OpenMatlabEngine() theMatlabInterface is NULL, so returning false\n"));
      #endif
      return false;
   }
}


//------------------------------------------------------------------------------
// bool CloseMatlabEngine()
//------------------------------------------------------------------------------
bool Moderator::CloseMatlabEngine()
{
   #ifdef DEBUG_MATLAB
   MessageInterface::ShowMessage
      (wxT("Moderator::CloseMatlabEngine() theMatlabInterface=<%p>\n"),
       theMatlabInterface);
   #endif
   
   if (theMatlabInterface != NULL)
   {
      #ifdef DEBUG_MATLAB
      MessageInterface::ShowMessage
         (wxT("Moderator::CloseMatlabEngine() calling theMatlabInterface->Close()\n"));
      #endif
      
      if (theMatlabInterface->Close() == 1)
         return true;
      else
         return false;
   }
   else
   {
      #ifdef DEBUG_MATLAB
      MessageInterface::ShowMessage
         (wxT("Moderator::OpenMatlabEngine() theMatlabInterface is NULL, so returning false\n"));
      #endif
      return false;
   }
}


//---------------------------- Plug-in modules ---------------------------------

//------------------------------------------------------------------------------
// void LoadPlugins()
//------------------------------------------------------------------------------
/**
 * Method that loads in the plug-in libraries listed in a user's startup file.
 * 
 * The GMAT startup file may list one or more plug-in libraries by name.  This 
 * method retrieves the list of libraries, and loads them into GMAT.
 * 
 * @note The current code looks for exactly one library -- the VF13ad library --
 *       and loads it into GMAT if found.  The generic updates for any user 
 *       library will be added in a later build.
 */ 
//------------------------------------------------------------------------------
void Moderator::LoadPlugins()
{
   StringArray pluginList = theFileManager->GetPluginList();

   // This is done for all plugins in the startup file
   for (StringArray::const_iterator i = pluginList.begin(); 
         i != pluginList.end(); ++i)
   {

      #ifndef __WIN32__
   
         #ifdef DEBUG_PLUGIN_REGISTRATION
            MessageInterface::ShowMessage(
                  wxT("*** Loading dynamic library \"%s\": "), i->c_str());
         #endif
         LoadAPlugin(*i);

      #else
         
         #ifdef DEBUG_PLUGIN_REGISTRATION
            MessageInterface::ShowMessage(
                  wxT("*** Loading dynamic library \"%s\": "), i->c_str());
         #endif
         LoadAPlugin(*i);
        
      #endif
   }
   
   if (theUiInterpreter != NULL)
      theUiInterpreter->BuildCreatableObjectMaps();
   theScriptInterpreter->BuildCreatableObjectMaps();
}

//------------------------------------------------------------------------------
// void LoadAPlugin(wxString pluginName)
//------------------------------------------------------------------------------
/**
 * Method that loads a plug-in library into memory.
 * 
 * This method loads a plug-in library into memory, and retrieves and registers 
 * any Factories contained in that plug-in.  If the library is not found, this 
 * method just returns.
 * 
 * @param pluginName The file name for the plug-in library.  The name should not 
 *                   include the file extension (e.g. ".ddl" or ".so").
 */ 
//------------------------------------------------------------------------------
void Moderator::LoadAPlugin(wxString pluginName)
{
   // Set platform specific slash style
   #ifdef DEBUG_PLUGIN_REGISTRATION
      MessageInterface::ShowMessage(wxT("Input plugin name: \"%s\"\n"), pluginName.c_str());
   #endif

   wxChar fSlash = wxT('/');
   wxChar bSlash = wxT('\\');
   wxChar osSlash = wxT('\\');       // Default to Windows, but change if *nix

   #ifndef _WIN32
      osSlash = wxT('/');          // Mac or Linux
   #endif

   #ifdef DEBUG_PLUGIN_REGISTRATION
      MessageInterface::ShowMessage(wxT("OS slash is \"%c\"\n"), osSlash);
   #endif

   for (UnsignedInt i = 0; i < pluginName.length(); ++i)
   {
      if ((pluginName[i] == fSlash) || (pluginName[i] == bSlash))
         pluginName[i] = osSlash;
   }

   #ifdef DEBUG_PLUGIN_REGISTRATION
      MessageInterface::ShowMessage(wxT("Used plugin name:  \"%s\"\n"), pluginName.c_str());
   #endif

   DynamicLibrary *theLib = LoadLibrary(pluginName);

   if (theLib != NULL)
   {
      Integer fc = theLib->GetFactoryCount();

      if (fc > 0)
      {
         // Do the GMAT factory dance
         #ifdef DEBUG_PLUGIN_REGISTRATION
            MessageInterface::ShowMessage(
               wxT("Library %s contains %d %s.\n"), pluginName.c_str(), fc,
               ( fc==1 ? wxT("factory") : wxT("factories")));
         #endif
            
         // Now pass factories to the FactoryManager
         Factory *newFactory = NULL;
         for (Integer i = 0; i < fc; ++i)
         {
            newFactory = theLib->GetGmatFactory(i);
            if (newFactory != NULL)
            {
               if (theFactoryManager->RegisterFactory(newFactory) == false)
                  MessageInterface::ShowMessage(
                        wxT("Factory %d in library %s failed to register with the ")
                        wxT("Factory Manager.\n"), i, pluginName.c_str());
               else
               {
                  #ifdef DEBUG_PLUGIN_REGISTRATION
                     MessageInterface::ShowMessage(
                        wxT("Factory %d in library %s is now registered with the ")
                        wxT("Factory Manager!\n"), i, pluginName.c_str());

                     StringArray facts = newFactory->GetListOfCreatableObjects();

                     MessageInterface::ShowMessage(
                           wxT("The new factory creates these objects types:\n"));

                     for (UnsignedInt f = 0; f < facts.size(); ++f)
                        MessageInterface::ShowMessage(wxT("   %s\n"),
                              facts[f].c_str());
                  #endif
               }
            } 
            else
               MessageInterface::ShowMessage(
                     wxT("Factory %d in library %s was not constructed; a NULL ")
                     wxT("pointer was returned instead.\n"), i, pluginName.c_str());
         }
      }
      else
         MessageInterface::PutMessage(
            wxT("*** Library \"") + pluginName + wxT("\" does not contain a factory\n"));
      
      // Test to see if there might be TriggerManagers
      Integer triggerCount = theLib->GetTriggerManagerCount();
      #ifdef DEBUG_PLUGIN_REGISTRATION
         MessageInterface::ShowMessage(
            wxT("Library %s contains %d %s.\n"), pluginName.c_str(), triggerCount,
            ( triggerCount == 1 ? wxT("TriggerManager") : wxT("TriggerManagers")));
      #endif

      for (Integer i = 0; i < triggerCount; ++i)
      {
         TriggerManager *tm = theLib->GetTriggerManager(i);
         triggerManagers.push_back(tm);

         #ifdef DEBUG_PLUGIN_REGISTRATION
            MessageInterface::ShowMessage(
               wxT("   TriggerManager %d of type %s is now registered.\n"), i,
               tm->GetTriggerTypeString().c_str());
         #endif
      }

      // Check for new GUI elements
      Integer menuCount = theLib->GetMenuEntryCount();
      #ifdef DEBUG_PLUGIN_REGISTRATION
         MessageInterface::ShowMessage(
            wxT("Library %s contains %d %s.\n"), pluginName.c_str(), menuCount,
            ( menuCount == 1 ? wxT("menu entry") : wxT("menu entries")));
      #endif

      for (Integer i = 0; i < menuCount; ++i)
      {
         Gmat::PluginResource* res = theLib->GetMenuEntry(i);
         if (res != NULL)
         {
            #ifdef DEBUG_PLUGIN_REGISTRATION
               MessageInterface::ShowMessage(wxT("Adding user resource node:\n")
                     wxT("   Name: %s\n   Parent: %s\n   type: %d\n")
                     wxT("   subtype: %s\n"), res->nodeName.c_str(),
                     res->parentNodeName.c_str(), res->type,
                     res->subtype.c_str());
            #endif
            userResources.push_back(res);
         }

         #ifdef DEBUG_PLUGIN_REGISTRATION
            MessageInterface::ShowMessage(
               wxT("   Menu entry %d for node %s is now registered.\n"), i,
               res->nodeName.c_str());
         #endif
      }
   }
   else
      MessageInterface::PutMessage(
         wxT("*** Unable to load the dynamic library \"") + pluginName + wxT("\"\n"));
}

//------------------------------------------------------------------------------
// Dynamic library specific code
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// DynamicLibrary *LoadLibrary(const wxString &libraryName)
//------------------------------------------------------------------------------
/**
 * Loads a dynamic library into memory.
 * 
 * Creates a DynamicLibrary object and uses that instance to provide the 
 * interfaces GMAT uses to load a dynamic library into memory.  If the library 
 * is not found or if it could not be loaded, a message is written out stating 
 * that the library did not open.
 * 
 * @param libraryName The file name for the plug-in library.  The name should 
 *                    not include the file extension.
 * 
 * @return The DynamicLibrary object that supplies the library interfaces, or a
 *         NULL pointer if the library did not load.
 */ 
//------------------------------------------------------------------------------
DynamicLibrary *Moderator::LoadLibrary(const wxString &libraryName)
{
   DynamicLibrary *theLib = new DynamicLibrary(libraryName);
   if (theLib->LoadDynamicLibrary())
   {
      userLibraries[libraryName] = theLib;
   }
   else
   {
      MessageInterface::ShowMessage(wxT("*** Library \"%s\" did not open.\n"),
            libraryName.c_str());
      delete theLib;
      theLib = NULL;
   }
   
   return theLib;
}

//------------------------------------------------------------------------------
// bool IsLibraryLoaded(const wxString &libName)
//------------------------------------------------------------------------------
/**
 * Method that checks to see if a specified library has been loaded.
 * 
 * @param libName The name of the library.
 * 
 * @return true if the library has been loaded, false if not.
 */ 
//------------------------------------------------------------------------------
bool Moderator::IsLibraryLoaded(const wxString &libName)
{
   bool retval = false;
   if (userLibraries.find(libName) != userLibraries.end())
      retval = true;
   
   return retval;
}

//------------------------------------------------------------------------------
// void (*GetDynamicFunction(const wxString &funName, 
//                           const wxString &libraryName))()
//------------------------------------------------------------------------------
/**
 * Retrieves a specified function from a specified library.
 * 
 * @param funName The name of the requested function.
 * @param libraryName Teh name of the library that contains the function.
 * 
 * @return A function pointer for the specified function, or NULL if the 
 *         function is not found.  The returned function pointer has the 
 *         signature
 *             void (*)()
 *         and should be cast to the correct signature.
 */ 
//------------------------------------------------------------------------------
void (*Moderator::GetDynamicFunction(const wxString &funName, 
      const wxString &libraryName))()
{
   void (*theFunction)() = NULL;
   if (IsLibraryLoaded(libraryName))
      theFunction = userLibraries[libraryName]->GetFunction(funName);
   return theFunction;
}

//------------------------ End of Plug-in Code ---------------------------------


//----- ObjectType
//------------------------------------------------------------------------------
// wxString GetObjectTypeString(Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Returns object type name of given object type.
 *
 * @param <type> object type
 *
 * @return object type name
 */
//------------------------------------------------------------------------------
wxString Moderator::GetObjectTypeString(Gmat::ObjectType type)
{
   if (type >= Gmat::SPACECRAFT && type <= Gmat::PROP_SETUP)
      return GmatBase::OBJECT_TYPE_STRING[type - Gmat::SPACECRAFT];
   else
      return wxT("UnknownObject");
}

//----- interpreter
//------------------------------------------------------------------------------
// GuiInterpreter* GetGuiInterpreter()
//------------------------------------------------------------------------------
/**
 * Returns GuiInterpreter pointer.
 *
 * @return GuiInterpreter pointer
 */
//------------------------------------------------------------------------------
ScriptInterpreter* Moderator::GetUiInterpreter()
{
   return theUiInterpreter;
}

//------------------------------------------------------------------------------
// ScriptInterpreter* GetScriptInterpreter()
//------------------------------------------------------------------------------
/**
 * Returns ScriptInterpreter pointer.
 *
 * @return ScriptInterpreter pointer
 */
//------------------------------------------------------------------------------
ScriptInterpreter* Moderator::GetScriptInterpreter()
{
   return theScriptInterpreter;
}


//------------------------------------------------------------------------------
// void SetGuiInterpreter(GuiInterpreter *guiInterp)
//------------------------------------------------------------------------------
/**
 * Sets GuiInterpreter pointer.
 */
//------------------------------------------------------------------------------
void Moderator::SetUiInterpreter(ScriptInterpreter *uiInterp)
{
   theUiInterpreter = uiInterp;
   theUiInterpreter->BuildCreatableObjectMaps();
}


//------------------------------------------------------------------------------
// void SetScriptInterpreter(ScriptInterpreter *scriptInterp)
//------------------------------------------------------------------------------
/**
 * Sets ScriptInterpreter pointer.
 */
//------------------------------------------------------------------------------
void Moderator::SetScriptInterpreter(ScriptInterpreter *scriptInterp)
{
   //loj: allow setting only for the first time?
   if (theScriptInterpreter == NULL)
      theScriptInterpreter = scriptInterp;
}


//------------------------------------------------------------------------------
// void SetInterpreterMapAndSS(Interpreter *interp)
//------------------------------------------------------------------------------
/**
 * Sets Interpreter ObjectMap and SolarSystem to current pointers in use.
 * 
 * @param interp The Interpreter that is setup. 
 */
//------------------------------------------------------------------------------
void Moderator::SetInterpreterMapAndSS(Interpreter *interp)
{
   interp->SetObjectMap(objectMapInUse, true);
   interp->SetSolarSystemInUse(theSolarSystemInUse);
}

//----- object finding
//------------------------------------------------------------------------------
// void SetObjectMap(ObjectMap *objMap)
//------------------------------------------------------------------------------
void Moderator::SetObjectMap(ObjectMap *objMap)
{
   if (objMap != NULL)
   {
      objectMapInUse = objMap;
      
      #ifdef DEBUG_OBJECT_MAP
      MessageInterface::ShowMessage
         (wxT("Moderator::SetObjectMap() objectMapInUse was set to input objMap <%p>\n"),
          objMap);
      ShowObjectMap(wxT("Moderator::SetObjectMap() Here is the object map in use"));
      #endif
   }
}


//------------------------------------------------------------------------------
// void SetObjectManageOption(Integer option)
//------------------------------------------------------------------------------
/*
 * Sets object manage option. Usually objects created inside GmatFunction uses
 * object maps passed to Moderator. All objects created in the main sequence and
 * through the GUI are managed through Configuration Manager.
 *
 * @param <option>  0, if object is not managed
 *                  1, if configuration object map is used for finding objects
 *                     and adding new objects (default)
 *                  2, if function object map is used for finding objects and
 *                     adding new objects including automatic objects
 */
//------------------------------------------------------------------------------
void Moderator::SetObjectManageOption(Integer option)
{
   #ifdef DEBUG_OBJECT_MANAGE
   MessageInterface::ShowMessage
      (wxT("Moderator::SetObjectManageOption() option = %d\n"), option);
   #endif
   objectManageOption = option;
}


//------------------------------------------------------------------------------
// Integer GetObjectManageOption()
//------------------------------------------------------------------------------
/*
 * returns object manage option.
 */
//------------------------------------------------------------------------------
Integer Moderator::GetObjectManageOption()
{
   return objectManageOption;
}


//------------------------------------------------------------------------------
// bool ResetObjectPointer(GmatBase *newObj, const wxString &name)
//------------------------------------------------------------------------------
/*
 * Sets configured object pointer with new pointer.
 *
 * @param  newObj  New object pointer
 * @param  name  Name of the configured object to be reset
 *
 * @return  true if pointer was reset, false otherwise
 */
//------------------------------------------------------------------------------
void Moderator::ResetObjectPointer(ObjectMap *objMap, GmatBase *newObj,
                                   const wxString &name)
{
   #if DEBUG_RESET_OBJECT
   MessageInterface::ShowMessage
      (wxT("Moderator::ResetObjectPointer() entered, objMap=<%p>, newObj=<%p>, ")
       wxT("name='%s'\n"), objMap, obj, newObj, name.c_str());
   #endif
   
   if (objMap->find(name) != objMap->end())
   {
      GmatBase *mapObj = (*objMap)[name];
      if (mapObj->GetName() == name)
      {
         // We want to replace if it has the same sub type
         if (newObj->IsOfType(mapObj->GetTypeName()))
         {
            #if DEBUG_RESET_OBJECT
            MessageInterface::ShowMessage
               (wxT("   Replacing mapObj=<%p> with newObj=<%p>\n"), mapObj, newObj);
            #endif
            (*objMap)[name] = newObj;
         }
      }
   }
   
   #if DEBUG_RESET_OBJECT
   MessageInterface::ShowMessage(wxT("Moderator::ResetObjectPointer() leaving\n"));
   #endif
}


//----- factory
//------------------------------------------------------------------------------
// const StringArray& GetListOfFactoryItems(Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Returns names of all configurable items of object type.
 *
 * @param <type> object type
 *
 * @return array of configurable item names; return empty array if none
 */
//------------------------------------------------------------------------------
const StringArray& Moderator::GetListOfFactoryItems(Gmat::ObjectType type)
{
   return theFactoryManager->GetListOfItems(type);
}

//------------------------------------------------------------------------------
// const StringArray& GetListOfAllFactoryItems()
//------------------------------------------------------------------------------
/**
 * Return a list of all items that can be created.
 *
 * @return list of all creatable items.
 */
//------------------------------------------------------------------------------
const StringArray& Moderator::GetListOfAllFactoryItems()
{
   return theFactoryManager->GetListOfAllItems();
}

//------------------------------------------------------------------------------
// const StringArray& GetListOfAllFactoryItemsExcept(const ObjectTypeArray &types)
//------------------------------------------------------------------------------
/**
 * Returns names of all configurable items of object type except given type
 *
 * @param <type> object types to be excluded
 *
 * @return array of configurable item names excluding types;
 *         return empty array if none
 */
//------------------------------------------------------------------------------
const StringArray& Moderator::GetListOfAllFactoryItemsExcept(const ObjectTypeArray &types)
{
   return theFactoryManager->GetListOfAllItemsExcept(types);
}

//------------------------------------------------------------------------------
// const StringArray& GetListOfViewableItems(Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Return a list of all viewable objets via the GUI
 *
 * @param  type  The enumerated ObjectType defined in gmatdef.hpp
 *
 * @return list of all viewable objects
 */
//------------------------------------------------------------------------------
const StringArray& Moderator::GetListOfViewableItems(Gmat::ObjectType type)
{
   return theFactoryManager->GetListOfViewableItems(type);
}

//------------------------------------------------------------------------------
// const StringArray& GetListOfUnviewableItems(Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Return a list of all unviewable objects via the GUI
 *
 * @return list of all viewable objects
 */
//------------------------------------------------------------------------------
const StringArray& Moderator::GetListOfUnviewableItems(Gmat::ObjectType type)
{
   return theFactoryManager->GetListOfUnviewableItems(type);
}


//------------------------------------------------------------------------------
// bool DoesObjectTypeMatchSubtype(const wxString &theType,
//       const wxString &theSubtype)
//------------------------------------------------------------------------------
/**
 * Checks if a creatable object type matches a subtype.
 *
 * @param coreType The ObjectType of the candidate object
 * @param theType The script identifier for the object type
 * @param theSubtype The subtype being checked
 *
 * @return true if the scripted type and subtype match, false if the type does
 *         not match the subtype
 */
//------------------------------------------------------------------------------
bool Moderator::DoesObjectTypeMatchSubtype(const Gmat::ObjectType coreType,
      const wxString &theType, const wxString &theSubtype)
{
   return theFactoryManager->DoesObjectTypeMatchSubtype(coreType, theType,
         theSubtype);
}


//----- configuration
//------------------------------------------------------------------------------
// ObjectMap* GetConfiguredObjectMap()
//------------------------------------------------------------------------------
/*
 * Returns configured object map
 */
//------------------------------------------------------------------------------
ObjectMap* Moderator::GetConfiguredObjectMap()
{
   return theConfigManager->GetObjectMap();
}


//------------------------------------------------------------------------------
// const StringArray& GetListOfObjects(Gmat::ObjectType type,
//                                     bool excludeDefaultObjects)
//------------------------------------------------------------------------------
/**
 * Returns names of all configured items of object type.
 *
 * @param <type> object type
 * @param <excludeDefaultObjects> set this flag to true if default objects
 *           should be execluded, such as  default coordinate systems
 *
 * @return array of configured item names of the type; return empty array if none
 *  return all configured item if type is UNKNOWN_OBJECT
 */
//------------------------------------------------------------------------------
const StringArray& Moderator::GetListOfObjects(Gmat::ObjectType type,
                                               bool excludeDefaultObjects)
{
   tempObjectNames.clear();
   
   if (type == Gmat::UNKNOWN_OBJECT)
      return theConfigManager->GetListOfAllItems();
   
   if (type == Gmat::CELESTIAL_BODY || type == Gmat::SPACE_POINT)
   {
      tempObjectNames.clear();
      
      if (theSolarSystemInUse == NULL)
         return tempObjectNames;
      
      if (type == Gmat::CELESTIAL_BODY)
      {
         // add bodies to the list
         tempObjectNames = theSolarSystemInUse->GetBodiesInUse();
      }
      else if (type == Gmat::SPACE_POINT)
      {
         // add Spacecraft to the list
         tempObjectNames = theConfigManager->GetListOfItems(Gmat::SPACECRAFT);
         
         // add bodies to the list
         StringArray bodyList = theSolarSystemInUse->GetBodiesInUse();
         for (UnsignedInt i=0; i<bodyList.size(); i++)
            tempObjectNames.push_back(bodyList[i]);
         
         // add CalculatedPoint to the list
         StringArray calptList =
            theConfigManager->GetListOfItems(Gmat::CALCULATED_POINT);
         // Do not add default (built-in) barycenter(s) on option
         #ifdef DEBUG_LIST_CALCULATED_POINT
            MessageInterface::ShowMessage(wxT("There are %d configured calculated points.\n"), (Integer) calptList.size());
         #endif
         if (excludeDefaultObjects)
         {
            #ifdef DEBUG_LIST_CALCULATED_POINT
               MessageInterface::ShowMessage(wxT("--- Excluding default Calculated Point objects .....\n"));
            #endif
            for (UnsignedInt i=0; i<calptList.size(); i++)
            {
               if (calptList[i] != GmatSolarSystemDefaults::SOLAR_SYSTEM_BARYCENTER_NAME)
                  tempObjectNames.push_back(calptList[i]);
            }
         }
         else
         {
            #ifdef DEBUG_LIST_CALCULATED_POINT
               MessageInterface::ShowMessage(wxT("--- NOT Excluding default Calculated Point objects .....\n"));
            #endif
            for (UnsignedInt i=0; i<calptList.size(); i++)
               tempObjectNames.push_back(calptList[i]);
         }
         
         StringArray osptList =
            theConfigManager->GetListOfItems(Gmat::SPACE_POINT);
         for (UnsignedInt i=0; i<osptList.size(); i++)
         {
            // do not add the same object name
            if (find(tempObjectNames.begin(), tempObjectNames.end(), osptList[i])
                == tempObjectNames.end())
               tempObjectNames.push_back(osptList[i]);
         }
      }
      
      return tempObjectNames;
   }
   
   // Do not add default coordinate systems on option
   if (type == Gmat::COORDINATE_SYSTEM && excludeDefaultObjects)
   {
      tempObjectNames.clear();
      StringArray csObjNames = theConfigManager->GetListOfItems(type);
      for (UnsignedInt i=0; i<csObjNames.size(); i++)
         if (csObjNames[i] != wxT("EarthMJ2000Eq") && csObjNames[i] != wxT("EarthMJ2000Ec") &&
             csObjNames[i] != wxT("EarthFixed"))
            tempObjectNames.push_back(csObjNames[i]);
      return tempObjectNames;
   }
   
   // Do not add default barycenter on option
   if (type == Gmat::CALCULATED_POINT && excludeDefaultObjects)
   {
      tempObjectNames.clear();
      StringArray cpNames = theConfigManager->GetListOfItems(type);
      for (UnsignedInt i=0; i<cpNames.size(); i++)
         if (cpNames[i] != GmatSolarSystemDefaults::SOLAR_SYSTEM_BARYCENTER_NAME)
            tempObjectNames.push_back(cpNames[i]);
      return tempObjectNames;
   }

   return theConfigManager->GetListOfItems(type);
}


//------------------------------------------------------------------------------
// const StringArray& GetListOfObjects(const wxString &typeName,
//                                     bool excludeDefaultObjects)
//------------------------------------------------------------------------------
/**
 * Returns names of all configured items of object type name
 *
 * @param <typeName> object type name
 * @param <excludeDefaultObjects> set this flag to true if default objects
 *           should be execluded, such as  default coordinate systems
 *
 * @return array of configured item names of the type; return empty array if none
 *  return all configured item if type is UNKNOWN_OBJECT
 */
//------------------------------------------------------------------------------
const StringArray& Moderator::GetListOfObjects(const wxString &typeName,
                                               bool excludeDefaultObjects)
{
   if (typeName == wxT("UnknownObject"))
      return theConfigManager->GetListOfAllItems();
   
   if (typeName == wxT("CelestialBody") || typeName == wxT("SpacePoint"))
   {
      tempObjectNames.clear();
      
      if (theSolarSystemInUse == NULL)
         return tempObjectNames;
      
      if (typeName == wxT("CelestialBody"))
      {
         // add bodies to the list
         tempObjectNames = theSolarSystemInUse->GetBodiesInUse();
      }
      else if (typeName == wxT("SpacePoint"))
      {
         // add Spacecraft to the list
         tempObjectNames = theConfigManager->GetListOfItems(Gmat::SPACECRAFT);
         
         // add bodies to the list
         StringArray bodyList = theSolarSystemInUse->GetBodiesInUse();
         for (UnsignedInt i=0; i<bodyList.size(); i++)
            tempObjectNames.push_back(bodyList[i]);
         
         // add CalculatedPoint to the list
         StringArray calptList =
            theConfigManager->GetListOfItems(Gmat::CALCULATED_POINT);
         if (excludeDefaultObjects)
         {
            #ifdef DEBUG_LIST_CALCULATED_POINT
               MessageInterface::ShowMessage(wxT("--- Excluding default Calculated Point objects .....\n"));
            #endif
            for (UnsignedInt i=0; i<calptList.size(); i++)
            {
               if (calptList[i] != GmatSolarSystemDefaults::SOLAR_SYSTEM_BARYCENTER_NAME)
                  tempObjectNames.push_back(calptList[i]);
            }
         }
         else
         {
            #ifdef DEBUG_LIST_CALCULATED_POINT
               MessageInterface::ShowMessage(wxT("--- NOT Excluding default Calculated Point objects .....\n"));
            #endif
            for (UnsignedInt i=0; i<calptList.size(); i++)
               tempObjectNames.push_back(calptList[i]);
         }
         for (UnsignedInt i=0; i<calptList.size(); i++)
            tempObjectNames.push_back(calptList[i]);
         
         StringArray osptList =
            theConfigManager->GetListOfItems(Gmat::SPACE_POINT);
         for (UnsignedInt i=0; i<osptList.size(); i++)
            tempObjectNames.push_back(osptList[i]);
      }
      
      return tempObjectNames;
   }
   
   // Do not add default coordinate systems on option
   if (typeName == wxT("CoordinateSystem") && excludeDefaultObjects)
   {
      tempObjectNames.clear();
      StringArray csObjNames = theConfigManager->GetListOfItems(typeName);
      for (UnsignedInt i=0; i<csObjNames.size(); i++)
         if (csObjNames[i] != wxT("EarthMJ2000Eq") && csObjNames[i] != wxT("EarthMJ2000Ec") &&
             csObjNames[i] != wxT("EarthFixed"))
            tempObjectNames.push_back(csObjNames[i]);
      return tempObjectNames;
   }
   // Do not add default barycenter on option
   if (typeName == wxT("CalculatedPoint") && excludeDefaultObjects)
   {
      tempObjectNames.clear();
      StringArray cpNames = theConfigManager->GetListOfItems(typeName);
      for (UnsignedInt i=0; i<cpNames.size(); i++)
         if (cpNames[i] != GmatSolarSystemDefaults::SOLAR_SYSTEM_BARYCENTER_NAME)
            tempObjectNames.push_back(cpNames[i]);
      return tempObjectNames;
   }
   
   return theConfigManager->GetListOfItems(typeName);
}


//------------------------------------------------------------------------------
// GmatBase* GetConfiguredObject(const wxString &name)
//------------------------------------------------------------------------------
GmatBase* Moderator::GetConfiguredObject(const wxString &name)
{
   #if DEBUG_CONFIG
   MessageInterface::ShowMessage
      (wxT("Moderator::GetConfiguredObject() entered: name=%s\n"), name.c_str());
   #endif
   
   wxString newName = name;
   
   // check for SolarSystem first until SolarSystem can be configured(LOJ: 2009.02.19)
   if (name == wxT("SolarSystem") || name == wxT("Solar System"))
      return theSolarSystemInUse;
   
   // Ignore array indexing of Array
   wxString::size_type index = name.find_first_of(wxT("(["));
   if (index != name.npos)
   {
      newName = name.substr(0, index);
      
      #if DEBUG_CONFIG
      MessageInterface::ShowMessage
         (wxT("Moderator::GetConfiguredObject() entered: newName=%s\n"), newName.c_str());
      #endif
   }
   
   GmatBase *obj = theConfigManager->GetItem(newName);
   
   if (obj == NULL)
   {
      #if DEBUG_CONFIG
      MessageInterface::ShowMessage
         (wxT("   Trying SolarSystem, theSolarSystemInUse=<%p>\n"), theSolarSystemInUse);
      #endif
      
      // try SolarSystem
      if (theSolarSystemInUse)
         obj = (GmatBase*)(theSolarSystemInUse->GetBody(newName));
   }
   
   #if DEBUG_CONFIG
   if (obj)
   {
      MessageInterface::ShowMessage
         (wxT("Moderator::GetConfiguredObject() Found object: name=%s, type=%s, ")
          wxT("addr=%p\n"), obj->GetName().c_str(), obj->GetTypeName().c_str(), obj);
   }
   else
   {
      MessageInterface::ShowMessage
         (wxT("Moderator::GetConfiguredObject() Cannot find object: name=%s\n"),
          newName.c_str());
   }
   #endif
   
   return obj;
}


//------------------------------------------------------------------------------
// bool ReconfigureItem(GmatBase *newobj, const wxString &name)
//------------------------------------------------------------------------------
/*
 * Sets configured object pointer with new pointer.
 *
 * @param  newobj  New object pointer
 * @param  name  Name of the configured object to be reset
 *
 * @return  true if pointer was reset, false otherwise
 */
//------------------------------------------------------------------------------
bool Moderator::ReconfigureItem(GmatBase *newobj, const wxString &name)
{
   // Reconfigure item only if name found in the configuration.
   // Changed due to GmatFunction implementation (loj: 2008.06.25)
   if (GetConfiguredObject(name))
      return theConfigManager->ReconfigureItem(newobj, name);
   else
      return true;
}


//------------------------------------------------------------------------------
// wxString GetNewName(const wxString &name, Integer startCount)
//------------------------------------------------------------------------------
/*
 * It gives new name by adding counter to the input name.
 *
 * @param <name> Base name to used to generate new name
 * @param <startCount> Starting counter
 * @return new name
 */
//------------------------------------------------------------------------------
wxString Moderator::GetNewName(const wxString &name, Integer startCount)
{
   if (name == wxT(""))
      return wxT("");
   
   return theConfigManager->GetNewName(name, startCount);
}


//------------------------------------------------------------------------------
// wxString AddClone(const wxString &name)
//------------------------------------------------------------------------------
/*
 * Adds the clone of the named object to configuration.
 * It gives new name by adding counter to the name to be cloned.
 *
 * return new name if object was cloned and added to configuration, blank otherwise
 */
//------------------------------------------------------------------------------
wxString Moderator::AddClone(const wxString &name)
{
   if (name == wxT(""))
      return wxT("");

   return theConfigManager->AddClone(name);
}


//------------------------------------------------------------------------------
// bool RenameObject(Gmat::ObjectType type, const wxString &oldName
//                           const wxString &newName)
//------------------------------------------------------------------------------
/**
 * Renames configured item
 *
 * @param <type> object type
 * @param <oldName>  old object name
 * @param <newName>  new object name
 *
 * @return true if the item has been removed; false otherwise
 */
//------------------------------------------------------------------------------
bool Moderator::RenameObject(Gmat::ObjectType type, const wxString &oldName,
                                     const wxString &newName)
{
   #if DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("Moderator::RenameObject() type=%s, oldName=%s, newName=%s\n"),
       GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif
   
   // let's check to make sure it is a valid name
   if (!GmatStringUtil::IsValidName(newName, true))
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("'%s' is not a valid object name.\nPlease enter a different name.\n"),
         newName.c_str());
      return false;
   }
   
   // check to make sure it is not a command type
   StringArray commandNames = GetListOfFactoryItems(Gmat::COMMAND);
   for (Integer i=0; i<(Integer)commandNames.size(); i++)
   {
      if (commandNames[i] == newName)
      {
         MessageInterface::PopupMessage
            (Gmat::WARNING_, wxT("'%s' is not a valid object name.\nPlease enter a different name.\n"),
            newName.c_str());
         return false;
      }
   }
   
   #if DEBUG_RENAME
   MessageInterface::ShowMessage(wxT("   Calling theConfigManager->RenameItem()\n"));
   #endif
   bool renamed = theConfigManager->RenameItem(type, oldName, newName);
   
   //--------------------------------------------------
   // rename object name used in mission sequence
   //--------------------------------------------------
   #if DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("Moderator::RenameObject() ===> Change Command ref object names\n"));
   #endif
   
   int sandboxIndex = 0; //handles one sandbox for now
   GmatCommand *cmd = commands[sandboxIndex]->GetNext();
   GmatCommand *child;
   wxString typeName;
   
   while (renamed && cmd != NULL)
   {
      typeName = cmd->GetTypeName();
      #if DEBUG_RENAME
      MessageInterface::ShowMessage
         (wxT("...typeName=%12s, '%s'\n"), typeName.c_str(),
          cmd->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
      #endif
      
      renamed = cmd->RenameRefObject(type, oldName, newName);
      child = cmd->GetChildCommand(0);
      
      while (renamed && (child != NULL) && (child != cmd))
      {
         typeName = child->GetTypeName();
         #if DEBUG_RENAME
         MessageInterface::ShowMessage
            (wxT("......typeName=%12s, '%s'\n"), typeName.c_str(),
             cmd->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
         #endif
         if (typeName.find(wxT("End")) == typeName.npos)
            renamed = child->RenameRefObject(type, oldName, newName);
         
         child = child->GetNext();
      }
      
      cmd = cmd->GetNext();
   }
   
   #if DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("Moderator::RenameObject() rename status=%d\n"), renamed);
   #endif

   return renamed;
}

//------------------------------------------------------------------------------
// bool RemoveObject(Gmat::ObjectType type, const wxString &name,
//                           bool delIfNotUsed)
//------------------------------------------------------------------------------
/**
 * Removes item from the configured list.
 *
 * @param <type> object type
 * @param <name> object name
 * @param <delIfNotUse> flag indicating delete if item is not used in the mission
 *                      sequence
 *
 * @return true if the item has been removed; false otherwise
 */
//------------------------------------------------------------------------------
bool Moderator::RemoveObject(Gmat::ObjectType type, const wxString &name,
                             bool delOnlyIfNotUsed)
{
   GmatCommand *cmd = GetFirstCommand();
   
   #if DEBUG_REMOVE
   MessageInterface::ShowMessage
      (wxT("Moderator::RemoveObject() type=%d, name=%s, delOnlyIfNotUsed=%d\n"),
       type, name.c_str(), delOnlyIfNotUsed);
   MessageInterface::ShowMessage(GmatCommandUtil::GetCommandSeqString(cmd));
   #endif
   
   if (!delOnlyIfNotUsed)
   {
      return theConfigManager->RemoveItem(type, name);
   }
   else
   {
      // remove if object is not used in other resource
      #if DEBUG_REMOVE
      MessageInterface::ShowMessage(wxT("   Checking if '%s' is used in resource\n"), name.c_str());
      #endif
      GmatBase *obj = theConfigManager->GetFirstItemUsing(type, name);
      if (obj != NULL)
      {
         MessageInterface::ShowMessage
            (wxT("*** WARNING *** Cannot remove \"%s.\"  It is used in the %s ")
             wxT("object named \"%s\"\n"), name.c_str(), obj->GetTypeName().c_str(),
             obj->GetName().c_str());
         return false;
      }
      else
      {
         #if DEBUG_REMOVE
         MessageInterface::ShowMessage
            (wxT("   '%s' is not used in resource, checking command\n"), name.c_str());
         #endif
         // remove if object is not used in the command sequence
         wxString cmdName;
         if (GmatCommandUtil::FindObject(cmd, type, name, cmdName))
         {
            MessageInterface::ShowMessage
               (wxT("*** WARNING *** Cannot remove \"%s.\"  It is used in the %s ")
                wxT("command.\n"),  name.c_str(), cmdName.c_str());
            return false;
         }
         else
         {
            bool retval = theConfigManager->RemoveItem(type, name);
            #if DEBUG_REMOVE
            MessageInterface::ShowMessage
               (wxT("Moderator::RemoveObject() returning %d from ")
                wxT("theConfigManager->RemoveItem()\n"), retval);
            #endif
            return retval;
         }
      }
   }
}


//------------------------------------------------------------------------------
// bool HasConfigurationChanged(Integer sandboxNum = 1)
//------------------------------------------------------------------------------
bool Moderator::HasConfigurationChanged(Integer sandboxNum)
{
   bool rsrcChanged = theConfigManager->HasConfigurationChanged();
   bool cmdsChanged = commands[sandboxNum-1]->HasConfigurationChanged();
   
   #if DEBUG_CONFIG
   MessageInterface::ShowMessage
      (wxT("Moderator::HasConfigurationChanged() rsrcChanged=%d, ")
       wxT("cmdsChanged=%d\n"), rsrcChanged, cmdsChanged);
   #endif
   
   return (rsrcChanged || cmdsChanged);
}


//------------------------------------------------------------------------------
// void ConfigurationChanged(GmatBase *obj, bool tf)
//------------------------------------------------------------------------------
void Moderator::ConfigurationChanged(GmatBase *obj, bool tf)
{
   #if DEBUG_CONFIG
   MessageInterface::ShowMessage
      (wxT("Moderator::ConfigurationChanged() obj type=%s, name='%s', changed=%d\n"),
       obj->GetTypeName().c_str(), obj->GetName().c_str(), tf);
   #endif
   
   if (obj != NULL)
   {
      if (obj->IsOfType(Gmat::COMMAND))
         ((GmatCommand*)obj)->ConfigurationChanged(true);
      else
         theConfigManager->ConfigurationChanged(true);
   }
}


//------------------------------------------------------------------------------
// void ResetConfigurationChanged(bool resetResource = true,
//                                bool resetCommands = true,
//                                Integer sandboxNum = 1)
//------------------------------------------------------------------------------
void Moderator::ResetConfigurationChanged(bool resetResource, bool resetCommands,
                                          Integer sandboxNum)
{
   #if DEBUG_CONFIG
   MessageInterface::ShowMessage
      (wxT("Moderator::ResetConfigurationChanged() entered\n"));
   #endif
   
   if (resetResource)
      theConfigManager->ConfigurationChanged(false);
   
   if (resetCommands)
      SetCommandsUnchanged(sandboxNum-1);
}

// SolarSystem
//------------------------------------------------------------------------------
// SolarSystem* GetDefaultSolarSystem()
//------------------------------------------------------------------------------
/**
 * Retrieves a default solar system object pointer.
 *
 * @return a default solar system object pointer
 */
//------------------------------------------------------------------------------
SolarSystem* Moderator::GetDefaultSolarSystem()
{
   return theConfigManager->GetDefaultSolarSystem();
}


//------------------------------------------------------------------------------
// SolarSystem* CreateSolarSystem(const wxString &name)
//------------------------------------------------------------------------------
SolarSystem* Moderator::CreateSolarSystem(const wxString &name)
{
   #if DEBUG_SOLAR_SYSTEM
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateSolarSystem() creating '%s'\n"), name.c_str());
   #endif
   
   // There is no factory to create SolarSystem so just create by new
   // SolarSystem constructor Creates available planetary ephem sorce list.
   // Also set to DE405 as a default planetary ephem source
   SolarSystem *ss = new SolarSystem(name);
   
   #ifdef DEBUG_MEMORY
   wxString funcName;
   funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
   MemoryTracker::Instance()->Add
      (ss, name, wxT("Moderator::CreateSolarSystem()"), funcName);
   #endif
   
   #if DEBUG_SOLAR_SYSTEM
   MessageInterface::ShowMessage(wxT("Moderator::CreateSolarSystem() returning %p\n"), ss);
   #endif
   
   return ss;
}


//------------------------------------------------------------------------------
// SolarSystem* GetSolarSystemInUse(Integer manage = 1)
//------------------------------------------------------------------------------
/*
 * Returns SolarSystem in use from configuration or object map in use
 *
 * @param  manage  If value is 1 it will return SolarSystem from the configuration
 *                 if value is 2 it will return SolarSystem from the object map in use
 */
//------------------------------------------------------------------------------
SolarSystem* Moderator::GetSolarSystemInUse(Integer manage)
{
   #if DEBUG_SOLAR_SYSTEM_IN_USE
   MessageInterface::ShowMessage
      (wxT("Moderator::GetSolarSystemInUse() entered, manage=%d, objectMapInUse=<%p>, ")
       wxT("theInternalSolarSystem=<%p>\n"), manage, objectMapInUse, theInternalSolarSystem);
   #endif
   
   SolarSystem *ss = NULL;
   if (manage == 1)
   {
      ss = theConfigManager->GetSolarSystemInUse();
      #if DEBUG_SOLAR_SYSTEM_IN_USE
      MessageInterface::ShowMessage(wxT("   Using SolarSystem from configuration <%p>\n"), ss);
      #endif
   }
   else
   {
      ObjectMap::iterator pos = objectMapInUse->find(wxT("SolarSystem"));
      if (pos != objectMapInUse->end())
      {
         ss = (SolarSystem*)pos->second;
         #if DEBUG_SOLAR_SYSTEM_IN_USE
         MessageInterface::ShowMessage(wxT("   Using SolarSystem from objectMapInUse <%p>\n"), ss);
         #endif
      }
      
      if (ss == NULL)
      {
         ss = theInternalSolarSystem;
         #if DEBUG_SOLAR_SYSTEM_IN_USE
         MessageInterface::ShowMessage(wxT("   Using Internal SolarSystem <%p>\n"), ss);
         #endif
      }      
   }
   
   // if SolarSystem is NULL, there is some problem
   if (ss == NULL)
      throw GmatBaseException
         (wxT("Moderator::GetSolarSystemInUse() The SolarSystem in use is UNSET.\n"));
   
   #if DEBUG_SOLAR_SYSTEM_IN_USE
   MessageInterface::ShowMessage
      (wxT("Moderator::GetSolarSystemInUse() returning <%p>\n"), ss);
   #endif
   
   return ss;
}


//------------------------------------------------------------------------------
// void SetSolarSystemInUse(SolarSystem *ss)
//------------------------------------------------------------------------------
void Moderator::SetSolarSystemInUse(SolarSystem *ss)
{
   if (ss != NULL)
      theConfigManager->SetSolarSystemInUse(ss);
   else
      throw GmatBaseException
         (wxT("Moderator::SetSolarSystemInUse() cannot set NULL SolarSystem\n"));
}


//------------------------------------------------------------------------------
// void SetInternalSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
/*
 * Sets the internal solar system. The internal solar system is initially set to
 * theSolarSystemInUse for creating main objects and commands. When creating
 * objects and commands for GmatFunction, it will use the solar system cloned in
 * the Sandbox during the Sandbox initialization. Setting this internal solar
 * system happens during the GmatFunction initialization.
 */
//------------------------------------------------------------------------------
void Moderator::SetInternalSolarSystem(SolarSystem *ss)
{
   if (ss != NULL)
   {
      #if DEBUG_SOLAR_SYSTEM_IN_USE
      MessageInterface::ShowMessage
         (wxT("Moderator::SetInternalSolarSystem() entered, ss=<%p>'%s'\n"),
          ss, ss->GetName().c_str());
      #endif
      
      theInternalSolarSystem = ss;
   }
}


//------------------------------------------------------------------------------
// bool SetSolarSystemInUse(const wxString &name)
//------------------------------------------------------------------------------
bool Moderator::SetSolarSystemInUse(const wxString &name)
{
   return theConfigManager->SetSolarSystemInUse(name);
}


// Create object
//------------------------------------------------------------------------------
// GmatBase* CreateOtherObject(Gmat::ObjectType objType, const wxString &type,
//                             const wxString &name, bool createDefault)
//------------------------------------------------------------------------------
GmatBase* Moderator::CreateOtherObject(Gmat::ObjectType objType, const wxString &type,
                                       const wxString &name, bool createDefault)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateOtherObject() objType=%d, type='%s', name='%s', createDefault=%d, ")
       wxT("objectManageOption=%d\n"), objType, type.c_str(), name.c_str(), createDefault,
       objectManageOption);
   #endif
   
   GmatBase *obj = NULL;
   if (FindObject(name) == NULL)
   {
      obj = theFactoryManager->CreateObject(objType, type, name);
      
      if (obj == NULL)
      {
         throw GmatBaseException
            (wxT("The Moderator cannot create an object of type \"") + type + wxT("\"\n"));
      }
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateOtherObject()"), funcName);
      }
      #endif
      
      // Manage it if it is a named object
      try
      {
         if (name != wxT("") && objectManageOption == 1)
         {
            theConfigManager->AddObject(objType, obj);
            #if DEBUG_CREATE_RESOURCE
            MessageInterface::ShowMessage(wxT("   ==> '%s' added to configuration\n"), name.c_str());
            #endif
         }
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(wxT("Moderator::CreateOtherObject()\n") +
                                       e.GetFullMessage());
      }
      
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateOtherObject() returning <%p>\n"), obj);
      #endif
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateOtherObject() Unable to create an object ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
      return FindObject(name);
   }
}


// CalculatedPoint
//------------------------------------------------------------------------------
// CalculatedPoint* CreateCalculatedPoint(const wxString &type,
//                                        const wxString &name,
//                                        bool addDefaultBodies = true)
//------------------------------------------------------------------------------
/**
 * Creates a calculated point object by given type and name.
 *
 * @param <type> object type
 * @param <name> object name
 * @param <addDefaultBodies> true if add default bodies requested (true)
 *
 * @return a CalculatedPoint object pointer
 */
//------------------------------------------------------------------------------
CalculatedPoint* Moderator::CreateCalculatedPoint(const wxString &type,
                                                  const wxString &name,
                                                  bool addDefaultBodies)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateCalculatedPoint() type='%s', name='%s', ")
       wxT("addDefaultBodies=%d, objectManageOption=%d\n"), type.c_str(), name.c_str(),
       addDefaultBodies, objectManageOption);
   #endif
   
   if (GetCalculatedPoint(name) == NULL)
   {
      #ifdef DEBUG_CREATE_CALC_POINT
         MessageInterface::ShowMessage(wxT("Moderator::Creating new %s named %s\n"),
               type.c_str(), name.c_str());
      #endif
      CalculatedPoint *obj = theFactoryManager->CreateCalculatedPoint(type, name);
      
      if (obj == NULL)
      {
         throw GmatBaseException
            (wxT("The Moderator cannot create a CalculatedPoint type \"") + type + wxT("\"\n"));
      }
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateCalculatedPoint()"), funcName);
      }
      #endif
      
      // add default bodies
      if (type == wxT("LibrationPoint"))
      {
         if (addDefaultBodies)
         {
            obj->SetStringParameter(wxT("Primary"), wxT("Sun"));
            obj->SetStringParameter(wxT("Point"), wxT("L1"));
            obj->SetStringParameter(wxT("Secondary"), wxT("Earth"));
            
            #ifdef __CREATE_DEFAULT_BC__
               // first create default Earth-Moon Barycenter
               CalculatedPoint *defBc = GetCalculatedPoint(wxT("DefaultBC"));

               if (defBc == NULL)
                  defBc = CreateCalculatedPoint(wxT("Barycenter"), wxT("DefaultBC"));

               obj->SetStringParameter(wxT("Secondary"), wxT("DefaultBC"));
               obj->SetRefObject(defBc, Gmat::SPACE_POINT, wxT("DefaultBC"));
            #endif
            
            // Set body and J2000Body pointer, so that GUI can create LibrationPoint
            // and use it in Coord.System conversion
            SpacePoint *sun = (SpacePoint*)FindObject(wxT("Sun"));
            SpacePoint *earth = (SpacePoint*)FindObject(wxT("Earth"));
            
            if (sun->GetJ2000Body() == NULL)
               sun->SetJ2000Body(earth);
            
            #if DEBUG_CREATE_RESOURCE
            MessageInterface::ShowMessage
               (wxT("Moderator::Setting sun <%p> and earth <%p> to LibrationPoint %s\n"), sun, earth, name.c_str());
            #endif
            
            obj->SetRefObject(sun, Gmat::SPACE_POINT, wxT("Sun"));
            obj->SetRefObject(earth, Gmat::SPACE_POINT, wxT("Earth"));
         }
      }
      else if (type == wxT("Barycenter"))
      {
         if (addDefaultBodies)
         {
//            obj->SetStringParameter("BodyNames", "Earth");
            ((CalculatedPoint*) obj)->SetDefaultBody(wxT("Earth"));
            ((CalculatedPoint*) obj)->SetDefaultBody(wxT("Luna"));

            // Set body and J2000Body pointer, so that GUI can create LibrationPoint
            // and use it in Coord.System conversion
//            SpacePoint *earth = (SpacePoint*)FindObject("Earth");
//            obj->SetRefObject(earth, Gmat::SPACE_POINT, "Earth");

            // obj->SetStringParameter("BodyNames", "Luna");
            // SpacePoint *luna = (SpacePoint*)FindObject("Luna");
            // if (luna->GetJ2000Body() == NULL)
            //    luna->SetJ2000Body(earth);
            // obj->SetRefObject(luna, Gmat::SPACE_POINT, "Luna");
         }
      }
      
      // Manage it if it is a named CalculatedPoint
      try
      {
         if (name != wxT("") && objectManageOption == 1)
            theConfigManager->AddCalculatedPoint(obj);
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(wxT("Moderator::CreateCalculatedPoint()\n") +
                                       e.GetFullMessage());
      }
      
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateCalculatedPoint() Unable to create CalculatedPoint ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
      return GetCalculatedPoint(name);
   }
}


//------------------------------------------------------------------------------
// CalculatedPoint* GetCalculatedPoint(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a calclulated point object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a CalculatedPoint object pointer, return null if name not found
 */
//------------------------------------------------------------------------------
CalculatedPoint* Moderator::GetCalculatedPoint(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (CalculatedPoint*)FindObject(name);
}


// CelestialBody
//------------------------------------------------------------------------------
// CelestialBody* CreateCelestialBody(const wxString &type,
//                                    const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a celestial body object by given type and name.
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return a CelestialBody object pointer
 */
//------------------------------------------------------------------------------
CelestialBody* Moderator::CreateCelestialBody(const wxString &type,
                                              const wxString &name)
{
   #ifdef DEBUG_CREATE_BODY
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateCelestialBody() called with type = '%s' and name = '%s'\n"),
       type.c_str(), name.c_str());
   if (GetCelestialBody(name) != NULL) 
      MessageInterface::ShowMessage(wxT("... that body alreday exists\n"));
   #endif
   if (GetCelestialBody(name) == NULL)
   {
      CelestialBody *obj = theFactoryManager->CreateCelestialBody(type, name);
      
      if (obj == NULL)
         throw GmatBaseException
            (wxT("The Moderator cannot create a CelestialBody type \"") + type + wxT("\"\n"));
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateCelestialBody()"), funcName);
      }
      #endif
      
      // Add new celestial body to solar syatem in use
      Integer manage = 0; // Just set other than 1 here
      SolarSystem *ss = GetSolarSystemInUse(manage);
      obj->SetUserDefined(true);
      obj->SetSolarSystem(ss);
      obj->SetUpBody(); //Added so that it works inside a GmatFunction (LOJ: 2010.04.30)
      ss->AddBody(obj);
      #ifdef DEBUG_CREATE_BODY
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateCelestialBody() Created CelestialBody <%p> ")
          wxT("\"%s\" and added to Solar System <%p>\n"), obj, name.c_str(), ss);
      #endif
      
      // Manually set configuration changed to true here since
      // SolarSystem is not configured yet
      theConfigManager->ConfigurationChanged(true);
      
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateCelestialBody() Unable to create CelestialBody ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
      return GetCelestialBody(name);
   }
}


//------------------------------------------------------------------------------
// CelestialBody* GetCelestialBody(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a celestial body object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a CelestialBody object pointer, return null if name not found
 */
//------------------------------------------------------------------------------
CelestialBody* Moderator::GetCelestialBody(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (CelestialBody*)FindObject(name);
}


// Spacecraft
//------------------------------------------------------------------------------
// SpaceObject* CreateSpacecraft(const wxString &type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a spacecraft object by given name.
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return spacecraft object pointer
 */
//------------------------------------------------------------------------------
SpaceObject* Moderator::CreateSpacecraft(const wxString &type,
                                         const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateSpacecraft() type = '%s', name = '%s'\n"),
       type.c_str(), name.c_str());
   #endif
   
   if (GetSpacecraft(name) == NULL)
   {
      Spacecraft *obj = (Spacecraft*)(theFactoryManager->CreateSpacecraft(type, name));
      
      if (obj == NULL)
         throw GmatBaseException
            (wxT("The Moderator cannot create a Spacecraft type \"") + type + wxT("\"\n"));
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateSpacecraft()"), funcName);
      }
      #endif
      
      // Create internal and default CoordinateSystems if they do not exist.
      // This code will make new mission to work after script errors occur.
      // This change was made while looking at Bug 1532 (LOJ: 2009.11.13)
      if (theInternalCoordSystem == NULL)
         CreateInternalCoordSystem();
      CreateDefaultCoordSystems();
      // Create the default Solar System barycenter
      CreateDefaultBarycenter();
      
      if (type == wxT("Spacecraft"))
      {
         // Set internal and default CoordinateSystem
         obj->SetInternalCoordSystem(theInternalCoordSystem);
         obj->SetRefObjectName(Gmat::COORDINATE_SYSTEM, wxT("EarthMJ2000Eq"));
      }
      
      // Manage it if it is a named Spacecraft
      try
      {
         if (name != wxT("") && objectManageOption == 1)
            theConfigManager->AddSpacecraft(obj);
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(wxT("Moderator::CreateSpacecraft()\n") +
                                       e.GetFullMessage());
      }
      
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateSpacecraft() returning <%p>'%s'\n"), obj,
          obj->GetName().c_str());
      #endif
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateSpacecraft() Unable to create Spacecraft ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
      return GetSpacecraft(name);
   }
}


//------------------------------------------------------------------------------
// SpaceObject* GetSpacecraft(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a spacecraft object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a SpaceObject object pointer, return null if name not found
 */
//------------------------------------------------------------------------------
SpaceObject* Moderator::GetSpacecraft(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else      
      return (SpaceObject*)FindObject(name);
}


//------------------------------------------------------------------------------
// wxString GetSpacecraftNotInFormation()
//------------------------------------------------------------------------------
/**
 * This method finds the first spacecraft name sorted by ascending order not
 * in any formation.
 *
 * @return  The first spacecraft name not in any formation.
 *          return "" if such spacecraft is not found.
 */
//------------------------------------------------------------------------------
wxString Moderator::GetSpacecraftNotInFormation()
{
   StringArray scList = GetListOfObjects(Gmat::SPACECRAFT);
   StringArray fmList = GetListOfObjects(Gmat::FORMATION);
   int numSc = scList.size(), numFm = fmList.size();
   
   if (numSc == 0 && numFm == 0)
      return wxT("");
   
   if (numSc > 0 && numFm == 0)
      return GetDefaultSpacecraft()->GetName();
   
   // formation exists
   StringArray fmscListAll;
   
   //------------------------------------------
   // Merge spacecrafts in Formation
   //------------------------------------------
   for (int i=0; i<numFm; i++)
   {
      GmatBase *fm = GetConfiguredObject(fmList[i]);
      StringArray fmscList = fm->GetStringArrayParameter(fm->GetParameterID(wxT("Add")));
      fmscListAll.insert(fmscListAll.begin(), fmscList.begin(), fmscList.end());
   }
   
   // sort the lists in order to  set_difference()
   sort(scList.begin(), scList.end());
   sort(fmscListAll.begin(), fmscListAll.end());
   
   //------------------------------------------
   // Make list of spacecrafts not in Formation
   //------------------------------------------
   StringArray scsNotInForms;
   
   // The set_difference() algorithm produces a sequence that contains the
   // difference between the two ordered sets.
   set_difference(scList.begin(), scList.end(), fmscListAll.begin(),
                  fmscListAll.end(), back_inserter(scsNotInForms));
   
   if (scsNotInForms.size() > 0)
      return scsNotInForms[0];
   else
      return wxT("");
}


// SpacePoints
//------------------------------------------------------------------------------
// SpacePoint* CreateSpacePoint(const wxString &type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a spacepoint object by given name.
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return SpacePoint object pointer
 */
//------------------------------------------------------------------------------
SpacePoint* Moderator::CreateSpacePoint(const wxString &type,
                                        const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateSpacePoint() type = '%s', name = '%s'\n"),
       type.c_str(), name.c_str());
   #endif
   
   if (GetSpacePoint(name) == NULL)
   {
      SpacePoint *obj = (SpacePoint*)(theFactoryManager->CreateSpacePoint(type, name));
      
      if (obj == NULL)
      {
         throw GmatBaseException
            (wxT("The Moderator cannot create a SpacePoint type \"") + type + wxT("\"\n"));
      }
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateSpacePoint()"), funcName);
      }
      #endif
            
      // Manage it if it is a named SpacePoint
      try
      {
         if (name != wxT("") && objectManageOption == 1)
            theConfigManager->AddSpacePoint(obj);
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(wxT("Moderator::CreateSpacePoint()\n") +
                                       e.GetFullMessage());
      }
   
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateSpacePoint() Unable to create SpacePoint ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
      return GetSpacePoint(name);
   }
}

//------------------------------------------------------------------------------
// SpacePoint* GetSpacePoint(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a SpacePoint object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a SpacePoint object pointer, return null if name not found
 */
//------------------------------------------------------------------------------
SpacePoint* Moderator::GetSpacePoint(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (SpacePoint*)FindObject(name);
}


// Hardware
//------------------------------------------------------------------------------
// Hardware* CreateHardware(const wxString &type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a Hardware object by given name.
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return Hardware object pointer
 */
//------------------------------------------------------------------------------
Hardware* Moderator::CreateHardware(const wxString &type, const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateHardware() type = '%s', name = '%s'\n"),
       type.c_str(), name.c_str());
   #endif
   
   if (GetHardware(name) == NULL)
   {
      Hardware *obj = theFactoryManager->CreateHardware(type, name);
      
      if (obj == NULL)
      {
         throw GmatBaseException
            (wxT("The Moderator cannot create a Hardware type \"") + type + wxT("\"\n"));
      }
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateHardware()"), funcName);
      }
      #endif
      
      // Manage it if it is a named Hardware
      try
      {
         if (name != wxT("") && objectManageOption == 1)
            theConfigManager->AddHardware(obj);
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(wxT("Moderator::CreateHardware()\n") +
                                       e.GetFullMessage());
      }
      
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateHardware() Unable to create Hardware ")
          wxT("name: \"%s\" already exists\n"), name.c_str());
      #endif
      return GetHardware(name);
   }
}


//------------------------------------------------------------------------------
// Hardware* GetHardware(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a Hardware object pointer by given name and add to configuration.
 *
 * @param <name> object name
 *
 * @return a Hardware object pointer, return null if name not found
 */
//------------------------------------------------------------------------------
Hardware* Moderator::GetHardware(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (Hardware*)FindObject(name);
}


// Propagator
//------------------------------------------------------------------------------
// Propagator* CreatePropagator(const wxString &type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a propagator object by given type and name. Actually this creates
 * Integrator.
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return a propagator object pointer
 */
//------------------------------------------------------------------------------
Propagator* Moderator::CreatePropagator(const wxString &type,
                                        const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreatePropagator() type = '%s', name = '%s'\n"),
       type.c_str(), name.c_str());
   #endif
   
   // GMAT doesn't name propagators, so we don't need to check the configuration
   // for them.  PropSetups are the only things that get named for propagation.
   Propagator *obj = theFactoryManager->CreatePropagator(type, name);
      
   if (obj ==  NULL)
      throw GmatBaseException
         (wxT("The Moderator cannot create a Propagator type \"") + type + wxT("\"\n"));
      
   #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreatePropagator()"), funcName);
      }
   #endif
      
   #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreatePropagator() returning new Propagator <%p>'%s'\n"),
          obj, obj->GetName().c_str());
      #endif
      
      return obj;
}

//------------------------------------------------------------------------------
// Propagator* GetPropagator(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a propagator object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a propagator object pointer, return null if name not found
 */
//------------------------------------------------------------------------------
Propagator* Moderator::GetPropagator(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (Propagator*)FindObject(name);
}

// PhysicalModel
//------------------------------------------------------------------------------
// PhysicalModel* CreateDefaultPhysicalModel(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a default physical model of full Earth gravity force with JGM2 file.
 *
 * @param <name> object name
 *
 * @return a physical model object pointer
 */
//------------------------------------------------------------------------------
PhysicalModel* Moderator::CreateDefaultPhysicalModel(const wxString &name)
{
   wxString type = wxT("GravityField");
   
   if (GetPhysicalModel(name) == NULL)
   {
      PhysicalModel *obj =
         theFactoryManager->CreatePhysicalModel(type, name);
      
      if (obj ==  NULL)
         throw GmatBaseException
               (wxT("The Moderator cannot create a PhysicalModel type \"") + type +
                wxT("\"\n"));
      
      // set the EOP file, since it's a GravityField object
      HarmonicField *hf = (HarmonicField*) obj;
      hf->SetEopFile(theEopFile);

      #ifdef DEBUG_MEMORY
      wxString funcName;
      funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
      MemoryTracker::Instance()->Add
         (obj, name, wxT("Moderator::CreateDefaultPhysicalModel()"), funcName);
      #endif
      
      SolarSystem *ss = GetSolarSystemInUse(objectManageOption);
      obj->SetName(wxT("Earth"));
      obj->SetSolarSystem(ss);
      obj->SetBody(wxT("Earth"));
      obj->SetBodyName(wxT("Earth"));
      
      if (type == wxT("GravityField"))
         obj->SetStringParameter(wxT("PotentialFile"), GetFileName(wxT("JGM2_FILE")));
      
      // Manage it if it is a named PhysicalModel
      try
      {
         if (name != wxT("") && objectManageOption == 1)
            theConfigManager->AddPhysicalModel(obj);
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(wxT("Moderator::CreatePhysicalModel()\n") +
                                       e.GetFullMessage());
      }
      
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreatePhysicalModel() Unable to create PhysicalModel ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
      return GetPhysicalModel(name);
   }
}

//------------------------------------------------------------------------------
// PhysicalModel* CreatePhysicalModel(const wxString &type,
//                                    const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a physical model object by given type and name.
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return a physical model object pointer
 */
//------------------------------------------------------------------------------
PhysicalModel* Moderator::CreatePhysicalModel(const wxString &type,
                                              const wxString &name)
{
   #ifdef DEBUG_CREATE_PHYSICAL_MODEL
      MessageInterface::ShowMessage(wxT("Now attempting to create a PhysicalModel of type %s with name %s\n"),
            type.c_str(), name.c_str());
   #endif
   PhysicalModel *obj = GetPhysicalModel(name);
   if (obj == NULL)
   {
      obj = theFactoryManager->CreatePhysicalModel(type, name);
      
      if (obj ==  NULL)
         throw GmatBaseException
            (wxT("The Moderator cannot create a PhysicalModel type \"") + type + wxT("\"\n"));
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreatePhysicalModel()"), funcName);
      }
      #endif
      
      // Manage it if it is a named PhysicalModel
      try
      {
         if (name != wxT("") && objectManageOption == 1)
            theConfigManager->AddPhysicalModel(obj);
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(wxT("Moderator::CreatePhysicalModel()\n") +
                                       e.GetFullMessage());
      }
      
//      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreatePhysicalModel() Unable to create PhysicalModel ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
//      return GetPhysicalModel(name);
   }
   if ((obj != NULL) && obj->IsOfType(wxT("HarmonicField")))
   {
      HarmonicField *hf = (HarmonicField*) obj;
      hf->SetEopFile(theEopFile);
   }
   if ((obj != NULL) && obj->IsOfType(wxT("RelativisticCorrection")))
   {
      RelativisticCorrection *rc = (RelativisticCorrection*) obj;
      rc->SetEopFile(theEopFile);
   }
   return obj;
}

//------------------------------------------------------------------------------
// PhysicalModel* GetPhysicalModel(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a physical model object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a physical model object pointer, return null if name not found
 */
//------------------------------------------------------------------------------
PhysicalModel* Moderator::GetPhysicalModel(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (PhysicalModel*)FindObject(name);
}

// AtmosphereModel
//------------------------------------------------------------------------------
// AtmosphereModel* CreateAtmosphereModel(const wxString &type,
//                                        const wxString &name,
//                                        const wxString &body = wxT("Earth"))
//------------------------------------------------------------------------------
/**
 * Creates an atmosphere model object by given type and name and add to
 * configuration.
 *
 * @param <type> object type
 * @param <name> object name
 * @param <body> the body for which the atmosphere model is requested
 *
 * @return a atmosphereModel object pointer
 */
//------------------------------------------------------------------------------
AtmosphereModel* Moderator::CreateAtmosphereModel(const wxString &type,
                                                  const wxString &name,
                                                  const wxString &body)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateAtmosphereModel() type = '%s', name = '%s', body = '%s'\n"),
       type.c_str(), name.c_str(), body.c_str());
   #endif
   
   // if AtmosphereModel name doesn't exist, create AtmosphereModel
   if (GetAtmosphereModel(name) == NULL)
   {
      AtmosphereModel *obj =
         theFactoryManager->CreateAtmosphereModel(type, name, body);
      
      if (obj ==  NULL)
         throw GmatBaseException
            (wxT("The Moderator cannot create an AtmosphereModel type \"") + type + wxT("\"\n"));
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateAtmosphereModel()"), funcName);
      }
      #endif
      
      // Manage it if it is a named AtmosphereModel
      try
      {
         if (name != wxT("") && objectManageOption == 1)
            theConfigManager->AddAtmosphereModel(obj);
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage
            (wxT("Moderator::CreateAtmosphereModel()\n") + e.GetFullMessage());
      }
    
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateAtmosphereModel() Unable to create AtmosphereModel ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
      return GetAtmosphereModel(name);
   }
}

//------------------------------------------------------------------------------
// AtmosphereModel* GetAtmosphereModel(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves an atmosphere model object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a AtmosphereModel pointer, return null if name not found
 */
//------------------------------------------------------------------------------
AtmosphereModel* Moderator::GetAtmosphereModel(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (AtmosphereModel*)FindObject(name);
}

// Burn
//------------------------------------------------------------------------------
// Burn* CreateBurn(const wxString &type, const wxString &name,
//                  bool createDefault)
//------------------------------------------------------------------------------
/**
 * Creates a burn object by given type and name and add to configuration.
 * If createDefault is true, it will create "Local" coordinate system with
 * "VNB" axes. Usually this flag is set to true if ImpulsiveBurn object is
 * created from the GUI.
 *
 * @param <type> object type
 * @param <name> object name
 * @param <createDefault> set to true if default burn to be created (false)
 *
 * @return a burn object pointer
 */
//------------------------------------------------------------------------------
Burn* Moderator::CreateBurn(const wxString &type,
                            const wxString &name, bool createDefault)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateBurn() type = '%s, name = '%s'\n"), type.c_str(),
       name.c_str());
   #endif
   
   // if Burn name doesn't exist, create Burn
   if (GetBurn(name) == NULL)
   {
      Burn *obj = theFactoryManager->CreateBurn(type, name);
      
      if (obj ==  NULL)
         throw GmatBaseException
            (wxT("The Moderator cannot create Burn type \"") + type + wxT("\"\n"));
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateBurn()"), funcName);
      }
      #endif
      
      // Set default Axes to VNB
      if (createDefault)
      {
         obj->SetStringParameter(obj->GetParameterID(wxT("CoordinateSystem")), wxT("Local"));
         obj->SetStringParameter(obj->GetParameterID(wxT("Axes")), wxT("VNB"));
      }
      
      // Manage it if it is a named Burn
      try
      {
         if (name != wxT("") && objectManageOption == 1)
            theConfigManager->AddBurn(obj);
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(wxT("Moderator::CreateBurn()\n") +
                                       e.GetFullMessage());
      }
      
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateBurn() Unable to create Burn ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
      return GetBurn(name);
   }
}

//------------------------------------------------------------------------------
// Burn* GetBurn(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a burn object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a burn pointer, return null if name not found
 */
//------------------------------------------------------------------------------
Burn* Moderator::GetBurn(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (Burn*)FindObject(name);
}

// Parameter
//------------------------------------------------------------------------------
// bool Moderator::IsParameter(const wxString &str)
//------------------------------------------------------------------------------
/**
 * Checks to see if a given type is a Parameter. If str has '.', it parses
 * string to get type before checking.
 *
 * @param <str> object type string
 *
 * @return true if the type is a registered parameter, false if not.
 */
//------------------------------------------------------------------------------
bool Moderator::IsParameter(const wxString &str)
{
   if (str == wxT("")) return false;
   StringArray sar = theFactoryManager->GetListOfItems(Gmat::PARAMETER);
   wxString type;
   
   if (str.find(wxT(".")) == str.npos)
   {
      type = str;
   }
   else
   {
      wxString ownerName, depObj;
      GmatStringUtil::ParseParameter(str, type, ownerName, depObj);
   }
   
   if (find(sar.begin(), sar.end(), type) != sar.end()) {
      #ifdef DEBUG_LOOKUP_RESOURCE
      MessageInterface::ShowMessage(wxT("Found parameter \"%s\"\n"), type.c_str());
      #endif
      return true;
   }

   #ifdef DEBUG_LOOKUP_RESOURCE
   MessageInterface::ShowMessage(wxT("Could not find parameter \"%s\"\n"),
                                 type.c_str());
   #endif
   
   return false;   
}


//------------------------------------------------------------------------------
// Parameter* CreateAutoParameter(const wxString &type, const wxString &name
//                            const wxString &ownerName,
//                            const wxString &depName, bool manage)
//------------------------------------------------------------------------------
/**
 * Creates a parameter object by given type and name and add to configuration.
 *
 * @param <type> parameter type
 * @param <name> parameter name
 * @param <ownerName> parameter owner name (wxT(""))
 * @param <depName> dependent object name (wxT(""))
 * @param <manage>  0, if parameter is not managed
 *                  1, if parameter is added to configuration (default)
 *                  2, if Parameter is added to function object map
 *
 * @return a parameter object pointer
 */
//------------------------------------------------------------------------------
Parameter* Moderator::CreateAutoParameter(const wxString &type,
                                          const wxString &name,
                                          bool &alreadyManaged,
                                          const wxString &ownerName,
                                          const wxString &depName,
                                          Integer manage)
{
   #if DEBUG_CREATE_PARAMETER
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateAutoParameter() type='%s', name='%s', ownerName='%s', ")
       wxT("depName='%s', manage=%d\n"), type.c_str(), name.c_str(), ownerName.c_str(),
       depName.c_str(), manage);
   #endif
   
   // if managing and Parameter already exist, give warning and return existing
   // Parameter
   alreadyManaged = false;
   Parameter *param = GetParameter(name);
   #if DEBUG_CREATE_PARAMETER
   MessageInterface::ShowMessage
      (wxT("   managed param = <%p> '%s'\n", param, param ? param->GetName().c_str() : "NULL"));
   #endif
   
   // if Parameter was created during GmatFunction parsing, just set reference object
   if (param != NULL && manage != 0)
   {
      #if DEBUG_CREATE_PARAMETER
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** Moderator::CreateAutoParameter() Unable to create ")
          wxT("Parameter name: %s already exist\n"), name.c_str());
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateAutoParameter() returning <%s><%p>\n"), param->GetName().c_str(),
          param);
      #endif
      
      // set Parameter reference object
      SetParameterRefObject(param, type, name, ownerName, depName, manage);
      
      // if Parameter is managed in the function object map, add it (loj: 2008.12.16)
      // so that we won't create multiple Parameters. FindObject() finds object from
      // objectMapInUse which can be object map from configuration or passed function
      // object map
      if (manage == 2)
         AddObject(param);
      
      alreadyManaged = true;
      return param;
   }
   
   return CreateParameter(type, name, ownerName, depName, manage);
}


//------------------------------------------------------------------------------
// Parameter* CreateParameter(const wxString &type, const wxString &name
//                            const wxString &ownerName = "",
//                            const wxString &depName = "", bool manage = 1)
//------------------------------------------------------------------------------
/**
 * Creates a parameter object by given type and name and add to configuration.
 *
 * @param <type> parameter type
 * @param <name> parameter name
 * @param <ownerName> parameter owner name ("")
 * @param <depName> dependent object name ("")
 * @param <manage>  0, if parameter is not managed
 *                  1, if parameter is added to configuration (default)
 *                  2, if Parameter is added to function object map
 *
 * @return a parameter object pointer
 *
 * @note manage option will override member data objectManageOption so do not
 * use objectManageOption here.
 */
//------------------------------------------------------------------------------
Parameter* Moderator::CreateParameter(const wxString &type,
                                      const wxString &name,
                                      const wxString &ownerName,
                                      const wxString &depName,
                                      Integer manage)
{   
   #if DEBUG_CREATE_PARAMETER
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateParameter() type='%s', name='%s', ownerName='%s', ")
       wxT("depName='%s', manage=%d\n"), type.c_str(), name.c_str(), ownerName.c_str(),
       depName.c_str(), manage);
   MessageInterface::ShowMessage(wxT("   objectManageOption=%d\n"), objectManageOption);
   #endif
   
   // if managing and Parameter already exist, give warning and return existing
   // Parameter
   Parameter *param = GetParameter(name);
   #if DEBUG_CREATE_PARAMETER
   MessageInterface::ShowMessage
      (wxT("   managed param = <%p> '%s'\n", param, param ? param->GetName().c_str() : "NULL"));
   #endif
   
   // if Parameter was created during GmatFunction parsing, just set reference object
   if (param != NULL && manage != 0)
   {
      #if DEBUG_CREATE_PARAMETER
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** Moderator::CreateParameter() Unable to create ")
          wxT("Parameter name: %s already exist\n"), name.c_str());
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateParameter() returning <%s><%p>\n"), param->GetName().c_str(),
          param);
      #endif
      
      // set Parameter reference object
      SetParameterRefObject(param, type, name, ownerName, depName, manage);
      
      // if Parameter is managed in the function object map, add it (loj: 2008.12.16)
      // so that we won't create multiple Parameters. FindObject() finds object from
      // objectMapInUse which can be object map from configuration or passed function
      // object map
      if (manage == 2)
         AddObject(param);
      
      return param;
   }
   
   // Check for deprecated Element* on Thruster, new Parameters are ThrustDirection*
   wxString newType = type;
   if (type == wxT("Element1") || type == wxT("Element2") || type == wxT("Element3"))
   {
      Integer numDots = GmatStringUtil::NumberOfOccurrences(name, wxT('.'));
      if (numDots > 1)
      {
         newType = GmatStringUtil::Replace(newType, wxT("Element"), wxT("ThrustDirection"));
         #if DEBUG_CREATE_PARAMETER
         MessageInterface::ShowMessage
            (wxT("   Parameter type '%s' in '%s' changed to '%s'\n"), type.c_str(),
             name.c_str(), newType.c_str());
         #endif
      }
   }
   
   // Ceate new Parameter
   param = theFactoryManager->CreateParameter(newType, name);
   
   #ifdef DEBUG_MEMORY
   if (param)
   {
      wxString funcName;
      funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
      MemoryTracker::Instance()->Add
         (param, name, wxT("Moderator::CreateParameter()"), funcName);
   }
   #endif
   
   #if DEBUG_CREATE_PARAMETER
   MessageInterface::ShowMessage
      (wxT("   new param = <%p><%s>\n", param, param ? param->GetName().c_str() : "NULL"));
   #endif
   
   if (param == NULL)
   {
      throw GmatBaseException
         (wxT("The Moderator cannot create a Parameter type \"") + newType +
          wxT("\" named \"") + name + wxT("\"\n"));
   }
   
   // We don't know the owner type the parameter before create,
   // so validate owner type after create.
   if (ownerName != wxT("") && manage != 0)
      CheckParameterType(param, newType, ownerName);
   
   // set Parameter reference object
   SetParameterRefObject(param, newType, name, ownerName, depName, manage);
   
   // Add to configuration if manage flag is true and it is a named parameter
   try
   {
      // check if object is to be managed in configuration(loj: 2008.03.18)
      // @note Do not use objectManageOption here since manage flag overrides
      // this option for automatic objects such as Paramters
      if (manage == 1)
      {
         bool oldFlag = theConfigManager->HasConfigurationChanged();
         
         if (param->GetName() != wxT(""))
            theConfigManager->AddParameter(param);
         
         // if system paramter, set configuration changed to old flag.
         if (param->GetKey() == GmatParam::SYSTEM_PARAM)
         {
            #if DEBUG_CREATE_PARAMETER
            MessageInterface::ShowMessage
               (wxT("   Resetting configuration changed flag for SystemParameter ")
                wxT("'%s' to %d\n"), param->GetName().c_str(), oldFlag);
            #endif
            
            theConfigManager->ConfigurationChanged(oldFlag);
         }
      }
      else if (manage == 2) //LOJ: Add object if managed in FOS(2009.03.09)
      {
         AddObject(param);
      }
   }
   catch (BaseException &e)
   {
      // Use exception to remove Visual C++ warning
      e.GetMessageType();
      #if DEBUG_CREATE_PARAMETER
      MessageInterface::ShowMessage(wxT("Moderator::CreateParameter()\n") +
                                    e.GetFullMessage() + wxT("\n"));
      #endif
   }
   
   #if DEBUG_CREATE_PARAMETER
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateParameter() returning <%s><%p>\n"), param->GetName().c_str(),
       param);
   #endif
   
   return param;
}


//------------------------------------------------------------------------------
// Parameter* GetParameter(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a parameter object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a parameter object pointer, return null if name not found
 */
//------------------------------------------------------------------------------
Parameter* Moderator::GetParameter(const wxString &name)
{
   Parameter *obj = NULL;
   
   if (name != wxT(""))
   {
      // find Parameter from the current object map in use (loj: 2008.05.23)
      GmatBase* obj = FindObject(name);
      if (obj != NULL && obj->IsOfType(Gmat::PARAMETER))
         return (Parameter*)obj;
   }
   
   return obj;
}

// ODEModel
//------------------------------------------------------------------------------
// ODEModel* CreateODEModel(const wxString &name)
//------------------------------------------------------------------------------
/*
 * Creates ODEModel with given name
 */
//------------------------------------------------------------------------------
ODEModel* Moderator::CreateODEModel(const wxString &type,
                                    const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateODEModel() name='%s', objectManageOption=%d\n"),
       name.c_str(), objectManageOption);
   #endif
   
   ODEModel *obj = GetODEModel(name);
   
   if (obj == NULL)
   {
      obj = theFactoryManager->CreateODEModel(type, name);
      
      if (obj == NULL)
      {
         MessageInterface::ShowMessage(wxT("No fm\n"));
         throw GmatBaseException
            (wxT("The Moderator cannot create ODEModel named \"") + name + wxT("\"\n"));
      }
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateODEModel()"), funcName);
      }
      #endif
      
      // Create default physical model
      PhysicalModel *pm = CreateDefaultPhysicalModel(wxT(""));
      pm->SetName(wxT("_DefaultInternalForce_"));
      obj->AddForce(pm);
      
      // Manage it if it is a named ODEModel
      try
      {
         if (obj->GetName() != wxT("") && objectManageOption == 1)
            theConfigManager->AddODEModel(obj);
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(wxT("Moderator::CreateODEModel()\n") +
                                       e.GetFullMessage() + wxT("\n"));
      }
      
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateODEModel() returning new ODEModel, <%p> '%s'\n"),
          obj, obj->GetName().c_str());
      #endif
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateODEModel() Unable to create ODEModel ")
          wxT("name: %s already exist <%p>\n"), name.c_str(), obj);
      #endif
      return obj;
   }
}


//------------------------------------------------------------------------------
// ODEModel* GetODEModel(const wxString &name)
//------------------------------------------------------------------------------
ODEModel* Moderator::GetODEModel(const wxString &name)
{
   ODEModel *fm = NULL;
   
   if (name != wxT(""))
   {
      // Find ODEModel from the current object map in use (loj: 2008.06.20)
      GmatBase* obj = FindObject(name);
      if (obj != NULL && obj->IsOfType(Gmat::ODE_MODEL))
      {
         fm = (ODEModel*)obj;
         
         #if DEBUG_CREATE_RESOURCE
         MessageInterface::ShowMessage
            (wxT("Moderator::GetODEModel() name='%s', returning <%p>\n"), name.c_str(), fm);
         #endif
         
         return fm;
      }
   }
   
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::GetODEModel() name='%s', returning <%p>\n"), name.c_str(), fm);
   #endif
   
   return fm;
}


//------------------------------------------------------------------------------
// bool AddToODEModel(const wxString &ODEModelName,
//                      const wxString &forceName)
//------------------------------------------------------------------------------
bool Moderator::AddToODEModel(const wxString &odeModelName,
                                const wxString &forceName)
{
   bool status = true;
   ODEModel *fm = theConfigManager->GetODEModel(odeModelName);
   PhysicalModel *physicalModel = theConfigManager->GetPhysicalModel(forceName);
   fm->AddForce(physicalModel);
   return status;
}


// Solver
//------------------------------------------------------------------------------
// Solver* CreateSolver(const wxString &type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a solver object by given type and name and add to configuration.
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return a solver object pointer
 */
//------------------------------------------------------------------------------
Solver* Moderator::CreateSolver(const wxString &type, const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateSolver() type = '%s', name = '%s'\n"), type.c_str(),
       name.c_str());
   #endif
   
   if (GetSolver(name) == NULL)
   {
      Solver *obj = theFactoryManager->CreateSolver(type, name);

      if (obj == NULL)
      {
         throw GmatBaseException
            (wxT("The Moderator cannot create Solver type \"") + type + wxT("\"\n"));
      }
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateSolver()"), funcName);
      }
      #endif
      
      // Manage it if it is a named Solver
      try
      {
         if (obj->GetName() != wxT("") && objectManageOption == 1)
            theConfigManager->AddSolver(obj);
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(wxT("Moderator::CreateSolver()\n") +
                                       e.GetFullMessage());
      }
      
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateSolver() Unable to create Solver ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
      return GetSolver(name);
   }
}


//------------------------------------------------------------------------------
// Solver* GetSolver(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a solver object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a solver object pointer, return null if name not found
 */
//------------------------------------------------------------------------------
Solver* Moderator::GetSolver(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (Solver*)FindObject(name);
}

// PropSetup
//------------------------------------------------------------------------------
// PropSetup* CreateDefaultPropSetup(const wxString &name)
//------------------------------------------------------------------------------
PropSetup* Moderator::CreateDefaultPropSetup(const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage(wxT("====================\n"));
   MessageInterface::ShowMessage(wxT("Moderator::CreateDefaultPropSetup() name='%s'\n"),
                                 name.c_str());
   #endif
   
   // create PropSetup, PropSetup constructor creates default RungeKutta89 Integrator
   // and Earth PointMassForce
   PropSetup *propSetup = CreatePropSetup(name);
   
   // Create default force model with Earth primary body with JGM2
   // Changed back to unnamed ForceModel, since FinalPass() should not be called
   // from ScriptInterpreter::Interpret(GmatCommand *inCmd, ...) when parsing
   // commands in ScriptEvent for Bug 2436 fix (LOJ: 2011.07.05)
   // Why unnamed FM? Changed back to named FM, since it causes error in
   // Interpreter::FinalPass() when parsing ScriptEvent (Begin/EndScript) from GUI.
   // The error is about undefined DefaultProp_ForceModel(LOJ: 2011.01.12)
   //ODEModel *fm= CreateODEModel(wxT("ForceModel", name + "_ForceModel"));
   // Create unnamed ODEModel when creating default PropSetup (LOJ: 2009.11.23)
   // and delete fm after setting it to PropSetup (see below)
   ODEModel *fm = CreateODEModel(wxT("ForceModel"), wxT(""));
   fm->SetName(name + wxT("_ForceModel"));
   
   //=======================================================
   #if 0
   //=======================================================
   GravityField *gravForce = new GravityField("", "Earth");
   
   #ifdef DEBUG_MEMORY
   wxString funcName;
   funcName = currentFunction ? "function: " + currentFunction->GetName() : "";
   MemoryTracker::Instance()->Add
      (gravForce, gravForce->GetName(),
       "Moderator::CreateDefaultPropSetup(), *gravForce = new GravityField", funcName);
   #endif
   
   gravForce->SetName("Earth");
   gravForce->SetSolarSystem(theSolarSystemInUse);
   gravForce->SetBody("Earth");
   gravForce->SetBodyName("Earth");
   gravForce->SetStringParameter("PotentialFile", GetFileName("JGM2_FILE"));
   //=======================================================
   #endif
   //=======================================================
   
   propSetup->SetODEModel(fm);
   
   // Why unnamed FM? Commented out (LOJ: 2011.01.12)
   // PropSetup::SetODEModel() clones the ODEModel, so delete it from here (LOJ: 2009.11.23)
   //#ifdef DEBUG_MEMORY
   //wxString funcName = currentFunction ? "function: " + currentFunction->GetName() : "";
   //MemoryTracker::Instance()->Remove
   //   (fm, fm->GetName(), "Moderator::CreateDefaultPropSetup()"
   //    "deleting fm", funcName);
   //#endif
   //delete fm;
   
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreatePropSetup() returning new DefaultPropSetup <%p>\n"), propSetup);
   #endif
   
   return propSetup;
}

//------------------------------------------------------------------------------
// PropSetup* CreatePropSetup(const wxString &name)
//------------------------------------------------------------------------------
/*
 * Creates PropSetup which contains Integrator and ODEModel.
 */
//------------------------------------------------------------------------------
PropSetup* Moderator::CreatePropSetup(const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage(wxT("====================\n"));
   MessageInterface::ShowMessage(wxT("Moderator::CreatePropSetup() name='%s'\n"),
                                 name.c_str());
   #endif
   
   if (GetPropSetup(name) == NULL)
   {
      PropSetup *propSetup = theFactoryManager->CreatePropSetup(name);
      
      if (propSetup == NULL)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("The Moderator cannot create a PropSetup.\n")
             wxT("Make sure PropSetup is correct type and registered to ")
             wxT("PropSetupFactory.\n"));
         return NULL;
      }
      
      #ifdef DEBUG_MEMORY
      if (propSetup)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (propSetup, name, wxT("Moderator::CreatePropSetup()"), funcName);
      }
      #endif
      
      // PropSetup creates default Integrator(RungeKutta89)
      // and default ODEModel (PointMassForce body=Earth)
      
      if (name != wxT("") && objectManageOption == 1)
         theConfigManager->AddPropSetup(propSetup);
      
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreatePropSetup() returning new PropSetup <%p>\n"), propSetup);
      #endif
      
      return propSetup;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreatePropSetup() Unable to create PropSetup ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
      return GetPropSetup(name);
   }
}

//------------------------------------------------------------------------------
// PropSetup* GetPropSetup(const wxString &name)
//------------------------------------------------------------------------------
PropSetup* Moderator::GetPropSetup(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (PropSetup*)FindObject(name);
}


// MeasurementModel
//------------------------------------------------------------------------------
// MeasurementModel* CreateMeasurementModel(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a new named MeasurementModel and adds it to the configuration
 *
 * @param name The name of the new MeasurementModel
 *
 * @return The new MeasurementModel
 */
//------------------------------------------------------------------------------
MeasurementModel* Moderator::CreateMeasurementModel(const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage(wxT("====================\n"));
   MessageInterface::ShowMessage(wxT("Moderator::GetMeasurementModel() name='%s'\n"),
                                 name.c_str());
   #endif

   if (GetMeasurementModel(name) == NULL)
   {
      MeasurementModel *obj = theFactoryManager->CreateMeasurementModel(name);

      if (obj == NULL)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("The Moderator cannot create a MeasurementModel.\n")
             wxT("Make sure MeasurementModel is correct type and registered to ")
             wxT("MeasurementModelFactory.\n"));
         return NULL;
      }

      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : ("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateMeasurementModel()"), funcName);
      }
      #endif
      
      if (name != wxT("") && objectManageOption == 1)
         theConfigManager->AddMeasurementModel(obj);
      
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateMeasurementModel() returning new MeasurementModel ")
               wxT("<%p>\n"), obj);
      #endif

      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateMeasurementModel() Unable to create ")
          wxT("MeasurementModel name: %s already exist\n"), name.c_str());
      #endif
      return GetMeasurementModel(name);
   }
}

//------------------------------------------------------------------------------
// MeasurementModel* GetMeasurementModel(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a measurement model from the configuration
 *
 * @param name The name of the MeasurementModel object
 *
 * @return The named MeasurementModel
 */
//------------------------------------------------------------------------------
MeasurementModel* Moderator::GetMeasurementModel(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (MeasurementModel*)FindObject(name);
}

// TrackingSystem
//------------------------------------------------------------------------------
// TrackingSystem* CreateTrackingSystem(const wxString &type,
//       const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a new named TrackingSystem and adds it to the configuration
 *
 * @param type The type of the new TrackingSystem
 * @param name The name of the new TrackingSystem
 *
 * @return The new TrackingSystem
 */
//------------------------------------------------------------------------------
TrackingSystem* Moderator::CreateTrackingSystem(const wxString &type,
         const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage(wxT("====================\n"));
   MessageInterface::ShowMessage(wxT("Moderator::CreateTrackingSystem() type=%s, ")
            wxT("name='%s'\n"), type.c_str(), name.c_str());
   #endif

   if (GetTrackingSystem(name) == NULL)
   {
      TrackingSystem *obj = theFactoryManager->CreateTrackingSystem(type, name);

      if (obj == NULL)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("The Moderator cannot create a TrackingSystem.\n")
             wxT("Make sure TrackingSystem is correct type and registered to ")
             wxT("TrackingSystemFactory.\n"));
         return NULL;
      }

      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateTrackingSystem()"), funcName);
      }
      #endif
      
      if (name != wxT("") && objectManageOption == 1)
         theConfigManager->AddTrackingSystem(obj);
      
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateTrackingSystem() returning new TrackingSystem ")
               wxT("<%p>\n"), obj);
      #endif

      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateTrackingSystem() Unable to create ")
          wxT("TrackingSystem name: %s already exists\n"), name.c_str());
      #endif
      return GetTrackingSystem(name);
   }
}

//------------------------------------------------------------------------------
// TrackingSystem* GetTrackingSystem(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a TrackingSystem from the configuration
 *
 * @param name The name of the TrackingSystem object
 *
 * @return The named TrackingSystem
 */
//------------------------------------------------------------------------------
TrackingSystem* Moderator::GetTrackingSystem(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (TrackingSystem*)FindObject(name);
}

// TrackingData
//------------------------------------------------------------------------------
// TrackingData* CreateTrackingData(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a new named TrackingData object and adds it to the configuration
 *
 * @param name The name of the new TrackingData object
 *
 * @return The new TrackingData object
 */
//------------------------------------------------------------------------------
TrackingData* Moderator::CreateTrackingData(const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage(wxT("====================\n"));
   MessageInterface::ShowMessage(wxT("Moderator::CreateTrackingData() name='%s'\n"),
                                 name.c_str());
   #endif

   if (GetTrackingData(name) == NULL)
   {
      TrackingData *obj = theFactoryManager->CreateTrackingData(name);

      if (obj == NULL)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("The Moderator cannot create a TrackingData object.")
             wxT("\nMake sure TrackingData is correct type and registered to ")
             wxT("TrackingDataFactory.\n"));
         return NULL;
      }

      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateTrackingData()"), funcName);
      }
      #endif
      
      if (name != wxT("") && objectManageOption == 1)
         theConfigManager->AddTrackingData(obj);
      
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateTrackingData() returning new TrackingData ")
               wxT("<%p>\n"), obj);
      #endif

      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateTrackingData() Unable to create ")
          wxT("TrackingData name: %s already exists\n"), name.c_str());
      #endif
      return GetTrackingData(name);
   }
}

//------------------------------------------------------------------------------
// TrackingData* GetTrackingData(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a TrackingData object from the configuration
 *
 * @param name The name of the TrackingData object
 *
 * @return The named TrackingData object
 */
//------------------------------------------------------------------------------
TrackingData* Moderator::GetTrackingData(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (TrackingData*)FindObject(name);
}


//------------------------------------------------------------------------------
// CoreMeasurement* CreateMeasurement(const wxString &type,
//       const wxString &name)
//------------------------------------------------------------------------------
/**
 * This method calls the FactoryManager to create a new CoreMeasurement object
 *
 * @param type The type of measurement object needed
 * @param name The new object's name, is it is to be configured
 *
 * @return The new object's pointer
 */
//------------------------------------------------------------------------------
CoreMeasurement* Moderator::CreateMeasurement(const wxString &type,
      const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage(wxT("====================\n"));
   MessageInterface::ShowMessage(wxT("Moderator::CreateMeasurement() name='%s'\n"),
                                 name.c_str());
   #endif

   if (GetMeasurement(type, name) == NULL)
   {
      CoreMeasurement *obj = theFactoryManager->CreateMeasurement(type, name);

      if (obj == NULL)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("The Moderator cannot create a Measurement.\n")
             wxT("Make sure Measurement %s is correct type and registered to ")
             wxT("MeasurementFactory.\n"), type.c_str());
         return NULL;
      }

      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateMeasurementModel()"), funcName);
      }
      #endif

      if (name != wxT("") && objectManageOption == 1)
         theConfigManager->AddMeasurement(obj);

      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateMeasurement() returning new Measurement ")
               wxT("<%p>\n"), obj);
      #endif

      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateMeasurement() Unable to create ")
          wxT("Measurement name: %s already exist\n"), name.c_str());
      #endif
      return GetMeasurement(type, name);
   }
}

//------------------------------------------------------------------------------
// CoreMeasurement* GetMeasurement(const wxString &type,
//       const wxString &name)
//------------------------------------------------------------------------------
/**
 * This method finds a configured CoreMeasurement
 *
 * @param type The type of measurement object needed
 * @param name The new object's name, is it is to be configured
 *
 * @return The object's pointer; NULL if the object is not in the configuration
 */
//------------------------------------------------------------------------------
CoreMeasurement* Moderator::GetMeasurement(const wxString &type,
      const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (CoreMeasurement*)FindObject(name);
}


// DataFile
//------------------------------------------------------------------------------
// DataFile* CreateDataFile(const wxString &type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a new named DataFile and adds it to the configuration
 *
 * @param type The type of the DataFile object
 * @param name The name of the new DataFile
 *
 * @return The new DataFile
 */
//------------------------------------------------------------------------------
DataFile* Moderator::CreateDataFile(const wxString &type,
                                    const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage(wxT("====================\n"));
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateDataFile() type='%s', name='%s'\n"), type.c_str(),
       name.c_str());
   #endif
   
   if (GetDataFile(name) == NULL)
   {
      DataFile *df = theFactoryManager->CreateDataFile(type, name);
      
      if (df == NULL)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("The Moderator cannot create a DataFile.\n")
             wxT("Make sure DataFile is correct type and registered to ")
             wxT("DataFileFactory.\n"));
         return NULL;
      }
      
      #ifdef DEBUG_MEMORY
      if (df)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (df, name, wxT("Moderator::CreateDataFile()"), funcName);
      }
      #endif
      
      if (name != wxT("") && objectManageOption == 1)
         theConfigManager->AddDataFile(df);
      
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateDataFile() returning new DataFile ")
               wxT("<%p>\n"), df);
      #endif
      
      return df;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateDataFile() Unable to create ")
          wxT("DataFile name: %s already exists\n"), name.c_str());
      #endif
      return GetDataFile(name);
   }
}


//------------------------------------------------------------------------------
// DataFile* GetDataFile(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a measurement DataFile from the configuration
 *
 * @param name The name of the DataFile object
 *
 * @return The named DataFile
 */
//------------------------------------------------------------------------------
DataFile* Moderator::GetDataFile(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (DataFile*)FindObject(name);
}


// ObType
//------------------------------------------------------------------------------
// ObType* CreateObType(const wxString &type, const wxString &name)
//------------------------------------------------------------------------------
ObType* Moderator::CreateObType(const wxString &type, const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage(wxT("====================\n"));
   MessageInterface::ShowMessage(wxT("Moderator::CreateObType() name='%s'\n"),
                                 name.c_str());
   #endif

   if (GetObType(name) == NULL)
   {
      ObType *ot = theFactoryManager->CreateObType(type, name);

      if (ot == NULL)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("The Moderator cannot create a ObType.\n")
             wxT("Make sure ObType is correct type and registered to a ")
             wxT("ObTypeFactory.\n"));
         return NULL;
      }

      #ifdef DEBUG_MEMORY
      if (ot)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (ot, name, wxT("Moderator::CreateObType()"), funcName);
      }
      #endif

      if (name != wxT("") && objectManageOption == 1)
         theConfigManager->AddObType(ot);
      
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateObType() returning new ObType ")
               wxT("<%p>\n"), ot);
      #endif

      return ot;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateDataFile() Unable to create ")
          wxT("DataFile name: %s already exists\n"), name.c_str());
      #endif
      return GetObType(name);
   }
}

//------------------------------------------------------------------------------
// ObType* GetObType(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a ObType from the configuration
 *
 * @param name The name of the ObType object
 *
 * @return The named ObType  (Should always return NULL)
 */
//------------------------------------------------------------------------------
ObType* Moderator::GetObType(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (ObType*)FindObject(name);
}


//------------------------------------------------------------------------------
// EventLocator* Moderator::CreateEventLocator(const wxString &type,
//                          const wxString &name)
//------------------------------------------------------------------------------
/**
 * Calls the FactoryManager to create an EventLocator
 *
 * @param type The type of event locator to be created
 * @param name The name of the new EventLocator
 *
 * @return The named EventLocator
 */
//------------------------------------------------------------------------------
EventLocator* Moderator::CreateEventLocator(const wxString &type,
                         const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage(wxT("====================\n"));
   MessageInterface::ShowMessage(wxT("Moderator::CreateEventLocator() name='%s'\n"),
                                 name.c_str());
   #endif

   if (GetEventLocator(name) == NULL)
   {
      EventLocator *el = theFactoryManager->CreateEventLocator(type, name);

      if (el == NULL)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("The Moderator cannot create an EventLocator.\n")
             wxT("Make sure EventLocator is correct type and registered to a ")
             wxT("EventLocatorFactory.\n"));
         return NULL;
      }

      #ifdef DEBUG_MEMORY
      if (el)
      {
         wxString funcName;
         funcName = currentFunction ?
               wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (el, name, wxT("Moderator::CreateEventLocator()"), funcName);
      }
      #endif

      if (name != wxT("") && objectManageOption == 1)
         theConfigManager->AddEventLocator(el);

      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateEventLocator() returning new EventLocator ")
               wxT("<%p>\n"), el);
      #endif

      return el;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateEventLocator() Unable to create ")
          ("EventLocator name: %s already exists\n"), name.c_str());
      #endif
      return GetEventLocator(name);
   }
}


//------------------------------------------------------------------------------
// EventLocator* GetEventLocator(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a previously created EventLoactor
 *
 * @param name The name of the EventLocator
 *
 * @return A pointer to the EventLocator, or NULL if it is not in the
 *         configuration.
 */
//------------------------------------------------------------------------------
EventLocator* Moderator::GetEventLocator(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (EventLocator*)FindObject(name);
}



//------------------------------------------------------------------------------
// Interpolator* CreateInterpolator(const wxString &type,
//                                  const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a Interpolator object by given type and name.
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return a Interpolator object pointer
 */
//------------------------------------------------------------------------------
Interpolator* Moderator::CreateInterpolator(const wxString &type,
                                            const wxString &name)
{
   //loj: 3/22/04 theFactoryManager->CreateInterpolator() not implemented
   return NULL;
}

//------------------------------------------------------------------------------
// Interpolator* GetInterpolator(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a Interpolator object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a Interpolator object pointer, return null if name not found
 */
//------------------------------------------------------------------------------
Interpolator* Moderator::GetInterpolator(const wxString &name)
{
   return NULL;
}

// CoordinateSystem
//------------------------------------------------------------------------------
// CoordinateSystem* CreateCoordinateSystem(const wxString &name,
//                   bool createDefault = false, bool internal = false,
//                   Integer manage = 1)
//------------------------------------------------------------------------------
/**
 * Creates coordinate system
 *
 * @param  name  Name of the coordinate system to create
 * @param  createDefault  If this flag is set, it will create MJ2000Eq system
 * @param  internal  If this flag is set, it will not configure the CS
 * @param  manage  The value to use for managing the newly created CS
 *                 0 = do not add to configuration
 *                 1 = add to configuration
 */
//------------------------------------------------------------------------------
CoordinateSystem* Moderator::CreateCoordinateSystem(const wxString &name,
                                                    bool createDefault,
                                                    bool internal, Integer manage)
{
   #if DEBUG_CREATE_COORDSYS
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateCoordinateSystem() name='%s', createDefault=%d, ")
       wxT("internal=%d, manage=%d, theSolarSystemInUse=%p\n"), name.c_str(), createDefault,
       internal, manage, theSolarSystemInUse);
   #endif
   
   CoordinateSystem *obj = GetCoordinateSystem(name);
   
   if (obj == NULL)
   {
      #if DEBUG_CREATE_COORDSYS
      MessageInterface::ShowMessage
         (wxT("   Cannot find CoordinateSystem named '%s', so create\n"), name.c_str());
      #endif
      
      obj = theFactoryManager->CreateCoordinateSystem(name);
      
      if (obj == NULL)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("The Moderator cannot create a CoordinateSystem.\n")
             wxT("Make sure CoordinateSystem is correct type and registered to ")
             wxT("CoordinateSystemFactory.\n"));
         
         return NULL;
         
         //throw GmatBaseException
         //   ("The Moderator cannot create a CoordinateSystem type \"" + type + "\"\n");
      }
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateCoordinateSystem()"), funcName);
      }
      #endif
      
      // Manage it if it is a named CoordinateSystem
      try
      {
         if (name != wxT("") && !internal && manage != 0)
         {
            if (manage == 1)
               theConfigManager->AddCoordinateSystem(obj);
            else
               // Do we really want to add a new CoordinateSystem to the
               // function object map? (loj: 2008.06.27)
               AddObject(obj);
         }
         
         // Call GetSolarSystemInUse() to get SolarSystem from configuration
         // or object map in use (loj: 2008.06.27)
         SolarSystem *ss = GetSolarSystemInUse(manage);
         CelestialBody *earth = ss->GetBody(wxT("Earth"));
         #if DEBUG_CREATE_COORDSYS
            MessageInterface::ShowMessage
               (wxT("Mod::CreateCS = SolarSystem found at <%p>\n"), ss);
            MessageInterface::ShowMessage
               (wxT("Mod::CreateCS = Earth found at <%p>\n"), earth);
         #endif
         
         // Set J2000Body and SolarSystem
         obj->SetStringParameter(wxT("J2000Body"), wxT("Earth"));
         obj->SetRefObject(earth, Gmat::SPACE_POINT, wxT("Earth"));
         obj->SetSolarSystem(ss);
         obj->Initialize();
         
         if (createDefault)
         {
            // create MJ2000Eq AxisSystem with Earth as origin
            AxisSystem *axis = CreateAxisSystem(wxT("MJ2000Eq"), wxT("MJ2000Eq_Earth"));
            obj->SetStringParameter(wxT("J2000Body"), wxT("Earth"));
            obj->SetStringParameter(wxT("Origin"), wxT("Earth"));
            obj->SetRefObject(earth, Gmat::SPACE_POINT, wxT("Earth"));
            obj->SetRefObject(axis, Gmat::AXIS_SYSTEM, axis->GetName());
            obj->SetSolarSystem(ss);
            obj->Initialize();
            
            // Since CoordinateSystem clones AxisSystem, delete it from here
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Remove
               (axis, wxT("localAxes"), wxT("Moderator::CreateCoordinateSystem()"),
                wxT("deleting localAxes"));
            #endif
            delete axis;
         }
      }
      catch (BaseException &e)
      {
         // Use exception to remove Visual C++ warning
         e.GetMessageType();
         #if DEBUG_CREATE_COORDSYS
         MessageInterface::ShowMessage(wxT("Moderator::CreateCoordinateSystem() %s\n"),
                                       e.GetFullMessage().c_str());
         #endif
      }
      
      #if DEBUG_CREATE_COORDSYS
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateCoordinateSystem() returning new %p\n"), obj);
      #endif
      
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_COORDSYS
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateCoordinateSystem() Unable to create CoordinateSystem ")
          wxT("name: %s already exist\n"), name.c_str());      
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateCoordinateSystem() returning existing %p\n"), obj);
      #endif
      
      return obj;
   }
}


//------------------------------------------------------------------------------
// CoordinateSystem* GetCoordinateSystem(const wxString &name, Integer manage = 1)
//------------------------------------------------------------------------------
CoordinateSystem* Moderator::GetCoordinateSystem(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (CoordinateSystem*)FindObject(name);
}


//------------------------------------------------------------------------------
// const StringArray& GetDefaultCoordinateSystemNames()
//------------------------------------------------------------------------------
const StringArray& Moderator::GetDefaultCoordinateSystemNames()
{
   return defaultCoordSystemNames;
}


// Subscriber
//------------------------------------------------------------------------------
// Subscriber* CreateSubscriber(const wxString &type, const wxString &name,
//                              const wxString &fileName = "",
//                              bool createDefault = false)
//------------------------------------------------------------------------------
/**
 * Creates a subscriber object by given type and name if not already created.
 *
 * @param <type> object type
 * @param <name> object name
 * @param <fileName> file name if used
 * @param <createDefalut> create default Subscriber by setting ref. object if true
 *
 * @return a subscriber object pointer
 */
//------------------------------------------------------------------------------
Subscriber* Moderator::CreateSubscriber(const wxString &type,
                                        const wxString &name,
                                        const wxString &fileName,
                                        bool createDefault)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateSubscriber() type='%s', name='%s', fileName='%s'\n")
       wxT("   createDefault=%d\n"), type.c_str(), name.c_str(), fileName.c_str(),
       createDefault);
   MessageInterface::ShowMessage
      (wxT("   currentFunction = <%p>'%s', objectManageOption=%d\n"), currentFunction,
       currentFunction ? currentFunction->GetName().c_str() : wxT("NULL"),
       objectManageOption);
   #endif
   
   if (GetSubscriber(name) == NULL)
   {      
      Subscriber *obj = theFactoryManager->CreateSubscriber(type, name, fileName);
      
      if (obj == NULL)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("Cannot create a Subscriber type: %s.\n")
             wxT("Make sure %s is correct type and registered to SubscriberFactory.\n"),
             type.c_str(), type.c_str());
         
         return NULL;
         
         //throw GmatBaseException
         //   (wxT("The Moderator cannot create a Subscriber type\"" + type + "\"\n"));
      }
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateSubscriber()"), funcName);
      }
      #endif
      
      // Manage it if it is a named Subscriber
      try
      {
         if (obj->GetName() != wxT("") && objectManageOption == 1)
            theConfigManager->AddSubscriber(obj);
         
         #if DEBUG_CREATE_RESOURCE
         MessageInterface::ShowMessage
            (wxT("Moderator::CreateSubscriber() Creating default subscriber...\n"));
         #endif
         
         if (createDefault)
         {
            if (type == wxT("OrbitView"))
            {
               // add default spacecraft and coordinate system
               obj->SetStringParameter(wxT("Add"), GetDefaultSpacecraft()->GetName());
               obj->SetStringParameter(wxT("Add"), wxT("Earth"));
               obj->SetStringParameter(wxT("CoordinateSystem"), wxT("EarthMJ2000Eq"));
            }
            else if (type == wxT("GroundTrackPlot"))
            {
               // add default spacecraft and earth
               obj->SetStringParameter(wxT("Add"), GetDefaultSpacecraft()->GetName());
               obj->SetStringParameter(wxT("Add"), wxT("Earth"));
            }
            else if (type == wxT("XYPlot"))
            {
               // add default x,y parameter to XYPlot
               obj->SetStringParameter(XyPlot::XVARIABLE, GetDefaultX()->GetName());
               obj->SetStringParameter(XyPlot::YVARIABLES, GetDefaultY()->GetName(), 0);
               obj->Activate(true);
            }
            else if (type == wxT("ReportFile"))
            {
               // add default parameters to ReportFile
               obj->SetStringParameter(obj->GetParameterID(wxT("Filename")),
                                       name + wxT(".txt"));
               obj->SetStringParameter(wxT("Add"), GetDefaultX()->GetName());
               obj->SetStringParameter(wxT("Add"), GetDefaultY()->GetName());
               obj->Activate(true);
               
               // To validate and create element wrappers
               theScriptInterpreter->ValidateSubscriber(obj);
            }
            else if (type == wxT("EphemerisFile"))
            {
               // add default spacecraft and coordinate system
               obj->SetStringParameter(wxT("Spacecraft"), GetDefaultSpacecraft()->GetName());
            }
         }
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(wxT("Moderator::CreateSubscriber()\n") +
                                       e.GetFullMessage());
      }
      
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateSubscriber() Unable to create Subscriber ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
      
      return GetSubscriber(name);
   }
}

//------------------------------------------------------------------------------
// Subscriber* GetSubscriber(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a subscriber object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a subscriber object pointer, return null if name not found
 */
//------------------------------------------------------------------------------
Subscriber* Moderator::GetSubscriber(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (Subscriber*)FindObject(name);
}


//------------------------------------------------------------------------------
// Integer GetNumberOfActivePlots()
//------------------------------------------------------------------------------
/**
 * Returns number of active plots which means plots with ShowPlot is on.
 */
//------------------------------------------------------------------------------
Integer Moderator::GetNumberOfActivePlots()
{
   Integer activePlotCount = 0;
   Subscriber *obj;
   StringArray names = theConfigManager->GetListOfItems(Gmat::SUBSCRIBER);
   
   #ifdef DEBUG_ACTIVE_PLOTS
   MessageInterface::ShowMessage
      (wxT("Moderator::GetNumberOfActivePlots() subscriber count = %d\n"), names.size());
   #endif
   
   //@todo
   // Should we create a new class GmatPlot and derive XYPlot and OrbitPlot from it?
   for (Integer i=0; i<(Integer)names.size(); i++)
   {
      obj = theConfigManager->GetSubscriber(names[i]);
      if (obj->IsOfType(wxT("XYPlot")))
      {
         if ( ((XyPlot*)obj)->GetBooleanParameter(wxT("ShowPlot")) )
            activePlotCount++;
      }
      else if (obj->IsOfType(wxT("OrbitPlot")))
      {
         if ( ((OrbitPlot*)obj)->GetBooleanParameter(wxT("ShowPlot")) )
            activePlotCount++;
      }
   }
   
   #ifdef DEBUG_ACTIVE_PLOTS
   MessageInterface::ShowMessage
      (wxT("Moderator::GetNumberOfActivePlots() returning %d\n"), activePlotCount);
   #endif
   
   return activePlotCount;
}


//------------------------------------------------------------------------------
// Subscriber* CreateEphemerisFile(const wxString &type,
//                                 const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a subscriber object by given type and name if not already created.
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return a subscriber object pointer
 */
//------------------------------------------------------------------------------
Subscriber* Moderator::CreateEphemerisFile(const wxString &type,
                                           const wxString &name)
{
   #if DEBUG_CREATE_EPHEMFILE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateEphemerisFile() type='%s', name='%s'\n"),
       type.c_str(), name.c_str());
   MessageInterface::ShowMessage
      (wxT("   in function = <%p>'%s'\n"), currentFunction,
       currentFunction ? currentFunction->GetName().c_str() : wxT("NULL"));
   #endif
   
   if (GetEphemerisFile(name) == NULL)
   {      
      Subscriber *obj = (Subscriber*)(theFactoryManager->CreateEphemerisFile(type, name));
      
      if (obj == NULL)
      {
         #ifdef __USE_DATAFILE__
            if (type == wxT("CcsdsEphemerisFile"))
            {
               MessageInterface::PopupMessage
                  (Gmat::ERROR_, wxT("**** ERROR **** Cannot create an EphemerisFile type: %s.\n")
                   wxT("Make sure to specify PLUGIN = libDataFile and PLUGIN = libCcsdsEphemerisFile\n")
                   wxT("in the gmat_start_file and make sure such dlls exist.  ")
                   wxT("Make sure that libpcre-0 and libpcrecpp-0 also exist in the path\n"),
                   type.c_str(), type.c_str());

               return NULL;
            }
         #else
            #ifdef DEBUG_CREATE_EPHEMFILE
            MessageInterface::ShowMessage
               (wxT("CreateEphemerisFile() Creating a subscriber of type EphemerisFile\n"));
            #endif
            // Try again with wxT("EphemerisFile") type
            obj = (Subscriber*)(theFactoryManager->CreateSubscriber(wxT("EphemerisFile"), name));
         #endif
      }
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateEphemerisFile()"), funcName);
      }
      #endif
      
      // Manage it if it is a named EphemerisFile
      try
      {
         if (name != wxT("") && objectManageOption == 1)
            theConfigManager->AddSubscriber(obj);
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(wxT("Moderator::CreateEphemerisFile()\n") +
                                       e.GetFullMessage());
      }
      
      #if DEBUG_CREATE_EPHEMFILE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateEphemerisFile() returning <%p>\n"), obj);
      #endif
      
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_EPHEMFILE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateEphemerisFile() Unable to create EphemerisFile ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
      
      return GetEphemerisFile(name);
   }
}

//------------------------------------------------------------------------------
// Subscriber* GetEphemerisFile(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a subscriber object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a subscriber object pointer, return null if name not found
 */
//------------------------------------------------------------------------------
Subscriber* Moderator::GetEphemerisFile(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return (Subscriber*)FindObject(name);
}

// Function
//------------------------------------------------------------------------------
// Function* CreateFunction(const wxString &type, const wxString &name,
//                          Integer manage = 1)
//------------------------------------------------------------------------------
/**
 * Creates a function object by given type and name.
 *
 * @param <type> object type
 * @param <name> object name
 * @param <manage> if 0, new function is not managed
 *                    1, new function is added to configuration
 * @return a Function object pointer
 */
//------------------------------------------------------------------------------
Function* Moderator::CreateFunction(const wxString &type,
                                    const wxString &name,
                                    Integer manage)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateFunction() type = '%s', name = '%s', manage=%d\n"),
       type.c_str(), name.c_str(), manage);
   MessageInterface::ShowMessage
      (wxT("   in function = <%p>'%s'\n"), currentFunction,
       currentFunction ? currentFunction->GetName().c_str() : wxT("NULL"));
   #endif
   
   if (GetFunction(name) == NULL)
   {
      Function *obj = theFactoryManager->CreateFunction(type, name);
      
      if (obj == NULL)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("Cannot create a Function type: %s.\n")
             wxT("Make sure %s is correct type and registered to FunctionFactory.\n"),
             type.c_str(), type.c_str());
         
         return NULL;
         
         //throw GmatBaseException
         //   (wxT("The Moderator cannot create a Function type \"" + type + "\"\n"));
      }
      
      #ifdef DEBUG_MEMORY
      if (obj)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (obj, name, wxT("Moderator::CreateFunction()"), funcName);
      }
      #endif
      
      // Manage it if it is a named Function
      try
      {
         if (name != wxT("") && manage == 1)
            theConfigManager->AddFunction(obj);
         else if (currentFunction != NULL && manage == 0)
            unmanagedFunctions.push_back(obj);
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage(wxT("Moderator::CreateFunction()\n") +
                                       e.GetFullMessage());
      }
      
      return obj;
   }
   else
   {
      #if DEBUG_CREATE_RESOURCE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateFunction() Unable to create Function ")
          wxT("name: %s already exist\n"), name.c_str());
      #endif
      
      return GetFunction(name);
   }
}


//------------------------------------------------------------------------------
// Function* GetFunction(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a function object pointer by given name.
 *
 * @param <name> object name
 *
 * @return a Function object pointer, return null if name not found
 */
//------------------------------------------------------------------------------
Function* Moderator::GetFunction(const wxString &name)
{
   if (name == wxT(""))
      return NULL;
   else
      return theConfigManager->GetFunction(name);
}


//----- Non-configurable Items

// StopCondition
//------------------------------------------------------------------------------
// StopCondition* CreateStopCondition(const wxString &type,
//                                    const wxString &name)
//------------------------------------------------------------------------------
StopCondition* Moderator::CreateStopCondition(const wxString &type,
                                              const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage(wxT("Moderator::CreateStopCondition() type = '%s', ")
                                 wxT("name = '%s'\n"), type.c_str(), name.c_str());
   #endif
   
   StopCondition *stopCond = theFactoryManager->CreateStopCondition(type, name);
   
   if (stopCond ==  NULL)
   {
      throw GmatBaseException
         (wxT("The Moderator cannot create StopCondition type \"") + type + wxT("\"\n"));
   }
   
   #ifdef DEBUG_MEMORY
   if (stopCond)
   {
      wxString funcName;
      funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
      MemoryTracker::Instance()->Add
         (stopCond, name, wxT("Moderator::CreateStopCondition()"), funcName);
   }
   #endif
   
   return stopCond;
}


//------------------------------------------------------------------------------
// AxisSystem* CreateAxisSystem(const wxString &type,
//                              const wxString &name, Integer manage)
//------------------------------------------------------------------------------
/**
 * Creates a AxisSystem object by given type and name.
 *
 * @param <type> object type
 * @param <name> object name
 * @param <manage>  0, if parameter is not managed
 *                  1, if parameter is added to configuration (default)
 *                  2, if Parameter is added to function object map
 *
 * @return a AxisSystem object pointer
 */
//------------------------------------------------------------------------------
AxisSystem* Moderator::CreateAxisSystem(const wxString &type,
                                        const wxString &name, Integer manage)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateAxisSystem() type = '%s', name = '%s', manage=%d\n"),
       type.c_str(), name.c_str(), manage);
   #endif
   
   AxisSystem *axisSystem = theFactoryManager->CreateAxisSystem(type, name);
   
   if (axisSystem == NULL)
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_, wxT("Cannot create a AxisSystem type: %s.\n")
          wxT("Make sure %s is correct type and registered to AxisSystemFactory.\n"),
          type.c_str(), type.c_str());
      
      return NULL;
      
      //throw GmatBaseException
      //   (wxT("The Moderator cannot create AxisSystem type \"" + type + "\"\n"));
   }
   
   #ifdef DEBUG_MEMORY
   if (axisSystem)
   {
      wxString funcName;
      funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
      MemoryTracker::Instance()->Add
         (axisSystem, name, wxT("Moderator::CreateAxisSystem()"), funcName);
   }
   #endif
   
   // set origin and j2000body
   axisSystem->SetOrigin((SpacePoint*)FindObject(axisSystem->GetOriginName()));
   axisSystem->SetJ2000Body((SpacePoint*)FindObject(axisSystem->GetJ2000BodyName()));
   
   // Notes: AxisSystem is not configured. It is local to CoordinateSystem
   // and gets deleted when CoordinateSystem is deleted.
   
   // DJC added 5/11/05.  The ScriptInterpreter does not have the parms needed
   // to set these references, so defaults are set here.  This might need to be
   // fixed later.
   
   /// @todo Evaluate how the AxixSystem file usage really should be set
   
   // Set required internal references if they are used
   if (axisSystem->UsesEopFile() == GmatCoordinate::REQUIRED)
      axisSystem->SetEopFile(theEopFile);
   if (axisSystem->UsesItrfFile() == GmatCoordinate::REQUIRED)
      axisSystem->SetCoefficientsFile(theItrfFile);
   
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateAxisSystem() returning <%p>\n"), axisSystem);
   #endif
   
   return axisSystem;
}

// MathNode
//------------------------------------------------------------------------------
// MathNode* CreateMathNode(const wxString &type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a MathNode object by given type and name.
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return a MathNode object pointer
 */
//------------------------------------------------------------------------------
MathNode* Moderator::CreateMathNode(const wxString &type,
                                    const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage(wxT("Moderator::CreateMathNode() type = '%s', ")
                                 wxT("name = '%s'\n"), type.c_str(), name.c_str());
   #endif
   
   MathNode *mathNode = theFactoryManager->CreateMathNode(type, name);
   
   if (mathNode ==  NULL)
   {
      throw GmatBaseException
         (wxT("The Moderator cannot create MathNode type \"") + type + wxT("\"\n"));
   }
   
   #ifdef DEBUG_MEMORY
   if (mathNode)
   {
      wxString funcName;
      funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
      MemoryTracker::Instance()->Add
         (mathNode, name, wxT("Moderator::CreateMathNode()"), funcName);
   }
   #endif
   
   return mathNode;
}

//------------------------------------------------------------------------------
// Attitude* CreateAttitude(const wxString &type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates an Attitude object by given type and name.
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return an Attitude object pointer
 */
//------------------------------------------------------------------------------
Attitude* Moderator::CreateAttitude(const wxString &type,
                                    const wxString &name)
{
   #if DEBUG_CREATE_RESOURCE
   MessageInterface::ShowMessage(wxT("Moderator::CreateAttitude() type = '%s', ")
                                 wxT("name = '%s'\n"), type.c_str(), name.c_str());
   #endif
   
   Attitude *att = theFactoryManager->CreateAttitude(type, name);
   
   if (att == NULL)
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_, wxT("Cannot create an Attitude type: %s.\n")
          wxT("Make sure %s is correct type and registered to AttitudeFactory.\n"),
          type.c_str(), type.c_str());
      
      return NULL;
   }
   
   #ifdef DEBUG_MEMORY
   if (att)
   {
      wxString funcName;
      funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
      MemoryTracker::Instance()->Add
         (att, name, wxT("Moderator::CreateAttitude()"), funcName);
   }
   #endif
   
   return att;
}


//GmatCommand
//------------------------------------------------------------------------------
// GmatCommand* InterpretGmatFunction(const wxString fileName)
//------------------------------------------------------------------------------
/**
 * Retrieves a function object pointer by given name.
 *
 * @param <fileName>  Full path and name of the GmatFunction file.
 *
 * @return A command list that is executed to run the function.
 */
//------------------------------------------------------------------------------
GmatCommand* Moderator::InterpretGmatFunction(const wxString &fileName)
{
   #if DEBUG_GMAT_FUNCTION
   MessageInterface::ShowMessage
      (wxT("Moderator::InterpretGmatFunction() fileName='%s'\n"), fileName.c_str());
   #endif
   
   GmatCommand *cmd = NULL;
   if (fileName != wxT(""))
      cmd =  theScriptInterpreter->InterpretGmatFunction(fileName);
   
   ResetConfigurationChanged();
   
   #if DEBUG_GMAT_FUNCTION
   MessageInterface::ShowMessage
      (wxT("Moderator::InterpretGmatFunction() returning <%p><%s>\n"), cmd,
       cmd ? cmd->GetTypeName().c_str() : wxT("NULL"));
   #endif
   
   return cmd;
}


//------------------------------------------------------------------------------
// GmatCommand* InterpretGmatFunction(Function *funct, ObjectMap *objMap,
//                                    SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 * Retrieves a function object pointer by given name.
 *
 * @param <funct>  The GmatFunction pointer
 * @param <objMap> The object map pointer to be used for finding objects
 * @param <ss>     The solar system to be used
 *
 * @return A command list that is executed to run the function.
 */
//------------------------------------------------------------------------------
GmatCommand* Moderator::InterpretGmatFunction(Function *funct, ObjectMap *objMap,
                                              SolarSystem *ss)
{
   #if DEBUG_GMAT_FUNCTION
   MessageInterface::ShowMessage
      (wxT("Moderator::InterpretGmatFunction() function=<%p>, objMap=<%p>\n"),
       funct, objMap);
   #endif
   
   currentFunction = funct;
   
   // If input objMap is NULL, use configured objects
   // use input object map otherwise   
   if (objMap == NULL)
   {
      objectMapInUse = theConfigManager->GetObjectMap();
      #ifdef DEBUG_OBJECT_MAP
      MessageInterface::ShowMessage
         (wxT("Moderator::InterpretGmatFunction() objectMapInUse was set to the ")
          wxT("configuration map <%p>\n"), objectMapInUse);
      #endif
   }
   else
   {
      objectMapInUse = objMap;
      #ifdef DEBUG_OBJECT_MAP
      MessageInterface::ShowMessage
         (wxT("Moderator::InterpretGmatFunction() objectMapInUse was set to ")
          wxT("input objMap <%p>\n"), objectMapInUse);
      #endif
   }
   
   // If input SolarSystem is NULL, use default SolarSystemInUse
   // use input SolarSystem otherwise      
   SolarSystem *solarSystemInUse = GetSolarSystemInUse();
   if (ss != NULL)
   {
      solarSystemInUse = ss;
      theInternalSolarSystem = ss;
   }
   
   #if DEBUG_GMAT_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   Setting objectMapInUse<%p> to theScriptInterpreter\n")
       wxT("   Setting theSolarSystemInUse<%p> to theScriptInterpreter\n"),
       objectMapInUse, solarSystemInUse);
   #endif
   
   #if DEBUG_GMAT_FUNCTION > 1
   ShowObjectMap(wxT("Moderator::InterpretGmatFunction() Here is the object map in use"));
   #endif
   
   // Set solar system in use and object map for GmatFunction
   SetSolarSystemAndObjectMap(solarSystemInUse, objectMapInUse, true,
                              wxT("InterpretGmatFunction()"));
   
   GmatCommand *cmd = NULL;
   cmd = theScriptInterpreter->InterpretGmatFunction(funct);
   
   // reset current function to NULL
   currentFunction = NULL;
   
   #if DEBUG_GMAT_FUNCTION
   MessageInterface::ShowMessage
      (wxT("Moderator::InterpretGmatFunction() returning <%p><%s>\n"), cmd,
       cmd ? cmd->GetTypeName().c_str() : wxT("NULL"));
   #endif
   
   return cmd;
} // InterpretGmatFunction()


//------------------------------------------------------------------------------
// GmatCommand* CreateCommand(const wxString &type, const wxString &name
//                            bool &retFlag)
//------------------------------------------------------------------------------
/**
 * Creates command from the factory
 *
 * @param  type  The command type name such as Propagate, Maneuver
 * @param  name  The name of the command, usally blank
 * @param  retFlag  The return flag
 */
//------------------------------------------------------------------------------
GmatCommand* Moderator::CreateCommand(const wxString &type,
                                      const wxString &name, bool &retFlag)
{
   #if DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateCommand() entered: type = '%s', name = '%s'\n"),
       type.c_str(), name.c_str());
   #endif
   
   GmatCommand *cmd = theFactoryManager->CreateCommand(type, name);
   
   if (cmd == NULL)
   {
      //MessageInterface::PopupMessage
      //   (Gmat::ERROR_, wxT("Cannot create a Command type: %s.\n")
      //    wxT("Make sure %s is correct type and registered to Commandactory.\n"),
      //    type.c_str(), type.c_str());
      
      throw GmatBaseException
         (wxT("The Moderator cannot create a Command type \"") + type + wxT("\"\n"));
   }
   
   #ifdef DEBUG_MEMORY
   if (cmd)
   {
      wxString funcName;
      funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
      MemoryTracker::Instance()->Add
         (cmd, type, wxT("Moderator::CreateCommand()"), funcName);
   }
   #endif
   
   #if DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateCommand() returning <%p><%s>, retFlag=%d\n"), cmd,
       cmd->GetTypeName().c_str(), retFlag);
   #endif
   
   retFlag = true;
   return cmd;
}


//------------------------------------------------------------------------------
// GmatCommand* CreateDefaultCommand(const wxString &type,
//                                   const wxString &name,
//                                   GmatCommand *refCmd = NULL)
//------------------------------------------------------------------------------
/*
 * Creates a command with default settings. The input refCmd is only used for
 * EndFiniteBurn to match with BeginFiniteBurn. This method is usually called
 * from the GUI.
 *
 * @param  type  Command type
 * @param  name  Command name
 * @param  refCmd  Referenced command name
 *
 * @return  New command pointer
 */
//------------------------------------------------------------------------------
GmatCommand* Moderator::CreateDefaultCommand(const wxString &type,
                                             const wxString &name,
                                             GmatCommand *refCmd)
{
   #if DEBUG_DEFAULT_COMMAND
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateDefaultCommand() entered: type = ") +
       type + wxT(", name = ") + name + wxT("\n"));
   #endif
   
   GmatCommand *cmd = theFactoryManager->CreateCommand(type, name);
   
   if (cmd == NULL)
      throw GmatBaseException
         (wxT("The Moderator cannot create a Command type \"") + type + wxT("\"\n"));
   
   #ifdef DEBUG_MEMORY
   if (cmd)
   {
      wxString funcName;
      funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
      MemoryTracker::Instance()->Add
         (cmd, type, wxT("Moderator::CreateDefaultCommand()"), funcName);
   }
   #endif
   
   Integer id;
   
   try
   {
      if (type == wxT("If") || type == wxT("While"))
      {
         wxString str = GetDefaultSpacecraft()->GetName() + wxT(".ElapsedDays");
         cmd->SetCondition(str, wxT("<"), wxT("1.0"));
      }
      else if (type == wxT("For"))
      {
         CreateParameter(wxT("Variable"), wxT("I"));
         cmd->SetStringParameter(wxT("IndexName"), wxT("I"));
         cmd->SetStringParameter(wxT("StartName"), wxT("1"));
         cmd->SetStringParameter(wxT("EndName"), wxT("10"));
      }
      else if (type == wxT("Save"))
      {
         cmd->SetRefObjectName(Gmat::SPACECRAFT,
                               GetDefaultSpacecraft()->GetName());
      }
      else if (type == wxT("ClearPlot") || type == wxT("MarkPoint") ||
               type == wxT("PenUp") || type == wxT("PenDown"))
      {
         Subscriber *defSub = GetDefaultSubscriber(wxT("XYPlot"), false, false);
         if (defSub != NULL)
         {
            // Set default XYPlot
            if (defSub != NULL)
               cmd->SetStringParameter(cmd->GetParameterID(wxT("Subscriber")),
                                       defSub->GetName(), 0);
         }
         else
         {
            if (type == wxT("PenUp") || type == wxT("PenDown"))
            {
               // default XYPlot not found so set default GroundTrackPlot
               defSub = GetDefaultSubscriber(wxT("GroundTrackPlot"), false, false);          
               if (defSub != NULL)
                  cmd->SetStringParameter(cmd->GetParameterID(wxT("Subscriber")),
                                          defSub->GetName(), 0);
            }
         }
      }
      else if (type == wxT("Toggle"))
      {
         cmd->SetStringParameter(cmd->GetParameterID(wxT("Subscriber")),
                                 GetDefaultSubscriber(wxT("OrbitView"))->GetName());
      }
      else if (type == wxT("Report"))
      {
         Subscriber *sub = GetDefaultSubscriber(wxT("ReportFile"), false);
         Parameter *param = GetDefaultX();
         cmd->SetStringParameter(wxT("ReportFile"), sub->GetName());
         cmd->SetStringParameter(wxT("Add"), param->GetName());
         cmd->SetRefObject(sub, Gmat::SUBSCRIBER, sub->GetName(), 0);
         cmd->SetRefObject(param, Gmat::PARAMETER, param->GetName(), 0);
      }
      else if (type == wxT("Propagate"))
      {
         cmd->SetObject(GetDefaultPropSetup()->GetName(), Gmat::PROP_SETUP);
         
         StringArray formList = GetListOfObjects(Gmat::FORMATION);
         
         if (formList.size() == 0)
         {
            cmd->SetObject(GetDefaultSpacecraft()->GetName(), Gmat::SPACECRAFT);
         }
         else
         {
            // Get first spacecraft name not in formation
            wxString scName = GetSpacecraftNotInFormation();
            if (scName != wxT(""))
               cmd->SetObject(scName, Gmat::SPACECRAFT);
            else
               cmd->SetObject(formList[0], Gmat::SPACECRAFT);
         }
         
         cmd->SetRefObject(CreateDefaultStopCondition(), Gmat::STOP_CONDITION, wxT(""), 0);
         cmd->SetSolarSystem(theSolarSystemInUse);
      }
      else if (type == wxT("Maneuver"))
      {
         // set burn
         id = cmd->GetParameterID(wxT("Burn"));
         cmd->SetStringParameter(id, GetDefaultBurn(wxT("ImpulsiveBurn"))->GetName());
         
         // set spacecraft
         id = cmd->GetParameterID(wxT("Spacecraft"));
         cmd->SetStringParameter(id, GetDefaultSpacecraft()->GetName());
      }
      else if (type == wxT("BeginFiniteBurn"))
      {
         // set burn
         cmd->SetRefObjectName(Gmat::FINITE_BURN, GetDefaultBurn(wxT("FiniteBurn"))->GetName());
         
         // set spacecraft
         cmd->SetRefObjectName(Gmat::SPACECRAFT, GetDefaultSpacecraft()->GetName());
      }
      else if (type == wxT("EndFiniteBurn"))
      {
         // get burn name of BeginFiniteBurn
         if (refCmd)
         {
            // set burn
            cmd->SetRefObjectName(Gmat::FINITE_BURN,
                                  refCmd->GetRefObjectName(Gmat::FINITE_BURN));
            
            // set spacecraft
            StringArray scNames = refCmd->GetRefObjectNameArray(Gmat::SPACECRAFT);
            for (UnsignedInt i=0; i<scNames.size(); i++)
               cmd->SetRefObjectName(Gmat::SPACECRAFT, scNames[i]);
         }
         else
         {
            // set burn
            cmd->SetRefObjectName(Gmat::FINITE_BURN, GetDefaultBurn(wxT("FiniteBurn"))->GetName());
         
            // set spacecraft
            cmd->SetRefObjectName(Gmat::SPACECRAFT, GetDefaultSpacecraft()->GetName());
         }
      }
      else if (type == wxT("Target"))
      {
         // set solver
         Solver *solver = CreateSolver(wxT("DifferentialCorrector"),
                                       GetDefaultSolver()->GetName());
         id = cmd->GetParameterID(wxT("Targeter"));
         cmd->SetStringParameter(id, solver->GetName());
      }
      else if (type == wxT("Optimize"))
      {
         // set solver
         Solver *solver = CreateSolver(wxT("DifferentialCorrector"),
                                       GetDefaultSolver()->GetName());
         id = cmd->GetParameterID(wxT("OptimizerName"));
         cmd->SetStringParameter(id, solver->GetName());
      }
      else if (type == wxT("Vary"))
      {
         // set solver
         Solver *solver = CreateSolver(wxT("DifferentialCorrector"),
                                       GetDefaultSolver()->GetName());
         id = cmd->GetParameterID(wxT("SolverName"));
         cmd->SetStringParameter(id, solver->GetName());
         
         // set sover pointer, so that GetGeneratingString() can write correctly
         cmd->SetRefObject(solver, Gmat::SOLVER);
         
         // set variable parameter
         id = cmd->GetParameterID(wxT("Variable"));
         cmd->SetStringParameter(id, GetDefaultBurn(wxT("ImpulsiveBurn"))->GetName() + wxT(".Element1"));
         
         id = cmd->GetParameterID(wxT("InitialValue"));
         cmd->SetStringParameter(id, wxT("0.5"));
         
         id = cmd->GetParameterID(wxT("Perturbation"));
         cmd->SetStringParameter(id, wxT("0.0001"));
         
         id = cmd->GetParameterID(wxT("Lower"));
         cmd->SetStringParameter(id, wxT("0.0"));
         
         id = cmd->GetParameterID(wxT("Upper"));
         wxString ss(wxT(""));
         ss << GmatMathConstants::PI;
         cmd->SetStringParameter(id, ss);
      
         id = cmd->GetParameterID(wxT("MaxStep"));
         cmd->SetStringParameter(id, wxT("0.2"));
         
      }
      else if (type == wxT("Achieve"))
      {
         // Get default solver
         Solver *solver = GetDefaultSolver();

         #if DEBUG_DEFAULT_COMMAND
         MessageInterface::ShowMessage
            (wxT("Moderator::CreateDefaultCommand() cmd=%s, solver=%s\n"),
             cmd->GetTypeName().c_str(), solver->GetTypeName().c_str());
         #endif
         
         id = cmd->GetParameterID(wxT("TargeterName"));
         cmd->SetStringParameter(id, solver->GetName());
         
         // set goal parameter
         id = cmd->GetParameterID(wxT("Goal"));
         cmd->SetStringParameter(id, GetDefaultSpacecraft()->GetName() + wxT(".Earth.RMAG"));
         
         id = cmd->GetParameterID(wxT("GoalValue"));
         cmd->SetStringParameter(id, wxT("42165.0")); 
         
         id = cmd->GetParameterID(wxT("Tolerance"));
         cmd->SetStringParameter(id, wxT("0.1"));
      }
      else
      {
         // We need to set actual command string so that it can be saved to script
         wxString typeName = cmd->GetTypeName();
         wxString genStr = cmd->GetGeneratingString();
         // If there is comment only, prepend command string
         if (GmatStringUtil::StartsWith(genStr, wxT("%")))
            cmd->SetGeneratingString(typeName + wxT("; ") + genStr);
      }
      
      // for creating ElementWrapper
      #if DEBUG_DEFAULT_COMMAND
      MessageInterface::ShowMessage
         (wxT("   Moderator calling theScriptInterpreter->ValidateCommand()\n"));
      #endif
      theScriptInterpreter->ValidateCommand(cmd);
      
   }
   catch (BaseException &e)
   {
      MessageInterface::ShowMessage(e.GetFullMessage());
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
   
   #if DEBUG_DEFAULT_COMMAND
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateDefaultCommand() returning cmd=<%p><%s>\n"), cmd,
       cmd->GetTypeName().c_str());
   #endif
   
   return cmd;
}


//------------------------------------------------------------------------------
// bool AppendCommand(GmatCommand *cmd, Integer sandboxNum)
//------------------------------------------------------------------------------
/**
 * Appends command to last command
 *
 * @param  cmd  The command pointer to be appended
 * @param  sandboxNum  The sandbox number
 * @return  Return true if command successfully appended
 */
//------------------------------------------------------------------------------
bool Moderator::AppendCommand(GmatCommand *cmd, Integer sandboxNum)
{
   #if DEBUG_RESOURCE_COMMAND
   MessageInterface::ShowMessage
      (wxT("==========> Moderator::AppendCommand() cmd=(%p)%s\n"),
       cmd, cmd->GetTypeName().c_str());
   #endif
   
   // Get last command and append
   GmatCommand *lastCmd = GmatCommandUtil::GetLastCommand(commands[sandboxNum-1]);
   
   #if DEBUG_RESOURCE_COMMAND
   MessageInterface::ShowMessage
      (wxT("     lastCmd=(%p)%s\n"), lastCmd, lastCmd->GetTypeName().c_str());
   #endif
   
   if (lastCmd != NULL)
      return lastCmd->Append(cmd);
   else
      return commands[sandboxNum-1]->Append(cmd);
}


//------------------------------------------------------------------------------
// GmatCommand* AppendCommand(const wxString &type, const wxString &name,
//                           bool &retFlag, Integer sandboxNum)
//------------------------------------------------------------------------------
GmatCommand* Moderator::AppendCommand(const wxString &type,
                                      const wxString &name, bool &retFlag,
                                      Integer sandboxNum)
{
   #if DEBUG_RESOURCE_COMMAND
   MessageInterface::ShowMessage
      (wxT("==========> Moderator::AppendCommand() type='%s', name='%s'\n"),
       type.c_str(), name.c_str());
   #endif
   
   GmatCommand *cmd = theFactoryManager->CreateCommand(type, name);
   
   if (cmd != NULL)
   {
      #ifdef DEBUG_MEMORY
      if (cmd)
      {
         wxString funcName;
         funcName = currentFunction ? wxT("function: ") + currentFunction->GetName() : wxT("");
         MemoryTracker::Instance()->Add
            (cmd, type, wxT("Moderator::AppendCommand()"), funcName);
      }
      #endif
      
      retFlag = AppendCommand(cmd, sandboxNum);
   }
   else
   {
      throw GmatBaseException
         (wxT("The Moderator cannot create a Command type \"") + type + wxT("\"\n"));
   }
   
   #if DEBUG_RESOURCE_COMMAND
   MessageInterface::ShowMessage
      (wxT("==========> Moderator::AppendCommand() returning <%p>, retFlag=%d\n"),
       cmd, retFlag);
   #endif
   
   return cmd;
}


//------------------------------------------------------------------------------
// bool InsertCommand(GmatCommand *cmd, GmatCommand *prevCmd, Integer sandboxNum)
//------------------------------------------------------------------------------
/**
 * Inserts command into the command sequence after previous command
 * 
 * @param cmd  Pointer to GmatCommand that is inserted
 * @param prev Pointer to GmatCommand preceding this GmatCommand
 *
 * @return true on success, false on failure.
 */
//------------------------------------------------------------------------------
bool Moderator::InsertCommand(GmatCommand *cmd, GmatCommand *prevCmd,
                              Integer sandboxNum)
{
   #if DEBUG_COMMAND_INSERT
   MessageInterface::ShowMessage(wxT("==========> Moderator::InsertCommand() entered\n"));
   ShowCommand(wxT("     inserting cmd = "), cmd, wxT(" after prevCmd = "), prevCmd);
   #endif
   
   return commands[sandboxNum-1]->Insert(cmd, prevCmd);
}


//------------------------------------------------------------------------------
// GmatCommand* DeleteCommand(GmatCommand *cmd, Integer sandboxNum)
//------------------------------------------------------------------------------
/*
 * Removes a command from the command sequence. The caller has to delete the command.
 *
 * If deleting branch command,
 * it will remove and delete all children from the branch. If deleting ScriptEvent,
 * it will remove and delete all commands including EndScrpt between BeginScrint
 * and EndScript.
 */
//------------------------------------------------------------------------------
GmatCommand* Moderator::DeleteCommand(GmatCommand *cmd, Integer sandboxNum)
{
   #if DEBUG_COMMAND_DELETE
   ShowCommand(wxT("==========> Moderator::DeleteCommand() cmd = "), cmd);
   #endif
   
   if (cmd == NULL)
      return NULL;
   
   GmatCommand *remvCmd;
   if (cmd->GetTypeName() != wxT("BeginScript"))
   {
      GmatCommand *remvCmd = commands[sandboxNum-1]->Remove(cmd);
      
      #if DEBUG_COMMAND_DELETE
      ShowCommand(wxT("   Removed = "), remvCmd);
      #endif
      
      #if DEBUG_COMMAND_DELETE
      ShowCommand(wxT("==========> Moderator::DeleteCommand() Returning "), remvCmd);
      #endif
      
      return remvCmd;
   }
   
   //-------------------------------------------------------
   // Remove commands inside Begin/EndScript block
   //-------------------------------------------------------

   // Check for previous command, it should not be NULL,
   // since wxT("NoOp") is the first command
   
   GmatCommand *prevCmd = cmd->GetPrevious();
   if (prevCmd == NULL)
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_, wxT("Moderator::DeleteCommand() *** INTERNAL ERROR *** \n")
          wxT("The previous command cannot be NULL.\n"));
      return NULL;
   }
   
   GmatCommand *first = GetFirstCommand();
   
   #if DEBUG_COMMAND_DELETE
   wxString cmdString1 = GmatCommandUtil::GetCommandSeqString(first);
   MessageInterface::ShowMessage(wxT("     ==> Current sequence:"));
   MessageInterface::ShowMessage(cmdString1);
   #endif
   
   GmatCommand *current = cmd->GetNext();
   
   #if DEBUG_COMMAND_DELETE
   GmatCommand *nextCmd = GmatCommandUtil::GetNextCommand(cmd);
   ShowCommand(wxT("     prevCmd = "), prevCmd, wxT(" nextCmd = "), nextCmd);
   #endif
   
   // Get matching EndScript for BeginScript
   GmatCommand *endScript = GmatCommandUtil::GetMatchingEnd(cmd);
   
   #if DEBUG_COMMAND_DELETE
   ShowCommand(wxT("     endScript = "), endScript);
   #endif
   
   GmatCommand* next;
   while (current != NULL)
   {
      #if DEBUG_COMMAND_DELETE
      ShowCommand(wxT("     current = "), current);
      #endif
      
      if (current == endScript)
         break;
      
      next = current->GetNext();
      
      #if DEBUG_COMMAND_DELETE
      ShowCommand(wxT("     removing and deleting "), current);
      #endif
      
      remvCmd = cmd->Remove(current);
      
      // check remvCmd first
      if (remvCmd != NULL)
      {
         remvCmd->ForceSetNext(NULL);
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (remvCmd, remvCmd->GetTypeName(), wxT("Moderator::DeleteCommand()"));
         #endif
         delete remvCmd;
      }
      
      current = next;
   }
   
   //-------------------------------------------------------
   // Remove and delete EndScript
   //-------------------------------------------------------
   #if DEBUG_COMMAND_DELETE
   ShowCommand(wxT("     removing and deleting "), current);
   #endif
   
   remvCmd = cmd->Remove(current);
   remvCmd->ForceSetNext(NULL);
   
   if (remvCmd != NULL)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (remvCmd, remvCmd->GetTypeName(), wxT("Moderator::DeleteCommand()"));
      #endif
      delete remvCmd;
      remvCmd = NULL;
   }
   
   next = cmd->GetNext();
   
   #if DEBUG_COMMAND_DELETE
   ShowCommand(wxT("     next    = "), next, wxT(" nextCmd = "), nextCmd);
   #endif
   
   //-------------------------------------------------------
   // Remove and delete BeginScript
   //-------------------------------------------------------
   #if DEBUG_COMMAND_DELETE
   ShowCommand(wxT("     removing and deleting "), cmd);
   #endif
   
   // Remove BeginScript
   remvCmd = first->Remove(cmd);
   
   // Set next command NULL
   cmd->ForceSetNext(NULL);
   if (cmd != NULL)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (cmd, cmd->GetTypeName(), wxT("Moderator::DeleteCommand()"));
      #endif
      delete cmd;
      cmd = NULL;
   }
   
   #if DEBUG_COMMAND_DELETE
   wxString cmdString2 = GmatCommandUtil::GetCommandSeqString(first);
   MessageInterface::ShowMessage(wxT("     ==> sequence after delete:"));
   MessageInterface::ShowMessage(cmdString2);
   ShowCommand(wxT("==========> Moderator::DeleteCommand() Returning cmd = "), cmd);
   #endif
   
   // Just return cmd, it should be deleted by the caller.
   return cmd;
}


//------------------------------------------------------------------------------
// GmatCommand* GetFirstCommand(Integer sandboxNum)
//------------------------------------------------------------------------------
GmatCommand* Moderator::GetFirstCommand(Integer sandboxNum)
{
   if (commands.empty())
   {
      return NULL;
   }
   return commands[sandboxNum-1];
}


//------------------------------------------------------------------------------
// void SetCommandsUnchanged(Integer whichList)
//------------------------------------------------------------------------------
/**
 * Resets the command list to the unchanged state.
 * 
 * @param <whichList>   index indicating which command list gets updated
 */
//------------------------------------------------------------------------------
void Moderator::SetCommandsUnchanged(Integer whichList)
{
   commands[whichList]->ConfigurationChanged(false, true);
}


//------------------------------------------------------------------------------
// void ValidateCommand(GmatCommand *cmd)
//------------------------------------------------------------------------------
/**
 * Validates the command.
 */
//------------------------------------------------------------------------------
void Moderator::ValidateCommand(GmatCommand *cmd)
{
   theScriptInterpreter->ValidateCommand(cmd);
}


// CoordinateSystem
//------------------------------------------------------------------------------
// CoordinateSystem* GetInternalCoordinateSystem()
//------------------------------------------------------------------------------
/**
 * @return internal CoordinateSystem.
 */
//------------------------------------------------------------------------------
CoordinateSystem* Moderator::GetInternalCoordinateSystem()
{
   return theInternalCoordSystem;
}

//Planetary files
//------------------------------------------------------------------------------
// const StringArray& GetPlanetarySourceTypes()
//------------------------------------------------------------------------------
/**
 * @return a planetary source types
 */
//------------------------------------------------------------------------------
const StringArray& Moderator::GetPlanetarySourceTypes()
{
   return theSolarSystemInUse->GetPlanetarySourceTypes();
}


//------------------------------------------------------------------------------
// const StringArray& GetPlanetarySourceNames()
//------------------------------------------------------------------------------
/**
 * @return a planetary source file names
 */
//------------------------------------------------------------------------------
const StringArray& Moderator::GetPlanetarySourceNames()
{
   return theSolarSystemInUse->GetPlanetarySourceNames();
}


//------------------------------------------------------------------------------
// const StringArray& GetPlanetarySourceTypesInUse()
//------------------------------------------------------------------------------
/**
 * @return a planetary source types in use
 */
//------------------------------------------------------------------------------
const StringArray& Moderator::GetPlanetarySourceTypesInUse()
{
   return theSolarSystemInUse->GetPlanetarySourceTypesInUse();
}


////------------------------------------------------------------------------------
//// StringArray& GetAnalyticModelNames()
////------------------------------------------------------------------------------
///**
// * @return available planetary analytic model names.
// */
////------------------------------------------------------------------------------
//const StringArray& Moderator::GetAnalyticModelNames()
//{
//   return theSolarSystemInUse->GetAnalyticModelNames();
//}
//
//
////------------------------------------------------------------------------------
//// bool SetAnalyticModelToUse(const wxString &modelName)
////------------------------------------------------------------------------------
//bool Moderator::SetAnalyticModelToUse(const wxString &modelName)
//{
//   return theSolarSystemInUse->SetAnalyticModelToUse(modelName);
//}


//------------------------------------------------------------------------------
// bool SetPlanetarySourceName(const wxString &sourceType,
//                           const wxString &fileName)
//------------------------------------------------------------------------------
bool Moderator::SetPlanetarySourceName(const wxString &sourceType,
                                       const wxString &fileName)
{
   return theSolarSystemInUse->SetPlanetarySourceName(sourceType, fileName);
}


//------------------------------------------------------------------------------
// wxString GetPlanetarySourceName(const wxString &sourceType)
//------------------------------------------------------------------------------
wxString Moderator::GetPlanetarySourceName(const wxString &sourceType)
{
   return theSolarSystemInUse->GetPlanetarySourceName(sourceType);
}


//------------------------------------------------------------------------------
// Integer SetPlanetarySourceTypesInUse(const StringArray &sourceTypes)
//------------------------------------------------------------------------------
/*
 * @param <sourceTypes> list of file type in the priority order of use
 *
 * @return 0, if error setting any of planetary file in the list.
 *         1, if error setting first planetary file in the list, but set to
 *            next available file.
 *         2, if successfuly set to first planetary file in the list
 */
//------------------------------------------------------------------------------
Integer Moderator::SetPlanetarySourceTypesInUse(const StringArray &sourceTypes)
{
   return theSolarSystemInUse->SetPlanetarySourceTypesInUse(sourceTypes);
}


//------------------------------------------------------------------------------
// Integer GetPlanetarySourceId(const wxString &sourceType)
//------------------------------------------------------------------------------
Integer Moderator::GetPlanetarySourceId(const wxString &sourceType)
{
   return theSolarSystemInUse->GetPlanetarySourceId(sourceType);
}


//------------------------------------------------------------------------------
// wxString GetPlanetarySourceNameInUse()
//------------------------------------------------------------------------------
wxString Moderator::GetCurrentPlanetarySource()
{
   return theSolarSystemInUse->GetCurrentPlanetarySource();
}


// Potential field files
//------------------------------------------------------------------------------
// wxString GetPotentialFileName(const wxString &fileType)
//------------------------------------------------------------------------------
wxString Moderator::GetPotentialFileName(const wxString &fileType)
{
   if (fileType == wxT("JGM2"))
      return theFileManager->GetFullPathname(wxT("JGM2_FILE"));
   else if (fileType == wxT("JGM3"))
      return theFileManager->GetFullPathname(wxT("JGM3_FILE"));
   else if (fileType == wxT("EGM96"))
      return theFileManager->GetFullPathname(wxT("EGM96_FILE"));
   else if (fileType == wxT("LP165P"))
      return theFileManager->GetFullPathname(wxT("LP165P_FILE"));
   else if (fileType == wxT("MGNP180U"))
      return theFileManager->GetFullPathname(wxT("MGNP180U_FILE"));
   else if (fileType == wxT("MARS50C"))
      return theFileManager->GetFullPathname(wxT("MARS50C_FILE"));
   else
      return wxT("Unknown Potential File Type:") + fileType;
}


//------------------------------------------------------------------------------
// wxString GetFileName(const wxString &fileType)
//------------------------------------------------------------------------------
wxString Moderator::GetFileName(const wxString &fileType)
{
   return theFileManager->GetFullPathname(fileType);
}


// Mission
//------------------------------------------------------------------------------
// bool LoadDefaultMission()
//------------------------------------------------------------------------------
bool Moderator::LoadDefaultMission()
{
   #if DEBUG_DEFAULT_MISSION
   MessageInterface::ShowMessage(wxT("Moderator::LoadDefaultMission() entered\n"));
   #endif
   
   theScriptInterpreter->SetHeaderComment(wxT(""));
   theScriptInterpreter->SetFooterComment(wxT(""));
   
   ClearCommandSeq(true, true);
   ClearResource();
   
   // Set object manage option to configuration
   objectManageOption = 1;
   
   CreateDefaultMission();
   
   #if DEBUG_DEFAULT_MISSION
   MessageInterface::ShowMessage(wxT("Moderator::LoadDefaultMission() leaving\n"));
   #endif
   
   return true;
}

// Resource
//------------------------------------------------------------------------------
// bool ClearResource()
//------------------------------------------------------------------------------
bool Moderator::ClearResource()
{
   #if DEBUG_SEQUENCE_CLEARING
   MessageInterface::ShowMessage(wxT("Moderator::ClearResource() entered\n"));
   MessageInterface::ShowMessage(wxT("   Removing configured objects...\n"));
   #endif
   
   theConfigManager->RemoveAllItems();
   
   #if DEBUG_SEQUENCE_CLEARING
   MessageInterface::ShowMessage(wxT("   Clearing Sandbox objects...\n"));
   #endif
   
   ClearAllSandboxes();
   
   // Delete solar system in use. We want to begin with default solar system
   // before creating default mission or new script is read.
   #ifndef __DISABLE_SOLAR_SYSTEM_CLONING__
   // Do not delete SolarSystem in case user wants to create a new mission from
   // the GUI after getting errors in script build. This will fix Bug 1532 (LOJ: 2009.11.13)
   if (!isRunReady && endOfInterpreter)
   {
      #if DEBUG_SEQUENCE_CLEARING | DEBUG_FINALIZE > 0
      MessageInterface::ShowMessage
         (wxT(".....Mod::ClearResource - <%p>theSolarSystemInUse was not deleted since ")
          wxT("there was script errors\n"), theSolarSystemInUse);
      #endif
   }
   else
   {
      if (theSolarSystemInUse != NULL)
      {
         #if DEBUG_SEQUENCE_CLEARING | DEBUG_FINALIZE > 0
         MessageInterface::ShowMessage
            (wxT(".....Mod::ClearResource - deleting (%p)theSolarSystemInUse\n"), theSolarSystemInUse);
         #endif
         if (theInternalSolarSystem == theSolarSystemInUse) theInternalSolarSystem = NULL;
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (theSolarSystemInUse, theSolarSystemInUse->GetName(),
             wxT("deleting theSolarSystemInUse in Moderator::ClearResource()"));
         #endif
         
         delete theSolarSystemInUse;
         theSolarSystemInUse = NULL;
      }
   }
   #endif
   
   #if DEBUG_SEQUENCE_CLEARING
   MessageInterface::ShowMessage(wxT("Moderator::ClearResource() returning true\n"));
   #endif
   
   return true;
}

// Command Sequence
//------------------------------------------------------------------------------
// bool ClearCommandSeq(bool leaveFirstCmd, bool callRunComplete,
//                     Integer sandboxNum)
//------------------------------------------------------------------------------
/*
 * Deletes whole command sequence.
 *
 * @param  leaveFirstCmd  Set this flag to true if the first command should be
 *                        left undeleted (true)
 * @param  callRunComplete  Set this flag to true if RunComplete() should be
 *                        called for all commands (true)
 * @param  sandBoxNum  The sandobx number (1)
 */
//------------------------------------------------------------------------------
bool Moderator::ClearCommandSeq(bool leaveFirstCmd, bool callRunComplete,
                                Integer sandboxNum)
{
   #if DEBUG_SEQUENCE_CLEARING
   MessageInterface::ShowMessage(wxT("Moderator::ClearCommandSeq() entered\n"));
   #endif
   
   if (commands.empty())
   {
      #if DEBUG_SEQUENCE_CLEARING
      MessageInterface::ShowMessage
         (wxT("Moderator::ClearCommandSeq() exiting, command array is empty\n"));
      #endif
      return true;
   }
   
   GmatCommand *cmd = commands[sandboxNum-1];
   bool retval =
      GmatCommandUtil::ClearCommandSeq(cmd, leaveFirstCmd, callRunComplete);
   
   #if DEBUG_SEQUENCE_CLEARING
   MessageInterface::ShowMessage
      (wxT("Moderator::ClearCommandSeq() returning %s\n", retval ? "true" : "false"));
   #endif
   
   return retval;
}


// sandbox
//------------------------------------------------------------------------------
// void ClearAllSandboxes()
//------------------------------------------------------------------------------
void Moderator::ClearAllSandboxes()
{
   for (UnsignedInt i=0; i<sandboxes.size(); i++)
      if (sandboxes[i])
         sandboxes[i]->Clear();
   
   #ifdef DEBUG_MEMORY
   StringArray tracks = MemoryTracker::Instance()->GetTracks(false, false);
   MessageInterface::ShowMessage
      (wxT("===> There are %d memory tracks after Sandbox clear\n"), tracks.size());
   #endif
}


//------------------------------------------------------------------------------
// GmatBase* GetInternalObject(const wxString &name, Integer sandboxNum = 1)
//------------------------------------------------------------------------------
GmatBase* Moderator::GetInternalObject(const wxString &name, Integer sandboxNum)
{
   return sandboxes[sandboxNum-1]->GetInternalObject(name);
}


//------------------------------------------------------------------------------
// Integer RunMission(Integer sandboxNum)
//------------------------------------------------------------------------------
/*
 * Adds configured objects to sandbox and execute. The number of sandbox created
 * is declared in ther header as Gmat::MAX_SANDBOX. But currently only 1 sandbox
 * is used for running the mission.
 *
 * @param  sandboxNum  The sandbox number (1 to Gmat::MAX_SANDBOX)
 *
 * @return  1 if run was successful
 *         -1 if sandbox number is invalid
 *         -2 if execution interrupted by user
 *         -3 if exception thrown during the run
 *         -4 if unknown error occurred
 */
//------------------------------------------------------------------------------
Integer Moderator::RunMission(Integer sandboxNum)
{
   //MessageInterface::ShowMessage(wxT("\n========================================\n"));
   //MessageInterface::ShowMessage("Moderator::RunMission() entered\n");
   MessageInterface::ShowMessage(wxT("Running mission...\n"));
   Integer status = 1;
   // Set to 1 to always run the mission and get the sandbox error message
   // Changed this code while looking at Bug 1532 (LOJ: 2009.11.13)
   isRunReady = 1;
   
   #if DEBUG_CONFIG
   MessageInterface::ShowMessage
      (wxT("Moderator::RunMission() HasConfigurationChanged()=%d\n"), HasConfigurationChanged());
   #endif
   
   clock_t t1 = clock(); // Should I clock after initilization?
   
   if (isRunReady)
   {
      // clear sandbox
      if (sandboxNum > 0 && sandboxNum <= Gmat::MAX_SANDBOX)
      {
         #if DEBUG_RUN
         MessageInterface::ShowMessage
            (wxT("Moderator::RunMission() before sandboxes[%d]->Clear()\n"), sandboxNum-1);
         #endif
         
         sandboxes[sandboxNum-1]->Clear();
         
         #if DEBUG_RUN
         MessageInterface::ShowMessage
            (wxT("Moderator::RunMission() after sandboxes[%d]->Clear()\n"), sandboxNum-1);
         #endif
      }
      else
      {
         status = -1;
         MessageInterface::PopupMessage(Gmat::ERROR_,
                                        wxT("Invalid Sandbox number") + sandboxNum);
         return status;
      }
      
      try
      {
         
         // add objects to sandbox
         AddSolarSystemToSandbox(sandboxNum-1);
         AddTriggerManagersToSandbox(sandboxNum-1);
         AddInternalCoordSystemToSandbox(sandboxNum-1);
         AddPublisherToSandbox(sandboxNum-1);
         AddSubscriberToSandbox(sandboxNum-1);
         AddOtherObjectsToSandbox(sandboxNum-1);
         
         // add command sequence to sandbox
         AddCommandToSandbox(sandboxNum-1);
         
         #if DEBUG_RUN
         MessageInterface::ShowMessage
            (wxT("Moderator::RunMission() after AddCommandToSandbox()\n"));
         #endif
         
         // initialize Sandbox
         InitializeSandbox(sandboxNum-1);
         

         if (!loadSandboxAndPause)
         {
            #if DEBUG_RUN
            MessageInterface::ShowMessage
               (wxT("Moderator::RunMission() after InitializeSandbox()\n"));
            #endif

            // reset user interrupt flag
            GmatGlobal::Instance()->SetRunInterrupted(false);

            // execute sandbox
            runState = Gmat::RUNNING;
            ExecuteSandbox(sandboxNum-1);

            #if DEBUG_RUN
            MessageInterface::ShowMessage
               (wxT("Moderator::RunMission() after ExecuteSandbox()\n"));
            #endif
         }
         else
         {
            // Execute only the PrepareMissionSequence command
            GmatCommand *cmd = commands[sandboxNum-1]->GetNext();
            if (cmd->GetTypeName() == wxT("PrepareMissionSequence"))
               cmd->Execute();
         }
      }
      catch (BaseException &e)
      {
         wxString msg = e.GetFullMessage();
         
         // assign status
         // Look for "interrupted" (loj: 2008.02.05)
         //if (msg.find("Execution interrupted") != msg.npos)
         if (msg.find(wxT("interrupted")) != msg.npos)
         {
            status = -2;
            MessageInterface::ShowMessage(wxT("GMAT execution stopped by user.\n"));      
         }
         else
         {
            status = -3;
//            msg = wxT("**** ERROR **** ") + msg;
            // Dunn would like to note that this is the popup message we were
            // getting that only said "ERROR" and did not provide a message.
            // We might want to debug that some day.
            MessageInterface::PopupMessage(Gmat::ERROR_, msg + wxT("\n"));
         }
      }
      catch (...)
      {
         MessageInterface::ShowMessage
            (wxT("Moderator::RunMission() Unknown error occurred.\n"));
         status = -4;
         //throw; // LOJ: We want to finish up the clearing process below
      }
   }
   else
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_, wxT("Cannot Run Mission. No mission sequence defined.\n"));
      status = -4;
   }
   
   runState = Gmat::IDLE;
   thePublisher->SetRunState(runState);
   thePublisher->NotifyEndOfRun();
   if (theUiInterpreter != NULL)
      theUiInterpreter->NotifyRunCompleted();
   
   #if DEBUG_RUN > 1
   MessageInterface::ShowMessage(wxT("===> status=%d\n"), status);
   #endif
   
   if (status == 1)
      MessageInterface::ShowMessage(wxT("Mission run completed.\n"));
   else if (status == -2)
      MessageInterface::ShowMessage(wxT("*** Mission run interrupted.\n"));
   else
      MessageInterface::ShowMessage(wxT("*** Mission run failed.\n"));
   
   clock_t t2 = clock();
   MessageInterface::ShowMessage
      (wxT("===> Total Run Time: %f seconds\n"), (Real)(t2-t1)/CLOCKS_PER_SEC);
   
   #ifdef DEBUG_MEMORY
   StringArray tracks = MemoryTracker::Instance()->GetTracks(false, false);
   MessageInterface::ShowMessage
      (wxT("===> There are %d memory tracks after MissionRun\n"), tracks.size());
   #endif
   
   // show final state
   #ifdef __SHOW_FINAL_STATE__
   showFinalState = true;
   #endif
   
   if (showFinalState)
   {
      GmatCommand *cmd = GetFirstCommand();
      MessageInterface::ShowMessage(GmatCommandUtil::GetCommandSeqString(cmd));
      GmatCommand *lastCmd = GmatCommandUtil::GetLastCommand(cmd);
      
      MessageInterface::ShowMessage(wxT("\n========== Final State ==========\n"));
      MessageInterface::ShowMessage(lastCmd->GetStringParameter(wxT("MissionSummary")));
      MessageInterface::ShowMessage(wxT("\n\n"));      
   }
   else
   {
      MessageInterface::ShowMessage(wxT("\n========================================\n"));
   }
   
   // Reset solar system in use and object map (LOJ: 2009.03.19)
   // So that users can create new objects from the GUI after GmatFunction run.
   objectMapInUse = theConfigManager->GetObjectMap();
   SetSolarSystemAndObjectMap(theSolarSystemInUse, objectMapInUse, false,
                              wxT("RunMission()"));
   
   return status;
} // RunMission()


//------------------------------------------------------------------------------
// Integer ChangeRunState(const wxString &state, Integer sandboxNum)
//------------------------------------------------------------------------------
/**
 * Changes run state.
 *
 * @param <state> run state string ("Stop", "Pause", "Resume")
 * @param <snadobxNum> sandbox number
 *
 * @return a status code
 *    0 = successful, <0 = error (tbd)
 */
//------------------------------------------------------------------------------
Integer Moderator::ChangeRunState(const wxString &state, Integer sandboxNum)
{
   #if DEBUG_USER_INTERRUPT
   MessageInterface::ShowMessage
      (wxT("Moderator::ChangeRunState(%s) entered\n"), state.c_str());
   #endif
   
   if (state == wxT("Stop"))
   {
      runState = Gmat::IDLE;
      GmatGlobal::Instance()->SetRunInterrupted(true);
   }
   
   else if (state == wxT("Pause"))
      runState = Gmat::PAUSED;
   
   else if (state == wxT("Resume"))
      runState = Gmat::RUNNING;
   
   else
      ; // no action
   
   return 0;
}


//------------------------------------------------------------------------------
// Gmat::RunState GetUserInterrupt()
//------------------------------------------------------------------------------
/**
 * Checks to see if the user has requested that the run stop or pause.
 * 
 * This method is called by the Sandbox periodically during a run to determine
 * if the user has requested that the run terminate before the mission sequence
 * has finished executing.
 * 
 * @return The expected state of the system (RUNNING, PAUSED, or IDLE).
 */
//------------------------------------------------------------------------------
Gmat::RunState Moderator::GetUserInterrupt()
{
   #if DEBUG_USER_INTERRUPT
   MessageInterface::ShowMessage(wxT("Moderator::GetUserInterrupt() entered\n"));
   #endif
   
   // give MainFrame input focus
   if (theUiInterpreter != NULL)
      theUiInterpreter->SetInputFocus();
   return runState;
}


//------------------------------------------------------------------------------
// Gmat::RunState GetRunState()
//------------------------------------------------------------------------------
/**
 * @return the state of the system (Gmat::RUNNING, Gmat::PAUSED, Gmat::IDLE)
 */
//------------------------------------------------------------------------------
Gmat::RunState Moderator::GetRunState()
{
   #if DEBUG_RUN
   MessageInterface::ShowMessage
      (wxT("Moderator::GetRunsState() isRunReady=%d, endOfInterpreter=%d\n"),
       isRunReady, endOfInterpreter);
   #endif
   
   // return RUNNING so that Matlab can wait for building objects
   if (!isRunReady && !endOfInterpreter)
      return Gmat::RUNNING;
   
   #if DEBUG_RUN
   MessageInterface::ShowMessage
      (wxT("Moderator::GetRunsState() runState=%d\n"), runState);
   #endif

   return runState;
}


// Script
//------------------------------------------------------------------------------
// bool InterpretScript(const wxString &filename, bool readBack = false,
//                      const wxString &newPath = wxT(""))
//------------------------------------------------------------------------------
/**
 * Creates objects from script file. If readBack is true, it will save to
 * to new directory and interpret from it.  If newPath is blank "", then
 * it will create default directory "AutoSaved".
 *
 * @param <filename> input script file name
 * @param <readBack> true will read scripts, save, and read back in
 * @param <newPath> new path to be used for saving scripts
 *
 * @return true if successful; false otherwise
 */
//------------------------------------------------------------------------------
bool Moderator::InterpretScript(const wxString &filename, bool readBack,
                                const wxString &newPath)
{
   bool isGoodScript = false;
   isRunReady = false;
   endOfInterpreter = false;
   runState = Gmat::IDLE;
   
   //MessageInterface::ShowMessage(wxT("========================================\n"));
   MessageInterface::ShowMessage
      (wxT("\nInterpreting scripts from the file.\n***** file: ") + filename + wxT("\n"));
   
   try
   {
      PrepareNextScriptReading();
      isGoodScript = theScriptInterpreter->Interpret(filename);
      
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->SetScript(filename);
      #endif
      
      if (readBack)
      {
         #if DEBUG_INTERPRET
         MessageInterface::ShowMessage(wxT("===> newPath=%s\n"), newPath.c_str());
         #endif
         
         wxString newpath = newPath;
         wxString sep = theFileManager->GetPathSeparator();
         UnsignedInt index = filename.find_last_of(wxT("/\\"));
         wxString fname = filename.substr(index+1);
         
         if (newpath == wxT(""))
            newpath = filename.substr(0, index) + sep + wxT("AutoSave") + sep;
         
         wxString newfile = newpath + fname;
         
         #if DEBUG_INTERPRET
         MessageInterface::ShowMessage
            (wxT("===> newpath=%s\n===> newfile=%s\n"), newpath.c_str(), newfile.c_str());
         #endif
         
         if (!theFileManager->DoesDirectoryExist(newpath))
         {
            wxString cmd = wxT("mkdir ") + newpath;
            
            int status = system(cmd.char_str());
            if (status != 0)
            {
               #if DEBUG_INTERPRET
               MessageInterface::ShowMessage
                  (wxT("===> cmd=%s, status=%d\n"), cmd.c_str(), status);
               #endif
            }
         }
         
         SaveScript(newfile);
         InterpretScript(newfile);
      }
      
      if (isGoodScript)
      {
         #if DEBUG_INTERPRET
         MessageInterface::ShowMessage
            (wxT("Moderator::InterpretScript() successfully interpreted the script\n"));
         #endif
         
         isRunReady = true;
      }
      else
      {
         MessageInterface::ShowMessage(wxT("\n========================================\n"));
      }
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      isRunReady = false;
   }
   
   ResetConfigurationChanged();
   endOfInterpreter = true;
   
   if (isGoodScript)
   {
      // Append BeginMissionSequence command if not there (New since 2010.07.09)
      GmatCommand *first = GetFirstCommand();
      GmatCommand *second = first->GetNext();

      
      #if DEBUG_INTERPRET
      ShowCommand(wxT("first cmd = "), first, wxT(" second cmd = "), second);
      #endif
      
      wxString firstCommandType = 
         (second != NULL ? second->GetTypeName() : wxT(""));
      
      if (!IsSequenceStarter(firstCommandType))
      {
         // Show warning message for now (LOJ: 2010.07.15)
         wxString firstCmdStr;
         if (second == NULL)
         {
            firstCmdStr = wxT("There is no command detected.");
         }
         else
         {
            //if (second != NULL)            
            firstCmdStr = wxT("The first command detected is \n'");
            firstCmdStr = firstCmdStr +
               second->GetGeneratingString(Gmat::NO_COMMENTS) + wxT("'");
         }
         
         wxString knownStartCommands = wxT("   [") + GetStarterStringList() + wxT("]\n");
         //firstCmdStr = firstCmdStr + second->GetGeneratingString() + "'";
         MessageInterface::PopupMessage
            (Gmat::WARNING_, wxT("*** WARNING *** Mission Sequence start command ")
             wxT("is missing.  One will be required in future builds.  Recognized ")
             wxT("start commands are\n") + knownStartCommands + firstCmdStr);
         
         #if DEBUG_INTERPRET
         MessageInterface::ShowMessage
            (wxT("==> Inserting 'BeginMissionSequence' after '%s'\n"),
             first->GetTypeName().c_str());
         #endif
         bool retval;
         GmatCommand *bms = CreateCommand(wxT("BeginMissionSequence"), wxT(""), retval);
         InsertCommand(bms, first);
      }
      
      if (second != NULL && second->GetTypeName() == wxT("PrepareMissionSequence"))
         loadSandboxAndPause = true;
      else
         loadSandboxAndPause = false;
      
      #if DEBUG_INTERPRET > 1
      MessageInterface::ShowMessage(GetScript());
      #endif
      
      #if DEBUG_INTERPRET > 0
      GmatCommand *cmd = GetFirstCommand();
      MessageInterface::ShowMessage(GmatCommandUtil::GetCommandSeqString(cmd));
      MessageInterface::ShowMessage(wxT("Moderator::InterpretScript() returning %d\n"), isGoodScript);
      #endif

   }
   
   return isGoodScript;
}


//------------------------------------------------------------------------------
// bool InterpretScript(std::istringstream *ss, bool clearObjs)
//------------------------------------------------------------------------------
/**
 * Creates objects from stringstream
 *
 * @param <ss> input istringstream
 * @param <clearObjs> clears objects and mission sequence if true
 * @return true if successful; false otherwise
 */
//------------------------------------------------------------------------------
bool Moderator::InterpretScript(wxInputStream *ss, bool clearObjs)
{
   bool isGoodScript = false;
   isRunReady = false;
   endOfInterpreter = false;
   runState = Gmat::IDLE;
   
   //MessageInterface::ShowMessage(wxT("========================================\n"));
   MessageInterface::ShowMessage
      (wxT("\nInterpreting scripts from the input stream\n"));
   
   try
   {
      PrepareNextScriptReading(clearObjs);
      
      // Set istream and Interpret
      theScriptInterpreter->SetInStream(ss);
      isGoodScript = theScriptInterpreter->Interpret();
      
      if (isGoodScript)
      {
         #if DEBUG_INTERPRET
         MessageInterface::ShowMessage
            (wxT("Moderator::InterpretScript() successfully interpreted the script\n"));
         #endif
         
         isRunReady = true;
      }
      else
      {
         MessageInterface::ShowMessage(wxT("\n========================================\n"));
      }
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      isRunReady = false;
   }
   
   ResetConfigurationChanged();
   endOfInterpreter = true;
   
   #if DEBUG_INTERPRET
   GmatCommand *cmd = GetFirstCommand();
   MessageInterface::ShowMessage(GmatCommandUtil::GetCommandSeqString(cmd));
   MessageInterface::ShowMessage(GetScript());
   #endif
   
   return isGoodScript;
}


//------------------------------------------------------------------------------
// bool SaveScript(const wxString &filename,
//                 Gmat::WriteMode mode = Gmat::SCRIPTING)
//------------------------------------------------------------------------------
/**
 * Builds scripts from objects and write to a file.
 *
 * @param <filename> output script file name
 * @param <writeMode> write mode object(one of Gmat::SCRIPTING, Gmat::MATLAB_STRUCT)
 *
 * @return true if successful; false otherwise
 */
//------------------------------------------------------------------------------
bool Moderator::SaveScript(const wxString &filename, Gmat::WriteMode mode)
{
   #ifdef DEBUG_SAVE_SCRIPT
   MessageInterface::ShowMessage
      (wxT("Moderator::SaveScript() entered\n   file: %s, mode: %d\n"),
       filename.c_str(), mode);
   MessageInterface::ShowMessage(wxT("The Script is saved to " + filename + "\n"));
   #endif
   
   bool status = false;
   
   try
   {
      status = theScriptInterpreter->Build(filename, mode);
      if (status)
         ResetConfigurationChanged();
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
   
   return status;
}


//------------------------------------------------------------------------------
// wxString GetScript(Gmat::WriteMode mode = Gmat::SCRIPTING)
//------------------------------------------------------------------------------
/**
 * Returns built scripts from objects
 *
 * @param <writeMode> write mode object(one of Gmat::SCRIPTING, Gmat::MATLAB_STRUCT)
 *
 * @return built scripts from objects
 */
//------------------------------------------------------------------------------
wxString Moderator::GetScript(Gmat::WriteMode mode)
{
   //MessageInterface::ShowMessage(wxT("Moderator::GetScript() mode: %d\n"), mode);
   
   try
   {
      wxStringOutputStream osStringStream;
      theScriptInterpreter->SetOutStream(&osStringStream);
      
      if (theScriptInterpreter->Build(mode))
      {
         return osStringStream.GetString();
      }
      else
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("Unable to build script from objects\n"));
         return wxT("");
      }
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage() + wxT("\n"));
      return wxT("");
   }
}


//------------------------------------------------------------------------------
// Integer RunScript(Integer sandboxNum = 1)
//------------------------------------------------------------------------------
/**
 * Executes commands built from the script file.
 *
 * @param <sandboxNum> sandbox number
 *
 * @return a status code
 *    0 = successful, <0 = error (tbd)
 */
//------------------------------------------------------------------------------
Integer Moderator::RunScript(Integer sandboxNum)
{
   MessageInterface::ShowMessage(wxT("Moderator::RunScript() entered\n"));
   return RunMission(sandboxNum);
}

//------------------------------------------------------------------------------
// bool StartMatlabServer()
//------------------------------------------------------------------------------
/**
 * Interface used to tell an Interpreter to start the MATLAB server.
 */
//------------------------------------------------------------------------------
bool Moderator::StartMatlabServer()
{
   if (theUiInterpreter != NULL)
   {
      theUiInterpreter->StartMatlabServer();
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
// std::vector<Gmat::PluginResources*> *Moderator::GetPluginResourceList()
//------------------------------------------------------------------------------
/**
 * Passes the list of plugin resources to the GUI
 *
 * @return The resource list parsed during initialization
 */
//------------------------------------------------------------------------------
std::vector<Gmat::PluginResource*> *Moderator::GetPluginResourceList()
{
   return &userResources;
}

//---------------------------------
//  private
//---------------------------------

// initialization
//------------------------------------------------------------------------------
// void CreatePlanetaryCoeffFile()
//------------------------------------------------------------------------------
void Moderator::CreatePlanetaryCoeffFile()
{
   #if DEBUG_INITIALIZE
   //MessageInterface::ShowMessage("========================================\n");
   MessageInterface::ShowMessage(wxT("Moderator initializing planetary coefficient file...\n"));
   #endif
   
   wxString nutFileName =
      theFileManager->GetFullPathname(wxT("NUTATION_COEFF_FILE"));
   MessageInterface::ShowMessage(wxT("Setting nutation file to %s\n"),
                                 nutFileName.c_str());
   wxString planFileName =
      theFileManager->GetFullPathname(wxT("PLANETARY_COEFF_FILE"));
   MessageInterface::ShowMessage(wxT("Setting planetary coeff. file to %s\n"),
                                 planFileName.c_str());
   
   theItrfFile = new ItrfCoefficientsFile(nutFileName, planFileName);
   theItrfFile->Initialize();
   GmatGlobal::Instance()->SetItrfCoefficientsFile(theItrfFile);
}


//------------------------------------------------------------------------------
// void CreateTimeFile()
//------------------------------------------------------------------------------
void Moderator::CreateTimeFile()
{
   #if DEBUG_INITIALIZE
   //MessageInterface::ShowMessage("========================================\n");
   MessageInterface::ShowMessage(wxT("Moderator initializing time file...\n"));
   #endif
   
   wxString filename = theFileManager->GetFullPathname(wxT("LEAP_SECS_FILE"));
   MessageInterface::ShowMessage(wxT("Setting leap seconds file to %s\n"),
                                 filename.c_str());
   theLeapSecsFile = new LeapSecsFileReader(filename);
   theLeapSecsFile->Initialize();
   
   filename = theFileManager->GetFullPathname(wxT("EOP_FILE"));
   theEopFile = new EopFile(filename);
   theEopFile->Initialize();
   
   TimeConverterUtil::SetLeapSecsFileReader(theLeapSecsFile);
   TimeConverterUtil::SetEopFile(theEopFile);
   GmatGlobal::Instance()->SetEopFile(theEopFile);
}


// prepare next script reading
//------------------------------------------------------------------------------
// void PrepareNextScriptReading(bool clearObjs = true)
//------------------------------------------------------------------------------
/*
 * This method prepares for next script reading by clearing commands and resource,
 * resetting object pointers
 *
 * @param <clearObjs> set to true if clearing commands and resource (true)
 */
//------------------------------------------------------------------------------
void Moderator::PrepareNextScriptReading(bool clearObjs)
{
   #if DEBUG_RUN
   MessageInterface::ShowMessage
      (wxT("Moderator::PrepareNextScriptReading() entered, %sclearing objects\n%s\n"),
       clearObjs ? wxT("") : wxT("Not"),
       "======================================================================");
   #endif
   
   // Set object manage option to configuration
   objectManageOption = 1;
   
   // Clear command sequence before resource (loj: 2008.07.10)
   if (clearObjs)
   {
      //clear both resource and command sequence
      #if DEBUG_RUN
      MessageInterface::ShowMessage(wxT(".....Clearing both resource and command sequence...\n"));
      #endif
      
      ClearCommandSeq(true, true);
      ClearResource();
   }
   
   // Set object map in use
   objectMapInUse = theConfigManager->GetObjectMap();
   
   #ifdef DEBUG_OBJECT_MAP
   MessageInterface::ShowMessage
      (wxT("ObjectMapInUse was set to the configuration map <%p>\n"), objectMapInUse);
   #endif
   
   #if DEBUG_RUN
   MessageInterface::ShowMessage(wxT(".....Creating SolarSystem in use...\n"));
   #endif
   CreateSolarSystemInUse();
   
   // Need default CS's in case they are used in the script
   #if DEBUG_RUN
   MessageInterface::ShowMessage(wxT(".....Creating Default CoordinateSystem...\n"));
   #endif
   CreateDefaultCoordSystems();
   // Create the default Solar System barycenter
   CreateDefaultBarycenter();
   
   #if DEBUG_OBJECT_MAP > 1
   ShowObjectMap(wxT("   Moderator::PrepareNextScriptReading() Here is the configured object map"));
   #endif
   
   // Set solar system in use and object map (loj: 2008.03.31)
   #if DEBUG_RUN
   MessageInterface::ShowMessage(wxT(".....Setting SolarSystem and ObjectMap to Interpreter...\n"));
   #endif
   
   // Reset initial solar system in use and object map 
   SetSolarSystemAndObjectMap(theSolarSystemInUse, objectMapInUse, false,
                              wxT("PrepareNextScriptReading()"));
   currentFunction = NULL;
   
   // Delete unmanaged functions (LOJ: 2009.03.24)
   // This causes crash on Func_AssignmentTest after running
   // Func_MatlabObjectPassingCheck.script
   // so disable until it is fully tested (LOJ: 2009.04.08)
   #ifdef __ENABLE_CLEAR_UNMANAGED_FUNCTIONS__
   #if DEBUG_RUN > 0
   MessageInterface::ShowMessage
      (wxT(".....Moderator::PrepareNextScriptReading() deleting %d unmanaged functions\n"),
       unmanagedFunctions.size());
   #endif
   for (UnsignedInt i=0; i<unmanagedFunctions.size(); i++)
   {
      GmatBase *func = (GmatBase*)(unmanagedFunctions[i]);
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (func, func->GetName(), wxT("Moderator::PrepareNextScriptReading()"),
          wxT("deleting unmanaged function"));
      #endif
      delete func;
      func = NULL;
   }
   unmanagedFunctions.clear();
   #endif
   
   #if DEBUG_RUN
   MessageInterface::ShowMessage
      (wxT("Moderator::PrepareNextScriptReading() exiting\n")
       wxT("======================================================================\n"));
   #endif
} // PrepareNextScriptReading


//------------------------------------------------------------------------------
// void CreateSolarSystemInUse()
//------------------------------------------------------------------------------
/**
 * Creates SolarSystem in use by cloning the default SolarSystem if cloning
 * SolarSystem is enabled, else it just copies value of the default SolarSystem
 * to SolarSystem in use.  It also creates internal CoordinateSystem in use.
 */
//------------------------------------------------------------------------------
void Moderator::CreateSolarSystemInUse()
{
   //-----------------------------------------------------------------
   #ifndef __DISABLE_SOLAR_SYSTEM_CLONING__
   //-----------------------------------------------------------------   
      // delete old SolarSystem in use and create new from default
      if (theSolarSystemInUse != NULL)
      {
         #if DEBUG_INITIALIZE
         MessageInterface::ShowMessage
            (wxT("Moderator deleting old solar system in use <%p>\n"), theSolarSystemInUse);
         #endif
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (theSolarSystemInUse, theSolarSystemInUse->GetName(),
             wxT("Moderator::CreateSolarSystemInUse()"));
         #endif
         delete theSolarSystemInUse;
      }
      
      theSolarSystemInUse = NULL;
      
      #if DEBUG_SOLAR_SYSTEM_IN_USE
      MessageInterface::ShowMessage
         (wxT(".....Setting theSolarSystemInUse to clone of theDefaultSolarSystem <%p>...\n"),
          theDefaultSolarSystem);
      #endif
      
      theSolarSystemInUse = theDefaultSolarSystem->Clone();
      theSolarSystemInUse->SetName(wxT("SolarSystem"));
      
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Add
         (theSolarSystemInUse, theSolarSystemInUse->GetName(),
          wxT("Moderator::CreateSolarSystemInUse()"),
          wxT("theSolarSystemInUse = theDefaultSolarSystem->Clone()"));
      #endif
      
      #if DEBUG_SOLAR_SYSTEM_IN_USE
      MessageInterface::ShowMessage
         (wxT(".....Setting SolarSystemInUse to theInternalSolarSystem...\n"));
      #endif
      theInternalSolarSystem = theSolarSystemInUse;
      
      // set solar system in use
      SetSolarSystemInUse(theSolarSystemInUse);
      
      #if DEBUG_INITIALIZE
      MessageInterface::ShowMessage
         (wxT("Moderator created new solar system in use: %p\n"), theSolarSystemInUse);
      #endif
      
   //-----------------------------------------------------------------
   #else
   //-----------------------------------------------------------------
      if (theSolarSystemInUse == NULL)
      {
         theSolarSystemInUse = theDefaultSolarSystem->Clone();
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Add
            (theSolarSystemInUse, theSolarSystemInUse->GetName(),
             wxT("Moderator::CreateSolarSystemInUse()"),
             wxT("theSolarSystemInUse = theDefaultSolarSystem->Clone()"));
         #endif
         theSolarSystemInUse->SetName(wxT("SolarSystem"));
         SetSolarSystemInUse(theSolarSystemInUse);
      }
      else
      {
         theSolarSystemInUse->Copy(theDefaultSolarSystem);
         theSolarSystemInUse->SetName(wxT("SolarSystem"));
      }
      
      theInternalSolarSystem = theSolarSystemInUse;
      
      #if DEBUG_SOLAR_SYSTEM_IN_USE
      MessageInterface::ShowMessage
         (wxT("Moderator::CreateSolarSystemInUse() theSolarSystemInUse=<%p>, ")
          wxT("theDefaultSolarSystem=<%p>\n"), theSolarSystemInUse,  theDefaultSolarSystem);
      #endif
   //-----------------------------------------------------------------
   #endif
   //-----------------------------------------------------------------
   
   // delete old theInternalCoordSystem and create new one
   if (theInternalCoordSystem)
   {
      #if DEBUG_SOLAR_SYSTEM_IN_USE
      MessageInterface::ShowMessage
         (wxT(".....deleting (%p)theInternalCoordSystem\n"), theInternalCoordSystem);
      #endif
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (theInternalCoordSystem, theInternalCoordSystem->GetName(),
          wxT("Moderator::CreateSolarSystemInUse()"));
      #endif
      delete theInternalCoordSystem;
      theInternalCoordSystem = NULL;
   }
   
   CreateInternalCoordSystem();   
}


//------------------------------------------------------------------------------
// void CreateInternalCoordSystem()
//------------------------------------------------------------------------------
/*
 * Creates the internal coordinate system. This coordinate system is used for
 * publishing data for OpenGL plot.
 * Currently it is EarthMJ2000Eq system.
 */
//------------------------------------------------------------------------------
void Moderator::CreateInternalCoordSystem()
{
   #if DEBUG_INITIALIZE
   //MessageInterface::ShowMessage(wxT("========================================\n"));
   MessageInterface::ShowMessage(wxT("Moderator creating internal coordinate system...\n"));
   #endif
   
   if (theInternalCoordSystem != NULL)
   {
      #if DEBUG_INITIALIZE
      MessageInterface::ShowMessage
         (wxT("..... <%p>theInternalCoordSystem already exists\n"),theInternalCoordSystem);
      #endif
   }
   else
   {
      // Create internal CoordinateSystem with no name, since we don't want
      // it to be configured.
      theInternalCoordSystem =
         CreateCoordinateSystem(wxT("InternalEarthMJ2000Eq"), true, true);
      
      #if DEBUG_INITIALIZE
      MessageInterface::ShowMessage
         (wxT(".....created  <%p>theInternalCoordSystem\n"), theInternalCoordSystem);
      #endif
   }
}


//------------------------------------------------------------------------------
// void CreateDefaultCoordSystems()
//------------------------------------------------------------------------------
void Moderator::CreateDefaultCoordSystems()
{
   #if DEBUG_INITIALIZE
   MessageInterface::ShowMessage(wxT("========================================\n"));
   MessageInterface::ShowMessage
      (wxT("Moderator checking if default coordinate systems should be created...\n"));
   #endif
   
   defaultCoordSystemNames.clear();
   
   try
   {
      SpacePoint *earth = (SpacePoint*)GetConfiguredObject(wxT("Earth"));
      SolarSystem *ss = GetSolarSystemInUse();
      
      // EarthMJ2000Eq
      CoordinateSystem *eqcs = GetCoordinateSystem(wxT("EarthMJ2000Eq"));
      defaultCoordSystemNames.push_back(wxT("EarthMJ2000Eq"));
      if (eqcs == NULL)
      {
         eqcs = CreateCoordinateSystem(wxT("EarthMJ2000Eq"), true);
         
         #if DEBUG_INITIALIZE
         MessageInterface::ShowMessage
            (wxT(".....created <%p>'%s'\n"), eqcs, eqcs->GetName().c_str());
         #endif
      }
      else
      {
         #if DEBUG_INITIALIZE
         MessageInterface::ShowMessage
            (wxT(".....found <%p>'%s'\n"), eqcs, eqcs->GetName().c_str());
         #endif
         eqcs->SetSolarSystem(ss);
         eqcs->Initialize();
      }
      
      // EarthMJ2000Ec
      CoordinateSystem *eccs = GetCoordinateSystem(wxT("EarthMJ2000Ec"));
      defaultCoordSystemNames.push_back(wxT("EarthMJ2000Ec"));
      if (eccs == NULL)
      {
         eccs = CreateCoordinateSystem(wxT("EarthMJ2000Ec"), false);
         #if DEBUG_INITIALIZE
         MessageInterface::ShowMessage
            (wxT(".....created <%p>'%s'\n"), eccs, eccs->GetName().c_str());
         #endif
         AxisSystem *ecAxis = CreateAxisSystem(wxT("MJ2000Ec"), wxT("MJ2000Ec_Earth"));
         eccs->SetStringParameter(wxT("Origin"), wxT("Earth"));
         eccs->SetStringParameter(wxT("J2000Body"), wxT("Earth"));
         eccs->SetRefObject(ecAxis, Gmat::AXIS_SYSTEM, ecAxis->GetName());
         eccs->SetOrigin(earth);
         eccs->SetJ2000Body(earth);
         eccs->SetSolarSystem(ss);
         eccs->Initialize();
         
         // Since CoordinateSystem clones AxisSystem, delete it from here
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (ecAxis, wxT("localAxes"), wxT("Moderator::CreateDefaultCoordSystems()"),
             wxT("deleting localAxes"));
         #endif
         delete ecAxis;
      }
      else
      {
         #if DEBUG_INITIALIZE
         MessageInterface::ShowMessage
            (wxT(".....found <%p>'%s'\n"), eccs, eccs->GetName().c_str());
         #endif
         eccs->SetSolarSystem(ss);
         eccs->Initialize();
      }
      
      // EarthFixed
      CoordinateSystem *bfcs = GetCoordinateSystem(wxT("EarthFixed"));
      defaultCoordSystemNames.push_back(wxT("EarthFixed"));
      if (bfcs == NULL)
      {
         bfcs = CreateCoordinateSystem(wxT("EarthFixed"), false);
         #if DEBUG_INITIALIZE
         MessageInterface::ShowMessage
            (wxT(".....created <%p>'%s'\n"), bfcs, bfcs->GetName().c_str());
         #endif
         BodyFixedAxes *bfecAxis =
            (BodyFixedAxes*)CreateAxisSystem(wxT("BodyFixed"), wxT("BodyFixed_Earth"));
         bfecAxis->SetEopFile(theEopFile);
         bfecAxis->SetCoefficientsFile(theItrfFile);
         bfcs->SetStringParameter(wxT("Origin"), wxT("Earth"));
         bfcs->SetStringParameter(wxT("J2000Body"), wxT("Earth"));
         bfcs->SetRefObject(bfecAxis, Gmat::AXIS_SYSTEM, bfecAxis->GetName());
         bfcs->SetOrigin(earth);
         bfcs->SetJ2000Body(earth);
         bfcs->SetSolarSystem(ss);
         bfcs->Initialize();
         
         // Since CoordinateSystem clones AxisSystem, delete it from here
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (bfecAxis, wxT("localAxes"), wxT("Moderator::CreateDefaultCoordSystems()"),
             wxT("deleting localAxes"));
         #endif
         delete bfecAxis;
      }
      else
      {
         #if DEBUG_INITIALIZE
         MessageInterface::ShowMessage
            (wxT(".....found <%p>'%s'\n"), bfcs, bfcs->GetName().c_str());
         #endif
         bfcs->SetSolarSystem(ss);
         bfcs->Initialize();
      }
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_,
          wxT("Moderator::CreateDefaultCoordSystems() Error occurred during default ")
          wxT("coordinate system creation. ") +  e.GetFullMessage());
   }
}

//------------------------------------------------------------------------------
// void CreateDefaultBarycenter()
//------------------------------------------------------------------------------
void Moderator::CreateDefaultBarycenter()
{
   #if DEBUG_INITIALIZE
   MessageInterface::ShowMessage(wxT("========================================\n"));
   MessageInterface::ShowMessage
      (wxT("Moderator checking if default barycenter should be created...\n"));
   #endif

   try
   {
      SolarSystem *ss = GetSolarSystemInUse();

      // Solar System Barycenter
      Barycenter *bary = (Barycenter*) GetCalculatedPoint(GmatSolarSystemDefaults::SOLAR_SYSTEM_BARYCENTER_NAME);
      if (bary == NULL)
      {
         bary = (Barycenter*) CreateCalculatedPoint(wxT("Barycenter"), GmatSolarSystemDefaults::SOLAR_SYSTEM_BARYCENTER_NAME, false);

         #if DEBUG_INITIALIZE
         MessageInterface::ShowMessage
            (wxT(".....created <%p>'%s'\n"), bary, bary->GetName().c_str());
         #endif
      }
      else
      {
         #if DEBUG_INITIALIZE
         MessageInterface::ShowMessage
            (wxT(".....found <%p>'%s'\n"), bary, bary->GetName().c_str());
         #endif
      }
      bary->SetSolarSystem(ss);
      bary->SetIsBuiltIn(true);
      bary->Initialize();  // ????
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_,
          wxT("Moderator::CreateDefaultBarycenter() Error occurred during default ")
          wxT("barycenter creation. ") +  e.GetFullMessage());
   }
}

//------------------------------------------------------------------------------
// void CreateDefaultMission()
//------------------------------------------------------------------------------
void Moderator::CreateDefaultMission()
{
   #if DEBUG_INITIALIZE
   MessageInterface::ShowMessage(wxT("========================================\n"));
   MessageInterface::ShowMessage(wxT("Moderator creating default mission...\n"));
   #endif
   
   try
   {
      //----------------------------------------------------
      // Create default resource
      //----------------------------------------------------
      
      // Create solar system in use
      CreateSolarSystemInUse();
      
      // Create default coordinate systems
      CreateDefaultCoordSystems();
      // Create the default Solar System barycenter
      CreateDefaultBarycenter();
      
      // Spacecraft
      Spacecraft *sc = (Spacecraft*)CreateSpacecraft(wxT("Spacecraft"), wxT("DefaultSC"));
      sc->SetInternalCoordSystem(theInternalCoordSystem);
      sc->SetRefObject(GetCoordinateSystem(wxT("EarthMJ2000Eq")),
                       Gmat::COORDINATE_SYSTEM, wxT("EarthMJ2000Eq"));
      
      #if DEBUG_DEFAULT_MISSION
      MessageInterface::ShowMessage(wxT("-->default Spacecraft created\n"));
      #endif
      
      // PropSetup
      CreateDefaultPropSetup(wxT("DefaultProp"));
      
      #if DEBUG_DEFAULT_MISSION
      MessageInterface::ShowMessage(wxT("-->default PropSetup created\n"));
      #endif
      
      //--------------------------------------------------------------
      // test Burn Parameter
      //--------------------------------------------------------------
      
      #ifdef __CREATE_HARDWARE__
      // Hardware 
      CreateHardware(wxT("FuelTank"), wxT("DefaultFuelTank"));
      CreateHardware(wxT("Thruster"), wxT("DefaultThruster"));
      
      #if DEBUG_DEFAULT_MISSION > 0
      MessageInterface::ShowMessage(wxT("-->default hardware created\n"));
      #endif
      #endif
      
      #ifdef __CREATE_VNB_COORD__
      // Create VNB CoordinateSystem
      CoordinateSystem *vnb = CreateCoordinateSystem(wxT("VNB"), false);
      ObjectReferencedAxes *orAxis =
         (ObjectReferencedAxes*)CreateAxisSystem(wxT("ObjectReferenced"),
                                                 wxT("ObjectReferenced"));
      orAxis->SetEopFile(theEopFile);
      orAxis->SetCoefficientsFile(theItrfFile);
      orAxis->SetStringParameter(wxT("XAxis"), wxT("V"));
      orAxis->SetStringParameter(wxT("YAxis"), wxT("N"));
      orAxis->SetStringParameter(wxT("Primary"), wxT("Earth"));
      orAxis->SetStringParameter(wxT("Secondary"), wxT("DefaultSC"));
      vnb->SetStringParameter(wxT("Origin"), wxT("Earth"));
      vnb->SetRefObject(orAxis, Gmat::AXIS_SYSTEM, orAxis->GetName());
      
      // Since CoordinateSystem clones AxisSystem, delete it from here
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (orAxis, wxT("localAxes"), wxT("Moderator::CreateDefaultMission()"),
          wxT("deleting localAxes"));
      #endif
      delete orAxis;
      
      #if DEBUG_DEFAULT_MISSION > 0
      MessageInterface::ShowMessage(wxT("-->default vnb coordinate system created\n"));
      #endif
      #endif
      
      // ImpulsiveBurn
      GetDefaultBurn(wxT("ImpulsiveBurn"));
      #if DEBUG_DEFAULT_MISSION > 0
      MessageInterface::ShowMessage(wxT("-->default impulsive burn created\n"));
      #endif
      
      // ImpulsiveBurn parameters
      CreateParameter(wxT("Element1"), wxT("DefaultIB.Element1"));
      CreateParameter(wxT("Element2"), wxT("DefaultIB.Element2"));
      CreateParameter(wxT("Element3"), wxT("DefaultIB.Element3"));
      CreateParameter(wxT("V"), wxT("DefaultIB.V"));
      CreateParameter(wxT("N"), wxT("DefaultIB.N"));
      CreateParameter(wxT("B"), wxT("DefaultIB.B"));
      #if DEBUG_DEFAULT_MISSION
      MessageInterface::ShowMessage(wxT("-->default impulsive burn parameters created\n"));
      #endif
      //--------------------------------------------------------------
      
      // Time parameters
      CreateParameter(wxT("ElapsedSecs"), wxT("DefaultSC.ElapsedSecs"));
      CreateParameter(wxT("ElapsedDays"), wxT("DefaultSC.ElapsedDays"));      
      CreateParameter(wxT("CurrA1MJD"), wxT("DefaultSC.CurrA1MJD"));
      CreateParameter(wxT("A1ModJulian"), wxT("DefaultSC.A1ModJulian"));
      CreateParameter(wxT("A1Gregorian"), wxT("DefaultSC.A1Gregorian"));
      CreateParameter(wxT("TAIModJulian"), wxT("DefaultSC.TAIModJulian"));
      CreateParameter(wxT("TAIGregorian"), wxT("DefaultSC.TAIGregorian"));
      CreateParameter(wxT("TTModJulian"), wxT("DefaultSC.TTModJulian"));
      CreateParameter(wxT("TTGregorian"), wxT("DefaultSC.TTGregorian"));
      CreateParameter(wxT("TDBModJulian"), wxT("DefaultSC.TDBModJulian"));
      CreateParameter(wxT("TDBGregorian"), wxT("DefaultSC.TDBGregorian"));
      CreateParameter(wxT("TCBModJulian"), wxT("DefaultSC.TCBModJulian"));
      CreateParameter(wxT("TCBGregorian"), wxT("DefaultSC.TCBGregorian"));
      CreateParameter(wxT("UTCModJulian"), wxT("DefaultSC.UTCModJulian"));
      CreateParameter(wxT("UTCGregorian"), wxT("DefaultSC.UTCGregorian"));      
      #if DEBUG_DEFAULT_MISSION > 1
      MessageInterface::ShowMessage(wxT("-->default time parameters  created\n"));
      #endif
      
      // Cartesian parameters
      CreateParameter(wxT("X"), wxT("DefaultSC.EarthMJ2000Eq.X"));
      CreateParameter(wxT("Y"), wxT("DefaultSC.EarthMJ2000Eq.Y"));
      CreateParameter(wxT("Z"), wxT("DefaultSC.EarthMJ2000Eq.Z"));
      CreateParameter(wxT("VX"), wxT("DefaultSC.EarthMJ2000Eq.VX"));
      CreateParameter(wxT("VY"), wxT("DefaultSC.EarthMJ2000Eq.VY"));
      CreateParameter(wxT("VZ"), wxT("DefaultSC.EarthMJ2000Eq.VZ"));
      #if DEBUG_DEFAULT_MISSION > 1
      MessageInterface::ShowMessage(wxT("-->default cartesian parameters created\n"));
      #endif
      
      // Keplerian parameters
      CreateParameter(wxT("SMA"), wxT("DefaultSC.Earth.SMA"));
      CreateParameter(wxT("ECC"), wxT("DefaultSC.Earth.ECC"));
      CreateParameter(wxT("INC"), wxT("DefaultSC.Earth.INC"));
      CreateParameter(wxT("RAAN"), wxT("DefaultSC.Earth.RAAN"));
      CreateParameter(wxT("AOP"), wxT("DefaultSC.EarthMJ2000Eq.AOP"));
      CreateParameter(wxT("TA"), wxT("DefaultSC.Earth.TA"));
      CreateParameter(wxT("MA"), wxT("DefaultSC.Earth.MA"));
      CreateParameter(wxT("EA"), wxT("DefaultSC.Earth.EA"));
      CreateParameter(wxT("HA"), wxT("DefaultSC.Earth.HA"));
      CreateParameter(wxT("MM"), wxT("DefaultSC.Earth.MM"));
      #if DEBUG_DEFAULT_MISSION > 1
      MessageInterface::ShowMessage(wxT("-->default keplerian parameters created\n"));
      #endif
      
      // Orbital parameters
      CreateParameter(wxT("VelApoapsis"), wxT("DefaultSC.Earth.VelApoapsis"));
      CreateParameter(wxT("VelPeriapsis"), wxT("DefaultSC.Earth.VelPeriapsis"));
      CreateParameter(wxT("Apoapsis"), wxT("DefaultSC.Earth.Apoapsis"));
      CreateParameter(wxT("Periapsis"), wxT("DefaultSC.Earth.Periapsis"));
      CreateParameter(wxT("OrbitPeriod"), wxT("DefaultSC.Earth.OrbitPeriod"));
      CreateParameter(wxT("RadApo"), wxT("DefaultSC.Earth.RadApo"));
      CreateParameter(wxT("RadPer"), wxT("DefaultSC.Earth.RadPer"));
      CreateParameter(wxT("C3Energy"), wxT("DefaultSC.Earth.C3Energy"));
      CreateParameter(wxT("Energy"), wxT("DefaultSC.Earth.Energy"));
      #if DEBUG_DEFAULT_MISSION > 1
      MessageInterface::ShowMessage(wxT("-->default orbital parameters created\n"));
      #endif
      
      // Spherical parameters
      CreateParameter(wxT("RMAG"), wxT("DefaultSC.Earth.RMAG"));
      CreateParameter(wxT("RA"), wxT("DefaultSC.Earth.RA"));
      CreateParameter(wxT("DEC"), wxT("DefaultSC.EarthMJ2000Eq.DEC"));
      CreateParameter(wxT("VMAG"), wxT("DefaultSC.EarthMJ2000Eq.VMAG"));
      CreateParameter(wxT("RAV"), wxT("DefaultSC.EarthMJ2000Eq.RAV"));
      CreateParameter(wxT("DECV"), wxT("DefaultSC.EarthMJ2000Eq.DECV"));
      CreateParameter(wxT("AZI"), wxT("DefaultSC.EarthMJ2000Eq.AZI"));
      CreateParameter(wxT("FPA"), wxT("DefaultSC.EarthMJ2000Eq.FPA"));
      #if DEBUG_DEFAULT_MISSION > 1
      MessageInterface::ShowMessage(wxT("-->default spherical parameters created\n"));
      #endif
      
      // Angular parameters
      CreateParameter(wxT("SemilatusRectum"), wxT("DefaultSC.Earth.SemilatusRectum"));
      CreateParameter(wxT("HMAG"), wxT("DefaultSC.HMAG"));
      CreateParameter(wxT("HX"), wxT("DefaultSC.EarthMJ2000Eq.HX"));
      CreateParameter(wxT("HY"), wxT("DefaultSC.EarthMJ2000Eq.HY"));
      CreateParameter(wxT("HZ"), wxT("DefaultSC.EarthMJ2000Eq.HZ"));
      CreateParameter(wxT("DLA"), wxT("DefaultSC.EarthMJ2000Eq.DLA"));
      CreateParameter(wxT("RLA"), wxT("DefaultSC.EarthMJ2000Eq.RLA"));
      #if DEBUG_DEFAULT_MISSION > 1
      MessageInterface::ShowMessage(wxT("-->default angular parameters created\n"));
      #endif

      #ifdef __ENABLE_ATMOS_DENSITY__
      // Environmental parameters
      CreateParameter(wxT("AtmosDensity"), wxT("DefaultSC.Earth.AtmosDensity"));
      #if DEBUG_DEFAULT_MISSION > 1
      MessageInterface::ShowMessage(wxT("-->default env parameters created\n"));
      #endif
      #endif
      
      // Planet parameters
      CreateParameter(wxT("Altitude"), wxT("DefaultSC.Earth.Altitude"));
      CreateParameter(wxT("MHA"), wxT("DefaultSC.Earth.MHA"));
      CreateParameter(wxT("Longitude"), wxT("DefaultSC.Earth.Longitude"));
      CreateParameter(wxT("Latitude"), wxT("DefaultSC.Earth.Latitude"));
      CreateParameter(wxT("LST"), wxT("DefaultSC.Earth.LST"));
      CreateParameter(wxT("BetaAngle"), wxT("DefaultSC.Earth.BetaAngle"));
      #if DEBUG_DEFAULT_MISSION > 1
      MessageInterface::ShowMessage(wxT("-->default planet parameters created\n"));
      #endif
      
      // B-Plane parameters
      CreateParameter(wxT("BdotT"), wxT("DefaultSC.Earth.BdotT"));
      CreateParameter(wxT("BdotR"), wxT("DefaultSC.Earth.BdotR"));
      CreateParameter(wxT("BVectorMag"), wxT("DefaultSC.Earth.BVectorMag"));
      CreateParameter(wxT("BVectorAngle"), wxT("DefaultSC.Earth.BVectorAngle"));
      #if DEBUG_DEFAULT_MISSION > 1
      MessageInterface::ShowMessage(wxT("-->default b-plane parameters created\n"));
      #endif
      
      // Attitude parameters
      CreateParameter(wxT("DCM11"), wxT("DefaultSC.DCM11"));
      CreateParameter(wxT("DCM12"), wxT("DefaultSC.DCM12"));
      CreateParameter(wxT("DCM13"), wxT("DefaultSC.DCM13"));
      CreateParameter(wxT("DCM21"), wxT("DefaultSC.DCM21"));
      CreateParameter(wxT("DCM22"), wxT("DefaultSC.DCM22"));
      CreateParameter(wxT("DCM23"), wxT("DefaultSC.DCM23"));
      CreateParameter(wxT("DCM31"), wxT("DefaultSC.DCM31"));
      CreateParameter(wxT("DCM32"), wxT("DefaultSC.DCM32"));
      CreateParameter(wxT("DCM33"), wxT("DefaultSC.DCM33"));
      CreateParameter(wxT("EulerAngle1"), wxT("DefaultSC.EulerAngle1"));
      CreateParameter(wxT("EulerAngle2"), wxT("DefaultSC.EulerAngle2"));
      CreateParameter(wxT("EulerAngle3"), wxT("DefaultSC.EulerAngle3"));
      CreateParameter(wxT("MRP1"), wxT("DefaultSC.MRP1"));  // Dunn Added
      CreateParameter(wxT("MRP2"), wxT("DefaultSC.MRP2"));  // Dunn Added
      CreateParameter(wxT("MRP3"), wxT("DefaultSC.MRP3"));  // Dunn Added
      CreateParameter(wxT("Q1"), wxT("DefaultSC.Q1"));
      CreateParameter(wxT("Q2"), wxT("DefaultSC.Q2"));
      CreateParameter(wxT("Q3"), wxT("DefaultSC.Q3"));
      CreateParameter(wxT("Q4"), wxT("DefaultSC.Q4"));
      CreateParameter(wxT("AngularVelocityX"), wxT("DefaultSC.AngularVelocityX"));
      CreateParameter(wxT("AngularVelocityY"), wxT("DefaultSC.AngularVelocityY"));
      CreateParameter(wxT("AngularVelocityZ"), wxT("DefaultSC.AngularVelocityZ"));
      CreateParameter(wxT("EulerAngleRate1"), wxT("DefaultSC.EulerAngleRate1"));
      CreateParameter(wxT("EulerAngleRate2"), wxT("DefaultSC.EulerAngleRate2"));
      CreateParameter(wxT("EulerAngleRate3"), wxT("DefaultSC.EulerAngleRate3"));
      #if DEBUG_DEFAULT_MISSION > 1
      MessageInterface::ShowMessage(wxT("-->default attitude parameters created\n"));
      #endif
      
      // Ballistic/Mass parameters
      CreateParameter(wxT("DryMass"), wxT("DefaultSC.DryMass"));
      CreateParameter(wxT("Cd"), wxT("DefaultSC.Cd"));
      CreateParameter(wxT("Cr"), wxT("DefaultSC.Cr"));
      CreateParameter(wxT("DragArea"), wxT("DefaultSC.DragArea"));
      CreateParameter(wxT("SRPArea"), wxT("DefaultSC.SRPArea"));
      CreateParameter(wxT("TotalMass"), wxT("DefaultSC.TotalMass"));
      #if DEBUG_DEFAULT_MISSION > 1
      MessageInterface::ShowMessage(wxT("-->default ballistic/mass parameters created\n"));
      #endif
      
      // STM and A-Matrix parameters
      CreateParameter(wxT("OrbitSTM"), wxT("DefaultSC.OrbitSTM"));
      CreateParameter(wxT("OrbitSTMA"), wxT("DefaultSC.OrbitSTMA"));
      CreateParameter(wxT("OrbitSTMB"), wxT("DefaultSC.OrbitSTMB"));
      CreateParameter(wxT("OrbitSTMC"), wxT("DefaultSC.OrbitSTMC"));
      CreateParameter(wxT("OrbitSTMD"), wxT("DefaultSC.OrbitSTMD"));
      #if DEBUG_DEFAULT_MISSION > 1
      MessageInterface::ShowMessage(wxT("-->default STM parameters created\n"));
      #endif
      
      #ifdef DEBUG_CREATE_VAR
      // User variable
      Parameter *var = CreateParameter(wxT("Variable"), wxT("DefaultSC_EarthMJ2000Eq_Xx2"));
      var->SetStringParameter(wxT("Expression"), wxT("DefaultSC.EarthMJ2000Eq.X * 2.0"));
      var->SetRefObjectName(Gmat::PARAMETER, wxT("DefaultSC.EarthMJ2000Eq.X"));
      #endif
      
      #if DEBUG_DEFAULT_MISSION
      MessageInterface::ShowMessage(wxT("-->default parameters created\n"));
      #endif
      
      // Set parameter description and object name
      StringArray params = GetListOfObjects(Gmat::PARAMETER);
      Parameter *param;
      
      for (unsigned int i=0; i<params.size(); i++)
      {
         param = GetParameter(params[i]);

         // need spacecraft if system parameter
         if (param->GetKey() == GmatParam::SYSTEM_PARAM)
         {
            if (param->GetOwnerType() == Gmat::SPACECRAFT)
            {
               //MessageInterface::ShowMessage("name = '%s'\n", param->GetName().c_str());
               //param->SetStringParameter("Expression", param->GetName());
               param->SetRefObjectName(Gmat::SPACECRAFT, wxT("DefaultSC"));
               
               if (param->NeedCoordSystem())
               {
                  param->SetRefObjectName(Gmat::COORDINATE_SYSTEM, wxT("EarthMJ2000Eq"));
                  if (param->IsOriginDependent())
                     param->SetStringParameter(wxT("DepObject"), wxT("Earth"));
                  else if (param->IsCoordSysDependent())
                     param->SetStringParameter(wxT("DepObject"), wxT("EarthMJ2000Eq"));
               }
            }
            else if (param->GetOwnerType() == Gmat::IMPULSIVE_BURN)
            {
               //MessageInterface::ShowMessage("name = '%s'\n", param->GetName().c_str());
               param->SetRefObjectName(Gmat::IMPULSIVE_BURN, wxT("DefaultIB"));
            }
         }
      }
      
      #if DEBUG_DEFAULT_MISSION
      MessageInterface::ShowMessage(wxT("-->ref. object to parameters are set\n"));
      #endif
      
      // StopCondition
      StopCondition *stopOnElapsedSecs =
         CreateStopCondition(wxT("StopCondition"), wxT("StopOnDefaultSC.ElapsedSecs"));
      stopOnElapsedSecs->SetStringParameter(wxT("EpochVar"), wxT("DefaultSC.A1ModJulian"));
      stopOnElapsedSecs->SetStringParameter(wxT("StopVar"), wxT("DefaultSC.ElapsedSecs"));
      // Dunn changed ElapsedSecs for default mission to 12000.0 so the spacecraft
      // icon will stop on the near side of the earth where we can see it.  This
      // was required in two locations, so look for it again below.
      stopOnElapsedSecs->SetStringParameter(wxT("Goal"), wxT("12000.0"));
      //stopOnElapsedSecs->SetStringParameter("Goal", "8640.0");
      
      #if DEBUG_DEFAULT_MISSION
      MessageInterface::ShowMessage(wxT("-->default StopCondition created\n"));
      #endif
      
      // Subscribers
      // OrbitView
      GetDefaultSubscriber(wxT("OrbitView"));
      GetDefaultSubscriber(wxT("GroundTrackPlot"));
      
      #if DEBUG_DEFAULT_MISSION
      MessageInterface::ShowMessage(wxT("-->default Subscribers created\n"));
      #endif
      
      //----------------------------------------------------
      // Create default mission sequence
      //----------------------------------------------------
      bool retval;
      
      // Append BeginMissionSequence command (New since 2010.07.09)
      //GmatCommand *cmd = AppendCommand("BeginMissionSequence", "", retval);
      AppendCommand(wxT("BeginMissionSequence"), wxT(""), retval);
      
      // Propagate Command
      GmatCommand *propCommand = CreateCommand(wxT("Propagate"), wxT(""), retval);
      propCommand->SetObject(wxT("DefaultProp"), Gmat::PROP_SETUP);
      propCommand->SetObject(wxT("DefaultSC"), Gmat::SPACECRAFT);
      propCommand->SetRefObject(stopOnElapsedSecs, Gmat::STOP_CONDITION, wxT(""), 0);
      propCommand->SetSolarSystem(theSolarSystemInUse);
      
      #if DEBUG_MULTI_STOP
      //----------------------------------------------------
      //just for testing multiple stopping condition
      //----- StopCondition 2
      StopCondition *stopOnX =
         CreateStopCondition(wxT("StopCondition"), wxT("StopOnDefaultSC.EarthMJ2000Eq.X"));
      stopOnX->SetStringParameter(wxT("EpochVar"), wxT("DefaultSC.A1ModJulian"));
      stopOnX->SetStringParameter(wxT("StopVar"), wxT("DefaultSC.EarthMJ2000Eq.X"));
      stopOnX->SetStringParameter(wxT("Goal"), wxT("5000.0"));
      propCommand->SetRefObject(stopOnX, Gmat::STOP_CONDITION, wxT(""), 1);
      #endif
      
      #if DEBUG_MULTI_STOP > 1
      StopCondition *stopOnPeriapsis =
         CreateStopCondition(wxT("StopCondition"), wxT("StopOnDefaultSC.Earth.Periapsis"));
      stopOnPeriapsis->SetStringParameter(wxT("EpochVar"), wxT("DefaultSC.A1ModJulian"));
      stopOnPeriapsis->SetStringParameter(wxT("StopVar"), wxT("DefaultSC.Earth.Periapsis"));
      propCommand->SetRefObject(stopOnPeriapsis, Gmat::STOP_CONDITION, wxT(""), 2);
      //----------------------------------------------------
      #endif

      #if DEBUG_DEFAULT_MISSION
      MessageInterface::ShowMessage(wxT("-->default Propagate command created\n"));
      #endif
      
      // Append Propagate command
      AppendCommand(propCommand);
      
      #if DEBUG_DEFAULT_MISSION
      MessageInterface::ShowMessage
         (wxT("-->Setting SolarSystem <%p> and ObjectMap <%p> to theScriptInterpreter\n"),
          theSolarSystemInUse, theConfigManager->GetObjectMap());
      #endif
      
      // Reset initial solar system in use and object map
      objectMapInUse = theConfigManager->GetObjectMap();
      SetSolarSystemAndObjectMap(theSolarSystemInUse, objectMapInUse, false,
                                 wxT("CreateDefaultMission()"));
      
      loadSandboxAndPause = false;
      isRunReady = true;
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_,
          wxT("*** Error occurred during default mission creation.\n    The default ")
          wxT("mission will not run.\n    Message: ") + e.GetFullMessage());
   }
   
   #if DEBUG_INITIALIZE
   MessageInterface::ShowMessage(wxT("Moderator successfully created default mission\n"));
   #endif
} // CreateDefaultMission()


// Parameter reference object setting
//------------------------------------------------------------------------------
// void CheckParameterType(Parameter *param, const wxString &type,
//                         const wxString &ownerName)
//------------------------------------------------------------------------------
void Moderator::CheckParameterType(Parameter *param, const wxString &type,
                                   const wxString &ownerName)
{
   GmatBase *obj = FindObject(ownerName);
   if (obj)
   {
      #if DEBUG_CREATE_PARAMETER
      MessageInterface::ShowMessage
         (wxT("   Found owner object, name='%s', addr=<%p>\n"), obj->GetName().c_str(), obj);
      #endif
      
      if (param->GetOwnerType() != obj->GetType())
      {
         wxString paramOwnerType =
            GmatBase::GetObjectTypeString(param->GetOwnerType());
         
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (param, param->GetName(), wxT("Moderator::CheckParameterType()"));
         #endif
         delete param;
         param = NULL;
         
         if (paramOwnerType == wxT(""))
            throw GmatBaseException
               (wxT("Cannot find the object type which has \"") + type +
                wxT("\" as a Parameter type"));
         else
            throw GmatBaseException
               (wxT("Parameter type: ") + type + wxT(" should be property of ") +
                paramOwnerType);
      }
   }
}


//------------------------------------------------------------------------------
// void SetParameterRefObject(Parameter *param, const wxString &type, ...)
//------------------------------------------------------------------------------
/**
 * Sets parameter reference object
 *
 * @param <param> parameter pointer
 * @param <type> parameter type
 * @param <name> parameter name
 * @param <ownerName> parameter owner name
 * @param <depName> dependent object name
 */
//------------------------------------------------------------------------------
void Moderator::SetParameterRefObject(Parameter *param, const wxString &type,
                                      const wxString &name,
                                      const wxString &ownerName,
                                      const wxString &depName, Integer manage)
{
   #if DEBUG_PARAMETER_REF_OBJ
   MessageInterface::ShowMessage
      (wxT("Moderator::SetParameterRefObject() param=<%p>, type=<%s>, name=<%s>, ")
       wxT("owner=<%s>, dep=<%s>, manage=%d\n"),  param, type.c_str(), name.c_str(),
       ownerName.c_str(), depName.c_str(), manage);
   #endif
   
   // Why do we need to set expression for non-Variable? (loj: 2008.08.09)
   // Expression is set in the ScriptInterpreter or ParameterCreateDialog for
   // new Parameters
   // If type is Variable, don't set expression
   //if (type != "Variable")
   //   param->SetStringParameter("Expression", name);
   
   // Set parameter owner and dependent object
   if (ownerName != wxT(""))
   {
      param->SetRefObjectName(param->GetOwnerType(), ownerName);
      param->AddRefObject((Parameter*)FindObject(ownerName));
   }
   
   wxString newDep = depName;
   
   // Set dependent object name if depName is not blank,
   // if depName is blank get default dep name
   if (depName != wxT(""))
      param->SetStringParameter(wxT("DepObject"), depName);
   else
      newDep = param->GetStringParameter(wxT("DepObject"));
   
   // Set SolarSystem
   param->SetSolarSystem(theSolarSystemInUse);
   param->SetInternalCoordSystem(theInternalCoordSystem);
   
   #if DEBUG_PARAMETER_REF_OBJ
   MessageInterface::ShowMessage
      (wxT("   name='%s', newDep='%s'\n"), name.c_str(), newDep.c_str());
   #endif
   
   if (newDep != wxT(""))
      param->AddRefObject(FindObject(newDep));
   
   // I'm not sure if we always use EarthMJ2000Eq (loj: 2008.06.03)
   if (param->NeedCoordSystem())
      param->AddRefObject(FindObject(wxT("EarthMJ2000Eq")));
   
   // create parameter dependent coordinate system
   if (type == wxT("Longitude") || type == wxT("Latitude") || type == wxT("Altitude") ||
       type == wxT("MHA") || type == wxT("LST"))
   {
      // need body-fixed CS
      StringTokenizer st(name, wxT("."));
      StringArray tokens = st.GetAllTokens();
      
      if (tokens.size() == 2 || (tokens.size() == 3 && tokens[1] == wxT("Earth")))
      {
         #if DEBUG_PARAMETER_REF_OBJ
         MessageInterface::ShowMessage(wxT("   Creating 'EarthFixed' CoordinateSystem\n"));
         #endif
         
         // default EarthFixed
         CoordinateSystem *cs = CreateCoordinateSystem(wxT("EarthFixed"), false, false, manage);
         param->SetRefObjectName(Gmat::COORDINATE_SYSTEM, wxT("EarthFixed"));
         // Set CoordinateSystem to param (2008.06.26)
         // It will work without setting CS pointer since EarthFixed is a default
         // CoordinateSyste, but for consistency just set here
         param->SetRefObject(cs, Gmat::COORDINATE_SYSTEM, wxT("EarthFixed"));
      }
      else if (tokens.size() == 3)
      {
         wxString origin = tokens[1];
         wxString axisName = origin + wxT("Fixed");
         
         #if DEBUG_PARAMETER_REF_OBJ
         MessageInterface::ShowMessage
            (wxT("   Creating '%s' CoordinateSystem\n"), axisName.c_str());
         #endif
         
         CoordinateSystem *cs = CreateCoordinateSystem(axisName, false, false, manage);
         
         // create BodyFixedAxis with origin
         AxisSystem *axis = CreateAxisSystem(wxT("BodyFixed"), wxT("BodyFixed_Earth"), manage);
         cs->SetStringParameter(wxT("Origin"), origin);
         cs->SetRefObject(FindObject(origin), Gmat::SPACE_POINT, origin);
         cs->SetRefObject(axis, Gmat::AXIS_SYSTEM, axis->GetName());
         cs->SetStringParameter(wxT("J2000Body"), wxT("Earth"));
         cs->SetRefObject(FindObject(wxT("Earth")), Gmat::SPACE_POINT, wxT("Earth"));
         cs->SetSolarSystem(theSolarSystemInUse);
         cs->Initialize();
         
         // Since CoordinateSystem clones AxisSystem, delete it from here
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (axis, wxT("localAxes"), wxT("Moderator::SetParameterRefObject()"),
             wxT("deleting localAxes"));
         #endif
         delete axis;
         
         param->SetRefObjectName(Gmat::COORDINATE_SYSTEM, axisName);
         // Set CoordinateSystem to param (2008.06.26)
         // This will fix problem with NULL output CS pointer if Paremeter is
         // used in the GmatFunction
         param->SetRefObject(cs, Gmat::COORDINATE_SYSTEM, axisName);
      }
      else
      {
         MessageInterface::ShowMessage(wxT("===> Invalid parameter name: %s\n"),
                                       name.c_str());
      }
   }
}


// object map
//------------------------------------------------------------------------------
// GmatBase* FindObject(const wxString &name)
//------------------------------------------------------------------------------
/*
 * Finds object from the object map in use by name base on objectManageOption.
 * if objectManageOption is 
 *    0, if object is not managed
 *    1, if configuration object map is used for finding objects and adding new
 *          objects
 *    2, if function object map is used for finding objects and adding new
 *          objects including automatic objects
 */
//------------------------------------------------------------------------------
GmatBase* Moderator::FindObject(const wxString &name)
{
   #if DEBUG_FIND_OBJECT
   MessageInterface::ShowMessage
      (wxT("Moderator::FindObject() entered, name='%s', objectMapInUse=<%p>\n"),
       name.c_str(), objectMapInUse);
   #endif
   
   if (name == wxT(""))
   {
      #if DEBUG_FIND_OBJECT
      MessageInterface::ShowMessage
         (wxT("Moderator::FindObject() name is blank, so just returning NULL\n"));
      #endif
      return NULL;
   }
   
   if (objectMapInUse == NULL)
   {
      #if DEBUG_FIND_OBJECT
      MessageInterface::ShowMessage
         (wxT("Moderator::FindObject() objectMapInUse is NULL, so just returning NULL\n"));
      #endif
      return NULL;
   }
   
   // Ignore array indexing of Array
   wxString newName = name;
   wxString::size_type index = name.find_first_of(wxT("(["));
   if (index != name.npos)
   {
      newName = name.substr(0, index);
      
      #if DEBUG_FIND_OBJECT
      MessageInterface::ShowMessage(wxT("   newName=%s\n"), newName.c_str());
      #endif
   }
   
   #if DEBUG_FIND_OBJECT > 1
   ShowObjectMap(wxT("Moderator::FindObject() Here is the object map in use"));
   #endif
   
   GmatBase *obj = NULL;
   
   if (objectMapInUse->find(newName) != objectMapInUse->end())
      obj = (*objectMapInUse)[newName];
   
   // check objectManageOption
   Integer manage = 1;
   if (objectManageOption != 1)
      manage = 2;
   
   // If object not found, try SolarSystem
   if (obj == NULL)
   {
      SolarSystem *ss = GetSolarSystemInUse(manage);
      
      if (ss)
      {
         obj = (GmatBase*)(ss->GetBody(newName));
         #if DEBUG_FIND_OBJECT
         if (obj)
         {
            MessageInterface::ShowMessage
               (wxT("   Found '%s' from the SolarSystem <%p> in the map\n"),
                name.c_str(), ss);
         }
         #endif
      }
   }
   
   #if DEBUG_FIND_OBJECT
   MessageInterface::ShowMessage
      (wxT("Moderator::FindObject() returning <%p><%s>'%s'\n"), obj, 
       obj ? obj->GetTypeName().c_str() : wxT("NULL"),
       obj ? obj->GetName().c_str() : wxT("NULL"));
   #endif
   
   return obj;
}


//------------------------------------------------------------------------------
// bool AddObject(GmatBase *obj)
//------------------------------------------------------------------------------
/*
 * Adds object to the objectMapInUse
 *
 * @param  obj  object to add to object map
 *
 * @return true if successfully added, false otherwise
 */
//------------------------------------------------------------------------------
bool Moderator::AddObject(GmatBase *obj)
{
   #if DEBUG_ADD_OBJECT
   MessageInterface::ShowMessage
      (wxT("Moderator::AddObject() entered, obj=<%p><%s><%s>, objectMapInUse=<%p>\n"),
       obj, obj ? obj->GetTypeName().c_str() : wxT("NULL"),
       obj ? obj->GetName().c_str() : wxT("NULL"), objectMapInUse);
   #endif
   
   if (obj == NULL || (obj && obj->GetName() == wxT("")))
   {
      #if DEBUG_ADD_OBJECT
      MessageInterface::ShowMessage
         (wxT("Moderator::AddObject() returning false, has NULL obj or name is blank\n"));
      #endif
      
      return false;
   }
   
   if (objectMapInUse == NULL)
      throw GmatBaseException
         (wxT("Moderator::AddObject() cannot add object named \"") + obj->GetName() +
          wxT("\" to unset object map in use"));
   
   #ifdef DEBUG_OBJECT_MAP
   MessageInterface::ShowMessage
      (wxT("Moderator::Moderator::AddObject() Adding <%p><%s>'%s' to objectMapInUse <%p>\n"),
       obj, obj->GetTypeName().c_str(), obj->GetName().c_str(), objectMapInUse);
   #endif
   
   // if name not found in the object map, then insert (loj: 2008.12.16)
   if (objectMapInUse->find(obj->GetName()) == objectMapInUse->end())
      objectMapInUse->insert(std::make_pair(obj->GetName(), obj));
   else
   {
      #if DEBUG_ADD_OBJECT
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** Moderator::AddObject() '%s' is already in the object map, so ignored\n"),
          obj->GetName().c_str());
      #endif
   }
   
   #if DEBUG_ADD_OBJECT
   MessageInterface::ShowMessage(wxT("Moderator::AddObject() returning true\n"));
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// void SetSolarSystemAndObjectMap(SolarSystem *ss, ObjectMap *objMap,
//         bool forFunction, const wxString &callFrom)
//------------------------------------------------------------------------------
/*
 * Sets the solar system in use and configured object map to interpreters.
 *
 * @param <ss> Pointer to the solar system
 * @param <objMap> Pointer to the object map
 * @param <forFunction> True if setting object map for function (false)
 */
//------------------------------------------------------------------------------
void Moderator::SetSolarSystemAndObjectMap(SolarSystem *ss, ObjectMap *objMap,
                                           bool forFunction,
                                           const wxString &callFrom)
{
   #if DEBUG_OBJECT_MAP
   MessageInterface::ShowMessage
      (wxT("=====> Moderator::%s setting solarSystemInUse=<%p>, ")
       wxT("objectMapInUse=<%p> %s\n"), callFrom.c_str(), ss, objMap,
       forFunction ? wxT("for function") : wxT(""));
   #endif
   
   // Set solar system in use and object map 
   theScriptInterpreter->SetSolarSystemInUse(ss);
   theScriptInterpreter->SetObjectMap(objMap);
   if (!forFunction)
      theScriptInterpreter->SetFunction(NULL);
   if (theUiInterpreter != NULL)
   {
      theUiInterpreter->SetSolarSystemInUse(ss);
      theUiInterpreter->SetObjectMap(objMap);
      if (!forFunction)
         theUiInterpreter->SetFunction(NULL);
   }
}


//------------------------------------------------------------------------------
// bool IsSequenceStarter(const wxString &commandType)
//------------------------------------------------------------------------------
/*
 * Determines if a command identifies a mission control sequence start command
 *
 * @param commandType Type name for the command
 *
 * @return true if the command is a MCS start command
 */
//------------------------------------------------------------------------------
bool Moderator::IsSequenceStarter(const wxString &commandType)
{
   bool retval = false;

   if (sequenceStarters.empty())
      GetSequenceStarters();
   if (find(sequenceStarters.begin(), sequenceStarters.end(), commandType) != 
         sequenceStarters.end())
      retval = true;

   return retval;
}


//------------------------------------------------------------------------------
// const StringArray& GetSequenceStarters()
//------------------------------------------------------------------------------
/*
 * Retrieves a StringArray listing the mission control sequence start commands
 *
 * @return The array naming the MCS start commands
 */
//------------------------------------------------------------------------------
const StringArray& Moderator::GetSequenceStarters()
{
   sequenceStarters.clear();
   sequenceStarters = theFactoryManager->GetListOfItems(Gmat::COMMAND, wxT("SequenceStarters"));

   return sequenceStarters;
}

//------------------------------------------------------------------------------
// const wxString& GetStarterStringList()
//------------------------------------------------------------------------------
/*
 * Retrieves a string listing the mission control sequence start commands
 *
 * This method is used to set teh list in exception related to the start commands
 *
 * @return The array naming the MCS start commands
 */
//------------------------------------------------------------------------------
const wxString& Moderator::GetStarterStringList()
{
   if (starterList == wxT(""))
   {
      if (sequenceStarters.empty())
         GetSequenceStarters();
      for (UnsignedInt i = 0; i < sequenceStarters.size(); ++i)
      {
         starterList += sequenceStarters[i];
         if (i+1 < sequenceStarters.size())
            starterList += wxT(", ");
      }
   }

   return starterList;
}


// default objects
//------------------------------------------------------------------------------
// Spacecraft* GetDefaultSpacecraft()
//------------------------------------------------------------------------------
/*
 * Returns first spacecraft not in the Formation
 */
//------------------------------------------------------------------------------
Spacecraft* Moderator::GetDefaultSpacecraft()
{
   StringArray soConfigList = GetListOfObjects(Gmat::SPACECRAFT);
   
   if (soConfigList.size() > 0)
   {
      // return 1st Spacecraft
      SpaceObject *so = GetSpacecraft(soConfigList[0]);
      return (Spacecraft*)so;
   }
   else
   {
      // create Spacecraft
      return (Spacecraft*)CreateSpacecraft(wxT("Spacecraft"), wxT("DefaultSC"));
   }
}


//------------------------------------------------------------------------------
// PropSetup* GetDefaultPropSetup()
//------------------------------------------------------------------------------
PropSetup* Moderator::GetDefaultPropSetup()
{
   StringArray configList = GetListOfObjects(Gmat::PROP_SETUP);
   
   if (configList.size() > 0)
   {
      // return 1st PropSetup from the list
      return GetPropSetup(configList[0]);
   }
   else
   {
      // create PropSetup
      return CreateDefaultPropSetup(wxT("DefaultProp"));
   }
}


//------------------------------------------------------------------------------
// Burn* GetDefaultBurn(const wxString &type)
//------------------------------------------------------------------------------
Burn* Moderator::GetDefaultBurn(const wxString &type)
{
   StringArray configList = GetListOfObjects(Gmat::BURN);

   if (configList.size() > 0)
   {
      for (UnsignedInt i=0; i<configList.size(); i++)
         if (GetBurn(configList[i])->IsOfType(type))
            return GetBurn(configList[i]);
   }
   
   Burn *burn = NULL;
   
   if (type == wxT("ImpulsiveBurn"))
      burn = CreateBurn(wxT("ImpulsiveBurn"), wxT("DefaultIB"));
   else if (type == wxT("FiniteBurn"))
      burn = CreateBurn(wxT("FiniteBurn"), wxT("DefaultFB"));
   
   return burn;
}


//------------------------------------------------------------------------------
// Hardware* GetDefaultHardware(const wxString &type)
//------------------------------------------------------------------------------
Hardware* Moderator::GetDefaultHardware(const wxString &type)
{
   StringArray configList = GetListOfObjects(Gmat::HARDWARE);

   if (configList.size() > 0)
   {
      for (UnsignedInt i=0; i<configList.size(); i++)
         if (GetHardware(configList[i])->IsOfType(type))
            return GetHardware(configList[i]);
   }
   
   Hardware *hw = NULL;
   
   if (type == wxT("FuelTank"))
      hw = CreateHardware(wxT("FuelTank"), wxT("DefaultFuelTank"));
   else if (type == wxT("Thruster"))
      hw = CreateHardware(wxT("Thruster"), wxT("DefaultThruster"));
   
   return hw;
}


//------------------------------------------------------------------------------
// Subscriber* GetDefaultSubscriber(const wxString &type, bool addObjects = true,
//                                  bool createIfNoneFound = true)
//------------------------------------------------------------------------------
/**
 * Returns default subcriber of given type, if createIfNoneFound is true, it will
 * create default subscriber.
 */
//------------------------------------------------------------------------------
Subscriber* Moderator::GetDefaultSubscriber(const wxString &type, bool addObjects,
                                            bool createIfNoneFound)
{
   StringArray configList = GetListOfObjects(Gmat::SUBSCRIBER);
   int subSize = configList.size();
   Subscriber *sub = NULL;
   
   for (int i=0; i<subSize; i++)
   {
      sub = (Subscriber*)GetConfiguredObject(configList[i]);
      if (sub->GetTypeName() == type)
         return sub;
   }
   
   // If not creating default subscriber, just return NULL
   if (!createIfNoneFound)
      return NULL;
   
   if (type == wxT("OrbitView"))
   {
      // create default OrbitView
      sub = CreateSubscriber(wxT("OrbitView"), wxT("DefaultOrbitView"));
      sub->SetStringParameter(wxT("Add"), wxT("DefaultSC"));
      sub->SetStringParameter(wxT("Add"), wxT("Earth"));
      sub->SetStringParameter(wxT("CoordinateSystem"), wxT("EarthMJ2000Eq"));
      sub->SetStringParameter(wxT("ViewPointVector"), wxT("[30000 0 0]"));
      sub->Activate(true);
   }
   else if (type == wxT("GroundTrackPlot"))
   {
      // create default GroundTrackPlot
      sub = CreateSubscriber(wxT("GroundTrackPlot"), wxT("DefaultGroundTrackPlot"));
      sub->SetStringParameter(wxT("Add"), wxT("DefaultSC"));
      sub->SetStringParameter(wxT("Add"), wxT("Earth"));
      sub->Activate(true);
   }
   else if (type == wxT("XYPlot"))
   {
      // create default XYPlot
      sub = CreateSubscriber(wxT("XYPlot"), wxT("DefaultXYPlot"));
      sub->SetStringParameter(wxT("XVariable"), wxT("DefaultSC.A1ModJulian"));
      sub->SetStringParameter(wxT("YVariables"), wxT("DefaultSC.EarthMJ2000Eq.X"), 0);      
      sub->SetStringParameter(wxT("YVariables"), wxT("DefaultSC.EarthMJ2000Eq.Y"), 1);
      sub->SetStringParameter(wxT("YVariables"), wxT("DefaultSC.EarthMJ2000Eq.Z"), 2);
      sub->Activate(true);
   }
   else if (type == wxT("ReportFile"))
   {
      // create default ReportFile
      sub = CreateSubscriber(wxT("ReportFile"), wxT("DefaultReportFile"));
      wxString scName = GetDefaultSpacecraft()->GetName();
      sub->SetStringParameter(sub->GetParameterID(wxT("Filename")),
                              wxT("DefaultReportFile.txt"));
      
      if (addObjects)
      {
         sub->SetStringParameter(wxT("Add"), scName + wxT(".A1ModJulian"));
         sub->SetStringParameter(wxT("Add"), scName + wxT(".EarthMJ2000Eq.X"));
         sub->SetStringParameter(wxT("Add"), scName + wxT(".EarthMJ2000Eq.Y"));
         sub->SetStringParameter(wxT("Add"), scName + wxT(".EarthMJ2000Eq.Z"));
         sub->SetStringParameter(wxT("Add"), scName + wxT(".EarthMJ2000Eq.VX"));
         sub->SetStringParameter(wxT("Add"), scName + wxT(".EarthMJ2000Eq.VY"));
         sub->SetStringParameter(wxT("Add"), scName + wxT(".EarthMJ2000Eq.VZ"));
      }
      sub->Activate(true);
      
      // To validate and create element wrappers
      theScriptInterpreter->ValidateSubscriber(sub);
   }
   else
   {
      MessageInterface::ShowMessage
         (wxT("*** ERROR *** GetDefaultSubscriber() Undefined subscriber type: %s\n"),
          type.c_str());
   }

   return sub;
}

//------------------------------------------------------------------------------
// Solver* GetDefaultSolver()
//------------------------------------------------------------------------------
Solver* Moderator::GetDefaultSolver()
{
   StringArray configList = GetListOfObjects(Gmat::SOLVER);
   
   if (configList.size() > 0)
   {
      // return 1st Burn from the list
      return GetSolver(configList[0]);
   }
   else
   {
      // create Solver
      return CreateSolver(wxT("DifferentialCorrector"), wxT("DefaultDC"));
   }
}

//------------------------------------------------------------------------------
// StopCondition* CreateDefaultStopCondition()
//------------------------------------------------------------------------------
StopCondition* Moderator::CreateDefaultStopCondition()
{
   StopCondition *stopCond = NULL;
   Parameter *param;
   
   Spacecraft *sc = GetDefaultSpacecraft();
   wxString scName = sc->GetName();
   
   wxString epochVar = scName + wxT(".A1ModJulian");
   wxString stopVar = scName + wxT(".ElapsedSecs");
   
   #ifdef DEBUG_DEFAULT_MISSION
   MessageInterface::ShowMessage
      (wxT("Moderator::CreateDefaultStopCondition() scName=%s, epochVar=%s, ")
       wxT("stopVar=%s\n"), scName.c_str(), epochVar.c_str(), stopVar.c_str());
   #endif
   
   if (GetParameter(epochVar) == NULL)
   {
      param = CreateParameter(wxT("A1ModJulian"), epochVar);
      param->SetRefObjectName(Gmat::SPACECRAFT, scName);
   }
   
   if (GetParameter(stopVar) == NULL)
   {
      param = CreateParameter(wxT("ElapsedSecs"), stopVar);
      param->SetRefObjectName(Gmat::SPACECRAFT, scName);
   }
   
   wxString stopCondName = wxT("StopOn") + stopVar;
   
   stopCond = CreateStopCondition(wxT("StopCondition"), wxT("StopOn") + stopVar);
   
   stopCond->SetStringParameter(wxT("EpochVar"), epochVar);
   stopCond->SetStringParameter(wxT("StopVar"), stopVar);
   // Dunn changed ElapsedSecs for default mission to 12000.0 so the spacecraft
   // icon will stop on the near side of the earth where we can see it.
   stopCond->SetStringParameter(wxT("Goal"), wxT("12000.0"));
   //stopCond->SetStringParameter("Goal", "8640.0");
   return stopCond;
}


//------------------------------------------------------------------------------
// Parameter* GetDefaultX()
//------------------------------------------------------------------------------
Parameter* Moderator::GetDefaultX()
{
   Spacecraft *sc = GetDefaultSpacecraft();
   Parameter* param = GetParameter(sc->GetName() + wxT(".A1ModJulian"));

   if (param == NULL)
   {
      param = CreateParameter(wxT("A1ModJulian"), sc->GetName() + wxT(".A1ModJulian"));
      param->SetRefObjectName(Gmat::SPACECRAFT, sc->GetName());
   }
   
   return param;
}


//------------------------------------------------------------------------------
// Parameter* GetDefaultY()
//------------------------------------------------------------------------------
Parameter* Moderator::GetDefaultY()
{
   Spacecraft *sc = GetDefaultSpacecraft();
   Parameter* param = GetParameter(sc->GetName() + wxT(".EarthMJ2000Eq.X"));
   
   if (param == NULL)
   {
      param = CreateParameter(wxT("X"), sc->GetName() + wxT(".EarthMJ2000Eq.X"));
      param->SetRefObjectName(Gmat::SPACECRAFT, sc->GetName());
   }
   
   return param;
}


// sandbox
//------------------------------------------------------------------------------
// void AddSolarSystemToSandbox(Integer index)
//------------------------------------------------------------------------------
void Moderator::AddSolarSystemToSandbox(Integer index)
{
   #if DEBUG_RUN
   MessageInterface::ShowMessage
      (wxT("Moderator::AddSolarSystemToSandbox() entered\n"));
   MessageInterface::ShowMessage
      (wxT("   Adding theSolarSystemInUse<%p> to Sandbox\n"), theSolarSystemInUse);
   #endif
   
   //If we are ready to configure SolarSystem by name
   // Script will have something like, "Create SolarSystem anotherSS;"
   #ifdef __USE_CONFIGURED_SOLAR_SYSTEM__
      SolarSystem *solarSys = theConfigManager->GetSolarSystemInUse(name);
      sandboxes[index]->AddSolarSystem(solarSys);
   #else
      sandboxes[index]->AddSolarSystem(theSolarSystemInUse);
   #endif
}


//------------------------------------------------------------------------------
// void Moderator::AddTriggerManagersToSandbox(Integer index)
//------------------------------------------------------------------------------
/**
 * Passes TriggerManager array to a Sandbox so the Sandbox can clone managers
 *
 * @param index The index of the sandbox getting the trigger managers
 */
//------------------------------------------------------------------------------
void Moderator::AddTriggerManagersToSandbox(Integer index)
{
   sandboxes[index]->AddTriggerManagers(&triggerManagers);
}


//------------------------------------------------------------------------------
// void AddInternalCoordSystemToSandbox(Integer index)
//------------------------------------------------------------------------------
void Moderator::AddInternalCoordSystemToSandbox(Integer index)
{
   #if DEBUG_RUN
   MessageInterface::ShowMessage
      (wxT("Moderator::AddInternalCoordSystemToSandbox() entered.\n"));
   MessageInterface::ShowMessage
      (wxT("   Adding theInternalCoordSystem<%p> to Sandbox\n"), theInternalCoordSystem);
   #endif
   
   sandboxes[index]->SetInternalCoordSystem(theInternalCoordSystem);
   
}


//------------------------------------------------------------------------------
// void AddPublisherToSandbox(Integer index)
//------------------------------------------------------------------------------
void Moderator::AddPublisherToSandbox(Integer index)
{
   #if DEBUG_RUN
   MessageInterface::ShowMessage
      (wxT("Moderator::AddPublisherToSandbox() entered.\n"));
   MessageInterface::ShowMessage
      (wxT("   Adding thePublisher<%p> to Sandbox\n"), thePublisher);
   #endif
   
   thePublisher->UnsubscribeAll();
   sandboxes[index]->SetPublisher(thePublisher);
   
}


//------------------------------------------------------------------------------
// void HandleCcsdsEphemerisFile(ObjectMap *objMap, bool deleteOld = false)
//------------------------------------------------------------------------------
/**
 * Creates plug-in CcsdsEphemerisFile object if EphemerisFile type is CCSDS
 * and replaces the old one.
 */
//------------------------------------------------------------------------------
void Moderator::HandleCcsdsEphemerisFile(ObjectMap *objMap, bool deleteOld)
{
   #ifdef DEBUG_CCSDS_EPHEMERIS
   MessageInterface::ShowMessage
      (wxT("Moderator::HandleCcsdsEphemerisFile() entered, objMap=<%p>\n"), objMap);
   #endif
   
   GmatBase *obj;
   
   for (ObjectMap::iterator i = objMap->begin(); i != objMap->end(); ++i)
   {
      obj = i->second;
      
      //==============================================================
      // Special handling for CcsdsEphemerisFile plug-in
      //==============================================================
      // This is needed since we create EphemerisFile object first
      // from the script "Create EphemerisFile" and then create
      // CcsdsEphemerisFile if file format contains CCSDS.
      // It will create CcsdsEphemerisFile object via
      // plug-in factory and replace the object pointer.
      //==============================================================
      // Create CcsdsEphemerisFile if file format is CCSDS
      if (obj->IsOfType(Gmat::EPHEMERIS_FILE))
      {
         wxString name = obj->GetName();
         wxString format = obj->GetStringParameter(wxT("FileFormat"));
         
         #ifdef DEBUG_CCSDS_EPHEMERIS
         MessageInterface::ShowMessage
            (wxT("   Format of the object<%p><%s>'%s' is '%s'\n"),
             obj, obj->GetTypeName().c_str(), name.c_str(), format.c_str());
         #endif
         
         if (format.find(wxT("CCSDS")) != format.npos)
         {
            // Check type name to avoid recreating a CcsdsEphemerisFile object for re-runs
            if (obj->GetTypeName() != wxT("CcsdsEphemerisFile"))
            {
               #ifdef DEBUG_CCSDS_EPHEMERIS
               MessageInterface::ShowMessage(wxT("   About to create new CcsdsEphemerisFile\n"));
               #endif
               
               // Create unnamed CcsdsEphemerisFile
               GmatBase *newObj = CreateEphemerisFile(wxT("CcsdsEphemerisFile"), wxT(""));
               if (newObj == NULL)
               {
                  throw GmatBaseException
                     (wxT("Moderator::AddSubscriberToSandbox() Cannot continue due to missing ")
                      wxT("CcsdsEphemerisFile plugin dll\n"));
               }
               
               newObj->SetName(name);
               ResetObjectPointer(objMap, newObj, name);
               ResetObjectPointer(objectMapInUse, newObj, name);
               newObj->Copy(obj);
               newObj->TakeAction(wxT("ChangeTypeName"), wxT("CcsdsEphemerisFile"));
               
               #ifdef DEBUG_CCSDS_EPHEMERIS
               MessageInterface::ShowMessage
                  (wxT("   New object <%p><%s>'%s' created\n"), newObj, newObj->GetTypeName().c_str(),
                   name.c_str());
               #endif
               
               GmatBase *oldObj = obj;
               obj = newObj;
               
               // Delete old object on option
               if (deleteOld)
               {
                  #ifdef DEBUG_CCSDS_EPHEMERIS
                  MessageInterface::ShowMessage
                     (wxT("   Deleting old object <%p><%s>'%s'\n"), oldObj, oldObj->GetTypeName().c_str(),
                      name.c_str());
                  #endif
                  #ifdef DEBUG_MEMORY
                  MemoryTracker::Instance()->Remove
                     (oldObj, oldObj->GetName(), wxT("Moderator::HandleCcsdsEphemerisFile()"));
                  #endif
                  delete oldObj;
               }
            }
         }
      }
   }
   
   #ifdef DEBUG_CCSDS_EPHEMERIS
   ShowObjectMap(wxT("In Moderator::HandleCcsdsEphemerisFile()"), objMap);
   MessageInterface::ShowMessage
      (wxT("Moderator::HandleCcsdsEphemerisFile() leaving\n"));
   #endif
}

//------------------------------------------------------------------------------
// void AddSuscriberToSandbox(Integer index)
//------------------------------------------------------------------------------
void Moderator::AddSubscriberToSandbox(Integer index)
{
   #ifdef __USE_DATAFILE__
   // Handle CcsdsEphemerisFile which uses DataFile plugin
   HandleCcsdsEphemerisFile(objectMapInUse, false);
   #endif
   
   Subscriber *obj;
   StringArray names = theConfigManager->GetListOfItems(Gmat::SUBSCRIBER);
   
   #if DEBUG_RUN
   MessageInterface::ShowMessage
      (wxT("Moderator::AddSubscriberToSandbox() count = %d\n"), names.size());
   #endif
   
   for (Integer i=0; i<(Integer)names.size(); i++)
   {
      obj = theConfigManager->GetSubscriber(names[i]);
      sandboxes[index]->AddSubscriber(obj);
      
      #if DEBUG_RUN > 1
      MessageInterface::ShowMessage
         (wxT("   Adding <%p><%s>'%s'\n"), obj, obj->GetTypeName().c_str(), 
          obj->GetName().c_str());
      #endif
   }
}


//------------------------------------------------------------------------------
// void AddOtherObjectsToSandbox(Integer index)
//------------------------------------------------------------------------------
void Moderator::AddOtherObjectsToSandbox(Integer index)
{
   GmatBase *obj;
   StringArray names = theConfigManager->GetListOfAllItems();
   
   #if DEBUG_RUN
   MessageInterface::ShowMessage
      (wxT("Moderator::AddOtherObjectsToSandbox() count = %d\n"), names.size());
   #endif
   
   for (Integer i=0; i<(Integer)names.size(); i++)
   {
      obj = theConfigManager->GetItem(names[i]);

      // Skip subscribers since those are handled separately
      if (obj->IsOfType(Gmat::SUBSCRIBER))
         continue;
      
      #ifdef DEBUG_RUN
      MessageInterface::ShowMessage
         (wxT("   Adding <%p><%s>'%s'\n"), obj, obj->GetTypeName().c_str(),
          obj->GetName().c_str());
      #endif
      sandboxes[index]->AddObject(obj);
   }
}


//------------------------------------------------------------------------------
// void AddCommandToSandbox(Integer index)
//------------------------------------------------------------------------------
void Moderator::AddCommandToSandbox(Integer index)
{
   #if DEBUG_RUN
   MessageInterface::ShowMessage
      (wxT("Moderator::AddCommandToSandbox() entered\n"));
   #endif
   
   GmatCommand *cmd = commands[index]->GetNext();
   
   if (cmd != NULL)
   {
      #if DEBUG_RUN > 1
      MessageInterface::ShowMessage
         (wxT("   Adding <%p><%s>\n"), cmd, cmd->GetTypeName().c_str());
      #endif
      
      sandboxes[index]->AddCommand(cmd);
   }
}


//------------------------------------------------------------------------------
// void InitializeSandbox(Integer index)
//------------------------------------------------------------------------------
void Moderator::InitializeSandbox(Integer index)
{
   sandboxes[index]->Initialize();
}

//------------------------------------------------------------------------------
// void ExecuteSandbox(Integer index)
//------------------------------------------------------------------------------
void Moderator::ExecuteSandbox(Integer index)
{
   sandboxes[index]->Execute();
}

//---------------------------------
// private
//---------------------------------

//------------------------------------------------------------------------------
// void ShowCommand(const wxString &title1, GmatCommand *cmd1,
//                  const wxString &title2, GmatCommand *cmd2)
//------------------------------------------------------------------------------
void Moderator::ShowCommand(const wxString &title1, GmatCommand *cmd1,
                            const wxString &title2, GmatCommand *cmd2)
{
   if (title2 == wxT(""))
   {
      if (cmd1 == NULL)
         MessageInterface::ShowMessage(wxT("%s<%p><NULL>\n"), title1.c_str(), cmd1);
      else
         MessageInterface::ShowMessage
            (wxT("%s<%p><%s>\n"), title1.c_str(), cmd1, cmd1->GetTypeName().c_str());
   }
   else
   {
      if (cmd2 == NULL)
         MessageInterface::ShowMessage
            (wxT("%s<%p><NULL>%s<%p><NULL>\n"), title1.c_str(), cmd1, title2.c_str(), cmd2);
      else
         MessageInterface::ShowMessage
            (wxT("%s<%p><%s>%s<%p><%s>\n"), title1.c_str(), cmd1, cmd1->GetTypeName().c_str(),
             title2.c_str(), cmd2, cmd2->GetTypeName().c_str());
   }
}


//------------------------------------------------------------------------------
// void ShowObjectMap(const wxString &title, ObjectMap *objMap = NULL)
//------------------------------------------------------------------------------
void Moderator::ShowObjectMap(const wxString &title, ObjectMap *objMap)
{
   MessageInterface::ShowMessage(title + wxT("\n"));
   if (objMap != NULL)
   {
      MessageInterface::ShowMessage
         (wxT(" passedObjectMap = <%p>, it has %d objects\n"), objMap, objMap->size());
      for (ObjectMap::iterator i = objMap->begin(); i != objMap->end(); ++i)
      {
         MessageInterface::ShowMessage
            (wxT("   %30s  <%p><%s>\n"), i->first.c_str(), i->second,
             i->second == NULL ? wxT("NULL") : (i->second)->GetTypeName().c_str());
      }
   }
   
   if (objectMapInUse == NULL)
   {
      MessageInterface::ShowMessage(wxT("\nThe objectMapInUse is NULL\n"));
      return;
   }
   
   MessageInterface::ShowMessage
      (wxT(" objectMapInUse = <%p>, it has %d objects\n"), objectMapInUse, objectMapInUse->size());
   for (ObjectMap::iterator i = objectMapInUse->begin();
        i != objectMapInUse->end(); ++i)
   {
      MessageInterface::ShowMessage
         (wxT("   %30s  <%p><%s>\n"), i->first.c_str(), i->second,
          i->second == NULL ? wxT("NULL") : (i->second)->GetTypeName().c_str());
   }
}


//------------------------------------------------------------------------------
// Moderator()
//------------------------------------------------------------------------------
/*
 * Constructor
 */
//------------------------------------------------------------------------------
Moderator::Moderator()
{
   isRunReady = false;
   showFinalState = false;
   loadSandboxAndPause = false;
   theDefaultSolarSystem = NULL;
   theSolarSystemInUse = NULL;
   theInternalCoordSystem = NULL;
   theInternalSolarSystem = NULL;
   runState = Gmat::IDLE;
   objectManageOption = 1;
   
   theMatlabInterface = NULL;
   
   // The motivation of adding this data member was due to Parameter creation
   // in function mode. When Parameter is created, the Moderator automatically
   // sets it's refeence object. For example, Sat.X, it sets Sat object pointer
   // found from the current object map. Since we don't always want to use
   // configuration to find object, this objectMapInUse was added. (loj: 2008.05.23)
   objectMapInUse = NULL;
   currentFunction = NULL;
   
   sandboxes.reserve(Gmat::MAX_SANDBOX);
   commands.reserve(Gmat::MAX_SANDBOX);
}


//------------------------------------------------------------------------------
// ~Moderator()
//------------------------------------------------------------------------------
Moderator::~Moderator()
{
}
