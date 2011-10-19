//$Id: PropagatePanel.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                              PropagatePanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Waka Waktola
// Created: 2003/08/29
// Modified:
//    2004/05/06 Allison Greene: inherit from GmatPanel
//    2004/10/20 Linda Jun: rename from PropgateCommandPanel
/**
 * This class contains the Propagate Command Setup window.
 */
//------------------------------------------------------------------------------

#include <wx/variant.h> 
#include "PropagatePanel.hpp"
#include "ParameterSelectDialog.hpp"
#include "SpaceObjectSelectDialog.hpp"
#include "PropagatorSelectDialog.hpp"
#include "StringUtil.hpp"               // for SeparateBy()
#include "MessageInterface.hpp"

//#define DEBUG_PROPAGATE_PANEL
//#define DEBUG_PROPAGATE_PANEL_LOAD
//#define DEBUG_PROPAGATE_PANEL_SAVE
//#define DEBUG_PROPAGATE_PANEL_STOPCOND
//#define DEBUG_RENAME

//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(PropagatePanel, GmatPanel)
   EVT_GRID_CELL_LEFT_CLICK(PropagatePanel::OnCellLeftClick)
   EVT_GRID_CELL_RIGHT_CLICK(PropagatePanel::OnCellRightClick)
   EVT_GRID_CELL_CHANGE(PropagatePanel::OnCellValueChange)
   EVT_CHECKBOX(ID_CHECKBOX, PropagatePanel::OnCheckBoxChange)
   EVT_COMBOBOX(ID_COMBOBOX, PropagatePanel::OnComboBoxChange)
   EVT_TEXT(ID_TEXTCTRL, PropagatePanel::OnTextChange)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// PropagatePanel(wxWindow *parent, GmatCommand *cmd)
//------------------------------------------------------------------------------
/**
 * A constructor.
 */
//------------------------------------------------------------------------------
PropagatePanel::PropagatePanel(wxWindow *parent, GmatCommand *cmd)
   : GmatPanel(parent)
{
   #ifdef DEBUG_PROPAGATE_PANEL
   MessageInterface::ShowMessage (wxT("PropagatePanel::PropagatePanel() entered\n"));
   #endif
   
   thePropCmd = (Propagate *)cmd;
   
   InitializeData();   
   mObjectTypeList.Add(wxT("Spacecraft"));
   
   Create();
   Show();
   EnableUpdate(false);
   
   // Default values
   mPropModeChanged = false;
   mPropDirChanged = false;
   mPropSatChanged = false;
   mStopCondChanged = false;
   mStopTolChanged = false;
   isPropGridDisabled = false;
   canClose = true;
   
   // Listen for Propagator or Spacecraft name change
   theGuiManager->AddToResourceUpdateListeners(this);
}


//------------------------------------------------------------------------------
// ~PropagatePanel()
//------------------------------------------------------------------------------
PropagatePanel::~PropagatePanel()
{
   mObjectTypeList.Clear();
   theGuiManager->RemoveFromResourceUpdateListeners(this);
}


//------------------------------------------------------------------------------
// virtual bool PrepareObjectNameChange()
//------------------------------------------------------------------------------
bool PropagatePanel::PrepareObjectNameChange()
{
   // Save GUI data
   wxCommandEvent event;
   OnApply(event);
   
   return GmatPanel::PrepareObjectNameChange();
}


//------------------------------------------------------------------------------
// virtual void ObjectNameChanged(Gmat::ObjectType type, const wxString &oldName,
//                                const wxString &newName)
//------------------------------------------------------------------------------
/*
 * Reflects resource name change to this panel.
 * By the time this method is called, the base code already changed reference
 * object name, so all we need to do is re-load the data.
 */
//------------------------------------------------------------------------------
void PropagatePanel::ObjectNameChanged(Gmat::ObjectType type,
                                       const wxString &oldName,
                                       const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("PropagatePanel::ObjectNameChanged() type=%d, oldName=<%s>, ")
       wxT("newName=<%s>, mDataChanged=%d\n"), type, oldName.c_str(), newName.c_str(),
       mDataChanged);
   #endif
   
   if (type != Gmat::PROP_SETUP && type != Gmat::SPACECRAFT &&
       type != Gmat::PARAMETER)
      return;
   
   // Initialize GUI data and re-load from base
   InitializeData();
   LoadData();
   
   // We don't need to save data if object name changed from the resouce tree
   // while this panel is opened, since base code already has new name
   EnableUpdate(false);
}


