//$Id: XyPlot.cpp 9851 2011-09-09 18:49:52Z lindajun $
//------------------------------------------------------------------------------
//                                  XyPlot
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
// Created: 2004/01/22
//
/**
 * Implements XyPlot class.
 */
//------------------------------------------------------------------------------

#include "XyPlot.hpp"
#include "PlotInterface.hpp"     // for XY plot
#include "SubscriberException.hpp"
#include "MessageInterface.hpp"  // for ShowMessage()

//#define DEBUG_XYPLOT_INIT 1
//#define DEBUG_XYPLOT_PARAM 1
//#define DEBUG_XYPLOT_OBJECT 1 
//#define DEBUG_XYPLOT_UPDATE 2
//#define DEBUG_ACTION_REMOVE 1
//#define DEBUG_RENAME 1

//---------------------------------
// static data
//---------------------------------

const wxString
XyPlot::PARAMETER_TEXT[XyPlotParamCount - SubscriberParamCount] =
{
   wxT("XVariable"),
   wxT("YVariables"),
   wxT("PlotTitle"),
   wxT("XAxisTitle"),
   wxT("YAxisTitle"),
   wxT("ShowGrid"),
   wxT("DataCollectFrequency"),
   wxT("UpdatePlotFrequency"),
   wxT("ShowPlot"),
   wxT("UseLines"),
   wxT("LineWidth"),
   wxT("UseMarkers"),
   wxT("MarkerSize"),
   wxT("Drawing"),
   wxT("IndVar"),
   wxT("Add"),
   wxT("Grid"),
}; 

const Gmat::ParameterType
XyPlot::PARAMETER_TYPE[XyPlotParamCount - SubscriberParamCount] =
{
   Gmat::OBJECT_TYPE,      // wxT("IndVar"),wxT("XVariable")
   Gmat::OBJECTARRAY_TYPE, // wxT("Add"),wxT("YVariables")
   Gmat::STRING_TYPE,      // wxT("PlotTitle"),
   Gmat::STRING_TYPE,      // wxT("XAxisTitle"),
   Gmat::STRING_TYPE,      // wxT("YAxisTitle"),
   Gmat::BOOLEAN_TYPE,     // wxT("ShowGrid")
   Gmat::INTEGER_TYPE,     // wxT("DataCollectFrequency"),
   Gmat::INTEGER_TYPE,     // wxT("UpdatePlotFrequency"),
   Gmat::BOOLEAN_TYPE,     // wxT("ShowPlot"),
   Gmat::BOOLEAN_TYPE,     // wxT("UseLines"),
   Gmat::INTEGER_TYPE,     // wxT("LineWidth"),
   Gmat::BOOLEAN_TYPE,     // wxT("UseMarkers"),
   Gmat::INTEGER_TYPE,     // wxT("MarkerSize"),
   Gmat::BOOLEAN_TYPE,     // wxT("Drawing")
   Gmat::OBJECT_TYPE,      // wxT("IndVar"),wxT("XVariable")
   Gmat::OBJECTARRAY_TYPE, // wxT("Add"),wxT("YVariables")
   Gmat::ON_OFF_TYPE,      // wxT("Grid"),wxT("ShowGrid")
};

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// XyPlot(const wxString &name, Parameter *xParam,
//        Parameter *firstYParam, const wxString &plotTitle,
//        const wxString &xAxisTitle, const wxString &yAxisTitle,
//        bool drawGrid)
//------------------------------------------------------------------------------
XyPlot::XyPlot(const wxString &name, Parameter *xParam,
               Parameter *firstYParam, const wxString &plotTitle,
               const wxString &xAxisTitle, const wxString &yAxisTitle) :
   Subscriber(wxT("XYPlot"), name)
{
   // GmatBase data
   objectTypes.push_back(Gmat::XY_PLOT);
   objectTypeNames.push_back(wxT("XYPlot"));
   parameterCount = XyPlotParamCount;
   
   mDrawGrid = true;
   mNumYParams = 0;
   
   mXParamName = wxT("");
   mNumXParams = 0;
   
   mXParam = xParam;
   if (firstYParam != NULL)
      AddYParameter(firstYParam->GetName(), mNumYParams);
   
   mOldName = instanceName;
   mPlotTitle = plotTitle;
   mXAxisTitle = xAxisTitle;
   mYAxisTitle = yAxisTitle;
   
   mIsXyPlotWindowSet = false;
   mDataCollectFrequency = 1;
   mUpdatePlotFrequency = 10;
   mNumDataPoints = 0;           // Found by Bob Wiegand w/ Valgrind

   useLines = true;
   lineWidth = 1;
   useMarkers = false;
   markerSize = 3;
   drawing = true;
   breakCount = 0;
}


