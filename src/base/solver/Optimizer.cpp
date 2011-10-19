//$Id: Optimizer.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                         Optimizer
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P
//
// Author: Wendy C. Shoan/GSFC
// Created: 2006.07.14
//
/**
 * Implementation for the optimizer base class. 
 *
 */
//------------------------------------------------------------------------------


#include <sstream>
#include "Optimizer.hpp"
#include "Rmatrix.hpp"
#include "RealUtilities.hpp"     // for GmatMathUtil::Abs()
#include "MessageInterface.hpp"

//#define DEBUG_SET_RESULT

//------------------------------------------------------------------------------
// static data
//------------------------------------------------------------------------------

const wxString
Optimizer::PARAMETER_TEXT[OptimizerParamCount -SolverParamCount] =
{
   wxT("ObjectiveFunction"),
   wxT("Tolerance"),
   wxT("EqualityConstraintNames"),
   wxT("InequalityConstraintNames"),
   wxT("PlotCost"),
};

const Gmat::ParameterType
Optimizer::PARAMETER_TYPE[OptimizerParamCount - SolverParamCount] =
{
   Gmat::STRING_TYPE,
   Gmat::REAL_TYPE,
   Gmat::STRINGARRAY_TYPE,
   Gmat::STRINGARRAY_TYPE,
   Gmat::BOOLEAN_TYPE,
};

const Integer Optimizer::EQ_CONST_START   = 1000;
const Integer Optimizer::INEQ_CONST_START = 2000;

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

Optimizer::Optimizer(wxString typeName, wxString name) :
   Solver                  (typeName, name),
   objectiveDefined        (false),
   objectiveFnName         (wxT("")),
   cost                    (0.0),   // valid value?
   tolerance               (0.0),   // valid value?
   converged               (false),
   eqConstraintCount       (0),
   ineqConstraintCount     (0)
{
   objectTypeNames.push_back(wxT("Optimizer"));
   parameterCount = OptimizerParamCount;
}


Optimizer::~Optimizer()
{
   FreeArrays();
}


Optimizer::Optimizer(const Optimizer &opt) :
   Solver                  (opt),
   objectiveDefined        (false),
   objectiveFnName         (wxT("")),
   cost                    (opt.cost), 
   tolerance               (opt.tolerance), 
   converged               (false),
   eqConstraintCount       (opt.eqConstraintCount),
   ineqConstraintCount     (opt.ineqConstraintCount)
{
   eqConstraintNames    = opt.eqConstraintNames;
   ineqConstraintNames  = opt.ineqConstraintNames;
   eqConstraintValues   = opt.eqConstraintValues;
   ineqConstraintValues = opt.ineqConstraintValues;
   gradient             = opt.gradient;
   //eqConstraintJacobian = opt.eqConstraintJacobian;
   //ineqConstraintJacobian = opt.ineqConstraintJacobian;
   parameterCount       = opt.parameterCount;
}


Optimizer& 
    Optimizer::operator=(const Optimizer& opt)
{
    if (&opt == this)
        return *this;

   Solver::operator=(opt);
   
   objectiveFnName  = opt.objectiveFnName;
   cost             = opt.cost;
   tolerance        = opt.tolerance;
   converged        = opt.converged;
   
   FreeArrays();
   eqConstraintCount    = opt.eqConstraintCount;
   ineqConstraintCount  = opt.ineqConstraintCount;
   eqConstraintNames    = opt.eqConstraintNames;
   ineqConstraintNames  = opt.ineqConstraintNames;
   eqConstraintValues   = opt.eqConstraintValues;
   ineqConstraintValues = opt.ineqConstraintValues;
   gradient             = opt.gradient;
   //eqConstraintJacobian = opt.eqConstraintJacobian;
   //ineqConstraintJacobian = opt.ineqConstraintJacobian;
   parameterCount       = opt.parameterCount;
 
   return *this;
}

bool Optimizer::IsParameterReadOnly(const Integer id) const
{
   if ((id == OBJECTIVE_FUNCTION) ||
       (id == EQUALITY_CONSTRAINT_NAMES) ||
       (id == INEQUALITY_CONSTRAINT_NAMES) ||
       (id == PLOT_COST_FUNCTION))
      return true;
      
   return Solver::IsParameterReadOnly(id);
}


