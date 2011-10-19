//$Id: Achieve.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                 Achieve
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
// Created: 2004/01/26
//
/**
 * Definition for the Achieve command class
 */
//------------------------------------------------------------------------------


#include "Achieve.hpp"
#include "StringUtil.hpp"  // for ToReal()
#include <sstream>
#include "MessageInterface.hpp"

//#define DEBUG_ACHIEVE_PARSE
//#define DEBUG_ACHIEVE_PARAMS
//#define DEBUG_ACHIEVE_INIT
//#define DEBUG_ACHIEVE_EXEC
//#define DEBUG_WRAPPER_CODE


//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//---------------------------------
// static data
//---------------------------------
const wxString
   Achieve::PARAMETER_TEXT[AchieveParamCount - GmatCommandParamCount] =
   {
      wxT("TargeterName"),
      wxT("Goal"),
      wxT("GoalValue"),
      wxT("Tolerance")
   };
   
const Gmat::ParameterType
   Achieve::PARAMETER_TYPE[AchieveParamCount - GmatCommandParamCount] =
   {
      Gmat::STRING_TYPE,
      Gmat::STRING_TYPE,
      Gmat::STRING_TYPE,
      Gmat::STRING_TYPE,
   };


//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Achieve()
//------------------------------------------------------------------------------
/**
 * Creates an Achieve command.  (default constructor)
 */
//------------------------------------------------------------------------------
Achieve::Achieve() :
   GmatCommand             (wxT("Achieve")),
   targeterName            (wxT("")),
   goalName                (wxT("")),
   goal                    (NULL),
   achieveName             (wxT("")),
   achieve                 (NULL),
   toleranceName           (wxT("0.1")),
   tolerance               (NULL),
   goalId                  (-1),
   targeter                (NULL),
   targeterDataFinalized   (false)
{
   settables.push_back(wxT("Tolerance")); 
   parameterCount = AchieveParamCount;
}


//------------------------------------------------------------------------------
//  ~Achieve()
//------------------------------------------------------------------------------
/**
 * Destroys the Achieve command.  (destructor)
 */
//------------------------------------------------------------------------------
Achieve::~Achieve()
{
   ClearWrappers();
}

    
//------------------------------------------------------------------------------
//  Achieve(const Achieve& t)
//------------------------------------------------------------------------------
/**
 * Constructor that replicates a Achieve command.  (Copy constructor)
 *
 * @param t Command that is replicated here.
 */
//------------------------------------------------------------------------------
Achieve::Achieve(const Achieve& t) :
   GmatCommand             (t),
   targeterName            (t.targeterName),
   goalName                (t.goalName),
   goal                    (NULL),
   achieveName             (t.achieveName),
   achieve                 (NULL),
   toleranceName           (t.toleranceName),
   tolerance               (NULL),
   goalId                  (t.goalId),
   targeter                (NULL),
   targeterDataFinalized   (false)//,
{
   parameterCount = AchieveParamCount;
}


//------------------------------------------------------------------------------
//  Achieve& operator=(const Achieve& t)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the Achieve command.
 *
 * @param t Command that is replicated here.
 *
 * @return A reference to this instance.
 */
//------------------------------------------------------------------------------
Achieve& Achieve::operator=(const Achieve& t)
{
   if (this == &t)
      return *this;
    
   GmatCommand::operator=(t);
   targeterName          = t.targeterName;
   goalName              = t.goalName;
   goal                  = NULL;
   achieveName           = t.achieveName;
   achieve               = NULL;
   toleranceName         = t.toleranceName;
   tolerance             = NULL;
   goalId                = t.goalId;
   targeter              = NULL;
   targeterDataFinalized = false;

   return *this;
}



//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the Achieve.
 *
 * @return clone of the Achieve.
 */
//------------------------------------------------------------------------------
GmatBase* Achieve::Clone() const
{
   return (new Achieve(*this));
}


//------------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//------------------------------------------------------------------------------
/**
 * Renames referenced objects.
 *
 * @param type Type of the object that is renamed.
 * @param oldName The current name for the object.
 * @param newName The name the object has when this operation is complete.
 *
 * @return true on success.
 */
