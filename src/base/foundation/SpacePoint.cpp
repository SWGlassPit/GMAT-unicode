//$Id: SpacePoint.cpp 9809 2011-08-29 15:56:34Z wendys-dev $
//------------------------------------------------------------------------------
//                                  SpacePoint
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G and MOMS Task order 124
//
// Author: Wendy C. Shoan
// Created: 2004/12/07
//
/**
 * Implementation of the SpacePoint class.  This is the base class for SpaceObject
 * (base class for Spacecraft and Formation), CelestialBody (base class for Star,
 * Planet, and Moon), and CalculatedPoint (base class for LibrationPoint and
 * Barycenter)  - any object that can be used as an origin, primary, or
 * secondary when defining a CoordinateSystem.
 *
 */
//------------------------------------------------------------------------------

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "SpacePoint.hpp"
#include "A1Mjd.hpp"
#include "Rvector6.hpp"
#include "Rvector3.hpp"
#include "StringUtil.hpp"
#include "CoordinateConverter.hpp"      // for Convert()
#include "MessageInterface.hpp"
#ifdef __USE_SPICE__
   #include "SpiceInterface.hpp"
#endif


//#define DEBUG_J2000_STATE
//#define DEBUG_SPACE_POINT_CLOAKING
//#define DEBUG_SPICE_KERNEL
//#define DEBUG_SPACE_POINT_ORBIT_KERNELS
//#define DEBUG_ATTITUDE

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//---------------------------------
// static data
//---------------------------------

const wxString
SpacePoint::PARAMETER_TEXT[SpacePointParamCount - GmatBaseParamCount] =
{
   wxT("J2000BodyName"),
   wxT("NAIFId"),
   wxT("NAIFIdReferenceFrame"),
   wxT("OrbitSpiceKernelName"),
   wxT("AttitudeSpiceKernelName"),
   wxT("SCClockSpiceKernelName"),
   wxT("FrameSpiceKernelName"),
};

const Gmat::ParameterType
SpacePoint::PARAMETER_TYPE[SpacePointParamCount - GmatBaseParamCount] =
{
   Gmat::STRING_TYPE,       // wxT("J2000BodyName")
   Gmat::INTEGER_TYPE,      // wxT("NAIFId")
   Gmat::INTEGER_TYPE,      // wxT("NAIFIdReferenceFrame")
   Gmat::STRINGARRAY_TYPE,  // wxT("OrbitSpiceKernelName")
   Gmat::STRINGARRAY_TYPE,  // wxT("AttitudeSpiceKernelName")
   Gmat::STRINGARRAY_TYPE,  // wxT("SCClockSpiceKernelName")
   Gmat::STRINGARRAY_TYPE,  // wxT("FrameSpiceKernelName")
};

#ifdef __USE_SPICE__
   const Integer SpacePoint::UNDEFINED_NAIF_ID           = SpiceInterface::DEFAULT_NAIF_ID;
   const Integer SpacePoint::UNDEFINED_NAIF_ID_REF_FRAME = SpiceInterface::DEFAULT_NAIF_ID_REF_FRAME;
#else
   const Integer SpacePoint::UNDEFINED_NAIF_ID           = -123456789;
   const Integer SpacePoint::UNDEFINED_NAIF_ID_REF_FRAME = -123456789;
#endif


//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  SpacePoint(Gmat::ObjectType ofType, const wxString &itsType,
//             const wxString &itsName);
//---------------------------------------------------------------------------
/**
 * Constructs base SpacePoint structures used in derived classes
 * (default constructor).
 *
 * @param <ofType>  Gmat::ObjectTypes enumeration for the object.
 * @param <itsType> GMAT script string associated with this type of object.
 * @param <itsName> Optional name for the object.  Defaults to wxT("").
 *
 * @note There is no parameter free constructor for SpacePoint.  Derived 
 *       classes must pass in the typeId and typeStr parameters.
 */
//---------------------------------------------------------------------------
SpacePoint::SpacePoint(Gmat::ObjectType ofType, const wxString &itsType,
                       const wxString &itsName) :
GmatBase(ofType,itsType,itsName),
theSolarSystem (NULL),
inertialCS     (NULL),
bodyFixedCS    (NULL),
j2000Body      (NULL),
j2000BodyName  (wxT("Earth")),
naifId         (UNDEFINED_NAIF_ID),
naifIdRefFrame (UNDEFINED_NAIF_ID),
spiceSetupDone (false),
hasAttitude    (false)
{
   objectTypes.push_back(Gmat::SPACE_POINT);
   objectTypeNames.push_back(wxT("SpacePoint"));
   SaveAllAsDefault();
}

