//$Id: SpacecraftData.cpp 9811 2011-08-29 21:15:18Z wendys-dev $
//------------------------------------------------------------------------------
//                                  SpacecraftData
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc.
//
// Author: Linda Jun
// Created: 2009.03.20
//
/**
 * Implements Spacecraft Spacecraft related data class.
 */
//------------------------------------------------------------------------------

#include "gmatdefs.hpp"
#include "SpacecraftData.hpp"
#include "ParameterException.hpp"
#include "StringUtil.hpp"          // ToString()
#include "GmatConstants.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_SPACECRAFTDATA_INIT
//#define DEBUG_SC_OWNED_OBJ


const wxString
SpacecraftData::VALID_OBJECT_TYPE_LIST[SpacecraftDataObjectCount] =
{
   wxT("Spacecraft")
}; 

const Real SpacecraftData::BALLISTIC_REAL_UNDEFINED = GmatRealConstants::REAL_UNDEFINED_LARGE;

//------------------------------------------------------------------------------
// SpacecraftData()
//------------------------------------------------------------------------------
SpacecraftData::SpacecraftData(const wxString &name)
   : RefData(name)
{
   mSpacecraft = NULL;
}


//------------------------------------------------------------------------------
// SpacecraftData(const SpacecraftData &copy)
//------------------------------------------------------------------------------
SpacecraftData::SpacecraftData(const SpacecraftData &copy)
   : RefData(copy)
{
   mSpacecraft = copy.mSpacecraft;
}


//------------------------------------------------------------------------------
// SpacecraftData& operator= (const SpacecraftData& right)
//------------------------------------------------------------------------------
SpacecraftData& SpacecraftData::operator= (const SpacecraftData& right)
{
   if (this == &right)
      return *this;
   
   RefData::operator=(right);
   
   mSpacecraft = right.mSpacecraft;

   return *this;
}


//------------------------------------------------------------------------------
// ~SpacecraftData()
//------------------------------------------------------------------------------
SpacecraftData::~SpacecraftData()
{
}


//------------------------------------------------------------------------------
// Real GetReal(Integer item)
//------------------------------------------------------------------------------
/**
 * Retrives Spacecraft or spacecraft owned hardware element by integer id.
 */
//------------------------------------------------------------------------------
Real SpacecraftData::GetReal(Integer item)
{
   if (mSpacecraft == NULL)
      InitializeRefObjects();
   
   switch (item)
   {
   case DRY_MASS:
      return mSpacecraft->GetRealParameter(wxT("DryMass"));
   case DRAG_COEFF:
      return mSpacecraft->GetRealParameter(wxT("Cd"));
   case REFLECT_COEFF:
      return mSpacecraft->GetRealParameter(wxT("Cr"));
   case DRAG_AREA:
      return mSpacecraft->GetRealParameter(wxT("DragArea"));
   case SRP_AREA:      
      return mSpacecraft->GetRealParameter(wxT("SRPArea"));
   case TOTAL_MASS:
      return mSpacecraft->GetRealParameter(wxT("TotalMass"));
      
   // for Spacecraft owned FuelTank
   case FUEL_MASS:
      return GetOwnedObjectProperty(Gmat::FUEL_TANK, wxT("FuelMass"));
   case PRESSURE:
      return GetOwnedObjectProperty(Gmat::FUEL_TANK, wxT("Pressure"));
   case TEMPERATURE:
      return GetOwnedObjectProperty(Gmat::FUEL_TANK, wxT("Temperature"));
   case REF_TEMPERATURE:
      return GetOwnedObjectProperty(Gmat::FUEL_TANK, wxT("RefTemperature"));
   case VOLUME:
      return GetOwnedObjectProperty(Gmat::FUEL_TANK, wxT("Volume"));
   case FUEL_DENSITY:
      return GetOwnedObjectProperty(Gmat::FUEL_TANK, wxT("FuelDensity"));
      
   // for Spacecraft owned Thruster
   case DUTY_CYCLE:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("DutyCycle"));
   case THRUSTER_SCALE_FACTOR:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("ThrustScaleFactor"));
   case GRAVITATIONAL_ACCEL:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("GravitationalAccel"));
      
   // Thrust Coefficients
   case C1:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C1"));
   case C2:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C2"));
   case C3:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C3"));
   case C4:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C4"));
   case C5:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C5"));
   case C6:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C6"));
   case C7:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C7"));
   case C8:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C8"));
   case C9:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C9"));
   case C10:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C10"));
   case C11:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C11"));
   case C12:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C12"));
   case C13:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C13"));
   case C14:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C14"));
   case C15:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C15"));
   case C16:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("C16"));
      
   // Impulse Coefficients
   case K1:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K1"));
   case K2:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K2"));
   case K3:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K3"));
   case K4:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K4"));
   case K5:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K5"));
   case K6:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K6"));
   case K7:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K7"));
   case K8:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K8"));
   case K9:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K9"));
   case K10:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K10"));
   case K11:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K11"));
   case K12:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K12"));
   case K13:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K13"));
   case K14:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K14"));
   case K15:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K15"));
   case K16:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("K16"));
      
   // Thruster ThrustDirections
   case THRUST_DIRECTION1:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("ThrustDirection1"));
   case THRUST_DIRECTION2:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("ThrustDirection2"));
   case THRUST_DIRECTION3:
      return GetOwnedObjectProperty(Gmat::THRUSTER, wxT("ThrustDirection3"));
      
   default:
      // otherwise, there is an error   
      throw ParameterException
         (wxT("SpacecraftData::GetReal() Unknown parameter id: ") +
          GmatStringUtil::ToString(item));
   }
}


