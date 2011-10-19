//$Id: DifferentialCorrector.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                         DifferentialCorrector
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
// Author: Darrel J. Conway, Thinking Systems, Inc.
// Created: 2003/12/29
//
/**
 * Implementation for the differential corrector targeter.
 *
 * @todo Rework the mathematics using Rvector code.
 */
//------------------------------------------------------------------------------


#include "DifferentialCorrector.hpp"
#include "Rmatrix.hpp"
#include "RealUtilities.hpp"     // for GmatMathUtil::Abs()
#include "MessageInterface.hpp"

#include <cmath>
#include <sstream>

//#define DEBUG_STATE_MACHINE
//#define DEBUG_DC_INIT 1
//#define DEBUG_JACOBIAN
//#define DEBUG_VARIABLES_CALCS
//#define DEBUG_TARGETING_MODES

// Turn on other debug if working on modes
#ifdef DEBUG_TARGETING_MODES
   #define DEBUG_STATE_MACHINE
#endif

//---------------------------------
// static data
//---------------------------------

const wxString
DifferentialCorrector::PARAMETER_TEXT[DifferentialCorrectorParamCount -
                                      SolverParamCount] =
{
   wxT("Goals"),
   wxT("DerivativeMethod")
};

const Gmat::ParameterType
DifferentialCorrector::PARAMETER_TYPE[DifferentialCorrectorParamCount -
                                      SolverParamCount] =
{
   Gmat::STRINGARRAY_TYPE,
   Gmat::ENUMERATION_TYPE
};


//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// DifferentialCorrector(wxString name)
//------------------------------------------------------------------------------
DifferentialCorrector::DifferentialCorrector(wxString name) :
   Solver                  (wxT("DifferentialCorrector"), name),
   goalCount               (0),
   goal                    (NULL),
   tolerance               (NULL),
   nominal                 (NULL),
   achieved                (NULL),
   backAchieved            (NULL),
   jacobian                (NULL),
   inverseJacobian         (NULL),
   indx                    (NULL),
   b                       (NULL),
   derivativeMethod        (wxT("ForwardDifference")),
   diffMode                (1),
   firstPert               (true),
   incrementPert           (true)
{
   #if DEBUG_DC_INIT
   MessageInterface::ShowMessage
      (wxT("DifferentialCorrector::DC(constructor) entered\n"));
   #endif
   objectTypeNames.push_back(wxT("DifferentialCorrector"));
   parameterCount = DifferentialCorrectorParamCount;
   
   AllowScaleFactors = false;
}


//------------------------------------------------------------------------------
// DifferentialCorrector::~DifferentialCorrector()
//------------------------------------------------------------------------------
DifferentialCorrector::~DifferentialCorrector()
{
   FreeArrays();
}


//------------------------------------------------------------------------------
// DifferentialCorrector(const DifferentialCorrector &dc) :
//------------------------------------------------------------------------------
DifferentialCorrector::DifferentialCorrector(const DifferentialCorrector &dc) :
   Solver                  (dc),
   goalCount               (dc.goalCount),
   goal                    (NULL),
   tolerance               (NULL),
   nominal                 (NULL),
   achieved                (NULL),
   backAchieved            (NULL),
   jacobian                (NULL),
   inverseJacobian         (NULL),
   indx                    (NULL),
   b                       (NULL),
   derivativeMethod        (dc.derivativeMethod),
   diffMode                (dc.diffMode),
   firstPert               (dc.firstPert),
   incrementPert           (dc.incrementPert)
{
   #if DEBUG_DC_INIT
   MessageInterface::ShowMessage
      (wxT("DifferentialCorrector::DC(COPY constructor) entered\n"));
   #endif
   goalNames.clear();

   parameterCount = dc.parameterCount;
}


