//$Id: CoordPanel.cpp 9676 2011-07-01 17:53:28Z wendys-dev $
//------------------------------------------------------------------------------
//                              CoordPanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Allison Greene
// Created: 2005/03/11
/**
 * This class contains the Coordinate System Panel for CoordSystemConfigPanel
 * and CoordSysCreateDialog.
 */
//------------------------------------------------------------------------------
#include "CoordPanel.hpp"
#include "AxisSystem.hpp"
#include "SpacePoint.hpp"
#include "TimeSystemConverter.hpp"  // for Convert()
#include "StringUtil.hpp"           // for ToReal()
#include "MessageInterface.hpp"
#include "GmatConstants.hpp"
#include <wx/config.h>
#include <sstream>

//#define DEBUG_COORD_PANEL 1
//#define DEBUG_COORD_PANEL_SAVE
//#define DEBUG_COORD_EPOCH
//#define DEBUG_COORD_PANEL_PRIMARY_SECONDARY

//------------------------------------------------------------------------------
// CoordPanel()
//------------------------------------------------------------------------------
/**
 * A constructor.
 */
//------------------------------------------------------------------------------
CoordPanel::CoordPanel(wxWindow *parent, bool enableAll)
   : wxPanel(parent)
{
   theGuiInterpreter = GmatAppData::Instance()->GetGuiInterpreter();
   theGuiManager = GuiItemManager::GetInstance();
   
   mShowPrimaryBody = false;
   mShowSecondaryBody = false;
   mShowEpoch = false;
   mShowXyz = false;
   mShowUpdate = false;
   
   mEnableAll = enableAll;
   
////   epochValue = "21545";
//   wxStringstream mjdStr("");
//   mjdStr << GmatTimeConstants::MJD_OF_J2000;
//   epochValue = mjdStr.str();
//   epochFormatValue = "A1ModJulian";
      
   Create();
}


//------------------------------------------------------------------------------
// ~CoordPanel()
//------------------------------------------------------------------------------
CoordPanel::~CoordPanel()
{
   // Unregisger GUI components
   theGuiManager->UnregisterComboBox(wxT("SpacePoint"), originComboBox);
   theGuiManager->UnregisterComboBox(wxT("SpacePoint"), primaryComboBox);
   theGuiManager->UnregisterComboBox(wxT("SpacePoint"), secondaryComboBox);
}


