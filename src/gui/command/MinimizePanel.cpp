//$Id: MinimizePanel.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                              MinimizePanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc.
//
// Author: Allison Greene
// Created: 2006/09/20
/**
 * This class contains the Minimize command setup window.
 */
//------------------------------------------------------------------------------
#include "gmatwxdefs.hpp"
#include "GmatAppData.hpp"
#include "ParameterSelectDialog.hpp"
#include "MinimizePanel.hpp"
#include "Array.hpp"
#include <wx/variant.h>

// base includes
#include "gmatdefs.hpp"
#include "GuiInterpreter.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_MINIMIZE_PANEL 1

//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(MinimizePanel, GmatPanel)
   EVT_BUTTON(ID_BUTTON, MinimizePanel::OnButtonClick)
   EVT_TEXT(ID_TEXTCTRL, MinimizePanel::OnTextChange)
   EVT_COMBOBOX(ID_COMBO, MinimizePanel::OnSolverSelection)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// MinimizePanel(wxWindow *parent, GmatCommand *cmd)
//------------------------------------------------------------------------------
MinimizePanel::MinimizePanel(wxWindow *parent, GmatCommand *cmd)
   : GmatPanel(parent)
{
   mMinimizeCommand = (Minimize *)cmd;
   theGuiManager = GuiItemManager::GetInstance();
   
   solverName = wxT("");
   minParam = NULL;
   mVarNameChanged = false;
   
   mObjectTypeList.Add(wxT("Spacecraft"));
   
   Create();
   Show();
   
   EnableUpdate(false);
}


//------------------------------------------------------------------------------
// ~MinimizePanel()
//------------------------------------------------------------------------------
MinimizePanel::~MinimizePanel()
{
   mObjectTypeList.Clear();
   theGuiManager->UnregisterComboBox(wxT("Optimizer"), mSolverComboBox);
}


//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
void MinimizePanel::Create()
{
   int bsize = 2; // bordersize

   // Optimizer
   wxStaticText *solverStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Optimizer"),
                       wxDefaultPosition, wxDefaultSize, 0);
   mSolverComboBox = theGuiManager->GetOptimizerComboBox(this, ID_COMBO,
                        wxSize(120,-1));
   
   // Variable to be Minimized
   wxStaticText *variableStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Variable to be Minimized"), 
                       wxDefaultPosition, wxDefaultSize, 0);
   mVariableTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                                     wxDefaultPosition, wxSize(250,-1), 0);
   
   // Choose button
   mChooseButton = new
      wxButton(this, ID_BUTTON, wxT("Edit"), wxDefaultPosition, wxDefaultSize, 0);
 
   wxBoxSizer *panelSizer = new wxBoxSizer(wxHORIZONTAL);
   wxBoxSizer *solverSizer = new wxBoxSizer(wxVERTICAL);
   wxBoxSizer *variableSizer = new wxBoxSizer(wxVERTICAL);
   wxBoxSizer *variableInterfaceSizer = new wxBoxSizer(wxHORIZONTAL);
   
   solverSizer->Add(solverStaticText, 0, wxALIGN_CENTER|wxALL, bsize);
   solverSizer->Add(mSolverComboBox, 0, wxALIGN_CENTER|wxALL, bsize);
   
   variableInterfaceSizer->Add(mVariableTextCtrl, 0, wxALIGN_CENTER|wxALL, bsize);
   variableInterfaceSizer->Add(mChooseButton, 0, wxALIGN_CENTER|wxALL, bsize);
   
   variableSizer->Add(variableStaticText, 0, wxALIGN_CENTER|wxALL, bsize);
   variableSizer->Add(variableInterfaceSizer, 0, wxALIGN_CENTER|wxALL, bsize);

   panelSizer->Add(solverSizer, 0, wxALIGN_CENTER|wxALL, bsize);
   panelSizer->Add(variableSizer, 0, wxALIGN_CENTER|wxALL, bsize);
   
   theMiddleSizer->Add(panelSizer, 0, wxGROW|wxALIGN_CENTER|wxALL, bsize);
}