bool Optimizer::IsParameterReadOnly(const wxString &label) const
{
   return IsParameterReadOnly(GetParameterID(label));
}


bool Optimizer::Initialize()
{
   if (variableNames.size() == 0)  // constraints are not required
   {
      wxString errorMessage = wxT("Optimizer cannot initialize: ");
      errorMessage += wxT("No variables are set.\n");
      throw SolverException(errorMessage);
   }
   
   FreeArrays();

   Solver::Initialize();
   
   return true;
}

//------------------------------------------------------------------------------
// Integer SetSolverResults(Real *data, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Sets up the data fields used for the results of an iteration.
 * 
 * @param <data> An array of data appropriate to the results used in the
 *               algorithm (for instance, constraints).
 * @param <name> A label for the data parameter.  Defaults to the empty
 *               string.
 * @param <type> string indicating type of result to set
 * 
 * @return The ID used for this variable.
 */
//------------------------------------------------------------------------------
Integer Optimizer::SetSolverResults(Real *data,
                                    const wxString &name,
                                    const wxString &type)
{
   #ifdef DEBUG_SET_RESULT
      MessageInterface::ShowMessage(
         wxT("Optimizer::SetSolverResults - name = %s, type = %s, data = %.16f\n"),
         name.c_str(), type.c_str(), data[0]); 
   #endif
   
   if (type == wxT("Objective"))
   {
      // need to check here if the name is not the same as the 
      // objectiveFnName? (error)
      objectiveDefined = true;
      objectiveFnName = name;
      cost = data[0];
      return 0;
   }
   else if (type == wxT("EqConstraint"))
   {
      //if (eqConstraintNames[eqConstraintCount] != name)
      //  throw SolverException(
      //     wxT("Mismatch between parsed and configured equality constraint"));
      ///eqConstraintValues[eqConstraintCount] = data[0];
      eqConstraintNames.push_back(name);
      eqConstraintValues.push_back(data[0]);
      ++eqConstraintCount;
      return EQ_CONST_START + eqConstraintCount - 1;
    }
    else if (type == wxT("IneqConstraint"))
    {
      //if (ineqConstraintNames[ineqConstraintCount] != name)
      //  throw SolverException(
      //     wxT("Mismatch between parsed and configured inequality constraint"));
      //ineqConstraintValues[ineqConstraintCount] = data[0];
      ineqConstraintNames.push_back(name);
      ineqConstraintValues.push_back(data[0]);
      ++ineqConstraintCount;
      return INEQ_CONST_START + ineqConstraintCount - 1;
    }
    // add Gradient and Jacobian later ...
    else
    {
       throw SolverException(
           wxT("Unknown type passed in to SetSolverResults"));
    }
}

//------------------------------------------------------------------------------
// void SetResultValue(Integer id, Real value, const wxString &resultType = wxT(""))
//------------------------------------------------------------------------------
/**
 * Passes in the results obtained from a run in the Optimizer loop.
 * 
 * @param <id>    The ID used for this result.
 * @param <value> The corresponding result.
 */
//------------------------------------------------------------------------------
void Optimizer::SetResultValue(Integer id, Real value, 
                               const wxString &resultType)
{
   #ifdef DEBUG_SET_RESULT // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ debug ~~~~
      MessageInterface::ShowMessage(wxT("In Optimizer::SetResultValue\n"));
      MessageInterface::ShowMessage(
      wxT("   id = %d; value = %.12f, resultType = %s\n"),
      id, value, resultType.c_str());
   #endif // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ end debug ~~~~
   if (resultType == wxT("Objective"))
   {
      cost = value;
   }
   else if (resultType == wxT("EqConstraint"))
   {
      if (id > (EQ_CONST_START + eqConstraintCount))
        throw SolverException(
           wxT("id range error for equality constraint"));
     eqConstraintValues[id - EQ_CONST_START] = value;
    }
    else if (resultType == wxT("IneqConstraint"))
    {
       if (id > (INEQ_CONST_START + ineqConstraintCount))
        throw SolverException(
           wxT("id range error for inequality constraint"));
     ineqConstraintValues[id - INEQ_CONST_START] = value;
    }
    // add Gradient and Jacobian later ...
    else
    {
       throw SolverException(
           wxT("Unknown result type passed in to SetResultValue"));
    }
}