//------------------------------------------------------------------------------
bool Achieve::RenameRefObject(const Gmat::ObjectType type,
                              const wxString &oldName,
                              const wxString &newName)
{
   if (type == Gmat::SOLVER)
   {
      if (targeterName == oldName)
         targeterName = newName;
   }
   // make sure the wrappers know to rename any objects they may be using
   if (goal)
   {
      goal->RenameObject(oldName, newName);
      goalName = goal->GetDescription();
   }
   if (achieve)
   {
      achieve->RenameObject(oldName, newName);
      achieveName = achieve->GetDescription();
   }
   if (tolerance)
   {
      tolerance->RenameObject(oldName, newName);
      toleranceName = tolerance->GetDescription();
   }
   
   return true;
}


//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by the Achieve.
 *
 * @return the list of object types.
 * 
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& Achieve::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::SOLVER);
   return refObjectTypes;
}


//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref objects used by the Achieve.
 *
 * @param <type> The type of object desired, or Gmat::UNKNOWN_OBJECT for the
 *               full list.
 * 
 * @return the list of object names.
 * 
 */
//------------------------------------------------------------------------------
const StringArray& Achieve::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   refObjectNames.clear();
   
   if (type == Gmat::UNKNOWN_OBJECT ||
       type == Gmat::SOLVER)
   {
      refObjectNames.push_back(targeterName);
   }
     
   return refObjectNames;
}


// Parameter accessors

//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter text, given the input parameter ID.
 *
 * @param id Id for the requested parameter text.
 *
 * @return parameter text for the requested parameter.
 */
//------------------------------------------------------------------------------
wxString Achieve::GetParameterText(const Integer id) const
{
   if (id >= GmatCommandParamCount && id < AchieveParamCount)
      return PARAMETER_TEXT[id - GmatCommandParamCount];
   return GmatCommand::GetParameterText(id);
}


//------------------------------------------------------------------------------
//  Integer  GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter ID, given the input parameter string.
 *
 * @param str string for the requested parameter.
 *
 * @return ID for the requested parameter.
 */
//------------------------------------------------------------------------------
Integer Achieve::GetParameterID(const wxString &str) const
{
   for (Integer i = GmatCommandParamCount; i < AchieveParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatCommandParamCount])
         return i;
   }

   return GmatCommand::GetParameterID(str);
}


//------------------------------------------------------------------------------
//  Gmat::ParameterType  GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type, given the input parameter ID.
 *
 * @param id ID for the requested parameter.
 *
 * @return parameter type of the requested parameter.
 */
//------------------------------------------------------------------------------
Gmat::ParameterType Achieve::GetParameterType(const Integer id) const
{
   if (id >= GmatCommandParamCount && id < AchieveParamCount)
      return PARAMETER_TYPE[id - GmatCommandParamCount];

   return GmatCommand::GetParameterType(id);
}


//------------------------------------------------------------------------------
//  wxString  GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type string, given the input parameter ID.
 *
 * @param id ID for the requested parameter.
 *
 * @return parameter type string of the requested parameter.
 */
//------------------------------------------------------------------------------
wxString Achieve::GetParameterTypeString(const Integer id) const
{
   return GmatCommand::PARAM_TYPE_STRING[GetParameterType(id)];
}


//------------------------------------------------------------------------------
//  Real  GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the Real parameter value, given the input parameter ID.
 *
 * @param id ID for the requested parameter value.
 *
 * @return Real value of the requested parameter.
 */
//------------------------------------------------------------------------------
Real Achieve::GetRealParameter(const Integer id) const
{
   if (id == goalNameID)
      if (goal) return goal->EvaluateReal();
   if (id == goalValueID)
      if (achieve) return achieve->EvaluateReal();
   if (id == toleranceID)
      if (tolerance) return tolerance->EvaluateReal();
    
   return GmatCommand::GetRealParameter(id);
}


//------------------------------------------------------------------------------
//  Real  SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * This method sets the Real parameter value, given the input parameter ID.
 *
 * @param id    ID for the parameter whose value to change.
 * @param value value for the parameter.
 *
 * @return Real value of the requested parameter.
 */
//------------------------------------------------------------------------------
Real Achieve::SetRealParameter(const Integer id, const Real value)
{
   return GmatCommand::SetRealParameter(id, value);
}


//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter ID.
 *
 * @param id ID for the requested parameter.
 *
 * @return  string value of the requested parameter.
 */
