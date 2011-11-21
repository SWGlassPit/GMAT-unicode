//$Id: EventLocator.cpp 9869 2011-09-15 18:54:39Z djcinsb $
//------------------------------------------------------------------------------
//                           EventLocator
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under NASA Prime
// Contract NNG10CP02C, Task Order 28
//
// Author: Darrel J. Conway, Thinking Systems, Inc.
// Created: Jul 6, 2011
//
/**
 * Definition of the ...
 */
//------------------------------------------------------------------------------


#include "EventLocator.hpp"
#include "EventException.hpp"
#include "FileManager.hpp"      // for GetPathname()
#include "MessageInterface.hpp"


//#define DEBUG_DUMPEVENTDATA

#ifdef DEBUG_DUMPEVENTDATA
   #include <fstream>
   std::ofstream dumpfile("LocatorData.txt");
#endif

//------------------------------------------------------------------------------
// Static data
//------------------------------------------------------------------------------
const wxString
EventLocator::PARAMETER_TEXT[EventLocatorParamCount - GmatBaseParamCount] =
{
   wxT("Spacecraft"),        // SATNAMES,
   wxT("Tolerance"),         // TOLERANCE,
   wxT("Filename"),           // EVENT_FILENAME,
   wxT("IsActive")           // IS_ACTIVE
};

const Gmat::ParameterType
EventLocator::PARAMETER_TYPE[EventLocatorParamCount - GmatBaseParamCount] =
{
   Gmat::STRINGARRAY_TYPE,       // SATNAMES,
   Gmat::REAL_TYPE,              // TOLERANCE,
   Gmat::STRING_TYPE,            // EVENT_FILENAME,
   Gmat::BOOLEAN_TYPE            // IS_ACTIVE
};


//------------------------------------------------------------------------------
// Public Methods
//------------------------------------------------------------------------------


EventLocator::EventLocator(const wxString &typeStr,
      const wxString &nomme) :
   GmatBase       (Gmat::EVENT_LOCATOR, typeStr, nomme),
   filename       (wxT("LocatedEvents.txt")),
   efCount        (0),
   lastData       (NULL),
   isActive       (true),
   eventTolerance (1.0e-3),
   solarSys       (NULL)
{
   objectTypes.push_back(Gmat::EVENT_LOCATOR);
   objectTypeNames.push_back(wxT("EventLocator"));
}

EventLocator::~EventLocator()
{
   if (lastData != NULL)
      delete [] lastData;

   // todo: Delete the member EventFunctions
}

EventLocator::EventLocator(const EventLocator& el):
   GmatBase          (el),
   filename          (el.filename),
   efCount           (0),
   lastData          (NULL),
   isActive          (el.isActive),
   satNames          (el.satNames),
   targets           (el.targets),
   eventTolerance    (el.eventTolerance),
   solarSys          (el.solarSys)
{
}

EventLocator& EventLocator::operator=(const EventLocator& el)
{
   if (this != &el)
   {
      GmatBase::operator =(el);

      filename       = el.filename;
      efCount        = 0;
      lastData       = NULL;
      isActive       = el.isActive;
      satNames       = el.satNames;
      targets        = el.targets;
      eventTolerance = el.eventTolerance;
      solarSys       = el.solarSys;

      eventFunctions.clear();
      maxSpan.clear();
      lastSpan.clear();
   }

   return *this;
}


wxString EventLocator::GetParameterText(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < EventLocatorParamCount)
      return PARAMETER_TEXT[id - GmatBaseParamCount];
   return GmatBase::GetParameterText(id);
}

Integer EventLocator::GetParameterID(const wxString &str) const
{
   for (Integer i = GmatBaseParamCount; i < EventLocatorParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatBaseParamCount])
         return i;
   }

   return GmatBase::GetParameterID(str);
}

Gmat::ParameterType EventLocator::GetParameterType(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < EventLocatorParamCount)
      return PARAMETER_TYPE[id - GmatBaseParamCount];

   return GmatBase::GetParameterType(id);
}

wxString EventLocator::GetParameterTypeString(const Integer id) const
{
   return GmatBase::PARAM_TYPE_STRING[GetParameterType(id)];
}

