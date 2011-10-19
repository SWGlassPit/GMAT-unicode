//$Id: PropagatorFactory.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                            PropagatorFactory
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
// Created: 2003/10/14
//
/**
 *  Implementation code for the PropagatorFactory class, responsible
 *  for creating Propagator objects.
 */
//------------------------------------------------------------------------------
#include "gmatdefs.hpp"
#include "Factory.hpp"
#include "PropagatorFactory.hpp"
#include "RungeKutta89.hpp"
#include "DormandElMikkawyPrince68.hpp" 
#include "RungeKuttaFehlberg56.hpp" 
#include "PrinceDormand45.hpp" 
#include "PrinceDormand78.hpp" 
#include "AdamsBashforthMoulton.hpp"
#include "BulirschStoer.hpp"

// Ephemeris propagators
#ifdef __USE_SPICE__
#include "SPKPropagator.hpp"
#endif

// Not yet implemented:
//#include "Cowell.hpp"
/// @todo add others here for future builds

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
Propagator* PropagatorFactory::CreateObject(const wxString &ofType,
                                          const wxString &withName)
{
   return CreatePropagator(ofType, withName);
}

//------------------------------------------------------------------------------
//  CreatePropagator(const wxString &ofType, const wxString &withName)
//------------------------------------------------------------------------------
/**
 * This method creates and returns an object of the requested Propagator class 
 *
 * @param <ofType> the Propagator object to create and return.
 * @param <withName> the name to give the newly-created Propagator object.
 */
//------------------------------------------------------------------------------
Propagator* PropagatorFactory::CreatePropagator(const wxString &ofType,
                                                const wxString &withName)
{
   if (ofType == wxT("RungeKutta89"))
      return new RungeKutta89(withName);
   if (ofType == wxT("PrinceDormand78"))
      return new PrinceDormand78(withName);
   if (ofType == wxT("PrinceDormand45"))
      return new PrinceDormand45(withName);
//   if (ofType == wxT("DormandElMikkawyPrince68"))
//      return new DormandElMikkawyPrince68(withName);
   if (ofType == wxT("RungeKutta68"))
      return new DormandElMikkawyPrince68(withName);
//   if (ofType == wxT("RungeKuttaFehlberg56"))
//      return new RungeKuttaFehlberg56(withName);
   if (ofType == wxT("RungeKutta56"))
      return new RungeKuttaFehlberg56(withName);
   if (ofType == wxT("BulirschStoer"))
      return new BulirschStoer(withName);
   if (ofType == wxT("AdamsBashforthMoulton"))
      return new AdamsBashforthMoulton(withName);
//   if (ofType == wxT("Cowell"))
//      return new Cowell(withName);
   // EphemerisPropagators
   #ifdef __USE_SPICE__
      if (ofType == wxT("SPK"))
         return new SPKPropagator(withName);
   #endif

   /// @todo add others here as needed
   else
      return NULL;
}


//------------------------------------------------------------------------------
//  PropagatorFactory()
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class PropagatorFactory
 * (default constructor).
 *
 *
 */
//------------------------------------------------------------------------------
PropagatorFactory::PropagatorFactory() 
   :
   Factory(Gmat::PROPAGATOR)
{
   if (creatables.empty())
   {
      creatables.push_back(wxT("RungeKutta89"));
      creatables.push_back(wxT("PrinceDormand78"));
      creatables.push_back(wxT("PrinceDormand45"));
//      creatables.push_back(wxT("DormandElMikkawyPrince68"));
            creatables.push_back(wxT("RungeKutta68"));
//      creatables.push_back(wxT("RungeKuttaFehlberg56"));
            creatables.push_back(wxT("RungeKutta56"));
      creatables.push_back(wxT("BulirschStoer"));
      creatables.push_back(wxT("AdamsBashforthMoulton"));
//      creatables.push_back(wxT("Cowell"));
      
      #ifdef __USE_SPICE__
         creatables.push_back(wxT("SPK"));
      #endif
   }
}

//------------------------------------------------------------------------------
//  PropagatorFactory(StringArray createList)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class PropagatorFactory
 * (constructor).
 *
 * @param <createList> initial list of creatable objects
 *
 */
//------------------------------------------------------------------------------
PropagatorFactory::PropagatorFactory(StringArray createList) :
   Factory(createList, Gmat::PROPAGATOR)
{
}

//------------------------------------------------------------------------------
//  PropagatorFactory(const PropagatorFactory &fact)
//------------------------------------------------------------------------------
/**
   * This method creates an object of the class PropagatorFactory 
   * (copy constructor).
   *
   * @param <fact> the factory object to copy to wxT("this") factory.
   */
//------------------------------------------------------------------------------
PropagatorFactory::PropagatorFactory(const PropagatorFactory &fact) 
   :
   Factory(fact)
{

}

//------------------------------------------------------------------------------
//  PropagatorFactory& operator= (const PropagatorFactory &fact)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the PropagatorFactory base class.
 *
 * @param <fact> the PropagatorFactory object whose data to assign to wxT("this")
 *  factory.
 *
 * @return wxT("this") PropagatorFactory with data of input factory fact.
 */
//------------------------------------------------------------------------------
PropagatorFactory& PropagatorFactory::operator= (const PropagatorFactory &fact)
{
   Factory::operator=(fact);
   return *this;
}

//------------------------------------------------------------------------------
// ~PropagatorFactory()
//------------------------------------------------------------------------------
/**
   * Destructor for the PropagatorFactory base class.
   *
   */
//------------------------------------------------------------------------------
PropagatorFactory::~PropagatorFactory()
{
   // deletes handled by Factory destructor
}

//---------------------------------
//  protected methods
//---------------------------------

//---------------------------------
//  private methods
//---------------------------------