//------------------------------------------------------------------------------
// void EnableOptions(AxisSystem *axis)
//------------------------------------------------------------------------------
void CoordPanel::EnableOptions(AxisSystem *axis)
{
   #if DEBUG_COORD_PANEL
   MessageInterface::ShowMessage
      (wxT("CoordPanel::EnableOptions() axis=(%p)%s\n"), axis,
       typeComboBox->GetStringSelection().c_str());
   #endif
   
   // save epoch value locally
   epochValue = epochTextCtrl->GetValue();
   
   wxString typeStr = typeComboBox->GetStringSelection();
   AxisSystem* tmpAxis = NULL;
   
   if (typeStr == wxT(""))
      typeStr = wxT("MJ2000Eq");

   if (axis == NULL)
      // create a temp axis to use flags
      tmpAxis = (AxisSystem *)theGuiInterpreter->
         CreateObject(typeStr.c_str(), wxT("")); // Use no name
   else
      tmpAxis = axis;
   
   if (tmpAxis == NULL)
      return;
   
   if (tmpAxis->UsesPrimary() == GmatCoordinate::NOT_USED)
      mShowPrimaryBody = false;
   else
      mShowPrimaryBody = true; 
   
   if (tmpAxis->UsesSecondary() == GmatCoordinate::NOT_USED)
      mShowSecondaryBody = false;
   else
      mShowSecondaryBody = true; 
   
   if (tmpAxis->UsesEpoch() == GmatCoordinate::NOT_USED)
      mShowEpoch = false;
   else
   {
      mShowEpoch = true; 
      
      // get the epoch format and value from tmpAxis
      Real epoch = tmpAxis->GetEpoch().Get();
      epochValue = theGuiManager->ToWxString(epoch);
//      epochFormatValue = wxString(tmpAxis->GetEpochFormat().c_str());

      #if DEBUG_COORD_PANEL
      MessageInterface::ShowMessage
         (wxT("CoordPanel::EnableOptions() about to set epoch value to %12.10f (string = %s)\n"),
               epoch, epochValue.c_str());
      #endif
      // set the text ctrl
      epochTextCtrl->SetValue(epochValue);
//      formatComboBox->SetValue(epochFormatValue);
   }
   #ifdef DEBUG_COORD_EPOCH
      MessageInterface::ShowMessage(wxT("mShowEpoch = %s\n"), (mShowEpoch? wxT("true") : wxT("false")));
   #endif
   
   if ((tmpAxis->UsesXAxis() == GmatCoordinate::NOT_USED) &&
       (tmpAxis->UsesYAxis() == GmatCoordinate::NOT_USED) &&
       (tmpAxis->UsesZAxis() == GmatCoordinate::NOT_USED))
      mShowXyz = false;
   else
      mShowXyz = true; 
   
   if (tmpAxis->UsesNutationUpdateInterval() == GmatCoordinate::NOT_USED)
      mShowUpdate = false;
   else
      mShowUpdate = true; 

   if (typeStr == wxT("ObjectReferenced"))
      SetDefaultObjectRefAxis();
   else if ((typeStr == wxT("TOEEq")) || (typeStr == wxT("TOEEc")))
      SetDefaultEpochRefAxis();
   else if ((typeStr == wxT("TODEq")) || (typeStr == wxT("TODEc")))
      SetDefaultEpochRefAxis();
   else if((typeStr == wxT("MOEEq")) || (typeStr == wxT("MOEEc")))
      SetDefaultEpochRefAxis();
   
   if (mEnableAll)
   {
      primaryStaticText->Enable(mShowPrimaryBody);
      primaryComboBox->Enable(mShowPrimaryBody);
      secondaryStaticText->Enable(mShowSecondaryBody);
      secondaryComboBox->Enable(mShowSecondaryBody);
//      formatStaticText->Enable(mShowEpoch);
      // arg: 1/23/05 - for now never enable
//      formatComboBox->Enable(false);
      epochStaticText->Enable(mShowEpoch);
      epochTextCtrl->Enable(mShowEpoch);
      xStaticText->Enable(mShowXyz);
      xComboBox->Enable(mShowXyz);
      yStaticText->Enable(mShowXyz);
      yComboBox->Enable(mShowXyz);
      zStaticText->Enable(mShowXyz);
      zComboBox->Enable(mShowXyz);
//      updateStaticText->Enable(mShowUpdate);
//      secStaticText->Enable(mShowUpdate);
//      intervalTextCtrl->Enable(mShowUpdate);
      
      // disable some items
      if (typeStr == wxT("GSE") || typeStr == wxT("GSM"))
      {
         primaryComboBox->SetStringSelection(wxT("Earth"));
         secondaryComboBox->SetStringSelection(wxT("Sun"));
         
         primaryStaticText->Enable(false);
         primaryComboBox->Enable(false);
         secondaryStaticText->Enable(false);
         secondaryComboBox->Enable(false);
      }
   }
   else  // disable all of them
   {
      originStaticText->Enable(false);
      typeStaticText->Enable(false);
      primaryStaticText->Enable(false);
//      formatStaticText->Enable(false);
      secondaryStaticText->Enable(false);
      epochStaticText->Enable(false);
      originComboBox->Enable(false);
      typeComboBox->Enable(false);
      primaryComboBox->Enable(false);
//      formatComboBox->Enable(false);
      secondaryComboBox->Enable(false);
      epochTextCtrl->Enable(false);
      xStaticText->Enable(false);
      xComboBox->Enable(false);
      yStaticText->Enable(false);
      yComboBox->Enable(false);
      zStaticText->Enable(false);
      zComboBox->Enable(false);
//      updateStaticText->Enable(false);
//      secStaticText->Enable(false);
//      intervalTextCtrl->Enable(false);
   }
   
   // 07/18/2006 commented out because it caused a crash
   //theGuiInterpreter->RemoveItemIfNotUsed(Gmat::AXIS_SYSTEM, "tmpAxis");
}


//------------------------------------------------------------------------------
// void SetDefaultAxis()
//------------------------------------------------------------------------------
void CoordPanel::SetDefaultAxis()
{
   // default settings
   typeComboBox->SetValue(wxT("MJ2000Eq"));
   originComboBox->SetValue(wxT("Earth"));
   primaryComboBox->SetValue(wxT("Earth"));
   secondaryComboBox->SetValue(wxT("Luna"));
//   formatComboBox->SetValue(epochFormatValue);
////   epochTextCtrl->SetValue("21545");
//   wxStringstream mjdStr("");
//   mjdStr << GmatTimeConstants::MJD_OF_J2000;
//   epochTextCtrl->SetValue(mjdStr.str());
   epochTextCtrl->SetValue(epochValue);
   xComboBox->SetValue(wxT("R"));
   yComboBox->SetValue(wxT(""));;
   zComboBox->SetValue(wxT("N"));
//   intervalTextCtrl->SetValue("60");
}


//------------------------------------------------------------------------------
// void SetDefaultEpochRefAxis()
//------------------------------------------------------------------------------
void CoordPanel::SetDefaultEpochRefAxis()
{
   // default settings
//   formatComboBox->SetValue(epochFormatValue);
////   epochTextCtrl->SetValue("21545");
//   wxStringstream mjdStr("");
//   mjdStr << GmatTimeConstants::MJD_OF_J2000;
//   epochTextCtrl->SetValue(mjdStr.str());
    epochTextCtrl->SetValue(epochValue);
}


//------------------------------------------------------------------------------
// void SetDefaultObjectRefAxis()
//------------------------------------------------------------------------------
void CoordPanel::SetDefaultObjectRefAxis()
{
   // default settings
   primaryComboBox->SetValue(wxT("Earth"));
   secondaryComboBox->SetValue(wxT("Luna"));
   xComboBox->SetValue(wxT("R"));
   yComboBox->SetValue(wxT(""));;
   zComboBox->SetValue(wxT("N"));
}


