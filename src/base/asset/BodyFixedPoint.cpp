//$Id: BodyFixedPoint.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                            BodyFixedPoint
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
// Author: Wendy C. Shoan, NASA/GSFC (moved from GroundStation code,
//         original author: Darrel J. Conway, Thinking Systems, Inc.)
// Created: 2008.08.22

// Modified:
//    2010.03.25 Thomas Grubb
//      - Fixed check for latitude to be between -90 and 90
//    2010.03.23 Thomas Grubb/Steve Hughes
//      - Fixed SetStringParameter to correctly check for statetype values
//    2010.03.19 Thomas Grubb
//      - Overrode Copy method
//      - Changed StateType values from (Cartesian, Geographical) to
//        (Cartesian, Spherical (Geographical deprecates to Spherical))
//      - Added Location Units labels code
//      - Added checks that Latitude, Longitude between 0 and 360 and altitude
//        greater than or equal to 0
//
/**
 * Implements the Groundstation class used to model ground based tracking stations.
 */
//------------------------------------------------------------------------------

#include "BodyFixedPoint.hpp"
#include "AssetException.hpp"
#include "MessageInterface.hpp"
#include "RealUtilities.hpp"
#include "StringUtil.hpp"
#include "TimeTypes.hpp"
#include "GmatDefaults.hpp"


//#define DEBUG_OBJECT_MAPPING
//#define DEBUG_INIT
//#define DEBUG_BF_REF
//#define TEST_BODYFIXED_POINT
//#define DEBUG_BODYFIXED_STATE
//#define DEBUG_BODYFIXED_SET_REAL


//---------------------------------
// static data
//---------------------------------

/// Labels used for the ground station parameters.
const wxString
BodyFixedPoint::PARAMETER_TEXT[BodyFixedPointParamCount - SpacePointParamCount] =
   {
         wxT("CentralBody"),
         wxT("StateType"),         // Cartesian or Spherical
         wxT("HorizonReference"),  // Sphere or Ellipsoid
         wxT("Location1"),         // X or Latitude value
         wxT("Location2"),         // Y or Longitude value
         wxT("Location3"),         // Z or Altitude value
         wxT("LOCATION_LABEL_1"),  // "X" or "Latitude"
         wxT("LOCATION_LABEL_2"),  // "Y" or "Longitude"
         wxT("LOCATION_LABEL_3"),  // "Z" or "Altitude"
         wxT("LOCATION_UNITS_1"),  // "km" or "deg"
         wxT("LOCATION_UNITS_2"),  // "km" or "deg"
         wxT("LOCATION_UNITS_3")   // "km" or "km"
   };

const Gmat::ParameterType
BodyFixedPoint::PARAMETER_TYPE[BodyFixedPointParamCount - SpacePointParamCount] =
   {
         Gmat::OBJECT_TYPE,
         Gmat::ENUMERATION_TYPE,
         Gmat::ENUMERATION_TYPE,
         Gmat::REAL_TYPE,
         Gmat::REAL_TYPE,
         Gmat::REAL_TYPE,
         Gmat::STRING_TYPE,
         Gmat::STRING_TYPE,
         Gmat::STRING_TYPE
   };



//---------------------------------
// public methods
//---------------------------------


//---------------------------------------------------------------------------
//  BodyFixedPoint(const wxString &itsName)
//---------------------------------------------------------------------------
/**
 * Constructs a BodyFixedPoint object (default constructor).
 *
 * @param <itsName> Optional name for the object.  Defaults to "".
 */
//---------------------------------------------------------------------------
BodyFixedPoint::BodyFixedPoint(const wxString &itsType, const wxString &itsName,
      const Gmat::ObjectType objType) :
   SpacePoint           (objType, itsType, itsName),
   cBodyName            (wxT("Earth")),
   theBody              (NULL),
   meanEquatorialRadius (GmatSolarSystemDefaults::PLANET_EQUATORIAL_RADIUS[GmatSolarSystemDefaults::EARTH]),
   flattening           (GmatSolarSystemDefaults::PLANET_FLATTENING[GmatSolarSystemDefaults::EARTH]),
   stateType            (wxT("Cartesian")),
   horizon              (wxT("Sphere")),
   solarSystem          (NULL),
   bfcsName             (wxT("")),
   bfcs                 (NULL),
   mj2kcsName           (wxT("")),
   mj2kcs               (NULL)
{
   objectTypes.push_back(Gmat::BODY_FIXED_POINT);
   objectTypeNames.push_back(wxT("BodyFixedPoint"));
   parameterCount = BodyFixedPointParamCount;

   // assumes StateType = Cartesian
   locationLabels.push_back(wxT("X"));
   locationLabels.push_back(wxT("Y"));
   locationLabels.push_back(wxT("Z"));

   // assumes StateType = Cartesian
   locationUnits.push_back(wxT("km"));
   locationUnits.push_back(wxT("km"));
   locationUnits.push_back(wxT("km"));

   location[0] = GmatSolarSystemDefaults::PLANET_EQUATORIAL_RADIUS[GmatSolarSystemDefaults::EARTH];
   location[1] = 0.0;
   location[2] = 0.0;

   bfLocation[0] = GmatSolarSystemDefaults::PLANET_EQUATORIAL_RADIUS[GmatSolarSystemDefaults::EARTH];
   bfLocation[1] = 0.0;
   bfLocation[2] = 0.0;
}

