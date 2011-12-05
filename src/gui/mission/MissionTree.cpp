//$Id: MissionTree.cpp 10034 2011-12-02 21:44:39Z lindajun $
//------------------------------------------------------------------------------
//                              MissionTree
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Allison Greene
// Created: 2003/09/02
/**
 * This class provides the tree for missions.
 */
//------------------------------------------------------------------------------
#include "gmatwxdefs.hpp"
#include "bitmaps/folder.xpm"
#include "bitmaps/OpenFolder.xpm"
#include "bitmaps/file.xpm"
#include "bitmaps/propagateevent.xpm"
#include "bitmaps/target.xpm"
#include "bitmaps/whileloop.xpm"
#include "bitmaps/forloop.xpm"
#include "bitmaps/if.xpm"
#include "bitmaps/scriptevent.xpm"
#include "bitmaps/varyevent.xpm"
#include "bitmaps/achieveevent.xpm"
#include "bitmaps/deltav.xpm"
#include "bitmaps/callfunction.xpm"
#include "bitmaps/nestreturn.xpm"
#include "bitmaps/saveobject.xpm"
#include "bitmaps/equalsign.xpm"
#include "bitmaps/toggle.xpm"
#include "bitmaps/beginfb.xpm"
#include "bitmaps/endfb.xpm"
#include "bitmaps/report.xpm"
#include "bitmaps/penup.xpm"
#include "bitmaps/pendown.xpm"
#include "bitmaps/optimize.xpm"
#include "bitmaps/mt_Default.xpm"
#include "bitmaps/mt_Stop.xpm"
#include "bitmaps/mt_MarkPoint.xpm"
#include "bitmaps/mt_ClearPlot.xpm"
#include "bitmaps/mt_Global.xpm"
#include "bitmaps/mt_SaveMission.xpm"
#include "bitmaps/mt_Minimize.xpm"
#include "bitmaps/mt_NonlinearConstraint.xpm"
#include "bitmaps/mt_RunSimulator.xpm"
#include "bitmaps/mt_RunEstimator.xpm"

#include <wx/tipwin.h>

#include "MissionTree.hpp"
#include "MissionTreeItemData.hpp"
#include "ViewTextFrame.hpp"
#include "ShowSummaryDialog.hpp"

#include "GmatAppData.hpp"
#include "GmatMainFrame.hpp"
#include "GmatNotebook.hpp"
#include "MessageInterface.hpp"
#include "CommandUtil.hpp"         // for GetNextCommand()
#include "StringUtil.hpp"          // for GmatStringUtil::

// This will automatically add all viewable commands including plugin commands.
// It uses new array called viewables in the CommandFactory.
// If plugin commands need to be shown in the popup menu, just add them to
// viewables in a plugin factory constructors(LOJ: 2010.07.13)
#define __AUTO_ADD_NEW_COMMANDS__

// Should we sort the command list?
#define __SORT_COMMAND_LIST__

// Should we enable multiple mission sequence?
//#define__ENABLE_MULTIPLE_SEQUENCE__

// If actions to be saved and playback for testing
// Define this by setting the compiler parameter -D
//#define __TEST_MISSION_TREE_ACTIONS__

#ifdef __TEST_MISSION_TREE_ACTIONS__
#include <fstream>
#include "StringTokenizer.hpp"
//#define DEBUG_MISSION_TREE_ACTIONS
#endif

// For debug output
//#define DEBUG_MISSION_TREE_SHOW_CMD 1
//#define DEBUG_APPEND_COMMAND 1
//#define DEBUG_MISSION_TREE_APPEND 1
//#define DEBUG_MISSION_TREE_INSERT 1
//#define DEBUG_MISSION_TREE_DELETE 1
//#define DEBUG_MISSION_TREE_CHILD 1
//#define DEBUG_MISSION_TREE_FIND 2
//#define DEBUG_FIND_ITEM_PARENT 2
//#define DEBUG_MISSION_TREE_MENU 1
//#define DEBUG_MISSION_TREE 1
//#define DEBUG_ADD_ICONS
//#define DEBUG_BUILD_TREE_ITEM 1
//#define DEBUG_VIEW_COMMANDS 1

//------------------------------
// event tables for wxWindows
//------------------------------

//------------------------------------------------------------------------------
// EVENT_TABLE(MissionTree, wxTreeCtrl)
//------------------------------------------------------------------------------
/**
 * Events Table for the menu and tool bar
 *
 * @note Indexes event handler functions.
 */
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(MissionTree, wxTreeCtrl)
   EVT_PAINT(DecoratedTree::OnPaint)
   //EVT_UPDATE_UI(-1, DecoratedTree::OnPaint)
   //ag: 10/20/2004 Commented out so that the position of the click is not
   // checked to open up a panel from the variables/goals boxes
   //loj: 11/4/04 Uncommented so that double click on Target/If/For/While folder
   // will not collapse
   
   // wxMouseEvent event
   EVT_LEFT_DCLICK(MissionTree::OnDoubleClick)
   
   // wxTreeEvent
   EVT_TREE_ITEM_RIGHT_CLICK(-1, MissionTree::OnItemRightClick)
   EVT_TREE_ITEM_ACTIVATED(-1, MissionTree::OnItemActivated)
   EVT_TREE_BEGIN_LABEL_EDIT(-1, MissionTree::OnBeginEditLabel)
   EVT_TREE_END_LABEL_EDIT(-1, MissionTree::OnEndEditLabel)
   
   // wxCommandEvent
   EVT_MENU(POPUP_OPEN, MissionTree::OnOpen)
   EVT_MENU(POPUP_CLOSE, MissionTree::OnClose)
   
   EVT_MENU(POPUP_ADD_MISSION_SEQ, MissionTree::OnAddMissionSeq)
   EVT_MENU(POPUP_APPEND, MissionTree::OnPopupAppend)
   
   EVT_MENU_RANGE(POPUP_APPEND_PROPAGATE, POPUP_APPEND_SWITCH,
                  MissionTree::OnAppend)
   EVT_MENU_RANGE(POPUP_INSERT_BEFORE_PROPAGATE, POPUP_INSERT_BEFORE_SWITCH,
                  MissionTree::OnInsertBefore)
   EVT_MENU_RANGE(POPUP_INSERT_AFTER_PROPAGATE, POPUP_INSERT_AFTER_SWITCH,
                  MissionTree::OnInsertAfter)
   
   EVT_MENU_RANGE(AUTO_APPEND_COMMAND, AUTO_INSERT_BEFORE_COMMAND - 1,
                  MissionTree::OnAutoAppend)
   EVT_MENU_RANGE(AUTO_INSERT_BEFORE_COMMAND, AUTO_INSERT_AFTER_COMMAND - 1,
                  MissionTree::OnAutoInsertBefore)
   EVT_MENU_RANGE(AUTO_INSERT_AFTER_COMMAND, AUTO_END - 1,
                  MissionTree::OnAutoInsertAfter)
   
   EVT_MENU(POPUP_COLLAPSE, MissionTree::OnCollapse)
   EVT_MENU(POPUP_EXPAND, MissionTree::OnExpand)
   
   EVT_MENU(POPUP_RUN, MissionTree::OnRun)
   
   EVT_MENU(POPUP_RENAME, MissionTree::OnRename)
   EVT_MENU(POPUP_DELETE, MissionTree::OnDelete)
   
   EVT_MENU(POPUP_SHOW_DETAIL, MissionTree::OnShowDetail)
   EVT_MENU(POPUP_SHOW_SCRIPT, MissionTree::OnShowScript)
   EVT_MENU(POPUP_SHOW_MISSION_SEQUENCE, MissionTree::OnShowMissionSequence)
   EVT_MENU(POPUP_COMMAND_SUMMARY, MissionTree::OnShowCommandSummary)
   EVT_MENU(POPUP_MISSION_SUMMARY_ALL, MissionTree::OnShowMissionSummaryAll)
   EVT_MENU(POPUP_MISSION_SUMMARY_PHYSICS, MissionTree::OnShowMissionSummaryPhysics)
   
   EVT_MENU_RANGE(POPUP_DOCK_MISSION_TREE, POPUP_UNDOCK_MISSION_TREE,
                  MissionTree::OnDockUndockMissionTree)
   
   #ifdef __TEST_MISSION_TREE_ACTIONS__
   EVT_MENU(POPUP_START_SAVE_ACTIONS, MissionTree::OnStartSaveActions)
   EVT_MENU(POPUP_STOP_SAVE_ACTIONS, MissionTree::OnStopSaveActions)
   EVT_MENU(POPUP_READ_ACTIONS, MissionTree::OnPlaybackActions)
   #endif
   
END_EVENT_TABLE()

//------------------------------
// public methods
//------------------------------

//------------------------------------------------------------------------------
// MissionTree(wxWindow *parent, const wxWindowID id,
//              const wxPoint &pos, const wxSize &size, long style)
//------------------------------------------------------------------------------
/**
 * Constructs MissionTree object.
 *
 * @param <parent> input parent.
 * @param <id> input id.
 * @param <pos> input position.
 * @param <size> input size.
 * @param <style> input style.
 *
 * @note Creates the tree for missions and adds a default mission.
 */
//------------------------------------------------------------------------------
MissionTree::MissionTree(wxWindow *parent, const wxWindowID id,
                         const wxPoint &pos, const wxSize &size, long style)
   : DecoratedTree(parent, id, pos, size, style),
     inScriptEvent(false),
     inFiniteBurn(false),
     mShowDetailedItem(false)
{
   mParent = parent;
   theNotebook = NULL;
   theGuiInterpreter = GmatAppData::Instance()->GetGuiInterpreter();
   theGuiManager = GuiItemManager::GetInstance();
   mViewCommands.Add(wxT("All"));
   mViewAll = true;
   mUsingViewLevel = true;
   mWriteMissionSeq = false;
   mViewLevel = 10;
   
   // Set mWriteMissionSeq to true if debugging or
   // DEBUG_MISSION_TREE = ON in the startup file.
   #ifdef DEBUG_MISSION_TREE_SHOW_CMD
   mWriteMissionSeq = true;
   #endif
   
   if (GmatGlobal::Instance()->IsMissionTreeDebugOn())
      mWriteMissionSeq = true;
   
   //-----------------------------------------------------------------
   #ifdef __AUTO_ADD_NEW_COMMANDS__
   //-----------------------------------------------------------------
   
   StringArray cmds = theGuiInterpreter->GetListOfViewableCommands();
   #ifdef DEBUG_COMMAND_LIST
   GmatStringUtil::WriteStringArray
      (cmds, wxT("===> Here is the viewable command list"), wxT("   "));
   #endif
   for (unsigned int i = 0; i < cmds.size(); i++)
      mCommandList.Add(cmds[i].c_str());
   
   CreateCommandIdMap();
   
   //-----------------------------------------------------------------
   #else
   //-----------------------------------------------------------------
   
   mCommandList.Clear();
   mCommandList.Add(wxT("Propagate"));
   mCommandList.Add(wxT("Maneuver"));
   mCommandList.Add(wxT("BeginFiniteBurn"));
   mCommandList.Add(wxT("EndFiniteBurn"));
   mCommandList.Add(wxT("Target"));
   mCommandList.Add(wxT("Optimize"));
   mCommandList.Add(wxT("CallGmatFunction"));
   if (GmatGlobal::Instance()->IsMatlabAvailable())
      mCommandList.Add(wxT("CallMatlabFunction"));
   mCommandList.Add(wxT("Report"));
   mCommandList.Add(wxT("Toggle"));
   mCommandList.Add(wxT("Save"));
   mCommandList.Add(wxT("Stop"));
   //mCommandList.Add("GMAT");
   mCommandList.Add(wxT("Equation"));
   mCommandList.Add(wxT("ScriptEvent"));
   
   //-----------------------------------------------------------------
   #endif
   //-----------------------------------------------------------------
   
   // Build commands for view control since MissionTree show ControlFlow commands
   // and Vary, Achieve in sub nodes such as ControlLogic and Target node.
   mCommandListForViewControl = mCommandList;
   mCommandListForViewControl.Add(wxT("For"));
   mCommandListForViewControl.Add(wxT("If"));
   mCommandListForViewControl.Add(wxT("Else"));
   mCommandListForViewControl.Add(wxT("While"));
   mCommandListForViewControl.Add(wxT("Achieve"));
   mCommandListForViewControl.Add(wxT("Vary"));
   mCommandListForViewControl.Add(wxT("Minimize"));
   mCommandListForViewControl.Add(wxT("NonlinearConstraint"));
   mCommandListForViewControl.Add(wxT("CallMatlabFunction"));
   
   //mCommandListForViewControl.Add(wxT("EndTarget"));
   //mCommandListForViewControl.Add(wxT("EndOptimize"));
   //mCommandListForViewControl.Add(wxT("EndFor"));
   //mCommandListForViewControl.Add(wxT("EndIf"));
   //mCommandListForViewControl.Add(wxT("EndWhile"));
   
   // Should we sort the command list?
   #ifdef __SORT_COMMAND_LIST__
   mCommandList.Sort();
   #endif
   
   SetParameter(BOXCOUNT, 0);
   SetParameter(DRAWOUTLINE, 0);
   
   InitializeCounter();
   AddIcons();
   
   // Now this is called from GmatNotebook after MissionTreeToolBar is created
   //AddDefaultMission();
   
   // for auto-testing of MissionTree actions
   #ifdef __TEST_MISSION_TREE_ACTIONS__
   mSaveActions = false;
   mPlaybackActions = false;
   mActionsOutFile = wxT("MissionTreeActionsOut.txt");
   mResultsFile = wxT("MissionTreeResults.txt");
   #endif
}


//------------------------------------------------------------------------------
// void SetMainFrame(GmatMainFrame *gmf)
//------------------------------------------------------------------------------
void MissionTree::SetMainFrame(GmatMainFrame *gmf)
{
   theMainFrame = gmf;
}


//------------------------------------------------------------------------------
// void SetNotebook(GmatNotebook *notebook)
//------------------------------------------------------------------------------
void MissionTree::SetNotebook(GmatNotebook *notebook)
{
   theNotebook = notebook;
}


//------------------------------------------------------------------------------
// void ClearMission()
//------------------------------------------------------------------------------
/**
 * Clears Mission Sequence
 */
//------------------------------------------------------------------------------
void MissionTree::ClearMission()
{
   #if DEBUG_MISSION_TREE
   MessageInterface::ShowMessage(wxT("MissionTree::ClearMission() entered\n"));
   #endif
   
   // Collapse, so folder icon is closed
   #ifdef __WXMSW__
      Collapse(mMissionSeqSubId);
   #endif

   wxString itemText = GetItemText(mMissionSeqSubId);
   if (itemText.Find(wxT("...")) != wxNOT_FOUND)
   {
      itemText.Replace(wxT("..."), wxT(""));
      SetItemText(mMissionSeqSubId, itemText);
   }
   
   DeleteChildren(mMissionSeqSubId);
   
   #ifdef __TEST_MISSION_TREE_ACTIONS__
   if (mActionsOutStream.is_open())
      mActionsOutStream.close();
   if (mResultsStream.is_open())
      mResultsStream.close();
   if (mPlaybackResultsStream.is_open())
      mPlaybackResultsStream.close();
   #endif
}


//------------------------------------------------------------------------------
// void UpdateMission(bool resetCounter, bool viewAll = true, bool collapse = false)
//------------------------------------------------------------------------------
/**
 * Updates Mission Sequence
 */
//------------------------------------------------------------------------------
void MissionTree::UpdateMission(bool resetCounter, bool viewAll, bool collapse)
{
   #if DEBUG_MISSION_TREE
   MessageInterface::ShowMessage
      (wxT("MissionTree::UpdateMission() entered, resetCounter=%d, viewAll=%d, ")
       wxT("collapse=%d, mUsingViewLevel=%d\n"), resetCounter, viewAll, collapse,
       mUsingViewLevel);
   #endif
   
   if (resetCounter)
      InitializeCounter();
   
   //if (mUsingViewLevel)
   //   mViewAll = viewAll;
   
   ClearMission();
   UpdateCommand();
   
   if (collapse)
   {
      CollapseAllChildren(mMissionSeqSubId);
      Expand(mMissionSeqSubId);
   }
}


//------------------------------------------------------------------------------
// void UpdateMissionForRename()
//------------------------------------------------------------------------------
/**
 * Updates MissionTree nodes if show detail is turned on since it needs to show
 * new resource name on the nodes.
 */
//------------------------------------------------------------------------------
void MissionTree::UpdateMissionForRename()
{
   #if DEBUG_MISSION_TREE
   MessageInterface::ShowMessage(wxT("MissionTree::UpdateMissionForRename() entered\n"));
   #endif
   
   if (mShowDetailedItem)
   {
      ClearMission();
      UpdateCommand();
   }
}

//#define DEBUG_CHANGE_NODE_LABEL
//------------------------------------------------------------------------------
// void ChangeNodeLabel(const wxString &oldLabel)
//------------------------------------------------------------------------------
/*
 * Sets tree node label to new label if it is different from oldLabel.
 */
//------------------------------------------------------------------------------
void MissionTree::ChangeNodeLabel(const wxString &oldLabel)
{
   #ifdef DEBUG_CHANGE_NODE_LABEL
   MessageInterface::ShowMessage
      (wxT("MissionTree::ChangeNodeLabel() oldLabel=<%s>\n"), oldLabel.c_str());
   #endif
   
   wxTreeItemId itemId = FindChild(mMissionSeqSubId, oldLabel);
   if (itemId.IsOk())
   {
      MissionTreeItemData *item = (MissionTreeItemData *)GetItemData(itemId);
      GmatCommand *cmd = item->GetCommand();
      wxString newLabel = GetCommandString(cmd, oldLabel);
      
      #ifdef DEBUG_CHANGE_NODE_LABEL
      MessageInterface::ShowMessage(wxT("   newLabel=<%s>\n"), newLabel.c_str());
      #endif
      
      if (newLabel != oldLabel)
      {
         item->SetName(newLabel);
         item->SetTitle(newLabel);
         SetItemText(itemId, newLabel);
      }
   }
   else
   {
      #ifdef DEBUG_CHANGE_NODE_LABEL
      MessageInterface::ShowMessage(wxT("===> <%s> not found\n"), oldLabel.c_str());
      #endif
   }
}


//------------------------------------------------------------------------------
// void SetViewAll(bool viewAll)
//------------------------------------------------------------------------------
void MissionTree::SetViewAll(bool viewAll)
{
   mViewAll = viewAll;
}


//------------------------------------------------------------------------------
// void SetViewLevel(int level)
//------------------------------------------------------------------------------
void MissionTree::SetViewLevel(int level)
{
   #ifdef DEBUG_VIEW_LEVEL
   MessageInterface::ShowMessage
      ("MissionTree::SetViewLevel() entered, level = %d\n", level);
   #endif
   
   mUsingViewLevel = true;
   mViewLevel = level;
   if (level == 0) // Set level to 10 for showing all levels
      mViewLevel = 10;
   if (mViewLevel == 10)
      mViewAll = true;
   
   UpdateMission(true, false, false);
   
   #ifdef DEBUG_VIEW_LEVEL
   MessageInterface::ShowMessage
      ("MissionTree::SetViewLevel() leaving, mViewAll = %d\n", mViewAll);
   #endif
}


//------------------------------------------------------------------------------
// void SetViewCommands(const wxArrayString &viewCmds)
//------------------------------------------------------------------------------
void MissionTree::SetViewCommands(const wxArrayString &viewCmds)
{
   mViewCommands = viewCmds;
   
   #ifdef DEBUG_VIEW_COMMANDS
   MessageInterface::ShowMessage(wxT("\n=====> MissionTree::SetViewOption() entered\n"));
   MessageInterface::ShowMessage
      (wxT("mCommandListForViewControl has %d commands\n"), mCommandListForViewControl.GetCount());
   for (unsigned int i = 0; i < mCommandListForViewControl.size(); i++)
      MessageInterface::ShowMessage(wxT("   '%s'\n"), mCommandListForViewControl[i].c_str());
   MessageInterface::ShowMessage
      (wxT("mViewCommands has %d commands\n"), mViewCommands.GetCount());
   for (unsigned int i = 0; i < mViewCommands.size(); i++)
      MessageInterface::ShowMessage(wxT("   '%s'\n"), mViewCommands[i].c_str());
   #endif
   
   mUsingViewLevel = false;
   mViewAll = false;
   if (mViewCommands.GetCount() == 1 && mViewCommands[0] == wxT("All"))
      mViewAll = true;
   
   UpdateMission(true, false, false);
}


//------------------------------------------------------------------------------
// const wxArrayString& GetCommandList(bool forViewControl = false)
//------------------------------------------------------------------------------
const wxArrayString& MissionTree::GetCommandList(bool forViewControl)
{
   if (forViewControl)
      return mCommandListForViewControl;
   else
      return mCommandList;
}


//-------------------------------
// private methods
//-------------------------------

//------------------------------------------------------------------------------
// void InitializeCounter()
//------------------------------------------------------------------------------
/**
 * Initializes command counter.
 */
//------------------------------------------------------------------------------
void MissionTree::InitializeCounter()
{
   mScriptEventCount = 0;
   mTempCounter = 0;
   mNumManeuver = 0;
   mNumMissionSeq = 0;
   mNumPropagate = 0;
   mNumManeuver = 0;
   mNumTarget = 0;
   mNumOptimize = 0;
   mNumAchieve = 0;
   mNumVary = 0;
   mNumSave = 0;
   mNumReport = 0;
   mNumToggle = 0;
   mNumClearPlot = 0;
   mNumMarkPoint = 0;
   mNumPenUp = 0;
   mNumPenDown = 0;
   mNumIfStatement = 0;
   mNumWhileLoop = 0;
   mNumForLoop = 0;
   mNumDoWhile = 0;
   mNumSwitchCase = 0;
   mNumFunct = 0;
   mNumAssign = 0;
   mNumScriptEvent = 0;
   mNumFiniteBurn = 0;
   mNumStop = 0;
   mNumMinimize = 0;
   mNumNonlinearConstraint = 0;
   
   inScriptEvent = false;
   inFiniteBurn = false;
}


//------------------------------------------------------------------------------
// GmatCommand* CreateCommand(const wxString &cmdTypeName)
//------------------------------------------------------------------------------
GmatCommand* MissionTree::CreateCommand(const wxString &cmdTypeName)
{
   #ifdef DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("MissionTree::CreateCommand() entered, cmdTypeName='%s'\n"), cmdTypeName.c_str());
   #endif
   
   GmatCommand *cmd = NULL;
   try
   {
      if (cmdTypeName == wxT("IfElse"))
         cmd = theGuiInterpreter->CreateDefaultCommand(wxT("If"));
      else if (cmdTypeName == wxT("Equation"))
         cmd = theGuiInterpreter->CreateDefaultCommand(wxT("GMAT"));
      else
         cmd = theGuiInterpreter->CreateDefaultCommand(cmdTypeName.c_str());
   }
   catch (BaseException &be)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, be.GetFullMessage());
   }
   
   #ifdef DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("MissionTree::CreateCommand() returning <%p>\n"), cmd);
   #endif
   
   return cmd;
}


//------------------------------------------------------------------------------
// GmatCommand* CreateEndCommand(const wxString &cmdTypeName, GmatTree::ItemType &endType)
//------------------------------------------------------------------------------
GmatCommand* MissionTree::CreateEndCommand(const wxString &cmdTypeName,
														 GmatTree::ItemType &endType)
{
   #ifdef DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("MissionTree::CreateEndCommand() entered, cmdTypeName='%s'\n"), cmdTypeName.c_str());
   #endif
	
	GmatCommand *endCmd = NULL;
	
	if (cmdTypeName == wxT("Target"))
	{
		endCmd = CreateCommand(wxT("EndTarget"));
		endType = GmatTree::END_TARGET;
	}
	else if (cmdTypeName == wxT("For"))
	{
		endCmd = CreateCommand(wxT("EndFor"));
		endType = GmatTree::END_FOR_CONTROL;
	}
	else if (cmdTypeName == wxT("While"))
	{
		endCmd = CreateCommand(wxT("EndWhile"));
		endType = GmatTree::END_WHILE_CONTROL;
	}
	else if (cmdTypeName == wxT("If"))
	{
		endCmd = CreateCommand(wxT("EndIf"));
		endType = GmatTree::END_IF_CONTROL;
	}
	else if (cmdTypeName == wxT("ScriptEvent"))
	{
		endCmd = CreateCommand(wxT("EndScript"));
		endType = GmatTree::END_SCRIPT_EVENT;
	}
	else if (cmdTypeName == wxT("Optimize"))
	{
		endCmd = CreateCommand(wxT("EndOptimize"));
		endType = GmatTree::END_OPTIMIZE;
	}
	
   #ifdef DEBUG_CREATE_COMMAND
   MessageInterface::ShowMessage
      (wxT("MissionTree::CreateEndCommand() returning <%p>\n"), endCmd);
   #endif
   
   return endCmd;
}