//------------------------------------------------------------------------------
// void ShowAxisData(AxisSystem *axis)
//------------------------------------------------------------------------------
void CoordPanel::ShowAxisData(AxisSystem *axis)
{
   #if DEBUG_COORD_PANEL
   MessageInterface::ShowMessage
      (wxT("CoordPanel::ShowAxisData() axis=(%p)%s\n"), axis,
       axis->GetTypeName().c_str());
   #endif
   
   try
   {
      int sel = typeComboBox->FindString(axis->GetTypeName().c_str());
      typeComboBox->SetSelection(sel);
      EnableOptions(axis);
      
      #if DEBUG_COORD_PANEL
      MessageInterface::ShowMessage
         (wxT("mShowPrimaryBody=%d, mShowSecondaryBody=%d, mShowEpoch=%d, ")
          wxT("mShowXyz=%d, mShowUpdate\n"), mShowPrimaryBody, mShowSecondaryBody,
          mShowEpoch, mShowXyz, mShowUpdate);
      #endif
      
      if (mShowPrimaryBody)
      {
         #ifdef DEBUG_COORD_PANEL_PRIMARY_SECONDARY
            MessageInterface::ShowMessage(wxT("Primary is %s\n"), (axis->GetStringParameter(wxT("Primary"))).c_str());
         #endif
         primaryComboBox->
            SetStringSelection(axis->GetStringParameter(wxT("Primary")).c_str());
      }
      
      if (mShowSecondaryBody)
      {
         #ifdef DEBUG_COORD_PANEL_PRIMARY_SECONDARY
            MessageInterface::ShowMessage(wxT("Secondary is %s\n"), (axis->GetStringParameter(wxT("Secondary"))).c_str());
         #endif
         secondaryComboBox->
            SetStringSelection(axis->GetStringParameter(wxT("Secondary")).c_str());
      }
      
      if (mShowEpoch)
      {
//         wxString epochFormat = axis->GetEpochFormat();
//         formatComboBox->SetStringSelection(epochFormat.c_str());
         
         Real epoch = axis->GetEpoch().Get();
         #if DEBUG_COORD_PANEL
            MessageInterface::ShowMessage
               (wxT("CoordPanel::ShowAxisData() about to set the value of epoch to %12.10f\n"), epoch);
         #endif
         epochTextCtrl->SetValue(theGuiManager->ToWxString(epoch));
      }
      
      if (mShowXyz)
      {
         xComboBox->SetStringSelection(axis->GetXAxis().c_str());
         yComboBox->SetStringSelection(axis->GetYAxis().c_str());
         zComboBox->SetStringSelection(axis->GetZAxis().c_str());
      }
      
//      if (mShowUpdate)
//      {
//         /// @todo:
//         Real updateInterval = axis->GetRealParameter("UpdateInterval");
//
//         wxString updateStr;
//         wxStringstream buffer;
//         buffer.precision(18);
//         buffer << updateInterval;
//         updateStr.Printf ("%s",buffer.str().c_str());
//
//         intervalTextCtrl->SetValue(updateStr);
//      }
   }
   catch (BaseException &e)
   {
      MessageInterface::ShowMessage
         (wxT("CoordPanel::ShowAxisData() error occurred in getting data!\n%s\n"),
          e.GetFullMessage().c_str());
   }
}


//------------------------------------------------------------------------------
// AxisSystem* CreateAxis()
//------------------------------------------------------------------------------
AxisSystem* CoordPanel::CreateAxis()
{
   wxString priName = primaryComboBox->GetValue().Trim();
   wxString secName = secondaryComboBox->GetValue().Trim();
   wxString axisType = typeComboBox->GetValue().Trim();
//   wxString epochFormat = formatComboBox->GetValue().Trim();
   wxString epochStr = epochTextCtrl->GetValue().Trim();
//   wxString updateStr = intervalTextCtrl->GetValue().Trim();
   wxString xStr = xComboBox->GetValue();
   wxString yStr = yComboBox->GetValue();
   wxString zStr = zComboBox->GetValue();
   
   AxisSystem *axis = NULL;
   
   if (IsValidAxis(axisType, priName, secName, xStr, yStr, zStr))
   {
      // Create AxisSystem
      axis = (AxisSystem *)theGuiInterpreter->
         //CreateAxisSystem(wxString(axisType.c_str()), "");
         CreateObject(axisType.c_str(), wxT(""));
      
      if (axis != NULL)
      {
         try
         {
            if (axis->UsesPrimary()) //  && priName != "")
            {
               SpacePoint *primary = (SpacePoint *)theGuiInterpreter->
                  GetConfiguredObject(wxString(priName.c_str()));
               axis->SetPrimaryObject(primary);
            }
            
            if (axis->UsesSecondary()) //  && secName != "")
            {
               SpacePoint *secondary = (SpacePoint *)theGuiInterpreter->
                  GetConfiguredObject(wxString(secName.c_str()));
               axis->SetSecondaryObject(secondary);
            }
            
            if (axis->UsesXAxis() || axis->UsesYAxis() || axis->UsesZAxis())
            {
               // set the x, y, and z
               axis->SetXAxis(wxString(xStr.c_str()));
               axis->SetYAxis(wxString(yStr.c_str()));
               axis->SetZAxis(wxString(zStr.c_str()));
            }
            
//            axis->SetEpochFormat(wxString(epochFormat.c_str()));
//            axis->SetRealParameter("UpdateInterval", atof(updateStr.c_str()));
            
            // convert epoch to a1mjd
            // if Epoch is not in A1ModJulian, convert to A1ModJulian(loj: 1/23/07)
            if (axis->UsesEpoch())
            {
               Real a1mjd;
               GmatStringUtil::ToReal(epochStr.c_str(), a1mjd);

   //            if (epochFormat != "" && epochFormat != "A1ModJulian")
   //            {
   //               // Use TimsSystemConverter instead of TimeConverter
   //               Real inputMjd = -999.999;
   //               Real a1mjd;
   //               wxString a1mjdStr;
   //               TimeConverterUtil::Convert(epochFormat.c_str(), inputMjd,
   //                                          epochStr.c_str(), "A1ModJulian",
   //                                          a1mjd, a1mjdStr);
   //
   //               //wxString taiEpochStr = mTimeConverter.Convert
   //               //   (wxString(epochStr.c_str()), wxString(epochFormat.c_str()),
   //               //    "TAIModJulian");
   //
   //               //Real epoch = TimeConverterUtil::ConvertFromTaiMjd
   //               //   (TimeConverterUtil::A1MJD, atof(taiEpochStr.c_str()),
   //               //    GmatTimeConstants::JD_JAN_5_1941);
   //            }

               #if DEBUG_COORD_PANEL
                  MessageInterface::ShowMessage
                     (wxT("CoordPanel::CreateAxis() about to set the value of epoch on axis to %12.10f\n"),
                           a1mjd);
               #endif
               axis->SetEpoch(a1mjd);
            }
            
         }
         catch (BaseException &e)
         {
            MessageInterface::ShowMessage
               (wxT("CoordPanel::CreateAxis() error occurred in setting data!\n%s\n"),
                e.GetFullMessage().c_str());
            
            delete axis;
            axis = NULL;
         }
      }
   }
   
   return axis;
}


