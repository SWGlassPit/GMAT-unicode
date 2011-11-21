//$Id: EventLocator.hpp 9853 2011-09-09 20:08:55Z djcinsb $
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


#ifndef EventLocator_hpp
#define EventLocator_hpp


#include "GmatBase.hpp"
#include "LocatedEventTable.hpp"
#include "EventFunction.hpp"


class EventLocator: public GmatBase
{
public:
   EventLocator(const wxString &typeStr, const wxString &nomme = wxT(""));
   virtual ~EventLocator();
   EventLocator(const EventLocator& el);
   EventLocator& operator=(const EventLocator& el);

   // Inherited (GmatBase) methods for parameters
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;

   virtual bool         IsParameterReadOnly(const Integer id) const;
   virtual bool         IsParameterReadOnly(const wxString &label) const;

   virtual Real         GetRealParameter(const Integer id) const;
   virtual Real         SetRealParameter(const Integer id,
                                         const Real value);
   virtual Real         GetRealParameter(const Integer id,
                                         const Integer index) const;
   virtual Real         GetRealParameter(const Integer id, const Integer row,
                                         const Integer col) const;
   virtual Real         SetRealParameter(const Integer id,
                                         const Real value,
                                         const Integer index);
   virtual Real         SetRealParameter(const Integer id, const Real value,
                                         const Integer row, const Integer col);
   virtual Real         GetRealParameter(const wxString &label) const;
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value);
   virtual Real         GetRealParameter(const wxString &label,
                                         const Integer index) const;
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value,
                                         const Integer index);
   virtual Real         GetRealParameter(const wxString &label,
                                         const Integer row,
                                         const Integer col) const;
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value, const Integer row,
                                         const Integer col);

   virtual wxString  GetStringParameter(const Integer id) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value);
   virtual wxString  GetStringParameter(const Integer id,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value,
                                           const Integer index);
   virtual const StringArray&
                        GetStringArrayParameter(const Integer id) const;
   virtual const StringArray&
                        GetStringArrayParameter(const Integer id,
                                                const Integer index) const;
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value);
   virtual wxString  GetStringParameter(const wxString &label,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value,
                                           const Integer index);
   virtual const StringArray&
                        GetStringArrayParameter(const wxString &label) const;
   virtual const StringArray&
                        GetStringArrayParameter(const wxString &label,
                                                const Integer index) const;

   virtual void         SetSolarSystem(SolarSystem *ss);
//   virtual void         SetInternalCoordSystem(CoordinateSystem *cs);
//   virtual wxString  GetRefObjectName(const Gmat::ObjectType type) const;
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                              const wxString &name);

   virtual bool         Initialize();

   /// Evaluates contained EventFunctions and returns values and derivatives.
   virtual Real *Evaluate();
   /// Adds an event to the LocatedEventTable.
   void BufferEvent(Real epoch, wxString type, bool isStart);
   /// Writes the event data to file.
   void ReportEventData();
   /// Writes the event data statistics to file.
   void ReportEventStatistics();
   /// Retrieves data for a specified event.
   Real *GetEventData(wxString type, Integer whichOne = 0);
   /// Updates the data in the event table, possibly sorting as well
   void UpdateEventTable(SortStyle how);


protected:
   /// The collection of event functions used by the EventLocator.
   std::vector<EventFunction*> eventFunctions;
   /// The longest event duration encountered by the EventLocator.
   std::vector<Real> maxSpan;
   /// The most recent event duration encountered by the EventLocator.
   std::vector<Real>lastSpan;
   /// The LocatedEventTable for the EventLocator.
   LocatedEventTable eventTable;
   wxString filename;
   /// The number of event functions to be processed
   UnsignedInt efCount;
   /// The last data set computed
   Real *lastData;

   /// Names of the "target" spacecraft in the location
   StringArray satNames;
   /// Pointers to the sats -- using SpaceObject so Formations can be supported
   std::vector<SpaceObject*> targets;
   /// Event location tolerance
   Real eventTolerance;
   /// The space environment
   SolarSystem *solarSys;

   /// Published parameters for event locators
    enum
    {
       SATNAMES = GmatBaseParamCount,
       TOLERANCE,
       EVENT_FILENAME,
       EventLocatorParamCount
    };

    /// burn parameter labels
    static const wxString
       PARAMETER_TEXT[EventLocatorParamCount - GmatBaseParamCount];
    /// burn parameter types
    static const Gmat::ParameterType
       PARAMETER_TYPE[EventLocatorParamCount - GmatBaseParamCount];
};

#endif /* EventLocator_hpp */