//------------------------------------------------------------------------------
wxString Achieve::GetStringParameter(const Integer id) const
{
   if (id == targeterNameID)
      return targeterName;
        
   if (id == goalNameID)
      return goalName;
        
   if (id == goalValueID) 
      return achieveName;

   if (id == toleranceID) 
      return toleranceName;

   return GmatCommand::GetStringParameter(id);
}


//------------------------------------------------------------------------------
//  wxString  SetStringParameter(const Integer id, const wxString value)
//------------------------------------------------------------------------------
/**
 * This method sets the string parameter value, given the input
 * parameter ID.
 *
 * @param id    ID for the requested parameter.
 * @param value string value for the requested parameter.
 *
 * @return  success flag.
 */
//------------------------------------------------------------------------------
bool Achieve::SetStringParameter(const Integer id, const wxString &value)
{
   #ifdef DEBUG_ACHIEVE_PARAMS
   MessageInterface::ShowMessage
      (wxT("Achieve::SetStringParameter() id=%d, value=%s\n"), id, value.c_str());
   #endif
   
   if (id == targeterNameID) 
   {
      targeterName = value;
      return true;
   }

   if (id == goalNameID) 
   {
      goalName = value;
      if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), value) == 
          wrapperObjectNames.end())
         wrapperObjectNames.push_back(value);
      return true;
   }
   
   if (id == goalValueID) 
   {
      achieveName = value;
      
      if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), value) == 
          wrapperObjectNames.end())
         wrapperObjectNames.push_back(value);
      return true;
   }
   
   if (id == toleranceID) 
   {
      toleranceName = value;
      if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), value) == 
          wrapperObjectNames.end())
         wrapperObjectNames.push_back(value);
      
      // Do range check here if value is a real number
      Real tol;
      if (GmatStringUtil::ToReal(value, tol))
         SetTolerance(tol);
      
      return true;
   }

   return GmatCommand::SetStringParameter(id, value);
}


// Multiple variables on the same line are not allowed in the current build.
// const StringArray& Achieve::GetStringArrayParameter(const Integer id) const; 


//------------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                                     const wxString &name = "")
//------------------------------------------------------------------------------
/**
 * Sets referenced objects.
 *
 * @param obj reference object pointer.
 * @param type type of the reference object.
 * @param name name of the reference object.
 *
 * @return success of the operation.
 */
//------------------------------------------------------------------------------
bool Achieve::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                           const wxString &name)
{   
   if (type == Gmat::SOLVER) 
   {
      if (targeterName == obj->GetName()) 
      {
         targeter = (Solver*)obj;
         return true;
      }
      return false;
   }

   return GmatCommand::SetRefObject(obj, type, name);
}


//------------------------------------------------------------------------------
//  bool InterpretAction()
//------------------------------------------------------------------------------
/**
 * Parses the command string and builds the corresponding command structures.
 *
 * The Achieve command has the following syntax:
 *
 *     Achieve myDC(Sat1.SMA = 21545.0, {Tolerance = 0.1});
 *     Achieve myDC(Sat1.SMA = Var1, {Tolerance = 0.1});
 *     Achieve myDC(Sat1.SMA = Arr1(1,1), {Tolerance = 0.1});
 *     Achieve myDC(Sat1.SMA = Arr1(I,J), {Tolerance = 0.1});
 *
 * where myDC is a Solver used to Achieve a set of variables to achieve the
 * corresponding goals.  This method breaks the script line into the 
 * corresponding pieces, and stores the name of the Solver so it can be set to
 * point to the correct object during initialization.
 */
