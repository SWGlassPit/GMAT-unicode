//$Id: UniversePanel.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                              UniversePanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P.
//
//
// Author: Monisha Butler
// Created: 2003/09/10
// Modified: 2004/01/13 by Allison Greene for action handling
/**
 * This class allows user to specify where Universe information is 
 * coming from
 */
//------------------------------------------------------------------------------
#include "UniversePanel.hpp"
#include "MessageInterface.hpp"
#include "ShowScriptDialog.hpp"
#include "StringUtil.hpp"
#include <fstream>
#include "bitmaps/OpenFolder.xpm"
#include <wx/config.h>

//#define DEBUG_UNIVERSEPANEL_CREATE
//#define DEBUG_UNIVERSEPANEL_LOAD
//#define DEBUG_UNIVERSEPANEL_SAVE

//------------------------------
// event tables for wxWindows
//------------------------------
BEGIN_EVENT_TABLE(UniversePanel, GmatPanel)
   EVT_BUTTON(ID_BUTTON_OK, GmatPanel::OnOK)
   EVT_BUTTON(ID_BUTTON_APPLY, GmatPanel::OnApply)
   EVT_BUTTON(ID_BUTTON_CANCEL, GmatPanel::OnCancel)
   EVT_BUTTON(ID_BUTTON_SCRIPT, GmatPanel::OnScript)

   EVT_BUTTON(ID_BUTTON_BROWSE, UniversePanel::OnBrowseButton)
   EVT_BUTTON(ID_LSK_BUTTON_BROWSE, UniversePanel::OnLSKBrowseButton)
   EVT_COMBOBOX(ID_COMBOBOX, UniversePanel::OnComboBoxChange)
   EVT_CHECKBOX(ID_CHECKBOX, UniversePanel::OnCheckBoxChange)
   EVT_TEXT(ID_TEXT_CTRL, UniversePanel::OnTextCtrlChange)

END_EVENT_TABLE()
   
   
//---------------------------------
// public methods
//---------------------------------
//------------------------------------------------------------------------------
// UniversePanel(wxWindow *parent)
//------------------------------------------------------------------------------
/**
 * Constructs UniversePanel object.
 *
 * @param <parent> input parent.
 *
 * @note Creates the Universe GUI
 */
//------------------------------------------------------------------------------
UniversePanel::UniversePanel(wxWindow *parent):GmatPanel(parent)
{
   mHasFileTypesInUseChanged = false;
   mHasFileNameChanged = false;
   mHasLSKFileNameChanged = false;
   mHasTextModified = false;
   theSolarSystem = NULL;
   
   // get solar system in use
   theSolarSystem = theGuiInterpreter->GetSolarSystemInUse();
   if (theSolarSystem == NULL)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, wxT("The Solar System is NULL"));
   }
   else
   {
      #ifdef DEBUG_UNIVERSEPANEL_CREATE
      MessageInterface::ShowMessage
         (wxT("UniversePanel::UniversePanel() theSolarSystem=<%p>'%s'\n"), theSolarSystem,
          theSolarSystem->GetName().c_str());
      #endif
      
      Create();
      Show();
   }
}


//------------------------------------------------------------------------------
// ~UniversePanel()
//------------------------------------------------------------------------------
UniversePanel::~UniversePanel()
{
}


//------------------------------------------------------------------------------
// void OnScript()
//------------------------------------------------------------------------------
/**
 * Shows Scripts
 */
//------------------------------------------------------------------------------
void UniversePanel::OnScript(wxCommandEvent &event)
{
   wxString title = wxT("Object Script");
   // open separate window to show scripts?
   if (mObject != NULL) {
      title = wxT("Scripting for ");
      title += mObject->GetName().c_str();
   }
   ShowScriptDialog ssd(this, -1, title, mObject, true);
   ssd.ShowModal();
}



