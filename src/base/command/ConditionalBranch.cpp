//$Id: ConditionalBranch.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                ConditionalBranch
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
// Author:  Wendy Shoan/GSFC
// Created: 2004/09/27
//
/**
 * Definition for the ConditionalBranch command class
 */
//------------------------------------------------------------------------------

#include <sstream>
#include <ctype.h>                // for isalpha
#include "gmatdefs.hpp"
#include "ConditionalBranch.hpp"
#include "Parameter.hpp"
#include "StringUtil.hpp"         // for GetArrayIndex()
#include "MessageInterface.hpp"

//#define DEBUG_CONDITIONS 1
//#define DEBUG_CONDITIONS_INIT 1
//#define DEBUG_CONDBR_GET_GEN_STRING
//#define DEBUG_CONDBR_GET_PARAM
//#define DEBUG_WRAPPER_CODE
//#define DEBUG_OBJECT_MAP

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
ConditionalBranch::PARAMETER_TEXT[ConditionalBranchParamCount - BranchCommandParamCount] =
{
   wxT("NumberOfConditions"),
   wxT("NumberOfLogicalOperators"),
   wxT("LeftHandStrings"),
   wxT("OperatorStrings"),
   wxT("RightHandStrings"),
   wxT("LogicalOperators"),
};

const Gmat::ParameterType
ConditionalBranch::PARAMETER_TYPE[ConditionalBranchParamCount - BranchCommandParamCount] =
{
   Gmat::INTEGER_TYPE,     
   Gmat::INTEGER_TYPE, 
   Gmat::STRINGARRAY_TYPE,
   Gmat::STRINGARRAY_TYPE,
   Gmat::STRINGARRAY_TYPE,
   Gmat::STRINGARRAY_TYPE,
};

const wxString
ConditionalBranch::OPTYPE_TEXT[NumberOfOperators] =
{
   wxT("=="),
   wxT("~="),
   wxT(">"),
   wxT("<"),
   wxT(">="),
   wxT("<=")
};

const wxString
ConditionalBranch::LOGICAL_OPTYPE_TEXT[NumberOfLogicalOperators] =
{
   wxT("&"),
   wxT("|")
};

//------------------------------------------------------------------------------
//  ConditionalBranch(const wxString &typeStr)
//------------------------------------------------------------------------------
/**
 * Creates a ConditionalBranch command.  (constructor)
 */
//------------------------------------------------------------------------------
ConditionalBranch::ConditionalBranch(const wxString &typeStr) :
BranchCommand      (typeStr),
numberOfConditions (0),
numberOfLogicalOps (0)
{
   objectTypeNames.push_back(wxT("ConditionalBranch"));
   // nothing to add to settables here
}


//------------------------------------------------------------------------------
//  ConditionalBranch(const ConditionalBranch &cb)
//------------------------------------------------------------------------------
/**
 * Constructor that replicates a ConditionalBranch command.  (Copy constructor)
 */
//------------------------------------------------------------------------------
ConditionalBranch::ConditionalBranch(const ConditionalBranch &cb) :
BranchCommand      (cb),
numberOfConditions (cb.numberOfConditions),
numberOfLogicalOps (cb.numberOfLogicalOps)
{   
   Integer i = 0;
   for (i=0; i < numberOfConditions; i++)
   {
      lhsList.push_back((cb.lhsList).at(i));
      opStrings.push_back((cb.opStrings).at(i));
      opList.push_back((cb.opList).at(i));
      rhsList.push_back((cb.rhsList).at(i));
   }
   
   for (i=0; i < numberOfLogicalOps; i++)
   {
      logicalOpStrings.push_back((cb.logicalOpStrings).at(i));
      logicalOpList.push_back((cb.logicalOpList).at(i));
   }
   
   initialized = false;
}


//------------------------------------------------------------------------------
//  ConditionalBranch& operator=(const ConditionalBranch &cb)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the ConditionalBranch command.
 *
 * @return A reference to this instance.
 */
//------------------------------------------------------------------------------
ConditionalBranch& ConditionalBranch::operator=(const ConditionalBranch &cb)
{
   if (this == &cb)
      return *this;
   
   BranchCommand::operator=(cb);
   numberOfConditions = cb.numberOfConditions;
   numberOfLogicalOps = cb.numberOfLogicalOps;
   lhsList.clear();
   opList.clear();
   rhsList.clear();
   ClearWrappers();
   Integer i = 0;
   
   for (i=0; i < numberOfConditions; i++)
   {
      lhsList.push_back((cb.lhsList).at(i));
      opStrings.push_back((cb.opStrings).at(i));
      opList.push_back((cb.opList).at(i));
      rhsList.push_back((cb.rhsList).at(i));
   }
   
   logicalOpList.clear();
   for (i=0; i < numberOfLogicalOps; i++)
   {
      logicalOpStrings.push_back((cb.logicalOpStrings).at(i));
      logicalOpList.push_back((cb.logicalOpList).at(i));
   }
   
   initialized = false;
   return *this;
}


//------------------------------------------------------------------------------
//  ~ConditionalBranch()
//------------------------------------------------------------------------------
/**
 * Destroys the ConditionalBranch command.  (destructor)
 */
//------------------------------------------------------------------------------
ConditionalBranch::~ConditionalBranch()
{
   ClearWrappers();
}


//------------------------------------------------------------------------------
//  bool SetCondition(const wxString &lhs, const wxString &operation,
//                    const wxString &rhs, Integer atIndex)
//------------------------------------------------------------------------------
/**
 * This method sets a condition for the ConditionalBranch Command.
 *
 * @return true if successful; false otherwise.
 *
 */
