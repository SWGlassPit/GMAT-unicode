//$Id: Interpreter.hpp 9783 2011-08-19 21:43:47Z lindajun $
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
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
/**
 * Class definition for the Interpreter base class
 */
//------------------------------------------------------------------------------


#ifndef Interpreter_hpp
#define Interpreter_hpp

#include "gmatdefs.hpp"
#include "InterpreterException.hpp"
#include "GmatBase.hpp"
#include "TextParser.hpp"
#include "ScriptReadWriter.hpp"
#include "ElementWrapper.hpp"

// Forward references for GMAT core objects
class Spacecraft;
class Formation;
class Hardware;
class Propagator;
class ODEModel;
class PropSetup;
class PhysicalModel;
class SolarSystem;
class CelestialBody;
class Parameter;
class GmatCommand;
class CoordinateSystem;
class AxisSystem;
class Subscriber;
class Burn;
class Function;
class Moderator;
class Validator;
class Interface;

/**
 * Interpreter is the base class for the GMAT Interpreter subsystem.  
 * 
 * Interpreter defines the interfaces used to parse the text files that control 
 * execution in GMAT.  It also provides the interfaces to write text files out, 
 * either to the file system, the screen, or the GUI.
 */
class GMAT_API Interpreter
{
public:
   Interpreter(SolarSystem *ss = NULL, ObjectMap *objMap = NULL);
   virtual ~Interpreter();
   
   //------------------------------------------------------------------------------
   // bool Interpret()
   //------------------------------------------------------------------------------
   /**
    * Retrieves text input from a stream and translate it into GMAT objects and
    * actions.
    * 
    * This method gets overridden by derived classes.
    * 
    * @return true on success, false on failure.
    */
   //------------------------------------------------------------------------------
   virtual bool Interpret() = 0;
   
   //------------------------------------------------------------------------------
   // bool Build()
   //------------------------------------------------------------------------------
   /**
    * Accesses GMAT objects and actions and writes them to a stream.
    * 
    * This method gets overridden by derived classes.
    * 
    * @return true on success, false on failure.
    */
   //------------------------------------------------------------------------------
   virtual bool Build(Gmat::WriteMode mode) = 0;
   
   virtual Parameter* CreateSystemParameter(const wxString &str);
   virtual Parameter* CreateParameter(const wxString &type,
                                      const wxString &name,
                                      const wxString &ownerName = wxT(""),
                                      const wxString &depName = wxT(""));
   
   const StringArray& GetListOfObjects(Gmat::ObjectType type);
   const StringArray& GetListOfObjects(const wxString &typeName);
   const StringArray& GetListOfViewableSubtypesOf(Gmat::ObjectType type);
   const StringArray& GetListOfViewableCommands();
   
   GmatBase* GetConfiguredObject(const wxString &name);
   GmatBase* FindObject(const wxString &name, const wxString &ofType = wxT(""));
   GmatBase* CreateObject(const wxString &type, const wxString &name,
                          Integer manage = 1, bool createDefault = false);
   
   void SetConfiguredObjectMap();
   void SetSolarSystemInUse(SolarSystem *ss);
   SolarSystem* GetSolarSystemInUse();
   void SetObjectMap(ObjectMap *objMap, bool forFunction = false);
   ObjectMap* GetObjectMap();
   void SetFunction(Function *func);
   Function* GetFunction();
   
   const StringArray& GetErrorList() { return errorList; }
   void SetHeaderComment(const wxString &comment){headerComment = comment;}
   void SetFooterComment(const wxString &comment){footerComment = comment;}
   
   bool IsObjectType(const wxString &type);
   Gmat::ObjectType GetObjectType(const wxString &type);
   
   // to check commands
   bool ValidateCommand(GmatCommand *cmd);
   // to check subscriber
   bool ValidateSubscriber(GmatBase *obj);
   
   bool SetForceModelProperty(GmatBase *obj, const wxString &prop,
                              const wxString &value, GmatBase *fromObj);
   bool SetDragForceProperty(GmatBase *obj, const wxString &pmType,
                             const wxString &pmField, const wxString &value);
   bool SetMeasurementModelProperty(GmatBase *obj, const wxString &prop,
                              const wxString &value);
   bool SetTrackingDataProperty(GmatBase *obj, const wxString &prop,
                              const wxString &value);
   bool SetTrackingSystemProperty(GmatBase *obj, const wxString &prop,
                              const wxString &value);
   bool SetDataStreamProperty(GmatBase *obj, const wxString &property,
                              const wxString &value);
   bool FindOwnedObject(GmatBase *owner, const wxString toProp,
                        GmatBase **ownedObj, Integer &id, Gmat::ParameterType &type);
   