bool EventLocator::IsParameterReadOnly(const Integer id) const
{
   if (id == IS_ACTIVE)
      return true;

   return GmatBase::IsParameterReadOnly(id);
}

bool EventLocator::IsParameterReadOnly(const wxString &label) const
{
   return IsParameterReadOnly(GetParameterID(label));
}

Real EventLocator::GetRealParameter(const Integer id) const
{
   if (id == TOLERANCE)
      return eventTolerance;

   return GmatBase::GetRealParameter(id);
}

Real EventLocator::SetRealParameter(const Integer id, const Real value)
{
   if (id == TOLERANCE)
   {
      if (value > 0.0)
         eventTolerance = value;
      else
         throw EventException(
               wxT("Located event tolerance value must be a real number >= 0.0"));

      return eventTolerance;
   }

   return GmatBase::SetRealParameter(id, value);
}

Real EventLocator::GetRealParameter(const Integer id, const Integer index) const
{
   return GmatBase::GetRealParameter(id, index);
}

Real EventLocator::GetRealParameter(const Integer id, const Integer row,
                                      const Integer col) const
{
   return GmatBase::GetRealParameter(id, row, col);
}

Real EventLocator::SetRealParameter(const Integer id, const Real value,
      const Integer index)
{
   return GmatBase::SetRealParameter(id, value, index);
}

Real EventLocator::SetRealParameter(const Integer id, const Real value,
                                      const Integer row, const Integer col)
{
   return GmatBase::SetRealParameter(id, value, row, col);
}

Real EventLocator::GetRealParameter(const wxString &label) const
{
   return GetRealParameter(GetParameterID(label));
}

Real EventLocator::SetRealParameter(const wxString &label,
                                      const Real value)
{
   return SetRealParameter(GetParameterID(label), value);
}

Real EventLocator::GetRealParameter(const wxString &label,
                                      const Integer index) const
{
   return GetRealParameter(GetParameterID(label), index);
}

Real EventLocator::SetRealParameter(const wxString &label,
                                      const Real value,
                                      const Integer index)
{
   return SetRealParameter(GetParameterID(label), value, index);
}

Real EventLocator::GetRealParameter(const wxString &label,
                                      const Integer row,
                                      const Integer col) const
{
   return GetRealParameter(GetParameterID(label), row, col);
}

Real EventLocator::SetRealParameter(const wxString &label,
                                      const Real value, const Integer row,
                                      const Integer col)
{
   return SetRealParameter(GetParameterID(label), value, row, col);
}

wxString EventLocator::GetStringParameter(const Integer id) const
{
   if (id == EVENT_FILENAME)
      return filename;

   return GmatBase::GetStringParameter(id);
}

bool EventLocator::SetStringParameter(const Integer id,
                                        const wxString &value)
{
   if (id == EVENT_FILENAME)
   {
      if (value != wxT(""))
      {
         filename = value;
         return true;
      }
      return false;
   }

   return GmatBase::SetStringParameter(id, value);
}

wxString EventLocator::GetStringParameter(const Integer id,
                                        const Integer index) const
{
   if (id == SATNAMES)
   {
      if (index < (Integer)satNames.size())
         return satNames[index];
      else
         throw EventException(
               wxT("Index out of range when trying to access spacecraft name for ") +
               instanceName);
   }

   return GmatBase::GetStringParameter(id, index);
}

bool EventLocator::SetStringParameter(const Integer id,
                                        const wxString &value,
                                        const Integer index)
{
   if (id == SATNAMES)
   {
      if (index < (Integer)satNames.size())
      {
         satNames[index] = value;
         return true;
      }
      else
      {
         satNames.push_back(value);
         targets.push_back(NULL);
         return true;
      }
   }

   return GmatBase::SetStringParameter(id, value, index);
}

const StringArray& EventLocator::GetStringArrayParameter(const Integer id) const
{
   if (id == SATNAMES)
      return satNames;

   return GmatBase::GetStringArrayParameter(id);
}

