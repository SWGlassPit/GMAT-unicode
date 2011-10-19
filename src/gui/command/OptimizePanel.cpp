//$Id: OptimizePanel.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                           OptimizePanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc.
//
// Author: Linda Jun (NASA/GSFC)
// Created: 2007/01/26
//
/**
 * This class contains the Optimize setup window.
 */
//------------------------------------------------------------------------------
#include "gmatwxdefs.hpp"
#include "OptimizePanel.hpp"
#include "MessageInterface.hpp"

//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(OptimizePanel, GmatPanel)
   EVT_COMBOBOX(ID_COMBO, OptimizePanel::OnComboBoxChange)
   EVT_BUTTON(ID_APPLYBUTTON, OptimizePanel::OnApplyButtonPress)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// OptimizePanel()
//------------------------------------------------------------------------------
/**
 * A constructor.
 */
//------------------------------------------------------------------------------
OptimizePanel::OptimizePanel(wxWindow *parent, GmatCommand *cmd)
   : GmatPanel(parent)
{
   theCommand = cmd;
   
   if (theCommand != NULL)
   {
      Create();
      Show();
   }
   else
   {
      // show error message
   }
}


//------------------------------------------------------------------------------
// ~OptimizePanel()
//------------------------------------------------------------------------------
OptimizePanel::~OptimizePanel()
{
   theGuiManager->UnregisterComboBox(wxT("Optimizer"), mSolverComboBox);
}

//-------------------------------
// private methods
//-------------------------------

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
void OptimizePanel::Create()
{    
   Integer bsize = 5;

   //-------------------------------------------------------
   // Solver ComboBox
   //-------------------------------------------------------
   wxStaticText *solverNameStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Solver Name"), wxDefaultPosition,
                       wxDefaultSize, 0);

   wxStaticText *solverModeStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Solver Mode"), wxDefaultPosition,
                       wxDefaultSize, 0);
   
   mSolverComboBox =
      theGuiManager->GetOptimizerComboBox(this, ID_COMBO, wxSize(180,-1));
   
   StringArray options = theCommand->GetStringArrayParameter(wxT("SolveModeOptions"));
   wxArrayString theOptions;
   
   for (StringArray::iterator i = options.begin(); i != options.end(); ++i)
      theOptions.Add(i->c_str());
   
   mSolverModeComboBox =
      new wxComboBox(this, ID_COMBO, wxT(""), wxDefaultPosition, wxSize(180,-1),
                     theOptions, wxCB_READONLY);
   
   mApplyCorrectionsButton = new wxButton(this, ID_APPLYBUTTON,
         wxT("Apply Corrections"));

   wxFlexGridSizer *pageSizer = new wxFlexGridSizer(2);
   
   pageSizer->Add(solverNameStaticText, 0, wxALIGN_CENTER|wxALL, bsize);
   pageSizer->Add(mSolverComboBox, 0, wxALIGN_CENTER|wxALL, bsize);
   pageSizer->Add(solverModeStaticText, 0, wxALIGN_CENTER|wxALL, bsize);
   pageSizer->Add(mSolverModeComboBox, 0, wxALIGN_CENTER|wxALL, bsize);
   pageSizer->Add(mApplyCorrectionsButton, 0, wxALIGN_CENTER|wxALL, bsize);
   
   theMiddleSizer->Add(pageSizer, 0, wxGROW, bsize);

}


//------------------------------------------------------------------------------
// void LoadData()
//------------------------------------------------------------------------------
void OptimizePanel::LoadData()
{
   try
   {
      // Set the pointer for the "Show Script" button
      mObject = theCommand;
      
      wxString solverName =
         theCommand->GetStringParameter(wxT("SolverName"));
      
      mSolverComboBox->SetValue(solverName.c_str());
      
      wxString solverMode =
               theCommand->GetStringParameter(wxT("SolveMode"));
      mSolverModeComboBox->SetValue(solverMode.c_str());
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
}


//------------------------------------------------------------------------------
// void SaveData()
//------------------------------------------------------------------------------
void OptimizePanel::SaveData()
{
   try
   {
      wxString solverName = mSolverComboBox->GetValue().c_str();
      wxString solverMode = mSolverModeComboBox->GetValue().c_str();

      theCommand->SetStringParameter(theCommand->GetParameterID(wxT("SolverName")),
                                     solverName);
      theCommand->SetStringParameter(theCommand->GetParameterID(wxT("SolveMode")), 
            solverMode);

      EnableUpdate(false);
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
}


//------------------------------------------------------------------------------
// void OnComboBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void OptimizePanel::OnComboBoxChange(wxCommandEvent& event)
{
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnApplyButtonPress(wxCommandEvent& event)
//------------------------------------------------------------------------------
/**
 * Method that updates variables with solution values.
 */
//------------------------------------------------------------------------------
void OptimizePanel::OnApplyButtonPress(wxCommandEvent& event)
{
   theCommand->TakeAction(wxT("ApplyCorrections"));
}
