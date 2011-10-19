//$Id: CommandFactory.cpp 9603 2011-06-16 17:36:17Z djcinsb $
//------------------------------------------------------------------------------
//                            CommandFactory
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Wendy Shoan
// Created: 2003/10/09
//
/**
 *  Implementation code for the CommandFactory class, responsible for
 *  creating Command objects.
 */
//------------------------------------------------------------------------------

#include "gmatdefs.hpp"
#include "CommandFactory.hpp"
#include "GmatGlobal.hpp"     // for IsMatlabAvailable()
#include "NoOp.hpp"           // for NoOp command
#include "BeginMissionSequence.hpp" // for BeginMissionSequence command
#include "Toggle.hpp"         // for Toggle command
#include "Propagate.hpp"      // for Propagate command
#include "Maneuver.hpp"       // for Maneuver command
#include "Target.hpp"         // for Target command
#include "Vary.hpp"           // for Vary command
#include "Achieve.hpp"        // for Achieve command
#include "EndTarget.hpp"      // for EndTarget command
#include "For.hpp"            // for For command
#include "EndFor.hpp"         // for EndFor command
#include "If.hpp"             // for IF command
#include "Else.hpp"           // for Else command
#include "EndIf.hpp"          // for EndIf command
#include "While.hpp"          // for FOR command
#include "EndWhile.hpp"       // for EndFor command
#include "Assignment.hpp"     // for Assignment (GMAT) command  
#include "Report.hpp"         // for Report command
#include "Save.hpp"           // for Save command  
#include "SaveMission.hpp"    // for SaveMission command  
#include "Stop.hpp"           // for Save command  
#include "CallGmatFunction.hpp"   // for CallGmatFunction command
#include "BeginFiniteBurn.hpp"// for BeginFiniteBurn command
#include "EndFiniteBurn.hpp"  // for EndFiniteBurn command
#include "BeginScript.hpp"    // for BeginScript command
#include "EndScript.hpp"      // for EndScript command
#include "Optimize.hpp"       // for Optimize command
#include "EndOptimize.hpp"    // for EndOptimize command
#include "Minimize.hpp"       // for Minimize command
#include "NonlinearConstraint.hpp" // for NonlinearConstraint command

#include "ClearPlot.hpp"      // for ClearPlot command
#include "PenUp.hpp"          // for PenUp command
#include "PenDown.hpp"        // for PenDown command
#include "MarkPoint.hpp"      // for MarkPoint command
#include "Global.hpp"         // for Global command
#include "Create.hpp"         // for Create command

//******************************************************************************
// ElseIf does not work yet. (2008.08.29)
// The workaround is to use nested If-Else statements.
// The work that needs to be done concerns the conditions, IIRC - WCS
#ifdef __INCLUDE_ELSEIF__
#include "ElseIf.hpp"         // for ElseIf command
#endif
//******************************************************************************

//---------------------------------
//  public methods
//---------------------------------

//------------------------------------------------------------------------------
//  GmatCommand* CreateCommand(const wxString &ofType, 
//                             const wxString &withName)
//------------------------------------------------------------------------------
/**
 * This method creates and returns an object of the requested command class 
 *
 * @param <ofType>   type of command object to create and return.
 * @param <withName> name of the command (currently not used).
 *
 * @return command object
 *
 * @note As of 2003/10/14, we are ignoring the withname parameter.  Use of this
 *       parameter may be added later.
 */
