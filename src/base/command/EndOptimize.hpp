//$Id: EndOptimize.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                EndOptimize 
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
// Author:  Daniel Hunter/GSFC/MAB (CoOp)
// Created: 2006.07.20
//
/**
 * Declaration for the EndOptimize command class
 */
//------------------------------------------------------------------------------

#ifndef EndOptimize_hpp
#define EndOptimize_hpp

#include "GmatCommand.hpp"


class GMAT_API EndOptimize : public GmatCommand
{
public:
   EndOptimize();
   EndOptimize(const EndOptimize& eo);
   EndOptimize&            operator=(const EndOptimize& eo);
   virtual ~EndOptimize();
    
   virtual bool            Initialize();
   virtual bool            Execute();
    
   virtual bool            Insert(GmatCommand *cmd, GmatCommand *prev);

   // inherited from GmatBase
   virtual GmatBase*       Clone() const;
   virtual const wxString&
                           GetGeneratingString(Gmat::WriteMode mode,
                                               const wxString &prefix,
                                               const wxString &useName);
protected:

   enum
   {
      EndOptimizeParamCount = GmatCommandParamCount,
   };

   // save for possible later use
   //static const wxString
   //PARAMETER_TEXT[EndOptimizeParamCount - GmatCommandParamCount];   
   //static const Gmat::ParameterType
   //PARAMETER_TYPE[EndOptimizeParamCount - GmatCommandParamCount];
};


#endif /*EndOptimize_hpp*/
