//$Id: LatLonHgt.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              LatLonHgt
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed by Dr. Matthew P. Wilkins, Schafer Corporation
//
// Author: Matthew P. Wilkins
// Created: 2008/05/28
//
/**
 *
 * Implements LatLonHgt Base class to contain the associated elements
 * Latitude, Longitude, Height, and a flag to determine geocentric versus
 * geodetic angles.
 *
 */
//------------------------------------------------------------------------------

#include "LatLonHgt.hpp"
#include "GmatConstants.hpp"
#include "UtilityException.hpp"

//---------------------------------
//  static data
//---------------------------------
const wxString LatLonHgt::DATA_DESCRIPTIONS[NUM_DATA] =
{
   wxT("Latitude"),
   wxT("Longitude"),
   wxT("Height"),
   wxT("Type")
};

const wxString LatLonHgt::TYPE_DESCRIPTIONS[3] =
{
   wxT("Geocentric"),
   wxT("Geodetic"),
   wxT("Reduced")
};

const wxString LatLonHgt::HEIGHT_DESCRIPTIONS[3] =
{
   wxT("Ellipsoid"),
   wxT("Geoid"),
   wxT("MeanSeaLevel")
};


//---------------------------------
//  public methods
//---------------------------------

//------------------------------------------------------------------------------
//  LatLonHgt::LatLonHgt() 
//------------------------------------------------------------------------------
/**
 * Constructs base LatLonHgt structures 
 */
LatLonHgt::LatLonHgt() :
    latitude  (0.0),
    longitude (0.0),
    height (0.0),
    type (wxString(wxT("Geodetic"))),
    hgtRef (wxString(wxT("Ellipsoid")))
{
}

//------------------------------------------------------------------------------
//  LatLonHgt::LatLonHgt(Rvector3 cartPosition, Real er, Real flat) 
//------------------------------------------------------------------------------
/**
 * Constructs base LatLonHgt structures 
 */
LatLonHgt::LatLonHgt(const Rvector3 &cartPosition, const Real &equatorialRadius, 
		     const Real &flattening, const wxString &typ,
		     const wxString &hgtReference)
{

  CartesianToLatLonHgt(cartPosition,equatorialRadius,flattening,typ, hgtReference);

}

//------------------------------------------------------------------------------
//  LatLonHgt::LatLonHgt(Real lat,  Real lon, Real hgt, wxString typ)
//------------------------------------------------------------------------------
LatLonHgt::LatLonHgt(const Real &lat,  const Real &lon, const Real &hgt, 
		     const wxString &typ,
		     const wxString &hgtReference) :
    latitude  (lat),
    longitude (lon),
    height (hgt),
    type (typ),
    hgtRef (hgtReference)
{
}

//------------------------------------------------------------------------------
//   LatLonHgt::LatLonHgt(const LatLonHgt &llh)
//------------------------------------------------------------------------------
LatLonHgt::LatLonHgt(const LatLonHgt &llh) :
  latitude  (llh.latitude),
  longitude    (llh.longitude),
  height  (llh.height),
  type (llh.type),
  hgtRef (llh.hgtRef)
{
}

//------------------------------------------------------------------------------
//  LatLonHgt& LatLonHgt::operator=(const LatLonHgt &llh)
//------------------------------------------------------------------------------
LatLonHgt& LatLonHgt::operator=(const LatLonHgt &llh)
{
   if (this != &llh)
   {
      SetLatitude( llh.GetLatitude(), llh.GetType());
      SetLongitude( llh.GetLongitude() );
      SetHeight( llh.GetHeight() );
      SetHeightRef( llh.GetHeightRef() );
   }
   return *this;
}

//------------------------------------------------------------------------------
//  LatLonHgt::~LatLonHgt() 
//------------------------------------------------------------------------------
LatLonHgt::~LatLonHgt() 
{
}

//------------------------------------------------------------------------------
//  std::ostream& operator<<(std::ostream& output, LatLonHgt &llh)
//------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& output, LatLonHgt &llh)
{
    Rvector v(3, llh.latitude, llh.longitude, llh.height);

    output << v << llh.type << llh.hgtRef << std::endl;

    return output;
}