//---------------------------------------------------------------------------
// ~BodyFixedPoint()
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
//---------------------------------------------------------------------------
BodyFixedPoint::~BodyFixedPoint()
{
}

//---------------------------------------------------------------------------
//  BodyFixedPoint(const BodyFixedPoint& bfp)
//---------------------------------------------------------------------------
/**
 * Constructs a new BodyFixedPoint by copying the input instance (copy
 * constructor).
 *
 * @param bfp  BodyFixedPoint instance to copy to create "this" instance.
 */
//---------------------------------------------------------------------------
BodyFixedPoint::BodyFixedPoint(const BodyFixedPoint& bfp) :
   SpacePoint           (bfp),
   cBodyName            (bfp.cBodyName),
   theBody              (NULL),
   meanEquatorialRadius (bfp.meanEquatorialRadius),
   flattening           (bfp.flattening),
   locationLabels       (bfp.locationLabels),
   locationUnits        (bfp.locationUnits),
   stateType            (bfp.stateType),
   horizon              (bfp.horizon),
   solarSystem          (NULL),
   bfcsName             (bfp.bfcsName),
   bfcs                 (NULL),
   mj2kcsName           (bfp.mj2kcsName),
   mj2kcs               (NULL)
{
   location[0]   = bfp.location[0];
   location[1]   = bfp.location[1];
   location[2]   = bfp.location[2];;

   bfLocation[0] = bfp.bfLocation[0];
   bfLocation[1] = bfp.bfLocation[1];
   bfLocation[2] = bfp.bfLocation[2];
}


//---------------------------------------------------------------------------
//  BodyFixedPoint& operator=(const BodyFixedPoint& bfp)
//---------------------------------------------------------------------------
/**
 * Assignment operator for BodyFixedPoints.
 *
 * @param bfp The original that is being copied.
 *
 * @return Reference to this object
 */
//---------------------------------------------------------------------------
BodyFixedPoint& BodyFixedPoint::operator=(const BodyFixedPoint& bfp)
{
   if (&bfp != this)
   {
      SpacePoint::operator=(bfp);

      theBody              = bfp.theBody;
      meanEquatorialRadius = bfp.meanEquatorialRadius;
      flattening           = bfp.flattening;
      locationLabels       = bfp.locationLabels;
      locationUnits        = bfp.locationUnits;
      stateType            = bfp.stateType;
      horizon              = bfp.horizon;
      solarSystem          = bfp.solarSystem;
      bfcsName             = bfp.bfcsName;
      //bfcs                 = bfp.bfcs;       // yes or no?
      bfcs                 = NULL;
      mj2kcsName           = bfp.mj2kcsName;
      //mj2kcs               = bfp.mj2kcs;     // yes or no?
      mj2kcs               = NULL;

      location[0]          = bfp.location[0];
      location[1]          = bfp.location[1];
      location[2]          = bfp.location[2];;

      bfLocation[0]        = bfp.bfLocation[0];
      bfLocation[1]        = bfp.bfLocation[1];
      bfLocation[2]        = bfp.bfLocation[2];
   }

   return *this;
}

//---------------------------------------------------------------------------
//  bool Initialize()
//---------------------------------------------------------------------------
/**
 * Initializes this object.
 *
 */
