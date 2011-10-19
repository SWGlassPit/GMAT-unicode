//$Id: Report.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                            Report
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under MOMS Purchase
// order MOMS418823
//
// Author: Darrel Conway, Thinking Systems, Inc.
// Created: 2005/07/06
//
/**
 *  Class definition for the Report command.
 */
//------------------------------------------------------------------------------
 
#ifndef Report_hpp
#define Report_hpp

#include "GmatCommand.hpp"
#include "ReportFile.hpp"
#include "Parameter.hpp"
#include "ElementWrapper.hpp"

/**
 * The Report command is used to write data to a ReportFile at specific times.
 * 
 * The ReportFile object is a subscriber used to generate data in an ASCII file.
 * Parameters added directly to the ReportFile are written whenever the 
 * publisher is fed data.  Parmeters that need to be seen only at specific times 
 * in a script are published using this command.
 * 
 * @note The Publisher does not currently pass text strings correctly.  The
 *       Report command is set to directly write data to the ReportFile until
 *       this defect is corrected.
 */
class GMAT_API Report : public GmatCommand
{
public:
   Report();
   virtual ~Report();
   Report(const Report &rep);
   Report& operator=(const Report &rep);
   
   virtual GmatBase*    Clone() const;
   
   // Parameter accessor methods
   virtual Integer      GetParameterID(const wxString &str) const;
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
   
   // ElementWrapper accessor methods
   virtual const StringArray& 
                        GetWrapperObjectNameArray();
   virtual bool         SetElementWrapper(ElementWrapper* toWrapper,
                                          const wxString &withName);
   virtual void         ClearWrappers();
   
   // Object accessor methods
   virtual wxString  GetRefObjectName(const Gmat::ObjectType type) const;
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name,
                                     const Integer index);
   
   // Command methods
   virtual bool         TakeAction(const wxString &action,  
                                   const wxString &actionData = wxT(""));
   
   bool                 Initialize();
   virtual bool         Execute();
   virtual void         RunComplete();
   
   // Generating string method
   virtual const wxString&
                        GetGeneratingString(Gmat::WriteMode mode = Gmat::SCRIPTING,
                                            const wxString &prefix = wxT(""),
                                            const wxString &useName = wxT(""));
   
protected:
   /// Name of the subscriber
   wxString                rfName;
   /// The ReportFile subscriber that receives the data
   ReportFile                 *reporter;
   /// The ID for the subscriber
   Integer                    reportID;
   /// Array of parameter names
   StringArray                parmNames;
   /// Array of actual parameter names including index
   StringArray                actualParmNames;
   /// Number of parameters
   Integer                    numParams;
   /// Array of parameters that get written to the report
   std::vector<Parameter*>    parms;
   /// Flag indicating whether the header data has been written
   bool                       needsHeaders;
   /// Array of parameter row index
   std::vector<Integer>       parmRows;
   /// Array of parameter column index
   std::vector<Integer>       parmCols;
   /// ElementWraper pointers of parameters
   std::vector<ElementWrapper*> parmWrappers;
   
   void WriteHeaders(wxString &datastream, Integer colWidth);
   bool AddParameter(const wxString &paramName, Integer index,
                     Parameter *param = NULL);
   void DeleteParameters();
   
   enum
   {
      REPORTFILE = GmatCommandParamCount,
      ADD,
      ReportParamCount  /// Count of the parameters for this class
   };
   
private:

   static const wxString
      PARAMETER_TEXT[ReportParamCount - GmatCommandParamCount];
   static const Gmat::ParameterType
      PARAMETER_TYPE[ReportParamCount - GmatCommandParamCount];
};

#endif      // Report_hpp
