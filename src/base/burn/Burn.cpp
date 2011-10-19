//$Id: Burn.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                   Burn
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P.
//
// Author: Darrel J. Conway, Thinking Systems, Inc.
// Created: 2003/12/17
//
/**
 * Defines the Burn base class used for impulsive and finite maneuvers. 
 */
//------------------------------------------------------------------------------


#include "Burn.hpp"
#include "BurnException.hpp"
#include "ObjectReferencedAxes.hpp"
#include "MessageInterface.hpp"
#include <algorithm>                    // for find()

//#define DEBUG_BURN_PARAM
//#define DEBUG_BURN_GET
//#define DEBUG_BURN_SET
//#define DEBUG_BURN_INIT
//#define DEBUG_BURN_CONVERT

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//---------------------------------
// static data
//---------------------------------

/// Available local axes labels
StringArray Burn::localAxesLabels;

/// Labels used for the thruster element parameters.
const wxString
Burn::PARAMETER_TEXT[BurnParamCount - GmatBaseParamCount] =
{
   wxT("CoordinateSystem"),
   wxT("Origin"),
   wxT("Axes"),
   wxT("VectorFormat"), // deprecated
   wxT("Element1"),
   wxT("Element2"),
   wxT("Element3"),
   wxT("SpacecraftName"),
};

/// Types of the parameters used by thrusters.
const Gmat::ParameterType
Burn::PARAMETER_TYPE[BurnParamCount - GmatBaseParamCount] =
{
   Gmat::OBJECT_TYPE,         // "CoordinateSystem",
   Gmat::OBJECT_TYPE,         // "Origin",
   Gmat::ENUMERATION_TYPE,    // "Axes",
   Gmat::ENUMERATION_TYPE,    // "VectorFormat", // deprecated
   Gmat::REAL_TYPE,           // "Element1",
   Gmat::REAL_TYPE,           // "Element2",
   Gmat::REAL_TYPE,           // "Element3",
   Gmat::OBJECT_TYPE,         // "SpacecraftName",
};


//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
//  Burn(wxString typeStr, wxString nomme)
//------------------------------------------------------------------------------
/**
 * Constructs the Burn object (default constructor) with default VNB Local
 * CoordinateSystem.
 * 
 * @param <type>    Gmat::ObjectTypes enumeration for the object.
 * @param <typeStr> String text identifying the object type
 * @param <nomme>   Name for the object
 *
 * @Note coordSystem, localOrigin, spacecraft, solarSystem, and j2000Body are
 *       set through SetRefObject() during Sandbox initialization.
 *       localCoordSystem is created during initialization or when new
 *       spacecraft is set
 */
//------------------------------------------------------------------------------
Burn::Burn(Gmat::ObjectType type, const wxString &typeStr,
           const wxString &nomme) :
   GmatBase             (type, typeStr, nomme),
   solarSystem          (NULL),
   localCoordSystem     (NULL),
   coordSystem          (NULL),
   localOrigin          (NULL),
   j2000Body            (NULL),
   spacecraft           (NULL),
   coordSystemName      (wxT("Local")),
   localOriginName      (wxT("Earth")),
   localAxesName        (wxT("VNB")),
   j2000BodyName        (wxT("Earth")),
   satName              (wxT("")),
   usingLocalCoordSys   (true),
   isMJ2000EqAxes       (false),
   isSpacecraftBodyAxes (false),
   initialized          (false)
{
   objectTypes.push_back(Gmat::BURN);
   objectTypeNames.push_back(wxT("Burn"));
   parameterCount = BurnParamCount;
   
   deltaV[0] = deltaV[1] = deltaV[2] = 0.0;
   deltaVInertial[0] = deltaVInertial[1] = deltaVInertial[2] = 0.0;
   
   frameBasis[0][0] = frameBasis[1][1] = frameBasis[2][2] = 1.0;
   frameBasis[0][1] = frameBasis[1][0] = frameBasis[2][0] =
   frameBasis[0][2] = frameBasis[1][2] = frameBasis[2][1] = 0.0;
      
   // Available local axes labels
   // Since it is static data, clear it first
   localAxesLabels.clear();
   localAxesLabels.push_back(wxT("VNB"));
   localAxesLabels.push_back(wxT("LVLH"));
   localAxesLabels.push_back(wxT("MJ2000Eq"));
   localAxesLabels.push_back(wxT("SpacecraftBody"));
}


//------------------------------------------------------------------------------
//  ~Burn()
//------------------------------------------------------------------------------
/**
 * Destroys the Burn object (destructor).
 */
