//$Id: Parameter.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  Parameter
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
// Created: 2003/09/16
//
/**
 * Declares base class of parameters.
 */
//------------------------------------------------------------------------------
#ifndef Parameter_hpp
#define Parameter_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "Rvector6.hpp"
#include "Rmatrix.hpp"
#include "SolarSystem.hpp"
#include "CoordinateSystem.hpp"

namespace GmatParam
{
   enum ParameterKey
   {
      SYSTEM_PARAM, USER_PARAM, KeyCount
   };
   
   enum DepObject
   {
      COORD_SYS, ORIGIN, NO_DEP, OWNED_OBJ, DepObjectCount
   };
   
   enum CycleType
   {
      NOT_CYCLIC,
      ZERO_90,
      ZERO_180,
      ZERO_360,
      PLUS_MINUS_90,
      PLUS_MINUS_180,
      OTHER_CYCLIC
   };
};

class GMAT_API Parameter : public GmatBase
{
public:

   Parameter(const wxString &name, const wxString &typeStr,
             GmatParam::ParameterKey key, GmatBase *obj,
             const wxString &desc, const wxString &unit,
             GmatParam::DepObject depObj, Gmat::ObjectType ownerType,
             bool isTimeParam, bool isSettable, bool isPlottable,
             bool isReportable);
   Parameter(const Parameter &copy);
   Parameter& operator= (const Parameter& right);
   virtual ~Parameter();
   
   GmatParam::ParameterKey  GetKey() const;
   Gmat::ObjectType         GetOwnerType() const;
   Gmat::ParameterType      GetReturnType() const;
   GmatParam::CycleType     GetCycleType() const;
   
   void  SetKey(const GmatParam::ParameterKey &key);
   
   bool  IsAngleParameter() const;
   bool  IsTimeParameter() const;
   bool  IsPlottable() const;
   bool  IsReportable() const;
   bool  IsSettable() const;
   bool  IsCoordSysDependent() const;
   bool  IsOriginDependent() const;
   bool  NeedCoordSystem() const;
   
   bool operator==(const Parameter &right) const;
   bool operator!=(const Parameter &right) const;
   
   virtual wxString        ToString();
   
   virtual Real               GetReal() const;
   virtual const Rvector6&    GetRvector6() const;
   virtual const Rmatrix66&   GetRmatrix66() const;
   virtual const Rmatrix33&   GetRmatrix33() const;
   virtual const Rmatrix&     GetRmatrix() const;
   virtual const wxString& GetString() const;
   
   virtual void               SetReal(Real val);
   virtual void               SetRvector6(const Rvector6 &val);
   virtual void               SetRmatrix66(const Rmatrix66 &mat);
   virtual void               SetRmatrix33(const Rmatrix33 &mat);
   virtual void               SetRmatrix(const Rmatrix &mat);
   virtual void               SetString(const wxString &val);
   
   virtual Real               EvaluateReal();
   virtual const Rvector6&    EvaluateRvector6();
   virtual const Rmatrix66&   EvaluateRmatrix66();
   virtual const Rmatrix33&   EvaluateRmatrix33();
   virtual const Rmatrix&     EvaluateRmatrix();
   virtual const wxString& EvaluateString();
   
   virtual const wxString* GetParameterList() const;
   
   virtual CoordinateSystem*  GetInternalCoordSystem();
   
   virtual void         SetInternalCoordSystem(CoordinateSystem *cs);
   virtual void         SetSolarSystem(SolarSystem *ss);
   
   virtual bool         Initialize();
   virtual bool         Evaluate();
   
   // methods all SYSTEM_PARAM should implement
   virtual bool         AddRefObject(GmatBase *object, bool replaceName = false);
   virtual Integer      GetNumRefObjects() const;
   virtual bool         Validate();
   
   // methods inherited from GmatBase
   virtual void         Copy(const GmatBase*);
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   virtual bool         IsParameterReadOnly(const Integer id) const;
   
   virtual UnsignedInt  GetUnsignedIntParameter(const Integer id) const;
   virtual UnsignedInt  GetUnsignedIntParameter(const wxString &label) const;
   virtual UnsignedInt  SetUnsignedIntParameter(const Integer id,
                                                const UnsignedInt value);
   virtual UnsignedInt  SetUnsignedIntParameter(const wxString &label,
                                                const UnsignedInt value);
   
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value);
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value);
   
   virtual const wxString   GetCommentLine(Integer which = 1);
   virtual void                SetCommentLine(const wxString &comment,
                                              Integer which = 0);
   
protected:
   
   static const wxString PARAMETER_KEY_STRING[GmatParam::KeyCount];
   
   GmatParam::ParameterKey  mKey;
   wxString   mDesc;
   wxString   mUnit;
   wxString   mExpr;
   wxString   mOwnerName;
   wxString   mDepObjectName;
   wxString   mCommentLine2;
   wxString   mInitialValue;
   
   Gmat::ObjectType     mOwnerType;
   Gmat::ParameterType  mReturnType;
   GmatParam::DepObject mDepObj;
   GmatParam::CycleType mCycleType;
   UnsignedInt          mColor;
   
   bool mIsAngleParam;
   bool mIsTimeParam;
   bool mIsPlottable;
   bool mIsReportable;
   bool mIsSettable;
   bool mIsCoordSysDependent;
   bool mIsOriginDependent;
   bool mNeedCoordSystem;
   bool mIsCommentFromCreate;
   
   enum
   {
      OBJECT = GmatBaseParamCount,
      INITIAL_VALUE,
      EXPRESSION,
      DESCRIPTION,
      UNIT,
      DEP_OBJECT,
      COLOR,
      ParameterParamCount
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[ParameterParamCount - GmatBaseParamCount];
   static const wxString
      PARAMETER_TEXT[ParameterParamCount - GmatBaseParamCount];
};
#endif // Parameter_hpp

