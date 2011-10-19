//$Id: AxisSystemFactory.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                            AxisSystemFactory
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G and MOMS Task order 124
//
// Author: Wendy Shoan
// Created: 2004/12/23
//
/**
 *  Implementation code for the AxisSystemFactory class, responsible for
 *  creating AxisSystem objects.
 */
//------------------------------------------------------------------------------
#include "gmatdefs.hpp"
#include "Factory.hpp"
#include "AxisSystemFactory.hpp"
#include "AxisSystem.hpp"
#include "MessageInterface.hpp"
#include "MJ2000EqAxes.hpp"
#include "MJ2000EcAxes.hpp"
#include "BodyFixedAxes.hpp"
#include "BodyInertialAxes.hpp"
#include "EquatorAxes.hpp"
#include "ObjectReferencedAxes.hpp"
#include "TOEEqAxes.hpp"
#include "MOEEqAxes.hpp"
#include "TODEqAxes.hpp"
#include "MODEqAxes.hpp"
#include "TOEEcAxes.hpp"
#include "MOEEcAxes.hpp"
#include "TODEcAxes.hpp"
#include "MODEcAxes.hpp"
#include "GeocentricSolarEclipticAxes.hpp"
#include "GeocentricSolarMagneticAxes.hpp"
#include "TopocentricAxes.hpp"

//---------------------------------
//  public methods
//---------------------------------

//------------------------------------------------------------------------------
//  CreateObject(const wxString &ofType, const wxString &withName)
//------------------------------------------------------------------------------
/**
 * This method creates and returns an object of the requested ODEModel class
 * in generic way.
 *
 * @param <ofType> the ODEModel object to create and return.
 * @param <withName> the name to give the newly-created ODEModel object.
 *
 */
//------------------------------------------------------------------------------
AxisSystem* AxisSystemFactory::CreateObject(const wxString &ofType,
                                            const wxString &withName)
{
   return CreateAxisSystem(ofType, withName);
}

//------------------------------------------------------------------------------
// AxisSystem* CreateAxisSystem(wxString ofType, wxString withName)
//------------------------------------------------------------------------------
/**
 * This method creates and returns an AxisSystem object of the requested 
 * type.
 *
 * @param <ofType>   the AxisSystem object to create and return.
 * @param <withName> the name to give the newly-created AxisSystem object.
 *
 */
//------------------------------------------------------------------------------
AxisSystem*
AxisSystemFactory::CreateAxisSystem(const wxString &ofType,
                                    const wxString &withName)
{
   AxisSystem *withAxes = NULL;
   if (ofType == wxT("MJ2000Eq"))
   {
      withAxes = new MJ2000EqAxes(withName);
   }
   else if (ofType == wxT("MJ2000Ec"))
   {
      withAxes = new MJ2000EcAxes(withName);
   }
   else if (ofType == wxT("TOEEq"))
   {
      withAxes = new TOEEqAxes(withName);
   }
   else if (ofType == wxT("TOEEc"))
   {
      withAxes = new TOEEcAxes(withName);
   }
   else if (ofType == wxT("MOEEq"))
   {
      withAxes = new MOEEqAxes(withName);
   }
   else if (ofType == wxT("MOEEc"))
   {
      withAxes = new MOEEcAxes(withName);
   }
   else if (ofType == wxT("TODEq"))
   {
      withAxes = new TODEqAxes(withName);
   }
   else if (ofType == wxT("TODEc"))
   {
      withAxes = new TODEcAxes(withName);
   }
   else if (ofType == wxT("MODEq"))
   {
      withAxes = new MODEqAxes(withName);
   }
   else if (ofType == wxT("MODEc"))
   {
      withAxes = new MODEcAxes(withName);
   }
   else if (ofType == wxT("ObjectReferenced"))  
   {
      withAxes = new ObjectReferencedAxes(withName);
   }
   else if (ofType == wxT("Equator"))
   {
      withAxes = new EquatorAxes(withName);
   }
   else if (ofType == wxT("BodyFixed"))
   {
      withAxes = new BodyFixedAxes(withName);
   }
   else if (ofType == wxT("BodyInertial"))
   {
      withAxes = new BodyInertialAxes(withName);
   }
   else if ((ofType == wxT("GSE")) || (ofType == wxT("GeocentricSolarEcliptic")))
   {
      withAxes = new GeocentricSolarEclipticAxes(withName);
   }
   else if ((ofType == wxT("GSM")) || (ofType == wxT("GeocentricSolarMagnetic")))
   {
      withAxes = new GeocentricSolarMagneticAxes(withName);
   }
   else if (ofType == wxT("Topocentric"))
   {
      withAxes = new TopocentricAxes(withName);
   }
   return withAxes;
}


