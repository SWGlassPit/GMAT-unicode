//$Id: ParameterSelectDialog.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                              ParameterSelectDialog
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Linda Jun
// Created: 2004/02/25
//
/**
 * Implements ParameterSelectDialog class. This class shows dialog window where a
 * user parameter can be selected.
 */
//------------------------------------------------------------------------------

#include "ParameterSelectDialog.hpp"
#include "ParameterCreateDialog.hpp"
#include "ParameterInfo.hpp"            // for GetDepObjectType()
#include "Array.hpp"                    // for GetRowCount()
#include "MessageInterface.hpp"

//#define DEBUG_PSDIALOG_LOAD
//#define DEBUG_PSDIALOG_SAVE
//#define DEBUG_PSDIALOG_BUTTON
//#define DEBUG_PSDIALOG_OBJECT
//#define DEBUG_PSDIALOG_PROPERTY
//#define DEBUG_PSDIALOG_LISTBOX_SELECT
//#define DEBUG_PSDIALOG_LISTBOX_DOUBLE_CLICK
//#define DEBUG_PSDIALOG_MULTI_SELECT
//#define DEBUG_PSDIALOG_WHOLE_OBJECT
//#define DEBUG_PSDIALOG_CS

//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(ParameterSelectDialog, GmatDialog)
   EVT_BUTTON(ID_BUTTON_CANCEL, ParameterSelectDialog::OnCancel)
   EVT_COMBOBOX(COMBOBOX_ID, ParameterSelectDialog::OnComboBoxChange)
   EVT_BUTTON(BUTTON_ID, ParameterSelectDialog::OnButtonClick)
   EVT_LISTBOX(LISTBOX_ID, ParameterSelectDialog::OnListBoxSelect)
   EVT_LISTBOX_DCLICK(LISTBOX_ID, ParameterSelectDialog::OnListBoxDoubleClick)
   EVT_CHECKBOX(CHECKBOX_ID, ParameterSelectDialog::OnCheckBoxChange)
END_EVENT_TABLE()

   
//------------------------------------------------------------------------------
// ParameterSelectDialog(wxWindow *parent, ...)
//------------------------------------------------------------------------------
/* Shows parameter selection dialog.
 *
 * @param *parent          parent window pointer
 * @param &objectTypeList  list of object types to show in the type ComboBox
 * @param showOption       one of GuiItemManager::ShowParamOption for object
 *                           properties (SHOW_PLOTTABLE)
 * @param allowMultiSelect true if multiple selection is allowed (false)
 * @param allowString      true if selection of String is allowed (false)
 * @param allowWholeObject true if selection of entire object is allowed (false)
 * @param allowSysParam    true if selection of system parameter is allowed (true)
 * @param allowVariable    true if selection of Varialbe is allowed (true)
 * @param allowArray       true if selection of Array is allowed (true)
 * @param &objectType      default object type to show ("Spacecraft")
 * @param createParam      true if to create non-existant system parameter (true)
 */
//------------------------------------------------------------------------------
ParameterSelectDialog::ParameterSelectDialog
     (wxWindow *parent, const wxArrayString &objectTypeList, int showOption,
      bool allowMultiSelect, bool allowString, bool allowWholeObject, 
      bool allowSysParam, bool allowVariable, bool allowArray, 
      const wxString &objectType, bool createParam)
   : GmatDialog(parent, -1, wxString(wxT("ParameterSelectDialog")))
{
   mHasSelectionChanged = false;
   mIsParamSelected = false;
   mIsAddingMode = false;
   mCanClose = true;
   mUseUserParam = false;
   mObjectTypeList = objectTypeList;
   mShowOption = showOption;
   mAllowMultiSelect = allowMultiSelect;
   mAllowString = allowString;
   mAllowWholeObject = allowWholeObject;
   mAllowVariable = allowVariable;
   mAllowArray = allowArray;
   mAllowSysParam = allowSysParam;
   mCreateParam = createParam;
   mObjectType = objectType;
   
   mNumRow = -1;
   mNumCol = -1;
   
   mParamNameArray.Clear();
   
   #ifdef DEBUG_PSDIALOG
   MessageInterface::ShowMessage
      (wxT("ParameterSelectDialog() mObjectType=%s, mShowOption=%d, mAllowSysParam=%d, ")
       wxT("mAllowVariable=%d\n   mAllowArray=%d, mAllowString=%d, mAllowWholeObject=%d, ")
       wxT("mAllowMultiSelect=%d, mCreateParam=%d\n"), mObjectType.c_str(), mShowOption,
       mAllowSysParam, mAllowVariable, mAllowArray, mAllowString, mAllowWholeObject,
       mAllowMultiSelect, mCreateParam);
   #endif
   
   Create();
   ShowData();
}


//------------------------------------------------------------------------------
// ~ParameterSelectDialog()
//------------------------------------------------------------------------------
ParameterSelectDialog::~ParameterSelectDialog()
{
   #ifdef DEBUG_PSDIALOG
   MessageInterface::ShowMessage
      (wxT("ParameterSelectDialog::~ParameterSelectDialog() Unregister ")
       wxT("mObjectListBox:%p\n"), mObjectListBox);
   #endif
   
   theGuiManager->UnregisterListBox(mObjectType, mObjectListBox);
   theGuiManager->UnregisterComboBox(wxT("CoordinateSystem"), mCoordSysComboBox);
   theGuiManager->UnregisterComboBox(wxT("CelestialBody"), mCentralBodyComboBox);
}


//------------------------------------------------------------------------------
// bool HasSelectionChanged()
//------------------------------------------------------------------------------
bool ParameterSelectDialog::HasSelectionChanged()
{
   return ParameterSelectDialog::mHasSelectionChanged;
}


//------------------------------------------------------------------------------
// bool IsParamSelected()
//------------------------------------------------------------------------------
bool ParameterSelectDialog::IsParamSelected()
{
   return ParameterSelectDialog::mIsParamSelected;
}


//------------------------------------------------------------------------------
// wxString GetParamName()
//------------------------------------------------------------------------------
wxString ParameterSelectDialog::GetParamName()
{
   return mParamName;
}


//------------------------------------------------------------------------------
// void SetObjectType(const wxString &objType)
//------------------------------------------------------------------------------
void ParameterSelectDialog::SetObjectType(const wxString &objType)
{
   mObjectType = objType;
   mObjectTypeComboBox->SetValue(mObjectType);
}


//------------------------------------------------------------------------------
// wxArrayString& GetParamNameArray()
//------------------------------------------------------------------------------
wxArrayString& ParameterSelectDialog::GetParamNameArray()
{
   return mParamNameArray;
}