//---------------------------------------------------------------------------
//  SpacePoint(const SpacePoint &sp);
//---------------------------------------------------------------------------
/**
 * Constructs base SpacePoint structures used in derived classes, by copying 
 * the input instance (copy constructor).
 *
 * @param <sp>  SpacePoint instance to copy to create wxT("this") instance.
 */
//---------------------------------------------------------------------------
SpacePoint::SpacePoint(const SpacePoint &sp) :
GmatBase(sp),
theSolarSystem           (sp.theSolarSystem),
inertialCS               (NULL),
bodyFixedCS              (NULL),
j2000Body                (sp.j2000Body), //(NULL),
j2000BodyName            (sp.j2000BodyName),
naifId                   (sp.naifId),
naifIdRefFrame           (sp.naifIdRefFrame),
default_j2000BodyName    (sp.default_j2000BodyName),
default_naifId           (sp.default_naifId),
spiceSetupDone           (false),
orbitSpiceKernelNames    (sp.orbitSpiceKernelNames),
attitudeSpiceKernelNames (sp.attitudeSpiceKernelNames),
scClockSpiceKernelNames  (sp.scClockSpiceKernelNames),
frameSpiceKernelNames    (sp.frameSpiceKernelNames),
hasAttitude              (false)
{
}

//---------------------------------------------------------------------------
//  SpacePoint& operator=(const SpacePoint &sp)
//---------------------------------------------------------------------------
/**
 * Assignment operator for SpacePoint structures.
 *
 * @param <sp> The original that is being copied.
 *
 * @return Reference to this object
 */
//---------------------------------------------------------------------------
const SpacePoint& SpacePoint::operator=(const SpacePoint &sp)
{
   if (&sp == this)
      return *this;
   theSolarSystem           = sp.theSolarSystem;
   inertialCS               = NULL;
   bodyFixedCS              = NULL;
   j2000Body                = sp.j2000Body;
   j2000BodyName            = sp.j2000BodyName;
   naifId                   = sp.naifId;
   naifIdRefFrame           = sp.naifIdRefFrame;
   spiceSetupDone           = sp.spiceSetupDone;
   orbitSpiceKernelNames    = sp.orbitSpiceKernelNames;
   attitudeSpiceKernelNames = sp.attitudeSpiceKernelNames;
   scClockSpiceKernelNames  = sp.scClockSpiceKernelNames;
   frameSpiceKernelNames    = sp.frameSpiceKernelNames;
   default_j2000BodyName    = sp.default_j2000BodyName;
   default_naifId           = sp.default_naifId;
   hasAttitude              = false;
   
   return *this;
}
//---------------------------------------------------------------------------
//  ~SpacePoint(void)
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
//---------------------------------------------------------------------------
SpacePoint::~SpacePoint()
{
   if (inertialCS)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (inertialCS, inertialCS->GetTypeName(), wxT("SpacePoint::~SpacePoint()"),
          wxT("deleting inertialCS"));
      #endif
      delete inertialCS;
   }
   
   if (bodyFixedCS)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (bodyFixedCS, bodyFixedCS->GetTypeName(), wxT("SpacePoint::~SpacePoint()"),
          wxT("deleting bodyFixedCS"));
      #endif
      delete bodyFixedCS;
   }
}

//------------------------------------------------------------------------------
// virtual void SetSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
void SpacePoint::SetSolarSystem(SolarSystem *ss)
{
   theSolarSystem = ss;
}

//------------------------------------------------------------------------------
//  bool RequiresJ2000Body()
//------------------------------------------------------------------------------
/**
 * Identifies objects that need to have the J2000 body set in the Sandbox.
 *
 * @return true for all SpacePoint objects.
 */
//------------------------------------------------------------------------------
bool SpacePoint::RequiresJ2000Body()
{
   return true;
}


// methods for accessing the bodyName or body pointer
//------------------------------------------------------------------------------
//  const wxString  GetJ2000BodyName() const
//------------------------------------------------------------------------------
/**
 * This method returns the j2000 Body name.
 *
 * @return J2000 Body name.
 *
 */
//------------------------------------------------------------------------------
const wxString SpacePoint::GetJ2000BodyName() const
{
   if (j2000Body) return j2000Body->GetName();
   else           return j2000BodyName;
}

SpacePoint* SpacePoint::GetJ2000Body() const
{
   return j2000Body;
}

