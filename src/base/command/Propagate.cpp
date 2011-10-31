//$Id: Propagate.cpp 9789 2011-08-23 02:10:15Z djcinsb $
//------------------------------------------------------------------------------
//                                 Propagate
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
// Author: Darrel J. Conway
// Created: 2003/09/23
//
/**
 * Implementation for the Propagate command class
 */
//------------------------------------------------------------------------------

#include "Propagate.hpp"

#include "Propagator.hpp"
#include "ODEModel.hpp"

#include "Publisher.hpp"
#include "Parameter.hpp"
#include "TextParser.hpp" // for SeparateBrackets()
#include "StringUtil.hpp" // for Trim()
#include "AngleUtil.hpp"  // for PutAngleInDegRange()
#include "MessageInterface.hpp"

#include <sstream>
#include <cmath>

//#define DEBUG_PROPAGATE_ASSEMBLE
//#define DEBUG_PARSING
//#define DEBUG_PROPAGATE_OBJ 1
//#define DEBUG_PROPAGATE_INIT 1
//#define DEBUG_PROPAGATE_DIRECTION 1
//#define DEBUG_PROPAGATE_STEPSIZE 1
//#define DEBUG_PROPAGATE_EXE 1
//#define DEBUG_STOPPING_CONDITIONS 1
//#define DEBUG_FIRST_STEP_STOP 1
//#define DEBUG_FINITE_MANEUVER
//#define DEBUG_RENAME
//#define DEBUG_PROP_PERFORMANCE
//#define DEBUG_FIRST_CALL
//#define DEBUG_FIXED_STEP
//#define DEBUG_EPOCH_UPDATES
//#define DEBUG_EPOCH_SYNC
//#define DEBUG_SECANT_DETAILS
//#define DEBUG_BISECTION_DETAILS
//#define DEBUG_WRAPPERS
//#define DEBUG_PUBLISH_DATA
//#define DEBUG_TRANSIENT_FORCES
//#define DEBUG_FINAL_STEP

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//---------------------------------
// static data
//---------------------------------
wxString Propagate::PropModeList[PropModeCount] =
{
   wxT(""), wxT("Synchronized"), wxT("BackProp")
};

const wxString
Propagate::PARAMETER_TEXT[PropagateCommandParamCount - GmatCommandParamCount] =
{
   wxT("AvailablePropModes"),
   wxT("PropagateMode"),
   wxT("InterruptFrequency"),
   wxT("StopTolerance"),
   wxT("Spacecraft"),
   wxT("Propagator"),
   wxT("StopCondition"),
   wxT("PropForward")
};

const Gmat::ParameterType
Propagate::PARAMETER_TYPE[PropagateCommandParamCount - GmatCommandParamCount] =
{
   Gmat::STRINGARRAY_TYPE,
   Gmat::STRING_TYPE,
   Gmat::INTEGER_TYPE,
   Gmat::REAL_TYPE,
   Gmat::STRINGARRAY_TYPE,
   Gmat::STRING_TYPE,
   Gmat::STRINGARRAY_TYPE,
   Gmat::BOOLEAN_TYPE
};

#ifdef DUMP_PLANET_DATA
   std::ofstream Propagate::planetData;

   CelestialBody* Propagate::body[11] =
   {
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
   };

   Integer Propagate::bodiesDefined = 0;
#endif

#ifdef DEBUG_FIRST_CALL
static bool firstStepFired = false;
#endif

//---------------------------------
// public members
//---------------------------------

//------------------------------------------------------------------------------
//  Propagate()
//------------------------------------------------------------------------------
/**
 * Constructs the Propagate Command (default constructor).
 */
//------------------------------------------------------------------------------
Propagate::Propagate() :
   PropagationEnabledCommand   (wxT("Propagate")),
   direction                   (1.0),
   currentPropMode             (wxT("")),
   interruptCheckFrequency     (30),
   inProgress                  (false),
   hasFired                    (false),
   epochID                     (-1),
   stopInterval                (0.0),
   stopTrigger                 (-1),
   state                       (NULL),
   pubdata                     (NULL),
   stopCondMet                 (false),
   stopEpoch                   (0.0),
   stopAccuracy                (DEFAULT_STOP_TOLERANCE),
   timeAccuracy                (1.0e-6),
   dim                         (0),
   cartDim                     (0),
   singleStepMode              (false),
   currentMode                 (INDEPENDENT),
   stopCondEpochID             (-1),
   stopCondBaseEpochID         (-1),
   stopCondStopVarID           (-1)
{
   stepBrackets[0] = stepBrackets[1] = 0.0;
   parameterCount = PropagateCommandParamCount;

   firstStepTolerance = DEFAULT_STOP_TOLERANCE * 10.0;

   #ifdef DUMP_PLANET_DATA
      if (!planetData.is_open())
      {
         planetData.open(wxT("PlanetaryEphem.csv"));
         planetData.precision(18);
      }
   #endif
}


//------------------------------------------------------------------------------
//  ~Propagate()
//------------------------------------------------------------------------------
/**
 * Destroys the Propagate Command.
 */
//------------------------------------------------------------------------------
Propagate::~Propagate()
{
   EmptyBuffer();

   for (UnsignedInt i=0; i<stopWhen.size(); i++)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (stopWhen[i], stopWhen[i]->GetName(), wxT("Propagate::~Propagate()"),
          wxT("deleting stop condition"));
      #endif
      delete stopWhen[i];
   }

   if (pubdata)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (pubdata, wxT("pubdata"), wxT("Propagate::~Propagate()"),
          wxT("deleting pub data"));
      #endif
      delete [] pubdata;
   }

   for (std::vector<PropSetup*>::iterator ps = prop.begin(); ps != prop.end();
        ++ps)
   {
      PropSetup *oldPs = *ps;
      *ps = NULL;
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (oldPs, oldPs->GetName(), wxT("Propagate::~Propagate()"),
          wxT("deleting old PropSetup"));
      #endif
      delete oldPs;
   }

   for (std::vector<StringArray*>::iterator i = satName.begin();
        i != satName.end(); ++i)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         ((*i), wxT("satName"), wxT("Propagate::~Propagate()"),
          wxT("deleting sat names associated with PropSetup"));
      #endif
      delete (*i);
   }

   ClearWrappers();

   #ifdef DUMP_PLANET_DATA
      if (planetData.is_open())
         planetData.close();
   #endif
}


//------------------------------------------------------------------------------
//  Propagate(const Propagate &prp)
//------------------------------------------------------------------------------
/**
 * Constructs a Propagate Command based on another instance (copy constructor).
 *
 * @param <p> Original we are copying
 */
//------------------------------------------------------------------------------
Propagate::Propagate(const Propagate &prp) :
   PropagationEnabledCommand   (prp),
   propName                    (prp.propName),
   direction                   (prp.direction),
   satName                     (prp.satName),
   currentPropMode             (prp.currentPropMode),
   interruptCheckFrequency     (prp.interruptCheckFrequency),
   inProgress                  (false),
   hasFired                    (false),
   epochID                     (prp.epochID),
   stopInterval                (0.0),
   stopTrigger                 (-1),
   stopSatNames                (prp.stopSatNames),
   objectArray                 (prp.objectArray),
   elapsedTime                 (prp.elapsedTime),
   currEpoch                   (prp.currEpoch),
   state                       (NULL),
   pubdata                     (NULL),
   stopCondMet                 (false),
   stopEpoch                   (prp.stopEpoch),
   stopAccuracy                (prp.stopAccuracy),
   timeAccuracy                (prp.timeAccuracy),
   dim                         (prp.dim),
   cartDim                     (prp.cartDim),
   singleStepMode              (prp.singleStepMode),
   transientForces             (NULL),
   currentMode                 (prp.currentMode),
   stopCondEpochID             (prp.stopCondEpochID),
   stopCondBaseEpochID         (prp.stopCondBaseEpochID),
   stopCondStopVarID           (prp.stopCondStopVarID)
{
   parameterCount = prp.parameterCount;
   initialized = false;
   baseEpoch.clear();
   prop.clear();
   sats.clear();
   stopWhen.clear();
   stopSats.clear();
   satBuffer.clear();
   formBuffer.clear();
   p.clear();
   fm.clear();
   stepBrackets[0] = stepBrackets[1] = 0.0;
}


//------------------------------------------------------------------------------
//  Propagate& operator=(const Propagate &prp)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the Propagate Command.
 *
 * @param <p> Original we are copying
 *
 * @return reference to this copy
 */
//------------------------------------------------------------------------------
Propagate& Propagate::operator=(const Propagate &prp)
{
   if (&prp == this)
      return *this;

   // Call the base assignment operator
   GmatCommand::operator=(prp);

   propName                = prp.propName;
   direction               = prp.direction;
   satName                 = prp.satName;
   currentPropMode         = prp.currentPropMode;
   interruptCheckFrequency = prp.interruptCheckFrequency;
   inProgress              = false;
   hasFired                = false;
   epochID                 = prp.epochID;
   objectArray             = prp.objectArray;
   elapsedTime             = prp.elapsedTime;
   currEpoch               = prp.currEpoch;
   state                   = NULL;
   pubdata                 = NULL;
   stopCondMet             = false;
   stopEpoch               = prp.stopEpoch;
   stopAccuracy            = prp.stopAccuracy;
   timeAccuracy            = prp.timeAccuracy;
   dim                     = prp.dim;
   cartDim                 = prp.cartDim;
   singleStepMode          = prp.singleStepMode;
   currentMode             = prp.currentMode;
   stopCondEpochID         = prp.stopCondEpochID;
   stopCondBaseEpochID     = prp.stopCondBaseEpochID;
   stopCondStopVarID       = prp.stopCondStopVarID;
   initialized             = false;

   baseEpoch.clear();

   for (std::vector<PropSetup*>::iterator ps = prop.begin(); ps != prop.end();
        ++ps)
   {
      PropSetup *oldPs = *ps;
      *ps = NULL;
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (oldPs, oldPs->GetName(), wxT("Propagate::operator=()"),
          wxT("deleting old PropSetup"));
      #endif
      delete oldPs;
   }
   prop.clear();

   sats.clear();
   stopWhen.clear();
   stopSats.clear();
   satBuffer.clear();
   formBuffer.clear();
   p.clear();
   fm.clear();

   stepBrackets[0] = stepBrackets[1] = 0.0;

   return *this;
}


//------------------------------------------------------------------------------
// bool SetObject(const wxString &name, const Gmat::ObjectType type,
//         const wxString &associate, const Gmat::ObjectType associateType)
//------------------------------------------------------------------------------
/**
 * Sets objects referenced by the Propagate command
 *
 * @param <name> Name of the reference object.
 * @param <type> Type of the reference object.
 * @param <associate> Object associated with this reference object.
 * @param <associateType> Type of the associated object.
 *
 * @return true if the reference was set, false if not.
 */
//------------------------------------------------------------------------------
bool Propagate::SetObject(const wxString &name, const Gmat::ObjectType type,
                          const wxString &associate,
                          const Gmat::ObjectType associateType)
{
   #ifdef DEBUG_PROPAGATE_OBJ
   MessageInterface::ShowMessage
      (wxT("Propagate::SetObject() entered, name='%s', type=%d, associate='%s', ")
       wxT("associateType=%d\n"), name.c_str(), type, associate.c_str(), associateType);
   #endif

   Integer propNum = propName.size() - 1;

   switch (type) {
      case Gmat::SPACECRAFT:
      case Gmat::FORMATION:
         (satName[propNum])->push_back(name);
         return true;

      case Gmat::PROP_SETUP:
         {
            propName.push_back(name);
            if (name[0] == wxT('-'))
            {
               direction = -1.0;
               MessageInterface::ShowMessage(wxT("Please use the keyword \"BackProp\" ")
                  wxT("to set backwards propagation; the use of a minus sign in the ")
                  wxT("string \"%s\" is deprecated.\n"), name.c_str());
            }
            StringArray *satNameArray = new StringArray;
            //satName.push_back(new StringArray);
            satName.push_back(satNameArray);
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Add
               (satNameArray, wxT("satNameArray"), wxT("Propagate::SetObject()"),
                wxT("*satNameArray = new StringArray"));
            #endif
            return true;
         }

      default:
         break;
   }

   return GmatCommand::SetObject(name, type, associate, associateType);
}


//------------------------------------------------------------------------------
// bool SetObject(GmatBase *obj, const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Sets objects referenced by the Propagate command
 *
 * @param <name> Name of the reference object.
 * @param <type> Type of the reference object.
 *
 * @return true if the reference was set, false if not.
 */
//------------------------------------------------------------------------------
bool Propagate::SetObject(GmatBase *obj, const Gmat::ObjectType type)
{
   switch (type)
   {
   case Gmat::STOP_CONDITION:
      stopWhen.push_back((StopCondition *)obj);

      #ifdef DEBUG_STOPPING_CONDITIONS
         MessageInterface::ShowMessage(wxT("Adding stopping condition named %s\n"),
            obj->GetName().c_str());
      #endif

      stopCondEpochID = obj->GetParameterID(wxT("Epoch"));
      stopCondBaseEpochID = obj->GetParameterID(wxT("BaseEpoch"));
      stopCondStopVarID = obj->GetParameterID(wxT("StopVar"));
      return true;

   default:
      break;
   }

   return GmatCommand::SetObject(obj, type);
}

//------------------------------------------------------------------------------
// void ClearObject(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Clears the lists of objects referenced by the Propagate command.
 *
 * @param <type> Type of the objects to clear.
 */
//------------------------------------------------------------------------------
void Propagate::ClearObject(const Gmat::ObjectType type)
{
   switch (type)
   {
   case Gmat::SPACECRAFT:
   case Gmat::FORMATION:
      satName.clear();
      break;
   case Gmat::STOP_CONDITION:
      stopWhen.clear();
      break;

   default:
      break;
   }
}


//------------------------------------------------------------------------------
// GmatBase* GetObject(const Gmat::ObjectType type, const wxString objName)
//------------------------------------------------------------------------------
/**
 * Accesses objects referenced by the Propagate command.
 *
 * @param <type> Type of the reference object.
 * @param <objName> Name of the reference object.
 *
 * @return true if the reference was set, false if not.
 */
//------------------------------------------------------------------------------
GmatBase* Propagate::GetGmatObject(const Gmat::ObjectType type,
                               const wxString objName)
{
   if (type == Gmat::STOP_CONDITION)
   {
      if (stopWhen.empty())
         return NULL;
      else
         return stopWhen[0];
   }

   return GmatCommand::GetGmatObject(type, objName);
}

//------------------------------------------------------------------------------
//  const wxString GetGeneratingString()
//------------------------------------------------------------------------------
/**
 * Method used to retrieve the string that was parsed to build this GmatCommand.
 *
 * This method is used to retrieve the GmatCommand string from the script that
 * was parsed to build the GmatCommand.  It is used to save the script line, so
 * that the script can be written to a file without inverting the steps taken to
 * set up the internal object data.  As a side benefit, the script line is
 * available in the GmatCommand structure for debugging purposes.
 *
 * @param <mode>    Specifies the type of serialization requested.
 * @param <prefix>  Optional prefix appended to the object's name. (Used for
 *                  indentation)
 * @param <useName> Name that replaces the object's name (Not yet used
 *                  in commands).
 *
 * @return The script line that defines this GmatCommand.
 */
//------------------------------------------------------------------------------
const wxString& Propagate::GetGeneratingString(Gmat::WriteMode mode,
                                                  const wxString &prefix,
                                                  const wxString &useName)
{
   wxString gen = prefix + wxT("Propagate");

   // Construct the generating string
   UnsignedInt index = 0;

   if (direction < 0.0)
      gen += (wxT(" BackProp"));

   if (currentPropMode != wxT(""))
      gen += (wxT(" ") + currentPropMode);
   for (StringArray::iterator prop = propName.begin(); prop != propName.end();
        ++prop) {
      gen += wxT(" ") + (*prop) + wxT("(");
      // Spaceobjects that are propagated by this PropSetup
      StringArray *sats = satName[index];
      for (StringArray::iterator sc = sats->begin(); sc != sats->end(); ++sc) {
         // Add a comma if needed
         if (sc != sats->begin())
            gen += wxT(", ");
         gen += (*sc);
      }

      gen += wxT(")");
      ++index;
   }

   // Now the stopping conditions.  Note that stopping conditions are shown at
   // the end of the Propagate line, rather than inside of the PropSetup
   // delimiters.
   if (stopWhen.size() > 0) {
      gen += wxT(" {");

      for (std::vector<StopCondition*>::iterator stp = stopWhen.begin();
           stp != stopWhen.end(); ++stp) {
         wxString stopCondDesc;
         if (stp != stopWhen.begin())
            gen += wxT(", ");

         wxString stopName = (*stp)->GetStringParameter(stopCondStopVarID);
         stopCondDesc << stopName;

         if ((stopName.find(wxT(".Periapsis")) == wxString::npos) &&
             (stopName.find(wxT(".Apoapsis")) == wxString::npos))
            stopCondDesc << wxT(" = ") << (*stp)->GetStringParameter(wxT("Goal"));

         gen += stopCondDesc;
      }

      // Add the stop tolerance is it is not set to the default value
      if (stopAccuracy != DEFAULT_STOP_TOLERANCE)
      {
         gen += wxT(", StopTolerance = ");
         wxString stopTolDesc;
         stopTolDesc << stopAccuracy;
         gen += stopTolDesc;
      }
      gen += wxT("}");
   }

   generatingString = gen + wxT(";");
   // Then call the base class method
   return GmatCommand::GetGeneratingString(mode, prefix, useName);
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the Propagate.
 *
 * @return clone of the Propagate.
 */
//------------------------------------------------------------------------------
GmatBase* Propagate::Clone() const
{
   #ifdef DEBUG_CLONE
   MessageInterface::ShowMessage(wxT("Propagate::Clone() entered\n"));
   #endif
   return (new Propagate(*this));
}


//------------------------------------------------------------------------------
//  wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
/**
 * Accessor used to find the names of referenced objects.
 *
 * @param <type> reference object type.
 *
 * @return The name of the reference object.
 */
//------------------------------------------------------------------------------
wxString Propagate::GetRefObjectName(const Gmat::ObjectType type) const
{
   switch (type) {
      // Propagator setups
      case Gmat::PROP_SETUP:
         return propName[0];

      // Objects that get propagated
      case Gmat::SPACECRAFT:
      case Gmat::FORMATION:
         if (satName.size() > 0)
            return (*satName[0])[0];

      default:
         break;
   }

   return GmatCommand::GetRefObjectName(type);
}


//------------------------------------------------------------------------------
//  bool SetRefObjectName(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Accessor used to set the names of referenced objects.
 *
 * @param <type> type of the reference object.
 * @param <name> name of the reference object.
 *
 * @return success of the operation.
 */
//------------------------------------------------------------------------------
bool Propagate::SetRefObjectName(const Gmat::ObjectType type,
                                 const wxString &name)
{
   switch (type) {
      // Propagator setups
      case Gmat::PROP_SETUP:
         {
            propName.push_back(name);
            StringArray *satNameArray = new StringArray;
            //satName.push_back(new StringArray);
            satName.push_back(satNameArray);
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Add
               (satNameArray, wxT("satNameArray"), wxT("Propagate::SetRefObjectName()"),
                wxT("*satNameArray = new StringArray"));
            #endif

            return true;
         }

      // Objects that get propagated
      case Gmat::SPACECRAFT:
      case Gmat::FORMATION:
      {
         Integer propNum = propName.size()-1;
         satName[propNum]->push_back(name);
         return true;
      }

      default:
         break;
   }

   return GmatCommand::SetRefObjectName(type, name);
}

// Reference object accessor methods
//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name,
//                        const Integer index
//------------------------------------------------------------------------------
/**
 * Accessor for reference object pointers.
 *
 * @param <type> type of the reference object.
 * @param <name> name of the reference object.
 * @param <index> Index into the object array.
 *
 * @return reference object pointer.
 */
//------------------------------------------------------------------------------
GmatBase* Propagate::GetRefObject(const Gmat::ObjectType type,
                                  const wxString &name, const Integer index)
{
   switch (type)
   {
   case Gmat::PROP_SETUP:
      if (index < (Integer)prop.size())
      {
         return prop[index];
      }
      else
      {
         throw CommandException(wxT("Propagate::GetRefObject() invalid PropSetup index\n"));
      }
      break;
   case Gmat::STOP_CONDITION:
      if (index < (Integer)stopWhen.size())
      {
         return stopWhen[index];
      }
      else
      {
         throw CommandException(wxT("Propagate::GetRefObject() invalid index\n"));
      }
   default:
      break;
   }

   return GmatCommand::GetRefObject(type, name, index);
}

//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type, ...
//------------------------------------------------------------------------------
/**
 * Sets reference object pointer.
 *
 * @param <obj> Pointer to the reference object.
 * @param <type> type of the reference object.
 * @param <name> name of the reference object.
 * @param <index> Index into the object array.
 *
 * @return true if object successfully set, false otherwise
 */
//------------------------------------------------------------------------------
bool Propagate::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                             const wxString &name, const Integer index)
{
   #if DEBUG_PROPAGATE_OBJ
      MessageInterface::ShowMessage
         (wxT("Propagate::SetRefObject() entered, type=%s, name='%s', index=%d, objName='%s'\n"),
          obj->GetTypeName().c_str(), name.c_str(), index, obj->GetName().c_str());
   #endif

   switch (type)
   {
      case Gmat::STOP_CONDITION:
         {
            wxString satName = obj->GetName();
            Integer strt = satName.find(wxT("StopOn")) + 6;
            if (strt == (Integer)wxString::npos)
               strt = 0;
            Integer ndx = satName.find(wxT("."),0);
            if (ndx != (Integer)wxString::npos)
               satName = satName.substr(strt, ndx-strt);
            wxString stopStr = obj->GetStringParameter(wxT("StopVar"));
            wxString goalStr = obj->GetStringParameter(wxT("Goal"));
            Integer size = stopWhen.size();

            #if DEBUG_PROPAGATE_OBJ
            MessageInterface::ShowMessage
               (wxT("satName='%s', stopStr='%s', goalStr='%s', stopWhen.size()=%d, ")
                wxT("stopNames.size()=%d, goalNames.size()=%d\n"),
                satName.c_str(), stopStr.c_str(), goalStr.c_str(), stopWhen.size(),
                stopNames.size(), goalNames.size());
            #endif

            if ((stopWhen.empty() && index == 0) || (index == size))
            {
               stopWhen.push_back((StopCondition *)obj);
               stopSatNames.push_back(satName);
               stopNames.push_back(stopStr);
               goalNames.push_back(goalStr);
               stopWrappers.push_back(NULL);
               goalWrappers.push_back(NULL);
            }
            else if (index < size)
            {
               stopWhen[index] = (StopCondition *)obj;
               stopSatNames[index] = satName;
            }
            else
            {
               MessageInterface::ShowMessage
                  (wxT("Propagate::SetRefObject() index=%d is not next available ")
                   wxT("index=%d. Setting %s:%s failed\n"), index, size,
                   obj->GetTypeName().c_str(), obj->GetName().c_str());
               return false;
            }

            #if DEBUG_PROPAGATE_OBJ
               for (UnsignedInt  j=0; j<stopSatNames.size(); j++)
                  MessageInterface::ShowMessage(
                     wxT("Propagate::SetRefObject() stopSatNames=%s\n"),
                     stopSatNames[j].c_str());
            #endif

            #ifdef DEBUG_STOPPING_CONDITIONS
               MessageInterface::ShowMessage(
                  wxT("Adding stopping condition named %s\n"),
                  obj->GetName().c_str());
            #endif

            stopCondEpochID = obj->GetParameterID(wxT("Epoch"));
            stopCondBaseEpochID = obj->GetParameterID(wxT("BaseEpoch"));
            stopCondStopVarID = obj->GetParameterID(wxT("StopVar"));

            #if DEBUG_PROPAGATE_OBJ
            MessageInterface::ShowMessage(wxT("Propagate::SetRefObject() returning true\n"));
            #endif
            return true;
         }

      default:
         break;
   }

   return GmatCommand::SetRefObject(obj, type, name, index);
}

