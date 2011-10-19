//$Id: EphemerisFilePanel.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                           EphemerisFilePanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
// Author: Tuan Dang Nguyen
// Created: 2010/02/02
/**
 * This class is a generic setup panel used by objects derived from GmatBase
 */
//------------------------------------------------------------------------------

#include "EphemerisFilePanel.hpp"
#include "MessageInterface.hpp"
#include "bitmaps/OpenFolder.xpm"
#include "TimeSystemConverter.hpp"
#include <wx/config.h>

/// wxWidget event mappings for the panel
BEGIN_EVENT_TABLE(EphemerisFilePanel, GmatPanel)
   EVT_COMBOBOX(ID_COMBOBOX, EphemerisFilePanel::OnComboBoxChange)
   EVT_TEXT(ID_COMBOBOX, EphemerisFilePanel::OnComboBoxTextChange)
   EVT_TEXT(ID_TEXTCTRL, EphemerisFilePanel::OnTextChange)
   EVT_BUTTON(ID_BUTTON_BROWSE, EphemerisFilePanel::OnBrowse)
   EVT_CHECKBOX(ID_CHECKBOX, EphemerisFilePanel::OnCheck)
END_EVENT_TABLE()


//-----------------------------------------
// public methods
//-----------------------------------------

//------------------------------------------------------------------------------
// EphemerisFilePanel(wxWindow *parent, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Panel constructor
 * 
 * @param parent Owner for this panel
 * @param name Name of the object that is to be configured 
 */
//------------------------------------------------------------------------------
EphemerisFilePanel::EphemerisFilePanel(wxWindow *parent, const wxString &name)
   : GmatPanel                (parent),
     previousEpochFormat      (wxT(""))
{
   clonedObj = NULL;
   fileDialog = NULL;
   
   mObject = theGuiInterpreter->GetConfiguredObject(name.c_str());
   
   if (mObject != NULL)
   {
      Create();
      Show();
   }
   else
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("The object named \"%s\" does not exist\n"),
               name.c_str());
   }
}


//------------------------------------------------------------------------------
// ~EphemerisFilePanel()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
EphemerisFilePanel::~EphemerisFilePanel()
{
   // Unregister automatically registered ComboBoxes
   std::map<wxString, wxComboBox *>::iterator iter;
   for (iter = managedComboBoxMap.begin(); iter != managedComboBoxMap.end();
         ++iter)
      theGuiManager->UnregisterComboBox(iter->first, iter->second);
}


//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
/**
 * Inherited function that is called to create the panel.
 */
//------------------------------------------------------------------------------

void EphemerisFilePanel::Create()
{
   Integer bsize = 2;              // border size
   Integer id;  
   #if __WXMAC__
   int buttonWidth = 40;
   #else
   int buttonWidth = 25;
   #endif

   wxBitmap openBitmap = wxBitmap(OpenFolder_xpm);
   
   // get the config object
   wxConfigBase *pConfig = wxConfigBase::Get();
   // SetPath() understands ".."
   pConfig->SetPath(wxT("/Ephemeris File"));
   
   // 1. Create Options box:
   wxStaticBoxSizer *optionsStaticBoxSizer = new wxStaticBoxSizer(wxHORIZONTAL,
         this, wxT("Options"));
   
   wxFlexGridSizer *grid1 = new wxFlexGridSizer( 2, 0, 0 );
   grid1->AddGrowableCol(1);
   
   id = mObject->GetParameterID(wxT("Spacecraft"));
   wxStaticText * spacecraftStaticText =
      new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Spacecraft"),
            wxDefaultPosition, wxDefaultSize, 0 );
   spacecraftComboBox = (wxComboBox*)BuildControl(this, id);
   spacecraftComboBox->SetToolTip(pConfig->Read(wxT("SpacecraftHint")));
   grid1->Add(spacecraftStaticText, 0, wxALIGN_LEFT|wxALL, bsize );
   grid1->Add(spacecraftComboBox, 0, wxALIGN_LEFT|wxALL, bsize );
   
