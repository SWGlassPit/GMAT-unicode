//$Id: PlotInterface.hpp 9692 2011-07-12 19:20:21Z lindajun $
//------------------------------------------------------------------------------
//                             PlotInterface
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
// Created: 2003/12/18
//
/**
 * Declares PlotInterface class.
 */
//------------------------------------------------------------------------------
#ifndef PlotInterface_hpp
#define PlotInterface_hpp

#include "gmatdefs.hpp"
#include "Rvector.hpp"
#include "SolarSystem.hpp"
#include "CoordinateSystem.hpp"
#include "PlotReceiver.hpp"


/**
 * Interface functions for the OpenGL and XY plot classes.
 */
class GMAT_API PlotInterface
{

public:
   
   static void SetPlotReceiver(PlotReceiver *pr);
   
   // for OpenGL Plot
   static bool CreateGlPlotWindow(const wxString &plotName,
                        const wxString &oldName,
                        Integer numPtsToRedraw);
   
   static void SetViewType(GmatPlot::ViewType view);
   static void SetGlSolarSystem(const wxString &plotName, SolarSystem *ss);
   
   static void SetGlObject(const wxString &plotName,
                        const StringArray &objNames,
                        const UnsignedIntArray &objOrbitColors,
                        const std::vector<SpacePoint*> &objArray);
   
   static void SetGlCoordSystem(const wxString &plotName,
                        CoordinateSystem *internalCs,
                        CoordinateSystem *viewCs,
                        CoordinateSystem *viewUpCs);
   
   static void SetGl2dDrawingOption(const wxString &plotName,
                        const wxString &centralBodyName,
                        const wxString &textureMap,
                        Integer footPrintOption);
   
   static void SetGl3dDrawingOption(const wxString &plotName,
                        bool drawEcPlane, bool drawXyPlane,
                        bool drawWireFrame, bool drawAxes,
                        bool drawGrid, bool drawSunLine,
                        bool overlapPlot, bool usevpInfo,
                        bool drawStars, bool drawConstellations,
                        Integer starCount);
   
   static void SetGl3dViewOption(const wxString &plotName,
                        SpacePoint *vpRefObj, SpacePoint *vpVecObj,
                        SpacePoint *vdObj, Real vsFactor,
                        const Rvector3 &vpRefVec, const Rvector3 &vpVec,
                        const Rvector3 &vdVec, const wxString &upAxis,
                        bool usevpRefVec, bool usevpVec, bool usevdVec);
   
   static void SetGlDrawOrbitFlag(const wxString &plotName,
                        const std::vector<bool> &drawArray);
   
   static void SetGlShowObjectFlag(const wxString &plotName,
                        const std::vector<bool> &showArray);
   
   static void SetGlUpdateFrequency(const wxString &plotName, Integer updFreq);
   
   static bool IsThere(const wxString &plotName);
   
   static bool InitializeGlPlot(const wxString &plotName);
   static bool RefreshGlPlot(const wxString &plotName);
   static bool DeleteGlPlot(const wxString &plotName);
   static bool SetGlEndOfRun(const wxString &plotName);
   
   static bool UpdateGlPlot(const wxString &plotName,
                        const wxString &oldName,
                        const StringArray &scNames, const Real &time,
                        const RealArray &posX, const RealArray &posY,
                        const RealArray &posZ, const RealArray &velX,
                        const RealArray &velY, const RealArray &velZ,
                        const UnsignedIntArray &scColors, bool solving,
                        Integer solverOption, bool updateCanvas,
                        bool drawing, bool inFunction = false);
   
   static bool TakeGlAction(const wxString &plotName,
                        const wxString &action);
   
   // for XY plot
   static bool CreateXyPlotWindow(const wxString &plotName,
                        const wxString &oldName,
                        const wxString &plotTitle,
                        const wxString &xAxisTitle,
                        const wxString &yAxisTitle,
                        bool drawGrid = false);
   static bool DeleteXyPlot(const wxString &plotName);
   static bool AddXyPlotCurve(const wxString &plotName, int curveIndex,
                        int yOffset, Real yMin, Real yMax,
                        const wxString &curveTitle,
                        UnsignedInt penColor);
   static bool DeleteAllXyPlotCurves(const wxString &plotName,
                        const wxString &oldName);
   static bool DeleteXyPlotCurve(const wxString &plotName, int curveIndex);
   static void ClearXyPlotData(const wxString &plotName);
   static void XyPlotPenUp(const wxString &plotName);
   static void XyPlotPenDown(const wxString &plotName);
   static void XyPlotDarken(const wxString &plotName, Integer factor,
                        Integer index = -1, Integer curveNumber = -1);
   static void XyPlotLighten(const wxString &plotName, Integer factor,
                        Integer index = -1, Integer curveNumber = -1);
   static void XyPlotMarkPoint(const wxString &plotName, Integer index = -1,
                        Integer curveNumber = -1);
   static void XyPlotMarkBreak(const wxString &plotName, Integer index = -1,
                        Integer curveNumber = -1);
   static void XyPlotClearFromBreak(const wxString &plotName,
                        Integer startBreakNumber = -1, Integer endBreakNumber = -1,
                        Integer curveNumber = -1);

   static void XyPlotChangeWidth(const wxString &plotName,
                        Integer index = -1, Integer newWidth = 1, int forCurve = -1);
   static void XyPlotChangeStyle(const wxString &plotName,
                        Integer index = -1, Integer newStyle = 100, int forCurve = -1);

   
   static void XyPlotRescale(const wxString &plotName);

   static void XyPlotCurveSettings(const wxString &plotName,
                        bool useLines = true,
                        Integer lineWidth = 1,
                        Integer lineStyle = 100,
                        bool useMarkers = false,
                        Integer markerSize = 3,
                        Integer marker = 1,
                        bool useHiLow = false,
                        Integer forCurve = -1);

   static void SetXyPlotTitle(const wxString &plotName,
                        const wxString &plotTitle);
   static void ShowXyPlotLegend(const wxString &plotName);
   static bool RefreshXyPlot(const wxString &plotName);
   static bool UpdateXyPlot(const wxString &plotName,
                        const wxString &oldName,
                        const Real &xval, const Rvector &yvals,
                        const wxString &plotTitle,
                        const wxString &xAxisTitle,
                        const wxString &yAxisTitle,
                        Integer solverOption,
                        bool updateCanvas, bool drawGrid);
   static bool UpdateXyPlotData(const wxString &plotName,
                        const Real &xval, const Rvector &yvals,
                        const Rvector &hiError,
                        const Rvector &lowError);
   
   static bool UpdateXyPlotCurve(const wxString &plotName,
                        Integer whichCurve, const Real &xval, const Real &yval,
                        const Real hi = 0.0, const Real low = 0.0);

   static bool DeactivateXyPlot(const wxString &plotName);
   static bool ActivateXyPlot(const wxString &plotName);


private:

   PlotInterface();
   ~PlotInterface();
   
   static PlotReceiver *thePlotReceiver;

};

#endif