//------------------------------------------------------------------------------
// virtual ObjectArray& GetRefObjectArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Obtains an array of GmatBase pointers by type.
 *
 * @param type The type of objects requested.
 *
 * @return Reference to the array.
 */
//------------------------------------------------------------------------------
ObjectArray& Propagate::GetRefObjectArray(const Gmat::ObjectType type)
{
   objectArray.clear();

   switch (type)
   {
      case Gmat::STOP_CONDITION:
         for (UnsignedInt i=0; i<stopWhen.size(); i++)
            objectArray.push_back(stopWhen[i]);
         return objectArray;

      default:
         break;
   }

   return GmatCommand::GetRefObjectArray(type);
}


// Parameter accessor methods

//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieve the description for the parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return String description for the requested parameter.
 */
//------------------------------------------------------------------------------
wxString Propagate::GetParameterText(const Integer id) const
{
   if ((id < PropagateCommandParamCount) && (id >= GmatCommandParamCount))
      return PARAMETER_TEXT[id - GmatCommandParamCount];

   return GmatCommand::GetParameterText(id);
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * Retrieve the ID for the parameter given its description.
 *
 * @param <str> Description for the parameter.
 *
 * @return the parameter ID.
 */
//------------------------------------------------------------------------------
Integer Propagate::GetParameterID(const wxString &str) const
{
   for (Integer i = GmatCommandParamCount; i < PropagateCommandParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatCommandParamCount])
         return i;
   }

   return GmatCommand::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieve the enumerated type of the object.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The enumeration for the type of the parameter.
 */
//------------------------------------------------------------------------------
Gmat::ParameterType Propagate::GetParameterType(const Integer id) const
{
   if (id >= GmatCommandParamCount && id < PropagateCommandParamCount)
      return PARAMETER_TYPE[id - GmatBaseParamCount];

   return GmatCommand::GetParameterType(id);
}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieve the string associated with a parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return Text description for the type of the parameter.
 */
//------------------------------------------------------------------------------
wxString Propagate::GetParameterTypeString(const Integer id) const
{
   return GmatBase::PARAM_TYPE_STRING[GetParameterType(id)];
}


//------------------------------------------------------------------------------
// Integer GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieve the value for an Integer parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
Integer Propagate::GetIntegerParameter(const Integer id) const
{
   if (id == INTERRUPT_FREQUENCY)
      return interruptCheckFrequency;

   return GmatCommand::GetIntegerParameter(id);
}


//------------------------------------------------------------------------------
// Integer SetIntegerParameter(const Integer id, const Integer value)
//------------------------------------------------------------------------------
/**
 * Set the value for an Integer parameter.
 *
 * @param <id> The integer ID for the parameter.
 * @param <value> The new parameter value.
 *
 * @return the parameter value at the end of this call.
 */
//------------------------------------------------------------------------------
Integer Propagate::SetIntegerParameter(const Integer id, const Integer value)
{
   if (id == INTERRUPT_FREQUENCY) {
      if (value >= 0)
         interruptCheckFrequency = value;
      return interruptCheckFrequency;
   }

   return GmatCommand::SetIntegerParameter(id, value);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieve a string parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The string stored for this parameter.
 */
//------------------------------------------------------------------------------
wxString Propagate::GetStringParameter(const Integer id) const
{
   if (id == PROP_COUPLED)
      return currentPropMode;

   return GmatCommand::GetStringParameter(id);
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
/**
 * Change the value of a string parameter.
 *
 * @param <id> The integer ID for the parameter.
 * @param <value> The new string for this parameter.
 *
 * @return true if the string is stored.
 */
//------------------------------------------------------------------------------
bool Propagate::SetStringParameter(const Integer id, const wxString &value)
{
   if (id == PROP_COUPLED)
   {
      const StringArray pmodes = GetStringArrayParameter(AVAILABLE_PROP_MODES);
      if (find(pmodes.begin(), pmodes.end(), value) != pmodes.end())
      {
         // Back prop is a special case
         if (value == wxT("BackProp"))
         {
            direction = -1.0;
         }
         else
         {
            currentPropMode = value;
            for (Integer i = 0; i < PropModeCount; ++i)
               if (value == pmodes[i])
               {
                  currentMode = (PropModes)i;
                  return true;
               }
         }
      }
   }

   if (id == SAT_NAME)
   {
      Integer propNum = propName.size()-1;
      satName[propNum]->push_back(value);
      return true;
   }

   if (id == PROP_NAME)
   {
      wxString propNameString = value;
      if (propNameString[0] == wxT('-'))
      {
         direction = -1.0;
         MessageInterface::ShowMessage(wxT("Please use the keyword \"BackProp\" to ")
            wxT("set backwards propagation; the use of a minus sign in the string ")
            wxT("\"%s\" is deprecated.\n"), propNameString.c_str());
         propNameString = propNameString.substr(1);
      }
      propName.push_back(propNameString);
      StringArray *satNameArray = new StringArray;
      //satName.push_back(new StringArray);
      satName.push_back(satNameArray);
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Add
         (satNameArray, wxT("satNameArray"), wxT("Propagate::SetStringParameter()"),
          wxT("*satNameArray = new StringArray"));
      #endif
      return true;
   }

   return GmatCommand::SetStringParameter(id, value);
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value,
//                         const Integer index)
//------------------------------------------------------------------------------
/**
 * Change the value of a string parameter.
 *
 * @param id The integer ID for the parameter.
 * @param value The new string for this parameter.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return true if the string is stored, false if not.
 */
//------------------------------------------------------------------------------
bool Propagate::SetStringParameter(const Integer id, const wxString &value,
                                   const Integer index)
{
   if (id == SAT_NAME) {
      if (index < (Integer)propName.size())
         satName[index]->push_back(value);
      else
         throw CommandException(wxT("Propagate::SetStringParameter Attempting to ")
                         wxT("assign a spacecraft without an associated PropSetup"));
      return true;
   }

   return GmatCommand::SetStringParameter(id, value, index);
}

//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Access an array of string data.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The requested StringArray.
 */
//------------------------------------------------------------------------------
const StringArray& Propagate::GetStringArrayParameter(const Integer id) const
{
   static StringArray modeList;

   if (id == AVAILABLE_PROP_MODES) {
      modeList.clear();
      for (Integer i = 0; i < PropModeCount; ++i)
         // BackProp isn't really a prop sync mode
         if (PropModeList[i] != wxT("BackProp"))
            modeList.push_back(PropModeList[i]);
      return modeList;
   }

   if (id == SAT_NAME)
      return *satName[0];

   if (id == PROP_NAME) {
      return propName;
   }

   return GmatCommand::GetStringArrayParameter(id);
}

//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const Integer id,
//                                            const Integer index) const
//------------------------------------------------------------------------------
/**
 * Access an array of string data.
 *
 * @param id The integer ID for the parameter.
 * @param index The index when multiple StringArrays are supported.
 *
 * @return The requested StringArray.
 */
//------------------------------------------------------------------------------
const StringArray& Propagate::GetStringArrayParameter(const Integer id,
                                               const Integer index) const
{
   if (id == SAT_NAME)
      return *satName[index];

   return GmatCommand::GetStringArrayParameter(id, index);
}


//------------------------------------------------------------------------------
// Integer GetBooleanParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieve the value for an Boolean parameter.
 *
 * Propagate currently contains the following Boolean parameter:
 *
 *    PROP_FORWARDS     Evaluates true if the prop direction is forward in time,
 *                      false if propagating backwards.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
bool Propagate::GetBooleanParameter(const Integer id) const
{
   if (id == PROP_FORWARD)
      return (direction > 0.0 ? true : false);

   return GmatCommand::GetBooleanParameter(id);
}


//------------------------------------------------------------------------------
// Integer SetBooleanParameter(const Integer id, const bool value)
//------------------------------------------------------------------------------
/**
 * Set the value for an Boolean parameter.
 *
 * Propagate currently contains the following Boolean parameter:
 *
 *    PROP_FORWARDS     Set true if the prop direction is forward in time,
 *                      false if propagating backwards.
 *
 * @param <id> The integer ID for the parameter.
 * @param <value> The parameter setting
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
bool Propagate::SetBooleanParameter(const Integer id, const bool value)
{
   if (id == PROP_FORWARD)
   {
      if (value == true)
         direction = 1.0;
      else
         direction = -1.0;

      for (UnsignedInt i=0; i<stopWhen.size(); i++)
      {
         if (stopWhen[i] != NULL)
         {
            #ifdef DEBUG_PROPAGATE_DIRECTION
            MessageInterface::ShowMessage
               (wxT("Setting direction %f to StopCondition '%s'\n"), direction,
                stopWhen[i]->GetName().c_str());
            #endif
            stopWhen[i]->SetPropDirection(direction);  // Use direction of props
         }
      }
      return true;
   }

   return GmatCommand::SetBooleanParameter(id, value);
}


//------------------------------------------------------------------------------
// Integer GetBooleanParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Retrieve the value for an Boolean parameter.
 *
 * Propagate currently contains the following Boolean label:
 *
 *    "PropForward"     Evaluates true if the prop direction is forward in time,
 *                      false if propagating backwards.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
bool Propagate::GetBooleanParameter(const wxString &label) const
{
   return GetBooleanParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// Integer SetBooleanParameter(const wxString &label, const bool value)
//------------------------------------------------------------------------------
/**
 * Set the value for an Boolean parameter.
 *
 * Propagate currently contains the following Boolean parameter:
 *
 *    "PropForward"     Set true if the prop direction is forward in time,
 *                      false if propagating backwards.
 *
 * @param <id> The integer ID for the parameter.
 * @param <value> The parameter setting
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
bool Propagate::SetBooleanParameter(const wxString &label, const bool value)
{
   return SetBooleanParameter(GetParameterID(label), value);
}


Real Propagate::GetRealParameter(const Integer id) const
{
   if (id == STOP_ACCURACY)
      return stopAccuracy;
   return GmatCommand::GetRealParameter(id);
}

Real Propagate::SetRealParameter(const Integer id, const Real value)
{
   if (id == STOP_ACCURACY)
   {
      if (value > 0.0)
      {
         stopAccuracy = value;
         timeAccuracy = value;
         firstStepTolerance = stopAccuracy * 10.0;
      }
      else
      {
         wxString val;
         val << value;
         CommandException ce;
         ce.SetDetails(errorMessageFormatUnnamed.c_str(),
            val.c_str(), wxT("StopTolerance"), wxT("a Real number > 0.0"));
         throw ce;
      }
      return stopAccuracy;
   }
   return GmatCommand::SetRealParameter(id, value);
}

Real Propagate::GetRealParameter(const wxString &label) const
{
   return GetRealParameter(GetParameterID(label));
}

Real Propagate::SetRealParameter(const wxString &label, const Real value)
{
   return SetRealParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// bool TakeAction(const wxString &action, const wxString &actionData)
//------------------------------------------------------------------------------
/**
 * Interface used to support user actions.
 *
 * @param <action> The string descriptor for the requested action.
 * @param <actionData> Optional data used for the action.
 *
 * @return true if the action was performed, false if not.
 */
//------------------------------------------------------------------------------
bool Propagate::TakeAction(const wxString &action,
                           const wxString &actionData)
{
   #ifdef DEBUG_TAKE_ACTION
   MessageInterface::ShowMessage
      (wxT("Propagate::TakeAction() this=<%p> entered, action='%s', actionData='%s'\n"),
       this, action.c_str(), actionData.c_str());
   #endif

   if (action == wxT("Clear"))
   {
      if (actionData == wxT("Propagator"))
      {
         for (Integer i = 0; i < (Integer)satName.size(); ++i)
         {
            delete satName[i];
         }
         satName.clear();

         propName.clear();

         for (std::vector<PropSetup*>::iterator ps = prop.begin();
              ps != prop.end(); ++ps)
         {
            PropSetup *oldPs = *ps;
            *ps = NULL;
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Remove
               (oldPs, oldPs->GetName(), wxT("Propagate::TakeAction()"),
                wxT("deleting old PropSetup"));
            #endif
            delete oldPs;
         }
         prop.clear();
         p.clear();
         fm.clear();

         sats.clear();
      }
      else if (actionData == wxT("StopCondition"))
      {
         stopWhen.clear();
         stopSats.clear();
         stopSatNames.clear();
         ClearWrappers();
         stopNames.clear();
         goalNames.clear();
         return true;
      }
   }
   else if (action == wxT("SetStopSpacecraft"))
   {
      stopSatNames.push_back(actionData);
      return true;
   }
   else if (action == wxT("ResetLoopData"))
   {
      for (std::vector<Propagator*>::iterator i = p.begin(); i != p.end(); ++i)
      {
         (*i)->ResetInitialData();
      }
      return true;
   }
   else if (action == wxT("IsInFunction"))
   {
      if (GetCurrentFunction() == NULL)
         return false;
      else
         return true;
   }
   else if (action == wxT("PrepareToPropagate"))
   {
      PrepareToPropagate();
      return true;
   }

   return GmatCommand::TakeAction(action, actionData);
}


//------------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//------------------------------------------------------------------------------
/**
 * Renames referenced objects.
 *
 * @param type Type of the object that is renamed.
 * @param oldName The current name for the object.
 * @param newName The name the object has when this operation is complete.
 *
 * @return true on success.
 */
//------------------------------------------------------------------------------
bool Propagate::RenameRefObject(const Gmat::ObjectType type,
                                const wxString &oldName,
                                const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("Propagate::RenameRefObject() type=%s, oldName=%s, newName=%s\n"),
       GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif

   // Propagate needs to know about Spacecraft, Formation, PropSetup, Parameter
   if (type != Gmat::SPACECRAFT && type != Gmat::FORMATION &&
       type != Gmat::PROP_SETUP && type != Gmat::PARAMETER)
      return true;

   StringArray::iterator pos;

   if (type == Gmat::PROP_SETUP)
   {
      // rename PropSetup
      for (pos = propName.begin(); pos != propName.end(); ++pos)
         if (*pos == oldName)
            *pos = newName;
   }
   else
   {
      // rename space object name used in prop setup
      for (UnsignedInt prop = 0; prop < propName.size(); ++prop)
         for (pos = satName[prop]->begin(); pos != satName[prop]->end(); ++pos)
            if (*pos == oldName)
               *pos = newName;

      // rename space object name used in stopping condition
      for (UnsignedInt i = 0; i < stopSatNames.size(); ++i)
         if (stopSatNames[i] == oldName)
            stopSatNames[i] = newName;

      #ifdef DEBUG_RENAME
      MessageInterface::ShowMessage
         (wxT("Propagate::RenameRefObject() Rename StopCondtion Object\n"));
      #endif

      // rename stop condition parameter
      for (UnsignedInt i=0; i<stopWhen.size(); i++)
         stopWhen[i]->RenameRefObject(type, oldName, newName);
   }

   return true;
}


//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by the Propagate.
 *
 * @return the list of object types.
 *
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& Propagate::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::PROP_SETUP);
   refObjectTypes.push_back(Gmat::SPACECRAFT);
   refObjectTypes.push_back(Gmat::PARAMETER);
   return refObjectTypes;
}


//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref objects used by the Propagate.
 *
 * @param <type> The type of object desired, or Gmat::UNKNOWN_OBJECT for the
 *               full list.
 *
 * @return the list of object names.
 *
 */
//------------------------------------------------------------------------------
const StringArray& Propagate::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   #ifdef DEBUG_PROPAGATE_OBJ
   MessageInterface::ShowMessage
      (wxT("Propagate::GetRefObjectNameArray() <%p> entered, type=%d\n"), this, type);
   #endif

   refObjectNames.clear();

   if (type == Gmat::UNKNOWN_OBJECT ||
       type == Gmat::PROP_SETUP)
   {
      // Remove backward prop notation '-'
      wxString newPropName;
      for (UnsignedInt i=0; i<propName.size(); i++)
      {
         newPropName = propName[i];
         if (newPropName[0] == wxT('-'))
            newPropName = propName[i].substr(1);

         refObjectNames.push_back(newPropName);
      }
   }

   if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::SPACECRAFT)
   {
      refObjectNames.insert(refObjectNames.end(), stopSatNames.begin(),
                            stopSatNames.end());
   }

   if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::PARAMETER)
   {
      #ifdef DEBUG_PROPAGATE_OBJ
      MessageInterface::ShowMessage
         (wxT("   The type is Parameter, stopNames.size()=%d, goalNames.size()=%d, ")
          wxT("stopWhen.size()=%d\n"), stopNames.size(), goalNames.size(), stopWhen.size());
      #endif

      // Add LHS of stopping condition
      for (UnsignedInt i = 0; i < stopNames.size(); i++)
      {
         #ifdef DEBUG_PROPAGATE_OBJ
         MessageInterface::ShowMessage
            (wxT("      stopName[%d]='%s'\n"), i, stopNames[i].c_str());
         #endif

         if (!GmatStringUtil::IsNumber(stopNames[i]) &&
             find(refObjectNames.begin(), refObjectNames.end(), stopNames[i]) ==
             refObjectNames.end())
            refObjectNames.push_back(stopNames[i]);
      }

      // Add RHS of stopping condition
      for (UnsignedInt i = 0; i < goalNames.size(); i++)
      {
         #ifdef DEBUG_PROPAGATE_OBJ
         MessageInterface::ShowMessage
            (wxT("      goalName[%d]='%s'\n"), i, goalNames[i].c_str());
         #endif

         if (!GmatStringUtil::IsNumber(goalNames[i]) &&
             find(refObjectNames.begin(), refObjectNames.end(), goalNames[i]) ==
             refObjectNames.end())
            refObjectNames.push_back(goalNames[i]);
      }

      // Add StopCondition parameters
      for (UnsignedInt i = 0; i < stopWhen.size(); i++)
      {
         StringArray refNames = stopWhen[i]->GetRefObjectNameArray(Gmat::PARAMETER);
         for (UnsignedInt j = 0; j < refNames.size(); j++)
         {
            #ifdef DEBUG_PROPAGATE_OBJ
            MessageInterface::ShowMessage
               (wxT("      refNames[%d]='%s'\n"), j, refNames[j].c_str());
            #endif

            if (find(refObjectNames.begin(), refObjectNames.end(), refNames[j]) ==
                refObjectNames.end())
               refObjectNames.push_back(refNames[j]);
         }
      }
   }

   return refObjectNames;
}


//------------------------------------------------------------------------------
// bool InterpretAction()
//------------------------------------------------------------------------------
/**
 * Parses the command string and builds the corresponding command structures.
 *
 * The Propagate command has the following syntax:
 *
 *     Propagate prop(Sat1, ... , {Sat1.ElapsedDays = 700}) ...;
 *
 * where prop is a PropSetup, "Sat1, ... ," is the list of SpaceObjects that are
 * propagated, and the items in curly braces are the (optional) stopping
 * conditions.  The Propagate command supports simultaneous propagation of
 * multiple spacecraft, either in a single PropSetup or in a list of PropSetups
 * on the same Propagate command line.
 *
 * This method breaks the script line into the corresponding pieces, and stores
 * the names of the PropSetups, SpaceObjects, and StoppingConditions so they can
 * be set to point to the correct objects during initialization.
 *
 * @return true on successful parsing of the command.
 */
//------------------------------------------------------------------------------
bool Propagate::InterpretAction()
{
   #ifdef DEBUG_PROPAGATE_ASSEMBLE
   MessageInterface::ShowMessage
      (wxT("Propagate::InterpretAction() genString = \"%s\"\n"),
       generatingString.c_str());
   #endif

   Integer loc = generatingString.find(wxT("Propagate"), 0) + 9;
   wxString str = generatingString.c_str();

   if (generatingString.find(wxT("..")) != generatingString.npos)
      throw CommandException(wxT("Propagate::InterpretAction: Can not parse ")
            wxT("command\n ") + generatingString);

   // Verify bracket/parentheses matching
   Integer parenCount[2] = {0,0}, bracketCount[2] = {0,0};
   for (UnsignedInt i = 0; i < str.length(); ++i)
   {
      if (str[i] == wxT('('))
         ++parenCount[0];
      if (str[i] == wxT(')'))
         ++parenCount[1];

      if (str[i] == wxT('{'))
         ++bracketCount[0];
      if (str[i] == wxT('}'))
         ++bracketCount[1];
   }
   wxString errmsg;
   if (parenCount[0] != parenCount[1])
      errmsg = wxT("Parentheses are mismatched");
   if (bracketCount[0] != bracketCount[1])
   {
      if (errmsg.size() > 0)
         errmsg += wxT(" and ");
      errmsg += wxT("Brackets are mismatched");
   }
   if (errmsg.length() > 0)
      throw CommandException(errmsg);

   while (str[loc] == wxT(' '))
      ++loc;

   // Check to see if there are optional parameters (e.g. "Synchronized")
   CheckForOptions(loc, generatingString);
   // Now fill in the list of propagators
   AssemblePropagators(loc, generatingString);

   if (propName.size() == 0)
      throw CommandException(wxT("A Propagate command is not valid: no ")
            wxT("propagators are identified"));


   // Load up the array listing the objects referenced so they can be validated
   objects.clear();
   StringArray satList, satDuplicates;

   #ifdef DEBUG_PROPSETUP_NAMES
      MessageInterface::ShowMessage(wxT("PropSetup Names:"));
   #endif

   for (UnsignedInt i = 0; i < propName.size(); ++i)
   {
      if ((propName[i].c_str())[0] == wxT('-'))
         propName[i] = propName[i].substr(1);

      #ifdef DEBUG_PROPSETUP_NAMES
         MessageInterface::ShowMessage(wxT("   %s\n"), propName[i].c_str());
      #endif

      objects.push_back(propName[i]);

      #ifdef DEBUG_PROPAGATE_ASSEMBLE
         MessageInterface::ShowMessage(wxT("Satellites to propagate:\n"));
      #endif
      for (UnsignedInt j = 0; j < satName[i]->size(); ++j)
      {
         // todo: Point fix for STM.
         /** @todo: This point fix needs to be generalized so that a list of
          *         keywords isn't maintained here.
          */
         if (((*satName[i])[j] != wxT("STM")) && ((*satName[i])[j] != wxT("AMatrix")))
         {
            #ifdef DEBUG_PROPAGATE_ASSEMBLE
               MessageInterface::ShowMessage(wxT("  [%d][%d] = %s\n"), i, j,
                     (*satName[i])[j].c_str());
            #endif
            objects.push_back((*satName[i])[j]);
            satList.push_back((*satName[i])[j]);
         }
      }
   }

   // Look for repeated spacecraft names in list (will miss formation members)
   for (UnsignedInt i = 0; i < satList.size(); ++i)
   {
      wxString currentSat = satList[i];
      for (UnsignedInt j = i+1; j < satList.size(); ++j)
      {
         if (currentSat == satList[j])
            satDuplicates.push_back(satList[j]);
      }
   }
   if (satDuplicates.size() > 0)
      throw CommandException(wxT("Duplicate Spacecraft names in a single Propagate ")
            wxT("line are not allowed"));

   if ((bracketCount[0] > 0) && (stopNames.size() == 0))
      throw CommandException(wxT("Brackets for stopping conditions were found, ")
            wxT("but no stopping conditions detected"));

   return true;
}


//------------------------------------------------------------------------------
// const StringArray& GetWrapperObjectNameArray()
//------------------------------------------------------------------------------
const StringArray& Propagate::GetWrapperObjectNameArray()
{
   wrapperObjectNames.clear();
   wrapperObjectNames.insert(wrapperObjectNames.end(), stopNames.begin(),
                             stopNames.end());
   wrapperObjectNames.insert(wrapperObjectNames.end(), goalNames.begin(),
                             goalNames.end());
   return wrapperObjectNames;
}


