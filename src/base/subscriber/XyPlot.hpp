//$Id: XyPlot.hpp 9513 2011-04-30 21:23:06Z djcinsb $
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
 * Declares XyPlot class.
 */
//------------------------------------------------------------------------------
#ifndef XyPlot_hpp
#define XyPlot_hpp

#include "Subscriber.hpp"
#include "Parameter.hpp"


/**
 * Subscriber class used to drive the XyPlot components on the GUI
 */
class GMAT_API XyPlot : public Subscriber 
{
public:
   XyPlot(const wxString &name, Parameter *xParam = NULL,
          Parameter *firstYParam = NULL, const wxString &plotTitle = wxT(""),
          const wxString &xAxisTitle = wxT(""), const wxString &yAxisTitle = wxT(""));
   XyPlot(const XyPlot &orig);
   XyPlot& operator=(const XyPlot& orig);
   virtual ~XyPlot(void);
   
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
   
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name = wxT(""));
   
   virtual bool         HasRefObjectTypeArray();
   virtual const ObjectTypeArray&
                        GetRefObjectTypeArray();
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   
protected:

   bool SetXParameter(const wxString &paramName);
   bool AddYParameter(const wxString &paramName, Integer index);
   void BuildPlotTitle();
   bool ClearYParameters();
   bool RemoveYParameter(const wxString &name);
   bool ResetYParameters();
   bool PenUp();
   bool PenDown();
   bool MarkPoint();
   bool Darken(Integer factor);
   bool Lighten(Integer factor);
   bool MarkBreak();
   bool ClearFromBreak();
   
   void DeletePlotCurves();
   void WriteDeprecatedMessage(Integer id) const;

   Parameter *mXParam;
   std::vector<Parameter*> mYParams;
   
   Integer mNumXParams;
   Integer mNumYParams;
   
   wxString mXParamName;
   StringArray mYParamNames;
   StringArray mAllParamNames;
   
   wxString mOldName;
   wxString mPlotTitle;
   wxString mXAxisTitle;
   wxString mYAxisTitle;
   bool mDrawGrid;
   bool mIsXyPlotWindowSet;
   
   Integer mDataCollectFrequency;
   Integer mUpdatePlotFrequency;
   
   Integer mNumDataPoints;
   Integer mNumCollected;
   
   bool useLines;
   Integer lineWidth;
   bool useMarkers;
   Integer markerSize;
   bool drawing;
   Integer breakCount;

   // methods inherited from Subscriber
   virtual bool Distribute(Integer len);
   virtual bool Distribute(const Real * dat, Integer len);

public:
   enum
   {
      XVARIABLE = SubscriberParamCount,
      YVARIABLES,
      PLOT_TITLE,
      X_AXIS_TITLE,
      Y_AXIS_TITLE,
      SHOW_GRID,
      DATA_COLLECT_FREQUENCY,
      UPDATE_PLOT_FREQUENCY,
      SHOW_PLOT,
      USE_LINES,
      LINE_WIDTH,
      USE_MARKERS,
      MARKER_SIZE,
      DRAWING,
      IND_VAR,                 // deprecated
      ADD,                     // deprecated
      DRAW_GRID,               // deprecated
      XyPlotParamCount
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[XyPlotParamCount - SubscriberParamCount];
   static const wxString
      PARAMETER_TEXT[XyPlotParamCount - SubscriberParamCount];

};

#endif
