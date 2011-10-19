//$Id: Maneuver.cpp 9791 2011-08-24 16:17:06Z wendys-dev $
//------------------------------------------------------------------------------
//                                 Maneuver
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P
//
// Author: Darrel J. Conway
// Created: 2004/01/20
//
/**
 * Definition for the Maneuver command class
 */
//------------------------------------------------------------------------------


#include "Maneuver.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_MANEUVER 1
//#define DEBUG_MANEUVER_PARSE
//#define DEBUG_MANEUVER_INIT
//#define DEBUG_MANEUVER_EXEC

//------------------------------------------------------------------------------
// Maneuver()
//------------------------------------------------------------------------------
/**
 * Default constructor.
 */
//------------------------------------------------------------------------------
Maneuver::Maneuver() :
   GmatCommand (wxT("Maneuver")),
   burnName    (wxT("")),
   burn        (NULL),
   satName     (wxT("")),
   sat         (NULL),
   firedOnce   (false),
   // Parameter IDs
   burnNameID  (parameterCount),
   satNameID   (parameterCount+1)
{
   parameterCount     += 2;
   physicsBasedCommand = true;
}


//------------------------------------------------------------------------------
// ~Maneuver()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
Maneuver::~Maneuver()
{
}


//------------------------------------------------------------------------------
// Maneuver(const Maneuver& m)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param m The maneuver that is copied to this one.
 */
//------------------------------------------------------------------------------
Maneuver::Maneuver(const Maneuver& m) :
   GmatCommand     (m),
   burnName    (m.burnName),
   burn        (NULL),
   satName     (m.satName),
   sat         (NULL),
   firedOnce   (false),
   // Parameter IDs
   burnNameID  (m.burnNameID),
   satNameID   (m.satNameID)
{
   parameterCount = m.parameterCount;
}


//------------------------------------------------------------------------------
// Maneuver& operator=(const Maneuver& m)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the Maneuver command.
 *
 * @param m The maneuver that is copied to this one.
 *
 * @return this instance, with the internal data set to match the original's.
 */
//------------------------------------------------------------------------------
Maneuver& Maneuver::operator=(const Maneuver& m)
{
   if (this == &m)
      return *this;

   GmatCommand::operator=(m);
   burnName  = m.burnName;
   burn      = NULL;
   satName   = m.satName;
   sat       = NULL;
   firedOnce = false;

   burnNameID = m.burnNameID;
   satNameID = m.satNameID;

   return *this;
}

//------------------------------------------------------------------------------
//  bool SetObject(const wxString &name, const Gmat::ObjectType type,
//                 const wxString &associate,
//                 const Gmat::ObjectType associateType)
//------------------------------------------------------------------------------
/**
 * Sets an object used by the Maneuver command.
 *
 * @param name The name of the object.
 * @param type Type of object.
 * @param associate Name of an associated object, if needed.
 * @param associateType Type of associated object.
 *
 * @return true if the object was set.
 */
//------------------------------------------------------------------------------
bool Maneuver::SetObject(const wxString &name, const Gmat::ObjectType type,
                         const wxString &associate,
                         const Gmat::ObjectType associateType)
{
   return GmatCommand::SetObject(name, type, associate, associateType);
}


//------------------------------------------------------------------------------
//  bool SetObject(GmatBase *obj, const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Sets an object used by the Maneuver command.
 *
 * @param obj The object.
 * @param type Type of object.
 *
 * @return true if the object was set.
 */
//------------------------------------------------------------------------------
bool Maneuver::SetObject(GmatBase *obj, const Gmat::ObjectType type)
{
   return GmatCommand::SetObject(obj, type);
}



//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the Maneuver.
 *
 * @return clone of the Maneuver.
 */
//------------------------------------------------------------------------------
GmatBase* Maneuver::Clone() const
{
   return (new Maneuver(*this));
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
const wxString& Maneuver::GetGeneratingString(Gmat::WriteMode mode,
                                                 const wxString &prefix,
                                                 const wxString &useName)
{
   generatingString = prefix + wxT("Maneuver ");
   generatingString += burnName + wxT("(") + satName + wxT(");");
   
   return GmatCommand::GetGeneratingString(mode, prefix, useName);
}


//------------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//------------------------------------------------------------------------------
/**
 * This method renames an object used by the maneuver command.
 *
 * @param type The type of the object.
 * @param oldName The current name for the object.
 * @param newName The new name for the object.
 *
 * @return clone of the Maneuver.
 */
//------------------------------------------------------------------------------
bool Maneuver::RenameRefObject(const Gmat::ObjectType type,
                               const wxString &oldName,
                               const wxString &newName)
{
   #if DEBUG_MANEUVER
      MessageInterface::ShowMessage
         (wxT("Maneuver::RenameConfiguredItem() type=%s, oldName=%s, newName=%s\n"),
          GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif

   if (type != Gmat::SPACECRAFT && type != Gmat::IMPULSIVE_BURN)
      return true;

   if (type == Gmat::SPACECRAFT)
   {
      if (satName == oldName)
      {
         satName = newName;
         return true;
      }
   }
   else if (type == Gmat::IMPULSIVE_BURN)
   {
      if (burnName == oldName)
      {
         burnName = newName;
      }
   }

   return true;
}


//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by the Maneuver.
 *
 * @return the list of object types.
 *
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& Maneuver::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::IMPULSIVE_BURN);
   refObjectTypes.push_back(Gmat::SPACECRAFT);
   return refObjectTypes;
}