//------------------------------------------------------------------------------
//  <friend>
//  std::istream& operator>>(std::istream& input, LatLonHgt &llh)
//------------------------------------------------------------------------------
std::istream& operator>>(std::istream& input, LatLonHgt &llh)
{
    std::string tempString1, tempString2;
    input >> llh.latitude >> llh.longitude 
          >> llh.height >> tempString1 >> tempString2;
    llh.type = wxString::FromAscii(tempString1.c_str());
    llh.hgtRef = wxString::FromAscii(tempString2.c_str()); // ugly hack.  Should refactor to use wxInputStream

    return input;
}

//------------------------------------------------------------------------------
// Real LatLonHgt::GetLatitude() const
//------------------------------------------------------------------------------
Real LatLonHgt::GetLatitude() const
{
        return latitude;
}

//------------------------------------------------------------------------------
// void LatLonHgt::SetLatitude(const Real lat, const wxString typ) 
//------------------------------------------------------------------------------
void LatLonHgt::SetLatitude(const Real &lat, const wxString &typ)
{
        latitude = lat;
	SetType(typ);
}

//------------------------------------------------------------------------------
// Real LatLonHgt::GetLongitude() const
//------------------------------------------------------------------------------
Real LatLonHgt::GetLongitude() const
{
        return longitude;
}

//------------------------------------------------------------------------------
// void LatLonHgt::SetLongitude(const Real lon)
//------------------------------------------------------------------------------
void LatLonHgt::SetLongitude(const Real &lon) 
{
        longitude = lon;
}

//------------------------------------------------------------------------------
// Real LatLonHgt::GetHeight() const
//------------------------------------------------------------------------------
Real LatLonHgt::GetHeight() const
{
        return height;
}

//------------------------------------------------------------------------------
// void LatLonHgt::SetHeight(const Real hgt) 
//------------------------------------------------------------------------------
void LatLonHgt::SetHeight(const Real &hgt) 
{
        height = hgt;
}

//------------------------------------------------------------------------------
// wxString LatLonHgt::GetHeightRef) const
//------------------------------------------------------------------------------
wxString LatLonHgt::GetHeightRef() const
{
        return hgtRef;
}

//------------------------------------------------------------------------------
// void LatLonHgt::SetHeightRef(const wxString &hgtReference) 
//------------------------------------------------------------------------------
void LatLonHgt::SetHeightRef(const wxString &hgtReference) 
{
        hgtRef = hgtReference;
}

//------------------------------------------------------------------------------
// Real LatLonHgt::GetType() const
//------------------------------------------------------------------------------
wxString LatLonHgt::GetType() const
{
        return type;
}

//------------------------------------------------------------------------------
// void LatLonHgt::SetType(const wxString &typ) 
//------------------------------------------------------------------------------
void LatLonHgt::SetType(const wxString &typ) 
{
        type = typ;
}
  
