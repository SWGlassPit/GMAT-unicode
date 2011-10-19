//$Id: EndTarget.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                EndTarget
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
// Created: 2004/01/23
//
/**
 * Definition for the closing line of a Targeter loop
 */
//------------------------------------------------------------------------------


#include "EndTarget.hpp"
#include "BranchCommand.hpp"
#include "MessageInterface.hpp"


//------------------------------------------------------------------------------
//  EndTarget()
//------------------------------------------------------------------------------
/**
 * Creates an EndTarget command.  (default constructor)
 */
//------------------------------------------------------------------------------
EndTarget::EndTarget() :
   GmatCommand         (wxT("EndTarget"))
{
   objectTypeNames.push_back(wxT("BranchEnd"));
   depthChange = -1;
}


//------------------------------------------------------------------------------
//  ~EndTarget()
//------------------------------------------------------------------------------
/**
 * Destroys an EndTarget command.  (destructor)
 */
//------------------------------------------------------------------------------
EndTarget::~EndTarget()
{
}
    

//------------------------------------------------------------------------------
//  EndTarget(const EndTarget& et)
//------------------------------------------------------------------------------
/**
 * Creates an EndTarget command.  (copy constructor)
 *
 * @param et The command that is copied.
 */
//------------------------------------------------------------------------------
EndTarget::EndTarget(const EndTarget& et) :
   GmatCommand         (et)
{
}


//------------------------------------------------------------------------------
//  EndTarget& operator=(const EndTarget& et)
//------------------------------------------------------------------------------
/**
 * Creates an EndTarget command.  (copy constructor)
 *
 * @param et The command that is copied.
 *
 * @return This instance, configured like the input instance.
 */
//------------------------------------------------------------------------------
EndTarget& EndTarget::operator=(const EndTarget& et)
{
   if (this == &et)
      return *this;
    
   return *this;
}
    

//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 * Sets up the EndTarget command.
 *
 * @return true on success.
 */
//------------------------------------------------------------------------------
bool EndTarget::Initialize()
{
   GmatCommand::Initialize();
   
   // Validate that next points to the owning Target command
   if (!next)
      throw CommandException(wxT("EndTarget Command not properly reconnected"));
    
   if (next->GetTypeName() != wxT("Target"))
      throw CommandException(wxT("EndTarget Command not connected to Target Command"));
    
   return true;
}


//------------------------------------------------------------------------------
// bool Execute()
//------------------------------------------------------------------------------
/**
 * Runs the EndTarget command.
 *
 * The EndTarget command is basically a no-op command; it just marks the end of
 * the targeting loop.
 *
 * @return true always.
 */
//------------------------------------------------------------------------------
bool EndTarget::Execute()
{
   #ifdef DEBUG_TARGET_COMMANDS
      if (next)
         MessageInterface::ShowMessage(
            wxT("End Target points to a %s command\n"), next->GetTypeName().c_str());
      else
         MessageInterface::ShowMessage(
            wxT("EndTarget does not reconnect to Target comamnd\n"));
   #endif
   
   BuildCommandSummary(true);
   return true;
}


//------------------------------------------------------------------------------
// bool Insert(GmatCommand *cmd, GmatCommand *prev)
//------------------------------------------------------------------------------
/**
 * Inserts a command into the mission sequence.
 *
 * @param cmd  The command that gets inserted.
 * @param prev The command that will precede the inserted command.
 *
 * @return true.
 */
//------------------------------------------------------------------------------
bool EndTarget::Insert(GmatCommand *cmd, GmatCommand *prev)
{
   // if inserting after End statement for branch command, we want to 
   // insert right after the entire If command
   if (this == prev)
      return ((BranchCommand*)next)->InsertRightAfter(cmd);
   return false;
}


//---------------------------------------------------------------------------
// bool RenameRefObject(const Gmat::ObjectType type,
//                      const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
/*
 * Renames referenced objects
 *
 * @param <type> type of the reference object.
 * @param <oldName> old name of the reference object.
 * @param <newName> new name of the reference object.
 *
 * @return always true to indicate RenameRefObject() was implemented.
 */
//---------------------------------------------------------------------------
bool EndTarget::RenameRefObject(const Gmat::ObjectType type,
                                const wxString &oldName,
                                const wxString &newName)
{
   // There are no renamealbe objects
   return true;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the EndTarget.
 *
 * @return clone of the EndTarget.
 *
 */
//------------------------------------------------------------------------------
GmatBase* EndTarget::Clone() const
{
   return (new EndTarget(*this));
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
const wxString& EndTarget::GetGeneratingString(Gmat::WriteMode mode,
                                                  const wxString &prefix,
                                                  const wxString &useName)
{
   if (mode == Gmat::NO_COMMENTS)
   {
      generatingString = wxT("EndTarget;");
      return generatingString;
   }
   
   // Build the local string
   generatingString = prefix + wxT("EndTarget;");
   if ((next) && (next->GetTypeName() == wxT("Target")))
   {
      // To avoid keep appending, check for empty inline comment
      if (GetInlineComment() == wxT(""))
      {
         generatingString += wxT("  % For targeter ");
         generatingString += next->GetRefObjectName(Gmat::SOLVER);
      }
   }
   
   // Then call the base class method for preface and inline comments
   // We want preface comment to be indented
   return GmatCommand::GetGeneratingString(mode, prefix + wxT("   "), useName);
}