//------------------------------------------------------------------------------
bool Achieve::InterpretAction()
{
   // Clean out any old data
   wrapperObjectNames.clear();
   ClearWrappers();

   StringArray chunks = InterpretPreface();

   #ifdef DEBUG_ACHIEVE_PARSE
      MessageInterface::ShowMessage(wxT("Preface chunks as\n"));
      for (StringArray::iterator i = chunks.begin(); i != chunks.end(); ++i)
         MessageInterface::ShowMessage(wxT("   \"%s\"\n"), i->c_str());
      MessageInterface::ShowMessage(wxT("\n"));
   #endif
      
   if (chunks.size() <= 1)
      throw CommandException(wxT("Missing information for Achieve command.\n"));
   
   if (chunks[1].at(0) == '(')
      throw CommandException(wxT("Missing solver name for Achieve command.\n"));
      
   if ((chunks[1].find(wxT("[")) != chunks[1].npos) || (chunks[1].find(wxT("]")) != chunks[1].npos))
      throw CommandException(wxT("Brackets not allowed in Achieve command"));

   if (!GmatStringUtil::AreAllBracketsBalanced(chunks[1], wxT("({)}")))
   {
      throw CommandException
         (wxT("Parentheses, braces, or brackets are unbalanced or incorrectly placed\n"));
   }
   
   // Find and set the solver object name
   // This is the only setting in Achieve that is not in a wrapper
   StringArray currentChunks = parser.Decompose(chunks[1], wxT("()"), false);
   SetStringParameter(targeterNameID, currentChunks[0]);
   #ifdef DEBUG_ACHIEVE_PARSE
      MessageInterface::ShowMessage(wxT("current chunks as\n"));
      for (StringArray::iterator i = currentChunks.begin(); i != currentChunks.end(); ++i)
         MessageInterface::ShowMessage(wxT("   \"%s\"\n"), i->c_str());
      MessageInterface::ShowMessage(wxT("\n"));
   #endif
   
   wxString noSpaces2     = GmatStringUtil::RemoveAll(currentChunks[1],' ');
   #ifdef DEBUG_ACHIEVE_PARSE
      MessageInterface::ShowMessage(
         wxT("Achieve: noSpaces2 = %s\n"), noSpaces2.c_str());
   #endif   
   currentChunks = parser.Decompose(noSpaces2, wxT("()"), true, true);
   
   #ifdef DEBUG_ACHIEVE_PARSE
      MessageInterface::ShowMessage(
         "Achieve: after Decompose, current chunks = \n");
      for (Integer jj = 0; jj < (Integer) currentChunks.size(); jj++)
         MessageInterface::ShowMessage("   %s\n",
                                       currentChunks[jj].c_str());
   #endif

   // First chunk is the goal and achieve (target) value
   wxString lhs, rhs;
   if (!SeparateEquals(currentChunks[0], lhs, rhs, true))
   {
      throw CommandException(wxT("The goal \"") + lhs + 
         wxT("\" is missing the \"=\" operator or a goal value required for an ") + typeName + 
         wxT(" command.\n"));
   }
      
   goalName = lhs;
   achieveName = rhs;
   
   #ifdef DEBUG_ACHIEVE_PARSE
      MessageInterface::ShowMessage(
         wxT("Achieve:  setting goalName to %s\n"), goalName.c_str());
      MessageInterface::ShowMessage(
         wxT("Achieve:  setting achieveName to %s\n"), achieveName.c_str());
   #endif
   
   // if there are no more chunks, just return
   if (currentChunks.size() == (Integer) 1) return true;
   
   wxString noSpaces     = GmatStringUtil::RemoveAll(currentChunks[1],' ');
   // Now deal with the settable parameters
   currentChunks = parser.SeparateBrackets(noSpaces, wxT("{}"), wxT(","), true);
   
   #ifdef DEBUG_ACHIEVE_PARSE
      MessageInterface::ShowMessage(
         wxT("Achieve: After SeparateBrackets, current chunks = \n"));
      for (Integer jj = 0; jj < (Integer) currentChunks.size(); jj++)
         MessageInterface::ShowMessage(wxT("   %s\n"),
                                       currentChunks[jj].c_str());
   #endif
   
   // currentChunks now holds all of the pieces - no need for more separation  
   
   for (StringArray::iterator i = currentChunks.begin(); 
        i != currentChunks.end(); ++i)
   {
      bool isOK = SeparateEquals(*i, lhs, rhs, true);
      #ifdef DEBUG_ACHIEVE_PARSE
         MessageInterface::ShowMessage(wxT("Setting Achieve properties\n"));
         MessageInterface::ShowMessage(wxT("   \"%s\" = \"%s\"\n"), lhs.c_str(), rhs.c_str());
      #endif
      
      if (!isOK || lhs.empty() || rhs.empty())
         throw CommandException(wxT("The setting \"") + lhs + 
            wxT("\" is missing the \"=\" operator or a value required for an ") + typeName + 
            wxT(" command.\n"));
      
      if (IsSettable(lhs))
         SetStringParameter(GetParameterID(lhs), rhs);
      else
         throw CommandException(wxT("The setting \"") + lhs + 
            wxT("\" is not a valid setting for an ") + typeName + 
            wxT(" command.\n"));
   }
   
   return true;
}