//------------------------------------------------------------------------------
// Real LatLonHgt::CartesianToLatLonHgt(const Rvector3& position,
//                         const Real &equatorialRadius, const Real &flattening,
//			 const wxString typ)
//
// This routine will compute the geodetic latitude, longitude, and height
// above the reference ellipsoid for a space object in orbit.
//
//------------------------------------------------------------------------------
void LatLonHgt::CartesianToLatLonHgt(const Rvector3 &position,
                         const Real &equatorialRadius, const Real &flattening,
			 const wxString &typ, 
			 const wxString &hgtReference)
{

  Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening);
    
  // Get position (X, Y, Z)
  Real posX = position.Get(0); 
  Real posY = position.Get(1);
  Real posZ = position.Get(2);
  
  // Get longitude measured positive to the East of the Greenwich meridian 
  Real lon = GmatMathUtil::ATan2(posY,posX); 
  lon = GetDegree(lon,0.0,GmatMathConstants::TWO_PI);
  
  // Get geodetic latitude measured north from the equator
  Real rDeltaSat = GmatMathUtil::Sqrt(posX*posX+posY*posY);
  Real delta = GmatMathUtil::ATan2(posZ,rDeltaSat); 
  delta = GetDegree(delta,0.0,GmatMathConstants::TWO_PI);
  
  Real latGD = delta;
  Real latGDold = 0;
  Real tol = 1e-8;
  Real slat,clat,C;
  
  while (GmatMathUtil::Abs(latGD-latGDold) > tol) {
    
    latGDold = latGD;
    slat = GmatMathUtil::Sin(latGD);
    C = equatorialRadius/GmatMathUtil::Sqrt(1-eccentricity*eccentricity*slat*slat);
    latGD = GmatMathUtil::ATan2((posZ+C*eccentricity*eccentricity*GmatMathUtil::Sin(latGD)),rDeltaSat);

  }

  // Compute sin and cos of the geodetic latitude for convenience
  slat = GmatMathUtil::Sin(latGD);
  clat = GmatMathUtil::Cos(latGD);

  // C is the curvature in the meridian
  C = equatorialRadius/GmatMathUtil::Sqrt(1-eccentricity*eccentricity*slat*slat);

  // This is the height above the reference ellipsoid
  Real h_ellp = rDeltaSat/clat-C;

  Integer typeID = GetTypeID(typ);
  
  switch(typeID) {
    
    case GEODETIC_ID:

      SetLatitude(GmatMathUtil::Deg(latGD),typ);
      SetLongitude(lon);
      SetHeight(h_ellp);
      SetHeightRef(hgtReference);
      
      break;
      
    case GEOCENTRIC_ID:
       { 
         Real latGC = GeodeticToGeocentricLat(latGD, flattening);
         SetLatitude(GmatMathUtil::Deg(latGC),typ);
         SetLongitude(lon);
         SetHeight(h_ellp);
         SetHeightRef(hgtReference);
       }
       break;

    case REDUCED_ID:
       {
         Real latRD = GeodeticToReducedLat(latGD, flattening);
         SetLatitude(GmatMathUtil::Deg(latRD),typ);
         SetLongitude(lon);
         SetHeight(h_ellp);
         SetHeightRef(hgtReference);
       }
       break;

    default:

      UtilityException ex;
      
      ex.SetDetails(wxT("Undefined Latitude Type: %s"), type.c_str());
      throw ex;
      
  }  

}

//------------------------------------------------------------------------------
// Rvector LatLonHgt::GetSitePosition(const Real& equatorialRadiusconst Real& flattening)
// 
// These equations are exact for a site located on the reference ellipsoid
// where h_ellp is zero.
//
//------------------------------------------------------------------------------
Rvector3 LatLonHgt::GetSitePosition(const Real &equatorialRadius, const Real &flattening)
{

  Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening);
    
  // Convert latitude and longitude from degrees to radians
  Real lat  = GmatMathUtil::Rad( latitude ); 
  Real lon = GmatMathUtil::Rad( longitude );
  Real h_ellp = height;

  Real x,y,z,gdlat;
  Real clat,slat,clon,slon;
  Real C,S,r_delta,r_K;

  // Compute sin and cos of longitude for convenience
  clon = GmatMathUtil::Cos(lon);
  slon = GmatMathUtil::Sin(lon);

  Integer typeID = GetTypeID(type);
  
  switch (typeID) {
    
    case GEOCENTRIC_ID:
    
      // Compute geodetic latitude as a function of geocentric latitude
      gdlat = GeocentricToGeodeticLat(lat,flattening);

      // Compute sin and cos of geodetic latitude for convenience
      clat = GmatMathUtil::Cos(gdlat);
      slat = GmatMathUtil::Sin(gdlat);
     
      break;

    case GEODETIC_ID:

      // Compute geodetic sin and cos for convenience
      clat = GmatMathUtil::Cos(lat);
      slat = GmatMathUtil::Sin(lat);

      break;
      
    case REDUCED_ID:

      gdlat = ReducedToGeodeticLat(lat,flattening);
      
      // Compute sin and cos of geodetic latitude for convenience
      clat = GmatMathUtil::Cos(gdlat);
      slat = GmatMathUtil::Sin(gdlat);

      break;
      
    default:

      UtilityException ex;
      
      ex.SetDetails(wxT("Undefined Latitude Type: %s"), type.c_str());
      throw ex;
      
  }  

  // C is the radius of curvature in the meridian
  
  C = equatorialRadius/GmatMathUtil::Sqrt(1-eccentricity*eccentricity*slat*slat);
  S = C*(1-eccentricity*eccentricity);
  
  r_delta = (C+h_ellp)*clat;
  r_K = (S+h_ellp)*slat;
  
  // Calculate X, Y, and Z 
  x = r_delta*clon;
  y = r_delta*slon;
  z = r_K;
    
  // Return new position
  return Rvector3(x,y,z);

}