//------------------------------------------------------------------------------
bool ConditionalBranch::SetCondition(const wxString &lhs, 
                                     const wxString &operation,
                                     const wxString &rhs,
                                     Integer atIndex)
{
   #if DEBUG_CONDITIONS
   MessageInterface::ShowMessage
      (wxT("ConditionalBranch::SetCondition() lhs=(%s), operation=(%s), rhs=(%s)\n"),
       lhs.c_str(), operation.c_str(), rhs.c_str());
   #endif
   
   bool   retVal = false;
   OpType ot     = NumberOfOperators;
   
   // determine the operator
   for (Integer i = 0; i < NumberOfOperators; i++)
   {
      #if DEBUG_CONDITIONS
         MessageInterface::ShowMessage
         (wxT("ConditionalBranch::In loop ...  operation is %s, OPTYPE_TEXT is %s\n"),
          operation.c_str(), (OPTYPE_TEXT[i]).c_str());
      #endif
      if (operation == OPTYPE_TEXT[i])
      {
         ot = (OpType) i;
         #if DEBUG_CONDITIONS
            MessageInterface::ShowMessage
            (wxT("ConditionalBranch::operation is %s, OPTYPE_TEXT is %s\n"),
             operation.c_str(), (OPTYPE_TEXT[i]).c_str());
         #endif
         break;
      }
   }
   
   if (ot == NumberOfOperators)
   {
       wxString errMsg = wxT("The value of \"") + operation + 
            wxT("\" for the relational operator of conditional \"") 
            + typeName +
            wxT("\" is not an allowed value.  The allowed values are: ") +
            wxT(" [==, ~=, <, >, <=, >=]."); 
       throw CommandException(errMsg);
   }
   
   // put it at the end, if requested (and by default)
   if ((atIndex == -999) || (atIndex == numberOfConditions))
   {
      opStrings.push_back(operation);
      opList.push_back(ot);
      lhsList.push_back(lhs);
      rhsList.push_back(rhs);
      lhsWrappers.push_back(NULL); // placeholder for pointer to ElementWrapper
      rhsWrappers.push_back(NULL); // placeholder for pointer to ElementWrapper
      #if DEBUG_CONDITIONS
         MessageInterface::ShowMessage
         (wxT("ConditionalBranch::added condition to end of list\n"));
      #endif
      retVal = true;
      numberOfConditions++;
   }
   // assume that logical operators will be added in order
   else if ((atIndex < 0) || (atIndex > numberOfConditions))
   {
      throw CommandException(
            wxT("ConditionalCommand error: condition index out of bounds"));
   }
   // otherwise, replace an already-existing condition
   else 
   {
      opStrings.at(atIndex) = operation;
      opList.at(atIndex)    = ot;
      lhsList.at(atIndex)   = lhs;
      rhsList.at(atIndex)   = rhs;
      
      #if DEBUG_CONDITIONS
         MessageInterface::ShowMessage
         (wxT("ConditionalBranch::inserted condition into list\n"));
      #endif
      retVal = true;
   }
   
   return retVal;
}


//------------------------------------------------------------------------------
//  bool SetConditionOperator(const wxString &op, Integer atIndex)
//------------------------------------------------------------------------------
/**
 * This method sets a logical operator for the ConditionalBranch Command.
 *
 * @return true if successful; false otherwise.
 *
 * @note This method assumes that condition operators are added in order
 *
 */
//------------------------------------------------------------------------------
bool ConditionalBranch::SetConditionOperator(const wxString &op, 
                                             Integer atIndex)
{
   #if DEBUG_CONDITIONS
   MessageInterface::ShowMessage
      (wxT("ConditionalBranch::SetConditionOperator()op=%s, atIndex\n"),
       op.c_str(), atIndex);
   #endif
   
   bool          retVal = false;
   LogicalOpType ot     = NumberOfLogicalOperators;
   
   // determine the logical operator
   for (Integer i = 0; i < NumberOfLogicalOperators; i++)
   {
      if (op == LOGICAL_OPTYPE_TEXT[i])
      {
         ot = (LogicalOpType) i;
         break;
      }
   }
   
   if (ot == NumberOfLogicalOperators)
   {
       wxString errMsg = wxT("The value of \"") + op + 
            wxT("\" for the logical operator of conditional \"") 
            + typeName +
            wxT("\" is not an allowed value.  The allowed values are: ") +
            wxT(" [&,|]."); 
       throw CommandException(errMsg);
   }
   
   if ((atIndex == -999) || (atIndex == numberOfLogicalOps))
   {
      logicalOpStrings.push_back(op);
      logicalOpList.push_back(ot);
      retVal = true;
      numberOfLogicalOps++;
   }
   // assume that logical operators will be added in order
   else if ((atIndex < 0) || (atIndex > numberOfLogicalOps))
   {
      throw CommandException(
            wxT("ConditionalCommand error: logical operator index out of bounds"));
   }
   // put it at the end, if requested (and by default)
   // otherwise, replace an already-existing logical operator
   else 
   {
      logicalOpStrings.at(atIndex) = op;
      logicalOpList.at(atIndex)    = ot;
      retVal                       = true;
   }
   return retVal;
}

//------------------------------------------------------------------------------
//  bool RemoveCondition(Integer atIndex)
//------------------------------------------------------------------------------
/**
 * Removes the condition for the command, at index atIndex.
 * 
 * @param <atIndex>   where in the list to remove the condition from.
 */
