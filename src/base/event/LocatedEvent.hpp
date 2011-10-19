/*
 * LocatedEvent.hpp
 *
 *  Created on: Aug 17, 2011
 *      Author: djc
 */

#ifndef LOCATEDEVENT_HPP_
#define LOCATEDEVENT_HPP_

#include "gmatdefs.hpp"

class LocatedEvent
{
public:
   LocatedEvent();
   virtual ~LocatedEvent();
   LocatedEvent(const LocatedEvent& le);
   LocatedEvent& operator=(const LocatedEvent& le);

   /// The epoch of the data element.
   GmatEpoch epoch;
   /// Identifier for the type of datum represented
   wxString boundary;
   /// The type of the event.
   wxString type;
};

#endif /* LOCATEDEVENT_HPP_ */
