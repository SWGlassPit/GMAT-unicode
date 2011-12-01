//$Id: LocatedEventTable.hpp 9933 2011-09-30 20:49:24Z djcinsb $
//------------------------------------------------------------------------------
//                           LocatedEventTable
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
// Created: Sep 2, 2011
//
/**
 * Definition of the ...
 */
//------------------------------------------------------------------------------


#ifndef LocatedEventTable_hpp
#define LocatedEventTable_hpp

#include "LocatedEvent.hpp"


class OwnedPlot;


enum SortStyle
{
   CHRONOLOGICAL = 12000,  // Sorts the event data in time order.
   BY_TYPE,                // Groups the data by event type.
   DURATION_ASCENDING,     // Groups the event data from shortest to longest.
   DURATION_DESCENDING,    // Groups the event data from longest to shortest.
   BY_START,               // Sorts by start epoch
   UNSORTED                // Uses the current buffer ordering
};

/**
 * The table of events located during a run
 */
class GMAT_API LocatedEventTable
{
public:
   LocatedEventTable();
   virtual ~LocatedEventTable();
   LocatedEventTable(const LocatedEventTable& let);
   LocatedEventTable& operator=(const LocatedEventTable& let);

   void AddEvent(LocatedEvent *theEvent);
   void AddEvent(GmatEpoch epoch, wxString boundaryType, wxString eventType);
   Real GetMaxSpan(wxString eventType, wxString parties);
   Real GetLastSpan(wxString eventType, wxString parties = wxT(""));
   Real GetAverageSpan(wxString eventType, wxString parties = wxT(""));
   void SortEvents(SortStyle how, SortStyle secondaryStyle);
   std::vector<LocatedEvent*> *GetEvents();

   bool WriteToFile(wxString filename);
   void ShowPlot();
   void BuildPlot(const wxString &plotName);

protected:
   /// The table of located event boundaries
   std::vector<LocatedEvent*>    events;
   /// Main sort style
   SortStyle primarySortStyle;
   /// Secondary sort style
   SortStyle secondarySortStyle;
   /// The report order for the events
   std::vector<UnsignedInt> sortOrder;
   /// Flag indicating stale associations
   bool associationsCurrent;
   /// List of the types of events
   StringArray eventTypesWithNames;
   /// Plot of the event data
   OwnedPlot *thePlot;

   void BuildAssociations();
   void SortEvents();
   std::string BuildEventSummary();
};

#endif /* LocatedEventTable_hpp */
