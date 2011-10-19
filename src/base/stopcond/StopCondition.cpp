//$Id: StopCondition.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  StopCondition
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
// Author: Linda Jun
// Created: 2003/10/09
//
/**
 * Implements StopCondition class.
 */
//------------------------------------------------------------------------------

#include "StopCondition.hpp"
#include "StopConditionException.hpp"
#include "NotAKnotInterpolator.hpp"
#include "RealUtilities.hpp"           // for Abs()
#include "AngleUtil.hpp"               // for PutAngleInDegRange()
#include "KeplerianParameters.hpp"     // for KepEcc()
#include "SphericalParameters.hpp"     // for SphRMag()
#include "StringUtil.hpp"              // for ReplaceName()
#include "MessageInterface.hpp"
#include "GmatConstants.hpp"


//#define DEBUG_STOPCOND_INIT
//#define DEBUG_STOPCOND_GET
//#define DEBUG_STOPCOND_SET
//#define DEBUG_STOPCOND_OBJ
//#define DEBUG_STOPCOND_EVAL
//#define DEBUG_RENAME
//#define DEBUG_STOPCOND_PERIAPSIS
//#define DEBUG_BUFFER_FILLING
//#define DEBUG_CYCLIC_PARAMETERS
//#define DEBUG_STOPCOND_EPOCH
//#define DEBUG_WRAPPERS
//#define DEBUG_CYCLIC_TIME

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif


using namespace GmatMathUtil;

//---------------------------------
// static data
//---------------------------------
const Real StopCondition::STOP_COND_TOL = 1.0e-11;

const wxString
StopCondition::PARAMETER_TEXT[StopConditionParamCount - GmatBaseParamCount] =
{
   wxT("BaseEpoch"),
   wxT("Epoch"),
   wxT("EpochVar"),
   wxT("StopVar"),
   wxT("Goal"),
   wxT("Repeat"),
};

const Gmat::ParameterType
StopCondition::PARAMETER_TYPE[StopConditionParamCount - GmatBaseParamCount] =
{
   Gmat::REAL_TYPE,    //wxT("BaseEpoch"),
   Gmat::REAL_TYPE,    //wxT("Epoch"),
   Gmat::STRING_TYPE,  //wxT("EpochVar"),
   Gmat::STRING_TYPE,  //wxT("StopVar"),
   Gmat::STRING_TYPE,  //wxT("Goal"),
   Gmat::INTEGER_TYPE, //wxT("Repeat"),
};

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// StopCondition(const wxString &name, const wxString &desc,
//               Parameter *epochParam, Parameter *stopParam, const Real &goal
//               const Real &tol, const Integer repeatCount,
//               Interpolator *interp)
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
StopCondition::StopCondition(const wxString &name, const wxString &desc,
                             Parameter *epochParam, Parameter *stopParam,
                             const Real &goal, const Real &tol,
                             const Integer repeatCount,
                             Interpolator *interp)
   : GmatBase(Gmat::STOP_CONDITION, wxT("StopCondition"), name),
     mBaseEpoch           (0.0),
     internalEpoch        (0.0),
     currentGoalValue     (goal),
     mRepeatCount         (repeatCount),
     mSolarSystem         (NULL),
     mInterpolator        (interp),
     mDescription         (desc),
     mStopParamType       (wxT("")),
     mStopParamName       (wxT("")),
     mEpochParamName      (wxT("")),
     lhsString            (wxT("")),
     rhsString            (wxT("0.0")),
     mStopParam           (stopParam),
     mGoalParam           (NULL),
     mEpochParam          (epochParam),
     mEccParam            (NULL),
     mRmagParam           (NULL),
     lhsWrapper           (NULL),
     rhsWrapper           (NULL),
     mNumValidPoints      (0),
     mBufferSize          (0),
     mStopEpoch           (REAL_PARAMETER_UNDEFINED),
     mStopInterval        (0.0),
     previousEpoch        (-999999.999999),
     previousAchievedValue(-999999.999999),
     previousGoalValue    (-999999.999999),
     mUseInternalEpoch    (false),
     mInitialized         (false),
     mNeedInterpolator    (false),
     mAllowGoalParam      (false),
     mBackwardsProp       (false),
     activated            (true),
     isLhsCyclicCondition (false),
     isRhsCyclicCondition (false),
     isPeriapse           (false),
     isApoapse            (false),
     isCyclicTimeCondition(false),
     startValue           (0.0),
     lhsCycleType         (GmatParam::NOT_CYCLIC),
     rhsCycleType         (GmatParam::NOT_CYCLIC)
{
   objectTypeNames.push_back(wxT("StopCondition"));

   objectTypes.push_back(Gmat::STOP_CONDITION);
   objectTypeNames.push_back(wxT("StopCondition"));
   
   if (mStopParam != NULL)
   {
      mStopParamType = mStopParam->GetTypeName();
      mStopParamName = mStopParam->GetName();
   }
   
   if (epochParam == NULL)
      mUseInternalEpoch = true;
   
   // Create default Interpolator
   if (mInterpolator == NULL)
   {
      mInterpolator = new NotAKnotInterpolator(wxT("InternalInterpolator"));
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Add
         (mInterpolator, mInterpolator->GetName(), wxT("StopCondition::StopCondition()"),
          wxT("mInterpolator = (new NotAKnotInterpolator()"));
      #endif
   }
}


//------------------------------------------------------------------------------
// StopCondition(const StopCondition &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 */
//------------------------------------------------------------------------------
StopCondition::StopCondition(const StopCondition &copy)
   : GmatBase(copy),
     mBaseEpoch           (copy.mBaseEpoch),
     internalEpoch        (copy.internalEpoch),
     currentGoalValue     (copy.currentGoalValue),
     mRepeatCount         (copy.mRepeatCount),
     mSolarSystem         (copy.mSolarSystem),
     mDescription         (copy.mDescription),
     mStopParamType       (copy.mStopParamType),
     mStopParamName       (copy.mStopParamName),
     mEpochParamName      (wxT("")),
     lhsString            (copy.lhsString),
     rhsString            (copy.rhsString),
     mStopParam           (copy.mStopParam),
     mGoalParam           (copy.mGoalParam),
     mEpochParam          (copy.mEpochParam),
     mEccParam            (NULL),
     mRmagParam           (NULL),
     lhsWrapper           (copy.lhsWrapper),
     rhsWrapper           (copy.rhsWrapper),
     mNumValidPoints      (0),
     mBufferSize          (0),
     mStopEpoch           (copy.mStopEpoch),
     mStopInterval        (0.0),
     previousEpoch        (-999999.999999),
     previousAchievedValue(-999999.999999),
     previousGoalValue    (-999999.999999),
     mUseInternalEpoch    (copy.mUseInternalEpoch),
     mInitialized         (copy.mInitialized),
     mNeedInterpolator    (copy.mNeedInterpolator),
     mAllowGoalParam      (copy.mAllowGoalParam),
     mBackwardsProp       (copy.mBackwardsProp),
     activated            (copy.activated),
     isLhsCyclicCondition (copy.isLhsCyclicCondition),
     isRhsCyclicCondition (copy.isRhsCyclicCondition),
     isPeriapse           (copy.isPeriapse),
     isApoapse            (copy.isApoapse),
     isCyclicTimeCondition(copy.isCyclicTimeCondition),
     initialGoalValue     (copy.initialGoalValue),
     lhsCycleType         (copy.lhsCycleType),
     rhsCycleType         (copy.rhsCycleType)
{
   mAllRefObjectNames = copy.mAllRefObjectNames;  // Is this correct?
   
   if (copy.mInterpolator != NULL)
      if (copy.mInterpolator->GetName() == wxT("InternalInterpolator"))
      {
         mInterpolator = (Interpolator*)copy.mInterpolator->Clone();
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Add
            (mInterpolator, mInterpolator->GetName(), wxT("StopCondition::StopCondition(copy)"),
             wxT("mInterpolator = (Interpolator*)copy.mInterpolator->Clone()"));
         #endif
      }
   
   if (copy.mEccParam != NULL)
   {
      mEccParam = (Parameter*)copy.mEccParam->Clone();
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Add
         (mEccParam, mEccParam->GetName(), wxT("StopCondition::StopCondition(copy)"),
          wxT("mEccParam = (Parameter*)copy.mEccParam->Clone()"));
      #endif
   }
   
   if (copy.mRmagParam != NULL)
   {
      mRmagParam = (Parameter*)copy.mRmagParam->Clone();
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Add
         (mRmagParam, mRmagParam->GetName(), wxT("StopCondition::StopCondition(copy)"),
          wxT("mRmagParam = (Parameter*)copy.mRmagParam->Clone()"));
      #endif
   }
   
   CopyDynamicData(copy);
}


