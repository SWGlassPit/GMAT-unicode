//$Id: Moderator.hpp 9850 2011-09-09 18:48:32Z lindajun $
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
// Author: Linda Jun
// Created: 2003/08/25
//
/**
 * Declares operations of the GMAT executive. It is a singleton class -
 * only one instance of this class can be created.
 */
//------------------------------------------------------------------------------
#ifndef Moderator_hpp
#define Moderator_hpp

#include "gmatdefs.hpp"
// executive
#include "Sandbox.hpp"
#include "ScriptInterpreter.hpp"
#include "FactoryManager.hpp"
#include "ConfigManager.hpp"
#include "Publisher.hpp"
#include "FileManager.hpp"
// core
#include "AtmosphereModel.hpp"
#include "Attitude.hpp"
#include "AxisSystem.hpp"
#include "Burn.hpp"
#include "GmatCommand.hpp"
#include "CoordinateSystem.hpp"
#include "Function.hpp"
#include "Hardware.hpp"
#include "PhysicalModel.hpp"
#include "ODEModel.hpp"
#include "Propagator.hpp"
#include "Spacecraft.hpp"
#include "Formation.hpp"
#include "Parameter.hpp"
#include "StopCondition.hpp"
#include "Solver.hpp"
#include "SolarSystem.hpp"
#include "CelestialBody.hpp"
#include "PropSetup.hpp"
#include "Subscriber.hpp"
#include "Interpolator.hpp"
#include "CalculatedPoint.hpp"
#include "MathNode.hpp"
// files
#include "EopFile.hpp"
#include "ItrfCoefficientsFile.hpp"
#include "LeapSecsFileReader.hpp"
// plug-in code
#include "DynamicLibrary.hpp"
#include "TriggerManager.hpp"

class DataFile;
class ObType;
class Interface;
class EventLocator;

namespace Gmat
{
   const Integer MAX_SANDBOX = 4;
};

class GMAT_API Moderator
{
public:

   static Moderator* Instance();
   bool Initialize(const wxString &startupFile = wxT(""), bool isFromGui = false);
   void Finalize();
   void SetRunReady(bool flag = true);
   void SetShowFinalState(bool flag = true);
   
   //----- Matlab engine
   Interface* GetMatlabInterface();
   bool OpenMatlabEngine();
   bool CloseMatlabEngine();
   
   //----- Plug-in code
   void LoadPlugins();
   void LoadAPlugin(wxString pluginName);
   DynamicLibrary *LoadLibrary(const wxString &libraryName);
   bool IsLibraryLoaded(const wxString &libName);
   void (*GetDynamicFunction(const wxString &funName, 
                             const wxString &libraryName))();
   
   //----- ObjectType
   wxString GetObjectTypeString(Gmat::ObjectType type);  
   
   //----- interpreter
   static ScriptInterpreter* GetUiInterpreter();
   static ScriptInterpreter* GetScriptInterpreter();
   static void SetUiInterpreter(ScriptInterpreter *uiInterp);
   static void SetScriptInterpreter(ScriptInterpreter *scriptInterp);
   
   void SetInterpreterMapAndSS(Interpreter *interp);
   
   //----- object map
   void SetObjectMap(ObjectMap *objMap);
   void SetObjectManageOption(Integer option);
   Integer GetObjectManageOption();
   void ResetObjectPointer(ObjectMap *objMap, GmatBase *newobj,
                           const wxString &name);
   
   //----- factory
   const StringArray& GetListOfFactoryItems(Gmat::ObjectType type);
   const StringArray& GetListOfAllFactoryItems();
   const StringArray& GetListOfAllFactoryItemsExcept(const ObjectTypeArray &types);
   const StringArray& GetListOfViewableItems(Gmat::ObjectType type);
   const StringArray& GetListOfViewableItems(const wxString &typeName);
   const StringArray& GetListOfUnviewableItems(Gmat::ObjectType type);
   const StringArray& GetListOfUnviewableItems(const wxString &typeName);
   bool               DoesObjectTypeMatchSubtype(
                            const Gmat::ObjectType coreType,
                            const wxString &theType,
                            const wxString &theSubtype);
   
