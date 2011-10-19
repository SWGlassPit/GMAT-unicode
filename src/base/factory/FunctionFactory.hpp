//$Id: FunctionFactory.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              FunctionFactory
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P.
//
// Author: Allison Greene
// Created: 2004/9/22
//
/**
 * Implementation code for the FunctionFactory class, responsible for creating
 * Function objects.
 */
//------------------------------------------------------------------------------


#ifndef FunctionFactory_hpp
#define FunctionFactory_hpp

#include "Factory.hpp"
#include "Function.hpp"

class GMAT_API FunctionFactory : public Factory
{
public:
   Function* CreateFunction(const wxString &ofType, const wxString &withName = wxT(""));

   FunctionFactory();
   virtual ~FunctionFactory();
   FunctionFactory(StringArray createList);
   FunctionFactory(const FunctionFactory& fact);
   FunctionFactory& operator= (const FunctionFactory& fact);

};


#endif // FunctionFactory_hpp
