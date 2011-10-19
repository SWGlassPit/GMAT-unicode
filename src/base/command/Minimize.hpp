//$Id$
//------------------------------------------------------------------------------
//                                  Minimize
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
// Author: Daniel Hunter, Wendy Shoan (GSFC/MAB)
// Created: 2006.08.11
//
/**
 * Definition for the Minimize command class
 */
//------------------------------------------------------------------------------


#ifndef Minimize_hpp
#define Minimize_hpp
 

#include "GmatCommand.hpp"
#include "Solver.hpp"
#include "Variable.hpp"
#include "ElementWrapper.hpp"

/**
 * Command that manages processing for optimizer variables.
 */
class GMAT_API Minimize : public GmatCommand
{
public:
   Minimize();
   Minimize(const Minimize& m);
   Minimize&           operator=(const Minimize& m);
   virtual ~Minimize();

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
   /// The name of the spacecraft that gets maneuvered
   wxString         optimizerName;
   /// Name of the variable to be minimized
   wxString         objectiveName;
   // pointer to the Variable that is to be minimized
   //Variable            *objective;
   /// pointer to the wrapper containing the objective 
   ElementWrapper      *objective;
   /// value of the variable
   //Real                objectiveValue; // maybe I don't need this
   /// The optimizer instance used to manage the optimizer state machine
   Solver              *optimizer;
   /// Flag used to finalize the targeter data during execution
   bool                optimizerDataFinalized;
   /// is it a parameter?
   //bool                isMinimizeParm;
   /// Optimizer ID for the variable 
   Integer             objId;
   /// flag indicating whether or not the generating string has been interpreted
   bool                interpreted;

   
   // Parameter IDs
   enum 
   {
      OPTIMIZER_NAME = GmatCommandParamCount,
      OBJECTIVE_NAME,
      MinimizeParamCount
   };
   static const wxString
                       PARAMETER_TEXT[MinimizeParamCount - GmatCommandParamCount];
   static const Gmat::ParameterType
                       PARAMETER_TYPE[MinimizeParamCount - GmatCommandParamCount];

   //bool                InterpretParameter(const wxString text,
   //                                       wxString &paramType,
   //                                       wxString &paramObj,
   //                                       wxString &parmSystem);
   //bool                ConstructGoal(const char* str);

};


#endif  // Minimize_hpp

