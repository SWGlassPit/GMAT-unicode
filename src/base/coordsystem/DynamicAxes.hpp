//$Id: DynamicAxes.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  DynamicAxes
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under 
// MOMS Task order 124.
//
// Author: Wendy C. Shoan/GSFC/MAB
// Created: 2004/12/28
//
/**
 * Definition of the DynamicAxes class.  This is the base class for those
 * AxisSystem classes that implement dynamic systems.
 *
 */
//------------------------------------------------------------------------------

#ifndef DynamicAxes_hpp
#define DynamicAxes_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "AxisSystem.hpp"
#include "A1Mjd.hpp"

class GMAT_API DynamicAxes : public AxisSystem
{
public:

   // default constructor
   DynamicAxes(const wxString &itsType,
               const wxString &itsName = wxT(""));
   // copy constructor
   DynamicAxes(const DynamicAxes &dyn);
   // operator = for assignment
   const DynamicAxes& operator=(const DynamicAxes &dyn);
   // destructor
   virtual ~DynamicAxes();
   
   // initializes the DynamicAxes
   virtual bool Initialize(); // needed?  maybe later
      
   // all classes derived from GmatBase must supply this Clone method;
   // this must be implemented in the 'leaf' classes
   //virtual GmatBase*       Clone(void) const;

   // Parameter access methods - overridden from GmatBase
   /*
   virtual wxString     GetParameterText(const Integer id) const;     
   virtual Integer         GetParameterID(const wxString &str) const; 
   virtual Gmat::ParameterType
                           GetParameterType(const Integer id) const;
   virtual wxString     GetParameterTypeString(const Integer id) const;
   virtual Real            GetRealParameter(const Integer id) const;
   virtual Real            SetRealParameter(const Integer id,
                                            const Real value);
   virtual Real            GetRealParameter(const wxString &label) const;
   virtual Real            SetRealParameter(const wxString &label,
                                            const Real value);
    */
   // need to add (and to GmatBase) GetA1MjdParameter method(s)?????????
   
protected:

   enum
   {
      DynamicAxesParamCount = AxisSystemParamCount
   };
   
   //static const wxString PARAMETER_TEXT[DynamicAxesParamCount - 
   //                                        AxisSystemParamCount];
   
   //static const Gmat::ParameterType PARAMETER_TYPE[DynamicAxesParamCount - 
   //                                                AxisSystemParamCount];
   

};
#endif // DynamicAxes_hpp
