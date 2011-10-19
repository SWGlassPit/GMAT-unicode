//$Id: StopCondition.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              StopCondition
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
 * Declares StopCondition class.
 */
//------------------------------------------------------------------------------
#ifndef StopCondition_hpp
#define StopCondition_hpp

#include "gmatdefs.hpp"

#include "paramdefs.hpp"
#include "GmatBase.hpp"
#include "Interpolator.hpp"
#include "SolarSystem.hpp"
#include "Parameter.hpp"
#include "Spacecraft.hpp"
#include "Variable.hpp"
#include "ElementWrapper.hpp"

class GMAT_API StopCondition : public GmatBase
{
public:

   static const Real STOP_COND_TOL;

   StopCondition(const wxString &name = wxT(""),
                 const wxString &desc = wxT(""),
                 Parameter *epochParam = NULL, 
                 Parameter *stopParam = NULL,
                 const Real &goal = GmatBase::REAL_PARAMETER_UNDEFINED,
                 const Real &tol = STOP_COND_TOL,
                 const Integer repeatCount = 1,
                 Interpolator *interp = NULL);
   StopCondition(const StopCondition &copy);
   StopCondition& operator= (const StopCondition &right); 
   virtual ~StopCondition();
   
   bool Initialize();
   virtual bool Validate();
   virtual bool Evaluate();
   virtual bool IsTimeCondition();
   virtual bool AddToBuffer(bool isInitialPoint);
   virtual Real GetStopEpoch();
   void Reset();
   virtual Real GetStopInterval();
   
   bool IsInitialized();
   Integer GetBufferSize();
   wxString& GetDescription();
   Parameter* GetEpochParameter();
   Parameter* GetStopParameter();
   Parameter* GetGoalParameter();
   Interpolator* GetInterpolator();
   
   void SetDescription(const wxString &desc);
   void SetPropDirection(Real dir);
   void SetSolarSystem(SolarSystem *solarSystem);
   bool SetInterpolator(Interpolator *interp);
   bool SetEpochParameter(Parameter *param);
   bool SetStopParameter(Parameter *param);
   bool SetGoalParameter(Parameter *param);
   
   void SetLhsString(const wxString &str);
   void SetRhsString(const wxString &str);
   
   wxString GetLhsString();
   wxString GetRhsString();
   
   bool SetLhsWrapper(ElementWrapper* toWrapper);
   bool SetRhsWrapper(ElementWrapper* toWrapper);
   
   virtual bool SetSpacecraft(SpaceObject *sc);
   
   // methods inherited from GmatBase
   virtual GmatBase* Clone() const;
   
   virtual bool RenameRefObject(const Gmat::ObjectType type,
                                const wxString &oldName,
                                const wxString &newName);
   
   virtual const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                             const wxString &name = wxT(""));
   
   virtual wxString GetParameterText(const Integer id) const;
   virtual Integer GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType GetParameterType(const Integer id) const;
   virtual wxString GetParameterTypeString(const Integer id) const;

   virtual Integer GetIntegerParameter(const Integer id) const;
   virtual Integer GetIntegerParameter(const wxString &label) const;
   virtual Integer SetIntegerParameter(const Integer id,
                                       const Integer value);
   virtual Integer SetIntegerParameter(const wxString &label,
                                       const Integer value);
   
   virtual Real GetRealParameter(const Integer id) const;
   virtual Real GetRealParameter(const wxString &label) const;
   virtual Real SetRealParameter(const Integer id, const Real value);
   virtual Real SetRealParameter(const wxString &label, const Real value);
   
   virtual wxString GetStringParameter(const Integer id) const;
   virtual wxString GetStringParameter(const wxString &label) const;
   virtual bool SetStringParameter(const Integer id, const wxString &value);
   virtual bool SetStringParameter(const wxString &label,
                                   const wxString &value);
   
   // Accessors for the last condition evaluated
   virtual Real GetStopValue();
   virtual Real GetStopDifference();
   virtual Real GetStopGoal();
   virtual Real GetStopTime();
   virtual Real GetTimeMultiplier();
   virtual bool IsCyclicParameter();
   virtual bool GetRange(Real &min, Real &max);
   virtual Real PutInRange(const Real value, const Real min, const Real max, 
                           const  bool isReflection = false);
   virtual void SkipEvaluation(bool shouldSkip);
   virtual void UpdateBuffer();
   
