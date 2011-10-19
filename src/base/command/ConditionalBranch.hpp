//$Id: ConditionalBranch.hpp 9513 2011-04-30 21:23:06Z djcinsb $
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
// Author:  Wendy Shoan
// Created: 2004/09/27
//
/**
 * Declaration for the ConditionalBranch command class
 */
//------------------------------------------------------------------------------


#ifndef ConditionalBranch_hpp
#define ConditionalBranch_hpp

#include "gmatdefs.hpp"
#include "BranchCommand.hpp"
#include "Parameter.hpp"
#include "ElementWrapper.hpp"

/**
 * Command that manages processing for entry to the conditional
 * branch commands. 
 *
 * The ConditionalBranch command manages the conditional branch
 * commands.  
 *
 */
class GMAT_API ConditionalBranch : public BranchCommand
{
public:
   // default constructor
   ConditionalBranch(const wxString &typeStr);
   // copy constructor
   ConditionalBranch(const ConditionalBranch &cb);
   // operator =
   ConditionalBranch& operator=(const ConditionalBranch &cb);
   // destructor
   virtual ~ConditionalBranch();
         
   // Method to set up the condition(s) for the conditional branch commands
   virtual bool         SetCondition(const wxString &lhs, 
                                     const wxString &operation,
                                     const wxString &rhs,
                                     Integer atIndex = -999);
   virtual bool         SetConditionOperator(const wxString &op,
                                             Integer atIndex = -999);
   virtual bool         RemoveCondition(Integer atIndex);
   virtual bool         RemoveConditionOperator(Integer atIndex);
   
   
   virtual bool         Initialize();
   
   // inherited from GmatBase
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                     const wxString &oldName,
                                     const wxString &newName);
   virtual const ObjectTypeArray&
                       GetRefObjectTypeArray();
   virtual const StringArray&
                       GetRefObjectNameArray(const Gmat::ObjectType type);
   
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name);
   
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name,
                                     const Integer index);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name,
                                     const Integer index);
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   
   virtual Integer      GetIntegerParameter(const Integer id) const;
   virtual Integer      GetIntegerParameter(const wxString &label) const;
   virtual wxString  GetStringParameter(const Integer id,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value,
                                           const Integer index);
   virtual wxString  GetStringParameter(const wxString &label,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const wxString &label, 
                                           const wxString &value,
                                           const Integer index);
   virtual const StringArray& 
                        GetStringArrayParameter(const Integer id) const; 
   virtual const StringArray& 
                        GetStringArrayParameter(const wxString &label) const;

   virtual const StringArray& 
                       GetWrapperObjectNameArray();
   virtual bool        SetElementWrapper(ElementWrapper* toWrapper,
                                         const wxString &withName);
   virtual void        ClearWrappers();
   
protected:

   virtual bool         EvaluateCondition(Integer which);
   virtual bool         EvaluateAllConditions();
   
   virtual bool         SetStringArrayValue(Integer forArray, 
                                            const wxString &toValue,
                                            Integer forIndex);
                                            
   wxString          GetConditionalString();
   
   enum
   {
      NUMBER_OF_CONDITIONS = BranchCommandParamCount,
      NUMBER_OF_LOGICAL_OPS,
      LEFT_HAND_STRINGS,
      OPERATOR_STRINGS,
      RIGHT_HAND_STRINGS,
      LOGICAL_OPERATORS,
      ConditionalBranchParamCount
   };

   static const wxString
   PARAMETER_TEXT[ConditionalBranchParamCount - BranchCommandParamCount];
   
   static const Gmat::ParameterType
   PARAMETER_TYPE[ConditionalBranchParamCount - BranchCommandParamCount];

   enum OpType
   {
      EQUAL_TO = 0,
      NOT_EQUAL,
      GREATER_THAN,
      LESS_THAN,
      GREATER_OR_EQUAL,
      LESS_OR_EQUAL,
      NumberOfOperators
   };
   
   enum LogicalOpType
   {
      AND = 0,
      OR,
      NumberOfLogicalOperators
   };

   static const wxString OPTYPE_TEXT[NumberOfOperators];
   static const wxString LOGICAL_OPTYPE_TEXT[NumberOfLogicalOperators];

   /// Number of conditions for the conditional branch commands
   Integer                    numberOfConditions;
   /// Number  of separating logical operators (should be numberOfConditions - 1)
   Integer                    numberOfLogicalOps;
   /// Arrays representing conditions
   StringArray                lhsList;
   /// vector of pointers to ElementWrappers for the lhs
   std::vector<ElementWrapper*> lhsWrappers;
   //StringArray                lhsParamList;
   StringArray                opStrings;
   std::vector<OpType>        opList;
   StringArray                rhsList;
   /// vector of pointers to ElementWrappers for the rhs
   std::vector<ElementWrapper*> rhsWrappers;
   //StringArray                rhsParamList;
   StringArray                logicalOpStrings;
   std::vector<LogicalOpType> logicalOpList;
   
};
#endif  // ConditionalBranch_hpp