//------------------------------------------------------------------------------
// StopCondition& operator= (const StopCondition &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 */
//------------------------------------------------------------------------------
StopCondition& StopCondition::operator= (const StopCondition &right)
{
   if (this != &right)
   {
      GmatBase::operator=(right);
      
      mAllRefObjectNames = right.mAllRefObjectNames; // Is this correct?
      
      mBaseEpoch = right.mBaseEpoch;
      internalEpoch = right.internalEpoch;
      currentGoalValue = right.currentGoalValue;
      initialGoalValue = right.initialGoalValue;
      mRepeatCount = right.mRepeatCount;
      
      if (mInterpolator != NULL)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (mInterpolator, mInterpolator->GetName(), wxT("StopCondition::operator="),
             wxT("deleting mInterpolator"));
         #endif
         delete mInterpolator;
      }
      
      if (right.mInterpolator != NULL)
         if (right.mInterpolator->GetName() == wxT("InternalInterpolator"))
         {
            mInterpolator = (Interpolator*)right.mInterpolator->Clone();
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Add
               (mInterpolator, mInterpolator->GetName(), wxT("StopCondition::operator="),
                wxT("mInterpolator = (Interpolator*)right.mInterpolator->Clone()"));
            #endif
         }
      
      mSolarSystem = right.mSolarSystem;
      mDescription = right.mDescription;
      mStopParamType = right.mStopParamType;
      mStopParamName = right.mStopParamName;
      lhsString = right.lhsString;
      rhsString = right.rhsString;
      
      mStopEpoch = right.mStopEpoch;
      mStopInterval = right.mStopInterval;
      mStopParam = right.mStopParam;
      mEpochParam = right.mEpochParam;
      mGoalParam = right.mGoalParam;
      lhsWrapper = right.lhsWrapper;
      rhsWrapper = right.rhsWrapper;
      
      if (mEccParam != NULL)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (mEccParam, mEccParam->GetName(), wxT("StopCondition::operator="),
             wxT("deleting mEccParam"));
         #endif
         delete mEccParam;
      }
      
      if (right.mEccParam != NULL)
      {
         mEccParam = (Parameter*)right.mEccParam->Clone();
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Add
            (mEccParam, mEccParam->GetName(), wxT("StopCondition::operator="),
             wxT("mEccParam = (Parameter*)right.mEccParam->Clone()"));
         #endif
      }
      else
         mEccParam = NULL;
      
      if (mRmagParam != NULL)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (mRmagParam, mRmagParam->GetName(), wxT("StopCondition::operator="),
             wxT("deleting mRmagParam"));
         #endif
         delete mRmagParam;
      }
      
      if (right.mRmagParam != NULL)
      {
         mRmagParam = (Parameter*)right.mRmagParam->Clone();
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Add
            (mRmagParam, mRmagParam->GetName(), wxT("StopCondition::operator="),
             wxT("mRmagParam = (Parameter*)right.mRmagParam->Clone()"));
         #endif
      }
      else
         mRmagParam = NULL;
      
      mInitialized = right.mInitialized;
      mUseInternalEpoch = right.mUseInternalEpoch;
      mNeedInterpolator = right.mNeedInterpolator;
      mAllowGoalParam = right.mAllowGoalParam;
      mBackwardsProp = right.mBackwardsProp;
      activated = right.activated;
      
      previousEpoch = -999999.999999;
      previousAchievedValue = -999999.999999;
      previousGoalValue = -999999.999999;
      
      isLhsCyclicCondition  = right.isLhsCyclicCondition;
      isPeriapse            = right.isPeriapse;
      isApoapse             = right.isApoapse;
      isCyclicTimeCondition = right.isCyclicTimeCondition;
      lhsCycleType          = right.lhsCycleType;
      rhsCycleType          = right.rhsCycleType;
      
      CopyDynamicData(right);
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// virtual ~StopCondition()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
StopCondition::~StopCondition()
{
   #ifdef DEBUG_STOPCOND
   MessageInterface::ShowMessage
      (wxT("StopCondition::~StopCondition() entered, lhsWrapper=<%p>, rhsWrapper=<%p>, ")
       wxT("mEccParam=<%p>, mRmagParam=<%p>, mInterpolator=<%p>\n"), lhsWrapper,
       rhsWrapper, mEccParam, mRmagParam, mInterpolator);
   #endif
   
   ///@note lhsWrapper and rhsWrapper are deleted from the Propagate command
   
   if (mEccParam != NULL)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (mEccParam, mEccParam->GetName(), wxT("StopCondition::~StopCondition()"),
          wxT("deleting mEccParam"));
      #endif
      delete mEccParam;
   }
   
   if (mRmagParam != NULL)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (mRmagParam, mRmagParam->GetName(), wxT("StopCondition::~StopCondition()"),
          wxT("deleting mRmagParam"));
      #endif
      delete mRmagParam;
   }
   
   if (mInterpolator != NULL)
   {
      if (mInterpolator->GetName() == wxT("InternalInterpolator"))
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (mInterpolator, mInterpolator->GetName(), wxT("StopCondition::~StopCondition()"),
             wxT("deleting mInterpolator"));
         #endif
         delete mInterpolator;
      }
   }
}

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * @return true if single parameter value stopping condition has been met;
 *   false otherwise
 *
 * @exception <StopConditionException> thrown if unable to retrieve parameter
 *   value
 *
 * @note count for repeat stop count will be implemented in the next build.
 */
//------------------------------------------------------------------------------
bool StopCondition::Evaluate()
{
   #ifdef DEBUG_STOPCOND_EVAL
      MessageInterface::ShowMessage(
         wxT("StopCondition::Evaluate() entered, mNumValidPoints=%d, mAllowGoalParam=%d, ")
         wxT("isCyclicTimeCondition=%d\n"), mNumValidPoints, mAllowGoalParam,
         isCyclicTimeCondition);
   #endif
   
   bool goalMet = false, readyToTest = true;
   Real epoch;
   Real currentParmValue;
   
   if (mStopParam == NULL || (mAllowGoalParam && mGoalParam == NULL))
      Initialize();
   
   #ifdef DEBUG_BUFFER_FILLING
      MessageInterface::ShowMessage(
         wxT("StopCondition::Evaluate called, mNumValidPoints=%d, mAllowGoalParam=%d\n"),
         mNumValidPoints, mAllowGoalParam);
   #endif
   
   // evaluate goal
   if (mAllowGoalParam && mGoalParam != NULL || rhsWrapper != NULL)
   {
      if (mAllowGoalParam)
      {
         initialGoalValue = mGoalParam->EvaluateReal();
         currentGoalValue = initialGoalValue;
      }
      else
      {
         initialGoalValue = rhsWrapper->EvaluateReal();
         currentGoalValue = initialGoalValue;
      }
      
      if (isCyclicTimeCondition)
         currentGoalValue += startValue;
   }
   
   // set current epoch
   if (mUseInternalEpoch)
      epoch = internalEpoch;
   else
      epoch = mEpochParam->EvaluateReal();
   
   // set current value
   currentParmValue = mStopParam->EvaluateReal();
   
   #ifdef DEBUG_BUFFER_FILLING
   MessageInterface::ShowMessage
      (wxT("   currentParmValue=%.15f, currentGoalValue=%.15f\n"), currentParmValue,
       currentGoalValue);
   #endif
   
   if (isLhsCyclicCondition)
   {
      readyToTest = CheckCyclicCondition(currentParmValue);
      
      if (!readyToTest)
      {
         previousAchievedValue = currentParmValue;
         previousEpoch = epoch;
         previousGoalValue = currentGoalValue;
      }
   }
   
   if (isApoapse)
   {
      readyToTest = CheckOnApoapsis();
      if (!readyToTest)
      {
         previousAchievedValue = currentParmValue;
         previousEpoch = epoch;
         previousGoalValue = currentGoalValue;
      }
   }
   
   if (isPeriapse)
   {
      readyToTest = CheckOnPeriapsis();
      if (!readyToTest)
      {
         previousAchievedValue = currentParmValue;
         previousEpoch = epoch;
         previousGoalValue = currentGoalValue;
      }
   }
   
   if (mNumValidPoints == 0)
   {
      previousAchievedValue = currentParmValue;
      previousEpoch = epoch;
      previousGoalValue = currentGoalValue;
      ++mNumValidPoints;
      
      #ifdef DEBUG_STOPCOND_EVAL
      MessageInterface::ShowMessage
         (wxT("StopCondition::Evaluate()() returning false, mNumValidPoints is zero\n"));
      #endif
      return false;
   }
   
   if (!mStopParam->IsTimeParameter())
   {
      Real min, max;
      min = (currentParmValue<previousAchievedValue ? currentParmValue : previousAchievedValue);
      max = (currentParmValue>previousAchievedValue ? currentParmValue : previousAchievedValue);
      
      #ifdef DEBUG_STOPCOND_EVAL
         MessageInterface::ShowMessage(
            wxT("Evaluating: min = %lf, max = %lf, goal = %lf\n"), min, max, currentGoalValue);
      #endif
      
      if ((min != max) && readyToTest)
      {
         if ((currentGoalValue >= min) && (currentGoalValue <= max) && activated)
         {
            goalMet = true;
            mStopInterval = (epoch - previousEpoch) * GmatTimeConstants::SECS_PER_DAY;
      
            #ifdef DEBUG_STOPCOND_EVAL
               MessageInterface::ShowMessage(
                  wxT("Previous Epoch = %.12lf, Epoch  %.12lf, Values = [%.12lf  %.12lf], ")
                  wxT("StopInterval = %.12lf\n"), previousEpoch, epoch,
                  previousAchievedValue, currentParmValue, mStopInterval);
            #endif
         }
         else if (activated)
         {
            // Save the found values in for next time through
            previousEpoch = epoch;
            previousAchievedValue = currentParmValue;
            previousGoalValue = currentGoalValue;
         }
      }
   }
   else
   // for time data we don't need to interpolate
   {
      #ifdef DEBUG_CYCLIC_TIME
         MessageInterface::ShowMessage(wxT("Parameter %s is time based and %s\n"),
               mStopParam->GetTypeName().c_str(),
               isCyclicTimeCondition ? wxT("cyclic") : wxT("not cyclic"));
         if (isCyclicTimeCondition)
         {
            MessageInterface::ShowMessage(wxT("   Current goal value: %lf\n"),
                  currentGoalValue);
            MessageInterface::ShowMessage(wxT("   Previous achieved value: %lf\n"),
                  previousAchievedValue);
            MessageInterface::ShowMessage(wxT("   Current parm value: %lf\n"),
                  currentParmValue);
         }
      #endif

      Real prevGoalDiff = previousAchievedValue - currentGoalValue,
           currGoalDiff = currentParmValue - currentGoalValue;

      Real direction = 
           (currGoalDiff - prevGoalDiff > 0.0 ? 1.0 : -1.0);

      #ifdef DEBUG_STOPCOND_EVAL
         MessageInterface::ShowMessage(
            wxT("prev = %15.9lf, curr = %15.9lf, lhs = %15.9lf, rhs = %15.9lf, ")
            wxT("direction = %15.9lf, \n"), previousAchievedValue, currentParmValue, 
            prevGoalDiff, currGoalDiff, direction);
      #endif

      // todo: Fix the direction test code here!!!
//      if (mNumValidPoints == 1)
//         if (((2.0*currentGoalValue - currentParmValue - previousAchievedValue) * direction) < 0.0)
//            MessageInterface::ShowMessage(
//               wxT("Warning!  Time based stopping condition \"%s\" = %.10lf will ")
//               wxT("never be satisfied\n"),
//               instanceName.c_str(), currentGoalValue);
      
      // Goal met if it falls between previous and current values
      if ((currGoalDiff*direction >= 0.0) && 
          (prevGoalDiff*direction <= 0.0) && activated)
      {
         goalMet = true;
         
         #ifdef DEBUG_STOPCOND_EVAL
            MessageInterface::ShowMessage
               (wxT("StopCondition::Evaluate() mUseInternalEpoch = %d, ")
               wxT("epoch = %15.9lf, currentGoalValue = %15.9lf, ")
               wxT("currentParmValue = %15.9lf, ")
               wxT("previousAchievedValue = %15.9lf, ")
               wxT("GoalDiffs = [%15.9lf  %15.9lf]\n"),  
                mUseInternalEpoch, epoch, 
                currentGoalValue, currentParmValue, previousAchievedValue, 
                prevGoalDiff, currGoalDiff);
         #endif
      }
      else if (activated)
      {
         previousAchievedValue = currentParmValue;
         previousEpoch = epoch;
         previousGoalValue = currentGoalValue;
      }
   }
   
   #ifdef DEBUG_BUFFER_FILLING
      MessageInterface::ShowMessage(
         wxT("Value = %.12lf, Previous = %.12lf; Goal (%.12lf) %s, epoch = %15.12lf\n"), 
         currentParmValue, previousAchievedValue, currentGoalValue, (goalMet ? wxT("met") : wxT("not met")), epoch);
   #endif
   
   ++mNumValidPoints;
   
   #ifdef DEBUG_STOPCOND_EVAL
   MessageInterface::ShowMessage
      (wxT("StopCondition::Evaluate() returning %s\n"), goalMet ? wxT("goal met") : wxT("goal not met"));
   #endif
   return goalMet;
}