//------------------------------------------------------------------------------
bool ConditionalBranch::RemoveCondition(Integer atIndex)
{
   if ((atIndex < 0) || (atIndex >= numberOfConditions))
      throw CommandException(
            wxT("RemoveCondition error - condition index out of bounds."));
   ElementWrapper *ew;
   lhsList.erase(lhsList.begin() + atIndex);
   ew = lhsWrappers.at(atIndex);
   lhsWrappers.erase(lhsWrappers.begin() + atIndex);
   #ifdef DEBUG_MEMORY
   MemoryTracker::Instance()->Remove
      (ew, ew->GetDescription(), wxT("ConditionalBranch::RemoveCondition()"),
       GetGeneratingString(Gmat::NO_COMMENTS) + wxT(" deleting lhsWrapper"));
   #endif
   delete ew;
   opStrings.erase(opStrings.begin() + atIndex);
   opList.erase(opList.begin() + atIndex);
   rhsList.erase(rhsList.begin() + atIndex);
   ew = rhsWrappers.at(atIndex);
   rhsWrappers.erase(rhsWrappers.begin() + atIndex);
   #ifdef DEBUG_MEMORY
   MemoryTracker::Instance()->Remove
      (ew, ew->GetDescription(), wxT("ConditionalBranch::RemoveCondition()"),
       GetGeneratingString(Gmat::NO_COMMENTS) + wxT(" deleting rhsWrapper"));
   #endif
   delete ew;
   numberOfConditions--;
   return true;
}

//------------------------------------------------------------------------------
//  bool RemoveConditionOperator(Integer atIndex)
//------------------------------------------------------------------------------
/**
 * Removes the logical operator for the command, at index atIndex.
 * 
 * @param <atIndex>   where in the list to remove the logical operator from.
 */
//------------------------------------------------------------------------------
bool ConditionalBranch::RemoveConditionOperator(Integer atIndex)
{
   if ((atIndex < 0) || (atIndex >= numberOfLogicalOps))
      throw CommandException(
            wxT("RemoveConditionOperator error - condition index out of bounds."));
   logicalOpStrings.erase(logicalOpStrings.begin() + atIndex);
   logicalOpList.erase(logicalOpList.begin() + atIndex);
   numberOfLogicalOps--;
   return true;
}


//------------------------------------------------------------------------------
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * Performs the initialization needed to run the conditional branch command.
 *
 * @return true if the Command is initialized, false if an error occurs.
 */
//------------------------------------------------------------------------------
bool ConditionalBranch::Initialize()
{
   bool retval = BranchCommand::Initialize();
   
   // Set references for the wrappers   
   #ifdef DEBUG_CONDITIONS_INIT
   MessageInterface::ShowMessage
      (wxT("ConditionalBranch::Initialize() this='%s', Setting refs for %d lhs wrappers\n"),
       GetGeneratingString(Gmat::NO_COMMENTS).c_str(), lhsWrappers.size());
   #endif
   #ifdef DEBUG_OBJECT_MAP
   ShowObjectMaps();
   #endif
   
   for (std::vector<ElementWrapper*>::iterator i = lhsWrappers.begin();
        i < lhsWrappers.end(); i++)
   {
      #ifdef DEBUG_CONDITIONS_INIT
      MessageInterface::ShowMessage
         (wxT("   wrapper desc = '%s'\n"), (*i)->GetDescription().c_str());
      #endif
      
      if (SetWrapperReferences(*(*i)) == false)
         return false;      
      
      CheckDataType((*i), Gmat::REAL_TYPE, wxT("Conditional Command"));
   }
   
   #ifdef DEBUG_CONDITIONS_INIT
   MessageInterface::ShowMessage
      (wxT("ConditionalBranch::Initialize() Setting refs for %d rhs wrappers\n"),
       rhsWrappers.size());
   #endif
   
   for (std::vector<ElementWrapper*>::iterator j = rhsWrappers.begin();
        j < rhsWrappers.end(); j++)
   {
      #ifdef DEBUG_CONDITIONS_INIT
      MessageInterface::ShowMessage
         (wxT("   wrapper desc = '%s'\n"), (*j)->GetDescription().c_str());
      #endif
      
      if (SetWrapperReferences(*(*j)) == false)
         return false;
      
      CheckDataType((*j), Gmat::REAL_TYPE, wxT("Conditional Command"));
   }
   
   #ifdef DEBUG_CONDITIONS_INIT
   MessageInterface::ShowMessage
      (wxT("ConditionalBranch::Initialize() returning %d\n"), retval);
   #endif
   
   return retval;
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
bool ConditionalBranch::RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName)
{
   #if DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("ConditionalBranch::RenameRefObject() type=%d, oldName=%s, ")
       wxT("newName=%s\n"), type, oldName.c_str(), newName.c_str());
   #endif
   for (UnsignedInt i=0; i<lhsList.size(); i++)
   {
      if (lhsList[i] == oldName) lhsList[i] = newName;
      if (lhsWrappers.at(i) != NULL)
      {
         (lhsWrappers.at(i))->RenameObject(oldName, newName);
         lhsList.at(i) = (lhsWrappers.at(i))->GetDescription();
      }
   }
   for (UnsignedInt j=0; j<rhsList.size(); j++)
   {
      if (rhsList[j] == oldName) rhsList[j] = newName;
      if (rhsWrappers.at(j) != NULL)
      {
         (rhsWrappers.at(j))->RenameObject(oldName, newName);
         rhsList.at(j) = (rhsWrappers.at(j))->GetDescription();
      }
   }

   BranchCommand::RenameRefObject(type, oldName, newName);
   
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
const ObjectTypeArray& ConditionalBranch::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
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
const StringArray& ConditionalBranch::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   refObjectNames.clear();  // no ref objects here   
   return refObjectNames;
}

