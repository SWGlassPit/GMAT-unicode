//$Id$
//------------------------------------------------------------------------------
//                            GroundStationPanel
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
// Author: Thomas Grubb
// Created: 2010/03/11
// Modified: 
//    2010.03.26 Thomas Grubb 
//      - Fixed positive real number check in SaveData for Longitude
/**
 * This class contains information needed to set up users ground station
 * parameters.
 */
//------------------------------------------------------------------------------
#include "GroundStationPanel.hpp"
#include "GmatConstants.hpp"
#include "MessageInterface.hpp"
#include "GmatStaticBoxSizer.hpp"
#include "BodyFixedStateConverter.hpp"
#include "CelestialBody.hpp"
#include "GuiInterpreter.hpp"
#include <wx/config.h>
#include "StringUtil.hpp"
#include <wx/variant.h>

//------------------------------
// event tables for wxWindows
//------------------------------
BEGIN_EVENT_TABLE(GroundStationPanel, wxPanel)
   EVT_BUTTON(ID_BUTTON_OK, GmatPanel::OnOK)
   EVT_BUTTON(ID_BUTTON_APPLY, GmatPanel::OnApply)
   EVT_BUTTON(ID_BUTTON_CANCEL, GmatPanel::OnCancel)
   EVT_BUTTON(ID_BUTTON_SCRIPT, GmatPanel::OnScript)
   EVT_TEXT(ID_LOCATION_TEXTCTRL, GroundStationPanel::OnLocationTextChange)
   EVT_TEXT(ID_STATION_ID_TEXTCTRL, GroundStationPanel::OnStationIDTextChange)
//   EVT_TEXT(ID_HARDWARE_TEXTCTRL, GroundStationPanel::OnHardwareTextChange)
   EVT_COMBOBOX(ID_COMBOBOX, GroundStationPanel::OnComboBoxChange)
   EVT_COMBOBOX(ID_STATE_TYPE_COMBOBOX, GroundStationPanel::OnStateTypeComboBoxChange)
   EVT_COMBOBOX(ID_HORIZON_REFERENCE_COMBOBOX, GroundStationPanel::OnHorizonReferenceComboBoxChange)
   EVT_BUTTON(ID_BUTTON_HELP, GmatPanel::OnHelp)
END_EVENT_TABLE()

//------------------------------
// public methods
//------------------------------

//------------------------------------------------------------------------------
// GroundStationPanel(wxWindow *parent, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Constructs GroundStationPanel object.
 */
//------------------------------------------------------------------------------
GroundStationPanel::GroundStationPanel(wxWindow *parent, const wxString &name)
   : GmatPanel(parent)
{           
   wxString groundName = wxString(name);
   theGroundStation = (GroundStation*)theGuiInterpreter->GetConfiguredObject(groundName);

   guiManager     = GuiItemManager::GetInstance();
   guiInterpreter = GmatAppData::Instance()->GetGuiInterpreter();
   ss             = guiInterpreter->GetSolarSystemInUse();

   if (theGroundStation)
   {
      Create();
      Show();
   }
   else
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_, wxT("Cannot find the GroundStation object named ") + groundName);
   }
}

//------------------------------------------------------------------------------
// ~GroundStationPanel()
//------------------------------------------------------------------------------
GroundStationPanel::~GroundStationPanel()
{
    GuiItemManager::GetInstance()->UnregisterComboBox(wxT("CelestialBody"), centralBodyComboBox);
}

//-------------------------------
// private methods
//-------------------------------

