//$Id: VaryPanel.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                                   VaryPanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Linda Jun
// Created: 2004/10/12
//
/**
 * This class contains the setup for Target Vary command.
 */
//------------------------------------------------------------------------------

#include "gmatwxdefs.hpp"
#include "GmatAppData.hpp"
#include "ParameterSelectDialog.hpp"
#include "VaryPanel.hpp"
#include "Solver.hpp"
#include "GmatBaseException.hpp"
#include "gmatdefs.hpp"
#include "GuiInterpreter.hpp"
#include "MessageInterface.hpp"
#include "GmatStaticBoxSizer.hpp"

//#define DEBUG_VARYPANEL_LOAD
//#define DEBUG_VARYPANEL_SAVE
//#define DEBUG_VARYPANEL_SOLVER

//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(VaryPanel, GmatPanel)
   EVT_BUTTON(ID_BUTTON, VaryPanel::OnButton)
   EVT_TEXT(ID_TEXTCTRL, VaryPanel::OnTextChange)
   EVT_COMBOBOX(ID_COMBO, VaryPanel::OnSolverSelection)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// VaryPanel(wxWindow *parent, GmatCommand *cmd, bool inOptimize)
//------------------------------------------------------------------------------
/**
 * A constructor.
 */
//------------------------------------------------------------------------------
VaryPanel::VaryPanel(wxWindow *parent, GmatCommand *cmd, bool inOptimize)
   : GmatPanel(parent)
{
   #ifdef DEBUG_VARYPANEL
   MessageInterface::ShowMessage
      (wxT("VaryPanel::VaryPanel() entered, cmd=<%p><%s>, inOptimize=%d\n"),
       cmd, cmd ? cmd->GetTypeName().c_str() : wxT("NULL"), inOptimize);
   #endif
   
   mVaryCommand = (Vary *)cmd;
   inOptimizeCmd = inOptimize;
   
   mObjectTypeList.Add(wxT("Spacecraft"));
   mObjectTypeList.Add(wxT("ImpulsiveBurn"));
   
   Create();
   Show();
   
   solverChanged = false;
   variableChanged = false;
   EnableUpdate(false);
   
   #ifdef DEBUG_VARYPANEL
   MessageInterface::ShowMessage(wxT("VaryPanel::VaryPanel() leaving\n"));
   #endif
}

//------------------------------------------------------------------------------
// ~VaryPanel()
//------------------------------------------------------------------------------
VaryPanel::~VaryPanel()
{
   mObjectTypeList.Clear();
   theGuiManager->UnregisterComboBox(wxT("Solver"), mSolverComboBox);
}