//------------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                    const wxString &name)
//------------------------------------------------------------------------------
/**
 * This method sets a reference object for the ConditionalBranch Command.
 *
 * @param <obj>   pointer to the reference object
 * @param <type>  type of the reference object 
 * @param <name>  name of the reference object
 *
 * @return true if successful; otherwise, false.
 *
 */
//------------------------------------------------------------------------------
bool ConditionalBranch::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name)
{
   // Not handled here -- invoke the next higher SetRefObject call
   return BranchCommand::SetRefObject(obj, type, name);
}


//------------------------------------------------------------------------------
//  GmatBase* GetRefObject(const Gmat::ObjectType type,
//                         const wxString &name,
//                         const Integer index)
//------------------------------------------------------------------------------
/**
 * This method returns a reference object from the ConditionalBranch Command.
 *
 * @param <type>  type of the reference object requested
 * @param <name>  name of the reference object requested
 * @param <index> index into the array of reference objects
 *
 * @return pointer to the reference object requested.
 *
 */
//------------------------------------------------------------------------------
GmatBase* ConditionalBranch::GetRefObject(const Gmat::ObjectType type,
                                          const wxString &name,
                                          const Integer index)
{
   // Not handled here -- invoke the next higher GetRefObject call
   return BranchCommand::GetRefObject(type, name, index);
}


//------------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                    const wxString &name, const Integer index)
//------------------------------------------------------------------------------
/**
 * This method sets a reference object for the ConditionalBranch Command.
 *
 * @param <obj>   pointer to the reference object
 * @param <type>  type of the reference object 
 * @param <name>  name of the reference object
 * @param <index> index into the array of reference objects (where to put this
 *                 one)
 *
 * @return true if successful; otherwise, false.
 *
 */
//------------------------------------------------------------------------------
bool ConditionalBranch::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name, const Integer index)
{
   // Not handled here -- invoke the next higher SetRefObject call
   return BranchCommand::SetRefObject(obj, type, name, index);
}

//------------------------------------------------------------------------------
//  wxString  GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter text, given the input parameter ID.
 *
 * @param <id> Id for the requested parameter text.
 *
 * @return parameter text for the requested parameter.
 *
 */
//------------------------------------------------------------------------------
wxString ConditionalBranch::GetParameterText(const Integer id) const
{
   if (id >= BranchCommandParamCount && id < ConditionalBranchParamCount)
      return PARAMETER_TEXT[id - BranchCommandParamCount];
   return BranchCommand::GetParameterText(id);
}

//------------------------------------------------------------------------------
//  Integer  GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter ID, given the input parameter string.
 *
 * @param <str> string for the requested parameter.
 *
 * @return ID for the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Integer ConditionalBranch::GetParameterID(const wxString &str) const
{
   for (Integer i = BranchCommandParamCount; i < ConditionalBranchParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - BranchCommandParamCount])
         return i;
   }
   
   return BranchCommand::GetParameterID(str);
}

//------------------------------------------------------------------------------
//  Gmat::ParameterType  GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type, given the input parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return parameter type of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Gmat::ParameterType ConditionalBranch::GetParameterType(const Integer id) const
{
   if (id >= BranchCommandParamCount && id < ConditionalBranchParamCount)
      return PARAMETER_TYPE[id - BranchCommandParamCount];
   
   return BranchCommand::GetParameterType(id);
}

//------------------------------------------------------------------------------
//  wxString  GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type string, given the input parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return parameter type string of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
wxString ConditionalBranch::GetParameterTypeString(const Integer id) const
{
   return ConditionalBranch::PARAM_TYPE_STRING[GetParameterType(id)];
}

//------------------------------------------------------------------------------
//  Integer  GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the Integer parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return  Integer value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Integer ConditionalBranch::GetIntegerParameter(const Integer id) const
{
   if (id == NUMBER_OF_CONDITIONS)          return numberOfConditions;
   if (id == NUMBER_OF_LOGICAL_OPS)         return numberOfLogicalOps;
   //if (id == NUMBER_OF_REF_PARAMS)          return (Integer) params.size();
   
   return BranchCommand::GetIntegerParameter(id); 
}


//------------------------------------------------------------------------------
//  Integer  GetIntegerParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * This method returns the Integer parameter value, given the input
 * parameter ID.
 *
 * @param <label> label for the requested parameter.
 *
 * @return  Integer value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Integer ConditionalBranch::GetIntegerParameter(const wxString &label) const
{
   return GetIntegerParameter(GetParameterID(label));
}

//Integer      ConditionalBranch::SetIntegerParameter(const wxString &label, const Integer value);

//------------------------------------------------------------------------------
//  wxString GetStringParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
/**
 * This method gets a string parameter value of a StringArray, for the input
 * parameter ID, at the input index into the array.
 *
 * @param <id>    ID for the requested parameter.
 * @param <index> index into the StringArray.
 *
 * @return  string value of the requested StringArray parameter, at the 
 *          requested index.
 *
 */
