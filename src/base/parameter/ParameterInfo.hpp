//$Id: ParameterInfo.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                ParameterInfo
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
// Created: 2005/01/24
//
/**
 * Declares parameter info class.
 */
//------------------------------------------------------------------------------
#ifndef ParameterInfo_hpp
#define ParameterInfo_hpp

#include "gmatdefs.hpp"
#include "Parameter.hpp"
#include <map>

class GMAT_API ParameterInfo
{

public:

   static ParameterInfo* Instance();
   
   Integer GetNumParameters() const;
   const StringArray& GetTypesOfParameters();
   const StringArray& GetNamesOfParameters();
   Gmat::ObjectType GetObjectType(const wxString &type);
   GmatParam::DepObject GetDepObjectType(const wxString &name);
   bool IsPlottable(const wxString &type);
   bool IsReportable(const wxString &type);
   bool IsSettable(const wxString &type);
   
   void Add(const wxString &type, Gmat::ObjectType objectType,
            const wxString &name, GmatParam::DepObject depType,
            bool isPlottable, bool isReportable, bool isSettable);
   void Remove(const wxString &name);
   
protected:
private:
   
   static ParameterInfo *theInstance;
   
   std::map<wxString, GmatParam::DepObject> mParamDepObjMap;
   std::map<wxString, Gmat::ObjectType> mParamObjectTypeMap;
   std::map<wxString, bool> mParamPlottableMap;
   std::map<wxString, bool> mParamReportableMap;
   std::map<wxString, bool> mParamSettableMap;
   StringArray mParamTypes;
   StringArray mParamNames;
   Integer mNumParams;
   
   ParameterInfo();
   ~ParameterInfo();
   
};
#endif // ParameterInfo_hpp