//------------------------------------------------------------------------------
// XyPlot(const XyPlot &orig)
//------------------------------------------------------------------------------
XyPlot::XyPlot(const XyPlot &orig) :
   Subscriber(orig)
{
   mXParam = orig.mXParam;
   mYParams = orig.mYParams;
   
   mNumXParams = orig.mNumXParams;
   mNumYParams = orig.mNumYParams;
   
   mXParamName = orig.mXParamName;
   mYParamNames = orig.mYParamNames;
   
   mAllParamNames = orig.mAllParamNames;
   
   mOldName = orig.mOldName;
   mPlotTitle = orig.mPlotTitle;
   mXAxisTitle = orig.mXAxisTitle;
   mYAxisTitle = orig.mYAxisTitle;
   mDrawGrid = orig.mDrawGrid;
   mIsXyPlotWindowSet = orig.mIsXyPlotWindowSet;
   
   mDataCollectFrequency = orig.mDataCollectFrequency;
   mUpdatePlotFrequency = orig.mUpdatePlotFrequency;
   
   mNumDataPoints = orig.mNumDataPoints;
   mNumCollected = orig.mNumCollected;

   useLines   = orig.useLines;
   lineWidth  = orig.lineWidth;
   useMarkers = orig.useMarkers;
   markerSize = orig.markerSize;
   drawing    = orig.drawing;
   breakCount = orig.breakCount;
}


//------------------------------------------------------------------------------
// XyPlot& operator=(const XyPlot& orig)
//------------------------------------------------------------------------------
/**
 * The assignment operator
 */
//------------------------------------------------------------------------------
XyPlot& XyPlot::operator=(const XyPlot& orig)
{
   if (this == &orig)
      return *this;
   
   Subscriber::operator=(orig);
   
   mXParam = orig.mXParam;
   mYParams = orig.mYParams;
   
   mNumXParams = orig.mNumXParams;
   mNumYParams = orig.mNumYParams;
   
   mXParamName = orig.mXParamName;
   mYParamNames = orig.mYParamNames;
   
   mAllParamNames = orig.mAllParamNames;
   
   mOldName = orig.mOldName;
   mPlotTitle = orig.mPlotTitle;
   mXAxisTitle = orig.mXAxisTitle;
   mYAxisTitle = orig.mYAxisTitle;
   mDrawGrid = orig.mDrawGrid;
   mIsXyPlotWindowSet = orig.mIsXyPlotWindowSet;
   
   mDataCollectFrequency = orig.mDataCollectFrequency;
   mUpdatePlotFrequency = orig.mUpdatePlotFrequency;
   
   mNumDataPoints = orig.mNumDataPoints;
   mNumCollected = orig.mNumCollected;
   
   useMarkers = orig.useMarkers;
   drawing = orig.drawing;
   breakCount = orig.breakCount;

   return *this;
}


//------------------------------------------------------------------------------
// ~XyPlot(void)
//------------------------------------------------------------------------------
XyPlot::~XyPlot()
{
}

//------------------------------------------------------------------------------
// bool SetXParameter(const wxString &paramName)
//------------------------------------------------------------------------------
bool XyPlot::SetXParameter(const wxString &paramName)
{
   if (paramName != wxT(""))
   {
      mXParamName = paramName;
      mNumXParams = 1; //loj: only 1 X parameter for now
      return true;
   }
   
   return false;
}


//------------------------------------------------------------------------------
// bool AddYParameter(const wxString &paramName, Integer index)
//------------------------------------------------------------------------------
bool XyPlot::AddYParameter(const wxString &paramName, Integer index)
{
   #if DEBUG_XYPLOT_PARAM
   MessageInterface::ShowMessage(wxT("XyPlot::AddYParameter() name = %s\n"),
                                 paramName.c_str());
   #endif
   
   if (paramName != wxT("") && index == mNumYParams)
   {
      // if paramName not found, add (based on loj fix to XYPlot)
      if (find(mYParamNames.begin(), mYParamNames.end(), paramName) ==
          mYParamNames.end())
      {
         mYParamNames.push_back(paramName);
         mNumYParams = mYParamNames.size();
         mYParams.push_back(NULL);
         return true;
      }
   }

   return false;
}

//----------------------------------
// methods inherited from Subscriber
//----------------------------------