//------------------------------------------------------------------------------
wxString ConditionalBranch::GetStringParameter(const Integer id,
                                                  const Integer index) const
{
   wxString errorString = wxT("ConditionalCommand error: Requested index ");
   errorString += index;
   errorString += wxT(" is out of bounds for ");
   if (id == LEFT_HAND_STRINGS)
   {
      if (index < 0 || index >= (Integer) lhsList.size())
      {
         errorString += wxT("left hand side string list.");
         throw CommandException(errorString);
      }
      return (lhsList.at(index));
   }
   if (id == OPERATOR_STRINGS)   
   {
      if (index < 0 || index >= (Integer) opStrings.size())
      {
         errorString += wxT("operator string list.");
         throw CommandException(errorString);
      }
      return (opStrings.at(index));
   }
   if (id == RIGHT_HAND_STRINGS) 
   {
      if (index < 0 || index >= (Integer) rhsList.size())
      {
         errorString += wxT("right hand side string list.");
         throw CommandException(errorString);
      }
      return (rhsList.at(index));
   }
   if (id == LOGICAL_OPERATORS)  
   {
      if (index < 0 || index >= (Integer) logicalOpStrings.size())
      {
         errorString += wxT("logical operator string list.");
         throw CommandException(errorString);
      }
      return (logicalOpStrings.at(index));
   }
   
   return BranchCommand::GetStringParameter(id, index);
}

//------------------------------------------------------------------------------
//  bool SetStringParameter(const Integer id, const wxString &value,
//                          const Integer index) const
//------------------------------------------------------------------------------
 /**
 * This method sets a string parameter value of a StringArray, for the input
 * parameter ID, at the input index into the array.
 *
 * @param <id>    ID for the requested parameter.
 * @param <value> value to set the parameter to
 * @param <index> index into the StringArray.
 *
 * @return  success or failure of the operation.
 *
 */
//------------------------------------------------------------------------------
bool ConditionalBranch::SetStringParameter(const Integer id, 
                                           const wxString &value,
                                           const Integer index)
{
   wxString errorString = wxT("ConditionalCommand error: Requested index ");
   errorString += index;
   errorString += wxT(" is out of bounds for ");
   ElementWrapper *ew;
   if (id == LEFT_HAND_STRINGS)
   {
      if (index < 0 || index >= (Integer) lhsList.size())
      {
         errorString += wxT("left hand side string list.");
         throw CommandException(errorString);
      }
      lhsList.at(index) = value;
      ew = lhsWrappers.at(index);
      lhsWrappers.at(index) = NULL;
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (ew, ew->GetDescription(), wxT("ConditionalBranch::SetStringParameter()"),
          GetGeneratingString(Gmat::NO_COMMENTS) + wxT(" deleting lhsWrapper"));
      #endif
      delete ew;
      return true;
   }
   if (id == OPERATOR_STRINGS)   
   {
      if (index < 0 || index >= (Integer) opStrings.size())
      {
         errorString += wxT("operator string list.");
         throw CommandException(errorString);
      }
      opStrings.at(index) = value;
      return true;
   }
   if (id == RIGHT_HAND_STRINGS) 
   {
      if (index < 0 || index >= (Integer) rhsList.size())
      {
         errorString += wxT("right hand side string list.");
         throw CommandException(errorString);
      }
      rhsList.at(index) = value;
      ew = rhsWrappers.at(index);
      rhsWrappers.at(index) = NULL;
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (ew, ew->GetDescription(), wxT("ConditionalBranch::SetStringParameter()"),
          GetGeneratingString(Gmat::NO_COMMENTS) + wxT(" deleting rhsWrapper"));
      #endif
      delete ew;
      return true;
   }
   if (id == LOGICAL_OPERATORS)  
   {
      if (index < 0 || index >= (Integer) logicalOpStrings.size())
      {
         errorString += wxT("logical operator string list.");
         throw CommandException(errorString);
      }
      logicalOpStrings.at(index) = value;
      return true;
   }
   
   return BranchCommand::SetStringParameter(id, value, index);
}

//------------------------------------------------------------------------------
//  wxString GetStringParameter(const wxString &label, 
//                                 const Integer index) const
//------------------------------------------------------------------------------
/**
 * This method gets a string parameter value of a StringArray, for the input
 * parameter label, at the input index into the array.
 *
 * @param <label> label for the requested parameter.
 * @param <index> index into the StringArray.
 *
 * @return  string value of the requested StringArray parameter, at the 
 *          requested index.
 *
 */
//------------------------------------------------------------------------------
wxString ConditionalBranch::GetStringParameter(const wxString &label,
                                        const Integer index) const
{
   return GetStringParameter(GetParameterID(label), index);
}

//------------------------------------------------------------------------------
//  bool SetStringParameter(const wxString &label, const wxString &value,
//                          const Integer index) const
//------------------------------------------------------------------------------
/**
 * This method gets a string parameter value of a StringArray, for the input
 * parameter label, at the input index into the array.
 *
 * @param <label> label for the requested parameter.
 * @param <value> value to set the parameter to
 * @param <index> index into the StringArray.
 *
 * @return  success or failure of the operation.
 *
 */
//------------------------------------------------------------------------------
bool ConditionalBranch::SetStringParameter(const wxString &label, 
                                        const wxString &value,
                                        const Integer index)
{
   return SetStringParameter(GetParameterID(label), value, index);
}

//------------------------------------------------------------------------------
//  const StringArray&   GetStringArrayParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the StringArray parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return  StringArray value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
const StringArray& 
ConditionalBranch::GetStringArrayParameter(const Integer id) const
{
   if (id == LEFT_HAND_STRINGS)  return lhsList; //lhsParamList;
   if (id == OPERATOR_STRINGS)   return opStrings;
   if (id == RIGHT_HAND_STRINGS) return rhsList; //rhsParamList;
   if (id == LOGICAL_OPERATORS)  return logicalOpStrings;
   
   return BranchCommand::GetStringArrayParameter(id);
}