//------------------------------------------------------------------------------
// bool IsTimeCondition()
//------------------------------------------------------------------------------
/**
 * Checks to see if the stopping condition is a time based condition.
 * 
 * @return true if this is a time based stopping condition, false otherwise.
 */
//------------------------------------------------------------------------------
bool StopCondition::IsTimeCondition()
{
   return mStopParam->IsTimeParameter();
}


//------------------------------------------------------------------------------
// bool AddToBuffer(bool isInitialPoint)
//------------------------------------------------------------------------------
/**
 * Adds a data point to the ring buffer used for interpolation.
 * 
 * @param <isInitialPoint> Flag indicating if this is the first point for this 
 * stop evaluation.
 * 
 * @return true if a stop epoch can be evaluated; false if more points are 
 * required.
 */
//------------------------------------------------------------------------------
bool StopCondition::AddToBuffer(bool isInitialPoint)
{
   #ifdef DEBUG_BUFFER_FILLING
      MessageInterface::ShowMessage(
         wxT("StopCondition::AddToBuffer(%s) called, internalEpoch is %s\n"),
         (isInitialPoint ? wxT("true") : wxT("false")),
         (mUseInternalEpoch ? wxT("true") : wxT("false")));
   #endif
      
   // For now, skip time cases because they are already built
   if (IsTimeCondition())
      return true;
   
   // Flag indicating if both the ring buffer is full, and the stop condition 
   // is bracketed
   bool retval = false;
   
   Real epoch = 0.0;
   Real currentParmValue, goalValue;
   Real stopEpoch; //in A1Mjd
   
   // evaluate goal
   if (mAllowGoalParam && mGoalParam != NULL || rhsWrapper != NULL)
   {
      if (mAllowGoalParam)
         initialGoalValue = mGoalParam->EvaluateReal();
      else
         initialGoalValue = rhsWrapper->EvaluateReal();
      
      if (isCyclicTimeCondition)
         currentGoalValue = initialGoalValue + startValue;
      else
         currentGoalValue = initialGoalValue;
   }
   goalValue = currentGoalValue;

   // set current epoch
   if (mUseInternalEpoch)
      epoch = internalEpoch;
   else
      epoch = mEpochParam->EvaluateReal();
   
   // set current LHS value
   try
   {
      currentParmValue = mStopParam->EvaluateReal();
   }
   catch (BaseException &)
   {
      // try with rhsWrapper
      if (lhsWrapper != NULL)
         currentParmValue = lhsWrapper->EvaluateReal();
      else
         throw;
   }
   
   if(isLhsCyclicCondition)
   {
      // CheckCyclicCondition() puts currentGoalValue in range so commented out (LOJ: 2011.01.24)
      //currentGoalValue = PutInRange(currentGoalValue, 0.0, GmatMathUtil::TWO_PI_DEG);
      if (!CheckCyclicCondition(currentParmValue))
         return false;
   }
   
   #ifdef DEBUG_BUFFER_FILLING
      MessageInterface::ShowMessage(
         wxT("  New point: %.12lf, %.12lf\n"),
         epoch, currentParmValue);
   #endif
      
   // Actions taken to initialize the ring buffer
   if (isInitialPoint)
   {
      // Reset the internal buffer and the point count
      mNumValidPoints = 1;  // We always have the data for the initial point
      
      for (int i = 0; i < mBufferSize; ++i)
      {
         mEpochBuffer[i] = 0.0;
         lhsValueBuffer[i] = 0.0;
         rhsValueBuffer[i] = 0.0;
      }
      
      // Fill in the data for the first point
      lhsValueBuffer[mBufferSize-1] = previousAchievedValue;
      rhsValueBuffer[mBufferSize-1] = previousGoalValue;
      
      if (mUseInternalEpoch)
         mEpochBuffer[mBufferSize-1] = 0.0; 
      else
         mEpochBuffer[mBufferSize-1] = previousEpoch;
   }

   // Roll values in the ring buffer to make room for newest value
   for (int i = 0; i < mBufferSize-1; ++i)
   {
      mEpochBuffer[i]   = mEpochBuffer[i+1];
      lhsValueBuffer[i] = lhsValueBuffer[i+1];
      rhsValueBuffer[i] = rhsValueBuffer[i+1];
   }
   
   // Fill in the next data point
   mEpochBuffer[mBufferSize-1]   = epoch;
   rhsValueBuffer[mBufferSize-1] = currentGoalValue;
   lhsValueBuffer[mBufferSize-1] = currentParmValue;
   ++mNumValidPoints;
   
   // Only start looking for a solution when the ring buffer is full   
   if (mNumValidPoints >= mBufferSize)
   {
      Real minVal = lhsValueBuffer[0], maxVal = lhsValueBuffer[mBufferSize - 1];
      for (int i = 0; i < mBufferSize; ++i)
      {
         if (minVal > lhsValueBuffer[i])
            minVal = lhsValueBuffer[i];
         if (maxVal < lhsValueBuffer[i])
            maxVal = lhsValueBuffer[i];
      }
      
      #ifdef DEBUG_BUFFER_FILLING
         MessageInterface::ShowMessage(wxT("Min value = %.12lf, Max = %.12lf\n"),
            minVal, maxVal);
      #endif

      // If -- and only if -- the goal is bracketed, then interpolate the epoch
      if ((currentGoalValue >= minVal) && (currentGoalValue <= maxVal))
      {
         // Prep the interpolator
         mInterpolator->Clear();
         for (int i=0; i<mBufferSize; i++)
         {
            #ifdef DEBUG_STOPCOND_EVAL
            MessageInterface::ShowMessage
               (wxT("StopCondition::Evaluate() i=%d, lhsValueBuffer=%f, ")
                wxT("mEpochBuffer=%f\n"), i, lhsValueBuffer[i], mEpochBuffer[i]);
            #endif
            mInterpolator->AddPoint(lhsValueBuffer[i], &mEpochBuffer[i]);
         }
         
         // Finally, if we can interpolate an epoch, we have success!
         if (mInterpolator->Interpolate(currentGoalValue, &stopEpoch))
         {
            mStopEpoch = stopEpoch;
            retval = true;
         }
         
         #ifdef DEBUG_STOPCOND_EVAL
         MessageInterface::ShowMessage
            (wxT("StopCondition::Evaluate() mStopEpoch=%f\n"), mStopEpoch);
         #endif
      }
   }

   #ifdef DEBUG_BUFFER_FILLING
      MessageInterface::ShowMessage(wxT("Valid points: %d, Buffer contents:\n"),
         mNumValidPoints);
      for (int i=0; i<mBufferSize; i++)
      {
         MessageInterface::ShowMessage
            (wxT("   [%d]   %.12lf  %.12lf\n"), i, mEpochBuffer[i], lhsValueBuffer[i]);
      }
   #endif


   return retval;
}


//------------------------------------------------------------------------------
// Real GetStopEpoch()
//------------------------------------------------------------------------------
/**
 * Calculates the time to step (in seconds) needed to reach the stop epoch.
 * 
 * @return the time to step (in seconds)
 */