const StringArray& EventLocator::GetStringArrayParameter(const Integer id,
                                             const Integer index) const
{
   return GmatBase::GetStringArrayParameter(id, index);
}

wxString EventLocator::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}

bool EventLocator::SetStringParameter(const wxString &label,
                                        const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}

wxString EventLocator::GetStringParameter(const wxString &label,
                                        const Integer index) const
{
   return GetStringParameter(GetParameterID(label), index);
}

bool EventLocator::SetStringParameter(const wxString &label,
                                        const wxString &value,
                                        const Integer index)
{
   return SetStringParameter(GetParameterID(label), value, index);
}

const StringArray& EventLocator::GetStringArrayParameter(const wxString &label) const
{
   return GetStringArrayParameter(GetParameterID(label));
}

const StringArray& EventLocator::GetStringArrayParameter(const wxString &label,
                                             const Integer index) const
{
   return GetStringArrayParameter(GetParameterID(label), index);
}


bool EventLocator::GetBooleanParameter(const Integer id) const
{
   if (id == IS_ACTIVE)
      return isActive;

   return GmatBase::GetBooleanParameter(id);
}

bool EventLocator::SetBooleanParameter(const Integer id, const bool value)
{
   if (id == IS_ACTIVE)
   {
      isActive = value;
      return isActive;
   }
   return GmatBase::SetBooleanParameter(id, value);
}

bool EventLocator::GetBooleanParameter(const Integer id,
      const Integer index) const
{
   return GmatBase::GetBooleanParameter(id, index);
}

bool EventLocator::SetBooleanParameter(const Integer id, const bool value,
      const Integer index)
{
   return GmatBase::SetBooleanParameter(id, value, index);
}

bool EventLocator::GetBooleanParameter(const wxString &label) const
{
   return GetBooleanParameter(GetParameterID(label));
}

bool EventLocator::SetBooleanParameter(const wxString &label,
      const bool value)
{
   return SetBooleanParameter(GetParameterID(label), value);
}

bool EventLocator::GetBooleanParameter(const wxString &label,
      const Integer index) const
{
   return GetBooleanParameter(GetParameterID(label), index);
}

bool EventLocator::SetBooleanParameter(const wxString &label,
      const bool value, const Integer index)
{
   return SetBooleanParameter(GetParameterID(label), value, index);
}

void EventLocator::SetSolarSystem(SolarSystem *ss)
{
   solarSys = ss;
}


const StringArray& EventLocator::GetRefObjectNameArray(
      const Gmat::ObjectType type)
{
   refObjectNames.clear();
   if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::SPACEOBJECT)
   {
      // Get ref. objects for requesting type from the parent class
      GmatBase::GetRefObjectNameArray(type);

      // Add ref. objects for requesting type from this class
      refObjectNames.insert(refObjectNames.begin(), satNames.begin(),
            satNames.end());

      #ifdef DEBUG_EVENTLOCATOR_OBJECT
         MessageInterface::ShowMessage
            (wxT("EventLocator::GetRefObjectNameArray() this=<%p>'%s' returning %d ")
             wxT("ref. object names\n"), this, GetName().c_str(),
             refObjectNames.size());
         for (UnsignedInt i=0; i<refObjectNames.size(); i++)
            MessageInterface::ShowMessage(wxT("   '%s'\n"),
                  refObjectNames[i].c_str());
      #endif

      return refObjectNames;
   }

   return GmatBase::GetRefObjectNameArray(type);
}


bool EventLocator::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                              const wxString &name)
{
   for (UnsignedInt i = 0; i < satNames.size(); ++i)
   {
      if (satNames[i] == name)
      {
         if (obj->IsOfType(Gmat::SPACEOBJECT))
         {
            targets[i] = (SpaceObject*)obj;
            return true;
         }
         return false;
      }
   }
   return GmatBase::SetRefObject(obj, type, name);
}