//------------------------------------------------------------------------------
//  const StringArray&   GetStringArrayParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * This method returns the StringArray parameter value, given the input
 * parameter label.
 *
 * @param <label> label for the requested parameter.
 *
 * @return  StringArray value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
const StringArray& 
ConditionalBranch::GetStringArrayParameter(const wxString &label) const
{
   return GetStringArrayParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// const StringArray& GetWrapperObjectNameArray()
//------------------------------------------------------------------------------
const StringArray& ConditionalBranch::GetWrapperObjectNameArray()
{
   wrapperObjectNames.clear();

   for (StringArray::iterator i = lhsList.begin(); i < lhsList.end(); i++)
   {
      if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), 
          (*i)) == wrapperObjectNames.end())
         wrapperObjectNames.push_back((*i));
   }
   
   for (StringArray::iterator j = rhsList.begin(); j < rhsList.end(); j++)
   {
      if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), 
          (*j)) == wrapperObjectNames.end())
         wrapperObjectNames.push_back((*j));
   }
   
   #ifdef DEBUG_WRAPPERS
      MessageInterface::ShowMessage
         (wxT("ConditionalBranch::GetWrapperObjectNameArray() %s wrapper names are:\n"),
          cmd->GetTypeName().c_str());
      for (Integer ii=0; ii < (Integer) wrapperNames.size(); ii++)
         MessageInterface::ShowMessage(wxT("      %s\n"), wrapperNames[ii].c_str());
   #endif
      
   return wrapperObjectNames;
}


//------------------------------------------------------------------------------
// bool SetElementWrapper(ElementWrapper *toWrapper, const wxString &withName)
//------------------------------------------------------------------------------
bool ConditionalBranch::SetElementWrapper(ElementWrapper *toWrapper, 
                                          const wxString &withName)
{
   #ifdef DEBUG_WRAPPER_CODE
   MessageInterface::ShowMessage
      (wxT("ConditionalBranch::SetElementWrapper() this=<%p> '%s' entered\n"),
       this, GetGeneratingString(Gmat::NO_COMMENTS).c_str());
   #endif
   
   bool retval = false;
   ElementWrapper *ew;

   if (toWrapper == NULL) return false;
   // this would be caught by next part, but this message is more meaningful
   if (toWrapper->GetWrapperType() == Gmat::ARRAY_WT)
   {
      throw CommandException(wxT("A value of type \"Array\" on command \"") + typeName + 
                  wxT("\" is not an allowed value.\nThe allowed values are:")
                  wxT(" [ Real Number, Variable, Array Element, or Parameter ]. ")); 
   }
   CheckDataType(toWrapper, Gmat::REAL_TYPE, wxT("ConditionalBranch"), true);
   
   
   #ifdef DEBUG_WRAPPER_CODE   
   MessageInterface::ShowMessage
      (wxT("ConditionalBranch::SetElementWrapper() Setting wrapper <%p> '%s'"),
       toWrapper, withName.c_str());
   #endif
   Integer sz = lhsList.size();
   for (Integer i = 0; i < sz; i++)
   {
      if (lhsList.at(i) == withName)
      {
         #ifdef DEBUG_WRAPPER_CODE   
         MessageInterface::ShowMessage(
            wxT("   Found wrapper name \"%s\" in lhsList\n"), withName.c_str());
         #endif
         if (lhsWrappers.at(i) != NULL)
         {
            ew = lhsWrappers.at(i);
            lhsWrappers.at(i) = toWrapper;
            
            // Delete if wrapper name not found in the rhsList
            if (find(rhsList.begin(), rhsList.end(), withName) == rhsList.end())
            {
               #ifdef DEBUG_MEMORY
               MemoryTracker::Instance()->Remove
                  (ew, ew->GetDescription(), wxT("ConditionalBranch::SetElementWrapper()"),
                   GetGeneratingString(Gmat::NO_COMMENTS) + wxT(" deleting lhsWrapper"));
               #endif
               delete ew;
               ew = NULL;
            }
         }
         else lhsWrappers.at(i) = toWrapper;
         retval = true;
      }
   }
   sz = rhsList.size();
   for (Integer i = 0; i < sz; i++)
   {
      if (rhsList.at(i) == withName)
      {
         #ifdef DEBUG_WRAPPER_CODE   
         MessageInterface::ShowMessage(
            wxT("   Found wrapper name \"%s\" in rhsList\n"), withName.c_str());
         #endif
         if (rhsWrappers.at(i) != NULL)
         {
            ew = rhsWrappers.at(i);
            rhsWrappers.at(i) = toWrapper;
            
            // Delete if wrapper name not found in the lhsList
            if (find(lhsList.begin(), lhsList.end(), withName) == lhsList.end())
            {
               #ifdef DEBUG_MEMORY
               MemoryTracker::Instance()->Remove
                  (ew, ew->GetDescription(), wxT("ConditionalBranch::SetElementWrapper()"),
                   GetGeneratingString(Gmat::NO_COMMENTS) + wxT(" deleting rhsWrapper"));
               #endif
               delete ew;
               ew = NULL;
            }
         }
         else rhsWrappers.at(i) = toWrapper;
         retval = true;
      }
   }
   
   return retval;
}


