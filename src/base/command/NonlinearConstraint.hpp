//$Id$
//------------------------------------------------------------------------------
//                                  NonlinearConstraint
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
// Author: Wendy Shoan (GSFC/MAB)
// Created: 2006.08.14
//
/**
 * Definition for the NonlinearConstraint command class
 */
//------------------------------------------------------------------------------


#ifndef NonlinearConstraint_hpp
#define NonlinearConstraint_hpp
 

#include "GmatCommand.hpp"
#include "Solver.hpp"
#include "Parameter.hpp"
#include "ElementWrapper.hpp"

/**
 * Command that manages processing for targeter goals.
 */
class GMAT_API NonlinearConstraint : public GmatCommand
{
public:
   NonlinearConstraint();
   NonlinearConstraint(const NonlinearConstraint& nlc);
   NonlinearConstraint&           operator=(const NonlinearConstraint& nlc);
   virtual ~NonlinearConstraint();

   // inherited from GmatBase
   virtual GmatBase*   Clone() const;

   virtual bool        RenameRefObject(const Gmat::ObjectType type,
                                       const wxString &oldName,
                                       const wxString &newName);
   virtual const ObjectTypeArray&
                       GetRefObjectTypeArray();
   virtual const StringArray&
                       GetRefObjectNameArray(const Gmat::ObjectType type);
   
   
   // Parameter accessors
   virtual wxString GetParameterText(const Integer id) const;
   virtual Integer     GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                       GetParameterType(const Integer id) const;
   virtual wxString GetParameterTypeString(const Integer id) const;

   virtual Real        GetRealParameter(const Integer id) const;
   virtual Real        SetRealParameter(const Integer id,
                                        const Real value);
   virtual wxString GetStringParameter(const Integer id) const;
   virtual bool        SetStringParameter(const Integer id, 
                                          const wxString &value);
                                           
   virtual bool        SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                    const wxString &name = wxT(""));
    
   // Inherited methods overridden from the base class
   virtual bool        InterpretAction();
   virtual const StringArray& 
                       GetWrapperObjectNameArray();
   virtual bool        SetElementWrapper(ElementWrapper* toWrapper,
                                         const wxString &withName);
   virtual void        ClearWrappers();

   virtual bool        Initialize();
   virtual bool        Execute();
   virtual void        RunComplete();
   
   virtual const wxString&
                       GetGeneratingString(Gmat::WriteMode mode,
                                           const wxString &prefix,
                                           const wxString &useName);
    
protected:
   // Parameter IDs
   enum 
   {
      OPTIMIZER_NAME = GmatCommandParamCount,
      CONSTRAINT_ARG1,
      OPERATOR,
      CONSTRAINT_ARG2,
      TOLERANCE,
      NonlinearConstraintParamCount
   };
   static const wxString
                       PARAMETER_TEXT[NonlinearConstraintParamCount - GmatCommandParamCount];
   static const Gmat::ParameterType
                       PARAMETER_TYPE[NonlinearConstraintParamCount - GmatCommandParamCount];

   enum Operator
   {
      LESS_THAN_OR_EQUAL = 0,
      GREATER_THAN_OR_EQUAL,
      EQUAL
   };
   
   static const wxString OP_STRINGS[3];

   /// The name of the spacecraft that gets maneuvered
   wxString         optimizerName;
   /// The optimizer instance used to manage the optimizer state machine
   Solver              *optimizer;
   /// Name of the variable to be constrained
   wxString         arg1Name;
   // pointer to the Variable that is to be minimized
   //Parameter           *constraint;
   ElementWrapper      *arg1;
   /// most recent value of the variable
   Real                constraintValue; // do I still need this?
   /// name of the parameter part of the right-hand-side
   wxString         arg2Name;
   //Parameter           *nlcParm;
   ElementWrapper      *arg2;

   /// String of value array name  // I don't think I need any of this stuff
   //wxString         nlcArrName;
   /// constraint array row index variable name
   //wxString         nlcArrRowStr;
   /// constraint array column index variable name
   //wxString         nlcArrColStr;
   /// constraint array row index
   //Integer             nlcArrRow;
   /// constraint array column index
   //Integer             nlcArrCol;
   //Parameter           *nlcArrRowParm;
   //Parameter           *nlcArrColParm;

   /// flag indicating whether or not the constraint is an inequality
   /// constraint
   bool                isInequality;
   /// string to send into the optimizer, based on isInequality
   wxString         isIneqString;
   /// the desired value (right hand side of the constraint equation)
   Real                desiredValue;
   /// indicates what type of operator was passed in for the generating
   /// string
   Operator            op;
   /// tolerance for the constraint <future>
   Real                tolerance;  // <future>
   /// Flag used to finalize the targeter data during execution
   bool                optimizerDataFinalized;
   /// ID for this constraint (returned from the optimizer)
   Integer             constraintId;
   /// is the right hand side a parameter?
   //bool                isNlcParm;
   /// is the right hand side an array?
   //bool                isNlcArray;
   /// Pointer to the object that owns the goal
   //GmatBase            *constraintObject;
   /// Object ID for the parameter
   //Integer             parmId;
   /// flag indicating whether or not the generating string has been interpreted
   bool                interpreted;
   
   //bool                InterpretParameter(const wxString text,
   //                                       wxString &paramType,
   //                                       wxString &paramObj,
   //                                       wxString &parmSystem);
                                          

   //bool                ConstructConstraint(const char* str);

};


#endif  // NonlinearConstraint_hpp

