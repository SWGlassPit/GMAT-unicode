//$Id: OrbitPlot.hpp 9914 2011-09-26 19:07:00Z lindajun $
//------------------------------------------------------------------------------
//                                  OrbitPlot
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number X-XXXX-X
//
// Author: Linda Jun
// Created: 2011/05/23
//
/**
 * Declares OrbitPlot class.
 */
//------------------------------------------------------------------------------
#ifndef OrbitPlot_hpp
#define OrbitPlot_hpp

#include "Subscriber.hpp"
#include "SpacePoint.hpp"
#include "CoordinateSystem.hpp"
#include <map>

class GMAT_API OrbitPlot : public Subscriber
{
public:
   OrbitPlot(const wxString &type, const wxString &name);
   OrbitPlot(const OrbitPlot &op);
   OrbitPlot& operator=(const OrbitPlot&);
   virtual ~OrbitPlot();
   
   const StringArray&   GetSpacePointList();
   const StringArray&   GetSpacecraftList();
   const StringArray&   GetNonSpacecraftList();
   
   UnsignedInt          GetColor(const wxString &item, const wxString &objName);
   bool                 SetColor(const wxString &item, const wxString &objName,
                                 UnsignedInt value);
   bool                 GetShowObject(const wxString &name);
   void                 SetShowObject(const wxString &name, bool value);
   
   // methods inherited from Subscriber
   virtual void         Activate(bool state = true);
   
   // methods inherited from GmatBase
   virtual bool         Validate();
   virtual bool         Initialize();
   
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
   
   virtual Integer      GetIntegerParameter(const Integer id) const;
   virtual Integer      SetIntegerParameter(const Integer id, const Integer value);
   virtual Integer      GetIntegerParameter(const wxString &label) const;
   virtual Integer      SetIntegerParameter(const wxString &label,
                                            const Integer value);
   
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
   virtual bool         GetBooleanParameter(const wxString &label) const;
   virtual bool         SetBooleanParameter(const Integer id,
                                            const bool value);
   virtual bool         SetBooleanParameter(const wxString &label,
                                            const bool value);
   
   virtual const BooleanArray&
                        GetBooleanArrayParameter(const Integer id) const;
   virtual const BooleanArray&
                        GetBooleanArrayParameter(const wxString &label) const;
   virtual bool         SetBooleanArrayParameter(const Integer id,
                                                 const BooleanArray &valueArray);
   virtual bool         SetBooleanArrayParameter(const wxString &label,
                                                 const BooleanArray &valueArray);
   
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
   
   // for GUI population
   virtual Gmat::ObjectType
                        GetPropertyObjectType(const Integer id) const;
   
protected:
   
   CoordinateSystem *mViewCoordSystem;
   
   wxString mOldName;
   wxString mViewCoordSysName;
   
   // object names and arrays
   std::vector<SpacePoint*> mObjectArray;
   std::vector<SpacePoint*> mAllSpArray;
   BooleanArray mDrawOrbitArray;
   BooleanArray mDrawObjectArray;
   
   StringArray mScNameArray;
   StringArray mObjectNameArray;
   StringArray mAllSpNameArray;
   StringArray mAllRefObjectNames;
   
   Integer mAllSpCount;
   Integer mScCount;
   Integer mObjectCount;
   Integer mNonStdBodyCount;
   /// It uses predefined colors up to 15 objects, after 15 it uses red
   Integer mDefaultColorCount;
   
   // for data control
   Integer mDataCollectFrequency;
   Integer mUpdatePlotFrequency;
   Integer mNumPointsToRedraw;
   Integer mNumData;
   Integer mNumCollected;
   bool    mDrawingStatusChanged;
   
   // arrays for holding distributed data
   RealArray mScXArray;
   RealArray mScYArray;
   RealArray mScZArray;
   RealArray mScVxArray;
   RealArray mScVyArray;
   RealArray mScVzArray;
   
   // arrays for holding object colors
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
   
   // maps for object and color
   std::map<wxString, UnsignedInt> mOrbitColorMap;
   std::map<wxString, UnsignedInt> mTargetColorMap;
   std::map<wxString, bool> mDrawOrbitMap;
   std::map<wxString, bool> mShowObjectMap;
   
   /// Calls PlotInterface for plotting non-solver data  
   virtual bool         UpdateData(const Real *dat, Integer len);
   
   /// Calls PlotInterface for plotting solver data
   virtual bool         UpdateSolverData();
   
   /// Buffers published spacecraft orbit data
   virtual Integer      BufferOrbitData(const Real *dat, Integer len);
   
   /// Adds Spacecraft and other objects to object arrays and color maps
   bool                 AddSpacePoint(const wxString &name, Integer index,
                                      bool show = true);
   /// Clears all object arrays and color maps, called from TakeAction(wxT("Clear"))
   bool                 ClearSpacePointList();
   /// Removes SpacePoint object from the object and color array, called from TakeAction(wxT("Remove"))
   bool                 RemoveSpacePoint(const wxString &name);
   /// Finds the index of the element label from the element label array.
   Integer              FindIndexOfElement(StringArray &labelArray,
                                           const wxString &label);
   /// Builds dynamic arrays to pass to plotting canvas
   void                 BuildDynamicArrays();
   /// Clears dynamic arrays such as object name array, color array, etc.
   void                 ClearDynamicArrays();
   /// Updates plotting object information such as plotting object pointer, option flags, etc.
   void                 UpdateObjectList(SpacePoint *sp, bool show = false);
   
   enum
   {
      ADD = SubscriberParamCount,
      COORD_SYSTEM,
      DRAW_OBJECT,
      ORBIT_COLOR,
      TARGET_COLOR,
      DATA_COLLECT_FREQUENCY,
      UPDATE_PLOT_FREQUENCY,
      NUM_POINTS_TO_REDRAW,
      SHOW_PLOT,
      OrbitPlotParamCount
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[OrbitPlotParamCount - SubscriberParamCount];
   static const wxString
      PARAMETER_TEXT[OrbitPlotParamCount - SubscriberParamCount];
   
   const static int MAX_SP_COLOR = 15;
   static const UnsignedInt DEFAULT_ORBIT_COLOR[MAX_SP_COLOR];
   
};

#endif
