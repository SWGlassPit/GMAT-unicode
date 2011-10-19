//$Id: StateVector.cpp 9513 2011-04-30 21:23:06Z djcinsb $
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
 * Implements the StateVector class
 */
//------------------------------------------------------------------------------

#include "StateVector.hpp"
#include "StateVectorException.hpp"
#include "UtilityException.hpp"
#include "MessageInterface.hpp"

// #define DEBUG_STATEVECTOR 1 

// StateVector constant variables for the lists of state types and elements 
const wxString StateVector::STATE_LIST[StateTypeCount] =
   {
      wxT("Cartesian"), wxT("Keplerian"), wxT("ModifiedKeplerian"), 
      wxT("SphericalAZFPA"), wxT("SphericalRADEC")
   };

const wxString StateVector::ELEMENT_LIST[StateTypeCount][ElementTypeCount] =
   {
      {wxT("X"), wxT("Y"), wxT("Z"), wxT("VX"), wxT("VY"), wxT("VZ"),wxT(""),wxT("")},
      {wxT("SMA"), wxT("ECC"), wxT("INC"), wxT("RAAN"), wxT("AOP"), wxT("TA"), wxT("MA"), wxT("EA")},
      {wxT("RadPer"), wxT("RadApo"), wxT("INC"), wxT("RAAN"), wxT("AOP"), wxT("TA"), wxT("MA"), wxT("EA")},
      {wxT("RMAG"), wxT("RA"), wxT("DEC"), wxT("VMAG"), wxT("AZI"), wxT("FPA"), wxT(""), wxT("")},
      {wxT("RMAG"), wxT("RA"), wxT("DEC"), wxT("VMAG"), wxT("RAV"), wxT("DECV"), wxT(""), wxT("")}
   };

//-------------------------------------
// public methods
//-------------------------------------

//---------------------------------------------------------------------------
//  StateVector()
//---------------------------------------------------------------------------
/**
 * Creates default constructor.
 */
//---------------------------------------------------------------------------
StateVector::StateVector()
{
   DefineDefault();
}


//---------------------------------------------------------------------------
//  StateVector(const wxString &type)
//---------------------------------------------------------------------------
/**
 * Creates constructors with parameters.
 *
 * @param <type> state type
 *
 */
//---------------------------------------------------------------------------
StateVector::StateVector(const wxString &type)
{
   DefineDefault();
    
   // Check if invalid then use default
   if (!SetValue(type))
      MessageInterface::ShowMessage(wxT("\n****Warning: Invalid state type ***")
                                    wxT("\nUse default state values.\n"));  
}


//---------------------------------------------------------------------------
//  StateVector(const Rvector6 stateVector)
//---------------------------------------------------------------------------
/**
 * Creates constructors with parameters.
 *
 * @param <stateVector> state's values
 *
 */
//---------------------------------------------------------------------------
StateVector::StateVector(const Rvector6 stateVector)
{
   DefineDefault();
   mState = stateVector;
}


//---------------------------------------------------------------------------
//  StateVector(const wxString &type, const Rvector6 stateVector)
//---------------------------------------------------------------------------
/**
 * Creates constructors with parameters.
 *
 * @param <type> state's type 
 * @param <stateVector> state's values
 *
 */
//---------------------------------------------------------------------------
StateVector::StateVector(const wxString &type, const Rvector6 stateVector)
{
   DefineDefault();

   // Check for invalid state type
   if (!SetValue(type, stateVector))
      MessageInterface::ShowMessage(wxT("\n****Warning: Invalid state type ***")
                                    wxT("\nUse default state values.\n"));  
}


//---------------------------------------------------------------------------
//  StateVector(const StateVector &sv)
//---------------------------------------------------------------------------
/**
 * Copy Constructor for base StateVector structures.
 *
 * @param <sv> The original that is being copied.
 */
//---------------------------------------------------------------------------
StateVector::StateVector(const StateVector &sv)
{
    InitializeDataMethod(sv);
}


//---------------------------------------------------------------------------
//  ~StateVector(void)
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
//---------------------------------------------------------------------------
StateVector::~StateVector(void)
{
}


//---------------------------------------------------------------------------
//  StateVector& operator=(const StateVector &sv)
//---------------------------------------------------------------------------
/**
 * Assignment operator for StateVector structures.
 *
 * @param <sv> The original that is being copied.
 *
 * @return Reference to this object
 */