//---------------------------------------------------------------------------
bool BodyFixedPoint::Initialize()
{
   // Initialize the body data
   if (!theBody)
      throw AssetException(wxT("Unable to initialize ground station ") +
            instanceName + wxT("; its origin is not set\n"));

   // Get required data from the body
   flattening            = theBody->GetRealParameter(wxT("Flattening"));
   meanEquatorialRadius  = theBody->GetRealParameter(wxT("EquatorialRadius"));


   // Calculate the body-fixed Cartesian position
   // If it was input in Cartesian, we're done
   UpdateBodyFixedLocation();
//   if (stateType == wxT("Cartesian"))
//   {
//      bfLocation[0] = location[0];
//      bfLocation[1] = location[1];
//      bfLocation[2] = location[2];
//   }
//   // Otherwise, convert from input type to Cartesian
//   else if (stateType == wxT("Spherical"))
//   {
//      Rvector3 spherical(location[0], location[1], location[2]);
//      Rvector3 cart;
//      if (horizon == wxT("Sphere"))
//      {
//         cart = BodyFixedStateConverterUtil::SphericalToCartesian(spherical,
//                flattening, meanEquatorialRadius);
//         bfLocation[0] = cart[0];
//         bfLocation[1] = cart[1];
//         bfLocation[2] = cart[2];
//      }
//      else if (horizon == wxT("Ellipsoid"))
//      {
//         cart = BodyFixedStateConverterUtil::SphericalEllipsoidToCartesian(spherical,
//                flattening, meanEquatorialRadius);
//         bfLocation[0] = cart[0];
//         bfLocation[1] = cart[1];
//         bfLocation[2] = cart[2];
//      }
//      else
//         throw AssetException(wxT("Unable to initialize ground station \"") +
//               instanceName + wxT("\"; horizon reference is not a recognized type (known ")
//                     wxT("types are either \"Sphere\" or \"Ellipsoid\")"));
//   }
//   else
//      throw AssetException(wxT("Unable to initialize ground station \"") +
//            instanceName + wxT("\"; stateType is not a recognized type (known ")
//                  wxT("types are either \"Cartesian\" or \"Spherical\")"));

   #ifdef DEBUG_INIT
      MessageInterface::ShowMessage(wxT("...BodyFixedPoint %s Initialized!\n"), instanceName.c_str());
   #endif

   #ifdef TEST_BODYFIXED_POINT
      MessageInterface::ShowMessage(wxT("For %s, %s %s location [%lf ")
            wxT("%lf %lf] --> XYZ [%lf %lf %lf]\n"), instanceName.c_str(),
            stateType.c_str(), horizon.c_str(), location[0], location[1],
            location[2], bfLocation[0], bfLocation[1], bfLocation[2]);
      // Check the MJ2000 methods
      if (theBody == NULL)
      {
         MessageInterface::ShowMessage(
               wxT("Error initializing ground station %s: theBody is not set\n"),
               instanceName.c_str());
         return false;
      }
      if (bfcs == NULL)
      {
         MessageInterface::ShowMessage(
               wxT("Error initializing ground station %s: bfcs is not set\n"),
               instanceName.c_str());
         return false;
      }
      if (mj2kcs == NULL)
      {
         MessageInterface::ShowMessage(
               wxT("Error initializing ground station %s: mj2kcs is not set\n"),
               instanceName.c_str());
         return false;
      }

      Rvector6 j2kState = GetMJ2000State(GmatTimeConstants::MJD_OF_J2000);
      MessageInterface::ShowMessage(wxT("The resulting MJ2000 Cartesian state is ")
            wxT("\n   [%s]\n"), j2kState.ToString(16).c_str());
   #endif
   return true;
}

// Parameter access methods - overridden from GmatBase

//---------------------------------------------------------------------------
//  void Copy(const GmatBase* orig)
//---------------------------------------------------------------------------
/**
 * Sets this object to match another one.
 *
 * @param orig The original that is being copied.
 */
//---------------------------------------------------------------------------
void BodyFixedPoint::Copy(const GmatBase* orig)
{
   operator=(*((BodyFixedPoint *)(orig)));
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
 */
//------------------------------------------------------------------------------
wxString BodyFixedPoint::GetParameterText(const Integer id) const
{
   if (id >= SpacePointParamCount && id < BodyFixedPointParamCount)
      return PARAMETER_TEXT[id - SpacePointParamCount];
   return SpacePoint::GetParameterText(id);
}

//------------------------------------------------------------------------------
//  Integer  GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter ID, given the input parameter string.
 *
 * @param str string for the requested parameter.
 *
 * @return ID for the requested parameter.
 */
//------------------------------------------------------------------------------
Integer BodyFixedPoint::GetParameterID(const wxString &str) const
{
   // Handle 3 special cases
   if (str == locationLabels[0])
      return LOCATION_1;

   if (str == locationLabels[1])
      return LOCATION_2;

   if (str == locationLabels[2])
      return LOCATION_3;

   for (Integer i = SpacePointParamCount; i < BodyFixedPointParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - SpacePointParamCount])
         return i;
   }

   return SpacePoint::GetParameterID(str);
}

//------------------------------------------------------------------------------
//  Gmat::ParameterType  GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type, given the input parameter ID.
 *
 * @param id ID for the requested parameter.
 *
 * @return parameter type of the requested parameter.
 */
//------------------------------------------------------------------------------
Gmat::ParameterType BodyFixedPoint::GetParameterType(const Integer id) const
{
   if (id >= SpacePointParamCount && id < BodyFixedPointParamCount)
      return PARAMETER_TYPE[id - SpacePointParamCount];

   return SpacePoint::GetParameterType(id);
}

//------------------------------------------------------------------------------
//  wxString  GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type string, given the input parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return parameter type string of the requested parameter.
 */
//------------------------------------------------------------------------------
wxString BodyFixedPoint::GetParameterTypeString(const Integer id) const
{
   return GmatBase::PARAM_TYPE_STRING[GetParameterType(id)];
}

//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <id> Description for the parameter.
 *
 * @return true if the parameter is read only, false (the default) if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool BodyFixedPoint::IsParameterReadOnly(const Integer id) const
{
   if ((id >= SpacePointParamCount) && (id < BodyFixedPointParamCount))
      return ((id >= LOCATION_LABEL_1) && (id <= LOCATION_UNITS_3));

   return SpacePoint::IsParameterReadOnly(id);
}

//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <label> Description for the parameter.
 *
 * @return true if the parameter is read only, false (the default) if not.
 */
//---------------------------------------------------------------------------
bool BodyFixedPoint::IsParameterReadOnly(const wxString &label) const
{
   return IsParameterReadOnly(GetParameterID(label));
}

//---------------------------------------------------------------------------
// Gmat::ObjectType GetPropertyObjectType(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieves object type of parameter of given id.
 *
 * @param <id> ID for the parameter.
 *
 * @return parameter ObjectType
 */
