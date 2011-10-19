//$Id: Converter.hpp 9513 2011-04-30 21:23:06Z djcinsb $ 
//------------------------------------------------------------------------------
//                                 Converter
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author:  Joey Gurganus
// Created: 2004/03/22
//
/**
 * Definition of the Converter base class
 */
//------------------------------------------------------------------------------

#ifndef Converter_hpp
#define Converter_hpp

#include "GmatBase.hpp"

class GMAT_API Converter
{
public:
    // Default constructor
    Converter();
    Converter(const wxString &type);
    // Copy constructor
    Converter(const Converter &converter);
    // Assignment operator
    Converter& operator=(const Converter &converter);
    
    virtual wxString GetType();
    virtual void SetType(const wxString &type);

    // Destructor
    virtual ~Converter();

protected:
    // Declare protetced method data 
    wxString type;

private:

};

#endif // Converter_hpp
