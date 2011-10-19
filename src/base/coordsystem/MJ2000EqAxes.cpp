//$Id: MJ2000EqAxes.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  MJ2000EqAxes
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under 
// MOMS Task order 124.
//
// Author: Wendy C. Shoan
// Created: 2005/01/05
//
/**
 * Implementation of the MJ2000EqAxes class.  
 *
 */
//------------------------------------------------------------------------------

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "MJ2000EqAxes.hpp"
#include "InertialAxes.hpp"

//#define DEBUG_CONSTRUCTION

#ifdef DEBUG_CONSTRUCTION
   #include "MessageInterface.hpp"
#endif

//---------------------------------
// static data
//---------------------------------

/* placeholder - may be needed later
const wxString
MJ2000EqAxes::PARAMETER_TEXT[MJ2000EqAxesParamCount - InertialAxesParamCount] =
{
   "",
};

const Gmat::ParameterType
MJ2000EqAxes::PARAMETER_TYPE[MJ2000EqAxesParamCount - InertialAxesParamCount] =
{
};
*/

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  MJ2000EqAxes(const wxString &itsType,
//               const wxString &itsName);
//---------------------------------------------------------------------------
/**
 * Constructs base MJ2000EqAxes structures
 * (default constructor).
 *
 * @param itsType GMAT script string associated with this type of object.
 * @param itsName Optional name for the object.  Defaults to "".
 *
 */
//---------------------------------------------------------------------------
MJ2000EqAxes::MJ2000EqAxes(const wxString &itsName) :
InertialAxes(wxT("MJ2000Eq"),itsName)
{
   objectTypeNames.push_back(wxT("MJ2000EqAxes"));
   parameterCount = MJ2000EqAxesParamCount;
   #ifdef DEBUG_CONSTRUCTION
      MessageInterface::ShowMessage(wxT("Now constructing MJ2000Eq with name %s\n"),
         itsName.c_str());
   #endif
}

//---------------------------------------------------------------------------
//  MJ2000EqAxes(const MJ2000EqAxes &mj2000);
//---------------------------------------------------------------------------
/**
 * Constructs base MJ2000EqAxes structures used in derived classes, by copying 
 * the input instance (copy constructor).
 *
 * @param mj2000  MJ2000EqAxes instance to copy to create "this" instance.
 */
//---------------------------------------------------------------------------
MJ2000EqAxes::MJ2000EqAxes(const MJ2000EqAxes &mj2000) :
InertialAxes(mj2000)
{
}

//---------------------------------------------------------------------------
//  MJ2000EqAxes& operator=(const MJ2000EqAxes &mj2000)
//---------------------------------------------------------------------------
/**
 * Assignment operator for MJ2000EqAxes structures.
 *
 * @param mj2000 The original that is being copied.
 *
 * @return Reference to this object
 */
//---------------------------------------------------------------------------
const MJ2000EqAxes& MJ2000EqAxes::operator=(const MJ2000EqAxes &mj2000)
{
   if (&mj2000 == this)
      return *this;
   InertialAxes::operator=(mj2000);   
   return *this;
}
//---------------------------------------------------------------------------
//  ~MJ2000EqAxes(void)
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
//---------------------------------------------------------------------------
MJ2000EqAxes::~MJ2000EqAxes()
{
}

//---------------------------------------------------------------------------
//  bool MJ2000EqAxes::Initialize()
//---------------------------------------------------------------------------
/**
 * Initialization method for this MJ2000EqAxes.
 *
 */
//---------------------------------------------------------------------------
bool MJ2000EqAxes::Initialize()
{
   #ifdef DEBUG_CONSTRUCTION
      MessageInterface::ShowMessage(wxT("Now entering MJ2000Eq INIT with name %s\n"),
         instanceName.c_str());
   #endif
   InertialAxes::Initialize();
   // initialize the rotation matrix to be the identity matrix
   rotMatrix(0,0) = 1.0;
   rotMatrix(1,1) = 1.0;
   rotMatrix(2,2) = 1.0;
   // rotDotMatrix is still the default zero matrix
   #ifdef DEBUG_CONSTRUCTION
      MessageInterface::ShowMessage(wxT("Now LEAVING MJ2000Eq INIT with name %s\n"),
         instanceName.c_str());
   #endif
   
   return true;
}

//------------------------------------------------------------------------------
// public methods inherited from GmatBase
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the Planet.
 *
 * @return clone of the Planet.
 *
 */
//------------------------------------------------------------------------------
GmatBase* MJ2000EqAxes::Clone() const
{
   return (new MJ2000EqAxes(*this));
}

//------------------------------------------------------------------------------
//  wxString  GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter text, given the input parameter ID.
 *
 * @param id Id for the requested parameter text.
 *
 * @return parameter text for the requested parameter.
 *
 */
//------------------------------------------------------------------------------
/*wxString MJ2000EqAxes::GetParameterText(const Integer id) const
{
   if (id >= InertialAxesParamCount && id < MJ2000EqAxesParamCount)
      return PARAMETER_TEXT[id - InertialAxesParamCount];
   return InertialAxes::GetParameterText(id);
}
*/
//------------------------------------------------------------------------------
//  Integer  GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter ID, given the input parameter string.
 *
 * @param str string for the requested parameter.
 *
 * @return ID for the requested parameter.
 *
 */
//------------------------------------------------------------------------------
/*Integer MJ2000EqAxes::GetParameterID(const wxString &str) const
{
   for (Integer i = InertialAxesParamCount; i < MJ2000EqAxesParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - InertialAxesParamCount])
         return i;
   }
   
   return InertialAxes::GetParameterID(str);
}
*/
//------------------------------------------------------------------------------
//  Gmat::ParameterType  GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type, given the input parameter ID.
 *
 * @param id ID for the requested parameter.
 *
 * @return parameter type of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
/*Gmat::ParameterType MJ2000EqAxes::GetParameterType(const Integer id) const
{
   if (id >= InertialAxesParamCount && id < MJ2000EqAxesParamCount)
      return PARAMETER_TYPE[id - InertialAxesParamCount];
   
   return InertialAxes::GetParameterType(id);
}
*/
//------------------------------------------------------------------------------
//  wxString  GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type string, given the input parameter ID.
 *
 * @param id ID for the requested parameter.
 *
 * @return parameter type string of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
/*wxString MJ2000EqAxes::GetParameterTypeString(const Integer id) const
{
   return InertialAxes::PARAM_TYPE_STRING[GetParameterType(id)];
}
*/


//------------------------------------------------------------------------------
// protected methods
//------------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  void CalculateRotationMatrix(const A1Mjd &atEpoch,
//                               bool forceComputation = false)
//---------------------------------------------------------------------------
/**
 * This method will compute the rotMatrix and rotDotMatrix used for rotations
 * from/to this AxisSystem to/from the MJ2000EqAxes system.
 *
 * @param atEpoch  epoch at which to compute the roration matrix
 */
//---------------------------------------------------------------------------
void MJ2000EqAxes::CalculateRotationMatrix(const A1Mjd &atEpoch,
                                           bool forceComputation)
{
   // already computed in Initialize;
}