   //----- configuration
   ObjectMap* GetConfiguredObjectMap();
   const StringArray& GetListOfObjects(Gmat::ObjectType type,
                                       bool excludeDefaultObjects = false);
   const StringArray& GetListOfObjects(const wxString &typeName,
                                       bool excludeDefaultObjects = false);
   GmatBase* GetConfiguredObject(const wxString &name);
   bool ReconfigureItem(GmatBase *newobj, const wxString &name);
   wxString GetNewName(const wxString &name, Integer startCount);
   wxString AddClone(const wxString &name);
   bool RenameObject(Gmat::ObjectType type, const wxString &oldName,
                     const wxString &newName);
   bool RemoveObject(Gmat::ObjectType type, const wxString &name,
                     bool delOnlyIfNotUsed);
   bool HasConfigurationChanged(Integer sandboxNum = 1);
   void ConfigurationChanged(GmatBase *obj, bool tf);
   void ResetConfigurationChanged(bool resetResource = true,
                                  bool resetCommands = true,
                                  Integer sandboxNum = 1);
   
   // SolarSystem
   SolarSystem* GetDefaultSolarSystem();
   SolarSystem* CreateSolarSystem(const wxString &name);
   SolarSystem* GetSolarSystemInUse(Integer manage = 1);
   void SetSolarSystemInUse(SolarSystem *ss);
   void SetInternalSolarSystem(SolarSystem *ss);
   bool SetSolarSystemInUse(const wxString &name);
   
   // CalculatedPoint
   CalculatedPoint* CreateCalculatedPoint(const wxString &type,
                                          const wxString &name,
                                          bool addDefaultBodies = true);
   CalculatedPoint* GetCalculatedPoint(const wxString &name);
   
   // CelestialBody
   CelestialBody* CreateCelestialBody(const wxString &type,
                                      const wxString &name);
   CelestialBody* GetCelestialBody(const wxString &name);

   // Spacecraft
   SpaceObject* CreateSpacecraft(const wxString &type,
                                 const wxString &name);
   SpaceObject* GetSpacecraft(const wxString &name);
   wxString GetSpacecraftNotInFormation();
   
   // SpacePoints
   SpacePoint* CreateSpacePoint(const wxString &type,
                                 const wxString &name);
   SpacePoint* GetSpacePoint(const wxString &name);
   
   // Hardware
   Hardware* CreateHardware(const wxString &type,
                            const wxString &name);
   Hardware* GetHardware(const wxString &name);
   
   // Propagator
   Propagator* CreatePropagator(const wxString &type,
                                const wxString &name);
   Propagator* GetPropagator(const wxString &name);
   
   // PhysicalModel
   PhysicalModel* CreateDefaultPhysicalModel(const wxString &name);
   PhysicalModel* CreatePhysicalModel(const wxString &type,
                                      const wxString &name);
   PhysicalModel* GetPhysicalModel(const wxString &name);
   
   // AtmosphereModel
   AtmosphereModel* CreateAtmosphereModel(const wxString &type,
                                          const wxString &name,
                                          const wxString &body = wxT("Earth"));
   AtmosphereModel* GetAtmosphereModel(const wxString &name);
   
   // Burn
   Burn* CreateBurn(const wxString &type,
                    const wxString &name, bool createDefault = false);
   Burn* GetBurn(const wxString &name);
   