//------------------------------------------------------------------------------
Real StopCondition::GetStopEpoch()
{
   if (IsTimeCondition())
   {
      
      #ifdef DEBUG_STOPCOND_EPOCH
         MessageInterface::ShowMessage
            (wxT("StopCondition::GetStopEpoch()\n   Previous = %15.9lf\n   ")
            wxT("Current = %15.9lf\n   Goal = %15.9lf"), previousEpoch, previousAchievedValue, 
            currentGoalValue);
      #endif

      Real dt = (currentGoalValue - previousAchievedValue) * GetTimeMultiplier();
      return dt;
   }
      
   Real stopEpoch = 0.0;
   
   #ifdef DEBUG_STOPCOND_EPOCH
      MessageInterface::ShowMessage
         (wxT("StopCondition::Evaluate()\n   Ring buffer:\n"));
   #endif

   mInterpolator->Clear();
   for (int i=0; i<mBufferSize; i++)
   {
      #ifdef DEBUG_STOPCOND_EPOCH
         MessageInterface::ShowMessage
            (wxT("      i=%d, lhsValueBuffer=%.12lf, ")
             wxT("mEpochBuffer=%.12lf\n"), i, lhsValueBuffer[i], mEpochBuffer[i]);
      #endif
      
      mInterpolator->AddPoint(lhsValueBuffer[i], &mEpochBuffer[i]);
   }
   
   if (mInterpolator->Interpolate(currentGoalValue, &stopEpoch))
      mStopEpoch = stopEpoch;
   else
      throw StopConditionException(wxT("Unable to interpolate a stop epoch"));
  
   #ifdef DEBUG_STOPCOND_EPOCH
      MessageInterface::ShowMessage
         (wxT("   Interpolated epoch = %.12lf\n"), mStopEpoch);
   #endif
      
   return mStopEpoch;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the StopCondition.
 *
 * @return clone of the StopCondition.
 *
 */
//------------------------------------------------------------------------------
GmatBase* StopCondition::Clone() const
{
   return (new StopCondition(*this));
}


//---------------------------------
// protected methods
//---------------------------------

//------------------------------------------------------------------------------
// bool CheckOnPeriapsis()
//------------------------------------------------------------------------------
bool StopCondition::CheckOnPeriapsis()
{   
   #ifdef DEBUG_STOPCOND_PERIAPSIS
   MessageInterface::ShowMessage
      (wxT("StopCondition::CheckOnPeriapsis() entered, previousAchievedValue=%.15f, ")
       wxT("currentGoalValue=%.15f\n"), previousAchievedValue, currentGoalValue);
   #endif
   
   bool goalMet = false;
   
   // Eccentricity must be large enough to keep osculations from masking the
   // stop point
   //Real ecc = mEccParam->EvaluateReal();
   //Real rmag = mRmagParam->EvaluateReal();  // ???
   
   //#ifdef DEBUG_STOPCOND_PERIAPSIS
   //MessageInterface::ShowMessage(wxT("   ecc=%.15f\n"), ecc);
   //#endif
   
   //----------------------------------------------------------------------
   // A necessary condition for periapse stop: when moving forward in time,
   // R dot V must cross from negative to positive, so previous value must be 
   // less than the goal value. 
   //----------------------------------------------------------------------
   //if ((rmag <= mRange) && (ecc >= mEccTol))
//   if (ecc >= 1.0e-6)
   {
      if (( mBackwardsProp && (previousAchievedValue >= currentGoalValue))  ||  
          (!mBackwardsProp && (previousAchievedValue <= currentGoalValue)))
         goalMet = true;
   }
   
   #ifdef DEBUG_STOPCOND_PERIAPSIS
   MessageInterface::ShowMessage
      (wxT("StopCondition::CheckOnPeriapsis() returning %s\n"), goalMet ?
       wxT("goal met") : wxT("goal not met"));
   #endif
   
   return goalMet;
}


//------------------------------------------------------------------------------
// bool CheckOnApoapsis()
//------------------------------------------------------------------------------
bool StopCondition::CheckOnApoapsis()
{
   if (mEccParam == NULL)
      throw StopConditionException
         (wxT("StopCondition::CheckOnApoapsis() ECC parameter has NULL pointer.\n"));
   
   // Eccentricity must be large enough to keep asculations from masking the
   // stop point
   
//   Real ecc = mEccParam->EvaluateReal();
   
   //----------------------------------------------------------------------
   // A necessary condition for apoapse stop: when moving forward in time,
   // R dot V must cross from positive to negative, so previous value must be 
   // greater than the goal value. 
   //----------------------------------------------------------------------
   
   //if (ecc >= mEccTol)
//   if (ecc >= 1.0e-6)
   {
      if ((mBackwardsProp && (previousAchievedValue <= currentGoalValue))  ||  
          (!mBackwardsProp && (previousAchievedValue >= currentGoalValue)))
         return true;
   }
   
   return false;
}

//------------------------------------------------------------------------------
// bool CheckCyclicCondition(Real &value)
//------------------------------------------------------------------------------
/**
 * Performs range setting for cyclic stopping conditions, and checks to see that
 * the condition falls near the goal.
 * 
 * @param <value> Reference to the value of the parameter. On return from this 
 *                call, the input variable is changed to fall in the valid range
 *                for the variable, mapped around the current goal.
 * 
 * @return true if the stopping condition is near the goal, false otherwise.
 */ 
//------------------------------------------------------------------------------
bool StopCondition::CheckCyclicCondition(Real &value)
{
   Real min, max;
   bool retval = false;
   
   if (GetRange(min, max))
   {
      Real range2 = (max - min) / 2.0;
      currentGoalValue = PutInRange(currentGoalValue, min, max);
      value = PutInRange(value, currentGoalValue - range2, 
                   currentGoalValue + range2);
      previousAchievedValue = PutInRange(previousAchievedValue, currentGoalValue - range2, 
                   currentGoalValue + range2);
      
      #ifdef DEBUG_CYCLIC_PARAMETERS
         MessageInterface::ShowMessage(
            wxT("Cyclic condition: Goal = %lf, currValue = %lf, prevValue = %lf\n"),
            currentGoalValue, value, previousAchievedValue);
      #endif
         
      if (fabs(currentGoalValue - value) < (range2 / 2.0))
         retval = true;
   }
   
   return retval;
}

//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
bool StopCondition::Initialize()
{
   #ifdef DEBUG_STOPCOND_INIT
   MessageInterface::ShowMessage(wxT("StopCondition::Initialize() entered\n"));
   #endif
   
   mInitialized = false;
   isApoapse = false;
   isPeriapse = false;
   mStopInterval = 0.0;
   
   // clear local parameters
   if (mEccParam != NULL)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (mEccParam, mEccParam->GetName(), wxT("StopCondition::Initialize()"),
          wxT("deleting mEccParam"));
      #endif
      delete mEccParam;
   }
   
   if (mRmagParam != NULL)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (mRmagParam, mRmagParam->GetName(), wxT("StopCondition::Initialize()"),
          wxT("deleting mRmagParam"));
      #endif
      delete mRmagParam;
   }
   
   mEccParam = NULL;
   mRmagParam = NULL;
   
   wxString paramTypeName = mStopParam->GetTypeName();
   
   if (Validate())
   {
      if (mStopParamType == wxT("Apoapsis") ||
          mStopParamType == wxT("Periapsis"))
      {
         currentGoalValue = 0.0;
         initialGoalValue = 0.0;
         mAllowGoalParam = false;
         if (mStopParamType == wxT("Apoapsis"))
            isApoapse = true;
         if (mStopParamType == wxT("Periapsis"))
            isPeriapse = true;
      }
      else
      {
         if (rhsWrapper != NULL)
         {
            if (rhsWrapper->GetRefObject() != NULL)
               mAllowGoalParam = false;
            
            initialGoalValue = rhsWrapper->EvaluateReal();
            currentGoalValue = initialGoalValue;
         }
      }
      
      // Get isAngleParameter from the stop Parameter (LHS of stopping condition)
      bool angleParameter = mStopParam->IsAngleParameter();
      #ifdef DEBUG_STOPCOND_INIT
      MessageInterface::ShowMessage
         (wxT("   ==> Stop Parameter '%s' is%s an angle parameter\n"), paramTypeName.c_str(),
          angleParameter ? wxT("") : wxT(" not"));
      #endif
      
      if (angleParameter)
      {
         isLhsCyclicCondition = true;
         lhsCycleType = mStopParam->GetCycleType();
         #ifdef DEBUG_STOPCOND_INIT
         MessageInterface::ShowMessage(wxT("   ==> CycleType is %d\n"), lhsCycleType);
         #endif
      }
      
      if (mNeedInterpolator)
      {
         mBufferSize = mInterpolator->GetBufferSize();
         mEpochBuffer.reserve(mBufferSize);
         lhsValueBuffer.reserve(mBufferSize);
         rhsValueBuffer.reserve(mBufferSize);
         
         for (int i=0; i<mBufferSize; i++)
         {
            mEpochBuffer.push_back(0.0);
            lhsValueBuffer.push_back(0.0);
            rhsValueBuffer.push_back(0.0);
         }
         
         mNumValidPoints = 0;
         mInitialized = true;
      }
      else
      {
         mInitialized = true;
      }
   }
   
   #ifdef DEBUG_STOPCOND_INIT
   MessageInterface::ShowMessage
      (wxT("StopCondition::Initialize() returning mInitialized=%d, ")
       wxT("initialGoalValue=%.12f, currentGoalValue=%.12f\n"), mInitialized,
       initialGoalValue, currentGoalValue);
   #endif
   
   return mInitialized;
}