   bool FindPropertyID(GmatBase *obj, const wxString &chunk, GmatBase **owner,
                       Integer &id, Gmat::ParameterType &type);
   
   void BuildCreatableObjectMaps();
   StringArray GetCreatableList(Gmat::ObjectType type,
                                const wxString subType = wxT(""));
   
   virtual void SetInputFocus();
   virtual void NotifyRunCompleted();
   virtual void UpdateView(Integer type = 7);
   virtual void CloseCurrentProject();
   virtual void StartMatlabServer();
   
   Interface* GetMatlabInterface();
   bool OpenMatlabEngine();
   bool CloseMatlabEngine();   
   
protected:
   
   Moderator    *theModerator;
   SolarSystem  *theSolarSystem;
   Validator    *theValidator;
   
   // Object map to be used for finding objects
   ObjectMap    *theObjectMap;
   // String array to be used for finding temporary object names
   StringArray  tempObjectNames;
   
   /// A pointer to the ScriptReadWriter used when reading or writing script.
   ScriptReadWriter  *theReadWriter;
   TextParser        theTextParser;
   
   bool         inCommandMode;
   bool         inRealCommandMode;
   bool         initialized;
   bool         parsingDelayedBlock;
   bool         ignoreError;
   bool         inScriptEvent;
   
   /// For handling GmatFunction
   bool         inFunctionMode;
   bool         hasFunctionDefinition;
   Function     *currentFunction;
   
   /// For handling delayed blocks
   StringArray  delayedBlocks;
   StringArray  delayedBlockLineNumbers;
   
   /// Block type and comments
   wxString  headerComment;
   wxString  footerComment;
   wxString  currentBlock;
   wxString  currentLine;
   wxString  lineNumber;
   Gmat::BlockType currentBlockType;
   
   /// Error handling data
   bool        continueOnError;
   wxString errorMsg1;
   wxString errorMsg2;
   wxString debugMsg;
   StringArray errorList;
   
   void Initialize();
   void RegisterAliases();
   
   Parameter* GetArrayIndex(const wxString &arrayStr,
                            Integer &row, Integer &col);
   
   AxisSystem* CreateAxisSystem(wxString type, GmatBase *owner);
   
   // for commands
   bool         IsCommandType(const wxString &type);
   void         ParseAndSetCommandName(GmatCommand *cmd, const wxString &cmdType,
                                       const wxString &desc, wxString &newDesc);
   GmatCommand* CreateCommand(const wxString &type, const wxString &desc,
                              bool &retFlag, GmatCommand *inCmd = NULL);
   GmatCommand* AppendCommand(const wxString &type, bool &retFlag,
                              GmatCommand *inCmd = NULL);
   GmatCommand* CreateAssignmentCommand(const wxString &lhs,
                                        const wxString &rhs, bool &retFlag,
                                        GmatCommand *inCmd = NULL);
   
   bool AssembleCommand(GmatCommand *cmd, const wxString &desc);
   bool AssembleCallFunctionCommand(GmatCommand *cmd, const wxString &desc);
   bool AssembleConditionalCommand(GmatCommand *cmd, const wxString &desc);
   bool AssembleForCommand(GmatCommand *cmd, const wxString &desc);
   bool AssembleGeneralCommand(GmatCommand *cmd, const wxString &desc);
   bool AssembleTargetCommand(GmatCommand *cmd, const wxString &desc);
   bool AssembleOptimizeCommand(GmatCommand *cmd, const wxString &desc);
   bool AssembleFiniteBurnCommand(GmatCommand *cmd, const wxString &desc);
   bool AssembleReportCommand(GmatCommand *cmd, const wxString &desc);
   bool AssembleCreateCommand(GmatCommand *cmd, const wxString &desc);
   bool SetCommandRefObjects(GmatCommand *cmd, const wxString &desc);
   
   // for assignment
   GmatBase* MakeAssignment(const wxString &lhs, const wxString &rhs);
   
   // for setting whole object
   bool SetObjectToObject(GmatBase *toObj, GmatBase *fromObj, const wxString &rhs);
   bool SetObjectToProperty(GmatBase *toObj, GmatBase *fromOwner,
                            const wxString &fromProp);
   bool SetObjectToArray(GmatBase *toObj, const wxString &fromArray);
   bool SetObjectToValue(GmatBase *toObj, const wxString &value);
   
   // for setting property
   bool SetPropertyToObject(GmatBase *toOwner, const wxString &toProp,
                            GmatBase *fromObj);
   bool SetPropertyToProperty(GmatBase *toOwner, const wxString &toProp,
                              GmatBase *fromOwner, const wxString &fromProp);
   bool SetPropertyToArray(GmatBase *toOwner, const wxString &toProp,
                           const wxString &fromArray);
   bool SetPropertyToValue(GmatBase *toOwner, const wxString &toProp,
                           const wxString &value);
   