//------------------------------------------------------------------------------
Burn::~Burn()
{   
   if (usingLocalCoordSys && localCoordSystem)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (localCoordSystem, wxT("localCoordSystem"), wxT("Burn::~Burn()"),
          wxT("deleting localCoordSystem"));
      #endif
      delete localCoordSystem;
      localCoordSystem = NULL;
   }
}


//------------------------------------------------------------------------------
//  Burn(const Burn &b)
//------------------------------------------------------------------------------
/**
 * Constructs the Burn object (copy constructor).
 * 
 * @param <b> Object that is copied
 *
 * @Note coordSystem, localOrigin, spacecraft, solarSystem, and j2000Body are
 *       set through SetRefObject() during Sandbox initialization.
 *       localCoordSystem is created during initialization or when new
 *       spacecraft is set
 */
//------------------------------------------------------------------------------
Burn::Burn(const Burn &b) :
   GmatBase             (b),
   solarSystem          (b.solarSystem),
   localCoordSystem     (NULL),
   coordSystem          (b.coordSystem),
   localOrigin          (b.localOrigin),
   j2000Body            (b.j2000Body),
   spacecraft           (NULL),
   coordSystemName      (b.coordSystemName),
   localOriginName      (b.localOriginName),
   localAxesName        (b.localAxesName),
   j2000BodyName        (b.j2000BodyName),
   satName              (b.satName),
   vectorFormat         (b.vectorFormat),
   usingLocalCoordSys   (b.usingLocalCoordSys),
   isMJ2000EqAxes       (b.isMJ2000EqAxes),
   isSpacecraftBodyAxes (b.isSpacecraftBodyAxes),
   initialized          (false)
{
   deltaV[0] = b.deltaV[0];
   deltaV[1] = b.deltaV[1];
   deltaV[2] = b.deltaV[2];
   
   deltaVInertial[0] = 0.0;
   deltaVInertial[1] = 0.0;
   deltaVInertial[2] = 0.0;
   
   for (Integer i = 0; i < 3; i++)
      for (Integer j = 0; j < 3; j++)
         frameBasis[i][j]  = b.frameBasis[i][j];
}


//------------------------------------------------------------------------------
//  Burn& operator=(const Burn &b)
//------------------------------------------------------------------------------
/**
 * Sets one burn object to match another (assignment operator).
 * 
 * @param <b> The object that is copied.
 * 
 * @return this object, with the parameters set as needed.
 *
 * @Note coordSystem, localOrigin, spacecraft, solarSystem, and j2000Body are
 *       set through SetRefObject() during Sandbox initialization.
 *       localCoordSystem is created during initialization or when new
 *       spacecraft is set
 */
//------------------------------------------------------------------------------
Burn& Burn::operator=(const Burn &b)
{
   if (this == &b)
      return *this;
   
   GmatBase::operator=(b);
   
   solarSystem        = b.solarSystem;
   localCoordSystem   = NULL;
   coordSystem        = b.coordSystem;
   localOrigin        = b.localOrigin;
   j2000Body          = b.j2000Body;
   spacecraft         = NULL;
   coordSystemName    = b.coordSystemName;
   localOriginName    = b.localOriginName;
   localAxesName      = b.localAxesName;
   j2000BodyName      = b.j2000BodyName;
   satName            = b.satName;
   vectorFormat       = b.vectorFormat;
   usingLocalCoordSys = b.usingLocalCoordSys;
   localAxesLabels    = b.localAxesLabels;
   initialized        = false;
   
   deltaV[0] = b.deltaV[0];
   deltaV[1] = b.deltaV[1];
   deltaV[2] = b.deltaV[2];
   
   deltaVInertial[0] = 0.0;
   deltaVInertial[1] = 0.0;
   deltaVInertial[2] = 0.0;
   
   for (Integer i = 0; i < 3; i++)
      for (Integer j = 0; j < 3; j++)
         frameBasis[i][j]  = b.frameBasis[i][j];
   
   return *this;
}


//------------------------------------------------------------------------------
// bool IsUsingLocalCoordSystem()
//------------------------------------------------------------------------------
bool Burn::IsUsingLocalCoordSystem()
{
   return usingLocalCoordSys;
}


//------------------------------------------------------------------------------
//  wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Gets the name of the parameter with the input id.
 * 
 * @param <id> Integer id for the parameter.
 * 
 * @return The string name of the parameter.
 */
//------------------------------------------------------------------------------
wxString Burn::GetParameterText(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < BurnParamCount)
      return PARAMETER_TEXT[id - GmatBaseParamCount];
   return GmatBase::GetParameterText(id);
}