//------------------------------------------------------------------------------
// Real SetReal(Integer item, Real val)
//------------------------------------------------------------------------------
/**
 * Sets Spacecraft or spacecraft owned hardware element by integer id.
 */
//------------------------------------------------------------------------------
Real SpacecraftData::SetReal(Integer item, Real val)
{
   if (mSpacecraft == NULL)
      InitializeRefObjects();
   
   switch (item)
   {
   case DRY_MASS:
      return mSpacecraft->SetRealParameter(wxT("DryMass"), val);
   case DRAG_COEFF:
      return mSpacecraft->SetRealParameter(wxT("Cd"), val);
   case REFLECT_COEFF:
      return mSpacecraft->SetRealParameter(wxT("Cr"), val);
   case DRAG_AREA:
      return mSpacecraft->SetRealParameter(wxT("DragArea"), val);
   case SRP_AREA:      
      return mSpacecraft->SetRealParameter(wxT("SRPArea"), val);
   case TOTAL_MASS:
      return mSpacecraft->SetRealParameter(wxT("TotalMass"), val);
      
   // for Spacecraft owned FuelTank
   case FUEL_MASS:
      return SetOwnedObjectProperty(Gmat::FUEL_TANK, wxT("FuelMass"), val);
   case PRESSURE:
      return SetOwnedObjectProperty(Gmat::FUEL_TANK, wxT("Pressure"), val);
   case TEMPERATURE:
      return SetOwnedObjectProperty(Gmat::FUEL_TANK, wxT("Temperature"), val);
   case REF_TEMPERATURE:
      return SetOwnedObjectProperty(Gmat::FUEL_TANK, wxT("RefTemperature"), val);
   case VOLUME:
      return SetOwnedObjectProperty(Gmat::FUEL_TANK, wxT("Volume"), val);
   case FUEL_DENSITY:
      return SetOwnedObjectProperty(Gmat::FUEL_TANK, wxT("FuelDensity"), val);
      
   // for Spacecraft owned Thruster
   case DUTY_CYCLE:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("DutyCycle"), val);
   case THRUSTER_SCALE_FACTOR:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("ThrustScaleFactor"), val);
   case GRAVITATIONAL_ACCEL:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("GravitationalAccel"), val);
      
   // Thrust Coefficients
   case C1:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C1"), val);
   case C2:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C2"), val);
   case C3:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C3"), val);
   case C4:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C4"), val);
   case C5:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C5"), val);
   case C6:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C6"), val);
   case C7:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C7"), val);
   case C8:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C8"), val);
   case C9:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C9"), val);
   case C10:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C10"), val);
   case C11:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C11"), val);
   case C12:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C12"), val);
   case C13:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C13"), val);
   case C14:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C14"), val);
   case C15:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C15"), val);
   case C16:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("C16"), val);
      
   // Impulse Coefficients
   case K1:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K1"), val);
   case K2:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K2"), val);
   case K3:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K3"), val);
   case K4:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K4"), val);
   case K5:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K5"), val);
   case K6:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K6"), val);
   case K7:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K7"), val);
   case K8:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K8"), val);
   case K9:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K9"), val);
   case K10:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K10"), val);
   case K11:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K11"), val);
   case K12:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K12"), val);
   case K13:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K13"), val);
   case K14:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K14"), val);
   case K15:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K15"), val);
   case K16:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("K16"), val);
      
   // Thruster ThrustDirections
   case THRUST_DIRECTION1:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("ThrustDirection1"), val);
   case THRUST_DIRECTION2:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("ThrustDirection2"), val);
   case THRUST_DIRECTION3:
      return SetOwnedObjectProperty(Gmat::THRUSTER, wxT("ThrustDirection3"), val);
      
   default:
      // otherwise, there is an error   
      throw ParameterException
         (wxT("SpacecraftData::SetReal() Unknown parameter id: ") +
          GmatStringUtil::ToString(item));
   }
}

