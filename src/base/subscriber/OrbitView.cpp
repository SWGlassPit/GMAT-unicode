//$Id: OrbitView.cpp 9914 2011-09-26 19:07:00Z lindajun $
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
 * Implements OrbitView class.
 */
//------------------------------------------------------------------------------

#include "OrbitView.hpp"
#include "PlotInterface.hpp"       // for UpdateGlPlot()
#include "ColorTypes.hpp"          // for namespace GmatColor::
#include "SubscriberException.hpp" // for SubscriberException()
#include "MessageInterface.hpp"    // for ShowMessage()
#include "TextParser.hpp"          // for SeparateBrackets()
#include "StringUtil.hpp"          // for ToReal()
#include "CoordinateConverter.hpp" // for Convert()
#include <algorithm>               // for find(), distance()

//#define DBGLVL_INIT 1
//#define DBGLVL_DATA 1
//#define DBGLVL_DATA_LABELS 1
//#define DBGLVL_ADD 1
//#define DBGLVL_OBJ 2
//#define DBGLVL_PARAM 2
//#define DBGLVL_PARAM_STRING 2
//#define DBGLVL_PARAM_RVEC3 1
//#define DBGLVL_UPDATE 2
//#define DBGLVL_TAKE_ACTION 1
//#define DBGLVL_REMOVE_SP 1
//#define DBGLVL_RENAME 1
//#define DBGLVL_SOLVER_CURRENT_ITER 2

//---------------------------------
// static data
//---------------------------------
const wxString
OrbitView::PARAMETER_TEXT[OrbitViewParamCount - OrbitPlotParamCount] =
{
//    wxT("CoordinateSystem"),
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
   wxT("StarCount"),
   wxT("EnableStars"),
   wxT("EnableConstellations"),
   wxT("MinFOV"),
   wxT("MaxFOV"),
   wxT("InitialFOV"),
}; 


const Gmat::ParameterType
OrbitView::PARAMETER_TYPE[OrbitViewParamCount - OrbitPlotParamCount] =
{
//    Gmat::OBJECT_TYPE,            //wxT("CoordinateSystem")
   Gmat::OBJECT_TYPE,            //wxT("ViewPointRef"),
   Gmat::OBJECT_TYPE,            //wxT("ViewPointReference"),
   Gmat::STRING_TYPE,            //wxT("ViewPointRefType")
   Gmat::RVECTOR_TYPE,           //wxT("ViewPointRefVector"),
   Gmat::OBJECT_TYPE,            //wxT("ViewPointVector"),
   Gmat::STRING_TYPE,            //wxT("ViewPointVectorType"),
   Gmat::RVECTOR_TYPE,           //wxT("ViewPointVectorVector"),
   Gmat::OBJECT_TYPE,            //wxT("ViewDirection"),
   Gmat::STRING_TYPE,            //wxT("ViewDirectionType"),
   Gmat::RVECTOR_TYPE,           //wxT("ViewDirectionVector"),
   Gmat::REAL_TYPE,              //wxT("ViewScaleFactor"),
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
   
   Gmat::INTEGER_TYPE,           //wxT("StarCount")
   Gmat::ON_OFF_TYPE,            //wxT("EnableStars")
   Gmat::ON_OFF_TYPE,            //wxT("EnableConstellations")
   
   Gmat::INTEGER_TYPE,           //wxT("MinFOV")
   Gmat::INTEGER_TYPE,           //wxT("MaxFOV")
   Gmat::INTEGER_TYPE,           //wxT("InitialFOV")
};


//------------------------------------------------------------------------------
// OrbitView(const wxString &name)
//------------------------------------------------------------------------------
/**
 * The default constructor
 */
//------------------------------------------------------------------------------
OrbitView::OrbitView(const wxString &name)
   : OrbitPlot(wxT("OrbitView"), name)
{
   // GmatBase data
   parameterCount = OrbitViewParamCount;
   objectTypes.push_back(Gmat::ORBIT_VIEW);
   objectTypeNames.push_back(wxT("OrbitView"));
   
   mEclipticPlane = wxT("Off");
   mXYPlane = wxT("On");
   mWireFrame = wxT("Off");
   mAxes = wxT("On");
   mGrid = wxT("Off");
   mSunLine = wxT("Off");
   mOverlapPlot = wxT("Off");
   mUseInitialView = wxT("On");
   
   // stars
   mEnableStars = wxT("On");
   mEnableConstellations = wxT("On");
   mStarCount = 7000;
   
   // FOV - currentrly not used and will be removed later
   mMinFOV = 0;
   mMaxFOV = 90;
   mInitialFOV = 45;
   
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
   mViewPointRefVector.Set(0.0, 0.0, 0.0);
   mViewPointVecVector.Set(0.0, 0.0, 30000.0);
   mViewDirectionVector.Set(0.0, 0.0, -1.0);
   
   mViewUpCoordSystem = NULL;
   mViewCoordSysOrigin = NULL;
   mViewUpCoordSysOrigin = NULL;
   mViewPointRefObj = NULL;
   mViewPointObj = NULL;
   mViewDirectionObj = NULL;
}


