//$Id: EndOptimize.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                EndOptimize 
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
// Author:  Daniel Hunter/GSFC/MAB (CoOp)
// Created: 2006.07.20
//
/**
 * Implementation for the EndOptimize command class
 */
//------------------------------------------------------------------------------

#include "EndOptimize.hpp"
#include "BranchCommand.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_OPTIMIZER_COMMANDS

//------------------------------------------------------------------------------
// static data
//------------------------------------------------------------------------------

//VC++ error C2466: cannot allocate an array of constant size 0
// so commented out for possible later use
//const wxString
//EndOptimize::PARAMETER_TEXT[EndOptimizeParamCount - GmatCommandParamCount] = {};
//const Gmat::ParameterType
//EndOptimize::PARAMETER_TYPE[EndOptimizeParamCount - GmatCommandParamCount] = {};

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
EndOptimize::EndOptimize() :
   GmatCommand         (wxT("EndOptimize"))
{
   objectTypeNames.push_back(wxT("BranchEnd"));
   depthChange = -1;
   parameterCount = EndOptimizeParamCount;
}


//------------------------------------------------------------------------------
// copy constructor
//------------------------------------------------------------------------------
EndOptimize::EndOptimize(const EndOptimize& eo) :
   GmatCommand         (eo)
{
}

//------------------------------------------------------------------------------
// operator =
//------------------------------------------------------------------------------
EndOptimize& EndOptimize::operator=(const EndOptimize& eo)
{
   if (this == &eo)
      return *this;
    
   return *this;
}
    

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
EndOptimize::~EndOptimize()
{
   // nothing to do here at the moment ...... la de da de da
}
    
//------------------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------------------
bool EndOptimize::Initialize()
{
   GmatCommand::Initialize();
   
   // Validate that next points to the owning Optimize command
   if (!next)
      throw CommandException(wxT("EndOptimize Command not properly reconnected"));
    
   if (next->GetTypeName() != wxT("Optimize"))
      throw CommandException(wxT("EndOptimize Command not connected to Optimize Command"));
    
   return true;
}

//------------------------------------------------------------------------------
// Execute
//------------------------------------------------------------------------------
bool EndOptimize::Execute()
{
   #ifdef DEBUG_OPTIMIZER_COMMANDS
      if (next)
         MessageInterface::ShowMessage(
            wxT("End Optimize points to a %s command\n"), next->GetTypeName().c_str());
      else
         MessageInterface::ShowMessage(
            wxT("EndOptimize does not reconnect to Optimize comamnd\n"));
   #endif
   
   BuildCommandSummary(true);
   return true;
}

//------------------------------------------------------------------------------
// Insert
//------------------------------------------------------------------------------
bool EndOptimize::Insert(GmatCommand *cmd, GmatCommand *prev)
{
   // if inserting after End statement for branch command, we want to 
   // insert right after the entire Optimize command
   if (this == prev)
      return ((BranchCommand*)next)->InsertRightAfter(cmd);
   return false;
}

//------------------------------------------------------------------------------
// Clone
//------------------------------------------------------------------------------
GmatBase* EndOptimize::Clone() const
{
   return (new EndOptimize(*this));
}

//------------------------------------------------------------------------------
// GetGeneratingString
//------------------------------------------------------------------------------
const wxString& EndOptimize::GetGeneratingString(Gmat::WriteMode mode,
                                                    const wxString &prefix,
                                                    const wxString &useName)
{
   generatingString = prefix + wxT("EndOptimize;");
   if (mode == Gmat::NO_COMMENTS)
      return generatingString;
   
   if ((next) && (next->GetTypeName() == wxT("Optimize")))
   {
      if (showInlineComment)
      {
         // To avoid keep appending, check for empty inline comment
         if (GetInlineComment() == wxT(""))
         {
            generatingString += wxT("  % For optimizer ");
            generatingString += next->GetRefObjectName(Gmat::SOLVER);
         }
      }
   }
   return GmatCommand::GetGeneratingString(mode, prefix, useName);
}

//------------------------------------------------------------------------------
// protected methods
//------------------------------------------------------------------------------
// none at this time

