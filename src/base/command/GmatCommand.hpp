//$Id: GmatCommand.hpp 9795 2011-08-26 03:10:04Z wendys-dev $
//------------------------------------------------------------------------------
//                                  GmatCommand
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
// Created: 2003/09/23
//
/**
 * Definition for the GmatCommand base class
 */
//------------------------------------------------------------------------------


#ifndef Command_hpp
#define Command_hpp


#include <map>              // for mapping between object names and types
#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "CommandException.hpp"

// Headers used by commands that override InterpretAction
#include "TextParser.hpp"
#include "ElementWrapper.hpp"

// Headers for the referenced classes
#include "SolarSystem.hpp"    // for SolarSystem
#include "Publisher.hpp"      // For the Publisher and ...
#include "SpaceObject.hpp"    // for SpaceObjects
#include "TriggerManager.hpp" // Trigger managers, usually from a plugin

// Forward reference for the transient force vector
class PhysicalModel;

// forward reference for the calling FunctionManager
class FunctionManager;

// forward reference for the function containing this command
class Function;

// Forward reference for event locators
class EventLocator;

/**
 * GMAT GmatCommand Base Class, used for timeline elements in the script
 *
 * The GMAT GmatCommands all follow a "late-binding" philosophy, in that they do
 * not set object associations until the Sandbox has been populated with both
 * the objects that are used in the model and with the complete GmatCommand 
 * sequence.  Once the Sandbox is populated, it initializes the GmatCommand 
 * sequence by calling Initialize() on each GmatCommand, and then runs the
 * sequence by calling Execute() on the first GmatCommand in the sequence.
 */
class GMAT_API GmatCommand : public GmatBase
{
public:
   // class constructor
   GmatCommand(const wxString &typeStr);
   // class destructor
   virtual ~GmatCommand();
   // Copy constructor
   GmatCommand(const GmatCommand &c);
   // Assignment operator
   GmatCommand&         operator=(const GmatCommand &c);
   
   void                 SetGeneratingString(const wxString &gs);
   virtual const wxString&  
                        GetGeneratingString(
                           Gmat::WriteMode mode = Gmat::SCRIPTING,
                           const wxString &prefix = wxT(""),
                           const wxString &useName = wxT(""));
   
   // Methods for function
   virtual void         SetCurrentFunction(Function *function);
   virtual Function*    GetCurrentFunction();
   
   virtual void         SetCallingFunction(FunctionManager *fm);
   
   // other methods for setting up the object w.r.t the elements needed
   virtual const StringArray& 
                        GetWrapperObjectNameArray();
   virtual bool         SetElementWrapper(ElementWrapper* toWrapper,
                        const wxString &withName);
   virtual void         ClearWrappers();
   virtual void         CheckDataType(ElementWrapper* forWrapper,
                                      Gmat::ParameterType needType,
                                      const wxString &cmdName,
                                      bool ignoreUnsetReference = false);
   
   // Methods used to setup objects
   virtual bool         SetObject(const wxString &name,
                                  const Gmat::ObjectType type,
                                  const wxString &associate = wxT(""),
                                  const Gmat::ObjectType associateType =
                                       Gmat::UNKNOWN_OBJECT);
   virtual bool         SetObject(GmatBase *obj,
                                  const Gmat::ObjectType type);
   virtual GmatBase*    GetGmatObject(const Gmat::ObjectType type, 
                                  const wxString objName = wxT(""));
   
   virtual void         SetInternalCoordSystem(CoordinateSystem *cs);
   virtual void         SetupSummary(const wxString &csName, bool entireMission = false, bool physicsOnly = false);
   virtual void         SetSummaryName(const wxString &sumName);
   virtual wxString  GetSummaryName();
   virtual void         SetSolarSystem(SolarSystem *ss);
   virtual void         SetTriggerManagers(std::vector<TriggerManager*> *trigs);
   virtual void         SetObjectMap(std::map<wxString, GmatBase *> *map);
   virtual ObjectMap*   GetObjectMap();
   virtual void         SetGlobalObjectMap(std::map<wxString, GmatBase *> *map);
   virtual void         SetTransientForces(std::vector<PhysicalModel*> *tf);
   virtual void         SetEventLocators(std::vector<EventLocator*> *els);
   virtual void         SetPublisher(Publisher *p);
   virtual Publisher*   GetPublisher();
   
   // Methods used in validation
   virtual const StringArray& GetObjectList();
   virtual bool         Validate();