//------------------------------------------------------------------------------
// OrbitView(const OrbitView &ov)
//------------------------------------------------------------------------------
/**
 * The copy consturctor
 */
//------------------------------------------------------------------------------
OrbitView::OrbitView(const OrbitView &ov)
   : OrbitPlot(ov)
{
   mEclipticPlane = ov.mEclipticPlane;
   mXYPlane = ov.mXYPlane;
   mWireFrame = ov.mWireFrame;
   mAxes = ov.mAxes;
   mGrid = ov.mGrid;
   mSunLine = ov.mSunLine;
   mOverlapPlot = ov.mOverlapPlot;
   mUseInitialView = ov.mUseInitialView;

   // stars
   mEnableStars = ov.mEnableStars;
   mEnableConstellations = ov.mEnableConstellations;
   mStarCount = ov.mStarCount;
      
//    mViewCoordSysName = ov.mViewCoordSysName;
   
   // viewpoint
   mViewPointRefName = ov.mViewPointRefName;
   mViewPointRefType = ov.mViewPointRefType;
   mViewPointVecName = ov.mViewPointVecName;
   mViewPointVecType = ov.mViewPointVecType;
   mViewDirectionName = ov.mViewDirectionName;
   mViewDirectionType = ov.mViewDirectionType;
   mViewScaleFactor = ov.mViewScaleFactor;
   mViewPointRefVector = ov.mViewPointRefVector;
   mViewPointVecVector = ov.mViewPointVecVector;
   mViewDirectionVector = ov.mViewDirectionVector;
   mViewUpCoordSysName = ov.mViewUpCoordSysName;
   mViewUpAxisName = ov.mViewUpAxisName;
   
//    mViewCoordSystem = ov.mViewCoordSystem;
   mViewUpCoordSystem = ov.mViewCoordSystem;
   mViewCoordSysOrigin = ov.mViewCoordSysOrigin;
   mViewUpCoordSysOrigin = ov.mViewUpCoordSysOrigin;
   mViewPointRefObj = ov.mViewPointRefObj;
   mViewPointObj = ov.mViewPointObj;
   mViewDirectionObj = ov.mViewDirectionObj;
}


//------------------------------------------------------------------------------
// OrbitView& operator=(const OrbitView&)
//------------------------------------------------------------------------------
/**
 * The assignment operator
 */
//------------------------------------------------------------------------------
OrbitView& OrbitView::operator=(const OrbitView& ov)
{
   if (this == &ov)
      return *this;
   
   OrbitPlot::operator=(ov);
   
   mEclipticPlane = ov.mEclipticPlane;
   mXYPlane = ov.mXYPlane;
   mWireFrame = ov.mWireFrame;
   mAxes = ov.mAxes;
   mGrid = ov.mGrid;
   mSunLine = ov.mSunLine;
   mOverlapPlot = ov.mOverlapPlot;
   mUseInitialView = ov.mUseInitialView;
   
   // stars
   mEnableStars = ov.mEnableStars;
   mEnableConstellations = ov.mEnableConstellations;
   mStarCount = ov.mStarCount;
   
   // View coordinate system name
//    mViewCoordSysName = ov.mViewCoordSysName;
   
   // viewpoint
   mViewPointRefName = ov.mViewPointRefName;
   mViewPointRefType = ov.mViewPointRefType;
   mViewPointVecName = ov.mViewPointVecName;
   mViewPointVecType = ov.mViewPointVecType;
   mViewDirectionName = ov.mViewDirectionName;
   mViewDirectionType = ov.mViewDirectionType;
   mViewScaleFactor = ov.mViewScaleFactor;
   mViewPointRefVector = ov.mViewPointRefVector;
   mViewPointVecVector = ov.mViewPointVecVector;
   mViewDirectionVector = ov.mViewDirectionVector;
   mViewUpCoordSysName = ov.mViewUpCoordSysName;
   mViewUpAxisName = ov.mViewUpAxisName;
   
   // object pointers
//    mViewCoordSystem = ov.mViewCoordSystem;
   mViewUpCoordSystem = ov.mViewCoordSystem;
   mViewCoordSysOrigin = ov.mViewCoordSysOrigin;
   mViewUpCoordSysOrigin = ov.mViewUpCoordSysOrigin;
   mViewPointRefObj = ov.mViewPointRefObj;
   mViewPointObj = ov.mViewPointObj;
   mViewDirectionObj = ov.mViewDirectionObj;
   
   return *this;
}


//------------------------------------------------------------------------------
// ~OrbitView()
//------------------------------------------------------------------------------
/**
 * Destructor
 *
 * @note This destructor does not delete 3DView window, but clears data.
 *       3DView window is deleted when it is closed by the user or GMAT
 *       shuts down.
 */
//------------------------------------------------------------------------------
OrbitView::~OrbitView()
{
   PlotInterface::TakeGlAction(instanceName, wxT("ClearObjects"));
}


