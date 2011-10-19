//$Id: OwnedPlot.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  OwnedPlot
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
// Author: Darrel Conway, based on XyPlot code by Linda Jun
// Created: 2009/09/28
//
/**
 * Defines XyPlot class controlled by Sandbox elements rather than Subscribers.
 */
//------------------------------------------------------------------------------
#ifndef OwnedPlot_hpp
#define OwnedPlot_hpp

#include "GmatBase.hpp"


/**
 * The OwnedPlot class provides access to plotting capabilities for components
 * that need to display graphical information for that component only, without
 * generating object to object interactions or data passing through GMAT's
 * Publisher.  This component is wholly owned by another, which passes in data
 * and controls plot updates using the interfaces defined here.
 *
 * OwnedPlot objects talk to the graphical components through the PlotInterface
 * and PlotReceiver classes.  The standard GMAT GUI interface for this
 * communication is the GuiPlotReceiver, which connects the plot messages to a
 * XyPlot object displayed on the GUI.
 *
 * In spite of their location in GMAT's directory structure, OwnedPlots are not
 * Subscribers.  They are derived directly from GmatBase.
 */
class GMAT_API OwnedPlot : public GmatBase
{
public:
   OwnedPlot(const wxString &name, const wxString &plotTitle = wxT(""),
          const wxString &xAxisTitle = wxT(""),
          const wxString &yAxisTitle = wxT(""));
   OwnedPlot(const OwnedPlot &orig);
   OwnedPlot& operator=(const OwnedPlot& orig);
   virtual ~OwnedPlot(void);
   
   // methods inherited from Subscriber
   virtual bool         Initialize();
   
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
   
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   virtual bool         IsParameterReadOnly(const Integer id) const;
   
   virtual Integer      GetIntegerParameter(const Integer id) const;
   virtual Integer      GetIntegerParameter(const wxString &label) const;
   virtual Integer      SetIntegerParameter(const Integer id,
                                            const Integer value);
   virtual Integer      SetIntegerParameter(const wxString &label,
                                            const Integer value);
   
   virtual bool         GetBooleanParameter(const Integer id) const;
   virtual bool         GetBooleanParameter(const wxString &label) const;
   virtual bool         SetBooleanParameter(const Integer id,
                                            const bool value);
   virtual bool         SetBooleanParameter(const wxString &label,
                                            const bool value);
   
   virtual wxString  GetOnOffParameter(const Integer id) const;
   virtual bool         SetOnOffParameter(const Integer id, 
                                          const wxString &value);
   virtual wxString  GetOnOffParameter(const wxString &label) const;
   virtual bool         SetOnOffParameter(const wxString &label, 
                                          const wxString &value);
   
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value);
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value);
   
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value,
                                           const Integer index);
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value,
                                           const Integer index);
   
   virtual const StringArray&
                        GetStringArrayParameter(const Integer id) const;
   virtual const StringArray&
                        GetStringArrayParameter(const wxString &label) const;
   
   bool                 Activate();
   bool                 Deactivate();

   // Methods used to access the plot
   virtual void         SetData(std::vector<RealArray*> &dataBlast,
                              RealArray hiErrors, RealArray lowErrors);
   virtual void         SetCurveData(const Integer forCurve, RealArray *xData,
                              RealArray *yData, const RealArray *yhis = NULL,
                              const RealArray *ylows = NULL);

   virtual bool         MarkPoint(Integer whichOne = -1, Integer forCurve = -1);

   virtual Integer      SetUsedDataID(Integer id, Integer forCurve = -1);
   virtual void         SetUsedObjectID(Integer id);
   virtual Integer      UsesData(Integer id);
   virtual Integer      UsesObject(Integer id);

protected:
   StringArray  curveNames;
   IntegerArray curveDataIDs;
   
   // These arrays store the default settings, curve by curve  They default to
   // the plot level settings unless the user overrides them

   /// Curve colors; default is 0xFF0000 (Blue)
   IntegerArray curveColor;
   /// Line width used to draw lines and markers; default is 1
   IntegerArray curveLineWidth;
   /// Line style for each line; default is wxSOLID
   IntegerArray curveLineStyle;
   /// Marker styles for the curves; these iterate through the defined styles
   IntegerArray curveMarker;
   /// Marker size; default is 3
   IntegerArray curveMarkerSize;

   /// Curve by curve useLines setting
   std::vector<bool> curveUseLines;
   /// Curve by curve useMarkers setting
   std::vector<bool> curveUseMarkers;
   /// Curve by curve useHiLow setting
   std::vector<bool> curveUseHiLow;

   wxString mOldName;
   wxString mPlotTitle;
   wxString mXAxisTitle;
   wxString mYAxisTitle;
   wxString mDrawGrid;
   bool mIsOwnedPlotWindowSet;
   
   Integer mDataCollectFrequency;
   Integer mUpdatePlotFrequency;

   /// Default color
   Integer defaultColor;
   /// Default marker size; initialized to 3
   Integer markerSize;
   /// Default marker style; initialized to -1, meaning set by curve number
   Integer markerStyle;
   /// Default line width used to draw lines and markers; default is 1
   Integer lineWidth;
   /// Default line style; initialized to a solid line
   Integer lineStyle;   // If this breaks, check the enum value for wxSOLID

   bool useLines;
   bool useMarkers;
   bool useHiLow;

   IntegerArray supportedData;
   IntegerArray supportedObjects;

   bool                 active;
   bool                 showLegend;
   bool                 isEndOfReceive;
   bool                 isEndOfRun;
   bool                 isInitialized;
   wxString          mSolverIterations;
   Gmat::RunState       runstate;

   void BuildPlotTitle();
   bool ClearYParameters();
   bool RemoveYParameter(const wxString &name);
   bool ResetYParameters();
   bool PenUp();
   bool PenDown();
   bool RescaleData();

   void DeletePlotCurves();

   enum
   {
      ADD = GmatBaseParamCount,
      PLOT_TITLE,
      X_AXIS_TITLE,
      Y_AXIS_TITLE,
      DRAW_GRID,
      DATA_COLLECT_FREQUENCY,
      UPDATE_PLOT_FREQUENCY,
      SHOW_PLOT,
      SHOW_LEGEND,
      DEFAULT_COLOR,
      USE_LINES,
      LINE_WIDTH,
      LINE_STYLE,
      USE_MARKERS,
      MARKER_SIZE,
      MARKER_STYLE,
      USE_HI_LOW,
      OwnedPlotParamCount
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[OwnedPlotParamCount - GmatBaseParamCount];
   static const wxString
      PARAMETER_TEXT[OwnedPlotParamCount - GmatBaseParamCount];
};

#endif