//------------------------------------------------------------------------------
// bool IsAnyViewCommandInBranch(GmatCommand *branch)
//------------------------------------------------------------------------------
/**
 * Returns true if any view command found in the branch command
 */
//------------------------------------------------------------------------------
bool MissionTree::IsAnyViewCommandInBranch(GmatCommand *branch)
{
   #if DEBUG_VIEW_COMMANDS
   MessageInterface::ShowMessage
      (wxT("MissionTree::IsAnyViewCommandInBranch() branch=<%p><%s>'%s' entered\n"),
       branch, branch->GetTypeName().c_str(), branch->GetName().c_str());
   #endif
   Integer childNo = 0;
   GmatCommand* nextInBranch;
   GmatCommand* child;
   wxString typeName, cmdName;
   wxString branchTypeName = branch->GetTypeName().c_str();
   
   while((child = branch->GetChildCommand(childNo)) != NULL)
   {
      nextInBranch = child;
      while ((nextInBranch != NULL) && (nextInBranch != branch))
      {
         typeName = (nextInBranch->GetTypeName()).c_str();
         cmdName = (nextInBranch->GetName()).c_str();
         
         #if DEBUG_VIEW_COMMANDS
            MessageInterface::ShowMessage(wxT("-----"));
         MessageInterface::ShowMessage
            (wxT("   ----- <%p><%s>'%s'\n"), nextInBranch, typeName.c_str(), cmdName.c_str());
         #endif
         
         if (mViewCommands.Index(typeName) != wxNOT_FOUND)
         {
            #if DEBUG_VIEW_COMMANDS
            MessageInterface::ShowMessage
               (wxT("MissionTree::IsAnyViewCommandInBranch() returning true, found <%s>'%s' ")
                wxT("in <%s>'%s'\n"), typeName.c_str(), cmdName.c_str(), branchTypeName.c_str(),
                branch->GetName().c_str());
            MessageInterface::ShowMessage("***** Should I build tree item and return true?\n");
            #endif
            
            return true;
         }
         
         if (nextInBranch->GetChildCommand() != NULL)
         {
            if (IsAnyViewCommandInBranch(nextInBranch))
            {
               #if DEBUG_VIEW_COMMANDS
               MessageInterface::ShowMessage("***** Should the node be shown here?\n");
               #endif
            }
         }
         
         nextInBranch = nextInBranch->GetNext();
      }
      
      ++childNo;
   }
   
   #if DEBUG_VIEW_COMMANDS
   MessageInterface::ShowMessage
      (wxT("MissionTree::IsAnyViewCommandInBranch() returning false\n"));
   #endif
   return false;
}


//------------------------------------------------------------------------------
// wxTreeItemId BuildTreeItemInBranch(wxTreeItemId parent, GmatCommand *branch, ...)
//------------------------------------------------------------------------------
/**
 * Returns valid wxTreeItemId if any view command found in the branch command
 */
//------------------------------------------------------------------------------
wxTreeItemId MissionTree::BuildTreeItemInBranch(wxTreeItemId parent, GmatCommand *branch,
                                                Integer level, bool &isLastItemHidden)
{
   #if DEBUG_VIEW_COMMANDS
   MessageInterface::ShowMessage
      (wxT("MissionTree::BuildTreeItemInBranch() branch=<%p><%s> entered\n"),
       branch, branch->GetTypeName().c_str());
   #endif
   Integer childNo = 0;
   GmatCommand* nextInBranch;
   GmatCommand* child;
   wxString typeName, cmdName;
   wxString branchTypeName = branch->GetTypeName().c_str();
   wxTreeItemId node;
   
   while((child = branch->GetChildCommand(childNo)) != NULL)
   {
      nextInBranch = child;
      while ((nextInBranch != NULL) && (nextInBranch != branch))
      {
         typeName = (nextInBranch->GetTypeName()).c_str();
         cmdName = (nextInBranch->GetName()).c_str();
         
         #if DEBUG_VIEW_COMMANDS
         MessageInterface::ShowMessage
            (wxT("   ----- <%p><%s>'%s'\n"), nextInBranch, typeName.c_str(), cmdName.c_str());
         #endif
         
         if (mViewCommands.Index(typeName) != wxNOT_FOUND)
         {
            #if DEBUG_VIEW_COMMANDS
            MessageInterface::ShowMessage
               (wxT("MissionTree::BuildTreeItemInBranch() returning true, found <%s>'%s' ")
                wxT("in <%s>'%s'\n"), typeName.c_str(), cmdName.c_str(), branchTypeName.c_str(),
                branch->GetName().c_str());
            #endif
            
            return node;
         }
         
         if (nextInBranch->GetChildCommand() != NULL)
         {
            //@todo Need to finish this
            //node = BuildTreeItemInBranch(nextInBranch);
         }
         
         nextInBranch = nextInBranch->GetNext();
      }
      
      ++childNo;
   }
   
   #if DEBUG_VIEW_COMMANDS
   MessageInterface::ShowMessage
      (wxT("MissionTree::BuildTreeItemInBranch() returning\n"));
   #endif
   return node;
}


//------------------------------------------------------------------------------
// void ShowEllipsisInPreviousNode(wxTreeItemId parent, wxTreeItemId node)
//------------------------------------------------------------------------------
void MissionTree::ShowEllipsisInPreviousNode(wxTreeItemId parent, wxTreeItemId node)
{
   wxString itemText;
   wxTreeItemId prevId = GetPrevVisible(node);
   if (prevId.IsOk())
   {
      itemText = GetItemText(prevId);
      if (itemText.Find(wxT("...")) == wxNOT_FOUND)
      {
         itemText = itemText + wxT("...");
         SetItemText(prevId, itemText);
      }
   }
   else
   {
      itemText = GetItemText(parent);
      if (itemText.Find(wxT("...")) == wxNOT_FOUND)
      {
         itemText = itemText + wxT("...");
         SetItemText(parent, itemText);
      }
   }
   #if DEBUG_BUILD_TREE_ITEM
   MessageInterface::ShowMessage
      (wxT("   previous item = '%s'\n"), itemText.c_str());
   #endif
}


//------------------------------------------------------------------------------
// wxTreeItemId BuildTreeItem(wxTreeItemId parent, GmatCommand *cmd, ...)
//------------------------------------------------------------------------------
/**
 * Returns valid wxTreeItemId if command is visible.
 */
//------------------------------------------------------------------------------
wxTreeItemId MissionTree::BuildTreeItem(wxTreeItemId parent, GmatCommand *cmd,
                                        Integer level, bool &isLastItemHidden)
{
   wxString typeName = cmd->GetTypeName().c_str();
   wxString cmdName = cmd->GetName().c_str();
   wxTreeItemId node;
   
   #if DEBUG_BUILD_TREE_ITEM
   MessageInterface::ShowMessage
      (wxT("\nMissionTree::BuildTreeItem() entered, parent='%s', cmd=<%s>'%s', level=%d\n"),
       GetItemText(parent).c_str(), typeName.c_str(), cmdName.c_str(), level);
   MessageInterface::ShowMessage
      (wxT("   inScriptEvent=%d, mViewAll=%d, mUsingViewLevel=%d, mViewLevel=%d\n"),
       inScriptEvent, mViewAll, mUsingViewLevel, mViewLevel);
   #endif
   
   // if typeName not found in the view list and not showing all
   if (mViewCommands.Index(typeName) == wxNOT_FOUND && !mViewAll && !mUsingViewLevel)
   {
      if (cmd->GetTypeName() == wxT("BeginScript"))
         mScriptEventCount++;
      
      if (cmd->GetTypeName() == wxT("EndScript"))
         mScriptEventCount--;
      
      inScriptEvent = (mScriptEventCount == 0) ? false : true;
      
      bool viewCmdFoundInBranch = false;
      if (cmd->IsOfType(wxT("BranchCommand")))
      {
         if (IsAnyViewCommandInBranch(cmd))
            viewCmdFoundInBranch = true;
      }
      
      // Always show EndBranch command
      if (!cmd->IsOfType(wxT("EndBranch")) && !viewCmdFoundInBranch)
      {
         isLastItemHidden = true;
         #if DEBUG_BUILD_TREE_ITEM
         MessageInterface::ShowMessage
            (wxT("MissionTree::BuildTreeItem() returning '%s' node, hiding the node\n"),
             node.IsOk() ? wxT("good") : wxT("bad"));
         #endif
         return node;
      }
   }
   
   #if DEBUG_BUILD_TREE_ITEM
   MessageInterface::ShowMessage
      (wxT("   Creating command node for <%s>'%s'\n"), typeName.c_str(), cmdName.c_str());
   #endif
   
   node = UpdateCommandTree(parent, cmd, level);
   
   // If it is not a branch end, then show ellipsis
   if (isLastItemHidden && !cmd->IsOfType(wxT("BranchEnd")))
      ShowEllipsisInPreviousNode(parent, node);
   
   isLastItemHidden = false;
   
   if (cmd->GetTypeName() == wxT("BeginScript"))
      mScriptEventCount++;
   
   if (cmd->GetTypeName() == wxT("EndScript"))
      mScriptEventCount--;
   
   inScriptEvent = (mScriptEventCount == 0) ? false : true;
   
   #if DEBUG_BUILD_TREE_ITEM
   MessageInterface::ShowMessage
      (wxT("MissionTree::BuildTreeItem() returning '%s' node, showing the node\n"),
       node.IsOk() ? wxT("good") : wxT("bad"));
   #endif
   
   return node;
}


//------------------------------------------------------------------------------
// void UpdateCommand()
//------------------------------------------------------------------------------
/**
 * Updates commands in the mission sequence
 */
//------------------------------------------------------------------------------
void MissionTree::UpdateCommand()
{
   #if DEBUG_MISSION_TREE_SHOW_CMD
   MessageInterface::ShowMessage(wxT("MissionTree::UpdateCommand() entered\n"));
   #endif
   
   GmatCommand *cmd = theGuiInterpreter->GetFirstCommand();
   GmatCommand *child;
   MissionTreeItemData *seqItemData =
      (MissionTreeItemData *)GetItemData(mMissionSeqSubId);
   wxTreeItemId node;
   
   if (cmd->GetTypeName() == wxT("NoOp"))
      seqItemData->SetCommand(cmd);
   
   wxString typeName;
   bool isLastItemHidden = false;
   
   while (cmd != NULL)
   {
      node = BuildTreeItem(mMissionSeqSubId, cmd, 0, isLastItemHidden);
      
      if (isLastItemHidden)
      {
         cmd = cmd->GetNext();
         continue;
      }
      
      child = cmd->GetChildCommand(0);
      
      if (child != NULL)
      {
         ExpandChildCommand(node, cmd, 0);
      }
      
      cmd = cmd->GetNext();
   }
   
   Expand(mMissionSeqSubId);
   ////ScrollTo(mMissionSeqSubId);
   
   if (mWriteMissionSeq)
      ShowCommands(wxT("After Updating Command Sequence"));
   
}


//------------------------------------------------------------------------------
// wxTreeItemId& UpdateCommandTree(wxTreeItemId parent, GmatCommand *cmd, ...)
//------------------------------------------------------------------------------
/**
 * Updates commands in the mission sequence
 */
//------------------------------------------------------------------------------
wxTreeItemId& MissionTree::UpdateCommandTree(wxTreeItemId parent,
                                             GmatCommand *cmd, Integer level)
{
   #if DEBUG_MISSION_TREE
   MessageInterface::ShowMessage
      (wxT("MissionTree::UpdateCommandTree() entered, inScriptEvent=%d, cmd=<%p><%s>\n"),
       inScriptEvent, cmd, cmd->GetTypeName().c_str());
   #endif
   
   wxString cmdTypeName = cmd->GetTypeName().c_str();
   wxTreeItemId currId;
   mNewTreeId = currId;
   
   // If ScriptEvent mode or command is NoOp or BeginMissionSequence, don't add it 
   // This is different from command unviewable list
   if (inScriptEvent || cmdTypeName == wxT("NoOp") || cmdTypeName == wxT("BeginMissionSequence"))
   {
      #if DEBUG_MISSION_TREE
      MessageInterface::ShowMessage
         (wxT("MissionTree::UpdateCommandTree() leaving, command '%s' ignored\n"),
          cmdTypeName.c_str());
      #endif
      return mNewTreeId;
   }
   
   mNewTreeId = AppendCommand(parent, GetIconId(cmdTypeName),
                              GetCommandId(cmdTypeName), cmd,
                              GetCommandCounter(cmdTypeName),
                              *GetCommandCounter(cmdTypeName));
   
   #if DEBUG_MISSION_TREE
   MessageInterface::ShowMessage
      (wxT("   mUsingViewLevel=%d, mViewLevel=%d, level=%d\n"), mUsingViewLevel,
       mViewLevel, level);
   #endif
   
   if (mUsingViewLevel)
   {
      if (mViewLevel > level + 1)
         Expand(parent);
   }
   else
      Expand(parent);
   
   #if DEBUG_MISSION_TREE
   MessageInterface::ShowMessage
      (wxT("MissionTree::UpdateCommandTree() leaving, command <%s>'%s' added to tree\n"),
       cmdTypeName.c_str(), cmd->GetName().c_str());
   #endif
   
   return mNewTreeId;
}


//------------------------------------------------------------------------------
// void ExpandNode(wxTreeItemId node, const wxString &cmdType)
//------------------------------------------------------------------------------
/**
 * Expands the node or not based on the view level or view category.
 */
//------------------------------------------------------------------------------
void MissionTree::ExpandNode(wxTreeItemId node, const wxString &cmdType)
{
   #ifdef DEBUG_EXPAND_NODE
   MessageInterface::ShowMessage
      (wxT("MissionTree::ExpandNode() endtered, cmdType='%s'\n"), cmdType.c_str());
   #endif
   
   bool expand = false;
   int nodeLevel  = 0;
   wxTreeItemId parentId = GetItemParent(node);
   
   // Check upto 3 levels
   if (mUsingViewLevel)
   {
      if (parentId.IsOk())
      {
         nodeLevel = 1;
         wxTreeItemId gParentId = GetItemParent(parentId);
         if (gParentId.IsOk())
         {         
            nodeLevel = 2;
            wxTreeItemId ggParentId = GetItemParent(gParentId);
            if (ggParentId.IsOk())
               nodeLevel = 3;
         }
         
         if (mUsingViewLevel && mViewLevel >= nodeLevel)
            expand = true;
         
         if (expand)
            Expand(node);
      }
   }
   else
   {
      if (mViewCommands.Index(cmdType) != wxNOT_FOUND)
      {
         expand = true;
         if (parentId.IsOk())
         {
            MissionTreeItemData *parentItem = (MissionTreeItemData *)GetItemData(parentId); 
            GmatCommand *parentCmd = parentItem->GetCommand();
            if (parentCmd->IsOfType(wxT("BranchCommand")))
               expand = true;
         }
      }
      
      if (expand)
         Expand(parentId);
   }
   
   #ifdef DEBUG_EXPAND_NODE
   MessageInterface::ShowMessage
      (wxT("MissionTree::ExpandNode() nodeLevel = %d, mViewLevel = %d, expand = %d\n"),
       nodeLevel, mViewLevel, expand);
   #endif
}


//------------------------------------------------------------------------------
// void ExpandChildCommand(wxTreeItemId parent, GmatCommand *cmd)
//------------------------------------------------------------------------------
/**
 * Expands child commands in the mission sequence
 */
//------------------------------------------------------------------------------
void MissionTree::ExpandChildCommand(wxTreeItemId parent, GmatCommand *cmd,
                                     Integer level)
{
   #if DEBUG_MISSION_TREE_CHILD
   MessageInterface::ShowMessage
      (wxT("MissionTree::ExpandChildCommand() parent='%s', cmd='%s', level=%d\n"),
       GetItemText(parent).c_str(), cmd->GetTypeName().c_str(), level);
   #endif
   
   wxTreeItemId branchNode;
   wxTreeItemId node;
   wxTreeItemId elseNode;
   Integer childNo = 0;
   GmatCommand* nextInBranch;
   GmatCommand* child;
   wxString typeName;
   bool isLastItemHidden = false;
   bool useElseAsParent = false;
   
   while((child = cmd->GetChildCommand(childNo)) != NULL)
   {
      nextInBranch = child;
      
      #if DEBUG_MISSION_TREE_CHILD
      MessageInterface::ShowMessage(wxT("   nextInBranch='%s'\n"), nextInBranch->GetTypeName().c_str());
      MessageInterface::ShowMessage(wxT("   useElseAsParent=%d\n"), useElseAsParent);
      #endif
      
      while ((nextInBranch != NULL) && (nextInBranch != cmd))
      {
         #if DEBUG_MISSION_TREE_CHILD
         for (int i=0; i<=level; i++)
            MessageInterface::ShowMessage(wxT("-----"));
         MessageInterface::ShowMessage
            (wxT("----- (%p)'%s'\n"), nextInBranch, nextInBranch->GetTypeName().c_str());
         #endif
         
         // Special handling of Else since Else command is not a branch command
         // We want indent commands after Else
         if (useElseAsParent)
            node = BuildTreeItem(elseNode, nextInBranch, level, isLastItemHidden);
         else
            node = BuildTreeItem(parent, nextInBranch, level, isLastItemHidden);
			
         if (isLastItemHidden)
         {
            // If it is not a branch end, then show ellipsis
            if (!nextInBranch->IsOfType(wxT("BranchEnd")))
               ShowEllipsisInPreviousNode(parent, node);
            
            nextInBranch = nextInBranch->GetNext();
            continue;
         }
         
         if (nextInBranch->GetChildCommand() != NULL)
            ExpandChildCommand(node, nextInBranch, level+1);
         
         nextInBranch = nextInBranch->GetNext();
      }
      
      ++childNo;
   }
}


//------------------------------------------------------------------------------
// wxTreeItemId AppendCommand(wxTreeItemId parent, GmatTree::MissionIconType icon,
//                            GmatTree::ItemType type, GmatCommand *cmd,
//                            int *cmdCount, int endCount)
//------------------------------------------------------------------------------
/**
 * Appends command to command list and/or command tree.
 */
//------------------------------------------------------------------------------
wxTreeItemId
MissionTree::AppendCommand(wxTreeItemId parent, GmatTree::MissionIconType icon,
                           GmatTree::ItemType type, GmatCommand *cmd,
                           int *cmdCount, int endCount)
{
   #if DEBUG_APPEND_COMMAND
   MessageInterface::ShowMessage
      (wxT("MissionTree::AppendCommand('%s') type = \"%s\" and name = \"%s\"\n"),
       GetItemText(parent).c_str(),  cmd->GetTypeName().c_str(), 
       cmd->GetName().c_str());
   #endif
   
   wxString cmdTypeName = cmd->GetTypeName().c_str();
   wxString nodeName = cmd->GetName().c_str();
   
   wxTreeItemId node;
   
   // compose node name
   if (cmdTypeName.Contains(wxT("End")))
   {
      if (nodeName.Trim() == wxT(""))
         nodeName.Printf(wxT("%s%d"), cmdTypeName.c_str(), endCount);
   }
   else if (cmdTypeName.Contains(wxT("Else")))
   {
      if (nodeName.Trim() == wxT(""))
         nodeName.Printf(wxT("%s%d"), cmdTypeName.c_str(), endCount);
   }
   else
   {
      if (nodeName.Trim() == wxT("") || nodeName == cmdTypeName)
         nodeName.Printf(wxT("%s%d"), cmdTypeName.c_str(), ++(*cmdCount));      
   }
   
   // Show "ScriptEvent" instead of "BeginScript" to be more clear for user
   if (nodeName.Contains(wxT("BeginScript")))
      nodeName.Replace(wxT("BeginScript"), wxT("ScriptEvent"));
   
   // Show "Equation" instead of "GMAT" to be more clear for user
   if (nodeName.Contains(wxT("GMAT")))
      nodeName.Replace(wxT("GMAT"), wxT("Equation"));
   
   // Show command string as node label(loj: 2007.11.13)
   nodeName = GetCommandString(cmd, nodeName);
   /// Tell the command its name
   cmd->SetSummaryName(nodeName.c_str());
   
   #if DEBUG_APPEND_COMMAND
   MessageInterface::ShowMessage
      (wxT("MissionTree::AppendCommand() cmdTypeName='%s', nodeName='%s'\n"),
       cmdTypeName.c_str(), nodeName.c_str());
   #endif

   node = AppendItem(parent, nodeName, icon, -1,
                     new MissionTreeItemData(nodeName, type, nodeName, cmd));

   return node;
}