//------------------------------------------------------------------------------
// const StringArray& GetWrapperObjectNameArray()
//------------------------------------------------------------------------------
const StringArray& Achieve::GetWrapperObjectNameArray()
{
   wrapperObjectNames.clear();

   wrapperObjectNames.push_back(goalName);
   
   if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), 
       achieveName) == wrapperObjectNames.end())
      wrapperObjectNames.push_back(achieveName);

   if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), 
       toleranceName) == wrapperObjectNames.end())
      wrapperObjectNames.push_back(toleranceName);
   
   return wrapperObjectNames;
}


//------------------------------------------------------------------------------
// bool SetElementWrapper(ElementWrapper *toWrapper, const wxString &withName)
//------------------------------------------------------------------------------
bool Achieve::SetElementWrapper(ElementWrapper *toWrapper, 
              const wxString &withName)
{
   bool retval = false;

   if (toWrapper == NULL) return false;
   
   if (toWrapper->GetWrapperType() == Gmat::ARRAY_WT)
   {
      throw CommandException(wxT("A value of type \"Array\" on command \"") + 
                  typeName + 
                  wxT("\" is not an allowed value.\nThe allowed values are:")
                  wxT(" [ Real Number, Variable, Array Element, or Parameter ]. ")); 
   }
   
   CheckDataType(toWrapper, Gmat::REAL_TYPE, wxT("Achieve"), true);

   #ifdef DEBUG_WRAPPER_CODE   
   MessageInterface::ShowMessage(
               wxT("   Setting wrapper \"%s\" on Achieve command\n"), 
      withName.c_str());
   #endif

   if (goalName == withName)
   {
      if (toWrapper->GetWrapperType() == Gmat::NUMBER_WT)
      {
         wxString errmsg = wxT("The value of \"") + goalName;
         errmsg            += wxT("\" for field \"Goal\" on object \"");
         errmsg            += instanceName + wxT("\" is not an allowed value.\n");
         errmsg            += wxT("The allowed values are: ");
         errmsg            += wxT("[ Object Property, Array Element, Variable, ");
         errmsg            += wxT("or Parameter, excluding numbers].");
         throw CommandException(errmsg);
      }
      goal = toWrapper;
      #ifdef DEBUG_WRAPPER_CODE   
      MessageInterface::ShowMessage
         (wxT("   goal set to wrapper <%p>'%s'\n"), toWrapper, withName.c_str());
      #endif
      retval = true;
   }
   
   if (achieveName == withName)
   {
      if (achieve != NULL)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (achieve, achieve->GetDescription(),
             wxT("Achieve::SetElementWrapper()"),
             GetGeneratingString(Gmat::NO_COMMENTS) +
             wxT(" deleting achieve ew"));
         #endif
         delete achieve;
      }
      achieve = toWrapper;
      #ifdef DEBUG_WRAPPER_CODE   
      MessageInterface::ShowMessage
         (wxT("   achieve set to wrapper <%p>'%s'\n"), toWrapper, withName.c_str());
      #endif
      retval = true;
   }
   
   if (toleranceName == withName)
   {
      if (tolerance != NULL)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (tolerance, tolerance->GetDescription(),
             wxT("Achieve::SetElementWrapper()"),
             GetGeneratingString(Gmat::NO_COMMENTS) +
             wxT(" deleting tolerance ew"));
         #endif
         delete tolerance;
      }
      tolerance = toWrapper;
      #ifdef DEBUG_WRAPPER_CODE   
      MessageInterface::ShowMessage
         (wxT("   tolerance set to wrapper <%p>'%s'\n"), toWrapper, withName.c_str());
      #endif
      retval = true;
   }
   
   return retval;
}