   // Access methods inherited from GmatBase
   virtual wxString GetParameterText(const Integer id) const;
   virtual Integer     GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                       GetParameterType(const Integer id) const;
   virtual wxString GetParameterTypeString(const Integer id) const;
   
   virtual bool        IsParameterReadOnly(const Integer id) const;
   virtual bool        IsParameterReadOnly(const wxString &label) const;
   
   virtual wxString GetStringParameter(const Integer id) const;
   virtual wxString GetStringParameter(const Integer id,
                                          const Integer index) const;
   virtual bool        SetStringParameter(const Integer id, 
                                          const wxString &value);
   virtual bool        SetStringParameter(const Integer id, 
                                          const wxString &value,
                                          const Integer index);
   virtual wxString GetStringParameter(const wxString &label) const;
   virtual wxString GetStringParameter(const wxString &label,
                                          const Integer index) const;
   virtual bool        SetStringParameter(const wxString &label, 
                                          const wxString &value);
   virtual bool        SetStringParameter(const wxString &label, 
                                          const wxString &value,
                                          const Integer index);
   
   // methods to set conditions and operators between multiple conditions;
   // atIndex = -999 means add it to the end of the list; otherwise, add/replace
   // the condition at the specified index
   virtual bool         SetCondition(const wxString &lhs, 
                                     const wxString &operation, 
                                     const wxString &rhs, 
                                     Integer atIndex = -999);
   virtual bool         SetConditionOperator(const wxString &op, 
                                             Integer atIndex = -999);
   
   virtual bool         RemoveCondition(Integer atIndex);
   virtual bool         RemoveConditionOperator(Integer atIndex);
   
   // Sequence methods
   virtual bool         Initialize();
   virtual GmatCommand* GetNext();
   virtual GmatCommand* GetPrevious();
   virtual bool         ForceSetNext(GmatCommand *toCmd);     // dangerous!
   virtual bool         ForceSetPrevious(GmatCommand *toCmd); // dangerous!
   virtual bool         Append(GmatCommand *cmd);
   virtual bool         Insert(GmatCommand *cmd, GmatCommand *prev);
   virtual GmatCommand* Remove(GmatCommand *cmd);
   
   virtual GmatCommand* GetChildCommand(Integer whichOne = 0);
   virtual Integer      GetVariableCount();
   virtual Integer      GetGoalCount();
   
   virtual bool         InterpretAction();
   
   Integer              DepthIncrement();
   bool                 HasPropStateChanged();
   
   //---------------------------------------------------------------------------
   //  bool GmatCommand::Execute()
   //---------------------------------------------------------------------------
   /**
    * The method that is fired to perform the GmatCommand.
    *
    * Derived classes implement this method to perform their actions on
    * GMAT objects.
    *
    * @return true if the GmatCommand runs to completion, false if an error
    *         occurs.
    */
   //---------------------------------------------------------------------------
   virtual bool        Execute() = 0;
   virtual bool        SkipInterrupt();
   virtual void        RunComplete();
   
   bool                HasConfigurationChanged();
   virtual void        ConfigurationChanged(bool tf, bool setAll = false);
   virtual bool        HasAFunction();
   virtual bool        NeedsServerStartup();
   virtual bool        IsExecuting();
   
   virtual Integer     GetCloneCount();
   virtual GmatBase*   GetClone(Integer cloneIndex = 0);

protected:
   enum
   {
      COMMENT = GmatBaseParamCount,
      SUMMARY,
      MISSION_SUMMARY,
      GmatCommandParamCount
   };
   
   /// Command parameter labels
   static const wxString 
                     PARAMETER_TEXT[GmatCommandParamCount - GmatBaseParamCount];
   /// Command parameter types
   static const Gmat::ParameterType 
                     PARAMETER_TYPE[GmatCommandParamCount - GmatBaseParamCount];
   
