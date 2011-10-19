//$Id: PlanetData.cpp 9811 2011-08-29 21:15:18Z wendys-dev $
//------------------------------------------------------------------------------
//                                  PlanetData
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
// Created: 2004/12/13
// Modified:
//   2005/6/10 Linda Jun - Moved BetaAngle to AngularParamters.cpp
//
/**
 * Implements class which provides planet related data, such as HourAngle.
 *   MHA, Longitude, Altitude(Geodetic), Latitude(Geodetic), LST
 */
//------------------------------------------------------------------------------
#include "gmatdefs.hpp"
#include "PlanetData.hpp"
#include "ParameterException.hpp"
#include "GmatConstants.hpp"
#include "CelestialBody.hpp"
#include "AngleUtil.hpp"          // for PutAngleInDegRange()
#include "Linear.hpp"             // for GmatRealUtil::ToString()
#include "CalculationUtilities.hpp"
#include "MessageInterface.hpp"

//#define __COMPUTE_LONGITUDE_OLDWAY__

//#define DEBUG_PLANETDATA_INIT
//#define DEBUG_PLANETDATA_RUN
//#define DEBUG_ALTITUDE

using namespace GmatMathUtil;

//---------------------------------
// static data
//---------------------------------

const Real PlanetData::PLANET_REAL_UNDEFINED = GmatRealConstants::REAL_UNDEFINED_LARGE;

const wxString
PlanetData::VALID_OBJECT_TYPE_LIST[PlanetDataObjectCount] =
{
   wxT("Spacecraft"),
   wxT("SolarSystem"),
   wxT("CoordinateSystem"),
   wxT("SpacePoint")
}; 

const wxString PlanetData::VALID_PLANET_DATA_NAMES[LST_ID - LATITUDE + 1] =
{
   wxT("Latitude"),
   wxT("Longitude"),
   wxT("Altitude"),
   wxT("MHA"),
   wxT("LST")
};


//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// PlanetData()
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
PlanetData::PlanetData()
   : RefData()
{
   mCentralBodyName = wxT("");
   
   mSpacecraft = NULL;
   mSolarSystem = NULL;
   mCentralBody = NULL;
   mOrigin = NULL;
   mInternalCoordSystem = NULL;
   mOutCoordSystem = NULL;
}


//------------------------------------------------------------------------------
// PlanetData(const PlanetData &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the PlanetData object being copied.
 */
//------------------------------------------------------------------------------
PlanetData::PlanetData(const PlanetData &copy)
   : RefData(copy)
{
   mCentralBodyName = copy.mCentralBodyName;
   mSpacecraft = copy.mSpacecraft;
   mSolarSystem = copy.mSolarSystem;
   mCentralBody = copy.mCentralBody;
   mOrigin = copy.mOrigin;
   mInternalCoordSystem = copy.mInternalCoordSystem;
   mOutCoordSystem = copy.mOutCoordSystem;
}


//------------------------------------------------------------------------------
// PlanetData& operator= (const PlanetData &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the object being copied.
 *
 * @return reference to this object
 */
//------------------------------------------------------------------------------
PlanetData& PlanetData::operator= (const PlanetData &right)
{
   if (this == &right)
      return *this;

   RefData::operator=(right);
      
   mCentralBodyName = right.mCentralBodyName;
   mSpacecraft = right.mSpacecraft;
   mSolarSystem = right.mSolarSystem;
   mCentralBody = right.mCentralBody;
   mOrigin = right.mOrigin;
   mInternalCoordSystem = right.mInternalCoordSystem;
   mOutCoordSystem = right.mOutCoordSystem;
   
   return *this;
}


//------------------------------------------------------------------------------
// ~PlanetData()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
PlanetData::~PlanetData()
{
}


//------------------------------------------------------------------------------
// Real GetPlanetReal(Integer item)
//------------------------------------------------------------------------------
/**
 * Retrieves planet related parameters.
 */
//------------------------------------------------------------------------------
Real PlanetData::GetPlanetReal(Integer item)
{
   #ifdef DEBUG_PLANETDATA_RUN
   MessageInterface::ShowMessage(wxT("PlanetData::GetPlanetReal() item=%d\n"), item);
   #endif
   
   if (item < LATITUDE || item > LST_ID)
      throw ParameterException
         (wxT("PlanetData::GetPlanetReal() Unknown parameter ID: ") +
          GmatRealUtil::ToString(item));
   
   if (mSpacecraft == NULL || mSolarSystem == NULL)
      InitializeRefObjects();

   // Get current time
   Real a1mjd = mSpacecraft->GetEpoch();
   
   // Call GetHourAngle() on origin
   Real mha = mOrigin->GetHourAngle(a1mjd);

   Real epoch = mSpacecraft->GetRealParameter(wxT("A1Epoch"));
   Rvector6 instate = mSpacecraft->GetState().GetState();
   Rvector6 state;
   mCoordConverter.Convert(A1Mjd(epoch), instate, mInternalCoordSystem,
                           state, mOutCoordSystem);
   // get flattening for the body
   Real flatteningFactor =
      mOrigin->GetRealParameter(mOrigin->GetParameterID(wxT("Flattening")));

   Real equatorialRadius =
      mOrigin->GetRealParameter(mOrigin->GetParameterID(wxT("EquatorialRadius")));

   return GmatCalcUtil::CalculatePlanetData(VALID_PLANET_DATA_NAMES[item - LATITUDE], state, equatorialRadius, flatteningFactor, mha);
}


//------------------------------------------------------------------------------
// Real GetPlanetReal(const wxString &str)
//------------------------------------------------------------------------------
/**
 * Retrieves planet related parameters.
 */
