//$Id: OnOffWrapper.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  OnOffWrapper
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. 
//
// Author: Linda Jun/GSFC
// Created: 2007/07/24
//
/**
 * Declares OnOffWrapper class.
 */
//------------------------------------------------------------------------------
#ifndef OnOffWrapper_hpp
#define OnOffWrapper_hpp

#include "gmatdefs.hpp"
#include "ElementWrapper.hpp"

class GMAT_API OnOffWrapper : public ElementWrapper
{
public:

   OnOffWrapper();
   OnOffWrapper(const OnOffWrapper &copy);
   const OnOffWrapper& operator=(const OnOffWrapper &right);
   virtual ~OnOffWrapper();
   
   virtual Gmat::ParameterType GetDataType() const;
   
   virtual Real         EvaluateReal() const;
   virtual bool         SetReal(const Real val);
   
   virtual wxString  EvaluateOnOff() const;
   virtual bool         SetOnOff(const wxString &val);
   
protected:  

   // the string value of wxT("On") or wxT("Off")
   wxString value;
   
   virtual void         SetupWrapper();
};
#endif // OnOffWrapper_hpp