   // Parameter
   bool IsParameter(const wxString &type);
   Parameter* CreateAutoParameter(const wxString &type,
                                  const wxString &name,
                                  bool &alreadyManaged,
                                  const wxString &ownerName = wxT(""),
                                  const wxString &depName = wxT(""),
                                  Integer manage = 1);
   Parameter* CreateParameter(const wxString &type,
                              const wxString &name,
                              const wxString &ownerName = wxT(""),
                              const wxString &depName = wxT(""),
                              Integer manage = 1);
   Parameter* GetParameter(const wxString &name);
   void SetParameterRefObject(Parameter *param, const wxString &type,
                              const wxString &name,
                              const wxString &ownerName,
                              const wxString &depName, Integer manage);
   
   // ODEModel
   ODEModel* CreateDefaultODEModel(const wxString &name);
   ODEModel* CreateODEModel(const wxString &type, const wxString &name);
   ODEModel* GetODEModel(const wxString &name);
   bool AddToODEModel(const wxString &odeModelName,
                        const wxString &forceName);
   
   // Solver
   Solver* CreateSolver(const wxString &type,
                        const wxString &name);
   Solver* GetSolver(const wxString &name);
   
   // PropSetup
   PropSetup* CreateDefaultPropSetup(const wxString &name);
   PropSetup* CreatePropSetup(const wxString &name);
   PropSetup* GetPropSetup(const wxString &name);
   
   // MeasurementModel
   MeasurementModel* CreateMeasurementModel(const wxString &name);
   MeasurementModel* GetMeasurementModel(const wxString &name);
   
   // TrackingSystem
   TrackingSystem* CreateTrackingSystem(const wxString &type,
                                        const wxString &name);
   TrackingSystem* GetTrackingSystem(const wxString &name);

   // TrackingData
   TrackingData* CreateTrackingData(const wxString &name);
   TrackingData* GetTrackingData(const wxString &name);

   // Core Measurement
   CoreMeasurement* CreateMeasurement(const wxString &type,
         const wxString &name);
   CoreMeasurement* GetMeasurement(const wxString &type,
         const wxString &name);
   
   // DataFile
   DataFile* CreateDataFile(const wxString &type,
                            const wxString &name);
   DataFile* GetDataFile(const wxString &name);
   
   // ObType
   ObType* CreateObType(const wxString &type,
                        const wxString &name);
   ObType* GetObType(const wxString &name);

   // EventLocator
   EventLocator* CreateEventLocator(const wxString &type,
                            const wxString &name);
   EventLocator* GetEventLocator(const wxString &name);

   // Interpolator
   Interpolator* CreateInterpolator(const wxString &type,
                                    const wxString &name);
   Interpolator* GetInterpolator(const wxString &name);
   
   // CoordinateSystem
   CoordinateSystem* CreateCoordinateSystem(const wxString &name,
                                            bool createDefault = false,
                                            bool internal = false,
                                            Integer manage = 1);
   CoordinateSystem* GetCoordinateSystem(const wxString &name);
   const StringArray& GetDefaultCoordinateSystemNames();
   
   // Subscriber
   Subscriber* CreateSubscriber(const wxString &type,
                                const wxString &name,
                                const wxString &fileName = wxT(""),
                                bool createDefault = false);
   Subscriber* GetSubscriber(const wxString &name);
   
   // EphemerisFile
   Subscriber* CreateEphemerisFile(const wxString &type,
                                   const wxString &name);
   Subscriber* GetEphemerisFile(const wxString &name);
   void        HandleCcsdsEphemerisFile(ObjectMap *objMap, bool deleteOld = false);
   
   // Function
   Function* CreateFunction(const wxString &type,
                            const wxString &name,
                            Integer manage = 1);
   Function* GetFunction(const wxString &name);
   
   // Create other object
   GmatBase* CreateOtherObject(Gmat::ObjectType objType, const wxString &type,
                               const wxString &name, bool createDefault = false);
   
   //----- Non-Configurable Items
   // StopCondition
   StopCondition* CreateStopCondition(const wxString &type,
                                      const wxString &name);
   
   // AxisSystem
   AxisSystem* CreateAxisSystem(const wxString &type,
                                const wxString &name,
                                Integer manage = 1);
   