//------------------------------------------------------------------------------
// void ChangeEpoch(wxString &oldFormat)
//------------------------------------------------------------------------------
void CoordPanel::ChangeEpoch(wxString &oldFormat)
{
/*   wxString newFormat = formatComboBox->GetStringSelection().Trim();

   #if DEBUG_COORD_PANEL
   MessageInterface::ShowMessage
      ("CoordPanel::ChangeEpoch() oldFormat=%s, newFormat=%s\n",
       oldFormat.c_str(), newFormat.c_str());
   #endif
   
   if (newFormat != oldFormat)
   {
      wxString newEpoch =
         mTimeConverter.Convert(epochTextCtrl->GetValue().c_str(),
                                oldFormat.c_str(), newFormat.c_str());
      epochTextCtrl->SetValue(newEpoch.c_str());
      oldFormat = newFormat;
   }*/
   
//      wxString toEpochFormat = formatComboBox->GetStringSelection().c_str();    
//      wxString epochStr = epochTextCtrl->GetValue().c_str();
//      theSpacecraft->SetDateFormat(toEpochFormat);
//      epochTextCtrl->SetValue(theSpacecraft->GetStringParameter("Epoch").c_str());
//      oldFormat = toEpochFormat;
}


//---------------------------------
// private methods
//---------------------------------

//------------------------------------------------------------------------------
// bool IsValidAxis(const wxString &axisType, const wxString &priName,
//                  const wxString &secName, const wxString &xStr,
//                  const wxString &yStr, const wxString &zStr)
//------------------------------------------------------------------------------
bool CoordPanel::IsValidAxis(const wxString &axisType, const wxString &priName,
                             const wxString &secName, const wxString &xStr,
                             const wxString &yStr, const wxString &zStr)
{
   if (axisType == wxT(""))
   {
      MessageInterface::PopupMessage(Gmat::INFO_, wxT("Please select Axis."));
      return false;
   }
   
   if (axisType == wxT("ObjectReferenced"))
   {
      if (priName == wxT("") && secName == wxT(""))
      {
         MessageInterface::PopupMessage
            (Gmat::WARNING_,
             wxT("ObjectReferenced must have a primary and secondary body."));
         return false;
      }
      else if (priName == secName)
      {
         MessageInterface::PopupMessage
            (Gmat::WARNING_,
             wxT("The primary and the secondary body must be different."));
         return false;
      }
      
      return IsValidXYZ(xStr, yStr, zStr);
   }
   
   return true;
}


//------------------------------------------------------------------------------
// bool IsValidXYZ(const wxString &xStr, const wxString &yStr,
//                 const wxString &zStr)
//------------------------------------------------------------------------------
bool CoordPanel::IsValidXYZ(const wxString &xStr, const wxString &yStr,
                            const wxString &zStr)
{
   // Check to see if x,y,z are valid axes
   if (xStr.IsSameAs(wxT("")) && (yStr.IsSameAs(wxT("")) || zStr.IsSameAs(wxT(""))))
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_,  wxT("Please select 2 coordinates from X, Y, and Z."));
      return false;
   }
   else if (xStr.Contains(wxT("R")) && (yStr.Contains(wxT("R")) || zStr.Contains(wxT("R"))))
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("The X, Y, and Z axis must be orthogonal."));
      return false;
   }
   else if (xStr.Contains(wxT("V")) && (yStr.Contains(wxT("V")) || zStr.Contains(wxT("V"))))
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("The X, Y, and Z axis must be orthogonal."));
      return false;
   }
   else if (xStr.Contains(wxT("N")) && (yStr.Contains(wxT("N")) || zStr.Contains(wxT("N"))))
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("The X, Y, and Z axis must be orthogonal."));
      return false;
   }
   
   if (yStr.Contains(wxT("R")) && zStr.Contains(wxT("R")))
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("The X, Y, and Z axis must be orthogonal."));
      return false;
   }
   else if (yStr.Contains(wxT("V")) && zStr.Contains(wxT("V")))
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("The X, Y, and Z axis must be orthogonal."));
      return false;
   }
   else if (yStr.Contains(wxT("N")) && zStr.Contains(wxT("N")))
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("The X, Y, and Z axis must be orthogonal."));
      return false;
   }
   else if (yStr.IsSameAs(wxT("")) && zStr.IsSameAs(wxT("")))
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("The X, Y, and Z axis must be orthogonal."));
      return false;
   }
   
   // Check to make sure at least one is blank
   if (xStr.IsSameAs(wxT("")) || yStr.IsSameAs(wxT("")) || zStr.IsSameAs(wxT("")))
      return true;
   else
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_,  wxT("One coordinate must be a blank string."));
      return false;
   }
}


