//$Id: EventFunction.hpp 9853 2011-09-09 20:08:55Z djcinsb $
//------------------------------------------------------------------------------
//                           EventFunction
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under NASA Prime
// Contract NNG10CP02C, Task Order 28
//
// Author: Darrel J. Conway, Thinking Systems, Inc.
// Created: Jul 6, 2011
//
/**
 * Definition of the ...
 */
//------------------------------------------------------------------------------


#ifndef EventFunction_hpp
#define EventFunction_hpp

#include "GmatBase.hpp"


class SpaceObject;


class GMAT_API EventFunction //: public GmatBase
{
public:
   EventFunction(const wxString &typeStr);
   virtual ~EventFunction();
   EventFunction(const EventFunction& ef);
   EventFunction& operator=(const EventFunction& ef);

   // Methods used to identify the EventFunction
   wxString GetTypeName();
   wxString GetName();

   bool SetPrimary(SpaceObject *so);
   virtual bool Initialize();
   virtual Real* Evaluate() = 0;

protected:
   /// Event function type
   wxString       typeName;
   /// Event function descriptor
   wxString       instanceName;
   /// Current/most recently evaluated epoch, event value, derivative
   Real              eventData[3];
   /// SpaceObject that plays the role of “target” in the event computations.
   SpaceObject       *primary;
};

#endif /* EventFunction_hpp */