//---------------------------------------------------------------------------
Gmat::ObjectType BodyFixedPoint::GetPropertyObjectType(const Integer id) const
{
   switch (id)
   {
   case CENTRAL_BODY:
      return Gmat::CELESTIAL_BODY;
   default:
      return SpacePoint::GetPropertyObjectType(id);
   }
}


//---------------------------------------------------------------------------
// const StringArray& GetPropertyEnumStrings(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieves eumeration symbols of parameter of given id.
 *
 * @param <id> ID for the parameter.
 *
 * @return list of enumeration symbols
 */
//---------------------------------------------------------------------------
const StringArray& BodyFixedPoint::GetPropertyEnumStrings(const Integer id) const
{
   static StringArray enumStrings;
   switch (id)
   {
   case STATE_TYPE:
      enumStrings.clear();
      enumStrings.push_back(wxT("Cartesian"));
      enumStrings.push_back(wxT("Spherical"));
      return enumStrings;
   case HORIZON_REFERENCE:
      enumStrings.clear();
      enumStrings.push_back(wxT("Sphere"));
      enumStrings.push_back(wxT("Ellipsoid"));
      return enumStrings;
   default:
      return SpacePoint::GetPropertyEnumStrings(id);
   }
}


//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return  string value of the requested parameter.
 */
//------------------------------------------------------------------------------
wxString BodyFixedPoint::GetStringParameter(const Integer id) const
{
   if (id == CENTRAL_BODY)
   {
      if (theBody)
         return theBody->GetName();
      else
         return cBodyName;
   }

   if (id == STATE_TYPE)
      return stateType;

   if (id == HORIZON_REFERENCE)
      return horizon;

   if ((id >= LOCATION_LABEL_1) && (id <= LOCATION_LABEL_3))
      return locationLabels[id-LOCATION_LABEL_1];

   if ((id >= LOCATION_UNITS_1) && (id <= LOCATION_UNITS_3))
      return locationUnits[id-LOCATION_UNITS_1];

   return SpacePoint::GetStringParameter(id);
}

//------------------------------------------------------------------------------
//  wxString  SetStringParameter(const Integer id, const wxString value)
//------------------------------------------------------------------------------
/**
 * This method sets the string parameter value, given the input
 * parameter ID.
 *
 * @param id ID for the requested parameter.
 * @param value string value for the requested parameter.
 *
 * @return  success flag.
 */
//------------------------------------------------------------------------------
bool BodyFixedPoint::SetStringParameter(const Integer id,
                                       const wxString &value)
{
   if (IsParameterReadOnly(id))
       return false;

   static bool firstTimeWarning = true;
   bool        retval           = false;
   wxString stateTypeList    = wxT("Cartesian, Spherical");
   wxString horizonList      = wxT("Sphere, Ellipsoid");
   wxString currentStateType = stateType;
   wxString currentHorizon   = horizon;

   if (id == CENTRAL_BODY)
   {
      if (value != SolarSystem::EARTH_NAME)
      {
         wxString errmsg =
            wxT("The value of \"") + value + wxT("\" for field \"CentralBody\"")
            wxT(" on object \"") + instanceName + wxT("\" is not an allowed value.\n")
            wxT("The allowed values are: [ ") + SolarSystem::EARTH_NAME + wxT(" ]. ");
         throw AssetException(errmsg);
      }
      if (theBody)
         theBody = NULL;
      cBodyName = value;
      retval = true;
   }
   else if (id == STATE_TYPE)
   {
      wxString v = value;
      if (v == wxT("Geographical")) // deprecated value
      {
        v = wxT("Spherical");
        // write one warning per GMAT session
        if (firstTimeWarning)
        {
           wxString msg =
              wxT("The value of \"") + value + wxT("\" for field \"StateType\"")
              wxT(" on object \"") + instanceName + wxT("\" is not an allowed value.\n")
              wxT("The allowed values are: [ ") + stateTypeList + wxT(" ]. ");
           MessageInterface::ShowMessage(wxT("*** WARNING *** ") + msg + wxT("\n"));
           firstTimeWarning = false;
        }
      }

      if ((v == wxT("Cartesian")) || (v == wxT("Spherical")))
      {
         stateType = v;
         if (v == wxT("Cartesian"))
         {
            locationLabels[0] = wxT("X");
            locationLabels[1] = wxT("Y");
            locationLabels[2] = wxT("Z");
            locationUnits[0] = wxT("km");
            locationUnits[1] = wxT("km");
            locationUnits[2] = wxT("km");
         }
         else
         {
            locationLabels[0] = wxT("Latitude");
            locationLabels[1] = wxT("Longitude");
            locationLabels[2] = wxT("Altitude");
            locationUnits[0] = wxT("deg");
            locationUnits[1] = wxT("deg");
            locationUnits[2] = wxT("km");
         }
         if (currentStateType != stateType)
         {
            Rvector3 locIn(location[0], location[1], location[2]);
            Rvector3 locOut = BodyFixedStateConverterUtil::Convert(locIn, currentStateType, horizon, stateType, horizon,
                                                           flattening, meanEquatorialRadius);
            location[0] = locOut[0];
            location[1] = locOut[1];
            location[2] = locOut[2];
         }
         retval = true;
      }
      else
      {
         wxString errmsg =
            wxT("The value of \"") + value + wxT("\" for field \"StateType\"")
            wxT(" on object \"") + instanceName + wxT("\" is not an allowed value.\n")
            wxT("The allowed values are: [ ") + stateTypeList + wxT(" ]. ");
         throw AssetException(errmsg);
      }
   }
   else if (id == HORIZON_REFERENCE)
   {
      if ((value == wxT("Sphere")) || (value == wxT("Ellipsoid")))
      {
         horizon = value;
         if (currentHorizon != horizon)
         {
            Rvector3 locIn(location[0], location[1], location[2]);
            Rvector3 locOut = BodyFixedStateConverterUtil::Convert(locIn, stateType, currentHorizon, stateType, horizon,
                                                       flattening, meanEquatorialRadius);
            location[0] = locOut[0];
            location[1] = locOut[1];
            location[2] = locOut[2];
         }
         retval = true;
      }
      else
      {
         wxString errmsg =
            wxT("The value of \"") + value + wxT("\" for field \"HorizonReference\"")
            wxT(" on object \"") + instanceName + wxT("\" is not an allowed value.\n")
            wxT("The allowed values are: [ ") + horizonList + wxT(" ]. ");
         throw AssetException(errmsg);
      }
   }
   else
      retval = SpacePoint::SetStringParameter(id, value);

   return retval;
}