//------------------------------------------------------------------------------
//  Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * Gets the id corresponding to a named parameter.
 * 
 * @param <str> Name of the parameter.
 * 
 * @return The ID.
 */
//------------------------------------------------------------------------------
Integer Burn::GetParameterID(const wxString &str) const
{
   static bool vectorFormatFirstWarning = true;
   static bool vFirstWarning = true;
   static bool nFirstWarning = true;
   static bool bFirstWarning = true;
   
   #ifdef DEBUG_BURN_PARAM
   MessageInterface::ShowMessage
      (wxT("Burn::GetParameterID() str=%s\n"), str.c_str());
   #endif
   
   Integer id = -1;
   
   if (str == wxT("VectorFormat"))
   {
      if (vectorFormatFirstWarning)
      {
         MessageInterface::ShowMessage
            (wxT("*** WARNING *** \"VectorFormat\" field of Burn ")
             wxT("is deprecated and will be removed from a future build.\n"));
         vectorFormatFirstWarning = false;
      }
      return VECTORFORMAT;
   }
   
   if (str == wxT("V"))
   {
      if (vFirstWarning)
      {
         MessageInterface::ShowMessage
            (wxT("*** WARNING *** \"V\" field of Burn is deprecated and will be ")
             wxT("removed from a future build; please use \"Element1\" instead.\n"));
         vFirstWarning = false;
      }
      return DELTAV1;
   }
   
   if (str == wxT("N"))
   {
      if (nFirstWarning)
      {
         MessageInterface::ShowMessage
            (wxT("*** WARNING *** \"N\" field of Burn is deprecated and will be ")
             wxT("removed from a future build; please use \"Element2\" instead.\n"));
         nFirstWarning = false;
      }
      return DELTAV2;
   }
   
   if (str == wxT("B"))
   {
      if (bFirstWarning)
      {
         MessageInterface::ShowMessage
            (wxT("*** WARNING *** \"B\" field of Burn is deprecated and will be ")
             wxT("removed from a future build; please use \"Element3\" instead.\n"));
         bFirstWarning = false;
      }
      return DELTAV3;
   }
   
   for (Integer i = GmatBaseParamCount; i < BurnParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatBaseParamCount])
      {
         id = i;
         break;
      }
   }
   
   if (id != -1)
   {
      #ifdef DEBUG_BURN_PARAM
      MessageInterface::ShowMessage(wxT("Burn::GetParameterID() returning %d\n"), id);
      #endif
      
      return id;
   }
   
   return GmatBase::GetParameterID(str);
}


//------------------------------------------------------------------------------
//  Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Gets the type of a parameter.
 * 
 * @param <id> Integer ID of the parameter.
 * 
 * @return The type of the parameter.
 */
//------------------------------------------------------------------------------
Gmat::ParameterType Burn::GetParameterType(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < BurnParamCount)
      return PARAMETER_TYPE[id - GmatBaseParamCount];
      
   return GmatBase::GetParameterType(id);
}


//------------------------------------------------------------------------------
//  wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Gets the text description for the type of a parameter.
 * 
 * @param <id> Integer ID of the parameter.
 * 
 * @return The text description of the type of the parameter.
 */
//------------------------------------------------------------------------------
wxString Burn::GetParameterTypeString(const Integer id) const
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
bool Burn::IsParameterReadOnly(const Integer id) const
{
   if (id == SATNAME)
      return true;
   
   if (id == VECTORFORMAT)
      return true;
   
   if ((id == BURNORIGIN || id == BURNAXES))
      if (coordSystemName != wxT("Local"))
         return true;
   
   return GmatBase::IsParameterReadOnly(id);
}


//------------------------------------------------------------------------------
//  Real GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Gets the value for a Real parameter.
 * 
 * @param <id> Integer ID of the parameter.
 * 
 * @return The value of the parameter.
 */
//------------------------------------------------------------------------------
Real Burn::GetRealParameter(const Integer id) const
{
   #ifdef DEBUG_BURN_GET
   MessageInterface::ShowMessage(wxT("Burn::GetRealParameter() id=%d\n"), id);
   #endif
   
   if (id == DELTAV1)
      return deltaV[0];
   
   if (id == DELTAV2)
      return deltaV[1];
   
   if (id == DELTAV3)
      return deltaV[2];
   
   return GmatBase::GetRealParameter(id);
}


//------------------------------------------------------------------------------
//  Real SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * Sets the value for a Real parameter.
 * 
 * @param <id>    Integer ID of the parameter.
 * @param <value> New value for the parameter.
 * 
 * @return The value of the parameter.
 */