//------------------------------------------------------------------------------
// bool SetElementWrapper(ElementWrapper *toWrapper, const wxString &withName)
//------------------------------------------------------------------------------
bool Propagate::SetElementWrapper(ElementWrapper *toWrapper,
                                  const wxString &withName)
{
   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("Propagate::SetElementWrapper() entered with toWrapper=<%p>, withName='%s'\n"),
       toWrapper, withName.c_str());
   #endif

   if (toWrapper == NULL)
      return false;

   // this would be caught by next part, but this message is more meaningful
   if (toWrapper->GetWrapperType() == Gmat::ARRAY_WT)
   {
      throw CommandException(wxT("A value of type \"Array\" on command \"") + typeName +
                  wxT("\" is not an allowed value.\nThe allowed values are:")
                  wxT(" [ Real Number, Variable, Array Element, or Parameter ]. "));
   }

   CheckDataType(toWrapper, Gmat::REAL_TYPE, wxT("Propagate"), true);

   bool retval = false;
   ElementWrapper *ew;

   //-------------------------------------------------------
   // check stopping condition names
   //-------------------------------------------------------
   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("   Checking %d Propagate Stop Conditions\n"), stopNames.size());
   for (UnsignedInt i=0; i<stopNames.size(); i++)
      MessageInterface::ShowMessage(wxT("      %s\n"), stopNames[i].c_str());
   MessageInterface::ShowMessage(wxT("   There are %d stopWrappers\n"), stopWrappers.size());
   for (UnsignedInt i=0; i<stopWrappers.size(); i++)
      MessageInterface::ShowMessage
         (wxT("      <%p>'%s'\n"), stopWrappers[i], stopWrappers[i] ?
          stopWrappers[i]->GetDescription().c_str() : wxT("NULL"));
   #endif

   WrapperArray wrappersToDelete;
   Integer sz = stopNames.size();

   for (Integer i = 0; i < sz; i++)
   {
      if (stopNames.at(i) == withName)
      {
         #ifdef DEBUG_WRAPPERS
         MessageInterface::ShowMessage
            (wxT("   Found wrapper name \"%s\" in stopNames\n"), withName.c_str());
         #endif

         for (UnsignedInt j=0; j<stopWhen.size(); j++)
         {
            #ifdef DEBUG_WRAPPERS
            MessageInterface::ShowMessage
               (wxT("   stopWhen[j]->GetName()='%s'\n"), stopWhen[j]->GetName().c_str());
            #endif
            //if (stopWhen[j]->GetName() == (wxT("StopOn") + withName))
            if (stopWhen[j]->GetLhsString() == withName)
            {
               stopWhen[j]->SetStopParameter((Parameter*)toWrapper->GetRefObject());
               stopWhen[j]->SetLhsWrapper(toWrapper);
            }
         }

         if (stopWrappers.at(i) != NULL)
         {
            ew = stopWrappers.at(i);
            #ifdef DEBUG_WRAPPERS
            MessageInterface::ShowMessage
               (wxT("   Replacing stopWrapper[%d] <%p> with <%p>\n"), i, ew, toWrapper);
            #endif
            stopWrappers.at(i) = toWrapper;

            // Delete old wrapper if wrapper name not found in the goalNames
            if (find(goalNames.begin(), goalNames.end(), withName) == goalNames.end())
               wrappersToDelete.push_back(ew);
         }
         else
         {
            #ifdef DEBUG_WRAPPERS
            MessageInterface::ShowMessage
               (wxT("   Setting wrapper <%p> to stopWrappers[%d]\n"), toWrapper, i);
            #endif
            stopWrappers.at(i) = toWrapper;
         }
         retval = true;
      }
   }

   //-------------------------------------------------------
   // check goal names
   //-------------------------------------------------------
   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("   Checking %d Propagate Stop Goals\n"), goalNames.size());
   for (UnsignedInt i=0; i<goalNames.size(); i++)
      MessageInterface::ShowMessage(wxT("      %s\n"), goalNames[i].c_str());
   MessageInterface::ShowMessage(wxT("   There are %d goalWrappers\n"), goalWrappers.size());
   for (UnsignedInt i=0; i<goalWrappers.size(); i++)
      MessageInterface::ShowMessage
         (wxT("      <%p>'%s'\n"), goalWrappers[i], goalWrappers[i] ?
          goalWrappers[i]->GetDescription().c_str() : wxT("NULL"));
   #endif

   sz = goalNames.size();

   for (Integer i = 0; i < sz; i++)
   {
      if (goalNames.at(i) == withName)
      {
         #ifdef DEBUG_WRAPPERS
         MessageInterface::ShowMessage
            (wxT("   Found wrapper name \"%s\" in goalNames\n"), withName.c_str());
         #endif

         for (UnsignedInt j=0; j<stopWhen.size(); j++)
         {
            #ifdef DEBUG_WRAPPERS
            MessageInterface::ShowMessage
               (wxT("   stopWhen[j]->GetName()='%s'\n"), stopWhen[j]->GetName().c_str());
            #endif
            if (stopWhen[j]->GetRhsString() == withName)
            {
               stopWhen[j]->SetRhsWrapper(toWrapper);
            }
         }

         if (goalWrappers.at(i) != NULL)
         {
            ew = goalWrappers.at(i);
            #ifdef DEBUG_WRAPPERS
            MessageInterface::ShowMessage
               (wxT("   Replacing goalWrapper[%d] <%p> with <%p>\n"), i, ew, toWrapper);
            #endif
            goalWrappers.at(i) = toWrapper;

            // Delete old wrapper if wrapper name not found in the stopNames
            if (find(stopNames.begin(), stopNames.end(), withName) == stopNames.end())
               wrappersToDelete.push_back(ew);
         }
         else
         {
            #ifdef DEBUG_WRAPPERS
            MessageInterface::ShowMessage
               (wxT("   Setting wrapper <%p> to goalWrappers[%d]\n"), toWrapper, i);
            #endif
            goalWrappers.at(i) = toWrapper;
         }
         retval = true;
      }
   }

   // delete old wrappers (loj: 2008.12.15)
   for (WrapperArray::iterator ewi = wrappersToDelete.begin();
        ewi < wrappersToDelete.end(); ewi++)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         ((*ewi), (*ewi)->GetDescription(), wxT("Propagate::SetElementWrapper()"),
          wxT("deleting wrapper"));
      #endif
      delete (*ewi);
      (*ewi) = NULL;
   }

   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("Propagate::SetElementWrapper() returning %s\n", retval ? "true" : "false"));
   #endif
   return retval;
}


//------------------------------------------------------------------------------
// void ClearWrappers()
//------------------------------------------------------------------------------
/*
 * Deletes element wrappers and sets them to NULL without emptying the array.
 */
//------------------------------------------------------------------------------
void Propagate::ClearWrappers()
{
   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("Propagate::ClearWrappers() entered, stopNames.size()=%d, stopWrappers.size()=%d, ")
       wxT("goalNames.size()=%d, goalWrappers.size()=%d\n"), stopNames.size(), stopWrappers.size(),
       goalNames.size(), goalWrappers.size());
   #endif

   WrapperArray wrappersToDelete;
   ElementWrapper *ew;

   Integer sz = stopWrappers.size();
   for (Integer i = 0; i < sz; i++)
   {
      if (stopWrappers.at(i) != NULL)
      {
         ew = stopWrappers.at(i);
         stopWrappers.at(i) = NULL;
         if (find(wrappersToDelete.begin(), wrappersToDelete.end(), ew) ==
             wrappersToDelete.end())
         {
            wrappersToDelete.push_back(ew);
         }
      }
   }

   sz = goalWrappers.size();
   for (Integer i = 0; i < sz; i++)
   {
      if (goalWrappers.at(i) != NULL)
      {
         ew = goalWrappers.at(i);
         goalWrappers.at(i) = NULL;
         if (find(wrappersToDelete.begin(), wrappersToDelete.end(), ew) ==
             wrappersToDelete.end())
         {
            wrappersToDelete.push_back(ew);
         }
      }
   }

   // delete old wrappers
   for (WrapperArray::iterator ewi = wrappersToDelete.begin();
        ewi < wrappersToDelete.end(); ewi++)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         ((*ewi), (*ewi)->GetDescription(), wxT("Propagate::ClearWrappers()"),
          wxT("deleting wrapper"));
      #endif
      delete (*ewi);
      (*ewi) = NULL;
   }

   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("Propagate::ClearWrappers() leaving, stopNames.size()=%d, stopWrappers.size()=%d, ")
       wxT("goalNames.size()=%d, goalWrappers.size()=%d\n"), stopNames.size(), stopWrappers.size(),
       goalNames.size(), goalWrappers.size());
   #endif
}


//------------------------------------------------------------------------------
// void CheckForOptions(Integer &loc, wxString &generatingString)
//------------------------------------------------------------------------------
/**
 * Looks for propagator options that exist prior to any PropSetup names.
 *
 * @param <loc>               The current location in the generating string.
 * @param <generatingString>  The generating string.
 */
//------------------------------------------------------------------------------
void Propagate::CheckForOptions(Integer &loc, wxString &generatingString)
{
   wxString modeStr;
   currentMode = INDEPENDENT;

   #ifdef DEBUG_PROPAGATE_ASSEMBLE
      MessageInterface::ShowMessage(wxT("Propagate::CheckForOptions(%d, %s) ")
            wxT("entered\n"), loc, generatingString.c_str());
   #endif

   Integer maxLoc = loc;


   for (Integer modeId = INDEPENDENT+1; modeId != PropModeCount; ++modeId)
   {
      modeStr = PropModeList[modeId];
      modeStr += wxT(" ");

      #ifdef DEBUG_PROPAGATE_ASSEMBLE
         MessageInterface::ShowMessage(wxT("Propagate::CheckForOptions() looking")
                                wxT(" for \"%s\" starting at loc=%d in \n\"%s\""),
                                modeStr.c_str(), loc, generatingString.c_str());
      #endif

      Integer end = generatingString.find(modeStr, loc);
      if (end != (Integer)wxString::npos)
      {
         if (modeStr == wxT("BackProp "))
         {
            direction = -1.0;

            #ifdef DEBUG_PROPAGATE_ASSEMBLE
               MessageInterface::ShowMessage(wxT("\nDirection is now %d\n"), direction);
            #endif
         }
         else
         {
            currentMode = (PropModes)modeId;
            currentPropMode = PropModeList[modeId];
            #ifdef DEBUG_PROPAGATE_ASSEMBLE
               MessageInterface::ShowMessage(wxT("\nLocated at %d\n"), end);
               MessageInterface::ShowMessage(wxT("Mode is now %d\n"), currentMode);
            #endif
         }

         if (end >= maxLoc)
            maxLoc = end + modeStr.length();
      }
      else
      {
         #ifdef DEBUG_PROPAGATE_ASSEMBLE
         MessageInterface::ShowMessage(wxT("\n"));
         #endif
      }
   }

   loc = maxLoc;
}


//------------------------------------------------------------------------------
// void AssemblePropagators(Integer &loc, wxString& generatingString)
//------------------------------------------------------------------------------
/**
 * Parses the PropSetup portion of the Propagate command.
 *
 * @param <loc>               The current location in the generating string.
 * @param <generatingString>  The generating string.
 */
//------------------------------------------------------------------------------
void Propagate::AssemblePropagators(Integer &loc,
   wxString& generatingString)
{
   // First parse the pieces from the string, starting at loc
   StringArray setupStrings, stopStrings;

   FindSetupsAndStops(loc, generatingString, setupStrings, stopStrings);

   #ifdef DEBUG_PROPAGATE_ASSEMBLE
      // Output the chunks for debugging
      MessageInterface::ShowMessage(wxT("PropSetups:\n"));
      for (StringArray::iterator i = setupStrings.begin();
           i != setupStrings.end(); ++i)
         MessageInterface::ShowMessage(wxT("   '%s'\n"), i->c_str());
      MessageInterface::ShowMessage(wxT("StopConditions:\n"));
      for (StringArray::iterator i = stopStrings.begin();
           i != stopStrings.end(); ++i)
         MessageInterface::ShowMessage(wxT("   '%s'\n"), i->c_str());
   #endif

   // Now build the prop setups
   for (StringArray::iterator i = setupStrings.begin();
        i != setupStrings.end(); ++i)
      ConfigurePropSetup(*i);

   // and the stopping conditions
   for (StringArray::iterator i = stopStrings.begin();
        i != stopStrings.end(); ++i)
      ConfigureStoppingCondition(*i);

   // Finally, set the prop mode
   if (stopWhen.empty())
      singleStepMode = true;  // If not, run in single step mode
}


//------------------------------------------------------------------------------
// void Propagate::FindSetupsAndStops(Integer &loc,
//   wxString& generatingString, StringArray &setupStrings,
//   StringArray &stopStrings)
//------------------------------------------------------------------------------
/**
 * Breaks out the PropSetup object strings and the stopping condition strings.
 *
 * @param <loc>               The current location in the generating string.
 * @param <generatingString>  The generating string.
 * @param <setupStrings>      The container for the PropSetup strings.
 * @param <stopStrings>       The container for teh stopping condition strings.
 */
//------------------------------------------------------------------------------
void Propagate::FindSetupsAndStops(Integer &loc,
   wxString& generatingString, StringArray &setupStrings,
   StringArray &stopStrings)
{
   //=================================================================
   #ifdef __NOT_USING_TEXTPARSER__
   //=================================================================
   // First parse the pieces from the string, starting at loc
   wxString tempString, setupWithStop, oneStop;
   const wxChar *str = generatingString.c_str();
   Integer currentLoc = loc, parmstart, end, commaLoc;

   bool scanning = true;

   // First find the PropSetups
   parmstart = generatingString.find(wxT("("), currentLoc);
   while (scanning)
   {
      //end = generatingString.find(")", parmstart)+1;
      end = generatingString.find(wxT(")"), parmstart);

      if (end == (Integer)wxString::npos)
         throw CommandException(wxT("Propagate::AssemblePropagators: Propagate")
                                wxT(" string does not identify propagator"));
      ++end;

      if (generatingString[currentLoc] == wxT('-'))
      {
         direction = -1.0;
         MessageInterface::ShowMessage(wxT("Please use the keyword \"BackProp\" to ")
            wxT("set backwards propagation; the use of a minus sign in the string ")
            wxT("\"%s\" is deprecated.\n"), generatingString.c_str());
         ++currentLoc;
      }

      tempString = generatingString.substr(currentLoc, end-currentLoc);
      // Remove stop condition here
      if (tempString.find(wxT("{"), 0) != wxString::npos)
      {
         setupWithStop = tempString;

         Integer braceStart = setupWithStop.find(wxT("{"), 0),
                 braceEnd   = setupWithStop.find(wxT("}"), 0);

         if (braceEnd == (Integer)wxString::npos)
            throw CommandException(wxT("Propagate::AssemblePropagators: PropSetup")
                                  wxT(" string ") + tempString +
                                  wxT(" starts a stopping condition, but does not")
                                  wxT(" have a closing brace."));
         // Now remove the bracketed chunk from the string
         tempString = setupWithStop.substr(0, braceStart);
         // Remove the comma
         Integer commaLoc = braceStart - 1;
         while ((tempString[commaLoc] == wxT(',')) || (tempString[commaLoc] == wxT(' ')))
         {
            --commaLoc;
         }
         tempString = tempString.substr(0, commaLoc+1);

         // Add on the trailing chunk
         tempString += setupWithStop.substr(braceEnd+1);
      }

      setupStrings.push_back(tempString);
      currentLoc = end; // +1;  Valgrind fix from Joris Olympio

      // Skip trailing comma or white space
      while ((str[currentLoc] == wxT(',')) || (str[currentLoc] == wxT(' ')))
         ++currentLoc;
      parmstart = generatingString.find(wxT("("), currentLoc);
      if (parmstart == (Integer)wxString::npos)
         scanning = false;
   }

   // Now find the stopping conditions
   scanning = true;
   currentLoc = loc;

   parmstart = generatingString.find(wxT("{"), currentLoc);
   if ((wxString::size_type)parmstart == wxString::npos)
      scanning = false;

   while (scanning)
   {
      end = generatingString.find(wxT("}"), parmstart)+1;

      if (end == (Integer)wxString::npos+1)
      {
         throw CommandException(wxT("Propagate::AssemblePropagators: PropSetup")
                                  wxT(" string ") + generatingString +
                                  wxT(" starts a stopping condition, but does not")
                                  wxT(" have a closing brace."));
      }

      tempString = generatingString.substr(parmstart+1, end-parmstart-2);

      // Split out stops, one at a time
      currentLoc = 0;
      do
      {
         commaLoc = tempString.find(wxT(","), currentLoc);
         oneStop = tempString.substr(currentLoc, commaLoc - currentLoc);
         // Remove leading white space
         while (oneStop[0] == wxT(' '))
            oneStop = oneStop.substr(1);
         // Remove trailing white space
         currentLoc = oneStop.length() - 1;
         while (oneStop[currentLoc] == wxT(' '))
            --currentLoc;
         oneStop = oneStop.substr(0, currentLoc+1);
         stopStrings.push_back(oneStop);

         currentLoc = commaLoc + 1;
      } while (commaLoc != (Integer)wxString::npos);

      currentLoc = end; // +1;  Valgrind fix from Joris Olympio

      // Skip trailing comma or white space
      while ((str[currentLoc] == wxT(',')) || (str[currentLoc] == wxT(' ')))
         ++currentLoc;
      parmstart = generatingString.find(wxT("{"), currentLoc);
      if (parmstart == (Integer)wxString::npos)
         scanning = false;
   }

   //=================================================================
   #else
   //=================================================================

   TextParser tp;
   StringArray chunks;
   wxString str1 = generatingString.substr(loc);
   // Remove all blank spaces
   str1 = GmatStringUtil::RemoveAll(str1, wxT(' '));
   wxString str2;

   #ifdef DEBUG_PARSING
   MessageInterface::ShowMessage(wxT("str1 = '%s'\n"), str1.c_str());
   #endif

   chunks = GmatStringUtil::SeparateBy(str1, wxT(")"), true, true, false);

   for (UnsignedInt i = 0; i < chunks.size(); i++)
   {
      str2 = chunks[i];
      #ifdef DEBUG_PARSING
      MessageInterface::ShowMessage(wxT("str2 = '%s'\n"), str2.c_str());
      #endif

      wxString::size_type lastCloseParen = str2.find_last_of(wxT(")"));

      // Remove last ) after }
      if (lastCloseParen == (str2.size() - 1) && str2[lastCloseParen - 1] == wxT('}'))
      {
         // Remove last )
         str2 = GmatStringUtil::RemoveLastString(str2, wxT(")"));

         // Replace last comma before { with )
         wxString::size_type openBrace = str2.find(wxT("{"));
         wxString::size_type lastComma = str2.find_last_of(wxT(","), openBrace);
         if (lastComma != str2.npos && str2[lastComma-1] != wxT(')'))
            str2[lastComma] = wxT(')');
         else if (lastComma != str2.npos)
            str2.erase(lastComma, 1);

         #ifdef DEBUG_PARSING
         MessageInterface::ShowMessage(wxT("str2 = '%s'\n"), str2.c_str());
         #endif
      }

      StringArray parts = tp.SeparateAllBrackets(str2, wxT("{}"));

      #ifdef DEBUG_PARSING
      MessageInterface::ShowMessage
         (wxT("Now separate propagator setups and stop conditions\n"));
      #endif

      for (UnsignedInt i = 0; i < parts.size(); i++)
      {
         #ifdef DEBUG_PARSING
            MessageInterface::ShowMessage(wxT("   parts[%d] = '%s'\n"), i,
                  parts[i].c_str());
         #endif

         // If it does not starts with {, it is propagator and spacecraft
         if (parts[i][0] != wxT('{'))
         {
            #ifdef DEBUG_PARSING
            MessageInterface::ShowMessage(wxT("   adding prop setups\n"));
            #endif
            parts[i] = GmatStringUtil::Trim(parts[i]);
            setupStrings.push_back(parts[i]);
         }
         else
         {
            #ifdef DEBUG_PARSING
            MessageInterface::ShowMessage(wxT("   adding stop conditions\n"));
            #endif

            if (parts[i].find(wxT(",,")) != wxString::npos)
               throw CommandException(wxT("Stopping condition parsing error; is ")
                     wxT("there an extra comma?"));

            StringArray tempStops = tp.SeparateBrackets(parts[i], wxT("{}"), wxT(","), true);
            copy(tempStops.begin(), tempStops.end(), back_inserter(stopStrings));
         }
      }
   }

   //=================================================================
   #endif
   //=================================================================
}


//------------------------------------------------------------------------------
// void ConfigurePropSetup(wxString &setupDesc)
//------------------------------------------------------------------------------
/**
 * Builds the data needed for the a PropSetup.  Stopping conditions are handled
 * separately.
 *
 * @param <setupDesc>  The string describing the PropSetup.
 */
//------------------------------------------------------------------------------
void Propagate::ConfigurePropSetup(wxString &setupDesc)
{
   #ifdef DEBUG_PROPAGATE_ASSEMBLE
      MessageInterface::ShowMessage(wxT("Building PropSetup '%s'\n"),
         setupDesc.c_str());
   #endif

   // First separate the PropSetup from the SpaceObjects
   wxString prop, sats, sat;
   wxString::size_type loc = setupDesc.find(wxT("("));
   if (loc == wxString::npos)
      throw CommandException(wxT("The propsetup string '") + setupDesc +
         wxT("' does not identify any spacecraft for propagation on ")
         + wxT("the command line\n") + generatingString);
   prop = setupDesc.substr(0, loc);
   sats = setupDesc.substr(loc);

   CleanString(prop);

   #ifdef DEBUG_PROPAGATE_ASSEMBLE
      MessageInterface::ShowMessage(wxT("   PropSetup is '%s'\n"), prop.c_str());
   #endif
   SetObject(prop, Gmat::PROP_SETUP);

   // Next the SpaceObjects
   StringArray extras;
   extras.push_back(wxT("("));
   extras.push_back(wxT(")"));
   extras.push_back(wxT(","));

   loc = 0;
   while (loc != wxString::npos)
   {
      loc = sats.find(wxT(','));
      sat = sats.substr(0, loc);
      sats = sats.substr(loc+1);
      CleanString(sat, &extras);

      #ifdef DEBUG_PROPAGATE_ASSEMBLE
         MessageInterface::ShowMessage(wxT("   Found prop object \"%s\"\n"), sat.c_str());
      #endif
      SetObject(sat, Gmat::SPACECRAFT);
   }
}


//------------------------------------------------------------------------------
// void ConfigureStoppingCondition(wxString &stopDesc)
//------------------------------------------------------------------------------
/**
 * Builds the data needed for a stopping condition.  PropSetups are handled
 * separately.
 *
 * @param <stopDesc>  The string describing the stopping condition.
 */
