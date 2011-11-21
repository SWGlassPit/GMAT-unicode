//$Id: EventException.hpp 9853 2011-09-09 20:08:55Z djcinsb $
//------------------------------------------------------------------------------
//                               EventException
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Darrel J. Conway
// Created: 2004/1/13
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P.
//
/**
 * Exception class used by the Located Event subsystem.
 */
//------------------------------------------------------------------------------


#ifndef EventException_hpp
#define EventException_hpp

#include "BaseException.hpp"
#include "gmatdefs.hpp"          // For GMAT_API

class GMAT_API EventException : public BaseException
{
public:
   EventException(const wxString &details = wxT(""));
   virtual ~EventException();
   EventException(const EventException &be);
};

#endif // EventException_hpp
