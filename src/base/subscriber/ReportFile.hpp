//$Id: ReportFile.hpp 9907 2011-09-26 14:38:05Z wendys-dev $
//------------------------------------------------------------------------------
//                                  ReportFile
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
// Author: LaMont Ruley
// Created: 2003/10/23
// Modified:  2004/7/16 by Allison Greene to include copy constructor
//                         and operator=
//
/**
 * Definition for the ReportFile class.
 */
//------------------------------------------------------------------------------


#ifndef ReportFile_hpp
#define ReportFile_hpp


#include "Subscriber.hpp"
#include <fstream>

#include "Parameter.hpp"
#include <map>
#include <iostream>
#include <iomanip>

class GMAT_API ReportFile : public Subscriber
{
public:
   ReportFile(const wxString &typeName, const wxString &name,
              const wxString &fileName = wxT(""), 
              Parameter *firstParam = NULL);
   
   virtual ~ReportFile(void);
   
   ReportFile(const ReportFile &);
   ReportFile& operator=(const ReportFile&);
   
   // methods for this class
   wxString          GetDefaultFileName();
   wxString          GetPathAndFileName();
   Integer              GetNumParameters();
   bool                 AddParameter(const wxString &paramName, Integer index);
   bool                 AddParameterForTitleOnly(const wxString &paramName);
   bool                 WriteData(WrapperArray dataArray);
   
   // methods inherited from GmatBase
   virtual bool         Initialize();
   
   virtual GmatBase*    Clone(void) const;
   virtual void         Copy(const GmatBase* orig);
   
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
   
   virtual bool         GetBooleanParameter(const Integer id) const;
   virtual bool         SetBooleanParameter(const Integer id,
                                            const bool value);
   virtual bool         GetBooleanParameter(const wxString &label) const;
   virtual bool         SetBooleanParameter(const wxString &label,
                                            const bool value);
   virtual Integer      GetIntegerParameter(const Integer id) const;
   virtual Integer      SetIntegerParameter(const Integer id,
                                            const Integer value);
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
   
   virtual wxString  GetOnOffParameter(const Integer id) const;
   virtual bool         SetOnOffParameter(const Integer id, 
                                          const wxString &value);
   virtual wxString  GetOnOffParameter(const wxString &label) const;
   virtual bool         SetOnOffParameter(const wxString &label, 
                                          const wxString &value);
   
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name = wxT(""));
   
   virtual bool         HasRefObjectTypeArray();
   virtual const ObjectTypeArray&
                        GetRefObjectTypeArray();
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   
   // methods for setting up the items to subscribe
   virtual const StringArray&
                        GetWrapperObjectNameArray();
   
protected:
   /// Name of the output path
   wxString          outputPath;
   /// Name of the report file
   wxString          filename;
   /// Default file name of the report file when it is not set
   wxString          defFileName;
   /// Full file name with path
   wxString          fullPathName;
   /// Precision for output of real data
   Integer              precision;  
   /// Width of column
   Integer              columnWidth;   
   /// Write the headers on the top of the column
   bool                 writeHeaders;
   /// Left justify
   bool                 leftJustify;
   /// Fill right field with 0
   bool                 zeroFill;
   
   /// output data stream
   std::ofstream        dstream;
   std::vector<Parameter*> mParams;
   
   Integer              mNumParams;
   StringArray          mParamNames;
   StringArray          mAllRefObjectNames;
   Integer              lastUsedProvider;
   Real                 mLastReportTime;
   bool                 usedByReport;
   bool                 calledByReport;
   bool                 initial;
   
   virtual bool         OpenReportFile(void);
   void                 ClearParameters();
   void                 WriteHeaders();
   Integer              WriteMatrix(StringArray *output, Integer param,
                                    const Rmatrix &rmat, UnsignedInt &maxRow,
                                    Integer defWidth);
   
   // methods inherited from Subscriber
   virtual bool         Distribute(Integer len);
   virtual bool         Distribute(const Real * dat, Integer len);
   
   virtual bool         IsNotANumber(Real rval);

   enum
   {
      FILENAME = SubscriberParamCount,
      PRECISION,
      ADD,
      WRITE_HEADERS,
      LEFT_JUSTIFY,
      ZERO_FILL,
      COL_WIDTH,
      WRITE_REPORT,
      ReportFileParamCount  /// Count of the parameters for this class
   };

private:

   static const wxString
      PARAMETER_TEXT[ReportFileParamCount - SubscriberParamCount];
   static const Gmat::ParameterType
      PARAMETER_TYPE[ReportFileParamCount - SubscriberParamCount];
   
};


#endif