bool SpacePoint::SetJ2000BodyName(const wxString &toName)
{
   j2000BodyName = toName;
   return true;
}

void SpacePoint::SetJ2000Body(SpacePoint* toBody)
{
   #ifdef DEBUG_J2000_STATE
      if (toBody)
         MessageInterface::ShowMessage(wxT("Setting J2000 body to %s for %s\n"),
            toBody->GetName().c_str(), instanceName.c_str());
      else
         MessageInterface::ShowMessage(wxT("Setting J2000 body to NULL for %s\n"),
            instanceName.c_str());
   #endif
   
   j2000Body = toBody;

   #ifdef DEBUG_J2000_STATE
      if (j2000Body)
         MessageInterface::ShowMessage(wxT("J2000 body is now set\n"));
   #endif
}

bool SpacePoint::IsParameterCloaked(const Integer id) const
{
   #ifdef DEBUG_SPACE_POINT_CLOAKING
      MessageInterface::ShowMessage(wxT("In SpacePoint:IsParameterCloaked with id = %d %s)\n"),
            id, (GetParameterText(id)).c_str());
   #endif
   if (!cloaking) return false;
   if (id >= GmatBaseParamCount && id < SpacePointParamCount)
      return IsParameterEqualToDefault(id);
   
   return GmatBase::IsParameterCloaked(id);
}

bool SpacePoint::IsParameterEqualToDefault(const Integer id) const
{
   if (id == J2000_BODY_NAME)
   {
      return (j2000BodyName == default_j2000BodyName);
   }
   if (id == NAIF_ID)
   {
      return (default_naifId == naifId);
   }
   if (id == NAIF_ID_REFERENCE_FRAME)
   {
      return (default_naifIdRefFrame == naifIdRefFrame);
   }
   return GmatBase::IsParameterEqualToDefault(id);    
}

bool SpacePoint::SaveAllAsDefault()
{
   GmatBase::SaveAllAsDefault();
   default_j2000BodyName  = j2000BodyName;
   default_naifId         = naifId;
   default_naifIdRefFrame = naifIdRefFrame;
   return true;
}

bool SpacePoint::SaveParameterAsDefault(const Integer id)
{
   if (id == J2000_BODY_NAME)  
   {
      default_j2000BodyName = j2000BodyName;
      return true;
   }
   if (id == NAIF_ID)
   {
      default_naifId = naifId;
      return true;
   }
   if (id == NAIF_ID_REFERENCE_FRAME)
   {
      default_naifIdRefFrame = naifIdRefFrame;
      return true;
   }
   return GmatBase::SaveParameterAsDefault(id);
}


//------------------------------------------------------------------------------
//  virtual bool HasAttitude() const
//------------------------------------------------------------------------------
/**
 * @return true if attitude was computed or can be computed, false otherwise
 */
//------------------------------------------------------------------------------
bool SpacePoint::HasAttitude() const
{
   if (theSolarSystem != NULL && j2000Body != NULL)
      if (inertialCS == NULL && bodyFixedCS == NULL)
         return hasAttitude;
      else
         return true;
   else
      return false;
}


//------------------------------------------------------------------------------
// virtual const Rmatrix33& GetAttitude(Real a1mjdTime)
//------------------------------------------------------------------------------
/**
 * @return Computed cosine matrix if attitude can be computed for celestial body
 */
