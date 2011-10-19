//$Id: MathFactory.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              MathFactory
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P.
//
// Author: Linda Jun, NASA/GSFC
// Created: 2006/04/05
//
/**
 * Implementation code for the MathFactory class, responsible for creating 
 * MathNode objects.
 */
//------------------------------------------------------------------------------

#include "MathFactory.hpp"
#include "StringUtil.hpp"          // for Capitalize()
#include "MessageInterface.hpp"

// include list of MathNode classes here
#include "MathElement.hpp"
#include "Abs.hpp"
#include "Acos.hpp"
#include "Asin.hpp"
#include "Atan.hpp"
#include "Atan2.hpp"
#include "Add.hpp"
#include "Cos.hpp"
#include "DegToRad.hpp"
#include "Determinant.hpp"
#include "Divide.hpp"
#include "Exp.hpp"
#include "Inverse.hpp"
#include "Log.hpp"
#include "Log10.hpp"
#include "Multiply.hpp"
#include "Negate.hpp"
#include "Norm.hpp"
#include "Power.hpp"
#include "RadToDeg.hpp"
#include "Sin.hpp"
#include "Sqrt.hpp"
#include "Subtract.hpp"
#include "Tan.hpp"
#include "Transpose.hpp"
#include "FunctionRunner.hpp"      // for inline GmatFunction

//#define DEBUG_MATH_FACTORY 1

//---------------------------------
//  public methods
//---------------------------------

//------------------------------------------------------------------------------
//  CreateMathNode(const wxString &ofType, const wxString &withName)
//------------------------------------------------------------------------------
/**
 * This method creates and returns an object of the requested MathNode class 
 *
 * @param <ofType>   the MathNode object to create and return.
 * @param <withName> the name of the new object.
 *
 * @return The new object.
 */
//------------------------------------------------------------------------------
MathNode* MathFactory::CreateMathNode(const wxString &ofType,
                                      const wxString &withName)
{
   MathNode *mathNode = NULL;

   // The FIRST letter of function name can be either lower or upper case,
   // so capitalize the first letter of the type before checking.
   // eg) cos, Cos, add, Add.
   
   wxString newType = GmatStringUtil::Capitalize(ofType);
   
   #if DEBUG_MATH_FACTORY
   MessageInterface::ShowMessage
      (wxT("MathFactory::CreateMathNode() ofType=%s, newType=%s, withName=%s\n"),
       ofType.c_str(), newType.c_str(), withName.c_str());
   #endif
   
   // Leaf node
   if (ofType == wxT("MathElement"))
      mathNode = new MathElement(ofType, withName);
   
   // Simple math operations
   else if (newType == wxT("Add"))         // Add(x,y) or x+y
      mathNode = new Add(withName);
   else if (newType == wxT("Subtract"))    // Subtract(x,y) or x-y
      mathNode = new Subtract(withName);
   else if (newType == wxT("Multiply"))    // Multiply(x,y) or x*y
      mathNode = new Multiply(withName);
   else if (newType == wxT("Divide"))      // Divide(x,y) or x/y
      mathNode = new Divide(withName);
   else if (newType == wxT("Negate"))      // Negate(x)
      mathNode = new Negate(withName);
   else if (newType == wxT("Sqrt"))        // Sqrt(x)
      mathNode = new Sqrt(withName);
   else if (newType == wxT("Abs"))         // Abs(x)
      mathNode = new Abs(withName);
   
   // Power, Log functions
   else if (newType == wxT("Power"))       // power(x,y) or x^y
      mathNode = new Power(withName);
   else if (newType == wxT("Exp"))         // exp(x)
      mathNode = new Exp(withName);
   else if (newType == wxT("Log"))         // log(x)
      mathNode = new Log(withName);
   else if (newType == wxT("Log10"))       // log10(x)
      mathNode = new Log10(withName);
   
   // Matrix functions
   else if (newType == wxT("Transpose"))   // transpose(m) or m'
      mathNode = new Transpose(withName);
   else if (newType == wxT("Det"))         // det(m)
      mathNode = new Determinant(withName);
   else if (newType == wxT("Inv"))         // inv(m)
      mathNode = new Inverse(withName);
   else if (newType == wxT("Norm"))        // norm(m)
      mathNode = new Norm(withName);
   
   // Trigonometric functions
   else if (newType == wxT("Sin"))         // sin(x)
      mathNode = new Sin(withName);
   else if (newType == wxT("Cos"))         // cos(x)
      mathNode = new Cos(withName);
   else if (newType == wxT("Tan"))         // tan(x)
      mathNode = new Tan(withName);
   else if (newType == wxT("Asin"))        // asin(x)
      mathNode = new Asin(withName);
   else if (newType == wxT("Acos"))        // acos(x)
      mathNode = new Acos(withName);
   else if (newType == wxT("Atan"))        // atan(x)
      mathNode = new Atan(withName);
   else if (newType == wxT("Atan2"))       // atan2(y,x)
      mathNode = new Atan2(withName);
   
   // Unit conversion functions
   else if (newType == wxT("DegToRad") || newType == wxT("Deg2Rad"))
      mathNode = new DegToRad(withName);
   else if (newType == wxT("RadToDeg") || newType == wxT("Rad2Deg"))
      mathNode = new RadToDeg(withName);
   
   else if (newType == wxT("FunctionRunner"))
      mathNode = new FunctionRunner(withName);
   
   return mathNode;
}