//------------------------------------------------------------------------------
Real Burn::SetRealParameter(const Integer id, const Real value)
{
   #ifdef DEBUG_BURN_SET
   MessageInterface::ShowMessage
      (wxT("Burn::SetRealParameter() id=%d, value=%f\n"), id, value);
   #endif
   
   if (id == DELTAV1)
   {
      deltaV[0] = value;
      return deltaV[0];
   }
   
   if (id == DELTAV2)
   {
      deltaV[1] = value;
      return deltaV[1];
   }
   
   if (id == DELTAV3)
   {
      deltaV[2] = value;
      return deltaV[2];
   }
   
   return GmatBase::SetRealParameter(id, value);
}


//------------------------------------------------------------------------------
//  wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Gets the value for a wxString parameter.
 * 
 * @param <id> Integer ID of the parameter.
 * 
 * @return The value of the parameter.
 */
//------------------------------------------------------------------------------
wxString Burn::GetStringParameter(const Integer id) const
{
   if (id == BURNORIGIN)
      return localOriginName;
   
   if (id == BURNAXES)
      return localAxesName;
   
   if (id == COORDINATESYSTEM)
      return coordSystemName;
   
   if (id == SATNAME)
      return satName;
   
   if (id == VECTORFORMAT) // deprecated
      return vectorFormat;
   
   return GmatBase::GetStringParameter(id);
}


//------------------------------------------------------------------------------
//  bool SetStringParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * Sets the value for a wxString parameter.
 * 
 * @param <id>    Integer ID of the parameter.
 * @param <value> New value for the parameter.
 * 
 * @return The value of the parameter.
 */
//------------------------------------------------------------------------------
bool Burn::SetStringParameter(const Integer id, const wxString &value)
{
   #ifdef DEBUG_BURN_SET
   MessageInterface::ShowMessage
      (wxT("Burn::SetStringParameter() this=<%p> '%s', id=%d, value='%s'\n"), this,
       GetName().c_str(), id, value.c_str());
   #endif
   
   switch (id)
   {
   case COORDINATESYSTEM:
      coordSystemName = value;
      if (coordSystemName == wxT("Local"))
         usingLocalCoordSys = true;
      else
         usingLocalCoordSys = false;
      return true;
   case BURNORIGIN:
      localOriginName = value;
      #ifdef DEBUG_BURN_SET
      MessageInterface::ShowMessage
         (wxT("Burn::SetStringParameter() exiting, localOriginName set to '%s'\n"),
          value.c_str());
      #endif
      return true;
   case BURNAXES:
      {
         localAxesName = value;
         
         // Do we need to determin Local CS here?
         // Yes, old ImpulsiveBurn script doesn't have CoordinateSystem field, 
         // so Axes should be used to determine Local CS or not
         if (find(localAxesLabels.begin(), localAxesLabels.end(), localAxesName)
             != localAxesLabels.end())
         {
            #ifdef DEBUG_BURN_SET
            MessageInterface::ShowMessage
               (wxT("   Local axes '%s' found, so setting coordSystemName to Local\n"),
                localAxesName.c_str());
            #endif
            if (usingLocalCoordSys)
               coordSystemName = wxT("Local");
         }
         else
         {
            // write one warning per GMAT session
            static bool firstTimeWarning = true;
            wxString framelist = localAxesLabels[0];
            for (UnsignedInt n = 1; n < localAxesLabels.size(); ++n)
               framelist += wxT(", ") + localAxesLabels[n];
            
            wxString msg =
               wxT("The value of \"") + value + wxT("\" for field \"Axes\"")
               wxT(" on object \"") + instanceName + wxT("\" is not an allowed value.\n")
               wxT("The allowed values are: [ ") + framelist + wxT(" ]. ");
            
            if (firstTimeWarning)
            {
               firstTimeWarning = false;
               
               if (value == wxT("Inertial"))
                  MessageInterface::ShowMessage(wxT("*** WARNING *** ") + msg + wxT("\n"));
               else
                  throw BurnException(msg);
            }
            
            if (value == wxT("Inertial"))
            {
               coordSystemName = wxT("EarthMJ2000Eq");
               usingLocalCoordSys = false;
            }
            else
               throw BurnException(msg);
            
         }
         
         return true;
      }
   case VECTORFORMAT: // deprecated
      vectorFormat = value;
      return true;
   default:
      break;
   }
   
   return GmatBase::SetStringParameter(id, value);
}