//-------------------------------
// private methods
//-------------------------------

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
void CoordPanel::Create()
{
   Setup(this);
   LoadData();
}


//------------------------------------------------------------------------------
// void Setup( wxWindow *parent)
//------------------------------------------------------------------------------
void CoordPanel::Setup( wxWindow *parent)
{
   // get the config object
   wxConfigBase *pConfig = wxConfigBase::Get();
   // SetPath() understands ".."
   pConfig->SetPath(wxT("/Coordinate System"));

    // wxStaticText
   originStaticText = new wxStaticText( parent, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Origin"),
      wxDefaultPosition, wxDefaultSize, 0 );
   typeStaticText = new wxStaticText( parent, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Type"),
      wxDefaultPosition, wxDefaultSize, 0 );
   primaryStaticText = new wxStaticText( parent, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Primary"),
      wxDefaultPosition, wxDefaultSize, 0 );
//   formatStaticText = new wxStaticText( parent, ID_TEXT, wxT("Epoch "GUI_ACCEL_KEY"Format"),
//      wxDefaultPosition, wxDefaultSize, 0 );
   secondaryStaticText = new wxStaticText( parent, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Secondary"),
      wxDefaultPosition, wxDefaultSize, 0 );
   epochStaticText = new wxStaticText( parent, ID_TEXT, wxT("A1MJD ") wxT(GUI_ACCEL_KEY) wxT("Epoch"),
      wxDefaultPosition, wxDefaultSize, 0 );
//   updateStaticText = new wxStaticText( parent, ID_TEXT, wxT("Update "GUI_ACCEL_KEY"Interval"),
//      wxDefaultPosition, wxDefaultSize, 0 );
//   secStaticText = new wxStaticText( parent, ID_TEXT, wxT("seconds"),
//      wxDefaultPosition, wxDefaultSize, 0 );

   xStaticText = new wxStaticText( parent, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("X: "),
      wxDefaultPosition, wxDefaultSize, 0 );
   yStaticText = new wxStaticText( parent, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Y: "),
      wxDefaultPosition, wxDefaultSize, 0 );
   zStaticText = new wxStaticText( parent, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Z: "),
      wxDefaultPosition, wxDefaultSize, 0 );
          
   #if __WXMAC__
   wxStaticText *title1StaticText =
      new wxStaticText( this, ID_TEXT, wxT("Axes"),
                        wxDefaultPosition, wxSize(120,20),
                        wxBOLD);
   title1StaticText->SetFont(wxFont(14, wxSWISS, wxFONTFAMILY_TELETYPE, wxFONTWEIGHT_BOLD,
                                    true, wxT(""), wxFONTENCODING_SYSTEM));
   #endif

   //causing VC++ error => wxString emptyList[] = {};
   wxArrayString emptyList;

   // wxComboBox
   originComboBox = theGuiManager->GetSpacePointComboBox(this, ID_COMBO,
      wxSize(120,-1), false);
   originComboBox->SetToolTip(pConfig->Read(wxT("OriginHint")));
   typeComboBox = new wxComboBox
      ( parent, ID_COMBO, wxT(""), wxDefaultPosition, wxSize(150,-1), //0,
        emptyList, wxCB_DROPDOWN|wxCB_READONLY );
   typeComboBox->SetToolTip(pConfig->Read(wxT("TypeHint")));
   primaryComboBox = theGuiManager->GetSpacePointComboBox(this, ID_COMBO,
      wxSize(120,-1), false);
   primaryComboBox->SetToolTip(pConfig->Read(wxT("PrimaryHint")));
//   formatComboBox = new wxComboBox
//      ( parent, ID_COMBO, wxT(""), wxDefaultPosition, wxSize(120,-1), //0,
//        emptyList, wxCB_DROPDOWN|wxCB_READONLY );
//   formatComboBox->SetToolTip(pConfig->Read(wxT("EpochFormatHint")));
   secondaryComboBox = theGuiManager->GetSpacePointComboBox(this, ID_COMBO,
      wxSize(120,-1), false);
   secondaryComboBox->SetToolTip(pConfig->Read(wxT("SecondaryHint")));
   xComboBox = new wxComboBox
      ( parent, ID_COMBO, wxT(""), wxDefaultPosition, wxSize(60,-1), //0,
        emptyList, wxCB_DROPDOWN|wxCB_READONLY );
   xComboBox->SetToolTip(pConfig->Read(wxT("XHint")));
   yComboBox = new wxComboBox
      ( parent, ID_COMBO, wxT(""), wxDefaultPosition, wxSize(60,-1), //0,
        emptyList, wxCB_DROPDOWN|wxCB_READONLY );
   yComboBox->SetToolTip(pConfig->Read(wxT("YHint")));
   zComboBox = new wxComboBox
      ( parent, ID_COMBO, wxT(""), wxDefaultPosition, wxSize(60,-1), //0,
        emptyList, wxCB_DROPDOWN|wxCB_READONLY );
   zComboBox->SetToolTip(pConfig->Read(wxT("ZHint")));

   //wxTextCtrl
   epochTextCtrl = new wxTextCtrl( this, ID_TEXTCTRL, wxT(""),
      wxDefaultPosition, wxSize(120,-1), 0 );
   epochTextCtrl->SetToolTip(pConfig->Read(wxT("EpochHint")));
//   intervalTextCtrl = new wxTextCtrl( this, ID_TEXTCTRL, wxT(""),
//      wxDefaultPosition, wxSize(45,-1), 0 );
//   intervalTextCtrl->SetToolTip(pConfig->Read(wxT("UpdateIntervalHint")));

   // wx*Sizers
   wxBoxSizer *theMainSizer = new wxBoxSizer( wxVERTICAL );
   #if __WXMAC__
   wxBoxSizer *boxsizer4 = new wxBoxSizer( wxVERTICAL );
   #else
   wxStaticBox *staticbox1 = new wxStaticBox( parent, -1, wxT("Axes") );
   wxStaticBoxSizer *staticboxsizer1 = new wxStaticBoxSizer( staticbox1,
      wxVERTICAL );
   #endif
   wxFlexGridSizer *flexgridsizer1 = new wxFlexGridSizer( 3, 4, 0, 0 );
   wxBoxSizer *boxsizer1 = new wxBoxSizer( wxHORIZONTAL );
   wxBoxSizer *boxsizer2 = new wxBoxSizer( wxHORIZONTAL );
   wxBoxSizer *boxsizer3 = new wxBoxSizer( wxHORIZONTAL );

   boxsizer1->Add( originStaticText, 0, wxALIGN_CENTER|wxALL, 5 );
   boxsizer1->Add( originComboBox, 0, wxALIGN_CENTER|wxALL, 5 );

   // row 1
   flexgridsizer1->Add( typeStaticText, 0, wxALIGN_LEFT|wxALL, 5 );
   flexgridsizer1->Add( typeComboBox, 0, wxALIGN_LEFT|wxALL, 5 );
   flexgridsizer1->Add( 20, 20, 0, wxALIGN_LEFT|wxALL, 5 );
   flexgridsizer1->Add( 20, 20, 0, wxALIGN_LEFT|wxALL, 5 );

   // row 2
   flexgridsizer1->Add( primaryStaticText, 0, wxALIGN_LEFT|wxALL, 5 );
   flexgridsizer1->Add( primaryComboBox, 0, wxALIGN_LEFT|wxALL, 5 );
   flexgridsizer1->Add( secondaryStaticText, 0, wxALIGN_LEFT|wxALL, 5 );
   flexgridsizer1->Add( secondaryComboBox, 0, wxALIGN_LEFT|wxALL, 5 );

   // row 3
//   flexgridsizer1->Add( formatStaticText, 0, wxALIGN_LEFT|wxALL, 5 );
//   flexgridsizer1->Add( formatComboBox, 0, wxALIGN_LEFT|wxALL, 5 );
   flexgridsizer1->Add( epochStaticText, 0, wxALIGN_LEFT|wxALL, 5 );
   flexgridsizer1->Add( epochTextCtrl, 0, wxALIGN_LEFT|wxALL, 5 );

   boxsizer2->Add(xStaticText, 0, wxALIGN_CENTER|wxALL, 5 );
   boxsizer2->Add(xComboBox, 0, wxALIGN_CENTER|wxALL, 5 );
   boxsizer2->Add(yStaticText, 0, wxALIGN_CENTER|wxALL, 5 );
   boxsizer2->Add(yComboBox, 0, wxALIGN_CENTER|wxALL, 5 );
   boxsizer2->Add(zStaticText, 0, wxALIGN_CENTER|wxALL, 5 );
   boxsizer2->Add(zComboBox, 0, wxALIGN_CENTER|wxALL, 5 );

//   boxsizer3->Add(updateStaticText, 0, wxALIGN_CENTER|wxALL, 5 );
//   boxsizer3->Add(intervalTextCtrl, 0, wxALIGN_CENTER|wxALL, 5 );
//   boxsizer3->Add(secStaticText, 0, wxALIGN_CENTER|wxALL, 5 );

   #if __WXMAC__
   boxsizer4->Add( flexgridsizer1, 0, wxALIGN_CENTER|wxALL, 5 );
   boxsizer4->Add( boxsizer2, 0, wxALIGN_CENTER|wxALL, 5 );
   boxsizer4->Add( boxsizer3, 0, wxALIGN_CENTER|wxALL, 5 );
   
   theMainSizer->Add(boxsizer1, 0, wxALIGN_CENTRE|wxALL, 5);
   theMainSizer->Add(title1StaticText, 0, wxALIGN_LEFT|wxALL, 5);
   theMainSizer->Add(boxsizer4, 0, wxALIGN_CENTRE|wxALL, 5);
   #else
   staticboxsizer1->Add( flexgridsizer1, 0, wxALIGN_CENTER|wxALL, 5 );
   staticboxsizer1->Add( boxsizer2, 0, wxALIGN_CENTER|wxALL, 5 );
   staticboxsizer1->Add( boxsizer3, 0, wxALIGN_CENTER|wxALL, 5 );

   theMainSizer->Add(boxsizer1, 0, wxALIGN_CENTRE|wxALL, 5);
   theMainSizer->Add(staticboxsizer1, 0, wxALIGN_CENTRE|wxALL, 5);
   #endif
   
   if (!mEnableAll)
   {
      wxStaticText *msg =
         new wxStaticText(parent, ID_TEXT,
                          wxT("This is a default Coordinate ")
                          wxT("System and cannot be modified."),
                          wxDefaultPosition, wxDefaultSize, 0);
      msg->SetForegroundColour(*wxRED);
      theMainSizer->Add(msg, 0, wxALIGN_CENTRE|wxALL, 5);
   }
   
   this->SetAutoLayout( true );
   this->SetSizer( theMainSizer );
   theMainSizer->Fit( this );
   theMainSizer->SetSizeHints( this );
}