//------------------------------------------------------------------------------
// void ClearWrappers()
//------------------------------------------------------------------------------
void ConditionalBranch::ClearWrappers()
{
   #ifdef DEBUG_WRAPPER_CODE
   MessageInterface::ShowMessage
      (wxT("ConditionalBranch::ClearWrappers() this=<%p> '%s' entered\n"),
       this, GetGeneratingString(Gmat::NO_COMMENTS).c_str());
   #endif
   
   std::vector<ElementWrapper*> temp;
   Integer sz = (Integer) lhsWrappers.size();
   for (Integer i=0; i<sz; i++)
   {
      if (lhsWrappers.at(i) != NULL)
      {
         // Add lhs wrapper if it has not already added
         if (find(temp.begin(), temp.end(), lhsWrappers.at(i)) == temp.end())
            temp.push_back(lhsWrappers.at(i));
         
         lhsWrappers.at(i) = NULL;
      }
   }
   
   sz = (Integer) rhsWrappers.size();
   for (Integer i=0; i<sz; i++)
   {
      if (rhsWrappers.at(i) != NULL)
      {
         // Add rhs wrapper if it has not already added
         if (find(temp.begin(), temp.end(), rhsWrappers.at(i)) == temp.end())
            temp.push_back(rhsWrappers.at(i));
         
         rhsWrappers.at(i) = NULL;
      }
   }
   
   ElementWrapper *ew;
   for (UnsignedInt i = 0; i < temp.size(); ++i)
   {
      ew = temp[i];
      
      if (ew != NULL)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (ew, ew->GetDescription(), wxT("ConditionalBranch::ClearWrappers()"),
             //GetGeneratingString(Gmat::NO_COMMENTS) + wxT(" deleting lhsWrapper"));
             GetTypeName() + wxT(" deleting lhsWrapper"));
         #endif
         delete ew;
      }
      
      ew = NULL;
   }
   
   #ifdef DEBUG_WRAPPER_CODE
   MessageInterface::ShowMessage(wxT("ConditionalBranch::ClearWrappers() leaving\n"));
   #endif
}

//------------------------------------------------------------------------------
// protected methods
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  bool EvaluateCondition(Integer which)
//------------------------------------------------------------------------------
/**
 * This method evaluates the specified condition.
 *
 * @param which index into the array(s) of conditions
 *
 * @return false if which is out of bounds; result of evaluating the
 *         condition, otherwise.
 *
 * @note This method currently assumes that the rhs of the condition is a
 *       Real number.  In the future, we will need to allow for the possibility
 *       of the rhs of the condition being another parameter, which will need
 *       to be evaluated.
 */
//------------------------------------------------------------------------------
bool ConditionalBranch::EvaluateCondition(Integer which)
{
   #ifdef DEBUG_CONDITIONS
   MessageInterface::ShowMessage
      (wxT("ConditionalBranch::EvaluateCondition() entered; which = %d\n   ")
       wxT("Number of Conditions: %d\n"), which, numberOfConditions);      
   MessageInterface::ShowMessage
      (wxT("      lhs wrapper = %s        rhsWrapper = %s\n"), 
       (lhsList.at(which)).c_str(), (rhsList.at(which)).c_str());
   #endif
   if ((which < 0) || (which >= numberOfConditions))
   {
      #ifdef DEBUG_CONDITIONS
         MessageInterface::ShowMessage(
         wxT("ConditionalBranch::EvaluateCondition() - returning with FALSE!!!\n"));      
      
      #endif
      return false;
   }
   if ((lhsWrappers.at(which) == NULL) || (rhsWrappers.at(which) == NULL))
   {
      wxString errmsg = wxT("Error evaluating condition \"") + lhsList.at(which);
      errmsg += wxT(" ") + opStrings.at(which);
      errmsg += wxT(" ") + rhsList.at(which);
      errmsg += wxT("\" - wrapper is NULL\n");
      throw CommandException(errmsg);
   }
   Real lhsValue = (lhsWrappers.at(which))->EvaluateReal();   
   Real rhsValue = (rhsWrappers.at(which))->EvaluateReal();
   
   #ifdef DEBUG_CONDITIONS
      MessageInterface::ShowMessage(
         wxT("   lhs = %.18f,  rhs = %.18f\n"), lhsValue, rhsValue);
      MessageInterface::ShowMessage(
         wxT("   lhs - rhs = %.18f\n"), (lhsValue - rhsValue));
      MessageInterface::ShowMessage(
         wxT("   operator = %d\n"), (Integer) opList.at(which));
      MessageInterface::ShowMessage(
         wxT("   operator (as string) = %s\n"), 
         (OPTYPE_TEXT[(Integer) opList.at(which)]).c_str());
   #endif

   switch (opList.at(which))
   {
      case EQUAL_TO:
         return (lhsValue == rhsValue);
         break;
      case NOT_EQUAL:
         return (lhsValue != rhsValue);
         break;
      case GREATER_THAN:
         return (lhsValue > rhsValue);
         break;
      case LESS_THAN:
         #ifdef DEBUG_CONDITIONS
         if (lhsValue < rhsValue)
            MessageInterface::ShowMessage(wxT("   returning TRUE .......\n"));
         else
            MessageInterface::ShowMessage(wxT("   returning FALSE .......\n"));
         #endif
         return (lhsValue < rhsValue);
         break;
      case GREATER_OR_EQUAL:
         return (lhsValue >= rhsValue);
         break;
      case LESS_OR_EQUAL:
         return (lhsValue <= rhsValue);
         break;
      default:
         return false;
         break;
   }
   return false; 
   
}