//   id = mObject->GetParameterID("StateType");
//   wxStaticText * stateTypeStaticText =
//      new wxStaticText(this, ID_TEXT, wxT("State T"GUI_ACCEL_KEY"ype"),
//            wxDefaultPosition, wxDefaultSize, 0 );
//   stateTypeComboBox = (wxComboBox*) BuildControl(this, id);
//   stateTypeComboBox->SetToolTip(pConfig->Read(wxT("StateTypeHint")));
//   grid1->Add(stateTypeStaticText, 0, wxALIGN_LEFT|wxALL, bsize );
//   grid1->Add(stateTypeComboBox, 0, wxALIGN_LEFT|wxALL, bsize );
   
   id = mObject->GetParameterID(wxT("CoordinateSystem"));
   wxStaticText * coordinateSystemStaticText =
      new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Coordinate System"),
            wxDefaultPosition, wxDefaultSize, 0 );
   coordinateSystemComboBox = (wxComboBox*) BuildControl(this, id);
   coordinateSystemComboBox->SetToolTip(pConfig->Read(
         wxT("CoordinateSystemHint")));
   grid1->Add(coordinateSystemStaticText, 0, wxALIGN_LEFT|wxALL, bsize );
   grid1->Add(coordinateSystemComboBox, 0, wxALIGN_LEFT|wxALL, bsize );
   
   id = mObject->GetParameterID(wxT("WriteEphemeris"));
   writeEphemerisCheckBox = (wxCheckBox*) BuildControl(this, id);
   writeEphemerisCheckBox->SetToolTip(pConfig->Read(wxT("WriteEphemerisHint")));
   grid1->Add(writeEphemerisCheckBox, 0, wxALIGN_LEFT|wxALL, bsize );
   grid1->Add(0, 0, wxALIGN_LEFT|wxALL, bsize );
   
   optionsStaticBoxSizer->Add( grid1, 0, wxALIGN_LEFT|wxALL, bsize );
   
   
   // 2. Create File Settings box:
   wxStaticBoxSizer *fileSettingsStaticBoxSizer =
      new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("File Settings"));
   wxFlexGridSizer *grid2 = new wxFlexGridSizer( 3, 0, 0 );
   grid2->AddGrowableCol(1);
   
   id = mObject->GetParameterID(wxT("FileFormat"));
   wxStaticText * fileFormatStaticText =
      new  wxStaticText(this, ID_TEXT, wxT("File For") wxT(GUI_ACCEL_KEY) wxT("mat"),
            wxDefaultPosition, wxDefaultSize, 0 );
   fileFormatComboBox = (wxComboBox*) BuildControl(this, id);
   fileFormatComboBox->SetToolTip(pConfig->Read(wxT("FileFormatHint")));
   grid2->Add(fileFormatStaticText, 0, wxALIGN_LEFT|wxALL, bsize );
   grid2->Add(fileFormatComboBox, 0, wxALIGN_LEFT|wxALL, bsize );
   grid2->Add(0, 0, wxALIGN_CENTER|wxALL, bsize );
   
   id = mObject->GetParameterID(wxT("Filename"));
   wxStaticText * fileNameStaticText =
      new  wxStaticText(this, ID_TEXT, wxT("File ") wxT(GUI_ACCEL_KEY) wxT("Name"),
            wxDefaultPosition, wxDefaultSize, 0 );
   fileNameTextCtrl = (wxTextCtrl*) BuildControl(this, id);
   fileNameTextCtrl->SetToolTip(pConfig->Read(wxT("FilenameHint")));
   browseButton =
      new wxBitmapButton(this, ID_BUTTON_BROWSE, openBitmap, wxDefaultPosition,
                         wxSize(buttonWidth, 20));
   browseButton->SetToolTip(pConfig->Read(wxT("BrowseEphemerisFilenameHint")));
   grid2->Add(fileNameStaticText, 0, wxALIGN_LEFT|wxALL, bsize );
   grid2->Add(fileNameTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize );
   grid2->Add(browseButton, 0, wxALIGN_LEFT|wxALL, bsize );
   
   id = mObject->GetParameterID(wxT("Interpolator"));
   wxStaticText * interpolatorStaticText =
      new  wxStaticText(this, ID_TEXT, wxT("Interpolato") wxT(GUI_ACCEL_KEY) wxT("r"),
            wxDefaultPosition, wxDefaultSize, 0 );
   interpolatorComboBox = (wxComboBox*) BuildControl(this, id);
   interpolatorComboBox->SetToolTip(pConfig->Read(wxT("InterpolatorHint")));
   interpolatorComboBox->Enable(false);
   grid2->Add(interpolatorStaticText, 0, wxALIGN_LEFT|wxALL, bsize );
   grid2->Add(interpolatorComboBox, 0, wxALIGN_LEFT|wxALL, bsize );
   grid2->Add(0, 0, wxALIGN_CENTER|wxALL, bsize );
   
   id = mObject->GetParameterID(wxT("InterpolationOrder"));
   wxStaticText * interpolationOrderStaticText =
      new  wxStaticText(this, ID_TEXT, wxT("Interpolation ") wxT(GUI_ACCEL_KEY)
            wxT("Order"), wxDefaultPosition, wxDefaultSize, 0 );
   interpolationOrderTextCtrl = (wxTextCtrl*) BuildControl(this, id);
   interpolationOrderTextCtrl->SetToolTip(pConfig->Read(
         wxT("InterpolationOrderHint")));
   grid2->Add(interpolationOrderStaticText, 0, wxALIGN_LEFT|wxALL, bsize );
   grid2->Add(interpolationOrderTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize );
   grid2->Add(0, 0, wxALIGN_CENTER|wxALL, bsize );
   
   id = mObject->GetParameterID(wxT("StepSize"));
   wxStaticText * stepSizeStaticText =
      new  wxStaticText(this, ID_TEXT, wxT("S") wxT(GUI_ACCEL_KEY) wxT("tep Size"),
            wxDefaultPosition, wxDefaultSize, 0 );
   stepSizeComboBox = (wxComboBox*) BuildControl(this, id);
   stepSizeComboBox->SetToolTip(pConfig->Read(wxT("StepSizeHint")));
   grid2->Add(stepSizeStaticText, 0, wxALIGN_LEFT|wxALL, bsize );
   grid2->Add(stepSizeComboBox, 0, wxALIGN_LEFT|wxALL, bsize );
   grid2->Add(0, 0, wxALIGN_CENTER|wxALL, bsize );
   
   fileSettingsStaticBoxSizer->Add( grid2, 0, wxALIGN_LEFT|wxALL, bsize );
   
   
   // 3. Create Epoch box:
   wxStaticBoxSizer *epochStaticBoxSizer = new wxStaticBoxSizer(wxHORIZONTAL,
         this, wxT("Epoch"));
   wxFlexGridSizer *grid3 = new wxFlexGridSizer( 2, 0, 0 );
   grid3->AddGrowableCol(1);
   
   id = mObject->GetParameterID(wxT("EpochFormat"));
   wxStaticText * epochFormatStaticText =
      new  wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Epoch Format"),
            wxDefaultPosition, wxDefaultSize, 0 );
   epochFormatComboBox = (wxComboBox*) BuildControl(this, id);
   epochFormatComboBox->SetToolTip(pConfig->Read(wxT("EpochFormatHint")));
   grid3->Add(epochFormatStaticText, 0, wxALIGN_LEFT|wxALL, bsize );
   grid3->Add(epochFormatComboBox, 0, wxALIGN_LEFT|wxALL, bsize );
   previousEpochFormat = epochFormatComboBox->GetValue();
   
   id = mObject->GetParameterID(wxT("InitialEpoch"));
   wxStaticText * initialEpochStaticText =
      new  wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Initial Epoch"),
            wxDefaultPosition, wxDefaultSize, 0 );
   initialEpochComboBox = (wxComboBox*) BuildControl(this, id);
   initialEpochComboBox->SetToolTip(pConfig->Read(wxT("InitialEpochHint")));
   grid3->Add(initialEpochStaticText, 0, wxALIGN_LEFT|wxALL, bsize );
   grid3->Add(initialEpochComboBox, 0, wxALIGN_LEFT|wxALL, bsize );
   
   id = mObject->GetParameterID(wxT("FinalEpoch"));
   wxStaticText * finalEpochStaticText =
      new  wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Final Epoch"),
            wxDefaultPosition, wxDefaultSize, 0 );
   finalEpochComboBox = (wxComboBox*) BuildControl(this, id);
   finalEpochComboBox->SetToolTip(pConfig->Read(wxT("FinalEpochHint")));
   grid3->Add(finalEpochStaticText, 0, wxALIGN_LEFT|wxALL, bsize );
   grid3->Add(finalEpochComboBox, 0, wxALIGN_LEFT|wxALL, bsize );
   
   epochStaticBoxSizer->Add( grid3, 0, wxALIGN_LEFT|wxALL, bsize );
   
   // 4. Create theMiddleSizer:
   theMiddleSizer->Add(optionsStaticBoxSizer, 0, wxGROW, bsize);
   theMiddleSizer->Add(fileSettingsStaticBoxSizer, 0, wxGROW, bsize);
   theMiddleSizer->Add(epochStaticBoxSizer, 0, wxGROW, bsize);
}