protected:
   
   StringArray mAllRefObjectNames;
   
   Real mBaseEpoch;
   Real internalEpoch;
   Real currentGoalValue;
   Integer mRepeatCount;
   SolarSystem *mSolarSystem;
   
   Interpolator *mInterpolator;
   wxString mDescription;
   wxString mStopParamType;
   wxString mStopParamName;
   wxString mEpochParamName;
   wxString lhsString;
   wxString rhsString;
   
   /// left hand side Parameter of stopping condition
   Parameter *mStopParam;
   /// right hand side Parameter of stopping condition
   Parameter *mGoalParam;
   Parameter *mEpochParam;
   Parameter *mEccParam;
   Parameter *mRmagParam;
   
   ElementWrapper *lhsWrapper;
   ElementWrapper *rhsWrapper;
   
   /// ring buffer for epochs
   RealArray mEpochBuffer;
   /// ring buffer for associated LHS values
   RealArray lhsValueBuffer;
   /// ring buffer for associated RHS values
   RealArray rhsValueBuffer;
   
   Integer mNumValidPoints;
   Integer mBufferSize;
   Real mStopEpoch;
   Real mStopInterval;
   
   // History data used instead of ring buffer for general propagation (before
   // a stopping condition triggers
   Real previousEpoch;
   Real previousAchievedValue;
   Real previousGoalValue;
   
   bool mUseInternalEpoch;
   bool mInitialized;
   bool mNeedInterpolator;
   bool mAllowGoalParam;
   bool mBackwardsProp;
   bool activated;
   
   // Flags used to mark special cases
   bool isLhsCyclicCondition;
   bool isRhsCyclicCondition;
   bool isPeriapse;
   bool isApoapse;
   bool isCyclicTimeCondition;      // Used for Elapsed... time conditions
   Real startValue;
   Real initialGoalValue;
   
   // The CycleType moved to GmatParam namespace defined in Parameter.hpp
   //enum CycleType
   //{
   //   NOT_CYCLIC,
   //   ZERO_90,
   //   ZERO_180,
   //   ZERO_360,
   //   PLUS_MINUS_90,
   //   PLUS_MINUS_180,
   //   OTHER_CYCLIC
   //};
   
   // To handle stopping condition of Parameters in both sides
   // such as Sat1.TA = Sat2.TA (future implementation)
   GmatParam::CycleType lhsCycleType;
   GmatParam::CycleType rhsCycleType;
   
   enum TimeType
   {
      NOT_TIME_PARAM,
      SECOND_PARAM,
      MINUTE_PARAM,
      HOUR_PARAM,
      DAY_PARAM,
      EPOCH_PARAM,
      UNKNOWN_PARAM_TIME_TYPE
   };
   
   TimeType stopParamTimeType;
   
   enum
   {
      BASE_EPOCH = GmatBaseParamCount,
      EPOCH,
      EPOCH_VAR,
      STOP_VAR,
      GOAL,
      REPEAT_COUNT,
      StopConditionParamCount,
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[StopConditionParamCount - GmatBaseParamCount];
   static const wxString
      PARAMETER_TEXT[StopConditionParamCount - GmatBaseParamCount];

   bool CheckOnPeriapsis();
   bool CheckOnApoapsis();
   bool CheckCyclicCondition(Real &value);

private:
   void CopyDynamicData(const StopCondition &stopCond);
};

#endif // StopCondition_hpp