//-------------------------------
// private methods
//-------------------------------

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
void VaryPanel::Create()
{
   int bsize = 2; // bordersize
   
   // Targeter
   wxStaticText *solverStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Solver"),
                       wxDefaultPosition, wxSize(40, -1), 0);
   
   // Show all user-defined Solvers
   mSolverComboBox =
      theGuiManager->GetSolverComboBox(this, ID_COMBO, wxSize(180,-1));
   
   // Variable
   wxStaticText *varStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Variable"), 
                       wxDefaultPosition, wxSize(55, -1), 0);
   mVarNameTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                                     wxDefaultPosition, wxSize(250,-1), 0);
   mViewVarButton = new
      wxButton(this, ID_BUTTON, wxT("Edit"), wxDefaultPosition, wxDefaultSize, 0);
   
   // Initial Value
   wxStaticText *initialStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Initial Value"), 
                       wxDefaultPosition, wxDefaultSize, 0);
   mInitialTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                                     wxDefaultPosition, wxSize(100,-1), 0);
   
   // Perturbation
   pertStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Perturbation"), 
                       wxDefaultPosition, wxDefaultSize, 0);
   mPertTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                                  wxDefaultPosition, wxSize(100,-1), 0);
   
   // Lower
   lowerValueStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Lower"), 
                       wxDefaultPosition, wxDefaultSize, 0);
   mLowerValueTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                                   wxDefaultPosition, wxSize(100,-1), 0);

   // Upper
   upperValueStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Upper"), 
                       wxDefaultPosition, wxDefaultSize, 0);
   mUpperValueTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                                   wxDefaultPosition, wxSize(100,-1), 0);

   // Max Step
   maxStepStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Max Step"), 
                       wxDefaultPosition, wxDefaultSize, 0);
   mMaxStepTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                                 wxDefaultPosition, wxSize(100,-1), 0);

   // Additive Scale Factor
   additiveStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Additive Scale Factor"), 
                       wxDefaultPosition, wxDefaultSize, 0);
   mAdditiveTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                                   wxDefaultPosition, wxSize(100,-1), 0);

   // Multiplicative Scale Factor
   multiplicativeStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Multiplicative Scale Factor"), 
                       wxDefaultPosition, wxDefaultSize, 0);
   mMultiplicativeTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                                   wxDefaultPosition, wxSize(100,-1), 0);
   
   // wx*Sizers
   wxBoxSizer *panelSizer = new wxBoxSizer(wxVERTICAL);
   GmatStaticBoxSizer *varSetupSizer = new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Variable Setup"));
   wxFlexGridSizer *valueGridSizer = new wxFlexGridSizer(6, 0, 0);
   wxBoxSizer *solverBoxSizer = new wxBoxSizer(wxHORIZONTAL);
   wxBoxSizer *variableBoxSizer = new wxBoxSizer(wxHORIZONTAL);
   wxFlexGridSizer *scaleGridSizer = new wxFlexGridSizer(2, 0, 0);
   
   // Add to wx*Sizers
   
   solverBoxSizer->Add(solverStaticText, 0, wxALIGN_LEFT|wxALL, bsize);
   solverBoxSizer->Add(mSolverComboBox, 0, wxALIGN_LEFT|wxALL, bsize);
   
   variableBoxSizer->Add(varStaticText, 0, wxALIGN_LEFT|wxALL, bsize);
   variableBoxSizer->Add(mVarNameTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
   variableBoxSizer->Add(mViewVarButton, 0, wxALIGN_LEFT|wxALL, bsize);
   
   valueGridSizer->Add(40, 20, 0, wxALIGN_LEFT|wxALL, bsize);
   valueGridSizer->Add(initialStaticText, 0, wxALIGN_CENTER|wxALL, bsize);
   valueGridSizer->Add(pertStaticText, 0, wxALIGN_CENTER|wxALL, bsize);
   valueGridSizer->Add(lowerValueStaticText, 0, wxALIGN_CENTER|wxALL, bsize);
   valueGridSizer->Add(upperValueStaticText, 0, wxALIGN_CENTER|wxALL, bsize);
   valueGridSizer->Add(maxStepStaticText, 0, wxALIGN_CENTER|wxALL, bsize);
   
   valueGridSizer->Add(40, 20, 0, wxALIGN_CENTER|wxALL, bsize);
   valueGridSizer->Add(mInitialTextCtrl, 0, wxALIGN_CENTER|wxALL, bsize);
   valueGridSizer->Add(mPertTextCtrl, 0, wxALIGN_CENTER|wxALL, bsize);
   valueGridSizer->Add(mLowerValueTextCtrl, 0, wxALIGN_CENTER|wxALL, bsize);
   valueGridSizer->Add(mUpperValueTextCtrl, 0, wxALIGN_CENTER|wxALL, bsize);
   valueGridSizer->Add(mMaxStepTextCtrl, 0, wxALIGN_CENTER|wxALL, bsize);

   scaleGridSizer->Add(additiveStaticText, 0, wxALIGN_LEFT|wxALL, bsize);
   scaleGridSizer->Add(mAdditiveTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
   scaleGridSizer->Add(multiplicativeStaticText, 0, wxALIGN_LEFT|wxALL, bsize);
   scaleGridSizer->Add(mMultiplicativeTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);

   varSetupSizer->Add(variableBoxSizer, 0, wxALIGN_LEFT|wxALL, bsize);
   varSetupSizer->Add(valueGridSizer, 0, wxALIGN_LEFT|wxALL, bsize);

   panelSizer->Add(solverBoxSizer, 0, wxGROW|wxALIGN_CENTER|wxALL, bsize);
   panelSizer->Add(varSetupSizer, 0, wxGROW|wxALIGN_CENTER|wxALL, bsize);
   panelSizer->Add(scaleGridSizer, 0, wxGROW|wxALIGN_CENTER|wxALL, bsize);

   theMiddleSizer->Add(panelSizer, 0, wxGROW|wxALIGN_CENTER|wxALL, bsize);
}

//------------------------------------------------------------------------------
// void LoadData()
//------------------------------------------------------------------------------
void VaryPanel::LoadData()
{
   #ifdef DEBUG_VARYPANEL_LOAD
   MessageInterface::ShowMessage(wxT("VaryPanel::LoadData() entered\n"));
   MessageInterface::ShowMessage
      (wxT("   Command=<%p>'%s'\n"), mVaryCommand, mVaryCommand ?
       mVaryCommand->GetTypeName().c_str() : wxT("NULL"));
   #endif
   
   mVarNameTextCtrl->Disable(); // we don't want user to edit this box
   mViewVarButton->Enable();
   
   if (mVaryCommand == NULL)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, wxT("The Vary command is NULL\n"));
      return;
   }
   
   try
   {
      // Set the pointer for the "Show Script" button
      mObject = mVaryCommand;
      
      solverName =
         mVaryCommand->GetStringParameter(mVaryCommand->GetParameterID(wxT("SolverName")));
      
      #ifdef DEBUG_VARYPANEL_LOAD
      MessageInterface::ShowMessage(wxT("   solverName=%s\n"), solverName.c_str());
      #endif
      
      variableName =
         mVaryCommand->GetStringParameter(mVaryCommand->GetParameterID(wxT("Variable")));
      
      #ifdef DEBUG_VARYPANEL_LOAD
      MessageInterface::ShowMessage(wxT("   variableName=%s\n"), variableName.c_str());
      #endif
      
      wxString initValStr = 
         mVaryCommand->GetStringParameter(mVaryCommand->GetParameterID(wxT("InitialValue"))).c_str();
      wxString pertStr = 
         mVaryCommand->GetStringParameter(mVaryCommand->GetParameterID(wxT("Perturbation"))).c_str();
      wxString lowerStr = 
         mVaryCommand->GetStringParameter(mVaryCommand->GetParameterID(wxT("Lower"))).c_str();
      wxString upperStr = 
         mVaryCommand->GetStringParameter(mVaryCommand->GetParameterID(wxT("Upper"))).c_str();      
      wxString maxStepStr = 
         mVaryCommand->GetStringParameter(mVaryCommand->GetParameterID(wxT("MaxStep"))).c_str();
      wxString addSfStr = 
         mVaryCommand->GetStringParameter(mVaryCommand->GetParameterID(wxT("AdditiveScaleFactor"))).c_str();
      wxString multCfStr =
         mVaryCommand->GetStringParameter(mVaryCommand->GetParameterID(wxT("MultiplicativeScaleFactor"))).c_str();
      
      mSolverComboBox->SetStringSelection(solverName.c_str());
      mVarNameTextCtrl->SetValue(variableName.c_str());
      
      mInitialTextCtrl->SetValue(initValStr);
      mPertTextCtrl->SetValue(pertStr);
      mLowerValueTextCtrl->SetValue(lowerStr);
      mUpperValueTextCtrl->SetValue(upperStr);
      mMaxStepTextCtrl->SetValue(maxStepStr);
      mAdditiveTextCtrl->SetValue(addSfStr);
      mMultiplicativeTextCtrl->SetValue(multCfStr);
      
      //  Enalbe or disable fields depends on the solver type
      GmatBase *solver = theGuiInterpreter->GetConfiguredObject(solverName);
      if (solver != NULL)
      {
         mVaryCommand->SetRefObject(solver, Gmat::SOLVER, solverName);
         solver->SetStringParameter
            (solver->GetParameterID(wxT("Variables")), variableName);
         SetControlEnabling(solver);
      }
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
   
   #ifdef DEBUG_VARYPANEL_LOAD
   MessageInterface::ShowMessage(wxT("VaryPanel::LoadData() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void SaveData()
//------------------------------------------------------------------------------
void VaryPanel::SaveData()
{   
   #ifdef DEBUG_VARYPANEL_SAVE
   MessageInterface::ShowMessage(wxT("VaryPanel::SaveData() entered\n"));
   #endif
   
   canClose = true;
   wxString strInitVal, strPert, strLower, strUpper, strMaxStep;
   wxString strAddSf, strMultSf;
   
   //-----------------------------------------------------------------
   // check input values: Number, Variable, Array element, Parameter
   //-----------------------------------------------------------------
   
   wxString expRange = wxT("Real Number, Variable, Array element, Plottable Parameter");
   
   // Any plottable Parameters allowed, so use UNKNOWN_OBJECT
   if (mInitialTextCtrl->IsModified())
   {
      strInitVal = mInitialTextCtrl->GetValue().c_str();
      CheckVariable(strInitVal, Gmat::UNKNOWN_OBJECT,
                    wxT("InitialValue"), expRange, true);
   }
   
   if (mPertTextCtrl->IsModified())
   {
      strPert = mPertTextCtrl->GetValue().c_str();
      CheckVariable(strPert, Gmat::UNKNOWN_OBJECT,
                    wxT("Perturbation"), expRange, true);
   }
   
   if (mLowerValueTextCtrl->IsModified())
   {
      strLower = mLowerValueTextCtrl->GetValue().c_str();
      CheckVariable(strLower, Gmat::UNKNOWN_OBJECT,
                    wxT("Lower"), expRange, true);
   }
   
   if (mUpperValueTextCtrl->IsModified())
   {
      strUpper = mUpperValueTextCtrl->GetValue().c_str();
      CheckVariable(strUpper.c_str(), Gmat::UNKNOWN_OBJECT,
                    wxT("Upper"), expRange, true);
   }
   
   if (mMaxStepTextCtrl->IsModified())
   {
      strMaxStep = mMaxStepTextCtrl->GetValue().c_str();
      CheckVariable(strMaxStep.c_str(), Gmat::UNKNOWN_OBJECT,
                    wxT("MaxStep"), expRange, true);
   }
   
   if (mAdditiveTextCtrl->IsModified())
   {
      strAddSf = mAdditiveTextCtrl->GetValue().c_str();
      CheckVariable(strAddSf.c_str(), Gmat::UNKNOWN_OBJECT,
                    wxT("AdditiveScaleFactor"), expRange, true);
   }
   
   if (mMultiplicativeTextCtrl->IsModified())
   {
      strMultSf = mMultiplicativeTextCtrl->GetValue().c_str();
      CheckVariable(strMultSf.c_str(), Gmat::UNKNOWN_OBJECT,
                    wxT("MultiplicativeScaleFactor"), expRange, true);
   }
   
   if (!canClose)
      return;
   
   //-----------------------------------------------------------------
   // save values to base, base code should do the range checking
   //-----------------------------------------------------------------
   
   #ifdef DEBUG_VARYPANEL_SAVE
   MessageInterface::ShowMessage(wxT("   solverName=%s, variableName=%s\n"),
       solverName.c_str(), variableName.c_str());
   #endif
   
   Solver *solver = (Solver*)theGuiInterpreter->GetConfiguredObject(solverName);
   
   if (solver == NULL)
      throw GmatBaseException(wxT("Cannot find the solver: ") + solverName);
   
   bool validateCommand = false;
   
   try
   {
      if (solverChanged)
      {
         #ifdef DEBUG_VARYPANEL_SAVE
         MessageInterface::ShowMessage
            (wxT("   Solver changed, solver=<%p>'%s'\n"), solver, solver->GetName().c_str());
         #endif
         mVaryCommand->SetStringParameter(wxT("SolverName"), solverName);
         mVaryCommand->SetRefObject(solver, Gmat::SOLVER, solverName);
         solverChanged = false;
      }
      
      if (variableChanged)
      {
         #ifdef DEBUG_VARYPANEL_SAVE
         MessageInterface::ShowMessage
            (wxT("   Variable changed, variableName='%s'\n"), variableName.c_str());
         #endif
         validateCommand = true;
         mVaryCommand->SetStringParameter(wxT("Variable"), variableName);
         solver->SetStringParameter(wxT("Variables"), variableName);
         variableChanged = false;
      }
      
      if (mInitialTextCtrl->IsModified())
      {
         validateCommand = true;
         mVaryCommand->SetStringParameter(wxT("InitialValue"), strInitVal.c_str());
         mInitialTextCtrl->DiscardEdits();
      }
      
      if (mPertTextCtrl->IsModified())
      {
         validateCommand = true;
         mVaryCommand->SetStringParameter(wxT("Perturbation"), strPert.c_str());
         mPertTextCtrl->DiscardEdits();
      }
      
      if (mLowerValueTextCtrl->IsModified())
      {
         validateCommand = true;
         mVaryCommand->SetStringParameter(wxT("Lower"), strLower.c_str());
         mLowerValueTextCtrl->DiscardEdits();
      }
      
      if (mUpperValueTextCtrl->IsModified())
      {
         validateCommand = true;
         mVaryCommand->SetStringParameter(wxT("Upper"), strUpper.c_str());
         mUpperValueTextCtrl->DiscardEdits();
      }
      
      if (mMaxStepTextCtrl->IsModified())
      {
         validateCommand = true;
         mVaryCommand->SetStringParameter(wxT("MaxStep"), strMaxStep.c_str());
         mMaxStepTextCtrl->DiscardEdits();
      }
      
      if (mAdditiveTextCtrl->IsModified())
      {
         validateCommand = true;
         mVaryCommand->SetStringParameter(wxT("AdditiveScaleFactor"), strAddSf.c_str());
         mAdditiveTextCtrl->DiscardEdits();
      }
      
      if (mMultiplicativeTextCtrl->IsModified())
      {
         validateCommand = true;
         mVaryCommand->SetStringParameter(wxT("MultiplicativeScaleFactor"), strMultSf.c_str());
         mMultiplicativeTextCtrl->DiscardEdits();
      }
      
      // avoid unnecessary validation since it clears all wrappers and recreates them
      if (validateCommand)
      {
         #ifdef DEBUG_VARYPANEL_SAVE
         MessageInterface::ShowMessage(wxT("   Calling ValidateCommand()\n"));
         #endif
         
         if (!theGuiInterpreter->ValidateCommand(mVaryCommand))
            canClose = false;
      }
      
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      canClose = false;
   }
   
   #ifdef DEBUG_VARYPANEL_SAVE
   MessageInterface::ShowMessage(wxT("VaryPanel::SaveData() leaving\n"));
   #endif
}


//---------------------------------
// event handling
//---------------------------------

//------------------------------------------------------------------------------
// void VaryPanel::OnTextChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void VaryPanel::OnTextChange(wxCommandEvent& event)
{
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void VaryPanel::OnSolverSelection(wxCommandEvent &event)
//------------------------------------------------------------------------------
void VaryPanel::OnSolverSelection(wxCommandEvent &event)
{
   #ifdef DEBUG_VARYPANEL_SOLVER
   MessageInterface::ShowMessage(wxT("VaryPanel::OnSolverSelection() entered\n"));
   #endif
   
   solverName = mSolverComboBox->GetStringSelection().c_str();
   
   GmatBase *slvr = theGuiInterpreter->GetConfiguredObject(solverName);
   
   #ifdef DEBUG_VARYPANEL_SOLVER
   MessageInterface::ShowMessage
      (wxT("   solverName='%s', solver=<%p>'%s'\n"), solverName.c_str(), slvr,
       slvr ? slvr->GetName().c_str() : wxT("NULL"));
   #endif
   
   if (slvr == NULL)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, wxT("The solver ") + solverName + wxT(" is NULL"));
   }
   else
   {
      solverChanged = true;
      SetControlEnabling(slvr);
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void SetControlEnabling(GmatBase *slvr)
//------------------------------------------------------------------------------
/**
 * Enables and disables variable controls based on what the solver supports.
 * 
 * @param slvr The Solver that the Vary command configured from this panel uses.
 */
//------------------------------------------------------------------------------
void VaryPanel::SetControlEnabling(GmatBase *slvr)
{
   #ifdef DEBUG_SET_CONTROL
   MessageInterface::ShowMessage
      (wxT("VaryPanel::SetControlEnabling() entered, solver=<%p><%s>'%s'\n"), slvr,
       slvr->GetTypeName().c_str(), slvr->GetName().c_str());
   #endif
   if (slvr->GetBooleanParameter(slvr->GetParameterID(wxT("AllowScaleSetting"))))
   {
      additiveStaticText->Enable(true);
      multiplicativeStaticText->Enable(true);
      mAdditiveTextCtrl->Enable(true);
      mMultiplicativeTextCtrl->Enable(true);
   }
   else
   {
      additiveStaticText->Enable(false);
      multiplicativeStaticText->Enable(false);
      mAdditiveTextCtrl->Enable(false);
      mMultiplicativeTextCtrl->Enable(false);
   }
   
   if (slvr->GetBooleanParameter(slvr->GetParameterID(wxT("AllowRangeSettings"))))
   {
      lowerValueStaticText->Enable(true);
      mLowerValueTextCtrl->Enable(true);
      upperValueStaticText->Enable(true);
      mUpperValueTextCtrl->Enable(true);
   }
   else // in target
   {
      lowerValueStaticText->Enable(false);
      mLowerValueTextCtrl->Enable(false);
      upperValueStaticText->Enable(false);
      mUpperValueTextCtrl->Enable(false);
   }      
   
   if (slvr->GetBooleanParameter(slvr->GetParameterID(wxT("AllowStepsizeSetting"))))
   {
      maxStepStaticText->Enable(true);
      mMaxStepTextCtrl->Enable(true);
   }
   else
   {
      maxStepStaticText->Enable(false);
      mMaxStepTextCtrl->Enable(false);
   }
   
   if (slvr->GetBooleanParameter(slvr->GetParameterID(wxT("AllowVariablePertSetting"))))
   {
      pertStaticText->Enable(true);
      mPertTextCtrl->Enable(true);
   }
   else
   {
      pertStaticText->Enable(false);
      mPertTextCtrl->Enable(false);
   }
   #ifdef DEBUG_SET_CONTROL
   MessageInterface::ShowMessage
      (wxT("VaryPanel::SetControlEnabling() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void VaryPanel::OnButton(wxCommandEvent& event)
//------------------------------------------------------------------------------
void VaryPanel::OnButton(wxCommandEvent& event)
{
   if (event.GetEventObject() == mViewVarButton)  
   {
      wxString objType = wxT("ImpulsiveBurn");
      if (theGuiManager->GetNumImpulsiveBurn() == 0)
          objType = wxT("Spacecraft");
      
      ParameterSelectDialog paramDlg(this, mObjectTypeList,
                                     GuiItemManager::SHOW_SETTABLE, false,
                                     false, false, true, true, true, objType);
      
      paramDlg.ShowModal();
      
      if (paramDlg.IsParamSelected())
      {
         wxString newParamName = paramDlg.GetParamName();
         mVarNameTextCtrl->SetValue(newParamName);
         variableName = newParamName.c_str();
         variableChanged = true;
         EnableUpdate(true);
      }
   }
   else
      event.Skip();
}

