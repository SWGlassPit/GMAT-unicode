//$Id: PenUp.cpp 9850 2011-09-09 18:48:32Z lindajun $
//------------------------------------------------------------------------------
//                                 PenUp
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Darrel J. Conway
// Created: 2004/02/26
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
/**
 * Class implementation for the PenUp command
 */
//------------------------------------------------------------------------------


#include "PenUp.hpp"
#include "MessageInterface.hpp"
#include "StringUtil.hpp"

//#define DEBUG_PENUP


//------------------------------------------------------------------------------
// PenUp()
//------------------------------------------------------------------------------
/**
 * Constructor
 */
//------------------------------------------------------------------------------
PenUp::PenUp() :
   PlotCommand    (wxT("PenUp"))
{
}


//------------------------------------------------------------------------------
// ~PenUp()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
PenUp::~PenUp()
{
}


//------------------------------------------------------------------------------
// PenUp(const PenUp &c)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 * 
 * @param <c> The instance that gets copied.
 */
//------------------------------------------------------------------------------
PenUp::PenUp(const PenUp &c) :
   PlotCommand    (c)
{
}


//------------------------------------------------------------------------------
// PenUp& operator=(const PenUp &c)
//------------------------------------------------------------------------------
/**
 * Assignment operator
 * 
 * @param <c> The command that gets copied.
 * 
 * @return A reference to this instance.
 */
//------------------------------------------------------------------------------
PenUp& PenUp::operator=(const PenUp &c)
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
GmatBase* PenUp::Clone() const
{
   return new PenUp(*this);
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
bool PenUp::Initialize()
{
   #ifdef DEBUG_PENUP
      MessageInterface::ShowMessage(wxT("PenUp::Initialize() entered\n"));
   #endif
      
   PlotCommand::Initialize();
   
   GmatBase *sub;
   thePlotList.clear();
   
   for (unsigned int ii = 0; ii < plotNameList.size(); ii++)
   {
      if ((sub = FindObject(plotNameList.at(ii))) != NULL) 
      {
         if (sub->GetTypeName() == wxT("XYPlot") ||
             sub->GetTypeName() == wxT("OrbitView") ||
             sub->GetTypeName() == wxT("GroundTrackPlot"))
            thePlotList.push_back((Subscriber*) sub);
         else
            throw CommandException(
               wxT("Object named \"") + plotNameList.at(ii) +
               wxT("\" should be an XYPlot, OrbitView or GroundTrackPlot to use the ")
               wxT("PenUp command for this object, but it is a ") + 
               sub->GetTypeName());      
      }
      else 
      {
         MessageInterface::ShowMessage
            (wxT("PenUp command cannot find Plot \"%s\"; command has no effect.")
            wxT("\n"), (plotNameList.at(ii)).c_str());
         return false;
      }
   }

   return true;
}


//---------------------------------------------------------------------------
//  bool Execute()
//---------------------------------------------------------------------------
/**
 * The method that is fired to perform the PlotCommand.
 *
 * Derived classes implement this method to perform their actions on
 * GMAT objects.
 *
 * @return true if the PlotCommand runs to completion, false if an error
 *         occurs.
 */
//---------------------------------------------------------------------------
bool PenUp::Execute()
{
   for (unsigned int ii = 0; ii < thePlotList.size(); ii++)
   {
      if (thePlotList.at(ii))
         if (!(thePlotList.at(ii)->TakeAction(wxT("PenUp")))) return false;
   }
   return true;
}