   // for setting array
   bool SetArrayToObject(GmatBase *toArrObj, const wxString &toArray,
                         GmatBase *fromObj);
   bool SetArrayToProperty(GmatBase *toArrObj, const wxString &toArray,
                           GmatBase *fromOwner, const wxString &fromProp);
   bool SetArrayToArray(GmatBase *toArrObj, const wxString &toArray,
                        GmatBase *fromArrObj, const wxString &fromArray);
   bool SetArrayToValue(GmatBase *toArrObj, const wxString &toArray,
                        const wxString &value);
   
   // for setting/getting property value
   bool SetPropertyValue(GmatBase *obj, const Integer id,
                         const Gmat::ParameterType type,
                         const wxString &value,
                         const Integer index = -1, const Integer colIndex = -1);
   bool SetPropertyObjectValue(GmatBase *obj, const Integer id,
                               const Gmat::ParameterType type,
                               const wxString &value,
                               const Integer index = -1);
   bool SetPropertyStringValue(GmatBase *obj, const Integer id,
                               const Gmat::ParameterType type,
                               const wxString &value,
                               const Integer index = -1);
   
   wxString GetPropertyValue(GmatBase *obj, const Integer id);
   
   bool SetProperty(GmatBase *obj, const Integer id,
                    const Gmat::ParameterType type, const wxString &value);
   
   bool SetComplexProperty(GmatBase *obj, const wxString &prop,
                           const wxString &value);
   bool SetSolarSystemProperty(GmatBase *obj, const wxString &prop,
                               const wxString &value);
   
   // for setting/getting array value
   Real GetArrayValue(const wxString &arrayStr, Integer &row, Integer &col);
   bool IsArrayElement(const wxString &str);
   
   // for Variable expression
   bool ParseVariableExpression(Parameter *var, const wxString &exp);
   
   // for error handling
   void HandleError(const BaseException &e, bool writeLine = true, bool warning = false);
   void HandleErrorMessage(const BaseException &e, const wxString &lineNumber,
                           const wxString &line, bool writeLine = true,
                           bool warning = false);
   
   // for branch command checking
   bool IsBranchCommand(const wxString &str);
   bool CheckBranchCommands(const IntegerArray &lineNumbers,
                            const StringArray &lines);
   
   // for setting object inside branch command
   void SetObjectInBranchCommand(GmatCommand *brCmd, const wxString &branchType,
                                 const wxString &childType,
                                 const wxString &objName);
   
   // Final setting of reference object pointers needed by the GUI
   bool FinalPass();
   
   // for debug
   void WriteStringArray(const wxString &title1, const wxString &title2,
                         const StringArray &parts);
   void WriteForceModel(GmatBase *obj);
   
   // for GamtFunction handling
   bool CheckFunctionDefinition(const wxString &funcPathAndName,
                                GmatBase *function, bool fullCheck = true);
   bool BuildFunctionDefinition(const wxString &str);
   void ClearTempObjectNames();
   
   bool ValidateMcsCommands(GmatCommand *first, GmatCommand *parent = NULL,
         StringArray *missingObjects = NULL, wxString
         *accumulatedErrors = NULL);

private:
      
   StringArray   commandList;
   StringArray   atmosphereList;
   StringArray   attitudeList;
   StringArray   axisSystemList;
   StringArray   burnList;
   StringArray   calculatedPointList;
   StringArray   dataFileList;
   StringArray   ephemFileList;
   StringArray   functionList;
   StringArray   hardwareList;
   StringArray   measurementList;
   StringArray   trackingSystemList;
   StringArray   obtypeList;
   StringArray   odeModelList;
   StringArray   parameterList;
   StringArray   physicalModelList;
   StringArray   propagatorList;
   StringArray   solverList;
   StringArray   stopcondList;
   StringArray   subscriberList;
   StringArray   spacePointList;
   StringArray   celestialBodyList;
   
   StringArray   matlabFunctionNames;
   
   static StringArray   allObjectTypeList;
   static StringArray   viewableCommandList;
   static std::map<wxString, Gmat::ObjectType> objectTypeMap;
   bool IsParameterType(const wxString &desc);
   bool CheckForSpecialCase(GmatBase *obj, Integer id, wxString &value);
   bool CheckUndefinedReference(GmatBase *obj, bool writeLine = true);
   bool HandleMathTree(GmatCommand *cmd);

   static const wxString defaultIndicator;
};

#endif // INTERPRETER_HPP
