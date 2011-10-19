//$Id: EndWhile.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                             EndWHile
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P
//
// Author:  Wendy Shoan/GSFC/MAB
// Created: 2004/08/11
//
/**
* Definition for the closing line of the While Statement
 */
//------------------------------------------------------------------------------


#ifndef EndWhile_hpp
#define EndWhile_hpp


#include "GmatCommand.hpp"


class GMAT_API EndWhile : public GmatCommand
{
public:
   EndWhile();
   virtual ~EndWhile();

   EndWhile(const EndWhile& ew);
   EndWhile&              operator=(const EndWhile& ew);

   virtual bool            Initialize();
   virtual bool            Execute();
   
   virtual bool            Insert(GmatCommand *cmd, GmatCommand *prev);

   // inherited from GmatBase
   virtual bool            RenameRefObject(const Gmat::ObjectType type,
                                           const wxString &oldName,
                                           const wxString &newName);   
   virtual GmatBase*       Clone() const;
   virtual const wxString&
                           GetGeneratingString(Gmat::WriteMode mode,
                                               const wxString &prefix,
                                               const wxString &useName);
};


#endif // EndWhile_hpp
