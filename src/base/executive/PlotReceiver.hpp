//$Id: PlotReceiver.hpp 9846 2011-09-07 17:57:29Z wendys-dev $
//------------------------------------------------------------------------------
//                             PlotReceiver
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
// Author: Darrel Conway, based on code written by Linda Jun
// Created: 2008/06/16
//
/**
 * Declares PlotReceiver class.
 */
//------------------------------------------------------------------------------
#ifndef PlotReceiver_hpp
#define PlotReceiver_hpp

#include "gmatdefs.hpp"
#include "Rvector.hpp"
#include "SolarSystem.hpp"
#include "CoordinateSystem.hpp"

/**
 * PlotReceiver defines the interfaces used for 3D and XY plot classes.
 */

namespace GmatPlot
{
   enum ViewType
   {
      TRAJECTORY_PLOT,
      ENHANCED_3D_VIEW,
      GROUND_TRACK_PLOT,
   };
};

class GMAT_API PlotReceiver
{
public:

   // for OpenGL Plot
   void               SetViewType(GmatPlot::ViewType view);
   GmatPlot::ViewType GetViewType();
   
   virtual bool CreateGlPlotWindow(const wxString &plotName,
                        const wxString &oldName,
                        Real positionX, Real positionY,
                        Real width, Real height,
                        Integer numPtsToRedraw) = 0;
   
   virtual void SetGlSolarSystem(const wxString &plotName,
                        SolarSystem *ss) = 0;
   
   virtual void SetGlObject(const wxString &plotName,
                        const StringArray &objNames,
                        const UnsignedIntArray &objOrbitColors,
                        const std::vector<SpacePoint*> &objArray) = 0;
   
   virtual void SetGlCoordSystem(const wxString &plotName,
                        CoordinateSystem *internalCs,
                        CoordinateSystem *viewCs,
                        CoordinateSystem *viewUpCs) = 0;
   
   virtual void SetGl2dDrawingOption(const wxString &plotName,
                        const wxString &centralBodyName,
                        const wxString &textureMap,
                        Integer footPrintOption) = 0;
   
   virtual void SetGl3dDrawingOption(const wxString &plotName,
                        bool drawEcPlane, bool drawEqPlane,
                        bool drawWireFrame, bool drawAxes,
                        bool drawGrid, bool drawSunLine,
                        bool overlapPlot, bool usevpInfo,
                        bool drawStars, bool drawConstellations,
                        Integer starCount) = 0;
   
   virtual void SetGl3dViewOption(const wxString &plotName,
                        SpacePoint *vpRefObj, SpacePoint *vpVecObj,
                        SpacePoint *vdObj, Real vsFactor,
                        const Rvector3 &vpRefVec, const Rvector3 &vpVec,
                        const Rvector3 &vdVec, const wxString &upAxis,
                        bool usevpRefVec, bool usevpVec, bool usevdVec) = 0;
   
   virtual void SetGlDrawOrbitFlag(const wxString &plotName,
                        const std::vector<bool> &drawArray) = 0;
   
   virtual void SetGlShowObjectFlag(const wxString &plotName,
                        const std::vector<bool> &showArray) = 0;
   
   virtual void SetGlUpdateFrequency(const wxString &plotName, 
                        Integer updFreq) = 0;
   
   virtual bool IsThere(const wxString &plotName) = 0;
   
   virtual bool InitializeGlPlot(const wxString &plotName) = 0;
   virtual bool RefreshGlPlot(const wxString &plotName) = 0;
   virtual bool DeleteGlPlot(const wxString &plotName) = 0;
   virtual bool SetGlEndOfRun(const wxString &plotName) = 0;
   
   virtual bool UpdateGlPlot(const wxString &plotName,
                        const wxString &oldName,
                        const StringArray &scNames, const Real &time,
                        const RealArray &posX, const RealArray &posY,
                        const RealArray &posZ, const RealArray &velX,
                        const RealArray &velY, const RealArray &velZ,
                        const UnsignedIntArray &scColors, bool solving,
                        Integer solverOption, bool updateCanvas,
                        bool drawing, bool inFunction) = 0;
   
   virtual bool TakeGlAction(const wxString &plotName,
                        const wxString &action) = 0;
   