//---------------------------------
// private methods
//---------------------------------

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
void PropagatePanel::Create()
{
   #ifdef DEBUG_PROPAGATE_PANEL
   MessageInterface::ShowMessage(wxT("PropagatePanel::Create() entered\n"));
   #endif
   
   Integer bsize = 2; // bordersize
   
   // Propagate Mode
   wxStaticText *synchStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Propagate Mode:  "), 
         wxDefaultPosition, wxDefaultSize, 0);
   
   StringArray propModes = thePropCmd->GetStringArrayParameter
      (thePropCmd->GetParameterID(wxT("AvailablePropModes")));
   
   mPropModeCount = propModes.size();
   
   wxString *propModeList = new wxString[mPropModeCount];
   for (Integer i=0; i<mPropModeCount; i++)
   {
      propModeList[i] = propModes[i].c_str();
   }
   
   if (propModeList[0].IsEmpty())
      propModeList[0] = wxT("None");

   mPropModeComboBox =
      new wxComboBox(this, ID_COMBOBOX, propModeList[0].c_str(), 
         wxDefaultPosition, wxSize(150,-1), mPropModeCount, propModeList,
         wxCB_DROPDOWN|wxCB_READONLY);
   
   mPropModeComboBox->Enable(true);
   
   // Backwards propagate
   backPropCheckBox =
      new wxCheckBox(this, ID_CHECKBOX, wxT("Backwards Propagation"),
                      wxDefaultPosition, wxDefaultSize, 0);
   
   // Propagator Grid
   propGrid =
      new wxGrid(this, ID_GRID, wxDefaultPosition, wxSize(750,100), 
         wxWANTS_CHARS);
   
   propGrid->CreateGrid(MAX_PROP_ROW, MAX_PROP_COL, wxGrid::wxGridSelectCells);
   wxColour gridColor = wxTheColourDatabase->Find(wxT("DIM GREY"));
   propGrid->SetGridLineColour(gridColor);
   
   propGrid->SetColLabelValue(PROP_NAME_SEL_COL, wxT(""));
   propGrid->SetColLabelValue(PROP_NAME_COL, wxT("Propagator"));
   propGrid->SetColLabelValue(PROP_SOS_SEL_COL, wxT(""));
   propGrid->SetColLabelValue(PROP_SOS_COL, wxT("Spacecraft List"));
   propGrid->SetColSize(PROP_NAME_SEL_COL, 25);
   propGrid->SetColSize(PROP_NAME_COL, 340);
   propGrid->SetColSize(PROP_SOS_SEL_COL, 25);
   propGrid->SetColSize(PROP_SOS_COL, 340);
   
   propGrid->SetMargins(0, 0);
   propGrid->SetRowLabelSize(0);
   propGrid->SetScrollbars(5, 8, 15, 15);
   
   for (Integer i = 0; i < MAX_PROP_ROW; i++)
   {
      propGrid->SetReadOnly(i, PROP_NAME_SEL_COL, true);
      propGrid->SetReadOnly(i, PROP_SOS_SEL_COL, true);
      propGrid->SetCellValue(i, PROP_NAME_SEL_COL, wxT("  ... "));
      propGrid->SetCellValue(i, PROP_SOS_SEL_COL, wxT("  ... "));
      propGrid->SetCellBackgroundColour(i, PROP_NAME_SEL_COL, *wxLIGHT_GREY);
      propGrid->SetCellBackgroundColour(i, PROP_SOS_SEL_COL, *wxLIGHT_GREY);
   }
   
   // Stopping Condition Grid
   stopCondGrid =
      new wxGrid(this, ID_GRID, wxDefaultPosition, wxSize(750,100), 
         wxWANTS_CHARS);
   
   stopCondGrid->CreateGrid(MAX_STOPCOND_ROW, MAX_STOPCOND_COL, wxGrid::wxGridSelectCells);
   stopCondGrid->SetGridLineColour(gridColor);
   
   stopCondGrid->SetColLabelValue(STOPCOND_LEFT_SEL_COL, wxT(""));
   stopCondGrid->SetColLabelValue(STOPCOND_LEFT_COL, wxT("Parameter"));
   stopCondGrid->SetColLabelValue(STOPCOND_RELOPER_COL, wxT(""));
   stopCondGrid->SetColLabelValue(STOPCOND_RIGHT_SEL_COL, wxT(""));
   stopCondGrid->SetColLabelValue(STOPCOND_RIGHT_COL, wxT("Condition"));
   
   stopCondGrid->SetColSize(STOPCOND_LEFT_SEL_COL, 25);
   stopCondGrid->SetColSize(STOPCOND_LEFT_COL, 325);
   stopCondGrid->SetColSize(STOPCOND_RELOPER_COL, 30);
   stopCondGrid->SetColSize(STOPCOND_RIGHT_SEL_COL, 25);
   stopCondGrid->SetColSize(STOPCOND_RIGHT_COL, 325);
   
   for (Integer i = 0; i < MAX_STOPCOND_ROW; i++)
   {
      stopCondGrid->SetReadOnly(i, STOPCOND_LEFT_SEL_COL, true);
      stopCondGrid->SetReadOnly(i, STOPCOND_RELOPER_COL, true);
      stopCondGrid->SetReadOnly(i, STOPCOND_RIGHT_SEL_COL, true);
      stopCondGrid->SetReadOnly(i, STOPCOND_RIGHT_COL, true);
      stopCondGrid->SetCellValue(i, STOPCOND_LEFT_SEL_COL, wxT("  ..."));
      stopCondGrid->SetCellValue(i, STOPCOND_RIGHT_SEL_COL, wxT("  ..."));
      stopCondGrid->SetCellBackgroundColour(i, STOPCOND_LEFT_SEL_COL, *wxLIGHT_GREY);
      stopCondGrid->SetCellBackgroundColour(i, STOPCOND_RIGHT_SEL_COL, *wxLIGHT_GREY);
   }
   
   stopCondGrid->SetMargins(0, 0);
   stopCondGrid->SetRowLabelSize(0);
   stopCondGrid->SetScrollbars(5, 8, 15, 15);
   
   wxFlexGridSizer *propModeSizer = new wxFlexGridSizer(4, 0, 0);
   wxBoxSizer *pageSizer = new wxBoxSizer(wxVERTICAL);
   GmatStaticBoxSizer *propSizer = 
      new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Propagators and Spacecraft"));
   
   //Adding objects to sizers
   propModeSizer->Add(synchStaticText, 0, wxALIGN_LEFT|wxALL, bsize);
   propModeSizer->Add(mPropModeComboBox, 0, wxALIGN_LEFT|wxALL, bsize);
   propModeSizer->Add(200, 20, wxALIGN_CENTRE|wxALL, bsize);
   propModeSizer->Add(backPropCheckBox, 0, wxALIGN_LEFT|wxALL, bsize);
   
   propSizer->Add(propModeSizer, 0, wxALIGN_LEFT|wxALL, bsize);
   propSizer->Add(propGrid, 0, wxALIGN_CENTER|wxALL, bsize);
   
   // Stop tolerance
   wxStaticText *stopTolStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Stop Tolerance: "), 
                       wxDefaultPosition, wxSize(-1, -1), 0);
   mStopTolTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                                     wxDefaultPosition, wxSize(150,-1), 0);
   wxBoxSizer *stopTolSizer = new wxBoxSizer(wxHORIZONTAL);
   
   stopTolSizer->Add(stopTolStaticText, 0, wxALIGN_CENTER|wxALL, bsize);
   stopTolSizer->Add(mStopTolTextCtrl, 0, wxALIGN_CENTER|wxALL, bsize);
   
   // Stopping conditions
   GmatStaticBoxSizer *stopSizer =
      new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Stopping Conditions"));
   
   stopSizer->Add(stopTolSizer, 0, wxALIGN_LEFT|wxALL, 0);
   stopSizer->Add(stopCondGrid, 0, wxALIGN_CENTER|wxALL, 0);
   
   pageSizer->Add(propSizer, 0, wxGROW|wxALIGN_CENTER|wxALL, 0);
   pageSizer->Add(stopSizer, 0, wxGROW|wxALIGN_CENTER|wxALL, 0);
   
   theMiddleSizer->Add(pageSizer, 0, wxGROW|wxALIGN_CENTER|wxALL, bsize);
}


