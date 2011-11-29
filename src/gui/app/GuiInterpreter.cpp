//$Id: GuiInterpreter.cpp 9914 2011-09-26 19:07:00Z lindajun $
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
 * Implements the operations between GUI subsystem and the Moderator.
 */
//------------------------------------------------------------------------------
#if !defined __CONSOLE_APP__
#include "GmatAppData.hpp"
#include "GmatMainFrame.hpp"
#include "ResourceTree.hpp"
#include "MissionTree.hpp"
#include "OutputTree.hpp"
#endif

#include "gmatdefs.hpp"
#include "GuiInterpreter.hpp"
#include "Moderator.hpp"


GuiInterpreter* GuiInterpreter::instance = NULL;

//---------------------------------
// public
//---------------------------------

//------------------------------------------------------------------------------
// static Instance()
//------------------------------------------------------------------------------
GuiInterpreter* GuiInterpreter::Instance()
{
   if (instance == NULL)
      instance = new GuiInterpreter();
   return instance;
}


//------------------------------------------------------------------------------
// ~GuiInterpreter()
//------------------------------------------------------------------------------
GuiInterpreter::~GuiInterpreter()
{
}


//------------------------------------------------------------------------------
// bool Interpret(GmatCommand *inCmd, std::istringstream *ss)
//------------------------------------------------------------------------------
bool GuiInterpreter::Interpret(GmatCommand *inCmd, wxInputStream *ss)
{   
   SetInStream(ss);
   inScriptEvent = true;
   // We don't want parse first comment as header, so set skipHeader to true.
   bool retval = ScriptInterpreter::Interpret(inCmd, true);
   inScriptEvent = false;
   return retval;
}


//------------------------------------------------------------------------------
// void Finalize()
//------------------------------------------------------------------------------
void GuiInterpreter::Finalize()
{
   theModerator->Finalize();
}


//------------------------------------------------------------------------------
// GmatBase* GetRunningObject(const wxString &name)
//------------------------------------------------------------------------------
GmatBase* GuiInterpreter::GetRunningObject(const wxString &name)
{
   return theModerator->GetInternalObject(name);
}


//------------------------------------------------------------------------------
// const StringArray& GetListOfFactoryItems(Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Returns names of all creatable items of object type.
 *
 * @param <type> object type
 *
 * @return array of item names; return empty array if none
 */
