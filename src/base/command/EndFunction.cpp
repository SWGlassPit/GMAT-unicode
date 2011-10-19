//$Id$
//------------------------------------------------------------------------------
//                            EndFunction
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under MOMS Task
// order 124.
//
// Author: Darrel Conway, Thinking Systems, Inc.
// Created: 2005/08/30
//
/**
 * Implementation code for the EndFunction command, a wrapper that marks the end
 * of the commands in a GMAT function.
 */
//------------------------------------------------------------------------------
 

#include "EndFunction.hpp"


EndFunction::EndFunction() :
   GmatCommand    (wxT("EndFunction")),
   functionName         (wxT(""))
{
}


EndFunction::~EndFunction()
{
}


EndFunction::EndFunction(const EndFunction& ef) :
   GmatCommand          (ef),
   functionName         (ef.functionName)
{
}


EndFunction& EndFunction::operator=(const EndFunction& ef)
{
   if (this != &ef)
   {
      functionName = ef.functionName;
   }
   
   return *this;
}
 
GmatBase* EndFunction::GetRefObject(const Gmat::ObjectType type,
                                    const wxString &name)
{
   if (type == Gmat::UNKNOWN_OBJECT)  // Just find it by name
   {
      /// @todo Look up return object based on the name used in function call 
   }
   
   return GmatCommand::GetRefObject(type, name);
}

   
bool EndFunction::RenameRefObject(const Gmat::ObjectType type,
                                  const wxString &oldName,
                                  const wxString &newName)
{
   return true;
}


GmatBase* EndFunction::Clone() const
{
   return (new EndFunction(*this));
}


bool EndFunction::Execute()
{
   return true;
}
