//$Id: ThrusterCoefficientDialog.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                              ThrusterCoefficientDialog
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Waka Waktola
// Created: 2005/01/13
//
/**
 * Implements ThrusterCoefficientDialog class. This class shows dialog window where
 * thruster coefficients can be modified.
 * 
 */
//------------------------------------------------------------------------------

#include "ThrusterCoefficientDialog.hpp"
#include "StringUtil.hpp"
#include "MessageInterface.hpp"
#include <wx/variant.h>

//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ThrusterCoefficientDialog, GmatDialog)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// ThrusterCoefficientDialog(wxWindow *parent, wxWindowID id, 
//                           const wxString &title, GmatBase *obj,
//                           const wxString &type)
//------------------------------------------------------------------------------
ThrusterCoefficientDialog::
ThrusterCoefficientDialog(wxWindow *parent, wxWindowID id, 
                          const wxString &title, GmatBase *obj,
                          const wxString &type)
   : GmatDialog(parent, id, title, obj, wxDefaultPosition, wxDefaultSize)
{
   coefType = type;
   theObject = obj;
   
   coefNames.clear();
   coefValues.clear();
   
   if (obj != NULL)
   {
      Create();
      ShowData();
   }
}

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
void ThrusterCoefficientDialog::Create()
{
   coefGrid =
      new wxGrid( this, ID_GRID, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
   
   coefCount = Thruster::COEFFICIENT_COUNT;
   
   coefGrid->EnableDragGridSize(false);
   coefGrid->EnableDragColSize(false);
   coefGrid->CreateGrid(coefCount, 3);
   coefGrid->SetRowLabelSize(0);
   coefGrid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_CENTRE);
   
   coefGrid->SetColLabelValue(0, wxT("Coefficient"));
   coefGrid->SetColSize(0, 70);
   coefGrid->SetColLabelValue(1, wxT("Value"));
   coefGrid->SetColSize(1, 135);
   coefGrid->SetColLabelValue(2, wxT("Unit"));
   coefGrid->SetColSize(2, 80);
   
   // The first and third columns are read only
   for (int i=0; i<coefCount; i++)
   {
      coefGrid->SetReadOnly(i, 0);
      coefGrid->SetReadOnly(i, 2);
   }
   
   // wxSizers   
   theMiddleSizer->Add(coefGrid, 0, wxALIGN_CENTRE|wxGROW|wxALL, 3);
}

