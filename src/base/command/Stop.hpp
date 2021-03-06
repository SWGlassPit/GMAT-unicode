//$Id: Stop.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  Stop
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Darrel J. Conway
// Created: 2003/10/24
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
/**
 * Stop function for the command sequence -- typically used while debugging
 */
//------------------------------------------------------------------------------



#ifndef Stop_hpp
#define Stop_hpp

#include "GmatCommand.hpp" // inheriting class's header file

/**
 * Default command used to initialize the command sequence lists in the 
 * Moderator
 */
class GMAT_API Stop : public GmatCommand
{
public:
   Stop();
   virtual ~Stop();
   Stop(const Stop& noop);
   Stop&                   operator=(const Stop &noop);

   bool                    Execute(void);

   // inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   virtual const wxString&
                     GetGeneratingString(Gmat::WriteMode mode = Gmat::SCRIPTING,
                                         const wxString &prefix = wxT(""),
                                         const wxString &useName = wxT(""));
};


#endif // Stop_hpp