   /// Flag used to determine if associations have been made
   bool                 initialized;
   /// Map containing names and associated types
   ObjectTypeMap        association;
   /// List of the associated objects
   StringArray          objects;
   // pointer to the function that contains this command
   Function            *currentFunction;
   // pointer to the function that is calling this command (ignored for all but
   // CallFunction and Assignment)
   FunctionManager      *callingFunction;
   /// Pointer to the next GmatCommand in the sequence; NULL at the end
   GmatCommand          *next;
   /// Pointer to the previous GmatCommand in the sequence; NULL if at the start
   GmatCommand          *previous;
   /// Indicator of the current nesting level
   Integer              level;
   /// Object store obtained from the Sandbox
   ObjectMap            *objectMap;
   /// Object store obtained from the Sandbox
   ////std::map<wxString, GmatBase *> *globalObjectMap;
   ObjectMap            *globalObjectMap;
   /// Solar System, set by the local Sandbox
   SolarSystem          *solarSys;
   /// Trigger managers, set by the local Sandbox
   std::vector<TriggerManager*> *triggerManagers;
   /// Internal coordinate system, set by the local Sandbox
   CoordinateSystem     *internalCoordSys;
   /// transient forces to pass to the function
   std::vector<PhysicalModel *> 
                        *forces;
   /// Event locators used for event detection
   std::vector<EventLocator*>
                        *events;
   /// Publisher for data generated by this GmatCommand
   Publisher            *publisher;
   /// Stream ID issued by the Publisher to identify which Command is publishing
   Integer              streamID;
   /// Change in branch depth caused by this command
   Integer              depthChange;
   /// Flag indicating that the command changes the state vector for propagation
   bool                 commandChangedState;
   /// String used for the command summary data
   wxString          commandSummary;
   /// Coordinate System used for the Command Summary display
   wxString          summaryCoordSysName;
   /// Current coordinate system for Command Summary
   CoordinateSystem     *summaryCoordSys;
   /// flag indicating whether or not the summary for the command is part of
   /// a requested summary for the entire misison (or branch)
   bool                 summaryForEntireMission;
   /// flag indicating whether or not the entire-mission summary should only include
   /// physics-based commands
   bool                 missionPhysicsBasedOnly;
   /// flag indicating whether or not this command is a physics-based command
   bool                 physicsBasedCommand;
   /// flag indicating whether or not to include this type of command in a command summary
   bool                 includeInSummary;
   /// node name (on the GUI) for the command
   wxString          summaryName;

//   /// Optional comment string associated with the command
//   wxString          comment;
   /// Flag indicating that a command has been changed by a user
   bool                 commandChanged;
   
   /// Text parser used by commands that override InterpretAction
   TextParser           parser;
   /// The list of settable entities for the command
   StringArray          settables;
   /// The list of names of Wrapper objects
   StringArray          wrapperObjectNames;
   /// List used to initialize the local TextParser
   StringArray          commandNameList;
   
   /// Count of owned objects created through cloning
   Integer              cloneCount;

   virtual bool         AssignObjects();
   virtual bool         ClearObjects();
   virtual void         BuildCommandSummary(bool commandCompleted = true);
   virtual void         BuildCommandSummaryString(bool commandCompleted = true);
   virtual const wxString 
                        BuildMissionSummaryString(const GmatCommand* head = NULL);
   virtual const wxString
                        BuildNumber(Real value, bool useExp = false, Integer length = 17);
   // for command name
   virtual void         InsertCommandName(wxString &genString);
   
   // for Debug
   virtual void         ShowCommand(const wxString &prefix,
                                    const wxString &title1, GmatCommand *cmd1,
                                    const wxString &title2 = wxT(""),
                                    GmatCommand *cmd2 = NULL);
   virtual void         ShowObjectMaps(const wxString &title = wxT(""));
   
   // for the Parameters in Commands updates
   StringArray          InterpretPreface();
   bool                 IsSettable(const wxString &setDesc);
   bool                 SeparateEquals(const wxString &description,
                                       wxString &lhs, wxString &rhs,
                                       bool checkOp = false);
   
   // IDs used to buffer the command summary data
   static Integer       satEpochID;
   static Integer       satCdID;
   static Integer       satDragAreaID;
   static Integer       satCrID;
   static Integer       satSRPAreaID;
   static Integer       satTankID;
   static Integer       satThrusterID;
   static Integer       satDryMassID;
   static Integer       satTotalMassID;
   

   // Command summary data buffers
   Real                 *epochData;
   Real                 *stateData;
   Real                 *parmData;
   std::vector <SpaceObject*>
                        satVector;
   Integer              satsInMaps;

   // Temporary -- replace when convenient
   void CartToKep(const Rvector6 in, Rvector6 &out);
   GmatBase* FindObject(const wxString &name);
   
   // Method(s) used for ParametersInCommands
   bool                SetWrapperReferences(ElementWrapper &wrapper);
   
   // Used for deleting old ElementWrappers
   std::vector<ElementWrapper*> oldWrappers;   
   void                ClearOldWrappers();
   void                CollectOldWrappers(ElementWrapper **wrapper);
   void                DeleteOldWrappers();

   // Publish methods that are overridden as needed
   virtual void        PrepareToPublish(bool publishAll = true);
   virtual void        PublishData();

};

#endif // Command_hpp