   // for XY plot
   virtual bool CreateXyPlotWindow(const wxString &plotName,
                        const wxString &oldName,
                        Real positionX, Real positionY,
                        Real width, Real height,
                        const wxString &plotTitle,
                        const wxString &xAxisTitle,
                        const wxString &yAxisTitle,
                        bool drawGrid = false) = 0;
   virtual bool DeleteXyPlot(const wxString &plotName) = 0;
   virtual bool AddXyPlotCurve(const wxString &plotName, int curveIndex,
                        int yOffset, Real yMin, Real yMax,
                        const wxString &curveTitle,
                        UnsignedInt penColor) = 0;
   virtual bool DeleteAllXyPlotCurves(const wxString &plotName,
                        const wxString &oldName) = 0;
   virtual bool DeleteXyPlotCurve(const wxString &plotName,
                        int curveIndex) = 0;
   virtual void ClearXyPlotData(const wxString &plotName) = 0;
   virtual void XyPlotPenUp(const wxString &plotName) = 0;
   virtual void XyPlotPenDown(const wxString &plotName) = 0;
   virtual void XyPlotDarken(const wxString &plotName, Integer factor,
                        Integer index = -1, Integer forCurve = -1) = 0;
   virtual void XyPlotLighten(const wxString &plotName, Integer factor,
                        Integer index = -1, Integer forCurve = -1) = 0;

   virtual void XyPlotMarkPoint(const wxString &plotName, Integer index = -1,
                        Integer forCurve = -1) = 0;
   virtual void XyPlotMarkBreak(const wxString &plotName, Integer index = -1,
                        Integer curveNumber = -1) = 0;
   virtual void XyPlotClearFromBreak(const wxString &plotName,
                        Integer breakNumber, Integer index = -1, Integer curveNumber = -1) = 0;

   virtual void XyPlotChangeColor(const wxString &plotName,
                        Integer index = -1, UnsignedInt newColor = 0xffffff,
                        Integer forCurve = -1) = 0;
   virtual void XyPlotChangeMarker(const wxString &plotName,
                        Integer index = -1, Integer newMarker = -1, int forCurve = -1) = 0;
   virtual void XyPlotChangeWidth(const wxString &plotName,
                        Integer index = -1, Integer newWidth = 1, int forCurve = -1) = 0;
   virtual void XyPlotChangeStyle(const wxString &plotName,
                        Integer index = -1, Integer newStyle = 100, int forCurve = -1) = 0;

   virtual void XyPlotRescale(const wxString &plotName) = 0;
   virtual void XyPlotCurveSettings(const wxString &plotName,
                        bool useLines = true,
                        Integer lineWidth = 1,
                        Integer lineStyle = 100,
                        bool useMarkers = false,
                        Integer markerSize = 3,
                        Integer marker = 1,
                        bool useHiLow = false,
                        Integer forCurve = -1) = 0;

   virtual void SetXyPlotTitle(const wxString &plotName,
                        const wxString &plotTitle) = 0;
   virtual void ShowXyPlotLegend(const wxString &plotName) = 0;
   virtual bool RefreshXyPlot(const wxString &plotName) = 0;
   virtual bool UpdateXyPlot(const wxString &plotName,
                        const wxString &oldName,
                        const Real &xval, const Rvector &yvals,
                        const wxString &plotTitle,
                        const wxString &xAxisTitle,
                        const wxString &yAxisTitle,
                        bool updateCanvas, bool drawGrid) = 0;
   virtual bool UpdateXyPlotData(const wxString &plotName, const Real &xval,
                        const Rvector &yvals, const Rvector *yhis = NULL,
                        const Rvector *ylows = NULL) = 0;
   virtual bool UpdateXyPlotCurve(const wxString &plotName,
                        const Integer whichCurve, const Real xval,
                        const Real yval, const Real yhi = 0.0,
                        const Real ylow = 0.0) = 0;

   virtual bool DeactivateXyPlot(const wxString &plotName) = 0;
   virtual bool ActivateXyPlot(const wxString &plotName) = 0;

protected:
   GmatPlot::ViewType currentView;
   PlotReceiver();
   virtual ~PlotReceiver();
};

#endif
