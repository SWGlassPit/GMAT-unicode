//$Id: BeginFiniteBurnPanel.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                              BeginFiniteBurnPanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc.
// Author: Linda Jun
// Created: 2006/07/14
//
/**
 * This class contains the BeginFiniteBurn Setup window.
 */
//------------------------------------------------------------------------------

#include "BeginFiniteBurnPanel.hpp"
#include "ParameterSelectDialog.hpp"
#include "GmatStaticBoxSizer.hpp"
#include "StringTokenizer.hpp"          // for GetAllTokens()
#include "MessageInterface.hpp"
#include <algorithm>                    // for sort(), set_difference()
#include <wx/config.h>

//#define DEBUG_BEGINFBPANEL_CREATE
//#define DEBUG_BEGINFBPANEL_SAVE

//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(BeginFiniteBurnPanel, GmatPanel)
   EVT_BUTTON(ID_BUTTON_OK, GmatPanel::OnOK)
   EVT_BUTTON(ID_BUTTON_APPLY, GmatPanel::OnApply)
   EVT_BUTTON(ID_BUTTON_CANCEL, GmatPanel::OnCancel)
   EVT_BUTTON(ID_BUTTON_SCRIPT, GmatPanel::OnScript)
   EVT_BUTTON(ID_BUTTON, BeginFiniteBurnPanel::OnButtonClicked)
   EVT_COMBOBOX(ID_COMBOBOX, BeginFiniteBurnPanel::OnComboBoxChange)
   EVT_TEXT(ID_TEXTCTRL, BeginFiniteBurnPanel::OnTextUpdate)
END_EVENT_TABLE()

//------------------------------
// public methods
//------------------------------

//------------------------------------------------------------------------------
// BeginFiniteBurnPanel(wxWindow *parent)
//------------------------------------------------------------------------------
/**
 * Constructs BeginFiniteBurnPanel object.
 *
 * @param <parent> input parent.
 *
 * @note Creates the maneuver dialog box
 */
//------------------------------------------------------------------------------
BeginFiniteBurnPanel::BeginFiniteBurnPanel(wxWindow *parent, GmatCommand *cmd)
   : GmatPanel(parent)

{
   theCommand = cmd;
   mObjectTypeList.Add(wxT("Spacecraft"));
   
   if (theCommand != NULL)
   {
      Create();
      Show();
   }
}


//------------------------------------------------------------------------------
// ~BeginFiniteBurnPanel()
//------------------------------------------------------------------------------
BeginFiniteBurnPanel::~BeginFiniteBurnPanel()
{
   theGuiManager->UnregisterComboBox(wxT("FiniteBurn"), mFiniteBurnComboBox);
}


//-------------------------------
// protected methods
//-------------------------------

//------------------------------------------------------------------------------
// wxArrayString ToWxArrayString(const StringArray &array)
//------------------------------------------------------------------------------
/**
 * Converts wxString array to wxString array.
 */
//------------------------------------------------------------------------------
wxArrayString BeginFiniteBurnPanel::ToWxArrayString(const StringArray &array)
{
   wxArrayString newArray;
   for (UnsignedInt i=0; i<array.size(); i++)
      newArray.Add(array[i].c_str());

   return newArray;
}


//------------------------------------------------------------------------------
// wxString ToWxString(const wxArrayString &names)
//------------------------------------------------------------------------------
/**
 * Converts wxString array to wxString separated by comma.
 */
//------------------------------------------------------------------------------
wxString BeginFiniteBurnPanel::ToWxString(const wxArrayString &names)
{
   wxString str = wxT("");
   wxString delimiter = wxT(", ");
   if (names.Count() > 0)
   {
      str = names[0];
      
      for (unsigned int i=1; i<names.Count(); i++)
         str = str + delimiter + names[i];
   }
   
   return str;
}


//----------------------------------
// methods inherited from GmatPanel
//----------------------------------

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
/**
 *
 * @note Creates the panel for the Maneuver Command
 */
