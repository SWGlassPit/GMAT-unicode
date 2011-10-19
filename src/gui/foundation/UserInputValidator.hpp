//$Id$
//------------------------------------------------------------------------------
//                              UserInputValidator
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Linda Jun
// Created: 2004/02/02
//
/** Validates user input entered via the GUI.
 * Declares UserInputValidator class.
 */
//------------------------------------------------------------------------------

#ifndef UserInputValidator_hpp
#define UserInputValidator_hpp

#include "gmatwxdefs.hpp"
#include "GuiItemManager.hpp"
#include "GmatBase.hpp"

class UserInputValidator
{
public:
   
   // constructors
   UserInputValidator();
   ~UserInputValidator();
   
   void SetObject(GmatBase *obj);
   void SetGuiManager(GuiItemManager *manager);
   void SetWindow(wxWindow *window);
   bool IsInputValid();
   
   bool IsValidName(const wxString &name);
   bool CheckFileName(const wxString &str, const wxString &field,
                      bool onlyMsg = false);
   
   bool CheckReal(Real &rvalue, const wxString &str,
                  const wxString &field, const wxString &expRange,
                  bool onlyMsg = false, bool checkRange = false, 
                  bool positive = false, bool zeroOk = false);
   
   bool CheckInteger(Integer &ivalue, const wxString &str,
                     const wxString &field, const wxString &expRange,
                     bool onlyMsg = false, bool checkRange = false,
                     bool positive = false, bool zeroOk = false);
   
   bool CheckIntegerRange(Integer &ivalue, const wxString &str, 
                          const wxString &field,
                          Integer lower, Integer upper,
                          bool checkLower = true, bool checkUpper = true,
                          bool includeLower = false,
                          bool includeUpper = false);
   
   bool CheckVariable(const wxString &varName, Gmat::ObjectType ownerType,
                      const wxString &field, const wxString &expRange,
                      bool allowNumber = true, bool allowNonPlottable = false);
   
   bool CheckRealRange(const wxString &sValue, Real value, const wxString &field,
                       Real lower, Real upper,
                       bool checkLower = true, bool checkUpper = true,
                       bool includeLower = false,
                       bool includeUpper = false, bool isInteger = false);
   
   bool CheckTimeFormatAndValue(const wxString &format, const wxString& value,
                                const wxString &field,  bool checkRange = false);

   wxArrayString ToWxArrayString(const StringArray &array);
   wxString ToWxString(const wxArrayString &names);
   
protected:
   static wxString lessOrEq;
   static wxString lessThan;
   static wxString moreOrEq;
   static wxString moreThan;
   
   GmatBase       *mObject;
   GuiItemManager *mGuiManager;
   wxWindow       *mWindow;
   bool           mIsInputValid;;
   wxString    mObjectName;
   wxString    mMsgFormat;
   
   void SetErrorFlag();
   
};

#endif // UserInputValidator_hpp
