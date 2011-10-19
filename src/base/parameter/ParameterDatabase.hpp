//$Id: ParameterDatabase.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                ParameterDatabase
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Linda Jun
// Created: 2003/09/18
//
/**
 * Declares parameter database class.
 */
//------------------------------------------------------------------------------
#ifndef ParameterDatabase_hpp
#define ParameterDatabase_hpp

#include "gmatdefs.hpp"
#include "paramdefs.hpp"
#include "Parameter.hpp"

class GMAT_API ParameterDatabase
{

public:
   
   ParameterDatabase();
   ParameterDatabase(const ParameterDatabase &copy);
   ParameterDatabase& operator=(const ParameterDatabase &right);
   virtual ~ParameterDatabase();
   
   Integer GetNumParameters() const;
   const StringArray& GetNamesOfParameters();
   ParameterPtrArray GetParameters() const;
   
   bool HasParameter(const wxString &name) const;
   bool RenameParameter(const wxString &oldName, const wxString &newName);
   Integer GetParameterCount(const wxString &name) const;
   
   Parameter* GetParameter(const wxString &name) const;
   wxString GetFirstParameterName() const;
   bool SetParameter(const wxString &name, Parameter *param);
   
   void Add(const wxString &name, Parameter *param = NULL);
   void Add(Parameter *param);
   void Remove(const wxString &name);
   void Remove(const Parameter *param);
   
protected:
private:

   StringParamPtrMap *mStringParamPtrMap;
   StringArray mParamNames;
   Integer mNumParams;

};
#endif // ParameterDatabase_hpp