//------------------------------------------------------------------------------
// virtual bool Initialize()
//------------------------------------------------------------------------------
bool XyPlot::Initialize()
{
   if (GmatGlobal::Instance()->GetRunMode() == GmatGlobal::TESTING_NO_PLOTS)
      return true;
   
   #if DEBUG_XYPLOT_INIT
   MessageInterface::ShowMessage
      (wxT("XyPlot::Initialize() active=%d, mNumYParams=%d\n"), active, mNumYParams);
   #endif
   
   // Check if there are parameters selected for XyPlot
   if (active)
   {
      if (mNumXParams == 0 || mNumYParams == 0)
      {
         active = false;
         MessageInterface::PopupMessage
            (Gmat::WARNING_,
             wxT("*** WARNING *** The XYPlot named \"%s\" will not be shown.\n")
             wxT("No parameters were selected for X Axis or Y Axis.\n"),
             GetName().c_str());
         return false;
      }
      
      if (mXParam == NULL || mYParams[0] == NULL)
      {
         active = false;
         MessageInterface::PopupMessage
            (Gmat::WARNING_,
             wxT("*** WARNING *** The XYPlot named \"%s\" will not be shown.\n")
             wxT("The first parameter selected for X Axis or Y Axis is NULL\n"),
             GetName().c_str());
         return false;
      }
   }
   
   Subscriber::Initialize();
   
   bool status = false;
   DeletePlotCurves();
   
   if (active)
   {
      // build plot title
      BuildPlotTitle();
      
      // Create XyPlotWindow, if not exist
      #if DEBUG_XYPLOT_INIT
      MessageInterface::ShowMessage
         (wxT("XyPlot::Initialize() calling CreateXyPlotWindow()\n"));
      #endif
      
      PlotInterface::CreateXyPlotWindow(instanceName, mOldName, mPlotUpperLeft[0], mPlotUpperLeft[1],
                                        mPlotSize[0], mPlotSize[1], mPlotTitle,
                                        mXAxisTitle, mYAxisTitle, mDrawGrid);
      
      PlotInterface::SetXyPlotTitle(instanceName, mPlotTitle);
      mIsXyPlotWindowSet = true;
      
      // add to Y params to XyPlotWindow
      //loj: temp code
      int yOffset = 0; //loj: I don't know how this is used
      Real yMin = -40000.0; //loj: should parameter provide minimum value?
      Real yMax =  40000.0; //loj: should parameter provide maximum value?
      
      #if DEBUG_XYPLOT_INIT
      MessageInterface::ShowMessage
         (wxT("XyPlot::Initialize() Get curveTitle and penColor\n"));
      #endif
      
      for (int i=0; i<mNumYParams; i++)
      {
         wxString curveTitle = mYParams[i]->GetName();
         UnsignedInt penColor = mYParams[i]->GetUnsignedIntParameter(wxT("Color"));
            
         #if DEBUG_XYPLOT_INIT
         MessageInterface::ShowMessage(wxT("XyPlot::Initialize() curveTitle = %s\n"),
                                       curveTitle.c_str());
         #endif
         
         PlotInterface::AddXyPlotCurve(instanceName, i, yOffset, yMin, yMax,
                                       curveTitle, penColor);
      }
      
      PlotInterface::ShowXyPlotLegend(instanceName);
      status = true;
      
      #if DEBUG_XYPLOT_INIT
      MessageInterface::ShowMessage(wxT("XyPlot::Initialize() calling ClearXyPlotData()\n"));
      #endif
      
      PlotInterface::ClearXyPlotData(instanceName);
      PlotInterface::XyPlotCurveSettings(instanceName, useLines, lineWidth, 100,
            useMarkers, markerSize, -1);
   }
   else
   {
      #if DEBUG_XYPLOT_INIT
      MessageInterface::ShowMessage(wxT("XyPlot::Initialize() DeleteXyPlot()\n"));
      #endif
      
      status =  PlotInterface::DeleteXyPlot(instanceName);
   }
   
   #if DEBUG_XYPLOT_INIT
   MessageInterface::ShowMessage(wxT("XyPlot::Initialize() leaving stauts=%d\n"), status);
   #endif
   
   return status;
}

//---------------------------------
// methods inherited from GmatBase
//---------------------------------

//------------------------------------------------------------------------------
//  GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the XyPlot.
 *
 * @return clone of the XyPlot.
 *
 */
