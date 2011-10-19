//$Id: OrbitView.hpp 9596 2011-06-16 14:53:09Z lindajun $
//------------------------------------------------------------------------------
//                                  OrbitView
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
// Created: 2010/04/19
//
/**
 * Declares OrbitView class.
 */
//------------------------------------------------------------------------------
#ifndef OrbitView_hpp
#define OrbitView_hpp

#include "OrbitPlot.hpp"
#include "SpacePoint.hpp"
#include "CoordinateSystem.hpp"
#include <map>

class GMAT_API OrbitView : public OrbitPlot
{
public:
   OrbitView(const wxString &name);
   OrbitView(const OrbitView &ogl);
   OrbitView& operator=(const OrbitView&);
   virtual ~OrbitView(void);
   
   Rvector3             GetVector(const wxString &which);
   void                 SetVector(const wxString &which, const Rvector3 &value);
   
   // methods inherited from GmatBase
   virtual bool         Validate();
   virtual bool         Initialize();
   
   virtual GmatBase*    Clone() const;
   virtual void         Copy(const GmatBase* orig);
   
   virtual bool         TakeAction(const wxString &action,  
                                   const wxString &actionData = wxT(""));
   
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   
   // methods for parameters
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   virtual bool         IsParameterReadOnly(const Integer id) const;
   
   virtual Integer      GetIntegerParameter(const Integer id) const;
   virtual Integer      SetIntegerParameter(const Integer id, const Integer value);
   virtual Integer      GetIntegerParameter(const wxString &label) const;
   virtual Integer      SetIntegerParameter(const wxString &label,
                                            const Integer value);
   
   virtual Real         GetRealParameter(const Integer id) const;
   virtual Real         SetRealParameter(const Integer id, const Real value);
   virtual Real         GetRealParameter(const wxString &label) const;
   virtual Real         SetRealParameter(const wxString &label, const Real value);
   
   virtual Real         GetRealParameter(const Integer id,
                                         const Integer index) const;
   virtual Real         SetRealParameter(const Integer id,
                                         const Real value,
                                         const Integer index);
   
   virtual const Rvector& GetRvectorParameter(const Integer id) const;
   virtual const Rvector& SetRvectorParameter(const Integer id,
                                              const Rvector &value);
   virtual const Rvector& GetRvectorParameter(const wxString &label) const;
   virtual const Rvector& SetRvectorParameter(const wxString &label,
                                              const Rvector &value);
   
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual bool         SetStringParameter(const Integer id, const wxString &value);
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value);
   
   virtual bool         SetStringParameter(const Integer id, const wxString &value,
                                           const Integer index);
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value,
                                           const Integer index);
   
   virtual wxString  GetOnOffParameter(const Integer id) const;
   virtual bool         SetOnOffParameter(const Integer id, 
                                          const wxString &value);
   virtual wxString  GetOnOffParameter(const wxString &label) const;
   virtual bool         SetOnOffParameter(const wxString &label, 
                                          const wxString &value);
   
   virtual wxString  GetRefObjectName(const Gmat::ObjectType type) const;
   virtual bool         HasRefObjectTypeArray();
   virtual const ObjectTypeArray&
                        GetRefObjectTypeArray();
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name = wxT(""));
   
protected:

   void     PutRvector3Value(Rvector3 &rvec3, Integer id,
                             const wxString &sval, Integer index = -1);
   void     WriteDeprecatedMessage(Integer id) const;
   bool     UpdateSolverData();
   
   CoordinateSystem *mViewUpCoordSystem;
   SpacePoint *mViewCoordSysOrigin;
   SpacePoint *mViewUpCoordSysOrigin;
   SpacePoint *mViewPointRefObj;
   SpacePoint *mViewPointObj;
   SpacePoint *mViewDirectionObj;
   
   wxString mEclipticPlane;
   wxString mXYPlane;
   wxString mWireFrame;
   wxString mOverlapPlot;
   wxString mUseInitialView;
   wxString mAxes;
   wxString mGrid;
   wxString mSunLine;
   
   wxString mViewPointRefName;
   wxString mViewPointRefType;
   wxString mViewPointVecName;
   wxString mViewPointVecType;
   wxString mViewDirectionName;
   wxString mViewDirectionType;
   wxString mViewUpCoordSysName;
   wxString mViewUpAxisName;
   
   Rvector3 mViewPointRefVector;
   Rvector3 mViewPointVecVector;
   Rvector3 mViewDirectionVector;
   
   Real mViewScaleFactor;

   // stars
   wxString mEnableStars;
   wxString mEnableConstellations;
   Integer mStarCount;
   
   // FOV - currentrly not used and will be removed later
   Integer mMinFOV;
   Integer mMaxFOV;
   Integer mInitialFOV;
   
   enum
   {
      VIEWPOINT_REF = OrbitPlotParamCount,
      VIEWPOINT_REFERENCE,
      VIEWPOINT_REF_TYPE,
      VIEWPOINT_REF_VECTOR,
      VIEWPOINT_VECTOR,
      VIEWPOINT_VECTOR_TYPE,
      VIEWPOINT_VECTOR_VECTOR,
      VIEW_DIRECTION,
      VIEW_DIRECTION_TYPE,
      VIEW_DIRECTION_VECTOR,
      VIEW_SCALE_FACTOR,
      VIEW_UP_COORD_SYSTEM,
      VIEW_UP_AXIS,
      CELESTIAL_PLANE,
      XY_PLANE,
      WIRE_FRAME,
      AXES,
      GRID,
      EARTH_SUN_LINES,
      SUN_LINE,
      OVERLAP_PLOT,
      USE_INITIAL_VIEW,
      STAR_COUNT,
      ENABLE_STARS,
      ENABLE_CONSTELLATIONS,
      MIN_FOV,
      MAX_FOV,
      INITIAL_FOV,
      OrbitViewParamCount
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[OrbitViewParamCount - OrbitPlotParamCount];
   static const wxString
      PARAMETER_TEXT[OrbitViewParamCount - OrbitPlotParamCount];
   
   virtual bool Distribute(Integer len);
   virtual bool Distribute(const Real * dat, Integer len);
      
};

#endif