//------------------------------------------------------------------------------
//void SetParamNameArray(const wxArrayString &paramNames)
//------------------------------------------------------------------------------
void ParameterSelectDialog::SetParamNameArray(const wxArrayString &paramNames)
{
   mParamNameArray = paramNames;
   
   #ifdef DEBUG_PSDIALOG
   MessageInterface::ShowMessage
      (wxT("SetParamNameArray() param count=%d\n"), mParamNameArray.GetCount());
   #endif
   
   // show selected parameter
   for (unsigned int i=0; i<mParamNameArray.GetCount(); i++)
   {
      mSelectedListBox->Append(mParamNameArray[i]);
      #ifdef DEBUG_PSDIALOG
      MessageInterface::ShowMessage(wxT("param=%s\n"), mParamNameArray[i].c_str());
      #endif
   }
}


//------------------------------------------------------------------------------
// virtual void Create()
//------------------------------------------------------------------------------
void ParameterSelectDialog::Create()
{
   #ifdef DEBUG_PSDIALOG
   MessageInterface::ShowMessage(wxT("ParameterSelectDialog::Create() entered.\n"));
   #endif
   
   //------------------------------------------------------
   // create parameter sizer
   //------------------------------------------------------
   
   mParameterSizer = theGuiManager->
      Create3ColParameterSizer(this, &mEntireObjectCheckBox, CHECKBOX_ID,
                               &mObjectTypeComboBox, COMBOBOX_ID,
                               &mObjectListBox, LISTBOX_ID,
                               &mRowStaticText, TEXT_ID,
                               &mColStaticText, TEXT_ID,
                               &mRowTextCtrl, TEXTCTRL_ID,
                               &mColTextCtrl, TEXTCTRL_ID,
                               &mPropertyListBox, LISTBOX_ID,
                               &mCoordSysComboBox, COMBOBOX_ID,
                               &mCentralBodyComboBox, COMBOBOX_ID,
                               &mCoordSysLabel, &mCoordSysSizer,
                               &mUpButton, BUTTON_ID,
                               &mDownButton, BUTTON_ID,
                               &mAddButton, BUTTON_ID,
                               &mRemoveButton, BUTTON_ID,
                               &mAddAllButton, BUTTON_ID,
                               &mRemoveAllButton, BUTTON_ID,
                               &mSelectedListBox, LISTBOX_ID,
                               mObjectTypeList, mShowOption,
                               mAllowMultiSelect, mAllowString,
                               mAllowWholeObject, mAllowSysParam,
                               mAllowVariable, mAllowArray, mObjectType,
                               wxT("Parameter Select"));
   
   //------------------------------------------------------
   // add to parent sizer
   //------------------------------------------------------
   theMiddleSizer->Add(mParameterSizer, 0, wxALIGN_CENTRE|wxALL, 5);

}


//------------------------------------------------------------------------------
// virtual void LoadData()
//------------------------------------------------------------------------------
void ParameterSelectDialog::LoadData()
{   
   #ifdef DEBUG_PSDIALOG_LOAD
   MessageInterface::ShowMessage(wxT("ParameterSelectDialog::LoadData() entered.\n"));
   #endif
   
   if (mShowOption != GuiItemManager::SHOW_WHOLE_OBJECT_ONLY)
   {
      if (mAllowSysParam)
      {
         // Let's alway select the first item (loj: 2009.02.04)
         //if (!mAllowMultiSelect)
         mPropertyListBox->SetSelection(0);
         
         if (mObjectType == wxT("ImpulsiveBurn") || mAllowMultiSelect)
         {
            mCoordSysLabel->Hide();
            mCoordSysComboBox->SetValue(wxT(""));
            mCentralBodyComboBox->SetValue(wxT(""));
            mCoordSysComboBox->Hide();
            mCentralBodyComboBox->Hide();
         }
         else
         {
            mLastCoordSysName = mCoordSysComboBox->GetString(0);
            
            // show coordinate system or central body
            ShowCoordSystem();
         }
      }
   }
   
   // Let's alway select the first item (loj: 2009.02.04)   
   // fire ListBoxSect event to show array info or not if single selection
   //if (!mAllowMultiSelect)
   //{
      mObjectListBox->SetSelection(0);
      wxCommandEvent tempEvent;
      tempEvent.SetEventObject(mObjectListBox);
      OnListBoxSelect(tempEvent);
   //}
   
   // hide array element
   ShowArrayInfo(false);
   
   #ifdef DEBUG_PSDIALOG_LOAD
   MessageInterface::ShowMessage
      (wxT("ParameterSelectDialog::LoadData() exiting. mIsParamSelected=%d\n"),
       mIsParamSelected);
   #endif
}


//------------------------------------------------------------------------------
// virtual void SaveData()
//------------------------------------------------------------------------------
void ParameterSelectDialog::SaveData()
{
   #ifdef DEBUG_PSDIALOG_SAVE
   MessageInterface::ShowMessage(wxT("ParameterSelectDialog::SaveData() entered.\n"));
   #endif
   
   mCanClose = true;
   mParamNameArray.Clear();
   mIsParamSelected = false;
   
   if (mSelectedListBox->GetCount() > 0)
   {
      mIsParamSelected = true;
      mParamName = mSelectedListBox->GetString(0);
      
      for(unsigned int i=0; i<mSelectedListBox->GetCount(); i++)
      {
         #ifdef DEBUG_PSDIALOG_SAVE
         MessageInterface::ShowMessage
            (wxT("   adding %s\n"), mSelectedListBox->GetString(i).c_str());
         #endif
         
         mParamNameArray.Add(mSelectedListBox->GetString(i));
      }
   }
   
   #ifdef DEBUG_PSDIALOG_SAVE
   MessageInterface::ShowMessage
      (wxT("ParameterSelectDialog::SaveData() exiting. mIsParamSelected=%d\n"),
       mIsParamSelected);
   #endif
}


//------------------------------------------------------------------------------
// virtual void ResetData()
//------------------------------------------------------------------------------
void ParameterSelectDialog::ResetData()
{
   mIsParamSelected = false;
}


//------------------------------------------------------------------------------
// void OnCancel()
//------------------------------------------------------------------------------
/**
 * Resets selection changed flag to false.
 */
//------------------------------------------------------------------------------
void ParameterSelectDialog::OnCancel(wxCommandEvent &event)
{
   mHasSelectionChanged = false;
   GmatDialog::OnCancel(event);
}