//---------------------------------------------------------------------------
StateVector& StateVector::operator=(const StateVector &sv)
{
   if (&sv == this)
      return *this;

   // Duplicate the member data        
   InitializeDataMethod(sv);

   return *this;
}


//---------------------------------------------------------------------------
//  Rvector6 GetValue() const
//---------------------------------------------------------------------------
/**
 * Retrieve the value.
 *
 * @return the state's value.
 */
//---------------------------------------------------------------------------
Rvector6 StateVector::GetValue() const
{
   return mState;
}


//---------------------------------------------------------------------------
//  Rvector6 GetValue(const wxString &type) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value with the specific state type.
 *
 * @return the state's value.
 */
//---------------------------------------------------------------------------
Rvector6 StateVector::GetValue(const wxString &type) const
{
   return (mStateConverter.Convert(mState, mStateType, type, (Anomaly&)mAnomaly));
}


//---------------------------------------------------------------------------
//  bool SetValue(const wxString &type)
//---------------------------------------------------------------------------
/**
 * Set the value with the specific state type.
 *
 * @param <type>  state type 
 *
 * @return true when successful; otherwise, false.
 */
//---------------------------------------------------------------------------
bool StateVector::SetValue(const wxString &type) 
{
   if (!IsValidType(type))
      return false; 
   
   if (mStateType != type)
   {
       try
       {
          mState = mStateConverter.Convert(mState, mStateType, type, mAnomaly);
          mStateType = type;
       }
       catch(UtilityException &)
       {
          return false; 
       }
   }
   
   return true;
}


//---------------------------------------------------------------------------
//  bool SetValue(const Rvector6 state)
//---------------------------------------------------------------------------
/**
 * Set the value with the specific state type.
 *
 * @param <state>  state value
 *
 * @return true when successful; otherwise, false.
 */
//---------------------------------------------------------------------------
bool StateVector::SetValue(const Rvector6 state) 
{
   mState = state;
   
   // @todo:  need to make sure with anomaly and others
   return true;
}


//---------------------------------------------------------------------------
//  bool SetValue(const wxString &type,const Rvector6 state)
//---------------------------------------------------------------------------
/**
 * Set the value with the specific state type.
 *
 * @param <type>  state type 
 * @param <state>  state value
 *
 * @return true when successful; otherwise, false.
 */
//---------------------------------------------------------------------------
bool StateVector::SetValue(const wxString &type, const Rvector6 state) 
{
   if (IsValidType(type))
      return false;

   mStateType = type;
   mState = state;
   
   // @todo:  need to make sure with anomaly and others
   return true;
}


//---------------------------------------------------------------------------
//  Real GetElement(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for element.
 *
 * @param <id> The element ID.
 *
 * @return the element's value.
 */
//---------------------------------------------------------------------------
Real StateVector::GetElement(const Integer id) const
{
   #if DEBUG_STATEVECTOR   
   MessageInterface::ShowMessage(wxT("\n*** StateVector::GetElement(%d) ****\n"),id);
   #endif

   // Check for out of the range then throw exception
   if (id < 1 || id > 6)
      throw StateVectorException(wxT("StateVector::GetElement - out of range"));
   
   return mState[id - 1];    
}


//---------------------------------------------------------------------------
//  Real GetElement(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for element.
 *
 * @param <label> The label of the element.
 *
 * @return the element's value.
 */
//---------------------------------------------------------------------------
Real StateVector::GetElement(const wxString &label) const
{
   // Find the state type
   wxString findType = FindType(label);

   #if DEBUG_STATEVECTOR   
   MessageInterface::ShowMessage(wxT("\n*** StateVector::GetElement(%s), findType ")
                                 wxT("= %s\n"),label.c_str(), findType.c_str());
   #endif
   
   if (findType == wxT("NoFound"))
   {
      MessageInterface::ShowMessage(wxT("\nNo found due to invalid label.\n"));
      throw StateVectorException(wxT("\nNo found due to invalid label.\n"));
   }
   
   // First check with anomaly -> @todo: also need move into element1-6 below
   if (!mAnomaly.IsInvalid(label))
      return mAnomaly.GetValue(label);
   
   UnsignedInt id = GetElementID(label);
   
   if (mStateType == findType)
      return mState[id];
   
   // Do the conversion
   Rvector6 tempState = mStateConverter.Convert(mState, mStateType, findType,
                                                (Anomaly&)mAnomaly);
   
   return tempState[id];
}