//------------------------------------------------------------------------------
void Propagate::ConfigureStoppingCondition(wxString &stopDesc)
{
   #ifdef DEBUG_PROPAGATE_ASSEMBLE
      MessageInterface::ShowMessage(wxT("Building Stop '%s'\n"),
         stopDesc.c_str());
   #endif

   wxString lhs, rhs = wxT("");
   wxString::size_type loc;
   StringArray extras;
   extras.push_back(wxT("{"));
   extras.push_back(wxT("}"));
   extras.push_back(wxT("="));

   loc = stopDesc.find(wxT("="));
   if (loc == wxString::npos)
   {
      lhs = stopDesc;
      CleanString(lhs, &extras);
   }
   else
   {
      lhs = stopDesc.substr(0,loc);
      CleanString(lhs, &extras);
      rhs = stopDesc.substr(loc+1);
      CleanString(rhs, &extras);

      if (lhs == wxT("StopTolerance"))
      {
         Real rval;
         if (GmatStringUtil::ToReal(rhs, rval))
            SetRealParameter(STOP_ACCURACY, rval);
         else
         {
            CommandException ce;
            ce.SetDetails(errorMessageFormatUnnamed.c_str(),
               rhs.c_str(), wxT("StopTolerance"), wxT("a Real number > 0.0"));
            throw ce;
         }
         return;
      }
   }

   #ifdef DEBUG_PROPAGATE_ASSEMBLE
      MessageInterface::ShowMessage(wxT("   Stop = '%s' with value '%s'\n"),
         lhs.c_str(), rhs.c_str());
   #endif

   // Now to work!
   wxString paramType, paramObj, paramSystem;
   GmatStringUtil::ParseParameter(lhs, paramType, paramObj, paramSystem);

   // Create the stop parameter
   wxString paramName;
   if (paramSystem == wxT(""))
      //paramName = paramObj + "." + paramType;
      paramName = lhs;
   else
      paramName = paramObj + wxT(".") + paramSystem + wxT(".") + paramType;

   #ifdef DEBUG_PROPAGATE_ASSEMBLE
   MessageInterface::ShowMessage(wxT("   Creating local StopCondition\n"));
   #endif

   StopCondition *stopCond = new StopCondition(wxT("StopOn") + paramName);

   #ifdef DEBUG_MEMORY
   MemoryTracker::Instance()->Add
      (stopCond, stopCond->GetName(), wxT("Propagate::ConfigureStoppingCondition()"),
       wxT("*stopCond = new StopCondition(StopOn + paramName)"));
   #endif

   if (find(stopNames.begin(), stopNames.end(), paramName) == stopNames.end())
   {
      #ifdef DEBUG_PROPAGATE_ASSEMBLE
      MessageInterface::ShowMessage(wxT("   Adding '%s' to stopNames\n"), paramName.c_str());
      #endif
      stopNames.push_back(paramName);
      stopWrappers.push_back(NULL);
   }

   // Handle some static member initialization if this is the first opportunity
   if (stopCondEpochID == -1)
   {
      stopCondEpochID = stopCond->GetParameterID(wxT("Epoch"));
      stopCondBaseEpochID = stopCond->GetParameterID(wxT("BaseEpoch"));
      stopCondStopVarID = stopCond->GetParameterID(wxT("StopVar"));
   }

   // Setup for backwards propagation
   stopCond->SetPropDirection(direction);  // Use direction of props
   stopCond->SetStringParameter(stopCondStopVarID, paramName);
   SetObject(stopCond, Gmat::STOP_CONDITION);

   #ifdef DEBUG_PROPAGATE_ASSEMBLE
   MessageInterface::ShowMessage(wxT("lhs paramObj='%s'\n"), paramObj.c_str());
   #endif

   if (paramObj != wxT("") && !GmatStringUtil::IsNumber(paramObj))
      TakeAction(wxT("SetStopSpacecraft"), paramObj);


   if (paramType != wxT("Apoapsis") && paramType != wxT("Periapsis"))
   {
      #ifdef DEBUG_PROPAGATE_ASSEMBLE
         MessageInterface::ShowMessage(wxT("Propagate::AssemblePropagators()")
            wxT(" component = <%s>\n"), rhs.c_str());
      #endif

      // create goal parameter
      wxString component = rhs;

      GmatStringUtil::ParseParameter(rhs, paramType, paramObj, paramSystem);
      #ifdef DEBUG_PROPAGATE_ASSEMBLE
      MessageInterface::ShowMessage(wxT("rhs paramObj='%s'\n"), paramObj.c_str());
      #endif

      if (paramObj != wxT("") && !GmatStringUtil::IsNumber(paramObj))
         TakeAction(wxT("SetStopSpacecraft"), paramObj);

      if (find(goalNames.begin(), goalNames.end(), component) == goalNames.end())
      {
         #ifdef DEBUG_PROPAGATE_ASSEMBLE
         MessageInterface::ShowMessage(wxT("   Adding '%s' to goalNames\n"), component.c_str());
         #endif
         goalNames.push_back(component);
         goalWrappers.push_back(NULL);
      }

      stopCond->SetStringParameter(wxT("Goal"), component);
   }
   else
   {
      if (rhs.length() != 0)
      {
         throw CommandException(wxT("Stopping condition ") + paramType +
            wxT(" does not take a value, but it is set using the string '") +
            stopDesc + wxT("' in the line\n'") + generatingString + wxT("'"));
      }
   }
}


//------------------------------------------------------------------------------
// void CleanString(wxString &theString, const StringArray *extras)
//------------------------------------------------------------------------------
/**
 * Strips off leading and trailing whitespace, and additional characters if
 * specified.
 *
 * @param <theString>  The string that -- might -- need cleaned.
 * @param <extras>     All additional characters (other than a space) that
 *                     should be stripped off.
 */
//------------------------------------------------------------------------------
void Propagate::CleanString(wxString &theString, const StringArray *extras)
{
   UnsignedInt loc, len = theString.length();
   bool keepGoing = false;

   if (len == 0)
      return;

   // Clean up the start of the string
   for (loc = 0; loc < len; ++loc)
   {
      if ((theString[loc] != wxT(' ')) && (theString[loc] != wxT('\'')))
      {
         if (extras != NULL)
            for (StringArray::const_iterator i = extras->begin(); i != extras->end(); ++i)
               if (theString[loc] == (*i)[0])
                  keepGoing = true;
         if (!keepGoing)
            break;
         else
            keepGoing = false;
      }
   }
   theString = theString.substr(loc);

   // Clean up the end of the string
   keepGoing = false;
   for (loc = theString.length() - 1; loc >= 0; --loc)
   {
      if ((theString[loc] != wxT(' ')) && (theString[loc] != wxT('\'')))
      {
         if (extras != NULL)
            for (StringArray::const_iterator i = extras->begin(); i != extras->end(); ++i)
               if (theString[loc] == (*i)[0])
                  keepGoing = true;
         if (!keepGoing)
            break;
         else
            keepGoing = false;
      }
   }
   theString = theString.substr(0, loc+1);
}


//------------------------------------------------------------------------------
// void SetTransientForces(std::vector<PhysicalModel*> *tf)
//------------------------------------------------------------------------------
/**
 * Sets the array of transient forces, so it can be passed to the PropSetups.
 *
 * @param <tf> The array of transient forces.
 */
//------------------------------------------------------------------------------
void Propagate::SetTransientForces(std::vector<PhysicalModel*> *tf)
{
   transientForces = tf;
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 * Performs the initialization needed to run the Propagate command.
 *
 * @return true if the GmatCommand is initialized, false if an error occurs.
 */
//------------------------------------------------------------------------------
bool Propagate::Initialize()
{
   #if DEBUG_PROPAGATE_INIT
      MessageInterface::ShowMessage(wxT("Propagate::Initialize() <%p> entered\n"), this);
      MessageInterface::ShowMessage(wxT("  Size of propName is %d\n"),
                                    propName.size());
      for (UnsignedInt ind = 0; ind < propName.size(); ++ind)
         MessageInterface::ShowMessage(wxT("     %d:  %s\n"),
                                    propName.size(), propName[ind].c_str());

      MessageInterface::ShowMessage(wxT("  Direction is %s\n"),
                                    (direction == 1.0?wxT("Forwards"):wxT("Backwards")));
   #endif

   PropagationEnabledCommand::Initialize();

   inProgress = false;
   hasFired = false;
   UnsignedInt index = 0;
   sats.clear();
   SpaceObject *so;
   wxString pName;
   GmatBase *mapObj = NULL;

   // Ensure that we are using fresh objects when buffering stops
   EmptyBuffer();
   cloneCount = 0;

   // Remove old PropSetups
   for (std::vector<PropSetup*>::iterator ps = prop.begin(); ps != prop.end();
        ++ps)
   {
      PropSetup *oldPs = *ps;
      *ps = NULL;
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (oldPs, oldPs->GetName(), wxT("Propagate::Initialize()"),
          wxT("deleting oldPs"));
      #endif
      delete oldPs;
   }
   if (prop.size() > 0)
   {
      for (std::vector<PropSetup*>::iterator ps = prop.begin();
            ps != prop.end(); ++ps)
         delete (*ps);

      prop.clear();
      p.clear();
      fm.clear();
   }

   for (StringArray::iterator i = propName.begin(); i != propName.end(); ++i)
   {
      if (satName.size() <= index)
         throw CommandException(wxT("Size mismatch for SpaceObject names\n"));

      if ((*i)[0] == wxT('-'))
         pName = i->substr(1);
      else
        pName = *i;

      if ((mapObj = FindObject(pName)) == NULL)
         throw CommandException(
            wxT("Propagate command cannot find Propagator Setup \"") + (pName) +
            wxT("\"\n"));

      if (satName[index]->empty())
         throw CommandException(
            wxT("Propagate command does not have a SpaceObject for ") + (pName) +
            wxT(" in \n\"") + generatingString + wxT("\"\n"));

      if (stopWhen.empty())
         singleStepMode = true;
      else
         singleStepMode = false;

      PropSetup *clonedProp = (PropSetup *)(mapObj->Clone());
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Add
         (clonedProp, clonedProp->GetName(), wxT("Propagate::Initialize()"),
          wxT("(PropSetup *)(mapObj->Clone())"));
      #endif
      //prop.push_back((PropSetup *)(mapObj->Clone()));
      prop.push_back(clonedProp);
      ++cloneCount;
      if (!prop[index])
         return false;

      Propagator *p = prop[index]->GetPropagator();
      if (!p)
         throw CommandException(wxT("Propagator not set in PropSetup\n"));
      p->TakeAction(wxT("PrepareForRun"));

      // Toss the spacecraft into the prop state manager
      ODEModel *odem = prop[index]->GetODEModel();
      if ((!odem) && p->UsesODEModel())
         throw CommandException(wxT("ForceModel not set in PropSetup\n"));

      PropagationStateManager *psm = prop[index]->GetPropStateManager();
      StringArray::iterator scName;
//      StringArray owners, elements;

      /// @todo Check to see if All and All.Epoch belong in place for all modes.
//      owners.push_back("All");
//      elements.push_back("All.epoch");

      bool finiteBurnActive = false;

      for (scName = satName[index]->begin(); scName != satName[index]->end();
           ++scName)
      {
         #if DEBUG_PROPAGATE_INIT
            MessageInterface::ShowMessage(
                  wxT("   Adding '%s' to prop state manager '%s'\n"),
               scName->c_str(), i->c_str());
         #endif
         if ((mapObj = FindObject(*scName)) == NULL)
         {
            #if DEBUG_PROPAGATE_INIT
               MessageInterface::ShowMessage(wxT("   '%s' is not an object; ")
                     wxT("attempting to set as a prop property\n"),
                     scName->c_str());
            #endif
            if (psm->SetProperty(*scName) == false)
            {
               wxString errmsg = wxT("Unknown SpaceObject property \"");
               errmsg += *scName;
               errmsg += wxT("\"");
               throw CommandException(errmsg);
            }
         }
         else
         {
            psm->SetObject(mapObj);

            so = (SpaceObject*)mapObj;
            if (epochID == -1)
               epochID = so->GetParameterID(wxT("A1Epoch"));
            if (so->IsManeuvering())
               finiteBurnActive = true;
            sats.push_back(so);
            if (sats.size() > 1)
               if (so->GetRealParameter(epochID) !=
                     sats[0]->GetRealParameter(epochID))
               {
                  wxString errorString;
                  errorString << wxT("Coupled propagation epoch mismatch between ")
                              << sats[0]->GetName()
                              << wxT(" (epoch = ")
                              << sats[0]->GetRealParameter(epochID)
                              << wxT(") and ")
                              << so->GetName()
                              << wxT(" (epoch = ")
                              << so->GetRealParameter(epochID)
                              << wxT(")");
                  throw CommandException(errorString);
               }

            AddToBuffer(so);

            if (so->GetType() == Gmat::FORMATION)
               ((Formation*)(so))->BuildState();
//            FillFormation(so, owners, elements);
//            else
//            {
//               SetNames(so->GetName(), owners, elements);
//            }
         }
      }

      // Check for finite thrusts and update the force model if there are any
      if (finiteBurnActive == true)
      {
         if (odem != NULL)
            AddTransientForce(satName[index], odem, psm);
         else
            MessageInterface::ShowMessage(wxT("Spacecraft is performing a ")
                  wxT("finite maneuver but also propagating with an ephemeris ")
                  wxT("propagator; no independent maneuvering will be ")
                  wxT("performed.\n")); //, satName[index].c_str());
      }

      if (psm->BuildState() == false)
         throw CommandException(wxT("Could not build the state for the command \n") +
               generatingString);
      if (psm->MapObjectsToVector() == false)
         throw CommandException(wxT("Could not map state objects for the command\n") +
               generatingString);

      if (p->UsesODEModel())
      {
         odem->SetState(psm->GetState());
         // Set solar system to ForceModel for Propagate inside a GmatFunction
         odem->SetSolarSystem(solarSys);
      }
      else
      {
         ObjectArray pObjects;

         psm->GetStateObjects(pObjects, Gmat::SPACEOBJECT);
         for (UnsignedInt i = 0; i < pObjects.size(); ++i)
            p->SetRefObject(pObjects[i], Gmat::SPACEOBJECT,
                  pObjects[i]->GetName());
         p->SetSolarSystem(solarSys);
      }

      #ifdef DEBUG_PUBLISH_DATA
      MessageInterface::ShowMessage
            (wxT("Propagate::Initialize() '%s' registering published data\n"),
             GetGeneratingString(Gmat::NO_COMMENTS).c_str());
      #endif

//      streamID = publisher->RegisterPublishedData(this, streamID, owners,
//            elements);

      if (p->UsesODEModel())
         p->SetPhysicalModel(odem);
      p->SetRealParameter(wxT("InitialStepSize"),
         fabs(p->GetRealParameter(wxT("InitialStepSize"))) * direction);
      p->Initialize();

      // Set spacecraft parameters for forces that need them
      if (p->UsesODEModel())
         if (odem->SetupSpacecraftData(&sats, 0) <= 0)
            throw PropagatorException(wxT("Propagate::Initialize -- ")
                  wxT("ODE model cannot set spacecraft parameters"));

      ++index;
   } // End of loop through PropSetups

   // Prep the publisher
   StringArray owners, elements;
   owners.push_back(wxT("All"));
   elements.push_back(wxT("All.epoch"));

   for (UnsignedInt i = 0; i < prop.size(); ++i)
   {
      for (StringArray::iterator scName = satName[i]->begin();
           scName != satName[i]->end(); ++scName)
      {
         SpaceObject *so = NULL;
         for (UnsignedInt i = 0; i < sats.size(); ++i)
            if (sats[i]->GetName() == (*scName))
               so = (SpaceObject*)sats[i];
         if (so == NULL)
            continue;
         if (so->GetType() == Gmat::FORMATION)
            FillFormation(so, owners, elements);
         else
         {
            SetNames(so->GetName(), owners, elements);
         }
      }
   }
   streamID = publisher->RegisterPublishedData(this, streamID, owners,
         elements);

   initialized = true;

   stopSats.clear();
   // Setup spacecraft array used for stopping conditions
   for (StringArray::iterator sc = stopSatNames.begin();
        sc != stopSatNames.end(); ++sc)
   {
      if ((mapObj = FindObject(*sc)) == NULL)
      {
         wxString errmsg = wxT("Unknown SpaceObject \"");
         errmsg += *sc;
         errmsg += wxT("\" used in stopping conditions");
         throw CommandException(errmsg);
      }
      so = (SpaceObject*)mapObj;
      stopSats.push_back(so);
   }

   #if DEBUG_PROPAGATE_INIT
      for (UnsignedInt i=0; i<stopSats.size(); i++)
         MessageInterface::ShowMessage
            (wxT("   stopSats[%d]=%s\n"), i, stopSats[i]->GetName().c_str());
      for (UnsignedInt i=0; i<stopWhen.size(); i++)
         MessageInterface::ShowMessage
            (wxT("   stopWhen[%d]=%s\n"), i, stopWhen[i]->GetName().c_str());
   #endif

   if ((stopWhen.size() == 0) && !singleStepMode)
      throw CommandException(wxT("No stopping conditions specified!"));

   if (solarSys != NULL)
   {
      StringArray refNames;

      for (UnsignedInt i=0; i<stopWhen.size(); i++)
      {
         stopWhen[i]->SetSolarSystem(solarSys);

         //Set StopCondition parameters
         refNames = stopWhen[i]->GetRefObjectNameArray(Gmat::PARAMETER);

         for (UnsignedInt j=0; j<refNames.size(); j++)
         {
            #if DEBUG_PROPAGATE_INIT
               MessageInterface::ShowMessage(wxT("   refNames[%d]='%s'\n"), j,
                  refNames[j].c_str());
            #endif
            mapObj = FindObject(refNames[j]);
            stopWhen[i]->SetRefObject(mapObj, Gmat::PARAMETER, refNames[j]);
         }

         try
         {
            stopWhen[i]->Initialize();
            stopWhen[i]->SetSpacecraft((SpaceObject*)sats[0]);

            if (!stopWhen[i]->IsInitialized())
            {
               initialized = false;
               MessageInterface::ShowMessage(
                  wxT("Propagate::Initialize() StopCondition %s is not initialized.\n"),
                  stopWhen[i]->GetName().c_str());
               break;
            }
         }
         catch (BaseException &be)
         {
            CommandException ce;
            ce.SetDetails(wxT("%s in %s\n"), be.GetFullMessage().c_str(),
                          GetGeneratingString(Gmat::NO_COMMENTS).c_str());
            throw ce;
         }
      }
   }
   else
   {
      initialized = false;
      MessageInterface::ShowMessage
         (wxT("Propagate::Initialize() SolarSystem not set in StopCondition"));
   }

   #if DEBUG_PROPAGATE_INIT
      MessageInterface::ShowMessage(wxT("Propagate::Initialize() complete.\n"));
   #endif

   #ifdef DEBUG_PROPAGATE_DIRECTION
      MessageInterface::ShowMessage(wxT("Propagate::Initialize():")
                                    wxT(" Propagators Identified:\n"));
      for (StringArray::iterator i = propName.begin(); i != propName.end();
           ++i)
         MessageInterface::ShowMessage(wxT("   \"%s\" running %s\n"), i->c_str(),
         (direction > 0.0 ? wxT("forwards") : wxT("backwards")));
   #endif

   if (singleStepMode)
   {
      commandSummary = wxT("Command Summary: ");
      commandSummary += typeName;
      commandSummary += wxT(" Command\nSummary not available in single step mode\n");
   }

   #ifdef DUMP_PLANET_DATA
      if (body[0] == NULL)
         body[0] = solarSys->GetBody(wxT("Earth"));
      if (body[1] == NULL)
         body[1] = solarSys->GetBody(wxT("Sun"));
      if (body[2] == NULL)
         body[2] = solarSys->GetBody(wxT("Luna"));
      if (body[3] == NULL)
         body[3] = solarSys->GetBody(wxT("Mercury"));
      if (body[4] == NULL)
         body[4] = solarSys->GetBody(wxT("Venus"));
      if (body[5] == NULL)
         body[5] = solarSys->GetBody(wxT("Mars"));
      if (body[6] == NULL)
         body[6] = solarSys->GetBody(wxT("Jupiter"));
      if (body[7] == NULL)
         body[7] = solarSys->GetBody(wxT("Saturn"));
      if (body[8] == NULL)
         body[8] = solarSys->GetBody(wxT("Uranus"));
      if (body[9] == NULL)
         body[9] = solarSys->GetBody(wxT("Neptune"));
      if (body[10] == NULL)
         body[10] = solarSys->GetBody(wxT("Pluto"));

      bodiesDefined = 11;
   #endif

   #if DEBUG_PROPAGATE_INIT
      MessageInterface::ShowMessage
         (wxT("Propagate::Initialize() <%p> returning initialized=%d\n"), this, initialized);
   #endif

   return initialized;
}

//------------------------------------------------------------------------------
// void FillFormation(SpaceObject *so)
//------------------------------------------------------------------------------
/**
 * Fill in the components of a formation (recursively).
 *
 * @param <so> The SpaceObject that needs to be filled.
 */
//------------------------------------------------------------------------------
void Propagate::FillFormation(SpaceObject *so, StringArray& owners,
                              StringArray& elements)
{
   GmatBase *mapObj = NULL;
   static Integer soEpochId = -1;
   if ((so == NULL) || (so->GetType() != Gmat::FORMATION))
      throw CommandException(wxT("Invalid SpaceObject passed to FillFormation"));

   if (soEpochId == -1)
      soEpochId = so->GetParameterID(wxT("A1Epoch"));

   StringArray comps = so->GetStringArrayParameter(so->GetParameterID(wxT("Add")));
   SpaceObject *el;
   Real ep;

   for (StringArray::iterator i = comps.begin(); i != comps.end(); ++i)
   {
      if ((mapObj = FindObject(*i)) == NULL)
         throw CommandException(wxT("Formation ") + so->GetName() +
            wxT(" uses unknown object named '") + (*i) + wxT("'"));

      el = (SpaceObject*)mapObj;
      if (i == comps.begin())
      {
         ep = el->GetRealParameter(soEpochId);
         so->SetRealParameter(soEpochId, ep);
      }

      so->SetRefObject(el, el->GetType(), el->GetName());
      if (el->GetType() == Gmat::FORMATION)
         FillFormation(el, owners, elements);
      else     // Setup spacecraft data descriptions
         SetNames(el->GetName(), owners, elements);
   }

   ((Formation*)(so))->BuildState();
}


//------------------------------------------------------------------------------
// GmatCommand* GetNext()
//------------------------------------------------------------------------------
/**
 * Returns pointer to next command to be executed.
 *
 * Propagate::GetNext overrides the base class's GetNext method so that it can
 * poll the moderator for user interrupts periodically.  If the stopping
 * conditions have not yet been met, GetNext returns this object; otherwise, it
 * returns the next one in the command list.
 *
 * @return The next pointer, as described above.
 */
//------------------------------------------------------------------------------
GmatCommand* Propagate::GetNext()
{
   if (!inProgress)
      return next;
   return this;
}

//------------------------------------------------------------------------------
// void PrepareToPropagate()
//------------------------------------------------------------------------------
/**
 * Performs initialization needed immediately before propagating.
 */
//------------------------------------------------------------------------------
void Propagate::PrepareToPropagate()
{
   #ifdef DEBUG_PROPAGATE_INIT
      MessageInterface::ShowMessage
         (wxT("Propagate::PrepareToPropagate() <%p> entered\n"), this);
      GmatState *dstate = prop[0]->GetPropStateManager()->GetState();
      Integer dimension = dstate->GetSize();
      MessageInterface::ShowMessage(
            wxT("PrepareToPropagate top; State vector contents and fm state are\n")
            wxT("   Epoch = %.12lf\n"), (dstate->GetEpoch()));
      for (Integer index = 0; index < dimension; ++index)
      {
         MessageInterface::ShowMessage(wxT("   %d:   %.12lf\n"), index,
               (*dstate)[index]);
      }
   #endif

   dim = 0;

   if (hasFired == true)
   {
      // Handle the transient forces
      for (ObjectArray::iterator sc = sats.begin();
           sc != sats.end(); ++sc)
      {
         if (((SpaceObject*)(*sc))->IsManeuvering())
         {
            #ifdef DEBUG_FINITE_MANEUVER
               MessageInterface::ShowMessage(
                  wxT("SpaceObject %s is maneuvering\n"), (*sc)->GetName().c_str());
            #endif

            // Add the force
            for (UnsignedInt index = 0; index < prop.size(); ++index)
            {
               if (prop[index]->GetPropagator()->UsesODEModel())
               {
                  for (std::vector<PhysicalModel*>::iterator
                        i = transientForces->begin();
                        i != transientForces->end(); ++i)
                  {
                     #ifdef DEBUG_TRANSIENT_FORCES
                     MessageInterface::ShowMessage
                        (wxT("Propagate::PrepareToPropagate() Adding ")
                              wxT("transientForce<%p>'%s'\n"), *i,
                              (*i)->GetName().c_str());
                     #endif
                     prop[index]->GetODEModel()->AddForce(*i);

                     // Refresh ODE model mapping, since a new force was added
                     if (prop[index]->GetODEModel()->BuildModelFromMap()
                           == false)
                        throw CommandException(wxT("Unable to assemble the ODE ")
                              wxT("model  after adding a finite burn for ") +
                              (*i)->GetName());
                  }
               }
            }
         }
         #ifdef DEBUG_FINITE_MANEUVER
            else
               MessageInterface::ShowMessage(wxT("SpaceObject %s is not ")
                     wxT("maneuvering\n"), (*sc)->GetName().c_str());
         #endif
      }

      for (Integer n = 0; n < (Integer)prop.size(); ++n)
      {
         elapsedTime[n] = 0.0;
         currEpoch[n]   = 0.0;
         if (prop[n]->GetPropagator()->UsesODEModel())
         {
            fm[n]->SetTime(0.0);
            fm[n]->SetPropStateManager(prop[n]->GetPropStateManager());
            fm[n]->UpdateInitialData();
            dim += fm[n]->GetDimension();
         }
         else
         {
            p[n]->SetPropStateManager(prop[n]->GetPropStateManager());
            dim = p[n]->GetDimension();
         }

         p[n]->Initialize();
         p[n]->Update(direction > 0.0);
      }

      baseEpoch.clear();

      for (Integer n = 0; n < (Integer)prop.size(); ++n)
      {
         #if DEBUG_PROPAGATE_EXE
            MessageInterface::ShowMessage
               (wxT("Propagate::PrepareToPropagate() SpaceObject names\n"));

            MessageInterface::ShowMessage
               (wxT("SpaceObject Count = %d\n"), satName[n]->size());
            StringArray *sar = satName[n];
            for (Integer i=0; i < (Integer)satName[n]->size(); i++)
            {
               MessageInterface::ShowMessage
                  (wxT("   SpaceObjectName[%d] = %s\n"), i, (*sar)[i].c_str());
            }
         #endif

         if (satName[n]->empty())
            throw CommandException(
               wxT("Propagator has no associated space objects."));

         GmatBase* sat1 = FindObject(*satName[n]->begin());
         baseEpoch.push_back(sat1->GetRealParameter(epochID));

         if (prop[n]->GetPropagator()->UsesODEModel())
         {
            elapsedTime[n] = fm[n]->GetTime();
         }
         else
         {
            elapsedTime[n] = p[n]->GetTime();
            baseEpoch[n] -= elapsedTime[n] / GmatTimeConstants::SECS_PER_DAY;
         }
         currEpoch[n] = baseEpoch[n] + elapsedTime[n] /
            GmatTimeConstants::SECS_PER_DAY;
         #if DEBUG_PROPAGATE_DIRECTION
            MessageInterface::ShowMessage(
               wxT("Propagate::PrepareToPropagate() running %s %s.\n"),
               prop[n]->GetName().c_str(),
               (prop[n]->GetPropagator()->GetRealParameter(wxT("InitialStepSize")) > 0.0
                  ? wxT("forwards") : wxT("backwards")));
             MessageInterface::ShowMessage(wxT("   direction =  %lf.\n"),
               direction);
         #endif
      }

      // Now setup the stopping condition elements
      #if DEBUG_PROPAGATE_INIT
         if (p[0]->UsesODEModel())
         {
            MessageInterface::ShowMessage
               (wxT("Propagate::PrepareToPropagate() Propagate start; epoch = %f\n"),
             (baseEpoch[0] + fm[0]->GetTime() / GmatTimeConstants::SECS_PER_DAY));
            MessageInterface::ShowMessage
               (wxT("Propagate::PrepareToPropagate() Propagate start; fm epoch = %f\n"),
               (fm[0]->GetRealParameter(fm[0]->GetParameterID(wxT("Epoch")))));
         }
         else
         {
            MessageInterface::ShowMessage(wxT("Propagator state data (No ODE ")
                  wxT("Model):\n   To be filled in\n"));
         }
         Integer stopCondCount = stopWhen.size();
         MessageInterface::ShowMessage
            (wxT("Propagate::PrepareToPropagate() stopCondCount = %d\n"), stopCondCount);

         for (Integer i=0; i<stopCondCount; i++)
         {
            MessageInterface::ShowMessage
               (wxT("Propagate::PrepareToPropagate() stopCondName[%d]=%s\n"), i,
                      stopWhen[i]->GetName().c_str());
         }
      #endif

      stopCondMet = false;
      stopEpoch = 0.0;
      wxString stopVar;
      Real stopEpochBase;

      #ifdef DEBUG_STOPPING_CONDITIONS
         if (!singleStepMode)
            MessageInterface::ShowMessage(
               wxT("Stopping condition IDs are [%d, %d, %d]\n"),
               stopCondEpochID, stopCondBaseEpochID, stopCondStopVarID);
      #endif

      try
      {
         for (UnsignedInt i = 0; i < stopWhen.size(); ++i)
         {
            if (i >= stopSats.size())
               throw CommandException(wxT("Stopping condition ") +
               stopWhen[i]->GetName() + wxT(" has no associated spacecraft."));

            #if DEBUG_PROPAGATE_INIT
               MessageInterface::ShowMessage(
                  wxT("Propagate::PrepareToPropagate() stopSat = %s\n"),
                  stopSats[i]->GetName().c_str());
            #endif

            stopEpochBase = stopSats[i]->GetRealParameter(epochID);

            // StopCondition need new base epoch
            stopWhen[i]->SetRealParameter(stopCondBaseEpochID, stopEpochBase);
         }
      }
      catch (BaseException &ex)
      {
         // Use exception to remove Visual C++ warning
         ex.GetMessageType();
         MessageInterface::ShowMessage(
            wxT("Propagate::PrepareToPropagate() Exception while initializing stopping ")
            wxT("conditions\n"));
         inProgress = false;
         throw;
      }

      inProgress = true;
   }
   else
   {
      // Set the prop state managers for the PropSetup ODEModels
      for (std::vector<PropSetup*>::iterator i=prop.begin(); i != prop.end(); ++i)
      {
         ODEModel *ode = (*i)->GetODEModel();
         if (ode != NULL)    // Only do this for the PropSetups that integrate
            ode->SetPropStateManager((*i)->GetPropStateManager());
         else
            (*i)->GetPropagator()->SetPropStateManager(
                  (*i)->GetPropStateManager());
      }

      // Initialize the subsystem
      #ifdef DEBUG_PROPAGATE_INIT
         MessageInterface::ShowMessage(wxT("Initializing Propagate command\n"));
      #endif
      Initialize();

      // Loop through the PropSetups and build the models
      for (std::vector<PropSetup*>::iterator i=prop.begin(); i != prop.end(); ++i)
      {
         #ifdef DEBUG_PROPAGATE_INIT
            MessageInterface::ShowMessage(wxT("Building models for Setup %s\n"),
                  (*i)->GetName().c_str());
         #endif
         ODEModel *ode = (*i)->GetODEModel();
         if (ode != NULL)    // Only do this for the PropSetups that integrate
         {
            // Build the ODE model
            ode->SetPropStateManager((*i)->GetPropStateManager());
            if (ode->BuildModelFromMap() == false)
               throw CommandException(wxT("Unable to assemble the ODE model for ") +
                     (*i)->GetName());
         }
         else
         {
            (*i)->GetPropagator()->SetPropStateManager((*i)->
                  GetPropStateManager());
         }
      }

      p.clear();
      fm.clear();
      psm.clear();
      baseEpoch.clear();
      currEpoch.clear();

      #ifdef DEBUG_PROPAGATE_INIT
         MessageInterface::ShowMessage(wxT("Loading p and fm vectors\n"));
      #endif
      for (Integer n = 0; n < (Integer)prop.size(); ++n)
      {
         elapsedTime.push_back(0.0);

         p.push_back(prop[n]->GetPropagator());
         if (prop[n]->GetPropagator()->UsesODEModel())
         {
            fm.push_back(prop[n]->GetODEModel());
            dim += fm[n]->GetDimension();
         }
         else
         {
            fm.push_back(NULL);
            dim += p[n]->GetDimension();
         }

         psm.push_back(prop[n]->GetPropStateManager());
         currEpoch.push_back(psm[n]->GetState()->GetEpoch());

         #ifdef DEBUG_PROPAGATE_INIT
            MessageInterface::ShowMessage(wxT("Initializing propagator %d\n"), n);
         #endif
         p[n]->Initialize();
         psm[n]->MapObjectsToVector();

         p[n]->Update(direction > 0.0);
         if (prop[n]->GetPropagator()->UsesODEModel())
         {
            state = fm[n]->GetState();
            j2kState = fm[n]->GetJ2KState();
         }
         else
         {
            state = p[n]->GetState();
            j2kState = p[n]->GetJ2KState();
         }
         baseEpoch.push_back(psm[n]->GetState()->GetEpoch());

         #ifdef DEBUG_PROPAGATE_INIT
            GmatState *dstate = psm[n]->GetState();
            Integer dimension = dstate->GetSize();
            MessageInterface::ShowMessage(
                     wxT("State vector contents and fm state are\n")
                     wxT("   Epoch = %.12lf\n"), (dstate->GetEpoch()));
            for (Integer index = 0; index < dimension; ++index)
            {
               MessageInterface::ShowMessage(wxT("   %d:   %.12lf   %.12lf\n"), index,
                     (*dstate)[index], state[index]);
            }
         #endif

         #ifdef DEBUG_PROPAGATE_INIT
            MessageInterface::ShowMessage(wxT("Setting up stopping conditions\n"));
         #endif

         // Now setup the stopping condition elements
         #if DEBUG_PROPAGATE_INIT
            if (fm[0]!= NULL)
            {
               MessageInterface::ShowMessage
                  (wxT("Propagate::PrepareToPropagate() Propagate start; epoch = %f\n"),
                (baseEpoch[0] + fm[0]->GetTime() / GmatTimeConstants::SECS_PER_DAY));
               MessageInterface::ShowMessage
                  (wxT("Propagate::PrepareToPropagate() Propagate start; fm epoch = %f\n"),
                  (fm[0]->GetRealParameter(fm[0]->GetParameterID(wxT("Epoch")))));
            }

            Integer stopCondCount = stopWhen.size();
            MessageInterface::ShowMessage
               (wxT("Propagate::PrepareToPropagate() stopCondCount = %d\n"), stopCondCount);
            for (Integer i=0; i<stopCondCount; i++)
            {
               MessageInterface::ShowMessage
                  (wxT("Propagate::PrepareToPropagate() stopCondName[%d]=%s\n"), i,
                         stopWhen[i]->GetName().c_str());
            }
         #endif

         stopCondMet = false;
         stopEpoch = 0.0;
         wxString stopVar;
         Real stopEpochBase;

         try
         {
            for (UnsignedInt i = 0; i<stopWhen.size(); i++)
            {
               if (i >= stopSats.size())
                  throw CommandException(wxT("Stopping condition ") +
                  stopWhen[i]->GetName() + wxT(" has no associated spacecraft."));

               #if DEBUG_PROPAGATE_INIT
                  MessageInterface::ShowMessage(
                     wxT("Propagate::PrepareToPropagate() stopSat = %s, stopWhen = %s, ")
                     wxT("stopGoal = %s\n"), stopSats[i]->GetName().c_str(),
                     stopWhen[i]->GetName().c_str(),
                     stopWhen[i]->GetStringParameter(wxT("Goal")).c_str());
               #endif

               stopEpochBase = stopSats[i]->GetRealParameter(epochID);

               // StopCondition need new base epoch
               stopWhen[i]->SetRealParameter(stopCondBaseEpochID, stopEpochBase);
            }
         }
         catch (BaseException &ex)
         {
            // Use exception to remove Visual C++ warning
            ex.GetMessageType();
            MessageInterface::ShowMessage(
               wxT("Propagate::PrepareToPropagate() Exception while initializing stopping ")
               wxT("conditions\n"));
            inProgress = false;
            throw;
         }

         hasFired = true;
         inProgress = true;

         #ifdef DEBUG_FIRST_CALL
            if (state)
            {
               MessageInterface::ShowMessage(wxT("Debugging first step\n"));
               MessageInterface::ShowMessage(
                  wxT("State = [%16.9lf %16.9lf %16.9lf %16.14lf %16.14lf %16.14lf]\n"),
                  state[0], state[1], state[2], state[3], state[4], state[5]);
               MessageInterface::ShowMessage(
                  wxT("Propagator = \n%s\n"),
                  prop[0]->GetGeneratingString(Gmat::SCRIPTING, wxT("   ")).c_str());
            }
            else
               MessageInterface::ShowMessage(wxT("Debugging first step: State not set\n"));
            firstStepFired = true;
         #endif

      }
   }

   if (pubdata)
   {
      #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (pubdata, wxT("pubdata"), wxT("Propagate::PrepareToPropagate()"),
             wxT("deleting pub data"));
      #endif
      delete [] pubdata;
   }

   #ifdef DEBUG_PUBLISH_DATA
      MessageInterface::ShowMessage(wxT("Setting pubdata with dimension %d\n"),
               dim+1);
   #endif
   pubdata = new Real[dim+1];
   #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Add
         (pubdata, wxT("pubdata"), wxT("Propagate::PrepareToPropagate()"),
          wxT("pubdata = new Real[dim+1]"));
   #endif

   // Publish the data
   pubdata[0] = currEpoch[0];

   // Walk the PropSetups to load the pubdata array
   Integer index = 1, size;
   Real *js;
   for (UnsignedInt i = 0; i < prop.size(); ++i)
   {
      if (p[i]->UsesODEModel())
      {
         js = fm[i]->GetJ2KState();
         size = fm[i]->GetDimension();
      }
      else
      {
         js   = p[i]->GetJ2KState();
         size = p[i]->GetDimension();
      }
      memcpy(&pubdata[index], js, size*sizeof(Real));
      index += size;
   }

   #ifdef DEBUG_PUBLISH_DATA
      MessageInterface::ShowMessage
         (wxT("Propagate::PrepareToPropagate()\n   '%s'\nPublishing initial %d ")
               wxT("data to stream %d, 1st data = ["),
          GetGeneratingString(Gmat::NO_COMMENTS).c_str(), dim+1, streamID);
      for (Integer i = 0; i < dim+1; ++i)
      {
         MessageInterface::ShowMessage(wxT("%.12lf"), pubdata[i]);
         if (i < dim)
            MessageInterface::ShowMessage(wxT("   "));
         else
            MessageInterface::ShowMessage(wxT("]\n"));
      }
   #endif

// Start on a fix for bug 648; also uncomment the execute block for single
// step to work on this approach
//   // Only publish if there is an unpublished SpaceObject in the data
//   bool unpublishedObjectExists = false;
//   for (UnsignedInt i = 0; i < sats.size(); ++i)
//   {
//      SpaceObject *sc = (SpaceObject *)sats[i];
//      if (!sc->HasPublished())
//         unpublishedObjectExists = true;
//   }
//   if (unpublishedObjectExists)
//   {
//      #ifdef DEBUG_PUBLISH_DATA
//         MessageInterface::ShowMessage("Unpublished Object Found\n");
//      #endif
      publisher->Publish(this, streamID, pubdata, dim+1);
//   }
//   if (unpublishedObjectExists == true)
//   {
//      for (UnsignedInt i = 0; i < sats.size(); ++i)
//      {
//         SpaceObject *sc = (SpaceObject *)sats[i];
//         sc->HasPublished(true);
//      }
//   }
   #ifdef DEBUG_PROPAGATE_INIT
      MessageInterface::ShowMessage
         (wxT("Propagate::PrepareToPropagate() <%p> leaving\n"), this);
   #endif
}