//------------------------------------------------------------------------------
const StringArray& GuiInterpreter::GetListOfFactoryItems(Gmat::ObjectType type)
{
   return theModerator->GetListOfFactoryItems(type);
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
const StringArray& GuiInterpreter::GetListOfAllFactoryItems()
{
   return theModerator->GetListOfAllFactoryItems();
}


//------------------------------------------------------------------------------
// wxString GetStringOfAllFactoryItemsExcept(const ObjectTypeArray &types)
//------------------------------------------------------------------------------
/**
 * Return a wxString of all items that can be created except input object types
 *
 * @param <type> object types to be excluded
 *
 * @return list of all creatable items.
 */
//------------------------------------------------------------------------------
wxString GuiInterpreter::GetStringOfAllFactoryItemsExcept(const ObjectTypeArray &types)
{
   StringArray creatables = theModerator->GetListOfAllFactoryItemsExcept(types);
   wxString str;
   
   for (UnsignedInt i = 0; i < creatables.size(); i++)
      str = str + creatables[i] + wxT(" ");
   
   return str;
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
wxString GuiInterpreter::GetNewName(const wxString &name, Integer startCount)
{
   return theModerator->GetNewName(name, startCount);
}


//------------------------------------------------------------------------------
// wxString AddClone(const wxString &name)
//------------------------------------------------------------------------------
/*
 * Adds the clone of the named object to configuration.
 * It gives new name by adding counter to the name to be cloned.
 *
 * @return new name if object was cloned and added to configuration, blank otherwise
 */
//------------------------------------------------------------------------------
wxString GuiInterpreter::AddClone(const wxString &name)
{
   return theModerator->AddClone(name);
}


//------------------------------------------------------------------------------
// bool RenameObject(Gmat::ObjectType type, const wxString &oldName
//                   const wxString &newName)
//------------------------------------------------------------------------------
/**
 * Renames item from the configured list.
 *
 * @param <type> object type
 * @param <oldName>  old object name
 * @param <newName>  new object name
 *
 * @return true if the item has been removed; false otherwise
 */
//------------------------------------------------------------------------------
bool GuiInterpreter::RenameObject(Gmat::ObjectType type,
                                  const wxString &oldName,
                                  const wxString &newName)
{
   return theModerator->RenameObject(type, oldName, newName);
}


//------------------------------------------------------------------------------
// bool RemoveObject(Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Removes item from the configured list.
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return true if the item has been removed; false otherwise
 */
//------------------------------------------------------------------------------
bool GuiInterpreter::RemoveObject(Gmat::ObjectType type,
                                  const wxString &name)
{
   return theModerator->RemoveObject(type, name, false);
}


//------------------------------------------------------------------------------
// bool RemoveItemIfNotUsed(Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Removes item from the configured list if it is not used in the mission
 * sequence.
 *
 * @param <type> object type
 * @param <name> object name
 *
 * @return true if the item has been removed; false otherwise
 */
//------------------------------------------------------------------------------
bool GuiInterpreter::RemoveObjectIfNotUsed(Gmat::ObjectType type,
                                          const wxString &name)
{
   return theModerator->RemoveObject(type, name, true);
}


//------------------------------------------------------------------------------
// bool HasConfigurationChanged(Integer sandboxNum = 1)
//------------------------------------------------------------------------------
bool GuiInterpreter::HasConfigurationChanged(Integer sandboxNum)
{
   return theModerator->HasConfigurationChanged(sandboxNum);
}


//------------------------------------------------------------------------------
// void ConfigurationChanged(GmatBase *obj, bool tf)
//------------------------------------------------------------------------------
void GuiInterpreter::ConfigurationChanged(GmatBase *obj, bool tf)
{
   #if !defined __CONSOLE_APP__
   
   GmatMainFrame *mainFrame = GmatAppData::Instance()->GetMainFrame();
   theModerator->ConfigurationChanged(obj, tf);
   
   if (tf == true)
   {
      #ifdef DEBUG_SYNC_STATUS
      MessageInterface::ShowMessage
         (wxT("GuiInterpreter::ConfigurationChanged() Setting GUI dirty\n"));
      #endif
      mainFrame->UpdateGuiScriptSyncStatus(2, 0); // Set GUI dirty
   }
   
   #else
   
   theModerator->ConfigurationChanged(obj, tf);
   
   #endif
}


//------------------------------------------------------------------------------
// void ResetConfigurationChanged(bool resetResource = true,
//                                bool resetCommands = true,
//                                Integer sandboxNum = 1)
//------------------------------------------------------------------------------
void GuiInterpreter::ResetConfigurationChanged(bool resetResource,
                                               bool resetCommands,
                                               Integer sandboxNum)
{
   theModerator->ResetConfigurationChanged(resetResource, resetCommands, sandboxNum);
}


//------------------------------------------------------------------------------
// GmatBase* CreateObject(const wxString &type, const wxString &name,
//                        Integer manage, bool createDefault)
//------------------------------------------------------------------------------
/**
 * Creates an object by calling Interpreter::CreateObject()
 */
//------------------------------------------------------------------------------
GmatBase* GuiInterpreter::CreateObject(const wxString &type,
                                       const wxString &name,
                                       Integer manage, bool createDefault)
{
   #if !defined __CONSOLE_APP__
   
   #ifdef DEBUG_SYNC_STATUS
   MessageInterface::ShowMessage
      (wxT("GuiInterpreter::CreateObject() type='%s', name='%s', Setting GUI dirty\n"),
       type.c_str(), name.c_str());
   #endif
   
   GmatMainFrame *mainFrame = GmatAppData::Instance()->GetMainFrame();
   GmatBase *obj = Interpreter::CreateObject(type, name, manage, createDefault);
   if (obj == NULL)
      mainFrame->UpdateGuiScriptSyncStatus(3, 0); // Set GUI error
   else
      mainFrame->UpdateGuiScriptSyncStatus(2, 0); // Set GUI dirty
   
   return obj;
   
   #else
   
   return Interpreter::CreateObject(type, name, manage, createDefault);
   
   #endif
}


//------------------------------------------------------------------------------
// SolarSystem* GetDefaultSolarSystem()
//------------------------------------------------------------------------------
/**
 * Retrieves a default solar system object pointer.
 *
 * @return a default solar system object pointer
 */
//------------------------------------------------------------------------------
SolarSystem* GuiInterpreter::GetDefaultSolarSystem()
{
   return theModerator->GetDefaultSolarSystem();
}

//------------------------------------------------------------------------------
// SolarSystem* GetSolarSystemInUse()
//------------------------------------------------------------------------------
/**
 * Retrieves the solar system is use object pointer.
 *
 * @return a default solar system object pointer
 */
//------------------------------------------------------------------------------
SolarSystem* GuiInterpreter::GetSolarSystemInUse()
{
   return theModerator->GetSolarSystemInUse();
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
Parameter* GuiInterpreter::GetParameter(const wxString &name)
{
   return theModerator->GetParameter(name);
}


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
bool GuiInterpreter::IsParameter(const wxString &str)
{
   return theModerator->IsParameter(str);
}


//------------------------------------------------------------------------------
// Parameter* CreateParameter(const wxString &type, const wxString &name,
//                            const wxString &ownerName, const wxString &depName
//                            bool manage = true)
//------------------------------------------------------------------------------
/**
 * Calls the Moderator to create a Parameter. We need this to create an array
 * first and set size later when we create an array from the GUI.
 * 
 * @param  type       Type of parameter requested
 * @param  name       Name for the parameter.
 * @param  ownerName  object name of parameter requested ("")
 * @param  depName    Dependent object name of parameter requested ("")
 * @param  manage     true if created object to be added to configuration (true)
 * 
 * @return Pointer to the constructed Parameter.
 *
 * @Note We need this to create an array first and set size later when an array
 *       is created the GUI.
 */
//------------------------------------------------------------------------------
Parameter* GuiInterpreter::CreateParameter(const wxString &type, 
                                           const wxString &name,
                                           const wxString &ownerName,
                                           const wxString &depName,
                                           bool manage)
{
   return theModerator->CreateParameter(type, name, ownerName, depName, manage);
}


//------------------------------------------------------------------------------
// Subscriber* CreateSubscriber(const wxString &type,
//                              const const wxString &name,
//                              const wxString &filename = "",
//                              bool createDefault = true)
//------------------------------------------------------------------------------
/**
 * Creates a subscriber object by given type and name.
 *
 * @param <type> object type
 * @param <name> object name
 * @param <filename> file name if used
 *
 * @return a subscriber object pointer
 */
//------------------------------------------------------------------------------
Subscriber* GuiInterpreter::CreateSubscriber(const wxString &type,
                                             const wxString &name,
                                             const wxString &filename,
                                             bool createDefault)
{
   // Set object manage option to configuration object
   theModerator->SetObjectManageOption(1);
   return theModerator->
      CreateSubscriber(type, name, filename, createDefault);
}


//------------------------------------------------------------------------------
// Integer GetNumberOfActivePlots()
//------------------------------------------------------------------------------
Integer GuiInterpreter::GetNumberOfActivePlots()
{
   return theModerator->GetNumberOfActivePlots();
}


//------------------------------------------------------------------------------
// GmatBase* CreateDefaultPropSetup(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Creates a default PropSetup object.
 *
 * @param <name> PropSetup name
 *
 * @return a PropSetup object pointer
 */
//------------------------------------------------------------------------------
GmatBase* GuiInterpreter::CreateDefaultPropSetup(const wxString &name)
{
   return (GmatBase*)theModerator->CreateDefaultPropSetup(name);
}

GmatBase* GuiInterpreter::CreateNewODEModel(const wxString &name)
{
   return theModerator->CreateODEModel(wxT("ODEModel"), name);
}

//------------------------------------------------------------------------------
// CoordinateSystem* GetInternalCoordinateSystem()
//------------------------------------------------------------------------------
/**
 * @return a internal coordinate system pointer
 */
//------------------------------------------------------------------------------
CoordinateSystem* GuiInterpreter::GetInternalCoordinateSystem()
{
   return theModerator->GetInternalCoordinateSystem();
}


//------------------------------------------------------------------------------
// bool IsDefaultCoordinateSystem(const wxString &name)
//------------------------------------------------------------------------------
bool GuiInterpreter::IsDefaultCoordinateSystem(const wxString &name)
{
   if (name == wxT("EarthMJ2000Eq") || name == wxT("EarthMJ2000Ec") ||
       name == wxT("EarthFixed"))
      return true;
   else
      return false;
}


//------------------------------------------------------------------------------
// const StringArray& GetPlanetarySourceTypes()
//------------------------------------------------------------------------------
/**
 * @return a planetary source types for the solar system in use.
 */
//------------------------------------------------------------------------------
const StringArray& GuiInterpreter::GetPlanetarySourceTypes()
{
   return theModerator->GetPlanetarySourceTypes();
}


//------------------------------------------------------------------------------
// const StringArray& GetPlanetarySourceNames()
//------------------------------------------------------------------------------
/**
 * @return a planetary source file names of the solar system in use.
 */
//------------------------------------------------------------------------------
const StringArray& GuiInterpreter::GetPlanetarySourceNames()
{
   return theModerator->GetPlanetarySourceNames();
}


//------------------------------------------------------------------------------
// StringArray& GetPlanetarySourceTypesInUse()
//------------------------------------------------------------------------------
/*
 * @return planetary source type in use of the solar system in use.
 */
//------------------------------------------------------------------------------
const StringArray& GuiInterpreter::GetPlanetarySourceTypesInUse()
{
   return theModerator->GetPlanetarySourceTypesInUse();
}


////------------------------------------------------------------------------------
//// const StringArray& GetAnalyticModelNames()
////------------------------------------------------------------------------------
///*
// * @return analytic model name used of the solar system in use.
// */
////------------------------------------------------------------------------------
//const StringArray& GuiInterpreter::GetAnalyticModelNames()
//{
//   return theModerator->GetAnalyticModelNames();
//}
//
//
////------------------------------------------------------------------------------
//// bool SetAnalyticModelToUse(const wxString &modelName)
////------------------------------------------------------------------------------
//bool GuiInterpreter::SetAnalyticModelToUse(const wxString &modelName)
//{
//   return theModerator->SetAnalyticModelToUse(modelName);
//}
//

//------------------------------------------------------------------------------
// bool SetPlanetarySourceName(const wxString &sourceType,
//                           const wxString &filename)
//------------------------------------------------------------------------------
bool GuiInterpreter::SetPlanetarySourceName(const wxString &sourceType,
                                          const wxString &filename)
{
   return theModerator->SetPlanetarySourceName(sourceType, filename);
}


//------------------------------------------------------------------------------
// Integer SetPlanetarySourceTypesInUse(const StringArray &sourceTypes)
//------------------------------------------------------------------------------
/*
 * @param <sourceTypes> list of file type in the priority order of use
 */
//------------------------------------------------------------------------------
Integer GuiInterpreter::SetPlanetarySourceTypesInUse(const StringArray &sourceTypes)
{
   return theModerator->SetPlanetarySourceTypesInUse(sourceTypes);
}


//------------------------------------------------------------------------------
// wxString GetPlanetarySourceName(const wxString &sourceType)
//------------------------------------------------------------------------------
wxString GuiInterpreter::GetPlanetarySourceName(const wxString &sourceType)
{
   return theModerator->GetPlanetarySourceName(sourceType);
}


//------------------------------------------------------------------------------
// wxString GetPotentialFileName(const wxString &fileType)
//------------------------------------------------------------------------------
wxString GuiInterpreter::GetPotentialFileName(const wxString &fileType)
{
   return theModerator->GetPotentialFileName(fileType);
}


//------------------------------------------------------------------------------
// wxString GetFileName(const wxString &fileType)
//------------------------------------------------------------------------------
wxString GuiInterpreter::GetFileName(const wxString &fileType)
{
   return theModerator->GetFileName(fileType);
}


//------------------------------------------------------------------------------
// GmatBase* CreateStopCondition(const wxString &type, const wxString &name)
//------------------------------------------------------------------------------
GmatBase* GuiInterpreter::CreateStopCondition(const wxString &type,
                                              const wxString &name)
{
   return (GmatBase*)theModerator->CreateStopCondition(type, name);
}


//------------------------------------------------------------------------------
// GmatCommand* CreateDefaultCommand(const wxString &type,
//                                   const wxString &name = "",
//                                   const GmatCommand *refCmd = NULL)
//------------------------------------------------------------------------------
/**
 * Creates a default command object by given type and name.
 *
 * @param <type> command type
 * @param <name> command name
 *
 * @return a command object pointer
 */
//------------------------------------------------------------------------------
GmatCommand* GuiInterpreter::CreateDefaultCommand(const wxString &type,
                                                  const wxString &name,
                                                  GmatCommand *refCmd)
{
   return theModerator->CreateDefaultCommand(type, name, refCmd);
}


//------------------------------------------------------------------------------
// GmatCommand* AppendCommand(const wxString &type, const wxString &name,
//                        Integer sandboxNum)
//------------------------------------------------------------------------------
GmatCommand* GuiInterpreter::AppendCommand(const wxString &type,
                                           const wxString &name, bool &retFlag,
                                           Integer sandboxNum)
{
   return theModerator->AppendCommand(type, name, retFlag, sandboxNum);
}


//------------------------------------------------------------------------------
// GmatCommand* DeleteCommand(GmatCommand *cmd, Integer sandboxNum)
//------------------------------------------------------------------------------
GmatCommand* GuiInterpreter::DeleteCommand(GmatCommand *cmd, Integer snadboxNum)
{
   return theModerator->DeleteCommand(cmd, snadboxNum);
}


//------------------------------------------------------------------------------
// GmatCommand* GetFirstCommand(Integer sandboxNum)
//------------------------------------------------------------------------------
/**
 * Retrieves a first command in the sequence.
 *
 * @param <sandboxNum> sandbox number
 *
 * @return a next command object pointer, return null if no command found
 */
//------------------------------------------------------------------------------
GmatCommand* GuiInterpreter::GetFirstCommand(Integer sandboxNum)
{
   return theModerator->GetFirstCommand(sandboxNum);
}


//------------------------------------------------------------------------------
// bool AppendCommand(GmatCommand *cmd, Integer sandboxNum)
//------------------------------------------------------------------------------
/**
 * Appends a command.
 *
 * @param <cmd> command to append
 *
 * @return a command object pointer, return null if not appended
 */
//------------------------------------------------------------------------------
bool GuiInterpreter::AppendCommand(GmatCommand *cmd, Integer sandboxNum)
{
   return theModerator->AppendCommand(cmd, sandboxNum);
}


//------------------------------------------------------------------------------
// bool InsertCommand(GmatCommand *cmd, GmatCommand *prevCmd, Integer sandboxNum)
//------------------------------------------------------------------------------
bool GuiInterpreter::InsertCommand(GmatCommand *cmd, GmatCommand *prevCmd,
                                   Integer sandboxNum)
{
   return theModerator->InsertCommand(cmd, prevCmd, sandboxNum);
}


//------------------------------------------------------------------------------
// bool ClearResource()
//------------------------------------------------------------------------------
bool GuiInterpreter::ClearResource()
{
   return theModerator->ClearResource();
}


//------------------------------------------------------------------------------
// bool LoadDefaultMission()
//------------------------------------------------------------------------------
bool GuiInterpreter::LoadDefaultMission()
{
   return theModerator->LoadDefaultMission();
}


//------------------------------------------------------------------------------
// bool ClearCommandSeq(Integer sandboxNum = 1)
//------------------------------------------------------------------------------
bool GuiInterpreter::ClearCommandSeq(Integer sandboxNum)
{
   return theModerator->ClearCommandSeq(true, true, sandboxNum);
}


//------------------------------------------------------------------------------
// void ClearAllSandboxes()
//------------------------------------------------------------------------------
void GuiInterpreter::ClearAllSandboxes()
{
   theModerator->ClearAllSandboxes();
}


//------------------------------------------------------------------------------
// Integer RunMission(Integer sandboxNum)
//------------------------------------------------------------------------------
/**
 * Calls Moderator to run mission sequence.
 *
 * @param <snadobxNum> sandbox number
 *
 * @return a status code
 *    0 = successful, <0 = error (tbd)
 */
//------------------------------------------------------------------------------
Integer GuiInterpreter::RunMission(Integer sandboxNum)
{
   return theModerator->RunMission(sandboxNum);
}


//------------------------------------------------------------------------------
// Integer ChangeRunState(const wxString &state, Integer sandboxNum)
//------------------------------------------------------------------------------
/**
 * Calls Moderator to change run state.
 *
 * @param <state> run state string ("Stop", "Pause", "Resume")
 * @param <snadobxNum> sandbox number
 *
 * @return a status code
 *    0 = successful, <0 = error (tbd)
 */
//------------------------------------------------------------------------------
Integer GuiInterpreter::ChangeRunState(const wxString &state, Integer sandboxNum)
{
   return theModerator->ChangeRunState(state, sandboxNum);
}


//------------------------------------------------------------------------------
// bool InterpretScript(const wxString &filename, bool readBack = false,
//                      const wxString &newPath = "")
//------------------------------------------------------------------------------
/**
 * Creates objects from script file.
 *
 * @param <filename> input script file name
 * @param <readBack> true will read scripts, save, and read back in
 * @param <newPath> new path to be used for saving scripts
 *
 * @return true if successful; false otherwise
 */
//------------------------------------------------------------------------------
bool GuiInterpreter::InterpretScript(const wxString &filename, bool readBack,
                                     const wxString &newPath)
{
   return theModerator->InterpretScript(filename, readBack, newPath);
}


//------------------------------------------------------------------------------
// bool SaveScript(const wxString &filename, Gmat::WriteMode mode)
//------------------------------------------------------------------------------
/**
 * Builds scripts from objects and write to a file.
 *
 * @param <filename> output script file name
 *
 * @return true if successful; false otherwise
 */
//------------------------------------------------------------------------------
bool GuiInterpreter::SaveScript(const wxString &filename,
                                Gmat::WriteMode mode)
{
   return theModerator->SaveScript(filename, mode);
}


//------------------------------------------------------------------------------
// wxString GetScript(Gmat::WriteMode mode)
//------------------------------------------------------------------------------
/**
 * Returns Built scripts from objects
 *
 * @return built scripts from objects
 */
//------------------------------------------------------------------------------
wxString GuiInterpreter::GetScript(Gmat::WriteMode mode)
{
   return theModerator->GetScript(mode);
}


//------------------------------------------------------------------------------
// Integer RunScript(Integer sandboxNum)
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
Integer GuiInterpreter::RunScript(Integer sandboxNum)
{
   return theModerator->RunScript(sandboxNum);
}


//------------------------------------------------------------------------------
// void SetInputFocus()
//------------------------------------------------------------------------------
void GuiInterpreter::SetInputFocus()
{
#if !defined __CONSOLE_APP__
   GmatMainFrame *mainFrame = GmatAppData::Instance()->GetMainFrame();
   mainFrame->SetFocus();
   if (mainFrame->IsIconized())
      mainFrame->ProcessPendingEvent();
#endif
}


//------------------------------------------------------------------------------
// void NotifyRunCompleted()
//------------------------------------------------------------------------------
void GuiInterpreter::NotifyRunCompleted()
{
#if !defined __CONSOLE_APP__
   GmatAppData::Instance()->GetMainFrame()->NotifyRunCompleted();
#endif
}

void GuiInterpreter::UpdateView(Integer type)
{
   if (type & 0x01)
      UpdateResourceTree();
   if (type & 0x02)
      UpdateMissionTree();
   if (type & 0x04)
      UpdateOutputTree();
}


//------------------------------------------------------------------------------
// void UpdateResourceTree()
//------------------------------------------------------------------------------
void GuiInterpreter::UpdateResourceTree()
{
#if !defined __CONSOLE_APP__
   //close the open windows first
   GmatAppData *gmatAppData = GmatAppData::Instance();
   gmatAppData->GetMainFrame()->CloseAllChildren();
   gmatAppData->GetResourceTree()->UpdateResource(true);
#endif
}


//------------------------------------------------------------------------------
// void UpdateMissionTree()
//------------------------------------------------------------------------------
void GuiInterpreter::UpdateMissionTree()
{
#if !defined __CONSOLE_APP__
   GmatAppData::Instance()->GetMissionTree()->UpdateMission(true);
#endif
}


//------------------------------------------------------------------------------
// void UpdateOutputTree()
//------------------------------------------------------------------------------
void GuiInterpreter::UpdateOutputTree()
{
#if !defined __CONSOLE_APP__
   GmatAppData::Instance()->GetOutputTree()->UpdateOutput(false, true);
#endif
}


//------------------------------------------------------------------------------
// void CloseCurrentProject()
//------------------------------------------------------------------------------
void GuiInterpreter::CloseCurrentProject()
{
#if !defined __CONSOLE_APP__
   GmatAppData::Instance()->GetMainFrame()->CloseCurrentProject();
#endif
}


//------------------------------------------------------------------------------
// void StartMatlabServer()
//------------------------------------------------------------------------------
/*
 * Starts the MATLAB server.
 */
//------------------------------------------------------------------------------
void GuiInterpreter::StartMatlabServer()
{
   GmatAppData::Instance()->GetMainFrame()->StartMatlabServer();   
}


//---------------------------------
// private
//---------------------------------

//------------------------------------------------------------------------------
// GuiInterpreter()
//------------------------------------------------------------------------------
GuiInterpreter::GuiInterpreter()
   : ScriptInterpreter()
{
   Initialize();
   isInitialized = false;
}


//------------------------------------------------------------------------------
// GuiInterpreter(const GuiInterpreter&)
//------------------------------------------------------------------------------
GuiInterpreter::GuiInterpreter(const GuiInterpreter&)
{
}


//------------------------------------------------------------------------------
// GuiInterpreter& operator=(const GuiInterpreter &guiInterpreter)
//------------------------------------------------------------------------------
GuiInterpreter& GuiInterpreter::operator=(const GuiInterpreter &guiInterpreter)
{
   return *this;
}

std::vector<Gmat::PluginResource*> *GuiInterpreter::GetUserResources()
{
   return theModerator->GetPluginResourceList();
}
