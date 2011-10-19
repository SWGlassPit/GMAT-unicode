//$Id: CelestialBodyOrbitPanel.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                                  CelestialBodyOrbitPanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
// Author: Wendy C. Shoan
// Created: 2009.01.26
//
/**
 * This is the panel for the Properties tab on the notebook on the CelestialBody
 * Panel.
 *
 */
//------------------------------------------------------------------------------

#include "CelestialBodyOrbitPanel.hpp"
#include "SolarSystem.hpp"
#include "GmatBaseException.hpp"
#include "GmatAppData.hpp"
#include "MessageInterface.hpp"
#include "StringUtil.hpp"
#include "A1Mjd.hpp"
#include "bitmaps/OpenFolder.xpm"
#include <wx/config.h>
#include <fstream>

//#define DEBUG_CB_ORBIT_PANEL
//#define DEBUG_CB_ORBIT_SAVE

// event tables for wxMac/Widgets
BEGIN_EVENT_TABLE(CelestialBodyOrbitPanel, wxPanel)
   EVT_COMBOBOX(ID_COMBO_BOX_EPHEM_SOURCE, CelestialBodyOrbitPanel::OnEphemSourceComboBoxChange)
   EVT_TEXT(ID_TEXT_CTRL_EPHEM_FILE, CelestialBodyOrbitPanel::OnEphemFileTextCtrlChange)
   EVT_BUTTON(ID_BROWSE_BUTTON_EPHEM_FILE, CelestialBodyOrbitPanel::OnEphemFileBrowseButton)
   EVT_BUTTON(ID_BROWSE_BUTTON_SPK_FILE, CelestialBodyOrbitPanel::OnSpkFileBrowseButton)
   EVT_BUTTON(ID_REMOVE_BUTTON_SPK_FILE, CelestialBodyOrbitPanel::OnSpkFileRemoveButton)
   EVT_LISTBOX(ID_LIST_BOX_SPK_FILE, CelestialBodyOrbitPanel::OnSpkFileListBoxChange)
   EVT_TEXT(ID_TEXT_CTRL_NAIF_ID, CelestialBodyOrbitPanel::OnNaifIdTextCtrlChange)
   EVT_COMBOBOX(ID_COMBO_BOX_CENTRAL_BODY, CelestialBodyOrbitPanel::OnCentralBodyComboBoxChange)
   EVT_TEXT(ID_TEXT_CTRL_INITIAL_EPOCH, CelestialBodyOrbitPanel::OnEpochTextCtrlChange)
   EVT_TEXT(ID_TEXT_CTRL_SMA, CelestialBodyOrbitPanel::OnSMATextCtrlChange)
   EVT_TEXT(ID_TEXT_CTRL_ECC, CelestialBodyOrbitPanel::OnECCTextCtrlChange)
   EVT_TEXT(ID_TEXT_CTRL_INC, CelestialBodyOrbitPanel::OnINCTextCtrlChange)
   EVT_TEXT(ID_TEXT_CTRL_RAAN, CelestialBodyOrbitPanel::OnRAANTextCtrlChange)
   EVT_TEXT(ID_TEXT_CTRL_AOP, CelestialBodyOrbitPanel::OnAOPTextCtrlChange)
   EVT_TEXT(ID_TEXT_CTRL_TA, CelestialBodyOrbitPanel::OnTATextCtrlChange)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

CelestialBodyOrbitPanel::CelestialBodyOrbitPanel(GmatPanel *cbPanel, 
                              wxWindow *parent, CelestialBody *body) :
   wxPanel          (parent),
   dataChanged      (false),
   canClose         (true),
   theBody          (body),
   naifID           (-99),
   initialEpoch     (0.0),
   SMA              (0.0),
   ECC              (0.0),
   INC              (0.0),
   RAAN             (0.0),
   AOP              (0.0),
   TA               (0.0),
   ephemSrcChanged  (false),
   ephemFileChanged (false),
   spkFileChanged   (false),
   naifIDChanged    (false),
   cBodyChanged     (false),
   epochChanged     (false),
   SMAChanged       (false),
   ECCChanged       (false),
   INCChanged       (false),
   RAANChanged      (false),
   AOPChanged       (false),
   TAChanged        (false),
   spkFilesDeleted  (false),
   userDef          (false),
   allowSpiceForDefaultBodies (false),
   isSun            (false),
   theCBPanel       (cbPanel)
{
   guiManager     = GuiItemManager::GetInstance();
   guiInterpreter = GmatAppData::Instance()->GetGuiInterpreter();
   ss             = guiInterpreter->GetSolarSystemInUse();
   
#ifdef __USE_SPICE__
   spiceAvailable = true;
#else
   spiceAvailable = false;
#endif
   Create();
}

CelestialBodyOrbitPanel::~CelestialBodyOrbitPanel()
{
   guiManager->UnregisterComboBox(wxT("CelestialBody"), centralBodyComboBox);
}