//-------------------------------------
// Inherited methods from RefData
//-------------------------------------

//------------------------------------------------------------------------------
// virtual const wxString* GetValidObjectList() const
//------------------------------------------------------------------------------
const wxString* SpacecraftData::GetValidObjectList() const
{
   return VALID_OBJECT_TYPE_LIST;
}


//------------------------------------------------------------------------------
// bool ValidateRefObjects(GmatBase *param)
//------------------------------------------------------------------------------
/**
 * Validates reference objects for given parameter
 */
//------------------------------------------------------------------------------
bool SpacecraftData::ValidateRefObjects(GmatBase *param)
{
   int objCount = 0;
   for (int i=0; i<SpacecraftDataObjectCount; i++)
   {
      if (HasObjectType(VALID_OBJECT_TYPE_LIST[i]))
         objCount++;
   }
   
   if (objCount == SpacecraftDataObjectCount)
      return true;
   else
      return false;
}

//---------------------------------
// protected methods
//---------------------------------

//------------------------------------------------------------------------------
// virtual void InitializeRefObjects()
//------------------------------------------------------------------------------
void SpacecraftData::InitializeRefObjects()
{
   #ifdef DEBUG_SPACECRAFTDATA_INIT
   MessageInterface::ShowMessage
      (wxT("SpacecraftData::InitializeRefObjects() '%s' entered.\n"), mName.c_str());
   #endif
   
   mSpacecraft = (Spacecraft*)FindFirstObject(VALID_OBJECT_TYPE_LIST[SPACECRAFT]);
   
   if (mSpacecraft == NULL)
   {
      // Just write a message since Parameters in GmatFunction may not have ref. object
      // set until execution
      #ifdef DEBUG_SPACECRAFTDATA_INIT
      MessageInterface::ShowMessage
         (wxT("SpacecraftData::InitializeRefObjects() Cannot find Spacecraft object.\n"));
      #endif
      
      //throw ParameterException
      //   (wxT("SpacecraftData::InitializeRefObjects() Cannot find Spacecraft object.\n"));
   }
   
   #ifdef DEBUG_SPACECRAFTDATA_INIT
   MessageInterface::ShowMessage
      (wxT("SpacecraftData::InitializeRefObjects() '%s' leaving, mSpacecraft=<%p>'%s'\n"),
       mName.c_str(), mSpacecraft, mSpacecraft->GetName().c_str());
   #endif
}


//------------------------------------------------------------------------------
// virtual bool IsValidObjectType(Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Checks reference object type.
 *
 * @return return true if object is valid object, false otherwise
 */
//------------------------------------------------------------------------------
bool SpacecraftData::IsValidObjectType(Gmat::ObjectType type)
{
   for (int i=0; i<SpacecraftDataObjectCount; i++)
   {
      if (GmatBase::GetObjectTypeString(type) == VALID_OBJECT_TYPE_LIST[i])
         return true;
   }
   
   return false;

}