//------------------------------------------------------------------------------
// virtual bool Validate()
//------------------------------------------------------------------------------
/**
 * @return true if all necessary objects have been set; false otherwise
 *
 * @exception thrown if necessary object pointer is NULL
 */
//------------------------------------------------------------------------------
bool StopCondition::Validate()
{   
   #ifdef DEBUG_STOPCOND_INIT   
   MessageInterface::ShowMessage
      (wxT("StopCondition::Validate() entered, mUseInternalEpoch=%d, mEpochParam=<%p>, ")
       wxT("mStopParam=<%p>, mAllowGoalParam=%d, mGoalParam=<%p>, rhsWrapper=<%p>\n"),
       mUseInternalEpoch, mEpochParam, mStopParam, mAllowGoalParam, mGoalParam,
       rhsWrapper);
   #endif
   
   // check on epoch parameter
   if (!mUseInternalEpoch && mEpochParam == NULL)
   {
      throw StopConditionException
         (wxT("StopCondition::Validate() epoch parameter: ") + mEpochParamName +
          wxT(" has NULL pointer.\n"));
   }
   
   // check on stop parameter
   if (mStopParam == NULL)
   {
      #ifdef DEBUG_STOPCOND_INIT
      MessageInterface::ShowMessage
         (wxT("StopCondition::Validate() stop parameter '%s' is NULL\n"),
          mStopParamName.c_str());
      #endif
      //throw StopConditionException
      //   (wxT("StopCondition::Validate() stop parameter: ") + mStopParamName +
      //    wxT(" has NULL pointer.\n"));
      throw StopConditionException
         (wxT("Currently GMAT expects a Spacecraft Parameter to be on the LHS of ")
          wxT("stopping condition"));
   }
   
   // check if stop parameter is a system Parameter such as Sat.X
   if (mStopParam->GetKey() != GmatParam::SYSTEM_PARAM)
   {
      throw StopConditionException
         (wxT("Currently GMAT expects a Spacecraft Parameter to be on the LHS of ")
          wxT("stopping condition"));
   }
   
   isCyclicTimeCondition = false;
   
   // check on interpolator
   if (mStopParam->IsTimeParameter())
   {
      mNeedInterpolator = false;
      // Set time parameter type
      wxString timeTypeName = mStopParam->GetTypeName();
      if (timeTypeName == wxT("ElapsedSecs"))
      {
         stopParamTimeType = SECOND_PARAM;
         isCyclicTimeCondition = true;
      }
      else if (timeTypeName == wxT("ElapsedDays"))
      {
         stopParamTimeType = DAY_PARAM;
         isCyclicTimeCondition = true;
      }
      else if (timeTypeName.find(wxT("ModJulian")) != wxString::npos)
         stopParamTimeType = EPOCH_PARAM;
      else
         stopParamTimeType = UNKNOWN_PARAM_TIME_TYPE;
      
      #ifdef DEBUG_STOPCOND_INIT   
         MessageInterface::ShowMessage(
            wxT("Stop parameter \"%s\" has time type %d\n"), timeTypeName.c_str(), 
            stopParamTimeType);
      #endif
   }
   else
   {
      if (mInterpolator == NULL)
      {
         throw StopConditionException
            (wxT("StopCondition::Validate() Interpolator has NULL pointer.\n"));
      }
      
      mNeedInterpolator = true;
   }
   
   // check on goal parameter
   if (mAllowGoalParam && mGoalParam == NULL)
      throw StopConditionException
         (wxT("StopCondition::Validate() goal parameter: ") + rhsString +
          wxT(" has NULL pointer.\n"));
   
   // Apoapsis and Periapsis need additional parameters
   if (mStopParamType == wxT("Apoapsis") ||
       mStopParamType == wxT("Periapsis"))
   {
      // check on Ecc parameter
      if (mEccParam  == NULL)
      {
         #ifdef DEBUG_STOPCOND_INIT
         MessageInterface::ShowMessage
            (wxT("StopCondition::Validate(): Creating KepEcc...\n"));
         #endif
         
         mEccParam = new KepEcc(wxT(""));
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Add
            (mEccParam, mEccParam->GetName(), wxT("StopCondition::Validate()"),
             wxT("mEccParam = new KepEcc(")wxT(")"));
         #endif
         
         mEccParam->AddRefObject
            (mStopParam->GetRefObject(Gmat::SPACECRAFT, 
                                      mStopParam->GetRefObjectName(Gmat::SPACECRAFT)));
         mEccParam->AddRefObject
            (mStopParam->GetRefObject(Gmat::COORDINATE_SYSTEM, 
                                      mStopParam->GetRefObjectName(Gmat::COORDINATE_SYSTEM)));
         
         mEccParam->AddRefObject
            (mStopParam->GetRefObject(Gmat::SPACE_POINT, 
                                      mStopParam->GetRefObjectName(Gmat::SPACE_POINT)),
             true);
         
         mEccParam->SetInternalCoordSystem(mStopParam->GetInternalCoordSystem());            
         mEccParam->AddRefObject(mSolarSystem);
         mEccParam->Initialize();
      }
      
      // check on SphRMag parameter if wxT("Periapsis")
      if (mStopParamType == wxT("Periapsis"))
      {
         if (mRmagParam == NULL)
         {
            #ifdef DEBUG_STOPCOND_INIT
            MessageInterface::ShowMessage
               (wxT("StopCondition::Validate(): Creating SphRMag...\n"));
            #endif
            
            wxString depObjName = mStopParam->GetStringParameter(wxT("DepObject"));
            
            #ifdef DEBUG_STOPCOND_INIT
            MessageInterface::ShowMessage
               (wxT("StopCondition::Validate() depObjName of mStopParam=%s\n"),
                depObjName.c_str());
            #endif
            
            mRmagParam = new SphRMag(wxT(""));
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Add
               (mRmagParam, mRmagParam->GetName(), wxT("StopCondition::Validate()"),
                wxT("mRmagParam = new SphRMag(")wxT(")"));
            #endif
            
            mRmagParam->SetStringParameter(wxT("DepObject"), depObjName);
            
            mRmagParam->AddRefObject
               (mStopParam->GetRefObject(Gmat::SPACECRAFT,
                                         mStopParam->GetRefObjectName(Gmat::SPACECRAFT)));
            mRmagParam->AddRefObject
               (mStopParam->GetRefObject(Gmat::COORDINATE_SYSTEM,
                                         mStopParam->GetRefObjectName(Gmat::COORDINATE_SYSTEM)));
            mRmagParam->AddRefObject
               (mStopParam->GetRefObject(Gmat::SPACE_POINT,
                                         mStopParam->GetRefObjectName(Gmat::SPACE_POINT)),
                true);
            
            mRmagParam->SetInternalCoordSystem(mStopParam->GetInternalCoordSystem());
            mRmagParam->AddRefObject(mSolarSystem);
            mRmagParam->Initialize();
            
//            // set mRange
//            if (depObjName == wxT("Earth"))
//               mRange = 5.0e5;
//            else if (depObjName == wxT("Luna"))
//               //mRange = 2.0e5; // Swingby has this.
//               mRange = 5.0e5;
//            else
//               mRange = 1.0e10;
         }
      }
   }
   
   #ifdef DEBUG_STOPCOND_INIT
   MessageInterface::ShowMessage
      (wxT("StopCondition::Validate() returning true, mUseInternalEpoch=%d, mEpochParam=<%p>, ")
       wxT("mInterpolator=<%p>\n   mStopParamType=%s, mStopParamName=%s, mStopParam=<%p>, ")
       wxT("mGoalParam=<%p>\n"), mUseInternalEpoch, mEpochParam, mInterpolator, mStopParamType.c_str(),
       mStopParamName.c_str(), mStopParam, mGoalParam);
   #endif
   
   return true;
}

//------------------------------------------------------------------------------
// void Reset()
//------------------------------------------------------------------------------
void StopCondition::Reset()
{
   mNumValidPoints = 0;
}

//------------------------------------------------------------------------------
// Real GetStopInterval()
//------------------------------------------------------------------------------
Real StopCondition::GetStopInterval()
{
   return mStopInterval;
}


//------------------------------------------------------------------------------
// bool IsInitialized()
//------------------------------------------------------------------------------
bool StopCondition::IsInitialized()
{
   return mInitialized;
}


//------------------------------------------------------------------------------
// Integer GetBufferSize()
//------------------------------------------------------------------------------
Integer StopCondition::GetBufferSize()
{
   return mBufferSize;
}


//------------------------------------------------------------------------------
// wxString& GetDescription()
//------------------------------------------------------------------------------
wxString& StopCondition::GetDescription()
{
   return mDescription;
}


//------------------------------------------------------------------------------
// Parameter* GetEpochParameter()
//------------------------------------------------------------------------------
Parameter* StopCondition::GetEpochParameter()
{
   return mEpochParam;
}


//------------------------------------------------------------------------------
// Parameter* GetStopParameter()
//------------------------------------------------------------------------------
Parameter* StopCondition::GetStopParameter()
{
   return mStopParam;
}


//------------------------------------------------------------------------------
// Parameter* GetStopParameter()
//------------------------------------------------------------------------------
Parameter* StopCondition::GetGoalParameter()
{
   return mGoalParam;
}


//------------------------------------------------------------------------------
// Interpolator* GetInterpolator()
//------------------------------------------------------------------------------
Interpolator* StopCondition::GetInterpolator()
{
   return mInterpolator;
}


//------------------------------------------------------------------------------
// void SetDescription(const wxString &desc)
//------------------------------------------------------------------------------
void StopCondition::SetDescription(const wxString &desc)
{
   mDescription = desc;
}