//------------------------------------------------------------------------------
//  bool SetStringParameter(const Integer id, const Real value,
//                          const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets the value for a specific wxString element in an array.
 *
 * @param <id>    Integer ID of the parameter.
 * @param <value> New value for the parameter.
 * @param <index> Index for the element
 *
 * @return true on success
 */
//------------------------------------------------------------------------------
bool Burn::SetStringParameter(const Integer id, const wxString &value,
                              const Integer index)
{
   return GmatBase::SetStringParameter(id, value, index);
}


//------------------------------------------------------------------------------
//  const StringArray& GetPropertyEnumStrings(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Access an array of enumerated string data.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The requested StringArray; throws if the parameter is not a 
 *         StringArray.
 */
//------------------------------------------------------------------------------
const StringArray& Burn::GetPropertyEnumStrings(const Integer id) const
{
   if (id == BURNAXES)
      return localAxesLabels;
   
   return GmatBase::GetPropertyEnumStrings(id);
}


//------------------------------------------------------------------------------
//  const StringArray& GetPropertyEnumStrings(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Access an array of enumerated string data.
 *
 * @param <label> The parameter name.
 *
 * @return The requested StringArray
 */
//------------------------------------------------------------------------------
const StringArray& Burn::GetPropertyEnumStrings(const wxString &label) const
{
   return GetPropertyEnumStrings(GetParameterID(label));
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
const ObjectTypeArray& Burn::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::SPACE_POINT);
   // Spacecraft is not known until Maneuver or BeginFiniteBurn
   //refObjectTypes.push_back(Gmat::SPACECRAFT);
   if (!usingLocalCoordSys)
      refObjectTypes.push_back(Gmat::COORDINATE_SYSTEM);
   return refObjectTypes;
}


//------------------------------------------------------------------------------
// virtual const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
const StringArray& Burn::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   refObjectNames.clear();
   
   if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::SPACE_POINT)
      refObjectNames.push_back(localOriginName);
   
   // Spacecraft is not known until Maneuver or BeginFiniteBurn
   //if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::SPACECRAFT)
   //   refObjectNames.push_back(satName);
   
   if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::COORDINATE_SYSTEM)
      if (!usingLocalCoordSys)
         refObjectNames.push_back(coordSystemName);
   
   return refObjectNames;
}


//------------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                    const wxString &name)
//------------------------------------------------------------------------------
/**
* This method sets a reference object for the CoordinateSystem class.
 *
 * @param obj   pointer to the reference object
 * @param type  type of the reference object 
 * @param name  name of the reference object
 *
 * @return true if successful; otherwise, false.
 *
 */