//------------------------------------------------------------------------------
// operator=(const DifferentialCorrector& dc)
//------------------------------------------------------------------------------
DifferentialCorrector&
DifferentialCorrector::operator=(const DifferentialCorrector& dc)
{
   if (&dc == this)
      return *this;

   Solver::operator=(dc);

   FreeArrays();
   goalNames.clear();

   goalCount        = dc.goalCount;
   derivativeMethod = dc.derivativeMethod;
   diffMode         = dc.diffMode;
   firstPert        = dc.firstPert;
   incrementPert    = dc.incrementPert;

   return *this;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the DifferentialCorrector.
 *
 * @return clone of the DifferentialCorrector.
 */
//------------------------------------------------------------------------------
GmatBase* DifferentialCorrector::Clone() const
{
   GmatBase *clone = new DifferentialCorrector(*this);
   return (clone);
}


//---------------------------------------------------------------------------
//  void Copy(const GmatBase* orig)
//---------------------------------------------------------------------------
/**
 * Sets this object to match another one.
 *
 * @param orig The original that is being copied.
 */
//---------------------------------------------------------------------------
void DifferentialCorrector::Copy(const GmatBase* orig)
{
   operator=(*((DifferentialCorrector *)(orig)));
}

// Access methods overriden from the base class

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
wxString DifferentialCorrector::GetParameterText(const Integer id) const
{
   if ((id >= SolverParamCount) && (id < DifferentialCorrectorParamCount))
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
Integer DifferentialCorrector::GetParameterID(const wxString &str) const
{
   // Write deprecated message per GMAT session
   static bool writeDeprecatedMsg = true;

   // 1. This part will be removed for a future build:
   if (str == wxT("UseCentralDifferences"))
   {
      if (writeDeprecatedMsg)
      {
         MessageInterface::ShowMessage
            (deprecatedMessageFormat.c_str(), wxT("UseCentralDifferences"), GetName().c_str(),
             wxT("DerivativeMethod"));
         writeDeprecatedMsg = false;
      }
      return derivativeMethodID;
   }

   // 2. This part is kept for a future build:
   for (Integer i = SolverParamCount; i < DifferentialCorrectorParamCount; ++i)
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
Gmat::ParameterType DifferentialCorrector::GetParameterType(
                                              const Integer id) const
{
   if ((id >= SolverParamCount) && (id < DifferentialCorrectorParamCount))
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
wxString DifferentialCorrector::GetParameterTypeString(
                                      const Integer id) const
{
   return Solver::PARAM_TYPE_STRING[GetParameterType(id)];
}

//------------------------------------------------------------------------------
//  Integer  GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns an Integer parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return  Integer value of the requested parameter.
 */
//------------------------------------------------------------------------------
Integer DifferentialCorrector::GetIntegerParameter(const Integer id) const
{
   //if (id == maxIterationsID)
   //   return maxIterations;

   return Solver::GetIntegerParameter(id);
}


//------------------------------------------------------------------------------
//  Integer SetIntegerParameter(const Integer id, const Integer value)
//------------------------------------------------------------------------------
/**
 * This method sets an Integer parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 * @param <value> Integer value for the parameter.
 *
 * @return  The value of the parameter at the completion of the call.
 */
//------------------------------------------------------------------------------
Integer DifferentialCorrector::SetIntegerParameter(const Integer id,
                                                   const Integer value)
{
   //if (id == maxIterationsID)
   //{
   //   if (value > 0)
   //      maxIterations = value;
   //   else
   //      MessageInterface::ShowMessage(
   //         wxT("Iteration count for %s must be > 0; requested value was %d\n"),
   //         instanceName.c_str(), value);
   //   return maxIterations;
   //}

   return Solver::SetIntegerParameter(id, value);
}


//------------------------------------------------------------------------------
//  bool  GetBooleanParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the Boolean parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return  Boolean value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
bool DifferentialCorrector::GetBooleanParameter(const Integer id) const
{
//    if (id == useCentralDifferencingID)
//        return useCentralDifferences;

    return Solver::GetBooleanParameter(id);
}


//------------------------------------------------------------------------------
//  Integer SetBooleanParameter(const Integer id, const bool value)
//------------------------------------------------------------------------------
/**
 * This method sets a Boolean parameter value, given the input
 * parameter ID.
 *
 * @param <id>    ID for the requested parameter.
 * @param <value> Boolean value for the parameter.
 *
 * @return  The value of the parameter at the completion of the call.
 */
//------------------------------------------------------------------------------
bool DifferentialCorrector::SetBooleanParameter(const Integer id,
                                                const bool value)
{
//   if (id == useCentralDifferencingID)
//   {
//      useCentralDifferences = value;
//      return useCentralDifferences;
//   }

   return Solver::SetBooleanParameter(id, value);
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
wxString DifferentialCorrector::GetStringParameter(const Integer id) const
{
    //if (id == solverTextFileID)
    //    return solverTextFile;

   if (id == derivativeMethodID)
      return derivativeMethod;

   return Solver::GetStringParameter(id);
}


//------------------------------------------------------------------------------
//  Integer SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
/**
 * This method sets a string or string array parameter value, given the input
 * parameter ID.
 *
 * @param id    ID for the requested parameter.
 * @param value string value for the parameter.
 *
 * @return  The value of the parameter at the completion of the call.
 */
//------------------------------------------------------------------------------
bool DifferentialCorrector::SetStringParameter(const Integer id,
                                               const wxString &value)
{
   if (id == goalNamesID)
   {
      goalNames.push_back(value);
      return true;
   }

   if (id == derivativeMethodID)
   {
      bool retval = true;
      //   This is to handle deprecated value UseCentralDifferences = true
      if (value == wxT("true"))
         derivativeMethod = wxT("CentralDifference");
      //   This is to handle deprecated value UseCentralDifferences = false
      else if (value == wxT("false"))
         derivativeMethod = wxT("ForwardDifference");
      // Allowed values for DerivativeMethod
      else if (value == wxT("ForwardDifference") || value == wxT("CentralDifference") ||
               value == wxT("BackwardDifference"))
      {
         derivativeMethod = value;
         if (derivativeMethod == wxT("ForwardDifference"))
         {
            diffMode = 1;
         }
         else if(derivativeMethod == wxT("CentralDifference"))
         {
            diffMode = 0;
         }
         else if(derivativeMethod == wxT("BackwardDifference"))
         {
            diffMode = -1;
         }
      }
      //  All other values are not allowed!
      else
         retval = false;

      return retval;
   }

   return Solver::SetStringParameter(id, value);
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
const StringArray& DifferentialCorrector::GetStringArrayParameter(
                                                        const Integer id) const
{
    //if (id == variableNamesID)
    //    return variableNames;

    if (id == goalNamesID)
        return goalNames;

    return Solver::GetStringArrayParameter(id);
}


//------------------------------------------------------------------------------
//  bool TakeAction(const wxString &action, const wxString &actionData)
//------------------------------------------------------------------------------
/**
 * This method performs an action on the instance.
 *
 * TakeAction is a method overridden from GmatBase.  The only action defined for
 * a DifferentialCorrector is wxT("IncrementInstanceCount"), which the Sandbox uses
 * to tell an instance if if it is a reused instance (i.e. a clone) of the
 * configured instance of the DifferentialCorrector.
 *
 * @param <action>      Text label for the action.
 * @param <actionData>  Related action data, if needed.
 *
 * @return  The value of the parameter at the completion of the call.
 */
//------------------------------------------------------------------------------
bool DifferentialCorrector::TakeAction(const wxString &action,
                                       const wxString &actionData)
{
   if (action == wxT("ResetInstanceCount"))
   {
      instanceNumber = 0;
      return true;
   }

   if (action == wxT("IncrementInstanceCount"))
   {
      ++instanceNumber;
      return true;
   }

   if (action == wxT("Reset"))
   {
      currentState = INITIALIZING;
      // initialized = false;
      // Set nominal out of range to force retarget when in a loop
      for (Integer i = 0; i < goalCount; ++i)
      {
         nominal[i] = goal[i] + 10.0 * tolerance[i];
      }
   }

   if (action == wxT("SetMode"))
   {
      currentState = INITIALIZING;
      // initialized = false;
      // Set nominal out of range to force retarget when in a loop
      for (Integer i = 0; i < goalCount; ++i)
      {
         nominal[i] = goal[i] + 10.0 * tolerance[i];
      }
   }

   return Solver::TakeAction(action, actionData);
}


//------------------------------------------------------------------------------
// Integer SetSolverResults(Real *data, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Sets up the data fields used for the results of an iteration.
 *
 * @param <data> An array of data appropriate to the results used in the
 *               algorithm (for instance, tolerances for targeter goals).
 * @param <name> A label for the data parameter.  Defaults to the empty
 *               string.
 *
 * @return The ID used for this variable.
 */
//------------------------------------------------------------------------------
Integer DifferentialCorrector::SetSolverResults(Real *data,
                                                const wxString &name,
                                                const wxString &type)
{
    if (goalNames[goalCount] != name)
        throw SolverException(wxT("Mismatch between parsed and configured goal"));
    goal[goalCount] = data[0];
    tolerance[goalCount] = data[1];
    ++goalCount;
    return goalCount-1;
}


//------------------------------------------------------------------------------
// bool UpdateSolverGoal(Integer id, Real newValue)
//------------------------------------------------------------------------------
/**
 * Updates the targeter goals, for floating end point targeting.
 *
 * @param <id>       Id for the goal that is being reset.
 * @param <newValue> The new goal value.
 *
 * @return true on success, throws on failure.
 */
//------------------------------------------------------------------------------
bool DifferentialCorrector::UpdateSolverGoal(Integer id, Real newValue)
{
   // Only update during nominal runs
   if (currentState == NOMINAL) {
      if (id >= goalCount)
         throw SolverException(
            wxT("DifferentialCorrector member requested a parameter outside the ")
            wxT("range of the configured goals."));

      goal[id] = newValue;
   }
   return true;
}


//------------------------------------------------------------------------------
// bool UpdateSolverTolerance(Integer id, Real newValue)
//------------------------------------------------------------------------------
/**
 * Updates the targeter tolerances, for floating end point targeting.
 *
 * @param <id>       Id for the tolerance that is being reset.
 * @param <newValue> The new tolerance value.
 *
 * @return true on success, throws on failure.
 */
//------------------------------------------------------------------------------
bool DifferentialCorrector::UpdateSolverTolerance(Integer id, Real newValue)
{
   // Only update during nominal runs
   if (currentState == NOMINAL) {
      if (id >= goalCount)
         throw SolverException(
            wxT("DifferentialCorrector member requested a parameter outside the ")
            wxT("range of the configured goals."));

      tolerance[id] = newValue;
   }
   return true;
}


//------------------------------------------------------------------------------
// void SetResultValue(Integer id, Real value)
//------------------------------------------------------------------------------
/**
 * Passes in the results obtained from a run in the DifferentialCorrector loop.
 *
 * @param <id>    The ID used for this result.
 * @param <value> The corresponding result.
 */
//------------------------------------------------------------------------------
void DifferentialCorrector::SetResultValue(Integer id, Real value,
                                           const wxString &resultType)
{
   #ifdef DEBUG_STATE_MACHINE
      MessageInterface::ShowMessage(
            wxT("   State %d received id %d    value = %.12lf\n"), currentState, id,
            value);
   #endif
    if (currentState == NOMINAL)
    {
        nominal[id] = value;
    }

    if (currentState == PERTURBING)
    {
       if (firstPert)
          achieved[pertNumber][id] = value;
       else
          backAchieved[pertNumber][id] = value;
    }
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 * Initializes the DifferentialCorrector prior to targeting.
 */
//------------------------------------------------------------------------------
bool DifferentialCorrector::Initialize()
{
   // Setup the variable data structures
   Integer localVariableCount = variableNames.size();
   Integer localGoalCount = goalNames.size();

   #if DEBUG_DC_INIT
   MessageInterface::ShowMessage
      (wxT("DifferentialCorrector::Initialize() localVariableCount=%d, ")
       wxT("localGoalCount=%d\n"), localVariableCount, localGoalCount);
   #endif

   if (localVariableCount == 0 || localGoalCount == 0)
   {
      wxString errorMessage = wxT("Targeter cannot initialize: ");
      errorMessage += wxT("No goals or variables are set.\n");
      throw SolverException(errorMessage);
   }

   FreeArrays();

   // Setup the goal data structures
   goal      = new Real[localGoalCount];
   tolerance = new Real[localGoalCount];
   nominal   = new Real[localGoalCount];

   // And the sensitivity matrix
   Integer i;
   achieved        = new Real*[localVariableCount];
   backAchieved    = new Real*[localVariableCount];
   jacobian        = new Real*[localVariableCount];
   for (i = 0; i < localVariableCount; ++i)
   {
      jacobian[i]        = new Real[localGoalCount];
      achieved[i]        = new Real[localGoalCount];
      backAchieved[i]    = new Real[localGoalCount];
   }

   inverseJacobian = new Real*[localGoalCount];
   for (i = 0; i < localGoalCount; ++i)
   {
      inverseJacobian[i] = new Real[localVariableCount];
   }

   Solver::Initialize();

   // Allocate the LU arrays
   indx = new Integer[variableCount];
   b = new Real[variableCount];

   #if DEBUG_DC_INIT
      MessageInterface::ShowMessage
            (wxT("DifferentialCorrector::Initialize() completed\n"));
   #endif

   return true;
}


//------------------------------------------------------------------------------
//  Solver::SolverState AdvanceState()
//------------------------------------------------------------------------------
/**
 * The method used to walk the DifferentialCorrector through its state machine.
 *
 * @return solver state at the end of the process.
 */
//------------------------------------------------------------------------------
Solver::SolverState DifferentialCorrector::AdvanceState()
{
   switch (currentMode)
   {
      case INITIAL_GUESS:
         #ifdef DEBUG_TARGETING_MODES
            MessageInterface::ShowMessage(
                  wxT("Running in INITIAL_GUESS mode; state = %d\n"), currentState);
         #endif
            switch (currentState)
            {
               case INITIALIZING:
                  #ifdef DEBUG_STATE_MACHINE
                     MessageInterface::ShowMessage(
                           wxT("Entered state machine; INITIALIZING\n"));
                  #endif
                  iterationsTaken = 0;
                  WriteToTextFile();
                  ReportProgress();
                  CompleteInitialization();
                  status = INITIALIZED;
                  break;

               case NOMINAL:
                  #ifdef DEBUG_STATE_MACHINE
                     MessageInterface::ShowMessage(
                           wxT("Entered state machine; NOMINAL\n"));
                  #endif
                  WriteToTextFile();
                  currentState = FINISHED;
                  status = RUN;
                  break;

               case FINISHED:
               default:
                  #ifdef DEBUG_STATE_MACHINE
                     MessageInterface::ShowMessage(
                           wxT("Entered state machine; FINISHED\n"));
                  #endif
                  RunComplete();
                  ReportProgress();
                  break;
            }
         break;

      case SOLVE:
      default:
         #ifdef DEBUG_TARGETING_MODES
            MessageInterface::ShowMessage(
                  wxT("Running in SOLVE or default mode; state = %d\n"),
                  currentState);
         #endif
         switch (currentState)
         {
            case INITIALIZING:
               #ifdef DEBUG_STATE_MACHINE
                  MessageInterface::ShowMessage(
                        wxT("Entered state machine; INITIALIZING\n"));
               #endif
               iterationsTaken = 0;
               WriteToTextFile();
               ReportProgress();
               CompleteInitialization();
               status = INITIALIZED;
               break;

            case NOMINAL:
               #ifdef DEBUG_STATE_MACHINE
                  MessageInterface::ShowMessage(
                        wxT("Entered state machine; NOMINAL\n"));
               #endif
               ReportProgress();
               RunNominal();
               ReportProgress();
               status = RUN;
               break;

            case PERTURBING:
               #ifdef DEBUG_STATE_MACHINE
                  MessageInterface::ShowMessage(
                        wxT("Entered state machine; PERTURBING\n"));
               #endif
               ReportProgress();
               RunPerturbation();
               break;

            case CALCULATING:
               #ifdef DEBUG_STATE_MACHINE
                  MessageInterface::ShowMessage(
                        wxT("Entered state machine; CALCULATING\n"));
               #endif
               ReportProgress();
               CalculateParameters();
               break;

            case CHECKINGRUN:
                #ifdef DEBUG_STATE_MACHINE
                  MessageInterface::ShowMessage(
                        wxT("Entered state machine; CHECKINGRUN\n"));
               #endif
               CheckCompletion();
               ++iterationsTaken;
               if (iterationsTaken >= maxIterations)
               {
                  MessageInterface::ShowMessage(
                        wxT("Differential corrector %s %s\n"), instanceName.c_str(),
                        wxT("has exceeded the maximum number of allowed ")
                        wxT("iterations."));
                  currentState = FINISHED;
               }
               break;

            case FINISHED:
               #ifdef DEBUG_STATE_MACHINE
                  MessageInterface::ShowMessage(
                        wxT("Entered state machine; FINISHED\n"));
               #endif
               RunComplete();
               ReportProgress();
               break;

            case ITERATING:             // Intentional drop-through
            default:
               #ifdef DEBUG_STATE_MACHINE
                  MessageInterface::ShowMessage(wxT("Entered state machine; ")
                     wxT("Bad state for a differential corrector.\n"));
               #endif
               throw SolverException(
                     wxT("Solver state not supported for the targeter"));
         }
         break;
   }

   return currentState;
}


//------------------------------------------------------------------------------
//  void RunNominal()
//------------------------------------------------------------------------------
/**
 * Run out the nominal sequence, generating the wxT("current") targeter data.
 */
//------------------------------------------------------------------------------
void DifferentialCorrector::RunNominal()
{
   // On success, set the state to the next machine state
   WriteToTextFile();
   currentState = CHECKINGRUN;
}


//------------------------------------------------------------------------------
//  void RunPerturbation()
//------------------------------------------------------------------------------
/**
 * Run out a perturbation, generating data used to evaluate the Jacobian.
 */
//------------------------------------------------------------------------------
void DifferentialCorrector::RunPerturbation()
{
   // Calculate the perts, one at a time
   if (pertNumber != -1)
      // Back out the last pert applied
      variable.at(pertNumber) = lastUnperturbedValue;
   if (incrementPert)
      ++pertNumber;

   if (pertNumber == variableCount)  // Current set of perts have been run
   {
      currentState = CALCULATING;
      pertNumber = -1;
      return;
   }

   lastUnperturbedValue = variable.at(pertNumber);
   if (diffMode == 1)      // Forward difference
   {
      firstPert = true;
      variable.at(pertNumber) += perturbation.at(pertNumber);
      pertDirection.at(pertNumber) = 1.0;
   }
   else if (diffMode == 0) // Central difference
   {
      if (incrementPert)
      {
         firstPert = true;
         incrementPert = false;
         variable.at(pertNumber) += perturbation.at(pertNumber);
         pertDirection.at(pertNumber) = 1.0;
      }
      else
      {
         firstPert = false;
         incrementPert = true;
         variable.at(pertNumber) -= perturbation.at(pertNumber);
         pertDirection.at(pertNumber) = -1.0;
      }
   }
   else                    // Backward difference
   {
      firstPert = true;
      variable.at(pertNumber) -= perturbation.at(pertNumber);
      pertDirection.at(pertNumber) = -1.0;
   }

   if (variable[pertNumber] > variableMaximum[pertNumber])
   {
      if (diffMode == 0)
      {
         // Warn user that central differencing violates constraint and continue
         MessageInterface::ShowMessage(wxT("Warning!  Perturbation violates the ")
               wxT("maximum value for variable %s, but is being applied anyway to ")
               wxT("perform central differencing in the differential corrector ")
               wxT("%s\n"), variableNames[pertNumber].c_str(), instanceName.c_str());
      }
      else
      {
         pertDirection.at(pertNumber) = -1.0;
         variable[pertNumber] -= 2.0 * perturbation[pertNumber];
      }
   }

   if (variable[pertNumber] < variableMinimum[pertNumber])
   {
      if (diffMode == 0)
      {
         // Warn user that central differencing violates constraint and continue
         MessageInterface::ShowMessage(wxT("Warning!  Perturbation violates the ")
               wxT("minimum value for variable %s, but is being applied anyway to ")
               wxT("perform central differencing in the differential corrector ")
               wxT("%s\n"), variableNames[pertNumber].c_str(), instanceName.c_str());
      }
      else
      {
         pertDirection.at(pertNumber) = -1.0;
         variable[pertNumber] -= 2.0 * perturbation[pertNumber];
      }
   }

   WriteToTextFile();
}


//------------------------------------------------------------------------------
//  void CalculateParameters()
//------------------------------------------------------------------------------
/**
 * Updates the values for the variables based on the inverted Jacobian.
 */
//------------------------------------------------------------------------------
void DifferentialCorrector::CalculateParameters()
{
   // Build and invert the sensitivity matrix
   CalculateJacobian();
   InvertJacobian();

   std::vector<Real> delta;

   // Apply the inverse Jacobian to build the next set of variables
   for (Integer i = 0; i < variableCount; ++i)
   {
      delta.push_back(0.0);
      for (Integer j = 0; j < goalCount; j++)
         delta[i] += inverseJacobian[j][i] * (goal[j] - nominal[j]);
   }

   Real multiplier = 1.0, maxDelta;

   // First validate the variable changes
   for (Integer i = 0; i < variableCount; ++i)
   {
      if (fabs(delta.at(i)) > variableMaximumStep.at(i))
      {
         maxDelta = fabs(variableMaximumStep.at(i) / delta.at(i));
         if (maxDelta < multiplier)
            multiplier = maxDelta;
      }
   }

   #ifdef DEBUG_VARIABLES_CALCS
      MessageInterface::ShowMessage(wxT("Variable Values; Multiplier = %.15lf\n"),
            multiplier);
   #endif

   for (Integer i = 0; i < variableCount; ++i)
   {
      // Ensure that delta is not larger than the max allowed step
      try
      {
         #ifdef DEBUG_VARIABLES_CALCS
            MessageInterface::ShowMessage(
                  wxT("   %d:  %.15lf  +  %.15lf  *  %.15lf"), i, variable.at(i),
                  delta.at(i), multiplier);
         #endif
         variable.at(i) += delta.at(i) * multiplier;
         #ifdef DEBUG_VARIABLES_CALCS
            MessageInterface::ShowMessage(wxT("  ->  %.15lf\n"), variable.at(i));
         #endif

         // Ensure that variable[i] is in the allowed range
         if (variable.at(i) < variableMinimum.at(i))
            variable.at(i) = variableMinimum.at(i);
         if (variable.at(i) > variableMaximum.at(i))
            variable.at(i) = variableMaximum.at(i);
      }
      catch(std::exception &)
      {
         throw SolverException(wxT("Range error in Solver::CalculateParameters\n"));
      }
   }

   WriteToTextFile();
   currentState = NOMINAL;
}


//------------------------------------------------------------------------------
//  void CheckCompletion()
//------------------------------------------------------------------------------
/**
 * Determine whether or not the targeting run has converged.
 */
//------------------------------------------------------------------------------
void DifferentialCorrector::CheckCompletion()
{
   WriteToTextFile();
   bool converged = true;          // Assume convergence

   // check for lack of convergence
   for (Integer i = 0; i < goalCount; ++i)
   {
      if (GmatMathUtil::Abs(nominal[i] - goal[i]) > tolerance[i])
         converged = false;
   }

   if (!converged)
   {
      if (iterationsTaken < maxIterations-1)
      {
         // Set to run perts if not converged
         pertNumber = -1;
         // Build the first perturbation
         currentState = PERTURBING;
         RunPerturbation();
      }
      else
      {
         currentState = FINISHED;
         status = EXCEEDED_ITERATIONS;
      }
   }
   else
   {
      // If converged, we're done
      currentState = FINISHED;
      status = CONVERGED;
   }
}


//------------------------------------------------------------------------------
//  void RunComplete()
//------------------------------------------------------------------------------
/**
 * Updates the targeter text file at the end of a targeter run.
 */
//------------------------------------------------------------------------------
void DifferentialCorrector::RunComplete()
{
    WriteToTextFile();
}


//------------------------------------------------------------------------------
//  void CalculateJacobian()
//------------------------------------------------------------------------------
/**
 * Calculates the matrix of derivatives of the goals with respect to the
 * variables.
 */
//------------------------------------------------------------------------------
void DifferentialCorrector::CalculateJacobian()
{
   Integer i, j;

   if (diffMode != 0)
   {
      for (i = 0; i < variableCount; ++i)
      {
         for (j = 0; j < goalCount; ++j)
         {
             jacobian[i][j] = achieved[i][j] - nominal[j];
             jacobian[i][j] /= (pertDirection.at(i) * perturbation.at(i));
         }
      }
   }
   else        // Central differencing
   {
      for (i = 0; i < variableCount; ++i)
      {
         for (j = 0; j < goalCount; ++j)
         {
             jacobian[i][j] = achieved[i][j] - backAchieved[i][j]; // nominal[j];
             jacobian[i][j] /= (2.0 * perturbation.at(i));
         }
      }
   }
}


//------------------------------------------------------------------------------
//  void InvertJacobian()
//------------------------------------------------------------------------------
/**
 * Inverts the matrix of derivatives so that the change in the variables can be
 * estimated.
 */
//------------------------------------------------------------------------------
void DifferentialCorrector::InvertJacobian()
{
   #ifdef DEBUG_JACOBIAN
      MessageInterface::ShowMessage(wxT("Inverting %d by %d Jacobian\n"),
            variableCount, goalCount);
   #endif
   Rmatrix jac(variableCount, goalCount);
   for (Integer i = 0; i < variableCount; ++i)
      for (Integer j = 0; j < goalCount; ++j)
      {
         #ifdef DEBUG_JACOBIAN
            MessageInterface::ShowMessage(wxT("   jacobian[%d][%d] = %.14lf\n"), i,
                  j, jacobian[i][j]);
         #endif
         jac(i,j) = jacobian[i][j];
      }

   Rmatrix inv;
   if (variableCount == goalCount)
      inv = jac.Inverse();
   else
      inv = jac.Pseudoinverse();

   #ifdef DEBUG_JACOBIAN
      MessageInterface::ShowMessage(wxT("Inverse Jacobian is %d by %d\n"),
            variableCount, goalCount);
   #endif

   #ifdef DEBUG_DC_INVERSIONS
      wxString preface = wxT("   ");
      if (variableCount == goalCount)
         MessageInterface::ShowMessage(wxT("Inverse:\n%s\n"),
               (inv.ToString(16, false, preface).c_str()));
      else
         MessageInterface::ShowMessage(wxT("PseudoInverse:\n%s\n"),
               inv.ToString(16, false, preface).c_str());
   #endif

   for (Integer i = 0; i < goalCount; ++i)
      for (Integer j = 0; j < variableCount; ++j)
      {
         inverseJacobian[i][j] = inv(i,j);
         #ifdef DEBUG_JACOBIAN
            MessageInterface::ShowMessage(
                  wxT("   inverseJacobian[%d][%d] = %.14lf\n"), i, j,
                  inverseJacobian[i][j]);
         #endif
      }
}


//------------------------------------------------------------------------------
//  void FreeArrays()
//------------------------------------------------------------------------------
/**
 * Frees the memory used by the targeter, so it can be reused later in the
 * sequence.  This method is also called by the destructor when the script is
 * cleared.
 */
//------------------------------------------------------------------------------
void DifferentialCorrector::FreeArrays()
{
   Solver::FreeArrays();

   if (goal)
   {
      delete [] goal;
      goal = NULL;
   }

   if (tolerance)
   {
      delete [] tolerance;
      tolerance = NULL;
   }

   if (nominal)
   {
      delete [] nominal;
      nominal = NULL;
   }

   if (achieved)
   {
      for (Integer i = 0; i < variableCount; ++i)
         delete [] achieved[i];
      delete [] achieved;
      achieved = NULL;
   }

   if (backAchieved)
   {
      for (Integer i = 0; i < variableCount; ++i)
         delete [] backAchieved[i];
      delete [] backAchieved;
      backAchieved = NULL;
   }

   if (jacobian)
   {
      for (Integer i = 0; i < variableCount; ++i)
         delete [] jacobian[i];
      delete [] jacobian;
      jacobian = NULL;
   }

   if (inverseJacobian)
   {
      for (Integer i = 0; i < goalCount; ++i)
         delete [] inverseJacobian[i];
      delete [] inverseJacobian;
      inverseJacobian = NULL;
   }

   if (indx)
   {
      delete [] indx;
      indx = NULL;
   }

   if (b)
   {
      delete [] b;
      b = NULL;
   }
}


//------------------------------------------------------------------------------
//  wxString GetProgressString()
//------------------------------------------------------------------------------
/**
 * Generates a string that reporting the current differential corrector state.
 */
//------------------------------------------------------------------------------
wxString DifferentialCorrector::GetProgressString()
{
   StringArray::iterator current;
   Integer i;
   wxString progress;

   if (initialized)
   {
      switch (currentState)
      {
         case INITIALIZING:
            // This state is basically a wxT("paused state") used for the Target
            // command to finalize the initial data for the variables and
            // goals.  All that is written here is the header information.
            {
               Integer localVariableCount = variableNames.size(),
                       localGoalCount = goalNames.size();
               progress << wxT("************************************************")
                        << wxT("********\n")
                        << wxT("*** Performing Differential Correction ")
                        << wxT("(using \"") << instanceName << wxT("\")\n");

               // Write out the setup data
               progress << wxT("*** ") << localVariableCount << wxT(" variables; ")
                        << localGoalCount << wxT(" goals\n   Variables:  ");

               // Iterate through the variables and goals, writing them to
               // the file
               for (current = variableNames.begin(), i = 0;
                    current != variableNames.end(); ++current)
               {
                  if (current != variableNames.begin())
                     progress << wxT(", ");
                  progress << *current;
               }

               progress << wxT("\n   Goals:  ");

               for (current = goalNames.begin(), i = 0;
                    current != goalNames.end(); ++current)
               {
                  if (current != goalNames.begin())
                     progress << wxT(", ");
                  progress << *current;
               }

               if (solverMode != wxT(""))
                  progress << wxT("\n   SolverMode:  ")
                           << solverMode;


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
               progress << *current << wxT(" = ") << unscaledVariable.at(i);
               if (textFileMode == wxT("Verbose"))
                  progress << wxT("; targeter scaled value: ") << variable[i];
               ++i;
            }
            break;

         case PERTURBING:
            progress << wxT("   Completed iteration ") << iterationsTaken
                     << wxT(", pert ") << pertNumber+1 << wxT(" (")
                     << variableNames[pertNumber] << wxT(" = ")
                     << unscaledVariable.at(pertNumber);
            if (textFileMode == wxT("Verbose"))
               progress << wxT("; targeter scaled value: ") << variable[pertNumber];
            progress << wxT(")");
            break;

         case CALCULATING:
            // Just forces a blank line
            break;

         case CHECKINGRUN:
            // Iterate through the goals, writing them to the file
            progress << wxT("   Goals and achieved values:\n");

            for (current = goalNames.begin(), i = 0;
                 current != goalNames.end(); ++current)
            {
               progress << wxT("      ") << *current
                        << wxT("  Desired: ") << goal[i]
                        << wxT("  Achieved: ") << nominal[i]
                        << wxT("  Variance: ") << (goal[i] - nominal[i])
                        << wxT("\n");
               ++i;
            }

            break;

         case FINISHED:
            switch (currentMode)
            {
               case INITIAL_GUESS:
                  progress << wxT("\n*** Targeting Completed Initial Guess Run\n")
                           << wxT("***\n   Variable Values:\n");
                  for (current = variableNames.begin(), i = 0;
                       current != variableNames.end(); ++current)
                     progress << wxT("      ") << *current
                              << wxT(" = ") << unscaledVariable.at(i++) << wxT("\n");
                  progress << wxT("\n   Goal Values:\n");
                  for (current = goalNames.begin(), i = 0;
                       current != goalNames.end(); ++current)
                  {
                     progress << wxT("      ") << *current
                              << wxT("  Desired: ") << goal[i]
                              << wxT("  Achieved: ") << nominal[i]
                              << wxT("  Variance: ") << (goal[i] - nominal[i])
                              << wxT("\n");
                     ++i;
                  }
                  break;

               case SOLVE:
               default:
                  progress << wxT("\n*** Targeting Completed in ") << iterationsTaken
                           << wxT(" iterations");

                  if (iterationsTaken > maxIterations)
                     progress << wxT("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
                           << wxT("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n")
                           << wxT("!!! WARNING: Targeter did NOT converge!")
                           << wxT("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
                           << wxT("!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

                  progress << wxT("\nFinal Variable values:\n");
                  // Iterate through the variables, writing them to the string
                  for (current = variableNames.begin(), i = 0;
                       current != variableNames.end(); ++current)
                     progress << wxT("   ") << *current << wxT(" = ")
                              << unscaledVariable.at(i++) << wxT("\n");
            }
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
//  void WriteToTextFile()
//------------------------------------------------------------------------------
/**
 * Writes state data to the targeter text file.
 */
//------------------------------------------------------------------------------
void DifferentialCorrector::WriteToTextFile(SolverState stateToUse)
{
   #ifdef DEBUG_SOLVER_WRITE
   MessageInterface::ShowMessage
      (wxT("DC::WriteToTextFile() entered, stateToUse=%d, solverTextFile='%s', ")
       wxT("textFileOpen=%d, initialized=%d\n"), stateToUse, solverTextFile.c_str(),
       textFile.is_open(), initialized);
   #endif

   if (!showProgress)
      return;

   if (!textFile.is_open())
      OpenSolverTextFile();

   StringArray::iterator current;
   Integer i, j;
   if (initialized)
   {
      switch (currentState)
      {
         case INITIALIZING:
            // This state is basically a wxT("paused state") used for the Target
            // command to finalize the initial data for the variables and
            // goals.  All that is written here is the header information.
            {
               Integer localVariableCount = variableNames.size(),
                       localGoalCount = goalNames.size();
               textFile << wxT("************************************************")
                        << wxT("********\n")
                        << wxT("*** Targeter Text File\n")
                        << wxT("*** \n")
                        << wxT("*** Using Differential Correction\n***\n");

               // Write out the setup data
               textFile << wxT("*** ") << localVariableCount << wxT(" variables\n*** ")
                        << localGoalCount << wxT(" goals\n***\n*** ")
                        << wxT("Variables:\n***    ");

               // Iterate through the variables and goals, writing them to
               // the file
               for (current = variableNames.begin(), i = 0;
                    current != variableNames.end(); ++current)
               {
                  textFile << *current << wxT("\n***    ");
               }

               textFile << wxT("\n*** Goals:\n***    ");

               for (current = goalNames.begin(), i = 0;
                    current != goalNames.end(); ++current)
               {
                  textFile << *current << wxT("\n***    ");
               }

               if (solverMode != wxT(""))
                  textFile << wxT("\n*** SolverMode:  ")
                           << solverMode
                           <<wxT("\n***    ");

               textFile << wxT("\n****************************")
                        << wxT("****************************\n")
                        << std::endl;
            }
            break;

         case NOMINAL:
            textFile << wxT("Iteration ") << iterationsTaken+1
                     << wxT("\nRunning Nominal Pass\nVariables:\n   ");
            // Iterate through the variables, writing them to the file
            for (current = variableNames.begin(), i = 0;
                 current != variableNames.end(); ++current)
            {
               textFile << *current << wxT(" = ") << unscaledVariable.at(i);
               if ((textFileMode == wxT("Verbose")) || (textFileMode == wxT("Debug")))
                     textFile << wxT("; targeter scaled value: ") << variable.at(i);
               textFile << wxT("\n   ");
               ++i;
            }
            textFile << std::endl;
            break;

         case PERTURBING:
            if ((textFileMode == wxT("Verbose")) || (textFileMode == wxT("Debug")))
            {
               if (pertNumber != 0)
               {
                  // Iterate through the goals, writing them to the file
                  textFile << wxT("Goals and achieved values:\n   ");

                  for (current = goalNames.begin(), i = 0;
                       current != goalNames.end(); ++current)
                  {
                     textFile << *current << wxT("  Desired: ") << goal[i]
                              << wxT(" Achieved: ") << achieved[pertNumber-1][i]
                              << wxT("\n   ");
                     ++i;
                  }
                  textFile << std::endl;
               }
               textFile << wxT("Perturbing with variable values:\n   ");
               for (current = variableNames.begin(), i = 0;
                    current != variableNames.end(); ++current)
               {
                  textFile << *current << wxT(" = ") << unscaledVariable.at(i);
                  if ((textFileMode == wxT("Verbose")) || (textFileMode == wxT("Debug")))
                        textFile << wxT("; targeter scaled value: ") << variable.at(i);
                  textFile << wxT("\n   ");
                  ++i;
               }
               textFile << std::endl;
            }

            if (textFileMode == wxT("Debug"))
            {
               textFile << wxT("------------------------------------------------\n")
                        << wxT("Command stream data:\n")
                        << debugString << wxT("\n")
                        << wxT("------------------------------------------------\n");
            }

            break;

         case CALCULATING:
            if (textFileMode == wxT("Verbose"))
            {
               textFile << wxT("Calculating") << std::endl;

               // Iterate through the goals, writing them to the file
               textFile << wxT("Goals and achieved values:\n   ");

               for (current = goalNames.begin(), i = 0;
                    current != goalNames.end(); ++current)
               {
                   textFile << *current << wxT("  Desired: ") << goal[i]
                            << wxT(" Achieved: ") << achieved[variableCount-1][i]
                            << wxT("\n    ");
                   ++i;
               }
               textFile << std::endl;
            }

            textFile << wxT("\nJacobian (Sensitivity matrix):\n");
            for (i = 0; i < variableCount; ++i)
            {
               for (j = 0; j < goalCount; ++j)
               {
                  textFile << wxT("   ") << jacobian[i][j];
               }
               textFile << wxT("\n");
            }

            textFile << wxT("\n\nInverse Jacobian:\n");
            for (i = 0; i < goalCount; ++i)
            {
               for (j = 0; j < variableCount; ++j)
               {
                  textFile << wxT("   ") << inverseJacobian[i][j];
               }
               textFile << wxT("\n");
            }

            textFile << wxT("\n\nNew scaled variable estimates:\n   ");
            for (current = variableNames.begin(), i = 0;
                 current != variableNames.end(); ++current)
            {
               //textFile << *current << wxT(" = ") << variable[i++] << wxT("\n   ");
               textFile << *current << wxT(" = ") << variable.at(i++) << wxT("\n   ");
            }
            textFile << std::endl;
            break;

         case CHECKINGRUN:
            // Iterate through the goals, writing them to the file
            textFile << wxT("Goals and achieved values:\n   ");

            for (current = goalNames.begin(), i = 0;
                 current != goalNames.end(); ++current)
            {
               textFile << *current << wxT("  Desired: ") << goal[i]
                        << wxT(" Achieved: ") << nominal[i]
                        << wxT("\n   Tolerance: ") << tolerance[i]
                        << wxT("\n   ");
               ++i;
            }

            textFile << wxT("\n*****************************")
                     << wxT("***************************\n")
                     << std::endl;
            break;

         case FINISHED:
            textFile << wxT("\n****************************")
                     << wxT("****************************\n")
                     << wxT("*** Targeting Completed in ") << iterationsTaken
                     << wxT(" iterations")
                     << wxT("\n****************************")
                     << wxT("****************************\n")
                     << std::endl;

            break;

         case ITERATING:     // Intentional fall through
         default:
            throw SolverException(
               wxT("Solver state not supported for the targeter"));
      }
   }
}
