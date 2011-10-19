//$Id: OpenGlPlot.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  OpenGlPlot
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
// Created: 2003/12/16
//
/**
 * Declares OpenGlPlot class.
 */
//------------------------------------------------------------------------------
#ifndef OpenGlPlot_hpp
#define OpenGlPlot_hpp

#include "Subscriber.hpp"
#include "SpacePoint.hpp"
#include "CoordinateSystem.hpp"
#include <map>

class GMAT_API OpenGlPlot : public Subscriber
{
public:
   OpenGlPlot(const wxString &name);
   OpenGlPlot(const OpenGlPlot &ogl);
   OpenGlPlot& operator=(const OpenGlPlot&);
   virtual ~OpenGlPlot(void);
   
   const StringArray&   GetSpacePointList();
   const StringArray&   GetSpacecraftList();
   const StringArray&   GetNonSpacecraftList();
   
   UnsignedInt          GetColor(const wxString &item, const wxString &scName);
   bool                 SetColor(const wxString &item, const wxString &name,
                                 UnsignedInt value);
   bool                 GetShowObject(const wxString &name);
   void                 SetShowObject(const wxString &name, bool value);
   
   Rvector3             GetVector(const wxString &which);
   void                 SetVector(const wxString &which, const Rvector3 &value);
   
   // methods inherited from Subscriber
   virtual bool         Initialize();
   virtual void         Activate(bool state = true);
   
   // methods inherited from GmatBase
   virtual GmatBase*    Clone() const;
   virtual void         Copy(const GmatBase* orig);
   
   virtual bool         SetName(const wxString &who,
                                const wxString &oldName = wxT(""));
   
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
   
   virtual UnsignedInt  SetUnsignedIntParameter(const Integer id,
                                                const UnsignedInt value,
                                                const Integer index);
   
   virtual const UnsignedIntArray&
                        GetUnsignedIntArrayParameter(const Integer id) const;   
   virtual const StringArray&
                        GetStringArrayParameter(const Integer id) const;
   
   virtual bool         GetBooleanParameter(const Integer id) const;
   virtual bool         SetBooleanParameter(const Integer id,
                                            const bool value);
   
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

   bool     AddSpacePoint(const wxString &name, Integer index,
                          bool show = true);
   bool     ClearSpacePointList();
   bool     RemoveSpacePoint(const wxString &name);
   void     ClearDynamicArrays();
   void     UpdateObjectList(SpacePoint *sp, bool show = false);
   void     PutRvector3Value(Rvector3 &rvec3, Integer id,
                             const wxString &sval, Integer index = -1);
   void     PutUnsignedIntValue(Integer id, const wxString &sval);
   void     WriteDeprecatedMessage(Integer id) const;
   bool     UpdateSolverData();
   
   CoordinateSystem *mViewCoordSystem;
   CoordinateSystem *mViewUpCoordSystem;
   SpacePoint *mViewCoordSysOrigin;
   SpacePoint *mViewUpCoordSysOrigin;
   SpacePoint *mViewPointRefObj;
   SpacePoint *mViewPointObj;
   SpacePoint *mViewDirectionObj;
   std::vector<SpacePoint*> mObjectArray;
   std::vector<SpacePoint*> mAllSpArray;
   std::vector<bool> mDrawOrbitArray;
   std::vector<bool> mShowObjectArray;
   
   wxString mEclipticPlane;
   wxString mXYPlane;
   wxString mWireFrame;
   wxString mOverlapPlot;
   wxString mUseInitialView;
   wxString mPerspectiveMode;
   wxString mUseFixedFov;
   wxString mAxes;
   wxString mGrid;
   wxString mSunLine;
   
   wxString mOldName;
   wxString mViewCoordSysName;
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
   Real mFixedFovAngle;
   
   Integer mDataCollectFrequency;
   Integer mUpdatePlotFrequency;
   Integer mNumPointsToRedraw;
   Integer mNumData;
   Integer mNumCollected;
   
   Integer mAllSpCount;
   Integer mScCount;
   Integer mObjectCount;
   Integer mNonStdBodyCount;

	wxString mEnableStars;
	wxString mEnableConstellations;
	Integer mStarCount;

	Integer mMinFOV;
	Integer mMaxFOV;
	Integer mInitialFOV;
   
   StringArray mScNameArray;
   StringArray mObjectNameArray;
   StringArray mAllSpNameArray;
   StringArray mAllRefObjectNames;
   
   // arrays for holding distrubuted data
   RealArray mScXArray;
   RealArray mScYArray;
   RealArray mScZArray;
   RealArray mScVxArray;
   RealArray mScVyArray;
   RealArray mScVzArray;
   UnsignedIntArray mScOrbitColorArray;
   UnsignedIntArray mScTargetColorArray;
   UnsignedIntArray mOrbitColorArray;
   UnsignedIntArray mTargetColorArray;
   
   // arrays for holding solver current data
   std::vector<StringArray> mCurrScArray;
   std::vector<Real> mCurrEpochArray;
   std::vector<RealArray> mCurrXArray;
   std::vector<RealArray> mCurrYArray;
   std::vector<RealArray> mCurrZArray;
   std::vector<RealArray> mCurrVxArray;
   std::vector<RealArray> mCurrVyArray;
   std::vector<RealArray> mCurrVzArray;
   
   std::map<wxString, UnsignedInt> mOrbitColorMap;
   std::map<wxString, UnsignedInt> mTargetColorMap;
   std::map<wxString, bool> mDrawOrbitMap;
   std::map<wxString, bool> mShowObjectMap;
   
   enum
   {
      ADD = SubscriberParamCount,
      ORBIT_COLOR,
      TARGET_COLOR,
      COORD_SYSTEM,
      VIEWPOINT_REF,
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
      FIXED_FOV_ANGLE,
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
      PERSPECTIVE_MODE,
      USE_FIXED_FOV,
      DATA_COLLECT_FREQUENCY,
      UPDATE_PLOT_FREQUENCY,
      NUM_POINTS_TO_REDRAW,
      SHOW_PLOT,
		STAR_COUNT,
		ENABLE_STARS,
		ENABLE_CONSTELLATIONS,
		MIN_FOV,
		MAX_FOV,
		INITIAL_FOV,
      OpenGlPlotParamCount
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[OpenGlPlotParamCount - SubscriberParamCount];
   static const wxString
      PARAMETER_TEXT[OpenGlPlotParamCount - SubscriberParamCount];
   
   virtual bool Distribute(Integer len);
   virtual bool Distribute(const Real * dat, Integer len);
   
   const static int MAX_SP_COLOR = 15;
   static const UnsignedInt DEFAULT_ORBIT_COLOR[MAX_SP_COLOR];
   
};

#endif