bool EventLocator::Initialize()
{
   bool retval = false;

   StringArray badInits;
   efCount = eventFunctions.size();

   // Loop through the event functions, evaluating each and storing their data
   for (UnsignedInt i = 0; i < efCount; ++i)
   {
      if (eventFunctions[i]->Initialize() == false)
         badInits.push_back(eventFunctions[i]->GetName());
   }

   if (badInits.size() == 0)
      retval = true;
   else
   {
      wxString errorList;
      for (UnsignedInt i = 0; i < badInits.size(); ++i)
         errorList = errorList + wxT("   ") + badInits[i] + wxT("\n");
      throw EventException(wxT("These event functions failed to initialize:\n") +
            errorList);
   }

   if (lastData != NULL)
      delete [] lastData;
   if (efCount > 0)
      lastData = new Real[efCount * 3];

   return retval;
}


/// Evaluates the EventFunctions and returns their values and derivatives.
Real *EventLocator::Evaluate()
{
   Real *vals;

   #ifdef DEBUG_EVENTLOCATION
      MessageInterface::ShowMessage(wxT("Evaluating %d event functions; ")
            wxT("locator %s\n"), eventFunctions.size(), instanceName.c_str());
   #endif

   UnsignedInt i3;

   // Loop through the event functions, evaluating each and storing their data
   for (UnsignedInt i = 0; i < eventFunctions.size(); ++i)
   {
      i3 = i * 3;
      vals = eventFunctions[i]->Evaluate();

      #ifdef DEBUG_DUMPEVENTDATA
         dumpfile.precision(15);
         dumpfile << vals[0] << wxT(" ") << vals[1] << wxT(" ") << vals[2] << wxT(" ");
      #endif

      // Load the returned data into lastData
      lastData[  i3  ] = vals[0];
      lastData[i3 + 1] = vals[1];
      lastData[i3 + 2] = vals[2];
   }

   #ifdef DEBUG_DUMPEVENTDATA
      dumpfile << wxT("\n");
   #endif

   return lastData;
}

UnsignedInt EventLocator::GetFunctionCount()
{
   return eventFunctions.size();
}


void EventLocator::BufferEvent(Integer forEventFunction)
{
   // Build a LocatedEvent structure
   LocatedEvent *theEvent = new LocatedEvent;

   Real *theData = eventFunctions[forEventFunction]->GetData();
   theEvent->epoch = theData[0];
   theEvent->eventValue = theData[1];
   theEvent->type = eventFunctions[forEventFunction]->GetTypeName();
   theEvent->participants = eventFunctions[forEventFunction]->GetName();
   theEvent->boundary = eventFunctions[forEventFunction]->GetBoundaryType();
   theEvent->isEntry = eventFunctions[forEventFunction]->IsEventEntry();

   #ifdef DEBUG_EVENTLOCATION
      MessageInterface::ShowMessage(wxT("Adding event to event table:\n   ")
            wxT("%-20s%-30s%-15s%15.9lf\n"), theEvent->type.c_str(),
            theEvent->participants.c_str(), theEvent->boundary.c_str(),
            theEvent->epoch);
   #endif

   eventTable.AddEvent(theEvent);
}


/// Adds an event to the LocatedEventTable.
void EventLocator::BufferEvent(Real epoch, wxString type, bool isStart)
{
}

/// Writes the event data to file.
void EventLocator::ReportEventData()
{
   wxString fullFileName;

   if ((filename.find(wxT('/'), 0) == std::string::npos) &&
       (filename.find(wxT('\\'), 0) == std::string::npos))
   {
      FileManager *fm = FileManager::Instance();
      wxString outPath = fm->GetAbsPathname(FileManager::OUTPUT_PATH);

      // Check for terminating '/' and add if needed
      Integer len = outPath.length();
      if ((outPath[len-1] != wxT('/')) && (outPath[len-1] != wxT('\\')))
         outPath = outPath + wxT("/");

      fullFileName = outPath + filename;
   }
   else
      fullFileName = filename;

   eventTable.WriteToFile(fullFileName);
}

/// Writes the event data statistics to file.
void EventLocator::ReportEventStatistics()
{

}

/// Retrieves data for a specified event.
Real* EventLocator::GetEventData(wxString type, Integer whichOne)
{
   return lastData;
}

/// Updates the data in the event table, possibly sorting as well
void EventLocator::UpdateEventTable(SortStyle how)
{

}