//------------------------------------------------------------------------------
bool Burn::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                        const wxString &name)
{
   #ifdef DEBUG_BURN_SET
   MessageInterface::ShowMessage
      (wxT("Burn::SetRefObject() this=<%p> '%s', objType=%d, objTypeName=%s, ")
       wxT("objName=%s, type=%d, name=%s\n"), this, GetName().c_str(), obj->GetType(),
       obj->GetTypeName().c_str(), obj->GetName().c_str(), type, name.c_str());
   #endif
   
   switch (type)
   {
   case Gmat::COORDINATE_SYSTEM:
      {
         if (coordSystemName == name)
            coordSystem = (CoordinateSystem*)obj;
         
         return true;
      }
   case Gmat::SPACE_POINT:
   case Gmat::CELESTIAL_BODY:
      {
         // Set j2000Body here for now(loj: 2008.11.13)
         //@todo We need to make sure SetSolarSystem() is called during object
         // initialization in ObjectInitializer
         // localOriginName is set through SetStringParameter()
         //localOriginName = obj->GetName();
         if (localOriginName == obj->GetName())
            localOrigin = (SpacePoint*)obj;
         if (j2000BodyName == obj->GetName())
            j2000Body = (CelestialBody*)obj;
         return true;
      }
   case Gmat::SPACECRAFT:
      {
         satName = obj->GetName();
         spacecraft = (Spacecraft*)obj;
         return true;
      }
   default:
      return GmatBase::SetRefObject(obj, type, name);
   }
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
/**
 * Renames reference object name used in this class.
 *
 * @param <type> reference object type.
 * @param <oldName> object name to be renamed.
 * @param <newName> new object name.
 * 
 * @return true if object name changed, false if not.
 */
//---------------------------------------------------------------------------
bool Burn::RenameRefObject(const Gmat::ObjectType type,
                           const wxString &oldName,
                           const wxString &newName)
{
   if (type == Gmat::SPACECRAFT)
   {
      if (satName == oldName)
         satName = newName;
   }
   
   return true;
}


//------------------------------------------------------------------------------
//  void SetSpacecraftToManeuver(Spacecraft *sat)
//------------------------------------------------------------------------------
/**
 * Accessor method used by Maneuver to pass in the spacecraft pointer
 * 
 * @param <sat> the Spacecraft
 */
//------------------------------------------------------------------------------
void Burn::SetSpacecraftToManeuver(Spacecraft *sat)
{
   if (sat == NULL)
      return;
   
   #ifdef DEBUG_BURN_SET
   MessageInterface::ShowMessage
      (wxT("Burn::SetSpacecraftToManeuver() sat=<%p>'%s', usingLocalCoordSys=%d, ")
       wxT("localCoordSystem=<%p>\n"), sat, sat->GetName().c_str(), usingLocalCoordSys,
       localCoordSystem);
   #endif
   
   // If spcecraft is different, create new local coordinate system
   if (spacecraft != sat)
   {
      spacecraft = sat;
      satName = spacecraft->GetName();
      
      if (usingLocalCoordSys)
      {
         if (localCoordSystem)
         {
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Remove
               (localCoordSystem, wxT("localCoordSystem"), wxT("Burn::SetSpacecraftToManeuver()"),
                wxT("deleting localCoordSystem"));
            #endif
            delete localCoordSystem;
            localCoordSystem = NULL;
         }
         localCoordSystem = CreateLocalCoordinateSystem();
      }
   }
   
   #ifdef DEBUG_BURN_SET
   MessageInterface::ShowMessage(wxT("Burn::SetSpacecraftToManeuver() returning\n"));
   #endif
}


//------------------------------------------------------------------------------
//  void SetSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 * Sets the internal solar system pointer for objects that have one.
 *
 * @param ss   The solar system.
 */
//------------------------------------------------------------------------------
void Burn::SetSolarSystem(SolarSystem *ss)
{
   #ifdef DEBUG_BURN_SET
   MessageInterface::ShowMessage
      (wxT("Burn::SetSolarSystem() ss=<%p> '%s'\n"), ss, ss->GetName().c_str());
   #endif
   SolarSystem *oldSS = solarSystem;
   solarSystem = ss;
   
   if (solarSystem != oldSS)
      Initialize();
}


//------------------------------------------------------------------------------
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * Sets up the bodies used in the burn calculations.
 */
//------------------------------------------------------------------------------
bool Burn::Initialize()
{
   #ifdef DEBUG_BURN_INIT
   MessageInterface::ShowMessage
      (wxT("Burn::Initialize() <%p>'%s' entered, spacecraft=<%p>\n"), this,
       GetName().c_str(), spacecraft);
   #endif
   
   bool retval = GmatBase::Initialize();
   
   if (retval)
   {
      if ((!solarSystem))
         throw BurnException(wxT("Unable to initialize the burn object \"") + 
            instanceName + wxT("\"; the SolarSystem was not set."));
      
      j2000Body = solarSystem->GetBody(j2000BodyName);
      if (!localOrigin)
         localOrigin = solarSystem->GetBody(localOriginName);
      
      if ((!localOrigin) || (!j2000Body))
         throw BurnException(wxT("Unable to initialize the burn object ") + 
            instanceName + wxT("; either ") + j2000BodyName + wxT(" or ") + 
            localOriginName + wxT(" was not set for the burn."));
   }
   
   // delete old local coordinate system
   if (usingLocalCoordSys && localCoordSystem != NULL)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (localCoordSystem, wxT("localCoordSystem"), wxT("Burn::Initialize()"),
          wxT("deleting localCoordSystem"));
      #endif
      delete localCoordSystem;
      localCoordSystem = NULL;
   }
   
   // If spacecraft is available, create new local coordinate system
   if (usingLocalCoordSys && spacecraft != NULL)
      localCoordSystem = CreateLocalCoordinateSystem();
   
   if (usingLocalCoordSys && localCoordSystem == NULL)
      retval = false;
   
   #ifdef DEBUG_BURN_INIT
   MessageInterface::ShowMessage
      (wxT("Burn::Initialize() <%p>'%s' returning %d, localCoordSystem=<%p>\n"),
       this, GetName().c_str(), retval, localCoordSystem);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// CoordinateSystem* CreateLocalCoordinateSystem()
//------------------------------------------------------------------------------
CoordinateSystem* Burn::CreateLocalCoordinateSystem()
{
   #ifdef DEBUG_BURN_INIT
   MessageInterface::ShowMessage
      (wxT("Burn::CreateLocalCoordinateSystem() '%s' entered, usingLocalCoordSys=%d, ")
       wxT("spacecraft=<%p>, solarSystem=<%p>\n"), GetName().c_str(), usingLocalCoordSys,
       spacecraft, solarSystem);
   #endif
   
   // Why solarSystem gets NULL when running MMS script?
   // Added a check here for now (LOJ: 2009.04.22)
   if (solarSystem == NULL)
   {
      #ifdef DEBUG_BURN_INIT
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** Burn::CreateLocalCoordinateSystem() Unable to create local ")
          wxT("coordiante system, SolarSystem is NULL\n"));
      #endif
      throw BurnException
         (wxT("*** WARNING *** Burn::CreateLocalCoordinateSystem() Unable to create ")
          wxT("local coordiante system, SolarSystem is NULL\n"));
   }
   
   CoordinateSystem *localCS = NULL;
   
   // If coordinate system being used is local, then create
   if (usingLocalCoordSys)
   {
      if (spacecraft == NULL)
      {
         // Since spacecraft is set later, just return NULL for now
         #ifdef DEBUG_BURN_INIT
         MessageInterface::ShowMessage
            (wxT("Burn::CreateLocalCoordinateSystem() spacecraft is not set so, ")
             wxT("returning NULL\n"));
         #endif
         return NULL;
         //throw BurnException("Unable to initialize the Burn object " + 
         //   instanceName + " " + satName + " was not set for the burn.");
      }
      
      // Call CoordinateSystem static method to create a local coordinate system
      localOrigin = solarSystem->GetBody(localOriginName);
      localCS = CoordinateSystem::CreateLocalCoordinateSystem
         (wxT("Local"), localAxesName, spacecraft, localOrigin, spacecraft,
          j2000Body, solarSystem);
      
      if (localCS == NULL)
         return NULL;
      
      if (localAxesName == wxT("MJ2000Eq"))
         isMJ2000EqAxes = true;
      else if (localAxesName == wxT("SpacecraftBody"))
         isSpacecraftBodyAxes = true;
      
   }
   else
   {
      // If not using local cooordinate system, then it is using configured CS and
      // it should have been set by this time
      if (coordSystem)
      {
         throw BurnException
            (wxT("Unable to initialize the Burn object ") + 
             instanceName + wxT(" ") + coordSystemName + wxT(" was not set for the burn."));
      }
      localCS = coordSystem;
   }
   
   #ifdef DEBUG_BURN_INIT
   MessageInterface::ShowMessage
      (wxT("Burn::CreateLocalCoordinateSystem() returning <%p>\n"), localCS);
   #endif
   
   return localCS;
}