//------------------------------------------------------------------------------
//  bool EvaluateAllConditions()
//------------------------------------------------------------------------------
/**
 * This method evaluates the entire list of conditions, using their logical
 * operators.  Evaluation is from left to right.
 *
 * @return result of evaluating the entire list of conditions, from left to
 *         right.
 */
//------------------------------------------------------------------------------
bool ConditionalBranch::EvaluateAllConditions()
{
   #ifdef DEBUG_CONDITIONS
      MessageInterface::ShowMessage(
         wxT("   Entering EvaluateAllConditions with number of conditons = %d\n"),
         numberOfConditions);
   #endif
   if (numberOfConditions == 0)
      throw CommandException(
         wxT("Error in conditional statement - no conditions specified."));
   
   if (numberOfConditions != (numberOfLogicalOps + 1))
      throw CommandException(
         wxT("conditional statement incorrect - too few/many logical operators"));
   
   // divide into sets of higher-precedence AND operators, then OR them
   // @todo Create a LogicTree for this type, allowing use of parentheses as well
   bool         done            = false;
   Integer      current         = 0;
   bool         soFar           = false;
   bool         setOfCmdsFound  = false;
   bool         evalAnds        = true;
   IntegerArray andConditions;
   while (!done)
   {
      andConditions.clear();
      setOfCmdsFound  = false;
      evalAnds        = true;
      while (!setOfCmdsFound)
      {
         andConditions.push_back(current);
         if (current == (numberOfConditions-1)) // are we at the end of the list of conditions?
         {
            setOfCmdsFound = true;
            done           = true;
         }
         else
         {
            if (logicalOpList.at(current) == OR) setOfCmdsFound = true; // or is the operator an OR?
            current++;
         }
      }
      // found an OR, so evaluate the AND conditions
      for (Integer ii = 0; ii < (Integer) andConditions.size(); ii++)
      {
         evalAnds = evalAnds && EvaluateCondition(andConditions.at(ii));
      }
      // previous result OR current result from group of AND conditions
      soFar = soFar || evalAnds;
   } // not done
   andConditions.clear();
//   bool soFar = EvaluateCondition(0);
//   #ifdef DEBUG_CONDITIONS
//      MessageInterface::ShowMessage(
//         wxT("   After EvaluateCondition, soFar = %s\n", (soFar? "true" : "false")));
//   #endif
//
//
//   Integer i = 1;
//   for (i=1; i < numberOfConditions; i++)
//   {
//      switch (logicalOpList.at(i-1))
//      {
//         case AND:
//            soFar = soFar && EvaluateCondition(i);
//            break;
//         case OR:
//            soFar = soFar || EvaluateCondition(i);
//            break;
//         default:
//            throw CommandException(
//                  wxT("Unknown logical operator in conditional statement."));
//            break;
//      }
//   }
   
   #ifdef DEBUG_CONDITIONS
   if (soFar)
      MessageInterface::ShowMessage(wxT("   all are TRUE .......\n"));
   else
      MessageInterface::ShowMessage(wxT("   some are FALSE .......\n"));
   #endif
   
   return soFar;
}


// remove this?
bool ConditionalBranch::SetStringArrayValue(Integer forArray, 
                                            const wxString &toValue,
                                            Integer forIndex)
{
   return true;  // TEMPORARY
}


//------------------------------------------------------------------------------
//  wxString GetConditionalString()
//------------------------------------------------------------------------------
/**
 * This method builds the string that generates the condition list.
 *
 * @return The string description of the conditions.
 */
//------------------------------------------------------------------------------
wxString ConditionalBranch::GetConditionalString()
{
   #ifdef DEBUG_CONDBR_GET_GEN_STRING
      MessageInterface::ShowMessage(wxT("Entering ConditionalBranch::GetConditionalString\n"));
      MessageInterface::ShowMessage(wxT("... number of conditions = %d\n"), 
         numberOfConditions);
   #endif
   wxString cond;
   
   if ((lhsList.size() == 0) || (rhsList.size() == 0))
      throw CommandException(wxT("Conditional command is missing its conditions!"));
   
   // The first condition
   cond = lhsList[0] + wxT(" ") + opStrings[0] + wxT(" ") + rhsList[0];
   
   for (Integer i = 1; i < numberOfConditions; i++)
   {
   #ifdef DEBUG_CONDBR_GET_GEN_STRING
      MessageInterface::ShowMessage(wxT("Now adding condition %d to the string\n"), i);
      MessageInterface::ShowMessage(wxT("The logical operator = \"%s\"\n"), logicalOpStrings[i-1].c_str());
      MessageInterface::ShowMessage(wxT("The left-hand-side = \"%s\"\n"), lhsList[i].c_str());
      MessageInterface::ShowMessage(wxT("The operator = \"%s\"\n"), opStrings[i].c_str());
      MessageInterface::ShowMessage(wxT("The right-hand-side = \"%s\"\n"), rhsList[i].c_str());
   #endif
      cond += wxT(" ") + logicalOpStrings[i-1] + wxT(" ");
      cond += lhsList[i] + wxT(" ") + opStrings[i] + wxT(" ") + rhsList[i];
   }

   #ifdef DEBUG_CONDBR_GET_GEN_STRING
      MessageInterface::ShowMessage(wxT("Exiting ConditionalBranch::GetConditionalString\n"));
      MessageInterface::ShowMessage(wxT("... returning cond = %s\n"), cond.c_str());
   #endif
   return cond;
}