//------------------------------------------------------------------------------
// Integer LatLonHgt::GetNumData() const
//------------------------------------------------------------------------------
Integer LatLonHgt::GetNumData() const
{
   return NUM_DATA;
}

//------------------------------------------------------------------------------
// const wxString* LatLonHgt::GetDataDescriptions() const
//------------------------------------------------------------------------------
const wxString* LatLonHgt::GetDataDescriptions() const
{
   return DATA_DESCRIPTIONS;
}

//------------------------------------------------------------------------------
// const wxString* LatLonHgt::GetTypeDescriptions() const
//------------------------------------------------------------------------------
const wxString* LatLonHgt::GetTypeDescriptions() const
{
   return TYPE_DESCRIPTIONS;
}

//------------------------------------------------------------------------------
// const wxString* LatLonHgt::GetHeightDescriptions() const
//------------------------------------------------------------------------------
const wxString* LatLonHgt::GetHeightDescriptions() const
{
   return HEIGHT_DESCRIPTIONS;
}

//------------------------------------------------------------------------------
//  wxString* LatLonHgt::ToValueStrings(void)
//------------------------------------------------------------------------------
wxString* LatLonHgt::ToValueStrings(void)
{
   wxString ss(wxT(""));

   ss << GetLatitude();
   stringValues[0] = ss;
   
   ss.Clear();
   ss << GetLongitude();
   stringValues[1] = ss;
   
   ss.Clear();
   ss << GetHeight();
   stringValues[2] = ss;
   
   ss.Clear();
   ss << GetType();
   stringValues[3] = ss;
   
   return stringValues;
}

//------------------------------------------------------------------------------
// void LatLonHgt::GeocentricToReducedLat( const Real &flattening )
//------------------------------------------------------------------------------
void LatLonHgt::GeocentricToReducedLat( const Real &flattening )
{

  if (type == wxT("Geocentric")) {

    Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening); 
    Real lat = GmatMathUtil::Rad(latitude);
    Real rdlat = GmatMathUtil::ATan2(GmatMathUtil::Tan(lat),GmatMathUtil::Sqrt(1-eccentricity*eccentricity));
    
    SetLatitude(GmatMathUtil::Deg(rdlat),wxT("Reduced"));

  } else {

    UtilityException ex(wxString::FromAscii("GeocentricToReducedLat: Incorrect latitude type"));
    throw ex;
    
  }

}

//------------------------------------------------------------------------------
// void LatLonHgt::GeodeticToReducedLat( const Real &flattening )
//------------------------------------------------------------------------------
void LatLonHgt::GeodeticToReducedLat( const Real &flattening )
{

  if (type == wxT("Geodetic")) {

    Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening);
    Real lat = GmatMathUtil::Rad(latitude);
    Real rdlat = GmatMathUtil::ATan2(GmatMathUtil::Tan(lat)*GmatMathUtil::Sqrt(1-eccentricity*eccentricity),1);
    
    SetLatitude(GmatMathUtil::Deg(rdlat),wxT("Reduced"));

  } else {

    UtilityException ex(wxT("GeodeticToReducedLat: Incorrect latitude type"));
    throw ex;

  }
      
}

//------------------------------------------------------------------------------
// void LatLonHgt::ReducedToGeocentricLat( const Real &flattening )
//------------------------------------------------------------------------------
void LatLonHgt::ReducedToGeocentricLat( const Real &flattening )
{

  if (type == wxT("Reduced")) {

    Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening); 
    Real lat = GmatMathUtil::Rad(latitude);
    Real gclat =  GmatMathUtil::ATan2(GmatMathUtil::Tan(lat)*GmatMathUtil::Sqrt(1-eccentricity*eccentricity),1);
    
    SetLatitude(GmatMathUtil::Deg(gclat),wxT("Geocentric"));

  } else {

    UtilityException ex(wxT("ReducedToGeocentricLat: Incorrect latitude type"));
    throw ex;

  }    

}

