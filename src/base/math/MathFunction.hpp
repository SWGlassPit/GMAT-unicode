//$Id: MathFunction.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                   MathFunction
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P.
//
// Author: Waka Waktola
// Created: 2006/04/04
//
/**
 * Defines the Math functions class for Math in scripts.
 */
//------------------------------------------------------------------------------

#ifndef MathFunction_hpp
#define MathFunction_hpp

#include "GmatBase.hpp"
#include "MathNode.hpp"
#include "MathException.hpp"

class GMAT_API MathFunction : public MathNode
{
public:
   MathFunction(const wxString &typeStr, const wxString &nomme);
   virtual ~MathFunction();
   MathFunction(const MathFunction &mf);
   MathFunction& operator=(const MathFunction &mf);
   
   virtual Real Evaluate();
   virtual Rmatrix MatrixEvaluate();
   
   virtual bool SetChildren(MathNode *leftChild, MathNode *rightChild);
   virtual MathNode* GetLeft();
   virtual MathNode* GetRight();
   
protected:

   MathNode *leftNode;
   MathNode *rightNode;
   
//    /// Parameter IDs
//    enum
//    {
//       MathFunctionParamCount = MathNodeParamCount,
//    };
};

#endif //MathFunction_hpp