//------------------------------------------------------------------------------
const Rmatrix33& SpacePoint::GetAttitude(Real a1mjdTime)
{
   #ifdef DEBUG_ATTITUDE
   MessageInterface::ShowMessage
      (wxT("SpacePoint::GetAttitude() '%s' entered, epoch=%f, theSolarSystem=<%p>, ")
       wxT("j2000Body=<%p>\n   inertialCS=<%p>, bodyFixedCS=<%p>\n"), GetName().c_str(),
       a1mjdTime, theSolarSystem, j2000Body, inertialCS, bodyFixedCS);
   #endif
   
   // If not a celestial body, just return identity matrix
   if (!IsOfType(Gmat::CELESTIAL_BODY))
      return cosineMat;
   
   if (theSolarSystem != NULL && j2000Body != NULL)
   {
      Rvector6 currState = GetMJ2000State(a1mjdTime);
      Rvector6 outState;
      
      #ifdef DEBUG_ATTITUDE
      MessageInterface::ShowMessage
         (wxT("   currState=%s\n"), currState.ToString(16).c_str());
      #endif
      
      if (inertialCS == NULL && bodyFixedCS == NULL)
      {
         inertialCS =
            CoordinateSystem::
            CreateLocalCoordinateSystem(wxT("Sp_Inertial"), wxT("MJ2000Eq"), this,
                                        NULL, NULL, j2000Body, theSolarSystem);
         bodyFixedCS =
            CoordinateSystem::
            CreateLocalCoordinateSystem(wxT("Sp_BodyFixed"), wxT("BodyFixed"), this,
                                        NULL, NULL, j2000Body, theSolarSystem);
         #ifdef DEBUG_ATTITUDE
         MessageInterface::ShowMessage
            (wxT("   inertialCS=<%p>, bodyFixedCS=<%p>\n"), inertialCS, bodyFixedCS);
         #endif
         
         // if coordinate systems are still NULL, just return identity matrix
         if (inertialCS == NULL || bodyFixedCS == NULL)
            return cosineMat;
      }
      
      CoordinateConverter coordConverter;
      // The Attitude matrix is rotation matrix from inertial to body fixed,
      // but coordConverter.GetLastRotationMatrix() returns rotation matrix to inertial
      // so convert body fixed to inertial
      coordConverter.Convert(a1mjdTime, currState, bodyFixedCS, outState,
                             inertialCS, false, false);
      Rmatrix33 rotMat = coordConverter.GetLastRotationMatrix();
      
      #ifdef DEBUG_ATTITUDE
      MessageInterface::ShowMessage
         (wxT("   outState =%s\n"), outState.ToString(16).c_str());
      MessageInterface::ShowMessage
         (wxT("   rotMat   =\n%s"), rotMat.ToString(16, 25, false, wxT("      ")).c_str());
      #endif
      
      cosineMat = rotMat;
      hasAttitude = true;
   }
   else
   {
      hasAttitude = false;
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** SpacePoint::GetAttitude() Cannot compute attitude at epoch %f, ")
          wxT("SolarSystem or J2000Body is NULL\n"), a1mjdTime);
   }
   
   return cosineMat;
}


const Rvector3 SpacePoint::GetMJ2000Acceleration(const A1Mjd &atTime)
{
   return Rvector3(0.0,0.0,0.0);
}

void SpacePoint::RemoveSpiceKernelName(const wxString &kernelType,
                                       const wxString &fileName)
{
   if (kernelType == wxT("Orbit"))
   {
      StringArray::iterator i;
      i = find(orbitSpiceKernelNames.begin(), orbitSpiceKernelNames.end(), fileName);
      if (i != orbitSpiceKernelNames.end())  orbitSpiceKernelNames.erase(i);
   }
   else if (kernelType == wxT("Attitude"))
   {
      StringArray::iterator i;
      i = find(attitudeSpiceKernelNames.begin(), attitudeSpiceKernelNames.end(), fileName);
      if (i != attitudeSpiceKernelNames.end())  attitudeSpiceKernelNames.erase(i);
   }
   else if (kernelType == wxT("SCClock"))
   {
      StringArray::iterator i;
      i = find(scClockSpiceKernelNames.begin(), scClockSpiceKernelNames.end(), fileName);
      if (i != scClockSpiceKernelNames.end())  scClockSpiceKernelNames.erase(i);
   }
   else if (kernelType == wxT("Frame"))
   {
      StringArray::iterator i;
      i = find(frameSpiceKernelNames.begin(), frameSpiceKernelNames.end(), fileName);
      if (i != frameSpiceKernelNames.end())  frameSpiceKernelNames.erase(i);
   }
}



//------------------------------------------------------------------------------
// public methods inherited from GmatBase
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  wxString  GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter text, given the input parameter ID.
 *
 * @param <id> Id for the requested parameter text.
 *
 * @return parameter text for the requested parameter.
 *
 */
//------------------------------------------------------------------------------
wxString SpacePoint::GetParameterText(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < SpacePointParamCount)
      return PARAMETER_TEXT[id - GmatBaseParamCount];
   return GmatBase::GetParameterText(id);
}

//------------------------------------------------------------------------------
//  Integer  GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter ID, given the input parameter string.
 *
 * @param <str> string for the requested parameter.
 *
 * @return ID for the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Integer SpacePoint::GetParameterID(const wxString &str) const
{
   for (Integer i = GmatBaseParamCount; i < SpacePointParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatBaseParamCount])
         return i;
   }
   
   return GmatBase::GetParameterID(str);
}