//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Accessor method used to get a parameter value
 *
 * @param   label  label ID for the parameter
 *
 * @return the value of the parameter
 */
//------------------------------------------------------------------------------
wxString BodyFixedPoint::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}

//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
/**
* Accessor method used to get a parameter value
 *
 * @param    label Integer ID for the parameter
 * @param    value The new value for the parameter
 */
//------------------------------------------------------------------------------
bool BodyFixedPoint::SetStringParameter(const wxString &label,
                                           const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}

//------------------------------------------------------------------------------
//  GmatBase* GetRefObject(const Gmat::ObjectType type,
//                         const wxString &name)
//------------------------------------------------------------------------------
/**
 * This method returns a reference object from the BodyFixedPoint class.
 *
 * @param type  type of the reference object requested
 * @param name  name of the reference object requested
 *
 * @return pointer to the reference object requested.
 */
//------------------------------------------------------------------------------
GmatBase* BodyFixedPoint::GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name)
{
   #ifdef TEST_BODYFIXED_POINT
      MessageInterface::ShowMessage(wxT("Entering BodyFixedPoint::GetRefObject()"));
      MessageInterface::ShowMessage(wxT("name = %s, cBodyName = %s\n"),
            name.c_str(), cBodyName.c_str());
      if (!theBody)
         MessageInterface::ShowMessage(wxT("The Body is NULL, though!!!\n"));
   #endif
   if ((type == Gmat::SPACE_POINT) || (type == Gmat::CELESTIAL_BODY))
      if (name == cBodyName)
         return theBody;

   // Not handled here -- invoke the next higher GetRefObject call
   return SpacePoint::GetRefObject(type, name);
}

//------------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                    const wxString &name)
//------------------------------------------------------------------------------
/**
 * This method sets a reference object for the SpacePoint class.
 *
 * @param <obj>   pointer to the reference object
 * @param <type>  type of the reference object
 * @param <name>  name of the reference object
 *
 * @return true if successful; otherwise, false.
 */
//------------------------------------------------------------------------------
bool BodyFixedPoint::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                 const wxString &name)
{
   if (obj == NULL)
      return false;

   #ifdef DEBUG_OBJECT_MAPPING
      MessageInterface::ShowMessage
         (wxT("BodyFixedPoint::SetRefObject() this=%s, obj=<%p><%s>, type=<%d><%s> entered\n"),
          GetName().c_str(), obj, obj->GetName().c_str(), (Integer) type,
          GetObjectTypeString(type).c_str());
   #endif

   switch (type)
   {
      case Gmat::SPACE_POINT:
      case Gmat::CELESTIAL_BODY:
         if (obj->GetName() == cBodyName)
         {
            theBody = (SpacePoint*)obj;
            // Let ancestors process this object as well

            #ifdef DEBUG_OBJECT_MAPPING
               MessageInterface::ShowMessage
                  (wxT("BodyFixedPoint::Setting theBody to %s\n"),
                   theBody->GetName().c_str());
            #endif

//            SpacePoint::SetRefObject(obj, type, name);
            return true;
         }
         break;

      case Gmat::COORDINATE_SYSTEM:
         {
            if (!(obj->IsOfType(wxT("CoordinateSystem"))))
               throw AssetException(wxT("BodyFixedPoint expecting a CoordinateSystem\n"));
            CoordinateSystem *tmpCS = (CoordinateSystem*)obj;
            if ((name == bfcsName) &&
                (tmpCS->GetOriginName() == cBodyName))
            {
               #ifdef DEBUG_OBJECT_MAPPING
                  MessageInterface::ShowMessage
                     (wxT("BodyFixedPoint::Setting bfcs to %s\n"),
                      tmpCS->GetName().c_str());
               #endif
               bfcs = tmpCS;
               return true;
            }
            if ((name == mj2kcsName) &&
                (tmpCS->GetOriginName() == cBodyName))
            {
               #ifdef DEBUG_OBJECT_MAPPING
                  MessageInterface::ShowMessage
                     (wxT("BodyFixedPoint::Setting mj2kcs to %s\n"),
                      tmpCS->GetName().c_str());
               #endif
               mj2kcs = tmpCS;
               return true;
            }

            break;
         }
      default:
         break;
   }

   MessageInterface::ShowMessage(wxT("BodyFixedPoint::SetRefObject calling base for %s\n"), name.c_str());

   // Not handled here -- invoke the next higher SetRefObject call
   return SpacePoint::SetRefObject(obj, type, name);
}


