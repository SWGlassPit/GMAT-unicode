//$Id$
//------------------------------------------------------------------------------
//                                 Create
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Wendy C. Shoan
// Created: 2008.03.14
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CCA54C
//
/**
 * Class declaration for the Create command
 */
//------------------------------------------------------------------------------


#ifndef Create_hpp
#define Create_hpp

#include "ManageObject.hpp"


/**
 * Declaration of the Create command
 */
class GMAT_API Create : public ManageObject
{
public:
   Create();
   virtual          ~Create();
   Create(const Create &cr);
   Create&        operator=(const Create &cr);
   
   // Parameter access methods - overridden from GmatBase
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value);
   virtual bool         SetStringParameter(const wxString &label, 
                                           const wxString &value);
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name = wxT(""));
   virtual GmatBase*    Clone() const;
   
   bool                 Initialize();
   bool                 Execute();
   virtual void         RunComplete();

protected:
   enum
   {
      OBJECT_TYPE = ManageObjectParamCount, 
      CreateParamCount
   };
   static const wxString PARAMETER_TEXT[CreateParamCount - ManageObjectParamCount];
   
   static const Gmat::ParameterType PARAMETER_TYPE[CreateParamCount - ManageObjectParamCount];
   
   wxString  objType;
   GmatBase     *refObj;
   /// object names and sizes  when the object type is an Array
   StringArray  arrayNames;
   IntegerArray rows;
   IntegerArray columns;
   
   void SetArrayInfo();
   bool InsertIntoLOS(GmatBase *obj, const wxString &withName);
   bool InsertIntoObjectStore(GmatBase *obj, const wxString &withName);
};

#endif /* Create_hpp */