//------------------------------------------------------------------------------
// Real GetOwnedObjectProperty(Gmat::ObjectType objType, const wxString &propName)
//------------------------------------------------------------------------------
Real SpacecraftData::GetOwnedObjectProperty(Gmat::ObjectType objType,
                                            const wxString &propName)
{
   wxString type, owner, dep;
   GmatStringUtil::ParseParameter(mName, type, owner, dep);
   
   #ifdef DEBUG_SC_OWNED_OBJ
   MessageInterface::ShowMessage
      (wxT("SpacecraftData::GetOwnedObjectProperty() '%s' entered, objType=%d, propName='%s', ")
       wxT("type='%s', owner='%s', dep='%s'\n"), mName.c_str(), objType, propName.c_str(),
       type.c_str(), owner.c_str(), dep.c_str());
   #endif
   
   GmatBase *ownedObj = mSpacecraft->GetRefObject(objType, dep);
   
   #ifdef DEBUG_SC_OWNED_OBJ
   MessageInterface::ShowMessage
      (wxT("   ownedObj=<%p><%s>'%s'\n"), ownedObj, ownedObj ? ownedObj->GetTypeName().c_str() : wxT("NULL"),
       ownedObj ? ownedObj->GetName().c_str() : wxT("NULL"));
   #endif
   
   if (ownedObj == NULL)
   {
      ParameterException pe;
      pe.SetDetails(wxT("SpacecraftData::GetOwnedObjectProperty() %s \"%s\" is not ")
                    wxT("attached to Spacecraft \"%s\""),
                    GmatBase::GetObjectTypeString(objType).c_str(), dep.c_str(),
                    mSpacecraft->GetName().c_str());
      throw pe;
   }
   else
   {
      Real result = ownedObj->GetRealParameter(propName);
      
      #ifdef DEBUG_SC_OWNED_OBJ
      MessageInterface::ShowMessage
         (wxT("SpacecraftData::GetOwnedObjectProperty() returning %f\n"), result);
      #endif
      return result;
   }
}


//------------------------------------------------------------------------------
// Real SetOwnedObjectProperty(Gmat::ObjectType objType, const wxString &propName,
//                             Real val)
//------------------------------------------------------------------------------
Real SpacecraftData::SetOwnedObjectProperty(Gmat::ObjectType objType,
                                            const wxString &propName,
                                            Real val)
{
   wxString type, owner, dep;
   GmatStringUtil::ParseParameter(mName, type, owner, dep);
   
   #ifdef DEBUG_SC_OWNED_OBJ
   MessageInterface::ShowMessage
      (wxT("SpacecraftData::SetOwnedObjectProperty() '%s' entered, objType=%d, ")
       wxT("propName='%s', val=%f, type='%s', owner='%s', dep='%s',\n"),
       mName.c_str(), objType, propName.c_str(), val,
       type.c_str(), owner.c_str(), dep.c_str());
   #endif
   
   GmatBase *ownedObj = mSpacecraft->GetRefObject(objType, dep);
   
   #ifdef DEBUG_SC_OWNED_OBJ
   MessageInterface::ShowMessage
      (wxT("   ownedObj=<%p><%s>'%s'\n"), ownedObj, ownedObj ? ownedObj->GetTypeName().c_str() : wxT("NULL"),
       ownedObj ? ownedObj->GetName().c_str() : wxT("NULL"));
   #endif
   
   if (ownedObj == NULL)
   {
      ParameterException pe;
      pe.SetDetails(wxT("SpacecraftData::SetOwnedObjectProperty() %s \"%s\" is not ")
                    wxT("attached to Spacecraft \"%s\""),
                    GmatBase::GetObjectTypeString(objType).c_str(), dep.c_str(),
                    mSpacecraft->GetName().c_str());
      throw pe;
   }
   else
   {
      Real result = ownedObj->SetRealParameter(propName, val);
      
      #ifdef DEBUG_SC_OWNED_OBJ
      MessageInterface::ShowMessage
         (wxT("SpacecraftData::SetOwnedObjectProperty() '%s' returning %f\n"), mName.c_str(), result);
      #endif
      return result;
   }
}