//------------------------------------------------------------------------------
// bool Execute()
//------------------------------------------------------------------------------
/**
 * Propagate the assigned members to the desired stopping condition
 *
 * @return true if the Command runs to completion, false if an error
 *         occurs.
 */
//------------------------------------------------------------------------------
bool Propagate::Execute()
{
   #if DEBUG_PROPAGATE_EXE
      MessageInterface::ShowMessage
         (wxT("Propagate::Execute() <%s> entered.\n   initialized = %d, inProgress = %d\n"),
          GetGeneratingString().c_str(), initialized, inProgress);
   #endif

   if (initialized == false)
      throw CommandException(wxT("Propagate Command was not Initialized\n"));

   // Parm used to check for interrupt in the propagation
   Integer checkCount = 0, trigger = 0;

   try
   {
      // If command is not reentering from checking interrupts, do final prep
      if (!inProgress)
      {
         stepBrackets[0] = 0.0;
         checkFirstStep = false;
         PrepareToPropagate();

         // Check for initial stop condition before first step in while loop
         // eg) elapsed time of 0
         // Added to also check Gmat::SOLVING, since Optimize.cpp was updated to
         // set the run state to Gmat::SOLVING (LOJ: 2010.01.12)
         if (publisher->GetRunState() == Gmat::RUNNING ||
             publisher->GetRunState() == Gmat::SOLVING)
         {
            // remove any old stop conditions that may have reported valid
            triggers.clear();
            stopTrigger = -1;

            // Evaluate Stop conditions to set initial values
            for (UnsignedInt i=0; i<stopWhen.size(); i++)
            {
               Real accuracy = (stopWhen[i]->IsTimeCondition() ? timeAccuracy :
                                firstStepTolerance);
               stopWhen[i]->Reset();
               stopWhen[i]->Evaluate();
               #ifdef DEBUG_STOPPING_CONDITIONS
                  MessageInterface::ShowMessage
                     (wxT("Achieved: %.12lf Goal: %.12lf Difference: %.12le\n"),
                      stopWhen[i]->GetStopValue(), stopWhen[i]->GetStopGoal(),
                      stopWhen[i]->GetStopValue() - stopWhen[i]->GetStopGoal());
               #endif
               // Set the flag to check the first step only if
               //    (1) the stop value is <= stopAccuracy and
               //    (2) it was (one of) the last stop(s) triggered
               if (fabs(stopWhen[i]->GetStopValue() -
                     stopWhen[i]->GetStopGoal()) < accuracy)
               {
                  #ifdef DEBUG_FIRST_STEP_STOP
                     wxString scName = stopWhen[i]->GetName();
                     MessageInterface::ShowMessage(
                        wxT("*** FirstStep: Value = %.12lf, goal = %.12lf\n*** ")
                        wxT("Name = %s, WLST = %s\n"), stopWhen[i]->GetStopValue(),
                        stopWhen[i]->GetStopGoal(),
                        scName.c_str(),
                        (stopSats[i]->WasLastStopTriggered(scName) ?
                         wxT("true") : wxT("false")));
                  #endif

                  if (stopSats[i]->WasLastStopTriggered(stopWhen[i]->GetName()))
                  {
                     checkFirstStep = true;
                     stopWhen[i]->SkipEvaluation(true);
                  }
               }
            }
         }
      } // if (!inProgress)

      #ifdef DEBUG_EPOCH_SYNC
         MessageInterface::ShowMessage(wxT("Nominal steps executing:\n"));
      #endif

      while (!stopCondMet)
      {
         // Update the epoch on the force models
         for (UnsignedInt i = 0; i < fm.size(); ++i)
         {
            if (fm[i] != NULL)
               fm[i]->UpdateInitialData();
//            else
//               p[i]->LoadInitialData();
         }
         #ifdef DEBUG_EPOCH_UPDATES
            for (UnsignedInt i = 0; i < fm.size(); ++i)
               if (fm[i])
                  fm[i]->ReportEpochData();
         #endif

         for (UnsignedInt i = 0; i < fm.size(); ++i)
            if (fm[i])
               fm[i]->BufferState();
            else
               p[i]->BufferState();

         if (!TakeAStep())
            throw CommandException(
               wxT("Propagate::Execute() Propagator Failed to Step\n"));

         for (UnsignedInt i = 0; i < fm.size(); ++i)
         {
            // orbit related parameters use spacecraft for data
            #ifdef DEBUG_PROPAGATE_EXE
               MessageInterface::ShowMessage
                  (wxT("Propagate::Execute() Updating epoch data\n"));
            #endif
            if (fm[i])
            {
               elapsedTime[i] = fm[i]->GetTime();
               currEpoch[i] = baseEpoch[i] + elapsedTime[i] /
                  GmatTimeConstants::SECS_PER_DAY;
            }
            else
            {
               elapsedTime[i] = p[i]->GetTime();
               currEpoch[i] = baseEpoch[i] + elapsedTime[i] /
                  GmatTimeConstants::SECS_PER_DAY;
            }
            #ifdef DEBUG_PROPAGATE_EXE
               MessageInterface::ShowMessage
                  (wxT("   %d  BaseEpoch = %.12lf elapsedTime = %.12lf ")
                   wxT("currEpoch size = %.12lf\n"), i, baseEpoch[i],
                   elapsedTime[i], currEpoch[i]);
            #endif

            // Update spacecraft epoch, without argument the spacecraft epoch
            // won't get updated for consecutive Propagate command
            #ifdef DEBUG_PROPAGATE_EXE
               MessageInterface::ShowMessage
                  (wxT("Propagate::Execute() Updating SpaceObjects; first vector ")
                   wxT("element = %.12lf\n"), (psm[i]->GetState()->GetState())[0]);
            #endif
            if (fm[i])
               fm[i]->UpdateSpaceObject(currEpoch[i]);
            else
               p[i]->UpdateSpaceObject(currEpoch[i]);
         }

         // In single step mode, we're done!
         if (singleStepMode)
         {
            #ifdef DEBUG_PROPAGATE_EXE
               MessageInterface::ShowMessage
                  (wxT("Propagate::Breaking for Single Step\n"));
            #endif

/**
 *         We are not publishing the final point when running in single step
 *         mode; this code does that, but then we end up with repeated points
 *         because the last point from one step is the first point in the next.
 *         If we publish here, the Publisher or all Subscribers need to be able
 *         to handle the repeated point issues.
 */

//            pubdata[0] = currEpoch[0];
//            memcpy(&pubdata[1], j2kState, dim*sizeof(Real));
//            #ifdef DEBUG_PUBLISH_DATA
//               MessageInterface::ShowMessage
//                     ("***Propagate::Execute() SingleStep '%s' publishing current "
//                      "%d data to stream %d, 1st data = [%.12lf %.12lf %.12lf "
//                      "%.12lf %.12lf %.12lf %.12lf]\n",
//                      GetGeneratingString(Gmat::NO_COMMENTS).c_str(), dim+1,
//                      streamID, pubdata[0], pubdata[1], pubdata[2], pubdata[3],
//                      pubdata[4], pubdata[5], pubdata[6]);
//            #endif
//
//            publisher->Publish(this, streamID, pubdata, dim+1);

            break;
         }

         CheckStopConditions(epochID);

         if (!stopCondMet)
         {
            // No longer need to check stopping conditions at the first step
            checkFirstStep = false;

            // Publish the data here
            pubdata[0] = currEpoch[0];
            // For each PropSetup, fill the appropriate array elements
            Integer index = 1;
            Integer size = 0;
            for (UnsignedInt i = 0; i < prop.size(); ++i)
            {
               j2kState = p[i]->GetJ2KState();
               size = p[i]->GetDimension();
               memcpy(&pubdata[index], j2kState, size*sizeof(Real));
               index += size;
            }
            #ifdef DEBUG_PUBLISH_DATA
               MessageInterface::ShowMessage
                     (wxT("Propagate::Execute() '%s' publishing current %d data to ")
                      wxT("stream %d, data = [%f "),
                      GetGeneratingString(Gmat::NO_COMMENTS).c_str(), dim+1,
                      streamID, pubdata[0]);
               for (Integer i = 1; i < dim+1; ++i)
                  MessageInterface::ShowMessage(wxT(" %f"), pubdata[i]);
               MessageInterface::ShowMessage(wxT("]\n"));
            #endif

            publisher->Publish(this, streamID, pubdata, dim+1);
         }
         else
         {
            #ifdef DEBUG_STOPPING_CONDITIONS
               MessageInterface::ShowMessage(wxT("Stopping condition met\n"));
            #endif

            stopInterval = 0.0;
            for (UnsignedInt i = 0; i < fm.size(); ++i)
            {
               Real timestep = p[i]->GetStepTaken();
               if (fabs(timestep) > fabs(stopInterval))
                  stopInterval = timestep;

               switch (currentMode)
               {
                  case SYNCHRONIZED:
                     if (fm[0] != NULL)
                        elapsedTime[i] = fm[0]->GetTime();
                     else
                        elapsedTime[i] = p[0]->GetTime();
                     if (fm[i] != NULL)
                        fm[i]->SetTime(elapsedTime[i]);
                     else
                        p[i]->SetTime(elapsedTime[i]);
                     break;

                  case INDEPENDENT:
                  default:
                     if (fm[i] != NULL)
                        elapsedTime[i] = fm[i]->GetTime();
                     else
                        elapsedTime[i] = p[i]->GetTime();
               }

               currEpoch[i] = baseEpoch[i] +
                  elapsedTime[i] / GmatTimeConstants::SECS_PER_DAY;
            }

            #ifdef DEBUG_STOPPING_CONDITIONS
               MessageInterface::ShowMessage(wxT("   Prop #, baseEpoch, currEpoch, ")
                     wxT("elapsedTime\n"));
               for (UnsignedInt i = 0; i < fm.size(); ++i)
                  MessageInterface::ShowMessage(wxT("      %d, %.12lf, %.12lf, ")
                        wxT("%.12lf\n"), i, baseEpoch[i], currEpoch[i],
                        elapsedTime[i]);
            #endif

            stepBrackets[1] = stopInterval;

            #ifdef DEBUG_EPOCH_SYNC
               for (UnsignedInt i = 0; i < fm.size(); ++i)
                  MessageInterface::ShowMessage(
                     wxT("   SC MET!  Force model[%d] has base epoch %16.11lf, ")
                     wxT("time dt = %.11lf, elapsedTime = %.11lf\n"), i,
                     baseEpoch[i], fm[i]->GetTime(), elapsedTime[i]);
            #endif

            #ifdef DEBUG_EPOCH_UPDATES
               MessageInterface::ShowMessage(wxT("StopStep = %15.11lf\n"),
                  stopInterval);
            #endif
         }

         #if DEBUG_PROPAGATE_EXE
            MessageInterface::ShowMessage(
               wxT("Propagate::Execute() intermediate; epoch = %f\n"), currEpoch[0]);
         #endif

         // Periodically see if the user has stopped the run
         ++checkCount;
         if ((checkCount == interruptCheckFrequency) && !stopCondMet)
         {
            inProgress = true;
            return true;
         }

         #ifdef DEBUG_EPOCH_SYNC
            for (UnsignedInt i = 0; i < fm.size(); ++i)
               MessageInterface::ShowMessage(wxT("   Force model[%d] has base ")
                  wxT("epoch %16.11lf, time dt = %.11lf\n"),
                  i, baseEpoch[i], fm[i]->GetTime());
         #endif
      }
         #ifdef DEBUG_EPOCH_SYNC
            MessageInterface::ShowMessage(wxT("Nominal steps Finished\n"));
         #endif
   }
   catch (BaseException &ex)
   {
      // Use exception to remove Visual C++ warning
      ex.GetMessageType();
      inProgress = false;
      throw;
   }

   #ifdef DEBUG_EPOCH_UPDATES
      fm[0]->ReportEpochData();
   #endif

   inProgress = false;
   if (!singleStepMode)
   {
      for (UnsignedInt i = 0; i < fm.size(); ++i)
      {
         if (fm[i])
            fm[i]->RevertSpaceObject();
         else
            p[i]->RevertSpaceObject();

         // For synchronized propagation, the epochs can get out of sync here
         // if the stopping condition was applied to a later PropSetup.  The
         // Cartesian state is okay; it's just an issue with the epoch because
         // of the initial pass through the cubic spline.  Reset the epochs to
         // correct this issue.
         if (currentMode == SYNCHRONIZED)
            for (UnsignedInt i = 1; i < fm.size(); ++i)
               if (fm[i] != NULL)
                  fm[i]->SetTime(fm[0]->GetTime());
      }

      TakeFinalStep(epochID, trigger);

      // reset the stopping conditions so that scanning starts over
      for (UnsignedInt i=0; i<stopWhen.size(); i++)
         stopWhen[i]->Reset();
   }
   else
   {
      // clear first step stopping condition flags
      for (ObjectArray::iterator i = sats.begin();
           i != sats.end(); ++i)
         ((SpaceObject*)(*i))->ClearLastStopTriggered();
   }

   #ifdef DEBUG_EPOCH_UPDATES
      if (fm[0]);
         fm[0]->ReportEpochData();
   #endif

   ClearTransientForces();
   // Only build command summary if not in single step mode
   if (!singleStepMode)
      BuildCommandSummary(true);

   return true;
}


