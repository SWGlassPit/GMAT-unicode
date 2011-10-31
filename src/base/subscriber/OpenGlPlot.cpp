//$Id: OpenGlPlot.cpp 9513 2011-04-30 21:23:06Z djcinsb $
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
 * Implements OpenGlPlot class.
 */
//------------------------------------------------------------------------------

#include "OpenGlPlot.hpp"
#include "PlotInterface.hpp"       // for UpdateGlPlot()
#include "ColorTypes.hpp"          // for namespace GmatColor::
#include "SubscriberException.hpp" // for SubscriberException()
#include "MessageInterface.hpp"    // for ShowMessage()
#include "TextParser.hpp"          // for SeparateBrackets()
#include "StringUtil.hpp"          // for ToReal()
#include "CoordinateConverter.hpp" // for Convert()
#include <algorithm>               // for find(), distance()

#define __REMOVE_OBJ_BY_SETTING_FLAG__

//#define DBGLVL_OPENGL_INIT 1
//#define DBGLVL_OPENGL_DATA 1
//#define DBGLVL_OPENGL_DATA_LABELS 1
//#define DBGLVL_OPENGL_ADD 1
//#define DBGLVL_OPENGL_OBJ 2
//#define DBGLVL_OPENGL_PARAM 1
//#define DEBUG_OPENGL_PUT
//#define DBGLVL_OPENGL_PARAM_STRING 2
//#define DBGLVL_OPENGL_PARAM_RVEC3 1
//#define DBGLVL_OPENGL_UPDATE 2
//#define DBGLVL_TAKE_ACTION 1
//#define DBGLVL_REMOVE_SP 1
//#define DBGLVL_RENAME 1
//#define DBGLVL_SOLVER_CURRENT_ITER 2

//---------------------------------
// static data
//---------------------------------
const wxString
OpenGlPlot::PARAMETER_TEXT[OpenGlPlotParamCount - SubscriberParamCount] =
{
   wxT("Add"),
   wxT("OrbitColor"),
   wxT("TargetColor"),
   wxT("CoordinateSystem"),
   wxT("ViewPointRef"),
   wxT("ViewPointReference"),
   wxT("ViewPointRefType"),
   wxT("ViewPointRefVector"),
   wxT("ViewPointVector"),
   wxT("ViewPointVectorType"),
   wxT("ViewPointVectorVector"),
   wxT("ViewDirection"),
   wxT("ViewDirectionType"),
   wxT("ViewDirectionVector"),
   wxT("ViewScaleFactor"),
   wxT("FixedFovAngle"),
   wxT("ViewUpCoordinateSystem"),
   wxT("ViewUpAxis"),
   wxT("CelestialPlane"),
   wxT("XYPlane"),
   wxT("WireFrame"),
   wxT("Axes"),
   wxT("Grid"),
   wxT("EarthSunLines"),
   wxT("SunLine"),
   wxT("Overlap"),
   wxT("UseInitialView"),
   wxT("PerspectiveMode"),
   wxT("UseFixedFov"),
   wxT("DataCollectFrequency"),
   wxT("UpdatePlotFrequency"),
   wxT("NumPointsToRedraw"),
   wxT("ShowPlot"),
        wxT("StarCount"),
        wxT("EnableStars"),
        wxT("EnableConstellations"),
        wxT("MinFOV"),
        wxT("MaxFOV"),
        wxT("InitialFOV"),
}; 


const Gmat::ParameterType
OpenGlPlot::PARAMETER_TYPE[OpenGlPlotParamCount - SubscriberParamCount] =
{
   Gmat::OBJECTARRAY_TYPE,       //wxT("Add")
   Gmat::UNSIGNED_INTARRAY_TYPE, //wxT("OrbitColor"),
   Gmat::UNSIGNED_INTARRAY_TYPE, //wxT("TargetColor"),
   Gmat::OBJECT_TYPE,            //wxT("CoordinateSystem")
   Gmat::OBJECT_TYPE,            //wxT("ViewPointRef"),
   Gmat::OBJECT_TYPE,            //wxT("ViewPointReference"),
   Gmat::STRING_TYPE,            //wxT("ViewPointRefType")
   Gmat::RVECTOR_TYPE,           //wxT("ViewPointRefVector"),
   //Gmat::STRING_TYPE,            //wxT("ViewPointVector"),
   Gmat::OBJECT_TYPE,            //wxT("ViewPointVector"),
   Gmat::STRING_TYPE,            //wxT("ViewPointVectorType"),
   Gmat::RVECTOR_TYPE,           //wxT("ViewPointVectorVector"),
   Gmat::OBJECT_TYPE,            //wxT("ViewDirection"),
   Gmat::STRING_TYPE,            //wxT("ViewDirectionType"),
   Gmat::RVECTOR_TYPE,           //wxT("ViewDirectionVector"),
   Gmat::REAL_TYPE,              //wxT("ViewScaleFactor"),
   Gmat::REAL_TYPE,              //wxT("FixedFovAngle"),
   Gmat::OBJECT_TYPE,            //wxT("ViewUpCoordinaetSystem")
   Gmat::ENUMERATION_TYPE,       //wxT("ViewUpAxis")
   
   Gmat::ON_OFF_TYPE,            //wxT("CelestialPlane")
   Gmat::ON_OFF_TYPE,            //wxT("XYPlane")
   Gmat::ON_OFF_TYPE,            //wxT("WireFrame")
   Gmat::ON_OFF_TYPE,            //wxT("Axes")
   Gmat::ON_OFF_TYPE,            //wxT("Grid")
   Gmat::ON_OFF_TYPE,            //wxT("EarthSunLines")   
   Gmat::ON_OFF_TYPE,            //wxT("SunLine")   
   Gmat::ON_OFF_TYPE,            //wxT("Overlap")
   Gmat::ON_OFF_TYPE,            //wxT("LockView")
   Gmat::ON_OFF_TYPE,            //wxT("PerspectiveMode")
   Gmat::ON_OFF_TYPE,            //wxT("UseFixedFov")
   
   Gmat::INTEGER_TYPE,           //wxT("DataCollectFrequency")
   Gmat::INTEGER_TYPE,           //wxT("UpdatePlotFrequency")
   Gmat::INTEGER_TYPE,           //wxT("NumPointsToRedraw")
   
   Gmat::BOOLEAN_TYPE,           //wxT("ShowPlot")

        Gmat::INTEGER_TYPE,                             //wxT("StarCount")
        Gmat::ON_OFF_TYPE,                              //wxT("EnableStars")
        Gmat::ON_OFF_TYPE,                              //wxT("EnableConstellations")
        Gmat::INTEGER_TYPE,                             //wxT("MinFOV")
        Gmat::INTEGER_TYPE,                             //wxT("MaxFOV")
        Gmat::INTEGER_TYPE,                             //wxT("InitialFOV")
};


const UnsignedInt
OpenGlPlot::DEFAULT_ORBIT_COLOR[MAX_SP_COLOR] =
{
   GmatColor::RED32,       GmatColor::LIME32,    GmatColor::YELLOW32,
   GmatColor::AQUA32,      GmatColor::PINK32,    GmatColor::L_BLUE32,
   GmatColor::L_GRAY32,    GmatColor::BLUE32,    GmatColor::FUCHSIA32,
   GmatColor::BEIGE32,     GmatColor::RED32,     GmatColor::LIME32,
   GmatColor::YELLOW32,    GmatColor::AQUA32,    GmatColor::PINK32
};


//------------------------------------------------------------------------------
// OpenGlPlot(const wxString &name)
//------------------------------------------------------------------------------
/**
 * The default constructor
 */
//------------------------------------------------------------------------------
OpenGlPlot::OpenGlPlot(const wxString &name)
   : Subscriber(wxT("OpenGLPlot"), name)
{
   // GmatBase data
   parameterCount = OpenGlPlotParamCount;
   objectTypes.push_back(Gmat::ORBIT_VIEW);
   objectTypeNames.push_back(wxT("OpenGLPlot"));
   
   mEclipticPlane = wxT("Off");
   mXYPlane = wxT("On");
   mWireFrame = wxT("Off");
   mAxes = wxT("On");
   mGrid = wxT("Off");
   mSunLine = wxT("Off");
   mOverlapPlot = wxT("Off");
   mUseInitialView = wxT("On");
   mPerspectiveMode = wxT("Off");
   mUseFixedFov = wxT("Off");

        // stars
        mEnableStars = wxT("On");
        mEnableConstellations = wxT("On");
        mStarCount = 46000;

        // FOV
        mMinFOV = 0;
        mMaxFOV = 90;
        mInitialFOV = 45;
   
   mOldName = instanceName;
   mViewCoordSysName = wxT("EarthMJ2000Eq");
   mViewUpCoordSysName = wxT("EarthMJ2000Eq");
   mViewUpAxisName = wxT("Z");
   
   // viewpoint
   mViewPointRefName = wxT("Earth");
   mViewPointRefType = wxT("Object");
   mViewPointVecName = wxT("[ 0 0 30000 ]");
   mViewPointVecType = wxT("Vector");
   mViewDirectionName = wxT("Earth");
   mViewDirectionType= wxT("Object");
   mViewScaleFactor = 1.0;
   mFixedFovAngle = 45.0;
   mViewPointRefVector.Set(0.0, 0.0, 0.0);
   mViewPointVecVector.Set(0.0, 0.0, 30000.0);
   mViewDirectionVector.Set(0.0, 0.0, -1.0);
   
   mViewCoordSystem = NULL;
   mViewUpCoordSystem = NULL;
   mViewCoordSysOrigin = NULL;
   mViewUpCoordSysOrigin = NULL;
   mViewPointRefObj = NULL;
   mViewPointObj = NULL;
   mViewDirectionObj = NULL;
   
   mDataCollectFrequency = 1;
   mUpdatePlotFrequency = 50;
   mNumPointsToRedraw = 0;
   mNumData = 0;
   mNumCollected = 0;
   mScNameArray.clear();
   mObjectNameArray.clear();
   mAllSpNameArray.clear();
   mAllRefObjectNames.clear();
   mObjectArray.clear();
   mDrawOrbitArray.clear();
   mShowObjectArray.clear();
   mAllSpArray.clear();
   
   mScXArray.clear();
   mScYArray.clear();
   mScZArray.clear();
   mScVxArray.clear();
   mScVyArray.clear();
   mScVzArray.clear();
   mScOrbitColorArray.clear();
   mScTargetColorArray.clear();
   mOrbitColorArray.clear();
   mTargetColorArray.clear();
   
   mOrbitColorMap.clear();
   mTargetColorMap.clear();
   mDrawOrbitMap.clear();
   mShowObjectMap.clear();
   
   mAllSpCount = 0;
   mScCount = 0;
   mObjectCount = 0;
   mNonStdBodyCount = 0;
   
   // default planet color
   mOrbitColorMap[wxT("Earth")] = GmatColor::GREEN32;
   mOrbitColorMap[wxT("Luna")] = GmatColor::SILVER32;
   mOrbitColorMap[wxT("Sun")] = GmatColor::ORANGE32;
   mOrbitColorMap[wxT("Mercury")] = GmatColor::GRAY32;
   mOrbitColorMap[wxT("Venus")] = GmatColor::BEIGE32;
   mOrbitColorMap[wxT("Mars")] = GmatColor::L_GRAY32;
   mOrbitColorMap[wxT("Jupiter")] = GmatColor::L_BROWN32;
   mOrbitColorMap[wxT("Saturn")] = GmatColor::D_BROWN32;
   mOrbitColorMap[wxT("Uranus")] = GmatColor::BLUE32;
   mOrbitColorMap[wxT("Neptune")] = GmatColor::NAVY32;
   mOrbitColorMap[wxT("Pluto")] = GmatColor::PURPLE32;
   
}


//------------------------------------------------------------------------------
// OpenGlPlot(const OpenGlPlot &ogl)
//------------------------------------------------------------------------------
/**
 * The copy consturctor
 */