//------------------------------------------------------------------------------
GmatCommand* CommandFactory::CreateCommand(const wxString &ofType,
                                           const wxString &withName)
{
    if (ofType == wxT("NoOp"))
        return new NoOp;
    if (ofType == wxT("BeginMissionSequence"))
        return new BeginMissionSequence;
    else if (ofType == wxT("Propagate"))
        return new Propagate;
    else if (ofType == wxT("Toggle"))
        return new Toggle;
    else if (ofType == wxT("Maneuver"))
        return new Maneuver;
    else if (ofType == wxT("Target"))
        return new Target;
    else if (ofType == wxT("Vary"))
        return new Vary;
    else if (ofType == wxT("Achieve"))
        return new Achieve;
    else if (ofType == wxT("EndTarget"))
        return new EndTarget;
    else if (ofType == wxT("For"))
        return new For;
    else if (ofType == wxT("EndFor"))
        return new EndFor;
    else if (ofType == wxT("While"))
        return new While;
    else if (ofType == wxT("EndWhile"))
        return new EndWhile;
    else if (ofType == wxT("If"))
        return new If;
    else if (ofType == wxT("Else"))
        return new Else;
#ifdef __INCLUDE_ELSEIF__
    else if (ofType == wxT("ElseIf"))
        return new ElseIf;
#endif
    else if (ofType == wxT("EndIf"))
        return new EndIf;
    else if (ofType == wxT("GMAT") || ofType == wxT("Equation") || ofType == wxT("Assignment"))
        return new Assignment;
    else if (ofType == wxT("Report"))
        return new Report;
    else if (ofType == wxT("Save"))
        return new Save;
    else if (ofType == wxT("SaveMission"))
        return new SaveMission;
    // Actual creating of CallFunction is not allowed, but it should
    // be added to allowed creatables so that Interpreter can continue
    // with creating proper CallGmatFunction
    //else if (ofType == wxT("CallFunction"))
    //   return new CallFunction;
    else if (ofType == wxT("CallGmatFunction"))
        return new CallGmatFunction;
    else if (ofType == wxT("BeginFiniteBurn"))
        return new BeginFiniteBurn;
    else if (ofType == wxT("EndFiniteBurn"))
         return new EndFiniteBurn;
    else if (ofType == wxT("BeginScript"))
        return new BeginScript;
    else if (ofType == wxT("EndScript"))
         return new EndScript;
    else if (ofType == wxT("Stop"))
        return new Stop;
    else if (ofType == wxT("Optimize"))
        return new Optimize;
    else if (ofType == wxT("EndOptimize"))
        return new EndOptimize;
    else if (ofType == wxT("Minimize"))
        return new Minimize;
    else if (ofType == wxT("NonlinearConstraint"))
        return new NonlinearConstraint;
    else if (ofType == wxT("ClearPlot"))
        return new ClearPlot;
    else if (ofType == wxT("PenUp"))
        return new PenUp;
    else if (ofType == wxT("PenDown"))
        return new PenDown;
    else if (ofType == wxT("MarkPoint"))
        return new MarkPoint;
    else if (ofType == wxT("Global"))
        return new Global;
    else if (ofType == wxT("Create"))
        return new Create;
   // add more here .......
   else 
   {
      return NULL;   // doesn't match any known type of command
   }
   
}


//------------------------------------------------------------------------------
//  CommandFactory()
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class CommandFactory.
 * (default constructor)
 *
 */
