//$Id: ManageObject.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                 ManageObject
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Wendy C. Shoan
// Created: 2008.03.12
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CCA54C
//
/**
 * Class declaration for the ManageObject command- this is a base class for the 
 * Create and GLobal commands
 */
//------------------------------------------------------------------------------


#ifndef ManageObject_hpp
#define ManageObject_hpp

#include "GmatCommand.hpp"


/**
 * Base class for Create and Global commands
 */
class GMAT_API ManageObject : public GmatCommand
{
public:
   ManageObject(const wxString &typeStr);
   virtual          ~ManageObject();
   ManageObject(const ManageObject &mo);
   ManageObject&        operator=(const ManageObject &mo);
   
   // Parameter access methods - overridden from GmatBase
   virtual wxString    GetParameterText(const Integer id) const;
   virtual Integer        GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                          GetParameterType(const Integer id) const;
   virtual wxString    GetParameterTypeString(const Integer id) const;

   virtual wxString  GetStringParameter(const Integer id) const;
   virtual wxString  GetStringParameter(const Integer id,
                                           const Integer index) const;
   virtual wxString  GetStringParameter(const wxString &label,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value);
   virtual bool         SetStringParameter(const wxString &label, 
                                           const wxString &value);
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value,
                                           const Integer index);
   virtual bool         SetStringParameter(const wxString &label, 
                                           const wxString &value,
                                           const Integer index);
   virtual const StringArray& 
                        GetStringArrayParameter(const Integer id) const;

   bool                 Initialize();

protected:
   enum
   {
      OBJECT_NAMES = GmatCommandParamCount, 
      ManageObjectParamCount
   };
   static const wxString PARAMETER_TEXT[ManageObjectParamCount - GmatCommandParamCount];

   static const Gmat::ParameterType PARAMETER_TYPE[ManageObjectParamCount - GmatCommandParamCount];
  
   /// array of names for the objects to manage
   StringArray objectNames;
   
   bool InsertIntoGOS(GmatBase *obj, const wxString &withName);

};

#endif /* ManageObject_hpp */