void CelestialBodyOrbitPanel::SaveData()
{
   wxString strval;
   Real        tmpval;
   Integer     tmpint;
   bool        retval;
   bool        stateChanged = SMAChanged || ECCChanged || INCChanged || RAANChanged ||
                              AOPChanged || TAChanged;
   
   // don't do anything if no data has been changed.
   // note that dataChanged will be true if the user modified any combo box or
   // text ctrl, whether or not he/she actually changed the value
   canClose    = true;
   
   try
   {
      if (ephemSrcChanged)
      {
         strval = ephemSourceComboBox->GetValue();
         theBody->SetStringParameter(theBody->GetParameterID(wxT("PosVelSource")), strval);
      }
      if (ephemFileChanged)
      {
         strval = ephemFileTextCtrl->GetValue();
         #ifdef DEBUG_CB_ORBIT_PANEL
            MessageInterface::ShowMessage(wxT("ephemFileChanged is true : %s\n"),
                  strval.c_str());
         #endif
         std::ifstream filename(strval.char_str());

         if (!filename)
         {
            wxString errmsg = wxT("File \"") + strval;
            errmsg += wxT("\" does not exist.\n");
            MessageInterface::PopupMessage(Gmat::ERROR_, errmsg);
         }
         else
         {
            filename.close();
            theBody->SetStringParameter(theBody->GetParameterID(wxT("SourceFilename")), strval);
         }
      }
      if ((userDef || allowSpiceForDefaultBodies) && spiceAvailable && spkFileChanged)
      {
         #ifdef DEBUG_CB_ORBIT_PANEL
            MessageInterface::ShowMessage(wxT("spkFileChanged is true : %s\n"),
                  strval.c_str());
         #endif

         unsigned int numKernels = spkFileListBox->GetCount();
         for (unsigned int ii = 0; ii < numKernels; ii++)
         {
            strval = spkFileListBox->GetString(ii);
            std::ifstream filename(strval.char_str());

            if (!filename)
            {
               wxString errmsg = wxT("File \"") + strval;
               errmsg += wxT("\" does not exist.\n");
               MessageInterface::PopupMessage(Gmat::ERROR_, errmsg);
               canClose = false;
            }
            else
            {
               filename.close();
               theBody->SetStringParameter(theBody->GetParameterID(wxT("OrbitSpiceKernelName")),
                     strval);
            }
         }
      }
      if ((userDef || allowSpiceForDefaultBodies) && spiceAvailable && naifIDChanged)
      {
         strval = naifIDTextCtrl->GetValue();
         retval = theCBPanel->CheckInteger(tmpint, strval, wxT("NAIF ID"), wxT("Integer Number"));
         canClose = retval;
         if (retval)
         {
            theBody->SetIntegerParameter(theBody->GetParameterID(wxT("NAIFId")), tmpint);
         }
      }
      if ((userDef || allowSpiceForDefaultBodies) && spiceAvailable && spkFilesDeleted)
      {
         for (unsigned int ii = 0; ii < spkFilesToDelete.size(); ii++)
         {
            theBody->RemoveSpiceKernelName(wxT("Orbit"), spkFilesToDelete.at(ii));
         }
      }
      if (cBodyChanged)
      {
         strval = centralBodyComboBox->GetValue();
         theBody->SetStringParameter(theBody->GetParameterID(wxT("CentralBody")), strval);
      }

      if (!isSun)
      {
         if (epochChanged)
         {
            strval = initialEpochTextCtrl->GetValue();
            retval = theCBPanel->CheckReal(tmpval, strval, wxT("Initial Two Body Epoch"), wxT("Real Number"));
            canClose = retval;
            if (retval)
            {
               A1Mjd a1Epoch(tmpval);
               theBody->SetTwoBodyEpoch(a1Epoch);
            }
         }
         if (stateChanged)
         {
            #ifdef DEBUG_CB_ORBIT_PANEL
               MessageInterface::ShowMessage(wxT("state has been changed ...\n"),
                     strval.c_str());
            #endif
            Rvector6 elements;
            bool     retval[6];
            strval = SMATextCtrl->GetValue();
            retval[0] = theCBPanel->CheckReal(tmpval, strval, wxT("Initial SMA"), wxT("Real Number"));
            if (retval[0])
            {
               elements[0] = tmpval;
            }
            strval = ECCTextCtrl->GetValue();
            retval[1] = theCBPanel->CheckReal(tmpval, strval, wxT("Initial SMA"), wxT("Real Number"));
            if (retval[1])
            {
               elements[1] = tmpval;
            }
            strval = INCTextCtrl->GetValue();
            retval[2] = theCBPanel->CheckReal(tmpval, strval, wxT("Initial INC"), wxT("Real Number"));
            if (retval[2])
            {
               elements[2] = tmpval;
            }
            strval = RAANTextCtrl->GetValue();
            retval[3] = theCBPanel->CheckReal(tmpval, strval, wxT("Initial RAAN"), wxT("Real Number"));
            if (retval[3])
            {
               elements[3] = tmpval;
            }
            strval = AOPTextCtrl->GetValue();
            retval[4] = theCBPanel->CheckReal(tmpval, strval, wxT("Initial AOP"), wxT("Real Number"));
            if (retval[4])
            {
               elements[4] = tmpval;
            }
            strval = TATextCtrl->GetValue();
            retval[5] = theCBPanel->CheckReal(tmpval, strval, wxT("Initial TA"), wxT("Real Number"));
            if (retval[5])
            {
               elements[5] = tmpval;
            }
            #ifdef DEBUG_CB_ORBIT_PANEL
               MessageInterface::ShowMessage(
                     wxT("elements = %12.10f  %12.10f  %12.10f  %12.10f  %12.10f  %12.10f   ...\n"),
                     elements[0],elements[1],elements[2],elements[3],elements[4],elements[5]);
            #endif
            canClose = retval[0] && retval[1] && retval[2] && retval[3] && retval[4] && retval[5];
            if (canClose) theBody->SetTwoBodyElements(elements);
       }
      }


   }
   catch (BaseException &ex)
   {
      canClose = false;
      dataChanged = true;
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage());
   }
   
   #ifdef DEBUG_CB_ORBIT_SAVE
      MessageInterface::ShowMessage(wxT("at end of CBOrbitPanel::SaveData, canClose = %s\n"),
            (canClose? wxT("true") : wxT("false")));
   #endif
   if (canClose)
   {
      dataChanged = false;
      ResetChangeFlags(true);
   }
}