//------------------------------------------------------------------------------
// void LoadData()
//------------------------------------------------------------------------------
/**
 * Populates the panel with the configurable property data in the Solver 
 */
//------------------------------------------------------------------------------
void EphemerisFilePanel::LoadData()
{
   // load data from the core engine
   try
   {
      LoadControl(wxT("Spacecraft"));
      LoadControl(wxT("StateType"));
      LoadControl(wxT("CoordinateSystem"));
      LoadControl(wxT("WriteEphemeris"));
      LoadControl(wxT("FileFormat"));
      LoadControl(wxT("Filename"));
      LoadControl(wxT("Interpolator"));
      LoadControl(wxT("InterpolationOrder"));
      LoadControl(wxT("StepSize"));
      LoadControl(wxT("EpochFormat"));
      LoadControl(wxT("InitialEpoch"));
      LoadControl(wxT("FinalEpoch"));
   }
   catch (BaseException &e)
   {
      MessageInterface::ShowMessage
         (wxT("EphemerisFilePanel:LoadData() error occurred!\n%s\n"),
          e.GetFullMessage().c_str());
   }
   
   // explicitly disable apply button
   // it is turned on in each of the panels
   EnableUpdate(false);
}


//------------------------------------------------------------------------------
// void SaveData()
//------------------------------------------------------------------------------
/**
 * Passes configuration data from the panel to the EphemerisFile object
 */