//------------------------------------------------------------------------------
// void SetPropDirection(Real dir)
//------------------------------------------------------------------------------
void StopCondition::SetPropDirection(Real dir)
{   
   if (dir >= 1.0)
      mBackwardsProp = false;
   else
      mBackwardsProp = true;
   
   #ifdef DEBUG_STOPCOND
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetPropDirection() dir=%f, mBackwardsProp=%d\n"),
       dir, mBackwardsProp);
   #endif
}


//------------------------------------------------------------------------------
// void SetSolarSystem(SolarSystem *solarSystem)
//------------------------------------------------------------------------------
/**
 * Sets SolarSystem
 */
//------------------------------------------------------------------------------
void StopCondition::SetSolarSystem(SolarSystem *solarSystem)
{
   if (solarSystem != NULL)
   {
      mSolarSystem = solarSystem;
   }
   else
   {
      throw StopConditionException(
         wxT("Attempting to set solar system with NULL pointer."));
   }
}


//------------------------------------------------------------------------------
// bool SetInterpolator(Interpolator *interp)
//------------------------------------------------------------------------------
/**
 * Sets Interpolator to interpolate stop condition epoch.
 *
 * @return true if Interplator is successfully set; false otherwise
 */
//------------------------------------------------------------------------------
bool StopCondition::SetInterpolator(Interpolator *interp)
{
   if (interp != NULL)
   {
      if (mInterpolator->GetName() == wxT("InternalInterpolator"))
         delete mInterpolator;
      
      mInterpolator = interp;
      return true;
   }
   else
   {
      return false;
   }
}


//------------------------------------------------------------------------------
// bool SetEpochParameter(Parameter *param)
//------------------------------------------------------------------------------
/**
 * Sets epoch parameter which will be used to interpolate stop epoch
 *
 * @return true if EpochParam is successfully set; false otherwise
 */
//------------------------------------------------------------------------------
bool StopCondition::SetEpochParameter(Parameter *param)
{
   if (param != NULL)
   {
      mEpochParam = param;
      mUseInternalEpoch = false;
      return true;
   }
   else
   {
      return false;
   }
}


//------------------------------------------------------------------------------
// virtual bool SetStopParameter(Parameter *param)
//------------------------------------------------------------------------------
/**
 * Add parameter to stop condition.
 *
 * @return true if parameter has added to array.
 */
//------------------------------------------------------------------------------
bool StopCondition::SetStopParameter(Parameter *param)
{
   #ifdef DEBUG_STOP_PARAM
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetStopParameter() entered, param=<%p>'%s'\n"), param,
       param ? param->GetName().c_str() : wxT("NULL"));
   #endif
   
   if (param != NULL)
   {
      mStopParam = param;
      mStopParamType = mStopParam->GetTypeName();
      
      isCyclicTimeCondition = false;
      if (mStopParamType.find(wxT("Elapsed")) != mStopParamType.npos)
         isCyclicTimeCondition = true;
      
      if (param->IsTimeParameter())
         mInitialized = true;
      
      // Set ref object of lhsWrapper
      // Currently lhsWrapper is not used but we may want to stop propagation 
      // when stopVar = goalVar or stopVar = Sat.ElapsedDays in a future
      //if (lhsWrapper != NULL)
      //   lhsWrapper->SetRefObject(param);
      
      #ifdef DEBUG_STOP_PARAM
      MessageInterface::ShowMessage
         (wxT("StopCondition::SetStopParameter() returning true\n"));
      #endif
      
      return true;
   }
   
   #ifdef DEBUG_STOP_PARAM
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetStopParameter() returning false\n"));
   #endif
   return false;
}


//------------------------------------------------------------------------------
// virtual bool SetGoalParameter(Parameter *param)
//------------------------------------------------------------------------------
/**
 * Sets goal parameter to stop condition.
 *
 * @return true if parameter has added to array.
 */
//------------------------------------------------------------------------------
bool StopCondition::SetGoalParameter(Parameter *param)
{
   #ifdef DEBUG_STOPCOND_SET
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetGoalParameter() entered, param=<%p>\n"), param);
   #endif
   
   mGoalParam = param;
   
   // Set ref object of rhsWrapper
   if (rhsWrapper != NULL)
      rhsWrapper->SetRefObject(param);
   
   #ifdef DEBUG_STOPCOND_SET
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetGoalParameter() returning true\n"));
   #endif
   return true;
}


//------------------------------------------------------------------------------
// void SetLhsString(const wxString &str)
//------------------------------------------------------------------------------
void StopCondition::SetLhsString(const wxString &str)
{
   #ifdef DEBUG_STOPCOND_SET
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetLhsString() this=<%p>'%s' entered, str='%s', ")
       wxT("lhsString='%s'\n"), this, GetName().c_str(), str.c_str(), lhsString.c_str());
   #endif
   
   lhsString = str;
   
   #ifdef DEBUG_STOPCOND_SET
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetLhsString() this=<%p>'%s' leaving, str='%s', ")
       wxT("lhsString='%s'\n"), this, GetName().c_str(), str.c_str(), lhsString.c_str());
   #endif
}


//------------------------------------------------------------------------------
// void SetRhsString(const wxString &str)
//------------------------------------------------------------------------------
void StopCondition::SetRhsString(const wxString &str)
{
   #ifdef DEBUG_STOPCOND_SET
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetRhsString() this=<%p>'%s' entered, str='%s', ")
       wxT("rhsString='%s'\n"), this, GetName().c_str(), str.c_str(), rhsString.c_str());
   #endif
   
   rhsString = str;
   
   // remove leading blanks
   wxString::size_type pos = rhsString.find_first_not_of(' ');
   rhsString.erase(0, pos);
   
   // if str is just a number
   if (isdigit(rhsString[0]) || rhsString[0] == '.' || rhsString[0] == '-')
   {
      rhsString.ToDouble(&currentGoalValue);
      initialGoalValue = currentGoalValue;
      mAllowGoalParam = false;
   }
   else
   {
      mAllowGoalParam = true;
   }
   
   #ifdef DEBUG_STOPCOND_SET
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetRhsString() leaving, mAllowGoalParam=%d, rhsString='%s', ")
       wxT("initialGoalValue=%le, currentGoalValue=%le\n"), mAllowGoalParam, rhsString.c_str(),
       initialGoalValue, currentGoalValue);
   #endif
   
}


//------------------------------------------------------------------------------
// wxString GetLhsString()
//------------------------------------------------------------------------------
wxString StopCondition::GetLhsString()
{
   return lhsString;
}


//------------------------------------------------------------------------------
// wxString GetRhsString()
//------------------------------------------------------------------------------
wxString StopCondition::GetRhsString()
{
   return rhsString;
}


//------------------------------------------------------------------------------
// bool SetLhsWrapper(ElementWrapper *toWrapper)
//------------------------------------------------------------------------------
bool StopCondition::SetLhsWrapper(ElementWrapper *toWrapper)
{
   #ifdef DEBUG_WRAPPERS   
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetLhsWrapper() this=<%p>'%s' entered with toWrapper=<%p>\n"),
       this, GetName().c_str(), toWrapper);
   #endif
   
   if (toWrapper == NULL)
      return false;
   
   lhsWrapper = toWrapper;
   return true;
}


//------------------------------------------------------------------------------
// bool SetRhsWrapper(ElementWrapper *toWrapper)
//------------------------------------------------------------------------------
bool StopCondition::SetRhsWrapper(ElementWrapper *toWrapper)
{
   #ifdef DEBUG_WRAPPERS   
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetRhsWrapper() this=<%p>'%s' entered with toWrapper=<%p>\n"),
       this, GetName().c_str(), toWrapper);
   #endif
   
   if (toWrapper == NULL)
      return false;

   #ifdef DEBUG_WRAPPERS   
   MessageInterface::ShowMessage
      (wxT("   wrapper type=%d, desc='%s', value=%f, refObj=<%p>\n"),
       toWrapper->GetWrapperType(), toWrapper->GetDescription().c_str(), toWrapper->EvaluateReal(),
       toWrapper->GetRefObject());
   #endif
   
   rhsWrapper = toWrapper;
   return true;
}


//------------------------------------------------------------------------------
// virtual bool SetSpacecraft(Spacecraft *sc)
//------------------------------------------------------------------------------
/**
 * Sets spacecraft pointer to internal parameter used in stop condition.
 *
 * @return true if spacecraft has been set.
 */
//------------------------------------------------------------------------------
bool StopCondition::SetSpacecraft(SpaceObject *sc)
{
   if (mEccParam != NULL)
      mEccParam->SetRefObject(sc, Gmat::SPACECRAFT, sc->GetName());
   
   if (mRmagParam != NULL)
      mRmagParam->SetRefObject(sc, Gmat::SPACECRAFT, sc->GetName());
   
   return true;
}


//---------------------------------
// methods inherited from GmatBase
//---------------------------------

//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
bool StopCondition::RenameRefObject(const Gmat::ObjectType type,
                                    const wxString &oldName,
                                    const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("StopCondition::RenameRefObject() type=%s, oldName=%s, newName=%s\n"),
       GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif
   
   if (type != Gmat::SPACECRAFT && type != Gmat::PARAMETER)
      return true;
   
   //set new StopCondition name
   wxString name = GetName();
   wxString::size_type pos = name.find(oldName);
   if (pos != name.npos)
   {
      name = GmatStringUtil::ReplaceName(name, oldName, newName);
      SetName(name);
   }
   
   //set new epoch parameter name
   pos = mEpochParamName.find(oldName);
   if (pos != mEpochParamName.npos)
      mEpochParamName = GmatStringUtil::ReplaceName(mEpochParamName, oldName, newName);
   
   //set new stop parameter name
   pos = mStopParamName.find(oldName);
   if (pos != mStopParamName.npos)
      mStopParamName = GmatStringUtil::ReplaceName(mStopParamName, oldName, newName);
   
   //set new stop goal string
   pos = rhsString.find(oldName);
   if (pos != mStopParamName.npos)
      rhsString = GmatStringUtil::ReplaceName(rhsString, oldName, newName);
   
   return true;
}


