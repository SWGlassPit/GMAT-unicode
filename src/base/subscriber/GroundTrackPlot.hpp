//$Id: GroundTrackPlot.hpp 10025 2011-11-29 23:31:31Z djcinsb $
//------------------------------------------------------------------------------
//                                  GroundTrackPlot
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
// Created: 2011/05/31
//
/**
 * Declares GroundTrackPlot class.
 */
//------------------------------------------------------------------------------
#ifndef GroundTrackPlot_hpp
#define GroundTrackPlot_hpp

#include "OrbitPlot.hpp"

class GMAT_API GroundTrackPlot : public OrbitPlot
{
public:
   GroundTrackPlot(const wxString &name);
   GroundTrackPlot(const GroundTrackPlot &plot);
   GroundTrackPlot& operator=(const GroundTrackPlot &plot);
   virtual ~GroundTrackPlot();
   
   // methods inherited from GmatBase
   virtual bool         Validate();
   virtual bool         Initialize();
   
   virtual GmatBase*    Clone() const;
   virtual void         Copy(const GmatBase* orig);
   
   virtual bool         TakeAction(const wxString &action,  
                                   const wxString &actionData = wxT(""));
   
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   
   // methods for parameters
   virtual bool         IsParameterReadOnly(const Integer id) const;
   
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual bool         SetStringParameter(const Integer id, const wxString &value);
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value);
   virtual const ObjectTypeArray& GetTypesForList(const Integer id);
   virtual const ObjectTypeArray& GetTypesForList(const wxString &label);
   
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
   
   enum FootPrintOption
   {
      FP_NONE,
      FP_ALL,
      FootPrintOptionCount
   };
   
protected:
   
   /// Calls PlotInterface for plotting solver data
   virtual bool         UpdateSolverData();
   /// Handles published data
   virtual bool         Distribute(const Real * dat, Integer len);
   
   CelestialBody *centralBody;
   wxString centralBodyName;
   wxString footPrints;
   wxString textureMapFileName;
   FootPrintOption footPrintOption;
   
public:

   // for scripting
   enum
   {
      CENTRAL_BODY = OrbitPlotParamCount,
      TEXTURE_MAP,
      SHOW_FOOT_PRINTS,
      GroundTrackPlotParamCount
   };
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[GroundTrackPlotParamCount - OrbitPlotParamCount];
   static const wxString
      PARAMETER_TEXT[GroundTrackPlotParamCount - OrbitPlotParamCount];
   
   // for GUI population
   static StringArray footPrintOptions;
   virtual Gmat::ObjectType
                        GetPropertyObjectType(const Integer id) const;
   virtual const StringArray&
                        GetPropertyEnumStrings(const Integer id) const;
   virtual const StringArray&
                        GetPropertyEnumStrings(const wxString &label) const;
   
   static Integer GetFootPrintOptionCount() { return FootPrintOptionCount; }
   static const wxString* GetFootPrintOptionList();
   
private:
   
   static const wxString FOOT_PRINT_OPTION_TEXT[FootPrintOptionCount];
};

#endif