//----------------------------------
// methods inherited from GmatPanel
//----------------------------------

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
void GroundStationPanel::Create()
{
   // create local copy of ground station
   localGroundStation = new GroundStation(*theGroundStation);

    // Border size
   int minLabelSize;
   Integer bsize = 2;
   Integer labelSizeProportion = 0;
   Integer ctrlSizeProportion = 1;
   Integer unitSizeProportion = 0;
   
   // get the config object
   wxConfigBase *pConfig = wxConfigBase::Get();
   // SetPath() understands ".."
   pConfig->SetPath(wxT("/Ground Station"));

   //-----------------------------------------------------------------
   // Create controls in tab order
   //-----------------------------------------------------------------
   // Station ID
   wxStaticText *stationIDLabel =
      new wxStaticText( this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("ID") );
   stationIDTextCtrl =
      new wxTextCtrl( this, ID_STATION_ID_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(120,-1), 0 );
   stationIDTextCtrl->SetToolTip(pConfig->Read((GroundStation::PARAMETER_TEXT[GroundStation::STATION_ID-GroundStation::STATION_ID]+wxT("Hint"))));
   
   // Central Body
   wxStaticText *centralBodyLabel =
      new wxStaticText( this, ID_TEXT, wxT("Central ")wxT(GUI_ACCEL_KEY) wxT("Body"));
   StringArray centralBodyList;
   centralBodyList.push_back(wxT("Earth"));
   wxArrayString wxCentralBodyLabels = ToWxArrayString(centralBodyList);
   centralBodyComboBox = new wxComboBox( this, ID_COMBOBOX, wxT(""), wxDefaultPosition, wxSize(120,-1),
         wxCentralBodyLabels, wxCB_DROPDOWN|wxCB_READONLY);
// for now, we are only allowing Earth as the central body; when code is updated to use any body
// as central body, un-comment the following code:
//   centralBodyComboBox = GuiItemManager::GetInstance()->GetCelestialBodyComboBox(this, ID_COMBOBOX,
//                                                              wxSize(150,-1));
   centralBodyComboBox->SetToolTip(pConfig->Read((BodyFixedPoint::PARAMETER_TEXT[BodyFixedPoint::CENTRAL_BODY-BodyFixedPoint::CENTRAL_BODY]+wxT("Hint"))));
   
   // State Type
   wxStaticText *stateTypeLabel =
      new wxStaticText( this, ID_TEXT, wxT("State ") wxT(GUI_ACCEL_KEY) wxT("Type"));
   StringArray stateTypeList =
       localGroundStation->GetPropertyEnumStrings(BodyFixedPoint::STATE_TYPE);
   wxArrayString wxStateTypeLabels = ToWxArrayString(stateTypeList);
   stateTypeComboBox = 
      new wxComboBox( this, ID_STATE_TYPE_COMBOBOX, wxT(""), wxDefaultPosition, wxSize(120,-1),
                      wxStateTypeLabels, wxCB_DROPDOWN|wxCB_READONLY);
   stateTypeComboBox->SetToolTip(pConfig->Read((BodyFixedPoint::PARAMETER_TEXT[BodyFixedPoint::STATE_TYPE-BodyFixedPoint::CENTRAL_BODY]+wxT("Hint"))));
   
   // Horizon Reference
   wxStaticText *horizonReferenceLabel =
      new wxStaticText( this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Horizon Reference"));
   StringArray horizonReferenceList =
       localGroundStation->GetPropertyEnumStrings(BodyFixedPoint::HORIZON_REFERENCE);
   wxArrayString wxHorizonReferenceLabels = ToWxArrayString(horizonReferenceList);
   horizonReferenceComboBox = 
      new wxComboBox( this, ID_HORIZON_REFERENCE_COMBOBOX, wxT(""), wxDefaultPosition, wxSize(120,-1),
                      wxHorizonReferenceLabels, wxCB_DROPDOWN|wxCB_READONLY);
   horizonReferenceComboBox->SetToolTip(pConfig->Read((BodyFixedPoint::PARAMETER_TEXT[BodyFixedPoint::HORIZON_REFERENCE-BodyFixedPoint::CENTRAL_BODY]+wxT("Hint"))));
   
   // Location 1
   location1Label =
       new wxStaticText( this, ID_TEXT, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_1));
   location1TextCtrl =
      new wxTextCtrl( this, ID_LOCATION_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(120,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   //location1TextCtrl->SetToolTip(pConfig->Read(localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_1+"Hint")));
   location1Unit =
      new wxStaticText( this, ID_TEXT, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_UNITS_1));
   
   // Location 2
   location2Label =
       new wxStaticText( this, ID_TEXT, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_2));
   location2TextCtrl =
      new wxTextCtrl( this, ID_LOCATION_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(120,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   //location2TextCtrl->SetToolTip(pConfig->Read(localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_2+"Hint")));
   location2Unit =
       new wxStaticText( this, ID_TEXT, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_UNITS_2));
   
   // Location 3
   location3Label =
       new wxStaticText( this, ID_TEXT, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_3));
   location3TextCtrl =
      new wxTextCtrl( this, ID_LOCATION_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(120,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   //location3TextCtrl->SetToolTip(pConfig->Read(localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_3+"Hint")));
   location3Unit =
       new wxStaticText( this, ID_TEXT, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_UNITS_3));
   
   // Hardware
   //hardwareLabel =
   //    new wxStaticText( this, ID_TEXT, localGroundStation->GetStringParameter(BodyFixedPoint::HARDWARE));
//   wxStaticText *hardwareLabel =
//       new wxStaticText( this, ID_TEXT, wxT("Hardware"));
//   hardwareTextCtrl =
//      new wxTextCtrl( this, ID_HARDWARE_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(120,-1), 0 );
//   //hardwareTextCtrl->SetToolTip(pConfig->Read(BodyFixedPoint::PARAMETER_TEXT[BodyFixedPoint::HARDWARE-BodyFixedPoint::CENTRAL_BODY]wxT("Hint")));
//   hardwareTextCtrl->SetToolTip(pConfig->Read(wxT("HardwareHint")));
   
   // update labels and tooltips based on statetype
   UpdateControls();

   // set the min width for one of the labels for each GmatStaticBoxSizer
   minLabelSize = stationIDLabel->GetBestSize().x;
   minLabelSize = (minLabelSize < centralBodyLabel->GetBestSize().x) ? centralBodyLabel->GetBestSize().x : minLabelSize;
   minLabelSize = (minLabelSize < stateTypeLabel->GetBestSize().x) ? stateTypeLabel->GetBestSize().x : minLabelSize;
   minLabelSize = (minLabelSize < horizonReferenceLabel->GetBestSize().x) ? horizonReferenceLabel->GetBestSize().x : minLabelSize;
   minLabelSize = (minLabelSize < location1Label->GetBestSize().x) ? location1Label->GetBestSize().x : minLabelSize;
   minLabelSize = (minLabelSize < location2Label->GetBestSize().x) ? location2Label->GetBestSize().x : minLabelSize;
   minLabelSize = (minLabelSize < location3Label->GetBestSize().x) ? location3Label->GetBestSize().x : minLabelSize;
//   minLabelSize = (minLabelSize < hardwareLabel->GetBestSize().x) ? hardwareLabel->GetBestSize().x : minLabelSize;

   stationIDLabel->SetMinSize(wxSize(minLabelSize, stationIDLabel->GetMinHeight()));
   centralBodyLabel->SetMinSize(wxSize(minLabelSize, centralBodyLabel->GetMinHeight()));
//   hardwareLabel->SetMinSize(wxSize(minLabelSize, hardwareLabel->GetMinHeight()));

   //-----------------------------------------------------------------
   // Add to Station ID sizer
   //-----------------------------------------------------------------
   wxFlexGridSizer *flexGridSizer1 = new wxFlexGridSizer( 3, 0, 0 );
   //flexGridSizer1->AddGrowableCol(1);
   flexGridSizer1->Add(stationIDLabel, labelSizeProportion, wxALIGN_LEFT|wxALL, bsize);
   flexGridSizer1->Add(stationIDTextCtrl, ctrlSizeProportion, wxGROW|wxALL, bsize);
   flexGridSizer1->Add(0, unitSizeProportion, wxALIGN_LEFT|wxALL, bsize);
   
   theMiddleSizer->Add(flexGridSizer1, 0, wxEXPAND|wxALL, bsize);
   
   //-----------------------------------------------------------------
   // Add to location properties sizer
   //-----------------------------------------------------------------
   wxFlexGridSizer *flexGridSizer2 = new wxFlexGridSizer( 3, 0, 0 );
   //flexGridSizer2->AddGrowableCol(1);
   
   flexGridSizer2->Add(centralBodyLabel, labelSizeProportion, wxALIGN_LEFT|wxALL, bsize);
   flexGridSizer2->Add(centralBodyComboBox, ctrlSizeProportion, wxGROW|wxALL, bsize);
   flexGridSizer2->Add(0, unitSizeProportion, wxALIGN_LEFT|wxALL, bsize);

   flexGridSizer2->Add(stateTypeLabel, labelSizeProportion, wxALIGN_LEFT|wxALL, bsize);
   flexGridSizer2->Add(stateTypeComboBox, ctrlSizeProportion, wxGROW|wxALL, bsize);
   flexGridSizer2->Add(0, unitSizeProportion, wxALIGN_LEFT|wxALL, bsize);
   
   flexGridSizer2->Add(horizonReferenceLabel, labelSizeProportion, wxALIGN_LEFT|wxALL, bsize);
   flexGridSizer2->Add(horizonReferenceComboBox, ctrlSizeProportion, wxGROW|wxALL, bsize);
   flexGridSizer2->Add(0, unitSizeProportion, wxALIGN_LEFT|wxALL, bsize);

   flexGridSizer2->Add(location1Label, labelSizeProportion, wxALIGN_LEFT|wxALL, bsize);
   flexGridSizer2->Add(location1TextCtrl, ctrlSizeProportion, wxGROW|wxALL, bsize);
   flexGridSizer2->Add(location1Unit, unitSizeProportion, wxALIGN_LEFT|wxALL, bsize);

   flexGridSizer2->Add(location2Label, labelSizeProportion, wxALIGN_LEFT|wxALL, bsize);
   flexGridSizer2->Add(location2TextCtrl, ctrlSizeProportion, wxGROW|wxALL, bsize);
   flexGridSizer2->Add(location2Unit, unitSizeProportion, wxALIGN_LEFT|wxALL, bsize);

   flexGridSizer2->Add(location3Label, labelSizeProportion, wxALIGN_LEFT|wxALL, bsize);
   flexGridSizer2->Add(location3TextCtrl, ctrlSizeProportion, wxGROW|wxALL, bsize);
   flexGridSizer2->Add(location3Unit, unitSizeProportion, wxALIGN_LEFT|wxALL, bsize);

   // create the location Properties group box
   locationPropertiesSizer = new GmatStaticBoxSizer( wxVERTICAL, this, wxT("Location") );
   locationPropertiesSizer->Add(flexGridSizer2, 0, wxEXPAND|wxALL, bsize);

   //-----------------------------------------------------------------
   // Add to hardware properties sizer
   //-----------------------------------------------------------------
//   wxFlexGridSizer *flexGridSizer3 = new wxFlexGridSizer( 3, 0, 0 );
//   //flexGridSizer2->AddGrowableCol(1);
//
//   flexGridSizer3->Add(hardwareLabel, labelSizeProportion, wxALIGN_LEFT|wxALL, bsize);
//   flexGridSizer3->Add(hardwareTextCtrl, ctrlSizeProportion, wxGROW|wxALL, bsize);
//
//   // create the hardware Properties group box
//   hardwarePropertiesSizer = new GmatStaticBoxSizer( wxVERTICAL, this, "Hardware" );
//   hardwarePropertiesSizer->Add(flexGridSizer3, 0, wxEXPAND|wxALL, bsize);

   //-----------------------------------------------------------------
   // Now put ground & fuel properties sizers into the middle sizer
   //-----------------------------------------------------------------
   theMiddleSizer->Add(locationPropertiesSizer, 1, wxEXPAND|wxALL, bsize);
//   theMiddleSizer->Add(hardwarePropertiesSizer, 0, wxEXPAND|wxALL, bsize);
   theMiddleSizer->SetSizeHints(this);

}

//------------------------------------------------------------------------------
// void LoadData()
//------------------------------------------------------------------------------
void GroundStationPanel::LoadData()
{ 
   if (localGroundStation == NULL)
      return; 
   
   // Set object pointer for "Show Script"
   mObject = theGroundStation;

   try
   {
      stationIDTextCtrl->SetValue(wxVariant(localGroundStation->GetStringParameter(GroundStation::STATION_ID)));
      centralBodyComboBox->SetValue(wxVariant(localGroundStation->GetStringParameter(BodyFixedPoint::CENTRAL_BODY)));
      currentStateType = localGroundStation->GetStringParameter(BodyFixedPoint::STATE_TYPE);
      stateTypeComboBox->SetValue(wxVariant(currentStateType));
      currentHorizonReference = localGroundStation->GetStringParameter(BodyFixedPoint::HORIZON_REFERENCE);
      horizonReferenceComboBox->SetValue(wxVariant(currentHorizonReference));
      location1 = localGroundStation->GetRealParameter(BodyFixedPoint::LOCATION_1);
      location1TextCtrl->SetValue(wxVariant(location1));
      location2 = localGroundStation->GetRealParameter(BodyFixedPoint::LOCATION_2);
      location2TextCtrl->SetValue(wxVariant(location2));
      location3 = localGroundStation->GetRealParameter(BodyFixedPoint::LOCATION_3);
      location3TextCtrl->SetValue(wxVariant(location3));
      //hardwareTextCtrl->SetValue(wxVariant(localGroundStation->GetStringParameter(GroundStation::HARDWARE)));
      
      // update labels and tooltips based on statetype
      UpdateControls();
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
}


//------------------------------------------------------------------------------
// void SaveData()
//------------------------------------------------------------------------------
void GroundStationPanel::SaveData()
{
   canClose = true;
   
//   Real location1, location2, location3;
   wxString inputString;
   wxString text;
   
   //-----------------------------------------------------------------
   // validate user input for locations if state type is not cartesian
   //   X,Y,Z May be any real
   //   Latitude is -90 to 90, Longitude 0 to 360, Altitude is any Real
   // NOTE - we are just checking for Real or non-negative real here;
   //        base code checks for actual ranges on SetrealParameter call
   //-----------------------------------------------------------------
   try
   {
      // State Type
      text = stateTypeComboBox->GetValue();
      localGroundStation->SetStringParameter(BodyFixedPoint::STATE_TYPE, text);
   }
   catch (BaseException &ex)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage());
      canClose = false;
   }

   // Location 1 (X or Latitude)
   inputString = location1TextCtrl->GetValue();
   CheckReal(location1, inputString, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_1), wxT("Real Number"));
   
   // Location 2 (Y or Longitude)
   inputString = location2TextCtrl->GetValue();
   if (text != wxT("Cartesian"))
      CheckReal(location2, inputString, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_2), wxT("Real Number >= 0.0"), false, true, true, true);
   else
      CheckReal(location2, inputString, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_2), wxT("Real Number"));
   
   // Location 3 (Z or Altitude)
   inputString = location3TextCtrl->GetValue();
   CheckReal(location3, inputString, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_3), wxT("Real Number"));

   if (!canClose)
      return;
   
   //-----------------------------------------------------------------
   // save values to base, base code should do the range checking
   //-----------------------------------------------------------------
   try
   {
      // Station ID
      text = stationIDTextCtrl->GetValue();
      localGroundStation->SetStringParameter(GroundStation::STATION_ID, text);
   }
   catch (BaseException &ex)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage());
      canClose = false;
   }
   try
   {
      // Central Body
      text = centralBodyComboBox->GetValue();
      localGroundStation->SetStringParameter(BodyFixedPoint::CENTRAL_BODY, text);
   }
   catch (BaseException &ex)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage());
      canClose = false;
   }
   try
   {
      // Horizon Reference
      text = horizonReferenceComboBox->GetValue();
      localGroundStation->SetStringParameter(BodyFixedPoint::HORIZON_REFERENCE, text);
   }
   catch (BaseException &ex)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage());
      canClose = false;
   }
   try
   {
      // location 1
      localGroundStation->SetRealParameter(BodyFixedPoint::LOCATION_1, location1);
   }
   catch (BaseException &ex)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage());
      canClose = false;
   }
   try
   {
      // location 2
      localGroundStation->SetRealParameter(BodyFixedPoint::LOCATION_2, location2);
   }
   catch (BaseException &ex)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage());
      canClose = false;
   }
   try
   {
      // location 3
      localGroundStation->SetRealParameter(BodyFixedPoint::LOCATION_3, location3);
   }
   catch (BaseException &ex)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage());
      canClose = false;
   }
   try
   {
      // Hardware
//      text = hardwareTextCtrl->GetValue();
      //localGroundStation->SetStringParameter(GroundStation::HARDWARE, text);
   }
   catch (BaseException &ex)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage());
      canClose = false;
   }

   if (!canClose)
      return;
   // copy the current info into the ground station
   theGroundStation->Copy(localGroundStation);
   EnableUpdate(false);
}