//------------------------------------------------------------------------------
//  MathFactory()
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class MathFactory.
 * (default constructor)
 */
//------------------------------------------------------------------------------
MathFactory::MathFactory()
   : Factory(Gmat::MATH_NODE)
{
   isCaseSensitive = true;
   
   if (creatables.empty())
      BuildCreatables();
}


//------------------------------------------------------------------------------
//  MathFactory(StringArray createList)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class MathFactory.
 *
 * @param <createList> list of creatable MathNode objects
 */
//------------------------------------------------------------------------------
MathFactory::MathFactory(StringArray createList)
   : Factory(createList, Gmat::MATH_NODE)
{
   isCaseSensitive = true;
}


//------------------------------------------------------------------------------
//  MathFactory(const MathFactory &fact)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class MathFactory (called by
 * copy constructors of derived classes).  (copy constructor)
 *
 * @param <fact> the factory object to copy to wxT("this") factory.
 */
//------------------------------------------------------------------------------
MathFactory::MathFactory(const MathFactory &fact)
   : Factory(fact)
{
   isCaseSensitive = true;
   
   if (creatables.empty())
      BuildCreatables();
}


//------------------------------------------------------------------------------
//  MathFactory& operator= (const MathFactory &fact)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the MathFactory base class.
 *
 * @param <fact> the MathFactory object whose data to assign to wxT("this") factory.
 *
 * @return wxT("this") MathFactory with data of input factory fact.
 */
//------------------------------------------------------------------------------
MathFactory& MathFactory::operator=(const MathFactory &fact)
{
   Factory::operator=(fact);
   isCaseSensitive = true;
   return *this;
}


//------------------------------------------------------------------------------
// ~MathFactory()
//------------------------------------------------------------------------------
/**
 * Destructor for the MathFactory class.
 */
//------------------------------------------------------------------------------
MathFactory::~MathFactory()
{
   // deletes handled by Factory destructor
}

//---------------------------------
//  protected methods
//---------------------------------

//------------------------------------------------------------------------------
/**
 * Fills in creatable objects
 */
//------------------------------------------------------------------------------
void MathFactory::BuildCreatables()
{
   // The FIRST letter of function name can be either lower or upper case.
   
   // Math element
   creatables.push_back(wxT("MathElement"));
   
   // Simple math operations
   creatables.push_back(wxT("Negate"));
   creatables.push_back(wxT("Add"));
   creatables.push_back(wxT("Subtract"));
   creatables.push_back(wxT("Multiply"));
   creatables.push_back(wxT("Divide"));
   
   // Math functions
   creatables.push_back(wxT("Sqrt"));
   creatables.push_back(wxT("Abs"));
   creatables.push_back(wxT("Power"));
   creatables.push_back(wxT("Exp"));
   creatables.push_back(wxT("Log"));
   creatables.push_back(wxT("Log10"));
   
   // Matrix functions
   creatables.push_back(wxT("Transpose"));
   creatables.push_back(wxT("Det"));
   creatables.push_back(wxT("Inv"));
   creatables.push_back(wxT("Norm"));
   
   // Trigonometric functions
   creatables.push_back(wxT("Sin"));
   creatables.push_back(wxT("Cos"));
   creatables.push_back(wxT("Tan"));
   creatables.push_back(wxT("Asin"));
   creatables.push_back(wxT("Acos"));
   creatables.push_back(wxT("Atan"));
   creatables.push_back(wxT("Atan2"));
   
   // Unit conversion functions
   creatables.push_back(wxT("DegToRad"));  
   creatables.push_back(wxT("RadToDeg"));
   creatables.push_back(wxT("Deg2Rad"));  
   creatables.push_back(wxT("Rad2Deg"));
   
   // GmatFunction
   creatables.push_back(wxT("FunctionRunner"));  
}