//------------------------------------------------------------------------------
// void LoadData()
//------------------------------------------------------------------------------
void CoordPanel::LoadData()
{
   try
   {
      // Load axes types
      StringArray itemNames =
         theGuiInterpreter->GetListOfFactoryItems(Gmat::AXIS_SYSTEM);
      for (unsigned int i = 0; i<itemNames.size(); i++)
         typeComboBox->Append(wxString(itemNames[i].c_str()));
      
//      // insert a blank option for secondary
//      secondaryComboBox->Append("");
      
      StringArray reps = TimeConverterUtil::GetValidTimeRepresentations();
      
/*      // Load epoch types - hard coded for now
      wxString epochStrs[] =
      {
         wxT("TAIModJulian"),
         wxT("UTCModJulian"),
         wxT("TAIGregorian"),
         wxT("UTCGregorian"),
      };
      
      for (unsigned int i = 0; i<4; i++)
         formatComboBox->Append(wxString(epochStrs[i].c_str()));*/
      
//      for (unsigned int i = 0; i < reps.size(); i++)
//         formatComboBox->Append(reps[i].c_str());
      
      wxString xyzStrs[] =
      {
         wxT(""),
         wxT("R"),
         wxT("-R"),
         wxT("V"),
         wxT("-V"),
         wxT("N"),
         wxT("-N"),
      };
      
      for (unsigned int i=0; i<7; i++)
      {
         xComboBox->Append(wxString(xyzStrs[i].c_str()));
         yComboBox->Append(wxString(xyzStrs[i].c_str()));
         zComboBox->Append(wxString(xyzStrs[i].c_str()));
      }
   }
   catch (BaseException &e)
   {
      MessageInterface::ShowMessage
         (wxT("CoordPanel:LoadData() error occurred!\n%s\n"),
            e.GetFullMessage().c_str());
   }
   
}