//---------------------------------------------------------------------------
//  bool SetElement(const Integer id, const Real value)
//---------------------------------------------------------------------------
/**
 * Set the value for element.
 *
 * @param <id> The integer ID for the element.
 * @param <value> The new element value.
 *
 * @return true if successful; otherwise, false
 */
//---------------------------------------------------------------------------
bool StateVector::SetElement(const Integer id, const Real value)
{
   #if DEBUG_STATEVECTOR
   MessageInterface::ShowMessage(
       wxT("\n*** StateVector::SetElement(%d,%f) enters...\n\ttype = %s\n"), 
       id, value, type.c_str());
   #endif

   // Check for the coordinate representation then set the value
   if (id < 1 || id > 6)
      return false;

   mState[id - 1] = value;
   return true;
}


//---------------------------------------------------------------------------
//  void SetElement(const wxString &label, const Real value)
//---------------------------------------------------------------------------
/**
 * Set the value for element.
 *
 * @param <label> The label for the element.
 * @param <value> The new element value.
 *
 * @return true if successful; otherwise, false.
 */
//---------------------------------------------------------------------------
bool StateVector::SetElement(const wxString &label, const Real value)
{
   #if DEBUG_STATEVECTOR
   MessageInterface::ShowMessage(
       wxT("\n*** StateVector::SetElement(%s,%f) with type (%s) enters...\n"), 
       label.c_str(),value,type.c_str());
   #endif
   
   // Find the state type
   wxString findType = FindType(label);
   
   #if DEBUG_STATEVECTOR
   MessageInterface::ShowMessage(wxT("\nfindType = %s\n"), findType.c_str());
   #endif
   
   if (findType == wxT("NoFound"))
   {
       MessageInterface::ShowMessage(wxT("\nStateVector::SetElement(%s,%f), ")
                                     wxT("label(%s) has no found.\n"),
                                     label.c_str(), value, label.c_str());
       return false;
   }
   
   // Get the element id from the element's label
   UnsignedInt id = GetElementID(label);
   
   #if DEBUG_STATEVECTOR
   MessageInterface::ShowMessage(wxT("\nStateVector::SetElement..., id = %d\n"),id);
   #endif
   
   // if different type from current then do conversion
   if (findType != mStateType)
   {
      try
      {
         mState = mStateConverter.Convert(mState, mStateType, findType, mAnomaly);
         mStateType = findType;
      } 
      catch(UtilityException &)
      {
         return false;
      }  
   }
   
   mState[id] = value;
   
   return true;
}


//---------------------------------------------------------------------------
//  wxString GetType() const
//---------------------------------------------------------------------------
/**
 * Get the state type.
 *
 * @return the element's type. 
 */
//---------------------------------------------------------------------------
wxString StateVector::GetType() const
{
   return mStateType;
}


//---------------------------------------------------------------------------
//  bool SetType(const wxString &type) 
//---------------------------------------------------------------------------
/**
 * Set the state type.
 *
 * @param <type> Given element's type. 
 *
 * @return true if successful; otherwise, false.
 */
//---------------------------------------------------------------------------
bool StateVector::SetType(const wxString &type)
{
   if (!SetValue(type))
      return false; 
  
   return true; 
}


//---------------------------------------------------------------------------
//  wxString GetLabel(const Integer id)
//---------------------------------------------------------------------------
/**
 * Determines the label of the element ID. 
 *
 * @param <id>  element ID
 *
 * @return the element's label 
 */
//---------------------------------------------------------------------------
wxString StateVector::GetLabel(const Integer id) const
{
   #if DEBUG_STATEVECTOR
   MessageInterface::ShowMessage(wxT("\n*** StateVector::GetLabel(%d)\n"),id);
   #endif

   if (id < 1 || id > 6)
      throw StateVectorException(wxT("StateVector::GetElement - out of range"));
 
   // Check for Cartesian
   if (mStateType == STATE_LIST[0])
   {
      switch (id)
      {
         case 1:   return wxT("X");
         case 2:   return wxT("Y");
         case 3:   return wxT("Z");
         case 4:   return wxT("VX");
         case 5:   return wxT("VY");
         case 6:   return wxT("VZ");
      }
   }
   
   // Check for Keplerian and Modified Keplerian
   if (mStateType == STATE_LIST[1] || mStateType == STATE_LIST[2])
   {
      switch (id)
      {
         case 1:   if (mStateType == STATE_LIST[1]) return wxT("SMA");
                   return wxT("RadPer");
         case 2:   if (mStateType == STATE_LIST[1]) return wxT("ECC");
                   return wxT("RadApo");
         case 3:   return wxT("INC");
         case 4:   return wxT("RAAN");
         case 5:   return wxT("APO");
         case 6:   return mAnomaly.GetTypeString();
      }
   }

   // Check for Spherical with AZIFPA and RADEC
   if (mStateType == STATE_LIST[2] || mStateType == STATE_LIST[3])
   {
      switch (id)
      {
         case 1:   return wxT("RMAG"); 
         case 2:   return wxT("RA");
         case 3:   return wxT("DEC");
         case 4:   return wxT("VMAG");
         case 5:   if (mStateType == STATE_LIST[2]) return wxT("AZI");
                   return wxT("RAV");
         case 6:   if (mStateType == STATE_LIST[2]) return wxT("FPA");
                   return wxT("DECV");
      }
   }
   
   return wxT("");   // Won't happen unless the state type or element is no found
}