//------------------------------------------------------------------------------
OpenGlPlot::OpenGlPlot(const OpenGlPlot &ogl)
   : Subscriber(ogl)
{
   mEclipticPlane = ogl.mEclipticPlane;
   mXYPlane = ogl.mXYPlane;
   mWireFrame = ogl.mWireFrame;
   mAxes = ogl.mAxes;
   mGrid = ogl.mGrid;
   mSunLine = ogl.mSunLine;
   mOverlapPlot = ogl.mOverlapPlot;
   mUseInitialView = ogl.mUseInitialView;
   mPerspectiveMode = ogl.mPerspectiveMode;
   mUseFixedFov = ogl.mUseFixedFov;
   
   mOldName = ogl.mOldName;;
   mViewCoordSysName = ogl.mViewCoordSysName;
   
   // viewpoint
   mViewPointRefName = ogl.mViewPointRefName;
   mViewPointRefType = ogl.mViewPointRefType;
   mViewPointVecName = ogl.mViewPointVecName;
   mViewPointVecType = ogl.mViewPointVecType;
   mViewDirectionName = ogl.mViewDirectionName;
   mViewDirectionType = ogl.mViewDirectionType;
   mViewScaleFactor = ogl.mViewScaleFactor;
   mFixedFovAngle = ogl.mFixedFovAngle;
   mViewPointRefVector = ogl.mViewPointRefVector;
   mViewPointVecVector = ogl.mViewPointVecVector;
   mViewDirectionVector = ogl.mViewDirectionVector;
   mViewUpCoordSysName = ogl.mViewUpCoordSysName;
   mViewUpAxisName = ogl.mViewUpAxisName;
   
   mViewCoordSystem = ogl.mViewCoordSystem;
   mViewUpCoordSystem = ogl.mViewCoordSystem;
   mViewCoordSysOrigin = ogl.mViewCoordSysOrigin;
   mViewUpCoordSysOrigin = ogl.mViewUpCoordSysOrigin;
   mViewPointRefObj = ogl.mViewPointRefObj;
   mViewPointObj = ogl.mViewPointObj;
   mViewDirectionObj = ogl.mViewDirectionObj;

        // stars
        mStarCount = ogl.mStarCount;
        mEnableStars = ogl.mEnableStars;
        mEnableConstellations = ogl.mEnableConstellations;

        // FOV
        mMinFOV = ogl.mMinFOV;
        mMaxFOV = ogl.mMaxFOV;
        mInitialFOV = ogl.mInitialFOV;
   
   mDataCollectFrequency = ogl.mDataCollectFrequency;
   mUpdatePlotFrequency = ogl.mUpdatePlotFrequency;
   mNumPointsToRedraw = ogl.mNumPointsToRedraw;
   
   mAllSpCount = ogl.mAllSpCount;
   mScCount = ogl.mScCount;
   mObjectCount = ogl.mObjectCount;
   mNonStdBodyCount = ogl.mNonStdBodyCount;
   
   mObjectArray = ogl.mObjectArray;
   mDrawOrbitArray = ogl.mDrawOrbitArray;
   mShowObjectArray = ogl.mShowObjectArray;
   mAllSpArray = ogl.mAllSpArray;
   mScNameArray = ogl.mScNameArray;
   mObjectNameArray = ogl.mObjectNameArray;
   mAllSpNameArray = ogl.mAllSpNameArray;
   mAllRefObjectNames = ogl.mAllRefObjectNames;
   mScXArray = ogl.mScXArray;
   mScYArray = ogl.mScYArray;
   mScZArray = ogl.mScZArray;
   mScVxArray = ogl.mScVxArray;
   mScVyArray = ogl.mScVyArray;
   mScVzArray = ogl.mScVzArray;
   mScOrbitColorArray = ogl.mScOrbitColorArray;
   mScTargetColorArray = ogl.mScTargetColorArray;
   mOrbitColorArray = ogl.mOrbitColorArray;
   mTargetColorArray = ogl.mTargetColorArray;
   
   mOrbitColorMap = ogl.mOrbitColorMap;
   mTargetColorMap = ogl.mTargetColorMap;
   mDrawOrbitMap = ogl.mDrawOrbitMap;
   mShowObjectMap = ogl.mShowObjectMap;
   
   mNumData = ogl.mNumData;
   mNumCollected = ogl.mNumCollected;
}


//------------------------------------------------------------------------------
// OpenGlPlot& operator=(const OpenGlPlot&)
//------------------------------------------------------------------------------
/**
 * The assignment operator
 */
//------------------------------------------------------------------------------
OpenGlPlot& OpenGlPlot::operator=(const OpenGlPlot& ogl)
{
   if (this == &ogl)
      return *this;
   
   Subscriber::operator=(ogl);
   
   mEclipticPlane = ogl.mEclipticPlane;
   mXYPlane = ogl.mXYPlane;
   mWireFrame = ogl.mWireFrame;
   mAxes = ogl.mAxes;
   mGrid = ogl.mGrid;
   mSunLine = ogl.mSunLine;
   mOverlapPlot = ogl.mOverlapPlot;
   mUseInitialView = ogl.mUseInitialView;
   mPerspectiveMode = ogl.mPerspectiveMode;
   mUseFixedFov = ogl.mUseFixedFov;
   
   mOldName = ogl.mOldName;;
   mViewCoordSysName = ogl.mViewCoordSysName;
   
   // viewpoint
   mViewPointRefName = ogl.mViewPointRefName;
   mViewPointRefType = ogl.mViewPointRefType;
   mViewPointVecName = ogl.mViewPointVecName;
   mViewPointVecType = ogl.mViewPointVecType;
   mViewDirectionName = ogl.mViewDirectionName;
   mViewDirectionType = ogl.mViewDirectionType;
   mViewScaleFactor = ogl.mViewScaleFactor;
   mFixedFovAngle = ogl.mFixedFovAngle;
   mViewPointRefVector = ogl.mViewPointRefVector;
   mViewPointVecVector = ogl.mViewPointVecVector;
   mViewDirectionVector = ogl.mViewDirectionVector;
   mViewUpCoordSysName = ogl.mViewUpCoordSysName;
   mViewUpAxisName = ogl.mViewUpAxisName;
   
   mViewCoordSystem = ogl.mViewCoordSystem;
   mViewUpCoordSystem = ogl.mViewCoordSystem;
   mViewCoordSysOrigin = ogl.mViewCoordSysOrigin;
   mViewUpCoordSysOrigin = ogl.mViewUpCoordSysOrigin;
   mViewPointRefObj = ogl.mViewPointRefObj;
   mViewPointObj = ogl.mViewPointObj;
   mViewDirectionObj = ogl.mViewDirectionObj;
   
   mDataCollectFrequency = ogl.mDataCollectFrequency;
   mUpdatePlotFrequency = ogl.mUpdatePlotFrequency;
   mNumPointsToRedraw = ogl.mNumPointsToRedraw;
   
   mAllSpCount = ogl.mAllSpCount;
   mScCount = ogl.mScCount;
   mObjectCount = ogl.mObjectCount;
   mNonStdBodyCount = ogl.mNonStdBodyCount;
   
   mObjectArray = ogl.mObjectArray;
   mDrawOrbitArray = ogl.mDrawOrbitArray;
   mShowObjectArray = ogl.mShowObjectArray;
   mAllSpArray = ogl.mAllSpArray;
   mScNameArray = ogl.mScNameArray;
   mObjectNameArray = ogl.mObjectNameArray;
   mAllSpNameArray = ogl.mAllSpNameArray;
   mAllRefObjectNames = ogl.mAllRefObjectNames;
   mScXArray = ogl.mScXArray;
   mScYArray = ogl.mScYArray;
   mScZArray = ogl.mScZArray;
   mScVxArray = ogl.mScVxArray;
   mScVyArray = ogl.mScVyArray;
   mScVzArray = ogl.mScVzArray;
   mScOrbitColorArray = ogl.mScOrbitColorArray;
   mScTargetColorArray = ogl.mScTargetColorArray;
   mOrbitColorArray = ogl.mOrbitColorArray;
   mTargetColorArray = ogl.mTargetColorArray;
   
   mOrbitColorMap = ogl.mOrbitColorMap;
   mTargetColorMap = ogl.mTargetColorMap;
   mDrawOrbitMap = ogl.mDrawOrbitMap;
   mShowObjectMap = ogl.mShowObjectMap;
   
   mNumData = ogl.mNumData;
   mNumCollected = ogl.mNumCollected;
   
   return *this;
}


//------------------------------------------------------------------------------
// ~OpenGlPlot()
//------------------------------------------------------------------------------
/**
 * Destructor
 *
 * @note This destructor does not delete OpenGL plot window, but clears data.
 *       OpenGL plot window is deleted when it is closed by the user or GMAT
 *       shuts down.
 */
//------------------------------------------------------------------------------
OpenGlPlot::~OpenGlPlot()
{
   PlotInterface::TakeGlAction(instanceName, wxT("ClearObjects"));
}


//------------------------------------------------------------------------------
// const StringArray& GetSpacePointList()
//------------------------------------------------------------------------------
const StringArray& OpenGlPlot::GetSpacePointList()
{
   return mAllSpNameArray;
}


//------------------------------------------------------------------------------
// const StringArray& GetSpacecraftList()
//------------------------------------------------------------------------------
const StringArray& OpenGlPlot::GetSpacecraftList()
{
   return mScNameArray;
}


//------------------------------------------------------------------------------
// const StringArray& GetNonSpacecraftList()
//------------------------------------------------------------------------------
const StringArray& OpenGlPlot::GetNonSpacecraftList()
{
   return mObjectNameArray;
}


//------------------------------------------------------------------------------
// UnsignedInt GetColor(const wxString &item, const wxString &name)
//------------------------------------------------------------------------------
UnsignedInt OpenGlPlot::GetColor(const wxString &item,
                                 const wxString &name)
{
   #if DBGLVL_OPENGL_PARAM
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::GetColor() item=%s, name=%s\n"),
       item.c_str(), name.c_str());
   #endif
   
   if (item == wxT("Orbit"))
   {
      if (mOrbitColorMap.find(name) != mOrbitColorMap.end())
         return mOrbitColorMap[name];
   }
   else if (item == wxT("Target"))
   {
      if (mTargetColorMap.find(name) != mTargetColorMap.end())
         return mTargetColorMap[name];
   }
   
   return GmatBase::UNSIGNED_INT_PARAMETER_UNDEFINED;
}


//------------------------------------------------------------------------------
// bool SetColor(const wxString &item, const wxString &name,
//               UnsignedInt value)
//------------------------------------------------------------------------------
bool OpenGlPlot::SetColor(const wxString &item, const wxString &name,
                          UnsignedInt value)
{
   #if DBGLVL_OPENGL_PARAM
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::SetColor() item=%s, name=%s, value=%u\n"),
       item.c_str(), name.c_str(), value);
   #endif
   
   if (item == wxT("Orbit"))
   {
      if (mOrbitColorMap.find(name) != mOrbitColorMap.end())
      {
         mOrbitColorMap[name] = value;
         
         for (int i=0; i<mAllSpCount; i++)
            if (mAllSpNameArray[i] == name)
               mOrbitColorArray[i] = value;
         
         return true;
      }
   }
   else if (item == wxT("Target"))
   {
      if (mTargetColorMap.find(name) != mTargetColorMap.end())
      {
         mTargetColorMap[name] = value;
         
         for (int i=0; i<mAllSpCount; i++)
            if (mAllSpNameArray[i] == name)
               mTargetColorArray[i] = value;
         
         return true;
      }
   }
   
   return false;
}


//------------------------------------------------------------------------------
// bool GetShowObject(const wxString &name)
//------------------------------------------------------------------------------
bool OpenGlPlot::GetShowObject(const wxString &name)
{
   #if DBGLVL_OPENGL_PARAM
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::GetShowObject() name=%s returning %d\n"),
       name.c_str(), mDrawOrbitMap[name]);
   #endif
   
   return mShowObjectMap[name];
}


//------------------------------------------------------------------------------
// void SetShowObject(const wxString &name, bool value)
//------------------------------------------------------------------------------
void OpenGlPlot::SetShowObject(const wxString &name, bool value)
{
   #if DBGLVL_OPENGL_PARAM
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::SetShowObject() name=%s setting %d\n"), name.c_str(), value);
   #endif
   
   mShowObjectMap[name] = value;
   if (value)
      mDrawOrbitMap[name] = value;
}


//------------------------------------------------------------------------------
// Rvector3 GetVector(const wxString &which)
//------------------------------------------------------------------------------
Rvector3 OpenGlPlot::GetVector(const wxString &which)
{
   if (which == wxT("ViewPointReference"))
      return mViewPointRefVector;
   else if (which == wxT("ViewPointVector"))
      return mViewPointVecVector;
   else if (which == wxT("ViewDirection"))
      return mViewDirectionVector;
   else
      throw SubscriberException(which + wxT(" is unknown OpenGlPlot parameter\n"));
}


//------------------------------------------------------------------------------
// void SetVector(const wxString &which, const Rvector3 &value)
//------------------------------------------------------------------------------
void OpenGlPlot::SetVector(const wxString &which, const Rvector3 &value)
{
   #if DBGLVL_OPENGL_SET
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::SetVector() which=%s, value=%s\n"), which.c_str(),
       value.ToString().c_str());
   #endif
   
   if (which == wxT("ViewPointReference"))
      mViewPointRefVector = value;
   else if (which == wxT("ViewPointVector"))
      mViewPointVecVector = value;
   else if (which == wxT("ViewDirection"))
      mViewDirectionVector = value;
   else
      throw SubscriberException(which + wxT(" is unknown OpenGlPlot parameter\n"));
}


//----------------------------------
// inherited methods from Subscriber
//----------------------------------