//------------------------------------------------------------------------------
// void ClearWrappers()
//------------------------------------------------------------------------------
void Achieve::ClearWrappers()
{
   std::vector<ElementWrapper*> temp;
   if (goal)
   {
      temp.push_back(goal);
      goal = NULL;
   }
   if (achieve)
   {
      if (find(temp.begin(), temp.end(), achieve) == temp.end())
      {
         temp.push_back(achieve);
         achieve = NULL;
      }
   }
   if (tolerance)
   {
      if (find(temp.begin(), temp.end(), tolerance) == temp.end())
      {
         temp.push_back(tolerance);
         tolerance = NULL;
      }
   }
   
   ElementWrapper *wrapper;
   for (UnsignedInt i = 0; i < temp.size(); ++i)
   {
      wrapper = temp[i];
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (wrapper, wrapper->GetDescription(), wxT("Achieve::ClearWrappers()"),
          GetGeneratingString(Gmat::NO_COMMENTS) + wxT(" deleting wrapper"));
      #endif
      delete wrapper;
   }
}


//------------------------------------------------------------------------------
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * Performs the initialization needed to run the Achieve command.
 *
 * @return true if the GmatCommand is initialized, false if an error occurs.
 */
//------------------------------------------------------------------------------
bool Achieve::Initialize()
{
   #ifdef DEBUG_ACHIEVE_INIT
   MessageInterface::ShowMessage
      (wxT("Achieve::Initialize() entered, targeter=<%p>\n"), targeter);
   #endif
   
   bool retval = GmatCommand::Initialize();

   if (targeter == NULL)
      throw CommandException(
         wxT("Targeter not initialized for Achieve command\n  \"")
         + generatingString + wxT("\"\n"));
   
   Integer id = targeter->GetParameterID(wxT("Goals"));
   targeter->SetStringParameter(id, goalName);
   
   // Set references for the wrappers   
   #ifdef DEBUG_ACHIEVE_INIT
      MessageInterface::ShowMessage(wxT("Setting refs for goal\n"));
   #endif
   if (SetWrapperReferences(*goal) == false)
      return false;
   CheckDataType(goal, Gmat::REAL_TYPE, wxT("Achieve"));
   #ifdef DEBUG_ACHIEVE_INIT
      MessageInterface::ShowMessage(wxT("Setting refs for achieve\n"));
   #endif
   if (SetWrapperReferences(*achieve) == false)
      return false;
   CheckDataType(achieve, Gmat::REAL_TYPE, wxT("Achieve"));
   #ifdef DEBUG_ACHIEVE_INIT
      MessageInterface::ShowMessage(wxT("Setting refs for tolerance\n"));
   #endif
   if (SetWrapperReferences(*tolerance) == false)
      return false;
   CheckDataType(tolerance, Gmat::REAL_TYPE, wxT("Achieve"));
   
   // The targeter cannot be finalized until all of the loop is initialized
   targeterDataFinalized = false;
   
   #ifdef DEBUG_ACHIEVE_INIT
   MessageInterface::ShowMessage
      (wxT("Achieve::Initialize() exiting. targeter=<%p>\n"), targeter);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
//  bool Execute()
//------------------------------------------------------------------------------
/**
 * Achieve the variables defined for this targeting loop.
 *
 * This method (will eventually) feeds data to the targeter state machine in 
 * order to determine the variable values needed to achieve the user specified
 * goals.
 *
 * @return true if the GmatCommand runs to completion, false if an error
 *         occurs.
 */
//------------------------------------------------------------------------------
bool Achieve::Execute()
{
   #ifdef DEBUG_ACHIEVE_EXEC
   MessageInterface::ShowMessage
      (wxT("Achieve::Execute() targeterDataFinalized=%d\n   targeter=<%p>'%s'\n"),
       targeterDataFinalized, targeter, targeter->GetName().c_str());
   MessageInterface::ShowMessage
      (wxT("   goalName=%s, achieveName=%s\n"), goalName.c_str(), achieveName.c_str());
   #endif

   if (goal == NULL || achieve == NULL || tolerance == NULL)
      throw CommandException(wxT("NULL element wrappers found in Achieve command\n"));
   
   bool retval = true;
   if (!targeterDataFinalized) 
   {
      // Tell the targeter about the goals and tolerances
      Real goalData[2];
      goalData[0] = goal->EvaluateReal();
      goalData[1] = tolerance->EvaluateReal();
      goalId = targeter->SetSolverResults(goalData, goalName);
      targeterDataFinalized = true;
      
      #ifdef DEBUG_ACHIEVE_EXEC
      MessageInterface::ShowMessage
         (wxT("   Set goal data '%s' [%f, %f] to targeter<%p>'%s'\n"), goalName.c_str(),
          goalData[0], goalData[1], targeter, targeter->GetName().c_str());
      #endif
      
      return retval;
   }
   
   Real val = -999.999;
   
   // Evaluate the floating target (if there is one) and set it on the targeter
   val = achieve->EvaluateReal();
   #ifdef DEBUG_ACHIEVE_EXEC
   MessageInterface::ShowMessage
      (wxT("   Setting achieve = %f to targeter<%p>\n"), val, targeter);
   #endif
   targeter->UpdateSolverGoal(goalId, val);
   
   // Evaluate goal and pass it to the targeter
   val = goal->EvaluateReal();
   #ifdef DEBUG_ACHIEVE_EXEC
   MessageInterface::ShowMessage
      (wxT("   Setting goal = %f to targeter<%p>\n"), val, targeter);
   #endif
   targeter->SetResultValue(goalId, val);
   
   // Evaluate tolerance pass it to the targeter
   val = tolerance->EvaluateReal();
   #ifdef DEBUG_ACHIEVE_EXEC
   MessageInterface::ShowMessage
      (wxT("   Setting tolerance = %f to targeter<%p>\n"), val, targeter);
   #endif
   targeter->UpdateSolverTolerance(goalId, val);
   
   BuildCommandSummary(true);
   
   return retval;
}


//------------------------------------------------------------------------------
//  const wxString& GetGeneratingString()
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
 * @param mode    Specifies the type of serialization requested.  (Not yet used
 *                in commands)
 * @param prefix  Optional prefix appended to the object's name.  (Not yet used
 *                in commands)
 * @param useName Name that replaces the object's name.  (Not yet used in
 *                commands)
 *
 * @return The script line that, when interpreted, defines this Achieve command.
 */