//------------------------------------------------------------------------------
// void OnButtonClick(wxCommandEvent& event)
//------------------------------------------------------------------------------
void ParameterSelectDialog::OnButtonClick(wxCommandEvent& event)
{
   #ifdef DEBUG_PSDIALOG_BUTTON
   MessageInterface::ShowMessage(wxT("OnButtonClick() entered\n"));
   #endif
   
   if (event.GetEventObject() == mUpButton)
   {
      int sel = mSelectedListBox->GetSelection();
      
      if (sel-1 >= 0)
      {
         wxString frontString = mSelectedListBox->GetString(sel-1);
         mSelectedListBox->SetString(sel-1, mSelectedListBox->GetStringSelection());
         mSelectedListBox->SetString(sel, frontString);
         mSelectedListBox->SetSelection(sel-1);
         mHasSelectionChanged = true;
      }
   }
   else if (event.GetEventObject() == mDownButton)
   {
      unsigned int sel = mSelectedListBox->GetSelection();
      
      if (sel+1 >= 1 && (sel+1) < mSelectedListBox->GetCount())
      {
         wxString rearString = mSelectedListBox->GetString(sel+1);
         mSelectedListBox->SetString(sel+1, mSelectedListBox->GetStringSelection());
         mSelectedListBox->SetString(sel, rearString);      
         mSelectedListBox->SetSelection(sel+1);
         mHasSelectionChanged = true;
      }
   }
   else if (event.GetEventObject() == mAddButton)
   {
      if (mAllowMultiSelect)
      {
         // clear old selections
         mLastPropertySelections.Clear();
         
         // set adding mode to true
         mIsAddingMode = true;
         
         if (AddMultipleSelections())
            mHasSelectionChanged = true;
         
         // reset adding mode to false
         mIsAddingMode = false;
      }
      else
      {
         if (AddParameter())
            mHasSelectionChanged = true;
      }
   }
   else if (event.GetEventObject() == mRemoveButton)
   {
      RemoveParameter();
      mHasSelectionChanged = true;
   }
   else if (event.GetEventObject() == mAddAllButton)
   {
      AddAll();
      mHasSelectionChanged = true;
   }
   else if (event.GetEventObject() == mRemoveAllButton)
   {
      mSelectedListBox->Clear();
      mHasSelectionChanged = true;
   }
   
   if (mHasSelectionChanged)
      EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnListBoxSelect(wxCommandEvent& event)
//------------------------------------------------------------------------------
void ParameterSelectDialog::OnListBoxSelect(wxCommandEvent& event)
{
   #ifdef DEBUG_PSDIALOG_LISTBOX_SELECT
   MessageInterface::ShowMessage(wxT("OnListBoxSelect() entered\n"));
   #endif
   
   wxObject *obj = event.GetEventObject();
   
   if (obj == mObjectListBox)
   {
      if (mObjectListBox->IsEmpty())
         return;
      
      #ifdef DEBUG_PSDIALOG_LISTBOX_SELECT
      if (mAllowMultiSelect)
      {
         wxArrayInt selections;
         int count = mObjectListBox->GetSelections(selections);
         MessageInterface::ShowMessage
            (wxT("   mObjectListBox %d items selected.\n"), count);
      }
      #endif
      
      wxString objType = mObjectTypeComboBox->GetValue();
      
      #ifdef DEBUG_PSDIALOG_LISTBOX_SELECT
      MessageInterface::ShowMessage
         (wxT("   mObjectListBox item of <%s> selected\n"), objType.c_str());
      #endif
      
      wxString objStr = GetObjectSelection();
      
      // if array, show row and column
      if (objType == wxT("Array"))
      {         
         if (objStr == wxT(""))
         {
            // hide array element
            ShowArrayInfo(false);            
            return;
         }
         
         Parameter *param = theGuiInterpreter->GetParameter(objStr.c_str());
         Array *array = (Array*)param;
         
         // save ro and col so that we can do range check
         mNumRow = array->GetRowCount();
         mNumCol = array->GetColCount();
         
         #ifdef DEBUG_PSDIALOG_LISTBOX_SELECT
         MessageInterface::ShowMessage
            (wxT("   item=%s, mNumRow=%d, mNumCol=%d\n"), objStr.c_str(), mNumRow, mNumCol);
         #endif
         
         wxString str;
         str.Printf(wxT("%d"), mNumRow);
         mRowStaticText->SetLabel(wxT("Row [") + str + wxT("]"));
         str.Printf(wxT("%d"), mNumCol);
         mColStaticText->SetLabel(wxT("Col [") + str + wxT("]"));
         
         // show array element
         ShowArrayInfo(true);
      }
      else
      {
         // hide array element
         ShowArrayInfo(false);
      }
   }
   else if (obj == mPropertyListBox)
   {
      #ifdef DEBUG_PSDIALOG_LISTBOX_SELECT
      if (mAllowMultiSelect)
      {
         wxArrayInt selections;
         int count = mPropertyListBox->GetSelections(selections);
         MessageInterface::ShowMessage
            (wxT("   mPropertyListBox %d items selected.\n"), count);
      }
      #endif
      
      // show coordinate system or central body
      ShowCoordSystem();
   }   
}


//------------------------------------------------------------------------------
// void OnListBoxDoubleClick(wxCommandEvent& event)
//------------------------------------------------------------------------------
void ParameterSelectDialog::OnListBoxDoubleClick(wxCommandEvent& event)
{
   #ifdef DEBUG_PSDIALOG_LISTBOX_DOUBLE_CLICK
   MessageInterface::ShowMessage(wxT("OnListBoxDoubleClick() entered\n"));
   #endif
   
   wxObject *obj = event.GetEventObject();
   
   if (obj == mObjectListBox)
   {
      #ifdef DEBUG_PSDIALOG_LISTBOX_DOUBLE_CLICK
      MessageInterface::ShowMessage(wxT("   mObjectListBox double clicked\n"));
      #endif
      
      if (AddWholeObject())
         mHasSelectionChanged = true;
   }
   else if (obj == mPropertyListBox)
   {
      #ifdef DEBUG_PSDIALOG_LISTBOX_DOUBLE_CLICK
      MessageInterface::ShowMessage(wxT("   mPropertyListBox double clicked\n"));
      #endif
      
      if (mEntireObjectCheckBox->IsChecked())
      {
         wxLogMessage(wxT("Please unchek the Select Entire Object check box\n")
                      wxT("before adding the property to the liset."));
         return;
      }
      
      if (AddParameter())
         mHasSelectionChanged = true;
   }
   else if (obj == mSelectedListBox)
   {
      #ifdef DEBUG_PSDIALOG_LISTBOX_DOUBLE_CLICK
      MessageInterface::ShowMessage(wxT("   mSelectedListBox double clicked\n"));
      #endif
      
      RemoveParameter();
      mHasSelectionChanged = true;
   }
   
   if (mHasSelectionChanged)
      EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnComboBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void ParameterSelectDialog::OnComboBoxChange(wxCommandEvent& event)
{
   #ifdef DEBUG_PSDIALOG_COMBOBOX_CHANGE
   MessageInterface::ShowMessage(wxT("OnComboBoxChange() entered\n"));
   #endif
   
   wxObject *obj = event.GetEventObject();
   
   if (obj == mObjectTypeComboBox)
   {
      wxString objType = mObjectTypeComboBox->GetValue();
      
      #ifdef DEBUG_PSDIALOG_COMBOBOX_CHANGE
      MessageInterface::ShowMessage
         (wxT("   ObjectTypeComboBox changed to %s\n"), objType.c_str());
      #endif
      
      // Clear object ListBox
      mObjectListBox->Clear();
      
      if (objType == wxT("Spacecraft"))
      {
         // Show Spacecraft objects
         mObjectListBox->InsertItems(theGuiManager->GetSpacecraftList(), 0);
         
         // Set Spacecraft property
         mPropertyListBox->
            Set(theGuiManager->GetPropertyList(wxT("Spacecraft"),
                                               GuiItemManager::SHOW_PLOTTABLE));
         
         if (!mAllowMultiSelect)
            mPropertyListBox->SetSelection(0);
         
         ShowCoordSystem();
      }
      else if (objType == wxT("ImpulsiveBurn"))
      {
         // Show ImpulsiveBurn objects
         mObjectListBox->InsertItems(theGuiManager->GetImpulsiveBurnList(), 0);
         
         // Set ImpulsiveBurn property
         mPropertyListBox->
            Set(theGuiManager->GetPropertyList(wxT("ImpulsiveBurn"),
                                               GuiItemManager::SHOW_PLOTTABLE));
         
         if (!mAllowMultiSelect)
            mPropertyListBox->SetSelection(0);
      }
      else if (objType == wxT("Variable"))
      {
         // Show Variables
         if (theGuiManager->GetNumUserVariable() > 0)
         {
            mObjectListBox->InsertItems(theGuiManager->GetUserVariableList(), 0);
         }
         
         // There is no properties
         ClearProperties();
      }
      else if (objType == wxT("Array"))
      {
         // Show Arrays
         if (theGuiManager->GetNumUserArray() > 0)
         {
            mObjectListBox->InsertItems(theGuiManager->GetUserArrayList(), 0);
         }
         
         // There is no properties
         ClearProperties();
      }
      else if (objType == wxT("String"))
      {
         #ifdef DEBUG_PSDIALOG_COMBOBOX_CHANGE
         MessageInterface::ShowMessage
            (wxT("   theGuiManager->GetNumUserString() = %d\n"),
             theGuiManager->GetNumUserString());
         #endif
         
         // Show Strings
         if (theGuiManager->GetNumUserString() > 0)
         {
            mObjectListBox->InsertItems(theGuiManager->GetUserStringList(), 0);
         }
         
         // There is no properties
         ClearProperties();
      }
      else
      {
         mPropertyListBox->Clear();
      }
   }
   else if(obj == mCoordSysComboBox)
   {
      mLastCoordSysName = mCoordSysComboBox->GetValue();
   }
   
   // Clear last selections
   mLastObjectSelections.Clear();
   
   // hide array info for multiple selection
   if (mAllowMultiSelect)
      ShowArrayInfo(false);
   
   // fire ObjectListBox select event if single selection
   if ((!mAllowMultiSelect) && (obj != mCoordSysComboBox))
   {
      mObjectListBox->SetSelection(0);
      wxCommandEvent tempEvent;
      tempEvent.SetEventObject(mObjectListBox);
      OnListBoxSelect(tempEvent);
   }
}


//------------------------------------------------------------------------------
// void OnCheckBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void ParameterSelectDialog::OnCheckBoxChange(wxCommandEvent& event)
{
   if (event.GetEventObject() == mEntireObjectCheckBox)
   {
      bool allowWholeObject = mEntireObjectCheckBox->IsChecked();
      //#ifdef DEBUG_PSDIALOG_CHECK_BOX
      MessageInterface::ShowMessage
         (wxT("OnCheckBoxChange() IsChecked()=%d\n"),
          mEntireObjectCheckBox->IsChecked());
      MessageInterface::ShowMessage
         (wxT("On\nCheck\nBox\nChange() IsChecked()=%d\n"),
          mEntireObjectCheckBox->IsChecked());
      //#endif
      
      // if user can whole object, set ListBox style to wxLB_EXTENDED
      if (allowWholeObject)
         mObjectListBox->SetWindowStyle(wxLB_EXTENDED);
      else
      {
         // Deselect all objects and Select first one
         DeselectAllObjects();
         mObjectListBox->SetSelection(0);
         mObjectListBox->SetWindowStyle(wxLB_SINGLE);
      }
      
      Refresh();
   }
}


//------------------------------------------------------------------------------
// bool AddWholeObject()
//------------------------------------------------------------------------------
/*
 * @return true if selected object was added to the list. It will add if
 * whole object is allowed or Variable or String.
 */
//------------------------------------------------------------------------------
bool ParameterSelectDialog::AddWholeObject()
{
   #ifdef DEBUG_PSDIALOG_WHOLE_OBJECT
   MessageInterface::ShowMessage(wxT("AddWholeObject() entered\n"));
   #endif
   
   wxString objType = mObjectTypeComboBox->GetValue();
   wxString objName = GetObjectSelection();
   
   // check if type is Variable
   if (objType == wxT("Variable"))
   {
      AddParameter(objName);
      return true;
   }
   
   // check if type is String
   if (objType == wxT("String"))
   {
      if (mShowOption == GuiItemManager::SHOW_PLOTTABLE)
      {
         wxLogMessage(wxT("Selection of String object type is not allowed."));
         return false;
      }
      else
      {
         AddParameter(objName);
         return true;
      }
   }
   
   // check if whole object is allowed other than Variable or String
   if (mEntireObjectCheckBox->IsChecked())
   {
      if (mAllowWholeObject)
      {
         AddParameter(objName);
         return true;
      }
      else
      {
         wxLogMessage(wxT("Selection of entire object is not allowed."));
         return false;
      }
   }
   
   #ifdef DEBUG_PSDIALOG_WHOLE_OBJECT
   MessageInterface::ShowMessage(wxT("AddWholeObject() returning false\n"));
   #endif
   
   return false;
}


//------------------------------------------------------------------------------
// bool AddParameter()
//------------------------------------------------------------------------------
/*
 * @return true if parameter added to selected list box
 */
//------------------------------------------------------------------------------
bool ParameterSelectDialog::AddParameter()
{
   #ifdef DEBUG_PSDIALOG_PARAMETER
   MessageInterface::ShowMessage(wxT("AddParameter() entered\n"));
   #endif
   
   if (mAllowMultiSelect)
   {
      wxArrayInt objectSelects;
      wxArrayInt propertySelects;
      int objCount = mObjectListBox->GetSelections(objectSelects);
      int propCount = mPropertyListBox->GetSelections(propertySelects);
      if (objCount == 0 && propCount == 0)
      {
         #ifdef DEBUG_PSDIALOG_PARAMETER
         MessageInterface::ShowMessage
            (wxT("AddParameter() nothing selected, returning false\n"));
         #endif
         
         return false;
      }
   }
   
   // if whole object added or select whole object is checked
   if (AddWholeObject() || mEntireObjectCheckBox->IsChecked())
   {
      #ifdef DEBUG_PSDIALOG_PARAMETER
      MessageInterface::ShowMessage(wxT("AddParameter() returning true\n"));
      #endif
      
      return true;
   }
   
   // if add an array element
   if (mObjectTypeComboBox->GetValue() == wxT("Array") &&
       !mEntireObjectCheckBox->IsChecked())
   {
      #ifdef DEBUG_PSDIALOG_PARAMETER
      MessageInterface::ShowMessage(wxT("   adding array element\n"));
      #endif
      
      wxString rowStr = mRowTextCtrl->GetValue();
      wxString colStr = mColTextCtrl->GetValue();
      rowStr = rowStr.Strip(wxString::both);
      colStr = colStr.Strip(wxString::both);
      
      Integer row = -1;
      Integer col = -1;
      bool valid = false;
      
      // check for valid integer value first
      valid = CheckInteger(row, rowStr.c_str(), wxT("Row"), wxT("Integer >= 1 and =< [Dimension]"));
      valid = valid && CheckInteger(col, colStr.c_str(), wxT("Col"), wxT("Integer >= 1 and =< [Dimension]"));
      
      #ifdef DEBUG_PSDIALOG_PARAMETER
      MessageInterface::ShowMessage(wxT("   valid=%d, row=%d, col=%d\n"), valid, row, col);
      MessageInterface::ShowMessage(wxT("   mNumRow=%d, mNumCol=%d\n"), mNumRow, mNumCol);
      #endif
      
      if (!valid)
         return false;
      
      valid = true;
      
      // do row range checking
      if (row < 1 || row > mNumRow)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("Row index \"%s\" is out of range.\n")
             wxT("Valid range is between 1 and %d\n"), rowStr.c_str(), mNumRow);
         valid = false;
      }
      
      // do column range checking
      if (col < 1 || col > mNumCol)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("Column index \"%s\" is out of range.\n")
             wxT("Valid range is between 1 and %d\n"), colStr.c_str(), mNumCol);
         valid = false;
      }
      
      if (!valid)
         return false;
      
      wxString arrayStr = GetObjectSelection();
      wxString arrayElem = arrayStr + wxT("(") + rowStr + wxT(",") + colStr + wxT(")");
      
      #ifdef DEBUG_PSDIALOG_PARAMETER
      MessageInterface::ShowMessage(wxT("   adding %s\n"), arrayElem.c_str());
      #endif
      
      AddParameter(arrayElem);
      return true;
   }
   
   // now compose parameter name by adding property to object name
   wxString newParam = FormParameterName();
   
   // if newParam is properly created
   if (newParam != wxT(""))
   {
      // Create a system paramete if it does not exist
      if (mAllowSysParam && mCreateParam)
      {
         Parameter *param = GetParameter(newParam);
         if (param == NULL)
         {
            wxLogMessage(wxT("Cannot create a Parameter %s."), newParam.c_str());
            return false;
         }
      }
      
      AddParameter(newParam);
      return true;
   }
   
   #ifdef DEBUG_PSDIALOG_PARAMETER
   MessageInterface::ShowMessage(wxT("AddParameter() returning false\n"));
   #endif
   
   return false;
}