//------------------------------------------------------------------------------
CommandFactory::CommandFactory() :
    Factory(Gmat::COMMAND)
{
   if (creatables.empty())
   {
      creatables.push_back(wxT("Achieve"));
      creatables.push_back(wxT("Assignment"));
      creatables.push_back(wxT("BeginFiniteBurn"));
      creatables.push_back(wxT("BeginMissionSequence"));
      sequenceStarters.push_back(wxT("BeginMissionSequence"));
      creatables.push_back(wxT("BeginScript"));
      creatables.push_back(wxT("CallFunction"));
      creatables.push_back(wxT("CallGmatFunction"));
      creatables.push_back(wxT("ClearPlot"));
      creatables.push_back(wxT("Create"));
      creatables.push_back(wxT("Else"));
#ifdef __INCLUDE_ELSEIF__
      creatables.push_back(wxT("ElseIf"));
#endif
      creatables.push_back(wxT("EndFor"));
      creatables.push_back(wxT("EndIf"));
      creatables.push_back(wxT("EndOptimize"));
      creatables.push_back(wxT("EndTarget"));
      creatables.push_back(wxT("EndWhile"));
      creatables.push_back(wxT("EndScript"));
      creatables.push_back(wxT("EndFiniteBurn"));
      creatables.push_back(wxT("Equation"));
      creatables.push_back(wxT("For"));
      creatables.push_back(wxT("If"));
      creatables.push_back(wxT("GMAT"));
      creatables.push_back(wxT("Global"));
      creatables.push_back(wxT("Maneuver"));
      creatables.push_back(wxT("MarkPoint"));
      creatables.push_back(wxT("Minimize"));
      creatables.push_back(wxT("NonlinearConstraint"));
      creatables.push_back(wxT("NoOp"));
      creatables.push_back(wxT("Optimize"));
      creatables.push_back(wxT("PenUp"));
      creatables.push_back(wxT("PenDown"));
      creatables.push_back(wxT("Propagate"));
      creatables.push_back(wxT("Report"));
      creatables.push_back(wxT("Save"));
      creatables.push_back(wxT("SaveMission"));
      creatables.push_back(wxT("ScriptEvent"));
      creatables.push_back(wxT("Stop"));
      creatables.push_back(wxT("Target"));
      creatables.push_back(wxT("Toggle"));
      creatables.push_back(wxT("Vary"));
      creatables.push_back(wxT("While"));
   }
   
   // Now fill in unviewable commands
   // We don't want to show these commands in the MissionTree menu
   if (unviewables.empty())
   {
      // These commands do nothing
      unviewables.push_back(wxT("NoOp"));
      unviewables.push_back(wxT("BeginMissionSequence"));
      
      // These commands show as Equation in the MissionTree menu
      unviewables.push_back(wxT("Assignment"));
      unviewables.push_back(wxT("GMAT"));
      
      // These commands show as ScriptEvent in the MissionTree menu
      unviewables.push_back(wxT("BeginScript"));
      
      // These commands only works in object setup mode and inside a GmatFunction
      unviewables.push_back(wxT("Create"));
      
      // CallFunction is parent command of CallGmatFunction and CallMatlabFunction
      unviewables.push_back(wxT("CallFunction"));
      
      // These commands are only viewable under Target or Optimize
      unviewables.push_back(wxT("Achieve"));
      unviewables.push_back(wxT("Minimize"));
      unviewables.push_back(wxT("NonlinearConstraint"));
      unviewables.push_back(wxT("Vary"));
      
      // These commands are automatically created via GUI
      unviewables.push_back(wxT("For"));
      unviewables.push_back(wxT("If"));
      unviewables.push_back(wxT("Else"));
      unviewables.push_back(wxT("ElseIf"));
      unviewables.push_back(wxT("While"));
      unviewables.push_back(wxT("EndFor"));
      unviewables.push_back(wxT("EndIf"));
      unviewables.push_back(wxT("EndOptimize"));
      unviewables.push_back(wxT("EndTarget"));
      unviewables.push_back(wxT("EndWhile"));
      unviewables.push_back(wxT("EndScript"));
   }
}

//------------------------------------------------------------------------------
//  CommandFactory(StringArray createList)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class CommandFactory.
 *
 * @param <createList> list of creatable command objects
 *
 */
//------------------------------------------------------------------------------
CommandFactory::CommandFactory(StringArray createList) :
    Factory(createList,Gmat::COMMAND)
{
}

//------------------------------------------------------------------------------
//  CommandFactory(const CommandFactory& fact)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the (base) class CommandFactory (called by
 * copy constructors of derived classes).  (copy constructor)
 *
 * @param <fact> the factory object to copy to wxT("this") factory.
 */
//------------------------------------------------------------------------------
CommandFactory::CommandFactory(const CommandFactory& fact) :
    Factory(fact)
{
}

//------------------------------------------------------------------------------
//  CommandFactory& operator= (const CommandFactory& fact)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the CommandFactory base class.
 *
 * @param <fact> the CommandFactory object whose data to assign to wxT("this") factory.
 *
 * @return wxT("this") CommandFactory with data of input factory fact.
 */
//------------------------------------------------------------------------------
CommandFactory& CommandFactory::operator= (const CommandFactory& fact)
{
   Factory::operator=(fact);
   return *this;
}

//------------------------------------------------------------------------------
// ~CommandFactory()
//------------------------------------------------------------------------------
/**
 * Destructor for the CommandFactory base class.
 */
//------------------------------------------------------------------------------
CommandFactory::~CommandFactory()
{
   // deletes handled by Factory destructor
}


//------------------------------------------------------------------------------
// StringArray GetListOfCreatableObjects(const wxString &qualifier) const
//------------------------------------------------------------------------------
/**
 * Retrieves a list of creatable objects
 *
 * Override for the base class method so that sequence starter commands can be
 * identified
 *
 * @param qualifier The subtype for the command, if any
 *
 * @return The list of commands, qualified if requested
 */
//------------------------------------------------------------------------------
StringArray CommandFactory::GetListOfCreatableObjects(
                                  const wxString &qualifier) const
{
   if (qualifier != wxT(""))
   {
      if (qualifier == wxT("SequenceStarters"))
         return sequenceStarters;
   }

   return Factory::GetListOfCreatableObjects(qualifier);
}