//------------------------------------------------------------------------------
GmatBase* XyPlot::Clone() const
{
   return (new XyPlot(*this));
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
void XyPlot::Copy(const GmatBase* orig)
{
   operator=(*((XyPlot *)(orig)));
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
bool XyPlot::SetName(const wxString &who, const wxString &oldName)
{
   #if DEBUG_RENAME
   MessageInterface::ShowMessage(wxT("XyPlot::SetName() newName=%s\n"), who.c_str());
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
bool XyPlot::TakeAction(const wxString &action,
                        const wxString &actionData)
{
   #if DEBUG_ACTION_REMOVE
   MessageInterface::ShowMessage(wxT("XyPlot::TakeAction() action=%s, actionData=%s\n"),
                                 action.c_str(), actionData.c_str());
   #endif
   
   if (action == wxT("Clear"))
   {
      return ClearYParameters();
   }
   else if (action == wxT("Remove"))
   {
      return RemoveYParameter(actionData);
   }
   else if (action == wxT("ClearData"))
   {
      return ResetYParameters();
   }
   else if (action == wxT("PenUp"))
   {
      return PenUp();
   }
   else if (action == wxT("PenDown"))
   {
      return PenDown();
   }
   else if (action == wxT("MarkPoint"))
   {
      return MarkPoint();
   }
   else if (action == wxT("MarkBreak"))
   {
      return MarkBreak();
   }
   else if (action == wxT("ClearFromBreak"))
   {
      return ClearFromBreak();
   }
   else if (action == wxT("Darken"))
   {
      wxString data;
      data << actionData;
      long thefactor;
      data.ToLong(&thefactor);
      Integer factor = thefactor;
      return Darken(factor);
   }
   else if (action == wxT("Lighten"))
   {
      wxString data;
      data << actionData;
      long thefactor;
      data.ToLong(&thefactor);
      Integer factor = thefactor;
      return Lighten(factor);
   }
   // Add color change and marker change here(?)
   
   return false;
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
bool XyPlot::RenameRefObject(const Gmat::ObjectType type,
                             const wxString &oldName,
                             const wxString &newName)
{
   #if DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("XyPlot::RenameRefObject() type=%s, oldName=%s, newName=%s\n"),
       GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif
   
   if (type != Gmat::PARAMETER && type != Gmat::COORDINATE_SYSTEM &&
       type != Gmat::SPACECRAFT)
      return true;
   
   if (type == Gmat::PARAMETER)
   {
      // X parameter
      if (mXParamName == oldName)
         mXParamName = newName;
   
      // Y parameters
      for (unsigned int i=0; i<mYParamNames.size(); i++)
      {
         if (mYParamNames[i] == oldName)
            mYParamNames[i] = newName;
      }
   }
   else
   {
      wxString::size_type pos = mXParamName.find(oldName);
      
      if (pos != mXParamName.npos)
         mXParamName.replace(pos, oldName.size(), newName);
      
      for (unsigned int i=0; i<mYParamNames.size(); i++)
      {
         pos = mYParamNames[i].find(oldName);
         
         if (pos != mYParamNames[i].npos)
            mYParamNames[i].replace(pos, oldName.size(), newName);
      }
   }
   
   return true;
}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
wxString XyPlot::GetParameterText(const Integer id) const
{
   if (id >= SubscriberParamCount && id < XyPlotParamCount)
      return PARAMETER_TEXT[id - SubscriberParamCount];
   else
      return Subscriber::GetParameterText(id);
    
}

//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
Integer XyPlot::GetParameterID(const wxString &str) const
{
   for (int i=SubscriberParamCount; i<XyPlotParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - SubscriberParamCount])
      {
         switch (i)
            case IND_VAR:
            case ADD:
            case DRAW_GRID:
               WriteDeprecatedMessage(i);
         return i;
      }
   }
   // deprecated labels
   
   return Subscriber::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
Gmat::ParameterType XyPlot::GetParameterType(const Integer id) const
{
   if (id >= SubscriberParamCount && id < XyPlotParamCount)
      return PARAMETER_TYPE[id - SubscriberParamCount];
   else
      return Subscriber::GetParameterType(id);
}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
wxString XyPlot::GetParameterTypeString(const Integer id) const
{
   if (id >= SubscriberParamCount && id < XyPlotParamCount)
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
bool XyPlot::IsParameterReadOnly(const Integer id) const
{
   if (
       (id == PLOT_TITLE)             ||
       (id == X_AXIS_TITLE)           ||
       (id == Y_AXIS_TITLE)           ||
       (id == DATA_COLLECT_FREQUENCY) ||
       (id == UPDATE_PLOT_FREQUENCY)  ||
       (id == USE_LINES)              ||
       (id == LINE_WIDTH)             ||
       (id == USE_MARKERS)            ||
       (id == MARKER_SIZE)            ||
       (id == DRAWING)                ||
       (id == IND_VAR)                ||
       (id == ADD)                    ||
       (id == DRAW_GRID)            
      )
      return true;
   
   if ((id == UPPER_LEFT) || (id == SIZE))
      return false;

   return Subscriber::IsParameterReadOnly(id);
}

//------------------------------------------------------------------------------
// virtual Integer GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
Integer XyPlot::GetIntegerParameter(const Integer id) const
{
   switch (id)
   {
   case DATA_COLLECT_FREQUENCY:
      return mDataCollectFrequency;
   case UPDATE_PLOT_FREQUENCY:
      return mUpdatePlotFrequency;
   case LINE_WIDTH:
      return lineWidth;
   case MARKER_SIZE:
      return markerSize;
   default:
      return Subscriber::GetIntegerParameter(id);
   }
}

//------------------------------------------------------------------------------
// virtual Integer GetIntegerParameter(const wxString &label) const
//------------------------------------------------------------------------------
Integer XyPlot::GetIntegerParameter(const wxString &label) const
{
   return GetIntegerParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual Integer SetIntegerParameter(const Integer id, const Integer value)
//------------------------------------------------------------------------------
Integer XyPlot::SetIntegerParameter(const Integer id, const Integer value)
{
   switch (id)
   {
   case DATA_COLLECT_FREQUENCY:
      mDataCollectFrequency = value;
      return value;
   case UPDATE_PLOT_FREQUENCY:
      mUpdatePlotFrequency = value;
      return value;
   case LINE_WIDTH:
      lineWidth = value;
      return value;
   case MARKER_SIZE:
      markerSize = value;
      return value;
   default:
      return Subscriber::SetIntegerParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// virtual Integer SetIntegerParameter(const wxString &label,
//                                     const Integer value)
//------------------------------------------------------------------------------
Integer XyPlot::SetIntegerParameter(const wxString &label,
                                    const Integer value)
{
   return SetIntegerParameter(GetParameterID(label), value);
}


//---------------------------------------------------------------------------
//  wxString GetOnOffParameter(const Integer id) const
//---------------------------------------------------------------------------
wxString XyPlot::GetOnOffParameter(const Integer id) const
{
   switch (id)
   {
   case DRAW_GRID:
      WriteDeprecatedMessage(id);
      if (mDrawGrid)
         return wxT("On");
      else
         return wxT("Off");
   default:
      return Subscriber::GetOnOffParameter(id);
   }
}


//------------------------------------------------------------------------------
// wxString XyPlot::GetOnOffParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString XyPlot::GetOnOffParameter(const wxString &label) const
{
   return GetOnOffParameter(GetParameterID(label));
}


//---------------------------------------------------------------------------
//  bool SetOnOffParameter(const Integer id, const wxString &value)
//---------------------------------------------------------------------------
bool XyPlot::SetOnOffParameter(const Integer id, const wxString &value)
{
   switch (id)
   {
   case DRAW_GRID:
      WriteDeprecatedMessage(id);
      mDrawGrid = value == wxT("On");
      return true;
   default:
      return Subscriber::SetOnOffParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetOnOffParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
bool XyPlot::SetOnOffParameter(const wxString &label, const wxString &value)
{
   return SetOnOffParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
wxString XyPlot::GetStringParameter(const Integer id) const
{
   switch (id)
   {
   case IND_VAR:
      WriteDeprecatedMessage(id);
   case XVARIABLE:
      return mXParamName;
   case PLOT_TITLE:
      return mPlotTitle;
   case X_AXIS_TITLE:
      return mXAxisTitle;
   case Y_AXIS_TITLE:
      return mYAxisTitle;
   default:
      return Subscriber::GetStringParameter(id);
   }
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString XyPlot::GetStringParameter(const wxString &label) const
{
   #if DEBUG_XY_PARAM
   MessageInterface::ShowMessage(wxT("XyPlot::GetStringParameter() label = %s\n"),
                                 label.c_str());
   #endif
   
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
bool XyPlot::SetStringParameter(const Integer id, const wxString &value)
{
   #if DEBUG_XYPLOT_PARAM
   MessageInterface::ShowMessage(wxT("XyPlot::SetStringParameter() id = %d, ")
                                 wxT("value = %s \n"), id, value.c_str());
   #endif
   
   switch (id)
   {
   case IND_VAR:
      WriteDeprecatedMessage(id);
   case XVARIABLE:
      return SetXParameter(value);
   case ADD:
      WriteDeprecatedMessage(id);
   case YVARIABLES:
      return AddYParameter(value, mNumYParams);
   case PLOT_TITLE:
      mPlotTitle = value;
      return true;
   case X_AXIS_TITLE:
      mXAxisTitle = value;
      return true;
   case Y_AXIS_TITLE:
      mYAxisTitle = value;
      return true;
   default:
      return Subscriber::SetStringParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label,
//                         const wxString &value)
//------------------------------------------------------------------------------
bool XyPlot::SetStringParameter(const wxString &label,
                                const wxString &value)
{
   #if DEBUG_XYPLOT_PARAM
   MessageInterface::ShowMessage(wxT("XyPlot::SetStringParameter() label = %s, ")
                                 wxT("value = %s \n"), label.c_str(), value.c_str());
   #endif
   
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// virtual bool SetStringParameter(const Integer id, const wxString &value,
//                                 const Integer index)
//------------------------------------------------------------------------------
bool XyPlot::SetStringParameter(const Integer id, const wxString &value,
                                const Integer index)
{
   switch (id)
   {
   case ADD:
      WriteDeprecatedMessage(id);
   case YVARIABLES:
      return AddYParameter(value, index);
   default:
      return Subscriber::SetStringParameter(id, value, index);
   }
}


//------------------------------------------------------------------------------
// virtual bool SetStringParameter(const wxString &label,
//                                 const wxString &value,
//                                 const Integer index)
//------------------------------------------------------------------------------
bool XyPlot::SetStringParameter(const wxString &label,
                                const wxString &value,
                                const Integer index)
{
   #if DEBUG_XYPLOT_PARAM
   MessageInterface::ShowMessage
      (wxT("XyPlot::SetStringParameter() label=%s, value=%s, index=%d \n"),
       label.c_str(), value.c_str(), index);
   #endif
   
   return SetStringParameter(GetParameterID(label), value, index);
}


//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const Integer id) const
//------------------------------------------------------------------------------
const StringArray& XyPlot::GetStringArrayParameter(const Integer id) const
{
   switch (id)
   {
   case ADD:
      WriteDeprecatedMessage(id);
   case YVARIABLES:
      return mYParamNames;
   default:
      return Subscriber::GetStringArrayParameter(id);
   }
}


//------------------------------------------------------------------------------
// StringArray& GetStringArrayParameter(const wxString &label) const
//------------------------------------------------------------------------------
const StringArray& XyPlot::GetStringArrayParameter(const wxString &label) const
{
   return GetStringArrayParameter(GetParameterID(label));
}


bool XyPlot::GetBooleanParameter(const Integer id) const
{
   if (id == SHOW_PLOT)
      return active;
   if (id == USE_MARKERS)
      return useMarkers;
   if (id == USE_LINES)
      return useLines;
   if (id == SHOW_GRID)
      return mDrawGrid;
   if (id == DRAWING)
      return drawing;
   return Subscriber::GetBooleanParameter(id);
}

bool XyPlot::GetBooleanParameter(const wxString &label) const
{
   return GetBooleanParameter(GetParameterID(label));
}

bool XyPlot::SetBooleanParameter(const wxString &label, const bool value)
{
   return SetBooleanParameter(GetParameterID(label), value);
}

bool XyPlot::SetBooleanParameter(const Integer id, const bool value)
{
   if (id == SHOW_PLOT)
   {
      active = value;
      return active;
   }
   if (id == USE_MARKERS)
   {
      useMarkers = value;
      // Always have to have either markers or lines
      if (useMarkers == false)
         useLines = true;
      return useMarkers;
   }
   if (id == USE_LINES)
   {
      useLines = value;
      if (useLines == false)
         useMarkers = true;
      return useLines;
   }
   if (id == SHOW_GRID)
   {
      mDrawGrid = value;
      return true;
   }
   return Subscriber::SetBooleanParameter(id, value);
}

//------------------------------------------------------------------------------
// virtual GmatBase* GetRefObject(const Gmat::ObjectType type,
//                                const wxString &name)
//------------------------------------------------------------------------------
GmatBase* XyPlot::GetRefObject(const Gmat::ObjectType type,
                               const wxString &name)
{
   // if name is X parameter
   if (name == mXParamName)
   {
      return mXParam;
   }
   else
   {
      // name is Y parameter
      for (int i=0; i<mNumYParams; i++)
      {
         if (mYParamNames[i] == name)
            return mYParams[i];
      }
   }
   
   throw GmatBaseException(wxT("XyPlot::GetRefObject() the object name: ") + name +
                           wxT("not found\n"));
}


//------------------------------------------------------------------------------
// virtual bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                           const wxString &name = wxT(""))
//------------------------------------------------------------------------------
bool XyPlot::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                          const wxString &name)
{
   if (type == Gmat::PARAMETER)
   {
      // X parameter
      if (name == mXParamName)
      {
         mXParam = (Parameter*)obj;
         
         if (!mXParam->IsPlottable())
               throw SubscriberException
                  (wxT("The X parameter: ") + name + wxT(" of ") + instanceName +
                   wxT(" is not plottable\n"));
         
         #if DEBUG_XYPLOT_OBJECT
         MessageInterface::ShowMessage
            (wxT("XyPlot::SetRefObject() mXParam:%s successfully set\n"),
             obj->GetName().c_str());
         #endif
      }
      
      // Y parameters
      for (int i=0; i<mNumYParams; i++)
      {
         if (mYParamNames[i] == name)
         {
            mYParams[i] = (Parameter*)obj;
            
            if (!mYParams[i]->IsPlottable())
            {
               throw SubscriberException
                  (wxT("The Y parameter: ") + name + wxT(" of ") + instanceName +
                   wxT(" is not plottable\n"));
            }
            
            #if DEBUG_XYPLOT_OBJECT
            MessageInterface::ShowMessage
               (wxT("XyPlot::SetRefObject() mYParams[%s] successfully set\n"),
                obj->GetName().c_str());
            #endif
            
            return true;
         }
      }
   }
   
   return false;
}


//------------------------------------------------------------------------------
// virtual bool HasRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
bool XyPlot::HasRefObjectTypeArray()
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
const ObjectTypeArray& XyPlot::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::PARAMETER);
   return refObjectTypes;
}


//------------------------------------------------------------------------------
// virtual const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
const StringArray& XyPlot::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   mAllParamNames.clear();
   
   switch (type)
   {
   case Gmat::UNKNOWN_OBJECT:
   case Gmat::PARAMETER:
      // add x parameter
      if (mXParamName != wxT(""))
         mAllParamNames.push_back(mXParamName);
      
      // add y parameters
      for (int i=0; i<mNumYParams; i++)
         if (mYParamNames[i] != wxT(""))
            mAllParamNames.push_back(mYParamNames[i]);
      break;
   default:
      break;
   }
   
   return mAllParamNames;
}

//---------------------------------
// protected methods
//---------------------------------

//------------------------------------------------------------------------------
// void BuildPlotTitle()
//------------------------------------------------------------------------------
void XyPlot::BuildPlotTitle()
{
   //set X and Y axis title
   //if (mXAxisTitle == wxT(""))
   //{
      if (mXParam)
         mXAxisTitle = mXParam->GetName();
      else {
         mXAxisTitle = wxT("No X parameters");
         mYAxisTitle = wxT("empty");
         mPlotTitle  = wxT("Plot not fully initialized");
         return;
      }
      //}
      
   #if DEBUG_XYPLOT_INIT
   MessageInterface::ShowMessage(wxT("XyPlot::BuildPlotTitle() mXAxisTitle = %s\n"),
                                 mXAxisTitle.c_str());
   #endif
   
   mYAxisTitle = wxT("");
   for (int i= 0; i<mNumYParams-1; i++)
   {
      mYAxisTitle += (mYParams[i]->GetName() + wxT(", "));
   }
   mYAxisTitle += mYParams[mNumYParams-1]->GetName();
   
   #if DEBUG_XYPLOT_INIT
   MessageInterface::ShowMessage(wxT("XyPlot::BuildPlotTitle() mYAxisTitle = %s\n"),
                                 mYAxisTitle.c_str());
   #endif
   
   mPlotTitle = wxT("(") + mXAxisTitle + wxT(")") + wxT(" vs ") + wxT("(") + mYAxisTitle + wxT(")");
   
   #if DEBUG_XYPLOT_INIT
   MessageInterface::ShowMessage(wxT("XyPlot::BuildPlotTitle() mPlotTitle = %s\n"),
                                 mPlotTitle.c_str());
   #endif
}

//------------------------------------------------------------------------------
// bool ClearYParameters()
//------------------------------------------------------------------------------
bool XyPlot::ClearYParameters()
{
   DeletePlotCurves();
   mYParams.clear();
   mYParamNames.clear();
   mNumYParams = 0;
   mPlotTitle = wxT("");
   mXAxisTitle = wxT("");
   mYAxisTitle = wxT("");
   mIsXyPlotWindowSet = false;
   return true;
}

//------------------------------------------------------------------------------
// bool RemoveYParameter(const wxString &name)
//------------------------------------------------------------------------------
/*
 * Removes parameter from the Y parameter list
 *
 * @param <name> parameter name to be removed from the Y parameter list
 *
 * @return true if parameter was removed from the Y parameter list, false otherwise
 *
 */
//------------------------------------------------------------------------------
bool XyPlot::RemoveYParameter(const wxString &name)
{
   #if DEBUG_ACTION_REMOVE
   MessageInterface::ShowMessage
      (wxT("XyPlot::RemoveYParameter() name=%s\n--- Before remove:\n"), name.c_str());
   for (int i=0; i<mNumYParams; i++)
   {
      MessageInterface::ShowMessage(wxT("mYParamNames[%d]=%s\n"), i,
                                    mYParamNames[i].c_str());
   }
   #endif
   
   StringArray::iterator pos1;
   std::vector<Parameter*>::iterator pos2 = mYParams.begin();

   for (pos1 = mYParamNames.begin(); pos1 != mYParamNames.end(); pos1++)
   {
      if (*pos1 == name)
      {
         mYParamNames.erase(pos1);
         mYParams.erase(pos2);
         mNumYParams = mYParamNames.size();
         
         #if DEBUG_ACTION_REMOVE
         MessageInterface::ShowMessage(wxT("---After remove\n"));
         for (int i=0; i<mNumYParams; i++)
         {
            MessageInterface::ShowMessage(wxT("mYParamNames[%d]=%s\n"), i,
                                          mYParamNames[i].c_str());
         }
         #endif
         
         return true;
      }
      else
      {
         advance(pos2, 1);
      }
   }
   
   //------------------------------------------
   // loj: 9/29/04
   // Need to remove from PlotCurves also
   //------------------------------------------
   
   #if DEBUG_ACTION_REMOVE
   MessageInterface::ShowMessage(wxT("XyPlot::RemoveYParameter() name=%s not found\n"));
   #endif
   
   return false;
}

//------------------------------------------------------------------------------
// bool ResetYParameters()
//------------------------------------------------------------------------------
bool XyPlot::ResetYParameters()
{
   PlotInterface::ClearXyPlotData(instanceName);
   return true;
}

//------------------------------------------------------------------------------
// bool PenUp()
//------------------------------------------------------------------------------
bool XyPlot::PenUp()
{
   PlotInterface::XyPlotPenUp(instanceName);
   drawing = false;
   return true;
}

//------------------------------------------------------------------------------
// bool PenDown()
//------------------------------------------------------------------------------
bool XyPlot::PenDown()
{
   PlotInterface::XyPlotPenDown(instanceName);
   drawing = true;
   return true;
}

//------------------------------------------------------------------------------
// bool MarkPoint()
//------------------------------------------------------------------------------
/**
 * Places an X marker at the current point on all curves in a plot
 * @return true on success
 */
//------------------------------------------------------------------------------
bool XyPlot::MarkPoint()
{
   PlotInterface::XyPlotMarkPoint(instanceName);
   return true;
}

//------------------------------------------------------------------------------
// bool Darken(Integer factor)
//------------------------------------------------------------------------------
/**
 * Darkens the curves on a plot starting at the current position
 *
 * @param factor The darkening factor
 *
 * @return true on success, false on failure
 */
//------------------------------------------------------------------------------
bool XyPlot::Darken(Integer factor)
{
   PlotInterface::XyPlotDarken(instanceName, factor);
   return true;
}

//------------------------------------------------------------------------------
// bool Lighten(Integer factor)
//------------------------------------------------------------------------------
/**
 * Lightens the curves on a plot starting at the current position
 *
 * @param factor The lightening factor
 *
 * @return true on success, false on failure
 */
//------------------------------------------------------------------------------
bool XyPlot::Lighten(Integer factor)
{
   PlotInterface::XyPlotLighten(instanceName, factor);
   return true;
}


//------------------------------------------------------------------------------
// bool MarkBreak()
//------------------------------------------------------------------------------
/**
 * Sets a break point on all active curves
 *
 * @return true on success, false on failure
 */
//------------------------------------------------------------------------------
bool XyPlot::MarkBreak()
{
   if (mSolverIterOption == SI_CURRENT)
      PlotInterface::XyPlotMarkBreak(instanceName);
   return true;
}

//------------------------------------------------------------------------------
// bool ClearFromBreak()
//------------------------------------------------------------------------------
/**
 * Deletes all data after the most recent break point 
 *
 * @return true on success, false on failure
 */
//------------------------------------------------------------------------------
bool XyPlot::ClearFromBreak()
{
   if (mSolverIterOption == SI_CURRENT)
      PlotInterface::XyPlotClearFromBreak(instanceName);
   return true;
}


//------------------------------------------------------------------------------
// void DeletePlotCurves()
//------------------------------------------------------------------------------
void XyPlot::DeletePlotCurves()
{
   // delete exiting curves
   PlotInterface::DeleteAllXyPlotCurves(instanceName, mOldName);
}


// methods inherited from Subscriber
//------------------------------------------------------------------------------
// bool Distribute(int len)
//------------------------------------------------------------------------------
bool XyPlot::Distribute(int len)
{
   //loj: How do I convert data to Real data?
   return false;
}

//------------------------------------------------------------------------------
// bool Distribute(const Real * dat, Integer len)
//------------------------------------------------------------------------------
bool XyPlot::Distribute(const Real * dat, Integer len)
{
   if (GmatGlobal::Instance()->GetRunMode() == GmatGlobal::TESTING_NO_PLOTS)
      return true;
   
   #if DEBUG_XYPLOT_UPDATE > 1
   MessageInterface::ShowMessage
      (wxT("XyPlot::Distribute() entered. isEndOfReceive=%d, active=%d, runState=%d\n"),
       isEndOfReceive, active, runstate);
   #endif
   
   if (isEndOfReceive)
   {
      // if targetting and draw target is None, just return
      if (mSolverIterations == wxT("None") &&
          ((runstate == Gmat::TARGETING) || (runstate == Gmat::OPTIMIZING) ||
                (runstate == Gmat::SOLVING)))
         return true;
      
      if (active)
         return PlotInterface::RefreshXyPlot(instanceName);
   }
   
   // if targeting and draw target is None, just return
   if (mSolverIterations == wxT("None") &&
       ((runstate == Gmat::TARGETING) || (runstate == Gmat::OPTIMIZING) ||
             (runstate == Gmat::SOLVING)))
      return true;
   
   if (len > 0)
   {
      if (mXParam != NULL && mNumYParams > 0)
      {
         // get x param
         Real xval = mXParam->EvaluateReal();

         #if DEBUG_XYPLOT_UPDATE
         MessageInterface::ShowMessage(wxT("XyPlot::Distribute() xval = %f\n"), xval);
         #endif
         
         // get y params
         Rvector yvals = Rvector(mNumYParams);
         
         // put yvals in the order of parameters added
         for (int i=0; i<mNumYParams; i++)
         {
            if (mYParams[i] == NULL)
            {
               MessageInterface::PopupMessage
                  (Gmat::WARNING_,
                   wxT("*** WARNING *** The XYPlot named \"%s\" will not be shown.\n")
                   wxT("The parameter selected for Y Axis is NULL\n"),
                   GetName().c_str());
               return true;
            }
            
            yvals[i] = mYParams[i]->EvaluateReal();
            
            #if DEBUG_XYPLOT_UPDATE
            MessageInterface::ShowMessage
               (wxT("XyPlot::Distribute() yvals[%d] = %f\n"), i, yvals[i]);
            #endif
         }
         
         // update xy plot
         // X value must start from 0
         if (mIsXyPlotWindowSet)
         {
            mNumDataPoints++;
            
            if ((mNumDataPoints % mDataCollectFrequency) == 0)
            {
               mNumDataPoints = 0;
               mNumCollected++;
               bool update = (mNumCollected % mUpdatePlotFrequency) == 0;
               
               #if DEBUG_XYPLOT_UPDATE > 1
               MessageInterface::ShowMessage
                  (wxT("XyPlot::Distribute() calling PlotInterface::UpdateXyPlot()\n"));
               #endif
               
               // return flag is ignored here since it needs to return true
               // for all case
               PlotInterface::UpdateXyPlot(instanceName, mOldName, xval,
                     yvals, mPlotTitle, mXAxisTitle, mYAxisTitle,
                     mSolverIterOption, update, mDrawGrid);
               
               if (update)
                  mNumCollected = 0;
            }
         }
      }
   }
   
   //loj: always return true otherwise next subscriber will not call ReceiveData()
   //     in Publisher::Publish()
   return true;
}

//------------------------------------------------------------------------------
// void WriteDeprecatedMessage(Integer id) const
//------------------------------------------------------------------------------
void XyPlot::WriteDeprecatedMessage(Integer id) const
{
   // Write only one message per session
   static bool writeXVariable = true;
   static bool writeYVariables = true;
   static bool writeShowGrid = true;
   
   switch (id)
   {
   case IND_VAR:
      if (writeXVariable)
      {
         MessageInterface::ShowMessage
            (deprecatedMessageFormat.c_str(), wxT("IndVar"), GetName().c_str(),
             wxT("XVariable"));
         writeXVariable = false;
      }
      break;
   case ADD:
      if (writeYVariables)
      {
         MessageInterface::ShowMessage
            (deprecatedMessageFormat.c_str(), wxT("Add"), GetName().c_str(),
             wxT("YVariables"));
         writeYVariables = false;
      }
      break;
   case DRAW_GRID:
      if (writeShowGrid)
      {
         MessageInterface::ShowMessage
            (deprecatedMessageFormat.c_str(), wxT("Grid"), GetName().c_str(),
             wxT("ShowGrid"));
         writeShowGrid = false;
      }
      break;
   default:
      break;
   }
}