//------------------------------------------------------------------------------
// void UpdateControls()
//------------------------------------------------------------------------------
void GroundStationPanel::UpdateControls()
{
//   bool enableHorizon = localGroundStation->GetStringParameter(BodyFixedPoint::STATE_TYPE) != "Cartesian";
   bool enableHorizon = currentStateType != wxT("Cartesian");
   horizonReferenceComboBox->Enable(enableHorizon);

   // get the config object
   wxConfigBase *pConfig = wxConfigBase::Get();
   // SetPath() understands ".."
   pConfig->SetPath(wxT("/Ground Station"));

   location1Label->SetLabel(localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_1));
   location1TextCtrl->SetToolTip(pConfig->Read((localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_1+wxT("Hint")))));
   location1Unit->SetLabel(localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_UNITS_1));
   location2Label->SetLabel(localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_2));
   location2TextCtrl->SetToolTip(pConfig->Read((localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_2+wxT("Hint")))));
   location2Unit->SetLabel(localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_UNITS_2));
   location3Label->SetLabel(localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_3));
   location3TextCtrl->SetToolTip(pConfig->Read((localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_3+wxT("Hint")))));
   location3Unit->SetLabel(localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_UNITS_3));
}    


//------------------------------------------------------------------------------
// void OnLocationTextChange()
//------------------------------------------------------------------------------
void GroundStationPanel::OnLocationTextChange(wxCommandEvent &event)
{
   EnableUpdate(true);
}    