//------------------------------------------------------------------------------
// void ConvertDeltaVToInertial(Real *dv, Real *dvInertial, Real epoch)
//------------------------------------------------------------------------------
void Burn::ConvertDeltaVToInertial(Real *dv, Real *dvInertial, Real epoch)
{
   #ifdef DEBUG_BURN_CONVERT
   MessageInterface::ShowMessage
      (wxT("Burn::ConvertDeltaVToInertial(), usingLocalCoordSys=%d, coordSystemName='%s', ")
       wxT("coordSystem=<%p>'%s'\n"), usingLocalCoordSys, coordSystemName.c_str(),
       coordSystem, coordSystem ? coordSystem->GetName().c_str() : "NULL");
   #endif
   
   if (usingLocalCoordSys && localCoordSystem == NULL)
   {      
      throw BurnException
         (wxT("Unable to convert burn elements to Inertial, the local Coordinate ")
          wxT("System has not been created"));
   }
   else if (!usingLocalCoordSys && coordSystem == NULL)
   {
      throw BurnException
         (wxT("Unable to convert burn elements to Inertial, the Coordinate ")
          wxT("System has not been set"));      
   }
   
   Real inDeltaV[6], outDeltaV[6];
   for (Integer i=0; i<3; i++)
      inDeltaV[i] = dv[i];
   for (Integer i=3; i<6; i++)
      inDeltaV[i] = 0.0;
   
   // if not using local CS, use ref CoordinateSystem
   if (!usingLocalCoordSys)
   {     
      // Now rotate to MJ2000Eq axes, we don't want to translate so
      // set coincident to true
      coordSystem->ToMJ2000Eq(epoch, inDeltaV, outDeltaV, true);
      
      #ifdef DEBUG_BURN_CONVERT_ROTMAT
      Rmatrix33 rotMat = coordSystem->GetLastRotationMatrix();
      MessageInterface::ShowMessage
         (wxT("rotMat=\n%s\n"), rotMat.ToString(16, 20).c_str());
      #endif
      
      dvInertial[0] = outDeltaV[0];
      dvInertial[1] = outDeltaV[1];
      dvInertial[2] = outDeltaV[2];
   }
   else
   {
      // if MJ2000Eq axes rotation matrix is always identity matrix
      if (isMJ2000EqAxes)
      {
         dvInertial[0] = dv[0];
         dvInertial[1] = dv[1];
         dvInertial[2] = dv[2];
      }
      else if (isSpacecraftBodyAxes)
      {
         Rvector3 inDeltaV(dv[0], dv[1], dv[2]);
         Rvector3 outDeltaV;
         // Get attitude matrix from Spacecraft and transpose since
         // attitude matrix from spacecraft gives rotation matrix from
         // inertial to body
         Rmatrix33 inertialToBody = spacecraft->GetAttitude(epoch);
         Rmatrix33 rotMat = inertialToBody.Transpose();
         outDeltaV = inDeltaV * rotMat;
         for (Integer i=0; i<3; i++)
            dvInertial[i] = outDeltaV[i];
      }
      else
      {         
         // Now rotate to MJ2000Eq axes
         localCoordSystem->ToMJ2000Eq(epoch, inDeltaV, outDeltaV, true);

         dvInertial[0] = outDeltaV[0];
         dvInertial[1] = outDeltaV[1];
         dvInertial[2] = outDeltaV[2];
      }
   }
   
   #ifdef DEBUG_BURN_CONVERT
   MessageInterface::ShowMessage
      (wxT("Burn::ConvertDeltaVToInertial() returning\n")
       wxT("           dv = %f %f %f\n   dvInertial = %f %f %f\n"),
       dv[0], dv[1], dv[2], dvInertial[0], dvInertial[1], dvInertial[2]);
   #endif
}