//------------------------------------------------------------------------------
// virtual void LoadData()
//------------------------------------------------------------------------------
void ThrusterCoefficientDialog::LoadData()
{
   Integer paramID = 0;
   int coefCount = Thruster::COEFFICIENT_COUNT;
   
   if (coefType == wxT("C"))
   {
      paramID = theObject->GetParameterID(wxT("C1"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C1"));
      
      paramID = theObject->GetParameterID(wxT("C2"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C2"));
      
      paramID = theObject->GetParameterID(wxT("C3"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C3"));
      
      paramID = theObject->GetParameterID(wxT("C4"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C4"));
      
      paramID = theObject->GetParameterID(wxT("C5"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C5"));
      
      paramID = theObject->GetParameterID(wxT("C6"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C6"));
      
      paramID = theObject->GetParameterID(wxT("C7"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C7"));
      
      paramID = theObject->GetParameterID(wxT("C8"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C8"));
      
      paramID = theObject->GetParameterID(wxT("C9"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C9"));
      
      paramID = theObject->GetParameterID(wxT("C10"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C10"));
      
      paramID = theObject->GetParameterID(wxT("C11"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C11"));
      
      paramID = theObject->GetParameterID(wxT("C12"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C12"));
      
      paramID = theObject->GetParameterID(wxT("C13"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C13"));
      
      paramID = theObject->GetParameterID(wxT("C14"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C14"));
   
      paramID = theObject->GetParameterID(wxT("C15"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C15"));
      
      paramID = theObject->GetParameterID(wxT("C16"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("C16"));

      paramID = theObject->GetParameterID(wxT("C_UNITS"));
      StringArray coefUnits = theObject->GetStringArrayParameter(paramID);
      
      for (Integer i = 0; i < coefCount; i++)
      {
         coefGrid->SetCellValue(i, 0, coefNames[i].c_str());
         coefGrid->SetCellValue(i, 1, wxVariant(coefValues[i]));
         coefGrid->SetCellValue(i, 2, coefUnits[i].c_str());
      }
   }
   else if (coefType == wxT("K"))
   {
      paramID = theObject->GetParameterID(wxT("K1"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K1"));
      
      paramID = theObject->GetParameterID(wxT("K2"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K2"));
      
      paramID = theObject->GetParameterID(wxT("K3"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K3"));
      
      paramID = theObject->GetParameterID(wxT("K4"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K4"));
      
      paramID = theObject->GetParameterID(wxT("K5"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K5"));
      
      paramID = theObject->GetParameterID(wxT("K6"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K6"));
      
      paramID = theObject->GetParameterID(wxT("K7"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K7"));
      
      paramID = theObject->GetParameterID(wxT("K8"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K8"));
      
      paramID = theObject->GetParameterID(wxT("K9"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K9"));
      
      paramID = theObject->GetParameterID(wxT("K10"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K10"));
      
      paramID = theObject->GetParameterID(wxT("K11"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K11"));
      
      paramID = theObject->GetParameterID(wxT("K12"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K12"));
      
      paramID = theObject->GetParameterID(wxT("K13"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K13"));
      
      paramID = theObject->GetParameterID(wxT("K14"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K14"));
      
      paramID = theObject->GetParameterID(wxT("K15"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K15"));
      
      paramID = theObject->GetParameterID(wxT("K16"));
      coefValues.push_back(theObject->GetRealParameter(paramID));
      coefNames.push_back(wxT("K16"));
      
      paramID = theObject->GetParameterID(wxT("K_UNITS"));
      StringArray coefUnits = theObject->GetStringArrayParameter(paramID);
      
      for (Integer i = 0; i < coefCount; i++)
      {
         coefGrid->SetCellValue(i, 0, coefNames[i].c_str());
         coefGrid->SetCellValue(i, 1, wxVariant(coefValues[i]));
         coefGrid->SetCellValue(i, 2, coefUnits[i].c_str());
      }
   }
}

//------------------------------------------------------------------------------
// virtual void SaveData()
//------------------------------------------------------------------------------
void ThrusterCoefficientDialog::SaveData()
{
   #ifdef DEBUG_COEF_SAVE
   MessageInterface::ShowMessage
      (wxT("ThrusterCoefficientDialog::SaveData() entered\n"));
   #endif
   
   canClose = true;
   
   // Validate input values
   for (int i = 0; i < coefCount; i++)
   {
      wxString field = coefGrid->GetCellValue(i, 0).c_str();
      wxString input = coefGrid->GetCellValue(i, 1).c_str();
      #ifdef DEBUG_COEF_SAVE
      MessageInterface::ShowMessage(wxT("   %s = '%s'\n"), field.c_str(), input.c_str());
      #endif
      CheckReal(coefValues[i], input, field, wxT("Real Number"));
   }
   
   if (!canClose)
      return;
   
   // Save values
   Integer paramID = 0;
   
   #ifdef DEBUG_COEF_SAVE
   MessageInterface::ShowMessage(wxT("   Now saving coef values to Thruster\n"));
   #endif
   
   for (int i = 0; i < coefCount; i++)
   {
      paramID = theObject->GetParameterID(coefNames[i]);
      theObject->SetRealParameter(paramID, coefValues[i]);
   }
   
   #ifdef DEBUG_COEF_SAVE
   MessageInterface::ShowMessage
      (wxT("ThrusterCoefficientDialog::SaveData() exiting\n"));
   #endif
}  

//------------------------------------------------------------------------------
// virtual void ResetData()
//------------------------------------------------------------------------------
void ThrusterCoefficientDialog::ResetData()
{
}     

