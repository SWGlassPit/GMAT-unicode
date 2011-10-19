//$Id$
//------------------------------------------------------------------------------
//                                  BallisticMassReal
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc.
//
// Author: Linda Jun
// Created: 2009.03.20
//
/**
 * Declares BallisticMass real data class.
 */
//------------------------------------------------------------------------------
#ifndef BallisticMassReal_hpp
#define BallisticMassReal_hpp

#include "gmatdefs.hpp"
#include "RealVar.hpp"
#include "SpacecraftData.hpp"


class GMAT_API BallisticMassReal : public RealVar, public SpacecraftData
{
public:

   BallisticMassReal(const wxString &name, const wxString &typeStr, 
                     GmatBase *obj, const wxString &desc,
                     const wxString &unit);
   BallisticMassReal(const BallisticMassReal &copy);
   BallisticMassReal& operator=(const BallisticMassReal &right);
   virtual ~BallisticMassReal();
   
   // methods inherited from Parameter
   virtual Real EvaluateReal();
   
   virtual Integer GetNumRefObjects() const;
   virtual bool AddRefObject(GmatBase*obj, bool replaceName = false);
   virtual bool Validate();
   virtual bool Initialize();
   
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

protected:

};


#endif /*BallisticMassReal_hpp*/