//------------------------------------------------------------------------------
//  AxisSystemFactory()
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class AxisSystemFactory
 * (default constructor).
 *
 *
 */
//------------------------------------------------------------------------------
AxisSystemFactory::AxisSystemFactory() :
Factory(Gmat::AXIS_SYSTEM) //loj: 1/19/05 Changed from ATMOSPHERE
{
   if (creatables.empty())
   {
      creatables.push_back(wxT("MJ2000Eq"));
      creatables.push_back(wxT("MJ2000Ec"));
      creatables.push_back(wxT("TOEEq"));
      creatables.push_back(wxT("TOEEc"));
      creatables.push_back(wxT("MOEEq"));
      creatables.push_back(wxT("MOEEc"));
      creatables.push_back(wxT("TODEq"));
      creatables.push_back(wxT("TODEc"));
      creatables.push_back(wxT("MODEq"));
      creatables.push_back(wxT("MODEc"));
      creatables.push_back(wxT("ObjectReferenced"));
      creatables.push_back(wxT("Equator"));
      creatables.push_back(wxT("BodyFixed"));
      creatables.push_back(wxT("BodyInertial"));
      creatables.push_back(wxT("GSE"));
      creatables.push_back(wxT("GSM"));
      creatables.push_back(wxT("Topocentric"));
   }
}

//------------------------------------------------------------------------------
//  AxisSystemFactory(StringArray createList)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class AxisSystemFactory
 * (constructor).
 *
 * @param <createList> initial list of creatable objects for this factory.
 *
 */
//------------------------------------------------------------------------------
AxisSystemFactory::AxisSystemFactory(StringArray createList) :
Factory(createList,Gmat::AXIS_SYSTEM)
{
}

//------------------------------------------------------------------------------
//  AxisSystemFactory(const AxisSystemFactory& fact)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class AxisSystemFactory
 * (copy constructor).
 *
 * @param <fact> the factory object to copy to wxT("this") factory.
 */
//------------------------------------------------------------------------------
AxisSystemFactory::AxisSystemFactory(const AxisSystemFactory& fact) :
Factory(fact)
{
}

//------------------------------------------------------------------------------
//  AxisSystemFactory& operator= (const AxisSystemFactory& fact)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the AxisSystemFactory class.
 *
 * @param <fact> the AxisSystemFactory object whose data to assign to wxT("this")
 *               factory.
 *
 * @return wxT("this") AxisSystemFactory with data of input factory fact.
 */
//------------------------------------------------------------------------------
AxisSystemFactory& AxisSystemFactory::operator= (
                                             const AxisSystemFactory& fact)
{
   Factory::operator=(fact);
   return *this;
}

//------------------------------------------------------------------------------
// ~AxisSystemFactory()
//------------------------------------------------------------------------------
/**
* Destructor for the AxisSystemFactory base class.
 *
 */
//------------------------------------------------------------------------------
AxisSystemFactory::~AxisSystemFactory()
{
   // deletes handled by Factory base class
}

//---------------------------------
//  protected methods
//---------------------------------

//---------------------------------
//  private methods
//---------------------------------