//---------------------------------
// methods inherited from GmatPanel
//---------------------------------

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
/**
 * @param <scName> input spacecraft name.
 *
 * @note Creates the notebook for spacecraft information
 */
//------------------------------------------------------------------------------
void UniversePanel::Create()
{
   #ifdef DEBUG_UNIVERSEPANEL_CREATE
   MessageInterface::ShowMessage(wxT("UniversePanel::Create() entered\n"));
   #endif
   
   #if __WXMAC__
   int buttonWidth = 40;
   #else
   int buttonWidth = 25;
   #endif

   Integer bsize = 2; // border size
   wxBitmap openBitmap = wxBitmap(OpenFolder_xpm);
   
   // get the config object
   wxConfigBase *pConfig = wxConfigBase::Get();
   // SetPath() understands ".."
   pConfig->SetPath(wxT("/Solar System"));

   wxArrayString emptyArray;
   
   //-------------------------------------------------------
   // ephemeris update interval
   //-------------------------------------------------------
   wxStaticText *intervalStaticText =
      new wxStaticText(this, ID_TEXT, wxT("Ephemeris Update ") wxT(GUI_ACCEL_KEY) wxT("Interval"),
                       wxDefaultPosition, wxSize(-1,-1), 0);
   mIntervalTextCtrl = new wxTextCtrl(this, ID_TEXT_CTRL, wxT(""),
                                 wxDefaultPosition, wxSize(50,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
   mIntervalTextCtrl->SetToolTip(pConfig->Read(wxT("EphemerisUpdateIntervalHint")));
   wxStaticText *intervalUnitsStaticText =
      new wxStaticText(this, ID_TEXT, wxT("seconds"),
                       wxDefaultPosition, wxSize(-1,-1), 0);
   
   //-------------------------------------------------------
   // ephemeris source
   //-------------------------------------------------------
   wxStaticText *fileTypeLabel =
      new wxStaticText(this, ID_TEXT, wxT("Ephemeris ") wxT(GUI_ACCEL_KEY) wxT("Source"),
                       wxDefaultPosition, wxSize(-1,-1), 0);
   
   mFileTypeComboBox = 
      new wxComboBox(this, ID_COMBOBOX, wxT(""), wxDefaultPosition, wxDefaultSize,
                     emptyArray, wxCB_READONLY);
   mFileTypeComboBox->SetToolTip(pConfig->Read(wxT("EphemerisSourceHint")));
   
   //-------------------------------------------------------
   // ephemeris file
   //-------------------------------------------------------
   fileNameLabel =
      new wxStaticText(this, ID_TEXT, wxT("Ephemeris ") wxT(GUI_ACCEL_KEY) wxT("Filename"),
                       wxDefaultPosition, wxSize(-1,-1), 0);
   
   mFileNameTextCtrl =
      new wxTextCtrl(this, ID_TEXT_CTRL, wxT(""),
                     wxDefaultPosition, wxSize(300, -1),  0);
   mFileNameTextCtrl->SetToolTip(pConfig->Read(wxT("EphemerisFilenameHint")));
   
   
   mBrowseButton =
      new wxBitmapButton(this, ID_BUTTON_BROWSE, openBitmap, wxDefaultPosition,
                         wxSize(buttonWidth, 20));
   mBrowseButton->SetToolTip(pConfig->Read(wxT("BrowseEphemerisFilenameHint")));
   
   
   //-------------------------------------------------------
   // SPICE Leap Second Kernel (LSK)
   //-------------------------------------------------------
   lskNameLabel =
      new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Leap Second Kernel"),
                       wxDefaultPosition, wxSize(-1,-1), 0);

   mLSKFileNameTextCtrl =
      new wxTextCtrl(this, ID_TEXT_CTRL, wxT(""),
                     wxDefaultPosition, wxSize(300, -1),  0);
   mLSKFileNameTextCtrl->SetToolTip(pConfig->Read(wxT("LeapSecondFilenameHint")));


   mLSKBrowseButton =
      new wxBitmapButton(this, ID_LSK_BUTTON_BROWSE, openBitmap, wxDefaultPosition,
                         wxSize(buttonWidth, 20));
   mLSKBrowseButton->SetToolTip(pConfig->Read(wxT("BrowseLSKFilenameHint")));


   //-------------------------------------------------------
   // use TT for ephemeris
   //-------------------------------------------------------
   mOverrideCheckBox =
      new wxCheckBox(this, ID_CHECKBOX, wxT("Use ") wxT(GUI_ACCEL_KEY) wxT("TT for Ephemeris"),
                     wxDefaultPosition, wxSize(-1, -1), 0);
   mOverrideCheckBox->SetToolTip(pConfig->Read(wxT("UseTTForEphemerisHint")));
   
   
   //-------------------------------------------------------
   // Add to bottom grid sizer
   //-------------------------------------------------------    
   wxFlexGridSizer *bottomGridSizer = new wxFlexGridSizer(3, 0, 0);
   wxBoxSizer *intervalBoxSizer = new wxBoxSizer(wxHORIZONTAL);

   intervalBoxSizer->Add(mIntervalTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
   intervalBoxSizer->Add(intervalUnitsStaticText, 0, wxALIGN_LEFT|wxALL, bsize);

   bottomGridSizer->Add(intervalStaticText, 0, wxALIGN_LEFT|wxALL, bsize);
   bottomGridSizer->Add(intervalBoxSizer);
   bottomGridSizer->Add(20,20,0, wxALIGN_LEFT|wxALL, bsize);
   bottomGridSizer->Add(fileTypeLabel, 0, wxALIGN_LEFT|wxALL, bsize);
   bottomGridSizer->Add(mFileTypeComboBox, 0, wxALIGN_LEFT|wxALL, bsize);
   bottomGridSizer->Add(20,20,0, wxALIGN_LEFT|wxALL, bsize);
   bottomGridSizer->Add(fileNameLabel, 0, wxALIGN_LEFT|wxALL, bsize);
   bottomGridSizer->Add(mFileNameTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
   bottomGridSizer->Add(mBrowseButton, 0, wxALIGN_CENTRE|wxALL, bsize);
   bottomGridSizer->Add(lskNameLabel, 0, wxALIGN_LEFT|wxALL, bsize);
   bottomGridSizer->Add(mLSKFileNameTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
   bottomGridSizer->Add(mLSKBrowseButton, 0, wxALIGN_CENTRE|wxALL, bsize);
   bottomGridSizer->Add(mOverrideCheckBox, 0, wxALIGN_LEFT|wxALL, bsize);
   bottomGridSizer->Add(20,20,0, wxALIGN_LEFT|wxALL, bsize);
   bottomGridSizer->Add(20,20,0, wxALIGN_LEFT|wxALL, bsize);

   //-------------------------------------------------------
   // Add to pageSizer
   //------------------------------------------------------- 
   mPageSizer = new GmatStaticBoxSizer (wxVERTICAL, this, wxT("Options"));
   mPageSizer->Add(bottomGridSizer, 0, wxALIGN_CENTRE|wxALL, bsize);
   
   theMiddleSizer->Add(mPageSizer, 1, wxALIGN_CENTER|wxGROW|wxALL, bsize);
   
   #ifdef DEBUG_UNIVERSEPANEL_CREATE
   MessageInterface::ShowMessage(wxT("UniversePanel::Create() leaving\n"));
   #endif

}


//------------------------------------------------------------------------------
// virtual void LoadData()
//------------------------------------------------------------------------------
void UniversePanel::LoadData()
{
   #ifdef DEBUG_UNIVERSEPANEL_LOAD
   MessageInterface::ShowMessage(wxT("UniversePanel::LoadData() entered\n"));
   #endif
   
   try
   {
      mAllFileTypes = theGuiInterpreter->GetPlanetarySourceTypes();
      //mAnalyticModels = theGuiInterpreter->GetAnalyticModelNames();
      StringArray fileTypesInUse = theGuiInterpreter->GetPlanetarySourceTypesInUse();
      
      #ifdef DEBUG_UNIVERSEPANEL_LOAD
      MessageInterface::ShowMessage
         (wxT("   There are %d available file type(s)\n"), mAllFileTypes.size());
      for (unsigned int i=0; i<mAllFileTypes.size(); i++)
         MessageInterface::ShowMessage
            (wxT("      '%s'\n"), mAllFileTypes[i].c_str());
      MessageInterface::ShowMessage
         (wxT("   There are %d file type(s) in use\n"), fileTypesInUse.size());
      for (unsigned int i=0; i<fileTypesInUse.size(); i++)
         MessageInterface::ShowMessage
            (wxT("      '%s'\n"), fileTypesInUse[i].c_str());
      #endif
      
      // load  ephemeris update interval
      Real interval = theSolarSystem->GetEphemUpdateInterval();
      mIntervalTextCtrl->SetValue(theGuiManager->ToWxString(interval));
      
      #ifdef DEBUG_UNIVERSEPANEL_LOAD
      MessageInterface::ShowMessage(wxT("   Interval set to %f\n"), interval);
      #endif
      
      // available source
      for (unsigned int i=0; i<mAllFileTypes.size(); i++)
      {
         wxString type = mAllFileTypes[i].c_str();
         wxString typeName = theGuiInterpreter->GetPlanetarySourceName(mAllFileTypes[i]).c_str();
         mFileTypeNameMap[type] = typeName;
         mFileTypeComboBox->Append(type);
      }
      
      #ifdef DEBUG_UNIVERSEPANEL_LOAD
      MessageInterface::ShowMessage(wxT("   Here is the mapping of file types\n"));
      for (std::map<wxString, wxString>::iterator i = mFileTypeNameMap.begin();
           i != mFileTypeNameMap.end(); ++i)
         MessageInterface::ShowMessage
            (wxT("      <%-20s>   '%-30s'\n"), (i->first).c_str(), (i->second).c_str());
      #endif
      
      // available analytic models
      //for (unsigned int i=0; i<mAnalyticModels.size(); i++)
      //{
      //   mAnalyticModelComboBox->Append(mAnalyticModels[i].c_str());
      //}
      //   
      // set defaults
      //mAnalyticModelComboBox->SetSelection(0);
      
      wxString currentSource =
         theSolarSystem->GetStringParameter(theSolarSystem->GetParameterID(wxT("EphemerisSource")));
      mFileTypeComboBox->SetStringSelection(currentSource.c_str());
      
      #ifdef DEBUG_UNIVERSEPANEL_LOAD
      MessageInterface::ShowMessage
         (wxT("   Ephemeris source set to '%s'\n"), currentSource.c_str());
      #endif
      
      //mFileTypeComboBox->SetStringSelection(fileTypesInUse[0].c_str());
      //if (mFileTypeComboBox->GetStringSelection() == "Analytic")
      if (mFileTypeComboBox->GetStringSelection() == wxT("TwoBodyPropagation"))
      {
         fileNameLabel->SetLabel(wxT("Ephemeris ") wxT(GUI_ACCEL_KEY) wxT("Filename"));
         mBrowseButton->Disable();
         mFileNameTextCtrl->Disable();
         lskNameLabel->Show(false);
         mLSKBrowseButton->Show(false);
         wxWindow *windowWithFocus = FindFocus();
         if (windowWithFocus == mLSKFileNameTextCtrl)  mFileNameTextCtrl->SetFocus();
         mLSKFileNameTextCtrl->Show(false);
         //mPageSizer->Show(mAnaModelSizer, true);
      }
      else
      {
         mBrowseButton->Enable();
         mFileNameTextCtrl->Enable();
         if (mFileTypeComboBox->GetStringSelection() == wxT("SPICE"))
         {
            fileNameLabel->SetLabel(wxT("SPK ") wxT(GUI_ACCEL_KEY) wxT("Kernel"));
            lskNameLabel->Show(true);
            mLSKBrowseButton->Show(true);
            mLSKFileNameTextCtrl->Show(true);
         }
         else
         {
            fileNameLabel->SetLabel(wxT("DE ") wxT(GUI_ACCEL_KEY) wxT("Filename"));
            lskNameLabel->Show(false);
            mLSKBrowseButton->Show(false);
            wxWindow *windowWithFocus = FindFocus();
            if (windowWithFocus == mLSKFileNameTextCtrl)  mFileNameTextCtrl->SetFocus();
            mLSKFileNameTextCtrl->Show(false);
         }
         //mPageSizer->Show(mAnaModelSizer, false);
      }
      
      wxString selStr = mFileTypeComboBox->GetStringSelection();
      #ifdef DEBUG_UNIVERSEPANEL_LOAD
      MessageInterface::ShowMessage
         (wxT("   Selected ephemeris source is '%s'\n"), selStr.c_str());
      #endif
      if (selStr != wxT(""))
      {
         wxString fileName = mFileTypeNameMap[selStr];
         mFileNameTextCtrl->SetValue(fileName);
         
         #ifdef DEBUG_UNIVERSEPANEL_LOAD
         MessageInterface::ShowMessage
            (wxT("   Ephemeris file name set to '%s'\n"), fileName.c_str());
         #endif
      }
      
      wxString lskFile = (theSolarSystem->GetStringParameter(wxT("LSKFilename"))).c_str();
      mLSKFileNameTextCtrl->SetValue(lskFile);

      bool useTT = theSolarSystem->GetBooleanParameter(wxT("UseTTForEphemeris"));
      mOverrideCheckBox->SetValue(useTT);
      
      #ifdef DEBUG_UNIVERSEPANEL_LOAD
      MessageInterface::ShowMessage
         (wxT("   UseTTForEphemeris set to %s\n"), useTT ? wxT("true") : wxT("false"));
      #endif
      
      mPageSizer->Layout();
      mObject = theSolarSystem;
      EnableUpdate(false);
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
   
   #ifdef DEBUG_UNIVERSEPANEL_LOAD
   MessageInterface::ShowMessage(wxT("UniversePanel::LoadData() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// virtual void SaveData()
//------------------------------------------------------------------------------
void UniversePanel::SaveData()
{
   #ifdef DEBUG_UNIVERSEPANEL_SAVE
   MessageInterface::ShowMessage(wxT("UniversePanel::SaveData() entered\n"));
   MessageInterface::ShowMessage(wxT(" hasTextModified = %s\n"),
         (mHasTextModified? wxT("true"): wxT("false")));
   MessageInterface::ShowMessage(wxT(" mHasFileTypesInUseChanged = %s\n"),
         (mHasFileTypesInUseChanged? wxT("true"): wxT("false")));
   MessageInterface::ShowMessage(wxT(" mHasFileNameChanged = %s\n"),
         (mHasFileNameChanged? wxT("true"): wxT("false")));
   MessageInterface::ShowMessage(wxT(" mHasLSKFileNameChanged = %s\n"),
         (mHasLSKFileNameChanged? wxT("true"): wxT("false")));
   #endif
   
   canClose = true;
   wxString str;
   Real interval;
   
   //-----------------------------------------------------------------
   // check values from text field
   //-----------------------------------------------------------------
   
   if (mHasTextModified)
   {
      str = mIntervalTextCtrl->GetValue();
      CheckReal(interval, str, wxT("Interval"), wxT("Real Number >= 0.0"));
   }
   
   if (!canClose)
      return;
   
   //-----------------------------------------------------------------
   // save values to base, base code should do the range checking
   //-----------------------------------------------------------------
   try
   {
      // save ephemeris update interval, if changed
      if (mHasTextModified)
      {
         theSolarSystem->SetEphemUpdateInterval(interval);
         mHasTextModified = false;
      }
      
      // save planetary file types in use, if changed
      if (mHasFileTypesInUseChanged)
      {
         mFileTypesInUse.clear();
         wxString srcSelection = wxString(mFileTypeComboBox->GetStringSelection().c_str());
         mFileTypesInUse.push_back(srcSelection);
         theSolarSystem->SetStringParameter(theSolarSystem->GetParameterID(wxT("EphemerisSource")),
               srcSelection);
         #ifdef DEBUG_UNIVERSEPANEL_SAVE
         MessageInterface::ShowMessage
            (wxT("UniversePanel::SaveData() types=%s\n"),
             mFileTypesInUse[0].c_str());
         #endif
         theGuiInterpreter->SetPlanetarySourceTypesInUse(mFileTypesInUse);
         mHasFileTypesInUseChanged = false;
      }
         
      // save planetary file name, if changed
      if (mHasFileNameChanged)
      {
         wxString type = mFileTypeComboBox->GetStringSelection();
         str = mFileNameTextCtrl->GetValue();
         std::ifstream filename(str.char_str());
         
         // Check if the file doesn't exist then stop
//         if (type != "Analytic" && !filename) 
         if (type != wxT("TwoBodyPropagation") && !filename) 
         {
            wxString fieldName = wxT("DEFilename");
            if (type == wxT("SPICE"))
               fieldName = wxT("SPKFilename");
            MessageInterface::PopupMessage
               (Gmat::ERROR_, mMsgFormat.c_str(),
                str.c_str(), fieldName.c_str(), wxT("File must exist"));
            canClose = false;
            return;
         }
         filename.close();
         
         mFileTypeNameMap[mFileTypeComboBox->GetStringSelection()] = str.c_str();
         for (unsigned int i=0; i<mAllFileTypes.size(); i++)
         {
            wxString theType = mAllFileTypes[i].c_str();
            wxString name = wxString(mFileTypeNameMap[theType].c_str());
            theGuiInterpreter->SetPlanetarySourceName(mAllFileTypes[i], name);
            #ifdef DEBUG_UNIVERSEPANEL_SAVE
               wxString fieldName = wxT("DEFilename");
               if (type == wxT("SPICE"))
                  fieldName = wxT("SPKFilename");
//               MessageInterface::ShowMessage("theType = %s\n", theType.c_str());
               MessageInterface::ShowMessage(wxT("fieldName = %s\n"), fieldName.c_str());
               MessageInterface::ShowMessage(wxT("str = %s\n"), str.c_str());
            #endif
         }
//         theGuiInterpreter->SetPlanetarySourceName(type.c_str(), str);

         
         mHasFileNameChanged = false;
      }

      if (mHasLSKFileNameChanged)
      {
         wxString type = mFileTypeComboBox->GetStringSelection();
         str = mLSKFileNameTextCtrl->GetValue();
         std::ifstream filename(str.char_str());

         // Check if the file doesn't exist then stop
//         if (type != "Analytic" && !filename)
         if (type == wxT("SPICE") && !filename)
         {
            wxString fieldName = wxT("LSKFilename");
            MessageInterface::PopupMessage
               (Gmat::ERROR_, mMsgFormat.c_str(),
                str.c_str(), fieldName.c_str(), wxT("File must exist"));
            canClose = false;
            return;
         }
         filename.close();

         theSolarSystem->SetStringParameter(wxT("LSKFilename"), str);

         mHasLSKFileNameChanged = false;
      }

      // save analytical model, if changed 
//      if (mHasAnaModelChanged)
//      {
//         theGuiInterpreter->
//            SetAnalyticModelToUse(mAnalyticModelComboBox->GetStringSelection().c_str());
//         mHasAnaModelChanged = false;
//      }
      
      theSolarSystem->SetBooleanParameter(wxT("UseTTForEphemeris"),
                                          mOverrideCheckBox->IsChecked());
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      canClose = false;
      return;
   }
   
   #ifdef DEBUG_UNIVERSEPANEL_SAVE
   MessageInterface::ShowMessage(wxT("UniversePanel::SaveData() leaving\n"));
   #endif
}// end SaveData()


//------------------------------------------------------------------------------
// void OnBrowseButton(wxCommandEvent& event)
//------------------------------------------------------------------------------
void UniversePanel::OnBrowseButton(wxCommandEvent& event)
{
   wxString oldname = mFileNameTextCtrl->GetValue();
   wxFileDialog dialog(this, wxT("Choose a file"), wxT(""), wxT(""), wxT("*.*"));
   
   if (dialog.ShowModal() == wxID_OK)
   {
      wxString filename;
      
      filename = dialog.GetPath().c_str();

      if (!filename.IsSameAs(oldname))
      {
         mFileNameTextCtrl->SetValue(filename);
         mFileTypeNameMap[mFileTypeComboBox->GetStringSelection()] = filename;
         mHasFileNameChanged = true;
         EnableUpdate(true);
      }
   }
}

//------------------------------------------------------------------------------
// void OnLSKBrowseButton(wxCommandEvent& event)
//------------------------------------------------------------------------------
void UniversePanel::OnLSKBrowseButton(wxCommandEvent& event)
{
   wxString oldname = mLSKFileNameTextCtrl->GetValue();
   wxFileDialog dialog(this, wxT("Choose a file"), wxT(""), wxT(""), wxT("*.*"));

   if (dialog.ShowModal() == wxID_OK)
   {
      wxString filename;

      filename = dialog.GetPath().c_str();

      if (!filename.IsSameAs(oldname))
      {
         mLSKFileNameTextCtrl->SetValue(filename);
//         mFileTypeNameMap[mFileTypeComboBox->GetStringSelection()] = filename;
         mHasLSKFileNameChanged = true;
         EnableUpdate(true);
      }
   }

}



//------------------------------------------------------------------------------
// void OnComboBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void UniversePanel::OnComboBoxChange(wxCommandEvent& event)
{
   mFileNameTextCtrl->Enable();
   if (event.GetEventObject() == mFileTypeComboBox)
   {
      wxString type = mFileTypeComboBox->GetStringSelection();
      mFileNameTextCtrl->SetValue(mFileTypeNameMap[type]);

//      if (type == "Analytic")
      if (type == wxT("TwoBodyPropagation"))
      {
//         mPageSizer->Show(mAnaModelSizer, true);
         fileNameLabel->SetLabel(wxT("Ephemeris ") wxT(GUI_ACCEL_KEY) wxT("Filename"));
         mBrowseButton->Disable();
         mFileNameTextCtrl->Disable();
         lskNameLabel->Show(false);
         mLSKBrowseButton->Show(false);
         mLSKFileNameTextCtrl->Show(false);
         lskNameLabel->Disable();
         mLSKBrowseButton->Disable();
         lskNameLabel->Layout();
         // this next line is needed for the Mac - otherwise, when switching from SPICE,
         // the LSK text ctrl is still visible (even though it is disabled and hidden)
         mIntervalTextCtrl->SetFocus();
      }
      else
      {
//         mPageSizer->Show(mAnaModelSizer, false);
         mBrowseButton->Enable();
         mFileNameTextCtrl->Enable();
         if (type == wxT("SPICE"))
         {
            fileNameLabel->SetLabel(wxT("SPK ") wxT(GUI_ACCEL_KEY) wxT("Kernel"));
            lskNameLabel->Enable();
            mLSKBrowseButton->Enable();
            mLSKFileNameTextCtrl->Enable();
            lskNameLabel->Show(true);
            mLSKBrowseButton->Show(true);
            mLSKFileNameTextCtrl->Show(true);
         }
         else // "DE"
         {
            fileNameLabel->SetLabel(wxT("DE ") wxT(GUI_ACCEL_KEY) wxT("Filename"));
            lskNameLabel->Disable();
            mLSKFileNameTextCtrl->Disable();
            mLSKBrowseButton->Disable();
            lskNameLabel->Show(false);
            mLSKBrowseButton->Show(false);
            mLSKFileNameTextCtrl->Show(false);
            // this next line is needed for the Mac - otherwise, when switching from SPICE,
            // the LSK text ctrl is still visible (even though it is disabled and hidden)
            mFileNameTextCtrl->SetFocus();
         }
      }

      mPageSizer->Layout();
   }
//   else if (event.GetEventObject() == mAnalyticModelComboBox)
//   {
//      mHasAnaModelChanged = true;
//   }
   mHasFileTypesInUseChanged = true;
   EnableUpdate(true);
}

//------------------------------------------------------------------------------
// void OnCheckBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void UniversePanel::OnCheckBoxChange(wxCommandEvent& event)
{
   EnableUpdate(true);
}

//------------------------------------------------------------------------------
// void OnTextCtrlChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void UniversePanel::OnTextCtrlChange(wxCommandEvent& event)
{
   #ifdef DEBUG_UNIVERSEPANEL_SAVE
   MessageInterface::ShowMessage(wxT("UniversePanel::OnTextCtrlChange() entered\n"));
   MessageInterface::ShowMessage(wxT(" hasTextModified = %s\n"),
         (mHasTextModified? wxT("true"): wxT("false")));
   MessageInterface::ShowMessage(wxT(" mHasFileTypesInUseChanged = %s\n"),
         (mHasFileTypesInUseChanged? wxT("true"): wxT("false")));
   MessageInterface::ShowMessage(wxT(" mHasFileNameChanged = %s\n"),
         (mHasFileNameChanged? wxT("true"): wxT("false")));
   MessageInterface::ShowMessage(wxT(" mHasLSKFileNameChanged = %s\n"),
         (mHasLSKFileNameChanged? wxT("true"): wxT("false")));
   #endif
   if (event.GetEventObject() == mIntervalTextCtrl)
   {
      if (mIntervalTextCtrl->IsModified())
         mHasTextModified = true;
   }
   
   if (event.GetEventObject() == mFileNameTextCtrl)
   {
      if (mFileNameTextCtrl->IsModified())
      {
         mHasFileNameChanged = true;
         mFileTypeNameMap[mFileTypeComboBox->GetStringSelection()] = mFileNameTextCtrl->GetValue();
      }
   }
   
   if (event.GetEventObject() == mLSKFileNameTextCtrl)
   {
      if (mLSKFileNameTextCtrl->IsModified())
         mHasLSKFileNameChanged = true;
   }

   mPageSizer->Layout();
   EnableUpdate(true);

   #ifdef DEBUG_UNIVERSEPANEL_SAVE
   MessageInterface::ShowMessage(wxT("end of UniversePanel::OnTextCtrlChange() entered\n"));
   MessageInterface::ShowMessage(wxT(" hasTextModified = %s\n"),
         (mHasTextModified? wxT("true"): wxT("false")));
   MessageInterface::ShowMessage(wxT(" mHasFileTypesInUseChanged = %s\n"),
         (mHasFileTypesInUseChanged? wxT("true"): wxT("false")));
   MessageInterface::ShowMessage(wxT(" mHasFileNameChanged = %s\n"),
         (mHasFileNameChanged? wxT("true"): wxT("false")));
   MessageInterface::ShowMessage(wxT(" mHasLSKFileNameChanged = %s\n"),
         (mHasLSKFileNameChanged? wxT("true"): wxT("false")));
   #endif
}