//------------------------------------------------------------------------------
// void LatLonHgt::ReducedToGeodeticLat( const Real &flattening )
//------------------------------------------------------------------------------
void LatLonHgt::ReducedToGeodeticLat( const Real &flattening )
{

  if (type == wxT("Reduced")) {

    Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening);
    Real lat = GmatMathUtil::Rad(latitude);
    Real gdlat =  GmatMathUtil::ATan2(GmatMathUtil::Tan(lat),GmatMathUtil::Sqrt(1-eccentricity*eccentricity));
    
    SetLatitude(GmatMathUtil::Deg(gdlat),wxT("Geodetic"));

  } else {

    UtilityException ex(wxT("ReducedToGeodeticLat: Incorrect latitude type"));
    throw ex;

  }    
    
}

//------------------------------------------------------------------------------
// bool LatLonHgt::GeodeticToGeocentricLat( const Real &flattening )
//------------------------------------------------------------------------------
void LatLonHgt::GeodeticToGeocentricLat( const Real &flattening )
{

  if (type == wxT("Geodetic")) {

    Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening);
    Real lat = GmatMathUtil::Rad(latitude);
    Real gclat = GmatMathUtil::ATan2(GmatMathUtil::Tan(lat)*(1-eccentricity*eccentricity),1);
    
    SetLatitude(GmatMathUtil::Deg(gclat),wxT("Geocentric"));
    
  } else {
    
    UtilityException ex(wxT("GeodeticToGeocentricLat: Incorrect latitude type"));
    throw ex;

  }    
  
}

//------------------------------------------------------------------------------
// bool LatLonHgt::GeocentricToGeodeticLat( const Real &flattening )
//------------------------------------------------------------------------------
void LatLonHgt::GeocentricToGeodeticLat( const Real &flattening )
{

  if (type == wxT("Geocentric")) {

    Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening);
    Real lat = GmatMathUtil::Rad(latitude);
    Real gdlat = GmatMathUtil::ATan2(GmatMathUtil::Tan(lat),(1-eccentricity*eccentricity));
    
    SetLatitude(GmatMathUtil::Deg(gdlat),wxT("Geodetic"));
    
  } else {

    UtilityException ex(wxT("GeocentricToGeodeticLat: Incorrect latitude type"));
    throw ex;

  }    

}

//------------------------------------------------------------------------------
// Real LatLonHgt::GeodeticToGeocentricLat(const Real &gdlat, const Real &flattening)
//------------------------------------------------------------------------------
Real LatLonHgt::GeodeticToGeocentricLat(const Real &gdlat, const Real &flattening)
{

  Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening);
  return GmatMathUtil::ATan2(GmatMathUtil::Tan(gdlat)*(1-eccentricity*eccentricity),1);
  
}

//------------------------------------------------------------------------------
// Real LatLonHgt::GeocentricToGeodeticLat(const Real &gclat, const Real &flattening)
//------------------------------------------------------------------------------
Real LatLonHgt::GeocentricToGeodeticLat(const Real &gclat, const Real &flattening)
{

  Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening);
  return GmatMathUtil::ATan2(GmatMathUtil::Tan(gclat),(1-eccentricity*eccentricity));

}

//------------------------------------------------------------------------------
// Real LatLonHgt::GeocentricToReducedLat(const Real &gclat, const Real &flattening)
//------------------------------------------------------------------------------
Real LatLonHgt::GeocentricToReducedLat(const Real &gclat, const Real &flattening )
{

  Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening); 
  return GmatMathUtil::ATan2(GmatMathUtil::Tan(gclat),GmatMathUtil::Sqrt(1-eccentricity*eccentricity));
  
}