//------------------------------------------------------------------------------
//  Gmat::ParameterType  GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type, given the input parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return parameter type of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Gmat::ParameterType SpacePoint::GetParameterType(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < SpacePointParamCount)
      return PARAMETER_TYPE[id - GmatBaseParamCount];
   
   return GmatBase::GetParameterType(id);
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
 *
 */
//------------------------------------------------------------------------------
wxString SpacePoint::GetParameterTypeString(const Integer id) const
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
bool SpacePoint::IsParameterReadOnly(const Integer id) const
{
   if (id == J2000_BODY_NAME)
      return true;
   if (id == NAIF_ID) // set to false in appropriate derived classes
      return true;
   if (id == NAIF_ID_REFERENCE_FRAME) // set to false in appropriate derived classes
      return true;

   // Turn off parameters for string arrays if they are empty
   if (id == ORBIT_SPICE_KERNEL_NAME)
      if (orbitSpiceKernelNames.size() == 0)
         return true;
   if (id == ATTITUDE_SPICE_KERNEL_NAME)
      if (attitudeSpiceKernelNames.size() == 0)
         return true;
   if (id == SC_CLOCK_SPICE_KERNEL_NAME)
      if (scClockSpiceKernelNames.size() == 0)
         return true;
   if (id == FRAME_SPICE_KERNEL_NAME)
      if (frameSpiceKernelNames.size() == 0)
         return true;

   return GmatBase::IsParameterReadOnly(id);
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
bool SpacePoint::IsParameterReadOnly(const wxString &label) const
{
   return IsParameterReadOnly(GetParameterID(label));
}

//------------------------------------------------------------------------------
//  Integer  GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the Integer parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return  Integer value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Integer SpacePoint::GetIntegerParameter(const Integer id) const
{
   if (id == NAIF_ID)                 return naifId;
   if (id == NAIF_ID_REFERENCE_FRAME) return naifIdRefFrame;

   return GmatBase::GetIntegerParameter(id);
}


//---------------------------------------------------------------------------
//  Integer GetIntegerParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//---------------------------------------------------------------------------
Integer SpacePoint::GetIntegerParameter(const wxString &label) const
{
   return GetIntegerParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
//  Integer  SetIntegerParameter(const Integer id, const Integer value)
//------------------------------------------------------------------------------
/**
 * This method sets the Integer parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 * @param <value> Integer value for the requested parameter.
 *
 * @return  Integer value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Integer SpacePoint::SetIntegerParameter(const Integer id,
                                        const Integer value)
{
   #ifdef DEBUG_SPICE_KERNEL
      MessageInterface::ShowMessage(wxT("SpacePoint: setting %s to %d\n"),
            GetParameterText(id).c_str(), value);
   #endif
   if (id == NAIF_ID)
   {
      naifId              = value;
      return true;
   }
   if (id == NAIF_ID_REFERENCE_FRAME)
   {
      naifIdRefFrame      = value;
      return true;
   }

   return GmatBase::SetIntegerParameter(id,value);
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
 *
 */
//------------------------------------------------------------------------------
wxString SpacePoint::GetStringParameter(const Integer id) const
{
   if (id == J2000_BODY_NAME)   
   {
      if (j2000Body) return j2000Body->GetName();
      else           return j2000BodyName;
   }

   // return entire brace-enclosed array for the kernel name arrays
   // (needed so that assignments will work inside of GmatFunctions) wcs 2010.05.19
   wxString kernelArrayString = wxT("{");
   if (id == ORBIT_SPICE_KERNEL_NAME)
   {
      for (unsigned int ii = 0; ii < orbitSpiceKernelNames.size(); ii++)
      {
         if (ii != 0) kernelArrayString += wxT(",");
         kernelArrayString              += orbitSpiceKernelNames.at(ii);
      }
      kernelArrayString += wxT("}");
      return kernelArrayString;
   }
   if (id == ATTITUDE_SPICE_KERNEL_NAME)
   {
      for (unsigned int ii = 0; ii < attitudeSpiceKernelNames.size(); ii++)
      {
         if (ii != 0) kernelArrayString += wxT(",");
         kernelArrayString              += attitudeSpiceKernelNames.at(ii);
      }
      kernelArrayString += wxT("}");
      return kernelArrayString;
   }
   if (id == SC_CLOCK_SPICE_KERNEL_NAME)
   {
      for (unsigned int ii = 0; ii < scClockSpiceKernelNames.size(); ii++)
      {
         if (ii != 0) kernelArrayString += wxT(",");
         kernelArrayString              += scClockSpiceKernelNames.at(ii);
      }
      kernelArrayString += wxT("}");
      return kernelArrayString;
   }
   if (id == FRAME_SPICE_KERNEL_NAME)
   {
      for (unsigned int ii = 0; ii < frameSpiceKernelNames.size(); ii++)
      {
         if (ii != 0) kernelArrayString += wxT(",");
         kernelArrayString              += frameSpiceKernelNames.at(ii);
      }
      kernelArrayString += wxT("}");
      return kernelArrayString;
   }

   return GmatBase::GetStringParameter(id);
}

//------------------------------------------------------------------------------
//  wxString  SetStringParameter(const Integer id, const wxString value)
//------------------------------------------------------------------------------
/**
 * This method sets the string parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 * @param <value> string value for the requested parameter.
 *
 * @return  success flag.
 *
 */
//------------------------------------------------------------------------------

 bool SpacePoint::SetStringParameter(const Integer id, 
                                    const wxString &value)
{
   if (id == J2000_BODY_NAME) 
   {
      j2000BodyName = value; 
      return true;
   }
   
//   bool alreadyInList = false;
   if (id == ORBIT_SPICE_KERNEL_NAME)
   {
      #ifdef DEBUG_SPACE_POINT_ORBIT_KERNELS
          MessageInterface::ShowMessage(wxT("Entering SP::SetString with value = %s\n"),
             value.c_str());
      #endif
      // if it is the whole StringArray of kernel names, handle that here
      // (needed so that assignments will work inside of GmatFunctions) wcs 2010.05.18
      wxString value1 = GmatStringUtil::Trim(value);
      if (GmatStringUtil::IsEnclosedWithBraces(value1))
      {
         orbitSpiceKernelNames.clear();
         orbitSpiceKernelNames = GmatStringUtil::ToStringArray(value1);
         #ifdef DEBUG_SPACE_POINT_ORBIT_KERNELS
             MessageInterface::ShowMessage(wxT("In SP::SetString value IS enclosed with braces and values are:\n"));
             for (unsigned int ii = 0; ii < orbitSpiceKernelNames.size(); ii++)
                MessageInterface::ShowMessage(wxT("   (%d)   %s\n"), (Integer) ii,
                      (orbitSpiceKernelNames.at(ii)).c_str());
         #endif
      }
      else
      {
         if (find(orbitSpiceKernelNames.begin(), orbitSpiceKernelNames.end(),
             value) == orbitSpiceKernelNames.end())
            orbitSpiceKernelNames.push_back(value);
      }
      spiceSetupDone = false;
      return true;
   }
   if (id == ATTITUDE_SPICE_KERNEL_NAME)
   {
      // if it is the whole StringArray of kernel names, handle that here
      // (needed so that assignments will work inside of GmatFunctions) wcs 2010.05.18
      wxString value1 = GmatStringUtil::Trim(value);
      if (GmatStringUtil::IsEnclosedWithBraces(value1))
      {
         attitudeSpiceKernelNames.clear();
         attitudeSpiceKernelNames = GmatStringUtil::ToStringArray(value1);
      }
      else
      {
         if (find(attitudeSpiceKernelNames.begin(), attitudeSpiceKernelNames.end(),
             value) == attitudeSpiceKernelNames.end())
            attitudeSpiceKernelNames.push_back(value);
     }
      spiceSetupDone = false;
      return true;
   }
   if (id == SC_CLOCK_SPICE_KERNEL_NAME)
   {
      // if it is the whole StringArray of kernel names, handle that here
      // (needed so that assignments will work inside of GmatFunctions) wcs 2010.05.18
      wxString value1 = GmatStringUtil::Trim(value);
      if (GmatStringUtil::IsEnclosedWithBraces(value1))
      {
         scClockSpiceKernelNames.clear();
         scClockSpiceKernelNames = GmatStringUtil::ToStringArray(value1);
      }
      else
      {
         if (find(scClockSpiceKernelNames.begin(), scClockSpiceKernelNames.end(),
             value) == scClockSpiceKernelNames.end())
            scClockSpiceKernelNames.push_back(value);
      }
      spiceSetupDone = false;
      return true;
   }
   if (id == FRAME_SPICE_KERNEL_NAME)
   {
      // if it is the whole StringArray of kernel names, handle that here
      // (needed so that assignments will work inside of GmatFunctions) wcs 2010.05.18
      wxString value1 = GmatStringUtil::Trim(value);
      if (GmatStringUtil::IsEnclosedWithBraces(value1))
      {
         frameSpiceKernelNames.clear();
         frameSpiceKernelNames = GmatStringUtil::ToStringArray(value1);
      }
      else
      {
         if (find(frameSpiceKernelNames.begin(), frameSpiceKernelNames.end(),
             value) == frameSpiceKernelNames.end())
            frameSpiceKernelNames.push_back(value);
      }
      spiceSetupDone = false;
      return true;
   }


   return GmatBase::SetStringParameter(id, value);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Accessor method used to get a parameter value
 *
 * @param    <label>  label ID for the parameter
 *
 * @return the value of the parameter
 */
//------------------------------------------------------------------------------
wxString SpacePoint::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}

//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
/**
* Accessor method used to get a parameter value
 *
 * @param    <label> Integer ID for the parameter
 * @param    <value> The new value for the parameter
 */
//------------------------------------------------------------------------------

bool SpacePoint::SetStringParameter(const wxString &label,
                                       const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}

//---------------------------------------------------------------------------
//  const StringArray& GetStringArrayParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Accesses lists of names and other StringArray parameters.
 *
 * @param id The integer ID for the parameter.
 *
 * @return The requested StringArray; throws if the parameter is not a
 *         StringArray.
 */
//---------------------------------------------------------------------------
const StringArray& SpacePoint::GetStringArrayParameter(const Integer id) const
{
   if (id == ORBIT_SPICE_KERNEL_NAME)
      return orbitSpiceKernelNames;
   if (id == ATTITUDE_SPICE_KERNEL_NAME)
      return attitudeSpiceKernelNames;
   if (id == SC_CLOCK_SPICE_KERNEL_NAME)
      return scClockSpiceKernelNames;
   if (id == FRAME_SPICE_KERNEL_NAME)
      return frameSpiceKernelNames;
   return GmatBase::GetStringArrayParameter(id);
}

const StringArray& SpacePoint::GetStringArrayParameter(const wxString &label) const
{
   return GetStringArrayParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
//  GmatBase* GetRefObject(const Gmat::ObjectType type,
//                         const wxString &name)
//------------------------------------------------------------------------------
/**
 * This method returns a reference object from the SpacePoint class.
 *
 * @param <type>  type of the reference object requested
 * @param <name>  name of the reference object requested
 *
 * @return pointer to the reference object requested.
 *
 */
//------------------------------------------------------------------------------
GmatBase* SpacePoint::GetRefObject(const Gmat::ObjectType type,
                                   const wxString &name)
{
   switch (type)
   {
      case Gmat::SPACE_POINT:
         return j2000Body;
      default:
         break;
   }
   
   // Not handled here -- invoke the next higher GetRefObject call
   return GmatBase::GetRefObject(type, name);
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
 *
 */
//------------------------------------------------------------------------------
bool SpacePoint::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                              const wxString &name)
{
   
   switch (type)
   {
      case Gmat::SPACE_POINT:
      {
         j2000Body = (SpacePoint*) obj;
         return true;
      }
      default:
         break;
   }
   
   // Not handled here -- invoke the next higher SetRefObject call
   return GmatBase::SetRefObject(obj, type, name);
}


// DJC Added, 12/16/04
// This seems like it should NOT be needed, but GCC seems to be confused about 
// the overloaded versions of the following six methods:
// WCS modified 2010.05.04 - added string arrays for kernel names, so some of these
// need to be implemented

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
wxString SpacePoint::GetStringParameter(const Integer id,
                                           const Integer index) const
{
   switch (id)
   {
   case ORBIT_SPICE_KERNEL_NAME:
      {
         if ((index >= 0) && (index < (Integer) orbitSpiceKernelNames.size()))
            return orbitSpiceKernelNames[index];
         else
            throw GmatBaseException(wxT("Index into array of SPK kernels is out-of-bounds.\n"));
      }
   case ATTITUDE_SPICE_KERNEL_NAME:
      {
         if ((index >= 0) && (index < (Integer) attitudeSpiceKernelNames.size()))
            return attitudeSpiceKernelNames[index];
         else
            throw GmatBaseException(wxT("Index into array of CK kernels is out-of-bounds.\n"));
     }
   case SC_CLOCK_SPICE_KERNEL_NAME:
      {
         if ((index >= 0) && (index < (Integer) scClockSpiceKernelNames.size()))
            return scClockSpiceKernelNames[index];
         else
            throw GmatBaseException(wxT("Index into array of SCLK kernels is out-of-bounds.\n"));
      }
   case FRAME_SPICE_KERNEL_NAME:
      {
         if ((index >= 0) && (index < (Integer) frameSpiceKernelNames.size()))
            return frameSpiceKernelNames[index];
         else
            throw GmatBaseException(wxT("Index into array of FK kernels is out-of-bounds.\n"));
     }

      default:
         break;
   }
   return GmatBase::GetStringParameter(id, index);
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
bool SpacePoint::SetStringParameter(const Integer id, 
                                    const wxString &value, 
                                    const Integer index)
{
#ifdef DEBUG_SPICE_KERNEL
   MessageInterface::ShowMessage(
         wxT("Entering SpacePoint::SetStringParameter with id = %d, value = %s, and index = %d\n"),
         id, value.c_str(), index);
#endif
   if (index < 0)
   {
      GmatBaseException ex;
      ex.SetDetails(wxT("The index %d is out-of-range for field \"%s\""), index,
                    GetParameterText(id).c_str());
      throw ex;
   }
    switch (id)
   {
      case ORBIT_SPICE_KERNEL_NAME:
      {
         if (index < (Integer)orbitSpiceKernelNames.size())
            orbitSpiceKernelNames[index] = value;
         // Only add the orbit spice kernel name if it is not in the list already
         else if (find(orbitSpiceKernelNames.begin(), orbitSpiceKernelNames.end(),
               value) == orbitSpiceKernelNames.end())
            orbitSpiceKernelNames.push_back(value);
         spiceSetupDone = false;
         return true;
      }
   case ATTITUDE_SPICE_KERNEL_NAME:
      {
         if (index < (Integer)attitudeSpiceKernelNames.size())
            attitudeSpiceKernelNames[index] = value;
         // Only add the orbit spice kernel name if it is not in the list already
         else if (find(attitudeSpiceKernelNames.begin(), attitudeSpiceKernelNames.end(),
               value) == attitudeSpiceKernelNames.end())
            attitudeSpiceKernelNames.push_back(value);
         spiceSetupDone = false;
         return true;
     }
   case SC_CLOCK_SPICE_KERNEL_NAME:
      {
         if (index < (Integer)scClockSpiceKernelNames.size())
            scClockSpiceKernelNames[index] = value;
         // Only add the orbit spice kernel name if it is not in the list already
         else if (find(scClockSpiceKernelNames.begin(), scClockSpiceKernelNames.end(),
               value) == scClockSpiceKernelNames.end())
            scClockSpiceKernelNames.push_back(value);
         spiceSetupDone = false;
         return true;
      }
   case FRAME_SPICE_KERNEL_NAME:
      {
         if (index < (Integer)frameSpiceKernelNames.size())
            frameSpiceKernelNames[index] = value;
         // Only add the orbit spice kernel name if it is not in the list already
         else if (find(frameSpiceKernelNames.begin(), frameSpiceKernelNames.end(),
               value) == frameSpiceKernelNames.end())
            frameSpiceKernelNames.push_back(value);
         spiceSetupDone = false;
         return true;
     }

      default:
         break;
   }

   return GmatBase::SetStringParameter(id, value, index);
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
wxString SpacePoint::GetStringParameter(const wxString &label, 
                                           const Integer index) const
{
   Integer id = GetParameterID(label);
   return GetStringParameter(id, index);
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
bool SpacePoint::SetStringParameter(const wxString &label, 
                                    const wxString &value, 
                                    const Integer index)
{
   Integer id = GetParameterID(label);
   return SetStringParameter(id, value, index);
}


//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name, 
//                        const Integer index)
//------------------------------------------------------------------------------
/**
 * This method returns a pointer to a reference object contained in a vector of
 * objects in the SpacePoint class.
 *
 * @param type type of the reference object requested
 * @param name name of the reference object requested
 * @param index index for the particular object requested.
 *
 * @return pointer to the reference object requested.
 */
//------------------------------------------------------------------------------
GmatBase* SpacePoint::GetRefObject(const Gmat::ObjectType type,
                                   const wxString &name, 
                                   const Integer index)
{
   return GmatBase::GetRefObject(type, name, index);
}


//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name, const Integer index)
//------------------------------------------------------------------------------
/**
 * This method sets a pointer to a reference object in a vector of objects in 
 * the SpacePoint class.
 *
 * @param obj The reference object.
 * @param type type of the reference object requested
 * @param name name of the reference object requested
 * @param index index for the particular object requested.
 *
 * @return true if successful; otherwise, false.
 */
//------------------------------------------------------------------------------
bool SpacePoint::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                              const wxString &name, const Integer index)
{
   return GmatBase::SetRefObject(obj, type, name, index);
}
