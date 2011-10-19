//$Id: StateVector.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                               StateVector
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
// Author:  Joey Gurganus
// Created: 2005/06/02
//
/**
 * Definition of the StateVector class
 */
//------------------------------------------------------------------------------

#ifndef StateVector_hpp
#define StateVector_hpp

#include "gmatdefs.hpp"
#include "Rvector6.hpp"
#include "StateConverter.hpp"
#include "Anomaly.hpp"
#include "CoordinateSystem.hpp"

class GMAT_API StateVector
{
public:
   // Default constructor
   StateVector();
   StateVector(const wxString &type);
   StateVector(const Rvector6 stateVector);
   StateVector(const wxString &type, const Rvector6 stateVector);

   // Copy constructor
   StateVector(const StateVector &sv);
   // Assignment operator
   StateVector& operator=(const StateVector &sv);

   // Destructor
   virtual ~StateVector(void);

   // public methods
   Rvector6    GetValue() const;
   Rvector6    GetValue(const wxString &type) const;
   bool        SetValue(const wxString &type);
   bool        SetValue(Rvector6 state);
   bool        SetValue(const wxString &type, Rvector6 state);

   Real        GetElement(const Integer id) const;
   Real        GetElement(const wxString &label) const;
   bool        SetElement(const Integer id, const Real value);
   bool        SetElement(const wxString &label, const Real value); 

   wxString GetType() const;
   bool        SetType(const wxString &type);

   bool        IsValidType(const wxString &label) const;

   bool        IsElement(const Integer id, const wxString &label) const;
   bool        IsElement(const wxString &label) const;
   wxString GetLabel(const Integer id) const;

   bool        SetAnomaly(const Rvector6 kepl,const wxString &type);
   wxString GetAnomalyType() const;
   bool        SetAnomalyType(const wxString &type);

   bool        SetCoordSys(const CoordinateSystem *cs);

protected:
   // StateVector's element list
   enum StateType 
   {
        CARTESIAN, KEPLERIAN, MODIFIED_KEPLERIAN,
        SPHERICAL_AZFPA, SPHERICAL_RADEC, StateTypeCount
   };
   enum ElementType 
   {
        ELEMENT1, ELEMENT2, ELEMENT3, ELEMENT4, ELEMENT5, ELEMENT6,
        EXTRA_ELEMENT1, EXTRA_ELEMENT2, ElementTypeCount
   };
          
   static const wxString ELEMENT_LIST[StateTypeCount][ElementTypeCount];
   static const wxString STATE_LIST[StateTypeCount];
   
private:
   wxString             mStateType;
   Rvector6                mState;
   Anomaly                 mAnomaly;
   mutable StateConverter  mStateConverter;

   // private methods
   void        DefineDefault();
   void        InitializeDataMethod(const StateVector &s);
   wxString FindType(const wxString &label) const;
   UnsignedInt GetElementID(const wxString &label) const;

};

#endif // StateVector_hpp