//------------------------------------------------------------------------------
//  void TransformJ2kToBurnOrigin(const Real *scState, Real *state, Real epoch)
//------------------------------------------------------------------------------
/**
 * Resets the state to use the origin specified for the Burn.
 *
 * @param scState The input spacecraft state.
 * @param state   The transformed state.
 * @param epoch   The epoch of the input (and output) state.
 */
//------------------------------------------------------------------------------
void Burn::TransformJ2kToBurnOrigin(const Real *scState, Real *state, 
                                    Real epoch)
{
   #ifdef DEBUG_BURN_ORIGIN
      MessageInterface::ShowMessage(
         wxT("State transformation for Burn\n")
         wxT("   Input state =  [ %lf %lf %lf %.9lf %.9lf %.9lf ]\n"), scState[0], 
         scState[1], scState[2], scState[3], scState[4], scState[5]);
   #endif
      
   if ((j2000Body == NULL) || (localOrigin == NULL))
      Initialize();
   
   memcpy(state, scState, 6*sizeof(Real));
   if (j2000Body != localOrigin)
   {
      Rvector6 j2kState = j2000Body->GetMJ2000State(epoch);
      Rvector6 originState = localOrigin->GetMJ2000State(epoch);
      Rvector6 delta = j2kState - originState;
      
      #ifdef DEBUG_BURN_ORIGIN
         MessageInterface::ShowMessage(
            wxT("   j2000       =  [ %lf %lf %lf %.9lf %.9lf %.9lf ]\n"),
            j2kState[0], j2kState[1], j2kState[2], j2kState[3], j2kState[4], 
            j2kState[5]);
         MessageInterface::ShowMessage(
            wxT("   Origin      =  [ %lf %lf %lf %.9lf %.9lf %.9lf ]\n"),
            originState[0], originState[1], originState[2], originState[3], 
            originState[4], originState[5]);
         MessageInterface::ShowMessage(
            wxT("   Delta       =  [ %lf %lf %lf %.9lf %.9lf %.9lf ]\n"),
            delta[0], delta[1], delta[2], delta[3], delta[4], delta[5]);
      #endif
      
      state[0] += delta[0];
      state[1] += delta[1];
      state[2] += delta[2];
      state[3] += delta[3];
      state[4] += delta[4];
      state[5] += delta[5];
   }
   
   #ifdef DEBUG_BURN_ORIGIN
      MessageInterface::ShowMessage(
         wxT("   Output state = [ %lf %lf %lf %.9lf %.9lf %.9lf ]\n"),
         state[0], state[1], state[2], state[3], state[4], state[5]);
   #endif
}