//------------------------------------------------------------------------------
// void LoadData()
//------------------------------------------------------------------------------
void MinimizePanel::LoadData()
{
   #if DEBUG_MINIMIZE_PANEL
   MessageInterface::ShowMessage(wxT("MinimizePanel::LoadData() entered\n"));
   MessageInterface::ShowMessage(wxT("Command=%s\n"), mMinimizeCommand->GetTypeName().c_str());
   #endif

   try
   {
      // Set the pointer for the "Show Script" button
      mObject = mMinimizeCommand;
      
      wxString loadedSolverName = mMinimizeCommand->
         GetStringParameter(mMinimizeCommand->GetParameterID(wxT("OptimizerName")));

      wxString loadedVariableName = mMinimizeCommand->
         GetStringParameter(mMinimizeCommand->GetParameterID(wxT("ObjectiveName")));

      #if DEBUG_MINIMIZE_PANEL
      MessageInterface::ShowMessage(wxT("solverName=%s\n"), loadedSolverName.c_str());
      MessageInterface::ShowMessage(wxT("variable=%s\n"), loadedVariableName.c_str());
      #endif
      
      solverName = loadedSolverName.c_str();      
      variableName = loadedVariableName.c_str();
      
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
   
   ShowGoalSetup();
}


//------------------------------------------------------------------------------
// void SaveData()
//------------------------------------------------------------------------------
void MinimizePanel::SaveData()
{   
   #if DEBUG_MINIMIZE_PANEL
   MessageInterface::ShowMessage(wxT("MinimizePanel::SaveData() entered\n"));
   #endif
   
   canClose = true;
   
   //-----------------------------------------------------------------
   // check input value - Variable, Array element, Spacecraft Parameter
   //-----------------------------------------------------------------
   
   if (mVarNameChanged)
   {
      wxString varName = variableName.c_str();
      
      int retval = theGuiManager->IsValidVariable(varName, Gmat::SPACECRAFT);
      
      if (retval == -1)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("The variable \"") + varName + wxT("\" does not exist.\n")
             wxT("Press \"Choose\" or create from the resource tree."));
         
         canClose = false;
      }
      else if (retval == 0)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, mMsgFormat.c_str(),
             variableName.c_str(), wxT("Variable to be Minimized"),
             wxT("Variable, Array element, Spacecraft parameter"));
         
         canClose = false;
      }
      else
      {
         mVarNameChanged = false;
      }      
   }
   
   if (!canClose)
      return;
   
   //-----------------------------------------------------------------
   // save values to base, base code should do the range checking
   //-----------------------------------------------------------------
   try
   {
      mMinimizeCommand->SetStringParameter
         (mMinimizeCommand->GetParameterID(wxT("OptimizerName")),
          wxString(solverName.c_str()));
      
      mMinimizeCommand->SetStringParameter
         (mMinimizeCommand->GetParameterID(wxT("ObjectiveName")),
          wxString(variableName.c_str()));
          
      theGuiInterpreter->ValidateCommand(mMinimizeCommand);
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      canClose = false;
      return;
   }
}


//------------------------------------------------------------------------------
// void ShowGoalSetup()
//------------------------------------------------------------------------------
void MinimizePanel::ShowGoalSetup()
{
   wxString str;
   
   if (solverName == wxT(""))
   {
      mSolverComboBox->SetSelection(0);
      solverName = mSolverComboBox->GetStringSelection();
   }
   else
      mSolverComboBox->SetStringSelection(solverName);
      
   mVariableTextCtrl->SetValue(variableName);
}


//------------------------------------------------------------------------------
// void OnTextChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void MinimizePanel::OnTextChange(wxCommandEvent& event)
{
   if (mVariableTextCtrl->IsModified())
   {
      variableName = mVariableTextCtrl->GetValue();
      mVarNameChanged = true;
   }
   
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnSolverSelection(wxCommandEvent &event)
//------------------------------------------------------------------------------
void MinimizePanel::OnSolverSelection(wxCommandEvent &event)
{
   solverName = mSolverComboBox->GetStringSelection();
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnButtonClick(wxCommandEvent& event)
//------------------------------------------------------------------------------
void MinimizePanel::OnButtonClick(wxCommandEvent& event)
{
   if (event.GetEventObject() == mChooseButton)
   {      
      // show dialog to select parameter
      ParameterSelectDialog paramDlg(this, mObjectTypeList);
      paramDlg.ShowModal();

      if (paramDlg.IsParamSelected())
      {
         wxString newParamName = paramDlg.GetParamName();
         if (event.GetEventObject() == mChooseButton)
         {
            mVariableTextCtrl->SetValue(newParamName);
            variableName = newParamName;
         }
         
         EnableUpdate(true);
      }
   }
}