//------------------------------------------------------------------------------
void BeginFiniteBurnPanel::Create()
{
   #ifdef DEBUG_BEGINFBPANEL_CREATE
   MessageInterface::ShowMessage
      (wxT("BeginFiniteBurnPanel::Create() Entered. command=%s\n"),
       theCommand->GetTypeName().c_str());
   #endif
   
   int bsize = 3;
   // get the config object
   wxConfigBase *pConfig = wxConfigBase::Get();
   // SetPath() understands ".."
   pConfig->SetPath(wxT("/Begin Finite Burn"));

   
   //----------------------------------------------------------------------
   // Burns
   //----------------------------------------------------------------------
   // create burn label
   wxStaticText *burnLabel =
      new wxStaticText(this, ID_TEXT,
                       wxT(GUI_ACCEL_KEY) wxT("Burn"), wxDefaultPosition, wxSize(50, -1));
   
   #ifdef DEBUG_BEGINFBPANEL_CREATE
   MessageInterface::ShowMessage
      (wxT("BeginFiniteBurnPanel::Create() Calling theGuiManager->")
       wxT("GetFiniteBurnComboBox()\n"));
   #endif
   
   // create finite burn combo box
   mFiniteBurnComboBox =
      theGuiManager->GetFiniteBurnComboBox(this, ID_COMBOBOX, wxSize(150,-1));
   mFiniteBurnComboBox->SetToolTip(pConfig->Read(wxT("BurnHint")));
   
   //----------------------------------------------------------------------
   // Spacecraft
   //----------------------------------------------------------------------
   
   wxStaticText *satLabel =
      new wxStaticText(this, ID_TEXT,
                       wxT(GUI_ACCEL_KEY) wxT("Spacecraft"), wxDefaultPosition, wxSize(50,-1));
   mSatTextCtrl =
      new wxTextCtrl( this, ID_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(150,-1));
   mSatTextCtrl->SetToolTip(pConfig->Read(wxT("SpacecraftHint")));
   
   wxButton *selectSatButton =
      new wxButton(this, ID_BUTTON, wxT("E") wxT(GUI_ACCEL_KEY) wxT("dit"), wxDefaultPosition, wxDefaultSize);
   selectSatButton->SetToolTip(pConfig->Read(wxT("SelectSpacecraftHint")));
   
   //----------------------------------------------------------------------
   // add items to page sizer
   //----------------------------------------------------------------------
   wxFlexGridSizer *burnSatSizer = new wxFlexGridSizer(3, 0, 0);
   burnSatSizer->Add(burnLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   burnSatSizer->Add(mFiniteBurnComboBox, 0, wxALIGN_LEFT|wxALL, bsize);
   burnSatSizer->Add(20, 20);
   burnSatSizer->Add(satLabel, 0, wxALIGN_LEFT|wxALL, bsize);
   burnSatSizer->Add(mSatTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
   burnSatSizer->Add(selectSatButton, 0, wxALIGN_LEFT|wxALL, bsize);
   
   GmatStaticBoxSizer *optionsSizer = new GmatStaticBoxSizer( wxVERTICAL, this, wxT("Options") );
   optionsSizer->Add(burnSatSizer, 0, wxGROW|wxALIGN_LEFT|wxALL, 6);
   wxBoxSizer *pageSizer = new wxBoxSizer(wxVERTICAL);
   pageSizer->Add(optionsSizer, 1, wxALIGN_CENTER|wxGROW|wxALL, 6);
   
   // add to middle sizer
   theMiddleSizer->Add(pageSizer, 1, wxGROW|wxALIGN_CENTRE|wxALL, bsize);     
   
   #ifdef DEBUG_BEGINFBPANEL_CREATE
   MessageInterface::ShowMessage(wxT("BeginFiniteBurnPanel::Create() Exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
// virtual void LoadData()
//------------------------------------------------------------------------------
void BeginFiniteBurnPanel::LoadData()
{
   // Set the pointer for the "Show Script" button
   mObject = theCommand;
   
   try
   {
      // Get FiniteBurn from the command
      wxString burnName = theCommand->GetRefObjectName(Gmat::FINITE_BURN);
      
      #if DEBUG_BEGIN_FINITE_BURN
      MessageInterface::ShowMessage
         (wxT("BeginFiniteBurnPanel::LoadData() burnName=<%s>\n"), burnName.c_str());
      #endif
      
      mFiniteBurnComboBox->SetValue(burnName.c_str());
      
      // Get spacecraft list from the command
      StringArray scNames = theCommand->GetRefObjectNameArray(Gmat::SPACECRAFT);
      mSpacecraftList = ToWxArrayString(scNames);
      wxString scList = ToWxString(mSpacecraftList);
      mSatTextCtrl->SetValue(scList);
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
}


//------------------------------------------------------------------------------
// virtual void SaveData()
//------------------------------------------------------------------------------
void BeginFiniteBurnPanel::SaveData()
{
   #ifdef DEBUG_BEGINFBPANEL_SAVE
   MessageInterface::ShowMessage(wxT("BeginFiniteBurnPanel::SaveData() entered\n"));
   #endif
   
   canClose = true;
   wxString satNames = mSatTextCtrl->GetValue();
   
   //-----------------------------------------------------------------
   // check empty spacecraft list
   //-----------------------------------------------------------------
   if (satNames == wxT(""))
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_, wxT("Please enter Spacecrafts to begin maneuver\n"));
      canClose = false;
      return;
   }
   
   // In case user typed in spacecraft names, get value from textbox and
   // parse by blank or comma
   wxString scNames = satNames.c_str();
   StringTokenizer st(scNames, wxT(" ,"));
   StringArray scList = st.GetAllTokens();
   #ifdef DEBUG_BEGINFBPANEL_SAVE
   for (UnsignedInt i=0; i<scList.size(); i++)
      MessageInterface::ShowMessage
         (wxT("   selected spacecraft[%d] = '%s'\n"), i, scList[i].c_str());
   #endif
   
   //-----------------------------------------------------------------
   // check unknown spacecraft names
   //-----------------------------------------------------------------
   StringArray configList = theGuiInterpreter->GetListOfObjects(Gmat::SPACECRAFT);
   StringArray result;
   sort(scList.begin(), scList.end());
   sort(configList.begin(), configList.end());
   set_difference(scList.begin(), scList.end(), configList.begin(),
                  configList.end(), back_inserter(result));
   
   #ifdef DEBUG_BEGINFBPANEL_SAVE
   for (UnsignedInt i=0; i<result.size(); i++)
      MessageInterface::ShowMessage(wxT("   sc not configured[%d] = '%s'\n"), i, result[i].c_str());
   #endif
   
   if (result.size() > 0)
   {
      wxString scLabel = wxT("The spacecraft \"");
      wxString desc = wxT("\" is undefined.\n");
      if (result.size() > 1)
      {
         scLabel = wxT("The spacecrafts \"");
         desc = wxT("\" are undefined.\n");
      }
      
      wxString unknownSc = (ToWxString(ToWxArrayString(result))).c_str();
      wxString msg = scLabel + unknownSc + desc;
      MessageInterface::PopupMessage(Gmat::ERROR_, msg);
      canClose = false;
      return;
   }
   
   //-----------------------------------------------------------------
   // save values to base, base code should do the range checking
   //-----------------------------------------------------------------
   try
   {
      // save finite burn
      wxString burnString = mFiniteBurnComboBox->GetValue();
      theCommand->SetRefObjectName(Gmat::FINITE_BURN, burnString.c_str());
      
      // save spacecrafts
      int count = scList.size();
      theCommand->TakeAction(wxT("Clear"));
      
      for (int i=0; i<count; i++)
         theCommand->SetRefObjectName(Gmat::SPACECRAFT, scList[i]);
      
      mSpacecraftList = ToWxArrayString(scList);
      
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
   
   #ifdef DEBUG_BEGINFBPANEL_SAVE
   MessageInterface::ShowMessage(wxT("BeginFiniteBurnPanel::SaveData() exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
// void OnButtonClicked(wxCommandEvent& event)
//------------------------------------------------------------------------------
void BeginFiniteBurnPanel::OnButtonClicked(wxCommandEvent& event)
{
   // Allow multiple selection on spacecraft
   ParameterSelectDialog paramDlg(this, mObjectTypeList,
                                  GuiItemManager::SHOW_WHOLE_OBJECT_ONLY, true,
                                  false, true, false, false, false, wxT("Spacecraft"));
   
   paramDlg.SetParamNameArray(mSpacecraftList);
   paramDlg.ShowModal();
   
   if (paramDlg.HasSelectionChanged())
   {
      EnableUpdate(true);
      wxArrayString satNames = paramDlg.GetParamNameArray();
      wxString value = ToWxString(satNames);
      mSatTextCtrl->SetValue(value);
   }
}


//------------------------------------------------------------------------------
// void OnComboBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void BeginFiniteBurnPanel::OnComboBoxChange(wxCommandEvent& event)
{
   EnableUpdate(true);
}

//------------------------------------------------------------------------------
// void OnTextUpdate(wxCommandEvent& event)
//------------------------------------------------------------------------------
void BeginFiniteBurnPanel::OnTextUpdate(wxCommandEvent& event)
{
   EnableUpdate(true);
}


