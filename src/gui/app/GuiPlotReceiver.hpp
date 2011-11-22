//$Id: GuiPlotReceiver.hpp 9876 2011-09-16 20:58:50Z lindajun $
//------------------------------------------------------------------------------
//                             GuiPlotReceiver
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
 * Declares GuiPlotReceiver class.
 */
//------------------------------------------------------------------------------
#ifndef GuiPlotReceiver_hpp
#define GuiPlotReceiver_hpp

#include "gmatdefs.hpp"
#include "Rvector.hpp"
#include "SolarSystem.hpp"
#include "CoordinateSystem.hpp"
#include "PlotReceiver.hpp"


/**
 * The PlotReceiver used in GMAT's wxWidgets based GUI
 */
class GuiPlotReceiver : public PlotReceiver
{

public:
   static GuiPlotReceiver* Instance();
   
   // for OpenGL Plot
   virtual bool CreateGlPlotWindow(const wxString &plotName,
                        const wxString &oldName,
                        Real positionX, Real positionY,
                        Real width, Real height,
                        Integer numPtsToRedraw);
   
   virtual void SetGlSolarSystem(const wxString &plotName, SolarSystem *ss);
   
   virtual void SetGlObject(const wxString &plotName,
                        const StringArray &objNames,
                        const UnsignedIntArray &objOrbitColors,
                        const std::vector<SpacePoint*> &objArray);
   
   virtual void SetGlCoordSystem(const wxString &plotName,
                        CoordinateSystem *internalCs,
                        CoordinateSystem *viewCs,
                        CoordinateSystem *viewUpCs);
   
   virtual void SetGl2dDrawingOption(const wxString &plotName,
                        const wxString &centralBodyName,
                        const wxString &textureMap,
                        Integer footPrintOption);
   
   virtual void SetGl3dDrawingOption(const wxString &plotName,
                        bool drawEcPlane, bool drawXyPlane,
                        bool drawWireFrame, bool drawAxes,
                        bool drawGrid, bool drawSunLine,
                        bool overlapPlot, bool usevpInfo,
                        bool drawStars, bool drawConstellations,
                        Integer starCount);
   
   virtual void SetGl3dViewOption(const wxString &plotName,
                        SpacePoint *vpRefObj, SpacePoint *vpVecObj,
                        SpacePoint *vdObj, Real vsFactor,
                        const Rvector3 &vpRefVec, const Rvector3 &vpVec,
                        const Rvector3 &vdVec, const wxString &upAxis,
                        bool usevpRefVec, bool usevpVec, bool usevdVec);
   
   virtual void SetGlDrawOrbitFlag(const wxString &plotName,
                        const std::vector<bool> &drawArray);
   
   virtual void SetGlShowObjectFlag(const wxString &plotName,
                        const std::vector<bool> &showArray);
   
   virtual void SetGlUpdateFrequency(const wxString &plotName, Integer updFreq);
   
   virtual bool IsThere(const wxString &plotName);
   
   virtual bool InitializeGlPlot(const wxString &plotName);
   virtual bool RefreshGlPlot(const wxString &plotName);
   virtual bool DeleteGlPlot(const wxString &plotName);
   virtual bool SetGlEndOfRun(const wxString &plotName);
   
   virtual bool UpdateGlPlot(const wxString &plotName,
                        const wxString &oldName,
                        const StringArray &scNames, const Real &time,
                        const RealArray &posX, const RealArray &posY,
                        const RealArray &posZ, const RealArray &velX,
                        const RealArray &velY, const RealArray &velZ,
                        const UnsignedIntArray &scColors, bool solving,
                        Integer solverOption, bool updateCanvas,
                        bool drawing, bool inFunction = false);
   
   virtual bool TakeGlAction(const wxString &plotName,
                        const wxString &action);
   