//------------------------------------------------------------------------------
// void OnStationIDTextChange()
//------------------------------------------------------------------------------
void GroundStationPanel::OnStationIDTextChange(wxCommandEvent &event)
{
   EnableUpdate(true);
}    


//------------------------------------------------------------------------------
// void OnHardwareTextChange()
//------------------------------------------------------------------------------
void GroundStationPanel::OnHardwareTextChange(wxCommandEvent &event)
{
   EnableUpdate(true);
}    


//------------------------------------------------------------------------------
// void OnComboBoxChange()
//------------------------------------------------------------------------------
void GroundStationPanel::OnComboBoxChange(wxCommandEvent &event)
{
   EnableUpdate(true);
}    


//------------------------------------------------------------------------------
// void OnStateTypeComboBoxChange()
//------------------------------------------------------------------------------
void GroundStationPanel::OnStateTypeComboBoxChange(wxCommandEvent &event)
{
   wxString sttype       = stateTypeComboBox->GetValue();
   wxString inputString;
   Real        location1, location2, location3;
   if (sttype != currentStateType)
   {
      wxString bodyName = centralBodyComboBox->GetValue();
      // get a pointer to the celestial body
      CelestialBody *body = ss->GetBody(bodyName);
      if (!body)
      {
         wxString errmsg = wxT("Cannot find body ");
         errmsg += bodyName + wxT(" needed for GroundStation panel update.\n");
         throw GmatBaseException(errmsg);
      }
      Real meanRadius = body->GetRealParameter(body->GetParameterID(wxT("EquatorialRadius")));
      Real flattening = body->GetRealParameter(body->GetParameterID(wxT("Flattening")));
      // Convert location values to the appropriate values
      // Location 1 (Latitude)
      inputString = location1TextCtrl->GetValue();
      CheckReal(location1, inputString, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_1), wxT("Real Number"));

      // Location 2 (Longitude)
      inputString = location2TextCtrl->GetValue();
      if (currentStateType != wxT("Cartesian"))
         CheckReal(location2, inputString, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_2), wxT("Real Number >= 0.0"), false, true, true, true);
      else
         CheckReal(location2, inputString, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_2), wxT("Real Number"));

      // Location 3 (Altitude)
      inputString = location3TextCtrl->GetValue();
      CheckReal(location3, inputString, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_3), wxT("Real Number"));

      Rvector3 locInCurrent(location1, location2, location3);
      if (currentStateType == wxT("Spherical")) // latitude and longitude need to be passed in as radians
      {
         locInCurrent[0] *= GmatMathConstants::RAD_PER_DEG;
         locInCurrent[1] *= GmatMathConstants::RAD_PER_DEG;
      }
      Rvector3 locInNew = BodyFixedStateConverterUtil::Convert(locInCurrent, currentStateType,
                          currentHorizonReference, sttype, currentHorizonReference,
                          flattening, meanRadius);
      location1 = locInNew[0];
      location2 = locInNew[1];
      location3 = locInNew[2];
      if (sttype == wxT("Spherical")) // need to display DEGREES for latitude and longitude
      {
         location1 *= GmatMathConstants::DEG_PER_RAD;
         location2 *= GmatMathConstants::DEG_PER_RAD;
      }
      localGroundStation->SetStringParameter(BodyFixedPoint::STATE_TYPE, sttype);
      localGroundStation->SetRealParameter(BodyFixedPoint::LOCATION_1, location1);
      localGroundStation->SetRealParameter(BodyFixedPoint::LOCATION_2, location2);
      localGroundStation->SetRealParameter(BodyFixedPoint::LOCATION_3, location3);
      location1TextCtrl->SetValue(wxVariant(location1));
      location2TextCtrl->SetValue(wxVariant(location2));
      location3TextCtrl->SetValue(wxVariant(location3));
      currentStateType = sttype;
   }
   UpdateControls();
   EnableUpdate(true);
}