//------------------------------------------------------------------------------
// virtual bool Initialize()
//------------------------------------------------------------------------------
bool OpenGlPlot::Initialize()
{
   if (GmatGlobal::Instance()->GetRunMode() == GmatGlobal::TESTING_NO_PLOTS)
      return true;
   
   Subscriber::Initialize();
   
   // theInternalCoordSys is used only by OpenGL plot so check. (2008.06.16)
   if (theInternalCoordSystem == NULL)
   {
      active = false;
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("*** WARNING *** The OpenGL plot named \"%s\" will be turned off. ")
          wxT("It has a NULL internal coordinate system pointer.\n"), GetName().c_str());
      return false;
   }
   
   #if DBGLVL_OPENGL_INIT
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::Initialize() this=<%p>'%s', active=%d, isInitialized=%d, ")
       wxT("isEndOfReceive=%d, mAllSpCount=%d\n"), this, GetName().c_str(), active,
       isInitialized, isEndOfReceive, mAllSpCount);
   #endif
   
   bool foundSc = false;
   bool retval = false;
   Integer nullCounter = 0;
   
   if (mAllSpCount == 0)
   {
      active = false;
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("*** WARNING *** The OpenGL plot named \"%s\" will be turned off. ")
          wxT("No SpacePoints were added to plot.\n"), GetName().c_str());
      return false;
   }
   
   // check for spacecaft is included in the plot
   for (int i=0; i<mAllSpCount; i++)
   {
      #if DBGLVL_OPENGL_INIT > 1
      MessageInterface::ShowMessage
         (wxT("OpenGlPlot::Initialize() mAllSpNameArray[%d]=%s, addr=%d\n"),
          i, mAllSpNameArray[i].c_str(), mAllSpArray[i]);
      #endif
      
      if (mAllSpArray[i])
      {                  
         if (mAllSpArray[i]->IsOfType(Gmat::SPACECRAFT))
         {
            foundSc = true;
            break;
         }
      }
      else
         nullCounter++;
   }
   
   if (nullCounter == mAllSpCount)
   {
      active = false;
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("*** WARNING *** The OpenGL plot named \"%s\" will be turned off. ")
          wxT("%d SpaceObjects have NULL pointers.\n"), GetName().c_str(), nullCounter);
      return false;
   }
   
   if (!foundSc)
   {
      active = false;
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("*** WARNING *** The OpenGL plot named \"%s\" will be turned off. ")
          wxT("No Spacecraft was added to plot.\n"), GetName().c_str());
      return false;
   }
   
   
   //--------------------------------------------------------
   // start initializing for OpenGL plot
   //--------------------------------------------------------
   if (active && !isInitialized)
   {
      #if DBGLVL_OPENGL_INIT
      MessageInterface::ShowMessage
         (wxT("OpenGlPlot::Initialize() CreateGlPlotWindow() theSolarSystem=%p\n"),
          theSolarSystem);
      #endif
      
      if (PlotInterface::CreateGlPlotWindow
          (instanceName, mOldName, (mEclipticPlane == wxT("On")), (mXYPlane == wxT("On")),
           (mWireFrame == wxT("On")), (mAxes == wxT("On")), (mGrid == wxT("On")),
           (mSunLine == wxT("On")), (mOverlapPlot == wxT("On")), (mUseInitialView == wxT("On")),
           (mPerspectiveMode == wxT("On")), mNumPointsToRedraw, 
			  (mEnableStars == wxT("On")), (mEnableConstellations == wxT("On")), mStarCount))
      {
         #if DBGLVL_OPENGL_INIT
         MessageInterface::ShowMessage
            (wxT("   mViewPointRefObj=%p, mViewScaleFactor=%f\n"),
             mViewPointRefObj, mViewScaleFactor);
         #endif
         
         //--------------------------------------------------------
         // Set Spacecraft and non-Spacecraft objects.
         // If non-Spacecraft, position has to be computed in the
         // TrajPlotCanvas, so need to pass those object pointers.
         //--------------------------------------------------------
         
         ClearDynamicArrays();
         
         // add non-spacecraft plot objects to the list
         for (int i=0; i<mAllSpCount; i++)
         {
            #if DBGLVL_OPENGL_INIT > 1
            MessageInterface::ShowMessage
               (wxT("OpenGlPlot::Initialize() mAllSpNameArray[%d]=%s, addr=%d\n"),
                i, mAllSpNameArray[i].c_str(), mAllSpArray[i]);
            #endif
            
            if (mAllSpArray[i])
            {
               //add all objects to object list
               mObjectNameArray.push_back(mAllSpNameArray[i]);                  
               mDrawOrbitArray.push_back(mDrawOrbitMap[mAllSpNameArray[i]]);
               mShowObjectArray.push_back(mShowObjectMap[mAllSpNameArray[i]]);
               mOrbitColorArray.push_back(mOrbitColorMap[mAllSpNameArray[i]]);
               mTargetColorArray.push_back(mTargetColorMap[mAllSpNameArray[i]]);
               mObjectArray.push_back(mAllSpArray[i]);
               
               if (mAllSpArray[i]->IsOfType(Gmat::SPACECRAFT))
               {
                  mScNameArray.push_back(mAllSpNameArray[i]);
                  mScOrbitColorArray.push_back(mOrbitColorMap[mAllSpNameArray[i]]);
                  mScTargetColorArray.push_back(mTargetColorMap[mAllSpNameArray[i]]);
                  mScXArray.push_back(0.0);
                  mScYArray.push_back(0.0);
                  mScZArray.push_back(0.0);
                  mScVxArray.push_back(0.0);
                  mScVyArray.push_back(0.0);
                  mScVzArray.push_back(0.0);
               }
            }
            else
            {
               MessageInterface::PopupMessage
                  (Gmat::WARNING_, wxT("The SpacePoint name: %s has NULL pointer.\n")
                   wxT("It will be removed from the OpenGL plot.\n"),
                   mAllSpNameArray[i].c_str());
            }
         }
         
         mScCount = mScNameArray.size();
         mObjectCount = mObjectNameArray.size();
         
         // check ViewPoint info to see if any objects need to be
         // included in the non-spacecraft list
         if (mViewCoordSystem == NULL)
            throw SubscriberException
               (wxT("OpenGlPlot::Initialize() CoordinateSystem: ") + mViewCoordSysName +
                wxT(" not set\n"));
         
         if (mViewUpCoordSystem == NULL)
            throw SubscriberException
               (wxT("OpenGlPlot::Initialize() CoordinateSystem: ") + mViewUpCoordSysName +
                wxT(" not set\n"));               
         
         // Get View CoordinateSystem Origin pointer
         mViewCoordSysOrigin = mViewCoordSystem->GetOrigin();
         
         if (mViewCoordSysOrigin != NULL)
            UpdateObjectList(mViewCoordSysOrigin);
         
         // Get View Up CoordinateSystem Origin pointer
         mViewUpCoordSysOrigin = mViewUpCoordSystem->GetOrigin();
         
         if (mViewUpCoordSysOrigin != NULL)
            UpdateObjectList(mViewUpCoordSysOrigin);
         
         // Get ViewPointRef object pointer from the current SolarSystem
         if (mViewPointRefObj != NULL)
            UpdateObjectList(mViewPointRefObj);
         
         // Get ViewPoint object pointer from the current SolarSystem
         if (mViewPointObj != NULL)
            UpdateObjectList(mViewPointObj);
         
         // Get ViewDirection object pointer from the current SolarSystem
         if (mViewDirectionObj != NULL)
            UpdateObjectList(mViewDirectionObj);
         
         #if DBGLVL_OPENGL_INIT > 1
         MessageInterface::ShowMessage
            (wxT("   mScNameArray.size=%d, mScOrbitColorArray.size=%d\n"),
             mScNameArray.size(), mScOrbitColorArray.size());
         MessageInterface::ShowMessage
            (wxT("   mObjectNameArray.size=%d, mOrbitColorArray.size=%d\n"),
             mObjectNameArray.size(), mOrbitColorArray.size());
         
         bool draw, show;
         for (int i=0; i<mObjectCount; i++)
         {
            draw = mDrawOrbitArray[i] ? true : false;
            show = mShowObjectArray[i] ? true : false;
            MessageInterface::ShowMessage
               (wxT("   mObjectNameArray[%d]=%s, draw=%d, show=%d, color=%d\n"),
                i, mObjectNameArray[i].c_str(), draw, show, mOrbitColorArray[i]);
         }
         #endif
         
         #if DBGLVL_OPENGL_INIT
         MessageInterface::ShowMessage
            (wxT("   calling PlotInterface::SetGlSolarSystem(%p)\n"), theSolarSystem);
         #endif
         
         // set SolarSystem
         PlotInterface::SetGlSolarSystem(instanceName, theSolarSystem);
         
         #if DBGLVL_OPENGL_INIT
         MessageInterface::ShowMessage
            (wxT("   calling PlotInterface::SetGlObject()\n"));
         for (UnsignedInt i=0; i<mObjectArray.size(); i++)
            MessageInterface::ShowMessage
               (wxT("      mObjectArray[%d]=<%p>'%s'\n"), i, mObjectArray[i],
                mObjectArray[i]->GetName().c_str());
         #endif
         
         // set all object array and pointers
         PlotInterface::SetGlObject(instanceName, mObjectNameArray,
                                    mOrbitColorArray, mObjectArray);
         
         //--------------------------------------------------------
         // set CoordinateSystem
         //--------------------------------------------------------
         #if DBGLVL_OPENGL_INIT
         MessageInterface::ShowMessage
            (wxT("   theInternalCoordSystem = <%p>, origin = <%p>'%s'\n")
             wxT("   theDataCoordSystem     = <%p>, origin = <%p>'%s'\n")
             wxT("   mViewCoordSystem       = <%p>, origin = <%p>'%s'\n")
             wxT("   mViewUpCoordSystem     = <%p>, origin = <%p>'%s'\n")
             wxT("   mViewPointRefObj       = <%p>'%s'\n")
             wxT("   mViewPointObj          = <%p>'%s'\n")
             wxT("   mViewDirectionObj      = <%p>'%s'\n"),
             theInternalCoordSystem, theInternalCoordSystem->GetOrigin(),
             theInternalCoordSystem->GetOriginName().c_str(),
             theDataCoordSystem, theDataCoordSystem->GetOrigin(),
             theDataCoordSystem->GetOriginName().c_str(),
             mViewCoordSystem, mViewCoordSystem->GetOrigin(),
             mViewCoordSystem->GetOriginName().c_str(),
             mViewUpCoordSystem, mViewUpCoordSystem->GetOrigin(),
             mViewUpCoordSystem->GetOriginName().c_str(),
             mViewPointRefObj,
             mViewPointRefObj ? mViewPointRefObj->GetName().c_str() : wxT("NULL"),
             mViewPointObj,
             mViewPointObj ? mViewPointObj->GetName().c_str() : wxT("NULL"),
             mViewDirectionObj,
             mViewDirectionObj ? mViewDirectionObj->GetName().c_str() : wxT("NULL"));
         
         MessageInterface::ShowMessage
            (wxT("   calling PlotInterface::SetGlCoordSystem()\n"));
         #endif
         
         PlotInterface::SetGlCoordSystem(instanceName, theInternalCoordSystem,
                                         mViewCoordSystem, mViewUpCoordSystem);
         
         //--------------------------------------------------------
         // set viewpoint info
         //--------------------------------------------------------
         #if DBGLVL_OPENGL_INIT
         MessageInterface::ShowMessage
            (wxT("   calling PlotInterface::SetGlViewOption()\n"));
         #endif
         
         PlotInterface::SetGlViewOption
            (instanceName, mViewPointRefObj, mViewPointObj,
             mViewDirectionObj, mViewScaleFactor, mViewPointRefVector,
             mViewPointVecVector, mViewDirectionVector, mViewUpAxisName,
             (mViewPointRefType == wxT("Vector")), (mViewPointVecType == wxT("Vector")),
             (mViewDirectionType == wxT("Vector")), (mUseFixedFov == wxT("On")),
             mFixedFovAngle);
         
         PlotInterface::SetGlUpdateFrequency(instanceName, mUpdatePlotFrequency);
         
         //--------------------------------------------------------
         // set drawing object flag
         //--------------------------------------------------------
         PlotInterface::SetGlDrawOrbitFlag(instanceName, mDrawOrbitArray);
         PlotInterface::SetGlShowObjectFlag(instanceName, mShowObjectArray);
         
         isInitialized = true;
         retval = true;
      }
      else
      {
         retval = false;
      }
   }
   else
   {
      #if DBGLVL_OPENGL_INIT
      MessageInterface::ShowMessage
         (wxT("OpenGlPlot::Initialize() Plot is active and initialized, ")
          wxT("so calling DeleteGlPlot()\n"));
      #endif
      
      // Why do we want to delete plot if active and initialized?
      // This causes Global OpenGL plot not to show, so commented out (loj: 2008.10.08)
      //retval =  PlotInterface::DeleteGlPlot(instanceName);
   }
   
   #if DBGLVL_OPENGL_INIT
   MessageInterface::ShowMessage(wxT("OpenGlPlot::Initialize() exiting\n"));
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// void Activate(bool state)
//------------------------------------------------------------------------------
void OpenGlPlot::Activate(bool state)
{
   #ifdef DEBUG_OPENGL_ACTIVATE
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::Activate() this=<%p>'%s' entered, state=%d, isInitialized=%d\n"),
       this, GetName().c_str(), state, isInitialized);
   #endif
   
   Subscriber::Activate(state);
}


//---------------------------------
// inherited methods from GmatBase
//---------------------------------

//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the OpenGlPlot.
 *
 * @return clone of the OpenGlPlot.
 *
 */
//------------------------------------------------------------------------------
GmatBase* OpenGlPlot::Clone() const
{
   return (new OpenGlPlot(*this));
}


//---------------------------------------------------------------------------
// void Copy(const GmatBase* orig)
//---------------------------------------------------------------------------
/**
 * Sets this object to match another one.
 * 
 * @param orig The original that is being copied.
 */
//---------------------------------------------------------------------------
void OpenGlPlot::Copy(const GmatBase* orig)
{
   operator=(*((OpenGlPlot *)(orig)));
}


//------------------------------------------------------------------------------
// bool SetName(const wxString &who, const std;:string &oldName = wxT(""))
//------------------------------------------------------------------------------
/**
 * Set the name for this instance.
 *
 * @see GmatBase
 *
 */
//------------------------------------------------------------------------------
bool OpenGlPlot::SetName(const wxString &who, const wxString &oldName)
{
   #if DBGLVL_RENAME
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::SetName() newName=%s, oldName=%s\n"), who.c_str(),
       oldName.c_str());
   #endif
   
   if (oldName == wxT(""))
      mOldName = instanceName;
   else
      mOldName = oldName;
   
   return GmatBase::SetName(who);
}


//------------------------------------------------------------------------------
// virtual bool TakeAction(const wxString &action,  
//                         const wxString &actionData = wxT(""));
//------------------------------------------------------------------------------
/**
 * This method performs action.
 *
 * @param <action> action to perform
 * @param <actionData> action data associated with action
 * @return true if action successfully performed
 *
 */
