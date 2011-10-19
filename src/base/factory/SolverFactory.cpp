//$Id: SolverFactory.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                            SolverFactory
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P
//
// Author: Darrel Conway
// Created: 2004/02/02  (Groundhog Day)
//
/**
 *  Implementation code for the SolverFactory class, responsible for creating
 *  targeters, optimizers, and other parameteric scanning objects.
 */
//------------------------------------------------------------------------------

#include "gmatdefs.hpp"
#include "SolverFactory.hpp"

// Headers for the supported Solvers
#include "DifferentialCorrector.hpp"
//#include "QuasiNewton.hpp"
//#include "SteepestDescent.hpp"
//#include "Broyden.hpp"
//#include "ParametricScanner.hpp"
//#include "MonteCarlo.hpp"

#include "MessageInterface.hpp"  // temporary



//---------------------------------
//  public methods
//---------------------------------

//------------------------------------------------------------------------------
//  Solver* CreateSolver(const wxString &ofType, const wxString &withName)
//------------------------------------------------------------------------------
/**
 * This method creates and returns an object of the requested Solver class. 
 *
 * @param <ofType> type of Solver object to create and return.
 * @param <withName> the name for the newly-created Solver object.
 * 
 * @return A pointer to the created object.
 */
//------------------------------------------------------------------------------
Solver* SolverFactory::CreateSolver(const wxString &ofType,
                                    const wxString &withName)
{
   if (ofType == wxT("DifferentialCorrector"))
      return new DifferentialCorrector(withName);
     
   //if (ofType == wxT("QuasiNewton")) 
   //   return new QuasiNewton(withName);
   //else if (ofType == wxT("SteepestDescent"))
   //   return new SteepestDescent(withName);
   // else if (ofType == wxT("Broyden"))
   //   return new Broyden(withName);
   //else if (ofType == wxT("ParametricScanner"))
   //   return new ParametricScanner(withName);
   //else if (ofType == wxT("MonteCarlo"))
   //   return new MonteCarlo(withName);
    
   return NULL;
}


//------------------------------------------------------------------------------
//  SolverFactory()
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class SolverFactory. 
 * (default constructor)
 */
//------------------------------------------------------------------------------
SolverFactory::SolverFactory() :
    Factory     (Gmat::SOLVER)
{
   if (creatables.empty())
   {
      //creatables.push_back(wxT("QuasiNewton"));
      //creatables.push_back(wxT("SteepestDescent"));
      creatables.push_back(wxT("DifferentialCorrector"));
      //creatables.push_back(wxT("Broyden"));
      //creatables.push_back(wxT("ParametricScanner"));
      //creatables.push_back(wxT("MonteCarlo"));
   }
}

//------------------------------------------------------------------------------
//  SolverFactory(StringArray createList)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class SolverFactory.
 *
 * @param <createList> list of creatable solver objects
 *
 */
//------------------------------------------------------------------------------
SolverFactory::SolverFactory(StringArray createList) :
Factory(createList, Gmat::SOLVER)
{
}


//------------------------------------------------------------------------------
//  SolverFactory(const SolverFactory& fact)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class SolverFactory.  (copy constructor)
 *
 * @param <fact> the factory object to copy to wxT("this") factory.
 */
//------------------------------------------------------------------------------
SolverFactory::SolverFactory(const SolverFactory& fact) :
    Factory     (fact)
{
   if (creatables.empty())
   {
      //creatables.push_back(wxT("QuasiNewton"));
      //creatables.push_back(wxT("SteepestDescent"));
      creatables.push_back(wxT("DifferentialCorrector"));
      //creatables.push_back(wxT("Broyden"));
      //creatables.push_back(wxT("ParametricScanner"));
      //creatables.push_back(wxT("MonteCarlo"));
   }
}


//------------------------------------------------------------------------------
//  CommandFactory& operator= (const CommandFactory& fact)
//------------------------------------------------------------------------------
/**
 * SolverFactory operator for the SolverFactory base class.
 *
 * @param <fact> the SolverFactory object that is copied.
 *
 * @return wxT("this") SolverFactory with data set to match the input factory (fact).
 */
//------------------------------------------------------------------------------
SolverFactory& SolverFactory::operator=(const SolverFactory& fact)
{
   Factory::operator=(fact);
   return *this;
}
    

//------------------------------------------------------------------------------
// ~SolverFactory()
//------------------------------------------------------------------------------
/**
 * Destructor for the SolverFactory base class.
 */
//------------------------------------------------------------------------------
SolverFactory::~SolverFactory()
{
}


//------------------------------------------------------------------------------
// bool DoesObjectTypeMatchSubtype(const wxString &theType,
//       const wxString &theSubtype)
//------------------------------------------------------------------------------
/**
 * Checks if a creatable solver type matches a subtype.
 *
 * @param theType The script identifier for the object type
 * @param theSubtype The subtype being checked
 *
 * @return true if the scripted type and subtype match, false if the type does
 *         not match the subtype
 */
//------------------------------------------------------------------------------
bool SolverFactory::DoesObjectTypeMatchSubtype(const wxString &theType,
      const wxString &theSubtype)
{
   bool retval = false;

   if ((theType == wxT("Boundary Value Solvers")) || (theType == wxT("Targeter")))
   {
      if (theSubtype == wxT("DifferentialCorrector"))
         retval = true;
   }

   return retval;
}



//---------------------------------
//  protected methods
//---------------------------------

//---------------------------------
//  private methods
//---------------------------------

