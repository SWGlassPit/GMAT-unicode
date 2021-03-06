//$Id$
//------------------------------------------------------------------------------
//                                 Global
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Wendy C. Shoan
// Created: 2008.03.14
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CCA54C
//
/**
 * Class implementation for the Global command
 */
//------------------------------------------------------------------------------


#include "Global.hpp"
#include "MessageInterface.hpp"
#include "CommandException.hpp"

//#define DEBUG_GLOBAL

//---------------------------------
// static data
//---------------------------------
//const wxString
//Global::PARAMETER_TEXT[GlobalParamCount - ManageObjectParamCount] =
//{
//};

//const Gmat::ParameterType
//Global::PARAMETER_TYPE[GlobalParamCount - ManageObjectParamCount] =
//{
//};

//------------------------------------------------------------------------------
// Global()
//------------------------------------------------------------------------------
/**
 * Constructor
 */
//------------------------------------------------------------------------------
Global::Global() :
   ManageObject(wxT("Global"))
{
}


//------------------------------------------------------------------------------
// ~Global()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
Global::~Global()
{
}


//------------------------------------------------------------------------------
// Global(const Global &gl)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 * 
 * @param <gl> The instance that gets copied.
 */
//------------------------------------------------------------------------------
Global::Global(const Global &gl) :
   ManageObject(gl)
{
}


//------------------------------------------------------------------------------
// Global& operator=(const Global &gl)
//------------------------------------------------------------------------------
/**
 * Assignment operator
 * 
 * @param <gl> The command that gets copied.
 * 
 * @return A reference to this instance.
 */
//------------------------------------------------------------------------------
Global& Global::operator=(const Global &gl)
{
   if (&gl != this)
   {
      ManageObject::operator=(gl);
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Override of the GmatBase clone method.
 * 
 * @return A new copy of this instance.
 */
//------------------------------------------------------------------------------
GmatBase* Global::Clone() const
{
   return new Global(*this);
}



//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 * Method that initializes the internal data structures.
 * 
 * @return true if initialization succeeds.
 */
//------------------------------------------------------------------------------
bool Global::Initialize()
{
   #ifdef DEBUG_GLOBAL
      MessageInterface::ShowMessage(wxT("Global::Initialize() entered\n"));
   #endif
      
   ManageObject::Initialize();
   
   return true;
}


//---------------------------------------------------------------------------
//  bool GmatCommand::Execute()
//---------------------------------------------------------------------------
/**
 * The method that is fired to perform this Global command.
 *
 * @return true if the Global runs to completion, false if an error
 *         occurs.
 */
//---------------------------------------------------------------------------
bool Global::Execute()
{
   #ifdef DEBUG_GLOBAL
      MessageInterface::ShowMessage(wxT("Global::Execute() entered\n"));
   #endif
   GmatBase    *mapObj;
   wxString ex;
   Integer     sz = objectNames.size();
   for (Integer ii=0; ii<sz; ii++)
   {
      // get it from the LOS, if it's there
      if (objectMap->find(objectNames.at(ii)) != objectMap->end())
      {
         mapObj = (*objectMap)[objectNames.at(ii)];
         if (InsertIntoGOS(mapObj, objectNames.at(ii)))
            objectMap->erase(objectNames.at(ii));
      }
      else if (globalObjectMap->find(objectNames.at(ii)) != globalObjectMap->end())
      {
         mapObj = (*globalObjectMap)[objectNames.at(ii)];
         InsertIntoGOS(mapObj, objectNames.at(ii));
      }
      else
      {
         ex = wxT("Global::Execute - object of name \"") + objectNames.at(ii);
         ex += wxT("\" not found.\n");
         throw CommandException(ex);
      }
   }
      
   return true;
}
