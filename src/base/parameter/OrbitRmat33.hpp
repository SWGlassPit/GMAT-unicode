//$Id$
//------------------------------------------------------------------------------
//                                  OrbitRmat33
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
// Created: 2009.03.30
//
/**
 * Declares SpacecraftState Rmatrix data class.
 */
//------------------------------------------------------------------------------
#ifndef OrbitRmat33_hpp
#define OrbitRmat33_hpp

#include "gmatdefs.hpp"
#include "Rmat33Var.hpp"
#include "OrbitData.hpp"


class GMAT_API OrbitRmat33 : public Rmat33Var, public OrbitData
{
public:
   
   OrbitRmat33(const wxString &name, const wxString &typeStr, 
               GmatBase *obj, const wxString &desc, const wxString &unit,
               GmatParam::DepObject depObj, bool isSettable = false);
   OrbitRmat33(const OrbitRmat33 &copy);
   OrbitRmat33& operator=(const OrbitRmat33 &right);
   virtual ~OrbitRmat33();
   
   // methods inherited from Parameter
   virtual const Rmatrix& EvaluateRmatrix();
   
   virtual Integer      GetNumRefObjects() const;
   virtual CoordinateSystem* GetInternalCoordSystem();
   virtual void         SetSolarSystem(SolarSystem *ss);
   virtual void         SetInternalCoordSystem(CoordinateSystem *ss);
   virtual bool         AddRefObject(GmatBase*obj, bool replaceName = false);
   virtual bool         Validate();
   virtual bool         Initialize();
   
   // methods inherited from GmatBase
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   
   virtual wxString  GetRefObjectName(const Gmat::ObjectType type) const;
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool         SetRefObjectName(const Gmat::ObjectType type,
                                         const wxString &name);
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name = wxT(""));

protected:

};

#endif //OrbitRmat33_hpp
