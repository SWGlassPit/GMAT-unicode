//$Id: MathParser.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                   MathParser
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under MOMS task
// 124.
//
// Author: Darrel J. Conway
// Created: 2006/03/16
// Modified: 2006/04/10 Linda Jun, NASA/GSFC
//   - Added actual code
//
/**
 * Class definition for the MathParser.
 *
 * The MathParser class takes a line of script that evaluates to inline math,
 * and breaks that line apart into its component elements using a recursive 
 * descent algorithm.  The resulting representation is stored in a binary tree 
 * structure, which is calculated, depth first, when the expression needs to be
 * evaluated during execution of a script.
 */
//------------------------------------------------------------------------------
#ifndef MathParser_hpp
#define MathParser_hpp

#include "gmatdefs.hpp"
#include "MathNode.hpp"
#include "MathException.hpp"

class GMAT_API MathParser
{
public:
   
   MathParser();
   MathParser(const MathParser &copy);
   MathParser& operator=(const MathParser &right);
   virtual ~MathParser();
   
   bool         IsEquation(const wxString &str, bool checkMinusSign);
   wxString  FindLowestOperator(const wxString &str, Integer &opIndex,
                                   Integer start = 0);
   MathNode*    Parse(const wxString &str);
   StringArray  GetGmatFunctionNames();
   
protected:
   
   MathNode*    ParseNode(const wxString &str);
   MathNode*    CreateNode(const wxString &type, const wxString &exp);
   StringArray  Decompose(const wxString &str);
   
private:
   
   wxString  originalEquation;
   wxString  theEquation;
   Integer      theGmatFuncCount;
   
   StringArray  ParseParenthesis(const wxString &str);
   StringArray  ParseAddSubtract(const wxString &str);
   StringArray  ParseMultDivide(const wxString &str);
   StringArray  ParseMatrixOps(const wxString &str);
   StringArray  ParsePower(const wxString &str);
   StringArray  ParseUnary(const wxString &str);
   StringArray  ParseMathFunctions(const wxString &str);
   StringArray  ParseUnitConversion(const wxString &str);
   
   bool         IsMathFunction(const wxString &str);
   bool         HasFunctionName(const wxString &str, const StringArray &fnList);
   bool         IsParenPartOfFunction(const wxString &str);
   bool         IsGmatFunction(const wxString &name);
   bool         IsValidOperator(const wxString &str);
   
   wxString  GetFunctionName(UnsignedInt functionType, const wxString &str,
                                wxString &leftStr);
   wxString::size_type
                FindSubtract(const wxString &str, wxString::size_type start);
   wxString::size_type
                FindMatchingParen(const wxString &str,
                                  wxString::size_type start);
   wxString  FindOperatorFrom(const wxString &str,
                                 wxString::size_type start,
                                 wxString &left, wxString &right,
                                 wxString::size_type &opIndex);
   wxString  GetOperator(const IntegerMap::iterator &pos1,
                            const IntegerMap::iterator &pos2,
                            const IntegerMap &opIndexMap,
                            Integer &opIndex);
   wxString  FindOperator(const wxString &str, Integer &opIndex);
   wxString  GetOperatorName(const wxString &op, bool &opFound);
   void         BuildAllFunctionList();
   void         BuildGmatFunctionList(const wxString &str);
   void         BuildFunction(const wxString &str, const StringArray &fnList,
                              wxString &fnName, wxString &leftStr);
   void         FillItems(StringArray &items, const wxString &op,
                          const wxString &left, const wxString &right);
   void         WriteItems(const wxString &msg, StringArray &items);
   void         WriteNode(MathNode *node, UnsignedInt level);
   
   enum
   {
      MATH_FUNCTION,
      MATRIX_FUNCTION,
      MATRIX_OP,
      UNIT_CONVERSION,
      GMAT_FUNCTION,
   };
   
   StringArray  realFuncList;
   StringArray  matrixFuncList;
   StringArray  matrixOpList;
   StringArray  unitConvList;
   StringArray  gmatFuncList;
};


#endif //MathParser_hpp