   // for XY plot
   virtual bool CreateXyPlotWindow(const wxString &plotName,
                        const wxString &oldName,
                        Real positionX, Real positionY,
                        Real width, Real height,
                        const wxString &plotTitle,
                        const wxString &xAxisTitle,
                        const wxString &yAxisTitle,
                        bool drawGrid = false);
   virtual bool DeleteXyPlot(const wxString &plotName);
   virtual bool AddXyPlotCurve(const wxString &plotName, int curveIndex,
                        int yOffset, Real yMin, Real yMax,
                        const wxString &curveTitle,
                        UnsignedInt penColor);
   virtual bool DeleteAllXyPlotCurves(const wxString &plotName,
                        const wxString &oldName);
   virtual bool DeleteXyPlotCurve(const wxString &plotName, int curveIndex);
   virtual void ClearXyPlotData(const wxString &plotName);
   virtual void XyPlotPenUp(const wxString &plotName);
   virtual void XyPlotPenDown(const wxString &plotName);
   virtual void XyPlotDarken(const wxString &plotName, Integer factor,
                        Integer index = -1, Integer forCurve = -1);
   virtual void XyPlotLighten(const wxString &plotName, Integer factor,
                        Integer index = -1, Integer forCurve = -1);
   virtual void XyPlotMarkPoint(const wxString &plotName, Integer index = -1,
                        Integer forCurve = -1);
   virtual void XyPlotMarkBreak(const wxString &plotName, Integer index = -1,
                        Integer forCurve = -1);
   virtual void XyPlotClearFromBreak(const wxString &plotName,
                        Integer startBreakNumber, Integer endBreakNumber = -1,
                        Integer forCurve = -1);

   virtual void XyPlotChangeColor(const wxString &plotName,
                        Integer index = -1, UnsignedInt newColor = 0xffffff,
                        Integer forCurve = -1);
   virtual void XyPlotChangeMarker(const wxString &plotName,
                        Integer index = -1, Integer newMarker = -1, Integer forCurve = -1);
   virtual void XyPlotChangeWidth(const wxString &plotName,
                        Integer index = -1, Integer newWidth = 1, int forCurve = -1);
   virtual void XyPlotChangeStyle(const wxString &plotName,
                        Integer index = -1, Integer newStyle = 100, int forCurve = -1);

   virtual void XyPlotRescale(const wxString &plotName);
   virtual void XyPlotCurveSettings(const wxString &plotName,
                        bool useLines = true,
                        Integer lineWidth = 1,
                        Integer lineStyle = 100,
                        bool useMarkers = false,
                        Integer markerSize = 3,
                        Integer marker = 1,
                        bool useHiLow = false,
                        Integer forCurve = -1);
   
   virtual void SetXyPlotTitle(const wxString &plotName,
                        const wxString &plotTitle);
   virtual void ShowXyPlotLegend(const wxString &plotName);
   virtual bool RefreshXyPlot(const wxString &plotName);
   virtual bool UpdateXyPlot(const wxString &plotName,
                        const wxString &oldName,
                        const Real &xval, const Rvector &yvals,
                        const wxString &plotTitle,
                        const wxString &xAxisTitle,
                        const wxString &yAxisTitle,
                        bool updateCanvas, bool drawGrid);
   virtual bool UpdateXyPlotData(const wxString &plotName, const Real &xval,
                        const Rvector &yvals, const Rvector *yhis = NULL,
                        const Rvector *ylows = NULL);
   virtual bool UpdateXyPlotCurve(const wxString &plotName,
                        const Integer whichCurve, const Real xval,
                        const Real yval, const Real yhi = 0.0,
                        const Real ylow = 0.0);

   virtual bool DeactivateXyPlot(const wxString &plotName);
   virtual bool ActivateXyPlot(const wxString &plotName);

protected:
   bool ComputePlotPositionAndSize(bool isGLPlot, Real positionX,
                                   Real positionY, Real width, Real height,
                                   Integer &x, Integer &y, Integer &w, Integer &h);
   
private:
   GuiPlotReceiver();
   virtual ~GuiPlotReceiver();

   static GuiPlotReceiver* theGuiPlotReceiver;
};

#endif