void CelestialBodyOrbitPanel::LoadData()
{
   try
   {
      ephemSrc   = theBody->GetStringParameter(theBody->GetParameterID(wxT("PosVelSource")));
      ephemSourceComboBox->SetValue(ephemSrc.c_str());
      
      previousEphemSrc = ephemSrc;
      
      ephemFile  = theBody->GetSourceFileName();
      ephemFileTextCtrl->SetValue(ephemFile.c_str());

      
      centralBody = theBody->GetCentralBody();
      centralBodyComboBox->SetValue(centralBody.c_str());
      
      if ((userDef || allowSpiceForDefaultBodies) && spiceAvailable)
      {
         naifID = theBody->GetIntegerParameter(theBody->GetParameterID(wxT("NAIFId")));
         naifIDStringWX = guiManager->ToWxString(naifID);
         naifIDTextCtrl->SetValue(naifIDStringWX);
         spkFileArray             = theBody->GetStringArrayParameter(
                                    theBody->GetParameterID(wxT("OrbitSpiceKernelName")));
         unsigned int spkListSz   = spkFileArray.size();
         spkFileArrayWX           = new wxString[spkListSz];
         spkFiles.clear();
         for (unsigned int jj = 0; jj < spkListSz; jj++)
         {
            spkFiles.push_back(spkFileArray[jj]);
            spkFileArrayWX[jj] = spkFileArray[jj].c_str();
         }
         spkFileListBox->InsertItems(spkListSz, spkFileArrayWX, 0);
         spkFileListBox->SetSelection(spkListSz-1); // Select the last item
      }
      
      if (ephemSrc != wxT("DE405"))
      {
         ephemFileStaticText->Hide();
         ephemFileTextCtrl->Hide();
         ephemFileBrowseButton->Hide();
         mainBoxSizer->Layout();  
      }
      else
      {
         ephemFileStaticText->Show();
         ephemFileTextCtrl->Show();
         ephemFileBrowseButton->Show();
         orbitDataFlexGridSizer->Layout();
      }
      if (ephemSrc == wxT("TwoBodyPropagation"))
      {
         if (!isSun)
         {
            initialEpochTextCtrl->Enable();
            SMATextCtrl->Enable();
            ECCTextCtrl->Enable();
            INCTextCtrl->Enable();
            RAANTextCtrl->Enable();
            AOPTextCtrl->Enable();
            TATextCtrl->Enable();
            mainBoxSizer->Layout();
         }
      }
      else
      {
         if (!isSun)
         {
            initialEpochTextCtrl->Disable();
            SMATextCtrl->Disable();
            ECCTextCtrl->Disable();
            INCTextCtrl->Disable();
            RAANTextCtrl->Disable();
            AOPTextCtrl->Disable();
            TATextCtrl->Disable();
            mainBoxSizer->Layout();
         }
      }
      if ((userDef || allowSpiceForDefaultBodies) && spiceAvailable)
      {
         if (ephemSrc != wxT("SPICE"))
         {
            spkFileStaticText->Hide();
            spkFileListBox->Hide();
            spkFileBrowseButton->Hide();
            spkFileRemoveButton->Hide();
            naifIDStaticText->Hide();
            naifIDTextCtrl->Hide();
            naifIDBlankText->Hide();
            mainBoxSizer->Layout();
         }
         else // SPICE
         {
            spkFileStaticText->Show();
            spkFileListBox->Show();
            spkFileBrowseButton->Show();
            spkFileRemoveButton->Show();
            naifIDStaticText->Show();
            naifIDTextCtrl->Show();
            naifIDBlankText->Show();
            mainBoxSizer->Layout();
         }
      }
      if (!isSun)
      {
         initialEpoch = (theBody->GetTwoBodyEpoch()).Get();
         initialEpochStringWX = guiManager->ToWxString(initialEpoch);
         initialEpochTextCtrl->SetValue(initialEpochStringWX);
         
         Rvector6 elements = theBody->GetTwoBodyElements();
         SMA  = elements[0];
         ECC  = elements[1];
         INC  = elements[2];
         RAAN = elements[3];
         AOP  = elements[4];
         TA   = elements[5];
         SMAStringWX = guiManager->ToWxString(elements[0]);
         SMATextCtrl->SetValue(SMAStringWX);
         ECCStringWX = guiManager->ToWxString(elements[1]);
         ECCTextCtrl->SetValue(ECCStringWX);
         INCStringWX = guiManager->ToWxString(elements[2]);
         INCTextCtrl->SetValue(INCStringWX);
         RAANStringWX = guiManager->ToWxString(elements[3]);
         RAANTextCtrl->SetValue(RAANStringWX);
         AOPStringWX = guiManager->ToWxString(elements[4]);
         AOPTextCtrl->SetValue(AOPStringWX);
         TAStringWX = guiManager->ToWxString(elements[5]);
         TATextCtrl->SetValue(TAStringWX);
      }

      ResetChangeFlags();
     
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
}


//------------------------------------------------------------------------------
// private methods
//------------------------------------------------------------------------------
void CelestialBodyOrbitPanel::Create()
{
   int bSize     = 2;
   #if __WXMAC__
   int buttonWidth = 40;
   #else
   int buttonWidth = 25;
   #endif

   wxBitmap openBitmap = wxBitmap(OpenFolder_xpm);

   // get the config object
   wxConfigBase *pConfig = wxConfigBase::Get();
   // SetPath() understands ".."
   pConfig->SetPath(wxT("/Celestial Body Orbit"));

   if (theBody->GetName() == SolarSystem::SUN_NAME)  isSun = true;
   else                                              isSun = false;
   userDef = theBody->IsUserDefined();
   allowSpiceForDefaultBodies = ss->IsSpiceAllowedForDefaultBodies();
   
   // empty the temporary value strings
   ephemSourceStringWX    = wxT("");
   ephemFileStringWX      = wxT("");
   naifIDStringWX         = wxT("");
   centralBodyStringWX    = wxT("");
   initialEpochStringWX   = wxT("");
   SMAStringWX            = wxT("");
   ECCStringWX            = wxT("");
   INCStringWX            = wxT("");
   RAANStringWX           = wxT("");
   AOPStringWX            = wxT("");
   TAStringWX             = wxT("");
   
   // ephem source combo box
   sourceArray              = theBody->GetEphemSourceList();
   unsigned int ephemListSz = sourceArray.size();
   sourceArrayWX            = new wxString[ephemListSz];
   for (unsigned int jj = 0; jj < ephemListSz; jj++)
      sourceArrayWX[jj] = sourceArray[jj].c_str();
   ephemSourceStaticText = new wxStaticText(this, ID_TEXT, wxT("Ephemeris ") wxT(GUI_ACCEL_KEY) wxT("Source"),
                           wxDefaultPosition, wxSize(-1,-1), 0);
   ephemSourceComboBox   = new wxComboBox(this, ID_COMBO_BOX_EPHEM_SOURCE, sourceArrayWX[0],
                           wxDefaultPosition, wxDefaultSize, ephemListSz, sourceArrayWX,
                           wxCB_DROPDOWN|wxCB_READONLY);
   ephemSourceComboBox->SetToolTip(pConfig->Read(wxT("EphemerisSourceHint")));
   // ephem file
   ephemFileStaticText    =  new wxStaticText(this, ID_TEXT, wxT("Ephemeris ") wxT(GUI_ACCEL_KEY) wxT("File"),
                             wxDefaultPosition, wxSize(-1,-1), 0);
   ephemFileTextCtrl      = new wxTextCtrl(this, ID_TEXT_CTRL_EPHEM_FILE, wxT(""),
                            wxDefaultPosition, wxSize(150,-1), 0);
   ephemFileTextCtrl->SetToolTip(pConfig->Read(wxT("EphemerisFileHint")));
   ephemFileBrowseButton  = new wxBitmapButton(this, ID_BROWSE_BUTTON_EPHEM_FILE, 
                            openBitmap, wxDefaultPosition,
                            wxSize(buttonWidth, 20));
   ephemFileBrowseButton->SetToolTip(pConfig->Read(wxT("BrowseEphemerisFileHint"), wxT("Browse for file")));
   
   wxBoxSizer *spkButtonSizer = NULL;
   
   if ((userDef || allowSpiceForDefaultBodies) && spiceAvailable)
   {
      // naif ID for user-defined bodies
      naifIDStaticText   = new wxStaticText(this, ID_TEXT,wxT(GUI_ACCEL_KEY) wxT("NAIF ID"),
                           wxDefaultPosition, wxSize(-1,-1), 0);
      naifIDTextCtrl     = new wxTextCtrl(this, ID_TEXT_CTRL_NAIF_ID, wxT(""),
                           wxDefaultPosition, wxSize(80, -1), 0);
      naifIDTextCtrl->SetToolTip(pConfig->Read(wxT("NAIFIDHint")));
      naifIDBlankText    = new wxStaticText(this, ID_TEXT,wxT(""),
                           wxDefaultPosition, wxSize(-1,-1), 0);
      // SPK file(s)
      wxArrayString emptyList;
      spkFileStaticText   = new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("SPK Files"),
                            wxDefaultPosition, wxSize(-1,-1), 0);
      spkFileListBox      = new wxListBox(this, ID_LIST_BOX_SPK_FILE, wxDefaultPosition, wxSize(80, 100),
                            emptyList, wxLB_EXTENDED|wxLB_NEEDED_SB|wxLB_HSCROLL);
      spkFileListBox->SetToolTip(pConfig->Read(wxT("SPKFileListHint")));
      spkFileBrowseButton = new wxButton(this, ID_BROWSE_BUTTON_SPK_FILE, wxT(GUI_ACCEL_KEY) wxT("Add"),
                            wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
      spkFileBrowseButton->SetToolTip(pConfig->Read(wxT("AddSPKFileHint")));
      spkFileRemoveButton = new wxButton(this, ID_REMOVE_BUTTON_SPK_FILE, wxT(GUI_ACCEL_KEY) wxT("Remove"),
                            wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
      spkFileRemoveButton->SetToolTip(pConfig->Read(wxT("RemoveSPKFileHint")));
      spkButtonSizer = new wxBoxSizer(wxHORIZONTAL);
      spkButtonSizer->Add(spkFileBrowseButton,0, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
      spkButtonSizer->Add(spkFileRemoveButton,0, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
   }
   
   // central body
//   centralBodyArray      = ss->GetBodiesInUse();
//   unsigned int bodiesSz = centralBodyArray.size();
//   centralBodyArrayWX = new wxString[bodiesSz];
//   for (unsigned int ii = 0; ii < bodiesSz; ii++)
//      centralBodyArrayWX[ii] = wxT(centralBodyArray[ii].c_str());
   centralBodyStaticText = new wxStaticText(this, ID_TEXT, wxT("Central ") wxT(GUI_ACCEL_KEY) wxT("Body"),
                           wxDefaultPosition, wxSize(-1,-1), 0);
//   centralBodyComboBox   = new wxComboBox(this, ID_COMBO_BOX_CENTRAL_BODY, wxT(centralBodyArrayWX[0]),
//                           wxDefaultPosition, wxDefaultSize, bodiesSz, centralBodyArrayWX,
//                           wxCB_DROPDOWN|wxCB_READONLY);
   centralBodyComboBox = guiManager->GetCelestialBodyComboBox(this, ID_COMBO_BOX_CENTRAL_BODY, 
                                                              wxSize(150,-1));
   centralBodyComboBox->SetToolTip(pConfig->Read(wxT("CentralBodyHint")));
   
   if (!isSun)
   {
      // initial epoch
      initialEpochStaticText = new wxStaticText(this, ID_TEXT, wxT("Initial A1 ") wxT(GUI_ACCEL_KEY) wxT("Epoch"),
                               wxDefaultPosition, wxSize(-1,-1), 0);
      initialEpochTextCtrl   = new wxTextCtrl(this, ID_TEXT_CTRL_INITIAL_EPOCH, wxT(""),
                               wxDefaultPosition, wxSize(140, -1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
      initialEpochTextCtrl->SetToolTip(pConfig->Read(wxT("InitialA1EpochHint")));
      
      // SMA
      SMAStaticText      = new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("SMA"),
                           wxDefaultPosition, wxSize(-1,-1), 0);
      SMATextCtrl        = new wxTextCtrl(this, ID_TEXT_CTRL_SMA, wxT(""),
                           wxDefaultPosition, wxSize(140, -1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
      SMATextCtrl->SetToolTip(pConfig->Read(wxT("SMAHint")));
      SMAUnitsStaticText = new wxStaticText(this, ID_TEXT, wxT("km"),
                           wxDefaultPosition, wxSize(-1,-1), 0);
      
      // ECC
      ECCStaticText      = new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("ECC"),
                           wxDefaultPosition, wxSize(-1,-1), 0);
      ECCTextCtrl        = new wxTextCtrl(this, ID_TEXT_CTRL_ECC, wxT(""),
                           wxDefaultPosition, wxSize(140, -1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
      ECCTextCtrl->SetToolTip(pConfig->Read(wxT("ECCHint")));
      ECCUnitsStaticText = new wxStaticText(this, ID_TEXT, wxT(""),
                           wxDefaultPosition, wxSize(-1,-1), 0);
      
      // INC
      INCStaticText      = new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("INC"),
                           wxDefaultPosition, wxSize(-1,-1), 0);
      INCTextCtrl        = new wxTextCtrl(this, ID_TEXT_CTRL_INC, wxT(""),
                           wxDefaultPosition, wxSize(140, -1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
      INCTextCtrl->SetToolTip(pConfig->Read(wxT("INCHint")));
      INCUnitsStaticText = new wxStaticText(this, ID_TEXT, wxT("deg"),
                           wxDefaultPosition, wxSize(-1,-1), 0);
      
      // RAAN
      RAANStaticText      = new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("RAAN"),
                            wxDefaultPosition, wxSize(-1,-1), 0);
      RAANTextCtrl        = new wxTextCtrl(this, ID_TEXT_CTRL_RAAN, wxT(""),
                            wxDefaultPosition, wxSize(140, -1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
      RAANTextCtrl->SetToolTip(pConfig->Read(wxT("RAANHint")));
      RAANUnitsStaticText = new wxStaticText(this, ID_TEXT, wxT("deg"),
                            wxDefaultPosition, wxSize(-1,-1), 0);
      
      // AOP
      AOPStaticText      = new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("AOP"),
                           wxDefaultPosition, wxSize(-1,-1), 0);
      AOPTextCtrl        = new wxTextCtrl(this, ID_TEXT_CTRL_AOP, wxT(""),
                           wxDefaultPosition, wxSize(140, -1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
      AOPTextCtrl->SetToolTip(pConfig->Read(wxT("AOPHint")));
      AOPUnitsStaticText = new wxStaticText(this, ID_TEXT, wxT("deg"),
                           wxDefaultPosition, wxSize(-1,-1), 0);
      
      // TA
      TAStaticText      = new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("TA"),
                           wxDefaultPosition, wxSize(-1,-1), 0);
      TATextCtrl        = new wxTextCtrl(this, ID_TEXT_CTRL_TA, wxT(""),
                           wxDefaultPosition, wxSize(140, -1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
      TATextCtrl->SetToolTip(pConfig->Read(wxT("TAHint")));
      TAUnitsStaticText = new wxStaticText(this, ID_TEXT, wxT("deg"),
                           wxDefaultPosition, wxSize(-1,-1), 0);
   
   }
   
   // sizers
   
   orbitDataFlexGridSizer = new wxFlexGridSizer(3,0,0);
   orbitDataFlexGridSizer->Add(centralBodyStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
   orbitDataFlexGridSizer->Add(centralBodyComboBox,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
   //orbitDataFlexGridSizer->Add(30,20,0,wxGROW|wxALIGN_LEFT|wxALL, bSize);
   orbitDataFlexGridSizer->Add(0, 0);
   
   orbitDataFlexGridSizer->Add(ephemSourceStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
   orbitDataFlexGridSizer->Add(ephemSourceComboBox,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
   //orbitDataFlexGridSizer->Add(30,20,0,wxGROW|wxALIGN_LEFT|wxALL, bSize);
   orbitDataFlexGridSizer->Add(0, 0);
   
   orbitDataFlexGridSizer->Add(ephemFileStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
   orbitDataFlexGridSizer->Add(ephemFileTextCtrl,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
   orbitDataFlexGridSizer->Add(ephemFileBrowseButton,0, wxALIGN_CENTRE|wxALL, bSize);
   
   if ((userDef || allowSpiceForDefaultBodies) && spiceAvailable)
   {
      orbitDataFlexGridSizer->Add(naifIDStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      orbitDataFlexGridSizer->Add(naifIDTextCtrl,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      orbitDataFlexGridSizer->Add(0, 0);
      
      orbitDataFlexGridSizer->Add(spkFileStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      orbitDataFlexGridSizer->Add(spkFileListBox,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      orbitDataFlexGridSizer->Add(0, 0);
      
      orbitDataFlexGridSizer->Add(0, 0);
      orbitDataFlexGridSizer->Add(spkButtonSizer, 0, wxALIGN_CENTRE|wxALL, bSize);
      orbitDataFlexGridSizer->Add(0, 0);
   }
   
   wxFlexGridSizer *initialStateFlexGridSizer = NULL;
   if (!isSun)
   {
      initialStateFlexGridSizer = new wxFlexGridSizer(3,0,0);
      initialStateFlexGridSizer->Add(initialEpochStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(initialEpochTextCtrl,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(30,20,0,wxGROW|wxALIGN_LEFT|wxALL, bSize);
      
      initialStateFlexGridSizer->Add(SMAStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(SMATextCtrl,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(SMAUnitsStaticText,0, wxALIGN_LEFT|wxALL, bSize);
      
      initialStateFlexGridSizer->Add(ECCStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(ECCTextCtrl,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(ECCUnitsStaticText,0, wxALIGN_LEFT|wxALL, bSize);
      
      initialStateFlexGridSizer->Add(INCStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(INCTextCtrl,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(INCUnitsStaticText,0, wxALIGN_LEFT|wxALL, bSize);
      
      initialStateFlexGridSizer->Add(RAANStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(RAANTextCtrl,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(RAANUnitsStaticText,0, wxALIGN_LEFT|wxALL, bSize);
      
      initialStateFlexGridSizer->Add(AOPStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(AOPTextCtrl,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(AOPUnitsStaticText,0, wxALIGN_LEFT|wxALL, bSize);
      
      initialStateFlexGridSizer->Add(TAStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(TATextCtrl,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
      initialStateFlexGridSizer->Add(TAUnitsStaticText,0, wxALIGN_LEFT|wxALL, bSize);
      
      // Make state edit box growable
      initialStateFlexGridSizer->AddGrowableCol( 1 );
   }
   
   mainBoxSizer = new wxBoxSizer(wxHORIZONTAL);
   
   GmatStaticBoxSizer  *boxSizer1 = new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Ephemeris Data"));
   boxSizer1->Add(orbitDataFlexGridSizer, 0, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
   
   if (isSun)
   {
      mainBoxSizer->Add(boxSizer1, 1, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
      mainBoxSizer->Add(0,0);
   }
   else
   {
      GmatStaticBoxSizer  *boxSizer2 = new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Initial Two Body State"));
      boxSizer2->Add(initialStateFlexGridSizer, 0, wxGROW|wxALIGN_CENTER|wxALL, bSize);
      mainBoxSizer->Add(boxSizer1, 0, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
      mainBoxSizer->Add(boxSizer2, 1, wxGROW|wxALIGN_CENTRE|wxALL, bSize); 
   }
   
   // disable ephem source, ephem file, and central body for default bodies, since
   // those need to be set on the SolarSystem panel
   if (!userDef)
   {
      ephemSourceComboBox->Disable();
      ephemFileTextCtrl->Disable();
      ephemFileBrowseButton->Disable();
   }
   centralBodyComboBox->Disable(); // do not allow user to change central body (maybe make this
                                   // static text somewhere ...)
   
   // Added #if in case if we want to add another layer of lined box for consistency.
   // I like without another layer, too many lines, so set to 0 (LOJ: 2009.11.18)
   #if 1
   
   GmatStaticBoxSizer *mainStaticBoxSizer = new GmatStaticBoxSizer(wxHORIZONTAL, this, wxT(""));
   mainStaticBoxSizer->Add(mainBoxSizer, 1, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
   
   this->SetAutoLayout(true);
   this->SetSizer(mainStaticBoxSizer);
   mainStaticBoxSizer->Fit(this);
   mainStaticBoxSizer->SetSizeHints(this);
   
   #else
   
   this->SetAutoLayout(true);
   this->SetSizer(mainBoxSizer);
   mainBoxSizer->Fit(this);
   mainBoxSizer->SetSizeHints(this);
   
   #endif
}

void CelestialBodyOrbitPanel::ResetChangeFlags(bool discardMods)
{
   ephemSrcChanged  = false;
   ephemFileChanged = false;
   spkFileChanged   = false;
   naifIDChanged    = false;
   cBodyChanged     = false;
   epochChanged     = false;
   SMAChanged       = false;
   ECCChanged       = false;
   INCChanged       = false;
   RAANChanged      = false;
   AOPChanged       = false;
   TAChanged        = false;
   spkFilesDeleted  = false;
   
   if (discardMods)
   {
//      ephemSourceComboBox->DiscardEdits();
      ephemFileTextCtrl->DiscardEdits();
      if ((userDef || allowSpiceForDefaultBodies) && spiceAvailable)
      {
//         spkFileListBox->DiscardEdits();  // why doesn't this work??
         naifIDTextCtrl->DiscardEdits();
      }
//      centralBodyComboBox->DiscardEdits();
      if (!isSun)
      {
         initialEpochTextCtrl->DiscardEdits();
         SMATextCtrl->DiscardEdits();
         ECCTextCtrl->DiscardEdits();
         INCTextCtrl->DiscardEdits();
         RAANTextCtrl->DiscardEdits();
         AOPTextCtrl->DiscardEdits();
         TATextCtrl->DiscardEdits();
      }
   }
}

//Event Handling
void CelestialBodyOrbitPanel::OnEphemSourceComboBoxChange(wxCommandEvent &event)
{
   wxString newEphemSrc = (ephemSourceComboBox->GetStringSelection()).c_str();
   if (newEphemSrc == previousEphemSrc) return;
   ephemSrcChanged = true;
   dataChanged     = true;
//   ephemSrc        = newEphemSrc;
   theCBPanel->EnableUpdate(true);
   if (newEphemSrc != wxT("DE405"))
   {
      ephemFileStaticText->Hide();
      ephemFileTextCtrl->Hide();
      ephemFileBrowseButton->Hide();
      orbitDataFlexGridSizer->Layout();
      mainBoxSizer->Layout();
      
   }
   else
   {
      // re-insert those items here
      ephemFileStaticText->Show();
      ephemFileTextCtrl->Show();
      ephemFileBrowseButton->Show();
      orbitDataFlexGridSizer->Layout();
      mainBoxSizer->Layout();
   }
   if (newEphemSrc == wxT("TwoBodyPropagation"))
   {
      if (!isSun)
      {
         initialEpochTextCtrl->Enable();
         SMATextCtrl->Enable();
         ECCTextCtrl->Enable();
         INCTextCtrl->Enable();
         RAANTextCtrl->Enable();
         AOPTextCtrl->Enable();
         TATextCtrl->Enable();
         mainBoxSizer->Layout();
      }
   }
   else
   {
      if (!isSun)
      {
         initialEpochTextCtrl->Disable();
         SMATextCtrl->Disable();
         ECCTextCtrl->Disable();
         INCTextCtrl->Disable();
         RAANTextCtrl->Disable();
         AOPTextCtrl->Disable();
         TATextCtrl->Disable();
         mainBoxSizer->Layout();
      }
   }

   if ((userDef || allowSpiceForDefaultBodies) && spiceAvailable)
   {
      if (newEphemSrc != wxT("SPICE"))
      {
         spkFileStaticText->Hide();
         spkFileListBox->Hide();
         spkFileBrowseButton->Hide();
         spkFileRemoveButton->Hide();
         naifIDStaticText->Hide();
         naifIDTextCtrl->Hide();
         naifIDBlankText->Hide();
         naifIDTextCtrl->Disable();
         mainBoxSizer->Layout();
      }
      else // SPICE
      {
         spkFileStaticText->Show();
         spkFileListBox->Show();
         spkFileBrowseButton->Show();
         spkFileRemoveButton->Show();
         naifIDStaticText->Show();
         naifIDTextCtrl->Show();
         naifIDBlankText->Show();
         naifIDTextCtrl->Enable();
         mainBoxSizer->Layout();
      }
   }
   previousEphemSrc = newEphemSrc;
}

void CelestialBodyOrbitPanel::OnEphemFileTextCtrlChange(wxCommandEvent &event)
{
   if (ephemFileTextCtrl->IsModified())
   {
      ephemFileChanged  = true;
      dataChanged       = true;
      theCBPanel->EnableUpdate(true);
   }
}

void CelestialBodyOrbitPanel::OnEphemFileBrowseButton(wxCommandEvent &event)
{
   wxString oldFile = ephemFileTextCtrl->GetValue();
   wxFileDialog dialog(this, wxT("Choose a file"), wxT(""), wxT(""), wxT("*.*"));
   if (dialog.ShowModal() == wxID_OK)
   {
      wxString fileName = (dialog.GetPath()).c_str();
      if (!fileName.IsSameAs(oldFile))
      {
         ephemFileTextCtrl->SetValue(fileName);
         ephemFileChanged = true;
         dataChanged      = true;
         theCBPanel->EnableUpdate(true);
      }
   }
}

void CelestialBodyOrbitPanel::OnSpkFileBrowseButton(wxCommandEvent &event)
{
   wxArrayString oldFiles = spkFileListBox->GetStrings();
   wxFileDialog dialog(this, wxT("Choose a file to add"), wxT(""), wxT(""), wxT("*.*"));
   Integer foundAt = -99;
   if (dialog.ShowModal() == wxID_OK)
   {
      wxString fileName = (dialog.GetPath()).c_str();
      for (Integer ii = 0; ii < (Integer) oldFiles.GetCount(); ii++)
      {
         if (fileName.IsSameAs(oldFiles[ii]))
         {
            foundAt = ii;
            break;
         }
      }
      if (foundAt == -99) // not found, so it's new 
      {
         // Deselect current selections first
         wxArrayInt selections;
         int numSelect = spkFileListBox->GetSelections(selections);
         for (int i = 0; i < numSelect; i++)
            spkFileListBox->Deselect(selections[i]);
         
         spkFileChanged   = true;
         dataChanged      = true;
         spkFileListBox->Append(fileName);
         spkFileListBox->SetStringSelection(fileName);
         theCBPanel->EnableUpdate(true);
      }
   }
}

//------------------------------------------------------------------------------
// void OnSpkFileRemoveButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void CelestialBodyOrbitPanel::OnSpkFileRemoveButton(wxCommandEvent &event)
{
   wxArrayInt selections;
   int numSelect = spkFileListBox->GetSelections(selections);
   // get the string values and delete the selected names from the list
   wxString    fileSelected;
   for (int i = numSelect-1; i >= 0; i--)
   {
      fileSelected = spkFileListBox->GetString(selections[i]);
      spkFilesToDelete.push_back(fileSelected.c_str());
      spkFileListBox->Delete(selections[i]);
   }
   spkFilesDeleted = true;
   dataChanged     = true;
   theCBPanel->EnableUpdate(true);
   
   // Select the last item
   unsigned int count = spkFileListBox->GetCount();
   if (count > 0)
      spkFileListBox->SetSelection(count -1);
}

void CelestialBodyOrbitPanel::OnSpkFileListBoxChange(wxCommandEvent &event)
{
   spkFileChanged = true;
   dataChanged    = true;
   theCBPanel->EnableUpdate(true);
}

void CelestialBodyOrbitPanel::OnNaifIdTextCtrlChange(wxCommandEvent &event)
{
   if (naifIDTextCtrl->IsModified())
   {
      naifIDChanged   = true;
      dataChanged     = true;
      theCBPanel->EnableUpdate(true);
   }
}

void CelestialBodyOrbitPanel::OnCentralBodyComboBoxChange(wxCommandEvent &event)
{
   wxString newCentralBody = (centralBodyComboBox->GetStringSelection()).c_str();
   if (newCentralBody == centralBody) return;
   cBodyChanged       = true;
   dataChanged        = true;
   centralBody        = newCentralBody;
   theCBPanel->EnableUpdate(true);
}

void CelestialBodyOrbitPanel::OnEpochTextCtrlChange(wxCommandEvent &event)
{
   if (initialEpochTextCtrl->IsModified())
   {
      epochChanged    = true;
      dataChanged     = true;
      theCBPanel->EnableUpdate(true);
   }
}

void CelestialBodyOrbitPanel::OnSMATextCtrlChange(wxCommandEvent &event)
{
   if (SMATextCtrl->IsModified())
   {
      SMAChanged      = true;
      dataChanged     = true;
      theCBPanel->EnableUpdate(true);
   }
}

void CelestialBodyOrbitPanel::OnECCTextCtrlChange(wxCommandEvent &event)
{
   if (ECCTextCtrl->IsModified())
   {
      ECCChanged      = true;
      dataChanged     = true;
      theCBPanel->EnableUpdate(true);
   }
}

void CelestialBodyOrbitPanel::OnINCTextCtrlChange(wxCommandEvent &event)
{
   if (INCTextCtrl->IsModified())
   {
      INCChanged      = true;
      dataChanged     = true;
      theCBPanel->EnableUpdate(true);
   }
}

void CelestialBodyOrbitPanel::OnRAANTextCtrlChange(wxCommandEvent &event)
{
   if (RAANTextCtrl->IsModified())
   {
      RAANChanged     = true;
      dataChanged     = true;
      theCBPanel->EnableUpdate(true);
   }
}

void CelestialBodyOrbitPanel::OnAOPTextCtrlChange(wxCommandEvent &event)
{
   if (AOPTextCtrl->IsModified())
   {
      AOPChanged      = true;
      dataChanged     = true;
      theCBPanel->EnableUpdate(true);
   }
}

void CelestialBodyOrbitPanel::OnTATextCtrlChange(wxCommandEvent &event)
{
   if (TATextCtrl->IsModified())
   {
      TAChanged       = true;
      dataChanged     = true;
      theCBPanel->EnableUpdate(true);
   }
}