Real BodyFixedPoint::GetRealParameter(const Integer id) const
{
   if ((id >= LOCATION_1) && (id <= LOCATION_3))
   {
      if ((stateType == wxT("Cartesian")) || (id == LOCATION_3))  // all units are km
         return location[id - LOCATION_1];
      // need to return units of degrees for Spherical state latitude and longitude
      else
         return location[id - LOCATION_1] * GmatMathConstants::DEG_PER_RAD;
   }

   return SpacePoint::GetRealParameter(id);
}


Real BodyFixedPoint::SetRealParameter(const Integer id,
                                      const Real value)
{
   #ifdef DEBUG_BODYFIXED_SET_REAL
      MessageInterface::ShowMessage(wxT("Entering BFP::SetRealParameter with id = %d (%s) and value = %12.10f\n"),
            id, (GetParameterText(id)).c_str(), value);
//      MessageInterface::ShowMessage(wxT("stateType = %s and horizon = %s\n"),
//            stateType.c_str(), horizon.c_str());
   #endif
   if (((id == LOCATION_1) || (id == LOCATION_2)) && stateType == wxT("Spherical"))
   {
      // if Spherical statetype, then check if Latitude/Longitude are in the correct range
      if (id == LOCATION_1) // latitude
      {
         if ((value >= -90.0) && (value <= 90))
            location[id-LOCATION_1] = value * GmatMathConstants::RAD_PER_DEG;
         else
         {
            AssetException aException(wxT(""));
            aException.SetDetails(errorMessageFormat.c_str(),
                        GmatStringUtil::ToString(value, 16).c_str(),
                        GetStringParameter(id-LOCATION_1+LOCATION_LABEL_1).c_str(), wxT("Real Number >= -90.0 and <= 90.0"));
            throw aException;
         }
      }
      else // longitude (0-360)
         location[id-LOCATION_1] = (GmatMathUtil::Mod(value,360)) * GmatMathConstants::RAD_PER_DEG;
      return location[id-LOCATION_1];
   }
   else if ((id >= LOCATION_1) && (id <= LOCATION_3)) // not Spherical
   {
      location[id-LOCATION_1] = value;
      return location[id-LOCATION_1];
   }

   return SpacePoint::SetRealParameter(id, value);
}

//Real BodyFixedPoint::GetRealParameter(const Integer id,
//                                      const Integer index) const
//{
//   return SpacePoint::GetRealParameter(id, index);
//}
//
//Real BodyFixedPoint::GetRealParameter(const Integer id, const Integer row,
//                                      const Integer col) const
//{
//   return SpacePoint::GetRealParameter(id, row, col);
//}

Real BodyFixedPoint::GetRealParameter(const wxString &label) const
{
   return GetRealParameter(GetParameterID(label));
}

Real BodyFixedPoint::SetRealParameter(const wxString &label,
                                      const Real value)
{
   return SetRealParameter(GetParameterID(label), value);
}

// These indexed methods seem like they should NOT be needed, but GCC gets
// confused about the overloaded versions of the following six methods:

//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value from a vector of strings,
 * given the input parameter ID and the index into the vector.
 *
 * @param id ID for the requested parameter.
 * @param index index for the particular string requested.
 *
 * @return The requested string.
 */
//------------------------------------------------------------------------------
wxString BodyFixedPoint::GetStringParameter(const Integer id,
                                              const Integer index) const
{
   return SpacePoint::GetStringParameter(id, index);
}

//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value,
//                         const Integer index)
//------------------------------------------------------------------------------
/**
 * This method sets a value on a string parameter value in a vector of strings,
 * given the input parameter ID, the value, and the index into the vector.
 *
 * @param id ID for the requested parameter.
 * @param value The new string.
 * @param index index for the particular string requested.
 *
 * @return true if successful; otherwise, false.
 */
//------------------------------------------------------------------------------
bool BodyFixedPoint::SetStringParameter(const Integer id,
                                       const wxString &value,
                                       const Integer index)
{
   return SetStringParameter(id, value, index);
}

//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label,
//                                const Integer index) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value from a vector of strings,
 * given the label associated with the input parameter and the index into the
 * vector.
 *
 * @param label String identifier for the requested parameter.
 * @param index index for the particular string requested.
 *
 * @return The requested string.
 */
//------------------------------------------------------------------------------
wxString BodyFixedPoint::GetStringParameter(const wxString &label,
                                           const Integer index) const
{
   return SpacePoint::GetStringParameter(label,  index);
}

