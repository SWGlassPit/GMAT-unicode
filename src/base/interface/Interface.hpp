//$Id$
//------------------------------------------------------------------------------
//                                  Interface
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Linda Jun/GSFC
// Created: 2010/04/02
//
/**
 * Declaration of Interface class.
 */
//------------------------------------------------------------------------------
#ifndef Interface_hpp
#define Interface_hpp

#include "GmatBase.hpp" // inheriting class's header file

class GMAT_API Interface : public GmatBase
{
public:
   Interface(const wxString &type, const wxString &name);
   Interface(const Interface &interf);
   virtual ~Interface();
   
   virtual Integer      Open(const wxString &name = wxT(""));
   virtual Integer      Close(const wxString &name = wxT(""));
   
   // Parameter IDs
   enum
   {
      InterfaceParamCount = GmatBaseParamCount
   };
   
protected:
   
};

#endif // Interface_hpp

