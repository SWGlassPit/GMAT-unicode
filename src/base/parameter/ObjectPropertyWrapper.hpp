//$Id: ObjectPropertyWrapper.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                          ObjectPropertyWrapper
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P
//
// Author: Wendy C. Shoan/GSFC/MAB
// Created: 2007.01.18
//
/**
 * Definition of the ObjectPropertyWrapper class.
 *
 *
 */
//------------------------------------------------------------------------------

#ifndef ObjectPropertyWrapper_hpp
#define ObjectPropertyWrapper_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "ElementWrapper.hpp"

class GMAT_API ObjectPropertyWrapper : public ElementWrapper
{
public:

   // default constructor
   ObjectPropertyWrapper();
   // copy constructor
   ObjectPropertyWrapper(const ObjectPropertyWrapper &opw);
   // operator = 
   const ObjectPropertyWrapper& operator=(const ObjectPropertyWrapper &opw);
   // destructor
   virtual ~ObjectPropertyWrapper();
   
   virtual Gmat::ParameterType GetDataType() const;
   
   virtual const StringArray&  GetRefObjectNames();
   virtual bool                SetRefObjectName(const wxString &name,
                                            Integer index);
   virtual GmatBase*           GetRefObject(const wxString &name = wxT(""));
   virtual bool                SetRefObject(GmatBase *obj);
   virtual bool                RenameObject(const wxString &oldName, 
                                        const wxString &newName);
   
   virtual Real                EvaluateReal() const;
   virtual bool                SetReal(const Real toValue);
   virtual wxString         EvaluateString() const;
   virtual bool                SetString(const wxString &toValue); 
   virtual wxString         EvaluateOnOff() const;
   virtual bool                SetOnOff(const wxString &toValue);
   virtual bool                EvaluateBoolean() const;
   virtual bool                SetBoolean(const bool toValue); 
   virtual Integer             EvaluateInteger() const;
   virtual bool                SetInteger(const Integer toValue); 
   virtual bool                SetObject(GmatBase* obj);
   
   const Integer               GetPropertyId();

   virtual bool                TakeRequiredAction() const;

protected:  

   /// pointer to the base object
   GmatBase    *object;
   /// array of property ID names
   StringArray propIDNames;
   /// parameter Id for the property of the object
   Integer     propID;
   /// owned object name
   wxString ownedObjName;
   
   virtual void            SetupWrapper(); 
   
};
#endif // ObjectPropertyWrapper_hpp
