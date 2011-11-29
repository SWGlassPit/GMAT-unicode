//$Id: GuiInterpreter.hpp 9914 2011-09-26 19:07:00Z lindajun $
//------------------------------------------------------------------------------
//                              GuiInterpreter
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
 * Declares the operations between GUI subsystem and the Moderator.
 */
//------------------------------------------------------------------------------
#ifndef GuiInterpreter_hpp
#define GuiInterpreter_hpp

#include "ScriptInterpreter.hpp"
#include "gmatdefs.hpp"
#include <sstream>      // for std::istringstream

// GMAT_API not used here because the GuiInterpreter is not exported from a dll
class GuiInterpreter : public ScriptInterpreter
{
public:

   static GuiInterpreter* Instance();

   // Interpreter abstract methods
   virtual bool Interpret(GmatCommand *inCmd, wxInputStream *ss);
   
   void Finalize();
   
   //----- running object
   GmatBase* GetRunningObject(const wxString &name);
   
   //----- factory
   const StringArray& GetListOfFactoryItems(Gmat::ObjectType type);
   const StringArray& GetListOfAllFactoryItems();
   wxString GetStringOfAllFactoryItemsExcept(const ObjectTypeArray &types);
   
   //----- configuration
   wxString GetNewName(const wxString &name, Integer startCount);
   wxString AddClone(const wxString &name);
   bool RenameObject(Gmat::ObjectType type, const wxString &oldName,
                     const wxString &newName);
   bool RemoveObject(Gmat::ObjectType type, const wxString &name);
   bool RemoveObjectIfNotUsed(Gmat::ObjectType type, const wxString &name);
   bool HasConfigurationChanged(Integer sandboxNum = 1);
   void ConfigurationChanged(GmatBase *obj, bool tf);
   void ResetConfigurationChanged(bool resetResource = true,
                                  bool resetCommands = true,
                                  Integer sandboxNum = 1);
   
   // General Object
   GmatBase* CreateObject(const wxString &type, const wxString &name,
                          Integer manage = 1, bool createDefault = false);
   
   // SolarSystem
   SolarSystem* GetDefaultSolarSystem();
   SolarSystem* GetSolarSystemInUse();
   CoordinateSystem* GetInternalCoordinateSystem();
   bool IsDefaultCoordinateSystem(const wxString &name);
   
   // Parameter
   bool IsParameter(const wxString &type);
   Parameter* GetParameter(const wxString &name);
   virtual Parameter* CreateParameter(const wxString &type,
                                      const wxString &name,
                                      const wxString &ownerName = wxT(""),
                                      const wxString &depName = wxT(""),
                                      bool manage = true);
   
   // Subscriber
   Subscriber* CreateSubscriber(const wxString &type,
                                const wxString &name,
                                const wxString &filename = wxT(""),
                                bool createDefault = true);
   Integer GetNumberOfActivePlots();
   
   GmatBase* CreateDefaultPropSetup(const wxString &name);
   GmatBase* CreateNewODEModel(const wxString &name);
   
   // Planetary source
   const StringArray& GetPlanetarySourceTypes();
   const StringArray& GetPlanetarySourceNames();
   const StringArray& GetPlanetarySourceTypesInUse();
//   const StringArray& GetAnalyticModelNames();
//   bool SetAnalyticModelToUse(const wxString &modelName);
   bool SetPlanetarySourceName(const wxString &sourceType,
                               const wxString &filename);
   Integer SetPlanetarySourceTypesInUse(const StringArray &sourceTypes);
   wxString GetPlanetarySourceName(const wxString &sourceType);
   
   // Potential field files
   wxString GetPotentialFileName(const wxString &fileType);
   
   // Getting file names
   wxString GetFileName(const wxString &fileType);
   
   // StopCondition
   GmatBase* CreateStopCondition(const wxString &type,
                                 const wxString &name);
   
   // Command
   GmatCommand* CreateDefaultCommand(const wxString &type,
                                     const wxString &name = wxT(""),
                                     GmatCommand *refCmd = NULL);
   GmatCommand* AppendCommand(const wxString &type, const wxString &name,
                              bool &retFlag, Integer sandboxNum = 1);
   GmatCommand* DeleteCommand(GmatCommand *cmd, Integer sandboxNum = 1);
   GmatCommand* GetFirstCommand(Integer sandboxNum = 1);
   bool AppendCommand(GmatCommand *cmd, Integer sandboxNum = 1);
   bool InsertCommand(GmatCommand *cmd, GmatCommand *prevCmd,
                      Integer sandboxNum = 1);
   
   // Resource
   bool ClearResource();
   
   // Command sequence
   bool LoadDefaultMission();
   bool ClearCommandSeq(Integer sandboxNum = 1);
   
   // Sandbox
   void ClearAllSandboxes();
   Integer RunMission(Integer sandboxNum = 1);
   Integer ChangeRunState(const wxString &state, Integer sandboxNum = 1);
   
   // Script
   bool InterpretScript(const wxString &filename, bool readBack = false,
                        const wxString &newPath = wxT(""));
   bool SaveScript(const wxString &filename,
                   Gmat::WriteMode mode = Gmat::SCRIPTING);
   wxString GetScript(Gmat::WriteMode mode = Gmat::SCRIPTING);
   
   Integer RunScript(Integer sandboxNum = 1);

   // GUI control
   virtual void SetInputFocus();
   virtual void NotifyRunCompleted();
   virtual void UpdateView(Integer type = 7);
   virtual void CloseCurrentProject();
   
   void UpdateResourceTree();
   void UpdateMissionTree();
   void UpdateOutputTree();
   virtual void StartMatlabServer();
   
   std::vector<Gmat::PluginResource*> *GetUserResources();

private:

   GuiInterpreter();
   virtual ~GuiInterpreter();
   GuiInterpreter(const GuiInterpreter&);
   GuiInterpreter& operator=(const GuiInterpreter &guiInterpreter);

   // member data
   bool isInitialized;

   static GuiInterpreter *instance;
};


#endif // GuiInterpreter_hpp