//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value,
//                         const Integer index)
//------------------------------------------------------------------------------
/**
 * This method sets a value on a string parameter value in a vector of strings,
 * given the label associated with the input parameter and the index into the
 * vector.
 *
 * @param label String identifier for the requested parameter.
 * @param value The new string.
 * @param index index for the particular string requested.
 *
 * @return true if successful; otherwise, false.
 */
//------------------------------------------------------------------------------
bool BodyFixedPoint::SetStringParameter(const wxString &label,
                                       const wxString &value,
                                       const Integer index)
{
   return SpacePoint::SetStringParameter(label, value, index);
}


//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name,
//                        const Integer index)
//------------------------------------------------------------------------------
/**
 * This method returns a pointer to a reference object contained in a vector of
 * objects in the BodyFixedPoint class.
 *
 * @param type type of the reference object requested
 * @param name name of the reference object requested
 * @param index index for the particular object requested.
 *
 * @return pointer to the reference object requested.
 */
//------------------------------------------------------------------------------
GmatBase* BodyFixedPoint::GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name,
                                     const Integer index)
{
   return SpacePoint::GetRefObject(type, name, index);
}

//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name, const Integer index)
//------------------------------------------------------------------------------
/**
 * This method sets a pointer to a reference object in a vector of objects in
 * the BodyFixedPoint class.
 *
 * @param obj The reference object.
 * @param type type of the reference object requested
 * @param name name of the reference object requested
 * @param index index for the particular object requested.
 *
 * @return true if successful; otherwise, false.
 */
//------------------------------------------------------------------------------
bool BodyFixedPoint::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                  const wxString &name,
                                  const Integer index)
{
   // Call parent class to add objects to bodyList
   return SpacePoint::SetRefObject(obj, type, name, index);
}


//------------------------------------------------------------------------------
// virtual bool HasRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
bool BodyFixedPoint::HasRefObjectTypeArray()
{
   return true;
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------------------
wxString BodyFixedPoint::GetRefObjectName(const Gmat::ObjectType type) const
{
   return cBodyName;
}

const StringArray& BodyFixedPoint::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   #ifdef DEBUG_BF_REF
      MessageInterface::ShowMessage(wxT("In BFP::GetRefObjectNameArray, requesting type %d (%s)\n"),
            (Integer) type, (GmatBase::OBJECT_TYPE_STRING[type]).c_str());
   #endif

   static StringArray csNames;

   csNames.clear();

   if ((type == Gmat::COORDINATE_SYSTEM) || (type == Gmat::UNKNOWN_OBJECT))
   {
      csNames.push_back(bfcsName);
      csNames.push_back(mj2kcsName);
   }

   return csNames;
}

//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by this class.
 *
 * @return the list of object types.
 *
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& BodyFixedPoint::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::COORDINATE_SYSTEM);
   return refObjectTypes;
}

// Handle the J2000Body methods
//------------------------------------------------------------------------------
//  const Rvector6 GetMJ2000State(const A1Mjd &atTime)
//------------------------------------------------------------------------------
/**
 * Method returning the MJ2000 state of the SpacePoint at the time atTime.
 *
 * @param <atTime> Time for which the state is requested.
 *
 * @return state of the SpacePoint at time atTime.
 *
 * @note This method is pure virtual and must be implemented by the
 *       'leaf' (non-abstract derived) classes.
 */
//------------------------------------------------------------------------------
const Rvector6 BodyFixedPoint::GetMJ2000State(const A1Mjd &atTime)
{
   #ifdef DEBUG_BODYFIXED_STATE
      MessageInterface::ShowMessage(wxT("In GetMJ2000State for BodyFixedPoint %s\n"),
            instanceName.c_str());
   #endif

   UpdateBodyFixedLocation();
   Real     epoch = atTime.Get();
   Rvector6 bfState;

   // For now I'm ignoring velocity; this assumes bfLocation is kept up-to-date
   bfState.Set(bfLocation[0], bfLocation[1], bfLocation[2], 0.0, 0.0, 0.0);

   // Convert from the body-fixed location to a J2000 location,
   // assuming you have pointer to coordinate systems mj2k and bfcs,
   // where mj2k is a J2000 system and bfcs is BodyFixed
   #ifdef DEBUG_BODYFIXED_STATE
      MessageInterface::ShowMessage(wxT("... before call to Convert, epoch = %12.10f\n"),
            epoch);
      MessageInterface::ShowMessage(wxT(" ... bfcs = %s  and mj2kcs = %s\n"),
            (bfcs? wxT("NOT NULL") : wxT("NULL")), (mj2kcs? wxT("NOT NULL") : wxT("NULL")));
      MessageInterface::ShowMessage(wxT("bf state (in bfcs, cartesian) = %s\n"),
            (bfState.ToString()).c_str());
   #endif
   ccvtr.Convert(epoch, bfState, bfcs, j2000PosVel, mj2kcs);
   #ifdef DEBUG_BODYFIXED_STATE
      MessageInterface::ShowMessage(wxT("bf state (in mj2kcs, cartesian) = %s\n"),
            (j2000PosVel.ToString()).c_str());
   #endif

   return j2000PosVel;
}

