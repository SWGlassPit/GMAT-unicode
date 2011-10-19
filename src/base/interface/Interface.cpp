//$Id$
//------------------------------------------------------------------------------
//                                  Interface
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
// Author: Linda Jun/GSFC
// Created: 2010/04/02
//
/**
 * Implementation of Interface class.
 */
//------------------------------------------------------------------------------

#include "Interface.hpp"
#include "InterfaceException.hpp"

//------------------------------------------------------------------------------
//  Interface(const wxString &type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Constructs Interface instance (default constructor).
 */
//------------------------------------------------------------------------------
Interface::Interface(const wxString &type, const wxString &name) :
   GmatBase (Gmat::INTERFACE, type, name)
{
}


//------------------------------------------------------------------------------
//  Interface(const Interface &interf)
//------------------------------------------------------------------------------
/**
 * Constructs Interface instance (copy constructor). 
 *
 * @param  interf  Instance that gets copied.
 */
//------------------------------------------------------------------------------
Interface::Interface(const Interface &interf) :
   GmatBase (interf)
{
}


//------------------------------------------------------------------------------
//  ~Interface()
//------------------------------------------------------------------------------
/**
 * Class destructor.
 */
//------------------------------------------------------------------------------
Interface::~Interface()
{
}


//------------------------------------------------------------------------------
// virtual Integer Open(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Opens interface to other application such as MATLAB
 *
 * @param  name  Name of the interface to be used when opening [wxT("")]
 */
//------------------------------------------------------------------------------
Integer Interface::Open(const wxString &name)
{
   throw InterfaceException(wxT("Open() not defined for ") + typeName +
                            wxT(" named \"") + instanceName + wxT("\"\n"));
}


//------------------------------------------------------------------------------
// virtual Integer Close(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Closes interface to other application such as MATLAB
 *
 * @param  name  Name of the interface to be used when closing [wxT("")]
 */
//------------------------------------------------------------------------------
Integer Interface::Close(const wxString &name)
{
   throw InterfaceException(wxT("Close() not defined for ") + typeName +
                            wxT(" named \"") + instanceName + wxT("\"\n"));
}

