//$Id: PlanetReal.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                PlanetReal
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
// Created: 2004/12/13
//
/**
 * Declares PlanetReal class which provides base class for the planet
 * realated Real Parameters
 */
//------------------------------------------------------------------------------
#ifndef PlanetReal_hpp
#define PlanetReal_hpp

#include "gmatdefs.hpp"
#include "RealVar.hpp"
#include "PlanetData.hpp"


class GMAT_API PlanetReal : public RealVar, public PlanetData
{
public:

   PlanetReal(const wxString &name, const wxString &typeStr, 
              GmatBase *obj, const wxString &desc,
              const wxString &unit, Gmat::ObjectType ownerType,
              GmatParam::DepObject depObj);
   PlanetReal(const PlanetReal &copy);
   PlanetReal& operator=(const PlanetReal &right);
   virtual ~PlanetReal();

   // methods inherited from Parameter
   virtual Real EvaluateReal();

   virtual Integer GetNumRefObjects() const;
   virtual bool AddRefObject(GmatBase *obj, bool replaceName = false);
   virtual void SetSolarSystem(SolarSystem *ss);
   virtual void SetInternalCoordSystem(CoordinateSystem *ss);
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

#endif // PlanetReal_hpp