//---------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//---------------------------------------------------------------------------
/**
 * Retrieves reference object name array for given type. It will return all
 * object names if type is Gmat::UNKNOWN_NAME.
 *
 * @param <type> object type
 * @return reference object name.
 */
//------------------------------------------------------------------------------
const StringArray&
StopCondition::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   #ifdef DEBUG_STOPCOND_OBJ
   MessageInterface::ShowMessage
      (wxT("StopCondition::GetRefObjectNameArray() entered, type=%d, mAllowGoalParam=%d, ")
       wxT("lhsWrapper=<%p>, rhsWrapper=<%p>\n"), type, mAllowGoalParam, lhsWrapper,
       rhsWrapper);
   #endif
   
   mAllRefObjectNames.clear();
   
   if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::PARAMETER)
   {
      mAllRefObjectNames.push_back(mStopParamName);
      if (mAllowGoalParam || !GmatStringUtil::IsNumber(rhsString) && rhsWrapper != NULL)
         mAllRefObjectNames.push_back(rhsString);
   }
   
   #ifdef DEBUG_STOPCOND_OBJ
   MessageInterface::ShowMessage
      (wxT("StopCondition::GetRefObjectNameArray() returning %d ref object names\n"), \
       mAllRefObjectNames.size());
   for (UnsignedInt i = 0; i < mAllRefObjectNames.size(); i++)
      MessageInterface::ShowMessage
         (wxT("   mAllRefObjectNames[%d] = '%s'\n"), i, mAllRefObjectNames[i].c_str());
   #endif
   
   return mAllRefObjectNames;
}


//---------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name)
//---------------------------------------------------------------------------
bool StopCondition::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                 const wxString &name)
{
   #ifdef DEBUG_STOPCOND_SET
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetRefObject() <%p>'%s' entered, obj=<%p>'%s', type=%d, ")
       wxT("name='%s', mStopParamName=%s, rhsString=%s\n"), this, this->GetName().c_str(),
       obj, obj ? obj->GetName().c_str() : wxT("NULL"), type, name.c_str(),
       mStopParamName.c_str(), rhsString.c_str());
   #endif
   
   if (obj == NULL)
      return false;
   
   if (type == Gmat::PARAMETER)
   {
      if (name == mStopParamName)
         SetStopParameter((Parameter*)obj);
      if (name == rhsString)
         SetGoalParameter((Parameter*)obj);
      if (name == mEpochParamName)
         SetEpochParameter((Parameter*)obj);
      
      return true;
   }
   else if (type == Gmat::INTERPOLATOR)
   {
      SetInterpolator((Interpolator*)obj);
      return true;
   }
   
   return GmatBase::SetRefObject(obj, type, name);
}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
wxString StopCondition::GetParameterText(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < StopConditionParamCount)
      return PARAMETER_TEXT[id - GmatBaseParamCount];
   else
      return GmatBase::GetParameterText(id);
    
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
Integer StopCondition::GetParameterID(const wxString &str) const
{
   #ifdef DEBUG_STOPCOND_GET
   MessageInterface::ShowMessage
      (wxT("StopCondition::GetParameterID() str = %s\n"), str.c_str());
   #endif
   
   for (int i=GmatBaseParamCount; i<StopConditionParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatBaseParamCount])
         return i;
   }
   
   return GmatBase::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
Gmat::ParameterType StopCondition::GetParameterType(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < StopConditionParamCount)
      return PARAMETER_TYPE[id - GmatBaseParamCount];
   else
      return GmatBase::GetParameterType(id);
}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
wxString StopCondition::GetParameterTypeString(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < StopConditionParamCount)
      return GmatBase::PARAM_TYPE_STRING[GetParameterType(id)];
   else
      return GmatBase::GetParameterTypeString(id);
    
}


//------------------------------------------------------------------------------
// Integer GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
Integer StopCondition::GetIntegerParameter(const Integer id) const
{
   switch (id)
   {
   case REPEAT_COUNT:
      return mRepeatCount;
   default:
      return GmatBase::GetIntegerParameter(id);
   }
}