//------------------------------------------------------------------------------
const wxString& Achieve::GetGeneratingString(Gmat::WriteMode mode,
                                            const wxString &prefix,
                                            const wxString &useName)
{
   // Build the local string
   wxString gen = prefix + wxT("Achieve ") + targeterName + wxT("(") + goalName +
                     wxT(" = ") + achieveName + wxT(", {Tolerance = ") + toleranceName;
   
   generatingString = gen + wxT("});");
   
   // Then call the base class method for preface and inline comments
   return GmatCommand::GetGeneratingString(mode, prefix, useName);
}


//------------------------------------------------------------------------------
// void RunComplete()
//------------------------------------------------------------------------------
void Achieve::RunComplete()
{
   #ifdef DEBUG_ACHIEVE_EXEC
      MessageInterface::ShowMessage(
      wxT("In Achieve::RunComplete, targeterDataFinalized = %s, ... now setting it to false\n"),
      (targeterDataFinalized? wxT("true") : wxT("false")));
   #endif
   targeterDataFinalized = false;
   GmatCommand::RunComplete();
}


//------------------------------------------------------------------------------
// void SetTolerance(Real value)
//------------------------------------------------------------------------------
void Achieve::SetTolerance(Real value)
{
   #ifdef DEBUG_ACHIEVE_PARAM
   MessageInterface::ShowMessage
      (wxT("Achieve::SetTolerance() entered, value=%f, tolerance=<%p>\n"), value,
       tolerance);
   #endif
   
   if (value > 0.0)
   {
      if (tolerance)
      {
         tolerance->SetReal(value);
         #ifdef DEBUG_ACHIEVE_PARAM
         MessageInterface::ShowMessage
            (wxT("   value=%f set to tolerance<%p>\n"), value, tolerance);
         #endif
      }
   }
   else
   {
      CommandException ce;
      ce.SetDetails(errorMessageFormat.c_str(),
                    GmatStringUtil::ToString(value, GetDataPrecision()).c_str(),
                    wxT("Tolerance"),
                    wxT("Real Number, Array element, Variable, or Parameter > 0.0"));
      throw ce;
   }
   
   #ifdef DEBUG_ACHIEVE_PARAM
   MessageInterface::ShowMessage(wxT("Achieve::SetTolerance() leaving\n"));
   #endif
}

