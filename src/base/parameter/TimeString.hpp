//$Id: TimeString.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                TimeString
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
// Created: 2006/03/09
//
/**
 * Declares TimeString class which provides base class for time realated String
 * Parameters
 */
//------------------------------------------------------------------------------
#ifndef TimeString_hpp
#define TimeString_hpp

#include "gmatdefs.hpp"
#include "StringVar.hpp"
#include "TimeData.hpp"


class GMAT_API TimeString : public StringVar, public TimeData
{
public:

   TimeString(const wxString &name, const wxString &typeStr, 
            GmatBase *obj, const wxString &desc, const wxString &unit);
   TimeString(const TimeString &copy);
   TimeString& operator=(const TimeString &right);
   virtual ~TimeString();

   // methods inherited from Parameter
   virtual const wxString& EvaluateString();
   
   virtual Integer GetNumRefObjects() const;
   virtual bool Validate();
   virtual bool Initialize();
   virtual bool AddRefObject(GmatBase *obj, bool replaceName = false);
   
   // methods inherited from GmatBase
   virtual bool RenameRefObject(const Gmat::ObjectType type,
                                const wxString &oldName,
                                const wxString &newName);
   
   virtual wxString GetRefObjectName(const Gmat::ObjectType type) const;
   virtual const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool SetRefObjectName(const Gmat::ObjectType type,
                                 const wxString &name);
   virtual GmatBase* GetRefObject(const Gmat::ObjectType type,
                                  const wxString &name);
   virtual bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                             const wxString &name = wxT(""));
   virtual const wxString&
                    GetGeneratingString(Gmat::WriteMode mode = Gmat::SCRIPTING,
                                        const wxString &prefix = wxT(""),
                                        const wxString &useName = wxT(""));
protected:
   

};

#endif // TimeString_hpp