//------------------------------------------------------------------------------
bool OpenGlPlot::TakeAction(const wxString &action,
                            const wxString &actionData)
{
   #if DBGLVL_TAKE_ACTION
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::TakeAction() '%s' entered, action='%s', actionData='%s'\n"),
       GetName().c_str(), action.c_str(), actionData.c_str());
   #endif
   if (action == wxT("Clear"))
   {
      return ClearSpacePointList();
   }
   else if (action == wxT("Remove"))
   {
      return RemoveSpacePoint(actionData);
   }
   else if (action == wxT("Finalize"))
   {
      PlotInterface::DeleteGlPlot(instanceName);
   }
   
   return false;
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
bool OpenGlPlot::RenameRefObject(const Gmat::ObjectType type,
                                 const wxString &oldName,
                                 const wxString &newName)
{
   #if DBGLVL_RENAME
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::RenameRefObject() type=%s, oldName=%s, newName=%s\n"),
       GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif
   
   if (type != Gmat::SPACECRAFT && type != Gmat::COORDINATE_SYSTEM)
      return true;
   
   if (type == Gmat::SPACECRAFT)
   {
      // for spacecraft name
      for (int i=0; i<mAllSpCount; i++)
         if (mAllSpNameArray[i] == oldName)
            mAllSpNameArray[i] = newName;
      
      //----------------------------------------------------
      // Since spacecraft name is used as key for spacecraft
      // color map, I can't change the key name, so it is
      // removed and inserted with new name
      //----------------------------------------------------
      std::map<wxString, UnsignedInt>::iterator orbColorPos, targColorPos;
      std::map<wxString, bool>::iterator drawOrbitPos, showObjectPos;
      orbColorPos = mOrbitColorMap.find(oldName);
      targColorPos = mTargetColorMap.find(oldName);
      drawOrbitPos = mDrawOrbitMap.find(oldName);
      showObjectPos = mShowObjectMap.find(oldName);
   
      if (orbColorPos != mOrbitColorMap.end() &&
          targColorPos != mTargetColorMap.end())
      {
         // add new spacecraft name key and delete old
         mOrbitColorMap[newName] = mOrbitColorMap[oldName];
         mTargetColorMap[newName] = mTargetColorMap[oldName];
         mDrawOrbitMap[newName] = mDrawOrbitMap[oldName];
         mShowObjectMap[newName] = mShowObjectMap[oldName];
         mOrbitColorMap.erase(orbColorPos);
         mTargetColorMap.erase(targColorPos);
         mDrawOrbitMap.erase(drawOrbitPos);
         mShowObjectMap.erase(showObjectPos);
         
         #if DBGLVL_RENAME
         MessageInterface::ShowMessage(wxT("---After rename\n"));
         for (orbColorPos = mOrbitColorMap.begin();
              orbColorPos != mOrbitColorMap.end(); ++orbColorPos)
         {
            MessageInterface::ShowMessage
               (wxT("sc=%s, color=%d\n"), orbColorPos->first.c_str(), orbColorPos->second);
         }
         #endif
      }
   }
   else if (type == Gmat::COORDINATE_SYSTEM)
   {
      if (mViewCoordSysName == oldName)
         mViewCoordSysName = newName;

      if (mViewUpCoordSysName == oldName)
         mViewUpCoordSysName = newName;      
   }
   
   return true;
}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
wxString OpenGlPlot::GetParameterText(const Integer id) const
{
   if (id >= SubscriberParamCount && id < OpenGlPlotParamCount)
      return PARAMETER_TEXT[id - SubscriberParamCount];
   else
      return Subscriber::GetParameterText(id);
    
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
Integer OpenGlPlot::GetParameterID(const wxString &str) const
{
   for (int i=SubscriberParamCount; i<OpenGlPlotParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - SubscriberParamCount])
         return i;
   }
   
   return Subscriber::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