//------------------------------------------------------------------------------
// Real LatLonHgt::GeodeticToReducedLat(const Real &gdlat, const Real &flattening)
//------------------------------------------------------------------------------
Real LatLonHgt::GeodeticToReducedLat(const Real &gdlat, const Real &flattening )
{

  Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening);
  return GmatMathUtil::ATan2(GmatMathUtil::Tan(gdlat)*GmatMathUtil::Sqrt(1-eccentricity*eccentricity),1);
  
}

//------------------------------------------------------------------------------
// Real LatLonHgt::ReducedToGeocentricLat(const Real &rdlat, const Real &flattening)
//------------------------------------------------------------------------------
Real LatLonHgt::ReducedToGeocentricLat(const Real &rdlat, const Real &flattening )
{

  Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening); 
  return GmatMathUtil::ATan2(GmatMathUtil::Tan(rdlat)*GmatMathUtil::Sqrt(1-eccentricity*eccentricity),1);
  
}

//------------------------------------------------------------------------------
// Real LatLonHgt::ReducedToGeodeticLat(const Real &rdlat, const Real &flattening)
//------------------------------------------------------------------------------
Real LatLonHgt::ReducedToGeodeticLat(const Real &rdlat, const Real &flattening )
{

  Real eccentricity = GmatMathUtil::Sqrt(2*flattening-flattening*flattening);
  return GmatMathUtil::ATan2(GmatMathUtil::Tan(rdlat),GmatMathUtil::Sqrt(1-eccentricity*eccentricity));
  
}

//------------------------------------------------------------------------------
// Integer LatLonHgt::GetTypeID(const wxString &label)
//------------------------------------------------------------------------------
/**
 * Code used to obtain the type ID
 */
//------------------------------------------------------------------------------
Integer LatLonHgt::GetTypeID(const wxString &label)
{
   Integer retval = -1;
   
   if (label == wxT("Geocentric")) {
      return GEOCENTRIC_ID;
   } else if (label == wxT("Geodetic")) {
      return GEODETIC_ID;
   } else if (label == wxT("Reduced")) {
      return REDUCED_ID;
   } else
     return retval;

}

//------------------------------------------------------------------------------
// wxString LatLonHgt::GetTypeText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Code used to obtain the type text corresponding to a type ID
 */
//------------------------------------------------------------------------------
wxString LatLonHgt::GetTypeText(const Integer &id) const
{
   if ((id >= 0) && (id < EndTypeReps))
   {
      return TYPE_DESCRIPTIONS[id];
   }

   return wxT("INVALID");
}

//------------------------------------------------------------------------------
// Real LatLonHgt::GetDegree(const Real angle, const Real minAngle, 
//                           const Real maxAngle) 
//------------------------------------------------------------------------------
Real LatLonHgt::GetDegree(const Real angle, const Real minAngle, 
                          const Real maxAngle) 
{
   Real angleInRange = GmatMathUtil::Mod(angle,GmatMathConstants::TWO_PI);
   
   if (angleInRange < minAngle)
      angleInRange += GmatMathConstants::TWO_PI;

   else if (angleInRange > maxAngle)
      angleInRange -= GmatMathConstants::TWO_PI;

   return GmatMathUtil::Deg(angleInRange);
}

//------------------------------------------------------------------------------
// Integer LatLonHgt::GetTypeID(const wxString &label)
//------------------------------------------------------------------------------
/**
 * Code used to obtain the type ID
 */
//------------------------------------------------------------------------------
Integer LatLonHgt::GetHeightID(const wxString &label)
{
   Integer retval = -1;
   
   if (label == wxT("Ellipsoid")) {
      return ELLIPSOID_ID;
   } else if (label == wxT("Geoid")) {
      return GEOID_ID;
   } else if (label == wxT("MeanSeaLevel")) {
      return MEANSEALEVEL_ID;
   } else
     return retval;

}

//------------------------------------------------------------------------------
// wxString LatLonHgt::GetHeightText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Code used to obtain the type text corresponding to a type ID
 */
//------------------------------------------------------------------------------
wxString LatLonHgt::GetHeightText(const Integer &id) const
{
   if ((id >= 0) && (id < EndHeightReps))
   {
      return HEIGHT_DESCRIPTIONS[id];
   }

   return wxT("INVALID");
}