//---------------------------------------------------------------------------
//  bool IsElement(const Integer id, const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Determines if the ID is for the element. 
 *
 * @param <id>     element's ID 
 * @param <label>  element's label 
 *
 * @return true if it is a part of element; otherwise,false.
 */
//---------------------------------------------------------------------------
bool StateVector::IsElement(const Integer id, const wxString &label) const
{
   if (id < 1 || id > 6)
      return false;
 
   for (UnsignedInt i=0; i < StateTypeCount; i++)
   {
      if (ELEMENT_LIST[i][id-1] == label)
         return true;

      else if (id == 6 && (i == KEPLERIAN || i == MODIFIED_KEPLERIAN)) 
      {
         for (UnsignedInt temp = id; temp < ElementTypeCount; ++temp)
         {
            if (ELEMENT_LIST[i][temp] == label)
               return true;
         }
      }
   }

   return false;
}


//---------------------------------------------------------------------------
//  bool IsElement(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Determines if the label is for the element. 
 *
 * @param <label>  element's label
 *
 * @return true if it is a part of element; otherwise,false.
 */
//---------------------------------------------------------------------------
bool StateVector::IsElement(const wxString &label) const
{
   #if DEBUG_STATEVECTOR
   MessageInterface::ShowMessage(wxT("\nStateVector::IsElement(%s)\n"),
                                 label.c_str());
   #endif

   for (UnsignedInt i=0; i < StateTypeCount; i++)
   {
      for (UnsignedInt j=0; j < ElementTypeCount; j++)
      {
         if (ELEMENT_LIST[i][j] == label)
            return true;
      }
   }
   
   return false;
}


//---------------------------------------------------------------------------
//  bool SetAnomaly(const Rvector6 kepl,const wxString &type)
//---------------------------------------------------------------------------
/**
 * Set the anomaly. 
 *
 * @param <kepl>  Keplerian state
 * @param <type> anomaly type
 *
 * @return true if successful; otherwise,false.
 */
//---------------------------------------------------------------------------
bool StateVector::SetAnomaly(const Rvector6 kepl,const wxString &type)
{
   mAnomaly.Set(kepl[0], kepl[1], kepl[5], type);
  
   return true;
}


//---------------------------------------------------------------------------
//  wxString GetAnomalyType() const 
//---------------------------------------------------------------------------
/**
 * Get the anomaly type. 
 *
 * @return anomaly type.
 */
//---------------------------------------------------------------------------
wxString StateVector::GetAnomalyType() const
{
   return mAnomaly.GetTypeString();
}


//---------------------------------------------------------------------------
//  bool SetAnomalyType(const wxString &type) 
//---------------------------------------------------------------------------
/**
 * Set the anomaly type.
 *
 * @param <type> the anomaly type.
 *
 * @return true if successful; otherwise, false.
 */
//---------------------------------------------------------------------------
bool StateVector::SetAnomalyType(const wxString &type)
{
   try
   {
      mAnomaly.SetType(type);
   }
   catch(UtilityException &)
   {
      return false;
   }
   return true;
}


//---------------------------------------------------------------------------
//  bool IsValidType(const wxString &type) const
//---------------------------------------------------------------------------
/**
 * Check validity on the given state type. 
 *
 * @param <type> The state type.
 *
 * @return true if valid; otherwise, false.
 */
//---------------------------------------------------------------------------
bool StateVector::IsValidType(const wxString &type) const
{
   for (UnsignedInt i=0; i < 5; i++)
   {
       if (STATE_LIST[i] == type)
          return true;
   }
   return false;
}


