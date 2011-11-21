//$Id: MarkPoint.cpp 9850 2011-09-09 18:48:32Z lindajun $
//------------------------------------------------------------------------------
//                                MarkPoint
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Darrel J. Conway
// Created: 2009/10/06
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
/**
 * Class implementation for the MarkPoint command
 */
//------------------------------------------------------------------------------


#include "MarkPoint.hpp"
#include "XyPlot.hpp"
#include "MessageInterface.hpp"
#include "StringUtil.hpp"

//#define DEBUG_MarkPoint


//------------------------------------------------------------------------------
// MarkPoint()
//------------------------------------------------------------------------------
/**
 * Constructor
 */
//------------------------------------------------------------------------------
MarkPoint::MarkPoint() :
   PlotCommand(wxT("MarkPoint"))
{
}


//------------------------------------------------------------------------------
// ~MarkPoint()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
MarkPoint::~MarkPoint()
{
}


//------------------------------------------------------------------------------
// MarkPoint(const MarkPoint &c)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 * 
 * @param <c> The instance that gets copied.
 */
//------------------------------------------------------------------------------
MarkPoint::MarkPoint(const MarkPoint &c) :
   PlotCommand    (c)
{
}


//------------------------------------------------------------------------------
// MarkPoint& operator=(const MarkPoint &c)
//------------------------------------------------------------------------------
/**
 * Assignment operator
 * 
 * @param <c> The command that gets copied.
 * 
 * @return A reference to this instance.
 */
//------------------------------------------------------------------------------
MarkPoint& MarkPoint::operator=(const MarkPoint &c)
{
   if (&c != this)
   {
      PlotCommand::operator=(c);
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Override of the GmatBase clone method.
 * 
 * @return A new copy of this instance.
 */
//------------------------------------------------------------------------------
GmatBase* MarkPoint::Clone() const
{
   return new MarkPoint(*this);
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 * Method that initializes the internal data structures.
 * 
 * @return true if initialization succeeds.
 */
//------------------------------------------------------------------------------
bool MarkPoint::Initialize()
{
   #ifdef DEBUG_MarkPoint
      MessageInterface::ShowMessage(wxT("MarkPoint::Initialize() entered\n"));
   #endif
      
   PlotCommand::Initialize();
   
   GmatBase *xy;
   thePlotList.clear();
   
   for (unsigned int ii = 0; ii < plotNameList.size(); ii++)
   {
      if ((xy = FindObject(plotNameList.at(ii))) != NULL) 
      {
         if (xy->GetTypeName() == wxT("XYPlot")) 
            thePlotList.push_back((XyPlot*) xy);
         else
            throw CommandException(
               wxT("Object named \"") + plotNameList.at(ii) + wxT("\" should be an XYPlot to use the ")
               wxT("MarkPoint command for this object, but it is a ") +
               xy->GetTypeName());      
      }
      else 
      {
         MessageInterface::ShowMessage
            (wxT("MarkPoint command cannot find XY Plot \"%s\"; command has no effect.")
            wxT("\n"), (plotNameList.at(ii)).c_str());
         return false;
      }
   }
   
   #ifdef DEBUG_MarkPoint
      MessageInterface::ShowMessage
         (wxT("   thePlotList.size()=%d\n"), thePlotList.size());
      MessageInterface::ShowMessage(wxT("MarkPoint::Initialize() returning true\n"));
   #endif
   return true;
}


//---------------------------------------------------------------------------
//  bool PlotCommand::Execute()
//---------------------------------------------------------------------------
/**
 * The method that is fired to perform the PlotCommand.
 *
 * Derived classes implement this method to perform their actions on
 * GMAT objects.
 *
 * @return true if the GmatCommand runs to completion, false if an error
 *         occurs.
 */
//---------------------------------------------------------------------------
bool MarkPoint::Execute()
{
   #ifdef DEBUG_MarkPoint
      MessageInterface::ShowMessage
         (wxT("MarkPoint::Execute() thePlotList.size()=%d\n"), thePlotList.size());
   #endif
      
   for (unsigned int ii = 0; ii < thePlotList.size(); ii++)
   {
      if (thePlotList.at(ii))
         if (!(thePlotList.at(ii)->TakeAction(wxT("MarkPoint")))) return false;
   }
   
   #ifdef DEBUG_MarkPoint
      MessageInterface::ShowMessage(wxT("MarkPoint::Execute() returning true\n"));
   #endif
   return true;
}