//------------------------------------------------------------------------------
void EphemerisFilePanel::SaveData()
{
   canClose = true;
   
   //-----------------------------------------------------------------
   // save values to base, base code should do the range checking
   //-----------------------------------------------------------------
   clonedObj = mObject->Clone();
   try
   {
      SaveControl(wxT("Spacecraft"));
      SaveControl(wxT("StateType"));
      SaveControl(wxT("CoordinateSystem"));
      SaveControl(wxT("WriteEphemeris"));
      SaveControl(wxT("FileFormat"));
      SaveControl(wxT("Filename"));
      SaveControl(wxT("Interpolator"));
      SaveControl(wxT("InterpolationOrder"));
      SaveControl(wxT("StepSize"));
      SaveControl(wxT("EpochFormat"));
      try
      {
         SaveControl(wxT("InitialEpoch"));
      }
      catch (BaseException &e)
      {
         MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
         canClose = false;
      }
      
      try
      {
         SaveControl(wxT("FinalEpoch"));
      }
      catch (BaseException &e)
      {
         MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
         canClose = false;
      }
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      canClose = false;
   }
   
   if (canClose)
      mObject->Copy(clonedObj);

}


//------------------------------------------------------------------------------
// wxControl* BuildControl(wxWindow *parent, Integer index)
//------------------------------------------------------------------------------
/**
 * Builds a wxWidget control for an object property
 * 
 * @param parent The window that owns the control
 * @param index The index for the property that the constructed control
 *              represents
 * 
 * @return The new control
 */