//------------------------------------------------------------------------------
// bool SaveData(const wxString &coordName, AxisSystem *axis,
//               const wxString &epochFormat)
//------------------------------------------------------------------------------
bool CoordPanel::SaveData(const wxString &coordName, AxisSystem *axis,
                          wxString &epochFormat)
{
   #if DEBUG_COORD_PANEL
   MessageInterface::ShowMessage
      (wxT("CoordPanel::SaveData() coordName=%s, epochFormat=%s, epoch = %s\n"),
       coordName.c_str(), epochFormat.c_str(), epochValue.c_str());
   #endif
   
   bool canClose = true;
   
   try
   {
      wxString inputString;
      wxString msg = wxT("The value of \"%s\" for field \"%s\" on object \"")
                         + coordName + 
                        wxT("\" is not an allowed value. \n")
                        wxT("The allowed values are: [%s].");                        

      // create CoordinateSystem if not exist
      CoordinateSystem *coordSys =
         (CoordinateSystem*)theGuiInterpreter->GetConfiguredObject(coordName);
      
      if (coordSys == NULL)
      {
         coordSys = (CoordinateSystem*)
            theGuiInterpreter->CreateObject(wxT("CoordinateSystem"), coordName);
         
         #if DEBUG_COORD_PANEL
         MessageInterface::ShowMessage
            (wxT("CoordPanel::SaveData() coordName=%s created.\n"),
             coordName.c_str());
         #endif
      }
      
      //-------------------------------------------------------
      // set origin and Axis
      //-------------------------------------------------------
      wxString originName = originComboBox->GetValue().Trim();
      coordSys->SetStringParameter(wxT("Origin"), wxString(originName.c_str()));
      coordSys->SetRefObject(axis, Gmat::AXIS_SYSTEM, wxT(""));
      
      SpacePoint *origin =
         (SpacePoint*)theGuiInterpreter->GetConfiguredObject(originName.c_str());
      
      coordSys->SetOrigin(origin);
      
      CelestialBody *j2000body =
         (CelestialBody*)theGuiInterpreter->GetConfiguredObject(wxT("Earth"));
      
      // set Earth as J000Body if NULL
      if (origin->GetJ2000Body() == NULL)
      {
         j2000body->SetJ2000Body(j2000body);
         origin->SetJ2000Body(j2000body);

      }

      coordSys->SetJ2000Body(j2000body);
      
      //-------------------------------------------------------
      // set primary and secondary 
      //-------------------------------------------------------
      // Set primary body if exist
      if (primaryComboBox->IsEnabled())
      {
         wxString primaryName = primaryComboBox->GetValue().Trim();
         SpacePoint *primary = (SpacePoint*)theGuiInterpreter->
            GetConfiguredObject(primaryName.c_str());
         
         axis->SetStringParameter(wxT("Primary"), primaryName.c_str());
         axis->SetPrimaryObject(primary);
      }
      
      // set secondary body if exist
      if (secondaryComboBox->IsEnabled())
      {
         wxString secondaryName = secondaryComboBox->GetValue().Trim();

         axis->SetStringParameter(wxT("Secondary"), secondaryName.c_str());
         
         if (secondaryName != wxT(""))
         {
            SpacePoint *secondary = (SpacePoint*)theGuiInterpreter->
               GetConfiguredObject(secondaryName.c_str());
         
            axis->SetSecondaryObject(secondary);
            if (secondary->GetJ2000Body() == NULL)
               secondary->SetJ2000Body(j2000body);
         }
      }
      
      //-------------------------------------------------------
      // set new direction
      //-------------------------------------------------------
      // set X value if exist
      if (xComboBox->IsEnabled())
         axis->SetXAxis(xComboBox->GetValue().Trim().c_str());
      
      // set Y value if exist
      if (yComboBox->IsEnabled())
         axis->SetYAxis(yComboBox->GetValue().Trim().c_str());
      
      // set z value if exist
      if (zComboBox->IsEnabled())
         axis->SetZAxis(zComboBox->GetValue().Trim().c_str());
      
      //-------------------------------------------------------
      // set new epoch format and epoch
      //-------------------------------------------------------
      if (epochTextCtrl->IsEnabled())
      {     
         Real epoch, a1mjd;
         wxString savedEpoch = epochValue.c_str();
         #ifdef DEBUG_COORD_PANEL_SAVE
            MessageInterface::ShowMessage(wxT("In CoordPanel::SaveData, saving current epoch value (%s)\n"),
                  savedEpoch.c_str());
         #endif
         
         inputString = epochTextCtrl->GetValue();
         if ((GmatStringUtil::ToReal(inputString, &epoch)) && (epoch >= 6116.0))
         {
            epochValue = epochTextCtrl->GetValue();
            a1mjd      = epoch;
//            if (epochFormat != newEpochFormat)
//            {
//               axis->SetEpochFormat(newEpochFormat.c_str());
//               epochFormat = newEpochFormat;
//
//               //convert epoch to A1ModJulian if not in this format
//               //if (newEpochFormat != "TAIModJulian")
//               if (newEpochFormat != "" && newEpochFormat != "A1ModJulian")
//               {
//                  wxString a1mjdStr;
//                  TimeConverterUtil::Convert(epochFormat.c_str(), epoch,
//                                             "", "A1ModJulian",
//                                             a1mjd, a1mjdStr);
//
//                  //wxString taiEpochStr = mTimeConverter.Convert
//                  //   (epochStr, newEpochFormat.c_str(), "TAIModJulian");
//                  //epoch = TimeConverterUtil::ConvertFromTaiMjd
//                  //   (TimeConverterUtil::A1MJD, atof(taiEpochStr.c_str()),
//                  //    GmatTimeConstants::JD_JAN_5_1941);
//               }
//            }
            #ifdef DEBUG_COORD_PANEL_SAVE
               MessageInterface::ShowMessage(wxT("In CoordPanel::SaveData, setting epoch on axis to %12.10f\n"),
                     a1mjd);
            #endif
            axis->SetEpoch(a1mjd);
         }
         else
         {
//            epochTextCtrl->SetValue(epochValue);
            MessageInterface::PopupMessage(Gmat::ERROR_, msg.c_str(), 
               inputString.c_str(),wxT("Epoch"),wxT("Real Number >= 6116.0"));
            canClose = false;
         }
      }
      
      //-------------------------------------------------------
      // set new update interval
      //-------------------------------------------------------
//      if (intervalTextCtrl->IsEnabled())
//      {
//         Real interval;
//         inputString = intervalTextCtrl->GetValue();
//         if ((GmatStringUtil::ToReal(inputString,&interval)) &&
//             (interval >= 0.0))
//         {
//            axis->SetRealParameter("UpdateInterval", interval);
//         }
//         else
//         {
//            MessageInterface::PopupMessage(Gmat::ERROR_, msg.c_str(),
//               inputString.c_str(),"Update Interval","Real Number >= 0.0");
//            canClose = false;
//         }
//      }
      
      // set solar system
      coordSys->SetSolarSystem(theGuiInterpreter->GetSolarSystemInUse());
      coordSys->Initialize();

   }
   catch (BaseException &e)
   {
      MessageInterface::ShowMessage
         (wxT("*** Error *** %s\n"), e.GetFullMessage().c_str());
      canClose = false;
   }
   
   return canClose;
}