//------------------------------------------------------------------------------
// Rvector3 GetVector(const wxString &which)
//------------------------------------------------------------------------------
Rvector3 OrbitView::GetVector(const wxString &which)
{
   if (which == wxT("ViewPointReference"))
      return mViewPointRefVector;
   else if (which == wxT("ViewPointVector"))
      return mViewPointVecVector;
   else if (which == wxT("ViewDirection"))
      return mViewDirectionVector;
   else
      throw SubscriberException(which + wxT(" is unknown OrbitView parameter\n"));
}


//------------------------------------------------------------------------------
// void SetVector(const wxString &which, const Rvector3 &value)
//------------------------------------------------------------------------------
void OrbitView::SetVector(const wxString &which, const Rvector3 &value)
{
   #if DBGLVL_SET
   MessageInterface::ShowMessage
      (wxT("OrbitView::SetVector() which=%s, value=%s\n"), which.c_str(),
       value.ToString().c_str());
   #endif
   
   if (which == wxT("ViewPointReference"))
      mViewPointRefVector = value;
   else if (which == wxT("ViewPointVector"))
      mViewPointVecVector = value;
   else if (which == wxT("ViewDirection"))
      mViewDirectionVector = value;
   else
      throw SubscriberException(which + wxT(" is unknown OrbitView parameter\n"));
}


//----------------------------------
// inherited methods from Subscriber
//----------------------------------


//---------------------------------
// inherited methods from GmatBase
//---------------------------------

//------------------------------------------------------------------------------
//  bool Validate()
//------------------------------------------------------------------------------
/**
 * Performs any pre-run validation that the object needs.
 *
 * @return true unless validation fails.
 */
//------------------------------------------------------------------------------
bool OrbitView::Validate()
{
   // Anything to validate here?
   
   return OrbitPlot::Validate();
}