//------------------------------------------------------------------------------
wxControl *EphemerisFilePanel::BuildControl(wxWindow *parent, Integer index)
{
   wxControl *control = NULL;
   
   Gmat::ParameterType type = mObject->GetParameterType(index);
   
   switch (type)
   {
   case Gmat::BOOLEAN_TYPE:
      {
         wxCheckBox *cbControl;
         if (mObject->GetParameterText(index) == wxT("WriteEphemeris"))
            cbControl = new wxCheckBox(parent, ID_CHECKBOX,
                  wxT(GUI_ACCEL_KEY) wxT("Write Ephemeris"));
         else
            cbControl = new wxCheckBox(parent, ID_CHECKBOX,
                  (mObject->GetParameterText(index)).c_str());
         
         cbControl->SetValue(mObject->GetBooleanParameter(index));
         control = cbControl;
      }
      break;
   case Gmat::OBJECT_TYPE:
      {
         Gmat::ObjectType type = mObject->GetPropertyObjectType(index);
         if (type == Gmat::SPACE_POINT)
         {
            // The GuiItemManager automatically registers wxComboBox in order to
            // listen for any SpacePoint updates, so need to unregister
            // in the destructor
            wxComboBox *cbControl =
               theGuiManager->GetSpacePointComboBox(this, ID_COMBOBOX,
                                                    wxSize(180,-1), false);
            managedComboBoxMap.insert(std::make_pair(wxT("SpacePoint"), cbControl));
            control = cbControl;
         }
         else if (type == Gmat::CELESTIAL_BODY)
         {
            // The GuiItemManager automatically registers wxComboBox in order to
            // listen for any SpacePoint updates, so need to unregister
            // in the destructor
            wxComboBox *cbControl =
            theGuiManager->GetCelestialBodyComboBox(this, ID_COMBOBOX,
                                                    wxSize(180,-1));
            managedComboBoxMap.insert(std::make_pair(wxT("CelestialBody"),
                  cbControl));
            control = cbControl;
         }
         else if (type == Gmat::SPACECRAFT)
         {
            // The GuiItemManager automatically registers wxComboBox in order to
            // listen for any SpacePoint updates, so need to unregister
            // in the destructor
            wxComboBox *cbControl =
               theGuiManager->GetSpacecraftComboBox(this, ID_COMBOBOX,
                                                    wxSize(180,-1));
            managedComboBoxMap.insert(std::make_pair(wxT("Spacecraft"), cbControl));
            control = cbControl;
         }
         else if (type == Gmat::COORDINATE_SYSTEM)
         {
            // The GuiItemManager automatically registers wxComboBox in order to
            // listen for any SpacePoint updates, so need to unregister
            // in the destructor
            wxComboBox *cbControl =
               theGuiManager->GetCoordSysComboBox(this, ID_COMBOBOX,
                                                  wxSize(180,-1));
            managedComboBoxMap.insert(std::make_pair(wxT("CoordinateSystem"),
                  cbControl));
            control = cbControl;
         }
         else
         {
            // Check if enumeration strings available for owned object types
            wxArrayString enumList;
            StringArray enumStrings = mObject->GetPropertyEnumStrings(index);
            for (UnsignedInt i=0; i<enumStrings.size(); i++)
               enumList.Add(enumStrings[i].c_str());
            
            control = new wxComboBox(parent, ID_COMBOBOX, wxT(""),
                                     wxDefaultPosition, wxSize(180,-1),
                                     enumList, wxCB_READONLY);
         }
      }
      break;
   case Gmat::ENUMERATION_TYPE:
      {
         StringArray enumStrings = mObject->GetPropertyEnumStrings(index);
         wxArrayString enumList;
         for (UnsignedInt i=0; i<enumStrings.size(); i++)
            enumList.Add(enumStrings[i].c_str());
         
         wxComboBox *cbControl = NULL;
         if (enumStrings.size() == 1)
         {
            // Show the value even if value is not in the list
            cbControl =
               new wxComboBox(parent, ID_COMBOBOX, wxT(""), wxDefaultPosition,
                              wxSize(180,-1), enumList, 0);
         }
         else
         {
            // Do not show the value if value is not in the list
            cbControl =
               new wxComboBox(parent, ID_COMBOBOX, wxT(""), wxDefaultPosition,
                              wxSize(180,-1), enumList, wxCB_READONLY);
         }
         
         control = cbControl;
      }
      break;
      
   case Gmat::INTEGER_TYPE:
   case Gmat::REAL_TYPE:      
      control = new wxTextCtrl(parent, ID_TEXTCTRL, 
                               wxT(""), wxDefaultPosition, wxSize(180,-1), 0,
                               wxTextValidator(wxGMAT_FILTER_NUMERIC));
      break;
      
   case Gmat::STRING_TYPE:
   default:
      control = new wxTextCtrl(parent, ID_TEXTCTRL, 
                               wxT(""), wxDefaultPosition, wxSize(180,-1));
      break;
   }
   
   return control;
}