//------------------------------------------------------------------------------
// void AddParameter(const wxString &param)
//------------------------------------------------------------------------------
void ParameterSelectDialog::AddParameter(const wxString &param)
{
   #ifdef DEBUG_PSDIALOG_PARAMETER
   MessageInterface::ShowMessage(wxT("AddParameter() param=<%s>\n"), param.c_str());
   #endif
   
   // if the string wasn't found in the selected list, insert it
   if (mSelectedListBox->FindString(param) == wxNOT_FOUND)
   {
      if (!mAllowMultiSelect)
         mSelectedListBox->Clear();
      
      mSelectedListBox->Append(param);
      mSelectedListBox->SetStringSelection(param);
   }
}


//------------------------------------------------------------------------------
// bool AddMultipleSelections()
//------------------------------------------------------------------------------
bool ParameterSelectDialog::AddMultipleSelections()
{
   #ifdef DEBUG_PSDIALOG_MULTI_SELECT
   MessageInterface::ShowMessage(wxT("AddMultipleSelections() entered\n"));
   #endif
   
   wxString objType = mObjectTypeComboBox->GetValue();
   
   wxArrayInt selections;
   int count = 0;
   
   // add multiple selections if entire object is selected
   if (mEntireObjectCheckBox->GetValue() == true ||
       objType == wxT("Variable") || objType == wxT("String"))
   {
      count = mObjectListBox->GetSelections(selections);
      
      #ifdef DEBUG_PSDIALOG_MULTI_SELECT
      MessageInterface::ShowMessage(wxT("   entire object selection=%d\n"), count);
      #endif
      
      if (count > 0)
      {      
         for (int i=0; i<count; i++)
         {
            #ifdef DEBUG_PSDIALOG_MULTI_SELECT
            MessageInterface::ShowMessage(wxT("   setting selection %d\n"), selections[i]);
            #endif
            
            mObjectListBox->SetSelection(selections[i]);
            
            // fire mObjectListBox select and double click event
            wxCommandEvent tempEvent;
            tempEvent.SetEventObject(mObjectListBox);
            OnListBoxSelect(tempEvent);
            OnListBoxDoubleClick(tempEvent);
            
            mObjectListBox->Deselect(selections[i]);
         }
         
         return true;
      }
   }
   else if (mPropertyListBox->IsEmpty())
   {
      if (objType == wxT("Array"))
      {
         count = mObjectListBox->GetSelections(selections);
         
         #ifdef DEBUG_PSDIALOG_MULTI_SELECT
         MessageInterface::ShowMessage(wxT("   array component selection=%d\n"), count);
         #endif
         
         if (AddParameter())
         {
            #ifdef DEBUG_PSDIALOG_MULTI_SELECT
            MessageInterface::ShowMessage(wxT("AddMultipleSelections() returning true\n"));
            #endif
            return true;
         }
      }
      else
      {
         MessageInterface::ShowMessage(wxT("==> What could it be?\n"));
      }
   }
   else
   {
      // check if any object is selected
      wxArrayInt selections;
      if (mObjectListBox->GetSelections(selections) == 0)
      {
         wxLogMessage(wxT("Please select an object."));
         return false;
      }
      
      count = mPropertyListBox->GetSelections(selections);
      if (count == 0)
      {
         wxLogMessage(wxT("Please select a property."));
         return false;
      }
      
      //----------------------------------------------------
      // deselect all selections first
      //----------------------------------------------------
      for (int i=0; i<count; i++)
      {
         #ifdef DEBUG_PSDIALOG_MULTI_SELECT
         MessageInterface::ShowMessage(wxT("   deselecting %d\n"), selections[i]);
         #endif
         mPropertyListBox->Deselect(selections[i]);
      }
      
      //----------------------------------------------------
      // select one property at a time
      //----------------------------------------------------
      for (int i=0; i<count; i++)
      {
         #ifdef DEBUG_PSDIALOG_MULTI_SELECT
         MessageInterface::ShowMessage(wxT("   setting selection %d\n"), selections[i]);
         #endif
         
         mPropertyListBox->SetSelection(selections[i]);
         
         // fire mPropertyListBox select and double click event
         wxCommandEvent tempEvent;
         tempEvent.SetEventObject(mPropertyListBox);
         OnListBoxSelect(tempEvent);
         OnListBoxDoubleClick(tempEvent);
         
         mPropertyListBox->Deselect(selections[i]);
      }
      
      return true;
   }
   
   #ifdef DEBUG_PSDIALOG_MULTI_SELECT
   MessageInterface::ShowMessage(wxT("AddMultipleSelections() returning false\n"));
   #endif
   
   return false;
}


