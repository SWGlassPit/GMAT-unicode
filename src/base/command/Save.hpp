//$Id: Save.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  Save
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Darrel J. Conway
// Created: 2004/02/26
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
/**
 * Class implementation for the save command
 */
//------------------------------------------------------------------------------


#ifndef Save_hpp
#define Save_hpp

#include "GmatCommand.hpp"
#include <fstream>

/**
 * Command used to write objects to ASCII files.
 */
class GMAT_API Save : public GmatCommand
{
public:
   Save();
   virtual ~Save();
   Save(const Save& sv);
   Save&                operator=(const Save&);
   
   // inherited from GmatCommand
   virtual bool         Execute();
   virtual void         RunComplete();
   
   // inherited from GmatBase
   virtual bool         TakeAction(const wxString &action,  
                                   const wxString &actionData = wxT(""));
   virtual wxString  GetRefObjectName(const Gmat::ObjectType type) const;
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool         SetRefObjectName(const Gmat::ObjectType type,
                                         const wxString &name);
   virtual bool         Initialize();
   
   virtual GmatBase*    Clone() const;
   
   // Parameter accessors
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value);
   virtual bool         SetStringParameter(const wxString &label, 
                                           const wxString &value);
   virtual wxString  GetStringParameter(const Integer id,
                                           const Integer index) const;
   virtual wxString  GetStringParameter(const wxString &label,
                                           const Integer index) const;
   virtual const StringArray& 
                        GetStringArrayParameter(const Integer id) const;
   virtual const wxString&
                        GetGeneratingString(Gmat::WriteMode mode,
                                            const wxString &prefix,
                                            const wxString &useName);
      
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   
protected:
   // Parameter IDs
   enum  
   {
      OBJECT_NAMES = GmatCommandParamCount,
      SaveParamCount
   };
   
   /// Name of the save file -- for now, it is objectName.objectType
   StringArray          fileNameArray;
   /// Toggle to allow multiple writes
   bool                 appendData;
   /// Toggle to tell if file was written this run
   bool                 wasWritten;
   /// Name of the objects that are written
   StringArray          objNameArray;
   /// Pointer to the objects
   ObjectArray          objArray;
   /// Toggle to show or hide emply fields
   bool                 writeVerbose;
   /// File streams used for the output
   std::ofstream        *fileArray;
   
   void  UpdateOutputFileNames(Integer index, const wxString objName);
   void  WriteObject(UnsignedInt i, GmatBase *o);
   
   static const wxString
      PARAMETER_TEXT[SaveParamCount - GmatCommandParamCount];
   static const Gmat::ParameterType
      PARAMETER_TYPE[SaveParamCount - GmatCommandParamCount];
};

#endif // Save_hpp