//------------------------------------------------------------------------------
// void LoadControl(const wxString &label)
//------------------------------------------------------------------------------
/**
 * Sets the data for a control
 * 
 * @param label The control's label
 */
//------------------------------------------------------------------------------
void EphemerisFilePanel::LoadControl(const wxString &label)
{
   wxString valueString;
   bool valueBool;
   Integer valueInteger;
   
   //Integer index = mObject->GetParameterID(label);
   //Gmat::ParameterType type = mObject->GetParameterType(index);
   
   if (label == wxT("Spacecraft"))
   {
      valueString = mObject->GetStringParameter(label).c_str();
      spacecraftComboBox->SetValue(valueString);
   }
//   else if (label == "StateType")
//   {
//      valueString = wxT(mObject->GetStringParameter(label).c_str());
//      stateTypeComboBox->SetValue(valueString);
//   }
   else if (label == wxT("CoordinateSystem"))
   {
      valueString = mObject->GetStringParameter(label).c_str();
      coordinateSystemComboBox->SetValue(valueString);
   }
   else if (label == wxT("WriteEphemeris"))
   {
      valueBool = mObject->GetBooleanParameter(mObject->GetParameterID(label));
      writeEphemerisCheckBox->SetValue(valueBool);
   }
   else if (label == wxT("FileFormat"))
   {
      valueString = mObject->GetStringParameter(label).c_str();
      fileFormatComboBox->SetValue(valueString);

      // Set interpolator based on the format, per bug 2219
      valueString = mObject->GetStringParameter(wxT("Interpolator")).c_str();
      interpolatorComboBox->SetValue(valueString);
      // Use the order from the object as well
      valueInteger = mObject->GetIntegerParameter(wxT("InterpolationOrder"));
      interpolationOrderTextCtrl->SetValue(wxString::Format(wxT("%d"),valueInteger));
   }
   else if (label == wxT("Filename"))
   {
      valueString = mObject->GetStringParameter(label).c_str();
      fileNameTextCtrl->SetValue(valueString);
   }
   // Set interpolator based on the format, per bug 2219
//   else if (label == "Interpolator")
//   {
//      valueString = wxT(mObject->GetStringParameter(label).c_str());
//      interpolatorComboBox->SetValue(valueString);
//   }
   else if (label == wxT("InterpolationOrder"))
   {
      valueInteger = mObject->GetIntegerParameter(label.c_str());
      interpolationOrderTextCtrl->SetValue(wxString::Format(wxT("%d"),valueInteger));
   }
   else if (label == wxT("StepSize"))
   {
      valueString = mObject->GetStringParameter(label).c_str();
      stepSizeComboBox->SetValue(valueString);
   }
   else if (label == wxT("EpochFormat"))
   {
      valueString = mObject->GetStringParameter(label).c_str();
      epochFormatComboBox->SetValue(valueString);
      previousEpochFormat = epochFormatComboBox->GetValue();
   }
   else if (label == wxT("InitialEpoch"))
   {
      valueString = mObject->GetStringParameter(label).c_str();
      initialEpochComboBox->SetValue(valueString);
   }
   else if (label == wxT("FinalEpoch"))
   {
      valueString = mObject->GetStringParameter(label).c_str();
      finalEpochComboBox->SetValue(valueString);
   }
}


//------------------------------------------------------------------------------
// void SaveControl(const wxString &label)
//------------------------------------------------------------------------------
/**
 * Passes a control's data to the EphemerisFile.
 * 
 * @param label The string associated with the control.
 */