   // MathNode
   MathNode* CreateMathNode(const wxString &type,
                            const wxString &name = wxT(""));
   
   // AxisSystem
   Attitude* CreateAttitude(const wxString &type,
                            const wxString &name);
   
   // GmatCommand
   GmatCommand* InterpretGmatFunction(const wxString &fileName);
   GmatCommand* InterpretGmatFunction(Function *funct,
                                      ObjectMap *objMap = NULL,
                                      SolarSystem *ss = NULL);
   GmatCommand* CreateCommand(const wxString &type,
                              const wxString &name, bool &retFlag);
   GmatCommand* CreateDefaultCommand(const wxString &type,
                                     const wxString &name = wxT(""),
                                     GmatCommand *refCmd = NULL);
   GmatCommand* AppendCommand(const wxString &type,
                              const wxString &name, bool &retFlag,
                              Integer sandboxNum = 1);
   GmatCommand* DeleteCommand(GmatCommand *cmd, Integer sandboxNum = 1);
   GmatCommand* GetFirstCommand(Integer sanboxNum = 1);
   bool AppendCommand(GmatCommand *cmd, Integer sandboxNum = 1);
   bool InsertCommand(GmatCommand *cmd, GmatCommand *prevCmd,
                      Integer sandboxNum = 1);
   void SetCommandsUnchanged(Integer whichList = 0); 
   void ValidateCommand(GmatCommand *cmd);
   
   // CoordinateSystem
   CoordinateSystem* GetInternalCoordinateSystem();
   
   // Planetary files
   const StringArray& GetPlanetarySourceTypes();
   const StringArray& GetPlanetarySourceNames();
   const StringArray& GetPlanetarySourceTypesInUse();
//   const StringArray& GetAnalyticModelNames();
//   bool SetAnalyticModelToUse(const wxString &modelName);
   bool SetPlanetarySourceName(const wxString &sourceType,
                               const wxString &fileName);
   Integer SetPlanetarySourceTypesInUse(const StringArray &sourceTypes); 
   Integer GetPlanetarySourceId(const wxString &sourceType);
   wxString GetPlanetarySourceName(const wxString &sourceType);
   wxString GetCurrentPlanetarySource();
   
   // Potential field files
   wxString GetPotentialFileName(const wxString &fileType);
   
   // Getting file names
   // This will eventually replace Get*FileName() above (loj: 7/7/05)
   wxString GetFileName(const wxString &fileType);
   
   // Mission
   bool LoadDefaultMission();
   
   // Resource
   bool ClearResource();
   
   // Mission sequence
   bool ClearCommandSeq(bool leaveFirstCmd = true, bool callRunComplete = true,
                        Integer sandboxNum = 1);
   
   // Sandbox
   void ClearAllSandboxes();
   GmatBase* GetInternalObject(const wxString &name, Integer sandboxNum = 1);
   Integer RunMission(Integer sandboxNum = 1);
   Integer ChangeRunState(const wxString &state, Integer sandboxNum = 1);
   Gmat::RunState GetUserInterrupt();
   Gmat::RunState GetRunState();
   
   // Script
   bool InterpretScript(const wxString &filename, bool readBack = false,
                        const wxString &newPath = wxT(""));
   bool InterpretScript(wxInputStream *ss, bool clearObjs);
   bool SaveScript(const wxString &filename,
                   Gmat::WriteMode mode = Gmat::SCRIPTING);
   wxString GetScript(Gmat::WriteMode mode = Gmat::SCRIPTING);
   Integer RunScript(Integer sandboxNum = 1);
   
   // MATLAB Server Startup Interface
   bool StartMatlabServer();
   
   // Plugin GUI data
   std::vector<Gmat::PluginResource*> *GetPluginResourceList();

   bool IsSequenceStarter(const wxString &commandType);
   const wxString& GetStarterStringList();

private:
   