//------------------------------------------------------------------------------
// Integer GetIntegerParameter(const wxString &label) const
//------------------------------------------------------------------------------
Integer StopCondition::GetIntegerParameter(const wxString &label) const
{
   return GetIntegerParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// Integer SetIntegerParameter(const Integer id, const Integer value)
//------------------------------------------------------------------------------
Integer StopCondition::SetIntegerParameter(const Integer id,
                                               const Integer value)
{
   switch (id)
   {
   case REPEAT_COUNT:
      mRepeatCount = value;
      return mRepeatCount;
   default:
      return GmatBase::SetIntegerParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// Integer SetIntegerParameter(const wxString &label, const Integer value)
//------------------------------------------------------------------------------
Integer StopCondition::SetIntegerParameter(const wxString &label,
                                               const Integer value)
{
   return SetIntegerParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
Real StopCondition::GetRealParameter(const Integer id) const
{
   switch (id)
   {
   case BASE_EPOCH:
      return mBaseEpoch;
   case EPOCH:
      return internalEpoch;
   default:
      return GmatBase::GetRealParameter(id);
   }
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const wxString &label) const
//------------------------------------------------------------------------------
Real StopCondition::GetRealParameter(const wxString &label) const
{
   return GetRealParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
Real StopCondition::SetRealParameter(const Integer id, const Real value)
{
   switch (id)
   {
   case BASE_EPOCH:
      mBaseEpoch = value;

      if (mAllowGoalParam && mGoalParam != NULL || rhsWrapper != NULL)
      {
         if (mAllowGoalParam)
            initialGoalValue = mGoalParam->EvaluateReal();
         else
            initialGoalValue = rhsWrapper->EvaluateReal();
      }
      
      startValue = mStopParam->EvaluateReal();
      
      // Update target for cyclic time conditions
      if (isCyclicTimeCondition)
      {
         currentGoalValue = startValue + initialGoalValue;
         #ifdef DEBUG_CYCLIC_TIME
            MessageInterface::ShowMessage(wxT("  Current value: %.12lf  ")
                  wxT("New Stop: %.12lf\n"), startValue, currentGoalValue);
         #endif
      }
      else
         currentGoalValue = initialGoalValue;
      return mBaseEpoch;
   case EPOCH:
      internalEpoch = value;
      return internalEpoch;
   default:
      return GmatBase::GetRealParameter(id);
   }
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const wxString &label, const Real value)
//------------------------------------------------------------------------------
Real StopCondition::SetRealParameter(const wxString &label, const Real value)
{
   #ifdef DEBUG_STOPCOND_SET
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetRealParameter() label=%s, ")
       wxT("value=%le\n"), label.c_str(), value);
   #endif
   
   return SetRealParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
wxString StopCondition::GetStringParameter(const Integer id) const
{
   switch (id)
   {
   case EPOCH_VAR:
      return mEpochParamName;
   case STOP_VAR:
      return mStopParamName;
   case GOAL:
      return rhsString;
   default:
      return GmatBase::GetStringParameter(id);
   }
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString StopCondition::GetStringParameter(const wxString &label) const
{
   #ifdef DEBUG_STOPCOND_GET
   MessageInterface::ShowMessage
      (wxT("StopCondition::GetStringParameter() label = %s\n"), label.c_str());
   #endif
   
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
bool StopCondition::SetStringParameter(const Integer id, const wxString &value)
{
   #ifdef DEBUG_STOPCOND_SET
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetStringParameter() this=<%p>'%s' entered, id = %d, ")
       wxT("value = %s \n"), this, GetName().c_str(), id, value.c_str());
   #endif
   
   switch (id)
   {
   case EPOCH_VAR:
      mEpochParamName = value;
      return true;
   case STOP_VAR:
      mStopParamName = value;
      SetLhsString(value);
      return true;
   case GOAL:
      SetRhsString(value);
      return true;
   default:
      return GmatBase::SetStringParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label,
//                         const wxString &value)
//------------------------------------------------------------------------------
bool StopCondition::SetStringParameter(const wxString &label,
                                           const wxString &value)
{
   #ifdef DEBUG_STOPCOND_SET
   MessageInterface::ShowMessage
      (wxT("StopCondition::SetStringParameter() label = %s, ")
       wxT("value = %s \n"), label.c_str(), value.c_str());
   #endif
   
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// Real GetStopValue()
//------------------------------------------------------------------------------
/**
 * Used to access the last calculated stopping condition value.
 * 
 * @return The last calculated value obtained from the mStopParam member.
 */ 
//------------------------------------------------------------------------------
Real StopCondition::GetStopValue()
{
   return previousAchievedValue;
}


//------------------------------------------------------------------------------
// Real GetStopDifference()
//------------------------------------------------------------------------------
/**
 * Used to access the difference between the last calculated stopping condition 
 * value and the desired value.
 * 
 * @return The difference between the goal and achieved value (goal - achieved).
 */ 
//------------------------------------------------------------------------------
Real StopCondition::GetStopDifference()
{
   Real goalValue, achievedValue;
   if (mAllowGoalParam && mGoalParam || rhsWrapper != NULL)
   {
      if (mAllowGoalParam)
         goalValue = mGoalParam->EvaluateReal();
      else
         goalValue = rhsWrapper->EvaluateReal();
      
      if (isCyclicTimeCondition)
         goalValue += startValue;
   }
   else
      goalValue = currentGoalValue;
   
   #ifdef DEBUG_STOPCOND_EVAL
      MessageInterface::ShowMessage(wxT("%s goal (%s) = %16.9lf, value = %16.9lf\n"), 
         instanceName.c_str(), 
         (mGoalParam ? mGoalParam->GetName().c_str() : wxT("Fixed goal")), goalValue,
         mStopParam->EvaluateReal());
   #endif
      
   achievedValue = mStopParam->EvaluateReal();
   Real min, max, delta;
   GetRange(min, max);
   delta = (max - min) * 0.5;
   if (IsCyclicParameter())
      achievedValue = PutInRange(achievedValue, goalValue-delta, 
            goalValue + delta);
   
   return goalValue - achievedValue;
}


//------------------------------------------------------------------------------
// Real GetStopGoal()
//------------------------------------------------------------------------------
/**
 * Used to access the desired stopping value.
 * 
 * @return The goal value.
 */ 
//------------------------------------------------------------------------------
Real StopCondition::GetStopGoal()
{
   Real goalValue;
   if (mAllowGoalParam && mGoalParam || rhsWrapper != NULL)
   {
      if (mAllowGoalParam)
         goalValue = mGoalParam->EvaluateReal();
      else
         goalValue = rhsWrapper->EvaluateReal();
   }
   else
      goalValue = currentGoalValue;
   
   if (isLhsCyclicCondition)
   {
      #ifdef DEBUG_CYCLIC_PARAMETERS
         MessageInterface::ShowMessage(wxT("Goal: %.12lf is cyclic; "), goalValue);
      #endif
      Real min, max;
      GetRange(min, max);
      goalValue = PutInRange(goalValue, min, max);
      #ifdef DEBUG_CYCLIC_PARAMETERS
         MessageInterface::ShowMessage(wxT("adjusted to [%lf, %lf] gives: %.12lf\n"),
               min, max, goalValue);
      #endif
   }

   if (isCyclicTimeCondition)
      goalValue += startValue;

   return goalValue;
}


//------------------------------------------------------------------------------
// Real GetStopTime()
//------------------------------------------------------------------------------
/**
 * Used to access the last calculated stopping condition epoch.
 * 
 * @return The last calculated value obtained from the mEpochParam member.
 */ 
//------------------------------------------------------------------------------
Real StopCondition::GetStopTime()
{
   return previousEpoch;
}


//------------------------------------------------------------------------------
// Real GetTimeMultiplier()
//------------------------------------------------------------------------------
/**
 * Used to find the conversion factor for changing time parameters to seconds.
 * 
 * @return The factor.
 */ 
//------------------------------------------------------------------------------
Real StopCondition::GetTimeMultiplier()
{
   switch (stopParamTimeType)
   {
      case MINUTE_PARAM:
         return GmatTimeConstants::SECS_PER_MINUTE;

      case HOUR_PARAM:
         return GmatTimeConstants::SECS_PER_HOUR;
      
      case DAY_PARAM:
      case EPOCH_PARAM:
         return GmatTimeConstants::SECS_PER_DAY;
      
      // All intentional fall-throughs
      case SECOND_PARAM:
      case NOT_TIME_PARAM:
      case UNKNOWN_PARAM_TIME_TYPE:
      default:
         return 1.0;
   };
   
   return 1.0;
}


//------------------------------------------------------------------------------
// bool IsCyclicParameter()
//------------------------------------------------------------------------------
/**
 * Access method used to determine if the stop parameter is cyclic.
 * 
 * @return true for cyclic parameters, false if the parameter is not cyclic.
 */ 
//------------------------------------------------------------------------------
bool StopCondition::IsCyclicParameter()
{
   return isLhsCyclicCondition;
}


//------------------------------------------------------------------------------
// bool GetRange(Real &min, Real &max)
//------------------------------------------------------------------------------
/**
 * Retrieves the minimum and maximum values for the range variable.
 * 
 * @param <min> Reference to the variable that, upon return, contains the range 
 *              minimum
 * @param <max> Reference to the variable that, upon return, contains the range 
 *              maximum
 * 
 * @return true if the references were filled in, false if there has a problem 
 *         setting the range values.
 */ 
//------------------------------------------------------------------------------
bool StopCondition::GetRange(Real &min, Real &max)
{
   bool retval = true;
   
   if (!isLhsCyclicCondition)
      retval = false;
   else
   {
      // Changed to use CycleType from GmatParam in Parameter (LOJ: 2010.07.08)
      switch (lhsCycleType)
      {
         case GmatParam::ZERO_90:
            min = 0.0;
            max = 90.0;
            break;
            
         case GmatParam::ZERO_180:
            min = 0.0;
            max = 180.0;
            break;
            
         case GmatParam::ZERO_360:
            min = 0.0;
            max = 360.0;
            break;
            
         case GmatParam::PLUS_MINUS_90:
            min = -90.0;
            max = 90.0;
            break;
            
         case GmatParam::PLUS_MINUS_180:
            min = -180.0;
            max = 180.0;
            break;
            
         case GmatParam::OTHER_CYCLIC:   // Intentional drop-through
         case GmatParam::NOT_CYCLIC:     // Intentional drop-through
         default:
            retval = false;
      };
   }
   
   return retval;
}


//------------------------------------------------------------------------------
// Real PutInRange(const Real value, const Real min, const Real max, 
//                 bool isReflection)
//------------------------------------------------------------------------------
/**
 * Returns the parameter value that matches the input value for a cyclic 
 * parameter, remapped to fall between the minimum and maximum values allowed.
 * 
 * @param <value>        The input value of the parameter
 * @param <min>          The minimum value of the parameter
 * @param <max>          The maximum value of the parameter
 * @param <isReflection> Flag indicating if the parameter should be reflected 
 *                       into the valid range.  This flag is not yet used.
 * 
 * @return the parameter value, mapped into the valid range.
 */ 
//------------------------------------------------------------------------------
Real StopCondition::PutInRange(const Real value, const Real min, const Real max,
                                bool isReflection)
{
   if (min >= max)
      throw StopConditionException(wxT("PutInRange received bad range limits\n"));
   Real range = max - min, retval = value;
   
   while (retval < min)
      retval += range;

   while (retval > max)
      retval -= range;
      
   if (isReflection)
   {
      // Not yet needed or implemented
   }
      
   return retval;
}

//------------------------------------------------------------------------------
// void SkipEvaluation(bool shouldSkip)
//------------------------------------------------------------------------------
/**
 * Function used to toggle stopping conditions is active or inactive.
 * 
 * @param <shouldSkip> Flag indicating the status.  If this parameter is true,
 *                     the stopping condition is deactivated; if false, it is
 *                     activated. 
 */ 
//------------------------------------------------------------------------------
void StopCondition::SkipEvaluation(bool shouldSkip)
{
   if (mStopParam->GetName().find(wxT("ModJulian")) == wxString::npos)
      activated = !shouldSkip;
}


//------------------------------------------------------------------------------
// void UpdateBuffer()
//------------------------------------------------------------------------------
/**
 * Sets the most recent value of the stop parameter and epoch to be the 
 * previously calculated values. 
 */ 
//------------------------------------------------------------------------------
void StopCondition::UpdateBuffer()
{
   previousAchievedValue = mStopParam->EvaluateReal();

   // evaluate goal in case needed for cyclics
   if (mAllowGoalParam && mGoalParam != NULL || rhsWrapper != NULL)
   {
      if (mAllowGoalParam)
         initialGoalValue = mGoalParam->EvaluateReal();
      else
         initialGoalValue = rhsWrapper->EvaluateReal();
      
      if (isCyclicTimeCondition)
         currentGoalValue = startValue + initialGoalValue;
      else
         currentGoalValue = initialGoalValue;
   }
   
   // set current epoch
   Real epoch;
   if (mUseInternalEpoch)
      epoch = internalEpoch;
   else
      epoch = mEpochParam->EvaluateReal();
   
   if (isLhsCyclicCondition)
   {
      currentGoalValue = PutInRange(currentGoalValue, 0.0, GmatMathConstants::TWO_PI_DEG);
      CheckCyclicCondition(previousAchievedValue);
   }
   
   previousEpoch = epoch;
   
   #ifdef DEBUG_CYCLIC_PARAMETERS
      MessageInterface::ShowMessage(
         wxT("Updated \"previous\" values to [previousAchievedValue  previousEpoch]")
         wxT(" = [%.12lf  %.12lf]\n"), previousAchievedValue, previousEpoch);
   #endif
}


//---------------------------------
// private methods
//---------------------------------

//------------------------------------------------------------------------------
// void CopyDynamicData()
//------------------------------------------------------------------------------
void StopCondition::CopyDynamicData(const StopCondition &stopCond)
{
   mNumValidPoints = stopCond.mNumValidPoints;
   mBufferSize = stopCond.mBufferSize;

   if ((Integer)mEpochBuffer.size() < mBufferSize)
      mEpochBuffer.reserve(mBufferSize);
   
   if ((Integer)lhsValueBuffer.size() < mBufferSize)
      lhsValueBuffer.reserve(mBufferSize);
   
   if ((Integer)rhsValueBuffer.size() < mBufferSize)
      rhsValueBuffer.reserve(mBufferSize);
   
   for (int i=0; i<mBufferSize; i++)
   {
      mEpochBuffer[i] = stopCond.mEpochBuffer[i];
      lhsValueBuffer[i] = stopCond.lhsValueBuffer[i];
      rhsValueBuffer[i] = stopCond.rhsValueBuffer[i];
   }
}