//------------------------------------------------------------------------------
void EphemerisFilePanel::SaveControl(const wxString &label)
{
   wxString valueString;
   bool valueBool;
   Integer valueInteger;
   Real stepSize;
   
   Integer paramId = clonedObj->GetParameterID(label);
   
   if (label == wxT("Spacecraft"))
   {
      valueString = spacecraftComboBox->GetValue();
      clonedObj->SetStringParameter(paramId, valueString);
   }
   // Only allowed StateType is Cartesian for the 2010 release, per bug 2219
//   else if (label == "StateType")
//   {
//      valueString = stateTypeComboBox->GetValue();
//      clonedObj->SetStringParameter(paramId, valueString);
//   }
   else if (label == wxT("CoordinateSystem"))
   {
      valueString = coordinateSystemComboBox->GetValue();
      clonedObj->SetStringParameter(paramId, valueString);
   }
   else if (label == wxT("WriteEphemeris"))
   {
      valueBool =  writeEphemerisCheckBox->GetValue();
      clonedObj->SetBooleanParameter(paramId, valueBool);
   }
   else if (label == wxT("FileFormat"))
   {
      valueString = fileFormatComboBox->GetValue();
      clonedObj->SetStringParameter(paramId, valueString);
   }
   else if (label == wxT("Filename"))
   {
      valueString = fileNameTextCtrl->GetValue();
      if (CheckFileName(valueString, wxT("Filename")))
         clonedObj->SetStringParameter(paramId, valueString);
   }
   // Set interpolator based on the format, per bug 2219
//   else if (label == "Interpolator")
//   {
//      valueString = interpolatorComboBox->GetValue();
//      clonedObj->SetStringParameter(paramId, valueString);
//   }
   else if (label == wxT("InterpolationOrder"))
   {
      valueString = interpolationOrderTextCtrl->GetValue().c_str();
      if (CheckInteger(valueInteger, valueString, wxT("InterpolationOrder"),
                       wxT("Integer Number > 0.0"), false, true, true, false))
         clonedObj->SetIntegerParameter(paramId, valueInteger);
   }
   else if (label == wxT("StepSize"))
   {
      valueString = stepSizeComboBox->GetValue();
      if ((valueString == wxT("IntegratorSteps")) ||
          (CheckReal(stepSize, valueString, wxT("StepSize"), wxT("Real Number > 0.0 or ")
                wxT("equals 'IntegratorSteps'"), false, true, true, false)))
         clonedObj->SetStringParameter(paramId, valueString);
   }
   else if (label == wxT("EpochFormat"))
   {
      valueString = epochFormatComboBox->GetValue();
      clonedObj->SetStringParameter(paramId, valueString);
   }
   else if (label == wxT("InitialEpoch"))
   {
      valueString = initialEpochComboBox->GetValue();
      clonedObj->SetStringParameter(paramId, valueString);
   }
   else if (label == wxT("FinalEpoch"))
   {
      valueString = finalEpochComboBox->GetValue();
      clonedObj->SetStringParameter(paramId, valueString);
   }
   
}


//------------------------------------------------------------------------------
// void OnComboBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
/**
 * Event handler for comboboxes
 * 
 * Activates the Apply button when selection is changed.
 * 
 * @param event The triggering event.
 */