//------------------------------------------------------------------------------
Real PlanetData::GetPlanetReal(const wxString &str)
{
   if (str == wxT("Latitude"))
      return GetPlanetReal(LATITUDE);
   else if (str == wxT("Longitude"))
      return GetPlanetReal(LONGITUDE);
   else if (str == wxT("Altitude"))
      return GetPlanetReal(ALTITUDE);
   else if (str == wxT("MHA"))
      return GetPlanetReal(MHA_ID);
   else if (str == wxT("LST"))
      return GetPlanetReal(LST_ID);
   else
   {
      throw ParameterException
         (wxT("PlanetData::GetPlanetReal Unknown parameter name: ") + str);
   }
}


//------------------------------------------------------------------------------
// void SetInternalCoordSystem(CoordinateSystem *cs)
//------------------------------------------------------------------------------
/*
 * @param <cs> internal coordinate system what parameter data is representing.
 */
//------------------------------------------------------------------------------ 
void PlanetData::SetInternalCoordSystem(CoordinateSystem *cs)
{
   mInternalCoordSystem = cs;
}


//-------------------------------------
// Inherited methods from RefData
//-------------------------------------

//------------------------------------------------------------------------------
// virtual const wxString* GetValidObjectList() const
//------------------------------------------------------------------------------
const wxString* PlanetData::GetValidObjectList() const
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
bool PlanetData::ValidateRefObjects(GmatBase *param)
{
   int objCount = 0;
   for (int i=0; i<PlanetDataObjectCount; i++)
   {
      if (HasObjectType(VALID_OBJECT_TYPE_LIST[i]))
         objCount++;
   }

   if (objCount == PlanetDataObjectCount)
      return true;
   else
      return false;
}


//------------------------------------------------------------------------------
// virtual void InitializeRefObjects()
//------------------------------------------------------------------------------
void PlanetData::InitializeRefObjects()
{
   mSpacecraft = (Spacecraft*)FindFirstObject(VALID_OBJECT_TYPE_LIST[SPACECRAFT]);

   if (mSpacecraft == NULL)
      throw ParameterException
         (wxT("PlanetData::InitializeRefObjects() Cannot find Spacecraft object.\n")
          wxT("Make sure Spacecraft is set.\n"));
   
   mSolarSystem = (SolarSystem*)FindFirstObject(VALID_OBJECT_TYPE_LIST[SOLAR_SYSTEM]);
   if (mSolarSystem == NULL)
      throw ParameterException
         (wxT("PlanetData::InitializeRefObjects() Cannot find SolarSystem object\n"));
   
   mCentralBody = mSolarSystem->GetBody(mCentralBodyName);
   
   if (!mCentralBody)
      throw ParameterException(wxT("PlanetData::InitializeRefObjects() Body not found in the ")
                               wxT("SolarSystem: ") + mCentralBodyName + wxT("\n"));
   
   if (mInternalCoordSystem == NULL)
      throw ParameterException
         (wxT("PlanetData::InitializeRefObjects() Cannot find internal ")
          wxT("CoordinateSystem object\n"));
   #ifdef DEBUG_PLANETDATA_INIT
      MessageInterface::ShowMessage
         (wxT("PlanetData::InitializeRefObjects() getting output CS pointer.\n"));
   #endif
   
   mOutCoordSystem =
      (CoordinateSystem*)FindFirstObject(VALID_OBJECT_TYPE_LIST[COORD_SYSTEM]);
   
   if (mOutCoordSystem == NULL)
      throw ParameterException
         (wxT("PlanetData::InitializeRefObjects() Cannot find output ")
          wxT("CoordinateSystem object\n"));
   
   // if dependent body name exist and it is a CelestialBody, set gravity constant
   
   wxString originName =
      FindFirstObjectName(GmatBase::GetObjectType(VALID_OBJECT_TYPE_LIST[SPACE_POINT]));
   
   if (originName != wxT(""))
   {
      #ifdef DEBUG_PLANETDATA_INIT
         MessageInterface::ShowMessage
            (wxT("PlanetData::InitializeRefObjects() getting originName:%s pointer.\n"),
             originName.c_str());
      #endif
         
      mOrigin =
         (CelestialBody*)FindFirstObject(VALID_OBJECT_TYPE_LIST[SPACE_POINT]);
      
      if (!mOrigin)
      {
         throw ParameterException
            (wxT("PlanetData::InitializeRefObjects() Cannot find Origin object: ") +
             originName + wxT("\n"));
      }

   }
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
bool PlanetData::IsValidObjectType(Gmat::ObjectType type)
{
#ifdef DEBUG_PLANETDATA_INIT
   MessageInterface::ShowMessage
      (wxT("PlanetData::IsValidObjectType() CHECKING type %s.\n"),(GmatBase::GetObjectTypeString(type).c_str()));
#endif
   for (int i=0; i<PlanetDataObjectCount; i++)
   {
#ifdef DEBUG_PLANETDATA_INIT
   MessageInterface::ShowMessage
      (wxT("PlanetData::IsValidObjectType() has type %s.\n"),(VALID_OBJECT_TYPE_LIST[i]).c_str());
#endif
      if (GmatBase::GetObjectTypeString(type) == VALID_OBJECT_TYPE_LIST[i])
         return true;
      // Special case for CelestialBody origin
      if ((VALID_OBJECT_TYPE_LIST[i] == wxT("SpacePoint")) && (GmatBase::GetObjectTypeString(type) == wxT("CelestialBody")))
         return true;
   }
   
   return false;
}

