//$Id: PhysicalModelFactory.cpp 9701 2011-07-14 20:36:28Z wendys-dev $
//------------------------------------------------------------------------------
//                            PhysicalModelFactory
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
// Author: Wendy Shoan
// Created: 2003/10/22
//
/**
 *  Implementation code for the PhysicalModelFactory class, responsible for
 *  creating PhysicalModel objects.
 */
//------------------------------------------------------------------------------
#include "gmatdefs.hpp"
#include "Factory.hpp"
#include "PhysicalModelFactory.hpp"
#include "PointMassForce.hpp"
#include "SolarRadiationPressure.hpp"
#include "DragForce.hpp" 
#include "GravityField.hpp" 
#include "RelativisticCorrection.hpp"


//---------------------------------
//  public methods
//---------------------------------

//------------------------------------------------------------------------------
//  CreatePhysicalModel(const wxString &ofType, const wxString &withName)
//------------------------------------------------------------------------------
/**
 * This method creates and returns an object of the requested PhysicalModel 
 * class.
 *
 * @param <ofType> the PhysicalModel object to create and return.
 * @param <withName> the name to give the newly-created PhysicalModel object.
 *
 * @note As of 2003/10/14, we are ignoring the withName parameter.  Use of this
 *       parameter may be added later.
 */
//------------------------------------------------------------------------------
PhysicalModel* PhysicalModelFactory::CreateObject(const wxString &ofType,
                                                  const wxString &withName)
{
   return CreatePhysicalModel(ofType, withName);
}


//------------------------------------------------------------------------------
//  CreatePhysicalModel(const wxString &ofType, const wxString &withName)
//------------------------------------------------------------------------------
/**
 * This method creates and returns an object of the requested PhysicalModel 
 * class.
 *
 * @param <ofType> the PhysicalModel object to create and return.
 * @param <withName> the name to give the newly-created PhysicalModel object.
 *
 * @note As of 2003/10/14, we are ignoring the withName parameter.  Use of this
 *       parameter may be added later.
 */
//------------------------------------------------------------------------------
PhysicalModel* PhysicalModelFactory::CreatePhysicalModel(
                                     const wxString &ofType,
                                     const wxString &withName)
{
   if (ofType == wxT("PointMassForce"))
       return new PointMassForce(withName);
   else if (ofType == wxT("SolarRadiationPressure"))
       return new SolarRadiationPressure(withName);
   else if (ofType == wxT("DragForce"))
       return new DragForce(withName);
   else if (ofType == wxT("GravityField"))
       return new GravityField(withName, wxT("Earth"));
   else if (ofType == wxT("RelativisticCorrection"))
       return new RelativisticCorrection(withName, wxT("Earth"));
   return NULL;
}


//------------------------------------------------------------------------------
//  PhysicalModelFactory()
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class PhysicalModelFactory
 * (default constructor).
 *
 *
 */
//------------------------------------------------------------------------------
PhysicalModelFactory::PhysicalModelFactory() 
   :
   Factory(Gmat::PHYSICAL_MODEL)
{
   if (creatables.empty())
   {
      creatables.push_back(wxT("PointMassForce"));
      creatables.push_back(wxT("GravityField"));
      creatables.push_back(wxT("SolarRadiationPressure"));
      creatables.push_back(wxT("DragForce"));
      creatables.push_back(wxT("RelativisticCorrection"));
   }
}

//------------------------------------------------------------------------------
//  PhysicalModelFactory(StringArray createList)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class PhysicalModelFactory
 * (constructor).
 *
 * @param <createList> initial list of creatable objects for this factory.
 *
 */
//------------------------------------------------------------------------------
PhysicalModelFactory::PhysicalModelFactory(StringArray createList) 
   :
   Factory(createList,Gmat::PHYSICAL_MODEL)
{
}

//------------------------------------------------------------------------------
//  PhysicalModelFactory(const PhysicalModelFactory &fact)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class PhysicalModelFactory
 * (copy constructor).
 *
 * @param <fact> the factory object to copy to wxT("this") factory.
 */
//------------------------------------------------------------------------------
PhysicalModelFactory::PhysicalModelFactory(const PhysicalModelFactory &fact) 
   :
   Factory(fact)
{
}

//------------------------------------------------------------------------------
//  PhysicalModelFactory& operator= (const PhysicalModelFactory &fact)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the PhysicalModelFactory class.
 *
 * @param <fact> the PhysicalModelFactory object whose data to assign to wxT("this")
 *               factory.
 *
 * @return wxT("this") PhysicalModelFactory with data of input factory fact.
 */
//------------------------------------------------------------------------------
PhysicalModelFactory& PhysicalModelFactory::operator= (
                                             const PhysicalModelFactory &fact)
{
   Factory::operator=(fact);
   return *this;
}

//------------------------------------------------------------------------------
// ~PhysicalModelFactory()
//------------------------------------------------------------------------------
/**
* Destructor for the PhysicalModelFactory base class.
 *
 */
//------------------------------------------------------------------------------
PhysicalModelFactory::~PhysicalModelFactory()
{
   // deletes handled by Factory destructor
}

//---------------------------------
//  protected methods
//---------------------------------

//---------------------------------
//  private methods
//---------------------------------

