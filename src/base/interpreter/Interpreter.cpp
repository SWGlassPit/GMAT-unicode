//$Id: Interpreter.cpp 9783 2011-08-19 21:43:47Z lindajun $
//------------------------------------------------------------------------------
//                                  Interpreter
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Darrel J. Conway
// Created: 2003/08/28
// Rework:  2006/09/27 by Linda Jun (NASA/GSFC)
// Modified: 
//    2010.03.24 Thomas Grubb
//      - Fixed FinalPass method to gracefully print error message instead of 
//        access violation when celestial body is not set properly
//    2010.03.23 Thomas Grubb/Steve Hughes
//      - Fixed error message in SetPropertyToValue for invalid field values
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
/**
 * Class implementation for the Interpreter base class
 */
//------------------------------------------------------------------------------

#include "Interpreter.hpp"    // class's header file
#include "Moderator.hpp"
#include "StringTokenizer.hpp"
#include "ConditionalBranch.hpp"
#include "StringUtil.hpp"     // for ToReal()
#include "TimeTypes.hpp"
#include "Array.hpp"
#include "Assignment.hpp"     // for GetLHS(), GetRHS()
#include "Validator.hpp"
#include "ElementWrapper.hpp"
#include "MessageInterface.hpp"
#include "FileUtil.hpp"       // for DoesFileExist()
#include "GmatGlobal.hpp"     // for GetMatlabFuncNameExt()
#include "Covariance.hpp"

#include <stack>              // for checking matching begin/end control logic
#include <fstream>            // for checking GmatFunction declaration
#include <sstream>            // for checking GmatFunction declaration

//#define __DO_NOT_USE_OBJ_TYPE_NAME__

//#define DEBUG_HANDLE_ERROR
//#define DEBUG_INIT
//#define DEBUG_COMMAND_LIST
//#define DEBUG_COMMAND_VALIDATION
//#define DEBUG_OBJECT_LIST
//#define DEBUG_ARRAY_GET
//#define DEBUG_CREATE_OBJECT
//#define DEBUG_CREATE_CELESTIAL_BODY
//#define DEBUG_CREATE_PARAM
//#define DEBUG_CREATE_ARRAY
//#define DEBUG_CREATE_COMMAND
//#define DEBUG_CREATE_CALLFUNCTION
//#define DEBUG_VALIDATE_COMMAND
//#define DEBUG_WRAPPERS
//#define DEBUG_MAKE_ASSIGNMENT
//#define DEBUG_ASSEMBLE_COMMAND
//#define DEBUG_ASSEMBLE_CREATE
//#define DEBUG_ASSEMBLE_CONDITION
//#define DEBUG_ASSEMBLE_FOR
//#define DEBUG_ASSEMBLE_CALL_FUNCTION
//#define DEBUG_ASSEMBLE_REPORT_COMMAND
//#define DEBUG_SET
//#define DEBUG_SET_FORCE_MODEL
//#define DEBUG_SET_SOLAR_SYS
//#define DEBUG_CHECK_OBJECT
//#define DEBUG_CHECK_BRANCH
//#define DEBUG_SPECIAL_CASE
//#define DEBUG_PARSE_REPORT
//#define DEBUG_OBJECT_MAP
//#define DEBUG_FIND_OBJECT
//#define DEBUG_FIND_PROP_ID
//#define DEBUG_VAR_EXPRESSION
//#define DEBUG_MATH_TREE
//#define DEBUG_FUNCTION
//#define DBGLVL_FUNCTION_DEF 2
//#define DBGLVL_FINAL_PASS 2
//#define DEBUG_AXIS_SYSTEM
//#define DEBUG_SET_MEASUREMENT_MODEL
//#define DEBUG_ALL_OBJECTS

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//---------------------------------
// static data
//---------------------------------

StringArray Interpreter::allObjectTypeList = StringArray(1, wxT(""));
StringArray Interpreter::viewableCommandList = StringArray(1, wxT(""));
std::map<wxString, Gmat::ObjectType> Interpreter::objectTypeMap;

const wxString Interpreter::defaultIndicator = wxT("DFLT__");


//------------------------------------------------------------------------------
// Interpreter(SolarSystem *ss = NULL, ObjectMap *objMap = NULL)
//------------------------------------------------------------------------------
/**
 * Default constructor.
 *
 * @param  ss  The solar system to be used for findnig bodies
 * @param  objMap  The object map to be used for finding object 
 */
//------------------------------------------------------------------------------
Interpreter::Interpreter(SolarSystem *ss, ObjectMap *objMap)
{
   initialized = false;
   continueOnError = true;
   parsingDelayedBlock = false;
   ignoreError = false;
   inScriptEvent = false;
   inFunctionMode = false;
   hasFunctionDefinition = false;
   currentFunction = NULL;
   theSolarSystem = NULL;
   theObjectMap = NULL;
   
   theModerator  = Moderator::Instance();
   theReadWriter = ScriptReadWriter::Instance();
   theValidator = Validator::Instance();
   // Set Interpreter to singleton Validator
   theValidator->SetInterpreter(this);
   
   if (ss)
   {
      theSolarSystem = ss;
      theValidator->SetSolarSystem(ss);
   }
   
   if (objMap)
   {
      theObjectMap = objMap;
      theValidator->SetObjectMap(objMap);
      #ifdef DEBUG_OBJECT_MAP
      MessageInterface::ShowMessage
         (wxT("Interpreter setting object map <%p> to Validator\n"), theObjectMap);
      #endif
   }
   
   #ifdef DEBUG_INTERP
   MessageInterface::ShowMessage
      (wxT("Interpreter::Interpreter() initialized=%d, theModerator=%p, theReadWriter=%p, ")
       wxT("theValidator=%p\n"), initialized, theModerator, theReadWriter, theValidator);
   #endif
}


//------------------------------------------------------------------------------
// Interpreter()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
Interpreter::~Interpreter()
{
}


//------------------------------------------------------------------------------
// void Initialize()
//------------------------------------------------------------------------------
/**
 * Builds core lists of available objects.
 */