//------------------------------------------------------------------------------
void EphemerisFilePanel::OnComboBoxChange(wxCommandEvent& event)
{
   // Make the file type and interpolator compatible
   if (event.GetEventObject() == fileFormatComboBox)
   {
      // MessageInterface::ShowMessage("File format ComboBoxChange\n");
      wxString selection = fileFormatComboBox->GetValue();
      if (selection == wxT("SPK"))
         interpolatorComboBox->SetValue(wxT("Hermite"));
      if (selection == wxT("CCSDS-OEM"))
         interpolatorComboBox->SetValue(wxT("Lagrange"));
   }

   // Update the epoch data fields if the format changes and they are times
   if (event.GetEventObject() == epochFormatComboBox)
   {
      #ifdef DEBUG_TIME_CONVERSIONS
         MessageInterface::ShowMessage(wxT("Epoch format changed from '%s' to ")
               wxT("'%s'\n"), previousEpochFormat.c_str(),
               epochFormatComboBox->GetValue().c_str());
      #endif
      if (previousEpochFormat == wxT(""))
         return;

      wxString shownEpoch;
      wxString oldEpoch, newEpoch, epFormat;
      Real fromVal;
      Real toVal = -999.999;
      epFormat = epochFormatComboBox->GetValue().c_str();

      // Start epoch
      shownEpoch = initialEpochComboBox->GetValue();

      #ifdef DEBUG_TIME_CONVERSIONS
         MessageInterface::ShowMessage(wxT("   Displayed initial epoch is %s\n"),
               shownEpoch.c_str());
      #endif

      Integer id = mObject->GetParameterID(wxT("InitialEpoch"));
      StringArray enumStrings = mObject->GetPropertyEnumStrings(id);
      if (find(enumStrings.begin(), enumStrings.end(), shownEpoch.c_str()) ==
            enumStrings.end())
      {
         // Not in the list, so must be an epoch string
         oldEpoch = shownEpoch.c_str();

         if (previousEpochFormat.Find(wxT("ModJulian")) == wxNOT_FOUND)
            fromVal = -999.999;
         else
         {
            shownEpoch.ToDouble(&fromVal);
            if (fromVal < 6116.0)
               throw GmatBaseException(wxT("ModJulian epochs must be later than ")
                     wxT("(or equal to) 6116, the date Sputnik launched."));
         }

         TimeConverterUtil::Convert(previousEpochFormat.c_str(), fromVal,
               oldEpoch, epFormat, toVal, newEpoch);

         initialEpochComboBox->SetValue(newEpoch.c_str());
      }

      // End epoch
      shownEpoch = finalEpochComboBox->GetValue();

      #ifdef DEBUG_TIME_CONVERSIONS
         MessageInterface::ShowMessage(wxT("   Displayed final epoch is %s\n"),
               shownEpoch.c_str());
      #endif

      id = mObject->GetParameterID(wxT("FinalEpoch"));
      enumStrings = mObject->GetPropertyEnumStrings(id);
      if (find(enumStrings.begin(), enumStrings.end(), shownEpoch.c_str()) ==
            enumStrings.end())
      {
         // Not in the list, so must be an epoch string
         oldEpoch = shownEpoch.c_str();

         if (previousEpochFormat.Find(wxT("ModJulian")) == wxNOT_FOUND)
            fromVal = -999.999;
         else
         {
            shownEpoch.ToDouble(&fromVal);
            if (fromVal < 6116.0)
               throw GmatBaseException(wxT("ModJulian epochs must be later than ")
                     wxT("(or equal to) 6116, the date Sputnik launched."));
         }

         TimeConverterUtil::Convert(previousEpochFormat.c_str(), fromVal,
               oldEpoch, epFormat, toVal, newEpoch);

         finalEpochComboBox->SetValue(newEpoch.c_str());
      }

      previousEpochFormat = epochFormatComboBox->GetValue();
   }

   if (theApplyButton != NULL)
      EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnComboBoxTextChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
/**
 * Event handler for ComboBox text change
 * 
 * Activates the Apply button when selection is changed.
 * 
 * @param event The triggering event.
 */
//------------------------------------------------------------------------------
void EphemerisFilePanel::OnComboBoxTextChange(wxCommandEvent& event)
{
   if (theApplyButton != NULL)
      EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnTextChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
/**
 * Event handler for text boxes
 * 
 * Activates the Apply button when text is changed.
 * 
 * @param event The triggering event.
 */
//------------------------------------------------------------------------------
void EphemerisFilePanel::OnTextChange(wxCommandEvent& event)
{
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnWriteEphemerisChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void EphemerisFilePanel::OnCheck(wxCommandEvent& event)
{
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnBrowse(wxCommandEvent &event)
// This function is used to open file dialog.
//------------------------------------------------------------------------------
void EphemerisFilePanel::OnBrowse(wxCommandEvent &event)
{
   if (fileDialog == NULL)
   {
      // create fileDialog object when it does not exist
      wxString caption = wxT("Choose a File");
      wxString defaultDir = wxT("./output/");
      wxString defaultFile = wxEmptyString;
      wxString wildcard = wxT("*.*");
      
      fileDialog = new wxFileDialog(this, caption, 
                                    defaultDir, defaultFile, 
                                    wildcard, wxOPEN); 
   }
   else
   {
      // show fileDialog when it exists 
      fileDialog->Show();
   }
   
   if (fileDialog->ShowModal() == wxID_OK)
   {
      // change file name when a new file is chosen
      wxString filename = fileDialog->GetFilename();
      fileNameTextCtrl->SetValue(filename);
   }
   else
   {
      // only hide fileDialog when click on Cancel button
      fileDialog->Hide();
   }
   
}