//------------------------------------------------------------------------------
// void AddAll()
//------------------------------------------------------------------------------
void ParameterSelectDialog::AddAll()
{
   wxString objType = mObjectTypeComboBox->GetValue();
   bool entireObj = mEntireObjectCheckBox->IsChecked();
   
   if (objType == wxT("Variable") || objType == wxT("String") || entireObj)
   {
      if (mAllowMultiSelect)
      {
         // go through all of the property listbox
         int count = mObjectListBox->GetCount();
         for (int i=0; i<count; i++)
            mObjectListBox->SetSelection(i);
         
         AddMultipleSelections();
      }
      else
      {
         // go through object listbox
         int count = mObjectListBox->GetCount();
         for (int i=0; i<count; i++)
         {
            mObjectListBox->SetSelection(i);
            
            // fire ObjectListBox double click event
            wxCommandEvent tempEvent;
            tempEvent.SetEventObject(mObjectListBox);
            OnListBoxDoubleClick(tempEvent);
         }
      }
   }
   else
   {
      if (mAllowMultiSelect)
      {
         // check if object is selected
         if (GetObjectSelection() == wxT(""))
         {
            wxLogMessage(wxT("Please select an object."));
         }
         else
         {
            // go through all of the property listbox
            int count = mPropertyListBox->GetCount();
            for (int i=0; i<count; i++)
            {
               mPropertyListBox->SetSelection(i);         
               AddMultipleSelections();
            }
         }
      }
      else
      {
         // go through all of the property listbox
         int count = mPropertyListBox->GetCount();
         for (int i=0; i<count; i++)
         {
            mPropertyListBox->SetSelection(i);
            
            // fire mPropertyListBox select and double click event
            wxCommandEvent tempEvent;
            tempEvent.SetEventObject(mPropertyListBox);
            OnListBoxSelect(tempEvent);
            OnListBoxDoubleClick(tempEvent);
         }
      }
   }
}


