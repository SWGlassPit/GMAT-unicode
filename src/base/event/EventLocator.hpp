//$Id: EventLocator.hpp 10025 2011-11-29 23:31:31Z djcinsb $
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
 * Definition of the the event locator base class
 */
//------------------------------------------------------------------------------


#ifndef EventLocator_hpp
#define EventLocator_hpp


#include "GmatBase.hpp"
#include "LocatedEventTable.hpp"
#include "EventFunction.hpp"


/**
 * Base class for the event locators.
 *
 * EventLocators are container classes that collect together a set of event
 * functions defining a specific type of location process.  For example, the
 * EclipseLocator class collects together Penumbra, Umbra, and Antumbra event
 * functions.
 */
class GMAT_API EventLocator: public GmatBase
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

   virtual const Rvector&
                        GetRvectorParameter(const Integer id) const;
   virtual const Rvector&
                        SetRvectorParameter(const Integer id,
                                            const Rvector &value);
   virtual const Rvector&
                        GetRvectorParameter(const wxString &label) const;
   virtual const Rvector&
                        SetRvectorParameter(const wxString &label,
                                            const Rvector &value);

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

   virtual bool         GetBooleanParameter(const Integer id) const;
   virtual bool         SetBooleanParameter(const Integer id,
                                            const bool value);
   virtual bool         GetBooleanParameter(const Integer id,
                                            const Integer index) const;
   virtual bool         SetBooleanParameter(const Integer id,
                                            const bool value,
                                            const Integer index);
   virtual bool         GetBooleanParameter(const wxString &label) const;
   virtual bool         SetBooleanParameter(const wxString &label,
                                            const bool value);
   virtual bool         GetBooleanParameter(const wxString &label,
                                            const Integer index) const;
   virtual bool         SetBooleanParameter(const wxString &label,
                                            const bool value,
                                            const Integer index);

   virtual bool         TakeAction(const wxString &action,
                                   const wxString &actionData = wxT(""));
   virtual const ObjectTypeArray& GetTypesForList(const Integer id);
   virtual const ObjectTypeArray& GetTypesForList(const wxString &label);



   virtual void         SetSolarSystem(SolarSystem *ss);
//   virtual void         SetInternalCoordSystem(CoordinateSystem *cs);
//   virtual wxString  GetRefObjectName(const Gmat::ObjectType type) const;
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                              const wxString &name);

   virtual bool         Initialize();
   virtual Real         GetTolerance();

   virtual Real         *Evaluate(GmatEpoch atEpoch = -1.0,
                                  Real *forState = NULL);

   UnsignedInt GetFunctionCount();
   void BufferEvent(Integer forEventFunction = 0);
   void BufferEvent(Real epoch, wxString type, bool isStart);
   void ReportEventData();
   void ReportEventStatistics();
   Real *GetEventData(wxString type, Integer whichOne = 0);
   void UpdateEventTable(SortStyle how);
   virtual GmatEpoch GetLastEpoch(Integer index);

   // Methods used in integration
   virtual bool HasAssociatedStateObjects();
   virtual wxString GetAssociateName(UnsignedInt val = 0);
   wxString GetTarget(UnsignedInt forFunction);
   StringArray GetDefaultPropItems();
   Integer SetPropItem(const wxString &propItem);
   Integer GetPropItemSize(const Integer item);
   void SetStateIndices(UnsignedInt forFunction, Integer index,
         Integer associate);

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
   /// Epochs of the last events located
   GmatEpoch *lastEpochs;
   /// Flag used to turn the locator on or off (default is on)
   bool isActive;
   /// Flag used to show or hide plot of the data
   bool showPlot;

   /// Names of the "target" spacecraft in the location
   StringArray satNames;
   /// Pointers to the sats -- using SpaceObject so Formations can be supported
   std::vector<SpaceObject*> targets;
   /// Event location tolerance
   Real eventTolerance;
   /// The space environment
   SolarSystem *solarSys;
   /// Indices for values/derivatives in the state/derivative vectors
   std::vector<Integer> stateIndices;
   /// Start indices for the associated state data
   std::vector<Integer> associateIndices;
   /// Vector of event function values used in integration
   Rvector functionValues;

   /// Published parameters for event locators
    enum
    {
       SATNAMES = GmatBaseParamCount,
       TOLERANCE,
       EVENT_FILENAME,
       IS_ACTIVE,
       SHOW_PLOT,
       EPOCH,
       EVENT_FUNCTION,
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