//------------------------------------------------------------------------------
// wxTreeItemId InsertNodeToBranch(wxTreeItemId parentId, wxTreeItemId currId, ...)
//------------------------------------------------------------------------------
wxTreeItemId
MissionTree::InsertNodeToBranch(wxTreeItemId parentId, wxTreeItemId currId,
                                wxTreeItemId prevId, GmatTree::MissionIconType icon,
                                const wxString &nodeName, GmatTree::ItemType itemType,
                                GmatCommand *cmd, GmatCommand *currCmd, GmatCommand *prevCmd,
                                bool insertBefore)
{
   #if DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage("InsertNodeToBranch() entered, insertBefore = %d\n", insertBefore);
   #endif
   
   wxTreeItemId node;
   wxTreeItemId realParentId = parentId;
   
   if (insertBefore)
      realParentId = prevId;
   
   wxString realParentName = GetItemText(realParentId);
   
   #if DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage
      (wxT("   previous type is NoOp, BeginMissionSequence, BranchCommand, or Else\n"));
   MessageInterface::ShowMessage
      (wxT("   ==> realParentId=%u'%s'\n"), realParentId, realParentName.c_str());
   #endif
   
   
   if (!insertBefore)
   {
      #if DEBUG_MISSION_TREE_INSERT
      MessageInterface::ShowMessage(wxT("   411 inserting by realParentId and position 0\n"));
      #endif
      node = InsertItem(realParentId, 0, nodeName, icon, -1,
                        new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
   }
   else if (IsExpanded(prevId))
   {
      #if DEBUG_MISSION_TREE_INSERT
      MessageInterface::ShowMessage(wxT("   Previous item is expanded\n"));
      #endif
      if (currCmd->IsOfType(wxT("BranchEnd")))
      {
         if (prevCmd->IsOfType(wxT("BranchCommand")))
         {
            #if DEBUG_MISSION_TREE_INSERT
            MessageInterface::ShowMessage(wxT("   421 inserting by parentId and position 0\n"));
            #endif
            node = InsertItem(parentId, 0, nodeName, icon, -1,
                              new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
         }
         else
         {
            #if DEBUG_MISSION_TREE_INSERT
            MessageInterface::ShowMessage(wxT("   422 inserting by parentId and prevId\n"));
            #endif
            node = InsertItem(parentId, prevId, nodeName, icon, -1,
                              new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
         }
      }
      else
      {
         #if DEBUG_MISSION_TREE_INSERT
         MessageInterface::ShowMessage(wxT("   423 inserting by parentId and position 0\n"));
         #endif
         node = InsertItem(parentId, 0, nodeName, icon, -1,
                           new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
      }
   }
   else
   {
      #if DEBUG_MISSION_TREE_INSERT
      MessageInterface::ShowMessage(wxT("   Neither current or previous item is expanded\n"));
      #endif
      if (!insertBefore)
      {
         #if DEBUG_MISSION_TREE_INSERT
         MessageInterface::ShowMessage(wxT("   431 inserting by realParentId and position 0\n"));
         #endif
         node = InsertItem(realParentId, 0, nodeName, icon, -1,
                           new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
      }
      else
      {
         if (currCmd->IsOfType(wxT("BranchEnd")))
         {
            #if DEBUG_MISSION_TREE_INSERT
            MessageInterface::ShowMessage(wxT("   432 inserting by realParentId and position 0\n"));
            #endif
            node = InsertItem(realParentId, 0, nodeName, icon, -1,
                              new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
         }
         else
         {
            #if DEBUG_MISSION_TREE_INSERT
            MessageInterface::ShowMessage(wxT("   433 inserting by parentId and prevId\n"));
            #endif
            node = InsertItem(parentId, prevId, nodeName, icon, -1,
                              new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
            
         }
      }
   }
   
   #if DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage(wxT("InsertNodeToBranch() leaving\n"));
   #endif
   return node;
}


//------------------------------------------------------------------------------
// wxTreeItemId InsertNodeAfter(wxTreeItemId parentId, wxTreeItemId currId,
//------------------------------------------------------------------------------
wxTreeItemId
MissionTree::InsertNodeAfter(wxTreeItemId parentId, wxTreeItemId currId,
                             wxTreeItemId prevId, GmatTree::MissionIconType icon,
                             const wxString &nodeName, GmatTree::ItemType itemType,
                             GmatCommand *cmd, bool insertBefore)
{
   #if DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage(wxT("InsertNodeAfter() entered, insertBefore = %d\n"), insertBefore);
   #endif
   
   wxTreeItemId node;
   wxTreeItemId realParentId = parentId;
   wxTreeItemId realPrevId = prevId;
   
   if (insertBefore)
   {
      if (GetChildrenCount(GetItemParent(prevId)) > 0)
      {
         realParentId = GetItemParent(prevId);
      }
      else
      {
         realParentId = prevId;
         realPrevId = GetLastChild(prevId);
      }
   }
   
   wxTreeItemId prevVisId = GetPrevVisible(currId);
   wxString prevVisName = GetItemText(prevVisId);
   wxString parentName = GetItemText(realParentId);
   wxString realPrevName = GetItemText(realPrevId);
   
   #if DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage
      (wxT("   ==> realParentId=%u'%s', realPrevId=%u'%s', prevVisId=%u'%s'\n"),
       realParentId,  parentName.c_str(), realPrevId, realPrevName.c_str(),
       prevVisId, prevVisName.c_str());
   #endif

   if (GetItemParent(currId) == prevVisId)
   {
      if (insertBefore)
      {
         #if DEBUG_MISSION_TREE_INSERT
         MessageInterface::ShowMessage(wxT("   311 inserting by prevVisId and position 0\n"));
         #endif
         node = InsertItem(prevVisId, 0, nodeName, icon, -1,
                           new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
      }
      else
      {
         if (realPrevName == prevVisName)
         {
            #if DEBUG_MISSION_TREE_INSERT
            MessageInterface::ShowMessage(wxT("   312 appending to realPrevId\n"));
            #endif
            node = AppendItem(prevVisId, nodeName, icon, -1,
                              new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
         }
         else
         {
            #if DEBUG_MISSION_TREE_INSERT
            MessageInterface::ShowMessage(wxT("   313 insertnig by realParentId and realPrevId\n"));
            #endif
            node = InsertItem(realParentId, realPrevId, nodeName, icon, -1,
                              new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
         }
      }
   }
   else if (realPrevId == prevVisId)
   {
      if (GetItemParent(prevVisId) == GetItemParent(currId))
      {
         #if DEBUG_MISSION_TREE_INSERT
         MessageInterface::ShowMessage(wxT("   321 inserting by parentId and realPrevId\n"));
         #endif
         node = InsertItem(parentId, realPrevId, nodeName, icon, -1,
                           new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
      }
      else
      {
         #if DEBUG_MISSION_TREE_INSERT
         MessageInterface::ShowMessage(wxT("   322 inserting by parentId and realParentId\n"));
         #endif
         node = InsertItem(parentId, realParentId, nodeName, icon, -1,
                           new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
      }
   }
   else
   {
      #if DEBUG_MISSION_TREE_INSERT
      MessageInterface::ShowMessage(wxT("   331 inserting by realParentId and realPrevId\n"));
      #endif
      node = InsertItem(realParentId, realPrevId, nodeName, icon, -1,
                        new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
   }
   
   #if DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage(wxT("InsertNodeAfter() leaving\n"));
   #endif
   
   return node;
}


//------------------------------------------------------------------------------
// wxTreeItemId InsertCommand(wxTreeItemId parentId, wxTreeItemId currId,
//                            wxTreeItemId prevId, GmatTree::MissionIconType icon,
//                            GmatTree::ItemType itemType, const wxString &cmdName,
//                            GmatCommand *prevCmd, GmatCommand *cmd, int *cmdCount,
//                            bool insertBefore)
//------------------------------------------------------------------------------
/**
 * Inserts command to mission tree.
 *
 * @param  parentId  Parent item id
 * @param  currId  Currently selected item id
 * @param  prevId  Previous item id
 * @param  icon  Icon to be used
 * @param  itemType  Item type, such as GmatTree::PROPAGATE
 * @param  cmdName  Command type name to be inserted
 * @param  prevCmd  Previous command pointer
 * @param  cmd  Command pointer to be inserted
 * @param  cmdCount  Command counter to be appended to command name
 * @param  insertBefore  true if inserting before the current item,
 *                       false if inserterting after current item
 */
//------------------------------------------------------------------------------
wxTreeItemId
MissionTree::InsertCommand(wxTreeItemId parentId, wxTreeItemId currId,
                           wxTreeItemId prevId, GmatTree::MissionIconType icon,
                           GmatTree::ItemType itemType, const wxString &cmdName,
                           GmatCommand *prevCmd, GmatCommand *cmd, int *cmdCount,
                           bool insertBefore)
{
   #if DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage
      (wxT("MissionTree::InsertCommand() parentId='%s', currId='%s', prevId='%s', ")
       wxT("insertBefore=%d\n"), GetItemText(parentId).c_str(), GetItemText(currId).c_str(),
       GetItemText(prevId).c_str(), insertBefore);
   #endif
   
   MissionTreeItemData *currItem = (MissionTreeItemData *)GetItemData(currId);
   GmatCommand *currCmd = currItem->GetCommand();
	wxString parentName = GetItemText(parentId);
	wxString currItemName = GetItemText(currId);
   wxString currTypeName = currCmd->GetTypeName().c_str();
   wxString cmdTypeName = cmd->GetTypeName().c_str();   
   wxString prevTypeName = prevCmd->GetTypeName().c_str();
   wxString nodeName = cmd->GetName().c_str();
   wxTreeItemId node;
   GmatCommand *endCmd = NULL;
   GmatCommand *elseCmd = NULL;
   GmatTree::ItemType endType = GmatTree::END_TARGET;
   bool cmdAdded = false;
   
   #if DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage
      (wxT("   currItemName='%s', currTypeName='%s', prevTypeName='%s'\n"),
		 currItemName.c_str(), currTypeName.c_str(), prevTypeName.c_str());
   #endif
	
   // Show "Equation" instead of "GMAT" to be more clear for user
   if (currTypeName == wxT("GMAT"))
      currTypeName = wxT("Equation");
   if (cmdTypeName == wxT("GMAT"))
      cmdTypeName = wxT("Equation");
   if (prevTypeName == wxT("GMAT"))
      prevTypeName = wxT("Equation");
   
   // Show "ScriptEvent" instead of "BeginScript" to be more clear for user
   if (currTypeName == wxT("BeginScript"))
      currTypeName = wxT("ScriptEvent");
   if (cmdTypeName == wxT("BeginScript"))
      cmdTypeName = wxT("ScriptEvent");
   if (prevTypeName == wxT("BeginScript"))
      prevTypeName = wxT("ScriptEvent");
   
   #if DEBUG_MISSION_TREE_INSERT
   MissionTreeItemData *parentItem = (MissionTreeItemData *)GetItemData(parentId); 
   GmatCommand *parentCmd = parentItem->GetCommand();
   MessageInterface::ShowMessage
      (wxT("   cmdName='%s', cmdTypeName='%s', cmdCount=%d\n"), cmdName.c_str(),
       cmdTypeName.c_str(), *cmdCount);
   MessageInterface::ShowMessage
      (wxT("   parentCmd='%s', prevCmd='%s', prevTypeName='%s', currCmd='%s'\n"),
       parentCmd->GetTypeName().c_str(), prevCmd->GetTypeName().c_str(),
       prevTypeName.c_str(), currTypeName.c_str());
   #endif
   
   // If previous command is BeginScript, find matching EndScript,
   // since commands inside EndScript, including EndScript is not shown
   // on the tree.
   if (prevCmd->GetTypeName() == wxT("BeginScript"))
   {
      GmatCommand *endScript = GmatCommandUtil::GetMatchingEnd(prevCmd);
      #if DEBUG_MISSION_TREE_APPEND
      MessageInterface::ShowMessage
         (wxT("   setting prevCmd to '%s'\n"), prevCmd->GetTypeName().c_str());
      #endif
      prevCmd = endScript;
   }
   
   
   //@Note "BeginFiniteBurn" is not a branch command but need "EndFiniteBurn"
   
   //------------------------------------------------------------
   // Create End* command if branch command
   //------------------------------------------------------------
   if (cmdTypeName == wxT("Target") || cmdTypeName == wxT("For")  ||
       cmdTypeName == wxT("While")  ||  cmdTypeName == wxT("If")  ||
       cmdTypeName == wxT("ScriptEvent") || cmdTypeName == wxT("Optimize"))
   {
      #if DEBUG_MISSION_TREE_INSERT
      MessageInterface::ShowMessage
         (wxT("   Creating End* for '%s'\n"), cmdTypeName.c_str());
      #endif

		endCmd = CreateEndCommand(cmdTypeName, endType);
		
      #if DEBUG_MISSION_TREE_INSERT
      if (endCmd != NULL)
         MessageInterface::ShowMessage
            (wxT("   '%s' created\n"), endCmd->GetTypeName().c_str());
      #endif
      
      // create Else for IfElse
      if (cmdName == wxT("IfElse"))
      {
         elseCmd = CreateCommand(wxT("Else"));
         cmd->Append(elseCmd);
      }
      
      #if DEBUG_MISSION_TREE_INSERT
      MessageInterface::ShowMessage
         (wxT("   ==> Calling cmd->Append('%s')\n"), endCmd->GetTypeName().c_str());
      #endif
      
      cmdAdded = cmd->Append(endCmd);
      
      #if DEBUG_MISSION_TREE_INSERT
      WriteCommand(wxT("   "), wxT("previous of "), endCmd , wxT(" is "), endCmd->GetPrevious());
      #endif
      
   }
   
   //------------------------------------------------------------
   // Compose node name
   //------------------------------------------------------------
   if (cmdTypeName == wxT("Else"))
      nodeName.Printf(wxT("%s%d"), cmdTypeName.c_str(), (*cmdCount));
   
   // if command has name or command has the type name then append counter
   if (nodeName.Trim() == wxT("") || nodeName == cmdTypeName)
      nodeName.Printf(wxT("%s%d"), cmdTypeName.c_str(), ++(*cmdCount));
   
   // Show command string as node label(loj: 2007.11.13)
   cmd->SetSummaryName(nodeName.c_str());
   nodeName = GetCommandString(cmd, nodeName);
   
   #if DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage
      (wxT("   cmd='%s', nodeName='%s', cmdCount=%d\n"), cmdTypeName.c_str(),
       nodeName.c_str(), *cmdCount);
   #endif
   
   //------------------------------------------------------------
   // Insert command to mission sequence
   //------------------------------------------------------------
   
   #if DEBUG_MISSION_TREE_INSERT
   WriteCommand(wxT("   ==> before appending/inserting: "), wxT("previous of "), cmd, wxT(" is "),
                cmd->GetPrevious());
   #endif
   
   if (currCmd->GetTypeName() == wxT("NoOp") ||
       currCmd->GetTypeName() == wxT("BeginMissionSequence"))
   {
      #if DEBUG_MISSION_TREE_INSERT
      MessageInterface::ShowMessage(wxT("   ==> Calling gui->AppendCommand()\n"));
      #endif
      
      // Append to base command list
      cmdAdded = theGuiInterpreter->AppendCommand(cmd);
   }
   else
   {
      #if DEBUG_MISSION_TREE_INSERT
      MessageInterface::ShowMessage(wxT("   ==> Calling gui->InsertCommand()\n"));
      #endif
      
      cmdAdded = theGuiInterpreter->InsertCommand(cmd, prevCmd);
   }
   
   //------------------------------------------------------------
   // We need to set real previous command after command is
   // appended/inserted, since cmd->AppendCommand() or
   // cmd->InsertCommand() resets previous command.
   // So when ScriptEvent is modified, the old ScriptEvent is
   // deleted and new one can be insterted into correct place.
   //------------------------------------------------------------
   #if DEBUG_MISSION_TREE_INSERT
   WriteCommand(wxT("   ==>"), wxT(" Resetting previous of "), cmd, wxT("to "), prevCmd);
   #endif
   
   cmd->ForceSetPrevious(prevCmd);
   
   #if DEBUG_MISSION_TREE_INSERT
   WriteCommand(wxT("   ==> after  appending/inserting: "), wxT("previous of "), cmd, wxT(" is "),
                cmd->GetPrevious());
   #endif
   
   //MessageInterface::ShowMessage("   ==> cmdAdded = %d\n", cmdAdded);   
   // Why returning false eventhough it inserted?
   cmdAdded = true; 
   
   //------------------------------------------------------------
   // Insert command to mission tree
   //------------------------------------------------------------
   if (cmdAdded)
   {
      if (currCmd->GetTypeName() == wxT("NoOp") ||
          currCmd->GetTypeName() == wxT("BeginMissionSequence"))
      {
         node = AppendItem(currId, nodeName, icon, -1,
                           new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
      }
		else if (currId == prevId && !insertBefore)
		{
         #if DEBUG_MISSION_TREE_INSERT
			MessageInterface::ShowMessage
				(wxT("   111 inserting '%s' after '%s' from parent '%s'\n"), nodeName.c_str(),
				 currItemName.c_str(), parentName.c_str());
         #endif
			node = InsertItem(parentId, currId, nodeName, icon, -1,
									new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
		}
      else if (prevTypeName == wxT("NoOp")     || prevTypeName == wxT("BeginMissionSequence") ||
               prevTypeName == wxT("Target")   || prevTypeName == wxT("For")    ||
               prevTypeName == wxT("While")    || prevTypeName == wxT("If")     ||
               prevTypeName == wxT("Optimize"))
      {
         node = InsertNodeToBranch(parentId, currId, prevId, icon, nodeName, itemType, cmd,
                                   currCmd, prevCmd, insertBefore);
      }
      else if (prevTypeName.Contains(wxT("End")) && prevTypeName != wxT("EndScript") &&
               prevTypeName != wxT("EndFiniteBurn"))
      {
         wxTreeItemId realParentId = parentId;
         wxTreeItemId realPrevId = prevId;			
         wxString realParentName = GetItemText(realParentId);
         wxString realPrevName = GetItemText(realPrevId);
			
			if (realParentName == wxT(""))
			{
				realParentId = parentId;
				realParentName = GetItemText(realParentId);
			}
			
         #if DEBUG_MISSION_TREE_INSERT
         MessageInterface::ShowMessage(wxT("   ==> previous type contains End\n"));
         MessageInterface::ShowMessage
            (wxT("   ==> realParentId=%u'%s'\n"), realParentId, realParentName.c_str());
         MessageInterface::ShowMessage
            (wxT("   ==> realPrevId=%u'%s'\n"), realPrevId, realPrevName.c_str());
         #endif
			
			if (realParentId == realPrevId)
			{
            #if DEBUG_MISSION_TREE_INSERT
				MessageInterface::ShowMessage
					(wxT("   211 inserting '%s' after '%s' from parent '%s'\n"), nodeName.c_str(), currItemName.c_str(),
					 realParentName.c_str());
            #endif
				
				node = InsertItem(realParentId, currId, nodeName, icon, -1,
										new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
			}
			else if (realParentName != wxT(""))
			{
				MissionTreeItemData *realPrevItem = (MissionTreeItemData *)GetItemData(realPrevId);
				GmatCommand *realPrevCmd = realPrevItem->GetCommand();
				
				if (realPrevCmd->IsOfType(wxT("BranchEnd")))
				{
               #if DEBUG_MISSION_TREE_INSERT
					MessageInterface::ShowMessage
						(wxT("===> Previous node is BranchEnd, so setting previous to its parent\n"));
					#endif
					realPrevId = GetItemParent(realPrevId);					
				}
				
            #if DEBUG_MISSION_TREE_INSERT
				MessageInterface::ShowMessage
					(wxT("   221 inserting '%s' after '%s' from parent '%s'\n"), nodeName.c_str(), realPrevName.c_str(),
					 realParentName.c_str());
            #endif
				
				node = InsertItem(realParentId, realPrevId, nodeName, icon, -1,
										new MissionTreeItemData(nodeName, itemType, nodeName, cmd));
			}
			else
			{
            #if DEBUG_MISSION_TREE_INSERT
				MessageInterface::ShowMessage
					(wxT("   231 inserting '%s' after '%s', parent='%s'\n"), nodeName.c_str(),
					 currItemName.c_str(), realParentName.c_str());
            #endif
				node = InsertNodeAfter(realParentId, currId, prevId, icon, nodeName, itemType,
											  cmd, insertBefore);
			}
		}
      else
      {
         #if DEBUG_MISSION_TREE_INSERT
			MessageInterface::ShowMessage
				(wxT("   241 inserting '%s' after '%s', parent='%s'\n"), nodeName.c_str(),
				 currItemName.c_str(), GetItemText(parentId).c_str());
         #endif
         node = InsertNodeAfter(parentId, currId, prevId, icon, nodeName, itemType,
                                cmd, insertBefore);
      }
      
      
      //---------------------------------------------------------
      // Append End* command
      //---------------------------------------------------------
      if (cmdTypeName == wxT("Target") || cmdTypeName == wxT("For")  ||
          cmdTypeName == wxT("While")  || cmdTypeName == wxT("If")   ||
          cmdTypeName == wxT("Optimize"))
      {
         // append Else (temp code until Else is implemented)
         if (cmdName == wxT("IfElse"))
         {
            wxString elseName;
            elseName.Printf(wxT("Else%d"), (*cmdCount));
				elseCmd->SetSummaryName(elseName.c_str());
            
            wxTreeItemId elseNode =
               InsertItem(node, 0, elseName, icon, -1,
                          new MissionTreeItemData(elseName, GmatTree::ELSE_CONTROL,
                                                  elseName, elseCmd));
            
            wxString endName = wxT("End") + cmdTypeName;
            wxString tmpName;
            tmpName.Printf(wxT("%s%d"), endName.c_str(), *cmdCount);
				endCmd->SetSummaryName(tmpName.c_str());
            InsertItem(node, elseNode, tmpName, GmatTree::MISSION_ICON_NEST_RETURN, -1,
                       new MissionTreeItemData(tmpName, endType, tmpName, endCmd));
         }
         else
         {
            wxString endName = wxT("End") + cmdTypeName;
            wxString tmpName;
            tmpName.Printf(wxT("%s%d"), endName.c_str(), *cmdCount);
				endCmd->SetSummaryName(tmpName.c_str());
            InsertItem(node, 0, tmpName, GmatTree::MISSION_ICON_NEST_RETURN, -1,
                       new MissionTreeItemData(tmpName, endType, tmpName, endCmd));
         }
      }
   }
   else
   {
      MessageInterface::ShowMessage
         (wxT("*** ERROR *** Command:'%s' not appended or created\n"),
          cmd->GetTypeName().c_str());
   }
   
   return node;
}


//------------------------------------------------------------------------------
// void Append(const wxString &cmdTypeName)
//------------------------------------------------------------------------------
/*
 * Appends a command to the end of the branch identified by parent.
 * The parent is the current selection, assuming Append menu item only appears
 * on branch command, such as Target, If, For, Optimize.
 * It sets parent item id, current item id, and previous item id and pass them
 * to InsertCommand() with insertBefore flag to true.
 *
 * @param  cmdName  Command type name to be appended
 */
//------------------------------------------------------------------------------
void MissionTree::Append(const wxString &cmdTypeName)
{
   wxTreeItemId itemId = GetSelection();
   wxTreeItemId lastChildId = GetLastChild(itemId);
   wxTreeItemId parentId = GetItemParent(itemId);
   wxTreeItemId currId = itemId;
   wxString itemText = GetItemText(itemId);
   MissionTreeItemData *currItem = (MissionTreeItemData *)GetItemData(itemId);
   GmatCommand *currCmd = currItem->GetCommand();
   
   #if DEBUG_MISSION_TREE_SHOW_CMD
   ShowCommands(wxT("Before Appending '") + cmdTypeName + wxT("' to '") + itemText + wxT("'"));
   #endif
   
   #ifdef __TEST_MISSION_TREE_ACTIONS__
   if (mSaveActions)
   {
      wxString str;
      str.Printf(wxT("Append '%s' to '%s'\n"), cmdTypeName.c_str(), itemText.c_str());
      WriteActions(str);
   }
   #endif
   
   #if DEBUG_MISSION_TREE_APPEND
   MessageInterface::ShowMessage
      (wxT("\nMissionTree::Append() cmdTypeName='%s', itemId='%s', lastChildId='%s'\n"),
       cmdTypeName.c_str(), GetItemText(itemId).c_str(), GetItemText(lastChildId).c_str());
   WriteCommand("   ", "currCmd = ", currCmd);
   #endif
	
	
	// Now Else part is not indented (LOJ: 2011.12.02)
   //======================================================================
   // Note:
   // Previous command is the 2nd last visible command from the current node
   // For example:
   // Target
   //    If          <-- If appending command, previous command should be propagate
   //       Maneuver
   //       Else     <-- There is no Append for Else
   //       Propagate
   //       EndIf
   //    EndTarget
   //======================================================================
   wxTreeItemId prevId;
   
   if (lastChildId.IsOk() && GetItemText(lastChildId) != wxT(""))
      prevId = GetPrevVisible(lastChildId);
   else
      prevId = currId;
   
   #if DEBUG_MISSION_TREE_APPEND
   MessageInterface::ShowMessage(wxT("   prevId='%s'\n"), GetItemText(prevId).c_str());
   #endif
   
   MissionTreeItemData *prevItem = (MissionTreeItemData *)GetItemData(prevId);
   GmatCommand *prevCmd = prevItem->GetCommand();
   GmatCommand *cmd = NULL;
   
   #if DEBUG_MISSION_TREE_APPEND
   WriteCommand(wxT("   "), wxT("currCmd = "), currCmd, wxT(", prevCmd = "), prevCmd);
   #endif
   
   bool insertBefore = false;
   
   // For BranchCommand, use GmatCommandUtil::GetMatchingEnd() to get
   // previous command, since there may be commands not shown on the tree.
   // Note: Make sure this works on other than Windows also (loj: 12/15/06)
   if (currCmd->IsOfType(wxT("BranchCommand")))
   {
      #if DEBUG_MISSION_TREE_APPEND
      MessageInterface::ShowMessage("   ==> current item is BranchCommand\n");
      #endif
      
      parentId = itemId;
      currId = GetLastChild(itemId);
      
      GmatCommand *branchEnd = GmatCommandUtil::GetMatchingEnd(currCmd);
		GmatCommand *realPrevCmd = branchEnd->GetPrevious();
      wxString realPrevType = realPrevCmd->GetTypeName();
      
      #if DEBUG_MISSION_TREE_APPEND
      WriteCommand(wxT("   "), wxT("branchEnd = "), branchEnd, wxT(", realPrevCmd = "), realPrevCmd);
		MessageInterface::ShowMessage(wxT("   ==> setting prevCmd to end->GetPrevious()\n"));
      #endif
		
		prevCmd = realPrevCmd;
		
      #if DEBUG_MISSION_TREE_APPEND
      WriteCommand(wxT("   "), wxT("prevCmd = "), prevCmd);
      #endif
      
      // If previous command is BranchCommand and not current command
      if (prevCmd->IsOfType(wxT("BranchCommand")) && prevCmd != currCmd)
      {
         #if DEBUG_MISSION_TREE_APPEND
         MessageInterface::ShowMessage
            (wxT("   previous command is '%s' and not '%s'\n"), 
             prevCmd->GetTypeName().c_str(), currCmd->GetTypeName().c_str());
         #endif
         
         prevCmd = GmatCommandUtil::GetMatchingEnd(prevCmd);
         
         #if DEBUG_MISSION_TREE_APPEND
         MessageInterface::ShowMessage
            (wxT("   so setting prevCmd to '%s'\n"), prevCmd->GetTypeName().c_str(),
             currCmd->GetTypeName().c_str(), prevCmd->GetTypeName().c_str());
         #endif
      }
      
      #if DEBUG_MISSION_TREE_APPEND
      MessageInterface::ShowMessage(wxT("   previous item is %s\n"), GetItemText(prevId).c_str());
      #endif
      
      // If previous command is BranchEnd and visible,
      // previous item should be parent of BranchEnd
      if (prevCmd->IsOfType(wxT("BranchEnd")))
      {
         if (GetItemParent(prevId) != itemId)
            prevId = GetItemParent(prevId);
      }
   }
   else if (currId == mMissionSeqSubId)
   {
      #if DEBUG_MISSION_TREE_APPEND
      MessageInterface::ShowMessage(wxT("   current item is MissionSequence\n"));
      #endif
      
      parentId = itemId;
      if (lastChildId.IsOk() && GetItemText(lastChildId) != wxT(""))
         currId = lastChildId;
      prevId = currId;
      prevCmd = prevItem->GetCommand();
   }
   else
   {
      #if DEBUG_MISSION_TREE_APPEND
      MessageInterface::ShowMessage
         (wxT("   current Item is not BranckCommand, Else, or MissionSequence\n"));
      #endif
      
      // handle case prevItem is NULL
      if (prevItem != NULL)
      {
         #if DEBUG_MISSION_TREE_APPEND
         MessageInterface::ShowMessage(wxT("   setting prevCmd from prevItem\n"));
         #endif
         prevCmd = prevItem->GetCommand();
      }
      else
      {
         #if DEBUG_MISSION_TREE_APPEND
         MessageInterface::ShowMessage(wxT("   setting prevCmd from currItem\n"));
         #endif
         prevCmd = currItem->GetCommand();
      }
   }
   
   #if DEBUG_MISSION_TREE_APPEND
   MessageInterface::ShowMessage
      (wxT("   currCmd='%s', prevCmd='%s'\n"),  currCmd->GetTypeName().c_str(),
       prevCmd->GetTypeName().c_str());
   #endif
   
   // Create a new command
   cmd = CreateCommand(cmdTypeName);
   
   // Insert a node to tree
   if (cmd != NULL)
   {
      UpdateGuiManager(cmdTypeName);
      
      if (currCmd->GetTypeName() == wxT("NoOp") ||
          currCmd->GetTypeName() == wxT("BeginMissionSequence"))
      {
         // Use GetLastCommand() to get last command since some command
         // doesn't appear on the tree, such as EndScript
         prevCmd = GmatCommandUtil::GetLastCommand(prevCmd);
         #if DEBUG_MISSION_TREE_APPEND
         MessageInterface::ShowMessage
            (wxT("   Current command is NoOp or BeginMissionSequence\n"));
         WriteCommand(wxT("   ==>"), wxT(" new prevCmd = "), prevCmd);
         #endif
         
         if (prevCmd->IsOfType(wxT("BranchCommand")))
             prevCmd = GmatCommandUtil::GetMatchingEnd(prevCmd);
      }
      
      // Need to set previous command of new command
      cmd->ForceSetPrevious(prevCmd);
      
      #if DEBUG_MISSION_TREE_APPEND
      MessageInterface::ShowMessage
         (wxT("   ==> Calling InsertCommand(%s), parent='%s', current='%s', previous='%s'\n"),
			 insertBefore ? wxT("before") : wxT("after"), GetItemText(parentId).c_str(),
			 GetItemText(currId).c_str(), GetItemText(prevId).c_str());
      #endif
      
      // Added to tree node if visible command
      wxTreeItemId node =
         InsertCommand(parentId, currId, prevId, GetIconId(cmdTypeName),
                       GetCommandId(cmdTypeName), cmdTypeName, prevCmd, cmd,
                       GetCommandCounter(cmdTypeName), insertBefore);
      
      Expand(itemId);
      ////Expand(node);
      ExpandNode(node, cmdTypeName);
      SelectItem(node);
   }
   
   if (mWriteMissionSeq)
      ShowCommands(wxT("After Appending '") + cmdTypeName + wxT("' to '") + itemText + wxT("'"));
   
   #ifdef __TEST_MISSION_TREE_ACTIONS__
   if (mSaveActions || mPlaybackActions)
      WriteResults();
   #endif
}


//------------------------------------------------------------------------------
// void InsertBefore(const wxString &cmdTypeName)
//------------------------------------------------------------------------------
/*
 * Inserts a command before current selection.
 *
 * @param  cmdTypeName  Command type name to be inserted
 */
//------------------------------------------------------------------------------
void MissionTree::InsertBefore(const wxString &cmdTypeName)
{
   wxTreeItemId itemId = GetSelection();
   wxTreeItemId parentId = GetItemParent(itemId);
   wxString itemText = GetItemText(itemId);
   
   #if DEBUG_MISSION_TREE_SHOW_CMD
   ShowCommands(wxT("Before Inserting '") + cmdTypeName + wxT("' before '") + itemText + wxT("'"));
   #endif
   
   #ifdef __TEST_MISSION_TREE_ACTIONS__
   if (mSaveActions)
   {
      wxString str;
      str.Printf(wxT("Insert '%s' before '%s'\n"), cmdTypeName.c_str(), GetItemText(itemId).c_str());
      WriteActions(str);
   }
   #endif
	
   wxTreeItemId prevId = GetPrevVisible(itemId);
	
	#if DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage
      (wxT("MissionTree::InsertBefore('%s') parentId='%s', itemId='%s', ")
		 wxT("prevId='%s'\n"), cmdTypeName.c_str(), GetItemText(parentId).c_str(),
		 GetItemText(itemId).c_str(), GetItemText(prevId).c_str());
   #endif
	
   MissionTreeItemData *currItem = (MissionTreeItemData *)GetItemData(itemId);
   MissionTreeItemData *prevItem = (MissionTreeItemData *)GetItemData(prevId);
   
   // Do not insert anything if previous item is empty
   if (prevItem == NULL)
   {
      MessageInterface::ShowMessage
         (wxT("\n***************  Warning ***************")
          wxT("\nMissionTree::InsertBefore() has empty prevItem ")
          wxT("so it can't insert before this.")
          wxT("\n****************************************"));
      return;
   }
   
   GmatCommand *currCmd = currItem->GetCommand();
   GmatCommand *prevCmd = currCmd->GetPrevious();
   GmatCommand *realPrevCmd = currCmd->GetPrevious();
   GmatCommand *cmd = NULL;
   
   // We want to use real previous command via cmd->GetPrevious(), not from
   // the tree, because some commands are not visible from the tree.
      
   if (currCmd == NULL)
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_, wxT("*** Internal Error Occurred ***\n")
          wxT("Current item has empty command. Cannot insert the command.\n"));
      return;
   }
   
   #if DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage
      (wxT("MissionTree::InsertBefore('%s') currCmd='%s'(%p)\n"), cmdTypeName.c_str(),
       currCmd->GetTypeName().c_str(), currCmd);
   WriteCommand(wxT("   "), wxT("prevCmd = "), prevCmd, wxT(", realPrevCmd = "), realPrevCmd);
   #endif
   
   
   if (prevCmd == NULL)
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_, wxT("*** Internal Error Occurred ***\n")
          wxT("The previous command is empty. Cannot insert the command.\n"));
      
      ShowCommands(wxT("Before Insert: ") + cmdTypeName);
      MessageInterface::ShowMessage
         (wxT("InsertBefore('%s') currCmd='%s', addr=%p\n"),
          cmdTypeName.c_str(), currCmd->GetTypeName().c_str(), currCmd);
      
      return;
   }
   
   #if DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage
      (wxT("MissionTree::InsertBefore('%s') prevCmd='%s'(%p)\n"),
       cmdTypeName.c_str(), prevCmd->GetTypeName().c_str(), prevCmd);
   #endif
   
   // If previous command is BranchCmmand check to see we need to use matching
   // BranchEnd as previous command
   if (prevCmd->IsOfType(wxT("BranchCommand")))
   {
      // check if first child is current command
      if (prevCmd->GetChildCommand(0) == currCmd)
         realPrevCmd = prevCmd;
      else
         realPrevCmd = GmatCommandUtil::GetMatchingEnd(prevCmd);
   }
   
   #if DEBUG_MISSION_TREE_INSERT
   WriteCommand(wxT("   "), wxT("realPrevCmd = "), realPrevCmd);
   #endif

	
   if (realPrevCmd != NULL)
   {
      cmd = CreateCommand(cmdTypeName);      
      
      if (cmd != NULL)
      {
         // Set previous command to realPrevCmd (loj: 2007.05.16)
         cmd->ForceSetPrevious(realPrevCmd);
         
         #if DEBUG_MISSION_TREE_INSERT
         WriteCommand(wxT("   "), wxT("cmd->GetPrevious() = "), cmd->GetPrevious());
         #endif
         
         UpdateGuiManager(cmdTypeName);

			bool insertBefore = true;
			if (realPrevCmd->IsOfType(wxT("BranchEnd")))
				insertBefore = false;
			
         #if DEBUG_MISSION_TREE_INSERT
			MessageInterface::ShowMessage
				(wxT("   ==> Calling InsertCommand(%s), parent='%s', current='%s', previous='%s'\n"),
				 insertBefore ? wxT("before") : wxT("after"), GetItemText(parentId).c_str(),
				 GetItemText(itemId).c_str(), GetItemText(prevId).c_str());
         #endif
			
         wxTreeItemId node =
            InsertCommand(parentId, itemId, prevId, GetIconId(cmdTypeName),
                          GetCommandId(cmdTypeName), cmdTypeName, realPrevCmd, cmd,
                          GetCommandCounter(cmdTypeName), insertBefore);
         
         ////Expand(parentId);
         ////Expand(prevId);
         Expand(node);
         SelectItem(node);
      }
   }
   
   if (mWriteMissionSeq)
      ShowCommands(wxT("After Inserting '") + cmdTypeName + wxT("' before '") + itemText + wxT("'"));
   
   #ifdef __TEST_MISSION_TREE_ACTIONS__
   if (mSaveActions || mPlaybackActions)
      WriteResults();
   #endif
} // InsertBefore()


//------------------------------------------------------------------------------
// void InsertAfter(const wxString &cmdTypeName)
//------------------------------------------------------------------------------
/*
 * Inserts a command after current selection.
 *
 * @param  cmdTypeName  Command name to be inserted
 */
//------------------------------------------------------------------------------
void MissionTree::InsertAfter(const wxString &cmdTypeName)
{
   wxTreeItemId itemId = GetSelection();
   wxTreeItemId parentId = GetItemParent(itemId);
   wxString itemText = GetItemText(itemId);
   
   #if DEBUG_MISSION_TREE_SHOW_CMD
   ShowCommands(wxT("Before Inserting '") + cmdTypeName + wxT("' after '") + itemText + wxT("'"));
   #endif
   
   #ifdef __TEST_MISSION_TREE_ACTIONS__
   if (mSaveActions)
   {
      wxString str;
      str.Printf(wxT("Insert '%s' after '%s'\n"), cmdTypeName.c_str(), itemText.c_str());
      WriteActions(str);
   }
   #endif

	wxTreeItemId prevId = itemId;
   MissionTreeItemData *currItem = (MissionTreeItemData *)GetItemData(itemId);
   GmatCommand *currCmd = currItem->GetCommand();
   
   if (currCmd == NULL)
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_, wxT("*** Internal Error Occurred ***\n")
          wxT("Current item has empty command. Cannot insert the command.\n"));
      return;
   }
   
   #if DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage
      (wxT("MissionTree::InsertAfter('%s') currCmd='%s'(%p)\n"), cmdTypeName.c_str(),
       currCmd->GetTypeName().c_str(), currCmd);
   #endif
   
   GmatCommand *prevCmd = currCmd;   
   GmatCommand *cmd = NULL;
   
   if (currCmd != NULL)
   {
      cmd = CreateCommand(cmdTypeName);
      
      // Need to set previous command
      //cmd->ForceSetPrevious(currCmd);
      
      // Set parentId, itemId, prevId properly to pass to InsertCommand()
      // If current node is BranchCommand, it inserts after BranchEnd (2011.09.27 new Requirement)
      if (currCmd->IsOfType(wxT("BranchCommand")))
      {
         GmatCommand *branchEnd = NULL;
			branchEnd = GmatCommandUtil::GetMatchingEnd(currCmd);
         #if DEBUG_MISSION_TREE_INSERT
			MessageInterface::ShowMessage
				(wxT("   ==> Insert after BranchCommand, branchEnd=<%p><%s>\n"),
				 branchEnd, branchEnd->GetTypeName().c_str());
         #endif
			cmd->ForceSetPrevious(branchEnd);
			prevCmd = branchEnd;
			prevId = itemId;
			parentId = GetItemParent(itemId);
      }
      else if (currCmd->IsOfType(wxT("BranchEnd")))
      {
         cmd->ForceSetPrevious(currCmd);

			// If inserting after BranchEnd, insert it after Branch command
         #if DEBUG_MISSION_TREE_INSERT
			MessageInterface::ShowMessage(wxT("   ==> Insert after BranchEnd\n"));
         #endif
			
			// If it is not a Else then reassign nodes, since Else is not a really
			// BranchEnd in the tree(LOJ: 2011.12.02)
			if (!currCmd->IsOfType(wxT("Else")))
			{
				itemId = parentId;
				parentId = GetItemParent(itemId);
				prevId = itemId;
			}
      }
      else
      {
         cmd->ForceSetPrevious(currCmd);
         prevId = itemId;
      }
      
      
      if (cmd != NULL)
      {
         UpdateGuiManager(cmdTypeName);
			
         #if DEBUG_MISSION_TREE_INSERT
         MessageInterface::ShowMessage
				(wxT("   ==> Calling InsertCommand(insertAfter), parent='%s', currItem='%s', ")
				 wxT("prevItem='%s'\n"), GetItemText(parentId).c_str(), GetItemText(itemId).c_str(),
				 GetItemText(prevId).c_str());
         #endif

         wxTreeItemId node =
            InsertCommand(parentId, itemId, prevId, GetIconId(cmdTypeName),
                          GetCommandId(cmdTypeName), cmdTypeName, prevCmd, cmd,
                          GetCommandCounter(cmdTypeName), false);
         
         ////Expand(parentId);
         Expand(node);
         SelectItem(node);
      }
   }
   
   if (mWriteMissionSeq)
      ShowCommands(wxT("After Inserting '") + cmdTypeName + wxT("' after '") + itemText + wxT("'"));
   
   #ifdef __TEST_MISSION_TREE_ACTIONS__
   if (mSaveActions || mPlaybackActions)
      WriteResults();
   #endif
} // InsertAfter()


//------------------------------------------------------------------------------
// void DeleteCommand(const wxString &cmdName)
//------------------------------------------------------------------------------
/*
 * Deletes a command from the tree and command sequence.
 *
 * @param  cmdName  Command name to be deleted
 */
//------------------------------------------------------------------------------
void MissionTree::DeleteCommand(const wxString &cmdName)
{
   #if DEBUG_MISSION_TREE_DELETE
   MessageInterface::ShowMessage
      (wxT("MissionTree::Delete() entered, cmdName='%s'\n"), cmdName.c_str());
   #endif
   
   // Get selected item
   wxTreeItemId itemId = GetSelection();
   wxTreeItemId parentId = GetItemParent(itemId);
   wxString itemText = GetItemText(itemId);
   
   #if DEBUG_MISSION_TREE_SHOW_CMD
   ShowCommands(wxT("Before Deleting '") + cmdName + wxT("' from '") + itemText + wxT("'"));
   #endif
   
   #if DEBUG_MISSION_TREE_DELETE
   MessageInterface::ShowMessage
      (wxT("   itemId='%s', parentId='%s'\n"), itemText.c_str(),
       GetItemText(parentId).c_str());
   #endif
   
   #ifdef __TEST_MISSION_TREE_ACTIONS__
   if (mSaveActions)
   {
      wxString str;
      str.Printf(wxT("Delete '%s' from '%s'\n"), itemText.c_str(),
                 GetItemText(parentId).c_str());
      WriteActions(str);
   }
   #endif
   
   
   // delete from gui interpreter
   MissionTreeItemData *missionItem = (MissionTreeItemData *)GetItemData(itemId);
   if (missionItem == NULL)
   {
      // write error message
      MessageInterface::ShowMessage
         (wxT("\n*** ERROR *** could not delete '%s' due to NULL item\n"),
         cmdName.c_str());
      return;
   }
   
   GmatCommand *theCmd = missionItem->GetCommand();  
   if (theCmd == NULL)
   {
      // write error message
      MessageInterface::ShowMessage
         (wxT("\n*** ERROR *** could not delete '%s' due to NULL command\n"),
         cmdName.c_str());
      return;
   }
   
   // save command type to check if there is no more of this command
   wxString cmdType = theCmd->GetTypeName();
   
   #if DEBUG_MISSION_TREE_DELETE
   MessageInterface::ShowMessage
      (wxT("   Calling theGuiInterpreter->DeleteCommand('%s')\n"), theCmd->GetTypeName().c_str());
   MessageInterface::ShowMessage
      (wxT("   Previous of '%s' is '%s'\n"), theCmd->GetTypeName().c_str(),
       theCmd->GetPrevious()->GetTypeName().c_str());
   if (theCmd->GetNext() == NULL)
      MessageInterface::ShowMessage
         (wxT("   Next of '%s' is NULL\n"), theCmd->GetTypeName().c_str());
   else
      MessageInterface::ShowMessage
         (wxT("   Next of '%s' is '%s'\n"), theCmd->GetTypeName().c_str(),
          theCmd->GetNext()->GetTypeName().c_str());
   #endif
   
   GmatCommand *tmp = theGuiInterpreter->DeleteCommand(theCmd);
   if (tmp)
   {
      #if DEBUG_MISSION_TREE_DELETE
      MessageInterface::ShowMessage(wxT("   About to delete <%p>\n"), tmp);
      #endif
      delete tmp;
      tmp = NULL;
   }
   
   // reset counter if there is no more of this command
   #if DEBUG_MISSION_TREE_DELETE
   MessageInterface::ShowMessage
      (wxT("   Checking if the command counter needs to be reset\n"));
   #endif
   wxString seqString = theGuiInterpreter->GetScript();
   if (seqString.find(cmdType) == seqString.npos)
   {
      #if DEBUG_MISSION_TREE_DELETE
      MessageInterface::ShowMessage
         (wxT("   Resetting the command counter of '%s'\n"), cmdType.c_str());
      #endif
      int *cmdCounter = GetCommandCounter(cmdType);
      *cmdCounter = 0;
   }
   
   // delete from tree - if parent only has 1 child collapse
   #if DEBUG_MISSION_TREE_DELETE
   MessageInterface::ShowMessage
      (wxT("   Checking if the parent item needs to be collapsed\n"));
   #endif
   if (GetChildrenCount(parentId) <= 1)
   {
      #if DEBUG_MISSION_TREE_DELETE
      MessageInterface::ShowMessage
         (wxT("   About to collapse parent tree item '%s'\n"), GetItemText(parentId).c_str());
      #endif
      this->Collapse(parentId);
   }
   
   #if DEBUG_MISSION_TREE_DELETE
   MessageInterface::ShowMessage
      (wxT("   About to delete tree item '%s'\n"), itemText.c_str());
   #endif
   this->Delete(itemId);
   
   if (mWriteMissionSeq)
      ShowCommands(wxT("After Deleting '") + cmdName + wxT("' from '") + itemText + wxT("'"));
   
   #ifdef __TEST_MISSION_TREE_ACTIONS__
   if (mSaveActions || mPlaybackActions)
      WriteResults();
   #endif
   
   #if DEBUG_MISSION_TREE_DELETE
   MessageInterface::ShowMessage
      (wxT("MissionTree::Delete() leaving, cmdName='%s'\n"), cmdName.c_str());
   #endif
}


//------------------------------------------------------------------------------
// void UpdateGuiManager(const wxString &cmdName)
//------------------------------------------------------------------------------
/*
 * Calls GuiItemManager to update corresponding object list to dynamically
 * show new objects.
 *
 * @param  cmdName  Command name
 */
//------------------------------------------------------------------------------
void MissionTree::UpdateGuiManager(const wxString &cmdName)
{
   if (cmdName == wxT("Maneuver") || cmdName == wxT("BeginFiniteBurn") ||
       cmdName == wxT("Vary"))
      theGuiManager->UpdateBurn();
   
   if (cmdName == wxT("Target") || cmdName == wxT("Optimize") || cmdName == wxT("Vary") ||
       cmdName == wxT("Achieve") || cmdName == wxT("Minimize"))
      theGuiManager->UpdateSolver();
   
   if (cmdName == wxT("Report"))
      theGuiManager->UpdateSubscriber();
   
   // Always update parameter, since it is used in many commands
   theGuiManager->UpdateParameter();
}


//------------------------------------------------------------------------------
// void AddDefaultMission()
//------------------------------------------------------------------------------
/**
 * Adds a default mission to tree.
 */
//------------------------------------------------------------------------------
void MissionTree::AddDefaultMission()
{
   //----- Mission Sequence
   
   wxTreeItemId mission =
      AddRoot(wxT("Mission"), -1, -1,
              new MissionTreeItemData(wxT("Mission"), GmatTree::MISSIONS_FOLDER));
   
   //-----------------------------------------------------------------
   #ifdef __ENABLE_MULTIPLE_SEQUENCE__
   //-----------------------------------------------------------------
   mMissionSeqTopId =
      AppendItem(mission, wxT("Mission Sequence"), GmatTree::MISSION_ICON_FOLDER, -1,
                 new MissionTreeItemData(wxT("Mission Sequence"),
                                         GmatTree::MISSION_SEQ_TOP_FOLDER));
   
   SetItemImage(mMissionSeqTopId, GmatTree::MISSION_ICON_OPENFOLDER,
               wxTreeItemIcon_Expanded);
   
   AddDefaultMissionSeq(mMissionSeqTopId);
   
   //-----------------------------------------------------------------
   #else
   //-----------------------------------------------------------------
   
   mMissionSeqSubId =
      AppendItem(mission, wxT("Mission Sequence"), GmatTree::MISSION_ICON_FOLDER, -1,
                 new MissionTreeItemData(wxT("Mission Sequence"),
                                         GmatTree::MISSION_SEQ_SUB_FOLDER));
   
   SetItemImage(mMissionSeqSubId, GmatTree::MISSION_ICON_OPENFOLDER,
               wxTreeItemIcon_Expanded);
   
   //-----------------------------------------------------------------
   #endif
   //-----------------------------------------------------------------
   
   UpdateCommand();
   if (theNotebook)
      theNotebook->SetMissionTreeExpandLevel(10); // level > 3 expands all
   theGuiInterpreter->ResetConfigurationChanged(false, true);
   
}

//------------------------------------------------------------------------------
// void AddDefaultMissionSeq(wxTreeItemId item)
//------------------------------------------------------------------------------
void MissionTree::AddDefaultMissionSeq(wxTreeItemId item)
{
   #if DEBUG_MISSION_TREE
   MessageInterface::ShowMessage(wxT("MissionTree::AddDefaultMission() entered\n"));
   #endif
   
   #ifdef __ENABLE_MULTIPLE_SEQUENCE
   StringArray itemNames = theGuiInterpreter->GetListOfConfiguredItems(Gmat::MISSION_SEQ);
   
   int size = itemNames.size();
   for (int i = 0; i<size; i++)
   {
      wxString objName = wxString(itemNames[i].c_str());
      AppendItem(item, objName, GmatTree::MISSION_ICON_FOLDER, -1,
                 new MissionTreeItemData(objName,
                                         GmatTree::MISSION_SEQ_COMMAND));
   };
   #endif
   
   wxString name;   
   name.Printf(wxT("Sequence%d"), ++mNumMissionSeq);
   
   mMissionSeqSubId =
      AppendItem(item, name, GmatTree::MISSION_ICON_FOLDER, -1,
                 new MissionTreeItemData(name, GmatTree::MISSION_SEQ_SUB_FOLDER));
   
   SetItemImage(mMissionSeqSubId, GmatTree::MISSION_ICON_OPENFOLDER,
                wxTreeItemIcon_Expanded);
   
   Expand(item);
}

//------------------------------------------------------------------------------
// void AddIcons()
//------------------------------------------------------------------------------
/**
 * Adds icons to a list, so that they can be used in the tree.
 */
//------------------------------------------------------------------------------
void MissionTree::AddIcons()
{
   #ifdef DEBUG_ADD_ICONS
   MessageInterface::ShowMessage
      (wxT("ResourceTree::AddIcons() entered, GmatTree::MISSION_ICON_COUNT=%d\n"),
       GmatTree::MISSION_ICON_COUNT);
   #endif
   
   int sizeW = 16;
   int sizeH = 16;
   
   wxImageList *images = new wxImageList ( sizeW, sizeH, true );
   wxBitmap* bitmaps[GmatTree::MISSION_ICON_COUNT];
   int index = 0;
   long bitmapType = wxBITMAP_TYPE_PNG;
   
   // Show hourglass temporarily busy cursor
   wxBusyCursor wait;
   
   // Icons should follow the order in GmatTreeItemData::MissionIconType.
   theGuiManager->LoadIcon(wxT("propagateevent"), bitmapType, &bitmaps[index], propagateevent_xpm);
   theGuiManager->LoadIcon(wxT("target"), bitmapType, &bitmaps[++index], target_xpm);
   theGuiManager->LoadIcon(wxT("folder"), bitmapType, &bitmaps[++index], folder_xpm);
   theGuiManager->LoadIcon(wxT("file"), bitmapType, &bitmaps[++index], file_xpm);
   theGuiManager->LoadIcon(wxT("OpenFolder"), bitmapType, &bitmaps[++index], OpenFolder_xpm);
   
   theGuiManager->LoadIcon(wxT("whileloop"), bitmapType, &bitmaps[++index], whileloop_xpm);
   theGuiManager->LoadIcon(wxT("forloop"), bitmapType, &bitmaps[++index], forloop_xpm);
   theGuiManager->LoadIcon(wxT("if"), bitmapType, &bitmaps[++index], if_xpm);
   theGuiManager->LoadIcon(wxT("scriptevent"), bitmapType, &bitmaps[++index], scriptevent_xpm);
   theGuiManager->LoadIcon(wxT("varyevent"), bitmapType, &bitmaps[++index], varyevent_xpm);
   
   theGuiManager->LoadIcon(wxT("achieveevent"), bitmapType, &bitmaps[++index], achieveevent_xpm);
   theGuiManager->LoadIcon(wxT("deltav"), bitmapType, &bitmaps[++index], deltav_xpm);
   theGuiManager->LoadIcon(wxT("callfunction"), bitmapType, &bitmaps[++index], callfunction_xpm);
   theGuiManager->LoadIcon(wxT("nestreturn"), bitmapType, &bitmaps[++index], nestreturn_xpm);
   theGuiManager->LoadIcon(wxT("saveobject"), bitmapType, &bitmaps[++index], saveobject_xpm);
   
   theGuiManager->LoadIcon(wxT("equalsign"), bitmapType, &bitmaps[++index], equalsign_xpm);
   theGuiManager->LoadIcon(wxT("toggle"), bitmapType, &bitmaps[++index], toggle_xpm);
   theGuiManager->LoadIcon(wxT("beginfb"), bitmapType, &bitmaps[++index], beginfb_xpm);
   theGuiManager->LoadIcon(wxT("endfb"), bitmapType, &bitmaps[++index], endfb_xpm);
   theGuiManager->LoadIcon(wxT("report"), bitmapType, &bitmaps[++index], report_xpm);
   
   theGuiManager->LoadIcon(wxT("mt_Stop"), bitmapType, &bitmaps[++index], mt_Stop_xpm);
   theGuiManager->LoadIcon(wxT("penup"), bitmapType, &bitmaps[++index], penup_xpm);
   theGuiManager->LoadIcon(wxT("pendown"), bitmapType, &bitmaps[++index], pendown_xpm);
   theGuiManager->LoadIcon(wxT("mt_MarkPoint"), bitmapType, &bitmaps[++index], mt_MarkPoint_xpm);
   theGuiManager->LoadIcon(wxT("mt_ClearPlot"), bitmapType, &bitmaps[++index], mt_ClearPlot_xpm);

   theGuiManager->LoadIcon(wxT("mt_Global"), bitmapType, &bitmaps[++index], mt_Global_xpm);   
   theGuiManager->LoadIcon(wxT("mt_SaveMission"), bitmapType, &bitmaps[++index], mt_SaveMission_xpm);   
   theGuiManager->LoadIcon(wxT("optimize"), bitmapType, &bitmaps[++index], optimize_xpm);
   theGuiManager->LoadIcon(wxT("mt_Minimize"), bitmapType, &bitmaps[++index], mt_Minimize_xpm);
   theGuiManager->LoadIcon(wxT("mt_NonlinearConstraint"), bitmapType, &bitmaps[++index], mt_NonlinearConstraint_xpm);
   
   theGuiManager->LoadIcon(wxT("mt_RunSimulator"), bitmapType, &bitmaps[++index], mt_RunSimulator_xpm);
   theGuiManager->LoadIcon(wxT("mt_RunEstimator"), bitmapType, &bitmaps[++index], mt_RunEstimator_xpm);
   theGuiManager->LoadIcon(wxT("mt_Default"), bitmapType, &bitmaps[++index], mt_Default_xpm);
   
   // Rescale if bitmap size is not 16x16 and use high quality scale (LOJ: 2011.04.22)
   int w, h;
   for ( size_t i = 0; i < WXSIZEOF(bitmaps); i++ )
   {
      w = bitmaps[i]->GetWidth();
      h = bitmaps[i]->GetHeight();
      
      #ifdef DEBUG_ADD_ICONS
      MessageInterface::ShowMessage(wxT("   bitmaps[%2d], w=%d, h=%d\n"), i, w, h);
      #endif
      
      wxImage image = bitmaps[i]->ConvertToImage();
      if (w != sizeW || h != sizeH)
      {
         #ifdef DEBUG_ADD_ICONS
         MessageInterface::ShowMessage(wxT("   rescaling image to %d x %d\n"), sizeW, sizeH);
         #endif
         
         image.Rescale(sizeW, sizeH, wxIMAGE_QUALITY_HIGH);
      }
      
      images->Add(image);
      //images->Add(bitmaps[i]->ConvertToImage().Rescale(sizeW, sizeH));
   }
   
   AssignImageList(images);
   
   #ifdef DEBUG_ADD_ICONS
   MessageInterface::ShowMessage
      (wxT("ResourceTree::AddIcons() exiting, %d icons added\n"), index + 1);
   #endif
}


//------------------------------------------------------------------------------
// void OnItemRightClick(wxTreeEvent& event)
//------------------------------------------------------------------------------
/**
 * Brings up popup menu on a right click.
 *
 * @param <event> input event.
 */
//------------------------------------------------------------------------------
void MissionTree::OnItemRightClick(wxTreeEvent& event)
{
   //wxWidgets-2.6.3 does not need this but wxWidgets-2.8.0 needs to SelectItem
   SelectItem(event.GetItem());
   mLastClickPoint = event.GetPoint();
   ShowMenu(event.GetItem(), event.GetPoint());
}


//------------------------------------------------------------------------------
// void OnItemActivated(wxTreeEvent &event)
//------------------------------------------------------------------------------
/**
 * On a double click sends the TreeItemData to GmatMainFrame to open a new
 * window.
 *
 * @param <event> input event.
 */
//------------------------------------------------------------------------------
void MissionTree::OnItemActivated(wxTreeEvent &event)
{
   // get some info about this item
   wxTreeItemId itemId = event.GetItem();
   MissionTreeItemData *item = (MissionTreeItemData *)GetItemData(itemId);
   MissionTreeItemData *parent = (MissionTreeItemData *)GetItemData(GetItemParent(itemId));
   
   #if DEBUG_MISSION_TREE
   MessageInterface::ShowMessage
      (wxT("MissionTree::OnItemActivated() item='%s' parent='%s'\n"),
       item->GetTitle().c_str(), parent->GetTitle().c_str());
   #endif
   
   // Since VaryPanel is used for both Target and Optimize,
   // set proper id indicating Optimize Vary
   if ((item->GetItemType() == GmatTree::VARY) &&
       (parent->GetItemType() == GmatTree::OPTIMIZE))
      item->SetItemType(GmatTree::OPTIMIZE_VARY);
   
   theMainFrame->CreateChild(item);
}


//------------------------------------------------------------------------------
// void OnDoubleClick(wxMouseEvent &event)
//------------------------------------------------------------------------------
/**
 * Handles double click on an item.
 *
 * @param <event> input event.
 */
//------------------------------------------------------------------------------
void MissionTree::OnDoubleClick(wxMouseEvent &event)
{
   //MessageInterface::ShowMessage(wxT("MissionTree::OnDoubleClick() entered\n"));
   //wxPoint position = event.GetPosition();
   //MessageInterface::ShowMessage(wxT("Event position is %d %d\n"), position.x, position.y );
   
   wxTreeItemId itemId = GetSelection();
   MissionTreeItemData *item = (MissionTreeItemData *)GetItemData(itemId);
   MissionTreeItemData *parent = (MissionTreeItemData *)GetItemData(GetItemParent(itemId));
   
   #if DEBUG_MISSION_TREE
   MessageInterface::ShowMessage
      (wxT("MissionTree::OnDoubleClick() item='%s', parent='%s', theMainFrame=<%p>, ")
       wxT("theMainFrame->theMdiChildren=<%p>\n"), item->GetTitle().c_str(), parent->GetTitle().c_str(),
       theMainFrame);
   #endif
   
   // Since VaryPanel is used for both Target and Optimize,
   // set proper id indicating Optimize Vary
   if ((item->GetItemType() == GmatTree::VARY) &&
       (parent->GetItemType() == GmatTree::OPTIMIZE))
      item->SetItemType(GmatTree::OPTIMIZE_VARY);
   
   // Show panel here. because OnItemActivated() always collapse the node.
   theMainFrame->CreateChild(item);
   
   //CheckClickIn(position);
}


//------------------------------------------------------------------------------
// void ShowMenu(wxTreeItemId id, const wxPoint& pt)
//------------------------------------------------------------------------------
/**
 * Creates and shows a popup menu.
 *
 * @param <id> input TreeItemId.
 * @param <pt> input location for popup menu.
 */
//------------------------------------------------------------------------------
void MissionTree::ShowMenu(wxTreeItemId id, const wxPoint& pt)
{
   MissionTreeItemData *treeItem = (MissionTreeItemData *)GetItemData(id);
   wxString title = treeItem->GetTitle();
   GmatTree::ItemType itemType = treeItem->GetItemType();
   wxTreeItemId parent = GetItemParent(id);
   
   #if DEBUG_MISSION_TREE_MENU
   MessageInterface::ShowMessage
      (wxT("MissionTree::ShowMenu() itemType=%d\n"), itemType);
   #endif
   
#if wxUSE_MENUS
   wxMenu menu;
   
   if (itemType == GmatTree::MISSION_SEQ_TOP_FOLDER)
   {
      menu.Append(POPUP_ADD_MISSION_SEQ, wxT("Add Mission Sequence"));
      menu.Enable(POPUP_ADD_MISSION_SEQ, FALSE);
   }
   else if (itemType == GmatTree::MISSION_SEQ_SUB_FOLDER)
   {
      menu.Append(POPUP_COLLAPSE, wxT("Collapse All"));
      menu.Append(POPUP_EXPAND, wxT("Expand All"));

      if (mViewAll)
      {
         menu.AppendSeparator();
         menu.Append(POPUP_APPEND, wxT("Append"), CreateSubMenu(itemType, APPEND));
      }
      
      // If multiple sequence is enabled
      #ifdef __ENABLE_MULTIPLE_SEQUENCE__
      menu.Append(POPUP_DELETE, wxT("Delete"));
      #endif
      
      menu.Enable(POPUP_RENAME, FALSE);
      menu.AppendSeparator();
      menu.Append(POPUP_RUN, wxT("Run"));
      menu.AppendSeparator();
      
      menu.AppendCheckItem(POPUP_SHOW_DETAIL, wxT("Show Detail"));
      menu.Check(POPUP_SHOW_DETAIL, mShowDetailedItem);
      menu.Append(POPUP_SHOW_MISSION_SEQUENCE, wxT("Show Mission Sequence"));
      menu.Append(POPUP_SHOW_SCRIPT, wxT("Show Script"));
      menu.AppendSeparator();
      menu.Append(POPUP_MISSION_SUMMARY_ALL, wxT("Mission Summary - All"));
      menu.Append(POPUP_MISSION_SUMMARY_PHYSICS, wxT("Mission Summary - Physics"));
      
      menu.AppendSeparator();
      menu.Append(POPUP_DOCK_MISSION_TREE, wxT("Dock Mission Tree"));
      menu.Append(POPUP_UNDOCK_MISSION_TREE, wxT("Undock Mission Tree"));
      
      //----- for auto testing actions
      #ifdef __TEST_MISSION_TREE_ACTIONS__
      menu.AppendSeparator();
      menu.Append(POPUP_START_SAVE_ACTIONS, wxT("Start Save Actions"));
      menu.Append(POPUP_STOP_SAVE_ACTIONS, wxT("Stop Save Actions"));
      menu.Append(POPUP_READ_ACTIONS, wxT("Playback Actions"));
      #endif
   }
   else
   {
      // add to non-EndBranch item
      if (itemType < GmatTree::BEGIN_NO_PANEL)
      {
         menu.Append(POPUP_OPEN, wxT("Open"));
         menu.Append(POPUP_CLOSE, wxT("Close"));
      }
      
      if (mViewAll)
      {
         menu.AppendSeparator();
         if (itemType == GmatTree::TARGET)
         {
            menu.Append(POPUP_APPEND, wxT("Append"),
                        CreateTargetSubMenu(itemType, APPEND));
            menu.Append(POPUP_INSERT_BEFORE, wxT("Insert Before"),
                        CreateSubMenu(itemType, INSERT_BEFORE));         
            menu.Append(POPUP_INSERT_AFTER, wxT("Insert After"),
                        CreateTargetSubMenu(itemType, INSERT_AFTER));
         }
         else if (itemType == GmatTree::OPTIMIZE)
         {
            menu.Append(POPUP_APPEND, wxT("Append"),
                        CreateOptimizeSubMenu(itemType, APPEND));
            menu.Append(POPUP_INSERT_BEFORE, wxT("Insert Before"),
                        CreateSubMenu(itemType, INSERT_BEFORE));         
            menu.Append(POPUP_INSERT_AFTER, wxT("Insert After"),
                        CreateOptimizeSubMenu(itemType, INSERT_AFTER));
         }
         else if (itemType == GmatTree::END_TARGET)
         {
            menu.Append(POPUP_INSERT_BEFORE, wxT("Insert Before"),
                        CreateTargetSubMenu(itemType, INSERT_BEFORE));
            menu.Append(POPUP_INSERT_AFTER, wxT("Insert After"),
                        CreateSubMenu(itemType, INSERT_AFTER));
         }
         else if (itemType == GmatTree::END_OPTIMIZE)
         {
            menu.Append(POPUP_INSERT_BEFORE, wxT("Insert Before"),
                        CreateOptimizeSubMenu(itemType, INSERT_BEFORE));
            menu.Append(POPUP_INSERT_AFTER, wxT("Insert After"),
                        CreateSubMenu(itemType, INSERT_AFTER));
         }
         else 
         {
            GmatTree::ItemType itemType;
            if (IsInsideSolver(id, itemType))
            {
               if (itemType == GmatTree::TARGET)
               {
                  menu.Append(POPUP_INSERT_BEFORE, wxT("Insert Before"),
                              CreateTargetSubMenu(itemType, INSERT_BEFORE));
                  menu.Append(POPUP_INSERT_AFTER, wxT("Insert After"),
                              CreateTargetSubMenu(itemType, INSERT_AFTER));
               }
               else if (itemType == GmatTree::OPTIMIZE)
               {
                  menu.Append(POPUP_INSERT_BEFORE, wxT("Insert Before"),
                              CreateOptimizeSubMenu(itemType, INSERT_BEFORE));
                  menu.Append(POPUP_INSERT_AFTER, wxT("Insert After"),
                              CreateOptimizeSubMenu(itemType, INSERT_AFTER));
               }
            }
            else
            {
               menu.Append(POPUP_INSERT_BEFORE, wxT("Insert Before"),
                           CreateSubMenu(itemType, INSERT_BEFORE));
               menu.Append(POPUP_INSERT_AFTER, wxT("Insert After"),
                           CreateSubMenu(itemType, INSERT_AFTER));
            }
         }
         
         // Append is allowed for the control logic
         if ((itemType == GmatTree::IF_CONTROL) ||
				 // ELSE is not a BranchCommand so commented out (LOJ: 2011.12.01)
             //(itemType == GmatTree::ELSE_CONTROL) ||
             (itemType == GmatTree::FOR_CONTROL) ||
             (itemType == GmatTree::WHILE_CONTROL))
         {
            // Use menu.Insert() to make Append appear before insert before/after
            // just like other branch command
            size_t insertPos = menu.GetMenuItemCount() - 2;
            #ifdef DEBUG_MENU
            MessageInterface::ShowMessage(wxT("   ---> insertPos = %d\n"), insertPos);
            #endif
            
            GmatTree::ItemType itemType;
            if (IsInsideSolver(id, itemType))
            {
               if (itemType == GmatTree::TARGET)
               {
                  menu.Insert(insertPos, POPUP_APPEND, wxT("Append"),
                              CreateTargetSubMenu(itemType, APPEND));   
               }
               else if (itemType == GmatTree::OPTIMIZE)
               {
                  menu.Insert(insertPos, POPUP_APPEND, wxT("Append"),
                              CreateOptimizeSubMenu(itemType, APPEND));   
               }
            }
            else
            {
               menu.Insert(insertPos, POPUP_APPEND, wxT("Append"),
                           CreateSubMenu(itemType, APPEND));
            }
         }
      }
      
      // Delete applies to all, except End branch
      if (itemType < GmatTree::BEGIN_NO_PANEL || itemType == GmatTree::STOP)
      {
         menu.AppendSeparator();
         menu.Append(POPUP_RENAME, wxT("Rename"));
         menu.Append(POPUP_DELETE, wxT("Delete"));
      }
      
      menu.AppendSeparator();
      menu.Append(POPUP_COMMAND_SUMMARY, wxT("Command Summary"));
      
   }
   
   PopupMenu(&menu, pt);
   
   
#endif // wxUSE_MENUS
}


//------------------------------------------------------------------------------
// void OnAddMissionSeq(wxCommandEvent &event)
//------------------------------------------------------------------------------
void MissionTree::OnAddMissionSeq(wxCommandEvent &event)
{
   wxTreeItemId itemId = GetSelection();
   wxString name;
   
   name.Printf(wxT("Sequence%d"), ++mNumMissionSeq);
   
   mMissionSeqSubId =
      AppendItem(itemId, name, GmatTree::MISSION_ICON_FOLDER, -1,
                 new MissionTreeItemData(name, GmatTree::MISSION_SEQ_SUB_FOLDER));
    
   SetItemImage(mMissionSeqSubId, GmatTree::MISSION_ICON_OPENFOLDER,
                wxTreeItemIcon_Expanded);

   Expand(itemId);
}


//------------------------------------------------------------------------------
// void OnPopupAppend(wxCommandEvent &event)
//------------------------------------------------------------------------------
void MissionTree::OnPopupAppend(wxCommandEvent &event)
{
   #ifdef DEBUG_MISSION_TREE_APPEND
   MessageInterface::ShowMessage
      (wxT("MissionTree::OnPopupAppend() entered, event id = %d, itemStr = '%s'\n"),
       event.GetId(), event.GetString().c_str());
   #endif
   
}


//------------------------------------------------------------------------------
// void OnAppend(wxCommandEvent &event)
//------------------------------------------------------------------------------
void MissionTree::OnAppend(wxCommandEvent &event)
{
   #ifdef DEBUG_MISSION_TREE_APPEND
   MessageInterface::ShowMessage
      (wxT("MissionTree::OnAppend() entered, event id = %d, itemStr = '%s'\n"),
       event.GetId(), event.GetString().c_str());
   #endif
   
   switch (event.GetId())
   {
   case POPUP_APPEND_PROPAGATE:
      Append(wxT("Propagate"));
      break;
   case POPUP_APPEND_MANEUVER:
      Append(wxT("Maneuver"));
      break;
   case POPUP_APPEND_BEGIN_FINITE_BURN:
      Append(wxT("BeginFiniteBurn"));
      break;
   case POPUP_APPEND_END_FINITE_BURN:
      Append(wxT("EndFiniteBurn"));
      break;
   case POPUP_APPEND_TARGET:
      Append(wxT("Target"));
      break;
   case POPUP_APPEND_OPTIMIZE:
      Append(wxT("Optimize"));
      break;
   case POPUP_APPEND_VARY:
      Append(wxT("Vary"));
      break;
   case POPUP_APPEND_ACHIEVE:
      Append(wxT("Achieve"));
      break;
   case POPUP_APPEND_MINIMIZE:
      Append(wxT("Minimize"));
      break;
   case POPUP_APPEND_NON_LINEAR_CONSTRAINT:
      Append(wxT("NonlinearConstraint"));
      break;
   case POPUP_APPEND_CALL_GMAT_FUNCTION:
      Append(wxT("CallGmatFunction"));
      break;
   case POPUP_APPEND_CALL_MATLAB_FUNCTION:
      Append(wxT("CallMatlabFunction"));
      break;
   case POPUP_APPEND_ASSIGNMENT:
      Append(wxT("Equation"));
      break;
   case POPUP_APPEND_REPORT:
      Append(wxT("Report"));
      break;
   case POPUP_APPEND_TOGGLE:
      Append(wxT("Toggle"));
      break;
   case POPUP_APPEND_SAVE:
      Append(wxT("Save"));
      break;
   case POPUP_APPEND_STOP:
      Append(wxT("Stop"));
      break;
   case POPUP_APPEND_SCRIPT_EVENT:
      Append(wxT("BeginScript"));
      break;
   case POPUP_APPEND_IF:
      Append(wxT("If"));
      break;
   case POPUP_APPEND_IF_ELSE:
      Append(wxT("IfElse"));
      break;
   case POPUP_APPEND_ELSE:
      Append(wxT("Else"));
      break;
   case POPUP_APPEND_ELSE_IF:
      //Append(wxT("ElseIf"));
      break;
   case POPUP_APPEND_FOR:
      Append(wxT("For"));
      break;
   case POPUP_APPEND_WHILE:
      Append(wxT("While"));
      break;
   case POPUP_APPEND_D0_WHILE:
      //Append(wxT("Do"));
      break;
   case POPUP_APPEND_SWITCH:
      //Append(wxT("Switch"));
      break;
   default:
      break;
   }
   
   #ifdef DEBUG_MISSION_TREE_APPEND
   MessageInterface::ShowMessage(wxT("==> MissionTree::OnAppend() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void OnInsertBefore(wxCommandEvent &event)
//------------------------------------------------------------------------------
void MissionTree::OnInsertBefore(wxCommandEvent &event)
{
   switch (event.GetId())
   {
   case POPUP_INSERT_BEFORE_PROPAGATE:
      InsertBefore(wxT("Propagate"));
      break;
   case POPUP_INSERT_BEFORE_MANEUVER:
      InsertBefore(wxT("Maneuver"));
      break;
   case POPUP_INSERT_BEFORE_BEGIN_FINITE_BURN:
      InsertBefore(wxT("BeginFiniteBurn"));
      break;
   case POPUP_INSERT_BEFORE_END_FINITE_BURN:
      InsertBefore(wxT("EndFiniteBurn"));
      break;
   case POPUP_INSERT_BEFORE_TARGET:
      InsertBefore(wxT("Target"));
      break;
   case POPUP_INSERT_BEFORE_OPTIMIZE:
      InsertBefore(wxT("Optimize"));
      break;
   case POPUP_INSERT_BEFORE_VARY:
      InsertBefore(wxT("Vary"));
      break;
   case POPUP_INSERT_BEFORE_ACHIEVE:
      InsertBefore(wxT("Achieve"));
      break;
   case POPUP_INSERT_BEFORE_MINIMIZE:
      InsertBefore(wxT("Minimize"));
      break;
   case POPUP_INSERT_BEFORE_NON_LINEAR_CONSTRAINT:
      InsertBefore(wxT("NonlinearConstraint"));
      break;
   case POPUP_INSERT_BEFORE_CALL_GMAT_FUNCTION:
      InsertBefore(wxT("CallGmatFunction"));
      break;
   case POPUP_INSERT_BEFORE_CALL_MATLAB_FUNCTION:
      InsertBefore(wxT("CallMatlabFunction"));
      break;
   case POPUP_INSERT_BEFORE_ASSIGNMENT:
      //InsertBefore(wxT("GMAT"));
      InsertBefore(wxT("Equation"));
      break;
   case POPUP_INSERT_BEFORE_REPORT:
      InsertBefore(wxT("Report"));
      break;
   case POPUP_INSERT_BEFORE_TOGGLE:
      InsertBefore(wxT("Toggle"));
      break;
   case POPUP_INSERT_BEFORE_SAVE:
      InsertBefore(wxT("Save"));
      break;
   case POPUP_INSERT_BEFORE_STOP:
      InsertBefore(wxT("Stop"));
      break;
   case POPUP_INSERT_BEFORE_SCRIPT_EVENT:
      InsertBefore(wxT("BeginScript"));
      break;
   case POPUP_INSERT_BEFORE_IF:
      InsertBefore(wxT("If"));
      break;
   case POPUP_INSERT_BEFORE_IF_ELSE:
      InsertBefore(wxT("IfElse"));
      break;
   case POPUP_INSERT_BEFORE_ELSE:
      InsertBefore(wxT("Else"));
      break;
   case POPUP_INSERT_BEFORE_ELSE_IF:
      //InsertBefore(wxT("ElseIf"));
      break;
   case POPUP_INSERT_BEFORE_FOR:
      InsertBefore(wxT("For"));
      break;
   case POPUP_INSERT_BEFORE_WHILE:
      InsertBefore(wxT("While"));
      break;
   case POPUP_INSERT_BEFORE_D0_WHILE:
      //InsertBefore(wxT("Do"));
      break;
   case POPUP_INSERT_BEFORE_SWITCH:
      //InsertBefore(wxT("Switch"));
      break;
   default:
      break;
   }
}


//------------------------------------------------------------------------------
// void OnInsertAfter(wxCommandEvent &event)
//------------------------------------------------------------------------------
void MissionTree::OnInsertAfter(wxCommandEvent &event)
{
   switch (event.GetId())
   {
   case POPUP_INSERT_AFTER_PROPAGATE:
      InsertAfter(wxT("Propagate"));
      break;
   case POPUP_INSERT_AFTER_MANEUVER:
      InsertAfter(wxT("Maneuver"));
      break;
   case POPUP_INSERT_AFTER_BEGIN_FINITE_BURN:
      InsertAfter(wxT("BeginFiniteBurn"));
      break;
   case POPUP_INSERT_AFTER_END_FINITE_BURN:
      InsertAfter(wxT("EndFiniteBurn"));
      break;
   case POPUP_INSERT_AFTER_TARGET:
      InsertAfter(wxT("Target"));
      break;
   case POPUP_INSERT_AFTER_OPTIMIZE:
      InsertAfter(wxT("Optimize"));
      break;
   case POPUP_INSERT_AFTER_VARY:
      InsertAfter(wxT("Vary"));
      break;
   case POPUP_INSERT_AFTER_ACHIEVE:
      InsertAfter(wxT("Achieve"));
      break;
   case POPUP_INSERT_AFTER_MINIMIZE:
      InsertAfter(wxT("Minimize"));
      break;
   case POPUP_INSERT_AFTER_NON_LINEAR_CONSTRAINT:
      InsertAfter(wxT("NonlinearConstraint"));
      break;
   case POPUP_INSERT_AFTER_CALL_GMAT_FUNCTION:
      InsertAfter(wxT("CallGmatFunction"));
      break;
   case POPUP_INSERT_AFTER_CALL_MATLAB_FUNCTION:
      InsertAfter(wxT("CallMatlabFunction"));
      break;
   case POPUP_INSERT_AFTER_ASSIGNMENT:
      //InsertAfter(wxT("GMAT"));
      InsertAfter(wxT("Equation"));
      break;
   case POPUP_INSERT_AFTER_REPORT:
      InsertAfter(wxT("Report"));
      break;
   case POPUP_INSERT_AFTER_TOGGLE:
      InsertAfter(wxT("Toggle"));
      break;
   case POPUP_INSERT_AFTER_SAVE:
      InsertAfter(wxT("Save"));
      break;
   case POPUP_INSERT_AFTER_STOP:
      InsertAfter(wxT("Stop"));
      break;
   case POPUP_INSERT_AFTER_SCRIPT_EVENT:
      InsertAfter(wxT("BeginScript"));
      break;
   case POPUP_INSERT_AFTER_IF:
      InsertAfter(wxT("If"));
      break;
   case POPUP_INSERT_AFTER_IF_ELSE:
      InsertAfter(wxT("IfElse"));
      break;
   case POPUP_INSERT_AFTER_ELSE:
      InsertAfter(wxT("Else"));
      break;
   case POPUP_INSERT_AFTER_ELSE_IF:
      //InsertAfter(wxT("ElseIf"));
      break;
   case POPUP_INSERT_AFTER_FOR:
      InsertAfter(wxT("For"));
      break;
   case POPUP_INSERT_AFTER_WHILE:
      InsertAfter(wxT("While"));
      break;
   case POPUP_INSERT_AFTER_D0_WHILE:
      //InsertAfter(wxT("Do"));
      break;
   case POPUP_INSERT_AFTER_SWITCH:
      //InsertAfter(wxT("Switch"));
      break;
   default:
      break;
   }
}


//------------------------------------------------------------------------------
// void OnAutoAppend(wxCommandEvent &event)
//------------------------------------------------------------------------------
void MissionTree::OnAutoAppend(wxCommandEvent &event)
{
   #ifdef DEBUG_MISSION_TREE_APPEND
   MessageInterface::ShowMessage
      (wxT("=====> MissionTree::OnAutoAppend() entered, event id = %d\n"), event.GetId());
   #endif
   
   int menuId = event.GetId();   
   Append(idCmdMap[menuId]);
   
   #ifdef DEBUG_MISSION_TREE_APPEND
   MessageInterface::ShowMessage(wxT("=====> MissionTree::OnAutoAppend() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void OnAutoInsertBefore(wxCommandEvent &event)
//------------------------------------------------------------------------------
void MissionTree::OnAutoInsertBefore(wxCommandEvent &event)
{
   #ifdef DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage
      (wxT("MissionTree::OnAutoInsertBefore() entered, event id = %d\n"), event.GetId());
   #endif
   
   int menuId = event.GetId();   
   InsertBefore(idCmdMap[menuId]);
   
   #ifdef DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage(wxT("MissionTree::OnAutoInsertBefore() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void OnAutoInsertAfter(wxCommandEvent &event)
//------------------------------------------------------------------------------
void MissionTree::OnAutoInsertAfter(wxCommandEvent &event)
{
   #ifdef DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage
      (wxT("MissionTree::OnAutoInsertAfter() entered, event id = %d\n"), event.GetId());
   #endif
   
   wxString cmdString = event.GetString();
   int menuId = event.GetId();
   InsertAfter(idCmdMap[menuId]);
   
   #ifdef DEBUG_MISSION_TREE_INSERT
   MessageInterface::ShowMessage(wxT("MissionTree::OnAutoInsertAfter() leaving\n"));
   #endif
}


//---------------------------------
// Crete popup menu
//---------------------------------

//------------------------------------------------------------------------------
// wxMenu* CreateSubMenu(int type, ActionType action)
//------------------------------------------------------------------------------
/*
 * Creates popup menu. It will create proper menu id depends on the input
 * action.
 *
 * @param  type  Command type id
 * @param  action  One of APPEND, INSERT_BEFORE, INSERT_AFTER
 */
//------------------------------------------------------------------------------
wxMenu* MissionTree::CreateSubMenu(int type, ActionType action)
{
   #if DEBUG_MISSION_TREE_MENU
   MessageInterface::ShowMessage
      (wxT("MissionTree::CreateSubMenu() type=%d, action=%d\n"), type, action);
   #endif
   
   unsigned int i;
   wxMenu *menu = new wxMenu;
      
   for (i=0; i<mCommandList.GetCount(); i++)
      menu->Append(GetMenuId(mCommandList[i], action), mCommandList[i]);
   
   menu->Append(POPUP_CONTROL_LOGIC, wxT("Control Logic"),
                CreateControlLogicSubMenu(type, action));
   
   return menu;
}


//------------------------------------------------------------------------------
// wxMenu* CreateTargetSubMenu(int type, ActionType action)
//------------------------------------------------------------------------------
wxMenu* MissionTree::CreateTargetSubMenu(int type, ActionType action)
{
   wxMenu *menu;
   
   menu = CreateSubMenu(type, action);
   menu = AppendTargetSubMenu(menu, action);
   
   return menu;
}


//------------------------------------------------------------------------------
// wxMenu* CreateOptimizeSubMenu(int type, ActionType action)
//------------------------------------------------------------------------------
wxMenu* MissionTree::CreateOptimizeSubMenu(int type, ActionType action)
{
   wxMenu *menu;
   
   menu = CreateSubMenu(type, action);
   menu = AppendOptimizeSubMenu(menu, action);

   return menu;
}


//------------------------------------------------------------------------------
// wxMenu* AppendTargetSubMenu(wxMenu *menu, ActionType action)
//------------------------------------------------------------------------------
wxMenu* MissionTree::AppendTargetSubMenu(wxMenu *menu, ActionType action)
{
   switch (action)
   {
   case APPEND:
      menu->Append(POPUP_APPEND_VARY, wxT("Vary"));
      menu->Append(POPUP_APPEND_ACHIEVE, wxT("Achieve"));
      break;
      
   case INSERT_BEFORE:
      menu->Append(POPUP_INSERT_BEFORE_VARY, wxT("Vary"));
      menu->Append(POPUP_INSERT_BEFORE_ACHIEVE, wxT("Achieve"));
      break;
      
   case INSERT_AFTER:
      menu->Append(POPUP_INSERT_AFTER_VARY, wxT("Vary"));
      menu->Append(POPUP_INSERT_AFTER_ACHIEVE, wxT("Achieve"));
      break;
      
   default:
      break;
   }
   
   return menu;
}


//------------------------------------------------------------------------------
// wxMenu* AppendOptimizeSubMenu(wxMenu *menu, ActionType action)
//------------------------------------------------------------------------------
wxMenu* MissionTree::AppendOptimizeSubMenu(wxMenu *menu, ActionType action)
{
   switch (action)
   {
   case APPEND:
      menu->Append(POPUP_APPEND_VARY, wxT("Vary"));
      menu->Append(POPUP_APPEND_MINIMIZE, wxT("Minimize"));
      menu->Append(POPUP_APPEND_NON_LINEAR_CONSTRAINT, wxT("NonlinearConstraint"));
      break;
      
   case INSERT_BEFORE:
      menu->Append(POPUP_INSERT_BEFORE_VARY, wxT("Vary"));
      menu->Append(POPUP_INSERT_BEFORE_MINIMIZE, wxT("Minimize"));
      menu->Append(POPUP_INSERT_BEFORE_NON_LINEAR_CONSTRAINT, wxT("NonlinearConstraint"));
      break;
      
   case INSERT_AFTER:
      menu->Append(POPUP_INSERT_AFTER_VARY, wxT("Vary"));
      menu->Append(POPUP_INSERT_AFTER_MINIMIZE, wxT("Minimize"));
      menu->Append(POPUP_INSERT_AFTER_NON_LINEAR_CONSTRAINT, wxT("NonlinearConstraint"));
      break;
      
   default:
      break;
   }
   
   return menu;
}


//------------------------------------------------------------------------------
// wxMenu* CreateControlLogicSubMenu(int type, ActionType action)
//------------------------------------------------------------------------------
wxMenu* MissionTree::CreateControlLogicSubMenu(int type, ActionType action)
{
   #if DEBUG_MISSION_TREE_MENU
   MessageInterface::ShowMessage
      (wxT("MissionTree::CreateControlLogicMenu() type=%d, action=%d\n"),
       type, action);
   #endif
   
   wxMenu *menu = new wxMenu;
   bool addElse = false;
   
   if (type == GmatTree::IF_CONTROL || type == GmatTree::END_IF_CONTROL)
   {
      addElse = true;
      wxTreeItemId itemId = GetSelection();
      wxTreeItemId parentId = itemId;
      
      if (type == GmatTree::END_IF_CONTROL)
         parentId = GetItemParent(itemId);
      
      // show only one Else
      // We should look for in the first level children only,
      // so use FindElse() (LOJ: 2011.09.28)
      wxTreeItemId elseId = FindElse(parentId);
      if (elseId.IsOk() && GetItemText(elseId) != wxT(""))
         addElse = false;
   }
   
   switch (action)
   {
   case APPEND:
      menu->Append(POPUP_APPEND_IF, wxT("If"));
      menu->Append(POPUP_APPEND_IF_ELSE, wxT("If-Else"));
      
      if (addElse)
         menu->Append(POPUP_APPEND_ELSE, wxT("Else"));
      
      menu->Append(POPUP_APPEND_FOR, wxT("For"));
      menu->Append(POPUP_APPEND_WHILE, wxT("While")); 
      break;
      
   case INSERT_BEFORE:
      menu->Append(POPUP_INSERT_BEFORE_IF, wxT("If"));
      menu->Append(POPUP_INSERT_BEFORE_IF_ELSE, wxT("If-Else"));
      
      if (addElse)
         menu->Append(POPUP_INSERT_BEFORE_ELSE, wxT("Else"));
      
      menu->Append(POPUP_INSERT_BEFORE_FOR, wxT("For"));
      menu->Append(POPUP_INSERT_BEFORE_WHILE, wxT("While"));
      break;
      
   case INSERT_AFTER:
      menu->Append(POPUP_INSERT_AFTER_IF, wxT("If"));
      menu->Append(POPUP_INSERT_AFTER_IF_ELSE, wxT("If-Else"));
      
      if (addElse)
         menu->Append(POPUP_INSERT_AFTER_ELSE, wxT("Else"));
      
      menu->Append(POPUP_INSERT_AFTER_FOR, wxT("For"));
      menu->Append(POPUP_INSERT_AFTER_WHILE, wxT("While")); 
      break;
      
   default:
      break;
   }
   
   return menu;
}


//------------------------------------------------------------------------------
// void OnBeginEditLabel(wxTreeEvent& event)
//------------------------------------------------------------------------------
/**
 * Handles EVT_TREE_BEGIN_LABEL_EDIT for beginning editing a label.
 *
 * @param <event> input event.
 */
//------------------------------------------------------------------------------
void MissionTree::OnBeginEditLabel(wxTreeEvent& event)
{
   // if panel is currently opened give warning and veto
   wxTreeItemId itemId = GetSelection();
   GmatTreeItemData *selItem = (GmatTreeItemData *) GetItemData(itemId);
   if (theMainFrame->IsChildOpen(selItem))
   {
      wxLogWarning(selItem->GetTitle() + wxT(" cannot be renamed while panel is opened"));
      wxLog::FlushActive();
      event.Veto();
   }
}


//------------------------------------------------------------------------------
// void OnEndEditLabel(wxTreeEvent& event)
//------------------------------------------------------------------------------
/**
 * Handles EVT_TREE_END_LABEL_EDIT for ending editing a label.
 *
 * @param <event> input event.
 */
//------------------------------------------------------------------------------
void MissionTree::OnEndEditLabel(wxTreeEvent& event)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage(wxT("OnEndEditLabel() entered\n"));
   #endif
   
   wxString newLabel = event.GetLabel();
   wxTreeItemId itemId = event.GetItem();
   MissionTreeItemData *item = (MissionTreeItemData *)GetItemData(itemId);
   GmatCommand *cmd = item->GetCommand();
   
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("   old cmd name = '%s'\n"), cmd->GetName().c_str());
   #endif
   
   item->SetName(newLabel);
   item->SetTitle(newLabel);
   cmd->SetName(newLabel.c_str());
   cmd->SetSummaryName(newLabel.c_str());
   
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("   new cmd name = '%s'\n"), cmd->GetName().c_str());
   #endif
   
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage(wxT("OnEndEditLabel() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void OnRename()
//------------------------------------------------------------------------------
void MissionTree::OnRename(wxCommandEvent &event)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage(wxT("OnRename() entered\n"));
   #endif
   
   // get selected item
   wxTreeItemId itemId = GetSelection();
   GmatTreeItemData *selItem = (GmatTreeItemData *) GetItemData(itemId);
   MissionTreeItemData *item = (MissionTreeItemData *)selItem;
   GmatCommand *cmd = item->GetCommand();
   wxString cmdName = GetItemText(itemId);
   
   // if panel is currently opened give warning and return
   // Bug 547 fix (loj: 2008.11.25)
   if (theMainFrame->IsChildOpen(selItem))
   {
      wxLogWarning(selItem->GetTitle() + wxT(" cannot be renamed while panel is opened"));
      wxLog::FlushActive();
      return;
   }
   
   // Do we want to use rename dialog here?
   //=================================================================
   #if 1
   //=================================================================
   
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("   mLastClickPoint.x=%d, mLastClickPoint.y=%d\n"),
       mLastClickPoint.x, mLastClickPoint.y);
   #endif
   
   mLastClickPoint.y += 100;
   ViewTextDialog renameDlg(this, wxT("Rename"), true, mLastClickPoint,
                            wxSize(100, -1), wxDEFAULT_DIALOG_STYLE);
   renameDlg.AppendText(cmdName);
   renameDlg.ShowModal();
   
   if (renameDlg.HasTextChanged())
   {
      wxString newName = renameDlg.GetText();
      #ifdef DEBUG_RENAME
      MessageInterface::ShowMessage
         (wxT("  Setting command name to '%s'\n"), newName.c_str());
      #endif
      SetItemText(itemId, newName);
      item->SetName(newName);
      item->SetTitle(newName);
      cmd->SetName(newName.c_str());
      cmd->SetSummaryName(newName.c_str());
   }
   
   //=================================================================
   #else
   //=================================================================
   
   //@note
   // To enable this function, xTR_EDIT_LABELS style must be set when creating
   // MissionTree in GmatNotebook
   EditLabel(itemId);   
   
   //=================================================================
   #endif
   //=================================================================
   
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage(wxT("OnRename() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void OnDelete()
//------------------------------------------------------------------------------
void MissionTree::OnDelete(wxCommandEvent &event)
{
   // get selected item
   wxTreeItemId itemId = GetSelection();
   GmatTreeItemData *selItem = (GmatTreeItemData *) GetItemData(itemId);
   wxString cmdName = GetItemText(itemId);
   
   // if panel is currently opened give warning and return
   // Bug 547 fix (loj: 2008.11.25)
   if (theMainFrame->IsChildOpen(selItem))
   {
      wxLogWarning(selItem->GetTitle() + wxT(" cannot be deleted ")
                   wxT("while panel is opened"));
      wxLog::FlushActive();
      return;
   }
   
   DeleteCommand(cmdName);
   
}


//---------------------------------------------------------------------------
// void OnRun()
//--------------------------------------------------------------------------
void MissionTree::OnRun(wxCommandEvent &event)
{
   theGuiInterpreter->RunMission();
}


//---------------------------------------------------------------------------
// void OnShowDetail()
//--------------------------------------------------------------------------
void MissionTree::OnShowDetail(wxCommandEvent &event)
{
   mShowDetailedItem = event.IsChecked();
   UpdateMission(true);
}


//---------------------------------------------------------------------------
// void OnShowMissionSequence()
//--------------------------------------------------------------------------
void MissionTree::OnShowMissionSequence(wxCommandEvent &event)
{
   GmatCommand *cmd = theGuiInterpreter->GetFirstCommand();
   wxString str = GmatCommandUtil::GetCommandSeqString(cmd, false, false, wxT("   "));
   
   if (str == wxT(""))
      return;
   
   ViewTextFrame *vtf =
      new ViewTextFrame(theMainFrame, wxT("Show Mission Sequence"),
       50, 50, 800, 500, wxT("Temporary"), wxT("Mission Sequence"));
   
   vtf->AppendText(str);
   vtf->Show(true);
   
}

//---------------------------------------------------------------------------
// void OnShowScript()
//--------------------------------------------------------------------------
void MissionTree::OnShowScript(wxCommandEvent &event)
{
   wxString str = theGuiInterpreter->GetScript();
   
   if (str == wxT(""))
      return;
   
   ViewTextFrame *vtf =
      new ViewTextFrame(theMainFrame, wxT("Show Script"),
       50, 50, 800, 500, wxT("Temporary"), wxT("Script"));
   
   vtf->AppendText(str.c_str());
   vtf->Show(true);
   
}

//------------------------------------------------------------------------------
// void OnShowCommandSummary()
//------------------------------------------------------------------------------
void MissionTree::OnShowCommandSummary(wxCommandEvent &event)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage(wxT("OnShowCommandSummary() entered\n"));
   #endif
   
   // get selected item
   wxTreeItemId itemId = GetSelection();
   GmatTreeItemData *selItem = (GmatTreeItemData *) GetItemData(itemId);
   MissionTreeItemData *item = (MissionTreeItemData *)selItem;
   GmatCommand *cmd = item->GetCommand();
   
   // open window to show command summary
   if (cmd != NULL)
   {
      wxString title = wxT("Command Summary for ");
      if (cmd->GetName() != wxT(""))
         title += cmd->GetName().c_str();
      else
         title += cmd->GetTypeName().c_str();
      
      ShowSummaryDialog ssd(this, -1, title, cmd);
      ssd.ShowModal();
   }
   
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage(wxT("OnShowCommandSummary() leaving\n"));
   #endif
}


//---------------------------------------------------------------------------
// void MissionTree::OnShowMissionSummaryAll()
//--------------------------------------------------------------------------
void MissionTree::OnShowMissionSummaryAll(wxCommandEvent &event)
{
   wxString title = wxT("Mission Summary - All Commands");

   GmatCommand *firstCmd = theGuiInterpreter->GetFirstCommand();;
   if (firstCmd)
   {
      ShowSummaryDialog ssd(this, -1, title, firstCmd, true, false);
      ssd.ShowModal();
   }
   else
   {
      wxString errmsg = wxT("\'Mission Summary\' - unable to obtain pointer to first command.\n");
      MessageInterface::PopupMessage(Gmat::ERROR_, errmsg);
   }
}

//---------------------------------------------------------------------------
// void MissionTree::OnShowMissionSummaryPhysics()
//--------------------------------------------------------------------------
void MissionTree::OnShowMissionSummaryPhysics(wxCommandEvent &event)
{
   wxString title = wxT("Mission Summary - Physics-Based Commands");

   GmatCommand *firstCmd = theGuiInterpreter->GetFirstCommand();;
   if (firstCmd)
   {
      ShowSummaryDialog ssd(this, -1, title, firstCmd, true, true);
      ssd.ShowModal();
   }
   else
   {
      wxString errmsg = wxT("\'Mission Summary\' - unable to obtain pointer to first command.\n");
      MessageInterface::PopupMessage(Gmat::ERROR_, errmsg);
   }
}


//---------------------------------------------------------------------------
// void OnDockUndockMissionTree(wxCommandEvent &event)
//---------------------------------------------------------------------------
void MissionTree::OnDockUndockMissionTree(wxCommandEvent &event)
{
   if (event.GetId() == POPUP_DOCK_MISSION_TREE)
   {
      theMainFrame->CloseChild(wxT("Mission"), GmatTree::MISSION_TREE_UNDOCKED);
   }
   else if (event.GetId() == POPUP_UNDOCK_MISSION_TREE)
   {
      theNotebook->CreateUndockedMissionPanel();
   }
}


//---------------------------------------------------------------------------
// bool CheckClickIn(wxPoint position)
//--------------------------------------------------------------------------
bool MissionTree::CheckClickIn(wxPoint position)
{
   //MessageInterface::ShowMessage(wxT("Click position is %d %d\n"), position.x, position.y );
   //MissionTreeItemData *missionTreeItem = (MissionTreeItemData*) GetFirstVisibleItem();
   wxTreeItemId visibleItemId = GetFirstVisibleItem();
   MissionTreeItemData *missionTreeItemData = 
      (MissionTreeItemData*) GetItemData(visibleItemId);
   //MessageInterface::ShowMessage(wxT("Got first visible"));

   // loop through all the visible items on the mission tree
   // to compare the event click with the position of the box
   while (missionTreeItemData != NULL)
   {
      GmatTree::ItemType itemType = missionTreeItemData->GetItemType();
      // don't have to open any panels for top folders
      if ((itemType != GmatTree::MISSIONS_FOLDER)       &&
          (itemType != GmatTree::MISSION_SEQ_TOP_FOLDER)&&
          (itemType != GmatTree::MISSION_SEQ_SUB_FOLDER)&&
          (itemType != GmatTree::MISSION_SEQ_COMMAND))
      {
         // get the surrounding box to compare click and commands
         wxRect bound;
         int w, h;
       
         GetBoundingRect(visibleItemId, bound, TRUE);
         GetSize(&w, &h);
       
         // compare event click to see if it is in the box or the
         // icon which is size 16
         if ((position.x >= (bound.x - 16)) &&
             (position.x <= w-offset) &&
             (position.y <= bound.y+rowHeight+1) &&
             (position.y >= bound.y-1))
         {
            //MessageInterface::ShowMessage(wxT("\nInside a rect\n"));
              
            // set this item selected
            SelectItem(visibleItemId);
              
            // now that we know it is in a box, check to see
            // which box it is in
            // we only need to compare the left and the right, because
            // we already know it is within the top and the bottom
          
            // get box width
            int boxWidth = GetParameter(BOXWIDTH);
 
            // box count is 2, rightmost is for variables
            // next is goals, and the rest is the cmd panel
            int boxNum = 0;
          
            // check if in variables
            if ((position.x <= w-offset-boxWidth*boxNum) &&
                (position.x >= w-offset-boxWidth*(++boxNum)))
            {
               //MessageInterface::ShowMessage(wxT("\nInside variables"));
               MissionTreeItemData *item =
                  new MissionTreeItemData(wxT("Variables"),
                                          GmatTree::VIEW_SOLVER_VARIABLES);
               theMainFrame->CreateChild(item);
            }
            else if ((position.x <= w-offset-boxWidth*boxNum) &&
                     (position.x >= w-offset-boxWidth*(++boxNum)))
            {
               //MessageInterface::ShowMessage(wxT("\nInside goals"));
               MissionTreeItemData *item =
                  new MissionTreeItemData(wxT("Goals"),
                                          GmatTree::VIEW_SOLVER_GOALS);
               theMainFrame->CreateChild(item);
            }
            else
            {
               //MessageInterface::ShowMessage(wxT("\nOpen regular panel"));
               theMainFrame->CreateChild(missionTreeItemData);
            }
            
            // get out of while loop
            break;
         }
      }
      //MessageInterface::ShowMessage(wxT("Not equal to null"));
      visibleItemId = GetNextVisible(visibleItemId);
      //MessageInterface::ShowMessage(wxT("Got next visible id"));
      missionTreeItemData = (MissionTreeItemData*) GetItemData(visibleItemId);
      //MessageInterface::ShowMessage(wxT("Got next visible data"));
   }

   return false;
}


//------------------------------------------------------------------------------
// void OnCollapse(wxCommandEvent &event)
//------------------------------------------------------------------------------
/**
 * This method collapses MissionTree.
 */
//------------------------------------------------------------------------------
void MissionTree::OnCollapse(wxCommandEvent &event)
{
   // Get selected item
   GmatTreeItemData *currItem = (GmatTreeItemData *) GetItemData(GetSelection());
   wxTreeItemId selectId = currItem->GetId();
   wxTreeItemId currId = currItem->GetId();
   
   int numChildren = GetChildrenCount(currId);
   if (numChildren > 0)
   {
      wxTreeItemIdValue cookie;
      wxTreeItemId childId = GetFirstChild(currId, cookie);
      
      while (childId.IsOk())
      {
         Collapse(childId);
         childId = GetNextChild(currId, cookie);
      }
   }
   
   ScrollTo(selectId);
}


//------------------------------------------------------------------------------
// void OnExpand(wxCommandEvent &event)
//------------------------------------------------------------------------------
/**
 * This method expands MissionTree.
 */
//------------------------------------------------------------------------------
void MissionTree::OnExpand(wxCommandEvent &event)
{
   // Get selected item
   GmatTreeItemData *currItem = (GmatTreeItemData *) GetItemData(GetSelection());
   wxTreeItemId currId = currItem->GetId();
   
   ExpandAll();
   ScrollTo(currId);
}


//------------------------------------------------------------------------------
// void OnOpen(wxCommandEvent &event)
//------------------------------------------------------------------------------
/**
 * Open chosen from popup menu
 */
//------------------------------------------------------------------------------
void MissionTree::OnOpen(wxCommandEvent &event)
{
   // Get info from selected item
   GmatTreeItemData *item = (GmatTreeItemData *) GetItemData(GetSelection());
   theMainFrame->CreateChild(item);
}


//------------------------------------------------------------------------------
// void OnClose()
//------------------------------------------------------------------------------
/**
 * Close chosen from popup menu
 */
//------------------------------------------------------------------------------
void MissionTree::OnClose(wxCommandEvent &event)
{
   // Get info from selected item
   GmatTreeItemData *currItem = (GmatTreeItemData *) GetItemData(GetSelection());
   wxTreeItemId currId = currItem->GetId();
   
   int numChildren = GetChildrenCount(currId);
   if (numChildren > 0)
   {
      wxTreeItemIdValue cookie;
      MissionTreeItemData *item;
      wxTreeItemId childId = GetFirstChild(currId, cookie);
      
      while (childId.IsOk())
      {
         item = (MissionTreeItemData *)GetItemData(childId);
         
         #if DEBUG_MISSION_TREE_DELETE
         MessageInterface::ShowMessage
            (wxT("MissionTree::OnClose() while-loop, item->GetTitle(): \"%s\"\n"),
             item->GetTitle().c_str());
         #endif
         
         if (theMainFrame->IsChildOpen(item))
            theMainFrame->CloseActiveChild();
         
         childId = GetNextChild(currId, cookie);
         
         #if DEBUG_MISSION_TREE_DELETE
         MessageInterface::ShowMessage
            (wxT("MissionTree::OnClose() childId=<%s>\n"), GetItemText(childId).c_str());
         #endif
      }
   }
   
   
   // delete selected item panel, if its open, its activated
   if (theMainFrame->IsChildOpen(currItem))
      theMainFrame->CloseActiveChild();
   else
      return;
   
}


//------------------------------------------------------------------------------
// GmatTree::MissionIconType GetIconId(const wxString &cmd)
//------------------------------------------------------------------------------
GmatTree::MissionIconType MissionTree::GetIconId(const wxString &cmd)
{
   if (cmd == wxT("Propagate"))
      return GmatTree::MISSION_ICON_PROPAGATE;
   if (cmd == wxT("Maneuver"))
      return GmatTree::MISSION_ICON_DELTA_V;
   if (cmd == wxT("BeginFiniteBurn"))
      return GmatTree::MISSION_ICON_BEGIN_FB;
   if (cmd == wxT("EndFiniteBurn"))
      return GmatTree::MISSION_ICON_END_FB;
   if (cmd == wxT("Target"))
      return GmatTree::MISSION_ICON_TARGET;
   if (cmd == wxT("EndTarget"))
      return GmatTree::MISSION_ICON_NEST_RETURN;
   if (cmd == wxT("Optimize"))
      return GmatTree::MISSION_ICON_OPTIMIZE;
   if (cmd == wxT("EndOptimize"))
      return GmatTree::MISSION_ICON_NEST_RETURN;
   if (cmd == wxT("Achieve"))
      return GmatTree::MISSION_ICON_ACHIEVE;
   if (cmd == wxT("Minimize"))
      return GmatTree::MISSION_ICON_MINIMIZE;
   if (cmd == wxT("NonlinearConstraint"))
      return GmatTree::MISSION_ICON_NONLINEAR_CONSTRAINT;
   if (cmd == wxT("Vary"))
      return GmatTree::MISSION_ICON_VARY;
   if (cmd == wxT("Save"))
      return GmatTree::MISSION_ICON_SAVE;
   if (cmd == wxT("GMAT"))
      return GmatTree::MISSION_ICON_ASSIGNMENT;
   if (cmd == wxT("Equation"))
      return GmatTree::MISSION_ICON_ASSIGNMENT;
   if (cmd == wxT("Report"))
      return GmatTree::MISSION_ICON_REPORT;
   if (cmd == wxT("Toggle"))
      return GmatTree::MISSION_ICON_TOGGLE;
   if (cmd == wxT("For"))
      return GmatTree::MISSION_ICON_FOR;
   if (cmd == wxT("EndFor"))
      return GmatTree::MISSION_ICON_NEST_RETURN;
   if (cmd == wxT("If"))
      return GmatTree::MISSION_ICON_IF;
   if (cmd == wxT("IfElse"))
      return GmatTree::MISSION_ICON_IF;
   if (cmd == wxT("Else"))
      return GmatTree::MISSION_ICON_IF;
   if (cmd == wxT("EndIf"))
      return GmatTree::MISSION_ICON_NEST_RETURN;
   if (cmd == wxT("While"))
      return GmatTree::MISSION_ICON_WHILE;
   if (cmd == wxT("EndWhile"))
      return GmatTree::MISSION_ICON_NEST_RETURN;
   if (cmd == wxT("CallGmatFunction"))
      return GmatTree::MISSION_ICON_CALL_FUNCTION;
   if (cmd == wxT("CallMatlabFunction"))
      return GmatTree::MISSION_ICON_CALL_FUNCTION;
   if (cmd == wxT("Stop"))
      return GmatTree::MISSION_ICON_STOP;
   if (cmd == wxT("BeginScript"))
      return GmatTree::MISSION_ICON_SCRIPTEVENT;
   if (cmd == wxT("ScriptEvent"))
      return GmatTree::MISSION_ICON_SCRIPTEVENT;
   if (cmd == wxT("PenUp"))
      return GmatTree::MISSION_ICON_PEN_UP;
   if (cmd == wxT("PenDown"))
      return GmatTree::MISSION_ICON_PEN_DOWN;
   if (cmd == wxT("MarkPoint"))
      return GmatTree::MISSION_ICON_MARK_POINT;
   if (cmd == wxT("ClearPlot"))
      return GmatTree::MISSION_ICON_CLEAR_PLOT;
   if (cmd == wxT("Global"))
      return GmatTree::MISSION_ICON_GLOBAL;
   if (cmd == wxT("SaveMission"))
      return GmatTree::MISSION_ICON_SAVE_MISSION;
   if (cmd == wxT("Minimize"))
      return GmatTree::MISSION_ICON_MINIMIZE;
   if (cmd == wxT("NonlinearConstraint"))
      return GmatTree::MISSION_ICON_NONLINEAR_CONSTRAINT;
   if (cmd == wxT("RunSimulator"))
      return GmatTree::MISSION_ICON_RUN_SIMULATOR;
   if (cmd == wxT("RunEstimator"))
      return GmatTree::MISSION_ICON_RUN_ESTIMATOR;
   
   return GmatTree::MISSION_ICON_DEFAULT;
}


//------------------------------------------------------------------------------
// wxString GetCommandString(GmatCommand *cmd, const wxString &currStr)
//------------------------------------------------------------------------------
/*
 * Returns command string if command is not a BranchCommand or Begin/EndScript.
 */
//------------------------------------------------------------------------------
wxString MissionTree::GetCommandString(GmatCommand *cmd, const wxString &currStr)
{
   if (!mShowDetailedItem)
      return currStr;
   
   if (cmd->GetTypeName() == wxT("BeginScript") || cmd->GetTypeName() == wxT("EndScript"))
      return currStr;
   
   wxString cmdString;
   cmdString = cmd->GetGeneratingString(Gmat::NO_COMMENTS).c_str();
   
   #ifdef DEBUG_CMD_STRING
   MessageInterface::ShowMessage(wxT("GetCommandString() cmdString='%s'\n"), cmdString.c_str());
   #endif
   
   if (cmdString == wxT(";"))
      return currStr;
   else
      return cmdString;
}


//------------------------------------------------------------------------------
// GmatTree::ItemType GetCommandId(const wxString &cmd)
//------------------------------------------------------------------------------
GmatTree::ItemType MissionTree::GetCommandId(const wxString &cmd)
{
   if (cmd == wxT("Propagate"))
      return GmatTree::PROPAGATE;
   if (cmd == wxT("Maneuver"))
      return GmatTree::MANEUVER;
   if (cmd == wxT("BeginFiniteBurn"))
      return GmatTree::BEGIN_FINITE_BURN;
   if (cmd == wxT("EndFiniteBurn"))
      return GmatTree::END_FINITE_BURN;
   if (cmd == wxT("Target"))
      return GmatTree::TARGET;
   if (cmd == wxT("EndTarget"))
      return GmatTree::END_TARGET;
   if (cmd == wxT("Optimize"))
      return GmatTree::OPTIMIZE;
   if (cmd == wxT("EndOptimize"))
      return GmatTree::END_OPTIMIZE;
   if (cmd == wxT("Achieve"))
      return GmatTree::ACHIEVE;
   if (cmd == wxT("Minimize"))
      return GmatTree::MINIMIZE;
   if (cmd == wxT("NonlinearConstraint"))
      return GmatTree::NON_LINEAR_CONSTRAINT;
   if (cmd == wxT("Vary"))
      return GmatTree::VARY;
   if (cmd == wxT("Save"))
      return GmatTree::SAVE;
   if (cmd == wxT("Report"))
      return GmatTree::REPORT;
   if (cmd == wxT("For"))
      return GmatTree::FOR_CONTROL;
   if (cmd == wxT("EndFor"))
      return GmatTree::END_FOR_CONTROL;
   if (cmd == wxT("If"))
      return GmatTree::IF_CONTROL;
   if (cmd == wxT("IfElse"))
      return GmatTree::IF_CONTROL;
   if (cmd == wxT("Else"))
      return GmatTree::ELSE_CONTROL;
   if (cmd == wxT("EndIf"))
      return GmatTree::END_IF_CONTROL;
   if (cmd == wxT("While"))
      return GmatTree::WHILE_CONTROL;
   if (cmd == wxT("EndWhile"))
      return GmatTree::END_WHILE_CONTROL;
   if (cmd == wxT("CallGmatFunction"))
      return GmatTree::CALL_FUNCTION;
   if (cmd == wxT("CallMatlabFunction"))
      return GmatTree::CALL_FUNCTION;
   if (cmd == wxT("Stop"))
      return GmatTree::STOP;
   if (cmd == wxT("GMAT"))
      return GmatTree::ASSIGNMENT;
   if (cmd == wxT("Equation"))
      return GmatTree::ASSIGNMENT;
   if (cmd == wxT("BeginScript"))
      return GmatTree::SCRIPT_EVENT;
   if (cmd == wxT("MarkPoint") || cmd == wxT("ClearPlot"))
      return GmatTree::XY_PLOT_ACTION;
   if (cmd == wxT("PenUp") || cmd == wxT("PenDown"))
      return GmatTree::PLOT_ACTION;
   if (cmd == wxT("Toggle"))
      return GmatTree::TOGGLE;
   
   return GmatTree::OTHER_COMMAND;
}


//------------------------------------------------------------------------------
// void CreateCommandIdMap()
//------------------------------------------------------------------------------
void MissionTree::CreateCommandIdMap()
{
   wxString cmd;
   int cmdIndex = 0;
   
   for (unsigned int i=0; i<mCommandList.size(); i++)
   {
      cmd = mCommandList[i];
      CreateMenuIds(cmd, i);
      cmdIndex++;
   }
}


//------------------------------------------------------------------------------
// void CreateMenuIds(const wxString &cmd, int index)
//------------------------------------------------------------------------------
void MissionTree::CreateMenuIds(const wxString &cmd, int index)
{
   int id;
   wxString str, realCmd;
   realCmd = cmd;
   
   // If command to show is ScriptEvent, we want to create BeginScript
   if (cmd == wxT("ScriptEvent"))
      realCmd = wxT("BeginScript");
   
   // Create id for append
   str = wxT("AP*") + cmd;
   id = index + AUTO_APPEND_COMMAND + 1;
   cmdIdMap.insert(std::make_pair(str, id));
   idCmdMap.insert(std::make_pair(id, realCmd));
   
   // Create id for insert before
   str = wxT("IB*") + cmd;
   id = index + AUTO_INSERT_BEFORE_COMMAND + 1;
   cmdIdMap.insert(std::make_pair(str, id));
   idCmdMap.insert(std::make_pair(id, realCmd));
   
   // Create id for insert after
   str = wxT("IA*") + cmd;
   id = index + AUTO_INSERT_AFTER_COMMAND + 1;
   cmdIdMap.insert(std::make_pair(str, id));
   idCmdMap.insert(std::make_pair(id, realCmd));
}


//------------------------------------------------------------------------------
// int GetMenuId(const wxString &cmd, ActionType action)
//------------------------------------------------------------------------------
int MissionTree::GetMenuId(const wxString &cmd, ActionType action)
{
   #if DEBUG_MISSION_TREE_MENU
   MessageInterface::ShowMessage
      (wxT("MissionTree::GetMenuId() cmd='%s', action=%d\n"), cmd.c_str(), action);
   #endif
   
   int id = -1;
   
   //-----------------------------------------------------------------
   #ifdef __AUTO_ADD_NEW_COMMANDS__
   //-----------------------------------------------------------------
   wxString cmdStr = cmd;
   
   // Add prefix to command string
   if (action == APPEND)
      cmdStr = wxT("AP*") + cmdStr;
   else if (action == INSERT_BEFORE)
      cmdStr = wxT("IB*") + cmdStr;
   else if (action == INSERT_AFTER)
      cmdStr = wxT("IA*") + cmdStr;
   
   // check if command string is valid
   if (cmdIdMap.find(cmdStr) == cmdIdMap.end())
   {
      #if DEBUG_MISSION_TREE_MENU
      MessageInterface::ShowMessage
         (wxT("MissionTree::GetMenuId() The '%s' is not recognized command\n"), cmdStr.c_str());
      #endif
      return id;
   }
   
   id = cmdIdMap[cmdStr];
   
   #if DEBUG_MISSION_TREE_MENU
   MessageInterface::ShowMessage(wxT("MissionTree::GetMenuId() returning %d\n"), id);
   #endif
   
   return id;
   
   //-----------------------------------------------------------------
   #else
   //-----------------------------------------------------------------
   
   for (unsigned int i=0; i<mCommandList.Count(); i++)
   {
      if (action == APPEND)
      {
         if (cmd == wxT("Propagate"))
            return POPUP_APPEND_PROPAGATE;
         else if (cmd == wxT("Maneuver"))
            return POPUP_APPEND_MANEUVER;
         else if (cmd == wxT("BeginFiniteBurn"))
            return POPUP_APPEND_BEGIN_FINITE_BURN;
         else if (cmd == wxT("EndFiniteBurn"))
            return POPUP_APPEND_END_FINITE_BURN;
         else if (cmd == wxT("Target"))
            return POPUP_APPEND_TARGET;
         else if (cmd == wxT("Optimize"))
            return POPUP_APPEND_OPTIMIZE;
         else if (cmd == wxT("CallGmatFunction"))
            return POPUP_APPEND_CALL_GMAT_FUNCTION;
         else if (cmd == wxT("CallMatlabFunction"))
            return POPUP_APPEND_CALL_MATLAB_FUNCTION;
         else if (cmd == wxT("GMAT"))
            return POPUP_APPEND_ASSIGNMENT;
         else if (cmd == wxT("Equation"))
            return POPUP_APPEND_ASSIGNMENT;
         else if (cmd == wxT("Report"))
            return POPUP_APPEND_REPORT;
         else if (cmd == wxT("Toggle"))
            return POPUP_APPEND_TOGGLE;
         else if (cmd == wxT("Save"))
            return POPUP_APPEND_SAVE;
         else if (cmd == wxT("Stop"))
            return POPUP_APPEND_STOP;
         else if (cmd == wxT("ScriptEvent"))
            return POPUP_APPEND_SCRIPT_EVENT;
         else
         {
            MessageInterface::ShowMessage
               (wxT("MissionTree::GetMenuId() Unknown append command: '%s'\n"), cmd.c_str());
            return POPUP_APPEND_UNKNOWN;
         }
      }
      else if (action == INSERT_BEFORE)
      {
         if (cmd == wxT("Propagate"))
            return POPUP_INSERT_BEFORE_PROPAGATE;
         else if (cmd == wxT("Maneuver"))
            return POPUP_INSERT_BEFORE_MANEUVER;
         else if (cmd == wxT("BeginFiniteBurn"))
            return POPUP_INSERT_BEFORE_BEGIN_FINITE_BURN;
         else if (cmd == wxT("EndFiniteBurn"))
            return POPUP_INSERT_BEFORE_END_FINITE_BURN;
         else if (cmd == wxT("Target"))
            return POPUP_INSERT_BEFORE_TARGET;
         else if (cmd == wxT("Optimize"))
            return POPUP_INSERT_BEFORE_OPTIMIZE;
         else if (cmd == wxT("CallGmatFunction"))
            return POPUP_INSERT_BEFORE_CALL_GMAT_FUNCTION;
         else if (cmd == wxT("CallMatlabFunction"))
            return POPUP_INSERT_BEFORE_CALL_MATLAB_FUNCTION;
         else if (cmd == wxT("GMAT"))
            return POPUP_INSERT_BEFORE_ASSIGNMENT;
         else if (cmd == wxT("Equation"))
            return POPUP_INSERT_BEFORE_ASSIGNMENT;
         else if (cmd == wxT("Report"))
            return POPUP_INSERT_BEFORE_REPORT;
         else if (cmd == wxT("Toggle"))
            return POPUP_INSERT_BEFORE_TOGGLE;
         else if (cmd == wxT("Save"))
            return POPUP_INSERT_BEFORE_SAVE;
         else if (cmd == wxT("Stop"))
            return POPUP_INSERT_BEFORE_STOP;
         else if (cmd == wxT("ScriptEvent"))
            return POPUP_INSERT_BEFORE_SCRIPT_EVENT;
         else
         {
            MessageInterface::ShowMessage
               (wxT("MissionTree::GetMenuId() Unknown command:'%s'\n"), cmd.c_str());
            return POPUP_INSERT_BEFORE_UNKNOWN;
         }
      }
      else if (action == INSERT_AFTER)
      {
         if (cmd == wxT("Propagate"))
            return POPUP_INSERT_AFTER_PROPAGATE;
         else if (cmd == wxT("Maneuver"))
            return POPUP_INSERT_AFTER_MANEUVER;
         else if (cmd == wxT("BeginFiniteBurn"))
            return POPUP_INSERT_AFTER_BEGIN_FINITE_BURN;
         else if (cmd == wxT("EndFiniteBurn"))
            return POPUP_INSERT_AFTER_END_FINITE_BURN;
         else if (cmd == wxT("Target"))
            return POPUP_INSERT_AFTER_TARGET;
         else if (cmd == wxT("Optimize"))
            return POPUP_INSERT_AFTER_OPTIMIZE;
         else if (cmd == wxT("CallGmatFunction"))
            return POPUP_INSERT_AFTER_CALL_GMAT_FUNCTION;
         else if (cmd == wxT("CallMatlabFunction"))
            return POPUP_INSERT_AFTER_CALL_MATLAB_FUNCTION;
         else if (cmd == wxT("GMAT"))
            return POPUP_INSERT_AFTER_ASSIGNMENT;
         else if (cmd == wxT("Equation"))
            return POPUP_INSERT_AFTER_ASSIGNMENT;
         else if (cmd == wxT("Report"))
            return POPUP_INSERT_AFTER_REPORT;
         else if (cmd == wxT("Toggle"))
            return POPUP_INSERT_AFTER_TOGGLE;
         else if (cmd == wxT("Save"))
            return POPUP_INSERT_AFTER_SAVE;
         else if (cmd == wxT("Stop"))
            return POPUP_INSERT_AFTER_STOP;
         else if (cmd == wxT("ScriptEvent"))
            return POPUP_INSERT_AFTER_SCRIPT_EVENT;
         else
         {
            MessageInterface::ShowMessage
               (wxT("MissionTree::GetMenuId() Unknown command:'%s'\n"), cmd.c_str());
            return POPUP_INSERT_AFTER_UNKNOWN;
         }
      }
   }
   
   return id;
   
   //-----------------------------------------------------------------
   #endif
   //-----------------------------------------------------------------
   
}


//------------------------------------------------------------------------------
// int* GetCommandCounter(const wxString &cmd)
//------------------------------------------------------------------------------
int* MissionTree::GetCommandCounter(const wxString &cmd)
{
   if (cmd == wxT("Propagate"))
      return &mNumPropagate;
   if (cmd == wxT("Maneuver"))
      return &mNumManeuver;
   if (cmd == wxT("BeginFiniteBurn"))
      return &mNumFiniteBurn;
   if (cmd == wxT("Target"))
      return &mNumTarget;
   if (cmd == wxT("EndTarget"))
      return &mNumTarget;
   if (cmd == wxT("Optimize"))
      return &mNumOptimize;
   if (cmd == wxT("EndOptimize"))
      return &mNumOptimize;
   if (cmd == wxT("Achieve"))
      return &mNumAchieve;
   if (cmd == wxT("Vary"))
      return &mNumVary;
   if (cmd == wxT("Save"))
      return &mNumSave;
   if (cmd == wxT("Toggle"))
      return &mNumToggle;
   if (cmd == wxT("ClearPlot"))
      return &mNumClearPlot;
   if (cmd == wxT("MarkPoint"))
      return &mNumMarkPoint;
   if (cmd == wxT("PenUp"))
      return &mNumPenUp;
   if (cmd == wxT("PenDown"))
      return &mNumPenDown;
   if (cmd == wxT("Report"))
      return &mNumReport;
   if (cmd == wxT("For"))
      return &mNumForLoop;
   if (cmd == wxT("EndFor"))
      return &mNumForLoop;
   if (cmd == wxT("If"))
      return &mNumIfStatement;
   if (cmd == wxT("IfElse"))
      return &mNumIfStatement;
   if (cmd == wxT("Else"))
      return &mNumIfStatement;
   if (cmd == wxT("EndIf"))
      return &mNumIfStatement;
   if (cmd == wxT("While"))
      return &mNumWhileLoop;
   if (cmd == wxT("EndWhile"))
      return &mNumWhileLoop;
   if (cmd == wxT("CallGmatFunction"))
      return &mNumFunct;
   if (cmd == wxT("CallMatlabFunction"))
      return &mNumFunct;
   if (cmd == wxT("GMAT"))
      return &mNumAssign;
   if (cmd == wxT("Equation"))
      return &mNumAssign;
   if (cmd == wxT("Stop"))
      return &mNumStop;
   if (cmd == wxT("Minimize"))
      return &mNumMinimize;
   if (cmd == wxT("NonlinearConstraint"))
      return &mNumNonlinearConstraint;
   if (cmd == wxT("BeginScript"))
      return &mNumScriptEvent;
   if (cmd == wxT("ScriptEvent"))
      return &mNumScriptEvent;
   
   return &mTempCounter;
}


//------------------------------------------------------------------------------
// wxTreeItemId FindChild(wxTreeItemId parentId, const wxString &cmd)
//------------------------------------------------------------------------------
/*
 * Finds a item from the parent node of the tree. It compares item command name
 * and cmd for finding cmd.
 *
 * @param <parentId> Parent item id
 * @param <cmd> Comand string to find
 *
 */
//------------------------------------------------------------------------------
wxTreeItemId MissionTree::FindChild(wxTreeItemId parentId, const wxString &cmd)
{
   #if DEBUG_MISSION_TREE_FIND
   MessageInterface::ShowMessage
      (wxT("\nMissionTree::FindChild() parentId=<%s>, cmd=<%s>\n"),
       GetItemText(parentId).c_str(), cmd.c_str());
   #endif
   
   int numChildren = GetChildrenCount(parentId);
   wxTreeItemId childId;
   wxString childText;
   
   if (numChildren > 0)
   {
      wxTreeItemIdValue cookie;
      childId = GetFirstChild(parentId, cookie);
      
      while (childId.IsOk())
      {
         MissionTreeItemData *currItem = (MissionTreeItemData *)GetItemData(childId);
         GmatCommand *currCmd = currItem->GetCommand();
         wxString currCmdType = currCmd->GetTypeName().c_str();
         wxString currCmdName = currCmd->GetName().c_str();
         childText = GetItemText(childId);
         
         #if DEBUG_MISSION_TREE_FIND > 1
         MessageInterface::ShowMessage(wxT("---> childText   ='%s'\n"), childText.c_str());
         MessageInterface::ShowMessage(wxT("     cmdTypeName ='%s'\n"), currCmdType.c_str());
         MessageInterface::ShowMessage(wxT("     cmdName     ='%s'\n"), currCmdType.c_str());
         #endif
         
         if (currCmdName == cmd)
            break;
         
         if (GetChildrenCount(childId) > 0)
            FindChild(childId, cmd);
         
         childId = GetNextChild(parentId, cookie);
      }
   }
   
   return childId;
}


//------------------------------------------------------------------------------
// wxTreeItemId FindElse(wxTreeItemId parentId)
//------------------------------------------------------------------------------
/*
 * Finds a Else from the children of the parent node of the tree. It checks
 * only one level.
 *
 * @param <parentId> Parent item id
 *
 * @return  id of Else node
 * 
 */
//------------------------------------------------------------------------------
wxTreeItemId MissionTree::FindElse(wxTreeItemId parentId)
{
   #if DEBUG_MISSION_TREE_FIND
   MessageInterface::ShowMessage
      (wxT("\nMissionTree::FindElse() parentId=<%s>\n"), GetItemText(parentId).c_str());
   #endif
   
   int numChildren = GetChildrenCount(parentId);
   wxTreeItemId childId;
   
   if (numChildren > 0)
   {
      wxTreeItemIdValue cookie;
      childId = GetFirstChild(parentId, cookie);
      
      while (childId.IsOk())
      {
         MissionTreeItemData *currItem = (MissionTreeItemData *)GetItemData(childId);
         GmatCommand *currCmd = currItem->GetCommand();
         wxString currCmdType = currCmd->GetTypeName().c_str();
         if (currCmdType == wxT("Else"))
            break;
         
         childId = GetNextChild(parentId, cookie);
      }
   }
   
   return childId;
}


//------------------------------------------------------------------------------
// bool IsInsideSolver(wxTreeItemId itemId, GmatTree::ItemType &itemType)
//------------------------------------------------------------------------------
/*
 * Checks if an item is inside of solver (Target, Optimize) branch.
 *
 */
//------------------------------------------------------------------------------
bool MissionTree::IsInsideSolver(wxTreeItemId itemId, GmatTree::ItemType &itemType)
{
   #if DEBUG_FIND_ITEM_PARENT
   MessageInterface::ShowMessage
      (wxT("MissionTree::IsInsideSolver() itemId='%s'\n"), GetItemText(itemId).c_str());
   #endif
   
   wxTreeItemId parentId = GetItemParent(itemId);
   MissionTreeItemData *parentItem;
   GmatTree::ItemType parentType;
   
   // go through parents
   while (parentId.IsOk() && GetItemText(parentId) != wxT(""))
   {
      #if DEBUG_FIND_ITEM_PARENT > 1
      MessageInterface::ShowMessage(wxT("   parent='%s'\n"), GetItemText(parentId).c_str());
      #endif
      
      parentItem = (MissionTreeItemData *)GetItemData(parentId);
      parentType = parentItem->GetItemType();
      
      if (parentType == GmatTree::TARGET || parentType == GmatTree::OPTIMIZE)
      {
         #if DEBUG_FIND_ITEM_PARENT
         MessageInterface::ShowMessage
            (wxT("MissionTree::IsInsideSolver() returning true, parent='%s'\n"),
             GetItemText(parentId).c_str());
         #endif
         
         itemType = parentType;
         return true;
      }
      
      parentId = GetItemParent(parentId);
   }
   
   #if DEBUG_FIND_ITEM_PARENT
   MessageInterface::ShowMessage(wxT("MissionTree::IsInsideSolver() returning false\n"));
   #endif
   
   return false;
}


// for Debug
//------------------------------------------------------------------------------
// void ShowCommands(const wxString &msg = wxT(""))
//------------------------------------------------------------------------------
void MissionTree::ShowCommands(const wxString &msg)
{
   MessageInterface::ShowMessage(wxT("-------------------->%s\n"), msg.c_str());
   
   GmatCommand *cmd = theGuiInterpreter->GetFirstCommand();;
   
   while (cmd != NULL)
   {
      #ifdef DEBUG_MISSION_TREE_SHOW_CMD
      MessageInterface::ShowMessage
         (wxT("----- <%p> %s '%s' (%s)\n"), cmd, cmd->GetTypeName().c_str(),
          cmd->GetName().c_str(), cmd->GetSummaryName().c_str());
      #else
      MessageInterface::ShowMessage
         (wxT("----- %s '%s' (%s)\n"), cmd->GetTypeName().c_str(),
          cmd->GetName().c_str(), cmd->GetSummaryName().c_str());
      #endif
      
      if ((cmd->GetChildCommand(0)) != NULL)
         ShowSubCommands(cmd, 0);
      
      cmd = cmd->GetNext();
   }
   
   MessageInterface::ShowMessage(wxT("<--------------------\n"));
}


//------------------------------------------------------------------------------
// void ShowSubCommands(GmatCommand* brCmd, Integer level)
//------------------------------------------------------------------------------
void MissionTree::ShowSubCommands(GmatCommand* brCmd, Integer level)
{
   GmatCommand* current = brCmd;
   Integer childNo = 0;
   GmatCommand* nextInBranch;
   GmatCommand* child;
   
   while((child = current->GetChildCommand(childNo)) != NULL)
   {
      nextInBranch = child;
      while ((nextInBranch != NULL) && (nextInBranch != current))
      {
         for (int i=0; i<=level; i++)
            MessageInterface::ShowMessage(wxT("-----"));
         
         #ifdef DEBUG_MISSION_TREE_SHOW_CMD
         MessageInterface::ShowMessage
            (wxT("----- <%p> %s '%s' (%s)\n"), nextInBranch, nextInBranch->GetTypeName().c_str(),
             nextInBranch->GetName().c_str(), nextInBranch->GetSummaryName().c_str());
         #else
         MessageInterface::ShowMessage
            (wxT("----- %s '%s' (%s)\n"), nextInBranch->GetTypeName().c_str(),
             nextInBranch->GetName().c_str(), nextInBranch->GetSummaryName().c_str());
         #endif
         
         if (nextInBranch->GetChildCommand() != NULL)
            ShowSubCommands(nextInBranch, level+1);
         
         nextInBranch = nextInBranch->GetNext();
      }
      
      ++childNo;
   }
   
}


//------------------------------------------------------------------------------
// void WriteCommand(const wxString &prefix = wxT(""),
//                   const wxString &title1, GmatCommand *cmd1,
//                   const wxString &title2 = wxT(""), GmatCommand *cmd2 = NULL)
//------------------------------------------------------------------------------
/*
 * Writes command info to message window.
 */
//------------------------------------------------------------------------------
void MissionTree::WriteCommand(const wxString &prefix,
                               const wxString &title1, GmatCommand *cmd1,
                               const wxString &title2, GmatCommand *cmd2)
{
   if (title2 == wxT(""))
   {
      if (cmd1 == NULL)
         MessageInterface::ShowMessage
            (wxT("%s%sNULL<%p>'%s'\n"), prefix.c_str(), title1.c_str(), cmd1,
				 cmd1->GetSummaryName().c_str());
      else
         MessageInterface::ShowMessage
            (wxT("%s%s%s<%p>'%s'\n"), prefix.c_str(), title1.c_str(),
             cmd1->GetTypeName().c_str(), cmd1, cmd1->GetSummaryName().c_str());
   }
   else
   {
      if (cmd1 == NULL)
         MessageInterface::ShowMessage
            (wxT("%s%sNULL<%p>'%s'%s%s<%p>'%s'\n"), prefix.c_str(), title1.c_str(),
             cmd1, cmd1->GetSummaryName().c_str(), title2.c_str(),
				 cmd2->GetTypeName().c_str(), cmd2, cmd2->GetSummaryName().c_str());
      else if (cmd2 == NULL)
         MessageInterface::ShowMessage
            (wxT("%s%s%s<%p>'%s'%sNULL<%p>'%s'\n"), prefix.c_str(), title1.c_str(),
             cmd1->GetTypeName().c_str(), cmd1, cmd1->GetSummaryName().c_str(),
				 title2.c_str(), cmd2, cmd2->GetSummaryName().c_str());
      else
         MessageInterface::ShowMessage
            (wxT("%s%s%s<%p>'%s'%s%s<%p>'%s'\n"), prefix.c_str(), 
             title1.c_str(), cmd1->GetTypeName().c_str(), cmd1, cmd1->GetSummaryName().c_str(),
             title2.c_str(), cmd2->GetTypeName().c_str(), cmd2, cmd2->GetSummaryName().c_str());
   }
}


#ifdef __TEST_MISSION_TREE_ACTIONS__
//------------------------------------------------------------------------------
// void OnStartSaveActions()
//------------------------------------------------------------------------------
/**
 * Start saving actions on the tree to a text file for used in testing
 */
//------------------------------------------------------------------------------
void MissionTree::OnStartSaveActions(wxCommandEvent &event)
{
   #ifdef DEBUG_MISSION_TREE_ACTIONS
   MessageInterface::ShowMessage
      (wxT("MissionTree::OnStartSaveActions() mSaveActions=%d\n"), mSaveActions);
   #endif
   
   mSaveActions = true;
   mPlaybackActions = false;
   
   if (mActionsOutStream.is_open())
      mActionsOutStream.close();
   
   if (mResultsStream.is_open())
      mResultsStream.close();
   
   mActionsOutStream.open(mActionsOutFile.c_str());
   mResultsStream.open(mResultsFile.c_str());
}


//------------------------------------------------------------------------------
// void OnStopSaveActions()
//------------------------------------------------------------------------------
/**
 * Stops saving actions on the tree to a text file.
 */
//------------------------------------------------------------------------------
void MissionTree::OnStopSaveActions(wxCommandEvent &event)
{
   #ifdef DEBUG_MISSION_TREE_ACTIONS
   MessageInterface::ShowMessage
      (wxT("MissionTree::OnStopSaveActions() mSaveActions=%d\n"), mSaveActions);
   #endif
   
   mSaveActions = false;
   mActionsOutStream.close();
   mResultsStream.close();
}


//------------------------------------------------------------------------------
// void OnPlaybackActions()
//------------------------------------------------------------------------------
/**
 * Reads actions from a text file, parses, and initiate actions.
 */
//------------------------------------------------------------------------------
void MissionTree::OnPlaybackActions(wxCommandEvent &event)
{
   #ifdef DEBUG_MISSION_TREE_ACTIONS
   MessageInterface::ShowMessage
      (wxT("MissionTree::OnPlaybackActions() mSaveActions=%d\n"), mSaveActions);
   #endif
   
   wxString actionsInFile =
      wxFileSelector(wxT("Choose a file to open"), wxT(""), wxT(""), wxT("txt"),
                     wxT("Text files (*.txt)|*.txt"), wxOPEN);
   
   if (actionsInFile.empty())
      return;
   
   // clear command sequence and mission tree first
   #ifdef DEBUG_MISSION_TREE_ACTIONS
   MessageInterface::ShowMessage(wxT("   clearing command sequence and mission tree\n"));
   #endif
   
   ClearMission();
   theGuiInterpreter->ClearCommandSeq();
   InitializeCounter();
   
   wxString playbackResultsFile;
   
   // compose playback results file
   int dot = actionsInFile.Find('.');
   if (dot == wxNOT_FOUND)
      playbackResultsFile = actionsInFile + wxT("PbResults.txt");
   else
      playbackResultsFile = actionsInFile.Mid(0, dot) + wxT("PbResults.txt");
   
   #ifdef DEBUG_MISSION_TREE_ACTIONS
   MessageInterface::ShowMessage
      (wxT("   playback action file = '%s'\n   playback results file = '%s'\n"),
       actionsInFile.c_str(), playbackResultsFile.c_str());
   #endif
   
   mSaveActions = false;
   mPlaybackActions = true;
   
   // close output streams
   if (mPlaybackResultsStream.is_open())
      mPlaybackResultsStream.close();
   
   // open streams
   mPlaybackResultsStream.open(playbackResultsFile.c_str());
   
   if (!mPlaybackResultsStream.is_open())
   {
      MessageInterface::ShowMessage
         (wxT("\n*** ERROR *** Playback stopped due to error opening the file '%s'\n"),
          playbackResultsFile.c_str());
      return;
   }
   
   std::ifstream actionsInStream(actionsInFile.c_str());
   
   if (!actionsInStream.is_open())
   {
      MessageInterface::ShowMessage
         (wxT("\n*** ERROR *** Playback stopped due to error opening the file '%s'\n"),
          actionsInFile.c_str());
      return;
   }
   
   //-----------------------------------------------------------------
   // read in lines
   //-----------------------------------------------------------------
   char buffer[1024];
   StringArray lines;
   while (!actionsInStream.eof())
   {
      actionsInStream.getline(buffer, 1023);
      
      #ifdef DEBUG_MISSION_TREE_ACTIONS
      MessageInterface::ShowMessage(wxT("   <%s>\n"), buffer);
      #endif
      
      if (buffer[0] != '\0')
         lines.push_back(buffer);
   }
   
   actionsInStream.close();
   
   //--------------------------------------------------------------
   // Find the first item
   //--------------------------------------------------------------
   wxTreeItemId firstItemId = GetFirstVisibleItem();
   
   if (firstItemId.IsOk())
   {
      #ifdef DEBUG_MISSION_TREE_ACTIONS
      MessageInterface::ShowMessage
         (wxT("   first item is <%s>\n"), GetItemText(firstItemId).c_str());
      #endif
   }
   else
   {
      MessageInterface::ShowMessage
         (wxT("\n*** ERROR *** Playback stopped due to first item not found\n"));
      return;
   }
   
   //-----------------------------------------------------------------
   // parse lines into actions
   //-----------------------------------------------------------------
   int lineCount = lines.size();
   StringTokenizer stk;
   StringArray actions;
   
   for (int i=0; i<lineCount; i++)
   {
      stk.Set(lines[i], wxT(" "));
      actions = stk.GetAllTokens();
      
      #ifdef DEBUG_MISSION_TREE_ACTIONS
      MessageInterface::ShowMessage(wxT("\n"));
      for (UnsignedInt i=0; i<actions.size(); i++)
         MessageInterface::ShowMessage(wxT("<%s>"), actions[i].c_str());
      MessageInterface::ShowMessage(wxT("\n"));
      #endif
      
      //------------------------------------------
      // Sample actions:
      // Append Optimize to Mission Sequence
      // Append If to Optimize1
      // Append Equation to If1
      // Append While to Optimize1
      // Insert Maneuver after While1
      // Delete Stop1 from Mission Sequence
      // Delete Report2 from Optimize1
      //------------------------------------------
      
      //--------------------------------------------------------------
      // Find select item
      //--------------------------------------------------------------
      wxString selCmd = actions[3].c_str();
      wxString cmd = actions[1].c_str();
      
      #ifdef DEBUG_MISSION_TREE_ACTIONS
      MessageInterface::ShowMessage
         (wxT("   selCmd = '%s', cmd = '%s'\n"), selCmd.c_str(), cmd.c_str());
      #endif
      
      bool itemFound = false;
      wxTreeItemId itemId;
      
      //--------------------------------------------------------------
      // Select item
      //--------------------------------------------------------------
      if (actions[0] == wxT("Delete"))
      {
         itemId = FindChild(firstItemId, cmd);
      }
      else
      {
         if (selCmd == wxT("Mission"))
            itemId = firstItemId;
         else
            itemId = FindChild(firstItemId, selCmd);
      }
      
      if (itemId.IsOk() && GetItemText(itemId) != wxT(""))
      {
         SelectItem(itemId);
         itemFound = true;
      }
      
      #ifdef DEBUG_MISSION_TREE_ACTIONS
      wxTreeItemId selId = GetSelection();
      MessageInterface::ShowMessage
         (wxT("   GetSelection()='%s'\n"), GetItemText(selId).c_str());
      #endif
      
      
      //--------------------------------------------------------------
      // Do actions
      //--------------------------------------------------------------
      if (itemFound)
      {
         if (actions[0] == wxT("Append"))
         {
            Append(cmd);
         }
         else if (actions[0] == wxT("Insert"))
         {
            if (actions[2] == wxT("before"))
               InsertBefore(cmd);
            else
               InsertAfter(cmd);
         }
         else if (actions[0] == wxT("Delete"))
         {
            DeleteCommand(cmd);
         }
         else
         {
            MessageInterface::ShowMessage
               (wxT("\n*** ERROR *** Playback stopped due to unknown action \"%s\"\n"),
                actions[0].c_str());
         }
      }
      else
      {
         MessageInterface::ShowMessage
            (wxT("\n*** ERROR *** Playback stopped due to '%s' not found\n"),
             selCmd.c_str());
         return;
      }
   }
   
   // close playback results stream
   mPlaybackResultsStream.close();
   
   #ifdef DEBUG_MISSION_TREE_ACTIONS
   ShowCommands(wxT("After Playback"));
   #endif
}


//------------------------------------------------------------------------------
// void WriteActions(const wxString &str)
//------------------------------------------------------------------------------
void MissionTree::WriteActions(const wxString &str)
{
   #ifdef DEBUG_MISSION_TREE_ACTIONS
   MessageInterface::ShowMessage(wxT("\n..........%s"), str.c_str());
   #endif
   
   // write actions
   mActionsOutStream << str;
}


//------------------------------------------------------------------------------
// void WriteResults()
//------------------------------------------------------------------------------
void MissionTree::WriteResults()
{
   // write results
   GmatCommand *cmd = theGuiInterpreter->GetFirstCommand();
   
   if (mSaveActions)
      mResultsStream << GmatCommandUtil::GetCommandSeqString(cmd, false);
   else if (mPlaybackActions)
      mPlaybackResultsStream << GmatCommandUtil::GetCommandSeqString(cmd, false);
}
#endif

