//$Id: CelestialBodyFactory.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                            CelestialBodyFactory
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
// Created: 2004/01/29
//
/**
 *  Implementation code for the CelestialBodyFactory class, responsible for
 *  creating CelestialBody objects.
 */
//------------------------------------------------------------------------------
#include "gmatdefs.hpp"
#include "Factory.hpp"
#include "CelestialBodyFactory.hpp"
#include "Star.hpp"
#include "Planet.hpp"
#include "Moon.hpp"
#include "Comet.hpp"
#include "Asteroid.hpp"


//---------------------------------
//  public methods
//---------------------------------

//------------------------------------------------------------------------------
//  CreateCelestialBody(wxString ofType, wxString withName)
//------------------------------------------------------------------------------
/**
 * This method creates and returns an object of the requested CelestialBody 
 * class.
 *
 * @param <ofType> the CelestialBody object to create and return.
 * @param <withName> the name to give the newly-created CelestialBody object.
 *
 * @return a new 
 *
 */
//------------------------------------------------------------------------------
CelestialBody* CelestialBodyFactory::CreateCelestialBody(
                                     const wxString &ofType,
                                     const wxString &withName)
{
   if (ofType == wxT("Star"))
      return new Star(withName);
   else if (ofType == wxT("Planet"))
      return new Planet(withName);
   else if (ofType == wxT("Moon"))
      return new Moon(withName);
   else if (ofType == wxT("Comet"))
      return new Comet(withName);
   else if (ofType == wxT("Asteroid"))
      return new Asteroid(withName);
   // add more here ??  KBOs?  .......
   else
   {
      return NULL;   // doesn't match any known type of command
   }
}

//------------------------------------------------------------------------------
//  CelestialBodyFactory()
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class CelestialBodyFactory
 * (default constructor).
 *
 *
 */
//------------------------------------------------------------------------------
CelestialBodyFactory::CelestialBodyFactory() :
Factory(Gmat::CELESTIAL_BODY)
{
   if (creatables.empty())
   {
      creatables.push_back(wxT("Star"));
      creatables.push_back(wxT("Planet"));
      creatables.push_back(wxT("Moon"));
      creatables.push_back(wxT("Comet"));
      creatables.push_back(wxT("Asteroid"));
   }
}

//------------------------------------------------------------------------------
//  CelestialBodyFactory(StringArray createList)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class CelestialBodyFactory
 * (constructor).
 *
 * @param <createList> initial list of creatable objects for this factory.
 *
 */
//------------------------------------------------------------------------------
CelestialBodyFactory::CelestialBodyFactory(const StringArray &createList) :
Factory(createList,Gmat::CELESTIAL_BODY)
{
   if (creatables.empty())
   {
      creatables.push_back(wxT("Star"));
      creatables.push_back(wxT("Planet"));
      creatables.push_back(wxT("Moon"));
      creatables.push_back(wxT("Comet"));
      creatables.push_back(wxT("Asteroid"));
   }
}

//------------------------------------------------------------------------------
//  CelestialBodyFactory(const CelestialBodyFactory &fact)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class CelestialBodyFactory
 * (copy constructor).
 *
 * @param <fact> the factory object to copy to wxT("this") factory.
 */
//------------------------------------------------------------------------------
CelestialBodyFactory::CelestialBodyFactory(const CelestialBodyFactory &fact) :
Factory(fact)
{
   if (creatables.empty())
   {
      creatables.push_back(wxT("Star"));
      creatables.push_back(wxT("Planet"));
      creatables.push_back(wxT("Moon"));
      creatables.push_back(wxT("Comet"));
      creatables.push_back(wxT("Asteroid"));
   }
}

//------------------------------------------------------------------------------
//  CelestialBodyFactory& operator= (const CelestialBodyFactory &fact)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the CelestialBodyFactory class.
 *
 * @param <fact> the CelestialBodyFactory object whose data to assign to wxT("this")
 *               factory.
 *
 * @return wxT("this") CelestialBodyFactory with data of input factory fact.
 */
//------------------------------------------------------------------------------
CelestialBodyFactory& CelestialBodyFactory::operator= (
                                             const CelestialBodyFactory &fact)
{
   if (this == &fact) return *this;
   Factory::operator=(fact);
   if (creatables.empty())
   {
      creatables.push_back(wxT("Star"));
      creatables.push_back(wxT("Planet"));
      creatables.push_back(wxT("Moon"));
      creatables.push_back(wxT("Comet"));
      creatables.push_back(wxT("Asteroid"));
   }
   return *this;
}

//------------------------------------------------------------------------------
// ~CelestialBodyFactory()
//------------------------------------------------------------------------------
/**
* Destructor for the CelestialBodyFactory base class.
 *
 */
//------------------------------------------------------------------------------
CelestialBodyFactory::~CelestialBodyFactory()
{
   // deletes handled by Factory destructor
}

//---------------------------------
//  protected methods
//---------------------------------

//---------------------------------
//  private methods
//---------------------------------





