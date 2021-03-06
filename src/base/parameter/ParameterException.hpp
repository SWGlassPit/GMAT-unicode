//$Id: ParameterException.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                            ParameterException
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
// Author: Linda Jun
// Created: 2003/09/18
//
/**
 * Defines parameter exception.
 */
//------------------------------------------------------------------------------
#ifndef ParameterException_hpp
#define ParameterException_hpp

#include "BaseException.hpp"

class GMAT_API ParameterException : public BaseException
{
   public:
      ParameterException(const wxString& message = wxT("")) 
         : BaseException(message) {};
};

class InvalidDependencyException : public BaseException
{
   public:
      InvalidDependencyException(const wxString& message = wxT("")) 
         : BaseException(message) {};
};

#endif