//------------------------------------------------------------------------------
// bool SetCoordSys(const CoordinateSystem *cs)
//------------------------------------------------------------------------------
/**
 * Set the coordinate system for setting the Mu.
 *
 * @return true if successful; otherwise false
 */
//---------------------------------------------------------------------------
bool StateVector::SetCoordSys(const CoordinateSystem *cs)
{
   return mStateConverter.SetMu(cs);
}

//-------------------------------------
// private methods
//-------------------------------------

//------------------------------------------------------------------------------
// void DefineDefault()
//------------------------------------------------------------------------------
/**
 * Initialize data method as default. 
 */
//---------------------------------------------------------------------------
void StateVector::DefineDefault()
{
   mStateType = wxT("Cartesian");

   mState[0] = 7100.0;
   mState[1] = 0.0;
   mState[2] = 1300.0;
   mState[3] = 0.0;
   mState[4] = 7.35;
   mState[5] = 1.0;

   // Get the keplerian state and then initialize anomaly
   Rvector6 tempKepl = GetValue(wxT("Keplerian"));  
   mAnomaly.Set(tempKepl[0], tempKepl[1], tempKepl[5], wxT("TA"));
}


//---------------------------------------------------------------------------
//  void InitializeDataMethod(const StateVector &s)
//---------------------------------------------------------------------------
/**
 * Initialize data method from the parameter for copying spacecraft structures.
 *
 * @param <s> The original that is being copied.
 */
//---------------------------------------------------------------------------
void StateVector::InitializeDataMethod(const StateVector &s)
{
    // Duplicate the member data
    mStateType = s.mStateType;
    mAnomaly = s.mAnomaly;
    mStateConverter = s.mStateConverter;
    mState = s.mState;
}


//---------------------------------------------------------------------------
// wxString FindType(const wxString &label)
//---------------------------------------------------------------------------
/** 
 * Determine the state type from the given element.
 *     
 * @param <label> Given specific element.
 *     
 * @return Getting the state type from the given element.
 */    
//---------------------------------------------------------------------------
wxString StateVector::FindType(const wxString &label) const
{            
   #if DEBUG_STATEVECTOR
   MessageInterface::ShowMessage(wxT("\n*** StateVector::FindType-> label: %s"),
                                 label.c_str());
   #endif

   if (label == wxT("X") || label == wxT("Y") || label == wxT("Z") ||
       label == wxT("VX") || label == wxT("VY") || label == wxT("VZ"))
   {
      return STATE_LIST[0];
   }

   if (label == wxT("SMA") || label == wxT("ECC") || label == wxT("INC") || label == wxT("RAAN") ||
       label == wxT("AOP") || !(mAnomaly.IsInvalid(label)))
   {
      return STATE_LIST[1];
   }

   if (label == wxT("RadPer") || label == wxT("RadApo"))
      return STATE_LIST[2];

   if (label == wxT("RMAG") || label == wxT("RA") || label == wxT("DEC") || label == wxT("VMAG") ||
       label == wxT("AZI") || label == wxT("FPA"))
   {
      return STATE_LIST[3];
   }

   if (label == wxT("RAV") || label == wxT("DECV"))
      return STATE_LIST[4];

   return wxT("NoFound");
}


//---------------------------------------------------------------------------
// UnsignedInt GetElementID(const wxString &label) const
//---------------------------------------------------------------------------
/** 
 * Determine the state type from the given element.
 *     
 * @param <label> Given specific element.
 *     
 * @return the element ID.
 */    
//---------------------------------------------------------------------------
UnsignedInt StateVector::GetElementID(const wxString &label) const
{
   if (label == wxT("X") || label == wxT("SMA") || label == wxT("RadPer") || label == wxT("RMAG"))
      return 0;

   if (label == wxT("Y") || label == wxT("ECC") || label == wxT("RadApo") || label == wxT("RA"))
      return 1;

   if (label == wxT("Z") || label == wxT("INC") || label == wxT("DEC"))
      return 2;

   if (label == wxT("VX") || label == wxT("RAAN") || label == wxT("VMAG"))
      return 3;

   if (label == wxT("VY") || label == wxT("AOP") || label == wxT("AZI") || label == wxT("RAV"))
      return 4;

   if (label == wxT("VZ") || label == wxT("FPA") || label == wxT("DECV") || 
       !mAnomaly.IsInvalid(label))
      return 5;
   
   // Use default if no found
   return 0;
}