//------------------------------------------------------------------------------
// void OnHorizonReferenceComboBoxChange()
//------------------------------------------------------------------------------
void GroundStationPanel::OnHorizonReferenceComboBoxChange(wxCommandEvent &event)
{
   wxString horizon       = horizonReferenceComboBox->GetValue();
   wxString inputString;
   Real        location1, location2, location3;
   if (horizon != currentHorizonReference)
   {
      wxString bodyName = centralBodyComboBox->GetValue();
      // get a pointer to the celestial body
      CelestialBody *body = ss->GetBody(bodyName);
      if (!body)
      {
         wxString errmsg = wxT("Cannot find body ");
         errmsg += bodyName + wxT(" needed for GroundStation panel update.\n");
         throw GmatBaseException(errmsg);
      }
      Real meanRadius = body->GetRealParameter(body->GetParameterID(wxT("EquatorialRadius")));
      Real flattening = body->GetRealParameter(body->GetParameterID(wxT("Flattening")));
      // Convert location values to the appropriate values
      // Location 1 (Latitude)
      inputString = location1TextCtrl->GetValue();
      CheckReal(location1, inputString, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_1), wxT("Real Number"));

      // Location 2 (Longitude)
      inputString = location2TextCtrl->GetValue();
      if (currentStateType != wxT("Cartesian"))
         CheckReal(location2, inputString, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_2), wxT("Real Number >= 0.0"), false, true, true, true);
      else
         CheckReal(location2, inputString, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_2), wxT("Real Number"));

      // Location 3 (Altitude)
      inputString = location3TextCtrl->GetValue();
      CheckReal(location3, inputString, localGroundStation->GetStringParameter(BodyFixedPoint::LOCATION_LABEL_3), wxT("Real Number"));

      Rvector3 locInCurrent(location1, location2, location3);
      if (currentStateType == wxT("Spherical")) // latitude and longitude need to be passed in as radians
      {
         locInCurrent[0] *= GmatMathConstants::RAD_PER_DEG;
         locInCurrent[1] *= GmatMathConstants::RAD_PER_DEG;
      }
      MessageInterface::ShowMessage(wxT(" ... Spherical to new horizon ... loc = %12.10f  %12.10f  %12.10f\n"),
            locInCurrent[0], locInCurrent[1], locInCurrent[2]); // *************************
      Rvector3 locInNew = BodyFixedStateConverterUtil::Convert(locInCurrent, currentStateType,
                          currentHorizonReference, currentStateType, horizon,
                          flattening, meanRadius);
      MessageInterface::ShowMessage(wxT(" ... result =  %12.10f  %12.10f  %12.10f\n"),
            locInNew[0], locInNew[1], locInNew[2]); // *************************
      location1 = locInNew[0];
      location2 = locInNew[1];
      location3 = locInNew[2];
      if (currentStateType == wxT("Spherical")) // need to display DEGREES for latitude and longitude
      {
         location1 *= GmatMathConstants::DEG_PER_RAD;
         location2 *= GmatMathConstants::DEG_PER_RAD;
      }
      localGroundStation->SetStringParameter(BodyFixedPoint::HORIZON_REFERENCE, horizon);
      localGroundStation->SetRealParameter(BodyFixedPoint::LOCATION_1, location1);
      localGroundStation->SetRealParameter(BodyFixedPoint::LOCATION_2, location2);
      localGroundStation->SetRealParameter(BodyFixedPoint::LOCATION_3, location3);
      location1TextCtrl->SetValue(wxVariant(location1));
      location2TextCtrl->SetValue(wxVariant(location2));
      location3TextCtrl->SetValue(wxVariant(location3));
      currentHorizonReference = horizon;
   }
   UpdateControls();
   EnableUpdate(true);
}    