//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref objects used by the Maneuver.
 *
 * @param <type> The type of object desired, or Gmat::UNKNOWN_OBJECT for the
 *               full list.
 *
 * @return the list of object names.
 *
 */
//------------------------------------------------------------------------------
const StringArray& Maneuver::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   #ifdef DEBUG_MANEUVER_REFOBJ
   MessageInterface::ShowMessage(wxT("Maneuver::GetRefObjectNameArray(%d)\n"), type);
   #endif

   refObjectNames.clear();

   if (type == Gmat::UNKNOWN_OBJECT ||
       type == Gmat::IMPULSIVE_BURN)
   {
      refObjectNames.push_back(burnName);
   }

   if (type == Gmat::UNKNOWN_OBJECT ||
       type == Gmat::SPACECRAFT)
   {
      refObjectNames.push_back(satName);
   }

   #ifdef DEBUG_MANEUVER_REFOBJ
   MessageInterface::ShowMessage(wxT("===> returning\n"));
   for (UnsignedInt i=0; i<refObjectNames.size(); i++)
      MessageInterface::ShowMessage(wxT("   %s\n"), refObjectNames[i].c_str());
   #endif

   return refObjectNames;
}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter text, given the input parameter ID.
 *
 * @param id Id for the requested parameter text.
 *
 * @return parameter text for the requested parameter.
 */