//------------------------------------------------------------------------------
// bool TakeAStep(Real propStep)
//------------------------------------------------------------------------------
/**
 * Advances each of the contained PropSetups by one step.
 *
 * @param <propStep> The requested size of the step.
 *
 * @return true if the step succeeded.
 */
//------------------------------------------------------------------------------
bool Propagate::TakeAStep(Real propStep)
{
   bool retval = false;
   Real stepToTake;

   #ifdef DEBUG_FIXED_STEP
      std::vector<ODEModel *>::iterator fmod = fm.begin();
   #endif

//   for (std::vector<ODEModel *>::iterator f = fm.begin(); f != fm.end(); ++f)
//      (*f)->BufferState();


   std::vector<Propagator*>::iterator current = p.begin();
   if (propStep == 0.0)
   {
      switch (currentMode)
      {
         case INDEPENDENT:
            // Advance each propagator individually, without regard for the
            // epochs of the others
            #ifdef DEBUG_PROPAGATE_EXE
               MessageInterface::ShowMessage
                  (wxT("Propagate::TakeAStep() running in INDEPENDENT mode\n"));
            #endif
            while (current != p.end())
            {
               if (!(*current)->Step())
                  throw CommandException(
                     wxT("Propagator failed to take a good step\n"));
               ++current;
            }
            retval = true;
            break;

         case SYNCHRONIZED:
            // This mode advances the first propagator, and then brings the
            // others up to the epoch of that first one.
            #ifdef DEBUG_PROPAGATE_EXE
               MessageInterface::ShowMessage
                  (wxT("Propagate::TakeAStep() running in SYNCHRONIZED mode\n"));
            #endif
            if (!(*current)->Step())
               throw CommandException(wxT("Initial synchronized Propagator failed ")
                                      wxT("to take a good step\n"));
            stepToTake = (*current)->GetStepTaken();
            ++current;
            while (current != p.end())
            {
               if (!(*current)->Step(stepToTake))
                  throw CommandException(wxT("Propagator failed to take a good ")
                                         wxT("synchronized step\n"));
               ++current;
            }
            retval = true;
            break;

         default:
            #ifdef DEBUG_PROPAGATE_EXE
               MessageInterface::ShowMessage
                  (wxT("Propagate::TakeAStep() running in undefined mode ")
                  wxT("(mode = %d)\n"), currentMode);
            #endif
            retval = false;
      }
      #ifdef DEBUG_PROPAGATE_EXE
         MessageInterface::ShowMessage
            (wxT("Propagate::TakeAStep() Step Taken\n"));
      #endif
   }
   else
   {
      // Step all of the propagators by the input amount
      while (current != p.end())
      {
         #ifdef DEBUG_FIXED_STEP
            MessageInterface::ShowMessage(wxT("Stepping '%s' by %le seconds from ")
               wxT("epoch = %16.11lf\n"), (*current)->GetName().c_str(), propStep,
               (*fmod)->GetRealParameter((*fmod)->GetParameterID(wxT("Epoch"))));

            Integer fmSize = (*fmod)->GetDimension();
            MessageInterface::ShowMessage(wxT("Fmod has dim = %d\n"), fmSize);
            MessageInterface::ShowMessage(wxT("   Pre Prop:  "));

            Real *fmState = (*fmod)->GetState();
            for (Integer q = 0; q < fmSize; ++q)
               MessageInterface::ShowMessage(wxT(" %.12lf"), fmState[q]);
            MessageInterface::ShowMessage(wxT("\n"));
         #endif

         if (!(*current)->Step(propStep))
         {
            wxString size;
            size.Printf( wxT("%.12lf"), propStep);
            throw CommandException(wxT("Propagator ") + (*current)->GetName() +
               wxT(" failed to take a good final step (size = ") + size + wxT(")\n"));
         }


         #ifdef DEBUG_FIXED_STEP
            MessageInterface::ShowMessage(wxT("   Stepped to epoch = %16.11lf\n"),
               (*fmod)->GetRealParameter((*fmod)->GetParameterID(wxT("Epoch"))));
            MessageInterface::ShowMessage(wxT("   Post Prop: "));

            for (Integer q = 0; q < fmSize; ++q)
               MessageInterface::ShowMessage(wxT(" %.12lf"), fmState[q]);
            MessageInterface::ShowMessage(wxT("\n"));

            ++fmod;
         #endif

         ++current;
      }
      retval = true;
   }

   #ifdef DEBUG_PROPAGATE_STEPSIZE
      MessageInterface::ShowMessage(wxT("Prop step = %16.13lf\n"),
         p[0]->GetStepTaken());
   #endif

   #ifdef DUMP_PLANET_DATA
      Real epoch = sats[0]->GetRealParameter(wxT("A1Epoch"));
      Rvector6 planetrv;

      for (Integer h = 0; h < bodiesDefined; ++h)
      {
         if (body[h] != NULL)
         {
            planetrv = body[h]->GetState(epoch);
            planetData << (body[h]->GetName()) << wxT("  ") << epoch << wxT("  ")
                       << planetrv[0] << wxT("  ")
                       << planetrv[1] << wxT("  ")
                       << planetrv[2] << wxT("  ")
                       << planetrv[3] << wxT("  ")
                       << planetrv[4] << wxT("  ")
                       << planetrv[5] << wxT("\n");
         }
      }
   #endif

   return retval;
}


//------------------------------------------------------------------------------
// void CheckStopConditions(Integer epochID)
//------------------------------------------------------------------------------
/**
 * Checks the status of the stopping conditions.
 *
 * @param epochID The parameter ID associated with the epoch field.
 */
//------------------------------------------------------------------------------
void Propagate::CheckStopConditions(Integer epochID)
{
   //------------------------------------------
   // loop through StopCondition list
   //------------------------------------------
   #ifdef DEBUG_STOPPING_CONDITIONS
      try {
   #endif

      for (UnsignedInt i = 0; i < stopWhen.size(); i++)
      {
         // StopCondition need epoch for the Interpolator
         stopWhen[i]->SetRealParameter(stopCondEpochID,
            stopSats[i]->GetRealParameter(epochID));

         #ifdef DEBUG_STOPPING_CONDITIONS
            MessageInterface::ShowMessage(wxT("Evaluating \"%s\" Stopping ")
                  wxT("condition\n"), stopWhen[i]->GetName().c_str());
         #endif

         if (stopWhen[i]->Evaluate())
         {
            #ifdef DEBUG_STOPPING_CONDITIONS
               MessageInterface::ShowMessage(wxT("\"%s\" evaluates true!\n"),
                  stopWhen[i]->GetName().c_str());
            #endif

            stopInterval = stopWhen[i]->GetStopInterval();
            if (stopInterval == 0.0)
            {
               stopEpoch = stopWhen[i]->GetStopEpoch();
            }

            stopCondMet = true;
            if (stopTrigger < 0)
               stopTrigger = i;

            triggers.push_back(stopWhen[i]);

            #if DEBUG_PROPAGATE_EXE
               MessageInterface::ShowMessage
                  (wxT("Propagate::CheckStopConditions() %s met for %d\n"),
                   stopWhen[i]->GetName().c_str(), i);
            #endif

            #ifdef DEBUG_EPOCH_SYNC
               for (UnsignedInt i = 0; i < fm.size(); ++i)
                  MessageInterface::ShowMessage(wxT("   *** SC[%d] (%s) MET! ***\n"),
                     i, stopWhen[i]->GetName().c_str());
            #endif
         }
         else if (checkFirstStep)
         {
            #ifdef DEBUG_FIRST_STEP_STOP
               MessageInterface::ShowMessage(wxT("%d: "), i);
            #endif

            // Turn condition back on
            stopWhen[i]->SkipEvaluation(false);
            if (CheckFirstStepStop(i))
            {
               #ifdef DEBUG_FIRST_STEP_STOP
                  MessageInterface::ShowMessage(wxT("***Continuing from %s\n"),
                     stopWhen[i]->GetName().c_str());
               #endif

               stopInterval = stopWhen[i]->GetStopInterval();
               if (stopInterval == 0.0)
               {
                  stopEpoch = stopWhen[i]->GetStopEpoch();
               }
               stopCondMet = true;
               if (stopTrigger < 0)
                  stopTrigger = i;

               triggers.push_back(stopWhen[i]);
            }
         }
      }

   #ifdef DEBUG_STOPPING_CONDITIONS
      }
      catch (BaseException &ex) {
         MessageInterface::ShowMessage(
            wxT("Propagate::PrepareToPropagate() Exception while evaluating stopping ")
            wxT("conditions\n"));
         inProgress = false;
         throw;
      }
   #endif

   #ifdef DEBUG_EPOCH_SYNC
      for (UnsignedInt i = 0; i < stopWhen.size(); ++i)
         if (stopWhen[i]->Evaluate())
            MessageInterface::ShowMessage(wxT("   *** StopInterval[%d] = %.11lf\n"),
               i, stopWhen[i]->GetStopInterval());
   #endif
}


//------------------------------------------------------------------------------
// bool CheckFirstStep()
//------------------------------------------------------------------------------
/**
 * Method used during the first prop step to ensure that a stop encountered on
 * this step is not repeating the last stop encountered.
 *
 * @param <i> Index in the stopping condition list that needs to be checked
 *
 * @return true is the stop is a valid stop -- that is, if it is not a repeat of
 * the last stop -- and false if it is a repeat.
 */
//------------------------------------------------------------------------------
bool Propagate::CheckFirstStepStop(Integer i)
{
   #ifdef DEBUG_FIRST_STEP_STOP
      MessageInterface::ShowMessage(
         wxT("CheckFirstStepStop checking %s; returning valid stop condition: %s\n"),
         stopWhen[i]->GetName().c_str(),
         (stopSats[i]->WasLastStopTriggered(stopWhen[i]->GetName()) ?
          wxT("false") : wxT("true")));
   #endif

   if (stopSats[i]->WasLastStopTriggered(stopWhen[i]->GetName()))//;
   {
      // Only report as triggered if outside of the stop accuracy
      Real min, max, goal, temp;
      min = stopWhen[i]->GetStopValue();
      max = stopWhen[i]->GetStopParameter()->EvaluateReal();
      if (min > max)
      {
         temp = min;
         min = max;
         max = temp;
      }
      goal = stopWhen[i]->GetStopGoal();

      if (stopWhen[i]->IsCyclicParameter())
      {
         Real rangemin, rangemax;
         stopWhen[i]->GetRange(rangemin, rangemax);
         Real halfrange = (rangemax - rangemin)/2.0;
         min = stopWhen[i]->PutInRange(min, goal-halfrange, goal+halfrange);
         max = stopWhen[i]->PutInRange(max, goal-halfrange, goal+halfrange);
      }

      temp = fabs(goal - min);
      if (fabs(goal - max) < temp)
         temp = fabs(goal - max);

      // Only report true if outside of tolerance
      Real accuracy = (stopWhen[i]->IsTimeCondition() ? timeAccuracy :
                                                        firstStepTolerance);

      if (temp > accuracy)
         if ((goal > min) && (goal < max))
            return true;

      // Fill buffer data in the sc
      stopWhen[i]->UpdateBuffer();
   }

   return false;
}


//------------------------------------------------------------------------------
// void TakeFinalStep(Integer EpochID, Integer trigger)
//------------------------------------------------------------------------------
/**
 * Takes the final prop step based on data from the stopping conditions.
 *
 * @param epochID The parameter ID associated with the epoch field.
 * @param trigger Index indicating which stopping condition was met.
 */