//------------------------------------------------------------------------------
void Interpreter::Initialize()
{
   #ifdef DEBUG_INIT
   MessageInterface::ShowMessage
      (wxT("Interpreter::Initialize() entered, initialized=%d\n"), initialized);
   #endif
   
   errorList.clear();
   delayedBlocks.clear();
   delayedBlockLineNumbers.clear();
   inCommandMode = false;
   parsingDelayedBlock = false;
   ignoreError = false;
   
   if (initialized)
   {
      #ifdef DEBUG_INIT
      MessageInterface::ShowMessage
         (wxT("Interpreter::Initialize() already initialized so just returning\n"));
      #endif
      return;
   }
   
   BuildCreatableObjectMaps();
   
   // Register aliases used in scripting.  Plugins cannot use aliases, so this 
   // piece is performed outside of the creatable object map definitions.
   RegisterAliases();
   
   // Initialize TextParser command list
   theTextParser.Initialize(commandList);
   
   initialized = true;
   
   #ifdef DEBUG_INIT
   MessageInterface::ShowMessage(wxT("Interpreter::Initialize() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void BuildCreatableObjectMaps()
//------------------------------------------------------------------------------
/**
 * Constructs the lists of object type names available in the Factories.  
 * 
 * This method is called whenever factories are registered with the 
 * FactoryManager.  During system startup, the Moderator makes this call after 
 * registering the default factories.  The call is reissued whenever a user
 * created factory is registered using the plug-in interfaces.
 */
//------------------------------------------------------------------------------
void Interpreter::BuildCreatableObjectMaps()
{
   // Build a mapping for all of the defined commands
   commandList.clear();
   StringArray cmds = theModerator->GetListOfFactoryItems(Gmat::COMMAND);
   copy(cmds.begin(), cmds.end(), back_inserter(commandList));
   
   #ifdef DEBUG_INIT
   MessageInterface::ShowMessage(wxT("\nNumber of commands = %d\n"), cmds.size());
   #endif
   
   #ifdef DEBUG_COMMAND_LIST
      std::vector<wxString>::iterator pos1;
      MessageInterface::ShowMessage(wxT("Commands:\n"));      
      for (pos1 = cmds.begin(); pos1 != cmds.end(); ++pos1)
         MessageInterface::ShowMessage(wxT("   ") + *pos1 + wxT("\n"));
   #endif
      
   if (cmds.size() == 0)
   {
      throw InterpreterException(wxT("Command list is empty."));
   }
   
   // Build a mapping for all viewable commands via GUI
   viewableCommandList.clear();
   cmds = theModerator->GetListOfViewableItems(Gmat::COMMAND);
   copy(cmds.begin(), cmds.end(), back_inserter(viewableCommandList));
   
   #ifdef DEBUG_INIT
   MessageInterface::ShowMessage(wxT("\nNumber of viewable commands = %d\n"), cmds.size());
   #endif
   
   #ifdef DEBUG_COMMAND_LIST
   std::vector<wxString>::iterator pos;
   MessageInterface::ShowMessage(wxT("Viewable Commands:\n"));      
   for (pos = cmds.begin(); pos != cmds.end(); ++pos)
      MessageInterface::ShowMessage(wxT("   ") + *pos + wxT("\n"));   
   #endif
   
   // Build a mapping for all of the defined objects
   allObjectTypeList.clear();
   celestialBodyList.clear();
   objectTypeMap.clear();
   
   StringArray scs = theModerator->GetListOfFactoryItems(Gmat::SPACECRAFT);
   //copy(scs.begin(), scs.end(), back_inserter(spacecraftList));
   copy(scs.begin(), scs.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < scs.size(); i++)
      objectTypeMap.insert(std::make_pair(scs[i], Gmat::SPACECRAFT));
   
   StringArray cbs = theModerator->GetListOfFactoryItems(Gmat::CELESTIAL_BODY);
   copy(cbs.begin(), cbs.end(), back_inserter(celestialBodyList));
   copy(cbs.begin(), cbs.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < cbs.size(); i++)
      objectTypeMap.insert(std::make_pair(cbs[i], Gmat::CELESTIAL_BODY));
   
   atmosphereList.clear();
   StringArray atms = theModerator->GetListOfFactoryItems(Gmat::ATMOSPHERE);
   copy(atms.begin(), atms.end(), back_inserter(atmosphereList));
   copy(atms.begin(), atms.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < atmosphereList.size(); i++)
      objectTypeMap.insert(std::make_pair(atmosphereList[i], Gmat::ATMOSPHERE));
   
   attitudeList.clear();
   StringArray atts = theModerator->GetListOfFactoryItems(Gmat::ATTITUDE);
   copy(atts.begin(), atts.end(), back_inserter(attitudeList));
   copy(atts.begin(), atts.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < attitudeList.size(); i++)
      objectTypeMap.insert(std::make_pair(attitudeList[i], Gmat::ATTITUDE));
   
   axisSystemList.clear();
   StringArray axes = theModerator->GetListOfFactoryItems(Gmat::AXIS_SYSTEM);
   copy(axes.begin(), axes.end(), back_inserter(axisSystemList));
   copy(axes.begin(), axes.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < axisSystemList.size(); i++)
      objectTypeMap.insert(std::make_pair(axisSystemList[i], Gmat::AXIS_SYSTEM));
   
   burnList.clear();
   StringArray burns = theModerator->GetListOfFactoryItems(Gmat::BURN);
   copy(burns.begin(), burns.end(), back_inserter(burnList));
   copy(burns.begin(), burns.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < burnList.size(); i++)
      objectTypeMap.insert(std::make_pair(burnList[i], Gmat::BURN));
   
   calculatedPointList.clear();
   StringArray cals = theModerator->GetListOfFactoryItems(Gmat::CALCULATED_POINT);
   copy(cals.begin(), cals.end(), back_inserter(calculatedPointList));
   copy(cals.begin(), cals.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < calculatedPointList.size(); i++)
      objectTypeMap.insert(std::make_pair(calculatedPointList[i], Gmat::CALCULATED_POINT));
   
   dataFileList.clear();
   StringArray dfs = theModerator->GetListOfFactoryItems(Gmat::DATA_FILE);
   copy(dfs.begin(), dfs.end(), back_inserter(dataFileList));
   copy(dfs.begin(), dfs.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < dataFileList.size(); i++)
      objectTypeMap.insert(std::make_pair(dataFileList[i], Gmat::DATA_FILE));
   
   ephemFileList.clear();
   StringArray ephems = theModerator->GetListOfFactoryItems(Gmat::EPHEMERIS_FILE);
   copy(ephems.begin(), ephems.end(), back_inserter(ephemFileList));
   copy(ephems.begin(), ephems.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < ephemFileList.size(); i++)
      objectTypeMap.insert(std::make_pair(ephemFileList[i], Gmat::EPHEMERIS_FILE));
   
   functionList.clear();
   StringArray fns = theModerator->GetListOfFactoryItems(Gmat::FUNCTION);
   copy(fns.begin(), fns.end(), back_inserter(functionList));
   copy(fns.begin(), fns.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < functionList.size(); i++)
      objectTypeMap.insert(std::make_pair(functionList[i], Gmat::FUNCTION));
   
   hardwareList.clear();
   StringArray hws = theModerator->GetListOfFactoryItems(Gmat::HARDWARE);
   copy(hws.begin(), hws.end(), back_inserter(hardwareList));
   copy(hws.begin(), hws.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < hardwareList.size(); i++)
      objectTypeMap.insert(std::make_pair(hardwareList[i], Gmat::HARDWARE));
   
   measurementList.clear();
   StringArray measurements = theModerator->GetListOfFactoryItems(Gmat::CORE_MEASUREMENT);
   copy(measurements.begin(), measurements.end(), back_inserter(measurementList));
   copy(measurements.begin(), measurements.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < measurementList.size(); i++)
      objectTypeMap.insert(std::make_pair(measurementList[i], Gmat::CORE_MEASUREMENT));
   
   obtypeList.clear();
   StringArray obs = theModerator->GetListOfFactoryItems(Gmat::OBTYPE);
   copy(obs.begin(), obs.end(), back_inserter(obtypeList));
   copy(obs.begin(), obs.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < obtypeList.size(); i++)
      objectTypeMap.insert(std::make_pair(obtypeList[i], Gmat::OBTYPE));
   
   odeModelList.clear();
   StringArray odes = theModerator->GetListOfFactoryItems(Gmat::ODE_MODEL);
   copy(odes.begin(), odes.end(), back_inserter(odeModelList));
   copy(odes.begin(), odes.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < odeModelList.size(); i++)
      objectTypeMap.insert(std::make_pair(odeModelList[i], Gmat::ODE_MODEL));
   
   parameterList.clear();
   StringArray parms = theModerator->GetListOfFactoryItems(Gmat::PARAMETER);
   copy(parms.begin(), parms.end(), back_inserter(parameterList));
   copy(parms.begin(), parms.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < parameterList.size(); i++)
      objectTypeMap.insert(std::make_pair(parameterList[i], Gmat::PARAMETER));
   
   propagatorList.clear();
   StringArray props = theModerator->GetListOfFactoryItems(Gmat::PROPAGATOR);
   copy(props.begin(), props.end(), back_inserter(propagatorList));
   copy(props.begin(), props.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < propagatorList.size(); i++)
      objectTypeMap.insert(std::make_pair(propagatorList[i], Gmat::PROPAGATOR));
   
   physicalModelList.clear();
   StringArray forces = theModerator->GetListOfFactoryItems(Gmat::PHYSICAL_MODEL);
   copy(forces.begin(), forces.end(), back_inserter(physicalModelList));
   copy(forces.begin(), forces.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < physicalModelList.size(); i++)
      objectTypeMap.insert(std::make_pair(physicalModelList[i], Gmat::PHYSICAL_MODEL));
   
   solverList.clear();
   StringArray solvers = theModerator->GetListOfFactoryItems(Gmat::SOLVER);
   copy(solvers.begin(), solvers.end(), back_inserter(solverList));
   copy(solvers.begin(), solvers.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < solverList.size(); i++)
      objectTypeMap.insert(std::make_pair(solverList[i], Gmat::SOLVER));
   
   stopcondList.clear();
   StringArray stops = theModerator->GetListOfFactoryItems(Gmat::STOP_CONDITION);
   copy(stops.begin(), stops.end(), back_inserter(stopcondList));
   copy(stops.begin(), stops.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < stopcondList.size(); i++)
      objectTypeMap.insert(std::make_pair(stopcondList[i], Gmat::STOP_CONDITION));
   
   subscriberList.clear();
   StringArray subs = theModerator->GetListOfFactoryItems(Gmat::SUBSCRIBER);
   copy(subs.begin(), subs.end(), back_inserter(subscriberList));
   copy(subs.begin(), subs.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < subscriberList.size(); i++)
      objectTypeMap.insert(std::make_pair(subscriberList[i], Gmat::SUBSCRIBER));
   
   spacePointList.clear();
   StringArray spl = theModerator->GetListOfFactoryItems(Gmat::SPACE_POINT);
   copy(spl.begin(), spl.end(), back_inserter(spacePointList));
   copy(spl.begin(), spl.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < spacePointList.size(); i++)
      objectTypeMap.insert(std::make_pair(spacePointList[i], Gmat::SPACE_POINT));
   
   trackingSystemList.clear();
   StringArray tsl = theModerator->GetListOfFactoryItems(Gmat::TRACKING_SYSTEM);
   copy(tsl.begin(), tsl.end(), back_inserter(trackingSystemList));
   copy(tsl.begin(), tsl.end(), back_inserter(allObjectTypeList));
   for (UnsignedInt i = 0; i < trackingSystemList.size(); i++)
      objectTypeMap.insert(std::make_pair(trackingSystemList[i], Gmat::TRACKING_SYSTEM));
   
   #ifdef DEBUG_OBJECT_LIST
      std::vector<wxString>::iterator pos;
      
      MessageInterface::ShowMessage(wxT("\nSpacecraft:\n   "));
      for (pos = scs.begin(); pos != scs.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nAtmosphereModel:\n   "));
      for (pos = atms.begin(); pos != atms.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));

      MessageInterface::ShowMessage(wxT("\nAttitudes:\n   "));
      for (pos = atts.begin(); pos != atts.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));

      MessageInterface::ShowMessage(wxT("\nAxisSystems:\n   "));
      for (pos = axes.begin(); pos != axes.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nBurns:\n   "));
      for (pos = burns.begin(); pos != burns.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nCalculatedPoints:\n   "));
      for (pos = cals.begin(); pos != cals.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nDataFiles:\n   "));
      for (pos = dfs.begin(); pos != dfs.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nEphemerisFiles:\n   "));
      for (pos = ephems.begin(); pos != ephems.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nFunctions:\n   "));
      for (pos = fns.begin(); pos != fns.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nHardwares:\n   "));
      for (pos = hws.begin(); pos != hws.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nODEModels:\n   "));
      for (pos = odes.begin(); pos != odes.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nPhysicalModels:\n   "));
      for (pos = forces.begin(); pos != forces.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nParameters:\n   "));
      for (pos = parms.begin();  pos != parms.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nPropagators:\n   "));
      for (std::vector<wxString>::iterator pos = props.begin();
           pos != props.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nMeasurements:\n   "));
      for (pos = measurements.begin();
            pos != measurements.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nObservations:\n   "));
      for (pos = obs.begin();
            pos != obs.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nSolvers:\n   "));
      for (pos = solvers.begin(); pos != solvers.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nStopConds:\n   "));
      for (pos = stops.begin(); pos != stops.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nSubscribers:\n   "));
      for (pos = subs.begin(); pos != subs.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nTrackingSystems:\n   "));
      for (pos = tsl.begin(); pos != tsl.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\nOther SpacePoints:\n   "));
      for (pos = spl.begin(); pos != spl.end(); ++pos)
         MessageInterface::ShowMessage(*pos + wxT("\n   "));
      
      MessageInterface::ShowMessage(wxT("\n"));
   #endif
   
}


//------------------------------------------------------------------------------
// StringArray GetCreatableList(Gmat::ObjectType type, Integer subType)
//------------------------------------------------------------------------------
/**
 * Returns the list of objects of a given type that can be built.
 * 
 * This method returns the list of object types supported by the current Factory 
 * system.  A future build will allow specification of a subtype -- for example,
 * for solvers, subtypes could be targeters, optimizers, iterators, and 
 * odSolvers.  The subType parameter is included to support this feature when it
 * becomes available.
 * 
 * @param type The Gmat::ObjectType requested.
 * @param subType The subtype.
 * 
 * @return The list of creatable objects.
 * 
 * @note The current implementation only supports the types in the Interpreter's
 *       lists of objects.  A future implementation should call 
 *       Moderator::GetListOfFactoryItems() instead. 
 */
//------------------------------------------------------------------------------
StringArray Interpreter::GetCreatableList(Gmat::ObjectType type, 
      const wxString subType)
{
   StringArray clist;
   
   switch (type)
   {
      case Gmat::CELESTIAL_BODY:
         clist = celestialBodyList;
         break;
      
      case Gmat::ATMOSPHERE:
         clist = atmosphereList;
         break;
         
      case Gmat::ATTITUDE:
         clist = attitudeList;
         break;
         
      case Gmat::AXIS_SYSTEM:
         clist = axisSystemList;
         break;
         
      case Gmat::BURN:
         clist = burnList;
         break;
         
      case Gmat::CALCULATED_POINT:
         clist = calculatedPointList;
         break;
         
      case Gmat::COMMAND:
         clist = commandList;
         break;
         
      case Gmat::DATA_FILE:
         clist = dataFileList;
         break;
         
      case Gmat::FUNCTION:
         clist = functionList;
         break;
         
      case Gmat::HARDWARE:
         clist = hardwareList;
         break;
         
      case Gmat::CORE_MEASUREMENT:
         clist = measurementList;
         break;

      case Gmat::OBTYPE:
         clist = obtypeList;
         break;

      case Gmat::ODE_MODEL:
         clist = odeModelList;
         break;
         
      case Gmat::PARAMETER:
         clist = parameterList;
         break;
         
      case Gmat::PROPAGATOR:
         clist = propagatorList;
         break;
         
      case Gmat::PHYSICAL_MODEL:
         clist = physicalModelList;
         break;
         
      case Gmat::SOLVER:
         clist = solverList;
         break;
         
      case Gmat::STOP_CONDITION:
         clist = stopcondList;
         break;
         
      case Gmat::SUBSCRIBER:
         clist = subscriberList;
         break;
         
      case Gmat::SPACE_POINT:
         clist = spacePointList;
         break;
         
      case Gmat::TRACKING_SYSTEM:
         clist = trackingSystemList;
         break;
         
      // These are all intentional fall-throughs:
      case Gmat::SPACECRAFT:
      case Gmat::FORMATION:
      case Gmat::SPACEOBJECT:
      case Gmat::GROUND_STATION:
      case Gmat::IMPULSIVE_BURN:
      case Gmat::FINITE_BURN:
      case Gmat::TRANSIENT_FORCE:
      case Gmat::INTERPOLATOR:
      case Gmat::SOLAR_SYSTEM:
//      case Gmat::CELESTIAL_BODY:
      case Gmat::LIBRATION_POINT:
      case Gmat::BARYCENTER:
      case Gmat::PROP_SETUP:
      case Gmat::FUEL_TANK:
      case Gmat::THRUSTER:
      case Gmat::COORDINATE_SYSTEM:
      case Gmat::MATH_NODE:
      case Gmat::MATH_TREE:
      case Gmat::MEASUREMENT_MODEL:
      case Gmat::DATASTREAM:
      case Gmat::TRACKING_DATA:
      case Gmat::UNKNOWN_OBJECT:
      default:
         break;
   }
   
   if (subType != wxT(""))
   {
      #ifdef DEBUG_SUBTYPES
         MessageInterface::ShowMessage(wxT("List has %d members:\n"), clist.size());
         for (UnsignedInt j = 0; j < clist.size(); ++j)
            MessageInterface::ShowMessage(wxT("   %s\n"), clist[j].c_str());
      #endif

      StringArray temp;
      // Throw away objects that do not match the subtype
      for (UnsignedInt i = 0; i < clist.size(); ++i)
      {
         if (theModerator->DoesObjectTypeMatchSubtype(type, clist[i], subType))
            temp.push_back(clist[i]);
      }
      clist = temp;

      #ifdef DEBUG_SUBTYPES
         MessageInterface::ShowMessage(wxT("Revised list has %d members:\n"), clist.size());
         for (UnsignedInt j = 0; j < clist.size(); ++j)
            MessageInterface::ShowMessage(wxT("   %s\n"), clist[j].c_str());
      #endif
   }

   return clist;
}

//------------------------------------------------------------------------------
// void SetInputFocus()
//------------------------------------------------------------------------------
/*
 * Some GMAT UiInterpreters need to be able to obtain focus for message 
 * processing.  This method is overridden to perform run complete actions for 
 * those interpreters.
 */
//------------------------------------------------------------------------------
void Interpreter::SetInputFocus()
{}

//------------------------------------------------------------------------------
// void NotifyRunCompleted()
//------------------------------------------------------------------------------
/*
 * Some GMAT UiInterpreters need to know when a run is finished.  This method is
 * overridden to perform run complete actions for those interpreters.
 */
//------------------------------------------------------------------------------
void Interpreter::NotifyRunCompleted()
{} 

//------------------------------------------------------------------------------
// void NotifyRunCompleted(Integer type)
//------------------------------------------------------------------------------
/*
 * Some GMAT UiInterpreters need to update their view into the configured 
 * objects.  This method is overridden to perform those updates.  The parameter
 * maps to the following values:
 * 
 *  1   Configured objects
 *  2   Commands
 *  3   Commands and configured objects
 *  4   Outputs
 *  5   Outputs and configured objects
 *  6   Commands and Outputs
 *  7   Everything (Commands, outputs, configured objects)
 * 
 * The default value is 7. 
 */
//------------------------------------------------------------------------------
void Interpreter::UpdateView(Integer type)
{}


//------------------------------------------------------------------------------
// void SetInputFocus()
//------------------------------------------------------------------------------
/*
 * Some GMAT UiInterpreters need to take actions when a project is closed.  This
 * method tells them to take those actions.
 */
//------------------------------------------------------------------------------
void Interpreter::CloseCurrentProject()
{}

//------------------------------------------------------------------------------
// void StartMatlabServer()
//------------------------------------------------------------------------------
/*
 * Some GMAT Interpreters can start external servers -- for example, the MATLAB
 * server.  This method is overridden to perform that startup.
 */
//------------------------------------------------------------------------------
void Interpreter::StartMatlabServer()
{
   throw InterpreterException(
         wxT("This Interpreter cannot start the external server"));
}

//------------------------------------------------------------------------------
// Interface* GetMatlabInterface()
//------------------------------------------------------------------------------
Interface* Interpreter::GetMatlabInterface()
{
   return theModerator->GetMatlabInterface();
}

//------------------------------------------------------------------------------
// bool OpenMatlabEngine()
//------------------------------------------------------------------------------
bool Interpreter::OpenMatlabEngine()
{
   return theModerator->OpenMatlabEngine();
}

//------------------------------------------------------------------------------
// bool CloseMatlabEngine()
//------------------------------------------------------------------------------
bool Interpreter::CloseMatlabEngine()
{
   return theModerator->CloseMatlabEngine();
}

//------------------------------------------------------------------------------
// void RegisterAliases()
//------------------------------------------------------------------------------
/*
 * Some GMAT script identifiers can be accessed using multiple text string.
 * This method creates a mapping for these strings so that scripts can be parsed
 * correctly.
 */
//------------------------------------------------------------------------------
void Interpreter::RegisterAliases()
{
   ODEModel::SetScriptAlias(wxT("PrimaryBodies"), wxT("GravityField"));
   ODEModel::SetScriptAlias(wxT("Gravity"), wxT("GravityField"));
   ODEModel::SetScriptAlias(wxT("PointMasses"), wxT("PointMassForce"));
   ODEModel::SetScriptAlias(wxT("Drag"), wxT("DragForce"));
   ODEModel::SetScriptAlias(wxT("SRP"), wxT("SolarRadiationPressure"));
}


//------------------------------------------------------------------------------
// const StringArray& GetListOfObjects(Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Returns names of all configured items of object type.
 *
 * @param  type  object type
 *
 * @return array of configured item names; return empty array if none
 */
//------------------------------------------------------------------------------
const StringArray& Interpreter::GetListOfObjects(Gmat::ObjectType type)
{
   return theModerator->GetListOfObjects(type);
}


//------------------------------------------------------------------------------
// const StringArray& GetListOfObjects(const wxString &typeName)
//------------------------------------------------------------------------------
/**
 * Returns names of all configured items of given object type name.
 *
 * @param  typeName  object type name
 *
 * @return array of configured item names; return empty array if none
 */
//------------------------------------------------------------------------------
const StringArray& Interpreter::GetListOfObjects(const wxString &typeName)
{
   return theModerator->GetListOfObjects(typeName);
}


//------------------------------------------------------------------------------
// const StringArray& GetListOfViewableCommands();
//------------------------------------------------------------------------------
/**
 * Returns names of all viewable commands via GUI
 * @return array of item names; return empty array if none
 */
//------------------------------------------------------------------------------
const StringArray& Interpreter::GetListOfViewableCommands()
{
   return Interpreter::viewableCommandList;
}


//------------------------------------------------------------------------------
// const StringArray& GetListOfViewableSubtypesOf(Gmat::ObjectType type)
//------------------------------------------------------------------------------
const StringArray& Interpreter::GetListOfViewableSubtypesOf(Gmat::ObjectType type)
{
   return theModerator->GetListOfViewableItems(type);
}


//------------------------------------------------------------------------------
// GmatBase* GetConfiguredObject(const wxString &name)
//------------------------------------------------------------------------------
GmatBase* Interpreter::GetConfiguredObject(const wxString &name)
{
   return theModerator->GetConfiguredObject(name);
}


//------------------------------------------------------------------------------
// GmatBase* CreateObject(const wxString &type, const wxString &name,
//                        Integer manage, bool createDefault)
//------------------------------------------------------------------------------
/**
 * Calls the Moderator to build core objects and put them in the ConfigManager.
 *  
 * @param  type  Type for the requested object.
 * @param  name  Name for the object
 * @param  manage   0, if parameter is not managed
 *                  1, if parameter is added to configuration (default)
 *                  2, if Parameter is added to function object map
 * @param <createDefault> set to true if default object to be created (false)
 *
 * @return object pointer on success, NULL on failure.
 */
//------------------------------------------------------------------------------
GmatBase* Interpreter::CreateObject(const wxString &type,
                                    const wxString &name,
                                    Integer manage, bool createDefault)
{
   #ifdef DEBUG_CREATE_OBJECT
   MessageInterface::ShowMessage
      (wxT("Interpreter::CreateObject() type=<%s>, name=<%s>, manage=%d, createDefault=%d\n"),
       type.c_str(), name.c_str(), manage, createDefault);
   #endif
   
   debugMsg = wxT("In CreateObject()");
   GmatBase *obj = NULL;
   
   // if object to be managed and has non-blank name, and name is not valid, handle error
   if (manage == 1 && name != wxT(""))
   {
      bool isValid = false;
      
      // if type is Array, set flag to ignore bracket
      if (type == wxT("Array"))
         isValid = GmatStringUtil::IsValidName(name, true);
      else
         isValid = GmatStringUtil::IsValidName(name, false);
      
      if (!isValid)
      {
         #ifdef DEBUG_CREATE_OBJECT
         MessageInterface::ShowMessage
            (wxT("Object name %s is NOT valid\n"), name.c_str());
         #endif
         InterpreterException ex
            (type + wxT(" object can not be named to \"") + name + wxT("\""));
         HandleError(ex);
         return NULL;
      }
   }
   
   // Go through more checking if name is not blank
   if (name != wxT(""))
   {
      // object name cannot be any of command names
      if (IsCommandType(name))
      {
         InterpreterException ex
            (type + wxT(" object can not be named to Command \"") + name + wxT("\""));
         HandleError(ex);
         return NULL;
      }
      
      #ifdef __DO_NOT_USE_OBJ_TYPE_NAME__
      // object name cannot be any of object types
      if (IsObjectType(name))
      {
         InterpreterException ex
            (type + wxT(" object can not be named to Object Type \"") + name + wxT("\""));
         HandleError(ex);
         return NULL;
      }
      #endif
      
      // If object to be managed, give warning if name already exist
      if (manage == 1)
      {
         if ((name != wxT("EarthMJ2000Eq")) && 
             (name != wxT("EarthMJ2000Ec")) && 
             (name != wxT("EarthFixed")))
         {
            obj = FindObject(name);
            // Since System Parameters are created automatically as they are referenced,
            // do not give warning if creating a system parameter
            if (obj != NULL && ((obj->GetType() != Gmat::PARAMETER) ||
                                (obj->GetType() == Gmat::PARAMETER &&
                                 (!obj->IsOfType(wxT("SystemParameter"))))))
            {
               InterpreterException ex(wxT(""));
               ex.SetDetails(wxT("%s object named \"%s\" already exists"),
                             type.c_str(), name.c_str());
               HandleError(ex, true, true);
               return obj;
            }
         }
      }
   }
   #ifdef DEBUG_CREATE_CELESTIAL_BODY
   MessageInterface::ShowMessage
      (wxT("In CreateObject, about to set object manage option %d\n"), manage);
   #endif
   
   // Set manage option to Moderator
   theModerator->SetObjectManageOption(manage);
   
   //======================================================================
   // This block of code is the future implementation of creating objects
   // of non-special object types in general way. This will avoid adding
   // specific methods to the Moderator when we create new object type
   // through the plug-in code. This is just initial coding and needs
   // thorough testing. (LOJ: 2010.05.05)
   //======================================================================
   #ifdef __NEW_WAY_OF_CREATE_OBJECT__
   //======================================================================
   
   // Handle container or special object type creation
   if (type == wxT("PropSetup"))
      obj = (GmatBase*)theModerator->CreatePropSetup(name);
   
   // Handle Spacecraft
   else if (type == wxT("Spacecraft") || type == wxT("Formation"))
      obj = (GmatBase*)theModerator->CreateSpacecraft(type, name);
   
   // Handle AxisSystem
   else if (find(axisSystemList.begin(), axisSystemList.end(), type) != 
            axisSystemList.end())
      obj =(GmatBase*) theModerator->CreateAxisSystem(type, name);
   
   // Handle Burns
   else if (find(burnList.begin(), burnList.end(), type) != 
            burnList.end())
      obj = (GmatBase*)theModerator->CreateBurn(type, name, createDefault);
   
   // Handle CoordinateSystem
   else if (type == wxT("CoordinateSystem"))
      obj = (GmatBase*)theModerator->CreateCoordinateSystem(name, false, false, manage);
   
   // Handle CelestialBody
   else if (find(celestialBodyList.begin(), celestialBodyList.end(), type) != 
            celestialBodyList.end())
      obj = (GmatBase*)theModerator->CreateCelestialBody(type, name);
   
   // Handle CalculatedPoint
   else if (find(calculatedPointList.begin(), calculatedPointList.end(), type) != 
            calculatedPointList.end())
      obj =(GmatBase*) theModerator->CreateCalculatedPoint(type, name, true);
   
   // Handle Parameters
   else if (find(parameterList.begin(), parameterList.end(), type) != 
            parameterList.end())
      obj = (GmatBase*)CreateParameter(type, name, wxT(""), wxT(""));
   
   // Handle Subscribers
   else if (find(subscriberList.begin(), subscriberList.end(), type) != 
            subscriberList.end())
      obj = (GmatBase*)theModerator->CreateSubscriber(type, name);
   
   // Handle other registered creatable object types
   else if (find(allObjectTypeList.begin(), allObjectTypeList.end(), type) != 
            allObjectTypeList.end())
   {
      Gmat::ObjectType objType = GetObjectType(type);
      if (objType != Gmat::UNKNOWN_OBJECT)
         obj = theModerator->CreateOtherObject(objType, type, name);
      else
         obj = NULL;
   }
   else
   {
      obj = NULL;
   }
   
   //@note
   // Do not throw exception if obj == NULL, since caller uses return pointer
   // to test further.
   
   #ifdef DEBUG_CREATE_OBJECT
   if (obj != NULL)
   {
      MessageInterface::ShowMessage
         (wxT("Interpreter::CreateObject() type=<%s>, name=<%s> successfully created\n"),
          obj->GetTypeName().c_str(), obj->GetName().c_str());
   }
   #endif
   
   return obj;
   
   //======================================================================
   #else
   //======================================================================
   
   if (type == wxT("Spacecraft")) 
      obj = (GmatBase*)theModerator->CreateSpacecraft(type, name);
   
   else if (type == wxT("Formation")) 
      obj = (GmatBase*)theModerator->CreateSpacecraft(type, name);
   
   else if (type == wxT("PropSetup")) 
      obj = (GmatBase*)theModerator->CreatePropSetup(name);
   
   else if (type == wxT("MeasurementModel"))
      obj = (GmatBase*)theModerator->CreateMeasurementModel(name);

   else if (type == wxT("TrackingData"))
      obj = (GmatBase*)theModerator->CreateTrackingData(name);
   
   else if (type == wxT("DataFile"))
      obj = (GmatBase*)theModerator->CreateDataFile(type, name);
   
   else if (type == wxT("CoordinateSystem")) 
      //obj = (GmatBase*)theModerator->CreateCoordinateSystem(name, true);
      obj = (GmatBase*)theModerator->CreateCoordinateSystem(name, false, false, manage);
   
   else
   {
      #ifdef DEBUG_CREATE_CELESTIAL_BODY
      MessageInterface::ShowMessage(wxT("In CreateObject, type = %s\n"), type.c_str());
      MessageInterface::ShowMessage(wxT("In CreateObject, list of celestial body types are: \n"));
      for (unsigned int ii = 0; ii < celestialBodyList.size(); ii++)
         MessageInterface::ShowMessage(wxT(" ... %s\n"), (celestialBodyList.at(ii)).c_str());
      #endif
      // Handle Propagator
      if (find(propagatorList.begin(), propagatorList.end(), type) != 
          propagatorList.end())
         obj = (GmatBase*)theModerator->CreatePropagator(type, name);
      
      // Handle ODEModel
      if (find(odeModelList.begin(), odeModelList.end(), type) != 
          odeModelList.end())
         obj = (GmatBase*)theModerator->CreateODEModel(type, name);
      
      // Handle AxisSystem
      else if (find(axisSystemList.begin(), axisSystemList.end(), type) != 
               axisSystemList.end())
         obj =(GmatBase*) theModerator->CreateAxisSystem(type, name);
      
      // Handle Celestial Body
      else if (find(celestialBodyList.begin(), celestialBodyList.end(), type) != 
            celestialBodyList.end())
         obj = (GmatBase*)theModerator->CreateCelestialBody(type, name);
      
      // Handle Atmosphere Model
      else if (find(atmosphereList.begin(), atmosphereList.end(), type) != 
               atmosphereList.end())
         obj = (GmatBase*)theModerator->CreateAtmosphereModel(type, name);
      
      // Handle Attitude
      else if (find(attitudeList.begin(), attitudeList.end(), type) != 
               attitudeList.end())
         obj = (GmatBase*)theModerator->CreateAttitude(type, name);
      
      // Handle Burns
      else if (find(burnList.begin(), burnList.end(), type) != 
               burnList.end())
         obj = (GmatBase*)theModerator->CreateBurn(type, name, createDefault);
      
      // Handle CalculatedPoint (Barycenter, LibrationPoint)
      // Creates default Barycentor or LibrationPoint
      else if (find(calculatedPointList.begin(), calculatedPointList.end(), type) != 
               calculatedPointList.end())
         obj =(GmatBase*) theModerator->CreateCalculatedPoint(type, name, true);
      
      // Handle DataFiles
      else if (find(dataFileList.begin(), dataFileList.end(), type) != 
               dataFileList.end())
         obj = (GmatBase*)theModerator->CreateDataFile(type, name);
      
      // Handle Functions
      else if (find(functionList.begin(), functionList.end(), type) != 
               functionList.end())
         obj = (GmatBase*)theModerator->CreateFunction(type, name, manage);
      
      // Handle Hardware (tanks, thrusters, etc.)
      else if (find(hardwareList.begin(), hardwareList.end(), type) != 
               hardwareList.end())
         obj = (GmatBase*)theModerator->CreateHardware(type, name);
      
      // Handle Measurements
      else if (find(measurementList.begin(), measurementList.end(), type) !=
               measurementList.end())
         obj = (GmatBase*)theModerator->CreateMeasurement(type, name);

      // Handle Observations
      else if (find(obtypeList.begin(), obtypeList.end(), type) !=
            obtypeList.end())
         obj = (GmatBase*)theModerator->CreateObType(type, name);

      // Handle Parameters
      else if (find(parameterList.begin(), parameterList.end(), type) != 
               parameterList.end())
         obj = (GmatBase*)CreateParameter(type, name, wxT(""), wxT(""));
      
      // Handle PhysicalModel
      else if (find(physicalModelList.begin(), physicalModelList.end(), type) != 
               physicalModelList.end())
         obj = (GmatBase*)theModerator->CreatePhysicalModel(type, name);
      
      // Handle Solvers
      else if (find(solverList.begin(), solverList.end(), type) != 
               solverList.end())
         obj = (GmatBase*)theModerator->CreateSolver(type, name);
      
      // Handle Subscribers
      else if (find(subscriberList.begin(), subscriberList.end(), type) != 
               subscriberList.end())
         obj = (GmatBase*)theModerator->CreateSubscriber(type, name);
      
      // Handle EphemerisFile
      else if (find(ephemFileList.begin(), ephemFileList.end(), type) != 
               ephemFileList.end())
         obj = (GmatBase*)theModerator->CreateEphemerisFile(type, name);
      
      // Handle other SpacePoints
      else if (find(spacePointList.begin(), spacePointList.end(), type) != 
               spacePointList.end())
         obj = (GmatBase*)theModerator->CreateSpacePoint(type, name);
   
      // Handle TrackingSystems
      else if (find(trackingSystemList.begin(), trackingSystemList.end(), type) !=
               trackingSystemList.end())
         obj = (GmatBase*)theModerator->CreateTrackingSystem(type, name);
   }
   
   //@note
   // Do not throw exception if obj == NULL, since caller uses return pointer
   // to test further.
   
   
   #ifdef DEBUG_CREATE_OBJECT
   if (obj != NULL)
   {
      MessageInterface::ShowMessage
         (wxT("Interpreter::CreateObject() type=<%s>, name=<%s> successfully created\n"),
          obj->GetTypeName().c_str(), obj->GetName().c_str());
   }
   #endif
   
   return obj;
   #endif
}


//------------------------------------------------------------------------------
// void SetConfiguredObjectMap()
//------------------------------------------------------------------------------
/*
 * Sets object map in use to object map from the configuration.
 */
//------------------------------------------------------------------------------
void Interpreter::SetConfiguredObjectMap()
{
   theObjectMap = theModerator->GetConfiguredObjectMap();
   theValidator->SetObjectMap(theObjectMap);
}


//------------------------------------------------------------------------------
// void SetSolarSystemInUse(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 * Sets a current solar system in use.
 *
 * @param <ss> Pointer to the solar system
 *
 */
//------------------------------------------------------------------------------
void Interpreter::SetSolarSystemInUse(SolarSystem *ss)
{
   #ifdef DEBUG_SET_SOLAR_SYS
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetSolarSystemInUse() ss=<%p>\n"), ss);
   #endif
   
   if (ss != NULL)
   {
      theSolarSystem = ss;
      theValidator->SetSolarSystem(ss);
   }
}


//------------------------------------------------------------------------------
// SolarSystem* GetSolarSystemInUse()
//------------------------------------------------------------------------------
/**
 * Retrieves a current solar system in use.
 *
 * @return a default solar system object pointer
 */
//------------------------------------------------------------------------------
SolarSystem* Interpreter::GetSolarSystemInUse()
{
   return theSolarSystem;
}


//------------------------------------------------------------------------------
// void SetObjectMap(ObjectMap *objMap, bool forFunction)
//------------------------------------------------------------------------------
/**
 * Sets object map to be used for finding objects.
 * 
 * @param <objMap> Pointer to the object map
 * @param <forFunction> True if setting object map for function (false)
 */
//------------------------------------------------------------------------------
void Interpreter::SetObjectMap(ObjectMap *objMap, bool forFunction)
{
   #ifdef DEBUG_OBJECT_MAP
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetObjectMap() objMap=<%p>, forFunction=%d\n"), objMap,
       forFunction);
   #endif
   
   if (objMap != NULL)
   {
      if (forFunction)
      {
         #ifdef DEBUG_OBJECT_MAP
         MessageInterface::ShowMessage
            (wxT("Interpreter::SetObjectMap() Here is the current object map <%p>, ")
             wxT("it has %d objects\n"), objMap, objMap->size());
         for (std::map<wxString, GmatBase *>::iterator i = objMap->begin();
              i != objMap->end(); ++i)
         {
            MessageInterface::ShowMessage
               (wxT("   %30s  <%p><%s>\n"), i->first.c_str(), i->second,
                i->second == NULL ? wxT("NULL") : (i->second)->GetTypeName().c_str());
         }
         #endif
      }
      
      theObjectMap = objMap;
      theValidator->SetObjectMap(objMap);
   }
}


//------------------------------------------------------------------------------
// ObjectMap* GetObjectMap()
//------------------------------------------------------------------------------
/**
 * @return a current object map in use.
 */
//------------------------------------------------------------------------------
ObjectMap* Interpreter::GetObjectMap()
{
   return theObjectMap;
}


//------------------------------------------------------------------------------
// void SetFunction(Function *func)
//------------------------------------------------------------------------------
/*
 * Sets Function pointer for function mode interpreting and to the Validator.
 *
 * @param  func  The Function pointer to set
 */
//------------------------------------------------------------------------------
void Interpreter::SetFunction(Function *func)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetFunction() function=<%p>'%s'\n"), func,
       func ? func->GetName().c_str() : wxT("NULL"));
   #endif
   
   currentFunction = func;
   theValidator->SetFunction(func);
}


//------------------------------------------------------------------------------
// Function* GetFunction()
//------------------------------------------------------------------------------
/*
 * Retrives Function pointer currently set for function mode interpreting.
 */
Function* Interpreter::GetFunction()
{
   return currentFunction;
}


//------------------------------------------------------------------------------
// bool CheckUndefinedReference(GmatBase *obj, bool writeLine)
//------------------------------------------------------------------------------
/*
 * This method checks if reference object of given object exist through
 * the Validator.
 *
 * @param  obj  input object of undefined reference object to be checked
 * @param  writeLine  flag indicating whether or not line number should be
 *                    written for the error message
 */
//------------------------------------------------------------------------------
bool Interpreter::CheckUndefinedReference(GmatBase *obj, bool writeLine)
{
   debugMsg = wxT("In CheckUndefinedReference()");
   bool isValid = theValidator->CheckUndefinedReference(obj, continueOnError);
   
   // Handle error messages here
   if (!isValid)
   {
      StringArray errList = theValidator->GetErrorList();
      for (UnsignedInt i=0; i<errList.size(); i++)
         HandleError(InterpreterException(errList[i]), writeLine);
   }
   
   return isValid;
}


//------------------------------------------------------------------------------
// bool ValidateCommand(GmatCommand *cmd)
//------------------------------------------------------------------------------
/**
 * Checks the input command to make sure it wrappers are set up for it
 * correctly through the Validator, if necessary.
 *
 * @param  cmd  the command to validate
 */
//------------------------------------------------------------------------------
bool Interpreter::ValidateCommand(GmatCommand *cmd)
{
   #ifdef DEBUG_VALIDATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("Interpreter::ValidateCommand() cmd=<%p><%s>, inFunctionMode=%d\n"), cmd,
       cmd->GetTypeName().c_str(), inFunctionMode);
   #endif
   
   debugMsg = wxT("In ValidateCommand()");
   
   // Check if any Parameters need to be created
   StringArray names = cmd->GetWrapperObjectNameArray();
   
   #ifdef DEBUG_VALIDATE_COMMAND
   WriteStringArray(wxT("RefParameterNames for "), cmd->GetTypeName(), names);
   #endif
   
   // Create Parameters
   // Even in the function we still need to create automatic Parameters,
   // such sat.X in mySatX = sat.X in the assignment command, in order for Validator
   // to set wrapper reference for auto object used in the function command sequence
   // during the function initiaization. But we don't want to add to function's
   // automatic store at this time. It will be added during function initialization.
   #ifdef DEBUG_VALIDATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("   Calling CreateSystemParameter() for each ref. names\n"));
   #endif
   for (UnsignedInt i=0; i<names.size(); i++)
   {
      CreateSystemParameter(names[i]);
   }
   
   // If in function mode, just return true,
   // ValidateCommand() is called from GmatFunction::Initialize()
   if (inFunctionMode)
   {
      #ifdef DEBUG_VALIDATE_COMMAND
      MessageInterface::ShowMessage
         (wxT("Interpreter::ValidateCommand() in function mode, so just returning true\n"));
      #endif
      return true;
   }
   
   bool isValid = theValidator->ValidateCommand(cmd, continueOnError, 1);
   
   // Handle error messages here
   if (!isValid)
   {
      StringArray errList = theValidator->GetErrorList();
      for (UnsignedInt i=0; i<errList.size(); i++)
         HandleError(InterpreterException(errList[i]));
   }
   
   #ifdef DEBUG_VALIDATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("Interpreter::ValidateCommand() returning %d\n"), isValid);
   #endif
   
   return isValid;
   
} // ValidateCommand()


//------------------------------------------------------------------------------
// bool ValidateSubscriber(GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Checks the input subscriber to make sure it wrappers are set up for it
 * correctly, if necessary.
 *
 * @param <obj> the subscriber to validate
 */
//------------------------------------------------------------------------------
bool Interpreter::ValidateSubscriber(GmatBase *obj)
{
   if (obj == NULL)
      throw InterpreterException(wxT("The subscriber object to be validated is NULL"));
   
   // Now continue validation
   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("Interpreter::ValidateSubscriber() entered, obj=<%p><%s>\n"), obj,
       obj->GetName().c_str());
   #endif
   
   debugMsg = wxT("In ValidateSubscriber()");
   
   // This method can be called from other than Interpreter, so check if
   // object is SUBSCRIBER type
   //if (obj->GetType() != Gmat::SUBSCRIBER)
   if (!obj->IsOfType(Gmat::SUBSCRIBER))
   {
      InterpreterException ex
         (wxT("ElementWrapper for \"") + obj->GetName() + wxT("\" of type \"") +
          obj->GetTypeName() + wxT("\" cannot be created."));
      HandleError(ex);
      return false;
   }
   
   Subscriber *sub = (Subscriber*)obj;
   
   // We don't want to clear wrappers since Subscriber::ClearWrappers() changed to
   // also empty wrappers.  (LOJ: 2009.03.12)
   //sub->ClearWrappers();
   const StringArray wrapperNames = sub->GetWrapperObjectNameArray();
   
   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("In ValidateSubscriber, has %d wrapper names:\n"), wrapperNames.size());
   for (Integer ii=0; ii < (Integer) wrapperNames.size(); ii++)
      MessageInterface::ShowMessage(wxT("   %s\n"), wrapperNames[ii].c_str());
   #endif
   
   for (StringArray::const_iterator i = wrapperNames.begin();
        i != wrapperNames.end(); ++i)
   {
      try
      {
         ElementWrapper *ew = theValidator->CreateElementWrapper(*i, true);
         
         if (sub->SetElementWrapper(ew, *i) == false)
         {
            InterpreterException ex
               (wxT("ElementWrapper for \"") + (*i) +
                wxT("\" cannot be created for the Subscriber \"") + obj->GetName() + wxT("\""));
            HandleError(ex, false);
            return false;
         }
      }
      catch (BaseException &ex)
      {
         HandleError(ex);
         return false;
      }
   }
   
   return true;
   
} // ValidateSubscriber()


//---------------------------------
// protected
//---------------------------------

//------------------------------------------------------------------------------
// bool FindPropertyID(GmatBase *obj, const wxString &chunk, GmatBase **owner,
//                     Integer &id, Gmat::ParameterType &type)
//------------------------------------------------------------------------------
/*
 * Finds property ID for given property. If property not found in the obj,
 * it tries to find property from the owned objects.
 *
 * @param  obj    Object to find property
 * @param  chunk  String contains property
 * @param  owner  Address of new owner pointer to be returned
 * @param  id     Property ID to return (-1 if property not found)
 * @param  type   Property type to return
 *                (Gmat::UNKNOWN_ParameterType if property not found)
 *
 * @return true if property found
 *
 * For example, From wxT("FM.Gravity.Earth.Model")
 *   obj is FM pointer, chunk is wxT("Gravity.Earth.Model")
 */
//------------------------------------------------------------------------------
bool Interpreter::FindPropertyID(GmatBase *obj, const wxString &chunk,
                                 GmatBase **owner, Integer &id,
                                 Gmat::ParameterType &type)
{
   #ifdef DEBUG_FIND_PROP_ID
   MessageInterface::ShowMessage
      (wxT("Interpreter::FindPropertyID() obj=<%p><%s>, chunk=<%s>\n"), obj,
       (obj == NULL ? wxT("NULL") : obj->GetName().c_str()), chunk.c_str());
   #endif
   
   if (obj == NULL)
      return false;
   
   bool retval = false;
   StringArray parts = theTextParser.SeparateDots(chunk);
   Integer count = parts.size();
   wxString prop = parts[count-1];
   
   #ifdef DEBUG_FIND_PROP_ID
   MessageInterface::ShowMessage(wxT("   property=<%s>\n"), prop.c_str());
   #endif
   
   // Set initial output id and type
   id = -1;
   type = Gmat::UNKNOWN_PARAMETER_TYPE;
   
   try
   {
      id = obj->GetParameterID(prop);
      type = obj->GetParameterType(id);
      *owner = obj;
      retval = true;
   }
   catch (BaseException &)
   {
      // Owned objects are not configurable and they are local objects
      if (FindOwnedObject(obj, prop, owner, id, type))
      {
         retval = true;
      }
      else
      {
         // Bug 2445 fix
         // Check if it is property of associated objects, such as Hardware of Spacecraft.
         // Hardware objcts are configurable, but those are cloned before association. 
         // So that same Hardware can be associated with multiple Spacecraft.
         if (obj->IsOfType(Gmat::SPACECRAFT))
         {
            StringArray refObjNames = obj->GetRefObjectNameArray(Gmat::HARDWARE);
            #ifdef DEBUG_FIND_PROP_ID
            WriteStringArray(wxT("Hardware objects "), obj->GetName(), refObjNames);
            #endif
            
            GmatBase *refObj = NULL;
            for (UnsignedInt i = 0; i < refObjNames.size(); i++)
            {
               refObj = FindObject(refObjNames[i]);
               if (FindPropertyID(refObj, chunk, owner, id, type))
               {
                  retval = true;
                  break;
               }
            }
         }
      }
   }
   
   #ifdef DEBUG_FIND_PROP_ID
   MessageInterface::ShowMessage
      (wxT("Interpreter::FindPropertyID() returning owner=<%p><%s><%s>, retval=%d\n"),
       *owner, ((*owner) == NULL ? wxT("NULL") : (*owner)->GetTypeName().c_str()),
       ((*owner) == NULL ? wxT("NULL") : (*owner)->GetName().c_str()), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// GmatBase* FindObject(const wxString &name, const wxString &ofType = wxT(""))
//------------------------------------------------------------------------------
/**
 * Finds the object from the current object map.
 * (old method: Calls the Moderator to find a configured object.)
 *
 * @param  name    Name of the object.
 * @param  ofType  Type of object required; leave blank for no checking
 *
 * @return  object pointer found
 */
//------------------------------------------------------------------------------
GmatBase* Interpreter::FindObject(const wxString &name, 
                                  const wxString &ofType)
{
   return theValidator->FindObject(name, ofType);
}


//------------------------------------------------------------------------------
// bool IsCommandType(const wxString &type)
//------------------------------------------------------------------------------
/*
 * Returns true if input string is one of Command type that can be created.
 */
//------------------------------------------------------------------------------
bool Interpreter::IsCommandType(const wxString &type)
{
   if (find(commandList.begin(), commandList.end(), type) == commandList.end())
      return false;
   
   return true;
}


//------------------------------------------------------------------------------
// void ParseAndSetCommandName(GmatCommand *cmd, const wxString &cmdType, ...)
//------------------------------------------------------------------------------
/**
 * Parses command name from the command descriptoin, such as Propagate 'name' ...
 *
 * @param  cmd  Command pointer
 * @param  cmdType  Command type name
 * @param  desc  Original command input parameter
 * @param  newDesc  New command input parameter after command name removed
 * @return true if command name not found or command name found and enclosed
 *              with single quote; false otherwise
 */
//------------------------------------------------------------------------------
void Interpreter::ParseAndSetCommandName(GmatCommand *cmd, const wxString &cmdType,
                                         const wxString &desc, wxString &newDesc)
{
   #ifdef DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("ParseAndSetCommandName() entered, cmdType='%s', desc=<%s>\n"), cmdType.c_str(),
       desc.c_str());
   #endif
   if (desc.find(wxT("'")) != desc.npos)
   {
      if (desc[0] == '\'')
      {
         // if matching quote found, continue
         if (desc.find('\'', 1) != desc.npos)
         {
            StringArray parts = GmatStringUtil::SeparateBy(newDesc, wxT("'"));
            #ifdef DEBUG_CREATE_COMMAND
            WriteStringArray(wxT("   --->command parts"), wxT(""), parts);
            #endif
            
            //wxString cmdName = wxT("'") + parts[0] + wxT("'");
            wxString cmdName = parts[0];
            // Set command name
            if (parts.size() == 1)
            {
               cmd->SetName(cmdName);
               newDesc = wxT("");
            }
            else if (parts.size() >= 2)
            {
               newDesc = parts[1];
               cmd->SetName(cmdName);
            }
         }
         else
         {
            InterpreterException ex
               (wxT("Found invalid syntax for \"") + cmdType +
                wxT("\" command, possible missing single quote for the command name"));
            HandleError(ex);
         }
      }
   }
   #ifdef DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("ParseAndSetCommandName() leaving, newDesc=<%s>\n"), newDesc.c_str());
   #endif
}


//------------------------------------------------------------------------------
// GmatCommand* CreateCommand(const wxString &type, const wxString &desc)
//------------------------------------------------------------------------------
GmatCommand* Interpreter::CreateCommand(const wxString &type,
                                        const wxString &desc, bool &retFlag,
                                        GmatCommand *inCmd)
{
   #ifdef DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("Interpreter::CreateCommand() type=<%s>, inCmd=<%p>, \n   desc=<%s>\n"),
       type.c_str(), inCmd, desc.c_str());
   MessageInterface::ShowMessage
      (wxT("   inFunctionMode=%d, hasFunctionDefinition=%d\n"), inFunctionMode,
       hasFunctionDefinition);
   #endif
   
   GmatCommand *cmd = NULL;
   wxString type1 = type;
   wxString desc1 = desc;
   wxString cmdStr = type + wxT(" ") + desc;
   
   wxString realDesc; // Command description after name removed
   bool commandFound = false;
   
   // handle blank type
   if (type == wxT(""))
   {
      wxString::size_type index = desc.find(wxT("("));
      type1 = desc.substr(0, index);
   }
   
   if (IsCommandType(type1))
      commandFound = true;
   
   #ifdef DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("   type1='%s', commandFound=%d\n"), type1.c_str(), commandFound);
   #endif
   
   // Check for CallFunction
   if (type1[0] == '[')
   {
      #ifdef DEBUG_CREATE_COMMAND
      MessageInterface::ShowMessage
         (wxT("Interpreter::CreateCommand() detected [ and creating CallFunction ...\n"));
      #endif

      type1 = wxT("CallFunction");
      
      // Figure out if which CallFunction to be created.
      wxString funcName = GmatStringUtil::ParseFunctionName(desc);
      if (funcName != wxT(""))
      {
         GmatBase *func = FindObject(funcName);
         if (func != NULL && func->IsOfType(wxT("MatlabFunction")))
            type1 = wxT("CallMatlabFunction");
         else
            type1 = wxT("CallGmatFunction");
      }
      
      #ifdef DEBUG_CREATE_CALLFUNCTION
      MessageInterface::ShowMessage
         (wxT("   1 Now creating <%s> command and setting GenString to <%s>\n"),
          type1.c_str(), wxString(type1 + wxT(" ") + desc).c_str());
      #endif
      
      // Create CallFunction command and append to command sequence
      cmd = AppendCommand(type1, retFlag, inCmd);
      desc1 = type1 +  wxT("=") + desc;
      if (cmd != NULL)
         cmd->SetGeneratingString(desc1);
   }
   /// @TODO: This is a work around for a call function
   /// without any return parameters.  It should be updated in
   /// the design to handle this situation.
   else if ((desc1.find(wxT("=")) == desc1.npos) && (desc != wxT(""))
            && (!commandFound))
   {
      StringArray parts = theTextParser.SeparateSpaces(desc1);
      
      #ifdef DEBUG_CREATE_CALLFUNCTION
      WriteStringArray(wxT("Calling IsObjectType()"), wxT(""), parts);
      #endif
      
      if (IsObjectType(parts[0]))
      {
         InterpreterException ex(wxT("Found invalid command \"") + type1 + wxT("\""));
         HandleError(ex);
      }
      else if (!GmatStringUtil::IsValidName(type1 + desc, true))
      {
         InterpreterException ex
            (wxT("Found invalid function name \"") + type1 + desc + wxT("\""));
         HandleError(ex);
      }
      else
      {
         type1 = wxT("CallFunction");
         
         wxString funcName = GmatStringUtil::ParseFunctionName(desc);
         if (funcName != wxT(""))
         {
            GmatBase *func = FindObject(funcName);
            if (func != NULL && func->IsOfType(wxT("MatlabFunction")))
               type1 = wxT("CallMatlabFunction");
            else
               type1 = wxT("CallGmatFunction");
         }
         
         #ifdef DEBUG_CREATE_CALLFUNCTION
         MessageInterface::ShowMessage
            (wxT("   2 Now creating <%s> command and setting GenString to <%s>\n"),
             type1.c_str(), wxString(type1 + wxT(" ") + desc).c_str());
         #endif
         
         // Create command and append to command sequence
         cmd = AppendCommand(type1, retFlag, inCmd);
         desc1 = wxT("[] =") + type1 + desc;
         if (cmd != NULL)
            cmd->SetGeneratingString(desc1);
      }
   }
   else
   {
      if (type1 == wxT("CallFunction"))
      {
         wxString funcName = GmatStringUtil::ParseFunctionName(desc);
         
         #ifdef DEBUG_CREATE_CALLFUNCTION
         MessageInterface::ShowMessage(wxT("   funcName = '%s'\n"), funcName.c_str());
         #endif
         
         if (funcName != wxT(""))
         {
            GmatBase *funcPtr = FindObject(funcName);
            
            #ifdef DEBUG_CREATE_CALLFUNCTION
            MessageInterface::ShowMessage(wxT("   funcPtr=<%p>\n"), funcPtr);
            MessageInterface::ShowMessage
               (wxT("   matlabFunctionNames.size()=%d\n"), matlabFunctionNames.size());
            for (UnsignedInt ii = 0; ii < matlabFunctionNames.size(); ii++)
               MessageInterface::ShowMessage
                  (wxT("      '%s'\n"), matlabFunctionNames[ii].c_str());
            #endif
            
            // If function name found in matlabFunctionNames, create
            // CallMatlabFunction (LOJ: Bug 1967 fix)
            if (find(matlabFunctionNames.begin(), matlabFunctionNames.end(),
                     funcName) != matlabFunctionNames.end())
            {
               type1 = wxT("CallMatlabFunction");
            }
            else
            {
               if (funcPtr != NULL && funcPtr->IsOfType(wxT("MatlabFunction")))
                  type1 = wxT("CallMatlabFunction");
               else
                  type1 = wxT("CallGmatFunction");
            }
         }
      }
      
      // How do we detect MatlabFunction inside a GmatFunction?
      if (desc.find(wxT("MatlabFunction")) != desc.npos)
      {
         StringArray parts = GmatStringUtil::SeparateBy(desc, wxT(" "));
         if (parts.size() == 2)
         {
            #ifdef DEBUG_CREATE_CALLFUNCTION
            MessageInterface::ShowMessage
               (wxT("   Adding '%s' to matlabFunctionNames\n"), parts[1].c_str());
            #endif
            matlabFunctionNames.push_back(parts[1]);
         }
      }
      
//       #if defined (DEBUG_CREATE_COMMAND) || defined (DEBUG_CREATE_CALLFUNCTION)      
//       MessageInterface::ShowMessage
//          (wxT("   3 Now creating <%s> command and setting GenString to <%s>\n"),
//           type1.c_str(), wxString(type1 + wxT(" ") + desc).c_str());
//       #endif
      #if defined (DEBUG_CREATE_COMMAND) || defined (DEBUG_CREATE_CALLFUNCTION)      
      MessageInterface::ShowMessage(wxT("   3 Now creating <%s> command\n"), type1.c_str());
      #endif
      
      // Create a command and append to command sequence
      cmd = AppendCommand(type1, retFlag, inCmd);
      realDesc = desc;
      
      // If command is not call function, parse command name
      if (cmd != NULL && !cmd->IsOfType(wxT("CallFunction")))
         ParseAndSetCommandName(cmd, type1, desc, realDesc);
      
      #if defined (DEBUG_CREATE_COMMAND) || defined (DEBUG_CREATE_CALLFUNCTION)      
      MessageInterface::ShowMessage
         (wxT("   Setting GenString to <%s>\n"), wxString(type1 + wxT(" ") + realDesc).c_str());
      #endif
      cmd->SetGeneratingString(type1 + wxT(" ") + realDesc);
   }
   
   if (cmd == NULL)
   {
      retFlag = false;
      #ifdef DEBUG_CREATE_COMMAND
      MessageInterface::ShowMessage
         (wxT("CreateCommand() returning NULL for '%s', retFlag=%d\n"), type1.c_str(),
          retFlag);
      #endif
      return NULL;
   }
   
   #ifdef DEBUG_CREATE_COMMAND
   if (inCmd == NULL)
      MessageInterface::ShowMessage
         (wxT("   => '%s' created.\n"), cmd->GetTypeName().c_str());
   else
      MessageInterface::ShowMessage
         (wxT("   => '%s' created and appended to '%s'.\n"),
          cmd->GetTypeName().c_str(), inCmd->GetTypeName().c_str());
   MessageInterface::ShowMessage
      (wxT("   desc     = <%s>\n     desc1    = <%s>\n     realDesc = <%s>\n"),
       desc.c_str(), desc1.c_str(), realDesc.c_str());
   #endif
   
   // Now assemble command
   try
   {
      #ifdef DEBUG_CREATE_COMMAND
      MessageInterface::ShowMessage
         (wxT("   => Now calling %s->InterpretAction()\n"), type1.c_str());
      #endif
      
      // Set current function to command 
      cmd->SetCurrentFunction(currentFunction);
      
      // if command has its own InterpretAction(), just return cmd
      if (cmd->InterpretAction())
      {
         // if command is Assignment, check if GmatFunction needs to be created
         if (type1 == wxT("GMAT") && ((Assignment*)cmd)->GetMathTree() != NULL)
            HandleMathTree(cmd);
         
         #ifdef DEBUG_CREATE_COMMAND
         MessageInterface::ShowMessage(wxT("   => Now calling ValidateCommand()\n"));
         #endif
         retFlag  = ValidateCommand(cmd);
         
         #ifdef DEBUG_CREATE_COMMAND
         MessageInterface::ShowMessage(wxT("   ===> %s has own InterpretAction()\n"), type1.c_str());
         MessageInterface::ShowMessage
            (wxT("CreateCommand() leaving creating '%s', cmd=<%p>, retFlag=%d\n"), type1.c_str(),
             cmd, retFlag);
         #endif
         return cmd;
      }
      else
      {
         #ifdef DEBUG_CREATE_COMMAND
         MessageInterface::ShowMessage
            (wxT("   ===> %s does not have own InterpretAction()\n"), type1.c_str());
         #endif
      }
   }
   catch (BaseException &e)
   {
      HandleError(e);
      retFlag = false;
      
      #ifdef DEBUG_CREATE_COMMAND
      MessageInterface::ShowMessage
         (wxT("CreateCommand() leaving creating '%s', cmd=<%p>, retFlag=%d\n"), type1.c_str(),
          cmd, retFlag);
      #endif
      
      // Return cmd since command already created
      return cmd;
   }
   
   
   // Assemble commands those don't have InterpretAction()
   if (realDesc != wxT(""))
   {
      bool retval3 = true;
      bool retval1  = AssembleCommand(cmd, realDesc);
      
      if (retval1)
         retval3 = ValidateCommand(cmd);
      else
      {
         InterpreterException ex(wxT("Failed to parse ") + cmdStr);
         HandleError(ex);
      }
      
      retFlag = retval1 && retval3;
      
   }
   else
   {
      #ifdef DEBUG_CREATE_COMMAND
      MessageInterface::ShowMessage(wxT("   There is no command descriptions to assemble\n"));
      #endif
   }
   
   #ifdef DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("CreateCommand() leaving creating '%s', cmd=<%p>, retFlag=%d\n"), type1.c_str(),
       cmd, retFlag);
   #endif
   
   return cmd;;
}


//------------------------------------------------------------------------------
//GmatCommand* AppendCommand(const wxString &type, bool &retFlag,
//                           GmatCommand *inCmd)
//------------------------------------------------------------------------------
GmatCommand* Interpreter::AppendCommand(const wxString &type, bool &retFlag,
                                        GmatCommand *inCmd)
{
   #ifdef DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("AppendCommand() type=<%s>, inCmd=<%p>\n"), type.c_str(), inCmd);
   #endif
   
   GmatCommand *cmd = NULL;
   
   if (inCmd == NULL)
   {
      cmd = theModerator->AppendCommand(type, wxT(""), retFlag);
      
      #ifdef DEBUG_CREATE_COMMAND
      MessageInterface::ShowMessage
         (wxT("===> Appending command <%s> to the last command\n"),
          cmd->GetTypeName().c_str());
      #endif
   }
   else
   {
      cmd = theModerator->CreateCommand(type, wxT(""), retFlag);
      inCmd->Append(cmd);
      
      #ifdef DEBUG_CREATE_COMMAND
      MessageInterface::ShowMessage
         (wxT("===> Appending command <%s> to <%s>\n"), cmd->GetTypeName().c_str(),
          inCmd->GetTypeName().c_str());
      #endif
   }
   
   #ifdef DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("AppendCommand() returning <%p>, retFlag=%d\n"), cmd, retFlag);
   #endif
   
   return cmd;
}


//------------------------------------------------------------------------------
//bool AssembleCommand(GmatCommand *cmd, const wxString &desc)
//------------------------------------------------------------------------------
bool Interpreter::AssembleCommand(GmatCommand *cmd, const wxString &desc)
{
   bool retval = false;
   wxString type = cmd->GetTypeName();
   
   #ifdef DEBUG_ASSEMBLE_COMMAND
   MessageInterface::ShowMessage
      (wxT("Interpreter::AssembleCommand() cmd='%s'\n   desc=<%s>\n"),
       type.c_str(), desc.c_str());
   #endif
   
   if (cmd->IsOfType(wxT("For")))
      retval = AssembleForCommand(cmd, desc);
   else if (cmd->IsOfType(wxT("CallFunction")))
      retval = AssembleCallFunctionCommand(cmd, desc);
   else if (cmd->IsOfType(wxT("ConditionalBranch")))
      retval = AssembleConditionalCommand(cmd, desc);
   else
      retval = AssembleGeneralCommand(cmd, desc);
   
   #ifdef DEBUG_ASSEMBLE_COMMAND
   MessageInterface::ShowMessage
      (wxT("AssembleCommand() leaving assembling %s, retval=%d\n"),
       type.c_str(), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
//bool AssembleCallFunctionCommand(GmatCommand *cmd, const wxString &desc)
//------------------------------------------------------------------------------
bool Interpreter::AssembleCallFunctionCommand(GmatCommand *cmd,
                                              const wxString &desc)
{
   #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
   MessageInterface::ShowMessage
      (wxT("Interpreter::AssembleCallFunctionCommand() cmd='%s'\n   desc=<%s>\n"),
       cmd->GetTypeName().c_str(), desc.c_str());
   #endif
   
   debugMsg = wxT("In AssembleCallFunctionCommand()");
   bool retval = true;
   
   // Output
   wxString::size_type index1 = 0;
   wxString lhs;
   StringArray outArray;
   
   // get output arguments if there was an equal sign
   if (GmatStringUtil::IsThereEqualSign(desc))
   {
      index1 = desc.find(wxT("="));
      lhs = desc.substr(0, index1);
      outArray = theTextParser.SeparateBrackets(lhs, wxT("[]"), wxT(" ,"), true);
      index1 = index1 + 1;
   }
   
   // Function Name, Input
   StringArray inArray;
   wxString funcName;
   wxString::size_type index2 = desc.find(wxT("("), index1);
   
   #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   Starting index=%u, open parenthesis index=%u\n"), index1, index2);
   #endif
   
   if (index2 == desc.npos)
   {      
      funcName = desc.substr(index1);
   }
   else
   {
      funcName = desc.substr(index1, index2-index1);
      wxString rhs = desc.substr(index2);
      rhs = GmatStringUtil::RemoveOuterString(rhs, wxT("("), wxT(")"));
      
      #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
      MessageInterface::ShowMessage(wxT("   rhs=\"%s\"\n"), rhs.c_str());
      #endif
      
      // check if single quote found
      inArray = GmatStringUtil::SeparateByComma(rhs);
      
      #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
      MessageInterface::ShowMessage(wxT("   inArray.size()=%d\n"), inArray.size());
      #endif
   }
   
   funcName = GmatStringUtil::Trim(funcName);
   
   #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   Checking function name '%s'\n"), funcName.c_str());
   #endif
   
   // Check for blank name
   if (funcName == wxT(""))
   {
      InterpreterException ex(wxT("Found blank function name"));
      HandleError(ex);
      return false;
   }
   
   // Check for valid name
   if (!GmatStringUtil::IsValidName(funcName))
   {
      InterpreterException ex(wxT("Found invalid function name \"") + funcName + wxT("\""));
      HandleError(ex);
      return false;
   }
   
   #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
   MessageInterface::ShowMessage(wxT("   Setting funcName '%s'\n"), funcName.c_str());
   #endif
   
   // Special case for MatlabFunction
   // If in functin mode and function name is found from tempObjectNames,
   // add an extension
   wxString newFuncName = funcName;
   
   if (inFunctionMode)
   {
      if (find(tempObjectNames.begin(), tempObjectNames.end(), funcName) !=
          tempObjectNames.end())
      {
         GmatGlobal *global = GmatGlobal::Instance();
         newFuncName = funcName + global->GetMatlabFuncNameExt();
         
         #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
         MessageInterface::ShowMessage
            (wxT("   '%s' found in tempObjectNames, so setting '%s' as function ")
             wxT("name\n"), funcName.c_str(), newFuncName.c_str());
         #endif
      }
   }
   
   // Set function name to CallFunction
   retval = cmd->SetStringParameter(wxT("FunctionName"), newFuncName);
   
   #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
   MessageInterface::ShowMessage(wxT("   Setting input\n"));
   WriteStringArray(wxT("CallFunction Input"), wxT(""), inArray);
   #endif
   
   // Set input to CallFunction
   bool validInput = false;
   Real rval;
   
   if (inArray.size() == 0) //if no inputs, set validInput to true
      validInput = true;
   
   for (UnsignedInt i=0; i<inArray.size(); i++)
   {            
      // If input is single item, set it to CallFunction, otherwise set wxT("") (loj: 2008.08.22)
      // Should we do this here? Just hold off for now
      //==============================================================
      // The old way
      //==============================================================
      #if 0
      retval = cmd->SetStringParameter(wxT("AddInput"), inArray[i]);
      #endif
      //==============================================================
      
      
      //==============================================================
      // The new way
      //==============================================================
      #if 1
      wxString input = inArray[i];
      if (GmatStringUtil::IsEnclosedWith(inArray[i], wxT("'")))
         retval = cmd->SetStringParameter(wxT("AddInput"), input);
      else
      {
         StringArray varNames = GmatStringUtil::GetVarNames(input);
         if (varNames.size() > 1)
            input = wxT("");
         #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
         MessageInterface::ShowMessage
            (wxT("   Setting <%s> as input to CallFunction\n"), input.c_str());
         #endif      
         retval = cmd->SetStringParameter(wxT("AddInput"), input);
      }
      #endif
      //==============================================================
      
      // Check for valid input parameter
      validInput = false;
      
      // Check for number before object property.
      // This fixes Bug1903 (Failed to pass number and literal to a function)
      
      // String literal
      if (GmatStringUtil::IsEnclosedWith(inArray[i], wxT("'")))
      {
         validInput = true;
      }
      // Number
      else if (GmatStringUtil::ToReal(inArray[i], rval))
      {
         validInput = true;
      }
      // Parameter or object property
      else if (inArray[i].find('.') != wxString::npos)
      {
         // if input parameter is a system Parameter then create
         if (IsParameterType(inArray[i]))
         {
            Parameter *param = CreateSystemParameter(inArray[i]);
            if (param != NULL)
               validInput = true;
         }
      }
      // Whole object
      else
      {
         GmatBase *obj = FindObject(inArray[i]);
         if (obj != NULL)
            validInput = true;
      }
      
      #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
      MessageInterface::ShowMessage
         (wxT("   <%s> is %svalid input\n"), inArray[i].c_str(), validInput ? wxT("") : wxT("not "));
      #endif
      
      // if in function mode, ignore invalid parameter
      if (inFunctionMode)
         validInput = true;
      
      // if not in function mode, throw exception if invalid parameter
      if (!validInput)
      {
         InterpreterException ex
            (wxT("Nonexistent or disallowed CallFunction Input Parameter: \"") +
             inArray[i] +  wxT("\""));
         HandleError(ex);
         return false;
      }
   }
   
   if (!retval || !validInput)
   {
      #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
      MessageInterface::ShowMessage
         (wxT("Interpreter::AssembleCallFunctionCommand() returning false, ")
          wxT("retval=%d, validInput=%d\n"), retval, validInput);
      #endif
      return false;
   }
   
   // Set output to CallFunction
   #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
   MessageInterface::ShowMessage(wxT("   Setting output\n"));
   WriteStringArray(wxT("CallFunction Output"), wxT(""), outArray);
   #endif
   
   for (UnsignedInt i=0; i<outArray.size(); i++)
      retval = cmd->SetStringParameter(wxT("AddOutput"), outArray[i]);
   
   // if in function mode, just return retval
   if (inFunctionMode)
   {
      #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
      MessageInterface::ShowMessage
         (wxT("Interpreter::AssembleCallFunctionCommand() returning %d, it's in ")
          wxT("function mode\n"), retval);
      #endif
      return retval;
   }
   
   // See if Function is MatlabFunction since all MatlabFunctions are created
   // before mission sequence, if not, create as GmatFunction.
   GmatBase *func = FindObject(funcName);
   if (func == NULL)
      func = CreateObject(wxT("GmatFunction"), funcName);
   
   // Set function pointer to CallFunction command
   cmd->SetRefObject(func, Gmat::FUNCTION, funcName);
   
   #ifdef DEBUG_ASSEMBLE_CALL_FUNCTION
   MessageInterface::ShowMessage
      (wxT("Interpreter::AssembleCallFunctionCommand() returning %d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
//bool AssembleConditionalCommand(GmatCommand *cmd, const wxString &desc)
//------------------------------------------------------------------------------
bool Interpreter::AssembleConditionalCommand(GmatCommand *cmd,
                                             const wxString &desc)
{
   #ifdef DEBUG_ASSEMBLE_CONDITION
   MessageInterface::ShowMessage
      (wxT("Interpreter::AssembleConditionalCommand() cmd=<%p>'%s', desc='%s'\n"),
       cmd, cmd->GetTypeName().c_str(), desc.c_str());
   #endif
   
   debugMsg = wxT("In AssembleConditionalCommand()");
   bool retval = true;
   wxString type = cmd->GetTypeName();
   wxString opStr = wxT("~<=>&|");
   
   // conditional commands, for compatibility with MATLAB, should not have
   // parentheses (except to indicate array elements), brackets, or braces
   if (!GmatStringUtil::HasNoBrackets(desc))
   {
      wxString msg = 
         wxT("A conditional command is not allowed to contain brackets, braces, or ")
         wxT("parentheses (except to indicate an array element)");
      InterpreterException ex(msg);
      HandleError(ex);
      return false;
   }
   
   // This really becomes moot ...  wcs 2007.09.12
   // Remove enclosed parenthesis first
   Integer length = desc.size();
   wxString str1 = desc;
   if (desc[0] == '(' && desc[length-1] == ')')
   {
      str1 = desc.substr(1, length-2);
   }
   else
   {
      if (!GmatStringUtil::IsParenBalanced(desc))
      {
         InterpreterException ex(wxT("The Command has unbalanced parentheses"));
         HandleError(ex);
         return false;
      }
   }
   
   wxString::size_type start = 0;
   wxString::size_type right = 0;
   wxString::size_type op = 0;
   bool done = false;
   StringArray parts;
   wxString str2;
   
   // Parse conditions
   while (!done)
   {
      op = str1.find_first_of(opStr, start);
      if (op == str1.npos)
      {
         // Add final right of operator, if not blank
         str2 = GmatStringUtil::Trim(str1.substr(start));
         if (str2 != wxT(""))
            parts.push_back(str2);
         break;
      }
      
      // Add left of operator
      str2 = GmatStringUtil::Trim(str1.substr(start, op-start));
      parts.push_back(str2);
      
      // Add operator
      right = str1.find_first_not_of(opStr, op);      
      str2 = GmatStringUtil::Trim(str1.substr(op, right-op));
      parts.push_back(str2);
      
      start = op + 1;
      op = str1.find_first_of(opStr, start);
      
      // check for double ops (such as: == ~= >= <=)
      if (op != str1.npos && op == start)
         start = op + 1;
   }
   
   #ifdef DEBUG_ASSEMBLE_CONDITION
   WriteStringArray(wxT("After parsing conditions()"), wxT(""), parts);
   #endif
   
   Integer count = parts.size();
   for (Integer ii = 0; ii < count; ii++)
   {
      if (GmatStringUtil::IsBlank(parts.at(ii)))
      {
         InterpreterException ex(wxT("Missing field or operator in command"));
         HandleError(ex);
         return false;
      }
      wxString strUpper = GmatStringUtil::ToUpper(parts.at(ii));         
      if (strUpper.find(wxT(" OR ")) != strUpper.npos)
      {
         InterpreterException ex(wxT("\"OR\" is not a valid relational operator"));
         HandleError(ex);
         return false;
      }
      if (strUpper.find(wxT(" AND ")) != strUpper.npos)
      {
         InterpreterException ex(wxT("\"AND\" is not a valid relational operator"));
         HandleError(ex);
         return false;
      }
   }
   
   // assuming there is no boolean argument
   if (count < 3 || ((count-3)%4) != 0)
   {
      InterpreterException ex(wxT("The Command has an invalid number of conditions"));
      HandleError(ex);
      return false;
   }
   
   // Added try/catch block so that function name can be added to the error message
   try
   {
      ConditionalBranch *cb = (ConditionalBranch*)cmd;
      
      for (int i=0; i<count; i+=4)
      {
         #ifdef DEBUG_ASSEMBLE_CONDITION
         MessageInterface::ShowMessage
            (wxT("   lhs:<%s>, op:<%s>, rhs:<%s>\n"), parts[i].c_str(), parts[i+1].c_str(),
             parts[i+2].c_str());
         #endif
         
         // Try to create a parameter first if system parameter
         wxString type, ownerName, depObj;
         GmatStringUtil::ParseParameter(parts[i], type, ownerName, depObj);
         #ifdef DEBUG_ASSEMBLE_CONDITION // ---------------------------- debug ----
         MessageInterface::ShowMessage
            (wxT("   lhs: type = %s, ownerName = %s, depObj = %s\n"), 
             type.c_str(), ownerName.c_str(), depObj.c_str());
         #endif // ------------------------------------------------- end debug ----
         
         // Create Parameter if not in function, since Parameters are automatically
         // created in ValidateCommand
         if (!inFunctionMode)
         {
            if (theModerator->IsParameter(type))
               CreateParameter(type, parts[i], ownerName, depObj);
         }
         
         GmatStringUtil::ParseParameter(parts[i+2], type, ownerName, depObj);
         #ifdef DEBUG_ASSEMBLE_CONDITION // ---------------------------- debug ----
         MessageInterface::ShowMessage
            (wxT("   rhs: type = %s, ownerName = %s, depObj = %s\n"), 
             type.c_str(), ownerName.c_str(), depObj.c_str());
         #endif // ------------------------------------------------- end debug ----
         
         // Create Parameter if not in function, since Parameters are automatically
         // created in ValidateCommand
         if (!inFunctionMode)
         {
            if (theModerator->IsParameter(type))
               CreateParameter(type, parts[i+2], ownerName, depObj);
         }
         
         cb->SetCondition(parts[i], parts[i+1], parts[i+2]);
         
         if (count > i+3)
         {
            #ifdef DEBUG_ASSEMBLE_CONDITION
            MessageInterface::ShowMessage(wxT("   logOp=<%s>\n"), parts[i+3].c_str());
            #endif
            
            cb->SetConditionOperator(parts[i+3]);
         }
      }
   }
   catch (BaseException &e)
   {
      InterpreterException ex(e.GetFullMessage());
      HandleError(ex);
      return false;
   }
   
   return retval;
}


//------------------------------------------------------------------------------
//bool AssembleForCommand(GmatCommand *cmd, const wxString &desc)
//------------------------------------------------------------------------------
/* Parses For loop control expression
 *    It's syntax is 
 *       For index = start:increment:end
 */
//------------------------------------------------------------------------------
bool Interpreter::AssembleForCommand(GmatCommand *cmd, const wxString &desc)
{
   #ifdef DEBUG_ASSEMBLE_FOR
   MessageInterface::ShowMessage
      (wxT("Interpreter::AssembleForCommand() desc=<%s>\n"), desc.c_str());
   #endif
   
   debugMsg = wxT("In AssembleForCommand()");
   
   // For loop commands, for compatability with MATLAB, should not have
   // parentheses (except to indicate array elements), brackets, or braces
   if (!GmatStringUtil::HasNoBrackets(desc))
   {
      wxString msg = 
         wxT("A For command is not allowed to contain brackets, braces, or ")
         wxT("parentheses (except to indicate an array element)");
      InterpreterException ex(msg);
      HandleError(ex);
      return false;
   }
   
   bool retval = true;
   wxString::size_type equalSign = desc.find(wxT("="));
   
   if (equalSign == desc.npos)
   {
      InterpreterException ex(wxT("Cannot find equal sign (=) for For loop control"));
      HandleError(ex);
      return false;
   }
   
   wxString index = desc.substr(0, equalSign);
   index = GmatStringUtil::Trim(index);
   
   wxString substr = desc.substr(equalSign+1);
   if (substr.find(':') == substr.npos)
   {
      InterpreterException ex(wxT("Missing colon (:) for For loop control"));
      HandleError(ex);
      return false;
   }
   
   StringArray parts = theTextParser.SeparateBy(substr, wxT(":"));
   int count = parts.size();
   Integer numColons = 0;
   for (unsigned int ii = 0; ii < substr.size(); ii++)
      if (substr.at(ii) == ':') numColons++;
   if (numColons >= (Integer) count)
   {
      InterpreterException ex(wxT("Too many colons (:) for For loop control"));
      HandleError(ex);
      return false;
   }
   #ifdef DEBUG_ASSEMBLE_FOR
   MessageInterface::ShowMessage
      (wxT("Interpreter::AssembleForCommand() After SeparateBy, parts = \n"));
   for (Integer ii=0;ii<count;ii++)
      MessageInterface::ShowMessage(wxT("   <%s>\n"), parts[ii].c_str());
   #endif
   
   if (count < 2)
   {
      InterpreterException ex(wxT("Missing field, colon (:), or equal sign (=) for For loop control"));
      HandleError(ex);
      return false;
   }
   
   wxString start = parts[0];
   wxString end = parts[1];
   wxString step = wxT("1");
   
   if (count > 2)
   {
      step = parts[1];
      end = parts[2];
   }
   
   
   #ifdef DEBUG_ASSEMBLE_FOR
   MessageInterface::ShowMessage
      (wxT("Interpreter::AssembleForCommand() index=<%s>, start=<%s>, end=<%s>, ")
       wxT("step=<%s>\n"), index.c_str(), start.c_str(), end.c_str(), step.c_str());
   #endif
   
   cmd->SetStringParameter(wxT("IndexName"), index);
   cmd->SetStringParameter(wxT("StartName"), start);
   cmd->SetStringParameter(wxT("EndName"), end);
   cmd->SetStringParameter(wxT("IncrementName"), step);
   
   #ifdef DEBUG_ASSEMBLE_FOR
   MessageInterface::ShowMessage
      (wxT("Interpreter::AssembleForCommand() returning %d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
//bool AssembleGeneralCommand(GmatCommand *cmd, const wxString &desc)
//------------------------------------------------------------------------------
bool Interpreter::AssembleGeneralCommand(GmatCommand *cmd,
                                         const wxString &desc)
{
   bool retval = true;
   wxString type = cmd->GetTypeName();
   
   #ifdef DEBUG_ASSEMBLE_COMMAND
   MessageInterface::ShowMessage
      (wxT("AssembleGeneralCommand() cmd='%s', desc=<%s>\n"), cmd->GetTypeName().c_str(),
       desc.c_str());
   #endif
   
   if (type == wxT("Target") || type == wxT("Report") || type == wxT("BeginFiniteBurn") ||
       type == wxT("EndFiniteBurn") || type == wxT("Optimize"))
   {
      // first item is ref. object name
      
      if (type == wxT("Target"))
         retval = AssembleTargetCommand(cmd, desc);
      else if (type == wxT("Optimize"))
         retval = AssembleOptimizeCommand(cmd, desc);
      else if (type == wxT("Report"))
         retval = AssembleReportCommand(cmd, desc);
      else
         retval = AssembleFiniteBurnCommand(cmd, desc);
   }
   else if (type == wxT("Create"))
      retval = AssembleCreateCommand(cmd, desc);
   else if (type == wxT("Save") || type == wxT("Global"))
      retval = SetCommandRefObjects(cmd, desc);
   else
      retval = false;
   
   #ifdef DEBUG_ASSEMBLE_COMMAND
   MessageInterface::ShowMessage
      (wxT("AssembleGeneralCommand() leaving assemblilng %s, retval=%d\n"),
       type.c_str(), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool AssembleTargetCommand(GmatCommand *cmd, const wxString &desc)
//------------------------------------------------------------------------------
bool Interpreter::AssembleTargetCommand(GmatCommand *cmd, const wxString &desc)
{
   debugMsg = wxT("In AssembleTargetCommand()");
   
   // This command, for compatability with MATLAB, should not have
   // parentheses (except to indicate array elements), brackets, or braces
   if (!GmatStringUtil::HasNoBrackets(desc, false))
   {
      wxString msg = 
         wxT("The Target command is not allowed to contain brackets, braces, or ")
         wxT("parentheses");
      InterpreterException ex(msg);
      HandleError(ex);
      return false;
   }
   
   bool retval = true;
   StringArray parts = theTextParser.Decompose(desc, wxT("()"));
   cmd->SetRefObjectName(Gmat::SOLVER, parts[0]);
   
   // Make sure there is only one thing on the line
   if (parts.size() > 1)
   {
      InterpreterException ex
         (wxT("Unexpected text at end of Target command"));
      HandleError(ex);
      retval = false;
   }
   
   // Check if the Solver exist if not in Function mode
   if (!inFunctionMode)
   {
      GmatBase *obj = FindObject(parts[0], wxT("Solver"));
      if (obj == NULL)
      {
         InterpreterException ex
            (wxT("Cannot find the Solver \"") + parts[0] + wxT("\""));
         HandleError(ex);
         retval = false;
      }
   }
   
   return retval;
}


//------------------------------------------------------------------------------
// bool AssembleOptimizeCommand(GmatCommand *cmd, const wxString &desc)
//------------------------------------------------------------------------------
bool Interpreter::AssembleOptimizeCommand(GmatCommand *cmd, const wxString &desc)
{
   debugMsg = wxT("In AssembleOptimizeCommand()");
   
   // This command, for compatability with MATLAB, should not have
   // parentheses (except to indicate array elements), brackets, or braces
   if (!GmatStringUtil::HasNoBrackets(desc, false))
   {
      wxString msg = 
         wxT("The Optimize command is not allowed to contain brackets, braces, or ")
         wxT("parentheses");
      InterpreterException ex(msg);
      HandleError(ex);
      return false;
   }
   
   bool retval = true;
   StringArray parts = theTextParser.Decompose(desc, wxT("()"));
   cmd->SetRefObjectName(Gmat::SOLVER, parts[0]);
   
   // Make sure there is only one thing on the line
   if (parts.size() > 1)
   {
      InterpreterException ex
         (wxT("Unexpected text at end of Optimize command"));
      HandleError(ex);
      retval = false;
   }
   
   // Check if the Solver exist if not in Function mode
   if (!inFunctionMode)
   {
      GmatBase *obj = FindObject(parts[0], wxT("Solver"));
      if (obj == NULL)
      {
         InterpreterException ex
            (wxT("Cannot find the Solver \"") + parts[0] + wxT("\""));
         HandleError(ex);
         retval = false;
      }
   }
   
   return retval;
}


//------------------------------------------------------------------------------
// bool AssembleFiniteBurnCommand(GmatCommand *cmd, const wxString &desc)
//------------------------------------------------------------------------------
bool Interpreter::AssembleFiniteBurnCommand(GmatCommand *cmd, const wxString &desc)
{
   #ifdef DEBUG_ASSEMBLE_COMMAND
   MessageInterface::ShowMessage(wxT("Begin/EndFiniteBurn being processed ...\n"));
   #endif
   
   bool retval = true;
   debugMsg = wxT("In AssembleFiniteBurnCommand()");
   
   // Note:
   // Begin/EndFiniteBurn has the syntax: BeginFiniteBurn burn1(sat1 sat2)
   // First, check for errors in brackets
   if ((desc.find(wxT("[")) != desc.npos) || (desc.find(wxT("]")) != desc.npos))
   {
      InterpreterException ex
         (wxT("Brackets not allowed in ") + cmd->GetTypeName()+ wxT(" command"));
      HandleError(ex);
      retval = false;
   }
   
   if (!GmatStringUtil::AreAllBracketsBalanced(desc, wxT("({)}")))
   {
      InterpreterException ex
         (wxT("Parentheses, braces, or brackets are unbalanced or incorrectly placed"));
      HandleError(ex);
      retval = false;
   }
   
   // Get FiniteBurn name
   StringArray parts = theTextParser.Decompose(desc, wxT("()"), false);
   
   #ifdef DEBUG_ASSEMBLE_COMMAND
   wxString type = cmd->GetTypeName();
   WriteStringArray(type, wxT(""), parts);
   #endif
   
   if (parts.size() < 2)
   {
      InterpreterException ex
         (wxT("Missing ") + cmd->GetTypeName() + wxT(" parameter. Expecting ")
          wxT("\"FiniteBurnName(SpacecraftName)\""));
      HandleError(ex);
      retval = false;
   }
   else
   {
      cmd->SetRefObjectName(Gmat::FINITE_BURN, parts[0]);
      
      // Get Spacecraft names
      StringArray subParts = theTextParser.SeparateBrackets(parts[1], wxT("()"), wxT(","));
      
      #ifdef DEBUG_ASSEMBLE_COMMAND
      WriteStringArray(type, wxT(""), subParts);
      #endif
      
      Integer count = subParts.size();
      if (count == 0)
      {
         InterpreterException ex
            (cmd->GetTypeName() + wxT(" command must contain at least one spacecraft name"));
         HandleError(ex);
         retval = false;
      }
      Integer numCommas = GmatStringUtil::NumberOfOccurrences(parts[1],',');
      if (count != (numCommas + 1))
      {
         InterpreterException ex
            (wxT("Missing spacecraft name in ") + cmd->GetTypeName() + wxT(" command"));
         HandleError(ex);
         retval = false;
      }
      for (int i=0; i<count; i++)
      {
         if (GmatStringUtil::IsBlank(subParts[i]))
         {
            InterpreterException ex
               (wxT("Missing spacecraft name in ") + cmd->GetTypeName() + wxT(" command"));
            HandleError(ex);
            retval = false;
         }
         cmd->SetRefObjectName(Gmat::SPACECRAFT, subParts[i]);
      }
   }
   
   return retval;
}


//------------------------------------------------------------------------------
// bool AssembleReportCommand(GmatCommand *cmd, const wxString &desc)
//------------------------------------------------------------------------------
bool Interpreter::AssembleReportCommand(GmatCommand *cmd, const wxString &desc)
{
   #ifdef DEBUG_ASSEMBLE_REPORT_COMMAND
   MessageInterface::ShowMessage
      (wxT("AssembleReportCommand() cmd='%s', desc=<%s>\n"), cmd->GetTypeName().c_str(),
       desc.c_str());
   #endif
   
   debugMsg = wxT("In AssembleReportCommand()");
   bool retval = true;
   
   // This command, for compatability with MATLAB, should not have
   // parentheses (except to indicate array elements), brackets, or braces
   if (!GmatStringUtil::HasNoBrackets(desc, true))
   {
      wxString msg = 
         wxT("The Report command is not allowed to contain brackets, braces, or ")
         wxT("parentheses (except to indicate array elements)");
      InterpreterException ex(msg);
      HandleError(ex);
      return false;
   }
   
   // we only want to separate by spaces - commas are not allowed, 
   // not even in arrays (for this command)
   StringArray parts = GmatStringUtil::SeparateBy(desc, wxT(" "), true);
   Integer count = parts.size();
   
   #ifdef DEBUG_ASSEMBLE_REPORT_COMMAND 
   WriteStringArray(wxT("Parsing Report"), wxT(""), parts);
   #endif
   
   // checking items to report
   if (count < 2)
   {
      InterpreterException ex (wxT("There are no ReportFile or items to Report"));
      HandleError(ex);
      return false;
   }
   
   // Set ReportFile name
   cmd->SetStringParameter(wxT("ReportFile"), parts[0]);
   
   // Set reporting Parameter names
   for (int i=1; i<count; i++)
      cmd->SetStringParameter(wxT("Add"), parts[i]);
   
   GmatBase *obj = NULL;
   
   // See if we can set ReportFile pointer
   // We can skip checking for configured object if in Function mode
   if (!inFunctionMode)
   {
      obj = FindObject(parts[0]);
      
      if (obj == NULL)
      {
         InterpreterException ex
            (wxT("Cannot find the ReportFile \"") + parts[0] + wxT("\""));
         HandleError(ex);
         return false;
      }
      
      // Set ReportFile pointer
      cmd->SetRefObject(obj, Gmat::SUBSCRIBER, parts[0], 0);
   }
   
   // Create Parameters to report
   for (int i=1; i<count; i++)
   {
      obj = (GmatBase*)CreateSystemParameter(parts[i]);
      
      if (!inFunctionMode)
      {
         if (obj != NULL)
         {
            cmd->SetRefObject(obj, Gmat::PARAMETER, parts[i], 0);
         }
         else
         {
            InterpreterException ex
               (wxT("Nonexistent or disallowed Report Variable: \"") + parts[i] +
                wxT("\";\nIt will not be added to Report"));
            HandleError(ex);
            retval = false;
         }
      }
   }
   
   #ifdef DEBUG_ASSEMBLE_REPORT_COMMAND 
   MessageInterface::ShowMessage(wxT("AssembleReportCommand() returning %d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool AssembleCreateCommand(GmatCommand *cmd, const wxString &desc)
//------------------------------------------------------------------------------
bool Interpreter::AssembleCreateCommand(GmatCommand *cmd, const wxString &desc)
{   
   #ifdef DEBUG_ASSEMBLE_CREATE
   MessageInterface::ShowMessage
      (wxT("AssembleCreateCommand() Create command desc=<%s>\n"), desc.c_str());
   #endif
   
   debugMsg = wxT("In AssembleCreateCommand()");
   wxString::size_type typeIndex = desc.find_first_of(wxT(" "));
   wxString objTypeStr = desc.substr(0, typeIndex);
   wxString objNameStr = desc.substr(typeIndex+1);
   
   #ifdef DEBUG_ASSEMBLE_CREATE
   MessageInterface::ShowMessage(wxT("   Create object type=<%s>\n"), objTypeStr.c_str());
   MessageInterface::ShowMessage(wxT("   Create object name=<%s>\n"), objNameStr.c_str());
   #endif
   
   // check if object type is valid
   if (!IsObjectType(objTypeStr))
   {
      InterpreterException ex
         (wxT("Unknown object type \"") + objTypeStr + wxT("\" found in ") +
          cmd->GetTypeName() + wxT(" command"));
      HandleError(ex);
      return false;
   }
   
   //-----------------------------------------------------------------
   // check if comma is allowed in Create command (loj: 2008.08.29)
   //-----------------------------------------------------------------
   #ifdef __DISALLOW_COMMA_IN_CREATE__
   // check for comma, if comman is not allowed in Create command
   if (objNameStr.find(wxT(",")) != objNameStr.npos)
   {
      InterpreterException ex
         (wxT("Comma is not allowed in ") + cmd->GetTypeName() + wxT(" command"));
      HandleError(ex);
      return false;
   }
   StringArray objNames = GmatStringUtil::SeparateBy(objNameStr, wxT(" "), true);
   #else
   StringArray objNames = GmatStringUtil::SeparateBy(objNameStr, wxT(", "), true);
   #endif
   
   
   #ifdef DEBUG_ASSEMBLE_CREATE
   WriteStringArray(wxT("Create object names"), wxT(""), objNames);
   #endif
   
   if (objNames.size() == 0)
   {
      InterpreterException ex
         (wxT("Missing object name found in ") + cmd->GetTypeName() + wxT(" command"));
      HandleError(ex);
      return false;
   }
   
   wxString objTypeStrToUse = objTypeStr;
   // Special case for Propagator and OpenGLPlot
   if (objTypeStr == wxT("Propagator"))
      objTypeStrToUse = wxT("PropSetup");
   else if (objTypeStr == wxT("OpenGLPlot"))
      objTypeStrToUse = wxT("OrbitView");
   
   try
   {
      // if object is MatlabFunction make sure we add .m extenstion to avoid
      // automatically creating GmatFunction in the Sandbox::HandleGmatFunction()
      cmd->SetStringParameter(wxT("ObjectType"), objTypeStrToUse);
      for (UnsignedInt i=0; i<objNames.size(); i++)
         cmd->SetStringParameter(wxT("ObjectNames"), objNames[i]);
   }
   catch (BaseException &e)
   {
      MessageInterface::ShowMessage(e.GetFullMessage());
      throw;
   }
   
   //----------------------------------------------------------------------
   // Check all object names in the Create command for global objects.
   // If object type is automatic global and the same object names found in
   // the GlobalObjectStore, throw an exception exception
   // Just give warning for now (LOJ: 2010.12.16)
   //----------------------------------------------------------------------
   bool globalObjFound = false;
   wxString globalObjNames;
   StringArray defaultCSNames = theModerator->GetDefaultCoordinateSystemNames();
   
   for (UnsignedInt i = 0; i < objNames.size(); i++)
   {
      wxString name1 = objNames[i];
      if (find(defaultCSNames.begin(), defaultCSNames.end(), name1) != defaultCSNames.end())
      {
         wxString msg = 
            wxT("The default CoordinateSystem \"") + name1 + wxT("\" is automatic ")
            wxT("global object and was already created, so ignoring");
         InterpreterException ex(msg);
         HandleError(ex, true, true);
      }
      else
      {
         GmatBase *obj1 = FindObject(name1, objTypeStrToUse);
         if (obj1 != NULL && obj1->GetIsGlobal())
         {
            globalObjFound = true;
            globalObjNames = globalObjNames + name1 + wxT(" ");
         }
      }
   }
   
   if (globalObjFound)
   {
      wxString msg = 
         wxT("The following automatic global objects are already created, so ignoring: ") + globalObjNames;
      InterpreterException ex(msg);
      HandleError(ex, true, true);
      //return false;
   }
   
   
   //-------------------------------------------------------------------
   // Create an unmanaged object and set to command
   // Note: Generally unnamed object will not be added to configuration,
   //       but we need name for Array for syntax checking, so pass name
   //       and set false to unmanage Array objects
   //-------------------------------------------------------------------
   wxString name;
   // We also need named object for celetial body, so it can be added to the
   // solar system in use with name (LOJ: 2010.04.30)
   if ((objTypeStrToUse == wxT("Variable") || objTypeStrToUse == wxT("Array")) ||
       (find(celestialBodyList.begin(), celestialBodyList.end(), objTypeStrToUse) != 
        celestialBodyList.end()))
      name = objNames[0];
   
   #ifdef DEBUG_ASSEMBLE_CREATE
   MessageInterface::ShowMessage
      (wxT("   About to create reference object of '%s' for Create command\n"),
       objTypeStrToUse.c_str());
   #endif
   
   // We don't want to manage object to configuration, so pass 0
   GmatBase *obj = CreateObject(objTypeStrToUse, name, 0);
   
   #ifdef DEBUG_ASSEMBLE_CREATE
   MessageInterface::ShowMessage(wxT("   %s created\n"), obj->GetTypeName().c_str());
   #endif
   
   if (obj == NULL)
   {
      #ifdef DEBUG_ASSEMBLE_CREATE
         MessageInterface::ShowMessage(wxT("Reference object for Create command is NULL??\n"));
      #endif
      return false;
   }
   
   // Send the object to the Create command
   //cmd->SetRefObject(obj, Gmat::UNKNOWN_OBJECT, obj->GetName());
   cmd->SetRefObject(obj, GmatBase::GetObjectType(objTypeStrToUse), obj->GetName());
   
   // Special case for MatlabFunction
   // Since CallFunction does not know whether the function is Gmat or Matlab function,
   // add an extention to indicate it is MatlabFunction so that Sandbox can create
   // proper functions. Add the name to tempObjectNames so that when creating
   // CallFunction or Assignment command, it can look in the array to figure out
   // whether it is MatlabFunction or not.
   if (objTypeStrToUse == wxT("MatlabFunction"))
   {
      for (UnsignedInt i=0; i<objNames.size(); i++)
      {
         #ifdef DEBUG_ASSEMBLE_CREATE
         MessageInterface::ShowMessage
            (wxT("   Adding '%s' to tempObjectNames\n"), objNames[i].c_str());
         #endif
         tempObjectNames.push_back(objNames[i]);
      }
      
      #ifdef DEBUG_ASSEMBLE_CREATE
      MessageInterface::ShowMessage
         (wxT("   tempObjectNames.size()=%d\n"), tempObjectNames.size());
      #endif
   }
   
   #ifdef DEBUG_ASSEMBLE_CREATE
   MessageInterface::ShowMessage
      (wxT("AssembleCreateCommand() returning true, created obj=<%p>, objType=<%s>, ")
       wxT("objName=<%s>\n"), obj, obj->GetTypeName().c_str(), obj->GetName().c_str());
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// bool SetCommandRefObjects(GmatCommand *cmd, const wxString &desc)
//------------------------------------------------------------------------------
bool Interpreter::SetCommandRefObjects(GmatCommand *cmd, const wxString &desc)
{
   #ifdef DEBUG_ASSEMBLE_COMMAND   
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetCommandRefObjects() cmd=<%s>, desc=<%s>\n"),
       cmd->GetTypeName().c_str(), desc.c_str());
   #endif

   debugMsg = wxT("In SetCommandRefObjects()");
   
   // Save, Global commands, for compatability with MATLAB, should not have
   // parentheses (except to indicate array elements), brackets, or braces.
   // Since Create command can have wxT("Create Array vec[3,1]"), so do not check.
   if (!GmatStringUtil::HasNoBrackets(desc, false))
   {
      wxString msg = 
         wxT("The ") + cmd->GetTypeName() + wxT(" command is not allowed to contain ")
         wxT("brackets, braces, or parentheses");
      InterpreterException ex(msg);
      HandleError(ex);
      return false;
   }
   
   // we only want to separate by spaces - commas are not allowed, 
   // not even in arrays (for this command)
   StringArray parts = GmatStringUtil::SeparateBy(desc, wxT(" "), true);
   unsigned int numParts = parts.size();
   bool isOk = true;
   
   if (numParts == 0)
   {
      wxString msg = 
         wxT("The ") + cmd->GetTypeName() + wxT(" command has missing object names");
      InterpreterException ex(msg);
      HandleError(ex);
      return false;
   }
   
   #ifdef DEBUG_ASSEMBLE_COMMAND   
   WriteStringArray(wxT("object name parts"), wxT(""), parts);
   #endif
   
   for (unsigned int i=0; i<numParts; i++)
   {
      if (parts[i].find(',') != parts[i].npos)
      {
         wxString msg = 
            wxT("The ") + cmd->GetTypeName() + wxT(" command is not allowed to contain commas - ")
            wxT("separate objects by spaces");
         InterpreterException ex(msg);
         HandleError(ex);
         isOk = false;
      }
      else if (!GmatStringUtil::IsValidName(parts[i]))
      {
         wxString msg = 
            wxT("\"") + parts[i] + wxT("\" is an invalid object name in ") +
            cmd->GetTypeName() + wxT(" command");
         InterpreterException ex(msg);
         HandleError(ex);
         isOk = false;
      }
      else
      {
         cmd->SetStringParameter(wxT("ObjectNames"), parts[i]);
      }
   }
   
   return isOk;
}


//------------------------------------------------------------------------------
//GmatCommand* CreateAssignmentCommand(const wxString &lhs,
//                                     const wxString &rhs, bool &retFlag,
//                                     GmatCommand *inCmd)
//------------------------------------------------------------------------------
GmatCommand* Interpreter::CreateAssignmentCommand(const wxString &lhs,
                                                  const wxString &rhs,
                                                  bool &retFlag, GmatCommand *inCmd)
{
   #ifdef DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("Interpreter::CreateAssignmentCommand() lhs=<%s>, rhs=<%s>\n"), lhs.c_str(),
       rhs.c_str());
   #endif
   
   debugMsg = wxT("In CreateAssignmentCommand()");
   
   // First check if it is really assignment by checking blank in the lhs.
   // (The lhs must be Variable, String, Array, or object property and this is
   //  validated in the Assignment command)
   wxString::size_type index = lhs.find_last_of(wxT(" "));
   if (index != lhs.npos)
   {
      wxString cmd = lhs.substr(0, index);
      
      // See if it is an Array since array index can have blanks
      index = lhs.find(wxT("("));
      if (index != lhs.npos)
      {
         if (!IsArrayElement(lhs))
         {
            InterpreterException ex(wxT("\"") + cmd + wxT("\" is not a valid Command"));
            HandleError(ex);
            return NULL;
         }
      }
   }
   
   wxString desc = lhs + wxT(" = ") + rhs;
   return CreateCommand(wxT("GMAT"), desc, retFlag, inCmd);
}


//------------------------------------------------------------------------------
// Parameter* CreateSystemParameter(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a system Parameter from the input parameter name. If the name contains
 * dots, it consider it as a system parameter.  If it is not a system Parameter
 * it checks if object by given name is a Parameter.
 *
 * @param  name   parameter name to be parsed for Parameter creation
 *                Such as, sat1.Earth.ECC, sat1.SMA
 *
 * @return Created Paramteter pointer or pointer of the Parameter by given name
 *         NULL if it is not a system Parameter nor named object is not a Parameter
 *
 */
//------------------------------------------------------------------------------
Parameter* Interpreter::CreateSystemParameter(const wxString &str)
{
   #ifdef DEBUG_CREATE_PARAM
   MessageInterface::ShowMessage
      (wxT("Interpreter::CreateSystemParameter() entered, str='%s', inFunctionMode=%d\n"),
       str.c_str(), inFunctionMode);
   #endif
   
   Integer manage = 1;
   // if in function mode set manage = 2 (loj: 2008.12.16)
   if (inFunctionMode)
      manage = 2;
   
   bool paramCreated = false;
   Parameter *param = theValidator->CreateSystemParameter(paramCreated, str, manage);
   
   #ifdef DEBUG_CREATE_PARAM
   MessageInterface::ShowMessage
      (wxT("   Parameter '%s'%screated\n"), str.c_str(), paramCreated ? wxT(" ") : wxT(" NOT "));
   MessageInterface::ShowMessage
      (wxT("Interpreter::CreateSystemParameter() returning <%p><%s>'%s'\n"), param,
       (param == NULL) ? wxT("NULL") : param->GetTypeName().c_str(),
       (param == NULL) ? wxT("NULL") : param->GetName().c_str());
   #endif
   
   return param;
}


//------------------------------------------------------------------------------
// Parameter* CreateParameter(const wxString &type, const wxString &name,
//                            const wxString &ownerName, const wxString &depName)
//------------------------------------------------------------------------------
/**
 * Calls the Moderator to create a Parameter.
 * 
 * @param  type       Type of parameter requested
 * @param  name       Name for the parameter.
 * @param  ownerName  object name of parameter requested (wxT(""))
 * @param  depName    Dependent object name of parameter requested (wxT(""))
 * 
 * @return Pointer to the constructed Parameter.
 */
//------------------------------------------------------------------------------
Parameter* Interpreter::CreateParameter(const wxString &type, 
                                        const wxString &name,
                                        const wxString &ownerName,
                                        const wxString &depName)
{
   #ifdef DEBUG_CREATE_PARAM
   MessageInterface::ShowMessage
      (wxT("Interpreter::CreateParameter() type='%s', name='%s', ownerName='%s', ")
       wxT("depName='%s', inFunctionMode=%d\n"), type.c_str(), name.c_str(),
       ownerName.c_str(), depName.c_str(), inFunctionMode);
   #endif
   
   Integer manage = 1;
   if (inFunctionMode)
      manage = 0;
   
   return theValidator->CreateParameter(type, name, ownerName, depName, manage);
}


//------------------------------------------------------------------------------
// Parameter* GetArrayIndex(const wxString &arrayStr, Integer &row, Integer &col)
//------------------------------------------------------------------------------
/**
 * Retrives array index from the configured array.
 *
 * @param  arrayStr  String form of array (A(1,3), B(2,j), etc)
 *
 * @note Array name must be created and configured before acces.
 */
//------------------------------------------------------------------------------
Parameter* Interpreter::GetArrayIndex(const wxString &arrayStr,
                                      Integer &row, Integer &col)
{
   debugMsg = wxT("In GetArrayIndex()");
   wxString name, rowStr, colStr;
   
   // parse array name and index
   GmatStringUtil::GetArrayIndex(arrayStr, rowStr, colStr, row, col, name);
   
   // Remove - sign from the name
   if (name[0] == '-')
      name = name.substr(1);
   
   #ifdef DEBUG_ARRAY_GET
   MessageInterface::ShowMessage
      (wxT("Interpreter::GetArrayIndex() arrayStr=<%s>, name=<%s>, rowStr=<%s>, ")
       wxT("colStr=<%s>, row=%d, col=%d\n"), arrayStr.c_str(), name.c_str(),
       rowStr.c_str(), colStr.c_str(), row, col);
   #endif
   
   Parameter *param = (Parameter*)FindObject(name);
   
   // Note:
   // To catch errors as much as possible, limited return statement used
   // even when error found
   
   if (param == NULL)
   {
      InterpreterException ex(wxT("Array named \"") + name + wxT("\" is undefined"));
      HandleError(ex);
   }
   else
   {
      if (param->GetTypeName() != wxT("Array"))
      {
         InterpreterException ex(wxT("\"") + name + wxT("\" is not an Array"));
         HandleError(ex);
         return NULL;
      }
      
      if (rowStr == wxT("0") || colStr == wxT("0") ||rowStr == wxT("-1") || colStr == wxT("-1"))
      {
         InterpreterException ex(wxT("Index exceeds matrix dimensions"));
         HandleError(ex);
         return NULL;
      }
      
      // get row value
      if (row == -1 && rowStr != wxT("-1"))
      {
         Parameter *rowParam = (Parameter*)FindObject(rowStr);
         if (rowParam == NULL)
         {
            InterpreterException ex
               (wxT("Array row index named \"") + rowStr + wxT("\" is undefined"));
            HandleError(ex);
         }
         else
         {
            if (rowParam->GetReturnType() == Gmat::REAL_TYPE)
            {
               row = (Integer)rowParam->GetReal() - 1; // index starts at 0
            }
            else
            {
               InterpreterException ex
                  (wxT("Cannot handle row index of Array named \"") + name + wxT("\""));
               HandleError(ex);
            }
         }
      }
      
      // get column value
      if (col == -1 && colStr != wxT("-1"))
      {
         Parameter *colParam = (Parameter*)FindObject(colStr);
         if (colParam == NULL)
         {
            InterpreterException ex
               (wxT("Column index named \"") + colStr + wxT("\" is undefined"));
            HandleError(ex);
         }
         else
         {
            if (colParam->GetReturnType() == Gmat::REAL_TYPE)
            {
               col = (Integer)colParam->GetReal() - 1; // index starts at 0
            }
            else
            {
               InterpreterException ex
                  (wxT("Cannot handle column index of Array named \"") + name + wxT("\""));
               HandleError(ex);
            }
         }
      }
   }
   
   #ifdef DEBUG_ARRAY_GET
   MessageInterface::ShowMessage
      (wxT("   GetArrayIndex() row=%d, col=%d\n"), row, col);
   #endif
   
   if (param == NULL || row == -1 || col == -1)
      return NULL;
   else
      return param;
}


//------------------------------------------------------------------------------
// GmatBase* MakeAssignment(const wxString &lhs, const wxString &rhs)
//------------------------------------------------------------------------------
/*
 * Sets rhs to lhs.
 *
 * @param  lhs  Left hand side component
 * @param  rhs  Right hand side component
 *
 * @return return LHS object pointer
 */
//------------------------------------------------------------------------------
GmatBase* Interpreter::MakeAssignment(const wxString &lhs, const wxString &rhs)
{
   #ifdef DEBUG_MAKE_ASSIGNMENT
   MessageInterface::ShowMessage
      (wxT("Interpreter::MakeAssignment() lhs=<%s>, rhs=<%s>\n"), lhs.c_str(), rhs.c_str());
   MessageInterface::ShowMessage
      (wxT("   inFunctionMode=%d, hasFunctionDefinition=%d\n"), inFunctionMode,
       hasFunctionDefinition);
   #endif

   debugMsg = wxT("In MakeAssignment()");
   bool retval = false;
   
   // Separate dots
   StringArray lhsParts = theTextParser.SeparateDots(lhs);
   Integer lhsPartCount = lhsParts.size();
   StringArray rhsParts = theTextParser.SeparateDots(rhs);
   Integer rhsPartCount = rhsParts.size();
   wxString::size_type dot;
   wxString lhsObjName, rhsObjName;
   wxString lhsPropName, rhsPropName;
   GmatBase *lhsObj = NULL;
   GmatBase *rhsObj = NULL;
   bool isLhsObject = false;
   bool isRhsObject = false;
   bool isLhsArray = false;
   bool isRhsArray = false;
   bool isLhsVariable = false;
   bool isRhsVariable = false;
   bool isLhsString = false;
   bool isRhsString = false;
   bool isRhsNumber = false;
   
   currentBlock = lhs + wxT(" = ") + rhs;
   
   #ifdef DEBUG_MAKE_ASSIGNMENT
   WriteStringArray(wxT("lhs parts"), wxT(""), lhsParts);
   WriteStringArray(wxT("rhs parts"), wxT(""), rhsParts);
   #endif
   
   // check LHS
   if (lhsPartCount > 1)
   {
      lhsObjName = lhsParts[0];
      lhsObj = FindObject(lhsObjName);
      
      if (lhsObj == NULL)
      {
         if (lhs == wxT(""))
         {
            InterpreterException ex(wxT("Object field assignment is incomplete"));
            HandleError(ex);
         }
         else
         {
            InterpreterException ex
               (wxT("Cannot find LHS object named \"") + lhsObjName + wxT("\""));
            HandleError(ex);
         }
         return NULL;
      }
      
      dot = lhs.find('.');
      if (dot == lhs.npos)
         lhsPropName = lhsParts[1];
      else
         lhsPropName = lhs.substr(dot+1);
   }
   else
   {
      lhsObj = FindObject(lhs);
      
      if (lhsObj)
      {
         if (IsArrayElement(lhs))
            isLhsArray = true;
         else
            isLhsObject = true;
         
         if (lhsObj->IsOfType(wxT("Variable")))
            isLhsVariable = true;
         else if (lhsObj->IsOfType(wxT("String")))
            isLhsString = true;
      }
      else
      {
         if (lhs == wxT(""))
         {
            InterpreterException ex(wxT("Missing equal sign in object field assignment"));
            HandleError(ex);
         }
         else
         {
            InterpreterException ex(wxT("Cannot find LHS object named \"") + lhs + wxT("\""));
            HandleError(ex);
         }
         return NULL;
      }
   }
   
   #ifdef DEBUG_MAKE_ASSIGNMENT
   MessageInterface::ShowMessage
      (wxT("   isLhsObject=%d, isLhsArray=%d, isLhsVariable=%d, isLhsString=%d, ")
       wxT("lhsPropName=<%s>, lhsObj=<%p><%s>\n"), isLhsObject, isLhsArray,
       isLhsVariable, isLhsString, lhsPropName.c_str(), lhsObj,
       (lhsObj == NULL) ? wxT("NULL") : lhsObj->GetName().c_str() );
   #endif
   
   // check RHS
   if (rhsPartCount > 1)
   {
      rhsObjName = rhsParts[0];
      wxString objTypeStr = wxT("");
      // Check if RHS has open paren, then it should be an Array (loj: 2008.08.15)
      if (rhsObjName.find_first_of(wxT("(")) != rhsObjName.npos)
         objTypeStr = wxT("Array");
      rhsObj = FindObject(rhsObjName, objTypeStr);
      
      if (rhsObj == NULL)
      {
         //throw InterpreterException(wxT("Cannot find RHS object: ") + rhsObjName + wxT("\n"));
         
         #ifdef DEBUG_MAKE_ASSIGNMENT
         MessageInterface::ShowMessage
            (wxT("   Cannot find RHS object '%s' of type <%s>. It may be a string value\n"),
             rhsObjName.c_str(), objTypeStr.c_str());
         #endif
      }
      else
      {
         // Note: Do not set rhsObj to true here since it needs to create
         // a Parameter if needed.
         
         #ifdef DEBUG_MAKE_ASSIGNMENT
         MessageInterface::ShowMessage
            (wxT("   Found rhs object <%s>'%s', now checking for dot\n"),
             rhsObj->GetTypeName().c_str(), rhsObj->GetName().c_str());
         #endif
         
         // Check if it is CallFunction first
         dot = rhs.find('.');
         if (dot == rhs.npos)
         {
            rhsPropName = rhsParts[1];
         }
         else
         {
            wxString afterDot = rhs.substr(dot+1);
            // Check if rhs is a Parameter first
            // This will fix Bug 1669 (LOJ: 2009.12.08)
            if (theValidator->IsParameterType(rhs))
            {
               rhsPropName = afterDot;
            }
            else
            {
               // Check if it is object property
               GmatBase *toObj = NULL;
               Integer toId = -1;
               Gmat::ParameterType toType;
               if (FindPropertyID(rhsObj, afterDot, &toObj, toId, toType))
                  rhsPropName = afterDot;
               else
                  rhsPropName = rhsParts[1];
            }
         }
      }
   }
   else
   {
      // If firist RHS char is wxT("-") sign, use without it in finding name.
      // This is due to backward propagation. For example,
      // Propagate -prop(Sat1, Sat2, {Sat1.Periapsis})
      wxString newName = rhs;
      
      if (rhs[0] == '-')
         newName = rhs.substr(1);
      
      rhsObj = FindObject(newName);
      
      if (rhsObj)
      {
         if (rhsObj->IsOfType(wxT("Variable")))
            isRhsVariable = true;
         else if (rhsObj->IsOfType(wxT("String")))
            isRhsString = true;
         
         if (IsArrayElement(rhs))
            isRhsArray = true;
         else
         {
            // @note
            // We want to allow user to create object and name it with one of
            // ObjectTypes. e.g. Create Spacecraft Spacecraft.
            // So if name found in configuration and not an ObjectType, except
            // calculated PARAMETER, it will considered as string value.
            if (IsObjectType(newName) && rhsObj->GetType() != Gmat::PARAMETER)
               isRhsObject = false;
            else
               isRhsObject = true;
         }
      }
      else
      {
         if (GmatStringUtil::IsNumber(rhs))
            isRhsNumber = true;
      }
   }
   
   #ifdef DEBUG_MAKE_ASSIGNMENT
   MessageInterface::ShowMessage
      (wxT("   isRhsObject=%d, isRhsArray=%d, isRhsVariable=%d, isRhsString=%d, ")
       wxT("isRhsNumber=%d, rhsPropName=<%s>, rhsObj=<%p><%s>\n"),
       isRhsObject, isRhsArray, isRhsVariable, isRhsString, isRhsNumber,
       rhsPropName.c_str(), rhsObj,
       (rhsObj == NULL) ? wxT("NULL") : rhsObj->GetName().c_str() );
   #endif
   
   if (isLhsObject)
   {
      bool isAllowed = true;
      
      // Variable is allowed to set to only numbers (Bug 2043)
      if (isLhsVariable && !isRhsNumber)
         isAllowed = false;
      
      // String is allowed to set to only literals (Bug 2043)
      if (isAllowed && isLhsString && isRhsString)
         isAllowed = false;
      
      if (!isAllowed)
      {
         InterpreterException ex
            (wxT("Setting \"") + lhs + wxT("\" to \"") + rhs + wxT("\" is not allowed before BeginMissionSequence"));
         HandleError(ex);
         return false;
      }
      
      if (isRhsObject)
         retval = SetObjectToObject(lhsObj, rhsObj, rhs);
      else if (rhsPropName != wxT(""))
         retval = SetObjectToProperty(lhsObj, rhsObj, rhsPropName);
      else if (isRhsArray)
         retval = SetObjectToArray(lhsObj, rhs);
      else
         retval = SetObjectToValue(lhsObj, rhs);
   }
   else if (lhsPropName != wxT("")) // LHS is object property
   {
      bool isAllowed = true;
      GmatBase *toObj = NULL;
      Integer toId = -1;
      Gmat::ParameterType toType;
      
      // Check LHS property type
      FindPropertyID(lhsObj, lhsPropName, &toObj, toId, toType);
      
      // Only object type of property is allowed to set to another object
      if (toType != Gmat::OBJECT_TYPE && toType != Gmat::OBJECTARRAY_TYPE)
      {
         // Setting object property to Variable, Array and String are not allowd(Bug 2043)
         if (isRhsArray || isRhsVariable || isRhsString)
            isAllowed = false;
         
         // Setting object property to other property is not allowed(Bug 2043)
         // excluding FILENAME_TYPE which can have dots.
         if (isAllowed && rhsPropName != wxT("") && toType != Gmat::FILENAME_TYPE)
         {
            isAllowed = false;
         }
      }
      
      if (!isAllowed)
      {
         InterpreterException ex
            (wxT("Setting \"") + lhs + wxT("\" to \"") + rhs + wxT("\" is not allowed before BeginMissionSequence"));
         HandleError(ex);
         return false;
      }
      
      if (isRhsObject)
         retval = SetPropertyToObject(lhsObj, lhsPropName, rhsObj);
      else if (rhsPropName != wxT(""))
         retval = SetPropertyToProperty(lhsObj, lhsPropName, rhsObj, rhsPropName);
      else if (isRhsArray)
         retval = SetPropertyToArray(lhsObj, lhsPropName, rhs);
      else
         retval = SetPropertyToValue(lhsObj, lhsPropName, rhs);
   }
   else if (isLhsArray)
   {
      if (!isRhsNumber)
      {
         InterpreterException ex
            (wxT("Setting \"") + lhs + wxT("\" to \"") + rhs + wxT("\" is not allowed before BeginMissionSequence"));
         HandleError(ex);
         return false;
      }
      
      if (isRhsObject)
         retval = SetArrayToObject(lhsObj, lhs, rhsObj);
      else if (rhsPropName != wxT(""))
         retval = SetArrayToProperty(lhsObj, lhs, rhsObj, rhsPropName);
      else if (isRhsArray)
         retval = SetArrayToArray(lhsObj, lhs, rhsObj, rhs);
      else
         retval = SetArrayToValue(lhsObj, lhs, rhs);
   }
   else
   {
      InterpreterException ex
         (wxT("Interpreter::MakeAssignment() Internal error if it reached here."));
      HandleError(ex);
   }
   
   #ifdef DEBUG_MAKE_ASSIGNMENT
   MessageInterface::ShowMessage
      (wxT("Interpreter::MakeAssignment() returning lhsObj=%p\n"), lhsObj);
   #endif
   
   if (retval)
      return lhsObj;
   else
      return NULL;
}


//-------------------------------------------------------------------------------
// bool SetObjectToObject(GmatBase *toObj, GmatBase *fromObj, const wxString &rhs)
//-------------------------------------------------------------------------------
bool Interpreter::SetObjectToObject(GmatBase *toObj, GmatBase *fromObj,
                                    const wxString &rhs)
{
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetObjectToObject() to=%s, from=%s, rhs='%s'\n"),
       toObj->GetName().c_str(), fromObj->GetName().c_str(), rhs.c_str());
   #endif
   
   debugMsg = wxT("In SetObjectToObject()");
   
   // Copy object
   if (toObj->GetTypeName() == fromObj->GetTypeName())
   {
      toObj->Copy(fromObj);
   }
   else
   {
      InterpreterException ex(wxT("Object type of LHS and RHS are not the same."));
      HandleError(ex);
      return false;
   }
   
   // More handling for Variable
   if (toObj->IsOfType(wxT("Variable")))
   {
      // If first char is - sign, negate the value
      if (rhs[0] == '-')
      {
         Real rval = toObj->GetRealParameter(wxT("Value")) * -1;
         toObj->SetRealParameter(wxT("Value"), rval);
      }
      // Set Variable's InitialValue so when it is written out, it will have original string value (LOJ: 2010.09.21)
      toObj->SetStringParameter(wxT("InitialValue"), rhs);
   }
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetObjectToObject() returning true\n"));
   #endif
   
   return true;
}


//-------------------------------------------------------------------------------
// bool SetObjectToProperty(GmatBase *toObj, GmatBase *fromOwner,
//                          const wxString &fromProp)
//-------------------------------------------------------------------------------
bool Interpreter::SetObjectToProperty(GmatBase *toObj, GmatBase *fromOwner,
                                      const wxString &fromProp)
{
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("SetObjectToProperty() toObj=%s, fromOwner=%s, fromProp=%s\n"),
       toObj->GetName().c_str(), fromOwner->GetName().c_str(), fromProp.c_str());
   #endif
   
   debugMsg = wxT("In SetObjectToProperty()");
   wxString rhs = fromOwner->GetName() + wxT(".") + fromProp;
   Integer fromId = -1;
   Gmat::ParameterType fromType = Gmat::UNKNOWN_PARAMETER_TYPE;
   Parameter *rhsParam = NULL;
   
   if (toObj->GetTypeName() != wxT("Variable") && toObj->GetTypeName() != wxT("String"))
   {
      InterpreterException ex
         (wxT("Setting an object \"") + toObj->GetName() + wxT("\" to ") + fromProp +
          wxT("\" is not allowed"));
      HandleError(ex);
      return false;
   }
   
   try
   {
      fromId = fromOwner->GetParameterID(fromProp);
      fromType = fromOwner->GetParameterType(fromId);
   }
   catch (BaseException &e)
   {
      // try if fromProp is a system Parameter
      rhsParam = CreateSystemParameter(rhs);
      
      // it is not a Parameter, so handle error
      if (rhsParam == NULL)
      {
         // Try setting as Variable expression (loj: 2008.08.05)
         // to handle var = sat.A1ModJulian - GmatTimeConstants::MJD_OF_J2000 prior to mission sequence
         // It also shows correct expression in the GUI
         if (ParseVariableExpression((Parameter*)toObj, rhs))
            return true;
         else
         {
            HandleError(e);
            return false;
         }
      }
      
      fromType = rhsParam->GetReturnType();
      
      #ifdef DEBUG_SET
      MessageInterface::ShowMessage
         (wxT("SetObjectToProperty() rhs:%s is a parameter\n"), rhs.c_str());
      #endif
   }
   
   Parameter *toParam = (Parameter*)toObj;
   Gmat::ParameterType toType = toParam->GetReturnType();
   
   if (fromType == toType)
   {
      if (fromId == -1)
      {
         // LHS is a Variable or String, RHS is a Parameter
         if (toType == Gmat::STRING_TYPE || toType == Gmat::ENUMERATION_TYPE ||
             toType == Gmat::FILENAME_TYPE)
            toObj->SetStringParameter(wxT("Value"), rhsParam->GetString());
         else if (toType == Gmat::REAL_TYPE)
            ParseVariableExpression(toParam, rhs);
      }
      else
      {
         // LHS is a Variable or String, RHS is an ObjectProperty
         if (toType == Gmat::STRING_TYPE || toType == Gmat::ENUMERATION_TYPE ||
             toType == Gmat::FILENAME_TYPE)
            toObj->SetStringParameter(wxT("Value"), fromOwner->GetStringParameter(fromId));
         else if (toType == Gmat::REAL_TYPE)
         {
            // Check to see if fromProp is also a system Parameter, (loj: 2008.08.06)
            // if so Parameter takes higher presendence over ObjectProperty.
            rhsParam = CreateSystemParameter(rhs);
            if (rhsParam != NULL)
            {
               ParseVariableExpression(toParam, rhs);
            }
            else
               toObj->SetRealParameter(wxT("Value"), fromOwner->GetRealParameter(fromId));
         }
      }
   }
   else
   {
      //InterpreterException ex
      //   (wxT("Setting an object \"") + toObj->GetName() + wxT("\" to ") + fromProp +
      //    wxT("\" is not allowed"));
      InterpreterException ex
         (wxT("Setting \"") + fromProp + wxT("\" to an object \"") + toObj->GetName() +
          wxT("\" is not allowed"));
      HandleError(ex);
      return false;
   }
   
   return true;
}


//-------------------------------------------------------------------------------
// bool SetObjectToArray(GmatBase *toObj, const wxString &fromArray)
//-------------------------------------------------------------------------------
bool Interpreter::SetObjectToArray(GmatBase *toObj, const wxString &fromArray)
{
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetObjectToArray() toObj=%s, fromArray=%s\n"),
       toObj->GetName().c_str(), fromArray.c_str());
   #endif
   
   debugMsg = wxT("In SetObjectToArray()");
   
   if (toObj->GetTypeName() != wxT("Variable"))
   {
      //InterpreterException ex
      //   (wxT("Setting \"") + fromArray + wxT("\" to an object \"") + toObj->GetName() +
      //    wxT("\" is not allowed"));
      InterpreterException ex
         (wxT("Setting \"") + toObj->GetName() + wxT("\" to an array \"") + fromArray +
          wxT("\" is not allowed"));
      HandleError(ex);
      return false;
   }
   
   Integer row, col;
   Parameter *param = GetArrayIndex(fromArray, row, col);
   if (param == NULL)
      return false;
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("   SetObjectToArray() row=%d, col=%d\n"), row, col);
   #endif
   
   // Check for array index
   if (row == -1 || col == -1)
   {
      InterpreterException ex(wxT("Invalid array index: ") + fromArray);
      HandleError(ex);
      return false;
   }
   
   Real rval = GetArrayValue(fromArray, row, col);
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage(wxT("   SetObjectToArray() rval=%f\n"), rval);
   #endif

   try
   {
      // Handle minus sign
      if (fromArray[0] == '-')
         rval = -rval;
      toObj->SetRealParameter(wxT("Value"), rval);
   }
   catch (BaseException &e)
   {
      HandleError(e);
      return false;
   }
   
   // Set Variable's InitialValue so when it is written out, it will have original string value (LOJ: 2010.09.21)
   toObj->SetStringParameter(wxT("InitialValue"), fromArray);
   
   return true;
}


//-------------------------------------------------------------------------------
// bool SetObjectToValue(GmatBase *toObj, const wxString &value)
//-------------------------------------------------------------------------------
bool Interpreter::SetObjectToValue(GmatBase *toObj, const wxString &value)
{
   debugMsg = wxT("In SetObjectToValue()");
   wxString toObjType = toObj->GetTypeName();
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetObjectToValue() toObjType=<%s>, toObjName=%s, value=<%s>\n"),
       toObjType.c_str(), toObj->GetName().c_str(), value.c_str());
   #endif
   
   if (toObjType != wxT("Variable") && toObjType != wxT("String"))
   {
      //InterpreterException ex
      //   (wxT("Setting a String value \"") + value + wxT("\" to an object \"") + toObj->GetName() +
      //    wxT("\" of type \"") + toObjType + wxT("\" is not allowed"));
      InterpreterException ex
         (wxT("Setting an object \"") + toObj->GetName() + wxT("\" of type \"") + toObjType +
          wxT("\" to a value \"") + value + wxT("\" is not allowed"));
      HandleError(ex);
      return false;
   }
   
   if (toObjType == wxT("String"))
   {
      // check for unpaired single quotes
      if (GmatStringUtil::HasMissingQuote(value, wxT("'")))
      {
         InterpreterException ex(wxT("The string \"") + value + wxT("\" has missing single quote"));
         HandleError(ex);
         return false;
      }
      
      wxString valueToUse = GmatStringUtil::RemoveEnclosingString(value, wxT("'"));
      
      #ifdef DEBUG_SET
      MessageInterface::ShowMessage
         (wxT("   Calling %s->SetStringParameter(Expression, %s)\n"), toObj->GetName().c_str(),
          valueToUse.c_str());
      MessageInterface::ShowMessage
         (wxT("   Calling %s->SetStringParameter(Value, %s)\n"), toObj->GetName().c_str(),
          valueToUse.c_str());
      #endif
      
      toObj->SetStringParameter(wxT("Expression"), valueToUse);
      toObj->SetStringParameter(wxT("Value"), valueToUse);
   }
   else if (toObjType == wxT("Variable"))
   {
      Real rval;

      try
      {
         if (GmatStringUtil::ToReal(value, rval, true))
         {      
            #ifdef DEBUG_SET
            MessageInterface::ShowMessage(wxT("   SetObjectToValue() rval=%f\n"), rval);
            #endif
            
            toObj->SetRealParameter(wxT("Value"), rval);
         }
         else
         {
            // For bug 2025 fix, commented out (LOJ: 2010.09.16)
            //if (!ParseVariableExpression((Parameter*)toObj, value))
            //{
            //InterpreterException ex
            //(wxT("Setting \"") + value + wxT("\" to a Variable \"") + toObj->GetName() +
            //wxT("\" is not allowed"));
            InterpreterException ex
               (wxT("Setting an object \"") + toObj->GetName() + wxT("\" of type \"") + toObjType +
                wxT("\" to a value \"") + value + wxT("\" is not allowed"));
            HandleError(ex);
            return false;
            //}
         }
      }
      catch (BaseException &e)
      {
         HandleError(e);
         return false;
      }
   }
   
   // Set Variable's InitialValue so when it is written out, it will have original string value (LOJ: 2010.09.21)
   toObj->SetStringParameter(wxT("InitialValue"), value);
   return true;
}


//-------------------------------------------------------------------------------
// bool SetPropertyToObject(GmatBase *toOwner, const wxString &toProp,
//                          GmatBase *fromObj)
//-------------------------------------------------------------------------------
bool Interpreter::SetPropertyToObject(GmatBase *toOwner, const wxString &toProp,
                                      GmatBase *fromObj)
{
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetPropertyToObject() ownerType=%s, toOwner=%s, toProp=%s, ")
       wxT("fromObj=%s\n"), toOwner->GetTypeName().c_str(), toOwner->GetName().c_str(),
       toProp.c_str(), fromObj->GetName().c_str());
   #endif
   
   debugMsg = wxT("In SetPropertyToObject()");
   
   if (toOwner->GetType() == Gmat::ODE_MODEL)
   {
      wxString objName = fromObj->GetName();
      bool retval = SetForceModelProperty(toOwner, toProp, objName, fromObj);
      if (!retval)
      {
         InterpreterException ex
            (wxT("The value of \"") + objName + wxT("\" for field \"") + toProp +
             wxT("\" on ForceModel \"") + toOwner->GetName() + wxT("\" is not an allowed value"));
         HandleError(ex);
         return false;
      }
      
      return true;
   }
   
   
   GmatBase *toObj = NULL;
   Integer toId = -1;
   Gmat::ParameterType toType;
   
   try
   {
      FindPropertyID(toOwner, toProp, &toObj, toId, toType);
      
      if (toObj == NULL)
      {
         if (parsingDelayedBlock)
         {
            InterpreterException ex
               (wxT("The field name \"") + toProp + wxT("\" on object ") + toOwner->GetName() +
                wxT(" is not permitted"));
            HandleErrorMessage(ex, lineNumber, currentLine, true);
            return false;
         }
         
         delayedBlocks.push_back(currentBlock);
         wxString lineNumStr = GmatStringUtil::ToString(theReadWriter->GetLineNumber());
         delayedBlockLineNumbers.push_back(lineNumStr);
         
         #ifdef DEBUG_SET
         MessageInterface::ShowMessage
            (wxT("   ===> added to delayed blocks: line:%s, %s\n"), lineNumStr.c_str(),
             currentBlock.c_str());
         #endif
         
         return true;
      }
   }
   catch (BaseException &)
   {
      if (parsingDelayedBlock)
         return false;
      
      delayedBlocks.push_back(currentBlock);
      
      #ifdef DEBUG_SET
      MessageInterface::ShowMessage
         (wxT("   ===> added to delayed blocks: %s\n"), currentBlock.c_str());
      #endif
      
      return true;
   }
   
   toType = toObj->GetParameterType(toId);
   
   // Let's treat enumeration and filename type as string type
   if (toType == Gmat::ENUMERATION_TYPE || toType == Gmat::FILENAME_TYPE)
      toType = Gmat::STRING_TYPE;
   
   try
   {
      wxString fromTypeName = fromObj->GetTypeName();
      
      if (fromObj->GetType() == Gmat::PARAMETER)
      {
         Gmat::ParameterType fromType = ((Parameter*)fromObj)->GetReturnType();
         
         #ifdef DEBUG_SET
         MessageInterface::ShowMessage
            (wxT("   From object is a Parameter, toId=%d, fromType=%d, toType=%d\n"),
             toId, fromType, toType);
         #endif
         
         if (fromType == toType)
         {
            if (toType == Gmat::STRING_TYPE)
               toObj->SetStringParameter(toId, fromObj->GetStringParameter(wxT("Value")));
            else if (toType == Gmat::REAL_TYPE)
               toObj->SetRealParameter(toId, fromObj->GetRealParameter(wxT("Value")));
         }
         else
         {
            bool errorCond = false;
            if (fromTypeName == wxT("String"))
            {
               if (toType == Gmat::STRING_TYPE || toType == Gmat::STRINGARRAY_TYPE)
                  toObj->SetStringParameter(toId, fromObj->GetStringParameter(wxT("Value")));
               else if (toType == Gmat::OBJECT_TYPE || toType == Gmat::OBJECTARRAY_TYPE)
                  toObj->SetStringParameter(toId, fromObj->GetName());               
               else
                  errorCond = true;
            }
            else if (fromTypeName == wxT("Variable"))
            {
               if (toType == Gmat::REAL_TYPE)
                  toObj->SetRealParameter(toId, fromObj->GetRealParameter(wxT("Value")));
               // Added to fix GMAT XYPlot1.IndVar = Var; (loj: 2008.08.01)
               else if (toType == Gmat::OBJECT_TYPE && toObj->IsOfType(Gmat::XY_PLOT))
                  toObj->SetStringParameter(toId, fromObj->GetName());
               else
                  errorCond = true;
            }
            else
            {
               if (toType == Gmat::OBJECT_TYPE || toType == Gmat::OBJECTARRAY_TYPE)
                  toObj->SetStringParameter(toId, fromObj->GetName());
               else
                  errorCond = true;
            }
            
            if (errorCond)
            {
               InterpreterException ex
                  (wxT("The value of \"") + fromObj->GetName() + wxT("\" for field \"") + toProp +
                   wxT("\" on object ") + wxT("\"") + toOwner->GetName() + wxT("\" is not an allowed value"));
               HandleError(ex);
               return false;
            }
         }
      }
      else
      {
         #ifdef DEBUG_SET
         MessageInterface::ShowMessage
            (wxT("   Setting objType=%s, objName=%s\n"), fromTypeName.c_str(),
             fromObj->GetName().c_str());
         #endif
         
         toObj->SetStringParameter(toProp, fromObj->GetName());
         if (toObj->IsOwnedObject(toId))
         {
            toObj->SetRefObject(fromObj, fromObj->GetType(), fromObj->GetName());
            
            // Since CoordinateSystem::SetRefObject() clones AxisSystem, delete it from here
            if (toObj->GetType() == Gmat::COORDINATE_SYSTEM &&
                (fromObj->GetType() == Gmat::AXIS_SYSTEM))
            {
               #ifdef DEBUG_MEMORY
               MemoryTracker::Instance()->Remove
                  (fromObj, wxT("oldLocalAxes"), wxT("Interpreter::SetPropertyToObject()"),
                   wxT("deleting oldLocalAxes"));
               #endif
               delete fromObj;
               fromObj = NULL;
            }
         }
      }
   }
   catch (BaseException &ex)
   {
      HandleError(ex);
      return false;
   }
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetPropertyToObject() returning true\n"));
   #endif
   
   return true;
}


//-------------------------------------------------------------------------------
// bool SetPropertyToProperty(GmatBase *toOwner, const wxString &toProp,
//                            GmatBase *fromOwner, const wxString &fromProp)
//-------------------------------------------------------------------------------
bool Interpreter::SetPropertyToProperty(GmatBase *toOwner, const wxString &toProp,
                                        GmatBase *fromOwner, const wxString &fromProp)
{
   debugMsg = wxT("In SetPropertyToProperty()");
   bool retval = true;
   errorMsg1 = wxT("");
   errorMsg2 = wxT("");
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("SetPropertyToProperty() toOwner=%s<%s>, toProp=<%s>, fromOwner=<%s>, fromProp=<%s>\n"),
       toOwner->GetName().c_str(), toOwner->GetTypeName().c_str(), toProp.c_str(),
       fromOwner->GetName().c_str(), fromProp.c_str());
   #endif
   
   Integer toId = -1;
   Gmat::ParameterType toType = Gmat::UNKNOWN_PARAMETER_TYPE;
   wxString lhs = toOwner->GetName() + wxT(".") + toProp;
   wxString rhs = fromOwner->GetName() + wxT(".") + fromProp;
   wxString value;
   Parameter *lhsParam = NULL;
   Parameter *rhsParam = NULL;
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage(wxT("   lhs=%s, rhs=%s\n"), lhs.c_str(), rhs.c_str());
   #endif

   //-----------------------------------
   // try LHS property
   //-----------------------------------
   
   try
   {
      GmatBase *toObj = NULL;
      FindPropertyID(toOwner, toProp, &toObj, toId, toType);
   }
   catch (BaseException &)
   {
      #ifdef DEBUG_SET
      MessageInterface::ShowMessage
         (wxT("   Parameter ID of '%s' not found. So create a parameter '%s'\n"),
          toProp.c_str(), lhs.c_str());
      #endif
      lhsParam = CreateSystemParameter(lhs);
   }
   
   //-----------------------------------
   // try RHS property
   //-----------------------------------
   // try create parameter first if rhs type is OBJECT_TYPE
   if (toType == Gmat::OBJECT_TYPE)
      rhsParam = CreateSystemParameter(rhs);
   
   Integer fromId = -1;
   Gmat::ParameterType fromType = Gmat::UNKNOWN_PARAMETER_TYPE;
   bool isRhsProperty = true;
   
   try
   {
      fromId = fromOwner->GetParameterID(fromProp);   
      fromType = fromOwner->GetParameterType(fromId);
   }
   catch (BaseException &)
   {
      isRhsProperty = false;
      fromType = Gmat::STRING_TYPE;
   }
   
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("   toId=%d, toType=%d, fromId=%d, fromType=%d, lhsParam=%p, rhsParam=%p\n"),
       toId, toType, fromId, fromType, lhsParam, rhsParam);
   #endif
   
   //-----------------------------------
   // now set value
   //-----------------------------------
   
   if (lhsParam != NULL && rhsParam != NULL)
   {
      SetObjectToObject(lhsParam, rhsParam, fromProp);
   }
   else if (lhsParam == NULL && rhsParam != NULL)
   {
      if (toType == rhsParam->GetReturnType())
      {
         value = rhsParam->ToString();
         retval = SetProperty(toOwner, toId, toType, value);
      }
      else
      {
         retval = SetProperty(toOwner, toId, toType, rhs);
      }
   }
   else if (lhsParam != NULL && rhsParam == NULL)
   {
      if (lhsParam->GetReturnType() == fromType)
      {
         value = GetPropertyValue(fromOwner, fromId);
         lhsParam->SetString(value); 
         retval = true;
      }
   }
   else if (lhsParam == NULL && rhsParam == NULL)
   {
      if (toType == fromType)
      {
         if (toType == Gmat::STRING_TYPE || toType == Gmat::ENUMERATION_TYPE ||
             toType == Gmat::FILENAME_TYPE)
         {
            if (isRhsProperty)
            {
               value = GetPropertyValue(fromOwner, fromId);
               retval = SetPropertyValue(toOwner, toId, toType, value);
            }
            else
            {
               retval = SetPropertyValue(toOwner, toId, toType, rhs);
            }
         }
         else
         {
            value = GetPropertyValue(fromOwner, fromId);
            retval = SetProperty(toOwner, toId, toType, value);
         }
      }
      else
      {
         retval = SetProperty(toOwner, toId, toType, rhs);
      }
   }
   
   if (!retval)
   {
      if (errorMsg1 == wxT(""))
      {
         InterpreterException ex
            (wxT("The field name \"") + fromProp + wxT("\" on object ") + toOwner->GetName() +
             wxT(" is not permitted"));
         HandleError(ex);
      }
      else
      {
         InterpreterException ex
            (errorMsg1 + wxT("field \"") + toProp + wxT("\" on object ") + wxT("\"") +
             toOwner->GetName() + wxT("\" is not an allowed value.") + errorMsg2);
         HandleError(ex);
      }
   }
   
   return retval;
}


//-------------------------------------------------------------------------------
// bool SetPropertyToArray(GmatBase *toOwner, const wxString &toProp,
//                         const wxString &fromArray)
//-------------------------------------------------------------------------------
bool Interpreter::SetPropertyToArray(GmatBase *toOwner, const wxString &toProp,
                                     const wxString &fromArray)
{
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetPropertyToArray() toOwner=%s, toProp=%s, fromArray=%s\n"),
       toOwner->GetName().c_str(), toProp.c_str(), fromArray.c_str());
   #endif
   
   debugMsg = wxT("In SetPropertyToArray()");
   Integer toId = -1;
   Gmat::ParameterType toType = Gmat::UNKNOWN_PARAMETER_TYPE;
   
   // Check for property id
   try
   {
      toId = toOwner->GetParameterID(toProp);
      toType = toOwner->GetParameterType(toId);
   }
   catch (BaseException &ex)
   {
      HandleError(ex);
      return false;
   }
   
   // Property type must be Real type, so check
   if (toType != Gmat::REAL_TYPE)
   {
      InterpreterException ex
         (wxT("The value of \"") + fromArray + wxT("\" for field \"") + toProp +
          wxT("\" on object ") + wxT("\"") + toOwner->GetName() + wxT("\" is not an allowed value"));
      HandleError(ex);
      return false;
   }
      
   // Now try to set array to property
   Integer row, col;
   Real rval = GetArrayValue(fromArray, row, col);
   
   try
   {
      toOwner->SetRealParameter(toId, rval);
   }
   catch (BaseException &e)
   {
      HandleError(e);
      return false;
   }
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetPropertyToArray() exiting. rval=%f, row=%d, col=%d, \n"),
       rval, row, col);
   #endif
   
   return true;
}


//-------------------------------------------------------------------------------
// bool SetPropertyToValue(GmatBase *toOwner, const wxString &toProp,
//                         const wxString &value)
//-------------------------------------------------------------------------------
bool Interpreter::SetPropertyToValue(GmatBase *toOwner, const wxString &toProp,
                                     const wxString &value)
{
   debugMsg = wxT("In SetPropertyToValue()");
   bool retval = false;
   errorMsg1 = wxT("");
   errorMsg2 = wxT("");
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetPropertyToValue() objType=%s, objName=%s, toProp=%s, ")
       wxT("value=%s\n"), toOwner->GetTypeName().c_str(), toOwner->GetName().c_str(),
       toProp.c_str(), value.c_str());
   #endif
   
   if (toOwner->GetType() == Gmat::ODE_MODEL)
   {
      retval = SetForceModelProperty(toOwner, toProp, value, NULL);
   }
   else if (toOwner->GetType() == Gmat::MEASUREMENT_MODEL)
   {
      retval = SetMeasurementModelProperty(toOwner, toProp, value);
   }
   else if ((toOwner->GetType() == Gmat::DATASTREAM) ||
            (toOwner->GetType() == Gmat::DATA_FILE))
   {
      retval = SetDataStreamProperty(toOwner, toProp, value);
   }
   else if (toOwner->GetType() == Gmat::SOLAR_SYSTEM)
   {
      retval = SetSolarSystemProperty(toOwner, toProp, value);
   }
   else
   {
      StringArray parts = theTextParser.SeparateDots(toProp);
      
      // if property has multiple dots, handle separately
      if (parts.size() > 1)
      {
         retval = SetComplexProperty(toOwner, toProp, value);
      }
      else
      {
         GmatBase *toObj = NULL;
         Integer toId = -1;
         Gmat::ParameterType toType;
         
         FindPropertyID(toOwner, toProp, &toObj, toId, toType);
         
         if (toId == Gmat::PARAMETER_REMOVED)
         {
            InterpreterException ex
               (wxT("The field name \"") + toProp + wxT("\" on object ") + wxT("\"") +
                toOwner->GetName() + wxT("\" is no longer in use"));
            HandleError(ex, true, true);
            ignoreError = true;
            return false;
         }
         
         if (toObj == NULL)
         {
            if (parsingDelayedBlock)
            {
               InterpreterException ex
                  (wxT("The field name \"") + toProp + wxT("\" on object \"") + toOwner->GetName() +
                   wxT("\" is not permitted"));
               HandleErrorMessage(ex, lineNumber, currentLine, true);
               return false;
            }
            
            delayedBlocks.push_back(currentBlock);
            wxString lineNumStr = GmatStringUtil::ToString(theReadWriter->GetLineNumber());
            delayedBlockLineNumbers.push_back(lineNumStr);
            
            #ifdef DEBUG_SET
            MessageInterface::ShowMessage
               (wxT("   ===> added to delayed blocks: line:%s, %s\n"), lineNumStr.c_str(),
                currentBlock.c_str());
            #endif
            
            return true;
         }
         
         retval = SetProperty(toObj, toId, toType, value);
      }
   }
   
   if (retval == false && !ignoreError)
   {
      if (errorMsg1 == wxT(""))
      {
         InterpreterException ex
            (wxT("The value of \"") + value + wxT("\" for field \"") + toProp + wxT("\" on object ") + wxT("\"") +
             toOwner->GetName() + wxT("\" is not permitted"));
         HandleError(ex);
      }
      else
      {
         InterpreterException ex
            (errorMsg1 + wxT("field \"") + toProp + wxT("\" on object ") + wxT("\"") +
             toOwner->GetName() + wxT("\" is not an allowed value.") + errorMsg2);
         HandleError(ex);
      }
   }
   
   if (ignoreError)
      ignoreError = false;
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetPropertyToValue() returning retval=%d\n"), retval);
   #endif
   
   return retval;
}


//-------------------------------------------------------------------------------
// bool SetArrayToObject(GmatBase *toArrObj, const wxString &toArray,
//                       GmatBase *fromObj)
//-------------------------------------------------------------------------------
bool Interpreter::SetArrayToObject(GmatBase *toArrObj, const wxString &toArray,
                                   GmatBase *fromObj)
{
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetArrayToObject() toArrObj=%s, toArray=%s, fromObj=%s\n"),
       toArrObj->GetName().c_str(), toArray.c_str(), fromObj->GetName().c_str());
   #endif
   
   debugMsg = wxT("In SetArrayToObject()");
   
   if (fromObj->GetTypeName() != wxT("Variable"))
   {
      //InterpreterException ex
      //   (wxT("Cannot set object other than Variable or Array element."));
      //InterpreterException ex
      //   (wxT("Setting object \"") + fromObj->GetName() + wxT("\" to an array \"") + toArray +
      //    wxT("\" is not permitted."));
      InterpreterException ex
         (wxT("Setting an array \"") + toArray + wxT("\" to an object \"") + fromObj->GetName() + 
          wxT("\" is not permitted."));
      HandleError(ex);
      return false;
   }
   
   Real rval = fromObj->GetRealParameter(wxT("Value"));
   
   Integer row, col;
   Parameter *param = GetArrayIndex(toArray, row, col);
   if (param == NULL)
      return false;
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("   SetArrayToObject()rval=%f, row=%d, col=%d\n"), rval, row, col);
   #endif
   
   try
   {
      toArrObj->SetRealParameter(wxT("SingleValue"), rval, row, col);
   }
   catch (BaseException &e)
   {
      HandleError(e);
      return false;
   }
   
   // Set InitialValue so when it is written out, it will have original string value (LOJ: 2010.09.21)
   toArrObj->SetStringParameter(wxT("InitialValue"), toArray + wxT("=") + fromObj->GetName());
   return true;
}


//-------------------------------------------------------------------------------
// bool SetArrayToProperty(GmatBase *toArrObj, const wxString &toArray,
//                         GmatBase *fromOwner, const wxString &fromProp)
//-------------------------------------------------------------------------------
bool Interpreter::SetArrayToProperty(GmatBase *toArrObj, const wxString &toArray,
                                     GmatBase *fromOwner, const wxString &fromProp)
{
   #ifdef DEBGU_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetArrayToProperty() toArrObj=%s, toArray=%s, fromOwner=%s, ")
       wxT("fromProp=%s\n"), toArrObj->GetName().c_str(), toArray.c_str(),
       fromOwner->GetName().c_str(), fromProp.c_str());
   #endif
   
   debugMsg = wxT("In SetArrayToProperty()");
   
   // get object parameter id
   Integer fromId = fromOwner->GetParameterID(fromProp);
   
   if (fromOwner->GetParameterType(fromId) != Gmat::REAL_TYPE)
   {
      //InterpreterException ex
      //   (wxT("Setting non-Real type of \"") + fromProp + wxT("\" to an Array element \"") +
      //    toArray + wxT("\" is not allowed"));
      InterpreterException ex
         (wxT("Setting an array element \"") + toArray + wxT("\" to \"") + fromProp +
          wxT("\" is not allowed"));
      HandleError(ex);
      return false;
   }
   
   Real rval = fromOwner->GetRealParameter(fromId);
   
   Integer row, col;
   Parameter *param = GetArrayIndex(toArray, row, col);
   if (param == NULL)
      return false;
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("   SetArrayToProperty()rval=%f, row=%d, col=%d\n"), rval, row, col);
   #endif

   try
   {
      toArrObj->SetRealParameter(wxT("SingleValue"), rval, row, col);
   }
   catch (BaseException &e)
   {
      HandleError(e);
      return false;
   }
   
   // Set InitialValue so when it is written out, it will have original string value (LOJ: 2010.09.21)
   toArrObj->SetStringParameter(wxT("InitialValue"), toArray + wxT("=") + fromProp);
   return true;
}


//-------------------------------------------------------------------------------
// bool SetArrayToArray(GmatBase *toArrObj, const wxString &toArray,
//                      GmatBase *fromArrObj, const wxString &fromArray)
//-------------------------------------------------------------------------------
/**
 * Sets Array to Array, such as toArray = fromArray
 */
//-------------------------------------------------------------------------------
bool Interpreter::SetArrayToArray(GmatBase *toArrObj, const wxString &toArray,
                                  GmatBase *fromArrObj, const wxString &fromArray)
{
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetArrayToArray() toArrObj=%s, toArray=%s, ")
       wxT("fromArrObj=%s, fromArray=%s\n"), toArrObj->GetName().c_str(),
       toArray.c_str(), fromArrObj->GetName().c_str(), fromArray.c_str());
   #endif
   
   debugMsg = wxT("In SetArrayToArray()");
   Integer rowFrom, colFrom;
   Integer rowTo, colTo;
   
   Parameter *param = GetArrayIndex(toArray, rowTo, colTo);
   if (param == NULL)
      return false;
   
   param = GetArrayIndex(fromArray, rowFrom, colFrom);
   if (param == NULL)
      return false;
   
   Real rval = GetArrayValue(fromArray, rowFrom, colFrom);
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("   SetArrayToArray() rval=%f, rowFrom=%d, colFrom=%d, \n"),
       rval, rowFrom, colFrom);
   MessageInterface::ShowMessage
      (wxT("   SetArrayToArray() rowTo=%d, colTo=%d\n"), rowTo, colTo);
   #endif
   
   try
   {
      if (fromArray[0] == '-')
         toArrObj->SetRealParameter(wxT("SingleValue"), -rval, rowTo, colTo);
      else   
         toArrObj->SetRealParameter(wxT("SingleValue"), rval, rowTo, colTo);
   }
   catch (BaseException &e)
   {
      HandleError(e);
      return false;
   }
   
   // Set InitialValue so when it is written out, it will have original string value (LOJ: 2010.09.21)
   toArrObj->SetStringParameter(wxT("InitialValue"), toArray + wxT("=") + fromArray);
   return true;
}


//-------------------------------------------------------------------------------
// bool SetArrayToValue(GmatBase *array, const wxString &toArray,
//                      const wxString &value)
//-------------------------------------------------------------------------------
bool Interpreter::SetArrayToValue(GmatBase *array, const wxString &toArray,
                                  const wxString &value)
{
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetArrayToValue() array=%s, toArray=%s, value=%s\n"),
       array->GetName().c_str(), toArray.c_str(), value.c_str());
   #endif
   
   debugMsg = wxT("In SetArrayToValue()");
   Integer row, col;
   Real rval;
   
   Parameter *param = GetArrayIndex(toArray, row, col);
   if (param == NULL)
      return false;
   
   if (GmatStringUtil::ToReal(value, rval, true))
   {
      #ifdef DEBUG_SET
      MessageInterface::ShowMessage
         (wxT("   SetArrayToValue() rval=%f, row=%d, col=%d\n"), rval, row, col);
      #endif

      try
      {
         array->SetRealParameter(wxT("SingleValue"), rval, row, col);
      }
      catch (BaseException &)
      {
         InterpreterException ex(wxT("Index exceeds matrix dimensions"));
         HandleError(ex);
         return false;
      }
   }
   else
   {
      //InterpreterException ex
      //   (wxT("Setting \"") + value + wxT("\" to an object \"") + toArray +
      //    wxT("\" is not allowed"));
      InterpreterException ex
         (wxT("Setting an object \"") + toArray + wxT("\" to \"") + value +
          wxT("\" is not allowed"));
      HandleError(ex);
      return false;
   }
   
   // Set InitialValue so when it is written out, it will have original string value (LOJ: 2010.09.21)
   array->SetStringParameter(wxT("InitialValue"), toArray + wxT("=") + value);
   return true;
}


//------------------------------------------------------------------------------
// bool SetPropertyValue(GmatBase *obj, const Integer id,
//                       const Gmat::ParameterType type,
//                       const wxString &value, const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets parameters on GMAT objects.
 * 
 * @param  obj    Pointer to the object that owns the property.
 * @param  id     ID for the property.
 * @param  type   Type for the property.
 * @param  value  Value of the property.
 * @param  index  Index of the property in array.
 * 
 * @return true if the property is set, false otherwise.
 */
//------------------------------------------------------------------------------
bool Interpreter::SetPropertyValue(GmatBase *obj, const Integer id,
                                   const Gmat::ParameterType type,
                                   const wxString &value,
                                   const Integer index, const Integer colIndex)
{
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetPropertyValue() obj=<%s>, id=%d, type=%d, value=<%s>, index=%d\n"),
       obj->GetName().c_str(), id, type, value.c_str(), index);
   #endif
   
   debugMsg = wxT("In SetPropertyValue()");
   bool retval = false;
   wxString valueToUse = value;
   CheckForSpecialCase(obj, id, valueToUse);
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("   propertyType=%s\n"),
       type == -1 ? wxT("UNKNOWN_TYPE") : GmatBase::PARAM_TYPE_STRING[type].c_str());
   #endif
   
   if (type == -1)
      return false;
   
   switch (type)
   {
   case Gmat::OBJECT_TYPE:
   case Gmat::OBJECTARRAY_TYPE:
      {
         return SetPropertyObjectValue(obj, id, type, valueToUse, index);
      }
   case Gmat::ENUMERATION_TYPE:
   case Gmat::FILENAME_TYPE:
   case Gmat::STRING_TYPE:
   case Gmat::STRINGARRAY_TYPE:
      {
         return SetPropertyStringValue(obj, id, type, valueToUse, index);
      }
   case Gmat::INTEGER_TYPE:
   case Gmat::UNSIGNED_INT_TYPE:
      {
         Integer ival;
         if (GmatStringUtil::ToInteger(valueToUse, ival))
         {
            #ifdef DEBUG_SET
            MessageInterface::ShowMessage
               (wxT("   Calling '%s'->SetIntegerParameter(%d, %d)\n"),
                obj->GetName().c_str(), id, ival);
            #endif
            
            obj->SetIntegerParameter(id, ival);
            retval = true;
         }
         else
         {
            errorMsg1 = errorMsg1 + wxT("The value of \"") + valueToUse + wxT("\" for ");
            errorMsg2 = wxT(" Only integer number is allowed");
         }
         break;
      }
   case Gmat::UNSIGNED_INTARRAY_TYPE:
      {
         Integer ival;
         if (GmatStringUtil::ToInteger(valueToUse, ival))
         {
            #ifdef DEBUG_SET
            MessageInterface::ShowMessage
               (wxT("   Calling '%s'->SetUnsignedIntParameter(%d, %d, %d)\n"),
                obj->GetName().c_str(), id, ival, index);
            #endif
            
            obj->SetUnsignedIntParameter(id, ival, index);
            retval = true;
         }
         else
         {
            errorMsg1 = errorMsg1 + wxT("The value of \"") + valueToUse + wxT("\" for ");
            errorMsg2 = wxT(" Only integer number is allowed");
         }
         break;
      }
   case Gmat::REAL_TYPE:
   case Gmat::RVECTOR_TYPE:
      {
         Real rval;
         if (GmatStringUtil::ToReal(valueToUse, rval, true))
         {
            #ifdef DEBUG_SET
            wxString rvalStr =
               GmatStringUtil::ToString(rval, false, false, true, 17, 16);
            MessageInterface::ShowMessage
               (wxT("   Calling <%s>'%s'->SetRealParameter(%d, %s)\n"), obj->GetTypeName().c_str(),
                obj->GetName().c_str(), id, rvalStr.c_str());
            #endif
            
            if (type == Gmat::REAL_TYPE)
               obj->SetRealParameter(id, rval);
            else
               obj->SetRealParameter(id, rval, index);
            
            retval = true;
         }
         else
         {
            errorMsg1 = errorMsg1 + wxT("The value of \"") + valueToUse + wxT("\" for ");
            errorMsg2 = wxT(" The allowed value is Real number");
         }
         break;
      }
   case Gmat::RMATRIX_TYPE:
      {
         #ifdef DEBUG_SET
            MessageInterface::ShowMessage(wxT("Setting Rmatrix[%d, %d] from the data")
                  wxT(" \"%s\" with ID %d\n"), index, colIndex, value.c_str(), id);
         #endif
         Real rval;
         if (GmatStringUtil::ToReal(valueToUse, rval, true))
         {
            obj->SetRealParameter(id, rval, index, colIndex);
         }

         break;
      }
   case Gmat::BOOLEAN_TYPE:
      {
         bool tf;
         if (GmatStringUtil::ToBoolean(valueToUse, tf))
         {
            #ifdef DEBUG_SET
            MessageInterface::ShowMessage
               (wxT("   Calling '%s'->SetBooleanParameter(%d, %d)\n"),
                obj->GetName().c_str(), id, tf);
            #endif
            
            obj->SetBooleanParameter(id, tf);
            retval = true;
         }
         else
         {
            errorMsg1 = errorMsg1 + wxT("The value of \"") + valueToUse + wxT("\" for ");
            errorMsg2 = wxT(" The allowed values are: [true false]");
         }
         break;
      }
   case Gmat::BOOLEANARRAY_TYPE:
      {
         bool tf;
         if (GmatStringUtil::ToBoolean(valueToUse, tf))
         {
            #ifdef DEBUG_SET
            MessageInterface::ShowMessage
               (wxT("   Calling %s->SetBooleanParameter(%d, %s, %d) (for BooleanArray)\n"), 
                obj->GetName().c_str(), id, valueToUse.c_str(), index);
            #endif
            
            retval = obj->SetBooleanParameter(id, tf, index);
         }
         else
         {
            errorMsg1 = errorMsg1 + wxT("The value of \"") + valueToUse + wxT("\" for ");
            errorMsg2 = wxT(" The allowed values are: [true false]");
         }
         break;
      }
   case Gmat::ON_OFF_TYPE:
      {
         #ifdef DEBUG_SET
         MessageInterface::ShowMessage
            (wxT("   Calling '%s'->SetOnOffParameter(%d, %s)\n"),
             obj->GetName().c_str(), id, valueToUse.c_str());
         #endif
         
         if (valueToUse == wxT("On") || valueToUse == wxT("Off"))
         {
            retval = obj->SetOnOffParameter(id, valueToUse);
         }
         else
         {
            errorMsg1 = errorMsg1 + wxT("The value of \"") + valueToUse + wxT("\" for ");
            errorMsg2 = wxT(" The allowed values are: [On Off]");
         }
         break;
      }
   default:
      InterpreterException ex
         (wxT("Interpreter::SetPropertyValue() Cannot handle the type: ") +
          GmatBase::PARAM_TYPE_STRING[type] + wxT(" yet.\n"));
      HandleError(ex);
   }
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetPropertyValue() returning retval=%d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool SetPropertyObjectValue(GmatBase *obj, const Integer id, ...)
//------------------------------------------------------------------------------
bool Interpreter::SetPropertyObjectValue(GmatBase *obj, const Integer id,
                                         const Gmat::ParameterType type,
                                         const wxString &value,
                                         const Integer index)
{
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetPropertyObjectValue() obj=<%s> '%s', id=%d, type=%d, value='%s', ")
       wxT("index=%d\n"), obj->GetTypeName().c_str(), obj->GetName().c_str(), id, type,
       value.c_str(), index);
   #endif
   
   debugMsg = wxT("In SetPropertyObjectValue()");
   Parameter *param = NULL;

   // Remove enclosing single quotes first (LOJ: 2009.06.08)
   wxString valueToUse = value;
   valueToUse = GmatStringUtil::RemoveEnclosingString(valueToUse, wxT("'"));
   
   // Try creating Parameter first if it is not ObjectType
   if (!IsObjectType(valueToUse))
   {
      // It is not a one of object types, so create parameter
      param = CreateSystemParameter(valueToUse);
      
      #ifdef DEBUG_SET
      if (param)
         MessageInterface::ShowMessage
            (wxT("   param=(%p)%s type=%s returnType=%d\n"), param,
             param->GetName().c_str(), param->GetTypeName().c_str(),
             param->GetReturnType());
      #endif
   }
   else
   {
      // It is object type so get parameter (Bug 743 fix)
      param = theModerator->GetParameter(valueToUse);
      #ifdef DEBUG_SET
      MessageInterface::ShowMessage
         (wxT("   theModerator->GetParameter() returned %p\n"), param);
      #endif
   }
   
   try
   {
      if (param != NULL)
      {
         // Other than Subscriber, it can only take STRING_TYPE parameter
         if (param->GetReturnType() == Gmat::STRING_TYPE ||
             obj->IsOfType(Gmat::SUBSCRIBER))
         {
            #ifdef DEBUG_SET
            MessageInterface::ShowMessage
               (wxT("   Calling '%s'->SetStringParameter(%d, %s)\n"),
                obj->GetName().c_str(), id, valueToUse.c_str());
            #endif
            
            // Let base code check for the invalid values
            obj->SetStringParameter(id, valueToUse);
         }
         else
         {
            errorMsg1 = errorMsg1 + wxT("The value of \"") + valueToUse + wxT("\" for ");
            errorMsg2 = wxT("  The allowed value is Object Name");
            return false;
         }
      }
      else
      {
         // check if value is a number
         Real rval;
         Integer ival;
         if (GmatStringUtil::ToReal(valueToUse, rval, true) ||
             GmatStringUtil::ToInteger(valueToUse, ival, true))
         {
            #ifdef DEBUG_SET
            MessageInterface::ShowMessage
               (wxT("   '%s' is a Real or Integer value\n"), valueToUse.c_str());
            #endif
            
            // Handle special case for OrbitView.
            // ViewPointReference, ViewPointVector, and ViewDirection can have 
            // both vector and object name.
            if (obj->IsOfType(Gmat::ORBIT_VIEW))
            {
               obj->SetStringParameter(id, valueToUse, index);
            }
            else
            {
               errorMsg1 = errorMsg1 + wxT("The value of \"") + valueToUse + wxT("\" for ");
               errorMsg2 = wxT("  The allowed value is Object Name");
               return false;
            }
         }
         
         // check if value is an object name
         GmatBase *configObj = FindObject(valueToUse);
         
         // check if object name is the same as property type name (loj: 2008.11.06)
         // if so, we need to set configObj to NULL so that owned object can be
         // created if needed.
         // ex) Create Propagator RungeKutta89;
         //     GMAT  RungeKutta89.Type = RungeKutta89;
         if (configObj && obj->IsOwnedObject(id))
         {
            ObjectTypeArray refTypes = obj->GetRefObjectTypeArray();
            if (configObj->GetType() != refTypes[id])
               configObj = NULL;
         }
         
         if (configObj)
         {
            #ifdef DEBUG_SET
            MessageInterface::ShowMessage
               (wxT("   Found the object type of %s\n"), configObj->GetTypeName().c_str());
            #endif
            
            // Set as String parameter, so it can be validated in FinalPass()
            bool retval = true;
            if (index != -1)
            {
               #ifdef DEBUG_SET
               MessageInterface::ShowMessage
                  (wxT("   Calling '%s'->SetStringParameter(%d, %s, %d)\n"),
                   obj->GetName().c_str(), id, valueToUse.c_str(), index);
               #endif
               
               retval = obj->SetStringParameter(id, valueToUse, index);
            }
            
            // if it has no index or failed setting with index, try without index
            if (index == -1 || !retval)
            {
               #ifdef DEBUG_SET
               MessageInterface::ShowMessage
                  (wxT("   Calling '%s'->SetStringParameter(%d, %s)\n"),
                   obj->GetName().c_str(), id, valueToUse.c_str());
               #endif
               
               obj->SetStringParameter(id, valueToUse);
            }
         }
         else
         {
            #ifdef DEBUG_SET
            MessageInterface::ShowMessage
               (wxT("   Object not found, so try creating owned object\n"));
            #endif
            
            // Create Owned Object, if it is valid owned object type
            GmatBase *ownedObj = NULL;
            bool skipCreate = false;
            if (obj->IsOwnedObject(id))
            {
               // Handle named owned Propagator object for PropSetup
               // since Integrator is not created by Create command
               wxString ownedName = wxT("");
               if (obj->IsOfType(Gmat::PROP_SETUP))
               {
                  ownedName = valueToUse;
                  if (obj->GetParameterText(id) == wxT("FM"))
                  {
                     obj->SetStringParameter(id, ownedName);
                     skipCreate = true;
                  }
               }
               if (!skipCreate)
               {
                  ownedObj = CreateObject(valueToUse, ownedName, 0);
                  if (ownedObj == NULL)
                     MessageInterface::ShowMessage
                        (wxT("*** WARNING *** Owned object %s was not created for ")
                         wxT("'%s'; using default\n"), ownedName.c_str(),
                         obj->GetName().c_str());
               }
            }
            
            #ifdef DEBUG_SET
            if (ownedObj)
               MessageInterface::ShowMessage
                  (wxT("   Created ownedObjType: %s\n"), ownedObj->GetTypeName().c_str());
            #endif
            
            if (ownedObj)
            {
               #ifdef DEBUG_SET
               MessageInterface::ShowMessage
                  (wxT("   Calling '%s'->SetRefObject(%s(%p), %d)\n"), obj->GetName().c_str(),
                   ownedObj->GetTypeName().c_str(), ownedObj, ownedObj->GetType());
               #endif
               
               obj->SetRefObject(ownedObj, ownedObj->GetType(), ownedObj->GetName());
               
               // Since PropSetup::SetRefObject() clones Propagator and
               // CoordinateSystem::SetRefObject() clones AxisSystem, delete it from here
               // (LOJ: 2009.03.03)
               if ((obj->GetType() == Gmat::PROP_SETUP &&
                    (ownedObj->GetType() == Gmat::PROPAGATOR)) ||
                   (obj->GetType() == Gmat::COORDINATE_SYSTEM &&
                    (ownedObj->GetType() == Gmat::AXIS_SYSTEM)))
               {
                  #ifdef DEBUG_MEMORY
                  MemoryTracker::Instance()->Remove
                     (ownedObj, ownedObj->GetName(), wxT("Interpreter::SetPropertyObjectValue()"),
                      wxT("deleting oldOwnedObject"));
                  #endif
                  delete ownedObj;
                  ownedObj = NULL;
               }
            }
            else
            {
               // Special case of InternalODEModel in script
               // Since PropSetup no longer creates InternalODEModel
               // create it here (loj: 2008.11.06)
               if (!skipCreate)
               {
                  if (valueToUse == wxT("InternalODEModel"))
                  {
                     ownedObj = CreateObject(wxT("ForceModel"), valueToUse);
                     obj->SetRefObject(ownedObj, ownedObj->GetType(), valueToUse);
                  }
                  else
                  {
                     // Set as String parameter, so it can be caught in FinalPass()
                     #ifdef DEBUG_SET
                     MessageInterface::ShowMessage
                        (wxT("   Calling '%s'->SetStringParameter(%d, %s) so it can be ")
                         wxT("caught in FinalPass()\n"),
                         obj->GetName().c_str(), id, valueToUse.c_str());
                     #endif

                     obj->SetStringParameter(id, valueToUse);
                  }
               }
            }
         }
      }
      
      #ifdef DEBUG_SET
      MessageInterface::ShowMessage
         (wxT("Interpreter::SetPropertyObjectValue() returning true\n"));
      #endif
      
      return true;
   }
   catch (BaseException &ex)
   {
      HandleError(ex);
      ignoreError = true;
      
      #ifdef DEBUG_SET
      MessageInterface::ShowMessage
         (wxT("Interpreter::SetPropertyObjectValue() returning false\n"));
      #endif
      
      return false;
   }
}


//------------------------------------------------------------------------------
// bool SetPropertyStringValue(GmatBase *obj, const Integer id, ...)
//------------------------------------------------------------------------------
bool Interpreter::SetPropertyStringValue(GmatBase *obj, const Integer id,
                                         const Gmat::ParameterType type,
                                         const wxString &value,
                                         const Integer index)
{
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetPropertyStringValue() obj=%s, id=%d, type=%d, value=%s, ")
       wxT("index=%d\n"), obj->GetName().c_str(), id, type, value.c_str(), index);
   #endif
   
   debugMsg = wxT("In SetPropertyStringValue()");
   bool retval = true;
   wxString valueToUse = value;
   
   switch (type)
   {
   case Gmat::ENUMERATION_TYPE:
   case Gmat::FILENAME_TYPE:
   case Gmat::STRING_TYPE:
      {
         // remove enclosing quotes if used
         valueToUse = GmatStringUtil::RemoveEnclosingString(valueToUse, wxT("'"));
         
         try
         {
            if (index >= 0)
            {
               #ifdef DEBUG_SET
               MessageInterface::ShowMessage
                  (wxT("   Calling %s->SetStringParameter(%d, %s, %d)\n"),
                   obj->GetName().c_str(), id, valueToUse.c_str(), index);
               #endif
               
               retval = obj->SetStringParameter(id, valueToUse, index);
            }
            else
            {
               #ifdef DEBUG_SET
               MessageInterface::ShowMessage
                  (wxT("   Calling %s->SetStringParameter(%d, %s)\n"),
                   obj->GetName().c_str(), id, valueToUse.c_str());
               #endif
               
               retval = obj->SetStringParameter(id, valueToUse);
            }
         }
         catch (BaseException &ex)
         {
            HandleError(ex);
            ignoreError = true;
            retval = false;
         }
         break;
      }
   case Gmat::STRINGARRAY_TYPE:
      {         
         try
         {
            // remove enclosing quotes if used
            valueToUse = GmatStringUtil::RemoveEnclosingString(valueToUse, wxT("'"));
            #ifdef DEBUG_SET
            MessageInterface::ShowMessage
               (wxT("   Calling %s->SetStringParameter(%d, %s) (for StringArray)\n"),
                obj->GetName().c_str(), id, valueToUse.c_str());
            #endif
            
            retval = obj->SetStringParameter(id, valueToUse);
         }
         catch (BaseException &)
         {
            #ifdef DEBUG_SET
            MessageInterface::ShowMessage
               (wxT("   Calling %s->SetStringParameter(%d, %s, %d) (for StringArray)\n"), 
                     obj->GetName().c_str(), id,
                     valueToUse.c_str(), index);
            #endif
            
            // try with index
            retval = obj->SetStringParameter(id, valueToUse, index);
         }
         break;
      }
   default:
      break;
   }
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetPropertyStringValue() returning %d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// wxString GetPropertyValue(GmatBase *obj, const Integer id)
//------------------------------------------------------------------------------
wxString Interpreter::GetPropertyValue(GmatBase *obj, const Integer id)
{
   wxString sval;
   
   Gmat::ParameterType type = obj->GetParameterType(id);
   
   if (type == Gmat::OBJECT_TYPE)
   {
      sval = obj->GetStringParameter(id);
   }
   else if (type == Gmat::INTEGER_TYPE)
   {
      sval = GmatStringUtil::ToString(obj->GetIntegerParameter(id));
   }
   else if (type == Gmat::UNSIGNED_INT_TYPE)
   {
      sval = GmatStringUtil::ToString(obj->GetIntegerParameter(id));
   }
//    else if (type == Gmat::UNSIGNED_INTARRAY_TYPE)
//    {
//       sval = GmatStringUtil::ToString(obj->GetIntegerParameter(id));
//    }
   else if (type == Gmat::REAL_TYPE)
   {
      sval = GmatStringUtil::ToString(obj->GetRealParameter(id));
   }
   else if (type == Gmat::STRING_TYPE || type == Gmat::ENUMERATION_TYPE ||
            type == Gmat::FILENAME_TYPE)
   {
      sval = obj->GetStringParameter(id);
   }
//    else if (type == Gmat::STRINGARRAY_TYPE)
//    {
//       sval = obj->GetStringParameter(id));
//    }
   else if (type == Gmat::BOOLEAN_TYPE)
   {
      if (obj->GetBooleanParameter(id))
         sval = wxT("true");
      else
         sval = wxT("false");
   }
   else if (type == Gmat::ON_OFF_TYPE)
   {
      sval = obj->GetOnOffParameter(id);
   }
   
   return sval;
}


//------------------------------------------------------------------------------
// bool SetProperty(GmatBase *obj, const Integer id, const Gmat::ParameterType type
//                  const wxString &value)
//------------------------------------------------------------------------------
/**
 * Sets parameters on GMAT objects.
 * 
 * @param  obj    Pointer to the object that owns the property.
 * @param  id     property ID
 * @param  type   proerty Type
 * @param  value  Value of the property.
 * 
 * @return true if the property is set, false otherwise.
 */
//------------------------------------------------------------------------------
bool Interpreter::SetProperty(GmatBase *obj, const Integer id,
                              const Gmat::ParameterType type,
                              const wxString &value)
{
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetProperty() obj=%s, id=%d, type=%d, value=%s\n"),
       obj->GetName().c_str(), id, type, value.c_str());
   #endif
   
   bool retval = false;
   
   wxString valueToUse = value;
   CheckForSpecialCase(obj, id, valueToUse);

   // require the object to take its prerequisite action before setting the value
   obj->TakeRequiredAction(id);

   #ifdef DEBUG_SET
   MessageInterface::ShowMessage(wxT("   propertyType=%d\n"), obj->GetParameterType(id));
   #endif
   
   StringArray rhsValues;
   Integer count = 0;
   
   // if value has braces or brackets, setting multiple values
   if (value.find(wxT("{")) != value.npos || value.find(wxT("}")) != value.npos)
   {
      // first, check to see if it is a list of strings (e.g. file names);
      // in that case, we do not want to remove spaces inside the strings
      // or use space as a delimiter
      if (value.find(wxT("\'")) != value.npos)
      {
         wxString trimmed = GmatStringUtil::Trim(value);
         wxString inside  = GmatStringUtil::RemoveOuterString(trimmed, wxT("{"), wxT("}"));
         #ifdef DEBUG_SET
         MessageInterface::ShowMessage(wxT("------> found single quotes in %s\n"), value.c_str());
         MessageInterface::ShowMessage(wxT("------> trimmed =  %s\n"), trimmed.c_str());
         MessageInterface::ShowMessage(wxT("------> inside  =  %s\n"), inside.c_str());
         #endif
         rhsValues = GmatStringUtil::SeparateByComma(inside);
      }
      else
      {
         rhsValues = theTextParser.SeparateBrackets(value, wxT("{}"), wxT(" ,"));
      }
   }
   else if (value.find(wxT("[")) != value.npos || value.find(wxT("]")) != value.npos)
   {
      // first, check to see if it is a list of strings (e.g. file names);
      // in that case, we do not want to remove spaces inside the strings
      // or use space as a delimiter
      if (value.find(wxT("\'")) != value.npos)
      {
         wxString trimmed = GmatStringUtil::Trim(value);
         wxString inside = GmatStringUtil::RemoveOuterString(trimmed, wxT("["), wxT("]"));
         rhsValues = GmatStringUtil::SeparateByComma(inside);
      }
      else
      {
         rhsValues = theTextParser.SeparateBrackets(value, wxT("[]"), wxT(" ,"));
      }
   }
   
   count = rhsValues.size();
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage(wxT("   count=%d\n"), count);
   #endif
   
   // If rhs value is an array type, call method for setting whole array
   // or call SetPropertValue() with index
   if (count > 0)
   {
      bool setWithIndex = true;
      // See if object has a method to handle whole array
      if (type == Gmat::BOOLEANARRAY_TYPE)
      {
         setWithIndex = false;
         BooleanArray boolArray = GmatStringUtil::ToBooleanArray(value);
         if (boolArray.size() > 0)
         {
            try
            {
               retval = obj->SetBooleanArrayParameter(id, boolArray);
            }
            catch (BaseException &)
            {
               setWithIndex = true;
            }
         }
      }
      
      if (setWithIndex)
      {
         // Set value with index
         for (int i=0; i<count; i++)
            retval = SetPropertyValue(obj, id, type, rhsValues[i], i);
      }
   }
   else
   {
      retval = SetPropertyValue(obj, id, type, value);
   }
   
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetProperty() returning retval=%d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool SetComplexProperty(GmatBase *obj, const wxString &prop,
//                         const wxString &value)
//------------------------------------------------------------------------------
bool Interpreter::SetComplexProperty(GmatBase *obj, const wxString &prop,
                                     const wxString &value)
{
   bool retval = true;

   #ifdef DEBUG_SET
      MessageInterface::ShowMessage
         (wxT("Interpreter::SetComplexProperty() prop=%s, value=%s\n"),
          prop.c_str(), value.c_str());
   #endif
   
   StringArray parts = theTextParser.SeparateDots(prop);

   if (obj->GetType() == Gmat::SPACECRAFT)
   {
      Spacecraft *sc = (Spacecraft*)obj;
      
      if (parts[0] == wxT("Epoch"))
      {
         sc->SetDateFormat(parts[1]);
         sc->SetEpoch(value);
      }
      else
      {
         if (parts[0] != wxT("Covariance"))
            retval = false;
      }
   }


   if (parts[0] == wxT("Covariance"))
   {
      #ifdef DEBUG_SET
         MessageInterface::ShowMessage(wxT("Setting covariance elements:\n"));
      #endif

      Covariance* covariance = obj->GetCovariance();
      for (UnsignedInt i = 1; i < parts.size(); ++i)
      {
         Integer parmID = obj->GetParameterID(parts[i]);
         Integer covSize = obj->HasParameterCovariances(parmID);
         #ifdef DEBUG_SET
            MessageInterface::ShowMessage(wxT("   %s, with size %d\n"),
                  parts[i].c_str(), covSize);
         #endif

         if (covSize >= 0)
            covariance->AddCovarianceElement(parts[i], obj);
      }

      covariance->ConstructLHS();

      // Check the size of the inputs -- MUST be a square matrix
      StringArray rhsRows;
      if ((value.find(wxT("[")) == value.npos) || (value.find(wxT("]")) == value.npos))
         throw GmatBaseException(wxT("Covariance matrix definition is missing ")
               wxT("square brackets"));

      rhsRows = theTextParser.SeparateBrackets(value, wxT("[]"), wxT(";"));
      UnsignedInt rowCount = rhsRows.size();

      StringArray cells = theTextParser.SeparateSpaces(rhsRows[0]);
      UnsignedInt colCount = cells.size();

      if ((Integer)colCount > covariance->GetDimension())
         throw GmatBaseException(wxT("Input covariance matrix is larger than the ")
               wxT("matrix built from the input array"));

      for (UnsignedInt i = 1; i < rowCount; ++i)
      {
         cells = theTextParser.SeparateSpaces(rhsRows[i]);
       #ifdef DEBUG_SET
            MessageInterface::ShowMessage(wxT("   Found  %d columns in row %d\n"),
                  cells.size(), i+1);
       #endif

         if (cells.size() != rowCount)
            throw InterpreterException(wxT("Row/Column mismatch in the Covariance ")
                  wxT("matrix for ") + obj->GetName());
      }

      #ifdef DEBUG_SET
         MessageInterface::ShowMessage(wxT("Found %d rows and %d columns\n"),
               rowCount, colCount);
      #endif

      Integer id = obj->GetParameterID(parts[0]);
      Gmat::ParameterType type = obj->GetParameterType(id);

      for (UnsignedInt i = 0; i < colCount; ++i)
      {
         if (rowCount != 1)
            cells = theTextParser.SeparateSpaces(rhsRows[i]);
         for (UnsignedInt j = 0; j < colCount; ++j)
            if (i == j)
               SetPropertyValue(obj, id, type, cells[j], i, j);
            else
               // If a single row, it's the diagonal
               if (rowCount == 1)
                  SetPropertyValue(obj, id, type, wxT("0.0"), i, j);
               // Otherwise it's cell[j]
               else
                  SetPropertyValue(obj, id, type, cells[j], i, j);
      }

      #ifdef DEBUG_SET
         MessageInterface::ShowMessage(wxT("Covariance matrix set to:\n"));
         for (UnsignedInt i = 0; i < colCount; ++i)
         {
            MessageInterface::ShowMessage(wxT("   ["));
            for (UnsignedInt j = 0; j < colCount; ++j)
               MessageInterface::ShowMessage(wxT(" %.12lf "), (*obj->GetCovariance())(i,j));
            MessageInterface::ShowMessage(wxT("]\n"));
         }
      #endif
   }

   return retval;
}


//------------------------------------------------------------------------------
// bool SetForceModelProperty(GmatBase *obj, const wxString &prop,
//                            const wxString &value, GmatBase *fromObj)
//------------------------------------------------------------------------------
/**
 * Configures properties for an ODEModel
 *
 * ODEModel configuration is performed through calls to this method.  The method
 * sets general ODEModel parameters, and includes constructor calls for the
 * PhysicalModels that constitute the contributors, through superposition, to
 * the total derivative data at a given state.
 *
 * @param obj     The ODEModel that receives the setting
 * @param prop    The property that is being set
 * @param value   The value for the property
 * @param fromObj Object supplying the value, if used
 *
 * @return true if the property is set, false if not set
 */
//------------------------------------------------------------------------------
bool Interpreter::SetForceModelProperty(GmatBase *obj, const wxString &prop,
                                  const wxString &value, GmatBase *fromObj)
{
   #ifdef DEBUG_SET_FORCE_MODEL
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetForceModelProperty() entered, obj=<%p><%s>'%s', prop=%s, ")
       wxT("value=%s, fromObj=<%p>\n"), obj, obj->GetTypeName().c_str(), obj->GetName().c_str(),
       prop.c_str(), value.c_str(), fromObj);
   #endif
   
   debugMsg = wxT("In SetForceModelProperty()");
   bool retval = false;
   StringArray parts = theTextParser.SeparateDots(prop);
   Integer dotCount = parts.size();
   wxString pmType = parts[dotCount-1];
   Integer id;
   Gmat::ParameterType type;
   
   // Current ForceModel scripting, SRP is on for central body.
   //GMAT FM.CentralBody = Earth;
   //GMAT FM.PrimaryBodies = {Earth, Luna};
   //GMAT FM.PointMasses = {Sun, Jupiter}; 
   //GMAT FM.Drag = None; // deprecated (2011.07.22)
   //GMAT FM.SRP = On;
   //GMAT FM.GravityField.Earth.Degree = 20;
   //GMAT FM.GravityField.Earth.Order = 20;
   //GMAT FM.GravityField.Earth.Model = JGM2.cof;
   //GMAT FM.GravityField.Luna.Degree = 4;
   //GMAT FM.GravityField.Luna.Order = 4;
   //GMAT FM.GravityField.Luna.PotentialFile = LP165P.cof;
   
   // New Drag force script (2011.07.20)
   // GMAT FM.Drag = None; // deprecated
   // FM.Drag.AtmosphereModel = 'MarsGRAM2005'
   // FM.Drag.DensityModel = Mean
   // FM.Drag.InputFile = 'INPUT.nml' 
   // FM.Drag.F107 = 150 
   
   // For future scripting we want to specify body for Drag and SRP
   // e.g. FM.Drag.Earth = JacchiaRoberts;
   //      FM.Drag.Mars = MarsAtmos;
   //      FM.SRP.ShadowBodies = {Earth,Moon}
   
   ODEModel *forceModel = (ODEModel*)obj;
   wxString forceType = ODEModel::GetScriptAlias(pmType);
   wxString centralBodyName = forceModel->GetStringParameter(wxT("CentralBody"));
   
   #ifdef DEBUG_SET_FORCE_MODEL
   MessageInterface::ShowMessage
      (wxT("   pmType=%s, forceType=%s\n"), pmType.c_str(), forceType.c_str());
   #endif
   
   //------------------------------------------------------------
   // Set ForceModel CentralBody
   //------------------------------------------------------------
   if (pmType == wxT("CentralBody"))
   {
      id = obj->GetParameterID(wxT("CentralBody"));
      type = obj->GetParameterType(id);
      retval = SetPropertyValue(obj, id, type, value);
      return retval;
   }
   
   //------------------------------------------------------------
   // Create ForceModel owned PhysicalModel
   //------------------------------------------------------------
   
   else if (pmType == wxT("PrimaryBodies") || pmType == wxT("PointMasses"))
   {
      retval = true;
      StringArray bodies = theTextParser.SeparateBrackets(value, wxT("{}"), wxT(" ,"));
      
      for (UnsignedInt i=0; i<bodies.size(); i++)
      {
         #ifdef DEBUG_SET_FORCE_MODEL
         MessageInterface::ShowMessage(wxT("   bodies[%d]=%s\n"), i, bodies[i].c_str());
         #endif
         
         // We don't want to configure PhysicalModel, so set name after create
         ////PhysicalModel *pm = (PhysicalModel*)CreateObject(forceType, wxT(""));
         wxString forceName = forceType + wxT(".") + bodies[i];
         PhysicalModel *pm = (PhysicalModel*)CreateObject(forceType, wxT("0.")+forceName, 0);
         if (pm)
         {
            ////pm->SetName(forceType + wxT(".") + bodies[i]);
            pm->SetName(forceName);
            
            if (!pm->SetStringParameter(wxT("BodyName"), bodies[i]))
            {
               InterpreterException ex(wxT("Unable to set body for force ") + bodies[i]);
               HandleError(ex);
            }
            
            #ifdef DEBUG_SET_FORCE_MODEL
            MessageInterface::ShowMessage
               (wxT("   Adding type:<%s> name:<%s> to ForceModel:<%s>\n"),
                pm->GetTypeName().c_str(), pm->GetName().c_str(),
                forceModel->GetName().c_str());
            #endif
            
            // Since default GravityField is created when ForceModel is created
            // We need to empty the ForceModel before adding new force. (LOJ: 2010.09.01)
            // Add force to ForceModel
            forceModel->TakeAction(wxT("ClearDefaultForce"));
            forceModel->AddForce(pm);
            
            // Use JGM2 for default Earth gravity file, in case it is not
            // specified in the script
            if (pmType == wxT("PrimaryBodies") && bodies[i] == wxT("Earth"))
            {
               id = pm->GetParameterID(wxT("Model"));
               type = pm->GetParameterType(id);
               retval = SetPropertyValue(pm, id, type, wxT("JGM2"));
            }
            if (pmType == wxT("PrimaryBodies") && bodies[i] == wxT("Luna"))
            {
               id = pm->GetParameterID(wxT("Model"));
               type = pm->GetParameterType(id);
               retval = SetPropertyValue(pm, id, type, wxT("LP165P"));
            }
            if (pmType == wxT("PrimaryBodies") && bodies[i] == wxT("Venus"))
            {
               id = pm->GetParameterID(wxT("Model"));
               type = pm->GetParameterType(id);
               retval = SetPropertyValue(pm, id, type, wxT("MGNP180U"));
            }
            if (pmType == wxT("PrimaryBodies") && bodies[i] == wxT("Mars"))
            {
               id = pm->GetParameterID(wxT("Model"));
               type = pm->GetParameterType(id);
               retval = SetPropertyValue(pm, id, type, wxT("MARS50C"));
            }
         }
      }
      
      #ifdef DEBUG_SET_FORCE_MODEL
      MessageInterface::ShowMessage
         (wxT("Interpreter::SetForceModelProperty() returning %d\n"), retval);
      #endif
      return retval;
   }
   else if (pmType == wxT("Drag") || pmType == wxT("AtmosphereModel"))
   {
      // Write deprecated message, now we olny use Drag.AtmosphereModel to specify model name
      if (pmType == wxT("Drag") && value != wxT("None"))
      {
         InterpreterException ex
            (wxT("The field \"Drag\" of ForceModel \"") + obj->GetName() +
             wxT("\" will not be permitted in a future build; ")
             wxT("please use \"Drag.AtmosphereModel\" instead"));
         HandleError(ex, true, true);
      }
      
      // If value is None, do not create DragForce
      if (value == wxT("None"))
         return true;
      
      // Special handling for Drag
      // If field is AtmosphereModel, create DragForce and then AtmosphereModel.
      // It will also handle old script such as FM.Drag = JacchiaRoberts
      return SetDragForceProperty(obj, wxT("Drag"), pmType, value);
   }
   else if (pmType == wxT("SRP") || pmType == wxT("RelativisticCorrection"))
   {
      if (pmType == wxT("SRP"))
      {
         id = obj->GetParameterID(wxT("SRP"));
         type = obj->GetParameterType(id);
         retval = SetPropertyValue(obj, id, type, value);
         
         if (retval && value != wxT("On"))
            return true;
         else if (!retval)
            return false;
      }
      
      if (pmType == wxT("RelativisticCorrection"))
      {
         id = obj->GetParameterID(wxT("RelativisticCorrection"));
         type = obj->GetParameterType(id);
         retval = SetPropertyValue(obj, id, type, value);

         if (retval && value != wxT("On"))
            return true;
         else if (!retval)
            return false;
      }
      
      // Create PhysicalModel
      wxString forceName = pmType + wxT(".") + centralBodyName;
      //@note 0.ForceName indicates unmanaged internal forcename.
      // Added name for debugging purpose only
      PhysicalModel *pm = (PhysicalModel*)CreateObject(forceType, wxT("0.")+forceName, 0);
      pm->SetName(forceName);
      
      // Should we set SRP on ForceModel central body?
      pm->SetStringParameter(wxT("BodyName"), centralBodyName);
      
      #ifdef DEBUG_SET_FORCE_MODEL
      MessageInterface::ShowMessage
         (wxT("   Adding PhysicalModel <%p><%s>'%s' to ForceModel:<%s>\n"), pm,
          pm->GetTypeName().c_str(), pm->GetName().c_str(),
          forceModel->GetName().c_str());
      #endif
      
      // Add force to ForceModel
      forceModel->AddForce(pm);
      
      #ifdef DEBUG_SET_FORCE_MODEL
      MessageInterface::ShowMessage(wxT("Interpreter::SetForceModelProperty() returning true\n"));
      #endif
      return true;
   }
   // User defined forces
   else if (pmType == wxT("UserDefined"))
   {
      StringArray udForces = theTextParser.SeparateBrackets(value, wxT("{}"), wxT(" ,"));
      
      for (UnsignedInt i=0; i<udForces.size(); i++)
      {
         #ifdef DEBUG_SET_FORCE_MODEL
            MessageInterface::ShowMessage(wxT("   User defined force[%d] = %s\n"), 
                  i, udForces[i].c_str());
         #endif
         
         // We don't want to configure PhysicalModel, so set name after create
         ////PhysicalModel *pm = (PhysicalModel*)CreateObject(udForces[i], wxT(""));
         PhysicalModel *pm = (PhysicalModel*)CreateObject(udForces[i], udForces[i], 0);
         if (pm)
         {
            pm->SetName(udForces[i]);
            forceModel->AddForce(pm);
         }
         else
            throw InterpreterException
               (wxT("User defined force \"") + udForces[i] +  wxT("\" cannot be created\n"));
      }
   }
   
   
   //------------------------------------------------------------
   // Set ForceModel owned object properties
   //------------------------------------------------------------
   
   pmType = parts[0];
   forceType = ODEModel::GetScriptAlias(pmType);
   wxString propName = parts[dotCount-1];
   
   #ifdef DEBUG_SET_FORCE_MODEL
   MessageInterface::ShowMessage
      (wxT("   Setting pmType=%s, forceType=%s, propName=%s\n"), pmType.c_str(),
       forceType.c_str(), propName.c_str());
   #endif
   
   GmatBase *owner;
   Integer propId;
   Gmat::ParameterType propType;
   if (FindPropertyID(forceModel, propName, &owner, propId, propType))
   {
      id = owner->GetParameterID(propName);
      type = owner->GetParameterType(id);
      retval = SetPropertyValue(owner, id, type, value);
      if (fromObj != NULL)
         owner->SetRefObject(fromObj, fromObj->GetType(), value);
   }
   else
   {
      // Try owned object from ODEModel
      for (int i = 0; i < forceModel->GetOwnedObjectCount(); i++)
      {
         GmatBase *ownedObj = forceModel->GetOwnedObject(i);
         if (ownedObj && FindPropertyID(ownedObj, propName, &owner, propId, propType))
         {
            id = owner->GetParameterID(propName);
            type = owner->GetParameterType(id);
            retval = SetPropertyValue(owner, id, type, value);
            break;
         }
      }
   }
   
   #ifdef DEBUG_SET_FORCE_MODEL
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetForceModelProperty() returning %d\n"), retval);
   #endif
   return retval;
}


//------------------------------------------------------------------------------
// bool SetDragForceProperty(GmatBase *obj, const wxString &pmType, ...)
//------------------------------------------------------------------------------
/**
 * Creates DragForce and AtmosphereModel objects and adds to ODEModel.
 *
 * @param  obj  ODEModel object
 * @param  pmType  First field string (This is not the actual PhysicalModel type name)
 */
//------------------------------------------------------------------------------
bool Interpreter::SetDragForceProperty(GmatBase *obj,
                                       const wxString &pmType,
                                       const wxString &propName,
                                       const wxString &value)
{
   ODEModel *forceModel = (ODEModel*)obj;
   wxString forceType = ODEModel::GetScriptAlias(pmType);
   wxString centralBodyName = forceModel->GetStringParameter(wxT("CentralBody"));
   
   #ifdef DEBUG_SET_FORCE_MODEL
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetDragForceProperty() entered, forceType=%s, pmType=%s, ")
       wxT("propName=%s, value=%s, centralBodyName=%s\n"), forceType.c_str(), pmType.c_str(),
       propName.c_str(), value.c_str(), centralBodyName.c_str());
   #endif
   
   // Create DragForce
   //@note 0.ForceName indicates unmanaged internal forcename.
   // Added name for debugging purpose only
   wxString forceName = pmType + wxT(".") + centralBodyName;
   PhysicalModel *pm = (PhysicalModel*)CreateObject(forceType, wxT("0.")+forceName, 0);
   pm->SetName(forceName);
   
   #ifdef DEBUG_SET_FORCE_MODEL
   MessageInterface::ShowMessage
      (wxT("   PhysicalModel <%p><%s>'%s' created\n"), pm, pm->GetTypeName().c_str(),
       pm->GetName().c_str());
   #endif
   
   if (!pm->SetStringParameter(wxT("AtmosphereModel"), value))
   {
      InterpreterException ex
         (wxT("Unable to set AtmosphereModel for drag force"));
      HandleError(ex);
      ignoreError = true;
      return false;
   }
   
   // Create AtmosphereModel for the primary body
   if (value != wxT("BodyDefault"))
   {
      wxString valueToUse = GmatStringUtil::RemoveEnclosingString(value, wxT("'"));
      
      #ifdef DEBUG_SET_FORCE_MODEL
      MessageInterface::ShowMessage
         (wxT("   Creating AtmosphereModel of type '%s'\n"), valueToUse.c_str());
      #endif
      
      pm->SetStringParameter(wxT("BodyName"), centralBodyName);
      pm->SetStringParameter(wxT("AtmosphereBody"), centralBodyName);
      GmatBase *am = CreateObject(valueToUse, valueToUse, 0);
      if (am)
      {
         pm->SetRefObject(am, Gmat::ATMOSPHERE, am->GetName());
      }
      else
      {
         InterpreterException ex
            (wxT("Unable to create AtmosphereModel \"") + valueToUse + wxT("\" for drag force"));
         HandleError(ex);
         ignoreError = true;
         return false;
      }
   }
   
   #ifdef DEBUG_SET_FORCE_MODEL
   MessageInterface::ShowMessage
      (wxT("   Adding PhysicalModel <%p><%s>'%s' to ForceModel:<%s>\n"), pm,
       pm->GetTypeName().c_str(), pm->GetName().c_str(),
       forceModel->GetName().c_str());
   #endif
   
   // Add force to ForceModel
   forceModel->AddForce(pm);
   
   #ifdef DEBUG_SET_FORCE_MODEL
   MessageInterface::ShowMessage(wxT("Interpreter::SetDragForceProperty() returning true\n"));
   #endif
   return true;
}



//------------------------------------------------------------------------------
// bool Interpreter::SetMeasurementModelProperty(GmatBase *obj,
//          const wxString &property, const wxString &value)
//------------------------------------------------------------------------------
/**
 * This method configures properties on a MeasurementModel
 *
 * The method creates CoreMeasurements as needed, and passes the remaining
 * parameters to the SetProperty() method.
 *
 * @param obj        The MeasurementModel that is being configured
 * @param property   The property that is being set
 * @param value      The property's value
 *
 * @return true on success, false on failure
 */
//------------------------------------------------------------------------------
bool Interpreter::SetMeasurementModelProperty(GmatBase *obj,
         const wxString &property, const wxString &value)
{
   debugMsg = wxT("In SetMeasurementModelProperty()");
   bool retval = false;
   StringArray parts = theTextParser.SeparateDots(property);
   Integer count = parts.size();
   wxString propName = parts[count-1];

   #ifdef DEBUG_SET_MEASUREMENT_MODEL
      MessageInterface::ShowMessage
         (wxT("Interpreter::SetMeasurementModelProperty() mModel=%s, prop=%s, ")
          wxT("value=%s\n"), obj->GetName().c_str(), propName.c_str(),
          value.c_str());
   #endif

   if (propName == wxT("Type"))
   {
      GmatBase* model = CreateObject(value, wxT(""), 0, false);
      if (model != NULL)
      {
         if (model->IsOfType(Gmat::CORE_MEASUREMENT))
            retval = obj->SetRefObject(model, Gmat::CORE_MEASUREMENT, wxT(""));
      }
      else
         throw InterpreterException(wxT("Failed to create a ") + value +
               wxT(" core measurement"));
   }
   else
   {
      Integer id;
      Gmat::ParameterType type;

      StringArray parts = theTextParser.SeparateDots(property);
      // if property has multiple dots, handle separately
      if (parts.size() > 1)
      {
         retval = SetComplexProperty(obj, property, value);
         if (retval)
            return retval;
      }

      id = obj->GetParameterID(property);
      type = obj->GetParameterType(id);
      if (property == wxT("Covariance"))
      {
         // Check the size of the inputs -- MUST be a square matrix
         StringArray rhsRows;
         if ((value.find(wxT("[")) == value.npos) || (value.find(wxT("]")) == value.npos))
            throw GmatBaseException(wxT("Covariance matrix definition is missing ")
                  wxT("square brackets"));

         rhsRows = theTextParser.SeparateBrackets(value, wxT("[]"), wxT(";"));
         UnsignedInt rowCount = rhsRows.size();

         StringArray cells = theTextParser.SeparateSpaces(rhsRows[0]);
         UnsignedInt colCount = cells.size();

         Covariance *covariance = obj->GetCovariance();

         #ifdef DEBUG_SET
            MessageInterface::ShowMessage(wxT("%s covariance has dim %d, ")
                  wxT("row count = %d, colCount = %d\n"), obj->GetName().c_str(),
                  covariance->GetDimension(), rowCount, colCount);
         #endif

         if ((Integer)colCount > covariance->GetDimension())
            throw GmatBaseException(wxT("Input covariance matrix is larger than the ")
                  wxT("matrix built from the input array"));

         for (UnsignedInt i = 1; i < rowCount; ++i)
         {
            cells = theTextParser.SeparateSpaces(rhsRows[i]);
          #ifdef DEBUG_SET
               MessageInterface::ShowMessage(wxT("   Found  %d columns in row %d\n"),
                     cells.size(), i+1);
          #endif

            if (cells.size() != rowCount)
               throw InterpreterException(wxT("Row/Column mismatch in the Covariance ")
                     wxT("matrix for ") + obj->GetName());
         }

         #ifdef DEBUG_SET
            MessageInterface::ShowMessage(wxT("Found %d rows and %d columns\n"),
                  rowCount, colCount);
         #endif

         for (UnsignedInt i = 0; i < colCount; ++i)
         {
            if (rowCount != 1)
               cells = theTextParser.SeparateSpaces(rhsRows[i]);
            for (UnsignedInt j = 0; j < colCount; ++j)
               if (i == j)
                  SetPropertyValue(obj, id, type, cells[j], i, j);
               else
                  // If a single row, it's the diagonal
                  if (rowCount == 1)
                     SetPropertyValue(obj, id, type, wxT("0.0"), i, j);
                  // Otherwise it's cell[j]
                  else
                     SetPropertyValue(obj, id, type, cells[j], i, j);
         }

         retval = true;
      }
      else
         retval = SetProperty(obj, id, type, value);
   }

   return retval;
}

//------------------------------------------------------------------------------
// bool SetTrackingDataProperty(GmatBase *obj, const wxString &prop,
//                              const wxString &value)
//------------------------------------------------------------------------------
/**
 * This method...
 *
 * @param
 *
 * @return
 */
//------------------------------------------------------------------------------
bool Interpreter::SetTrackingDataProperty(GmatBase *obj,
         const wxString &property, const wxString &value)
{
   debugMsg = wxT("In SetTrackingDataProperty()");
   bool retval = false;
   StringArray parts = theTextParser.SeparateDots(property);
   Integer count = parts.size();
   wxString propName = parts[count-1];

   #ifdef DEBUG_SET_MEASUREMENT_MODEL
      MessageInterface::ShowMessage
         (wxT("Interpreter::SetTrackingDataProperty() mModel=%s, prop=%s, ")
          wxT("value=%s\n"), obj->GetName().c_str(), propName.c_str(),
          value.c_str());
   #endif

   if (propName == wxT("Type"))
   {
      GmatBase* model = CreateObject(value, wxT(""), 0, false);
      if (model != NULL)
      {
         if (model->IsOfType(Gmat::CORE_MEASUREMENT))
            retval = obj->SetRefObject(model, Gmat::CORE_MEASUREMENT, wxT(""));
      }
      else
         throw InterpreterException(wxT("Failed to create a ") + value +
               wxT(" core measurement"));
   }
   else
   {
      Integer id;
      Gmat::ParameterType type;

      StringArray parts = theTextParser.SeparateDots(property);
      // if property has multiple dots, handle separately
      if (parts.size() > 1)
      {
         retval = SetComplexProperty(obj, property, value);
         if (retval)
            return retval;
      }

      id = obj->GetParameterID(property);
      type = obj->GetParameterType(id);
      if (property == wxT("Covariance"))
      {
         // Check the size of the inputs -- MUST be a square matrix
         StringArray rhsRows;
         if ((value.find(wxT("[")) == value.npos) || (value.find(wxT("]")) == value.npos))
            throw GmatBaseException(wxT("Covariance matrix definition is missing ")
                  wxT("square brackets"));

         rhsRows = theTextParser.SeparateBrackets(value, wxT("[]"), wxT(";"));
         UnsignedInt rowCount = rhsRows.size();

         StringArray cells = theTextParser.SeparateSpaces(rhsRows[0]);
         UnsignedInt colCount = cells.size();

         Covariance *covariance = obj->GetCovariance();

         #ifdef DEBUG_SET
            MessageInterface::ShowMessage(wxT("%s covariance has dim %d, ")
                  wxT("row count = %d, colCount = %d\n"), obj->GetName().c_str(),
                  covariance->GetDimension(), rowCount, colCount);
         #endif

         if ((Integer)colCount > covariance->GetDimension())
            throw GmatBaseException(wxT("Input covariance matrix is larger than the ")
                  wxT("matrix built from the input array"));

         for (UnsignedInt i = 1; i < rowCount; ++i)
         {
            cells = theTextParser.SeparateSpaces(rhsRows[i]);
          #ifdef DEBUG_SET
               MessageInterface::ShowMessage(wxT("   Found  %d columns in row %d\n"),
                     cells.size(), i+1);
          #endif

            if (cells.size() != rowCount)
               throw InterpreterException(wxT("Row/Column mismatch in the Covariance ")
                     wxT("matrix for ") + obj->GetName());
         }

         #ifdef DEBUG_SET
            MessageInterface::ShowMessage(wxT("Found %d rows and %d columns\n"),
                  rowCount, colCount);
         #endif

         for (UnsignedInt i = 0; i < colCount; ++i)
         {
            if (rowCount != 1)
               cells = theTextParser.SeparateSpaces(rhsRows[i]);
            for (UnsignedInt j = 0; j < colCount; ++j)
               if (i == j)
                  SetPropertyValue(obj, id, type, cells[j], i, j);
               else
                  // If a single row, it's the diagonal
                  if (rowCount == 1)
                     SetPropertyValue(obj, id, type, wxT("0.0"), i, j);
                  // Otherwise it's cell[j]
                  else
                     SetPropertyValue(obj, id, type, cells[j], i, j);
         }

         retval = true;
      }
      else
         retval = SetProperty(obj, id, type, value);
   }

   return retval;
}


//------------------------------------------------------------------------------
// bool SetTrackingSystemProperty(GmatBase *obj, const wxString &prop,
//          const wxString &value)
//------------------------------------------------------------------------------
/**
 * This method...
 *
 * @param
 *
 * @return
 */
//------------------------------------------------------------------------------
bool Interpreter::SetTrackingSystemProperty(GmatBase *obj,
         const wxString &prop, const wxString &value)
{
   debugMsg = wxT("In SetTrackingSystemProperty()");
   bool retval = false;
   StringArray parts = theTextParser.SeparateDots(prop);

   // if property has multiple dots, handle separately
   if (parts.size() > 1)
   {
      retval = SetComplexProperty(obj, prop, value);
      if (retval)
         return retval;
   }

   Integer id;
   Gmat::ParameterType type;
   id = obj->GetParameterID(prop);
   type = obj->GetParameterType(id);
   retval = SetProperty(obj, id, type, value);
   return retval;
}


//------------------------------------------------------------------------------
// bool Interpreter::SetDataStreamProperty(GmatBase *obj,
//          const wxString &property, const wxString &value)
//------------------------------------------------------------------------------
/**
 * This method configures properties on a DataStream
 *
 * The method creates ObTypes as needed, and passes the remaining
 * parameters to the SetProperty() method.
 *
 * @param obj        The DataStream that is being configured
 * @param property   The property that is being set
 * @param value      The property's value
 *
 * @return true on success, false on failure
 */
//------------------------------------------------------------------------------
bool Interpreter::SetDataStreamProperty(GmatBase *obj,
         const wxString &property, const wxString &value)
{
   debugMsg = wxT("In SetDataStreamProperty()");
   bool retval = false;
   StringArray parts = theTextParser.SeparateDots(property);
   Integer count = parts.size();
   wxString propName = parts[count-1];

   #ifdef DEBUG_SET_MEASUREMENT_MODEL
      MessageInterface::ShowMessage
         (wxT("Interpreter::SetDataStreamProperty() mModel=%s, prop=%s, ")
          wxT("value=%s\n"), obj->GetName().c_str(), propName.c_str(),
          value.c_str());
   #endif

   if (propName == wxT("Format"))
   {
      GmatBase* obs = CreateObject(value, wxT(""), 0, false);
      if (obs != NULL)
      {
         if (obs->IsOfType(Gmat::OBTYPE))
            retval = obj->SetRefObject(obs, Gmat::OBTYPE);
      }
      else
         throw InterpreterException(wxT("Failed to create a ") + value +
               wxT(" observation type"));
   }
   else
   {
      Integer id;
      Gmat::ParameterType type;

      id = obj->GetParameterID(property);
      type = obj->GetParameterType(id);
      retval = SetProperty(obj, id, type, value);
   }

   return retval;
}



//------------------------------------------------------------------------------
// bool SetSolarSystemProperty(GmatBase *obj, const wxString &prop,
//                            const wxString &value)
//------------------------------------------------------------------------------
bool Interpreter::SetSolarSystemProperty(GmatBase *obj, const wxString &prop,
                                         const wxString &value)
{
   #ifdef DEBUG_SET_SOLAR_SYS
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetSolarSystemProperty() type=%s, name=%s, prop=%s, value=%s\n"),
       obj->GetTypeName().c_str(), obj->GetName().c_str(), prop.c_str(), value.c_str());
   #endif
   
   debugMsg = wxT("In SetSolarSystemProperty()");
   bool retval = false;
   StringArray parts = theTextParser.SeparateDots(prop);
   Integer count = parts.size();
   SolarSystem *solarSystem = (SolarSystem *)obj;
   
   if (count == 1)
   {
//      if (prop == wxT("Ephemeris"))
//      {
//         StringArray ephems = theTextParser.SeparateBrackets(value, wxT("{}"), wxT(" ,"));
//      
//         #ifdef DEBUG_SET_SOLAR_SYS
//         for (StringArray::iterator i = ephems.begin(); i != ephems.end(); ++i)
//            MessageInterface::ShowMessage(wxT("   Source = %s\n"), i->c_str());
//         #endif
//         
//         theModerator->SetPlanetarySourceTypesInUse(ephems);
//         retval = true;
//      }
//      else
//      {
         Integer id = obj->GetParameterID(prop);
         Gmat::ParameterType type = obj->GetParameterType(id);
         retval = SetPropertyValue(obj, id, type, value);
//      }
   }
   else
   {
      // Script has the form of:
      // GMAT SolarSystem.Earth.NutationUpdateInterval = 60.0;
      // GMAT SolarSystem.Earth.UseTTForEphemeris = true;
      // GMAT SolarSystem.Earth.DateFormat  = TAIModJulian;
      // GMAT SolarSystem.Earth.StateType   = Keplerian;
      // GMAT SolarSystem.Earth.InitalEpoch = 21544.500371
      // GMAT SolarSystem.Earth.SMA         = 149653978.978377
      
      wxString bodyName = parts[0];
      wxString newProp = parts[count-1];
      
      #ifdef DEBUG_SET_SOLAR_SYS
      MessageInterface::ShowMessage
         (wxT("   bodyName=%s, newProp=%s\n"), bodyName.c_str(), newProp.c_str());
      #endif
      
      // Cannot use FindPropertyID() because SolarSystem bodies have the
      // same property name. So use GetBody() instead.
      GmatBase *body = (GmatBase*)solarSystem->GetBody(bodyName);
      
      if (body == NULL)
      {
         InterpreterException ex
            (wxT("Body: ") + bodyName + wxT(" not found in the SolarSystem\n"));
         HandleError(ex);
      }
      
      try
      {
         Integer id = body->GetParameterID(newProp);
         Gmat::ParameterType type = body->GetParameterType(id);
         retval = SetPropertyValue(body, id, type, value);
      }
      catch (BaseException &e)
      {
         HandleError(e);
      }
   }
   
   #ifdef DEBUG_SET_SOLAR_SYS
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetSolarSystemProperty() prop=%s, retval=%d\n"),
       prop.c_str(), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool FindOwnedObject(GmatBase *owner, const wxString toProp,
//                      GmatBase **ownedObj, Integer &id, Gmat::OBJECT_TYPE &type)
//------------------------------------------------------------------------------
/*
 * Finds owned object and its property.
 *
 * @param  owner    Owner object to find owned object for property
 * @param  toProp   Property name to find
 * @param  id       Output owned property id (-1 if property not found)
 * @param  type     Output owned property type
 *                  (Gmat::UNKNOWN_PARAMETER_TYPE if property not found)
 *
 * @return  true if property found from the owned object
 */
//------------------------------------------------------------------------------
bool Interpreter::FindOwnedObject(GmatBase *owner, const wxString toProp,
                                  GmatBase **ownedObj, Integer &id,
                                  Gmat::ParameterType &type)
{
   #ifdef DEBUG_FIND_OBJECT
   MessageInterface::ShowMessage
      (wxT("Interpreter::FindOwnedObject() owner=<%s>, toProp=<%s>\n"),
       owner->GetName().c_str(), toProp.c_str());
   #endif
   
   debugMsg = wxT("In FindOwnedObject()");
   bool retval = false;
   Integer ownedObjCount = owner->GetOwnedObjectCount();
   Integer errorCount = 0;
   GmatBase *tempObj = NULL;
   *ownedObj = NULL;
   
   // Initialize output parameters
   id = -1;
   type = Gmat::UNKNOWN_PARAMETER_TYPE;
   
   #ifdef DEBUG_FIND_OBJECT
   MessageInterface::ShowMessage(wxT("   ownedObjCount=%d\n"), ownedObjCount);
   #endif
   
   if (ownedObjCount > 0)
   {
      for (int i=0; i<ownedObjCount; i++)
      {
         tempObj = owner->GetOwnedObject(i);
         if (ownedObj)
         {
            #ifdef DEBUG_FIND_OBJECT
            MessageInterface::ShowMessage
               (wxT("   i=%d, ownedObj type=<%s>, name=<%s>\n"), i,
                tempObj->GetTypeName().c_str(), tempObj->GetName().c_str());
            #endif
            
            try
            {
               id = tempObj->GetParameterID(toProp);
               type = tempObj->GetParameterType(id);
               *ownedObj = tempObj;
               retval = true;
               break;
            }
            catch (BaseException &)
            {
               errorCount++;
               continue;
            }
         }
      }
      
      if (errorCount == ownedObjCount)
      {
         // Throw error only when parsing delayed block, so that
         // duplicated error message will not be shown
         if (parsingDelayedBlock)
         {
            //@todo
            // Currently SolarSystem parameter is handled by the Moderator,
            // so it is an exceptional case.
            // Eventually we want to move parameter handling to SolarSyatem.
            if (owner->GetName() != wxT("SolarSystem"))
            {
               InterpreterException ex
                  (wxT("The field name \"") + toProp + wxT("\" on object ") + owner->GetName() +
                   wxT(" is not permitted"));
               HandleErrorMessage(ex, lineNumber, currentLine, true);
            }
         }
      }
   }
   
   #ifdef DEBUG_FIND_OBJECT
   MessageInterface::ShowMessage
      (wxT("   FindOwnedObject() returning retval=%d, ownedObj=<%p>\n"), retval, *ownedObj);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// Real GetArrayValue(const wxString &arrayStr, Integer &row, Integer &col)
//------------------------------------------------------------------------------
/**
 * Retrives configured array value by row and col.
 *
 * @param  arrayStr  String form of array (A(1,3), B(2,j), etc)
 *
 * @note Array name must be created and configured before access.
 */
//------------------------------------------------------------------------------
Real Interpreter::GetArrayValue(const wxString &arrayStr,
                                Integer &row, Integer &col)
{
   #ifdef DEBUG_SET
   MessageInterface::ShowMessage
      (wxT("Interpreter::GetArrayValue arrayStr=%s\n"), arrayStr.c_str());
   #endif
   
   debugMsg = wxT("In GetArrayValue()");
   Parameter *param = GetArrayIndex(arrayStr, row, col);
   
   if (row != -1 && col != -1)
      return param->GetRealParameter(wxT("SingleValue"), row, col);
   else
   {
      InterpreterException ex(wxT("Invalid row and column index\n"));
      HandleError(ex);
   }
   return 0.0;
}


//------------------------------------------------------------------------------
// bool IsArrayElement(const wxString &str)
//------------------------------------------------------------------------------
bool Interpreter::IsArrayElement(const wxString &str)
{
   bool retval = false;
   
   if (str.find(wxT("[")) != str.npos)
   {
      InterpreterException ex(wxT("\"") + str + wxT("\" is not a valid Array element"));
      HandleError(ex);
   }
   
   retval = GmatStringUtil::IsParenPartOfArray(str);

   #ifdef DEBUG_ARRAY_GET
   MessageInterface::ShowMessage
      (wxT("Interpreter::IsArrayElement() str=%s, array=%d\n"), str.c_str(), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool ParseVariableExpression(Parameter *var, const wxString &exp)
//------------------------------------------------------------------------------
bool Interpreter::ParseVariableExpression(Parameter *var, const wxString &exp)
{
   if (var == NULL)
   {
      InterpreterException ex
         (wxT("Interpreter::ParseVariableExpression() The variable is NULL\n"));
      HandleError(ex);
      return false;
   }
   
   #ifdef DEBUG_VAR_EXPRESSION
   MessageInterface::ShowMessage
      (wxT("Interpreter::ParseVariableExpression() entered, var=<%p>'%s', exp='%s'\n"),
       var, var->GetName().c_str(), exp.c_str());
   #endif
   
   // Check for invalid starting name such as 1(x) should give an error (loj: 2008.08.15)
   if (exp.find_first_of(wxT("(")) != exp.npos)
   {
      if (!GmatStringUtil::IsValidName(exp, true))
      {
         #ifdef DEBUG_VAR_EXPRESSION
         MessageInterface::ShowMessage
            (wxT("Interpreter::ParseVariableExpression() returning false, '%s' is not ")
             wxT("a valid name\n"), exp.c_str());
         #endif
         return false;
      }
   }
   
   // Parse the Parameter
   StringTokenizer st(exp, wxT("()*/+-^ "));
   StringArray tokens = st.GetAllTokens();
   Real rval;
   
   // Check if unexisting varibles used in expression
   for (unsigned int i=0; i<tokens.size(); i++)
   {
      #ifdef DEBUG_VAR_EXPRESSION
      MessageInterface::ShowMessage(wxT("   token:<%s> \n"), tokens[i].c_str());
      #endif
      
      if (!GmatStringUtil::ToReal(tokens[i], rval))
      {
         #ifdef DEBUG_VAR_EXPRESSION
         MessageInterface::ShowMessage
            (wxT("   It is not a number, so trying to create a Parameter\n"));
         #endif
         
         Parameter *param = CreateSystemParameter(tokens[i]);
         if (param)
         {
            #ifdef DEBUG_VAR_EXPRESSION
            MessageInterface::ShowMessage
               (wxT("   The Parameter '%s' found or created, so setting it to '%s' ")
                wxT("as ref object name\n"),  param->GetName().c_str(), var->GetName().c_str());
            #endif
            // set parameter names used in expression
            var->SetRefObjectName(Gmat::PARAMETER, tokens[i]);
         }
         else
         {
            #ifdef DEBUG_VAR_EXPRESSION
            MessageInterface::ShowMessage
               (wxT("Interpreter::ParseVariableExpression() returning false ")
                wxT("since '%s' is not allowed in the expression\n"), tokens[i].c_str());
            #endif
            
            //InterpreterException ex
            //   (wxT("The Variable \"") + tokens[i] + wxT("\" does not exist. ")
            //    wxT("It must be created first"));
            //HandleError(ex);
            return false;
         }
      }
   }
   
   var->SetStringParameter(wxT("Expression"), exp);
   
   return true;
}


//------------------------------------------------------------------------------
// AxisSystem* CreateAxisSystem(wxString type, GmatBase *owner)
//------------------------------------------------------------------------------
AxisSystem* Interpreter::CreateAxisSystem(wxString type, GmatBase *owner)
{
   #ifdef DEBUG_AXIS_SYSTEM
   MessageInterface::ShowMessage
      (wxT("Interpreter::CreateAxisSystem() type = '%s'\n"), type.c_str());
   #endif
   
   AxisSystem *axis = theValidator->CreateAxisSystem(type, owner);
   
   // Handle error messages here
   if (axis == NULL)
   {
      StringArray errList = theValidator->GetErrorList();
      for (UnsignedInt i=0; i<errList.size(); i++)
         HandleError(InterpreterException(errList[i]));
   }
   
   #ifdef DEBUG_AXIS_SYSTEM
   MessageInterface::ShowMessage
      (wxT("Interpreter::CreateAxisSystem() returning <%p>\n"), axis);
   #endif
   
   return axis;
}


//------------------------------------------------------------------------------
// void HandleError(const BaseException &e, bool writeLine, bool warning ...)
//------------------------------------------------------------------------------
void Interpreter::HandleError(const BaseException &e, bool writeLine, bool warning)
{
   if (writeLine)
   {
      Integer lineNum = theReadWriter->GetLineNumber();
      if (inScriptEvent)
         lineNum = lineNum - 1;

      #ifdef DEBUG_HANDLE_ERROR
      MessageInterface::ShowMessage
         (wxT("Interpreter::HandleError(), inScriptEvent=%d, lineNum=%d\n"),
          inScriptEvent, lineNum);
      #endif
      
      lineNumber = GmatStringUtil::ToString(lineNum);
      currentLine = theReadWriter->GetCurrentLine();
      
      HandleErrorMessage(e, lineNumber, currentLine, writeLine, warning);
   }
   else
   {
      HandleErrorMessage(e, wxT(""), wxT(""), writeLine, warning);
   }
}


//------------------------------------------------------------------------------
// void HandleErrorMessage(const BaseException &e, const wxString &lineNumber...)
//------------------------------------------------------------------------------
void Interpreter::HandleErrorMessage(const BaseException &e,
                                     const wxString &lineNumber,
                                     const wxString &line,
                                     bool writeLine, bool warning)
{
   wxString currMsg = wxT("");
   wxString msgKind = wxT("**** ERROR **** ");
   if (warning)
      msgKind = wxT("*** WARNING *** ");
   
   // Added function name in the message (loj: 2008.08.29)
   wxString fnMsg;
   if (currentFunction != NULL)
   {
      fnMsg = currentFunction->GetFunctionPathAndName();
      fnMsg = wxT("(In Function \"") + fnMsg + wxT("\")\n");
      if (!writeLine)
         fnMsg = wxT("\n") + fnMsg;
   }
   
   if (writeLine)
      currMsg = wxT(" in line:\n") + fnMsg + wxT("   \"") + lineNumber + wxT(": ") + line + wxT("\"\n");
   else
      currMsg = fnMsg;
   
   wxString msg = msgKind + e.GetFullMessage() + currMsg;
   
   #ifdef DEBUG_HANDLE_ERROR
   MessageInterface::ShowMessage(wxT("%s, continueOnError=%d\n"), debugMsg.c_str(), continueOnError);
   #endif
   
   if (continueOnError)
   {
      errorList.push_back(msg);
      
      #ifdef DEBUG_HANDLE_ERROR
      MessageInterface::ShowMessage(msg + wxT("\n"));
      #endif
   }
   else
   {
      if (warning)
         MessageInterface::ShowMessage(msg);
      else
      {
         // remove duplicate message
         msg = GmatStringUtil::Replace(msg, wxT("**** ERROR **** Interpreter Exception: "), wxT(""));
         throw InterpreterException(msg);
      }
   }
}


//------------------------------------------------------------------------------
// bool IsBranchCommand(const wxString &str)
//------------------------------------------------------------------------------
bool Interpreter::IsBranchCommand(const wxString &str)
{
   StringArray parts = theTextParser.SeparateSpaces(str);
   
   if (parts[0] == wxT("If") || parts[0] == wxT("EndIf") ||
       parts[0] == wxT("For") || parts[0] == wxT("EndFor") ||
       parts[0] == wxT("While") || parts[0] == wxT("EndWhile") ||
       parts[0] == wxT("Target") || parts[0] == wxT("EndTarget") ||
       parts[0] == wxT("Optimize") || parts[0] == wxT("EndOptimize") ||
       parts[0] == wxT("BeginScript") || parts[0] == wxT("EndScript"))
      return true;
   else
      return false;
   
}


//------------------------------------------------------------------------------
// bool CheckBranchCommands(const IntegerArray &lineNumbers,
//                          const StringArray &lines,)
//------------------------------------------------------------------------------
/**
 * Checks branch command matching end command.
 *
 * @return true if the all matches, false otherwise
 */
//------------------------------------------------------------------------------
bool Interpreter::CheckBranchCommands(const IntegerArray &lineNumbers,
                                      const StringArray &lines)
{
   #ifdef DEBUG_CHECK_BRANCH
   MessageInterface::ShowMessage(wxT("Interpreter::CheckBranchCommands()\n"));
   for (UnsignedInt i=0; i<lines.size(); i++)
      MessageInterface::ShowMessage(wxT("%d: %s\n"), lineNumbers[i], lines[i].c_str());
   #endif
   
   // Check for unbalaced branch commands
   
   debugMsg = wxT("In CheckBranchCommands()");
   std::stack<wxString> controlStack;
   wxString expEndStr, str, str1;
   bool retval = true;
   
   #ifdef DEBUG_CHECK_BRANCH
   MessageInterface::ShowMessage(wxT("   Now start checking\n"));
   #endif
   
   for (UnsignedInt i=0; i<lines.size(); i++)
   {
      str = lines[i];
      
      #ifdef DEBUG_CHECK_BRANCH
      MessageInterface::ShowMessage
         (wxT("   line=%d, str=%s\n"), lineNumbers[i], str.c_str());
      #endif
      
      if (GmatStringUtil::StartsWith(str, wxT("End")))
      {
         if (controlStack.empty())
         {
            InterpreterException ex(wxT("Found too many \"") + str + wxT("\""));
            HandleErrorMessage(ex, GmatStringUtil::ToString(lineNumbers[i]), str);
            retval = false;
            break;
         }
         
         str1 = controlStack.top();
         controlStack.pop();
         
         if (str1 == wxT("BeginScript"))
            expEndStr = wxT("EndScript");
         else
            expEndStr = wxT("End") + str1;
         
         if (expEndStr != str)
         {
            InterpreterException ex
               (wxT("Expecting \"") + expEndStr + wxT("\" but found \"") + str + wxT("\""));
            HandleErrorMessage(ex, GmatStringUtil::ToString(lineNumbers[i]), str);
            retval = false;
            break;
         }
      }
      else
      {
         controlStack.push(str);
      }
   }
   
   
   if (retval == true)
   {
      if (!controlStack.empty())
      {
         InterpreterException ex
            (wxT("Matching \"End") + controlStack.top() + wxT("\" not found for \"") +
             controlStack.top() + wxT("\""));
         HandleError(ex, false);
         retval = false;
      }
   }
   
   #ifdef DEBUG_CHECK_BRANCH
   MessageInterface::ShowMessage
      (wxT("Interpreter::CheckBranchCommands() returning %d\n"), retval);
   #endif

   return retval;
}


//------------------------------------------------------------------------------
// bool FinalPass()
//------------------------------------------------------------------------------
/**
 * Finishes up the Interpret call by setting internal references that are needed 
 * by the GUI.
 *
 * @return true if the references were set; false otherwise.
 *
 * @note: Most objects have reference objects already set in the SetObject*(),
 *        if parameter type is OBJECT_TYPE, so not requiring additional call to
 *        SetRefObject()
 */
//------------------------------------------------------------------------------
bool Interpreter::FinalPass()
{
   #if DBGLVL_FINAL_PASS
   MessageInterface::ShowMessage(wxT("Interpreter::FinalPass() entered\n"));
   #endif
   
   debugMsg = wxT("In FinalPass()");
   bool retval = true;
   GmatBase *obj = NULL;
   GmatBase *refObj;
   StringArray refNameList;
   wxString objName;
   StringArray objList;
   
   objList = theModerator->GetListOfObjects(Gmat::UNKNOWN_OBJECT);
   SolarSystem *ss = theModerator->GetSolarSystemInUse();
   objList.push_back(ss->GetName());

   StringArray theSSBodies = ss->GetBodiesInUse();
   // Do this to treat SS bodies like all other objects:
   for (UnsignedInt i = 0; i < theSSBodies.size(); ++i)
      objList.push_back(theSSBodies[i]);

   
   #if DBGLVL_FINAL_PASS > 0 //------------------------------ debug ----
   MessageInterface::ShowMessage(wxT("FinalPass:: All object list =\n"));
   for (Integer ii = 0; ii < (Integer) objList.size(); ii++)
      MessageInterface::ShowMessage(wxT("   %s\n"), (objList.at(ii)).c_str());
   #endif //------------------------------------------- end debug ----
   
   //----------------------------------------------------------------------
   // Check reference objects
   //----------------------------------------------------------------------
   for (StringArray::iterator i = objList.begin(); i != objList.end(); ++i)
   {
      obj = FindObject(*i);
      
      #if DBGLVL_FINAL_PASS > 1
      MessageInterface::ShowMessage
         (wxT("Checking ref. object on %s:%s\n"), obj->GetTypeName().c_str(),
          obj->GetName().c_str());
      #endif
      
      // check System Parameters separately since it follows certain naming
      // convention.  wxT("owner.dep.type") where owner can be either Spacecraft
      // or Burn for now
      if (obj == NULL)
         throw InterpreterException(wxT("The object ") + (*i) + wxT(" does not exist"));
      
      if (obj->GetType() == Gmat::PARAMETER)
      {
         wxString type, owner, depObj;
         Parameter *param = (Parameter*)obj;
         
         if (param->GetKey() == GmatParam::SYSTEM_PARAM)
         {
            objName = obj->GetName();            
            GmatStringUtil::ParseParameter(objName, type, owner, depObj);
            
            // Since we can create a system parameter as: Create A1ModJulian Time,
            // we don't want to check if owner is blank.
            if (owner != wxT(""))
            {
               refObj = FindObject(owner);
               if (refObj == NULL)
               {
                  InterpreterException ex
                     (wxT("Nonexistent object \"") + owner + wxT("\" referenced in \"") +
                      obj->GetName() + wxT("\""));
                  HandleError(ex, false);
                  retval = false;
               }
               else if (param->GetOwnerType() != refObj->GetType())
               {
                  InterpreterException ex
                     (wxT("\"") + type + wxT("\" is not property of \"") +
                      refObj->GetTypeName() + wxT("\""));
                  HandleError(ex, false);
                  retval = false;
               }
            }
         }
      }
      
      // check Function separately since it has inputs that can be any object type,
      // including Real number (1234.5678) and String literal ('abc')
      //
      // We don't want to check this in the FinalPass(), since it will be checked
      // when ScriptInterpreter::InterpretGmatFunction() is called
      else if (obj->GetType() == Gmat::FUNCTION)
      {
         // If GmatFunction, see if function file exist and the function name
         // matches the file name
         if (obj->GetTypeName() == wxT("GmatFunction"))
         {
            wxString funcPath = obj->GetStringParameter(wxT("FunctionPath"));
            #if DBGLVL_FUNCTION_DEF > 0
            MessageInterface::ShowMessage
               (wxT("Interpreter::FinalPass() calling CheckFunctionDefinition()\n"));
            #endif
            bool retval1 = CheckFunctionDefinition(funcPath, obj, false);
            retval = retval && retval1;
         }
      }
      
      //-----------------------------------------------------------------
      // Note: This section needs be modified as needed. 
      // GetRefObjectTypeArray() should be implemented if we want to
      // add to this list. This was added to write specific error messages.
      //-----------------------------------------------------------------
      else if ((obj->HasRefObjectTypeArray()))
      {
         // Set return flag to false if any check failed
         try
         {
            bool retval1 = CheckUndefinedReference(obj, false);
            retval = retval && retval1;
            
            // Subscribers uses ElementWrapper to handle Parameter, Variable,
            // Array, Array elements, so create wrappers in ValidateSubscriber()
            if (retval && obj->IsOfType(Gmat::SUBSCRIBER))
            {
               retval = retval && ValidateSubscriber(obj);
               // Since OrbitView has Validate() method
               if (!obj->Validate())
               {
                  retval = retval && false;
                  InterpreterException ex
                     (obj->GetLastErrorMessage() + wxT(" in \"") + obj->GetName() + wxT("\""));
                  HandleError(ex, false);
               }
            }
         }
         catch (BaseException &ex)
         {
            HandleError(ex, false);
            retval = false;
         }
      }
      else
      {
         try
         {
            // Check referenced SpacePoint used by given objects
            refNameList = obj->GetRefObjectNameArray(Gmat::SPACE_POINT);
            
            for (UnsignedInt j = 0; j < refNameList.size(); j++)
            {
               refObj = FindObject(refNameList[j]);
               if ((refObj == NULL) || !(refObj->IsOfType(Gmat::SPACE_POINT)))
               {
                  #if DBGLVL_FINAL_PASS > 1
                  MessageInterface::ShowMessage
                     (wxT("   refNameList[%d]=%s\n"), j, refNameList[j].c_str());
                  #endif
                  
                  InterpreterException ex
                     (wxT("Nonexistent SpacePoint \"") + refNameList[j] +
                      wxT("\" referenced in \"") + obj->GetName() + wxT("\""));
                  HandleError(ex, false);
                  retval = false;
               }
            }
         }
         catch (BaseException &e)
         {
            // Use exception to remove Visual C++ warning
            e.GetMessageType();
            #if DBGLVL_FINAL_PASS
            MessageInterface::ShowMessage(e.GetFullMessage());
            #endif
         }
      }
   }
   
   //-------------------------------------------------------------------
   // Special check for LibrationPoint.
   // Since the order of setting primary and secondary bodies can be
   // different, it cannot check for the same bodies in the base code
   // LibrationPoint::SetStringParameter(). Instead the checking is done
   // in here.  This allows repeated setting of bodies as shown in the
   // following script.
   //    GMAT Libration1.Primary = Sun;
   //    GMAT Libration1.Secondary = Earth;
   //    GMAT Libration1.Primary = Earth;
   //    GMAT Libration1.Secondary = Luna;
   //-------------------------------------------------------------------
   objList = theModerator->GetListOfObjects(Gmat::CALCULATED_POINT);
   
   #if DBGLVL_FINAL_PASS > 1
   MessageInterface::ShowMessage(wxT("FinalPass:: CalculatedPoint list =\n"));
   for (Integer ii = 0; ii < (Integer) objList.size(); ii++)
      MessageInterface::ShowMessage(wxT("   %s\n"), (objList.at(ii)).c_str());
   #endif
   
   for (StringArray::iterator i = objList.begin(); i != objList.end(); ++i)
   {
      obj = FindObject(*i);
      refNameList = obj->GetRefObjectNameArray(Gmat::SPACE_POINT);
      
      if (obj->GetTypeName() == wxT("LibrationPoint"))
      {
         wxString primary = obj->GetStringParameter(wxT("Primary"));
         wxString secondary = obj->GetStringParameter(wxT("Secondary"));
         
         #if DBGLVL_FINAL_PASS > 1
         MessageInterface::ShowMessage
            (wxT("   primary=%s, secondary=%s\n"), primary.c_str(), secondary.c_str());
         #endif
         
         if (primary == secondary)
         {
            InterpreterException ex
               (wxT("The Primary and Secondary bodies cannot be the same in the ")
                wxT("LibrationPoint \"") + obj->GetName() + wxT("\""));
            HandleError(ex, false);
            retval = false;
         }
      }
      
      //----------------------------------------------------------------
      // Now set ref objects to CalculatedPoint objects
      //----------------------------------------------------------------
      
      #if DBGLVL_FINAL_PASS > 1
      MessageInterface::ShowMessage
         (wxT("   Setting RefObject on obj=%s\n"), obj->GetName().c_str());
      #endif
      for (UnsignedInt j = 0; j < refNameList.size(); j++)
      {
         #if DBGLVL_FINAL_PASS > 1
         MessageInterface::ShowMessage
            (wxT("   refNameList[%d]=%s\n"), j, refNameList[j].c_str());
         #endif
         
         refObj = FindObject(refNameList[j]);
         if (refObj)
            obj->SetRefObject(refObj, Gmat::SPACE_POINT, refObj->GetName());
      }
   }
   
   // Update the owned ODE models based on the fully scripted original
   objList = theModerator->GetListOfObjects(Gmat::PROP_SETUP);
   #if DBGLVL_FINAL_PASS > 1
   MessageInterface::ShowMessage(wxT("FinalPass:: PropSetup list =\n"));
   for (Integer ii = 0; ii < (Integer) objList.size(); ii++)
      MessageInterface::ShowMessage(wxT("   %s\n"), (objList.at(ii)).c_str());
   #endif
   for (StringArray::iterator i = objList.begin(); i != objList.end(); ++i)
   {
      obj = FindObject(*i);
      if (obj != NULL)
      {
         if (((PropSetup*)obj)->GetPropagator()->UsesODEModel())
         {
            wxString refName = obj->GetStringParameter(wxT("FM"));
            GmatBase *configuredOde = FindObject(refName);
//            ODEModel *ode = ((PropSetup*)obj)->GetODEModel();
            
            #if DBGLVL_FINAL_PASS > 1
            MessageInterface::ShowMessage
               (wxT("   OdeModel='%s', configuredOde=<%p>\n"), refName.c_str(), configuredOde);
            #endif
            
            if (configuredOde != NULL)
            {
               if (configuredOde->IsOfType(Gmat::ODE_MODEL))
                  ((PropSetup*)obj)->SetODEModel(((ODEModel*)configuredOde));
               else
                  throw InterpreterException(wxT("The object named \"") +
                        refName + wxT("\", referenced by the Propagator \"") +
                        obj->GetName() + wxT("\" as an ODE model is the wrong ")
                              wxT("type; it is a ") + configuredOde->GetTypeName());
            }
            else
            {
               if ((refName != wxT("InternalODEModel")) &&
                   (refName != wxT("InternalForceModel")))
                  throw InterpreterException(wxT("The ODEModel named \"") +
                        refName + wxT("\", referenced by the Propagator \"") +
                        obj->GetName() + wxT("\" cannot be found"));

               // Create default ODE model
               configuredOde = CreateObject(wxT("ODEModel"), refName, 1);
               obj->SetRefObject(configuredOde, configuredOde->GetType(),
                     configuredOde->GetName());
            }
         }
      }
   }
   
   //----------------------------------------------------------------------
   // Initialize CoordinateSystem
   //----------------------------------------------------------------------
   objList = theModerator->GetListOfObjects(Gmat::COORDINATE_SYSTEM);
   
   #if DBGLVL_FINAL_PASS > 1//------------------------------ debug ----
   MessageInterface::ShowMessage(wxT("FinalPass:: CoordinateSystem list =\n"));
   for (Integer ii = 0; ii < (Integer) objList.size(); ii++)
      MessageInterface::ShowMessage(wxT("    %s\n"), (objList.at(ii)).c_str());
   #endif //------------------------------------------- end debug ----
   
   objList = theModerator->GetListOfObjects(Gmat::COORDINATE_SYSTEM);
   for (StringArray::iterator i = objList.begin(); i != objList.end(); ++i)
   {
      CoordinateSystem *cs = (CoordinateSystem*)FindObject(*i);
      #if DBGLVL_FINAL_PASS > 1
      MessageInterface::ShowMessage(wxT("Initializing CoordinateSystem '%s'\n"),
                                    i->c_str());
      #endif
      refNameList = cs->GetRefObjectNameArray(Gmat::SPACE_POINT);
      for (UnsignedInt j = 0; j < refNameList.size(); j++)
      {
         #if DBGLVL_FINAL_PASS > 1
         MessageInterface::ShowMessage
            (wxT("   refNameList[%d]=%s\n"), j, refNameList[j].c_str());
         #endif
         
         refObj = FindObject(refNameList[j]);
         if ((refObj == NULL) || !(refObj->IsOfType(Gmat::SPACE_POINT)))
         {
            // Checking for undefined ref objects already done for CoordinateSystem
            // so commented out to avoid duplicated message (LOJ: 2009.12.17)
            //InterpreterException ex
            //   (wxT("Nonexistent SpacePoint \"") + refNameList[j] +
            //    wxT("\" referenced in the CoordinateSystem \"") + cs->GetName() + wxT("\""));
            //HandleError(ex, false);
            retval = false;
         }
         else
         {
            cs->SetRefObject(refObj, Gmat::SPACE_POINT, refObj->GetName());
            cs->Initialize();
         }
      }
   }

   //-------------------------------------------------------------------
   // Special case for BodyFixedPoints, we need to set CoordinateSystem
   // pointers for the BodyFixed and the MJ2000Eq coordinate systems.  In
   // addition, we need to pass the pointer to the central body.
   //-------------------------------------------------------------------
   #if DBGLVL_FINAL_PASS > 1
      MessageInterface::ShowMessage(wxT("FinalPass:: about to get BodyFixedPoint list\n"));
   #endif
   objList = theModerator->GetListOfObjects(Gmat::BODY_FIXED_POINT);

   #if DBGLVL_FINAL_PASS > 1
   MessageInterface::ShowMessage(wxT("FinalPass:: BodyFixedPoint list =\n"));
   for (Integer ii = 0; ii < (Integer) objList.size(); ii++)
   MessageInterface::ShowMessage(wxT("   %s\n"), (objList.at(ii)).c_str());
   #endif

   for (StringArray::iterator i = objList.begin(); i != objList.end(); ++i)
   {
      obj = FindObject(*i);

      StringArray csNames = obj->GetRefObjectNameArray(Gmat::COORDINATE_SYSTEM);
      for (StringArray::iterator csName = csNames.begin();
           csName != csNames.end(); ++csName)
      {
         GmatBase *csObj = FindObject(*csName);

         // To catch as many errors we can, continue with next object
         if (csObj == NULL)
            continue;

         #if DBGLVL_FINAL_PASS > 1
               MessageInterface::ShowMessage
               (wxT("   Calling '%s'->SetRefObject(%s(%p), %d)\n"), obj->GetName().c_str(),
                csObj->GetName().c_str(), csObj, csObj->GetType());
         #endif

         if (csObj->GetType() != Gmat::COORDINATE_SYSTEM)
         {
            InterpreterException ex
            (wxT("The BodyFixedPoint \"") + obj->GetName() + wxT("\" failed to set ")
             wxT("\"CoordinateSystem\" to \"") + *csName + wxT("\""));
            HandleError(ex, false);
            retval = false;
            continue;
         }

         try
         {
            obj->SetRefObject(csObj, Gmat::COORDINATE_SYSTEM, csObj->GetName());
         }
         catch (BaseException &e)
         {
            InterpreterException ex
            (wxT("The BodyFixedPoint \"") + obj->GetName() + wxT("\" failed to set ")
             wxT("CoordinateSystem: ") + e.GetFullMessage());
            HandleError(ex, false);
            retval = false;
            continue;
         }
      }
      wxString cbName = obj->GetRefObjectName(Gmat::CELESTIAL_BODY);
      GmatBase *cbObj = FindObject(cbName);

      #if DBGLVL_FINAL_PASS > 1
         MessageInterface::ShowMessage
         (wxT("   Calling '%s'->SetRefObject(%s(%p), %d)\n"), obj->GetName().c_str(),
          cbObj->GetName().c_str(), cbObj, cbObj->GetType());
      #endif

      if ((cbObj == NULL) || (cbObj->GetType() != Gmat::CELESTIAL_BODY))
      {
         InterpreterException ex
         (wxT("The BodyFixedPoint \"") + obj->GetName() + wxT("\" failed to set ")
          wxT("\"CelestialBody\" to \"") + cbName + wxT("\""));
         HandleError(ex, false);
         retval = false;
         continue;
      }

      try
      {
         obj->SetRefObject(cbObj, Gmat::CELESTIAL_BODY, cbObj->GetName());
      }
      catch (BaseException &e)
      {
         InterpreterException ex
         (wxT("The BodyFixedPoint \"") + obj->GetName() + wxT("\" failed to set ")
          wxT("CelestialBody: ") + e.GetFullMessage());
         HandleError(ex, false);
         retval = false;
         continue;
      }
   }

   //-------------------------------------------------------------------
   // Special case for Spacecraft, we need to set CoordinateSyatem
   // pointer in which initial state is represented.  So that
   // Spacecraft can convert initial state in user representation to
   // internal representation (EarthMJ2000Eq Cartesian).
   //-------------------------------------------------------------------
   objList = theModerator->GetListOfObjects(Gmat::SPACECRAFT);

   #if DBGLVL_FINAL_PASS > 1
   MessageInterface::ShowMessage(wxT("FinalPass:: Spacecraft list =\n"));
   for (Integer ii = 0; ii < (Integer) objList.size(); ii++)
      MessageInterface::ShowMessage(wxT("   %s\n"), (objList.at(ii)).c_str());
   #endif

   for (StringArray::iterator i = objList.begin(); i != objList.end(); ++i)
   {
      obj = FindObject(*i);

      // Now we have more than one CoordinateSystem from Spacecraft.
      // In additions to Spacecraft's CS, it has to handle CS from Thrusters
      // and Attitude. (LOJ: 2009.09.24)
      //wxString csName = obj->GetRefObjectName(Gmat::COORDINATE_SYSTEM);
      StringArray csNames = obj->GetRefObjectNameArray(Gmat::COORDINATE_SYSTEM);
      for (StringArray::iterator csName = csNames.begin();
           csName != csNames.end(); ++csName)
      {
         GmatBase *csObj = FindObject(*csName);

         // To catch as many errors we can, continue with next object
         if (csObj == NULL)
            continue;

         #if DBGLVL_FINAL_PASS > 1
         MessageInterface::ShowMessage
            (wxT("   Calling '%s'->SetRefObject(%s(%p), %d)\n"), obj->GetName().c_str(),
             csObj->GetName().c_str(), csObj, csObj->GetType());
         #endif

         if (csObj->GetType() != Gmat::COORDINATE_SYSTEM)
         {
            InterpreterException ex
               (wxT("The Spacecraft \"") + obj->GetName() + wxT("\" failed to set ")
                wxT("\"CoordinateSystem\" to \"") + *csName + wxT("\""));
            HandleError(ex, false);
            retval = false;
            continue;
         }

         try
         {
            obj->SetRefObject(csObj, Gmat::COORDINATE_SYSTEM, csObj->GetName());
         }
         catch (BaseException &e)
         {
            InterpreterException ex
               (wxT("The Spacecraft \"") + obj->GetName() + wxT("\" failed to set ")
                wxT("CoordinateSystem: ") + e.GetFullMessage());
            HandleError(ex, false);
            retval = false;
            continue;
         }
      }
   }

   //-------------------------------------------------------------------
   // Special case for SolverBranchCommand such as Optimize, Target,
   // we need to set Solver object to SolverBranchCommand and then
   // to all Vary commands inside. Since Vary command's parameters are
   // different depends on the type of the Solver, such as
   // DifferentialCorrector or Optimizer. When user saves the script
   // without running, it will not write correctly since the Solve is
   // not set, so set it here.
   //-------------------------------------------------------------------
   GmatCommand *current = theModerator->GetFirstCommand();
   while (current != NULL)
   {
      if ((current->GetChildCommand(0)) != NULL)
         SetObjectInBranchCommand(current, wxT("SolverBranchCommand"), wxT("Vary"),
                                  wxT("SolverName"));
      
      current = current->GetNext();
   }
   
   // Validate the references used in the commands
   try
   {
      if (ValidateMcsCommands(theModerator->GetFirstCommand()) == false)
         retval = false;;
   }
   catch (BaseException &ex)
   {
      HandleError(ex, false, false);
      retval = false;
   }


   #if DBGLVL_FINAL_PASS
   MessageInterface::ShowMessage(wxT("Interpreter::FinalPass() returning %d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool Interpreter::ValidateMcsCommands(GmatCommand *first)
//------------------------------------------------------------------------------
/**
 * Checks that the commands in the Mission Control Sequence were built
 * acceptably when parsed.
 *
 * Note that acceptability at this level is necessary but not sufficient for a
 * control sequence to run.  Some commands need additional information,
 * generated in the Sandbox or during the run, to proceed.
 *
 * @param first The command at the start of the control sequence
 *
 * @return true if the command references have been validated and the commands
 *         were set up acceptably, false if not.
 */
//------------------------------------------------------------------------------
bool Interpreter::ValidateMcsCommands(GmatCommand *first, GmatCommand *parent,
      StringArray *missingObjects, wxString *accumulatedErrors)
{
   bool retval = true, cleanMissingObj = false, cleanAccError = false;
   GmatCommand *current = first;

   StringArray theObjects =
         theModerator->GetListOfObjects(Gmat::UNKNOWN_OBJECT);

   SolarSystem *ss = theModerator->GetSolarSystemInUse();
   theObjects.push_back(ss->GetName());

   StringArray theSSBodies = ss->GetBodiesInUse();
   // Do this to treat SS bodies like all other objects:
   for (UnsignedInt i = 0; i < theSSBodies.size(); ++i)
      theObjects.push_back(theSSBodies[i]);
   #ifdef DEBUG_ALL_OBJECTS
      for (unsigned int ii = 0; ii < theObjects.size(); ii++)
         MessageInterface::ShowMessage(wxT(" Obj %d :  %s\n"), (Integer) ii,
               (theObjects.at(ii)).c_str());
   #endif

   Integer beginMCSCount = 0;

   if (missingObjects == NULL)
   {
      missingObjects = new StringArray;
      cleanMissingObj = true;
   }
   if (accumulatedErrors == NULL)
   {
      accumulatedErrors = new wxString;
      cleanAccError = true;
   }

   Integer errorCount, validationErrorCount = 0;

   do
   {
      if (theModerator->IsSequenceStarter(current->GetTypeName()))
         ++beginMCSCount;

      StringArray refs;
      // Validate that objects exist for object references
      if (current)
      {
         #ifdef DEBUG_COMMAND_VALIDATION
            MessageInterface::ShowMessage(wxT("Checking \"%s\"; refs:\n"), current->
                  GetGeneratingString(Gmat::NO_COMMENTS, wxT(""), wxT("")).c_str());
         #endif

         errorCount = 0;
         refs = current->GetObjectList();
         #ifdef DEBUG_COMMAND_VALIDATION
            for (UnsignedInt i = 0; i < refs.size(); ++i)
               MessageInterface::ShowMessage(wxT("   %s\n"), refs[i].c_str());
         #endif

         wxString missing;

         for (UnsignedInt i = 0; i < refs.size(); ++i)
         {
            #ifdef DEBUG_COMMAND_VALIDATION
               MessageInterface::ShowMessage(wxT("   Looking for %s..."),
                     refs[i].c_str());
            #endif
            // Check to see if each referenced object exists
            if (find(theObjects.begin(), theObjects.end(),
                  refs[i]) == theObjects.end())
            {
               if (missing.length() == 0)
               {
                  missing = wxT("      \"");
                  missing += current->GetGeneratingString(Gmat::NO_COMMENTS);
                  missing += wxT("\" references missing object(s):");
                  ++errorCount;
                  retval = false;
               }
               if (errorCount == 1)
                  missing += wxT("  ") + refs[i];
               else
                  missing += wxT(", ") + refs[i];
               #ifdef DEBUG_COMMAND_VALIDATION
                  MessageInterface::ShowMessage(wxT("missing\n"));
               #endif
            }
            #ifdef DEBUG_COMMAND_VALIDATION
               else
                  MessageInterface::ShowMessage(wxT("Found!\n"));
            #endif
         }

         if (missing.length() > 0)
         {
            wxString msg;
            msg << errorCount << wxT(": ") << missing;
            missingObjects->push_back(missing);
         }
      }

      try
      {
         if (current->IsOfType(wxT("BranchCommand")))
         {
            retval &= ValidateMcsCommands(current->GetChildCommand(0), current,
                  missingObjects, accumulatedErrors);
         }
      }
      catch (BaseException &)
      {
         // Ignore the derived exception
      }

      // Call the command's Validate method to check internal validity
      if (current->Validate() == false)
      {
         #ifdef DEBUG_COMMAND_VALIDATION
            MessageInterface::ShowMessage(wxT("The command \"%s\" failed ")
                  wxT("validation\n"),
                  current->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
         #endif
         (*accumulatedErrors) += wxT("   The command \"") +
               current->GetGeneratingString(Gmat::NO_COMMENTS) +
               wxT("\" failed validation.\n");
         ++validationErrorCount;
         retval = false;
      }

      current = current->GetNext();
   }
   while ((current != NULL) && (current != first) && (current != parent));

   wxString exceptionError = (*accumulatedErrors);

   if ((missingObjects->size() > 0) || (validationErrorCount > 0) ||
       (beginMCSCount > 1))  // for now -- use this when BMS is REQUIRED:
//       (beginMCSCount != 1))
   {
      #ifdef DEBUG_VALIDATION
         MessageInterface::ShowMessage(wxT("\n   Missing objects:\n"));
      #endif
      if (missingObjects->size() > 0)
      {
         exceptionError += wxT("   Possible missing objects referenced:\n");
         for (UnsignedInt i = 0; i < missingObjects->size(); ++i)
         {
            #ifdef DEBUG_VALIDATION
               MessageInterface::ShowMessage(wxT("      %s\n"),
                     missingObjects[i].c_str());
            #endif
            exceptionError += (*missingObjects)[i] + wxT("\n");
         }
      }

      if (cleanMissingObj)
         delete missingObjects;
      if (cleanAccError)
         delete accumulatedErrors;

      if (beginMCSCount > 1)
         exceptionError += wxT("Too many Mission Sequence start ")
               wxT("commands (from the list [") + 
               theModerator->GetStarterStringList() + wxT("]) were found");

      if (beginMCSCount == 0)
         exceptionError += wxT("No Mission Sequence starter commands (from the ")
         wxT("list [") + theModerator->GetStarterStringList() + wxT("]) were found");

      throw InterpreterException(wxT("\n") + exceptionError);
   }

   if (cleanMissingObj)
      delete missingObjects;
   if (cleanAccError)
      delete accumulatedErrors;

   return retval;
}

//------------------------------------------------------------------------------
// void SetObjectInBranchCommand(GmatCommand *brCmd,
//       const wxString &branchType, const wxString childType,
//       const wxString &objName)
//------------------------------------------------------------------------------
void Interpreter::SetObjectInBranchCommand(GmatCommand *brCmd,
                                           const wxString &branchType,
                                           const wxString &childType,
                                           const wxString &objName)
{
   #ifdef DEBUG_BRANCH_COMMAND_OBJECT
   MessageInterface::ShowMessage
      (wxT("Interpreter::SetObjectInBranchCommand() entered, brCmd=<%p><%s>, branchType='%s', ")
       wxT("childType='%s'\n"), brCmd, brCmd->GetTypeName().c_str(), branchType.c_str(),
       childType.c_str());
   #endif
   
   GmatCommand* current = brCmd;
   Integer childNo = 0;
   GmatCommand* nextInBranch;
   GmatCommand* child;
   GmatBase *solver = NULL;
   wxString solverName;
   
   if (brCmd->IsOfType(branchType))
   {
      #ifdef DEBUG_BRANCH_COMMAND_OBJECT
      MessageInterface::ShowMessage
         (wxT("==> found type of '%s'\n"), brCmd, brCmd->GetTypeName().c_str());
      #endif
      
      solverName = brCmd->GetStringParameter(objName);
      solver = FindObject(solverName);
      
      #ifdef DEBUG_BRANCH_COMMAND_OBJECT
      MessageInterface::ShowMessage
         (wxT("   Found solver <%p><%s>'%s'\n"), solver, solver ? solver->GetTypeName().c_str() :
          wxT("NULL"), solver ? solver->GetName().c_str() : wxT("NULL"));
      #endif
   }
   
   while((child = current->GetChildCommand(childNo)) != NULL)
   {
      nextInBranch = child;
      
      while ((nextInBranch != NULL) && (nextInBranch != current))
      {
         #ifdef DEBUG_BRANCH_COMMAND_OBJECT
         MessageInterface::ShowMessage
            (wxT("   nextInBranch=<%p><%s>\n"), nextInBranch, nextInBranch->GetTypeName().c_str());
         #endif
         
         if (nextInBranch->GetTypeName() == childType)
         {
            #ifdef DEBUG_BRANCH_COMMAND_OBJECT
            MessageInterface::ShowMessage
               (wxT("   found '%s', setting <%p>'%s' to <%p><%s>\n"), childType.c_str(),
                solver, solver ? solver->GetName().c_str() : wxT("NULL"), nextInBranch,
                nextInBranch->GetTypeName().c_str());
            #endif
            
            if (solver != NULL)
               nextInBranch->SetRefObject(solver, Gmat::SOLVER, solver->GetName());
         }
         
         if (nextInBranch->GetChildCommand() != NULL)
            SetObjectInBranchCommand(nextInBranch, branchType, childType, objName);
         
         nextInBranch = nextInBranch->GetNext();
      }
      
      ++childNo;
   }
}


//------------------------------------------------------------------------------
// bool IsObjectType(const wxString &type)
//------------------------------------------------------------------------------
/*
 * Returns true if input string is one of Object type that can be created.
 */
//------------------------------------------------------------------------------
bool Interpreter::IsObjectType(const wxString &type)
{
   if (type == wxT("Spacecraft")) 
      return true;
   
   if (type == wxT("Formation")) 
      return true;
   
   if (type == wxT("Propagator")) 
      return true;
   
   if (type == wxT("ForceModel")) 
      return true;
   
   if (type == wxT("CoordinateSystem")) 
      return true;
   
   if (type == wxT("TrackingData"))
      return true;

   if (theSolarSystem->IsBodyInUse(type))
      return true;

   if (find(allObjectTypeList.begin(), allObjectTypeList.end(), type) !=
       allObjectTypeList.end())
      return true;
   
   return false;
}


//------------------------------------------------------------------------------
// Gmat::ObjectType GetObjectType(const wxString &type)
//------------------------------------------------------------------------------
/*
 * Returns corresponding Gmat::ObjectType, or Gmat::UNKNOWN_OBJECT if type is
 * not valid object type name.
 */
//------------------------------------------------------------------------------
Gmat::ObjectType Interpreter::GetObjectType(const wxString &type)
{
   if (objectTypeMap.find(type) != objectTypeMap.end())
      return objectTypeMap[type];
   else
      return Gmat::UNKNOWN_OBJECT;
}

//----------------------------------
// Private
//----------------------------------

//------------------------------------------------------------------------------
// bool IsParameterType(const wxString &desc)
//------------------------------------------------------------------------------
/*
 * Checks if input description is a Parameter.
 * If desctiption has dots, it will parse the components into Object, Depdency,
 * and Type. If type is one of the system parameters, it will return true.
 *
 * @param  desc  Input string to check for Parameter type
 * @return  true  if type is a Parameter type
 */
//------------------------------------------------------------------------------
bool Interpreter::IsParameterType(const wxString &desc)
{
   return theValidator->IsParameterType(desc);
}


//------------------------------------------------------------------------------
// bool CheckForSpecialCase(GmatBase *obj, Integer id, wxString &value)
//------------------------------------------------------------------------------
/**
 * Handles special alias for gravity field type.
 * such as JGM2, JGM3, EGM96, LP165P, etc.
 *
 * @param  obj    Pointer to the object that owns the parameter.
 * @param  id     ID for the parameter.
 * @param  value  Input/Output value of the parameter.
 */
//------------------------------------------------------------------------------
bool Interpreter::CheckForSpecialCase(GmatBase *obj, Integer id, 
                                     wxString &value)
{
   bool retval = false;
   wxString val = value;
   
   #ifdef DEBUG_SPECIAL_CASE
   MessageInterface::ShowMessage
      (wxT("Entered CheckForSpecialCase with \"") + value +
       wxT("\" being set on parameter \"") + obj->GetParameterText(id) + 
       wxT("\" for a \"") + obj->GetTypeName() + wxT("\" object\n"));
   #endif
   
   // JGM2, JGM3, EGM96, LP165P, etc.  are special strings in GMAT; handle them here
   if ((obj->GetTypeName() == wxT("GravityField")) &&
       (obj->GetParameterText(id) == wxT("PotentialFile")))
   {
      val = theModerator->GetPotentialFileName(value);
      if (val.find(wxT("Unknown Potential File Type")) == wxString::npos)
      {
         // Adding a default indicator to the string here, so that the HarmonicField object
         // can tell when it is reading a default file vs. one the user specified
         value = defaultIndicator + val;
         retval = true;
      }
   }
   
   #ifdef DEBUG_SPECIAL_CASE
   MessageInterface::ShowMessage
      (wxT("Leaving CheckForSpecialCase() value=%s, retval=%d\n"), value.c_str(),
       retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// void WriteStringArray(const wxString &title1, const wxString &title2,
//                       const StringArray &parts)
//------------------------------------------------------------------------------
void Interpreter::WriteStringArray(const wxString &title1,
                                   const wxString &title2,
                                   const StringArray &parts)
{
   MessageInterface::ShowMessage(wxT("   ========== %s%s, has %d parts\n"),
                                 title1.c_str(), title2.c_str(), parts.size());
   for (UnsignedInt i=0; i<parts.size(); i++)
      MessageInterface::ShowMessage(wxT("   %d: '%s'\n"), i, parts[i].c_str());
   MessageInterface::ShowMessage(wxT("\n"));
}


//------------------------------------------------------------------------------
// void WriteForceModel(GmatBase *obj)
//------------------------------------------------------------------------------
void Interpreter::WriteForceModel(GmatBase *obj)
{
   ODEModel *fm = (ODEModel*)obj;
   Integer numForces = fm->GetNumForces();
   MessageInterface::ShowMessage
      (wxT("   ODEModel '%s' has %d forces\n"), fm->GetName().c_str(), numForces);
   for (int i = 0; i < numForces; i++)
   {
      const PhysicalModel* force = fm->GetForce(i);
      MessageInterface::ShowMessage
         (wxT("      force[%d] = <%p><%s>'%s'\n"), i, force, force->GetTypeName().c_str(),
          force->GetName().c_str());
   }
}


//------------------------------------------------------------------------------
// bool CheckFunctionDefinition(const wxString &funcPath, GmatBase *function,
//                              bool fullCheck)
//------------------------------------------------------------------------------
/*
 * Opens function file and checks if it has valid function definition line.
 *
 * @param  funcPath  The full path and name of function file
 * @param  function  The Function pointer
 * @param  fullCheck set to true if checking fully for input and output arguments
 *
 */
//------------------------------------------------------------------------------
bool Interpreter::CheckFunctionDefinition(const wxString &funcPath,
                                          GmatBase *function, bool fullCheck)
{
   #if DBGLVL_FUNCTION_DEF > 0
   MessageInterface::ShowMessage
      (wxT("Interpreter::CheckFunctionDefinition() function=<%p>, fullCheck=%d\n   ")
       wxT("funcPath=<%s>\n"), function, fullCheck, funcPath.c_str());
   #endif
   
   debugMsg = wxT("In CheckFunctionDefinition()");
   bool retval = true;
   
   if (function == NULL)
   {
      MessageInterface::ShowMessage
         (wxT("** INTERNAL ERROR ** Cannot check function definition. ")
          wxT("function pointer is NULL\n"));
      retval = false;;
   }
   
   // check if function path exist
   if (!GmatFileUtil::DoesFileExist(funcPath))
   {
      InterpreterException ex
         (wxT("Nonexistent GmatFunction file \"") + funcPath +
          wxT("\" referenced in \"") + function->GetName() + wxT("\"\n"));
      HandleError(ex, false);
      retval = false;
   }
   
   // check for no extension of .gmf or wrong extension
   StringArray parts = GmatStringUtil::SeparateBy(funcPath, wxT("."));
   if ((parts.size() == 1) ||
       (parts.size() == 2 && parts[1] != wxT("gmf")))
   {
      InterpreterException ex
         (wxT("The GmatFunction file \"") + funcPath + wxT("\" has no or incorrect file ")
          wxT("extension referenced in \"") + function->GetName() + wxT("\"\n"));
      HandleError(ex, false);
      retval = false;
   }
   
   if (!retval || !fullCheck)
   {
      #if DBGLVL_FUNCTION_DEF > 0
      MessageInterface::ShowMessage
         (wxT("Interpreter::CheckFunctionDefinition() returning %d, fullCheck=%d\n"),
          retval, fullCheck);
      #endif
      return retval;
   }
   
   // check function declaration
   std::ifstream inStream(funcPath.char_str());
   wxString line;
   StringArray inputArgs;
   StringArray outputArgs;
   
   while (!inStream.eof())
   {
      // Use cross-platform getline()
      if (!GmatFileUtil::GetLine(&inStream, line))
      {
         InterpreterException ex
            (wxT("Error reading the GmatFunction file \"") +
             funcPath + wxT("\" referenced in \"") + function->GetName() + wxT("\"\n"));
         HandleError(ex, false);
         retval = false;
         break;
      }
      
      #if DBGLVL_FUNCTION_DEF > 1
      MessageInterface::ShowMessage(wxT("   line=<%s>\n"), line.c_str());
      #endif
      
      line = GmatStringUtil::Trim(line, GmatStringUtil::BOTH, true, true);
      
      // Skip empty line or comment line
      if (line[0] == '\0' || line[0] == '%')
         continue;
      
      //------------------------------------------------------
      // Parse function definition line
      //------------------------------------------------------
      bool hasOutput = false;
      if (line.find(wxT("=")) != line.npos)
         hasOutput = true;
      
      StringArray parts;
      if (hasOutput)
         parts = GmatStringUtil::SeparateBy(line, wxT("="), true);
      else
         parts = GmatStringUtil::SeparateBy(line, wxT(" "), true);
      
      StringArray::size_type numParts = parts.size();
      
      #if DBGLVL_FUNCTION_DEF > 1
      WriteStringArray(wxT("GmatFunction parts"), wxT(""), parts);
      #endif
         
      StringArray lhsParts;
      
      try
      {
         lhsParts = theTextParser.Decompose(parts[0], wxT("[]"), false);
      }
      catch (BaseException &)
      {
         InterpreterException ex
            (wxT("Invalid output argument list found in the GmatFunction file \"") +
             funcPath + wxT("\" referenced in \"") + function->GetName() + wxT("\"\n"));
         HandleError(ex, false);
         retval = false;
         break;
      }
      
      StringArray::size_type numLeft = lhsParts.size();
      
      #if DBGLVL_FUNCTION_DEF > 1
      WriteStringArray(wxT("GmatFunction lhsParts"), wxT(""), lhsParts);
      #endif
      
      //------------------------------------------------------
      // Check if first part is wxT("function")
      //------------------------------------------------------
      #if DBGLVL_FUNCTION_DEF > 0
      MessageInterface::ShowMessage(wxT("   Check if first part is function\n"));
      #endif
      
      if (numLeft > 0 && lhsParts[0] != wxT("function"))
      {
         InterpreterException ex
            (wxT("The \"function\" is missing in the GmatFunction file \"") +
             funcPath + wxT("\" referenced in \"") + function->GetName() + wxT("\"\n"));
         HandleError(ex, false);
         retval = false;
         break;
      }
      
      //------------------------------------------------------
      // Check for valid output arguments
      //------------------------------------------------------
      #if DBGLVL_FUNCTION_DEF > 0
      MessageInterface::ShowMessage(wxT("   Check for output arguments\n"));
      #endif
      
      if (hasOutput)
      {
         try
         {
            outputArgs =
               theTextParser.SeparateBrackets(lhsParts[1], wxT("[]"), wxT(","));
            
            #if DBGLVL_FUNCTION_DEF > 1
            WriteStringArray(wxT("GmatFunction outputArgs"), wxT(""), outputArgs);
            #endif
         }
         catch (BaseException &)
         {
            InterpreterException ex
               (wxT("Invalid output argument list found in the GmatFunction file \"") +
                funcPath + wxT("\" referenced in \"") + function->GetName() + wxT("\"\n"));
            HandleError(ex, false);
            retval = false;
            break;
         }
         
         
         if (outputArgs.size() == 0)
         {
            InterpreterException ex
               (wxT("The output argument list is empty in the GmatFunction file \"") +
                funcPath + wxT("\" referenced in \"") + function->GetName() + wxT("\"\n"));
            HandleError(ex, false);
            retval = false;
            break;
         }
      }
      
      //------------------------------------------------------
      // Check for missing function name
      //------------------------------------------------------
      #if DBGLVL_FUNCTION_DEF > 0
      MessageInterface::ShowMessage(wxT("   Check for missing function name\n"));
      MessageInterface::ShowMessage(wxT("   hasOutput=%d, numLeft=%d, numParts=%d\n"),
                                    hasOutput, numLeft, numParts);
      #endif
      
      if (numParts <= 1)
      {
         InterpreterException ex
            (wxT("The function name not found in the GmatFunction file \"") +
             funcPath + wxT("\" referenced in \"") + function->GetName() + wxT("\"\n"));
         HandleError(ex, false);
         retval = false;
         break;
      }
      
      //------------------------------------------------------
      // check function name and input arguments
      //------------------------------------------------------
      #if DBGLVL_FUNCTION_DEF > 0
      MessageInterface::ShowMessage(wxT("   Check for input arguments\n"));
      #endif
      
      StringArray rhsParts;
      try
      {
         rhsParts = theTextParser.Decompose(parts[1], wxT("()"), false);
         
         #if DBGLVL_FUNCTION_DEF > 1
         WriteStringArray(wxT("GmatFunction rhsParts"), wxT(""), rhsParts);
         #endif         
      }
      catch (BaseException &)
      {
         InterpreterException ex
            (wxT("The invalid input argument list found in the GmatFunction file \"") +
             funcPath + wxT("\" referenced in \"") + function->GetName() + wxT("\"\n"));
         HandleError(ex, false);
         retval = false;
         break;
      }
      
      //------------------------------------------------------
      // Check if function name matches the file name
      //------------------------------------------------------
      #if DBGLVL_FUNCTION_DEF > 0
      MessageInterface::ShowMessage(wxT("   Check if file has matching function name\n"));
      #endif
      
      wxString fileFuncName = rhsParts[0];
      wxString funcName = function->GetStringParameter(wxT("FunctionName"));
      
      #if DBGLVL_FUNCTION_DEF > 0
      MessageInterface::ShowMessage
         (wxT("   fileFuncName = %s', funcName = '%s'\n\n"), fileFuncName.c_str(), funcName.c_str());
      #endif
      
      if (fileFuncName != funcName)
      {
         InterpreterException ex
            (wxT("The function name \"") + fileFuncName + wxT("\" does not match with the ")
             wxT("GmatFunction file name \"") + funcPath + wxT("\" referenced in \"") +
             function->GetName() + wxT("\"\n"));
         HandleError(ex, false);
         retval = false;
      }
      
      //------------------------------------------------------
      // Check for valid input arguments
      //------------------------------------------------------
      #if DBGLVL_FUNCTION_DEF > 0
      MessageInterface::ShowMessage(wxT("   Check for input arguments\n"));
      #endif
      if (rhsParts.size() > 1)
      {
         try
         {
            inputArgs =
               theTextParser.SeparateBrackets(rhsParts[1], wxT("()"), wxT(","));
            
            #if DBGLVL_FUNCTION_DEF > 1
            WriteStringArray(wxT("GmatFunction inputArgs"), wxT(""), inputArgs);
            #endif
         }
         catch (BaseException &)
         {
            InterpreterException ex
               (wxT("Invalid input argument list found in the GmatFunction file \"") +
                funcPath + wxT("\" referenced in \"") + function->GetName() + wxT("\"\n"));
            HandleError(ex, false);
            retval = false;
            break;
         }
         
         if (inputArgs.size() == 0)
         {
            InterpreterException ex
               (wxT("The input argument list is empty in the GmatFunction file \"") +
                funcPath + wxT("\" referenced in \"") + function->GetName() + wxT("\"\n"));
            HandleError(ex, false);
            retval = false;
            break;
         }
         
         // check for duplicate input list
         #if DBGLVL_FUNCTION_DEF > 0
         MessageInterface::ShowMessage(wxT("   Check for duplicate input arguments\n"));
         #endif
         if (inputArgs.size() > 1)
         {
            StringArray multiples;
            // check for duplicate input names
            for (UnsignedInt i=0; i<inputArgs.size(); i++)
            {
               for (UnsignedInt j=0; j<inputArgs.size(); j++)
               {
                  if (i == j)
                     continue;
                  
                  if (inputArgs[i] == inputArgs[j])
                     if (find(multiples.begin(), multiples.end(), inputArgs[i]) == multiples.end())
                        multiples.push_back(inputArgs[i]);
               }
            }
            
            if (multiples.size() > 0)
            {
               wxString errMsg = wxT("Duplicate input of");
               
               for (UnsignedInt i=0; i<multiples.size(); i++)
                  errMsg = errMsg + wxT(" \"") + multiples[i] + wxT("\"");
               
               InterpreterException ex
                  (errMsg + wxT(" found in the GmatFunction file \"") +
                   funcPath + wxT("\" referenced in \"") + function->GetName() + wxT("\"\n"));
               HandleError(ex, false);
               retval = false;
               break;
            }
         }
      }
      
      break;
   }
   
   if (line == wxT(""))
   {
      InterpreterException ex
         (wxT("The GmatFunction file \"") + funcPath + wxT("\" referenced in \"") +
          function->GetName() + wxT("\" is empty\n"));
      HandleError(ex, false);
      retval = false;
   }
   
   // if function definition has been validated, check if all outputs are declared
   #if DBGLVL_FUNCTION_DEF > 0
   MessageInterface::ShowMessage(wxT("   Check for output declaration\n"));
   #endif
   if (retval && outputArgs.size() > 0)
   {
      wxString errMsg;
      IntegerArray rowCounts, colCounts;
      WrapperTypeArray outputTypes =
         GmatFileUtil::GetFunctionOutputTypes(&inStream, inputArgs, outputArgs, errMsg,
                                              rowCounts, colCounts);
      
      if (errMsg != wxT(""))
      {
         InterpreterException ex
            (errMsg + wxT(" found in the GmatFunction file \"") +
             funcPath + wxT("\" referenced in \"") + function->GetName() + wxT("\"\n"));
         HandleError(ex, false);
         retval = false;
      }
      else
      {
         ((Function*)function)->SetOutputTypes(outputTypes, rowCounts, colCounts);
      }
   }
   
   inStream.close();
   
   
   #if DBGLVL_FUNCTION_DEF > 0
   MessageInterface::ShowMessage
      (wxT("Interpreter::CheckFunctionDefinition() returning true\n"));
   #endif
   
   return retval;
   
} // CheckFunctionDefinition()


//------------------------------------------------------------------------------
// bool BuildFunctionDefinition(const wxString &str)
//------------------------------------------------------------------------------
/*
 * Sets function inputs and output to function from valid function definition
 * string.
 *
 * Note: This methods assumes that input string already has passed function
 *       validation check
 */
//------------------------------------------------------------------------------
bool Interpreter::BuildFunctionDefinition(const wxString &str)
{
   #if DBGLVL_FUNCTION_DEF > 0
   MessageInterface::ShowMessage
      (wxT("Interpreter::BuildFunctionDefinition() str=<%s>\n"), str.c_str());
   #endif
   
   wxString lhs;
   wxString rhs;
   StringArray parts = theTextParser.SeparateBy(str, wxT("="));
   
   #if DBGLVL_FUNCTION_DEF > 1
   WriteStringArray(wxT("parts"), wxT(""), parts);
   #endif
   
   // if function has no output
   if (parts.size() == 1)
   {
      wxString::size_type index = str.find_first_of(wxT(" "));
      lhs = str.substr(0, index);
      rhs = str.substr(index+1);
   }
   else
   {
      lhs = parts[0];
      rhs = parts[1];
   }
   
   StringArray lhsParts = theTextParser.Decompose(lhs, wxT("[]"), false);
   StringArray rhsParts = theTextParser.Decompose(rhs, wxT("()"), false);
   
   #if DBGLVL_FUNCTION_DEF > 1
   WriteStringArray(wxT("lhsParts"), wxT(""), lhsParts);
   WriteStringArray(wxT("rhsParts"), wxT(""), rhsParts);
   #endif
   
   wxString funcName;
   
   if (lhsParts[0] != wxT("function"))
      return false;
   
   if (!GmatStringUtil::IsValidName(rhsParts[0], false))
      return false;
   
   StringArray inputs, outputs;
   
   //------------------------------------------------------
   // parse inputs
   //------------------------------------------------------
   #if DBGLVL_FUNCTION_DEF > 0
   MessageInterface::ShowMessage(wxT("   parse inputs\n"));
   #endif
   
   if (rhsParts.size() > 1)
   {
      inputs = theTextParser.SeparateBy(rhsParts[1], wxT(", ()"));
      
      #if DBGLVL_FUNCTION_DEF > 1
      WriteStringArray(wxT("function inputs"), wxT(""), inputs);
      #endif
   }
   
   //------------------------------------------------------
   // parse outputs
   //------------------------------------------------------
   #if DBGLVL_FUNCTION_DEF > 0
   MessageInterface::ShowMessage(wxT("   parse outputs\n"));
   #endif
   if (lhsParts.size() > 1)
   {
      outputs = theTextParser.SeparateBy(lhsParts[1], wxT(", []"));
      
      #if DBGLVL_FUNCTION_DEF > 1
      WriteStringArray(wxT("function outputs"), wxT(""), outputs);
      #endif
   }
   
   #if DBGLVL_FUNCTION_DEF > 0
   MessageInterface::ShowMessage
      (wxT("   inFunctionMode=%d, currentFunction=<%p>\n"), inFunctionMode,
       currentFunction);
   #endif
   
   //------------------------------------------------------
   // set inputs and outputs to current function
   //------------------------------------------------------
   if (inFunctionMode && currentFunction != NULL)
   {
      for (UnsignedInt i=0; i<inputs.size(); i++)
         currentFunction->SetStringParameter(wxT("Input"), inputs[i]);

      for (UnsignedInt i=0; i<outputs.size(); i++)
         currentFunction->SetStringParameter(wxT("Output"), outputs[i]);
   }
   
   hasFunctionDefinition = true;
   
   #if DBGLVL_FUNCTION_DEF > 0
   MessageInterface::ShowMessage
      (wxT("Interpreter::BuildFunctionDefinition() returning true\n"));
   #endif
   
   return true;
   
} // BuildFunctionDefinition()


//------------------------------------------------------------------------------
// bool Interpreter::HandleMathTree(GmatCommand *cmd)
//------------------------------------------------------------------------------
bool Interpreter::HandleMathTree(GmatCommand *cmd)
{
   #ifdef DEBUG_MATH_TREE
   MessageInterface::ShowMessage
      (wxT("Interpreter::HandleMathTree() '%s', It is a math equation\n"),
       cmd->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
   #endif
   
   Assignment *equation = (Assignment*)cmd;
   wxString lhs = equation->GetLHS();
   wxString rhs = equation->GetRHS();
   
   // Handle GmatFunction in math
   StringArray gmatFuns = equation->GetGmatFunctionNames();
   
   #ifdef DEBUG_MATH_TREE
   MessageInterface::ShowMessage(wxT("   Found %d GmatFunctions\n"), gmatFuns.size());
   #endif
   
   for (UnsignedInt i=0; i<gmatFuns.size(); i++)
   {
      GmatBase *func = FindObject(gmatFuns[i]);
      Integer manage = 1;
      
      // Do not manage function if creating in function mode
      if (inFunctionMode)
         manage = 0;
      
      if (func == NULL)
         func = CreateObject(wxT("GmatFunction"), gmatFuns[i], manage);
      
      #ifdef DEBUG_MATH_TREE
      MessageInterface::ShowMessage
         (wxT("   Setting GmatFunction '%s'<%p> to equation<%p>\n"),
          func->GetName().c_str(), func, equation);
      #endif
      
      equation->SetFunction((Function*)func);
   }
   
   #ifdef DEBUG_MATH_TREE
   MessageInterface::ShowMessage(wxT("Interpreter::HandleMathTree() returning true\n"));
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// void Interpreter::ClearTempObjectNames()
//------------------------------------------------------------------------------
/*
 * Clears temporary object name array.
 * tempObjectNames is used for finding MatlabFunction names.
 * This method is called from the ScriptInterpreter::InterpretGmatFunction()
 */
//------------------------------------------------------------------------------
void Interpreter::ClearTempObjectNames()
{
   tempObjectNames.clear();
}