//------------------------------------------------------------------------------
//  const Rvector3 GetMJ2000Position(const A1Mjd &atTime)
//------------------------------------------------------------------------------
/**
 * Method returning the MJ2000 position of the SpacePoint at the time atTime.
 *
 * @param <atTime> Time for which the position is requested.
 *
 * @return position of the SpacePoint at time atTime.
 *
 * @note This method is pure virtual and must be implemented by the
 *       'leaf' (non-abstract derived) classes.
 */
//------------------------------------------------------------------------------
const Rvector3 BodyFixedPoint::GetMJ2000Position(const A1Mjd &atTime)
{
   Rvector6 rv = GetMJ2000State(atTime);
   j2000Pos = rv.GetR();
   return j2000Pos;
}

//------------------------------------------------------------------------------
//  const Rvector3 GetMJ2000Velocity(const A1Mjd &atTime)
//------------------------------------------------------------------------------
/**
 * Method returning the MJ2000 velocity of the SpacePoint at the time atTime.
 *
 * @param <atTime> Time for which the velocity is requested.
 *
 * @return velocity of the SpacePoint at time atTime.
 *
 * @note This method is pure virtual and must be implemented by the
 *       'leaf' (non-abstract derived) classes.
 */
//------------------------------------------------------------------------------
const Rvector3 BodyFixedPoint::GetMJ2000Velocity(const A1Mjd &atTime)
{
   Rvector6 rv = GetMJ2000State(atTime);
   j2000Vel = rv.GetV();
   return j2000Vel;
}

//------------------------------------------------------------------------------
//  bool GetBodyFixedLocation(const A1Mjd &atTime)
//------------------------------------------------------------------------------
/**
 * Method returning the BodyFixed location of the BodyFixedPoint
 * at the time atTime.
 *
 * @param <atTime> Time for which the location is requested.
 *
 * @return location of the BodyFixedPoint at time atTime.
 *
 * @note This method may be moved to an intermediate BodyFixedPoint
 * class, if/when appropriate.
 * @note time is ignored as the body-fixed-point is assumed not to move
 */
//------------------------------------------------------------------------------
const Rvector3 BodyFixedPoint::GetBodyFixedLocation(const A1Mjd &atTime)
{
   UpdateBodyFixedLocation();

   Rvector3 locBodyFixed;
   locBodyFixed[0] = bfLocation[0];
   locBodyFixed[1] = bfLocation[1];
   locBodyFixed[2] = bfLocation[2];

   return locBodyFixed;
}

//------------------------------------------------------------------------------
//  CoordinateSystem* GetBodyFixedCoordinateSystem() const
//------------------------------------------------------------------------------
/**
 * Method returning the BodyFixed coordinate system used by this BodyFixedPoint.
 *
  * @return the BodyFixed coordinate system.
 *
 * @note This method may be moved to an intermediate BodyFixedPoint
 * class, if/when appropriate.
 */
//------------------------------------------------------------------------------
CoordinateSystem* BodyFixedPoint::GetBodyFixedCoordinateSystem() const
{
   return bfcs;
}


//------------------------------------------------------------------------------
//  void SetSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------------------
void BodyFixedPoint::SetSolarSystem(SolarSystem *ss)
{
   solarSystem = ss;
}

//------------------------------------------------------------------------------
//  void UpdateBodyFixedLocation()
//------------------------------------------------------------------------------
/**
 * This method makes sure that the bfLocation is up-to-date (as new location
 * data may have been input)
 *
 */
//------------------------------------------------------------------------------
void BodyFixedPoint::UpdateBodyFixedLocation()
{
   if (stateType == wxT("Cartesian"))
   {
      bfLocation[0] = location[0];
      bfLocation[1] = location[1];
      bfLocation[2] = location[2];
   }
   // Otherwise, convert from input type to Cartesian
   else if (stateType == wxT("Spherical"))
   {
      Rvector3 spherical(location[0], location[1], location[2]);
      Rvector3 cart;
      if (horizon == wxT("Sphere"))
      {
         cart = BodyFixedStateConverterUtil::SphericalToCartesian(spherical,
                flattening, meanEquatorialRadius);
         bfLocation[0] = cart[0];
         bfLocation[1] = cart[1];
         bfLocation[2] = cart[2];
      }
      else if (horizon == wxT("Ellipsoid"))
      {
         cart = BodyFixedStateConverterUtil::SphericalEllipsoidToCartesian(spherical,
                flattening, meanEquatorialRadius);
         bfLocation[0] = cart[0];
         bfLocation[1] = cart[1];
         bfLocation[2] = cart[2];
      }
      else
         throw AssetException(wxT("Unable to set body fixed location for BodyFixedPoint \"") +
               instanceName + wxT("\"; horizon reference is not a recognized type (known ")
                     wxT("types are either \"Sphere\" or \"Ellipsoid\")"));
   }
   else
   {
      throw AssetException(wxT("Unable to set body fixed location for BodyFixedPoint \"") +
            instanceName + wxT("\"; state type is not a recognized type (known ")
                  wxT("types are either \"Cartesian\" or \"Spherical\")"));
   }

}