Gmat::ParameterType OpenGlPlot::GetParameterType(const Integer id) const
{
   if (id >= SubscriberParamCount && id < OpenGlPlotParamCount)
      return PARAMETER_TYPE[id - SubscriberParamCount];
   else
      return Subscriber::GetParameterType(id);
}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
wxString OpenGlPlot::GetParameterTypeString(const Integer id) const
{
   if (id >= SubscriberParamCount && id < OpenGlPlotParamCount)
      return GmatBase::PARAM_TYPE_STRING[GetParameterType(id - SubscriberParamCount)];
   else
      return Subscriber::GetParameterTypeString(id);
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <id> Description for the parameter.
 *
 * @return true if the parameter is read only, false (the default) if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool OpenGlPlot::IsParameterReadOnly(const Integer id) const
{
   //Note: We can remove PERSPECTIVE_MODE, USE_FIXED_FOV, FIXED_FOV_ANGLE
   //      when perspective mode is working.
   
   if (id == OVERLAP_PLOT ||
       id == PERSPECTIVE_MODE || id == USE_FIXED_FOV || id == FIXED_FOV_ANGLE ||
       id == EARTH_SUN_LINES || id == VIEWPOINT_REF || id == VIEWPOINT_REF_VECTOR ||
       id == VIEWPOINT_VECTOR_VECTOR || id == VIEW_DIRECTION_VECTOR ||
       id == VIEWPOINT_REF_TYPE || id == VIEWPOINT_VECTOR_TYPE ||
       id == VIEW_DIRECTION_TYPE)
      return true;
   
   return Subscriber::IsParameterReadOnly(id);
}


//------------------------------------------------------------------------------
// virtual Integer GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
Integer OpenGlPlot::GetIntegerParameter(const Integer id) const
{
   switch (id)
   {
   case DATA_COLLECT_FREQUENCY:
      return mDataCollectFrequency;
   case UPDATE_PLOT_FREQUENCY:
      return mUpdatePlotFrequency;
   case NUM_POINTS_TO_REDRAW:
      return mNumPointsToRedraw;
        case STAR_COUNT:
                return mStarCount;
        case MIN_FOV:
                return mMinFOV;
        case MAX_FOV:
                return mMaxFOV;
        case INITIAL_FOV:
                return mInitialFOV;
   default:
      return Subscriber::GetIntegerParameter(id);
   }
}


//------------------------------------------------------------------------------
// virtual Integer GetIntegerParameter(const wxString &label) const
//------------------------------------------------------------------------------
Integer OpenGlPlot::GetIntegerParameter(const wxString &label) const
{
   return GetIntegerParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual Integer SetIntegerParameter(const Integer id, const Integer value)
//------------------------------------------------------------------------------
Integer OpenGlPlot::SetIntegerParameter(const Integer id, const Integer value)
{
   switch (id)
   {
   case DATA_COLLECT_FREQUENCY:
      if (value > 0)
      {
         mDataCollectFrequency = value;
         return value;
      }
      else
      {
         SubscriberException se;
         se.SetDetails(errorMessageFormat.c_str(),
                       GmatStringUtil::ToString(value, 1).c_str(),
                       wxT("DataCollectFrequency"), wxT("Integer Number > 0"));
         throw se;
      }
   case UPDATE_PLOT_FREQUENCY:
      if (value > 0)
      {
         mUpdatePlotFrequency = value;
         return value;
      }
      else
      {
         SubscriberException se;
         se.SetDetails(errorMessageFormat.c_str(),
                       GmatStringUtil::ToString(value, 1).c_str(),
                       wxT("UpdatePlotFrequency"), wxT("Integer Number > 0"));
         throw se;
      }
   case NUM_POINTS_TO_REDRAW:
      if (value >= 0)
      {
         mNumPointsToRedraw = value;
         return value;
      }
      else
      {
         SubscriberException se;
         se.SetDetails(errorMessageFormat.c_str(),
                       GmatStringUtil::ToString(value, 1).c_str(),
                       wxT("NumPointsToRedraw"), wxT("Integer Number >= 0"));
         throw se;
      }
        case STAR_COUNT:
                if (value >= 0)
                {
                        mStarCount = value;
                        return value;
                }
                else
                {
                        SubscriberException se;
                        se.SetDetails(errorMessageFormat.c_str(),
                                                        GmatStringUtil::ToString(value, 1).c_str(),
                                                        wxT("StarCount"), wxT("Integer Value >= 0"));
                }
        case MIN_FOV:
                mMinFOV = value;
                return value;
        case MAX_FOV:
                mMaxFOV = value;
                return value;
        case INITIAL_FOV:
                mInitialFOV = value;
                return value;
   default:
      return Subscriber::SetIntegerParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// virtual Integer SetIntegerParameter(const wxString &label,
//                                     const Integer value)
//------------------------------------------------------------------------------
Integer OpenGlPlot::SetIntegerParameter(const wxString &label,
                                        const Integer value)
{
   return SetIntegerParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// virtual Real GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
Real OpenGlPlot::GetRealParameter(const Integer id) const
{
   switch (id)
   {
   case VIEW_SCALE_FACTOR:
      return mViewScaleFactor;
   case FIXED_FOV_ANGLE:
      return mFixedFovAngle;
   default:
      return Subscriber::GetRealParameter(id);
   }
}


//------------------------------------------------------------------------------
// virtual Real GetRealParameter(const wxString &label) const
//------------------------------------------------------------------------------
Real OpenGlPlot::GetRealParameter(const wxString &label) const
{
   #if DBGLVL_OPENGL_PARAM
     MessageInterface::ShowMessage
        (wxT("OpenGlPlot::GetRealParameter() label = %s\n"), label.c_str());
   #endif
   
   return GetRealParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual Real SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
Real OpenGlPlot::SetRealParameter(const Integer id, const Real value)
{
   switch (id)
   {
   case VIEW_SCALE_FACTOR:
      mViewScaleFactor = value;
      return value;
   case FIXED_FOV_ANGLE:
      mFixedFovAngle = value;
      return value;
   default:
      return Subscriber::SetRealParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// virtual Real SetRealParameter(const wxString &label, const Real value)
//------------------------------------------------------------------------------
Real OpenGlPlot::SetRealParameter(const wxString &label, const Real value)
{
   #if DBGLVL_OPENGL_PARAM
      MessageInterface::ShowMessage
         (wxT("OpenGlPlot::SetRealParameter() label = %s, value = %f \n"),
          label.c_str(), value);
   #endif
   
   return SetRealParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
Real OpenGlPlot::GetRealParameter(const Integer id, const Integer index) const
{
   switch (id)
   {
   case VIEWPOINT_REF_VECTOR:
      WriteDeprecatedMessage(id);
      return mViewPointRefVector[index];
      
   case VIEWPOINT_VECTOR_VECTOR:
      WriteDeprecatedMessage(id);
      return mViewPointVecVector[index];
      
   case VIEW_DIRECTION_VECTOR:
      WriteDeprecatedMessage(id);
      return mViewDirectionVector[index];
      
   default:
      return Subscriber::GetRealParameter(id, index);
   }
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value, const Integer index)
//------------------------------------------------------------------------------
Real OpenGlPlot::SetRealParameter(const Integer id, const Real value,
                                  const Integer index)
{
   switch (id)
   {
   case VIEWPOINT_REF_VECTOR:
      WriteDeprecatedMessage(id);
      mViewPointRefVector[index] = value;
      return value;
      
   case VIEWPOINT_VECTOR_VECTOR:
      WriteDeprecatedMessage(id);
      mViewPointVecVector[index] = value;
      return value;
      
   case VIEW_DIRECTION_VECTOR:
      WriteDeprecatedMessage(id);
      mViewDirectionVector[index] = value;
      return value;
      
   default:
      return Subscriber::SetRealParameter(id, value, index);
   }
}


//------------------------------------------------------------------------------
// virtual const Rvector& GetRvectorParameter(const Integer id) const
//------------------------------------------------------------------------------
const Rvector& OpenGlPlot::GetRvectorParameter(const Integer id) const
{
   switch (id)
   {
   case VIEWPOINT_REF_VECTOR:
      //WriteDeprecatedMessage(id);
      return mViewPointRefVector;
   case VIEWPOINT_VECTOR_VECTOR:
      {
         //WriteDeprecatedMessage(id);
         #if DBGLVL_OPENGL_PARAM
         Rvector vec = mViewPointVecVector;
         MessageInterface::ShowMessage
            (wxT("OpenGlPlot::GetRvectorParameter() returning = %s\n"),
             vec.ToString().c_str());
         #endif
         return mViewPointVecVector;
      }
   case VIEW_DIRECTION_VECTOR:
      //WriteDeprecatedMessage(id);
      return mViewDirectionVector;
   default:
      return Subscriber::GetRvectorParameter(id);
   }
}


//------------------------------------------------------------------------------
// virtual const Rvector& GetRvectorParameter(const wxString &label) const
//------------------------------------------------------------------------------
const Rvector& OpenGlPlot::GetRvectorParameter(const wxString &label) const
{
   #if DBGLVL_OPENGL_PARAM
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::GetRvectorParameter() label = %s\n"), label.c_str());
   #endif
   
   return GetRvectorParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual const Rvector& SetRvectorParameter(const Integer id,
//                                            const Rvector &value)
//------------------------------------------------------------------------------
const Rvector& OpenGlPlot::SetRvectorParameter(const Integer id,
                                               const Rvector &value)
{
   switch (id)
   {
   case VIEWPOINT_REF_VECTOR:
      WriteDeprecatedMessage(id);
      mViewPointRefVector[0] = value[0];
      mViewPointRefVector[1] = value[1];
      mViewPointRefVector[2] = value[2];
      return value;
      
   case VIEWPOINT_VECTOR_VECTOR:
      mViewPointVecVector[0] = value[0];
      mViewPointVecVector[1] = value[1];
      mViewPointVecVector[2] = value[2];
      return value;
      
   case VIEW_DIRECTION_VECTOR:
      WriteDeprecatedMessage(id);
      mViewDirectionVector[0] = value[0];
      mViewDirectionVector[1] = value[1];
      mViewDirectionVector[2] = value[2];
      return value;
      
   default:
      return Subscriber::SetRvectorParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// virtual const Rvector& SetRvectorParameter(const wxString &label,
//                                            const Rvector &value)
//------------------------------------------------------------------------------
const Rvector& OpenGlPlot::SetRvectorParameter(const wxString &label,
                                               const Rvector &value)
{
   #if DBGLVL_OPENGL_PARAM
   Rvector val = value;
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::SetRvectorParameter() label = %s, ")
       wxT("value = %s \n"), label.c_str(), val.ToString().c_str());
   #endif
   
   return SetRvectorParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
wxString OpenGlPlot::GetStringParameter(const Integer id) const
{
   #if DBGLVL_OPENGL_PARAM_STRING
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::GetStringParameter()<%s> id=%d<%s>\n"),
       instanceName.c_str(), id, GetParameterText(id).c_str());
   #endif
   
   switch (id)
   {
   case COORD_SYSTEM:
      return mViewCoordSysName;
   case VIEWPOINT_REF:      
      WriteDeprecatedMessage(id);
      if (mViewPointRefType == wxT("Vector"))
         return (wxT("[ ") + mViewPointRefVector.ToString(16) + wxT(" ]"));
      else
         return mViewPointRefName;
   case VIEWPOINT_REFERENCE:      
      if (mViewPointRefType == wxT("Vector"))         
         return (wxT("[ ") + mViewPointRefVector.ToString(16) + wxT(" ]"));
      else
         return mViewPointRefName;
   case VIEWPOINT_REF_TYPE:
      return mViewPointRefType;
   case VIEWPOINT_VECTOR:
      if (mViewPointVecType == wxT("Vector"))
         return (wxT("[ ") + mViewPointVecVector.ToString(16) + wxT(" ]"));
      else
         return mViewPointVecName;
   case VIEWPOINT_VECTOR_TYPE:
      return mViewPointVecType;
   case VIEW_DIRECTION:
      if (mViewDirectionType == wxT("Vector"))
         return (wxT("[ ") + mViewDirectionVector.ToString(16) + wxT(" ]"));
      else
         return mViewDirectionName;
   case VIEW_DIRECTION_TYPE:
      return mViewDirectionType;
   case VIEW_UP_COORD_SYSTEM:
      return mViewUpCoordSysName;
   case VIEW_UP_AXIS:
      return mViewUpAxisName;
   default:
      return Subscriber::GetStringParameter(id);
   }
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString OpenGlPlot::GetStringParameter(const wxString &label) const
{
   #if DBGLVL_OPENGL_PARAM_STRING
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::GetStringParameter() label = %s\n"), label.c_str());
   #endif
   
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
bool OpenGlPlot::SetStringParameter(const Integer id, const wxString &value)
{
   #if DBGLVL_OPENGL_PARAM_STRING
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::SetStringParameter() this=<%p>'%s', id=%d<%s>, value='%s'\n"),
       this, instanceName.c_str(), id, GetParameterText(id).c_str(), value.c_str());
   #endif
   
   switch (id)
   {
   case ADD:
      return AddSpacePoint(value, mAllSpCount);
   case ORBIT_COLOR:
   case TARGET_COLOR:
      if (value[0] == wxT('['))
         PutUnsignedIntValue(id, value);
      return true;
   case COORD_SYSTEM:
      mViewCoordSysName = value;
      return true;
   case VIEWPOINT_REF:
      WriteDeprecatedMessage(id);
      mViewPointRefName = value;
      mViewPointRefType = wxT("Object");
      
      // Handle deprecated value wxT("Vector")
      if (value == wxT("Vector") || GmatStringUtil::IsNumber(value))
         mViewPointRefType = wxT("Vector");
      
      if (value[0] == wxT('['))
      {
         PutRvector3Value(mViewPointRefVector, id, value);
         mViewPointRefType = wxT("Vector");
      }
      return true;
   case VIEWPOINT_REFERENCE:
      mViewPointRefName = value;
      mViewPointRefType = wxT("Object");
      
      // Handle deprecated value wxT("Vector")
      if (value == wxT("Vector") || GmatStringUtil::IsNumber(value))
         mViewPointRefType = wxT("Vector");
      
      if (value[0] == wxT('['))
      {
         PutRvector3Value(mViewPointRefVector, id, value);
         mViewPointRefType = wxT("Vector");
      }
   case VIEWPOINT_REF_TYPE:
      mViewPointRefType = value;
      return true;
   case VIEWPOINT_VECTOR:
      mViewPointVecName = value;
      mViewPointVecType = wxT("Object");
      
      // Handle deprecated value wxT("Vector")
      if (value == wxT("Vector") || GmatStringUtil::IsNumber(value))
         mViewPointVecType = wxT("Vector");
      
      if (value[0] == wxT('['))
      {
         PutRvector3Value(mViewPointVecVector, id, value);
         mViewPointVecType = wxT("Vector");
      }
      return true;
   case VIEWPOINT_VECTOR_TYPE:
      mViewPointVecType = value;
      return true;
   case VIEW_DIRECTION:
      mViewDirectionName = value;
      mViewDirectionType = wxT("Object");
      
      // Handle deprecated value wxT("Vector")
      if (value == wxT("Vector") || GmatStringUtil::IsNumber(value))
         mViewDirectionType = wxT("Vector");
      
      if (value[0] == wxT('['))
      {
         PutRvector3Value(mViewDirectionVector, id, value);
         mViewDirectionType = wxT("Vector");
      }
      return true;
   case VIEW_DIRECTION_TYPE:
      mViewDirectionType = value;
      return true;
   case VIEW_UP_COORD_SYSTEM:
      mViewUpCoordSysName = value;
      return true;
   case VIEW_UP_AXIS:
      mViewUpAxisName = value;
      return true;
   default:
      return Subscriber::SetStringParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
bool OpenGlPlot::SetStringParameter(const wxString &label,
                                    const wxString &value)
{
   #if DBGLVL_OPENGL_PARAM_STRING
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::SetStringParameter()<%s> label=%s, value=%s \n"),
       instanceName.c_str(), label.c_str(), value.c_str());
   #endif
   
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// virtual bool SetStringParameter(const Integer id, const wxString &value,
//                                 const Integer index)
//------------------------------------------------------------------------------
bool OpenGlPlot::SetStringParameter(const Integer id, const wxString &value,
                                    const Integer index)
{
   #if DBGLVL_OPENGL_PARAM_STRING
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::SetStringParameter()<%s> id=%d<%s>, value=%s, index= %d\n"),
       instanceName.c_str(), id, GetParameterText(id).c_str(), value.c_str(), index);
   #endif
   
   switch (id)
   {
   case ADD:
      return AddSpacePoint(value, index);
   case VIEWPOINT_REF:
      WriteDeprecatedMessage(id);
      mViewPointRefType = wxT("Vector");
      PutRvector3Value(mViewPointRefVector, id, value, index);
      return true;
   case VIEWPOINT_REFERENCE:
      mViewPointRefType = wxT("Vector");
      PutRvector3Value(mViewPointRefVector, id, value, index);
      return true;
   case VIEWPOINT_VECTOR:
      mViewPointVecType = wxT("Vector");
      PutRvector3Value(mViewPointVecVector, id, value, index);
      return true;
   case VIEW_DIRECTION:
      mViewDirectionType = wxT("Vector");
      PutRvector3Value(mViewDirectionVector, id, value, index);
      return true;
   default:
      return Subscriber::SetStringParameter(id, value, index);
   }
}


//------------------------------------------------------------------------------
// virtual bool SetStringParameter(const wxString &label,
//                                 const wxString &value,
//                                 const Integer index)
//------------------------------------------------------------------------------
bool OpenGlPlot::SetStringParameter(const wxString &label,
                                    const wxString &value,
                                    const Integer index)
{
   #if DBGLVL_OPENGL_PARAM_STRING
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::SetStringParameter() label = %s, value = %s, index = %d\n"),
       label.c_str(), value.c_str(), index);
   #endif
   
   return SetStringParameter(GetParameterID(label), value, index);
}


//------------------------------------------------------------------------------
// virtual const UnsignedIntArray&
// GetUnsignedIntArrayParameter(const Integer id) const
//------------------------------------------------------------------------------
const UnsignedIntArray&
OpenGlPlot::GetUnsignedIntArrayParameter(const Integer id) const
{   
   switch (id)
   {
   case ORBIT_COLOR:
      return mOrbitColorArray;
   case TARGET_COLOR:
      return mTargetColorArray;
   default:
      return Subscriber::GetUnsignedIntArrayParameter(id);
   }
}


//------------------------------------------------------------------------------
// virtual UnsignedInt SetUnsignedIntParameter(const Integer id,
//                                             const UnsignedInt value,
//                                             const Integer index)
//------------------------------------------------------------------------------
UnsignedInt OpenGlPlot::SetUnsignedIntParameter(const Integer id,
                                                const UnsignedInt value,
                                                const Integer index)
{
   #if DBGLVL_OPENGL_PARAM
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::SetUnsignedIntParameter() this=%s\n   id=%d, value=%u, index=%d, ")
       wxT("mAllSpCount=%d, mOrbitColorArray.size()=%d, mTargetColorArray.size()=%d\n"),
       instanceName.c_str(), id, value, index, mAllSpCount, mOrbitColorArray.size(),
       mTargetColorArray.size());
   #endif
   
   switch (id)
   {
   case ORBIT_COLOR:
      {
         Integer size = mAllSpNameArray.size();
         if (index >= size)
            throw SubscriberException
               (wxT("index out of bounds for ") + GetParameterText(id));
         
         for (int i=0; i<size; i++)
         {
            if (index == i)
               mOrbitColorMap[mAllSpNameArray[i]] = value;
            
            if (index < size)
               mOrbitColorArray[index] = value;
            else
               mOrbitColorArray.push_back(value);
         }
         return value;
      }
   case TARGET_COLOR:
      {
         Integer size = mAllSpNameArray.size();
         if (index >= size)
            throw SubscriberException
               (wxT("index out of bounds for ") + GetParameterText(id));
         
         for (int i=0; i<size; i++)
         {
            if (index == i)
               mTargetColorMap[mAllSpNameArray[i]] = value;
            
            if (index < size)
               mTargetColorArray[index] = value;
            else
               mTargetColorArray.push_back(value);
         }
         return value;
      }
   default:
      return Subscriber::SetUnsignedIntParameter(id, value, index);
   }
}


//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const Integer id) const
//------------------------------------------------------------------------------
const StringArray& OpenGlPlot::GetStringArrayParameter(const Integer id) const
{
   switch (id)
   {
   case ADD:
      return mAllSpNameArray;
   default:
      return Subscriber::GetStringArrayParameter(id);
   }
}


//------------------------------------------------------------------------------
// bool GetBooleanParameter(const Integer id) const
//------------------------------------------------------------------------------
bool OpenGlPlot::GetBooleanParameter(const Integer id) const
{
   if (id == SHOW_PLOT)
      return active;
   return Subscriber::GetBooleanParameter(id);
}


//------------------------------------------------------------------------------
// bool SetBooleanParameter(const Integer id, const bool value)
//------------------------------------------------------------------------------
bool OpenGlPlot::SetBooleanParameter(const Integer id, const bool value)
{
   #if DBGLVL_OPENGL_PARAM
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::SetBooleanParameter()<%s> id=%d, value=%d\n"),
       instanceName.c_str(), id, value);
   #endif
   
   if (id == SHOW_PLOT)
   {
      active = value;
      return active;
   }
   return Subscriber::SetBooleanParameter(id, value);
}


//---------------------------------------------------------------------------
//  wxString GetOnOffParameter(const Integer id) const
//---------------------------------------------------------------------------
wxString OpenGlPlot::GetOnOffParameter(const Integer id) const
{
   switch (id)
   {
   case CELESTIAL_PLANE:
      return mEclipticPlane;
   case XY_PLANE:
      return mXYPlane;
   case WIRE_FRAME:
      return mWireFrame;
   case AXES:
      return mAxes;
   case GRID:
      return mGrid;
   case EARTH_SUN_LINES:
      return mSunLine;
   case SUN_LINE:
      return mSunLine;
   case OVERLAP_PLOT:
      return mOverlapPlot;
   case USE_INITIAL_VIEW:
      return mUseInitialView;
   case PERSPECTIVE_MODE:
      return mPerspectiveMode;
   case USE_FIXED_FOV:
      return mUseFixedFov;
        case ENABLE_STARS:
                return mEnableStars;
        case ENABLE_CONSTELLATIONS:
                return mEnableConstellations;
   default:
      return Subscriber::GetOnOffParameter(id);
   }
}


//------------------------------------------------------------------------------
// wxString OpenGlPlot::GetOnOffParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString OpenGlPlot::GetOnOffParameter(const wxString &label) const
{
   return GetOnOffParameter(GetParameterID(label));
}


//---------------------------------------------------------------------------
//  bool SetOnOffParameter(const Integer id, const wxString &value)
//---------------------------------------------------------------------------
bool OpenGlPlot::SetOnOffParameter(const Integer id, const wxString &value)
{
   switch (id)
   {
   case CELESTIAL_PLANE:
      mEclipticPlane = value;
      return true;
   case XY_PLANE:
      mXYPlane = value;
      return true;
   case WIRE_FRAME:
      mWireFrame = value;
      return true;
   case AXES:
      mAxes = value;
      return true;
   case GRID:
      mGrid = value;
      return true;
   case EARTH_SUN_LINES:
      WriteDeprecatedMessage(id);
      mSunLine = value;
      return true;
   case SUN_LINE:
      mSunLine = value;
      return true;
   case OVERLAP_PLOT:
      mOverlapPlot = value;
      return true;
   case USE_INITIAL_VIEW:
      mUseInitialView = value;
      return true;
   case PERSPECTIVE_MODE:
      mPerspectiveMode = value;
      return true;
   case USE_FIXED_FOV:
      mUseFixedFov = value;
      return true;
        case ENABLE_STARS:
                mEnableStars = value;
                return true;
        case ENABLE_CONSTELLATIONS:
                mEnableConstellations = value;
                return true;
   default:
      return Subscriber::SetOnOffParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetOnOffParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
bool OpenGlPlot::SetOnOffParameter(const wxString &label, 
                                   const wxString &value)
{
   return SetOnOffParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// virtual wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
wxString OpenGlPlot::GetRefObjectName(const Gmat::ObjectType type) const
{
   #if DBGLVL_OPENGL_OBJ
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::GetRefObjectName() type: %s\n"),
       GmatBase::GetObjectTypeString(type).c_str());
   #endif
   
   if (type == Gmat::COORDINATE_SYSTEM)
   {
      return mViewCoordSysName; //just return this
   }
   
   #if DBGLVL_OPENGL_OBJ
   wxString msg = wxT("type: ") + GmatBase::GetObjectTypeString(type) + wxT(" not found");
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::GetRefObjectName() %s\n"), msg.c_str());
   #endif
   
   return Subscriber::GetRefObjectName(type);
}


//------------------------------------------------------------------------------
// virtual bool HasRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
bool OpenGlPlot::HasRefObjectTypeArray()
{
   return true;
}


//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by this class.
 *
 * @return the list of object types.
 * 
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& OpenGlPlot::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::SPACE_POINT);
   refObjectTypes.push_back(Gmat::COORDINATE_SYSTEM);
   return refObjectTypes;
}


//------------------------------------------------------------------------------
// virtual const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
const StringArray& OpenGlPlot::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   mAllRefObjectNames.clear();
   
   // if Draw Earth-Sun lines is on, add Earth and Sun
   if (mSunLine == wxT("On"))
   {
      AddSpacePoint(wxT("Earth"), mAllSpCount, false);
      AddSpacePoint(wxT("Sun"), mAllSpCount, false);
   }
   
   if (type == Gmat::COORDINATE_SYSTEM)
   {
      mAllRefObjectNames.push_back(mViewCoordSysName);
      mAllRefObjectNames.push_back(mViewUpCoordSysName);
   }
   else if (type == Gmat::SPACE_POINT)
   {
      mAllRefObjectNames = mAllSpNameArray;
      
      if (mViewPointRefType != wxT("Vector"))
      {
         if (find(mAllRefObjectNames.begin(), mAllRefObjectNames.end(),
                  mViewPointRefName) == mAllRefObjectNames.end())
            mAllRefObjectNames.push_back(mViewPointRefName);
      }
      
      if (mViewPointVecType != wxT("Vector"))
      {
         if (find(mAllRefObjectNames.begin(), mAllRefObjectNames.end(),
                  mViewPointVecName) == mAllRefObjectNames.end())
            mAllRefObjectNames.push_back(mViewPointVecName);
      }
      
      if (mViewDirectionType != wxT("Vector"))
      {
         if (find(mAllRefObjectNames.begin(), mAllRefObjectNames.end(),
                  mViewDirectionName) == mAllRefObjectNames.end())
            mAllRefObjectNames.push_back(mViewDirectionName);
      }
   }
   else if (type == Gmat::UNKNOWN_OBJECT)
   {
      #ifdef DEBUG_OPENGL_OBJ
      MessageInterface::ShowMessage
         (wxT("mViewPointRefType=%s, mViewPointVecType=%s, mViewDirectionType=%s\n"),
          mViewPointRefType.c_str(), mViewPointVecType.c_str(), mViewDirectionType.c_str());
      #endif
      
      mAllRefObjectNames = mAllSpNameArray;
      
      mAllRefObjectNames.push_back(mViewCoordSysName);
      
      if (mViewCoordSysName != mViewUpCoordSysName)
         mAllRefObjectNames.push_back(mViewUpCoordSysName);
      
      if (mViewPointRefType != wxT("Vector"))
      {
         if (find(mAllRefObjectNames.begin(), mAllRefObjectNames.end(),
                  mViewPointRefName) == mAllRefObjectNames.end())
            mAllRefObjectNames.push_back(mViewPointRefName);
      }
      
      if (mViewPointVecType != wxT("Vector"))
      {
         if (find(mAllRefObjectNames.begin(), mAllRefObjectNames.end(),
                  mViewPointVecName) == mAllRefObjectNames.end())
            mAllRefObjectNames.push_back(mViewPointVecName);
      }
      
      if (mViewDirectionType != wxT("Vector"))
      {
         if (find(mAllRefObjectNames.begin(), mAllRefObjectNames.end(),
                  mViewDirectionName) == mAllRefObjectNames.end())
            mAllRefObjectNames.push_back(mViewDirectionName);
      }
   }
   
   #if DBGLVL_OPENGL_OBJ
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::GetRefObjectNameArray() returning for type:%d\n"), type);
   for (unsigned int i=0; i<mAllRefObjectNames.size(); i++)
      MessageInterface::ShowMessage(wxT("   %s\n"), mAllRefObjectNames[i].c_str());
   #endif
   
   return mAllRefObjectNames;
}


//------------------------------------------------------------------------------
// virtual GmatBase* GetRefObject(const Gmat::ObjectType type,
//                                const wxString &name)
//------------------------------------------------------------------------------
GmatBase* OpenGlPlot::GetRefObject(const Gmat::ObjectType type,
                                   const wxString &name)
{
   if (type == Gmat::COORDINATE_SYSTEM)
   {
      if (name == mViewCoordSysName)
         return mViewCoordSystem;
      if (name == mViewUpCoordSysName)
         return mViewUpCoordSystem;
   }
   else if (type == Gmat::SPACE_POINT)
   {
      if (name == mViewPointRefName)
         return mViewPointRefObj;
      else if (name == mViewPointVecName)
         return mViewPointObj;
      else if (name == mViewDirectionName)
         return mViewDirectionObj;
   }
   
   return Subscriber::GetRefObject(type, name);
   
//    throw SubscriberException(wxT("OpenGlPlot::GetRefObject() the object name: ") + name +
//                            wxT("not found\n"));
}


//------------------------------------------------------------------------------
// virtual bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                           const wxString &name = wxT(""))
//------------------------------------------------------------------------------
/**
 * Set reference object pointer.
 *
 * @param <obj>  Reference object pointer to set to given object type and name
 * @param <type> Reference object type
 * @param <name> Reference object name
 */
//------------------------------------------------------------------------------
bool OpenGlPlot::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                              const wxString &name)
{
   #if DBGLVL_OPENGL_OBJ
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::SetRefObject() this=<%p>'%s', obj=<%p>'%s', type=%d[%s], name='%s'\n"),
       this, GetName().c_str(), obj, obj->GetName().c_str(), type,
       obj->GetTypeName().c_str(), name.c_str());
   #endif
   
   wxString realName = name;
   if (name == wxT(""))
      realName = obj->GetName();
   
   if (type == Gmat::COORDINATE_SYSTEM)
   {
      if (realName == mViewCoordSysName)
         mViewCoordSystem = (CoordinateSystem*)obj;
      if (realName == mViewUpCoordSysName)
         mViewUpCoordSystem = (CoordinateSystem*)obj;
      return true;
   }
   else if (obj->IsOfType(Gmat::SPACE_POINT))
   {
      #if DBGLVL_OPENGL_OBJ
      MessageInterface::ShowMessage(wxT("   mAllSpCount=%d\n"), mAllSpCount);
      #endif
      
      for (Integer i=0; i<mAllSpCount; i++)
      {
         #if DBGLVL_OPENGL_OBJ
         MessageInterface::ShowMessage
            (wxT("   mAllSpNameArray[%d]='%s'\n"), i, mAllSpNameArray[i].c_str());
         #endif
         
         if (mAllSpNameArray[i] == realName)
         {
            #if DBGLVL_OPENGL_OBJ > 1
            MessageInterface::ShowMessage
               (wxT("   Setting object to '%s'\n"), mAllSpNameArray[i].c_str());
            #endif
            
            mAllSpArray[i] = (SpacePoint*)(obj);
         }
      }
      
      #if DBGLVL_OPENGL_OBJ
      MessageInterface::ShowMessage
         (wxT("OpenGlPlot::SetRefObject() realName='%s', mViewPointRefName='%s', ")
          wxT("mViewPointVecName='%s', mViewDirectionName='%s'\n"), realName.c_str(),
          mViewPointRefName.c_str(), mViewPointVecName.c_str(),
          mViewDirectionName.c_str());
      #endif
      
      // ViewPoint info
      if (realName == mViewPointRefName)
         mViewPointRefObj = (SpacePoint*)obj;
      
      if (realName == mViewPointVecName)
         mViewPointObj = (SpacePoint*)obj;
      
      if (realName == mViewDirectionName)
         mViewDirectionObj = (SpacePoint*)obj;
      
      #if DBGLVL_OPENGL_OBJ
      MessageInterface::ShowMessage
         (wxT("OpenGlPlot::SetRefObject() mViewPointRefObj=<%p>, mViewPointObj=<%p>, ")
          wxT("mViewDirectionObj=<%p>\n"), mViewPointRefObj, mViewPointObj,
          mViewDirectionObj);
      #endif
      return true;
   }
   
   return Subscriber::SetRefObject(obj, type, realName);
}


//---------------------------------
// protected methods
//---------------------------------

//------------------------------------------------------------------------------
// bool AddSpacePoint(const wxString &name, Integer index, bool show = true)
//------------------------------------------------------------------------------
bool OpenGlPlot::AddSpacePoint(const wxString &name, Integer index, bool show)
{
   #if DBGLVL_OPENGL_ADD
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::AddSpacePoint()<%s> name=%s, index=%d, show=%d, mAllSpCount=%d\n"),
       instanceName.c_str(), name.c_str(), index, show, mAllSpCount);
   #endif
   
   // if name not in the list, add
   if (find(mAllSpNameArray.begin(), mAllSpNameArray.end(), name) ==
       mAllSpNameArray.end())
   {
      if (name != wxT("") && index == mAllSpCount)
      {
         mAllSpNameArray.push_back(name);
         mAllSpArray.push_back(NULL);
         mAllSpCount = mAllSpNameArray.size();
         
         mDrawOrbitMap[name] = show;
         mShowObjectMap[name] = show;
         
         if (mAllSpCount < MAX_SP_COLOR)
         {
            // If object is non-standard-body, use mNonStdBodyCount.
            // So that spacecraft color starts from DEFAULT_ORBIT_COLOR
            if (mOrbitColorMap.find(name) == mOrbitColorMap.end())
            {
               mOrbitColorMap[name] = DEFAULT_ORBIT_COLOR[mNonStdBodyCount];
               mTargetColorMap[name] = GmatColor::TEAL32;
               mOrbitColorArray.push_back(DEFAULT_ORBIT_COLOR[mNonStdBodyCount]);
               mTargetColorArray.push_back(GmatColor::TEAL32);
               mNonStdBodyCount++;
            }
            else
            {
               mOrbitColorArray.push_back(mOrbitColorMap[name]);
               mTargetColorArray.push_back(mTargetColorMap[name]);
            }
         }
         else
         {
            mOrbitColorMap[name] = GmatColor::RED32;
            mTargetColorMap[name] = GmatColor::TEAL32;
            mOrbitColorArray.push_back(GmatColor::RED32);
            mTargetColorArray.push_back(GmatColor::TEAL32);
         }
      }
   }
   
   #if DBGLVL_OPENGL_ADD   
   wxString objName;
   for (int i=0; i<mAllSpCount; i++)
   {
      objName = mAllSpNameArray[i];
      MessageInterface::ShowMessage
         (wxT("   mAllSpNameArray[%d]=%s, draw=%d, show=%d ")
          wxT("orbColor=%u, targColor=%u\n"), i, objName.c_str(), mDrawOrbitMap[objName],
          mShowObjectMap[objName], mOrbitColorMap[objName], mTargetColorMap[objName]);
      MessageInterface::ShowMessage
         (wxT("   mOrbitColorArray[%d]=%u, mTargetColorArray[%d]=%u\n"), i, mOrbitColorArray[i],
          i, mTargetColorArray[i]);
   }
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// bool ClearSpacePointList()
//------------------------------------------------------------------------------
bool OpenGlPlot::ClearSpacePointList()
{
   //MessageInterface::ShowMessage(wxT("OpenGlPlot::ClearSpacePointList()\n"));
   
   mAllSpNameArray.clear();
   mAllSpArray.clear();
   mObjectArray.clear();
   mDrawOrbitArray.clear();
   mShowObjectArray.clear();
   mScNameArray.clear();
   mObjectNameArray.clear();
   mOrbitColorArray.clear();
   mTargetColorArray.clear();
   
   mScXArray.clear();
   mScYArray.clear();
   mScZArray.clear();
   mScVxArray.clear();
   mScVyArray.clear();
   mScVzArray.clear();
   mOrbitColorMap.clear();
   mTargetColorMap.clear();
   mAllSpCount = 0;
   mScCount = 0;
   mObjectCount = 0;
   mNonStdBodyCount = 0;
   
   return true;
}


//------------------------------------------------------------------------------
// bool RemoveSpacePoint(const wxString &name)
//------------------------------------------------------------------------------
/*
 * Removes spacecraft from the spacecraft list
 *
 * @param <name> spacecraft name to be removed from the list
 *
 * @return true if spacecraft was removed from the list, false otherwise
 *
 */
//------------------------------------------------------------------------------
bool OpenGlPlot::RemoveSpacePoint(const wxString &name)
{
   //-----------------------------------------------------------------
   #ifdef __REMOVE_OBJ_BY_SETTING_FLAG__
   //-----------------------------------------------------------------

   for (UnsignedInt i=0; i<mObjectNameArray.size(); i++)
   {
      if (mObjectNameArray[i] == name)
      {
         mDrawOrbitArray[i] = false;
         PlotInterface::SetGlDrawOrbitFlag(instanceName, mDrawOrbitArray);
         return true;
      }
   }
   
   return false;
   
   //-----------------------------------------------------------------
   #else
   //-----------------------------------------------------------------
   
   bool removedFromScArray = false;
   bool removedFromAllSpArray = false;
   
   //-------------------------------------------------------
   // remove from mScNameArray
   //-------------------------------------------------------
   #if DBGLVL_REMOVE_SP
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::RemoveSpacePoint() name=%s\n--- Before remove from ")
       wxT("mScNameArray:\n"), name.c_str());
   MessageInterface::ShowMessage(wxT("mScCount=%d\n"), mScCount);
   for (int i=0; i<mScCount; i++)
   {
      MessageInterface::ShowMessage
         (wxT("mScNameArray[%d]=%s\n"), i, mScNameArray[i].c_str());
   }
   #endif
   
   StringArray::iterator scPos = 
      find(mScNameArray.begin(), mScNameArray.end(), name);
   
   if (scPos != mScNameArray.end())
   {
      MessageInterface::ShowMessage(wxT("sc to be erased=%s\n"), (*scPos).c_str());
      
      // erase given spacecraft from the arrays
      mScNameArray.erase(scPos);
      
      // just reduce the size of array
      mScOrbitColorArray.erase(mScOrbitColorArray.begin());
      mScTargetColorArray.erase(mScTargetColorArray.begin());
      mScXArray.erase(mScXArray.begin());
      mScYArray.erase(mScYArray.begin());
      mScZArray.erase(mScZArray.begin());
      mScVxArray.erase(mScVxArray.begin());
      mScVyArray.erase(mScVyArray.begin());
      mScVzArray.erase(mScVzArray.begin());
      
      mScCount = mScNameArray.size();
      
      // update color array
      for (int i=0; i<mScCount; i++)
      {
         mScOrbitColorArray[i] = mOrbitColorMap[mScNameArray[i]];
         mScTargetColorArray[i] = mTargetColorMap[mScNameArray[i]];
      }
      
      #if DBGLVL_REMOVE_SP
      MessageInterface::ShowMessage(wxT("---After remove from mScNameArray:\n"));
      MessageInterface::ShowMessage(wxT("mScCount=%d\n"), mScCount);
      for (int i=0; i<mScCount; i++)
      {
         MessageInterface::ShowMessage
            (wxT("mScNameArray[%d]=%s\n"), i, mScNameArray[i].c_str());
      }
      #endif
      
      removedFromScArray = true;
      //return true;
   }
   
   
   //-------------------------------------------------------
   // remove from mAllSpNameArray and mObjectNameArray
   //-------------------------------------------------------
   #if DBGLVL_REMOVE_SP
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::RemoveSpacePoint() name=%s\n--- Before remove from ")
       wxT("mAllSpNameArray:\n"), name.c_str());
   MessageInterface::ShowMessage(wxT("mAllSpCount=%d\n"), mAllSpCount);
   for (int i=0; i<mAllSpCount; i++)
   {
      MessageInterface::ShowMessage
         (wxT("mAllSpNameArray[%d]=%s\n"), i, mAllSpNameArray[i].c_str());
   }
   #endif
   
   StringArray::iterator spPos = 
      find(mAllSpNameArray.begin(), mAllSpNameArray.end(), name);
   StringArray::iterator objPos = 
      find(mObjectNameArray.begin(), mObjectNameArray.end(), name);
   
   if (spPos != mAllSpNameArray.end() && objPos != mObjectNameArray.end())
   {
      std::map<wxString, UnsignedInt>::iterator orbColorPos, targColorPos;
      orbColorPos = mOrbitColorMap.find(name);
      targColorPos = mTargetColorMap.find(name);
      
      if (orbColorPos != mOrbitColorMap.end() &&
          targColorPos != mTargetColorMap.end())
      {
         // erase given spacecraft name
         mAllSpNameArray.erase(spPos);
         mObjectNameArray.erase(objPos);
         mOrbitColorMap.erase(orbColorPos);
         mTargetColorMap.erase(targColorPos);
         
         // reduce the size of array
         mOrbitColorArray.erase(mOrbitColorArray.begin());
         mTargetColorArray.erase(mTargetColorArray.begin());
         
         mAllSpCount = mAllSpNameArray.size();
         
         // update color array
         for (int i=0; i<mAllSpCount; i++)
         {
            mOrbitColorArray[i] = mOrbitColorMap[mAllSpNameArray[i]];
            mTargetColorArray[i] = mTargetColorMap[mAllSpNameArray[i]];
         }
         
         #if DBGLVL_REMOVE_SP
         MessageInterface::ShowMessage(wxT("---After remove from mAllSpNameArray\n"));
         MessageInterface::ShowMessage(wxT("mAllSpCount=%d\n"), mAllSpCount);
         for (int i=0; i<mAllSpCount; i++)
         {
            MessageInterface::ShowMessage
               (wxT("mAllSpNameArray[%d]=%s\n"), i, mAllSpNameArray[i].c_str());
         }
         #endif
         
         removedFromAllSpArray = true;
      }
   }

   //-------------------------------------------------------
   // remove from mObjectArray
   //-------------------------------------------------------
   #if DBGLVL_REMOVE_SP
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::RemoveSpacePoint() name=%s\n--- Before remove from ")
       wxT("mObjectArray:\n"), name.c_str());
   MessageInterface::ShowMessage(wxT("size=%d\n"), mObjectArray.size());
   #endif
   
   for (std::vector<SpacePoint*>::iterator objptPos = mObjectArray.begin();
        objptPos != mObjectArray.end(); ++objptPos)
   {
      MessageInterface::ShowMessage
         (wxT("mObjectArray=%s\n"), (*objptPos)->GetName().c_str());
      if ((*objptPos)->GetName() == name)
      {
         mObjectArray.erase(objptPos);
         break;
      }
   }
   
   #if DBGLVL_REMOVE_SP
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::RemoveSpacePoint() name=%s\n--- After remove from ")
       wxT("mObjectArray:\n"), name.c_str());
   MessageInterface::ShowMessage(wxT("size=%d\n"), mObjectArray.size());
   #endif
   
   if (removedFromScArray && removedFromAllSpArray)
      // set all object array and pointers
      PlotInterface::SetGlObject(instanceName, mObjectNameArray,
                                 mOrbitColorArray, mObjectArray);
   
   return (removedFromScArray && removedFromAllSpArray);
   
   #endif
}


//------------------------------------------------------------------------------
// void ClearDynamicArrays()
//------------------------------------------------------------------------------
void OpenGlPlot::ClearDynamicArrays()
{
   mObjectNameArray.clear();
   mOrbitColorArray.clear();
   mTargetColorArray.clear();
   mObjectArray.clear();
   mDrawOrbitArray.clear();
   mShowObjectArray.clear();
   mScNameArray.clear();
   mScOrbitColorArray.clear();
   mScTargetColorArray.clear();
   mScXArray.clear();
   mScYArray.clear();
   mScZArray.clear();
   mScVxArray.clear();
   mScVyArray.clear();
   mScVzArray.clear();
}


//------------------------------------------------------------------------------
// void UpdateObjectList(SpacePoint *sp, bool show = false)
//------------------------------------------------------------------------------
/**
 * Add non-spacecraft object to the list.
 */
//------------------------------------------------------------------------------
void OpenGlPlot::UpdateObjectList(SpacePoint *sp, bool show)
{   
   // Add all spacepoint objects
   wxString name = sp->GetName();
   StringArray::iterator pos = 
      find(mObjectNameArray.begin(), mObjectNameArray.end(), name);
   
   // if name not found, add to arrays
   if (pos == mObjectNameArray.end())
   {
      mObjectNameArray.push_back(name);
      mOrbitColorArray.push_back(mOrbitColorMap[name]);
      mTargetColorArray.push_back(mTargetColorMap[name]);
      mObjectArray.push_back(sp);
      mDrawOrbitMap[name] = show;
      mShowObjectMap[name] = show;
      mDrawOrbitArray.push_back(show);
      mShowObjectArray.push_back(show);
      mObjectCount = mObjectNameArray.size();
   }
   
   #if DBGLVL_OPENGL_INIT > 1
   Integer draw, showObj;
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::UpdateObjectList() instanceName=%s\n"), instanceName.c_str());
   for (int i=0; i<mObjectCount; i++)
   {
      draw = mDrawOrbitArray[i] ? 1 : 0;
      showObj = mShowObjectArray[i] ? 1 : 0;
      MessageInterface::ShowMessage
         (wxT("   mObjectNameArray[%d]=%s, draw=%d, show=%d, color=%d\n"), i,
          mObjectNameArray[i].c_str(), draw, showObj, mOrbitColorArray[i]);
   }
   #endif
}


//------------------------------------------------------------------------------
// void PutRvector3Value(Rvector3 &rvec3, Integer id,
//                       const wxString &sval, Integer index = -1);
//------------------------------------------------------------------------------
/*
 * Converts input string to Real and store as Rvector3 element at idnex.
 *
 * @param rvec3  input Rvector3 where value to be stored
 * @param id     input Parameter ID used for formating error message
 * @param sval   input string value
 * @param index  input index to be used for storing a Rvector3 element
 *               if index is -1, whole 3 elements are converted and stored
 *               from a string format of wxT("[element1 element2 element3]")
 */
//------------------------------------------------------------------------------
void OpenGlPlot::PutRvector3Value(Rvector3 &rvec3, Integer id,
                                  const wxString &sval, Integer index)
{
   #if DBGLVL_OPENGL_PARAM_RVEC3
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::PutRvector3Value() id=%d, sval=%s, index=%d\n"),
       id, sval.c_str(), index);
   #endif
   
   wxString badVal;
   bool isValid = true;
   wxString field = GetParameterText(id);
   
   // Check index, throw exception if out of bound
   if (index < -1 || index > 2)
   {
      badVal = sval;
      isValid = false;
   }
   
   if (isValid)
   {
      // Convert string value to Real
      if (index != -1)
      {
         Real rval;
         if (GmatStringUtil::ToReal(sval, rval))
            rvec3[index] = rval;
         else
         {
            isValid = false;
            badVal = sval;
         }
      }
      else if (index == -1)
      {
         StringArray valArray;
         wxString svalue = sval;
         svalue = GmatStringUtil::Trim(svalue);
         wxString::size_type index1 = svalue.find_first_of(wxT("["));
         if (index1 != svalue.npos)
         {
            wxString::size_type index2 = svalue.find_last_of(wxT("]"));
            if (index2 != svalue.npos)
            {
               svalue = svalue.substr(index1+1, index2-index1-1);
            }
            else
            {
               isValid = false;
               badVal = sval;
            }
         }
         
         valArray = GmatStringUtil::SeparateBy(svalue, wxT(" ,"));         
         int arraySize = valArray.size();
         
         if (arraySize == 3)
         {
            Real rvals[3];
            bool rvalsOk[3];
            rvalsOk[0] = GmatStringUtil::ToReal(valArray[0], rvals[0]);
            rvalsOk[1] = GmatStringUtil::ToReal(valArray[1], rvals[1]);
            rvalsOk[2] = GmatStringUtil::ToReal(valArray[2], rvals[2]);
            
            // Detects first invalid input and throw exception
            if (!rvalsOk[0])
               badVal = valArray[0];
            else if (!rvalsOk[1])
               badVal = valArray[1];
            else if (!rvalsOk[2])
               badVal = valArray[2];
            
            if (rvalsOk[0] && rvalsOk[1] && rvalsOk[2])
               rvec3.Set(rvals[0], rvals[1], rvals[2]);
            else
               isValid = false;
         }
         else
         {
            isValid = false;
            badVal = sval;
         }
      }
   }
   
   if (!isValid)
   {
      SubscriberException se;
      se.SetDetails(errorMessageFormat.c_str(), badVal.c_str(), field.c_str(),
                    wxT("SpacecraftName, CelestialBodyName, LibrationPointName, ")
                    wxT("BarycenterName, or a 3-vector of numerical values"));
      throw se;
   }
}


//------------------------------------------------------------------------------
// void PutUnsignedIntValue(Integer id, const wxString &sval)
//------------------------------------------------------------------------------
void OpenGlPlot::PutUnsignedIntValue(Integer id, const wxString &sval)
{
   #ifdef DEBUG_OPENGL_PUT
   MessageInterface::ShowMessage
      (wxT("PutUnsignedIntValue() id=%d, sval='%s'\n"), id, sval.c_str());
   #endif
   
   UnsignedIntArray vals = GmatStringUtil::ToUnsignedIntArray(sval);
   for (UnsignedInt i=0; i<vals.size(); i++)
      SetUnsignedIntParameter(id, vals[i], i);
}


//------------------------------------------------------------------------------
// void WriteDeprecatedMessage(Integer id) const
//------------------------------------------------------------------------------
void OpenGlPlot::WriteDeprecatedMessage(Integer id) const
{
   // Write only one message per session
   static bool writeEarthSunLines = true;
   static bool writeViewpointRef = true;
   static bool writeViewpointRefVector = true;
   static bool writeViewpointVectorVector = true;
   static bool writeViewDirectionVector = true;
   
   switch (id)
   {
   case EARTH_SUN_LINES:
      if (writeEarthSunLines)
      {
         MessageInterface::ShowMessage
            (wxT("*** WARNING *** \"EarthSunLines\" is deprecated and will be ")
             wxT("removed from a future build; please use \"SunLine\" ")
             wxT("instead.\n"));
         writeEarthSunLines = false;
      }
      break;
   case VIEWPOINT_REF:
      if (writeViewpointRef)
      {
         MessageInterface::ShowMessage
            (wxT("*** WARNING *** \"ViewPointRef\" is deprecated and will be ")
             wxT("removed from a future build; please use \"ViewPointReference\" ")
             wxT("instead.\n"));
         writeViewpointRef = false;
      }
      break;
   case VIEWPOINT_REF_VECTOR:
      if (writeViewpointRefVector)
      {
         MessageInterface::ShowMessage
            (wxT("*** WARNING *** \"ViewPointRefVector\" is deprecated and will be ")
             wxT("removed from a future build.\n"));
         writeViewpointRefVector = false;
      }
      break;
   case VIEWPOINT_VECTOR_VECTOR:
      if (writeViewpointVectorVector)
      {
         MessageInterface::ShowMessage
            (wxT("*** WARNING *** \"ViewPointVectorVector\" is deprecated and will be ")
             wxT("removed from a future build.\n"));
         writeViewpointVectorVector = false;
      }
      break;
   case VIEW_DIRECTION_VECTOR:
      if (writeViewDirectionVector)
      {
         MessageInterface::ShowMessage
            (wxT("*** WARNING *** \"ViewDirectionVector\" is deprecated and will be ")
             wxT("removed from a future build.\n"));
         writeViewDirectionVector = false;
      }
      break;
   default:
      break;
   }
}


//------------------------------------------------------------------------------
// bool UpdateSolverData()
//------------------------------------------------------------------------------
bool OpenGlPlot::UpdateSolverData()
{
   int size = mCurrEpochArray.size();
   int last = size - 1;
   
   #if DBGLVL_SOLVER_CURRENT_ITER
   MessageInterface::ShowMessage(wxT("===> num buffered data = %d\n"), size);
   MessageInterface::ShowMessage(wxT("==========> now update solver plot\n"));
   #endif
   
   if (size == 0)
      return true;
   
   UnsignedIntArray colorArray = mScOrbitColorArray;
   if (runstate == Gmat::SOLVING)
      colorArray = mScTargetColorArray;
   else
      colorArray = mScOrbitColorArray;
   
   // Update plot with last iteration data
   for (int i=0; i<size-1; i++)
   {
      #if DBGLVL_SOLVER_CURRENT_ITER > 1
      for (int sc=0; sc<mScCount; sc++)
         MessageInterface::ShowMessage
            (wxT("   i=%d, sc=%d, solver epoch = %f, X,Y,Z = %f, %f, %f\n"), i, sc,
             mCurrEpochArray[i], mCurrXArray[i][sc], mCurrYArray[i][sc],
             mCurrZArray[i][sc]);
      #endif
      
      // Just buffer data up to last point - 1
      PlotInterface::
         UpdateGlPlot(instanceName, mOldName, mCurrScArray[i],
                      mCurrEpochArray[i], mCurrXArray[i], mCurrYArray[i],
                      mCurrZArray[i], mCurrVxArray[i], mCurrVyArray[i],
                      mCurrVzArray[i], colorArray, true, mSolverIterOption, false);
   }
   
   // Buffer last point and Update the plot
   PlotInterface::
      UpdateGlPlot(instanceName, mOldName, mCurrScArray[last],
                   mCurrEpochArray[last], mCurrXArray[last], mCurrYArray[last],
                   mCurrZArray[last], mCurrVxArray[last], mCurrVyArray[last],
                   mCurrVzArray[last], colorArray, true, mSolverIterOption, true);
   
   // clear arrays
   mCurrScArray.clear();
   mCurrEpochArray.clear();
   mCurrXArray.clear();
   mCurrYArray.clear();
   mCurrZArray.clear();
   mCurrVxArray.clear();
   mCurrVyArray.clear();
   mCurrVzArray.clear();
   
   if (runstate == Gmat::SOLVING)
      PlotInterface::TakeGlAction(instanceName, wxT("ClearSolverData"));
   
   return true;
}


//--------------------------------------
// methods inherited from Subscriber
//--------------------------------------

//------------------------------------------------------------------------------
// bool Distribute(int len)
//------------------------------------------------------------------------------
bool OpenGlPlot::Distribute(int len)
{
   //loj: How do I convert data to Real data?
   return false;
}


//------------------------------------------------------------------------------
// bool Distribute(const Real *dat, Integer len)
//------------------------------------------------------------------------------
bool OpenGlPlot::Distribute(const Real *dat, Integer len)
{
   #if DBGLVL_OPENGL_UPDATE
   MessageInterface::ShowMessage
      (wxT("OpenGlPlot::Distribute() instanceName=%s, active=%d, isEndOfRun=%d, ")
       wxT("isEndOfReceive=%d\n   mAllSpCount=%d, mScCount=%d, len=%d, runstate=%d\n"),
       instanceName.c_str(), active, isEndOfRun, isEndOfReceive, mAllSpCount,
       mScCount, len, runstate);
   #endif
   
   if (GmatGlobal::Instance()->GetRunMode() == GmatGlobal::TESTING_NO_PLOTS)
      return true;
   
   if (!active || mScCount <= 0)
      return true;
   
   // test isEndOfRun first
   if (isEndOfRun)
      return PlotInterface::SetGlEndOfRun(instanceName);
   
   if (isEndOfReceive)
   {
      if ((mSolverIterOption == SI_CURRENT) &&
          (runstate == Gmat::SOLVING || runstate == Gmat::SOLVEDPASS))
      {
         UpdateSolverData();
      }
      else
      {
         return PlotInterface::RefreshGlPlot(instanceName);
      }
   }
   
   
   if (len <= 0)
      return true;
   
   
   #if DBGLVL_OPENGL_DATA
   MessageInterface::ShowMessage(wxT("%s, len=%d\n"), GetName().c_str(), len);
   for (int i=0; i<len; i++)
      MessageInterface::ShowMessage(wxT("%.11f  "), dat[i]);
   MessageInterface::ShowMessage(wxT("\n"));
   #endif
   
   //------------------------------------------------------------
   // if targeting and draw target is None, just return
   //------------------------------------------------------------
   if ((mSolverIterOption == SI_NONE) && (runstate == Gmat::SOLVING))
   {
      #if DBGLVL_OPENGL_UPDATE > 1
      MessageInterface::ShowMessage
         (wxT("   Just returning: SolverIterations is %d and runstate is %d\n"),
          mSolverIterOption, runstate);
      #endif
      
      return true;
   }
   
   //------------------------------------------------------------
   // update plot data
   //------------------------------------------------------------
   
   CoordinateConverter coordConverter;
   mNumData++;
   
   #if DBGLVL_OPENGL_UPDATE > 1
   MessageInterface::ShowMessage
      (wxT("   mNumData=%d, mDataCollectFrequency=%d, currentProvider=<%p>\n"),
       mNumData, mDataCollectFrequency, currentProvider);
   #endif
   
   if ((mNumData % mDataCollectFrequency) == 0)
   {
      mNumData = 0;
      mNumCollected++;
      bool update = (mNumCollected % mUpdatePlotFrequency) == 0;
      
      #if DBGLVL_OPENGL_UPDATE > 1
      MessageInterface::ShowMessage
         (wxT("   currentProvider=%d, theDataLabels.size()=%d\n"),
          currentProvider, theDataLabels.size());
      #endif
      
      #if DBGLVL_OPENGL_UPDATE > 2
      MessageInterface::ShowMessage
         (wxT("OpenGlPlot::Distribute() Using new Publisher code\n"));
      #endif
      
      // @note
      // New Publisher code doesn't assign currentProvider anymore,
      // it just copies current labels. There was an issue with
      // provider id keep incrementing if data is regisgered and
      // published inside a GmatFunction
      StringArray dataLabels = theDataLabels[0];
            
      #if DBGLVL_OPENGL_DATA_LABELS
      MessageInterface::ShowMessage(wxT("   Data labels for %s =\n   "), GetName().c_str());
      for (int j=0; j<(int)dataLabels.size(); j++)
         MessageInterface::ShowMessage(wxT("%s "), dataLabels[j].c_str());
      MessageInterface::ShowMessage(wxT("\n"));
      #endif
      
      Integer idX, idY, idZ;
      Integer idVx, idVy, idVz;
      Integer scIndex = -1;
      
      for (int i=0; i<mScCount; i++)
      {
         idX = FindIndexOfElement(dataLabels, mScNameArray[i]+wxT(".X"));
         idY = FindIndexOfElement(dataLabels, mScNameArray[i]+wxT(".Y"));
         idZ = FindIndexOfElement(dataLabels, mScNameArray[i]+wxT(".Z"));
         
         idVx = FindIndexOfElement(dataLabels, mScNameArray[i]+wxT(".Vx"));
         idVy = FindIndexOfElement(dataLabels, mScNameArray[i]+wxT(".Vy"));
         idVz = FindIndexOfElement(dataLabels, mScNameArray[i]+wxT(".Vz"));
         
         #if DBGLVL_OPENGL_DATA_LABELS
         MessageInterface::ShowMessage
            (wxT("   mScNameArray[%d]=%s, idX=%d, idY=%d, idZ=%d, idVx=%d, idVy=%d, idVz=%d\n"),
             i, mScNameArray[i].c_str(), idX, idY, idZ, idVx, idVy, idVz);
         #endif
         
         // if any of index not found, continue with next spacecraft name
         if (idX  == -1 || idY  == -1 || idZ  == -1 ||
             idVx == -1 || idVy == -1 || idVz == -1)
            continue;
         
         scIndex++;
         
         // buffer data
         for (int sc=0; sc<mScCount; sc++)
         {
            // If distributed data coordinate system is different from view
            // coordinate system, convert data here.
            // if we convert after current epoch, it will not give correct
            // results, if origin is spacecraft,
            // ie, sat->GetMJ2000State(epoch) will not give correct results.
            
            #if DBGLVL_OPENGL_DATA
            MessageInterface::ShowMessage
               (wxT("   %s, %.11f, X,Y,Z = %f, %f, %f\n"), GetName().c_str(), dat[0],
                dat[idX], dat[idY], dat[idZ]);
            #endif
            
            if ((theDataCoordSystem != NULL && mViewCoordSystem != NULL) &&
                (mViewCoordSystem != theDataCoordSystem))
            {
               Rvector6 inState, outState;
               
               // convert position and velocity
               inState.Set(dat[idX], dat[idY], dat[idZ],
                           dat[idVx], dat[idVy], dat[idVz]);
               
               coordConverter.Convert(dat[0], inState, theDataCoordSystem,
                                      outState, mViewCoordSystem);
               
               mScXArray[scIndex] = outState[0];
               mScYArray[scIndex] = outState[1];
               mScZArray[scIndex] = outState[2];
               mScVxArray[scIndex] = outState[3];
               mScVyArray[scIndex] = outState[4];
               mScVzArray[scIndex] = outState[5];
            }
            else
            {
               mScXArray[scIndex] = dat[idX];
               mScYArray[scIndex] = dat[idY];
               mScZArray[scIndex] = dat[idZ];
               mScVxArray[scIndex] = dat[idVx];
               mScVyArray[scIndex] = dat[idVy];
               mScVzArray[scIndex] = dat[idVz];
            }
            
            #if DBGLVL_OPENGL_DATA
            MessageInterface::ShowMessage
               (wxT("   after buffering, scNo=%d, scIndex=%d, X,Y,Z = %f, %f, %f\n"),
                i, scIndex, mScXArray[scIndex], mScYArray[scIndex], mScZArray[scIndex]);
            #endif
            
            #if DBGLVL_OPENGL_DATA > 1
            MessageInterface::ShowMessage
               (wxT("   Vx,Vy,Vz = %f, %f, %f\n"),
                mScVxArray[scIndex], mScVyArray[scIndex], mScVzArray[scIndex]);
            #endif
         }
      }
      
      // if only showing current iteration, buffer data and return
      if (mSolverIterOption == SI_CURRENT)
      {
         // save data when targeting or last iteration
         if (runstate == Gmat::SOLVING || runstate == Gmat::SOLVEDPASS)
         {
            mCurrScArray.push_back(mScNameArray);
            mCurrEpochArray.push_back(dat[0]);
            mCurrXArray.push_back(mScXArray);
            mCurrYArray.push_back(mScYArray);
            mCurrZArray.push_back(mScZArray);
            mCurrVxArray.push_back(mScVxArray);
            mCurrVyArray.push_back(mScVyArray);
            mCurrVzArray.push_back(mScVzArray);
         }
         
         if (runstate == Gmat::SOLVING)
         {
            //MessageInterface::ShowMessage
            //   (wxT("=====> num buffered = %d\n"), mCurrEpochArray.size());
            return true;
         }
      }
      
      
      #if DBGLVL_OPENGL_UPDATE > 0
      MessageInterface::ShowMessage(wxT("==========> now update GL plot\n"));
      #endif
      
      bool solving = false;
      UnsignedIntArray colorArray = mScOrbitColorArray;
      if (runstate == Gmat::SOLVING)
      {
         solving = true;
         colorArray = mScTargetColorArray;
      }
      
      PlotInterface::
         UpdateGlPlot(instanceName, mOldName, mScNameArray,
                      dat[0], mScXArray, mScYArray, mScZArray,
                      mScVxArray, mScVyArray, mScVzArray,
                      colorArray, solving, mSolverIterOption, update);
      
      if (update)
         mNumCollected = 0;
   }
   
   //loj: always return true otherwise next subscriber will not call ReceiveData()
   //     in Publisher::Publish()
   return true;
}