   // initialization
   void CreatePlanetaryCoeffFile();
   void CreateTimeFile();
   
   // prepare next script reading
   void PrepareNextScriptReading(bool clearObjs = true);
   
   // create default objects
   void CreateSolarSystemInUse();
   void CreateInternalCoordSystem();
   void CreateDefaultCoordSystems();
   void CreateDefaultBarycenter();
   void CreateDefaultMission();
   
   // Parameter reference object setting
   void CheckParameterType(Parameter *param, const wxString &type,
                           const wxString &ownerName);
   
   // object map
   GmatBase* FindObject(const wxString &name);
   bool AddObject(GmatBase *obj);
   void SetSolarSystemAndObjectMap(SolarSystem *ss, ObjectMap *objMap,
                                   bool forFunction,
                                   const wxString &callFrom = wxT(""));

   // Handlers for the commands that can start a mission sequence
   StringArray sequenceStarters;
   wxString starterList;
   const StringArray& GetSequenceStarters();
   
   // default objects
   Spacecraft* GetDefaultSpacecraft();
   PropSetup*  GetDefaultPropSetup();
   Burn*       GetDefaultBurn(const wxString &type);
   Hardware*   GetDefaultHardware(const wxString &type);
   Solver*     GetDefaultSolver();
   Subscriber* GetDefaultSubscriber(const wxString &type,
                                    bool addObjects = true,
                                    bool createIfNoneFound = true);
   Parameter*  GetDefaultX();
   Parameter*  GetDefaultY();
   StopCondition* CreateDefaultStopCondition();
   
   // sandbox
   void AddSolarSystemToSandbox(Integer index);
   void AddTriggerManagersToSandbox(Integer index);
   void AddInternalCoordSystemToSandbox(Integer index);
   void AddPublisherToSandbox(Integer index);
   void AddSubscriberToSandbox(Integer index);
   void AddOtherObjectsToSandbox(Integer index);   
   void AddCommandToSandbox(Integer index);
   void InitializeSandbox(Integer index);
   void ExecuteSandbox(Integer index);
   
   // for Debug
   void ShowCommand(const wxString &title1, GmatCommand *cmd1,
                    const wxString &title2 = wxT(""), GmatCommand *cmd2 = NULL);
   void ShowObjectMap(const wxString &title, ObjectMap *objMap = NULL);
   
   Moderator();
   virtual ~Moderator();
   
   // member data
   bool isSlpAlreadyInUse;
   bool isRunReady;
   bool isFromGui;
   bool endOfInterpreter;
   bool showFinalState;
   bool loadSandboxAndPause;
   Integer objectManageOption;
   std::vector<Sandbox*> sandboxes;
   std::vector<TriggerManager*> triggerManagers;
   std::vector<GmatCommand*> commands;
   
   ObjectMap *objectMapInUse;
   Function *currentFunction;
   ObjectArray unmanagedFunctions;
   
   static Moderator *instance;
   static ScriptInterpreter *theUiInterpreter;
   static ScriptInterpreter *theScriptInterpreter;
   ConfigManager *theConfigManager;
   FactoryManager *theFactoryManager;
   FileManager *theFileManager;
   Publisher *thePublisher;
   
   SolarSystem *theDefaultSolarSystem;
   SolarSystem *theSolarSystemInUse;
   SolarSystem *theInternalSolarSystem;
   CoordinateSystem *theInternalCoordSystem;
   StringArray defaultCoordSystemNames;
   StringArray tempObjectNames;
   EopFile *theEopFile;
   ItrfCoefficientsFile *theItrfFile;
   LeapSecsFileReader *theLeapSecsFile;
   Interface *theMatlabInterface;
   Gmat::RunState runState;

   // Dynamic library data table
   std::map<wxString, DynamicLibrary*>   userLibraries;
   std::vector<Gmat::PluginResource*>  userResources;
};

#endif // Moderator_hpp