//------------------------------------------------------------------------------
// void RemoveParameter()
//------------------------------------------------------------------------------
void ParameterSelectDialog::RemoveParameter()
{
   int sel = mSelectedListBox->GetSelection();
   mSelectedListBox->Delete(sel);
   
   if (sel-1 < 0)
      mSelectedListBox->SetSelection(0);
   else
      mSelectedListBox->SetSelection(sel-1);
}


//------------------------------------------------------------------------------
// void ShowArrayInfo(bool show)
//------------------------------------------------------------------------------
void ParameterSelectDialog::ShowArrayInfo(bool show)
{
   mParameterSizer->Show(mRowStaticText, show, true);
   mParameterSizer->Show(mColStaticText, show, true);
   mParameterSizer->Show(mRowTextCtrl, show, true);
   mParameterSizer->Show(mColTextCtrl, show, true);
   mParameterSizer->Layout();
}


//------------------------------------------------------------------------------
// void ShowCoordSystem()
//------------------------------------------------------------------------------
void ParameterSelectDialog::ShowCoordSystem()
{
   #ifdef DEBUG_PSDIALOG_CS
   MessageInterface::ShowMessage(wxT("ShowCoordSystem() entered\n"));
   #endif
   
   wxString property = GetPropertySelection().c_str();
   
   if (property == wxT(""))
   {
      #ifdef DEBUG_PSDIALOG_CS
      MessageInterface::ShowMessage(wxT("ShowCoordSystem() property is empty, so just retun\n"));
      #endif
      return;
   }
   
   GmatParam::DepObject depObj = ParameterInfo::Instance()->GetDepObjectType(property);
   
   #ifdef DEBUG_PSDIALOG_CS
   MessageInterface::ShowMessage(wxT("   depObj=%d\n"), depObj);
   #endif
   
   if (depObj == GmatParam::COORD_SYS)
   {
      mCoordSysLabel->Show();
      mCoordSysLabel->SetLabel(wxT("Coordinate ") wxT(GUI_ACCEL_KEY) wxT("System"));
      
      mCoordSysComboBox->SetStringSelection(mLastCoordSysName);
      
      mCoordSysSizer->Remove(mCoordSysComboBox);
      mCoordSysSizer->Remove(mCentralBodyComboBox);
      mCoordSysSizer->Add(mCoordSysComboBox);
      mCoordSysComboBox->Show();
      mCentralBodyComboBox->Hide();
      mParameterSizer->Layout();
   }
   else if (depObj == GmatParam::ORIGIN)
   {
      mCoordSysLabel->Show();
      mCoordSysLabel->SetLabel(wxT("Central ") wxT(GUI_ACCEL_KEY) wxT("Body"));
      
      // I had to remove mCoordSysComboBox first and then mCentralBodyComboBox,
      // otherwise, mCentralBodyComboBox shows too far to right
      mCoordSysSizer->Remove(mCoordSysComboBox);
      mCoordSysSizer->Remove(mCentralBodyComboBox);
      mCoordSysSizer->Add(mCentralBodyComboBox);
      mCentralBodyComboBox->Show();
      mCoordSysComboBox->Hide();
      mParameterSizer->Layout();
   }
   else
   {
      mCoordSysSizer->Remove(mCentralBodyComboBox);
      mCoordSysSizer->Remove(mCoordSysComboBox);
      mCoordSysLabel->Hide();
      mCoordSysComboBox->Hide();
      mCentralBodyComboBox->Hide();
      mParameterSizer->Layout();
   }
   
   #ifdef DEBUG_PSDIALOG_CS
   MessageInterface::ShowMessage(wxT("ShowCoordSystem() exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
// void ClearProperties()
//------------------------------------------------------------------------------
void ParameterSelectDialog::ClearProperties()
{
   mPropertyListBox->Clear();
   
   mCoordSysLabel->Hide();
   mCoordSysComboBox->Hide();
   mCentralBodyComboBox->Hide();
   mParameterSizer->Layout();
}


//------------------------------------------------------------------------------
// void DeselectAllObjects()
//------------------------------------------------------------------------------
void ParameterSelectDialog::DeselectAllObjects()
{
   wxArrayInt selections;
   int count = mObjectListBox->GetSelections(selections);
   
   for (int i=0; i<count; i++)
      mObjectListBox->Deselect(selections[i]);
}


//------------------------------------------------------------------------------
// void DeselectObjects(wxArrayInt &newSelects, wxArrayInt &oldSelects)
//------------------------------------------------------------------------------
void ParameterSelectDialog::DeselectObjects(wxArrayInt &newSelects,
                                            wxArrayInt &oldSelects)
{
   #ifdef DEBUG_PSDIALOG_MULTI_SELECT
   MessageInterface::ShowMessage
      (wxT("DeselectObjects() entered, count of newSelects=%d, oldSelects=%d\n"),
       newSelects.GetCount(), oldSelects.GetCount());
   #endif
   
   if (newSelects.GetCount() == 1)
   {
      #ifdef DEBUG_PSDIALOG_MULTI_SELECT
      MessageInterface::ShowMessage
         (wxT("DeselectObjects() only one selection, so just return\n"));
      #endif
      
      return;
   }
   
   for (unsigned int i=0; i<oldSelects.GetCount(); i++)
   {
      #ifdef DEBUG_PSDIALOG_MULTI_SELECT
      MessageInterface::ShowMessage(wxT("   oldSelects[%d]=%d\n"), i, oldSelects[i]);
      #endif
      
      for (unsigned int j=0; j<newSelects.GetCount(); j++)
      {
         #ifdef DEBUG_PSDIALOG_MULTI_SELECT
         MessageInterface::ShowMessage(wxT("   newSelects[%d]=%d\n"), j, newSelects[j]);
         #endif
         
         if (oldSelects[i] == newSelects[j])
            mObjectListBox->Deselect(oldSelects[i]);
      }
   }
   
   // update selections
   mObjectListBox->GetSelections(newSelects);
   
   #ifdef DEBUG_PSDIALOG_MULTI_SELECT
   MessageInterface::ShowMessage
      (wxT("DeselectObjects() exiting, count of newSelects=%d, oldSelects=%d\n"),
       newSelects.GetCount(), oldSelects.GetCount());
   #endif
}


//------------------------------------------------------------------------------
// int GetLastPropertySelection()
//------------------------------------------------------------------------------
int ParameterSelectDialog::GetLastPropertySelection()
{
   wxArrayInt selections;
   int newCount = mPropertyListBox->GetSelections(selections);
   int lastSelect = -1;
   
   #ifdef DEBUG_PSDIALOG_PROPERTY
   MessageInterface::ShowMessage
      (wxT("GetLastPropertySelection() newCount=%d, oldCount=%d\n"), newCount,
       mLastPropertySelections.GetCount());
   #endif
   
   if (newCount == 1)
   {
      lastSelect = selections[0];
   }
   else
   {
      for (int i=0; i<newCount; i++)
      {
         if (mLastPropertySelections.Index(selections[i]) == wxNOT_FOUND)
         {
            lastSelect = selections[i];
            break;
         }
      }
   }
   
   mLastPropertySelections = selections;
   
   #ifdef DEBUG_PSDIALOG_PROPERTY
   MessageInterface::ShowMessage
      (wxT("GetLastPropertySelection() return %d\n"), lastSelect);
   #endif
   
   return lastSelect;
}


//------------------------------------------------------------------------------
// wxString GetObjectSelection()
//------------------------------------------------------------------------------
/*
 * return selected string of object ListBox.
 * If multiple selection is allowed, it returns first string selection.
 *
 * For Array, multiple selection is not allowed if entire object is not checked, 
 * since it has to show row and column.
 */
//------------------------------------------------------------------------------
wxString ParameterSelectDialog::GetObjectSelection()
{
   #ifdef DEBUG_PSDIALOG_OBJECT
   MessageInterface::ShowMessage(wxT("GetObjectSelection() entered\n"));
   #endif
   
   wxString object;
   if (mAllowMultiSelect)
   {
      wxArrayInt selections;
      int newCount = mObjectListBox->GetSelections(selections);
      int oldCount = mLastObjectSelections.GetCount();
      
      #ifdef DEBUG_PSDIALOG_OBJECT
      MessageInterface::ShowMessage
         (wxT("   oldCount=%d, newCount=%d\n"), oldCount, newCount);
      #endif
      
      wxString objectType = mObjectTypeComboBox->GetValue();
      
      // check if only one selections is allowed, if so deselect one one
      if (objectType == wxT("Array") || objectType == wxT("Spacecraft") ||
          objectType == wxT("ImpulsiveBurn"))
      {
         // allow only one selection if not entire object selection
         if (oldCount > 0 && mEntireObjectCheckBox->GetValue() == false)
            DeselectObjects(selections, mLastObjectSelections);
         
         #ifdef DEBUG_PSDIALOG_OBJECT
         MessageInterface::ShowMessage
            (wxT("   selected count=%d\n"), selections.GetCount());
         #endif
         
         if (newCount > 0)
            object = mObjectListBox->GetString(selections[0]);
         
         mLastObjectSelections = selections;
      }
      else
      {
         if (newCount > 0)
            object = mObjectListBox->GetString(selections[0]);
      }
   }
   else
   {
      object = mObjectListBox->GetStringSelection();
   }
   
   #ifdef DEBUG_PSDIALOG_OBJECT
   MessageInterface::ShowMessage(wxT("GetObjectSelection() returning %s\n"),
                                 object.c_str());
   #endif
   
   return object;
}


//------------------------------------------------------------------------------
// wxString GetPropertySelection()
//------------------------------------------------------------------------------
/*
 * return selected string of property ListBox.
 * If multiple selection is allowed, it returns last string selection in selection
 * mode and returns first string selections in adding mode.
 */
//------------------------------------------------------------------------------
wxString ParameterSelectDialog::GetPropertySelection()
{
   #ifdef DEBUG_PSDIALOG_PROPERTY
   MessageInterface::ShowMessage
      (wxT("GetPropertySelection() entered, mIsAddingMode=%d\n"), mIsAddingMode);
   #endif
   
   wxString property;
   if (mAllowMultiSelect)
   {
      if (mIsAddingMode)
      {
         wxArrayInt selections;
         int count = mPropertyListBox->GetSelections(selections);
         
         if (count > 0)
            property = mPropertyListBox->GetString(selections[0]);
      }
      else
      {
         int lastSelect = GetLastPropertySelection();
         
         if (lastSelect != -1)
            property = mPropertyListBox->GetString(lastSelect);
      }
   }
   else
   {
      property = mPropertyListBox->GetStringSelection();
   }
   
   #ifdef DEBUG_PSDIALOG_PROPERTY
   MessageInterface::ShowMessage(wxT("GetPropertySelection() returning %s\n"),
                                 property.c_str());
   #endif
   
   return property;
}


//------------------------------------------------------------------------------
// wxString FormParameterName()
//------------------------------------------------------------------------------
wxString ParameterSelectDialog::FormParameterName()
{
   #ifdef DEBUG_PSDIALOG_PARAMETER
   MessageInterface::ShowMessage
      (wxT("ParameterSelectDialog::FormParameterName() entered\n"));
   #endif
   
   bool selectEntireObject = mEntireObjectCheckBox->IsChecked();
   
   // make sure object is selected
   if (GetObjectSelection() == wxT(""))
   {
      wxLogMessage(wxT("Please select an object."));
      return wxT("");
   }
   
   wxString typeName = mObjectTypeComboBox->GetValue();
   wxString objectName = GetObjectSelection();
   
   #ifdef DEBUG_PSDIALOG_PARAMETER
   MessageInterface::ShowMessage
      (wxT("   type=<%s>, object = <%s>\n"), typeName.c_str(), objectName.c_str());
   #endif
   
   // return whole object or array element
   if (selectEntireObject || typeName == wxT("Variable") || typeName == wxT("String"))
   {
      return objectName;
   }
   else if (mObjectTypeComboBox->GetValue() == wxT("Array"))
   {
      return objectName + wxT("(1,1)"); //@todo: compose row and col
   }
   
   wxString paramName;
   wxString depObjName = wxT("");
   wxString propertyName = GetPropertySelection();
   
   if (propertyName == wxT(""))
   {
      wxLogMessage(wxT("Please select a property."));
      return wxT("");
   }
   
   // now compose object.dep.property
   if (mCoordSysComboBox->IsShown())
      depObjName = mCoordSysComboBox->GetValue();
   else if (mCentralBodyComboBox->IsShown())
      depObjName = mCentralBodyComboBox->GetValue();
   
   #ifdef DEBUG_PSDIALOG_PARAMETER
   MessageInterface::ShowMessage(wxT("   property=<%s>\n"), propertyName.c_str());
   #endif
   
   if (depObjName == wxT(""))
      paramName = objectName + wxT(".") + propertyName;
   else
      paramName = objectName + wxT(".") + depObjName + wxT(".") + propertyName;
   
   #ifdef DEBUG_PSDIALOG_PARAMETER
   MessageInterface::ShowMessage
      (wxT("ParameterSelectDialog::FormParameterName() returning paramName=%s\n"),
       paramName.c_str());
   #endif
   
   return paramName;
}


//------------------------------------------------------------------------------
// Parameter* GetParameter(const wxString &name)
//------------------------------------------------------------------------------
/*
 * @return existing parameter pointer, return newly created parameter pointer
 *         if it does not exist.
 */
//------------------------------------------------------------------------------
Parameter* ParameterSelectDialog::GetParameter(const wxString &name)
{
   Parameter *param = 
      theGuiInterpreter->GetParameter(wxString(name.c_str()));
   
   // create a parameter if it does not exist
   if (param == NULL)
   {
      #ifdef DEBUG_PSDIALOG_PARAMETER
      MessageInterface::ShowMessage
         (wxT("ParameterSelectDialog::GetParameter() Creating parameter:<%s>\n"),
          name.c_str());
      #endif
      
      wxString paramName(name.c_str());
      wxString objTypeName(mObjectTypeComboBox->GetValue().c_str());
      wxString objName(GetObjectSelection().c_str());
      wxString propName(GetPropertySelection().c_str());
      wxString depObjName = wxT("");
      
      if (mCoordSysComboBox->IsShown())
         depObjName = wxString(mCoordSysComboBox->GetValue().c_str());
      else if (mCentralBodyComboBox->IsShown())
         depObjName = wxString(mCentralBodyComboBox->GetValue().c_str());
      
      param = theGuiInterpreter->CreateParameter(propName, paramName);
      
      if (objTypeName == wxT("Spacecraft"))
         param->SetRefObjectName(Gmat::SPACECRAFT, objName);
      else if (objTypeName == wxT("ImpulsiveBurn"))
         param->SetRefObjectName(Gmat::IMPULSIVE_BURN, objName);
      else
         MessageInterface::PopupMessage
            (Gmat::WARNING_, wxT("*** WARNING *** %s is not a valid object for property %s\n")
             wxT("There will be no report generated for this parameter.\n"), objTypeName.c_str(),
             propName.c_str());
      
      if (depObjName != wxT(""))
         param->SetStringParameter(wxT("DepObject"), depObjName);
      
      if (param->IsCoordSysDependent())
         param->SetRefObjectName(Gmat::COORDINATE_SYSTEM, depObjName);
   }
   
   return param;
}