//------------------------------------------------------------------------------
void Propagate::TakeFinalStep(Integer EpochID, Integer trigger)
{
   #ifdef DEBUG_FINAL_STEP
     MessageInterface::ShowMessage(wxT("*** Start of TakeFinalStep\n"));
        MessageInterface::ShowMessage(wxT("      State data:\n"));
     MessageInterface::ShowMessage(wxT("         time: %.12lf\n"), p[0]->GetTime());
             MessageInterface::ShowMessage(wxT("         r:    [%.12lf %.12lf %.12lf]\n"),
        p[0]->GetState()[0], p[0]->GetState()[1], p[0]->GetState()[2]);
   #endif

   // We've passed a stop condition, so remember that step size.  Include a 10%
   // safety factor.
   stepBrackets[1] = stopInterval * 1.1;

   #if DEBUG_PROPAGATE_EXE
      MessageInterface::ShowMessage(
         wxT("Propagate::TakeFinalStep currEpoch = %f, stopEpoch = %f, ")
         wxT("elapsedTime = %f\n"), currEpoch[0], stopEpoch, elapsedTime[0]);
   #endif

   Real secsToStep = 1.0e99 * direction, dt = 0.0;
   StopCondition *stopper = NULL;

   #ifdef DEBUG_EPOCH_SYNC
      MessageInterface::ShowMessage(wxT("Top of final step code:\n"));
      for (UnsignedInt i = 0; i < fm.size(); ++i)
         if (fm[i])
            MessageInterface::ShowMessage(
               wxT("   Force model[%d] has base epoch %16.11lf, time dt = %.11lf, ")
               wxT("stopInterval = %.12lf\n"), i, baseEpoch[i], fm[i]->GetTime(),
               stopInterval);
         else
            MessageInterface::ShowMessage(
               wxT("   Propagator[%d] has base epoch %16.11lf, time dt = %.11lf, ")
               wxT("stopInterval = %.12lf\n"), i, baseEpoch[i], p[i]->GetTime(),
               stopInterval);
   #endif

   // Toggle propagators into final step mode
   /// @note This code should be removed if the minimum step is removed from
   /// the RK integrators
   for (std::vector<Propagator*>::iterator current = p.begin();
        current != p.end(); ++current)
      (*current)->SetAsFinalStep(true);

   // First save the spacecraft for later restoration
   for (UnsignedInt i = 0; i < fm.size(); ++i)
   {
      #if DEBUG_PROPAGATE_EXE
         MessageInterface::ShowMessage(wxT("   TakeFinalStep calling update with CurrentEpoch[%d] = %.12lf\n"), i,
            currEpoch[i]);
      #endif

      if (fm[i])
         fm[i]->UpdateSpaceObject(  // currEpoch[i]);
               baseEpoch[i] + fm[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);
      else
         p[i]->UpdateSpaceObject(  // currEpoch[i]);
               baseEpoch[i] + p[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);
   }
   BufferSatelliteStates(true);
   #ifdef DEBUG_FINAL_STEP
                MessageInterface::ShowMessage(wxT("   Buffered state data\n"));
                MessageInterface::ShowMessage(wxT("      State data:\n"));
                MessageInterface::ShowMessage(wxT("         time: %.12lf\n"), p[0]->GetTime());
                MessageInterface::ShowMessage(wxT("         r:    [%.12lf %.12lf %.12lf]\n"),
                p[0]->GetState()[0], p[0]->GetState()[1], p[0]->GetState()[2]);
   #endif

   #ifdef DEBUG_EPOCH_SYNC
      MessageInterface::ShowMessage(wxT("Prior to interpolation:\n"));
      for (UnsignedInt i = 0; i < fm.size(); ++i)
         MessageInterface::ShowMessage(
            wxT("   Force model[%d] has base epoch %16.11lf, time dt = %.11lf\n"),
            i, baseEpoch[i], fm[i]->GetTime());
   #endif

   // Interpolate to get the stop epoch
   if (stopTrigger < 0)
      throw CommandException(
         wxT("Stopping condition was not set for final step on the line \n") +
         GetGeneratingString(Gmat::SCRIPTING));

   for (std::vector<StopCondition*>::iterator i = triggers.begin();
        i != triggers.end(); ++i)
   {
      // Get estimated time to reach this stop condition, dt
      if ((*i)->IsTimeCondition())
      {
         dt = (*i)->GetStopEpoch();

         #ifdef DEBUG_PROPAGATE_STEPSIZE
            MessageInterface::ShowMessage(wxT("Stopping on time\n   current ")
               wxT("epoch = %.14lf\n   goal          = %.14lf\n   dt            = ")
               wxT("%.14lf\n   sc diff       = %.14lf\n   tolerance = %le\n"),
               currEpoch[0], (*i)->GetStopGoal(), dt,
               (*i)->GetStopDifference(), timeAccuracy);
         #endif
      }
      else
      {
         dt = InterpolateToStop(*i);

         #ifdef DEBUG_PROPAGATE_STEPSIZE
            MessageInterface::ShowMessage(
               wxT("Interpolated stop time = %.14lf\n"), dt);
         #endif
      }

      // If dt is closer to current epoch, save this stop condition as trigger
      if (fabs(secsToStep) > fabs(dt))
      {
         secsToStep = dt;
         stopper = *i;
      }
   }

   #ifdef DEBUG_FINAL_STEP
      MessageInterface::ShowMessage(wxT("   First search -> dt = %.12lf\n"), dt);
                MessageInterface::ShowMessage(wxT("      State data:\n"));
                MessageInterface::ShowMessage(wxT("         time: %.12lf\n"), p[0]->GetTime());
                MessageInterface::ShowMessage(wxT("         r:    [%.12lf %.12lf %.12lf]\n"),
                p[0]->GetState()[0], p[0]->GetState()[1], p[0]->GetState()[2]);
        #endif

   #if DEBUG_PROPAGATE_EXE
      MessageInterface::ShowMessage(
         wxT("Step = %.12lf sec, calculated off of %.12lf and  %.12lf\n"),
         secsToStep, stopEpoch, currEpoch[trigger]);
   #endif

   // Perform stepsize rounding.  Note that the rounding precision can be set
   // by redefining the macro TIME_ROUNDOFF at the top of
   // PropagationEnabledCommand.hpp.  Set it to 0.0 to prevent rounding.
   //
   // Note that this code makes the final propagated state match the granularity
   // given by other software (aka STK)
   if (TIME_ROUNDOFF != 0.0)
      secsToStep = std::floor(secsToStep / TIME_ROUNDOFF + 0.5) * TIME_ROUNDOFF;

   #if defined DEBUG_PROPAGATE_STEPSIZE | defined DEBUG_PROPAGATE_DIRECTION
      MessageInterface::ShowMessage
         (wxT("Propagate::TakeFinalStep secsToStep at stop = %16.10le\n"),
          secsToStep);
   #endif
   #ifdef DEBUG_PROPAGATE_DIRECTION
      MessageInterface::ShowMessage
         (wxT("   stopEpoch = %16.10lf\n   currEpoch = %16.10lf\n"),
          stopEpoch, currEpoch[trigger]);
   #endif

   Real accuracy = (stopper->IsTimeCondition() ? timeAccuracy : stopAccuracy);

   // If we are not at the final state, move to it
   if (secsToStep != 0.0)
   {
      #if DEBUG_PROPAGATE_EXE
         MessageInterface::ShowMessage(
            wxT("Propagate::TakeFinalStep: Step(%16.13le) from epoch = %16.10lf\n"),
            secsToStep,
            (baseEpoch[0] + fm[0]->GetTime() / GmatTimeConstants::SECS_PER_DAY));
      #endif

      #ifdef DEBUG_EPOCH_SYNC
         MessageInterface::ShowMessage(wxT("Before final step:\n"));
         for (UnsignedInt i = 0; i < fm.size(); ++i)
            MessageInterface::ShowMessage(wxT("   Force model[%d] has base ")
               wxT("epoch %16.11lf, time dt = %.11lf\n"),
               i, baseEpoch[i], fm[i]->GetTime());
         MessageInterface::ShowMessage(wxT("   step by time %.11lf\n"), secsToStep);
      #endif

      if (!TakeAStep(secsToStep))
         throw CommandException(wxT("Propagator Failed to Step fixed interval\n"));

      #ifdef DEBUG_EPOCH_SYNC
         MessageInterface::ShowMessage(wxT("After final step:\n"));
         for (UnsignedInt i = 0; i < fm.size(); ++i)
            MessageInterface::ShowMessage(
               wxT("   Force model[%d] has base epoch %16.11lf, time dt = %.11lf\n"),
               i, baseEpoch[i], fm[i]->GetTime());
      #endif

      #ifdef DEBUG_FINAL_STEP
         MessageInterface::ShowMessage(wxT("   After stepping to initial stop condition epoch\n"));
         MessageInterface::ShowMessage(wxT("      State data:\n"));
         MessageInterface::ShowMessage(wxT("         time: %.12lf\n"), p[0]->GetTime());
         MessageInterface::ShowMessage(wxT("         r:    [%.12lf %.12lf %.12lf]\n"),
               p[0]->GetState()[0], p[0]->GetState()[1], p[0]->GetState()[2]);
      #endif
      // Check the stopping accuracy
      for (UnsignedInt i = 0; i < fm.size(); ++i)
      {
         if (fm[i])
            fm[i]->UpdateSpaceObject(
               baseEpoch[i] + fm[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);
         else
            p[i]->UpdateSpaceObject(
                           baseEpoch[i] + p[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);
      }

      stopper->Evaluate();

      if (fabs(stopper->GetStopDifference()) > accuracy)
      {
         #ifdef DEBUG_STOPPING_CONDITIONS
            MessageInterface::ShowMessage(wxT("   Using accuracy of %.12f\n"), accuracy);
            MessageInterface::ShowMessage(wxT("   Stop condition %s missed by %le; ")
               wxT("refining step \n"), stopper->GetName().c_str(),
               stopper->GetStopDifference());
            MessageInterface::ShowMessage(
               wxT("   When stepping %15.10lf secs, parameter has value %.9le\n"),
               secsToStep, stopper->GetStopParameter()->EvaluateReal());
         #endif

         // The interpolated step was not close enough, so back it out
         BufferSatelliteStates(false);
         for (UnsignedInt i = 0; i < fm.size(); ++i)
         {
            // Back out the steps taken to build the ring buffer
            if (fm[i])
            {
               fm[i]->UpdateFromSpaceObject();
               fm[i]->SetTime(fm[i]->GetTime() - secsToStep);
            }
            else
            {
               p[i]->UpdateFromSpaceObject();
               p[i]->SetTime(p[i]->GetTime() - secsToStep);
            }
         }

         #ifdef DEBUG_FINAL_STEP
            MessageInterface::ShowMessage(wxT("   Finished backing out the spline generated step\n"));
            MessageInterface::ShowMessage(wxT("      State data:\n"));
            MessageInterface::ShowMessage(wxT("         time: %.12lf\n"), p[0]->GetTime());
            MessageInterface::ShowMessage(wxT("         r:    [%.12lf %.12lf %.12lf]\n"),
                  p[0]->GetState()[0], p[0]->GetState()[1], p[0]->GetState()[2]);
         #endif

         // Generate a better time step
         secsToStep = RefineFinalStep(secsToStep, stopper);

         // Perform stepsize rounding.  Note that the rounding precision can be
         // set by redefining the macro TIME_ROUNDOFF at the top of
         // PropagationEnabledCommand.hpp.  Set it to 0.0 to prevent rounding.
         //
         // Note that this code makes the final propagated state match the
         // granularity given by other software (aka STK)
         if (TIME_ROUNDOFF != 0.0)
            secsToStep = std::floor(secsToStep / TIME_ROUNDOFF + 0.5) *
                  TIME_ROUNDOFF;

         // If a refined step was needed, we still need to take it;
         // RefineFinalStep returns with the interpolated step backed out
         if (!TakeAStep(secsToStep))
            throw CommandException(
               wxT("Propagator Failed to Step fixed interval\n"));

         for (UnsignedInt i = 0; i < psm.size(); ++i)
         {
            if (fm[i])
               fm[i]->UpdateSpaceObject(
                  baseEpoch[i] + fm[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);
            else
               p[i]->UpdateSpaceObject(
                  baseEpoch[i] + p[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);
         }

         #ifdef DEBUG_STOPPING_CONDITIONS
            MessageInterface::ShowMessage(
               wxT("   Refined timestep of %15.10lf secs gives %.9le\n"),
               secsToStep, stopper->GetStopParameter()->EvaluateReal());
         #endif

         if (fabs(stopper->GetStopDifference()) > accuracy)
            MessageInterface::ShowMessage(wxT("**** WARNING **** For the line \"")
                  wxT("%s\" the achieved stop is outside of the stopping ")
                  wxT("tolerance (%le); the difference from the desired value is ")
                  wxT("%le\n"), GetGeneratingString(Gmat::NO_COMMENTS).c_str(),
                  accuracy, fabs(stopper->GetStopDifference()));
      }

      // Publish the final data point here
      if (fm[0])
         pubdata[0] = baseEpoch[0]+fm[0]->GetTime()/GmatTimeConstants::SECS_PER_DAY;
      else
         pubdata[0] = baseEpoch[0] + p[0]->GetTime()/GmatTimeConstants::SECS_PER_DAY;
      memcpy(&pubdata[1], j2kState, dim*sizeof(Real));
      #ifdef DEBUG_PUBLISH_DATA
         MessageInterface::ShowMessage
               (wxT("Propagate::TakeFinalStep() '%s' publishing final %d data to ")
                wxT("stream %d, 1st data = %f\n"),
                GetGeneratingString(Gmat::NO_COMMENTS).c_str(), dim+1, streamID,
                pubdata[0]);
      #endif

      Integer index = 1, size;
      for (UnsignedInt i = 0; i < prop.size(); ++i)
      {
         j2kState = p[i]->GetJ2KState();
         size = p[i]->GetDimension();
         memcpy(&pubdata[index], j2kState, size*sizeof(Real));
         index += size;
      }
      publisher->Publish(this, streamID, pubdata, dim+1);

      #if DEBUG_PROPAGATE_EXE
         MessageInterface::ShowMessage
            (wxT("Propagate::TakeFinalStep: Step(%16.13le) advanced to ")
            wxT("epoch = %16.10lf\n"), secsToStep,
            (baseEpoch[0] + fm[0]->GetTime() / GmatTimeConstants::SECS_PER_DAY));
      #endif

      // FlushBuffers here causes to write EphemerisFile multiple data segments
      // when propagating inside a loop using  stop condition of Sat.TAIModJulian = stopTime.
      // Stop condition of Sat.ElapsedSecs = 60 has no effects.
      // So passed false for end of data block. (LOJ: 2010.10.22)
      publisher->FlushBuffers(false);

      #if DEBUG_PROPAGATE_EXE
         MessageInterface::ShowMessage
            (wxT("Propagate::TakeFinalStep complete; epoch = %16.10lf\n"),
             (baseEpoch[0] + fm[0]->GetTime() / GmatTimeConstants::SECS_PER_DAY));
      #endif
   }

   // Clear previous stop conditions from the spacecraft, and then store the
   // stop name in the spacecraft that triggered it
   for (ObjectArray::iterator it = sats.begin();
        it != sats.end(); ++it)
      ((SpaceObject*)(*it))->ClearLastStopTriggered();

   if (stopper != NULL)
   {
      // Save the stop condition and reset for next pass
      Integer stopperIndex = 0;

      Real howClose = fabs(stopper->GetStopDifference());
      // First step tolerance is one order of magnitude above stop accuracy.
      firstStepTolerance = (howClose > accuracy ?
            howClose : accuracy) * 10.0;

      #ifdef DEBUG_FIRST_STEP_STOP
         MessageInterface::ShowMessage(wxT("First step tolerance = %le,    ")
               wxT("achieved = %le, desired = %le\n"), firstStepTolerance, howClose,
               accuracy);
      #endif

      for (std::vector<StopCondition*>::iterator i = stopWhen.begin();
            i != stopWhen.end(); ++i)
      {
         if ((*i == stopper) || (fabs((*i)->GetStopDifference()) <= accuracy))
         {
            #ifdef DEBUG_FIRST_STEP_STOP
               MessageInterface::ShowMessage(
                  wxT("Setting stop name '%s' on sat '%s'\n"),
                  stopper->GetName().c_str(),
                  stopSats[stopperIndex]->GetName().c_str());
            #endif

            stopSats[stopperIndex]->SetLastStopTriggered((*i)->GetName());
         }
         ++stopperIndex;
      }
      triggers.clear();
   }

   for (std::vector<StopCondition *>::iterator i = stopWhen.begin();
        i != stopWhen.end(); ++i)
   {
      if ((*i)->GetName() == wxT(""))
      {
         StopCondition *localSc = *i;
         stopWhen.erase(i);
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (localSc, localSc->GetName(), wxT("Propagate::TakeFinalStep()"),
             wxT("deleting localSc"));
         #endif
         delete localSc;
      }
   }

   // Toggle propagators out of final step mode
   /// @note This code should be removed if the minimum step is removed from
   /// the RK integrators
   for (std::vector<Propagator*>::iterator current = p.begin();
        current != p.end(); ++current)
      (*current)->SetAsFinalStep(false);
}


//------------------------------------------------------------------------------
// Real InterpolateToStop(StopCondition *sc)
//------------------------------------------------------------------------------
/**
 * Routine that drives the cubic spline, filling the ring buffer and
 * interpolating the time step needed to find the interval to the stop
 * condition.
 *
 * @param <sc> The stopping condition that is used for the interpolation.
 *
 * @return The time step to the stopping condition, as determined by the cubic
 * spline interpolator.
 */
//------------------------------------------------------------------------------
Real Propagate::InterpolateToStop(StopCondition *sc)
{
   // Now fill in the ring buffer
   Real ringStep = stopInterval / 4.0;
   Integer ringStepsTaken = 0;
   bool firstRingStep = true;
   bool stopIsBracketed = false;
   Real elapsedSeconds = 0.0;

   while ((!stopIsBracketed) && (ringStepsTaken < 8))
   {
      #ifdef DEBUG_STOPPING_CONDITIONS
         MessageInterface::ShowMessage(wxT("Taking ring step %d, step size = ")
               wxT("%.12lf\n"), ringStepsTaken, ringStep);
      #endif
      // Take a fixed prop step
      if (!TakeAStep(ringStep))
         throw CommandException(wxT("Propagator Failed to Step fixed interval ")
            wxT("while filling ring buffer\n"));
      elapsedSeconds += ringStep;

      // Update spacecraft for that step
      for (UnsignedInt i = 0; i < fm.size(); ++i)
      {
         if (fm[i])
            fm[i]->UpdateSpaceObject(
               baseEpoch[i] + fm[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);
         else
            p[i]->UpdateSpaceObject(
               baseEpoch[i] + p[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);
      }

      // Update the data in the stop condition
      sc->SetRealParameter(stopCondEpochID, elapsedSeconds);
      stopIsBracketed = sc->AddToBuffer(firstRingStep);

      ++ringStepsTaken;
      firstRingStep = false;
      #ifdef DEBUG_STOPPING_CONDITIONS
         MessageInterface::ShowMessage(wxT("   step = %.12lf, value = %.12lf\n"),
               elapsedSeconds, sc->GetStopValue());
      #endif
   }

   // Now interpolate the epoch...
   stopEpoch = sc->GetStopEpoch();

   #if DEBUG_PROPAGATE_EXE
      MessageInterface::ShowMessage(
         wxT("Propagate::TakeFinalStep set the stopEpoch = %.12lf\n"), stopEpoch);
   #endif

   // ...and restore the spacecraft and force models
   BufferSatelliteStates(false);
   for (UnsignedInt i = 0; i < fm.size(); ++i)
   {
      if (fm[i])
      {
         fm[i]->UpdateFromSpaceObject();
         // Back out the steps taken to build the ring buffer
         fm[i]->SetTime(fm[i]->GetTime() - ringStepsTaken * ringStep);
      }
      else
      {
         p[i]->UpdateFromSpaceObject();
         // Back out the steps taken to build the ring buffer
         p[i]->SetTime(p[i]->GetTime() - ringStepsTaken * ringStep);
      }

      #if DEBUG_PROPAGATE_EXE
         if (fm[i])
            MessageInterface::ShowMessage(
               wxT("Force model base Epoch = %.12lf  elapsedTime = %.12lf  ")
               wxT("net Epoch = %.12lf\n"), baseEpoch[i], fm[i]->GetTime(),
               baseEpoch[i] + fm[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);
         else
            MessageInterface::ShowMessage(
               wxT("Propagator base Epoch = %.12lf  elapsedTime = %.12lf  ")
               wxT("net Epoch = %.12lf\n"), baseEpoch[i], p[i]->GetTime(),
               baseEpoch[i] + p[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);
      #endif
   }

   return stopEpoch;
}


//------------------------------------------------------------------------------
// Real RefineFinalStep(Real secsToStep, StopCondition *stopper)
//------------------------------------------------------------------------------
/**
 * Routine that refines the solution found by the cubic spline, by solving for
 * the stopping condition using secants until the step produced falls within the
 * desired accuracy.
 *
 * @param <secsToStep>  First guess at the duration needed for stopping.
 * @param <stopper>     The stopping condition used for the refinement.
 *
 * @return The time step to the stopping condition, as determined by the secant
 * solver.
 */
//------------------------------------------------------------------------------
Real Propagate::RefineFinalStep(Real secsToStep, StopCondition *stopper)
{
   #ifdef DEBUG_SECANT_DETAILS
      MessageInterface::ShowMessage(wxT("\nRefineFinalStep(%16.13lf) entered.\n"),
            secsToStep);
   #endif

   bool closeEnough = false;
   bool nextTimeThrough = false;
   Integer attempts = 0;

   Real x[2], y[2], slope, target, intercept;
   Parameter *stopParam = stopper->GetStopParameter(),
             *targParam = stopper->GetGoalParameter();

   x[0] = x[1] = y[1] = 0.0;

   intercept = y[0] = stopParam->EvaluateReal();

   #ifdef DEBUG_SECANT_DETAILS
      MessageInterface::ShowMessage(wxT("InitialPoint: [%.12lf %.12lf]\n"), x[0], y[0]);
   #endif

   if (stopper->IsTimeCondition())
   {
      // Handle time based stopping condition refinement
      Real prevStep = secsToStep;

      while ((attempts < 50) && !closeEnough)
      {
         // Restore the spacecraft and force models to the end state of the last step
         if (attempts > 0)
         {
            BufferSatelliteStates(false);
            for (UnsignedInt i = 0; i < fm.size(); ++i)
            {
               if (fm[i])
               {
                  fm[i]->UpdateFromSpaceObject();
                  fm[i]->SetTime(fm[i]->GetTime() - prevStep);
               }
               else
               {
                  p[i]->UpdateFromSpaceObject();
                  p[i]->SetTime(p[i]->GetTime() - prevStep);
               }
            }
         }

         if (!TakeAStep(secsToStep))
            throw CommandException(wxT("Unable to take a good step while searching ")
               wxT("for stopping step in command\n   \"") + GetGeneratingString() +
               wxT("\"\n"));

         // Update spacecraft for that step
         for (UnsignedInt i = 0; i < fm.size(); ++i)
         {
            if (fm[i])
               fm[i]->UpdateSpaceObject(
                  baseEpoch[i] + fm[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);
            else
               p[i]->UpdateSpaceObject(
                     baseEpoch[i] + p[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);

         }

         if (targParam != NULL)
            target = targParam->EvaluateReal();
         else
            target = stopper->GetStopGoal();

         x[1] = secsToStep;
         y[1] = stopParam->EvaluateReal();

         #ifdef DEBUG_STOPPING_CONDITIONS
            if (fm[0])
               MessageInterface::ShowMessage(
                  wxT("[%d] Time based secant target: %16.12le\n")
                  wxT("     BaseEpoch: %16.9lf    fmTime: %16.9lf\n")
                  wxT("     Secant data points:\n")
                  wxT("        (%16.12le, %16.12le)\n")
                  wxT("        (%16.12le, %16.12le)\n")
                  wxT("     Secant timestep: %16.12lf\n"),
                  attempts, target, baseEpoch[0], fm[0]->GetTime(), x[0], y[0],
                  x[1], y[1], secsToStep);
            else
               MessageInterface::ShowMessage(
                  wxT("[%d] Time based secant target: %16.12le\n")
                  wxT("     BaseEpoch: %16.9lf    fmTime: %16.9lf\n")
                  wxT("     Secant data points:\n")
                  wxT("        (%16.12le, %16.12le)\n")
                  wxT("        (%16.12le, %16.12le)\n")
                  wxT("     Secant timestep: %16.12lf\n"),
                  attempts, target, baseEpoch[0], p[0]->GetTime(), x[0], y[0],
                  x[1], y[1], secsToStep);

         #endif

         if (fabs(target - y[1]) < timeAccuracy)
            closeEnough = true;
         else
         {
            prevStep = secsToStep;
            slope = (y[1] - y[0]) / (x[1] - x[0]);
            secsToStep = (target - y[0]) / slope;
         }

         ++attempts;
      }
   }
   else
   {
      // Handle non-time based stopping condition refinement
      while (!closeEnough && (attempts < 50))
      {
         target = stopper->GetStopGoal();
         if (stopper->IsCyclicParameter())
         {
            y[0] = GetRangedAngle(y[0], target);
            y[1] = GetRangedAngle(y[1], target);
         }

         if (nextTimeThrough)
         {
            // Restore spacecraft and force models to end state of last step
            BufferSatelliteStates(false);
            for (UnsignedInt i = 0; i < fm.size(); ++i)
            {
               if (fm[i])
               {
                  fm[i]->UpdateFromSpaceObject();
                  fm[i]->SetTime(fm[i]->GetTime() - secsToStep);
               }
               else
               {
                  p[i]->UpdateFromSpaceObject();
                  p[i]->SetTime(p[i]->GetTime() - secsToStep);
               }
            }

            if (x[1] == x[0])
            {
               #ifdef DEBUG_SECANT_DETAILS
                  MessageInterface::ShowMessage(wxT("Infinite slope error!!!\n")
                        wxT("[%2d] Secant target: %16.12le\n")
                        wxT("   Secant data points:\n")
                        wxT("      (%16.12le, %16.12le)\n")
                        wxT("      (%16.12le, %16.12le)\n")
                        wxT("   Secant timestep: %16.12lf\n"),
                        attempts, target, x[0], y[0], x[1], y[1], secsToStep);
               #endif
               Real bisectStep = 0.0;
               try
               {
                  bisectStep = BisectToStop(stopper);
               }
               catch (BaseException &ex)
               {
                  MessageInterface::ShowMessage(
                        wxT("Error found (%s) while bisecting after a zero slope ")
                        wxT("secant was detected.\n"), ex.GetFullMessage().c_str());
                  throw;
               }
               if (bisectStep == 0.0)
               {
                  // Changed to a warning message
                  // throw CommandException("Error refining timestep for "
                  //       "Propagate command: infinite slope in secant and "
                  //       "bisection failed to stop on \"" +
                  //       stopper->GetName() + "\"; Exiting\n");
                  MessageInterface::ShowMessage(wxT("**** WARNING **** The ")
                        wxT("secant and bisection methods failed when attempting ")
                        wxT("to stop with tolerance %le  on stopping condition %s;")
                        wxT("the achieved stopping condition error was %le\n"),
                        stopAccuracy, stopper->GetName().c_str(),
                        fabs(stopper->GetStopDifference()));

                  break;
               }

               secsToStep = bisectStep;
               break;
            }
            slope = (y[1] - y[0]) / (x[1] - x[0]);
            if (slope == 0.0)
            {
               ++attempts;
               #ifdef DEBUG_SECANT_DETAILS
                  MessageInterface::ShowMessage(wxT("Zero slope error!!!\n")
                        wxT("[%2d] Secant target: %16.12le\n")
                        wxT("   Secant data points:\n")
                        wxT("      (%16.12le, %16.12le)\n")
                        wxT("      (%16.12le, %16.12le)\n")
                        wxT("   Secant timestep: %16.12lf\n"),
                        attempts, target, x[0], y[0], x[1], y[1], secsToStep);
               #endif

               Real bisectStep = 0.0;
               try
               {
                  bisectStep = BisectToStop(stopper);
               }
               catch (BaseException &ex)
               {
                  MessageInterface::ShowMessage(
                        wxT("Error found (%s) while bisecting after a zero slope ")
                        wxT("secant was detected.\n"), ex.GetFullMessage().c_str());
                  throw;
               }
               if (bisectStep == 0.0)
               {
                  // Changed to a warning message
                  // throw CommandException("Error refining timestep for "
                  //       "Propagate command: zero slope in secant and "
                  //       "bisection failed to stop on \"" +
                  //       stopper->GetName() + "\"; Exiting\n");
                  MessageInterface::ShowMessage(wxT("**** WARNING **** The ")
                        wxT("secant and bisection methods failed when attempting ")
                        wxT("to stop with tolerance %le  on stopping condition %s;")
                        wxT("the achieved stopping condition error was %le\n"),
                        stopAccuracy, stopper->GetName().c_str(),
                        fabs(stopper->GetStopDifference()));

                  break;
               }

               secsToStep = bisectStep;
               break;
            }
            secsToStep = x[1] + (target - y[1]) / slope;

            #ifdef DEBUG_STOPPING_CONDITIONS
               MessageInterface::ShowMessage(
                     wxT("[%2d] Secant target: %16.12le\n")
                     wxT("   Secant data points:\n")
                     wxT("      (%16.12le, %16.12le)\n")
                     wxT("      (%16.12le, %16.12le)\n")
                     wxT("   Secant timestep: %16.12lf\n"),
                     attempts, target, x[0], y[0], x[1], y[1], secsToStep);
            #endif
         }
         else
         {
            if (stopper->IsCyclicParameter())
               y[0] = GetRangedAngle(y[0], target);
         }

         #ifdef DEBUG_STOPPING_CONDITIONS
            if (fm[0])
               MessageInterface::ShowMessage(
                     wxT("Before step, param = %16.12lf, Stepping from %16.12lf by ")
                     wxT("%16.12lf, "), stopParam->EvaluateReal(), fm[0]->GetTime(),
                     secsToStep);
            else
               MessageInterface::ShowMessage(
                     wxT("Before step, param = %16.12lf, Stepping from %16.12lf by ")
                     wxT("%16.12lf, "), stopParam->EvaluateReal(), p[0]->GetTime(),
                     secsToStep);
         #endif

         if (!TakeAStep(secsToStep))
            throw CommandException(
                  wxT("Unable to take a good step while searching ")
                  wxT("for stopping step in command\n   \"") +
                  GetGeneratingString() + wxT("\"\n"));

         #ifdef DEBUG_STOPPING_CONDITIONS
            if (fm[0])
               MessageInterface::ShowMessage(wxT("\nAfter step, param = %16.12lf, ")
                     wxT("Stepped to %16.12lf\n"), stopParam->EvaluateReal(),
                     fm[0]->GetTime());
            else
               MessageInterface::ShowMessage(wxT("\nAfter step, param = %16.12lf, ")
                     wxT("Stepped to %16.12lf\n"), stopParam->EvaluateReal(),
                     p[0]->GetTime());
         #endif

         // Update spacecraft for that step
         for (UnsignedInt i = 0; i < fm.size(); ++i)
         {
            if (fm[i])
               fm[i]->UpdateSpaceObject(
                  baseEpoch[i] + fm[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);
            else
               p[i]->UpdateSpaceObject(
                  baseEpoch[i] + p[i]->GetTime() / GmatTimeConstants::SECS_PER_DAY);
         }

         #ifdef DEBUG_SECANT_DETAILS
            MessageInterface::ShowMessage(
                  wxT("Secant run %d segment points: [%16.12lf %16.12lf], ")
                  wxT("[%16.12lf %16.12lf] => Estimate "), attempts, x[0], y[0],
                  x[1], y[1]);
         #endif

         // Buffer data for next iteration
         if (nextTimeThrough)
         {
            x[0] = x[1];
            y[0] = y[1];
         }
         else
            nextTimeThrough = true;

         // And store current results
         x[1] = secsToStep;
         y[1] = stopParam->EvaluateReal();
         if (stopper->IsCyclicParameter())
            y[1] = GetRangedAngle(y[1], target);

         #ifdef DEBUG_SECANT_DETAILS
            MessageInterface::ShowMessage(wxT("[%16.12lf %16.12lf]\n"), x[1], y[1]);
         #endif

         // Check to see if accuracy is within tolerance
         if (fabs(stopper->GetStopDifference()) < stopAccuracy)
         {
            #ifdef DEBUG_STOPPING_CONDITIONS
               MessageInterface::ShowMessage(wxT("Success!  Stop diff = %.13lf, ")
                     wxT("accuracy = %.13lf\n"), stopper->GetStopDifference(),
                     stopAccuracy);
            #endif
            closeEnough = true;
         }

         ++attempts;

         #ifdef DEBUG_STOPPING_CONDITIONS
            MessageInterface::ShowMessage(
               wxT("   %d: secsToStep = %16.12lf, Stop diff = %16.12lf, ")
               wxT("Accuracy = %16.12lf\n"), attempts, secsToStep,
               stopper->GetStopDifference(), stopAccuracy);
         #endif
      }
   }

   if (attempts == 50)
   {
      // Back out last step, then try bisection
      BufferSatelliteStates(false);
      for (UnsignedInt i = 0; i < fm.size(); ++i)
      {
         if (fm[i])
         {
            fm[i]->UpdateFromSpaceObject();
            fm[i]->SetTime(fm[i]->GetTime() - secsToStep);
         }
         else
         {
            p[i]->UpdateFromSpaceObject();
            p[i]->SetTime(p[i]->GetTime() - secsToStep);
         }
      }

      Real bisectSecsToStep = BisectToStop(stopper);
      if (bisectSecsToStep != 0.0)
         secsToStep = bisectSecsToStep;
      else
         MessageInterface::ShowMessage(
               wxT("WARNING: Failed to find a good stopping point for condition ")
               wxT("\"%s\" in 50 attempts, and bisection failed as well!\n"),
               stopper->GetName().c_str());
   }

   // Restore the spacecraft and force models to the end state of the last step
   BufferSatelliteStates(false);
   for (UnsignedInt i = 0; i < fm.size(); ++i)
   {
      if (fm[i])
      {
         fm[i]->UpdateFromSpaceObject();
         fm[i]->SetTime(fm[i]->GetTime() - secsToStep);
      }
      else
      {
         p[i]->UpdateFromSpaceObject();
         p[i]->SetTime(p[i]->GetTime() - secsToStep);
      }
   }

   return secsToStep;
}

//------------------------------------------------------------------------------
// Real BisectToStop(StopCondition *stopper)
//------------------------------------------------------------------------------
/**
 * Bisection method used as a "last resort" to find stopping point.
 *
 * @param <stopper> Stopping condition we're using.
 */
//------------------------------------------------------------------------------
Real Propagate::BisectToStop(StopCondition *stopper)
{
   Integer attempts = 0, attemptsMax = 52;  // 52 bits in ANSI double
   bool closeEnough = false;
   Real secsToStep = stepBrackets[1];
   Real values[2], currentValue, previousValue;
   Real target = 0.0;       // Bogus initialization to clear a warning
   Real dt = stepBrackets[1] - stepBrackets[0];
   Real increasing = 1.0;

   Parameter *stopParam = stopper->GetStopParameter(),
             *targParam = stopper->GetGoalParameter();

   currentValue = values[0] = values[1] = stopParam->EvaluateReal();
   previousValue = currentValue - 1.0;  // All that matters is that they differ

   while ((attempts < attemptsMax) && !closeEnough)
   {
      if (attempts > 0)
      {
         BufferSatelliteStates(false);
         for (UnsignedInt i = 0; i < fm.size(); ++i)
         {
            if (fm[i])
            {
               fm[i]->UpdateFromSpaceObject();
               fm[i]->SetTime(fm[i]->GetTime() - secsToStep);
            }
            else
            {
               p[i]->UpdateFromSpaceObject();
               p[i]->SetTime(p[i]->GetTime() - secsToStep);
            }
         }

         dt *= 0.5;

         if (attempts == 1)
         {

            values[1] = currentValue;
            secsToStep = stepBrackets[0] + dt;
            if (stopper->IsCyclicParameter())
               values[0] = GetRangedAngle(values[0], target);

            if (values[1] < values[0])
               increasing = -1.0;
         }
         else
         {
            if (currentValue > target)
            {
               secsToStep -= increasing * dt;
               if (increasing > 0.0)
                  values[1] = currentValue;
               else
                  values[0] = currentValue;
            }
            else
            {
               secsToStep += increasing * dt;
               if (increasing > 0.0)
                  values[0] = currentValue;
               else
                  values[1] = currentValue;
            }
         }
      }

      if (!TakeAStep(secsToStep))
         throw CommandException(wxT("Unable to take a good step while searching ")
            wxT("for stopping step in command\n   \"") + GetGeneratingString() +
            wxT("\"\n"));

      // Update spacecraft for that step
      for (UnsignedInt i = 0; i < fm.size(); ++i)
      {
         if (fm[i])
         {
            fm[i]->UpdateSpaceObject(baseEpoch[i] + fm[i]->GetTime() /
                  GmatTimeConstants::SECS_PER_DAY);
         }
         else
         {
            p[i]->UpdateSpaceObject(baseEpoch[i] + p[i]->GetTime() /
                  GmatTimeConstants::SECS_PER_DAY);
         }
      }

      if (targParam != NULL)
         target = targParam->EvaluateReal();
      else
         target = stopper->GetStopGoal();

      previousValue = currentValue;
      currentValue = stopParam->EvaluateReal();
      if (stopper->IsCyclicParameter())
      {
         currentValue = GetRangedAngle(currentValue, target);
      }

      #ifdef DEBUG_BISECTION_DETAILS
         if (attempts == 0)
         {
            MessageInterface::ShowMessage(
               wxT("Bisection[%d]: Stop(%16.13lf) = %16.13lf\n\n")
               wxT("           Stop(%16.13lf) = %16.13lf\n")
               wxT("           Goal           = %16.13lf\n"),
               attempts, stepBrackets[0], values[0], secsToStep, currentValue,
               target);
         }
         else
         {
            MessageInterface::ShowMessage(
               wxT("Bisection[%d]: Stop(%16.13lf) = %16.13lf\n")
               wxT("           Stop(%16.13lf) = %16.13lf\n\n")
               wxT("           Stop(%16.13lf) = %16.13lf\n")
               wxT("           Goal           = %16.13lf\n"),
               attempts, stepBrackets[0], values[0], stepBrackets[1], values[1],
               secsToStep, currentValue, target);
         }
      #endif

      ++attempts;

      if (fabs(target - currentValue) < stopAccuracy)
            closeEnough = true;

      if (previousValue == currentValue)
      {

         MessageInterface::ShowMessage(wxT("The command \"%s\" cannot satisfy ")
               wxT("the stopping tolerance of \"%le\" for the stopping condition ")
               wxT("\"%s\".  The achieved accuracy is \"%.12lf\".\n"),
               GetGeneratingString(Gmat::NO_COMMENTS).c_str(), stopAccuracy,
               stopper->GetName().c_str(), fabs(target - currentValue));

         closeEnough = true;
      }
   }

   if (attempts == attemptsMax)
      secsToStep = 0.0;

   return secsToStep;
}


//------------------------------------------------------------------------------
// void RunComplete()
//------------------------------------------------------------------------------
/**
 * Resets the Propagate command to an uninitialized state.
 */
//------------------------------------------------------------------------------
void Propagate::RunComplete()
{
   if (inProgress)
   {
      publisher->FlushBuffers();
      publisher->UnregisterPublishedData(this);
   }

   inProgress = false;
   hasFired = false;

   #ifdef DEBUG_FIRST_CALL
      firstStepFired = false;
   #endif

   // Clear transient forces(LOJ: 2009.05.07)
   ClearTransientForces();

   GmatCommand::RunComplete();
}

//------------------------------------------------------------------------------
// GmatBase* GetClone(Integer cloneIndex = 0)
//------------------------------------------------------------------------------
/**
 * Retrieves a pointer to a clone so its attributes can be accessed
 *
 * @param cloneIndex The index into the clone array
 *
 * @return The clone pointer, or NULL if no clone exists
 */
//------------------------------------------------------------------------------
GmatBase* Propagate::GetClone(Integer cloneIndex)
{
   #ifdef DEBUG_CLONE_UPDATES
      MessageInterface::ShowMessage(wxT("Entered Propagate::GetClone(%d)\n"),
            cloneIndex);
   #endif

   GmatBase *retPtr = NULL;

   /// todo: Handle the ancestor classes
   if ((cloneIndex >= 0) && (cloneIndex < (Integer)prop.size()))
      retPtr = prop[cloneIndex];

   return retPtr;
}


//------------------------------------------------------------------------------
// void AddTransientForce(StringArray *sats, ForceModel *p,
//       PropagationStateManager *propMan)
//------------------------------------------------------------------------------
/**
 * Passes transient forces into the ForceModel(s).
 *
 * @param sats The array of satellites used in the ForceModel.
 * @param p    The current ForceModel that is receiving the forces.
 * @param propMan PropagationStateManager for this PropSetup
 */
//------------------------------------------------------------------------------
void Propagate::AddTransientForce(StringArray *sats, ODEModel *p,
      PropagationStateManager *propMan)
{
   #ifdef DEBUG_TRANSIENT_FORCES
   MessageInterface::ShowMessage
      (wxT("Propagate::AddTransientForce() entered, ODEModel=<%p>, transientForces=<%p>\n"),
       p, transientForces);
   #endif

   // Find any transient force that is active and add it to the force model
   for (std::vector<PhysicalModel*>::iterator i = transientForces->begin();
        i != transientForces->end(); ++i)
   {
      StringArray tfSats = (*i)->GetRefObjectNameArray(Gmat::SPACECRAFT);
      // Loop through the spacecraft that go with the force model, and see if
      // they are in the spacecraft list for the current transient force
      for (StringArray::iterator current = sats->begin();
           current != sats->end(); ++current)
      {
         if (find(tfSats.begin(), tfSats.end(), *current) != tfSats.end())
         {
            #ifdef DEBUG_TRANSIENT_FORCES
            MessageInterface::ShowMessage
               (wxT("   Adding transientForce <%p>'%s' to ODEModel\n"), *i,
                (*i)->GetName().c_str());
            #endif
            p->AddForce(*i);
            if ((*i)->DepletesMass())
            {
               propMan->SetProperty(wxT("MassFlow"));
//               propMan->SetProperty("MassFlow",
//                     (*i)->GetRefObject(Gmat::SPACECRAFT, *current));
               #ifdef DEBUG_TRANSIENT_FORCES
                  MessageInterface::ShowMessage(wxT("   %s depletes mass\n"),
                        (*i)->GetName().c_str());
               #endif
            }
            break;      // Avoid multiple adds
         }
      }
   }

   #ifdef DEBUG_TRANSIENT_FORCES
      ODEModel *fm;
      PhysicalModel *pm;

      MessageInterface::ShowMessage(
         wxT("Propagate::AddTransientForces completed; force details:\n"));
      for (std::vector<PropSetup*>::iterator p = prop.begin();
           p != prop.end(); ++p)
      {
         fm = (*p)->GetODEModel();
         if (!fm)
            throw CommandException(wxT("ODEModel not set in PropSetup \"") +
                                   (*p)->GetName() + wxT("\""));
         MessageInterface::ShowMessage(
            wxT("   Forces in %s:\n"), fm->GetName().c_str());
         for (Integer i = 0; i < fm->GetNumForces(); ++i)
         {
            pm = fm->GetForce(i);
            MessageInterface::ShowMessage(
               wxT("      %15s   %s\n"), pm->GetTypeName().c_str(),
               pm->GetName().c_str());
         }
      }
   #endif
}


//------------------------------------------------------------------------------
// void ClearTransientForce()
//------------------------------------------------------------------------------
/**
 * Removes transient forces from the ForceModel(s) after propagation.
 */
//------------------------------------------------------------------------------
void Propagate::ClearTransientForces()
{
   #ifdef DEBUG_TRANSIENT_FORCES
   MessageInterface::ShowMessage
      (wxT("Propagate::ClearTransientForces() entered, prop.size()=%d\n"),
       prop.size());
   #endif

   ODEModel *fm;
   PhysicalModel *pm;

   // Loop through the forces in each force model, and remove transient ones
   for (std::vector<PropSetup*>::iterator p = prop.begin();
        p != prop.end(); ++p)
   {
      if ((*p)->GetPropagator()->UsesODEModel())
      {
         fm = (*p)->GetODEModel();
         if (!fm)
            throw CommandException(wxT("ForceModel not set in PropSetup \"") +
                                   (*p)->GetName() + wxT("\""));

         #ifdef DEBUG_TRANSIENT_FORCES
         MessageInterface::ShowMessage(wxT("   ODEModel=<%p>\n"), fm);
         #endif
         for (Integer i = 0; i < fm->GetNumForces(); ++i)
         {
            pm = fm->GetForce(i);
            #ifdef DEBUG_TRANSIENT_FORCES
            MessageInterface::ShowMessage
               (wxT("      Checking if pm<%p>'%s' is transient\n"), pm, pm->GetName().c_str());
            #endif
            if (pm->IsTransient())
            {
               #ifdef DEBUG_TRANSIENT_FORCES
               MessageInterface::ShowMessage(wxT("   calling fm->DeleteForce()\n"));
               #endif
               fm->DeleteForce(pm->GetName());
               --i;
            }
         }
      }
   }

   #ifdef DEBUG_TRANSIENT_FORCES
      MessageInterface::ShowMessage(
         wxT("Propagate::ClearTransientForces completed; force details:\n"));
      for (std::vector<PropSetup*>::iterator p = prop.begin();
           p != prop.end(); ++p)
      {
         if ((*p)->GetPropagator()->UsesODEModel())
         {
            fm = (*p)->GetODEModel();
            if (!fm)
               throw CommandException(wxT("ForceModel not set in PropSetup \"") +
                                      (*p)->GetName() + wxT("\""));
            #ifdef DEBUG_TRANSIENT_FORCES
            MessageInterface::ShowMessage(wxT("   ODEModel=<%p>\n"), fm);
            #endif
            MessageInterface::ShowMessage(
               wxT("      Forces in %s:\n"), fm->GetName().c_str());
            for (Integer i = 0; i < fm->GetNumForces(); ++i)
            {
               pm = fm->GetForce(i);
               MessageInterface::ShowMessage(
                   wxT("      %15s   %s\n"), pm->GetTypeName().c_str(),
                   pm->GetName().c_str());
             }
         }
      }
   #endif
}


//------------------------------------------------------------------------------
// void SetNames(const wxString& name, StringArray& owners,
//               StringArray& elements)
//------------------------------------------------------------------------------
/**
 * Sets the parameter names used when publishing Spacecraft data.
 *
 * @param <name>     Name of the Spacecraft that is referenced.
 * @param <owners>   Array of published data identifiers.
 * @param <elements> Individual elements of the published data.
 */
//------------------------------------------------------------------------------
void Propagate::SetNames(const wxString& name, StringArray& owners,
                         StringArray& elements)
{
   // Add satellite labels
   for (Integer i = 0; i < 6; ++i)
      owners.push_back(name);       // X, Y, Z, Vx, Vy, Vz

   elements.push_back(name+wxT(".X"));
   elements.push_back(name+wxT(".Y"));
   elements.push_back(name+wxT(".Z"));
   elements.push_back(name+wxT(".Vx"));
   elements.push_back(name+wxT(".Vy"));
   elements.push_back(name+wxT(".Vz"));
}


////------------------------------------------------------------------------------
//// void AddToBuffer(SpaceObject *so)
////------------------------------------------------------------------------------
///**
// * Adds satellites and formations to the state buffer.
// *
// * @param <so> The SpaceObject that is added.
// */
////------------------------------------------------------------------------------
//void Propagate::AddToBuffer(SpaceObject *so)
//{
//   #ifdef DEBUG_STOPPING_CONDITIONS
//      MessageInterface::ShowMessage("Buffering states for '%s'\n",
//         so->GetName().c_str());
//   #endif
//
//   if (so->IsOfType(Gmat::SPACECRAFT))
//   {
//      Spacecraft *clonedSat = (Spacecraft *)(so->Clone());
//      satBuffer.push_back(clonedSat);
//      #ifdef DEBUG_MEMORY
//      MemoryTracker::Instance()->Add
//         (clonedSat, clonedSat->GetName(), "Propagate::AddToBuffer()",
//          "(Spacecraft *)(so->Clone())");
//      #endif
//      //satBuffer.push_back((Spacecraft *)(so->Clone()));
//   }
//   else if (so->IsOfType(Gmat::FORMATION))
//   {
//      Formation *form = (Formation*)so;
//      Formation *clonedForm = (Formation *)(so->Clone());
//      #ifdef DEBUG_MEMORY
//      MemoryTracker::Instance()->Add
//         (clonedForm, clonedForm->GetName(), "Propagate::AddToBuffer()",
//          "(Formation *)(so->Clone())");
//      #endif
//      //formBuffer.push_back((Formation *)(so->Clone()));
//      formBuffer.push_back(clonedForm);
//      StringArray formSats = form->GetStringArrayParameter("Add");
//
//      for (StringArray::iterator i = formSats.begin(); i != formSats.end(); ++i)
//         AddToBuffer((SpaceObject *)(FindObject(*i)));
//   }
//   else
//      throw CommandException("Object " + so->GetName() + " is not either a "
//         "Spacecraft or a Formation; cannot buffer the object for propagator "
//         "stopping conditions.");
//}
//
//
////------------------------------------------------------------------------------
//// void EmptyBuffer()
////------------------------------------------------------------------------------
///**
// * Cleans up the satellite state buffer.
// */
////------------------------------------------------------------------------------
//void Propagate::EmptyBuffer()
//{
//   for (std::vector<Spacecraft *>::iterator i = satBuffer.begin();
//        i != satBuffer.end(); ++i)
//   {
//      #ifdef DEBUG_MEMORY
//      MemoryTracker::Instance()->Remove
//         ((*i), (*i)->GetName(), "Propagate::EmptyBuffer()", "deleting from satBuffer");
//      #endif
//      delete (*i);
//   }
//   satBuffer.clear();
//
//   for (std::vector<Formation *>::iterator i = formBuffer.begin();
//        i != formBuffer.end(); ++i)
//   {
//      #ifdef DEBUG_MEMORY
//      MemoryTracker::Instance()->Remove
//         ((*i), (*i)->GetName(), "Propagate::EmptyBuffer()", "deleting from fromBuffer");
//      #endif
//      delete (*i);
//   }
//   formBuffer.clear();
//}
//
////------------------------------------------------------------------------------
//// void BufferSatelliteStates(bool fillingBuffer)
////------------------------------------------------------------------------------
///**
// * Preserves satellite state data so it can be restored after interpolating the
// * stopping condition propagation time.
// *
// * @param <fillingBuffer> Flag used to indicate the fill direction.
// */
////------------------------------------------------------------------------------
//void Propagate::BufferSatelliteStates(bool fillingBuffer)
//{
//   Spacecraft *fromSat, *toSat;
//   Formation *fromForm, *toForm;
//   wxString soName;
//
//   for (std::vector<Spacecraft *>::iterator i = satBuffer.begin();
//        i != satBuffer.end(); ++i)
//   {
//      soName = (*i)->GetName();
//      if (fillingBuffer)
//      {
//         fromSat = (Spacecraft *)FindObject(soName);
//         toSat = *i;
//      }
//      else
//      {
//         fromSat = *i;
//         toSat = (Spacecraft *)FindObject(soName);
//      }
//
//      #ifdef DEBUG_STOPPING_CONDITIONS
//         MessageInterface::ShowMessage(
//            "   Sat is %s, fill direction is %s; fromSat epoch = %.12lf   "
//            "toSat epoch = %.12lf\n",
//            fromSat->GetName().c_str(),
//            (fillingBuffer ? "from propagator" : "from buffer"),
//            fromSat->GetRealParameter("A1Epoch"),
//            toSat->GetRealParameter("A1Epoch"));
//
//         MessageInterface::ShowMessage(
//            "   '%s' Satellite state:\n", fromSat->GetName().c_str());
//         Real *satrv = fromSat->GetState().GetState();
//         MessageInterface::ShowMessage(
//            "      %.12lf  %.12lf  %.12lf\n      %.12lf  %.12lf  %.12lf\n",
//            satrv[0], satrv[1], satrv[2], satrv[3], satrv[4], satrv[5]);
//      #endif
//
//      (*toSat) = (*fromSat);
//
//      #ifdef DEBUG_STOPPING_CONDITIONS
//         MessageInterface::ShowMessage(
//            "After copy, From epoch %.12lf to epoch %.12lf\n",
//            fromSat->GetRealParameter("A1Epoch"),
//            toSat->GetRealParameter("A1Epoch"));
//      #endif
//   }
//
//   for (std::vector<Formation *>::iterator i = formBuffer.begin();
//        i != formBuffer.end(); ++i)
//   {
//      soName = (*i)->GetName();
//      #ifdef DEBUG_STOPPING_CONDITIONS
//         MessageInterface::ShowMessage("Buffering formation %s, filling = %s\n",
//            soName.c_str(), (fillingBuffer?"true":"false"));
//      #endif
//      if (fillingBuffer)
//      {
//         fromForm = (Formation *)FindObject(soName);
//         toForm = *i;
//      }
//      else
//      {
//         fromForm = *i;
//         toForm = (Formation *)FindObject(soName);
//      }
//
//      #ifdef DEBUG_STOPPING_CONDITIONS
//         MessageInterface::ShowMessage(
//            "   Formation is %s, fill direction is %s; fromForm epoch = %.12lf"
//            "   toForm epoch = %.12lf\n",
//            fromForm->GetName().c_str(),
//            (fillingBuffer ? "from propagator" : "from buffer"),
//            fromForm->GetRealParameter("A1Epoch"),
//            toForm->GetRealParameter("A1Epoch"));
//      #endif
//
//      (*toForm) = (*fromForm);
//
//      toForm->UpdateState();
//
//      #ifdef DEBUG_STOPPING_CONDITIONS
//         Integer count = fromForm->GetStringArrayParameter("Add").size();
//
//         MessageInterface::ShowMessage(
//            "After copy, From epoch %.12lf to epoch %.12lf\n",
//            fromForm->GetRealParameter("A1Epoch"),
//            toForm->GetRealParameter("A1Epoch"));
//
//         MessageInterface::ShowMessage(
//            "   %s for '%s' Formation state:\n",
//            (fillingBuffer ? "Filling buffer" : "Restoring states"),
//            fromForm->GetName().c_str());
//
//         Real *satrv = fromForm->GetState().GetState();
//
//         for (Integer i = 0; i < count; ++i)
//            MessageInterface::ShowMessage(
//               "      %d:  %.12lf  %.12lf  %.12lf  %.12lf  %.12lf  %.12lf\n",
//               i, satrv[i*6], satrv[i*6+1], satrv[i*6+2], satrv[i*6+3],
//               satrv[i*6+4], satrv[i*6+5]);
//      #endif
//   }
//
//   #ifdef DEBUG_STOPPING_CONDITIONS
//      for (std::vector<Spacecraft *>::iterator i = satBuffer.begin();
//           i != satBuffer.end(); ++i)
//         MessageInterface::ShowMessage(
//            "   Epoch of '%s' is %.12lf\n", (*i)->GetName().c_str(),
//            (*i)->GetRealParameter("A1Epoch"));
//   #endif
//}
//
//

//------------------------------------------------------------------------------
// Real GetRangedAngle(const Real angle, const Real midpt)
//------------------------------------------------------------------------------
/**
 * Puts a cyclic parameter into its valid range.  Currently only implemented for
 * angles.
 *
 * @param <angle> The parameter value.
 * @param <midpt> The center of the range.
 * @param <min>   The minimum of the nominal range.  Not yet implemented.
 * @param <max>   The maximum of the nominal range.  Not yet implemented.
 *
 * @return The ranged value.
 */
//------------------------------------------------------------------------------
Real Propagate::GetRangedAngle(const Real angle, const Real midpt)
{
   #ifdef DEBUG_STOPPING_CONDITIONS
      MessageInterface::ShowMessage(
         wxT("Setting angle range for %.12lf around %.12lf\n"), angle, midpt);
   #endif

   return AngleUtil::PutAngleInDegRange(angle, midpt - GmatMathConstants::PI_DEG,
                                    midpt + GmatMathConstants::PI_DEG);
}