//------------------------------------------------------------------------------
// void InitializeData()
//------------------------------------------------------------------------------
void PropagatePanel::InitializeData()
{
   mPropModeCount = 1;
   mPropCount     = 0;
   mStopCondCount = 0;
   
   for (Integer i=0; i<MAX_PROP_ROW; i++)
   {
      mTempProp[i].isChanged = false;
      mTempProp[i].propName = wxT("");
      mTempProp[i].soNames = wxT("");
      mTempProp[i].soNameList.Clear();
   }
   
   for (Integer i=0; i<MAX_STOPCOND_ROW; i++)
   {
      mTempStopCond[i].isChanged = false;
      mTempStopCond[i].name = wxT("");
      mTempStopCond[i].desc = wxT("");
      mTempStopCond[i].varName = wxT("");
      mTempStopCond[i].relOpStr = wxT("");
      mTempStopCond[i].goalStr = wxT("");
      mTempStopCond[i].stopCondPtr = NULL;
   }
}


//------------------------------------------------------------------------------
// void DisplayPropagator()
//------------------------------------------------------------------------------
void PropagatePanel::DisplayPropagator()
{
   #ifdef DEBUG_PROPAGATE_PANEL
   MessageInterface::ShowMessage
      (wxT("PropagatePanel::DisplayPropagator() entered\n"));
   #endif
   
   wxString name;
   for (Integer i=0; i<mPropCount; i++)
   {
      propGrid->SetCellValue(i, PROP_NAME_COL, mTempProp[i].propName);
      propGrid->SetCellValue(i, PROP_SOS_COL, mTempProp[i].soNames);
   }
   
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void DisplayStopCondition()
//------------------------------------------------------------------------------
void PropagatePanel::DisplayStopCondition()
{
   #ifdef DEBUG_PROPAGATE_PANEL
   MessageInterface::ShowMessage
      (wxT("PropagatePanel::DisplayStopCondition() \n"));
   #endif
   
   //----- for stopCondGrid - show all
   for (Integer i=0; i<mStopCondCount; i++)
   {
      stopCondGrid->
         SetCellValue(i, STOPCOND_LEFT_COL, mTempStopCond[i].varName);

      if (mTempStopCond[i].varName.Contains(wxT(".Periapsis")) ||
           mTempStopCond[i].varName.Contains(wxT(".Apoapsis"))  ||
           mTempStopCond[i].varName.IsEmpty())
      {
              stopCondGrid->SetCellValue(i, STOPCOND_RELOPER_COL, wxT(""));
              stopCondGrid->SetCellValue(i, STOPCOND_RIGHT_COL, wxT(""));
              stopCondGrid->SetReadOnly(i, STOPCOND_RIGHT_COL, true);
      }
      else
      {
         stopCondGrid->SetCellValue(i, STOPCOND_RELOPER_COL, wxT("   = "));
         stopCondGrid->
            SetCellValue(i, STOPCOND_RIGHT_COL, mTempStopCond[i].goalStr);
         stopCondGrid->SetReadOnly(i, STOPCOND_RIGHT_COL, false);
      }   
   }
   
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void UpdateStopCondition()
//------------------------------------------------------------------------------
void PropagatePanel::UpdateStopCondition(Integer stopRow)
{
   #ifdef DEBUG_PROPAGATE_PANEL_STOPCOND
   MessageInterface::ShowMessage
      (wxT("PropagatePanel::UpdateStopCondition() entered, stopRow = %d\n"), stopRow);
   #endif
   
   wxString oldStopName = mTempStopCond[stopRow].name;
   mTempStopCond[stopRow].name = wxT("StopOn") + 
      stopCondGrid->GetCellValue(stopRow, STOPCOND_LEFT_COL);
   mTempStopCond[stopRow].varName = 
      stopCondGrid->GetCellValue(stopRow, STOPCOND_LEFT_COL);
   mTempStopCond[stopRow].relOpStr = 
      stopCondGrid->GetCellValue(stopRow, STOPCOND_RELOPER_COL);
   
   // if Apoapsis or Periapsis, disable goal
   if (mTempStopCond[stopRow].varName.Contains(wxT(".Periapsis")) ||
        mTempStopCond[stopRow].varName.Contains(wxT(".Apoapsis")))
   {
      mTempStopCond[stopRow].goalStr = wxT("0.0");
   }
   else
   {
      mTempStopCond[stopRow].goalStr = 
         stopCondGrid->GetCellValue(stopRow, STOPCOND_RIGHT_COL);
   }

   wxString nameStr = mTempStopCond[stopRow].name.c_str();
   wxString stopStr = mTempStopCond[stopRow].varName.c_str();
   wxString goalStr = mTempStopCond[stopRow].goalStr.c_str();
   
   #ifdef DEBUG_PROPAGATE_PANEL_STOPCOND
   MessageInterface::ShowMessage
      (wxT("   old name = '%s'\n   new name = '%s'\n   stop str = '%s'\n   goal str = '%s'\n"),
       oldStopName.c_str(), nameStr.c_str(), stopStr.c_str(), goalStr.c_str());
   #endif
   
   wxString str = FormatStopCondDesc(mTempStopCond[stopRow].varName,
                                     mTempStopCond[stopRow].relOpStr,
                                     mTempStopCond[stopRow].goalStr);
   mTempStopCond[stopRow].desc = str;
   
   #ifdef DEBUG_PROPAGATE_PANEL_STOPCOND
   MessageInterface::ShowMessage(wxT("   str='%s'\n"), str.c_str());
   #endif
   
   
   //-----------------------------------------------------------------
   // create StopCondition if new StopCondition
   //-----------------------------------------------------------------
   if (oldStopName.IsSameAs(wxT("")))
   {
      #ifdef DEBUG_PROPAGATE_PANEL_STOPCOND
      MessageInterface::ShowMessage(wxT("   Creating new stop condition\n"));
      #endif
      StopCondition *stopCond = 
         (StopCondition*)theGuiInterpreter-> CreateStopCondition
         (wxT("StopCondition"), stopStr);
      mTempStopCond[stopRow].stopCondPtr = stopCond;
      
      if (stopCond == NULL)
      {
         MessageInterface::ShowMessage
            (wxT("PropagatePanel::UpdateStopCondition() Unable to create ")
             wxT("StopCondition: name=%s\n"), mTempStopCond[stopRow].name.c_str());
      }
   }
   
   mTempStopCond[stopRow].isChanged = true;
   
   mStopCondChanged = true;
   EnableUpdate(true);
   
   #ifdef DEBUG_PROPAGATE_PANEL_STOPCOND
   MessageInterface::ShowMessage(wxT("PropagatePanel::UpdateStopCondition() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void GetNewPropagatorName(Integer row, Integer col)
//------------------------------------------------------------------------------
void PropagatePanel::GetNewPropagatorName(Integer row, Integer col)
{
   PropagatorSelectDialog propDlg(this, wxT(""));
   propDlg.ShowModal();
   
   if (propDlg.HasSelectionChanged())
   {
      wxString newPropName = propDlg.GetPropagatorName();
      #ifdef DEBUG_PROPAGATE_PANEL
      MessageInterface::ShowMessage
         (wxT("PropagatePanel::GetNewPropagatorName() newPropName = %s\n"),
          newPropName.c_str());
      #endif
      
      propGrid->SetCellValue(row, col, newPropName);
      mPropSatChanged = true;
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void GetNewSpaceObjectList(Integer row, Integer col)
//------------------------------------------------------------------------------
void PropagatePanel::GetNewSpaceObjectList(Integer row, Integer col)
{
   wxArrayString soExcList;
   Integer soCount = 0;
   
   for (Integer i=0; i<MAX_PROP_ROW; i++)
   {
      soCount = mTempProp[i].soNameList.Count();
      
      for (Integer j=0; j<soCount; j++)
         soExcList.Add(mTempProp[i].soNameList[j]);
   }
   
   SpaceObjectSelectDialog soDlg(this, mTempProp[row].soNameList, soExcList);
   soDlg.ShowModal();
   
   if (soDlg.HasSelectionChanged())
   {
      wxArrayString &newNames = soDlg.GetSpaceObjectNames();
      mTempProp[row].isChanged = true;
      mTempProp[row].soNames = wxT("");
      soCount = newNames.GetCount();
      
      #ifdef DEBUG_PROPAGATE_PANEL
      MessageInterface::ShowMessage
         (wxT("PropagatePanel::GetNewSpaceObjectList() new soCount=%d\n"), soCount);
      #endif
      
      mTempProp[row].soNameList.Clear();
      for (Integer j=0; j<soCount; j++)
      {
         mTempProp[row].soNameList.Add(newNames[j]);
         
         #ifdef DEBUG_PROPAGATE_PANEL
         MessageInterface::ShowMessage
            (wxT("PropagatePanel::GetNewSpaceObjectList() soNameList[%d]='%s'\n"),
             j, mTempProp[row].soNameList[j].c_str());
         #endif
      }
      
      if (soCount > 0)
      {
         for(Integer j=0; j < soCount-1; j++)
         {
            mTempProp[row].soNames += newNames[j].c_str();
            mTempProp[row].soNames += wxT(", ");
         }
         
         mTempProp[row].soNames += newNames[soCount-1].c_str();
      }
      
      mTempProp[row].soCount = soCount;
      
      propGrid->SetCellValue(row, col, mTempProp[row].soNames);
      
      mPropSatChanged = true;
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void GetNewStopCondLeftValue(Integer row, Integer col)
//------------------------------------------------------------------------------
void PropagatePanel::GetNewStopCondLeftValue(Integer row, Integer col)
{
   // show dialog to select parameter
   // we cannot allow Variables
   ParameterSelectDialog paramDlg(this, mObjectTypeList);
   paramDlg.ShowModal();
   
   if (paramDlg.HasSelectionChanged())
   {
      wxString newParamName = paramDlg.GetParamName();
      stopCondGrid->SetCellValue(row,STOPCOND_LEFT_COL,newParamName);
      
      // if Apoapsis or Periapsis, disable goal
      if (newParamName.Contains(wxT(".Periapsis")) ||
          newParamName.Contains(wxT(".Apoapsis")))
      {
         stopCondGrid->SetCellValue(row, STOPCOND_RELOPER_COL, wxT(""));
         stopCondGrid->SetCellValue(row, STOPCOND_RIGHT_COL, wxT(""));
         stopCondGrid->SetReadOnly(row, STOPCOND_RIGHT_COL, true);
      }
      else
      {
         stopCondGrid->SetCellValue(row, STOPCOND_RELOPER_COL, wxT("   = "));
         stopCondGrid->SetCellValue(row, STOPCOND_RIGHT_COL, wxT("0.0"));
         stopCondGrid->SetReadOnly(row, STOPCOND_RIGHT_COL, false);
      }
      
      mStopCondChanged = true;
      UpdateStopCondition(row);
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void GetNewStopCondRightValue(Integer row, Integer col)
//------------------------------------------------------------------------------
void PropagatePanel::GetNewStopCondRightValue(Integer row, Integer col)
{
   wxString paramName = stopCondGrid->GetCellValue(row, STOPCOND_LEFT_COL);
   
   // do nothing if parameter is empty
   if (paramName == wxT(""))
      return;
   
   // do nothing if parameter contains Apoapsis or Periapsis
   if (paramName.Contains(wxT(".Periapsis")) ||
       paramName.Contains(wxT(".Apoapsis")))
      return;
   
   // show dialog to select parameter
   // we can allow Variables
   ParameterSelectDialog paramDlg(this, mObjectTypeList);
   paramDlg.ShowModal();
   
   if (paramDlg.HasSelectionChanged())
   {
      wxString newParamName = paramDlg.GetParamName();
      stopCondGrid->SetCellValue(row, STOPCOND_RIGHT_COL, newParamName);
      mStopCondChanged = true;
      UpdateStopCondition(row);
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// wxString FormatStopCondDesc(const wxString &varName, 
//                             const wxString &relOpStr,
//                             const wxString &goalStr)
//------------------------------------------------------------------------------
wxString PropagatePanel::FormatStopCondDesc(const wxString &varName,
                                            const wxString &relOpStr,
                                            const wxString &goalStr)
{
   #ifdef DEBUG_PROPAGATE_PANEL
   MessageInterface::ShowMessage
      (wxT("PropagatePanel::FormatStopCondDesc() entered\n"));
   #endif

   wxString goalTempStr = goalStr;
   wxString desc;
   wxString opStr = relOpStr;
   
   if (varName.Contains(wxT("Apoapsis")) || varName.Contains(wxT("Periapsis")))
   {
      opStr = wxT("");
      goalTempStr = wxT("");
   }
   
   desc = varName + wxT(" ") + opStr + wxT(" ") + goalTempStr;
    
   return desc;
}


//------------------------------------------------------------------------------
// void OnTextChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void PropagatePanel::OnTextChange(wxCommandEvent& event)
{
   if (event.GetEventObject() == mStopTolTextCtrl)
   {
      mStopTolChanged = true;
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void OnCheckBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
/**
 * Handles CheckBox event.
 */
//------------------------------------------------------------------------------
void PropagatePanel::OnCheckBoxChange(wxCommandEvent& event)
{
   #ifdef DEBUG_PROPAGATE_PANEL
   MessageInterface::ShowMessage
      (wxT("PropagatePanel::OnCheckBoxChange() entered\n"));
   #endif

   if (event.GetEventObject() == backPropCheckBox)
   {
      mPropDirChanged = true;
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void OnComboBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void PropagatePanel::OnComboBoxChange(wxCommandEvent& event)
{
   #ifdef DEBUG_PROPAGATE_PANEL
   MessageInterface::ShowMessage
      (wxT("PropagatePanel::OnComboBoxChange() entered\n"));
   #endif
   
   if (event.GetEventObject() == mPropModeComboBox)
   {
      mPropModeChanged = true;
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void OnCellLeftClick(wxGridEvent& event)
//------------------------------------------------------------------------------
void PropagatePanel::OnCellLeftClick(wxGridEvent& event)
{
   Integer row = event.GetRow();
   Integer col = event.GetCol();
   
   #ifdef DEBUG_PROPAGATE_PANEL
   MessageInterface::ShowMessage
      (wxT("PropagatePanel::OnCellLeftClick() isPropGridDisabled=%d, row = %d, col = %d\n"),
       isPropGridDisabled, row, col);
   #endif
   
   // Propagate grid
   if (event.GetEventObject() == propGrid)
   {
      if (isPropGridDisabled)
         return;
      
      propGrid->SelectBlock(row, col, row, col);
      propGrid->SetGridCursor(row, col);
      
      if (col == PROP_NAME_SEL_COL)
         GetNewPropagatorName(row, col + 1);
      else if (col == PROP_SOS_SEL_COL)
         GetNewSpaceObjectList(row, col + 1);
   }
   // Stopping Condition grid
   else if (event.GetEventObject() == stopCondGrid)
   {
      stopCondGrid->SelectBlock(row, col, row, col);
      stopCondGrid->SetGridCursor(row, col);
      
      if (col == STOPCOND_LEFT_SEL_COL)
         GetNewStopCondLeftValue(row, col + 1);
      else if (col == STOPCOND_RIGHT_SEL_COL)
         GetNewStopCondRightValue(row, col + 1);
   }
}


//------------------------------------------------------------------------------
// void OnCellRightClick(wxGridEvent& event)
//------------------------------------------------------------------------------
void PropagatePanel::OnCellRightClick(wxGridEvent& event)
{
   Integer row = event.GetRow();
   Integer col = event.GetCol();
   
   #ifdef DEBUG_PROPAGATE_PANEL
   MessageInterface::ShowMessage
      (wxT("PropagatePanel::OnCellRightClick() row = %d, col = %d\n"), row, col);
   #endif
   
   // Propagate grid
   if (event.GetEventObject() == propGrid)
   {
      if (isPropGridDisabled)
         return;
      
      propGrid->SelectBlock(row, col, row, col);
      propGrid->SetGridCursor(row, col);
      
      if (col == PROP_NAME_COL)
         GetNewPropagatorName(row, col);
      else if (col == PROP_SOS_COL)
         GetNewSpaceObjectList(row, col);
   } 
   
   // Stopping Condition grid
   else if (event.GetEventObject() == stopCondGrid)
   {
      stopCondGrid->SelectBlock(row, col, row, col);
      stopCondGrid->SetGridCursor(row, col);
      
      if (col == STOPCOND_LEFT_COL)
         GetNewStopCondLeftValue(row, col);
      else if (col == STOPCOND_RIGHT_COL)
         GetNewStopCondRightValue(row, col);
   }
}


//------------------------------------------------------------------------------
// void OnCellValueChange(wxGridEvent& event)
//------------------------------------------------------------------------------
void PropagatePanel::OnCellValueChange(wxGridEvent& event)
{
   Integer row = event.GetRow();
   Integer col = event.GetCol();
   
   #ifdef DEBUG_PROPAGATE_PANEL
   MessageInterface::ShowMessage
      (wxT("PropagatePanel::OnCellValueChange() row=%d, col=%d, "), row, col);
   #endif
   
   if (event.GetEventObject() == stopCondGrid)
   {
      wxString paramName = stopCondGrid->GetCellValue(row, STOPCOND_LEFT_COL);
      wxString condValue = stopCondGrid->GetCellValue(row, STOPCOND_RIGHT_COL);
      
      #ifdef DEBUG_PROPAGATE_PANEL
      MessageInterface::ShowMessage
         (wxT("paramName=<%s>, condValue=<%s>\n"), paramName.c_str(), condValue.c_str());
      #endif
      
      if (col == STOPCOND_LEFT_COL)
      {
         // if Apoapsis or Periapsis, disable goal
         if (paramName.Contains(wxT(".Periapsis")) || paramName.Contains(wxT(".Apoapsis")))
         {
            stopCondGrid->SetCellValue(row, STOPCOND_RELOPER_COL, wxT(""));
            stopCondGrid->SetCellValue(row, STOPCOND_RIGHT_COL, wxT(""));
            stopCondGrid->SetReadOnly(row, STOPCOND_RIGHT_COL, true);
         }
         else if (paramName == wxT(""))
         {
            // do not show = sign if codition is empty
            if (condValue == wxT(""))
               stopCondGrid->SetCellValue(row, STOPCOND_RELOPER_COL, wxT(""));
         }
         else
         {
            stopCondGrid->SetCellValue(row, STOPCOND_RELOPER_COL, wxT("   = "));
            if (stopCondGrid->GetCellValue(row, STOPCOND_RIGHT_COL) == wxT(""))
               stopCondGrid->SetCellValue(row, STOPCOND_RIGHT_COL, wxT("0.0"));
            stopCondGrid->SetReadOnly(row, STOPCOND_RIGHT_COL, false);
         }
      }
      else if (col == STOPCOND_RIGHT_COL)
      {
         // do not show = sign if pameter and codition is empty
         if (paramName == wxT("") && condValue == wxT(""))
            stopCondGrid->SetCellValue(row, STOPCOND_RELOPER_COL, wxT(""));
      }
      
      mStopCondChanged = true;
   }
   
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void LoadData()
//------------------------------------------------------------------------------
void PropagatePanel::LoadData()
{
   #ifdef DEBUG_PROPAGATE_PANEL_LOAD
   MessageInterface::ShowMessage(wxT("PropagatePanel::LoadData() entered\n"));
   #endif
   
   // Set the pointer for the "Show Script" button
   mObject = thePropCmd;
   
   //----------------------------------
   // propagation mode
   //----------------------------------
   wxString mode =
      thePropCmd->GetStringParameter(thePropCmd->GetParameterID(wxT("PropagateMode")));
   mPropModeComboBox->SetStringSelection(mode.c_str());
   
   //----------------------------------
   // Backwards propagation
   //----------------------------------
   Integer PropDirectionId = thePropCmd->GetParameterID(wxT("PropForward"));
   bool backProp = !thePropCmd->GetBooleanParameter(PropDirectionId);
   backPropCheckBox->SetValue(backProp);
   
   //----------------------------------
   // propagator
   //----------------------------------
   Integer propId = thePropCmd->GetParameterID(wxT("Propagator"));
   
   // Get the list of propagators (aka the PropSetups)
   StringArray propNames = thePropCmd->GetStringArrayParameter(propId);
   mPropCount = propNames.size();
   
   StringArray soList;
   wxString name;
   
   Integer scId = thePropCmd->GetParameterID(wxT("Spacecraft"));

   #ifdef DEBUG_PROPAGATE_PANEL_LOAD
   MessageInterface::ShowMessage
      (wxT("PropagatePanel::LoadData() mPropCount=%d\n"), mPropCount);
   #endif
   
   Integer soCount = 0;
   
   if (mPropCount > MAX_PROP_ROW)
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("There are more propagators (%d) than GMAT can manage ")
          wxT("to show (%d).\nSo the propagator grid is set to uneditable.\n"),
          mPropCount, MAX_PROP_ROW);
      mPropCount = MAX_PROP_ROW;
      propGrid->EnableEditing(false);
      isPropGridDisabled = true;
   }
   
   for (Integer i=0; i<mPropCount; i++)
   {
      mTempProp[i].propName = propNames[i].c_str();
      
      // Get the list of spacecraft and formations
      soList = thePropCmd->GetStringArrayParameter(scId, i);
      soCount = soList.size();
      
      #ifdef DEBUG_PROPAGATE_PANEL_LOAD
      MessageInterface::ShowMessage
         (wxT("PropagatePanel::LoadData() propName=%s, soCount=%d\n"),
          propNames[i].c_str(), soCount);
      #endif
      
      Integer actualSoCount = 0;
      for (Integer j=0; j<soCount; j++)
      {
         #ifdef DEBUG_PROPAGATE_PANEL_LOAD
         MessageInterface::ShowMessage
            (wxT("PropagatePanel::LoadData() soNameList[%d]='%s'\n"),
             j, soList[j].c_str());
         #endif
         
         // verify space object actually exist
         if (theGuiInterpreter->GetConfiguredObject(soList[j]))
         {
            actualSoCount++;
            mTempProp[i].soNameList.Add(soList[j].c_str());
         }
         else
         {
            MessageInterface::PopupMessage
               (Gmat::WARNING_, wxT("The SpaceObject named '%s' was not created, ")
                wxT("so removed from the display list\n"), soList[j].c_str());
         }
      }
      
      #ifdef DEBUG_PROPAGATE_PANEL_LOAD
      MessageInterface::ShowMessage(wxT("   actualSoCount=%d\n"), actualSoCount);
      #endif
      
      soCount = actualSoCount;
      mTempProp[i].soCount = actualSoCount;
      
      if (soCount > 0)
      {
         for (Integer j=0; j<soCount-1; j++)
         {
            mTempProp[i].soNames += soList[j].c_str();
            mTempProp[i].soNames += wxT(", ");
         }
         
         mTempProp[i].soNames += soList[soCount-1].c_str();
      }
      
   } // for (Integer i=0; i<mPropCount; i++)
   
   backPropCheckBox->SetValue(backProp);
   
   //----------------------------------
   // stopping conditions
   //----------------------------------
   
   Real stopTol = thePropCmd->GetRealParameter(wxT("StopTolerance"));
   mStopTolTextCtrl->SetValue(theGuiManager->ToWxString(stopTol));
   
   ObjectArray &stopArray =
      thePropCmd->GetRefObjectArray(Gmat::STOP_CONDITION);
   mStopCondCount = stopArray.size();
   
   #ifdef DEBUG_PROPAGATE_PANEL_LOAD
   MessageInterface::ShowMessage
      (wxT("PropagatePanel::LoadData() mStopCondCount=%d\n"), mStopCondCount);
   #endif
   
   StopCondition  *stopCond;
   for (Integer i=0; i<mStopCondCount; i++)
   {
      stopCond = (StopCondition *)stopArray[i]; 
      
      #ifdef DEBUG_PROPAGATE_PANEL_LOAD
      MessageInterface::ShowMessage
         (wxT("PropagatePanel::LoadData() stopCond=%p, stopArray[%d]=%s\n"),
          stopCond, i, stopCond->GetName().c_str());
      #endif
      
      // StopCondition created from the script might not have been 
      // configured (unnamed)
      if (stopCond != NULL)
      {
         mTempStopCond[i].stopCondPtr = stopCond;
         mTempStopCond[i].name = stopCond->GetName().c_str();
         mTempStopCond[i].varName = 
            stopCond->GetStringParameter(wxT("StopVar"));
         mTempStopCond[i].goalStr = 
            stopCond->GetStringParameter(wxT("Goal"));
         wxString str = FormatStopCondDesc(mTempStopCond[i].varName,
                                           mTempStopCond[i].relOpStr,
                                           mTempStopCond[i].goalStr);
         mTempStopCond[i].desc = str;
      }
   }
   
   DisplayPropagator();
   DisplayStopCondition();
}


//------------------------------------------------------------------------------
// void SaveData()
//------------------------------------------------------------------------------
void PropagatePanel::SaveData()
{
   #ifdef DEBUG_PROPAGATE_PANEL_SAVE
   MessageInterface::ShowMessage(wxT("PropagatePanel::SaveData() entered\n"));
   #endif
   
   canClose = true;
   Integer blankProps = 0;
   wxArrayString emptyProps, emptySos;
   wxString propName, soNames;
   
   //-----------------------------------------------------------------
   // check valid propagators and space objects
   //-----------------------------------------------------------------
   for (Integer i=0; i<MAX_PROP_ROW; i++)
   {
      propName = propGrid->GetCellValue(i, PROP_NAME_COL);
      soNames = propGrid->GetCellValue(i, PROP_SOS_COL);
      
      if (propName == wxT("") && soNames == wxT(""))
         ++blankProps;
      else if (propName != wxT("") && soNames == wxT(""))
         emptySos.Add(propName);
      else if (propName == wxT("") && soNames != wxT(""))
         emptyProps.Add(soNames);
   }
   
   // check to see if there is at least one propagator
   if (blankProps == MAX_PROP_ROW)
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_,
          wxT("Propagate command must have at least one propagator.\n"));
      canClose = false;
   }
   
   // check to see if there is any missing propagators for space objects
   if (emptyProps.GetCount() > 0)
   {
      for (UnsignedInt i=0; i<emptyProps.GetCount(); i++)
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("Please select a Propagator for ")
             wxT("Spacecraft(s) \"%s\"\n"), emptyProps[i].c_str());
      
      canClose = false;
   }
   
   // check to see if there is any missing space objects to propagate
   if (emptySos.GetCount() > 0)
   {
      for (UnsignedInt i=0; i<emptySos.GetCount(); i++)
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("Please select Spacecraft(s) for ")
             wxT("Propagator \"%s\"\n"), emptySos[i].c_str());
      
      canClose = false;
   }
   
   //-----------------------------------------------------------------
   // check input values: Number, Variable, Array element, Parameter
   //-----------------------------------------------------------------
   Real stopTol;
   if (mStopTolChanged)
   {
      CheckReal(stopTol, mStopTolTextCtrl->GetValue().c_str(), wxT("StopTolerance"),
                wxT("Real Number > 0"), false, true);
   }
   
   if (mStopCondChanged)
   {
      for (Integer i=0; i<MAX_STOPCOND_ROW; i++)
      {
         if ((stopCondGrid->GetCellValue(i, STOPCOND_LEFT_COL) != wxT("")) ||
              (stopCondGrid->GetCellValue(i, STOPCOND_RIGHT_COL) != wxT("")))
         {
            CheckVariable(stopCondGrid->GetCellValue
                          (i, STOPCOND_LEFT_COL).c_str(), Gmat::SPACECRAFT, wxT("Parameter"),
                          wxT("Variable, Array element, plottable Parameter"), true, true);
            
            // check condition if parameter is not Periapsis nor Apoapsis
            wxString paramName = stopCondGrid->GetCellValue(i, STOPCOND_LEFT_COL);
            if (!paramName.Contains(wxT(".Periapsis")) && !paramName.Contains(wxT(".Apoapsis")))
            {
               CheckVariable(stopCondGrid->GetCellValue
                             (i, STOPCOND_RIGHT_COL).c_str(), Gmat::SPACECRAFT, wxT("Condition"),
                             wxT("Variable, Array element, plottable Parameter"), true, true);
            }
         }
      }
   }
   
   if (!canClose)
      return;
   
   //-----------------------------------------------------------------
   // save values to base, base code should do the range checking
   //-----------------------------------------------------------------
   try
   {
      //-------------------------------------------------------
      // Save propagation mode
      //-------------------------------------------------------
      if (mPropModeChanged)
      {
         #ifdef DEBUG_PROPAGATE_PANEL_SAVE
         MessageInterface::ShowMessage
            (wxT("PropagatePanel::SaveData() Save propagation mode \n"));
         #endif
         
         mPropModeChanged = false;
         wxString str = mPropModeComboBox->GetStringSelection();
         if (str.CmpNoCase(wxT("None")) == 0)
            str = wxT("");
         thePropCmd->SetStringParameter
            (thePropCmd->GetParameterID(wxT("PropagateMode")), str.c_str());
      }
      
      //---------------------------------------
      // Save propagator and spacecraft
      //---------------------------------------
      if (mPropSatChanged)
      {         
         #ifdef DEBUG_PROPAGATE_PANEL_SAVE
         MessageInterface::ShowMessage
            (wxT("PropagatePanel::SaveData() Save propagator and spacecraft \n"));
         #endif
         
         mPropSatChanged = false;
         Integer propId = thePropCmd->GetParameterID(wxT("Propagator"));
         Integer scId = thePropCmd->GetParameterID(wxT("Spacecraft"));
         
         // Clear propagator and spacecraft list
         thePropCmd->TakeAction(wxT("Clear"), wxT("Propagator"));
         
         Integer soCount = 0;
         mPropCount = 0;
         for (Integer i=0; i<MAX_PROP_ROW; i++)
         {
            #ifdef DEBUG_PROPAGATE_PANEL_SAVE
            MessageInterface::ShowMessage
               (wxT("PropagatePanel::SaveData() propagator name[%d]=%s\n"),
                i,propGrid->GetCellValue(i, PROP_NAME_COL).c_str());
            MessageInterface::ShowMessage
               (wxT("PropagatePanel::SaveData() spacecraft name[%d]=%s\n"),
                i,propGrid->GetCellValue(i, PROP_SOS_COL).c_str());
            #endif
            
            
            if ((propGrid->GetCellValue(i, PROP_NAME_COL) != wxT("")) || 
                 (propGrid->GetCellValue(i, PROP_SOS_COL) != wxT("")))
            {
               mTempProp[mPropCount].propName = 
                  propGrid->GetCellValue(i, PROP_NAME_COL).c_str(); 
            
               #ifdef DEBUG_PROPAGATE_PANEL_SAVE
               MessageInterface::ShowMessage
                  (wxT("PropagatePanel::SaveData() propName[%d]=%s\n"),
                   mPropCount,mTempProp[mPropCount].propName.c_str());
               #endif
               
               // saving propagator
               thePropCmd->SetStringParameter 
                  (propId, mTempProp[mPropCount].propName.c_str());
               
               // saving spacecraft
               wxString spacecraftStr = 
                  propGrid->GetCellValue(i, PROP_SOS_COL).c_str();
               StringArray parts = 
                  GmatStringUtil::SeparateBy(spacecraftStr, wxT(", "));
               
               soCount = parts.size();                    
               for (Integer j=0; j<soCount; j++)
               {
                  #ifdef DEBUG_PROPAGATE_PANEL_SAVE
                  MessageInterface::ShowMessage
                     (wxT("parts[%d] = '%s'\n"), j, parts[j].c_str());
                  #endif
                  
                  thePropCmd->SetStringParameter
                     (scId, wxString(parts[j].c_str()), mPropCount);
               }
               ++mPropCount;
            } // if
         } // for MAX_PROP_ROW
      } // if (mPropSatChanged)
      
      
      //---------------------------------------
      // Save the prop direction
      //---------------------------------------
      if (mPropDirChanged)
      {
         mPropDirChanged = false;
         thePropCmd->SetBooleanParameter(wxT("PropForward"),
                                         !(backPropCheckBox->IsChecked()));
      }
      
      //---------------------------------------
      // Save stop tolerance
      //---------------------------------------
      if (mStopTolChanged)
      {
         mStopTolChanged = false;
         thePropCmd->SetRealParameter(wxT("StopTolerance"), stopTol);
      }
      
      //---------------------------------------
      // Save stopping condition
      //---------------------------------------
      
      #ifdef DEBUG_PROPAGATE_PANEL_SAVE
      MessageInterface::ShowMessage
         (wxT("PropagatePanel::SaveData() mPropCount=%d\n"), mPropCount);
      #endif
      
      if (mStopCondChanged)
      {
         #ifdef DEBUG_PROPAGATE_PANEL_SAVE
         MessageInterface::ShowMessage
            (wxT("PropagatePanel::SaveData() Save stopping conditions\n"));
         #endif
         
         mStopCondChanged = false;
         thePropCmd->TakeAction(wxT("Clear"), wxT("StopCondition"));
         
         mStopCondCount = 0;
         // count number of stopping conditions
         for (Integer i=0; i<MAX_STOPCOND_ROW; i++)
         {
            if (stopCondGrid->GetCellValue(i, STOPCOND_LEFT_COL) != wxT(""))
            {
               UpdateStopCondition(i);

               StopCondition *currStop = mTempStopCond[mStopCondCount].stopCondPtr;
               wxString nameStr = mTempStopCond[i].name.c_str();
               wxString stopStr = mTempStopCond[i].varName.c_str();
               wxString goalStr = mTempStopCond[i].goalStr.c_str();
               
               #ifdef DEBUG_PROPAGATE_PANEL_SAVE
               MessageInterface::ShowMessage
                  (wxT("   Saving stop condition, name='%s', stop='%s', goal='%s'\n"),
                   nameStr.c_str(), stopStr.c_str(), goalStr.c_str());
               #endif
               
               currStop->SetName(nameStr);
               currStop->SetStringParameter(wxT("StopVar"), stopStr);
               currStop->SetStringParameter(wxT("Goal"), goalStr);
               
               thePropCmd->
                  SetRefObject(mTempStopCond[mStopCondCount].stopCondPtr, 
                               Gmat::STOP_CONDITION,wxT(""), mStopCondCount);
               
               mStopCondCount++;
               
               #ifdef DEBUG_PROPAGATE_PANEL_SAVE
               MessageInterface::ShowMessage
                  (wxT("PropagatePanel::SaveData() mStopCondCount=%d\n"), 
                   mStopCondCount);
               #endif
            } // STOPCOND_LEFT_COL != wxT("")
         } // for MAX_STOPCOND_ROW
         
         // Validate command to create stop condition wrappers
         theGuiInterpreter->ValidateCommand(thePropCmd);
         
      } // if (mStopCondChanged)
   } // try
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      canClose = false;
      return;
   }
}


   