//------------------------------------------------------------------------------
//  wxString GetProgressString()
//------------------------------------------------------------------------------
/**
 * Generates a string that reporting the current differential corrector state.
 */
//------------------------------------------------------------------------------
wxString Optimizer::GetProgressString()
{
   StringArray::iterator current;
   Integer i;
   wxString progress;

   if (initialized)
   {
      switch (currentState)
      {
         case INITIALIZING:
            // This state is basically a wxT("paused state") used for the Optimize
            // command to finalize the initial data for the variables and
            // goals.  All that is written here is the header information.
            {
               Integer localVariableCount = variableNames.size();
               Integer localEqCount       = eqConstraintNames.size();
               Integer localIneqCount     = ineqConstraintNames.size();
               progress << wxT("************************************************")
                        << wxT("********\n")
                        << wxT("*** Performing ")
                        << typeName 
                        << wxT(" Optimization ")
                        << wxT("(using \"") << instanceName << wxT("\")\n");

               // Write out the setup data
               progress << wxT("*** ") << localVariableCount << wxT(" variables; ")
                        << localEqCount << wxT(" equality constraints; ")
                        << localIneqCount << wxT(" inequality constraints\n   Variables:  ");

               // Iterate through the variables and goals, writing them to
               // the file
               for (current = variableNames.begin(), i = 0;
                    current != variableNames.end(); ++current)
               {
                  if (current != variableNames.begin())
                     progress << wxT(", ");
                  progress << *current;
               }

               if (localEqCount > 0)
               {
                  progress << wxT("\n   Equality Constraints:  ");
   
                  for (current = eqConstraintNames.begin(), i = 0;
                       current != eqConstraintNames.end(); ++current)
                  {
                     if (current != eqConstraintNames.begin())
                        progress << wxT(", ");
                     progress << *current;
                  }
               }
               
               if (localIneqCount > 0)
               {
                  progress << wxT("\n   Inequality Constraints:  ");
   
                  for (current = ineqConstraintNames.begin(), i = 0;
                       current != ineqConstraintNames.end(); ++current)
                  {
                     if (current != ineqConstraintNames.begin())
                        progress << wxT(", ");
                     progress << *current;
                  }
               }
               
               progress << wxT("\n****************************")
                        << wxT("****************************");
            }
            break;

         case NOMINAL:
            progress << instanceName << wxT(" Iteration ") << iterationsTaken+1
                     << wxT("; Nominal Pass\n   Variables:  ");
            // Iterate through the variables, writing them to the string
            for (current = variableNames.begin(), i = 0;
                 current != variableNames.end(); ++current)
            {
               if (current != variableNames.begin())
                  progress << wxT(", ");
               progress << *current << wxT(" = ") << variable[i++];
            }
            break;

         case PERTURBING:  // does this apply to optimization??
            progress << wxT("   Completed iteration ") << iterationsTaken
                     << wxT(", pert ") << pertNumber+1 << wxT(" (")
                     << variableNames[pertNumber] << wxT(" = ")
                     << variable[pertNumber] << wxT(")\n");
            break;

         case CALCULATING:
            // Just forces a blank line
            break;

         case CHECKINGRUN:
            // Iterate through the constraints, writing them to the file
            progress << wxT("   Equality Constraints and achieved values:\n      ");

            for (current = eqConstraintNames.begin(), i = 0;
                 current != eqConstraintNames.end(); ++current)
            {
               if (current != eqConstraintNames.begin())
                  progress << wxT(",  ");
                  // does this make sense???
               //progress << *current << wxT("  Desired: ") << eqConstaint[i]
               //         << wxT("  Achieved: ") << nominal[i];
               ++i;
            }

           progress << wxT("   Inequality Constraints and achieved values:\n      ");

            for (current = ineqConstraintNames.begin(), i = 0;
                 current != ineqConstraintNames.end(); ++current)
            {
               if (current != ineqConstraintNames.begin())
                  progress << wxT(",  ");
                  // does this make sense???
               //progress << *current << wxT("  Desired: ") << eqConstaint[i]
               //         << wxT("  Achieved: ") << nominal[i];
               ++i;
            }

            break;

         case RUNEXTERNAL:
            progress << instanceName << wxT(" Iteration ") << iterationsTaken+1
                     << wxT("; External Run\n   Variables:  ");
            // Iterate through the variables, writing them to the string
            for (current = variableNames.begin(), i = 0;
                 current != variableNames.end(); ++current)
            {
               if (current != variableNames.begin())
                  progress << wxT(", ");
               progress << *current << wxT(" = ") << variable[i++];
            }
            break;

         case FINISHED:
            progress << wxT("\n*** Optimization Completed in ") << iterationsTaken
                     << wxT(" iterations");
                     
            if (iterationsTaken > maxIterations)
               progress << wxT("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
                        << wxT("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n")
                        << wxT("!!! WARNING: Optimizer did NOT converge in ")
                        << maxIterations << wxT(" iterations!")
                        << wxT("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
                        << wxT("!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            
            progress << wxT("\nFinal Variable values:\n");
            // Iterate through the variables, writing them to the string
            for (current = variableNames.begin(), i = 0;
                 current != variableNames.end(); ++current)
               progress << wxT("   ") << *current << wxT(" = ") << variable[i++] << wxT("\n");
            break;

         case ITERATING:     // Intentional fall through
         default:
            throw SolverException(
               wxT("Solver state not supported for the targeter"));
      }
   }
   else
      return Solver::GetProgressString();
      
   return progress;

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
 */
//------------------------------------------------------------------------------
wxString Optimizer::GetParameterText(const Integer id) const
{
   if ((id >= SolverParamCount) && (id < OptimizerParamCount))
      return PARAMETER_TEXT[id - SolverParamCount];
   return Solver::GetParameterText(id);
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
 */
//------------------------------------------------------------------------------
Integer Optimizer::GetParameterID(const wxString &str) const
{
   for (Integer i = SolverParamCount; i < OptimizerParamCount; ++i)
   {
      if (str == PARAMETER_TEXT[i - SolverParamCount])
         return i;
   }

   return Solver::GetParameterID(str);
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
 */
//------------------------------------------------------------------------------
Gmat::ParameterType Optimizer::GetParameterType(
                                              const Integer id) const
{
   if ((id >= SolverParamCount) && (id < OptimizerParamCount))
      return PARAMETER_TYPE[id - SolverParamCount];

   return Solver::GetParameterType(id);
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
 */
//------------------------------------------------------------------------------
wxString Optimizer::GetParameterTypeString(
                                      const Integer id) const
{
   return Solver::PARAM_TYPE_STRING[GetParameterType(id)];
}

//------------------------------------------------------------------------------
//  Real  GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the Real parameter value, given the input parameter ID.
 *
 * @param <id> ID for the requested parameter value.
 *
 * @return  Real value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Real Optimizer::GetRealParameter(const Integer id) const
{
   if (id == OPTIMIZER_TOLERANCE)               return tolerance;

   return Solver::GetRealParameter(id);
}

//------------------------------------------------------------------------------
//  Real  SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * This method sets the Real parameter value, given the input parameter ID.
 *
 * @param <id> ID for the parameter whose value to change.
 * @param <value> value for the parameter.
 *
 * @return  Real value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Real Optimizer::SetRealParameter(const Integer id, const Real value)
{
   if (id == OPTIMIZER_TOLERANCE)
   {
      if (value <= 0.0)
         throw SolverException(
               wxT("The value entered for the optimizer tolerance on ") +
               instanceName + wxT(" is not an allowed value. The allowed value ")
               wxT("is: [Real > 0.0]."));

      tolerance = value;
      return tolerance;
   }
   return Solver::SetRealParameter(id, value);
}

//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return  string value of the requested parameter.
 */
//------------------------------------------------------------------------------
wxString Optimizer::GetStringParameter(const Integer id) const
{
    if (id == OBJECTIVE_FUNCTION)
        return objectiveFnName;
        
    return Solver::GetStringParameter(id);
}


//------------------------------------------------------------------------------
//  Integer SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
/**
 * This method sets a string or string array parameter value, given the input
 * parameter ID.
 *
 * @param <id>    ID for the requested parameter.
 * @param <value> string value for the parameter.
 *
 * @return  The value of the parameter at the completion of the call.
 */
//------------------------------------------------------------------------------
bool Optimizer::SetStringParameter(const Integer id,
                                   const wxString &value)
{
    if (id == OBJECTIVE_FUNCTION) 
    {
        objectiveFnName = value;
        return true;
    }
        
    
    return Solver::SetStringParameter(id, value);
}

// compiler complained again - so here they are ....
wxString Optimizer::GetStringParameter(const wxString& label) const
{
   return Solver::GetStringParameter(label);
}
bool Optimizer::SetStringParameter(const wxString& label,
                                           const wxString &value)
{
   return Solver::SetStringParameter(label, value);
}
wxString Optimizer::GetStringParameter(const Integer id,
                                                  const Integer index) const
{
   return Solver::GetStringParameter(id, index);
}

bool Optimizer::SetStringParameter(const Integer id, 
                                           const wxString &value,
                                           const Integer index)
{
   return Solver::SetStringParameter(id, value, index);
}

wxString Optimizer::GetStringParameter(const wxString &label,
                                                  const Integer index) const
{
   return Solver::GetStringParameter(label, index);
}

bool Optimizer::SetStringParameter(const wxString &label, 
                                           const wxString &value,
                                           const Integer index)
{
   return Solver::SetStringParameter(label, value, index);
}

//------------------------------------------------------------------------------
//  wxString  GetStringArrayParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return  StringArray value of the requested parameter.
 */
//------------------------------------------------------------------------------
const StringArray& Optimizer::GetStringArrayParameter(
                                                        const Integer id) const
{
        
    if (id == EQUALITY_CONSTRAINT_NAMES)
        return eqConstraintNames;
    if (id == INEQUALITY_CONSTRAINT_NAMES)
        return ineqConstraintNames;
        
    return Solver::GetStringArrayParameter(id);
}

//------------------------------------------------------------------------------
//  bool TakeAction(const wxString &action, const wxString &actionData)
//------------------------------------------------------------------------------
/**
 * This method performs an action on the instance.
 *
 * TakeAction is a method overridden from GmatBase.  The only actions defined for
 * an Optimizer are wxT("IncrementInstanceCount"), which the Sandbox uses
 * to tell an instance if if it is a reused instance (i.e. a clone) of the
 * configured instance of the Optimizer; and wxT("Reset") which resets the
 * optimizer data.
 *
 * @param <action>      Text label for the action.
 * @param <actionData>  Related action data, if needed.
 *
 * @return  The value of the parameter at the completion of the call.
 */
//------------------------------------------------------------------------------
bool Optimizer::TakeAction(const wxString &action,
                                       const wxString &actionData)
{
   if (action == wxT("IncrementInstanceCount"))
   {
      ++instanceNumber;
      return true;
   }
 
   if (action == wxT("Reset"))
   {
      currentState = INITIALIZING;
   }

   return Solver::TakeAction(action, actionData);
}



//------------------------------------------------------------------------------
// protected methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  void FreeArrays()
//------------------------------------------------------------------------------
/**
 * Frees (clears) the constraints used by the optimizer.  This method is also 
 * called by the destructor when the script is cleared.
 */
//------------------------------------------------------------------------------
void Optimizer::FreeArrays()
{
   Solver::FreeArrays();
   //eqConstraintNames.clear(); ????? do I need to do this?
   //ineqConstraintNames.clear(); ?????
   //eqConstraintCount - 0; ?????
   //ineqConstraintCount - 0; ?????
   eqConstraintValues.clear();
   ineqConstraintValues.clear();
    
}