//------------------------------------------------------------------------------
// virtual bool Initialize()
//------------------------------------------------------------------------------
bool OrbitView::Initialize()
{
   if (GmatGlobal::Instance()->GetRunMode() == GmatGlobal::TESTING_NO_PLOTS)
      return true;
   
   OrbitPlot::Initialize();
   
   // theInternalCoordSys is used only by 3DView so check. (2008.06.16)
   if (theInternalCoordSystem == NULL)
   {
      active = false;
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("*** WARNING *** The 3DView named \"%s\" will be turned off. ")
          wxT("It has a NULL internal coordinate system pointer.\n"), GetName().c_str());
      return false;
   }
   
   #if DBGLVL_INIT
   MessageInterface::ShowMessage
      (wxT("OrbitView::Initialize() this=<%p>'%s', active=%d, isInitialized=%d, ")
       wxT("isEndOfReceive=%d, mAllSpCount=%d\n"), this, GetName().c_str(), active,
       isInitialized, isEndOfReceive, mAllSpCount);
   #endif
   
   bool retval = false;
   
   //--------------------------------------------------------
   // start initializing for 3DView
   //--------------------------------------------------------
   if (active && !isInitialized)
   {
      #if DBGLVL_INIT
      MessageInterface::ShowMessage
         (wxT("OrbitView::Initialize() CreateGlPlotWindow() theSolarSystem=%p\n"),
          theSolarSystem);
      #endif
      
      // set ViewType
      PlotInterface::SetViewType(GmatPlot::ENHANCED_3D_VIEW);
      
      if (PlotInterface::CreateGlPlotWindow
          (instanceName, mOldName, mPlotUpperLeft[0], mPlotUpperLeft[1],
           mPlotSize[0], mPlotSize[1], mNumPointsToRedraw))
      {
         #if DBGLVL_INIT
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
         BuildDynamicArrays();
         
         
         // Should these moved to OrbitPlot? (Yes, for now)
         //mScCount = mScNameArray.size();
         //mObjectCount = mObjectNameArray.size();
         
         // check ViewPoint info to see if any objects need to be
         // included in the non-spacecraft list
         if (mViewCoordSystem == NULL)
            throw SubscriberException
               (wxT("OrbitView::Initialize() CoordinateSystem: ") + mViewCoordSysName +
                wxT(" not set\n"));
         
         if (mViewUpCoordSystem == NULL)
            throw SubscriberException
               (wxT("OrbitView::Initialize() CoordinateSystem: ") + mViewUpCoordSysName +
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
         
         // Add Sun to list if it was not added already to enable light source (LOJ: 2010.11.22)
         if (find(mObjectNameArray.begin(), mObjectNameArray.end(), wxT("Sun")) ==
             mObjectNameArray.end())
            UpdateObjectList(theSolarSystem->GetBody(wxT("Sun")), false);
         
         #if DBGLVL_INIT > 1
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
            show = mDrawObjectArray[i] ? true : false;
            MessageInterface::ShowMessage
               (wxT("   mObjectNameArray[%d]=%s, draw=%d, show=%d, color=%d\n"),
                i, mObjectNameArray[i].c_str(), draw, show, mOrbitColorArray[i]);
         }
         #endif
         
         #if DBGLVL_INIT
         MessageInterface::ShowMessage
            (wxT("   calling PlotInterface::SetGlSolarSystem(%p)\n"), theSolarSystem);
         #endif
         
         // set SolarSystem
         PlotInterface::SetGlSolarSystem(instanceName, theSolarSystem);
         
         #if DBGLVL_INIT
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
         #if DBGLVL_INIT
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
         // set drawing options
         //--------------------------------------------------------
         #if DBGLVL_INIT
         MessageInterface::ShowMessage
            (wxT("   calling PlotInterface::SetGlDrawingOption()\n"));
         #endif
         
         PlotInterface::SetGl3dDrawingOption
            (instanceName, (mEclipticPlane == wxT("On")), (mXYPlane == wxT("On")),
             (mWireFrame == wxT("On")), (mAxes == wxT("On")), (mGrid == wxT("On")),
             (mSunLine == wxT("On")), (mOverlapPlot == wxT("On")), (mUseInitialView == wxT("On")),
             (mEnableStars == wxT("On")), (mEnableConstellations == wxT("On")), mStarCount);
         
         //--------------------------------------------------------
         // set viewpoint info
         //--------------------------------------------------------
         #if DBGLVL_INIT
         MessageInterface::ShowMessage
            (wxT("   calling PlotInterface::SetGlViewOption()\n"));
         #endif
         
         PlotInterface::SetGl3dViewOption
            (instanceName, mViewPointRefObj, mViewPointObj,
             mViewDirectionObj, mViewScaleFactor, mViewPointRefVector,
             mViewPointVecVector, mViewDirectionVector, mViewUpAxisName,
             (mViewPointRefType == wxT("Vector")), (mViewPointVecType == wxT("Vector")),
             (mViewDirectionType == wxT("Vector")));
         
         PlotInterface::SetGlUpdateFrequency(instanceName, mUpdatePlotFrequency);
         
         //--------------------------------------------------------
         // set drawing object flag
         //--------------------------------------------------------
         PlotInterface::SetGlDrawOrbitFlag(instanceName, mDrawOrbitArray);
         PlotInterface::SetGlShowObjectFlag(instanceName, mDrawObjectArray);
         
         //--------------------------------------------------------
         // initialize GL
         //--------------------------------------------------------
         PlotInterface::InitializeGlPlot(instanceName);
         
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
      #if DBGLVL_INIT
      MessageInterface::ShowMessage
         (wxT("OrbitView::Initialize() Plot is active and initialized, ")
          wxT("so calling DeleteGlPlot()\n"));
      #endif
      
      // Why do we want to delete plot if active and initialized?
      // This causes Global 3DView not to show, so commented out (loj: 2008.10.08)
      // We still need to delete non-active plots so that plot persistency works,
      // so uncommented (loj: 2011.09.23)
      if (!active)
         retval =  PlotInterface::DeleteGlPlot(instanceName);
   }
   
   #if DBGLVL_INIT
   MessageInterface::ShowMessage(wxT("OrbitView::Initialize() exiting\n"));
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the OrbitView.
 *
 * @return clone of the OrbitView.
 *
 */
//------------------------------------------------------------------------------
GmatBase* OrbitView::Clone() const
{
   return (new OrbitView(*this));
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
void OrbitView::Copy(const GmatBase* orig)
{
   operator=(*((OrbitView *)(orig)));
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
bool OrbitView::TakeAction(const wxString &action,
                           const wxString &actionData)
{
   return OrbitPlot::TakeAction(action, actionData);
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
bool OrbitView::RenameRefObject(const Gmat::ObjectType type,
                                const wxString &oldName,
                                const wxString &newName)
{
   #if DBGLVL_RENAME
   MessageInterface::ShowMessage
      (wxT("OrbitView::RenameRefObject() type=%s, oldName=%s, newName=%s\n"),
       GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif
   
   if (type != Gmat::SPACECRAFT && type != Gmat::COORDINATE_SYSTEM)
      return true;
   
   if (type == Gmat::SPACECRAFT)
   {
      return OrbitPlot::RenameRefObject(type, oldName, newName);
   }
   else if (type == Gmat::COORDINATE_SYSTEM)
   {
//       if (mViewCoordSysName == oldName)
//          mViewCoordSysName = newName;

      if (mViewUpCoordSysName == oldName)
         mViewUpCoordSysName = newName;
      
      return OrbitPlot::RenameRefObject(type, oldName, newName);
      
   }
   
   return true;
}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
wxString OrbitView::GetParameterText(const Integer id) const
{
   if (id >= OrbitPlotParamCount && id < OrbitViewParamCount)
      return PARAMETER_TEXT[id - OrbitPlotParamCount];
   else
      return OrbitPlot::GetParameterText(id);
    
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
Integer OrbitView::GetParameterID(const wxString &str) const
{
   if (str == wxT("PerspectiveMode") || str == wxT("UseFixedFov") || str == wxT("FixedFovAngle") ||
       str == wxT("MinFOV") || str == wxT("MaxFOV") || str == wxT("InitialFOV"))
      return Gmat::PARAMETER_REMOVED;
   
   for (int i=OrbitPlotParamCount; i<OrbitViewParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - OrbitPlotParamCount])
         return i;
   }
   
   return OrbitPlot::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
Gmat::ParameterType OrbitView::GetParameterType(const Integer id) const
{
   if (id >= OrbitPlotParamCount && id < OrbitViewParamCount)
      return PARAMETER_TYPE[id - OrbitPlotParamCount];
   else
      return OrbitPlot::GetParameterType(id);
}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
wxString OrbitView::GetParameterTypeString(const Integer id) const
{
   return GmatBase::PARAM_TYPE_STRING[GetParameterType(id)];
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
bool OrbitView::IsParameterReadOnly(const Integer id) const
{
   if (id == OVERLAP_PLOT ||
       id == EARTH_SUN_LINES || id == VIEWPOINT_REF || id == VIEWPOINT_REF_VECTOR ||
       id == VIEWPOINT_VECTOR_VECTOR || id == VIEW_DIRECTION_VECTOR ||
       id == VIEWPOINT_REF_TYPE || id == VIEWPOINT_VECTOR_TYPE ||
       id == MIN_FOV || id == MAX_FOV || id == INITIAL_FOV ||
       id == VIEW_DIRECTION_TYPE)
      return true;
   
   return OrbitPlot::IsParameterReadOnly(id);
}


//------------------------------------------------------------------------------
// virtual Integer GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
Integer OrbitView::GetIntegerParameter(const Integer id) const
{
   switch (id)
   {
   case STAR_COUNT:
      return mStarCount;
   case MIN_FOV:
      return mMinFOV;
   case MAX_FOV:
      return mMaxFOV;
   case INITIAL_FOV:
      return mInitialFOV;
   default:
      return OrbitPlot::GetIntegerParameter(id);
   }
}


//------------------------------------------------------------------------------
// virtual Integer GetIntegerParameter(const wxString &label) const
//------------------------------------------------------------------------------
Integer OrbitView::GetIntegerParameter(const wxString &label) const
{
   return GetIntegerParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual Integer SetIntegerParameter(const Integer id, const Integer value)
//------------------------------------------------------------------------------
Integer OrbitView::SetIntegerParameter(const Integer id, const Integer value)
{
   switch (id)
   {
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
      return OrbitPlot::SetIntegerParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// virtual Integer SetIntegerParameter(const wxString &label,
//                                     const Integer value)
//------------------------------------------------------------------------------
Integer OrbitView::SetIntegerParameter(const wxString &label,
                                       const Integer value)
{
   return SetIntegerParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// virtual Real GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
Real OrbitView::GetRealParameter(const Integer id) const
{
   switch (id)
   {
   case VIEW_SCALE_FACTOR:
      return mViewScaleFactor;
   default:
      return OrbitPlot::GetRealParameter(id);
   }
}


//------------------------------------------------------------------------------
// virtual Real GetRealParameter(const wxString &label) const
//------------------------------------------------------------------------------
Real OrbitView::GetRealParameter(const wxString &label) const
{
   #if DBGLVL_PARAM
     MessageInterface::ShowMessage
        (wxT("OrbitView::GetRealParameter() label = %s\n"), label.c_str());
   #endif
   
   return GetRealParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual Real SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
Real OrbitView::SetRealParameter(const Integer id, const Real value)
{
   switch (id)
   {
   case VIEW_SCALE_FACTOR:
      mViewScaleFactor = value;
      return value;
   default:
      return OrbitPlot::SetRealParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// virtual Real SetRealParameter(const wxString &label, const Real value)
//------------------------------------------------------------------------------
Real OrbitView::SetRealParameter(const wxString &label, const Real value)
{
   #if DBGLVL_PARAM
      MessageInterface::ShowMessage
         (wxT("OrbitView::SetRealParameter() label = %s, value = %f \n"),
          label.c_str(), value);
   #endif
   
   return SetRealParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
Real OrbitView::GetRealParameter(const Integer id, const Integer index) const
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
      return OrbitPlot::GetRealParameter(id, index);
   }
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value, const Integer index)
//------------------------------------------------------------------------------
Real OrbitView::SetRealParameter(const Integer id, const Real value,
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
      return OrbitPlot::SetRealParameter(id, value, index);
   }
}


//------------------------------------------------------------------------------
// virtual const Rvector& GetRvectorParameter(const Integer id) const
//------------------------------------------------------------------------------
const Rvector& OrbitView::GetRvectorParameter(const Integer id) const
{
   switch (id)
   {
   case VIEWPOINT_REF_VECTOR:
      //WriteDeprecatedMessage(id);
      return mViewPointRefVector;
   case VIEWPOINT_VECTOR_VECTOR:
      {
         //WriteDeprecatedMessage(id);
         #if DBGLVL_PARAM
         Rvector vec = mViewPointVecVector;
         MessageInterface::ShowMessage
            (wxT("OrbitView::GetRvectorParameter() returning = %s\n"),
             vec.ToString().c_str());
         #endif
         return mViewPointVecVector;
      }
   case VIEW_DIRECTION_VECTOR:
      //WriteDeprecatedMessage(id);
      return mViewDirectionVector;
   default:
      return OrbitPlot::GetRvectorParameter(id);
   }
}


//------------------------------------------------------------------------------
// virtual const Rvector& GetRvectorParameter(const wxString &label) const
//------------------------------------------------------------------------------
const Rvector& OrbitView::GetRvectorParameter(const wxString &label) const
{
   #if DBGLVL_PARAM
   MessageInterface::ShowMessage
      (wxT("OrbitView::GetRvectorParameter() label = %s\n"), label.c_str());
   #endif
   
   return GetRvectorParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual const Rvector& SetRvectorParameter(const Integer id,
//                                            const Rvector &value)
//------------------------------------------------------------------------------
const Rvector& OrbitView::SetRvectorParameter(const Integer id,
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
      return OrbitPlot::SetRvectorParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// virtual const Rvector& SetRvectorParameter(const wxString &label,
//                                            const Rvector &value)
//------------------------------------------------------------------------------
const Rvector& OrbitView::SetRvectorParameter(const wxString &label,
                                              const Rvector &value)
{
   #if DBGLVL_PARAM
   Rvector val = value;
   MessageInterface::ShowMessage
      (wxT("OrbitView::SetRvectorParameter() label = %s, ")
       wxT("value = %s \n"), label.c_str(), val.ToString().c_str());
   #endif
   
   return SetRvectorParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
wxString OrbitView::GetStringParameter(const Integer id) const
{
   #if DBGLVL_PARAM_STRING
   MessageInterface::ShowMessage
      (wxT("OrbitView::GetStringParameter()<%s> id=%d<%s>\n"),
       instanceName.c_str(), id, GetParameterText(id).c_str());
   #endif
   
   switch (id)
   {
//    case COORD_SYSTEM:
//       return mViewCoordSysName;
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
      return OrbitPlot::GetStringParameter(id);
   }
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString OrbitView::GetStringParameter(const wxString &label) const
{
   #if DBGLVL_PARAM_STRING
   MessageInterface::ShowMessage
      (wxT("OrbitView::GetStringParameter() label = %s\n"), label.c_str());
   #endif
   
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
bool OrbitView::SetStringParameter(const Integer id, const wxString &value)
{
   #if DBGLVL_PARAM_STRING
   MessageInterface::ShowMessage
      (wxT("OrbitView::SetStringParameter() this=<%p>'%s', id=%d<%s>, value='%s'\n"),
       this, instanceName.c_str(), id, GetParameterText(id).c_str(), value.c_str());
   #endif
   
   switch (id)
   {
//    case COORD_SYSTEM:
//       mViewCoordSysName = value;
//       return true;
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
      return OrbitPlot::SetStringParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
bool OrbitView::SetStringParameter(const wxString &label,
                                   const wxString &value)
{
   #if DBGLVL_PARAM_STRING
   MessageInterface::ShowMessage
      (wxT("OrbitView::SetStringParameter()<%s> label=%s, value=%s \n"),
       instanceName.c_str(), label.c_str(), value.c_str());
   #endif
   
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// virtual bool SetStringParameter(const Integer id, const wxString &value,
//                                 const Integer index)
//------------------------------------------------------------------------------
bool OrbitView::SetStringParameter(const Integer id, const wxString &value,
                                   const Integer index)
{
   #if DBGLVL_PARAM_STRING
   MessageInterface::ShowMessage
      (wxT("OrbitView::SetStringParameter()<%s> id=%d<%s>, value=%s, index= %d\n"),
       instanceName.c_str(), id, GetParameterText(id).c_str(), value.c_str(), index);
   #endif
   
   switch (id)
   {
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
      return OrbitPlot::SetStringParameter(id, value, index);
   }
}


//------------------------------------------------------------------------------
// virtual bool SetStringParameter(const wxString &label,
//                                 const wxString &value,
//                                 const Integer index)
//------------------------------------------------------------------------------
bool OrbitView::SetStringParameter(const wxString &label,
                                   const wxString &value,
                                   const Integer index)
{
   #if DBGLVL_PARAM_STRING
   MessageInterface::ShowMessage
      (wxT("OrbitView::SetStringParameter() label = %s, value = %s, index = %d\n"),
       label.c_str(), value.c_str(), index);
   #endif
   
   return SetStringParameter(GetParameterID(label), value, index);
}


//---------------------------------------------------------------------------
//  wxString GetOnOffParameter(const Integer id) const
//---------------------------------------------------------------------------
wxString OrbitView::GetOnOffParameter(const Integer id) const
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
   case ENABLE_STARS:
      return mEnableStars;
   case ENABLE_CONSTELLATIONS:
      return mEnableConstellations;
   default:
      return OrbitPlot::GetOnOffParameter(id);
   }
}


//------------------------------------------------------------------------------
// wxString OrbitView::GetOnOffParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString OrbitView::GetOnOffParameter(const wxString &label) const
{
   return GetOnOffParameter(GetParameterID(label));
}


//---------------------------------------------------------------------------
//  bool SetOnOffParameter(const Integer id, const wxString &value)
//---------------------------------------------------------------------------
bool OrbitView::SetOnOffParameter(const Integer id, const wxString &value)
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
   case ENABLE_STARS:
      mEnableStars = value;
      return true;
   case ENABLE_CONSTELLATIONS:
      mEnableConstellations = value;
      return true;
   default:
      return OrbitPlot::SetOnOffParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetOnOffParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
bool OrbitView::SetOnOffParameter(const wxString &label, 
                                  const wxString &value)
{
   return SetOnOffParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// virtual wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
wxString OrbitView::GetRefObjectName(const Gmat::ObjectType type) const
{
   #if DBGLVL_OBJ
   MessageInterface::ShowMessage
      (wxT("OrbitView::GetRefObjectName() type: %s\n"),
       GmatBase::GetObjectTypeString(type).c_str());
   #endif
   
//    if (type == Gmat::COORDINATE_SYSTEM)
//    {
//       return mViewCoordSysName; //just return this
//    }
   
   #if DBGLVL_OBJ
   wxString msg = wxT("type: ") + GmatBase::GetObjectTypeString(type) + wxT(" not found");
   MessageInterface::ShowMessage
      (wxT("OrbitView::GetRefObjectName() %s\n"), msg.c_str());
   #endif
   
   return OrbitPlot::GetRefObjectName(type);
}


//------------------------------------------------------------------------------
// virtual bool HasRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
bool OrbitView::HasRefObjectTypeArray()
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
const ObjectTypeArray& OrbitView::GetRefObjectTypeArray()
{
   //@note  Olny leaf class clears refObjectTypes
   refObjectTypes.clear();
   refObjectTypes = OrbitPlot::GetRefObjectTypeArray();
   refObjectTypes.push_back(Gmat::COORDINATE_SYSTEM);
   return refObjectTypes;
}


//------------------------------------------------------------------------------
// virtual const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
const StringArray& OrbitView::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   refObjectNames.clear();
   refObjectNames = OrbitPlot::GetRefObjectNameArray(type);
   
   // if Draw Earth-Sun lines is on, add Earth and Sun
   if (mSunLine == wxT("On"))
   {
      AddSpacePoint(wxT("Earth"), mAllSpCount, false);
      AddSpacePoint(wxT("Sun"), mAllSpCount, false);
   }
   
   if (type == Gmat::COORDINATE_SYSTEM)
   {
//       refObjectNames.push_back(mViewCoordSysName);
      refObjectNames.push_back(mViewUpCoordSysName);
   }
   else if (type == Gmat::SPACE_POINT)
   {
      //refObjectNames = mAllSpNameArray;
      
      if (mViewPointRefType != wxT("Vector"))
      {
         if (find(refObjectNames.begin(), refObjectNames.end(),
                  mViewPointRefName) == refObjectNames.end())
            refObjectNames.push_back(mViewPointRefName);
      }
      
      if (mViewPointVecType != wxT("Vector"))
      {
         if (find(refObjectNames.begin(), refObjectNames.end(),
                  mViewPointVecName) == refObjectNames.end())
            refObjectNames.push_back(mViewPointVecName);
      }
      
      if (mViewDirectionType != wxT("Vector"))
      {
         if (find(refObjectNames.begin(), refObjectNames.end(),
                  mViewDirectionName) == refObjectNames.end())
            refObjectNames.push_back(mViewDirectionName);
      }
   }
   else if (type == Gmat::UNKNOWN_OBJECT)
   {
      #ifdef DEBUG_OBJ
      MessageInterface::ShowMessage
         (wxT("mViewPointRefType=%s, mViewPointVecType=%s, mViewDirectionType=%s\n"),
          mViewPointRefType.c_str(), mViewPointVecType.c_str(), mViewDirectionType.c_str());
      #endif
      
//       refObjectNames = mAllSpNameArray;      
//       refObjectNames.push_back(mViewCoordSysName);
      
      refObjectNames.insert(refObjectNames.end(), mAllSpNameArray.begin(),
                            mAllSpNameArray.end());
      
      if (mViewCoordSysName != mViewUpCoordSysName)
         refObjectNames.push_back(mViewUpCoordSysName);
      
      if (mViewPointRefType != wxT("Vector"))
      {
         if (find(refObjectNames.begin(), refObjectNames.end(),
                  mViewPointRefName) == refObjectNames.end())
            refObjectNames.push_back(mViewPointRefName);
      }
      
      if (mViewPointVecType != wxT("Vector"))
      {
         if (find(refObjectNames.begin(), refObjectNames.end(),
                  mViewPointVecName) == refObjectNames.end())
            refObjectNames.push_back(mViewPointVecName);
      }
      
      if (mViewDirectionType != wxT("Vector"))
      {
         if (find(refObjectNames.begin(), refObjectNames.end(),
                  mViewDirectionName) == refObjectNames.end())
            refObjectNames.push_back(mViewDirectionName);
      }
   }
   
   #if DBGLVL_OBJ
   MessageInterface::ShowMessage
      (wxT("OrbitView::GetRefObjectNameArray() returning for type:%d\n"), type);
   for (unsigned int i=0; i<refObjectNames.size(); i++)
      MessageInterface::ShowMessage(wxT("   %s\n"), refObjectNames[i].c_str());
   #endif
   
   return refObjectNames;
}


//------------------------------------------------------------------------------
// virtual GmatBase* GetRefObject(const Gmat::ObjectType type,
//                                const wxString &name)
//------------------------------------------------------------------------------
GmatBase* OrbitView::GetRefObject(const Gmat::ObjectType type,
                                  const wxString &name)
{
   if (type == Gmat::COORDINATE_SYSTEM)
   {
//       if (name == mViewCoordSysName)
//          return mViewCoordSystem;
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
   
   return OrbitPlot::GetRefObject(type, name);
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
bool OrbitView::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                             const wxString &name)
{
   #if DBGLVL_OBJ
   MessageInterface::ShowMessage
      (wxT("OrbitView::SetRefObject() this=<%p>'%s', obj=<%p>'%s', type=%d[%s], name='%s'\n"),
       this, GetName().c_str(), obj, obj->GetName().c_str(), type,
       obj->GetTypeName().c_str(), name.c_str());
   #endif
   
   wxString realName = name;
   if (name == wxT(""))
      realName = obj->GetName();
   
   if (type == Gmat::COORDINATE_SYSTEM)
   {
//       if (realName == mViewCoordSysName)
//          mViewCoordSystem = (CoordinateSystem*)obj;
      if (realName == mViewUpCoordSysName)
         mViewUpCoordSystem = (CoordinateSystem*)obj;
      
//       return true;
   }
   
   
   if (obj->IsOfType(Gmat::SPACE_POINT))
   {
      #if DBGLVL_OBJ
      MessageInterface::ShowMessage(wxT("   Setting View information...\n"));
      #endif
      
      // ViewPoint info
      if (realName == mViewPointRefName)
         mViewPointRefObj = (SpacePoint*)obj;
      
      if (realName == mViewPointVecName)
         mViewPointObj = (SpacePoint*)obj;
      
      if (realName == mViewDirectionName)
         mViewDirectionObj = (SpacePoint*)obj;
      
      #if DBGLVL_OBJ
      MessageInterface::ShowMessage
         (wxT("OrbitView::SetRefObject() mViewPointRefObj=<%p>, mViewPointObj=<%p>, ")
          wxT("mViewDirectionObj=<%p>\n"), mViewPointRefObj, mViewPointObj,
          mViewDirectionObj);
      #endif
   }
   
   return OrbitPlot::SetRefObject(obj, type, realName);
}


//---------------------------------
// protected methods
//---------------------------------

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
void OrbitView::PutRvector3Value(Rvector3 &rvec3, Integer id,
                                  const wxString &sval, Integer index)
{
   #if DBGLVL_PARAM_RVEC3
   MessageInterface::ShowMessage
      (wxT("OrbitView::PutRvector3Value() id=%d, sval=%s, index=%d\n"),
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
// void WriteDeprecatedMessage(Integer id) const
//------------------------------------------------------------------------------
void OrbitView::WriteDeprecatedMessage(Integer id) const
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
bool OrbitView::UpdateSolverData()
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
                      mCurrVzArray[i], colorArray, true, mSolverIterOption,
                      false, isDataOn);
   }
   
   // Buffer last point and Update the plot
   PlotInterface::
      UpdateGlPlot(instanceName, mOldName, mCurrScArray[last],
                   mCurrEpochArray[last], mCurrXArray[last], mCurrYArray[last],
                   mCurrZArray[last], mCurrVxArray[last], mCurrVyArray[last],
                   mCurrVzArray[last], colorArray, true, mSolverIterOption,
                   true, isDataOn);
   
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
bool OrbitView::Distribute(int len)
{
   //loj: How do I convert data to Real data?
   return false;
}


//------------------------------------------------------------------------------
// bool Distribute(const Real *dat, Integer len)
//------------------------------------------------------------------------------
bool OrbitView::Distribute(const Real *dat, Integer len)
{
   #if DBGLVL_UPDATE
   MessageInterface::ShowMessage
      (wxT("===========================================================================\n")
       wxT("OrbitView::Distribute() instanceName=%s, active=%d, isEndOfRun=%d, ")
       wxT("isEndOfReceive=%d\n   mAllSpCount=%d, mScCount=%d, len=%d, runstate=%d, ")
       wxT("isDataStateChanged=%d\n"), instanceName.c_str(), active, isEndOfRun, isEndOfReceive,
       mAllSpCount, mScCount, len, runstate, isDataStateChanged);
   #endif
   
   if (GmatGlobal::Instance()->GetRunMode() == GmatGlobal::TESTING_NO_PLOTS)
      return true;
   
   // if data state changed from on to off or vice versa, update plot data so
   // data points can be flagged.
   if (isDataStateChanged)
   {
      if (isDataOn)
         PlotInterface::TakeGlAction(instanceName, wxT("PenDown"));
      else
         PlotInterface::TakeGlAction(instanceName, wxT("PenUp"));
      
      isDataStateChanged = false;
   }
   
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
   
   
   #if DBGLVL_DATA
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
      #if DBGLVL_UPDATE > 1
      MessageInterface::ShowMessage
         (wxT("   Just returning: SolverIterations is %d and runstate is %d\n"),
          mSolverIterOption, runstate);
      #endif
      
      return true;
   }
   
   //------------------------------------------------------------
   // update plot data
   //------------------------------------------------------------

   UpdateData(dat, len);

   
   //loj: always return true otherwise next subscriber will not call ReceiveData()
   //     in Publisher::Publish()
   return true;
}