//------------------------------------------------------------------------------
wxString Maneuver::GetParameterText(const Integer id) const
{
   if (id == burnNameID) {
      return wxT("Burn");
   }

   else if (id == satNameID) {
      return wxT("Spacecraft");
   }

   return GmatCommand::GetParameterText(id);
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
Integer Maneuver::GetParameterID(const wxString &str) const
{
   if (str == wxT("Burn")) {
      return burnNameID;
   }

   else if (str == wxT("Spacecraft")) {
      return satNameID;
   }

   return GmatCommand::GetParameterID(str);
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
Gmat::ParameterType Maneuver::GetParameterType(const Integer id) const
{
   if (id == burnNameID) {
      return Gmat::STRING_TYPE;
   }

   if (id == satNameID) {
      return Gmat::STRING_TYPE;
   }

   return GmatCommand::GetParameterType(id);
}


//------------------------------------------------------------------------------
//  wxString  GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type string, given the input parameter ID.
 *
 * @param id ID for the requested parameter.
 *
 * @return parameter type string of the requested parameter.
 */
//------------------------------------------------------------------------------
wxString Maneuver::GetParameterTypeString(const Integer id) const
{
   if (id == burnNameID)
      return PARAM_TYPE_STRING[Gmat::STRING_TYPE];

   if (id == satNameID)
      return PARAM_TYPE_STRING[Gmat::STRING_TYPE];

   return GmatCommand::GetParameterTypeString(id);
}


//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter ID.
 *
 * @param id ID for the requested parameter.
 *
 * @return  string value of the requested parameter.
 */
//------------------------------------------------------------------------------
wxString Maneuver::GetStringParameter(const Integer id) const
{
   if (id == burnNameID)
      return burnName;

   if (id == satNameID)
      return satName;

   return GmatCommand::GetStringParameter(id);
}


//------------------------------------------------------------------------------
//  wxString  SetStringParameter(const Integer id, const wxString value)
//------------------------------------------------------------------------------
/**
 * This method sets the string parameter value, given the input
 * parameter ID.
 *
 * @param id    ID for the requested parameter.
 * @param value string value for the requested parameter.
 *
 * @return  success flag.
 */
//------------------------------------------------------------------------------
bool Maneuver::SetStringParameter(const Integer id, const wxString &value)
{
   if (id == burnNameID) {
      burnName = value;
      return true;
   }

   if (id == satNameID) {
      satName = value;
      return true;
   }

   return GmatCommand::SetStringParameter(id, value);
}


//------------------------------------------------------------------------------
//  bool InterpretAction()
//------------------------------------------------------------------------------
/**
 * Parses the command string and builds the corresponding command structures.
 *
 * The Maneuver command has the following syntax:
 *
 *     Maneuver burn1(sat1);
 *
 * where burn1 is an ImpulsiveBurn used to perform the maneuver, and sat1 is the
 * name of the spacecraft that is maneuvered.  This method breaks the script
 * line into the corresponding pieces, and stores the name of the ImpulsiveBurn
 * and the Spacecraft so they can be set to point to the correct objects during
 * initialization.
 */
//------------------------------------------------------------------------------
bool Maneuver::InterpretAction()
{
   StringArray chunks = InterpretPreface();

   // Find and set the burn object name ...
   StringArray currentChunks = parser.Decompose(chunks[1], wxT("()"), false);

   if (currentChunks.size() < 2)
      throw CommandException(wxT("Missing Maneuver parameter. Expecting ")
                             wxT("\"ImpulsiveBurnName(SpacecraftName)\"\n"));

   SetStringParameter(burnNameID, currentChunks[0]);

   #ifdef DEBUG_MANEUVER_PARSE
      MessageInterface::ShowMessage(wxT("In Maneuver, after Decompose, currentChunks = \n"));
      for (unsigned int ii=0; ii<currentChunks.size(); ii++)
         MessageInterface::ShowMessage(wxT("    %s\n"), currentChunks.at(ii).c_str());
   #endif
   // ... and the spacecraft that is maneuvered
   currentChunks = parser.SeparateBrackets(currentChunks[1], wxT("()"), wxT(","));
   #ifdef DEBUG_MANEUVER_PARSE
      MessageInterface::ShowMessage(wxT("In Maneuver, after Separatebrackets, currentChunks = \n"));
      for (unsigned int ii=0; ii<currentChunks.size(); ii++)
         MessageInterface::ShowMessage(wxT("    %s\n"), currentChunks.at(ii).c_str());
   #endif
   if (currentChunks.size() > 1)
      throw CommandException(wxT("Unexpected text after spacecraft name in Maneuver command\n"));

   SetStringParameter(satNameID, currentChunks[0]);

   return true;
}


//------------------------------------------------------------------------------
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * Performs the initialization needed to run the Maneuver.
 *
 * @return true if the GmatCommand is initialized, false if an error occurs.
 */
//------------------------------------------------------------------------------
bool Maneuver::Initialize()
{
   #ifdef DEBUG_MANEUVER_INIT
   MessageInterface::ShowMessage
      (wxT("Maneuver::Initialize() this=<%p>'%s' entered\n"), this,
       GetGeneratingString(Gmat::NO_COMMENTS).c_str());
   #endif
   
   GmatCommand::Initialize();

   GmatBase *mapObj = NULL;
   if ((mapObj = FindObject(burnName)) == NULL)
      throw CommandException(wxT("Maneuver command cannot find Burn"));
   burn = (Burn *)mapObj;
   if (!burn)
      return false;

   if ((mapObj = FindObject(satName)) == NULL)
      throw CommandException(wxT("Maneuver command cannot find Spacecraft"));
   sat = (Spacecraft *)mapObj;
   if (!sat)
      return false;
   
   // Register this Maneuver to the Publisher (LOJ: 2009.11.10)
   // Do we need to register? Currently Maneuver does not publish any data.
   // Just remove it for now.
   #ifdef __REGISTER_MANEUVER__
   StringArray owners, elements;
   owners.push_back(satName);
   elements.push_back(satName + wxT(".epoch"));
   streamID = publisher->RegisterPublishedData(this, streamID, owners, elements);
   
   #ifdef DEBUG_MANEUVER_INIT
   MessageInterface::ShowMessage(wxT("   streamID=%d\n"), streamID);
   MessageInterface::ShowMessage
      (wxT("Maneuver::Initialize() this=<%p>'%s' returning true\n"), this,
       GetGeneratingString(Gmat::NO_COMMENTS).c_str());
   #endif
   #endif
   
   firedOnce = false;
   return true;
}


//------------------------------------------------------------------------------
//  bool Execute()
//------------------------------------------------------------------------------
/**
 * Fires an impulsive maneuver.
 *
 * This method calls the Fire method on an ImpulsiveBurn object.
 *
 * @return true if the Maneuver is executed, false if not.
 */
//------------------------------------------------------------------------------
bool Maneuver::Execute()
{
   #ifdef DEBUG_MANEUVER_EXEC
      MessageInterface::ShowMessage(wxT("Maneuver::Execute this=<%p> maneuvering %s\n"),
            this, ((sat == NULL) ? wxT("a NULL spaceecraft") : sat->GetName().c_str()));
   #endif
      
   Real epoch = sat->GetRealParameter(wxT("A1Epoch"));
   
   #ifdef DEBUG_MANEUVER_EXEC
   Rvector6 state = sat->GetState(0); // Get cartesian state
   MessageInterface::ShowMessage
      (wxT("   state before maneuver at epoch %f\n   %s\n"), epoch, state.ToString().c_str());
   #endif
   
   burn->SetSpacecraftToManeuver(sat);
   
   // Set maneuvering to Publisher so that any subscriber can do its own action
   publisher->SetManeuvering(this, true, epoch, satName, wxT("ImpulsiveBurn"));
   
   bool retval = burn->Fire(NULL, epoch);
   
   // Reset maneuvering to Publisher so that any subscriber can do its own action
   publisher->SetManeuvering(this, false, epoch, satName, wxT("ImpulsiveBurn"));
   
   #ifdef DEBUG_MANEUVER_EXEC
   state = sat->GetState(0); // Get cartesian state
   MessageInterface::ShowMessage
      (wxT("   state after  maneuver at epoch %f \n   %s"), epoch, state.ToString().c_str());
   #endif
   
   firedOnce = true;
   BuildCommandSummary(true);
   
   #ifdef DEBUG_MANEUVER_EXEC
      MessageInterface::ShowMessage(wxT("Maneuver::Execute this=<%p> complete\n"), this);
   #endif

   return retval;
}


//-------------------------------------------
// Protected Methods
//-------------------------------------------


//------------------------------------------------------------------------------
// void BuildCommandSummary(bool commandCompleted)
//------------------------------------------------------------------------------
/**
 * This method...
 *
 * @param
 *
 * @return
 */
//------------------------------------------------------------------------------
void Maneuver::BuildCommandSummaryString(bool commandCompleted)
{
   bool canSummarize = commandCompleted && firedOnce;
   GmatCommand::BuildCommandSummaryString(canSummarize);

   if (canSummarize)
   {
      wxString data;

      data << wxT("\n")
           << wxT("\n        Maneuver Summary")
           << wxT("\n        -----------------")
           << wxT("\n        Impulsive Burn:     ") << burnName
           << wxT("\n        Spacecraft:         ") << burn->GetStringParameter(burn->GetParameterID(wxT("SpacecraftName")))
           << wxT("\n        Coordinate System:  ") << burn->GetStringParameter(burn->GetParameterID(wxT("CoordinateSystem")))
           << wxT("\n        Origin:             ") << burn->GetStringParameter(burn->GetParameterID(wxT("Origin")))
           << wxT("\n        Axes:               ") << burn->GetStringParameter(burn->GetParameterID(wxT("Axes")))

           << wxT("\n        Burn Vector:")
           << wxT("\n           Element 1:  ") << BuildNumber(burn->GetRealParameter(burn->GetParameterID(wxT("Element1"))))
           << wxT("\n           Element 2:  ") << BuildNumber(burn->GetRealParameter(burn->GetParameterID(wxT("Element2"))))
           << wxT("\n           Element 3:  ") << BuildNumber(burn->GetRealParameter(burn->GetParameterID(wxT("Element3"))))
           << wxT("\n");

      if (burn->GetBooleanParameter(burn->GetParameterID(wxT("DecrementMass"))))
      {
         Real thrust, tx, ty, tz;
         tx = burn->GetRealParameter(burn->GetParameterID(wxT("Element1")));
         ty = burn->GetRealParameter(burn->GetParameterID(wxT("Element2")));
         tz = burn->GetRealParameter(burn->GetParameterID(wxT("Element3")));
         thrust = GmatMathUtil::Sqrt(tx*tx + ty*ty + tz*tz);

         StringArray tanks =
               burn->GetStringArrayParameter(burn->GetParameterID(wxT("Tank")));
         wxString tanklist;

         if (tanks.size() == 1)
            tanklist = tanks[0];
         else
         {
            for (UnsignedInt i = 0; i < tanks.size(); ++i)
            {
               tanklist += tanks[i];
               tanklist += wxT(", ");
               if (i == (tanks.size() - 2))
                  tanklist += wxT("and ");
            }
         }

         data << wxT("\n        Mass depletion from ") << tanklist <<wxT(":  ")
              << wxT("\n           Thrust:       ")    << BuildNumber(thrust) << wxT(" N")
              << wxT("\n           Isp:          ")    << BuildNumber(burn->GetRealParameter(burn->GetParameterID(wxT("Isp")))) << wxT(" s")
              << wxT("\n           Mass change:  ")    << BuildNumber(burn->GetRealParameter(burn->GetParameterID(wxT("DeltaTankMass")))) << wxT(" kg")
              << wxT("\n");

      }
      else
         data << wxT("\n")
              << wxT("\n        No mass depletion\n");


      commandSummary = commandSummary + data;
   }
}
