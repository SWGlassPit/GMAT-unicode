//$Id$
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
 * Class implementation for the MathParser.
 */
//------------------------------------------------------------------------------

#include "MathParser.hpp"
#include "MathFunction.hpp"
#include "FunctionRunner.hpp"
#include "StringUtil.hpp"       // for ParseParameter()
#include "FileManager.hpp"      // for GetGmatFunctionPath()
#include "MessageInterface.hpp"

#ifdef __UNIT_TEST__
#include "MathFactory.hpp"
#else
#include "Moderator.hpp"
#endif

//#define DEBUG_PARSE 1
//#define DEBUG_PARSE_EQUATION 1
//#define DEBUG_MATH_PARSER 2
//#define DEBUG_DECOMPOSE 1
//#define DEBUG_PARENTHESIS 1
//#define DEBUG_FIND_OPERATOR 1
//#define DEBUG_ADD_SUBTRACT 1
//#define DEBUG_MULT_DIVIDE 1
//#define DEBUG_MATRIX_OPS 1
//#define DEBUG_FUNCTION 2
//#define DEBUG_ATAN2
//#define DEBUG_MATH_PARSER_PARAM 1
//#define DEBUG_INVERSE_OP 1
//#define DEBUG_POWER 1
//#define DEBUG_UNARY 1
//#define DEBUG_CREATE_NODE 1
//#define DEBUG_GMAT_FUNCTION 1

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//---------------------------------
// static data
//---------------------------------

//------------------------------------------------------------------------------
//  MathParser(wxString typeStr, wxString nomme)
//------------------------------------------------------------------------------
/**
 * Constructs the MathParser object (default constructor).
 * 
 * @param <typeStr> String text identifying the object type
 * @param <nomme>   Name for the object
 */
//------------------------------------------------------------------------------
MathParser::MathParser()
{
   theGmatFuncCount = 0;
   BuildAllFunctionList();
}


//------------------------------------------------------------------------------
//  MathParser(const MathParser &copy)
//------------------------------------------------------------------------------
/**
 * Constructs the MathParser object (copy constructor).
 * 
 * @param <copy> Object that is copied
 */
//------------------------------------------------------------------------------
MathParser::MathParser(const MathParser &copy)
{
   theGmatFuncCount = 0;
   BuildAllFunctionList();
}


//------------------------------------------------------------------------------
//  MathParser& operator=(const MathParser &right)
//------------------------------------------------------------------------------
/**
 * Sets one MathParser object to match another (assignment operator).
 * 
 * @param <right> The object that is copied.
 * 
 * @return this object
 */
//------------------------------------------------------------------------------
MathParser& MathParser::operator=(const MathParser &right)
{
   if (this == &right)
      return *this;
   
   theGmatFuncCount = 0;
   BuildAllFunctionList();
   return *this;
}


//------------------------------------------------------------------------------
//  ~MathParser(void)
//------------------------------------------------------------------------------
/**
 * Destroys the MathParser object (destructor).
 */
//------------------------------------------------------------------------------
MathParser::~MathParser()
{
}


//------------------------------------------------------------------------------
// bool IsEquation(const wxString &str, bool checkMinusSign)
//------------------------------------------------------------------------------
/*
 * Examines if given string is a math equation.
 * Call this method with RHS of assignement.
 *
 * @param str The string to be examined for an equation
 * @param checkMinusSign Check for leading minus sign.
 *        Set this flag to true if minus sign is a part of string and is not considered
 *        math unary operator
 * @return  true if string is an equation, means has math operators and/or functions
 *
 */
//------------------------------------------------------------------------------
bool MathParser::IsEquation(const wxString &str, bool checkMinusSign)
{
   theEquation = str;
   
   #if DEBUG_PARSE_EQUATION
   MessageInterface::ShowMessage
      (wxT("=================================================================\n"));
   MessageInterface::ShowMessage
      (wxT("MathParser::IsEquation() str=%s\n"), str.c_str());
   MessageInterface::ShowMessage
      (wxT("=================================================================\n"));
   #endif
   
   bool isEq = false;
   wxString left, right;
   Real rval;
   wxString::size_type opIndex;
   
   // Check if string is enclosed with quotes
   if (GmatStringUtil::IsEnclosedWith(str, wxT("'")))
   {
      isEq = false;
   }
   // Check if it is just a number
   else if (GmatStringUtil::ToReal(str, &rval))
   {
      isEq = false;
   }
   else
   {
      // build GmatFunction list first
      BuildGmatFunctionList(str);
      
      if (GetFunctionName(MATH_FUNCTION, str, left) != wxT("") ||
          GetFunctionName(MATRIX_FUNCTION, str, left) != wxT("") ||
          GetFunctionName(UNIT_CONVERSION, str, left) != wxT("") ||
          FindOperatorFrom(str, 0, left, right, opIndex) != wxT("") ||
          GetFunctionName(GMAT_FUNCTION, str, left) != wxT(""))
      {
         isEq = true;

         if (checkMinusSign)
         {
            // Check for - sign used as string
            if (GmatStringUtil::NumberOfOccurrences(str, wxT('-')) == 1 &&
                GmatStringUtil::StartsWith(str, wxT("-")) &&
                GmatStringUtil::IsSingleItem(str))
            {
               isEq = false;
            }
         }
      }
      else
      {
         // Check ' for matrix transpose and ^(-1) for inverse
         if (str.find(wxT("'")) != str.npos || str.find(wxT("^(-1)")) != str.npos)
            isEq = true;
      }
   }
   
   #if DEBUG_PARSE_EQUATION
   MessageInterface::ShowMessage
      (wxT("MathParser::IsEquation(%s) returning %u\n"), str.c_str(), isEq);
   #endif
   
   return isEq;
}


//------------------------------------------------------------------------------
// wxString FindLowestOperator(const wxString &str, Integer &opIndex,
//                                Integer start = 0)
//------------------------------------------------------------------------------
/*
 * Finds lowest operator from the input string.
 * Single operators are +, -, *, /, ^, '.
 *
 * Precedence of operators is (highest to lowest)
 *    Parentheses ()
 *    matrix Transpose('), power (^),  matrix power(^)
 *    Unary plus (+), unary minus (-)
 *    Multiplication (*), right division (/), matrix multiplication (*), matrix right division (/)
 *    Addition (+), subtraction (-)
 *
 * @param  str  Input string
 * @param  opIndex  Index of operator
 * @param  start  Index to start
 *
 * @return  Single operator,
 *          wxT(""), if operator not found
 */
//------------------------------------------------------------------------------
wxString MathParser::FindLowestOperator(const wxString &str,
                                           Integer &opIndex, Integer start)
{
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("=================================================================\n"));
   MessageInterface::ShowMessage
      (wxT("FindLowestOperator() entered, str=%s, start=%u, length=%u\n"), str.c_str(),
       start, str.size());
   MessageInterface::ShowMessage
      (wxT("=================================================================\n"));
   #endif
   
   Integer firstOpen = str.find_first_of(wxT("("), start);
   Integer start1 = start;
   Integer open1 = -1, close1 = -1;
   Integer length = str.size();
   Integer index = -1;
   bool isOuterParen;
   bool done = false;
   wxString opStr;
   IntegerMap opIndexMap;
   wxString substr;
   IntegerMap::iterator pos;
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage(wxT("   firstOpen=%u\n"), firstOpen);
   #endif
   
   if (firstOpen == (Integer)str.npos)
      firstOpen = -1;
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage(wxT("   firstOpen=%u\n"), firstOpen);
   #endif
   
   if (firstOpen > 0)
   {
      wxString::size_type inverseOp = str.find(wxT("^(-1)"));
      // Check for ^(-1) which goes toghether as inverse operator
      if (inverseOp != str.npos)
      {
         if (str.substr(firstOpen-1, 5) == wxT("^(-1)"))
         {
            #if DEBUG_FIND_OPERATOR
            MessageInterface::ShowMessage(wxT("   found ^(-1)\n"));
            #endif
            
            firstOpen = str.find_first_of(wxT("("), firstOpen + 3);
            if (firstOpen == (Integer)str.npos)
               firstOpen = inverseOp;
         }
      }
   }
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("FindLowestOperator() Find operator before first open parenthesis, ")
       wxT("firstOpen=%d\n"), firstOpen);
   #endif
   
   //-----------------------------------------------------------------
   // find a lowest operator before first open paren
   //-----------------------------------------------------------------
   if (firstOpen > 0)
   {
      substr = str.substr(0, firstOpen);
      opStr = FindOperator(substr, index);
      if (opStr != wxT(""))
         opIndexMap[opStr] = index;
   }
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("FindLowestOperator() Find lowest operator before last close parenthesis"));
   #endif
   //-----------------------------------------------------------------
   // find a lowest operator before last close paren
   //-----------------------------------------------------------------
   while (!done)
   {
      GmatStringUtil::FindMatchingParen(str, open1, close1, isOuterParen, start1);
      
      #if DEBUG_FIND_OPERATOR
      MessageInterface::ShowMessage(wxT("   open1=%u, close1=%u\n"), open1, close1);
      #endif
      
      // find next open parenthesis '('
      start1 = str.find(wxT('('), close1);
      
      if (start1 == -1)
      {
         #if DEBUG_FIND_OPERATOR
         MessageInterface::ShowMessage
            (wxT("   ===> There is no ( found after %d, so exiting while loop\n"), close1);
         #endif
         break;
      }
      
      substr = str.substr(close1+1, start1-close1-1);
      
      #if DEBUG_FIND_OPERATOR
      MessageInterface::ShowMessage(wxT("   substr=%s\n"), substr.c_str());
      #endif
      
      opStr = FindOperator(substr, index);
      
      if (opStr != wxT(""))
         opIndexMap[opStr] = close1 + index + 1;
   }
   
   
   //-----------------------------------------------------------------
   // find a lowest operator after last close paren
   //-----------------------------------------------------------------
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("FindLowestOperator() Find lowest operator after last close parenthesis"));
   #endif
   if (close1 != length-1)
   {
      #if DEBUG_FIND_OPERATOR
      MessageInterface::ShowMessage(wxT("   after last close parenthesis\n"));
      #endif
      
      substr = str.substr(close1+1);
      opStr = FindOperator(substr, index);
      
      if (opStr != wxT(""))
         opIndexMap[opStr] = close1 + index + 1;
   }
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage(wxT("   There are %d operators\n"), opIndexMap.size());
   for (pos = opIndexMap.begin(); pos != opIndexMap.end(); ++pos)
      MessageInterface::ShowMessage
         (wxT("      op=%s, index=%d\n"), pos->first.c_str(), pos->second);
   #endif
   
   IntegerMap::iterator pos1;
   IntegerMap::iterator pos2;
   Integer index1 = -1;
   Integer index2 = -1;
   wxString lastOp;
   bool opFound = false;
   bool unaryMinusFound = false;
   
   // find + or - first
   pos1 = opIndexMap.find(wxT("+"));
   pos2 = opIndexMap.find(wxT("-"));
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("   +op index=%d, -op index=%d\n"),
       pos1 == opIndexMap.end() ? -1 : pos1->second,
       pos2 == opIndexMap.end() ? -1 : pos2->second);
   #endif
   
   if (pos1 != opIndexMap.end() || pos2 != opIndexMap.end())
   {
      if (pos1 != opIndexMap.end())
         index1 = pos1->second;
      if (pos2 != opIndexMap.end())
         index2 = pos2->second;
      
      #if DEBUG_FIND_OPERATOR
      MessageInterface::ShowMessage(wxT("   index1=%d, index2=%d\n"), index1, index2);
      #endif
      
      // Check for unary - operator
      if (index2 == 0)
      {
         // Check for function
         if (firstOpen > 0 && IsParenPartOfFunction(str.substr(1, firstOpen-1)))
            unaryMinusFound = false;
         else
            unaryMinusFound = true;
      }
      
      if (!unaryMinusFound)
      {
         opStr = GetOperator(pos1, pos2, opIndexMap, index);
         opFound = true;
      }
      
//       // Check for unary - operator
//       if (index2 == 0)
//       {
//          // Check for function
//          if (firstOpen > 0 && IsParenPartOfFunction(str.substr(1, firstOpen-1)))
//             unaryMinusFound = false;
//          else
//             unaryMinusFound = true;
//       }
//       else
//       {
//          opStr = GetOperator(pos1, pos2, opIndexMap, index);
//          opFound = true;
//       }
   }
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("   unaryMinusFound=%d, opFound=%d\n"), unaryMinusFound, opFound);
   #endif
   
   if (!opFound)
   {
      // find * or /
      pos1 = opIndexMap.find(wxT("*"));
      pos2 = opIndexMap.find(wxT("/"));
      if (pos1 != opIndexMap.end() || pos2 != opIndexMap.end())
      {
         opStr = GetOperator(pos1, pos2, opIndexMap, index);
      }
      else
      {
         if (unaryMinusFound)
         {
            index = 0;
            opStr = wxT("-");
         }
         else
         {
            // find ^ and not ^(-1) which is inverse of matrix
            // find ' which is transpose of matrix (LOJ: 2010.07.29)
            pos1 = opIndexMap.find(wxT("^"));
            pos2 = opIndexMap.find(wxT("'"));
            if (pos1 != opIndexMap.end() || pos2 != opIndexMap.end())
            {
               opStr = GetOperator(pos1, pos2, opIndexMap, index);
            }
         }
      }
   }
   
   opIndex = index;
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("FindLowestOperator() returning opStr=%s, opIndex=%d\n"),
       opStr.c_str(), opIndex);
   #endif
   
   return opStr;
}


//------------------------------------------------------------------------------
// MathNode*  Parse(const wxString &str)
//------------------------------------------------------------------------------
/**
 * Breaks apart the text representation of an equation and uses the compoment
 * pieces to construct the MathTree.
 *
 * @param  str  Input equation to be parsed
 * @return constructed MathTree pointer
 */
//------------------------------------------------------------------------------
MathNode* MathParser::Parse(const wxString &str)
{
   originalEquation = str;
   theEquation = str;
   
   #if DEBUG_PARSE
   MessageInterface::ShowMessage
      (wxT("=================================================================\n"));
   MessageInterface::ShowMessage
      (wxT("MathParser::Parse() theEquation=%s\n"), theEquation.c_str());
   MessageInterface::ShowMessage
      (wxT("=================================================================\n"));
   #endif
   
   // first remove all blank spaces and semicoln
   wxString newEq = GmatStringUtil::RemoveAll(theEquation, wxT(' '));
   wxString::size_type index = newEq.find_first_of(wxT(';'));
   if (index != newEq.npos)
      newEq.erase(index);
   
   // second remove extra parenthesis (This need more testing - so commented out)
   //newEq = GmatStringUtil::RemoveExtraParen(newEq);
   
   // check if parenthesis are balanced
   if (!GmatStringUtil::IsParenBalanced(newEq))
      throw MathException(wxT("Found unbalanced parenthesis"));
   
   #if DEBUG_PARSE
   MessageInterface::ShowMessage
      (wxT("MathParser::Parse() newEq=%s\n"), newEq.c_str());
   #endif
   
   // build GmatFunction list first
   BuildGmatFunctionList(newEq);
   
   MathNode *topNode = ParseNode(newEq);
   
   #if DEBUG_PARSE
   WriteNode(topNode, 0);
   MessageInterface::ShowMessage
      (wxT("MathParser::Parse() returning topNode=<%p><%s>\n"), topNode,
       topNode->GetTypeName().c_str());
   MessageInterface::ShowMessage
      (wxT("=================================================================\n"));
   #endif
   
   return topNode;
}


//------------------------------------------------------------------------------
// StringArray GetGmatFunctionNames()
//------------------------------------------------------------------------------
StringArray MathParser::GetGmatFunctionNames()
{
   #if DEBUG_GMAT_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathParser::GetGmatFunctionNames() returning %d names\n"), theGmatFuncCount);
   for (Integer i=0; i<theGmatFuncCount; i++)
      MessageInterface::ShowMessage(wxT("   <%s>\n"), gmatFuncList[i].c_str());
   #endif
   
   return gmatFuncList;
}


//------------------------------------------------------------------------------
// MathNode* ParseNode(const wxString &str)
//------------------------------------------------------------------------------
MathNode* MathParser::ParseNode(const wxString &str)
{
   #if DEBUG_CREATE_NODE
   MessageInterface::ShowMessage(wxT("MathParser::ParseNode() str='%s'\n"), str.c_str());
   #endif
   
   StringArray items = Decompose(str);
   wxString op = items[0];
   wxString left = items[1];
   wxString right = items[2];
   
   #if DEBUG_CREATE_NODE
   WriteItems(wxT("MathParser::ParseNode() After Decompose()"), items);
   #endif
   
   MathNode *mathNode;
   
   // If operator is empty, create MathElement, create MathFunction otherwise
   if (op == wxT(""))
   {
      #if DEBUG_CREATE_NODE
      MessageInterface::ShowMessage
         (wxT("=====> Should create MathElement: '%s'\n"), str.c_str());
      #endif
      
      // Remove extra parenthesis before creating a node (LOJ: 2010.07.29)
      wxString str1 = GmatStringUtil::RemoveExtraParen(str);
      
      #if DEBUG_CREATE_NODE
      MessageInterface::ShowMessage
         (wxT("       Creating MathElement with '%s'\n"), str1.c_str());
      #endif
      
      if (str1 == wxT(""))
         throw MathException(wxT("Missing input arguments"));
      
      mathNode = CreateNode(wxT("MathElement"), str1);
   }
   else
   {
      #if DEBUG_CREATE_NODE
      MessageInterface::ShowMessage
         (wxT("=====> Should create MathNode: %s\n"), op.c_str());
      #endif
      
      wxString operands = wxT("( ") + left + wxT(", ") + right + wxT(" )");
      if (right == wxT(""))
         operands = wxT("( ") + left + wxT(" )");
      
      mathNode = CreateNode(op, operands);
      
      MathNode *leftNode = NULL;
      MathNode *rightNode = NULL;
      
      // if node is FunctionRunner, just create left node as MathElement(loj: 2008.08.25)
      // input nodes are created when FunctionRunner is created
      if (mathNode->IsOfType(wxT("FunctionRunner")))
      {
         leftNode = CreateNode(wxT("MathElement"), left);
         mathNode->SetChildren(leftNode, rightNode);
         
         leftNode->SetFunctionInputFlag(true);
      }
      else
      {
         #if DEBUG_CREATE_NODE
         MessageInterface::ShowMessage
            (wxT("   left='%s', right='%s'\n"), left.c_str(), right.c_str());
         #endif
         
         // check for empty argument for function
         if (left == wxT(""))
         {
            if (IsMathFunction(op))
               throw MathException(op + wxT("() - Missing input arguments"));
         }
         else
         {
            #if DEBUG_CREATE_NODE
            MessageInterface::ShowMessage
               (wxT("===============> Parse left node: '%s'\n"), left.c_str());
            #endif
            leftNode = ParseNode(left);
         }
         
         // check if two operands are needed
         if (right == wxT(""))
         {
            if (op == wxT("Add") || op == wxT("Subtract") || op == wxT("Multiply") || op == wxT("Divide"))
               throw MathException(op + wxT("() - Not enough input arguments"));
         }
         else
         {
            #if DEBUG_CREATE_NODE
            MessageInterface::ShowMessage
               (wxT("===============> Parse right node: '%s'\n"), right.c_str());
            #endif
            rightNode = ParseNode(right);
         }
         
         mathNode->SetChildren(leftNode, rightNode);
      }
   }
   
   #if DEBUG_CREATE_NODE
   MessageInterface::ShowMessage
      (wxT("MathParser::ParseNode() returning <%p><%s>\n"),
       mathNode, mathNode->GetTypeName().c_str());
   #endif
   
   return mathNode;
}


//------------------------------------------------------------------------------
// MathNode* CreateNode(const wxString &type, const wxString &exp)
//------------------------------------------------------------------------------
/**
 * Creates MathNode through the Moderator.
 *
 * @return StringArray of elements
 */
//------------------------------------------------------------------------------
MathNode* MathParser::CreateNode(const wxString &type, const wxString &exp)
{
   #if DEBUG_CREATE_NODE
   MessageInterface::ShowMessage
      (wxT("MathParser::CreateNode() type=%s, exp=%s\n"), type.c_str(),
       exp.c_str());
   #endif
   
   // check if type is GmatFunction
   wxString actualType = type;
   wxString nodeName = exp;
   
   // If node is FunctionRunner, add function name to node name (loj: 2008.08.21)
   if (IsGmatFunction(type))
   {
      actualType = wxT("FunctionRunner");
      nodeName = type + exp;
   }
   
   #if DEBUG_CREATE_NODE
   MessageInterface::ShowMessage(wxT("   actualType=%s\n"), actualType.c_str());
   #endif
   
   MathNode *node = NULL;
   
   #ifdef __UNIT_TEST__
   static MathFactory mf;
   node = mf.CreateMathNode(actualType, nodeName);
   #else
   Moderator* theModerator = Moderator::Instance();
   node = theModerator->CreateMathNode(actualType, nodeName);
   #endif
   
   if (node == NULL)
      throw MathException(wxT("Cannot create MathNode of \"") + actualType + wxT("\""));
   
   #ifdef DEBUG_MORE_MEMORY
   MessageInterface::ShowMessage
      (wxT("MathParser::CreateNode() node <%p><%s> '%s' created\n"), node, actualType.c_str());
   #endif
   
   if (actualType == wxT("FunctionRunner"))
   {
      FunctionRunner *fRunner = (FunctionRunner*)node;
      fRunner->SetFunctionName(type);
      
      // add function input arguments
      wxString exp1 = exp;
      exp1 = GmatStringUtil::RemoveOuterString(exp1, wxT("("), wxT(")"));
      
      #if DEBUG_CREATE_NODE > 1
      MessageInterface::ShowMessage(wxT("   exp1='%s'\n"), exp1.c_str());
      #endif
      
      StringArray inputs = GmatStringUtil::SeparateBy(exp1, wxT(","), true);
      for (UnsignedInt i=0; i<inputs.size(); i++)
      {
         //================================================================
         #ifdef __ALLOW_MATH_EXP_NODE__
         //================================================================
         
         // Create all input nodes (loj: 2008.08.25)
         MathNode *node = ParseNode(inputs[i]);
         
         #if DEBUG_CREATE_NODE > 1
         MessageInterface::ShowMessage(wxT("   inputs[%d] = '%s'\n"), i, inputs[i].c_str());
         MessageInterface::ShowMessage
            (wxT("------------------------------------------ Create input nodes\n"));
         #endif
         #if DEBUG_CREATE_NODE > 1
         WriteNode(node, 0);
         MessageInterface::ShowMessage
            (wxT("-------------------------------------------------------------\n"));
         #endif
         
         fRunner->AddInputNode(node);
         
         // If input is GmatFunction or math equation, set wxT("") (loj: 2008.08.22)
         // Should we do this here? Just hold off for now
         if (IsGmatFunction(inputs[i]) || IsEquation(inputs[i], false))
         {
            Integer type1, row1, col1;
            node->GetOutputInfo(type1, row1, col1);
            if (type1 == Gmat::REAL_TYPE)
               fRunner->AddFunctionInput(inputs[i]);
            else
               fRunner->AddFunctionInput(inputs[i]);
            
            #if DEBUG_CREATE_NODE
            MessageInterface::ShowMessage
               (wxT("   Setting \"\" to FunctionRunner, input has Function or Equation\n"));
            #endif
         }
         else
         {
            fRunner->AddFunctionInput(inputs[i]);
         }
         
         //================================================================
         #else
         //================================================================
         
         // check if input is math expression
         if (IsGmatFunction(inputs[i]) || IsEquation(inputs[i], false))
            throw MathException
               (wxT("*** WARNING *** Currently passing math expression to a ")
                wxT("function is not allowed in \"") + originalEquation + wxT("\""));
         
         fRunner->AddFunctionInput(inputs[i]);
         
         //================================================================
         #endif
         //================================================================
         
      }
      
      // add function output arguments
      fRunner->AddFunctionOutput(wxT(""));
      
      // set function inputs and outputs to FunctionManager through FunctionRunner
      fRunner->SetFunctionInputs();
      fRunner->SetFunctionOutputs();
   }
   
   #if DEBUG_CREATE_NODE
   MessageInterface::ShowMessage
      (wxT("MathParser::CreateNode() '%s' returning node <%p><%s>\n"), exp.c_str(), node,
       node->GetTypeName().c_str());
   #endif
   
   return node;
}


//------------------------------------------------------------------------------
// StringArray Decompose(const wxString &str)
//------------------------------------------------------------------------------
/**
 * Breaks string into operator, left and right elements.
 *
 * The order of parsing is as follows:
 *    ParseParenthesis()
 *    ParseAddSubtract()
 *    ParseMultDivide()
 *    ParsePower()
 *    ParseUnary()
 *    ParseMathFunctions()
 *    ParseMatrixOps()
 *    ParseUnitConversion()
 *
 * @return StringArray of elements
 */
//------------------------------------------------------------------------------
StringArray MathParser::Decompose(const wxString &str)
{
   #if DEBUG_DECOMPOSE
   MessageInterface::ShowMessage
      (wxT("MathParser::Decompose() entered, str=%s\n"), str.c_str());
   #endif
   
   StringArray items = ParseParenthesis(str);
   
   #if DEBUG_DECOMPOSE
   WriteItems(wxT("MathParser::Decompose() after ParseParenthesis() "), items);
   #endif
   
   // if no operator found and left is not empty, decompose again
   if (items[0] == wxT("") && items[1] != wxT(""))
      items = Decompose(items[1]);
   
   wxString str1;
   str1 = str;
   
   if (items[0] == wxT("") && str[0] == wxT('(') && str[str.size()-1] == wxT(')'))
   {
      if (GmatStringUtil::IsOuterParen(str))
      {
         str1 = str.substr(1, str.size()-2);
         #if DEBUG_DECOMPOSE
         MessageInterface::ShowMessage
            (wxT("MathParser::Decompose() Found outer parenthesis. str1=%s\n"), str1.c_str());
         #endif
      }
   }
   
   if (items[0] == wxT("function"))
      items[0] = wxT("");
   
   if (items[0] == wxT(""))
   {
      items = ParseAddSubtract(str1);
      
      if (items[0] == wxT("number"))
      {
         #if DEBUG_DECOMPOSE
         WriteItems(wxT("MathParser::Decompose(): It is a number, returning "), items);
         #endif
         items[0] = wxT("");
         return items;
      }
      
      if (items[0] == wxT(""))
      {
         items = ParseMultDivide(str1);
         
         if (items[0] == wxT(""))
         {
            items = ParsePower(str1);
            
            if (items[0] == wxT(""))
            {
               items = ParseUnary(str1);
                        
               if (items[0] == wxT(""))
               {
                  items = ParseMathFunctions(str1);
               
                  if (items[0] == wxT(""))
                  {
                     items = ParseMatrixOps(str1);
                  
                     if (items[0] == wxT(""))
                     {
                        items = ParseUnitConversion(str1);
                     
                     }
                  }
               }
            }
         }
      }
   }
   
   #if DEBUG_DECOMPOSE
   WriteItems(wxT("MathParser::Decompose(): returning "), items);
   #endif
   
   return items;
}


//------------------------------------------------------------------------------
// StringArray ParseParenthesis(const wxString &str)
//------------------------------------------------------------------------------
/**
 * Returns string array of operator, left and right
 */
//------------------------------------------------------------------------------
StringArray MathParser::ParseParenthesis(const wxString &str)
{
   #if DEBUG_PARENTHESIS
   MessageInterface::ShowMessage
      (wxT("MathParser::ParseParenthesis() entered, str=%s\n"), str.c_str());
   #endif
   
   StringArray items;
   wxString op = wxT("");
   wxString left = wxT("");
   wxString right = wxT("");
   wxString::size_type opIndex;
   
   //-----------------------------------------------------------------
   // if no opening parenthesis '(' found, just return
   //-----------------------------------------------------------------
   wxString::size_type index1 = str.find(wxT('('));
   
   if (index1 == str.npos)
   {
      FillItems(items, op, left, right);
      
      #if DEBUG_PARENTHESIS
      WriteItems(wxT("MathParser::ParseParenthesis(): open parenthesis not found.")
                 wxT(" returning "), items);
      #endif
      
      return items;
   }
   
   //-----------------------------------------------------------------
   // if lowest operator is + or - and not negate, just return
   //-----------------------------------------------------------------
   Integer index;
   wxString opStr1 = FindLowestOperator(str, index);
   #if DEBUG_PARENTHESIS
   MessageInterface::ShowMessage(wxT("   lowestOperator found =[%s]\n"), opStr1.c_str());
   #endif
   if ((opStr1 == wxT("+") || opStr1 == wxT("-")) && index != 0)
   {
      FillItems(items, op, left, right);
      
      #if DEBUG_PARENTHESIS
      WriteItems(wxT("MathParser::ParseParenthesis(): lowest + or - found.")
                 wxT(" returning "), items);
      #endif
      
      return items;      
   }
   
   //-----------------------------------------------------------------
   // if lowest operator is * or /, just return with operator
   //-----------------------------------------------------------------
   if (opStr1 == wxT("*") || opStr1 == wxT("/"))
   {      
      bool opFound1;      
      op = GetOperatorName(opStr1, opFound1);
      
      left = str.substr(0, index);
      right = str.substr(index+1, str.npos);
      FillItems(items, op, left, right);
      
      #if DEBUG_PARENTHESIS
      WriteItems(wxT("MathParser::ParseParenthesis(): lowest * or / found.")
                 wxT(" returning "), items);
      #endif
      
      return items;
   }
   
   //-----------------------------------------------------------------
   // if lowest operator is ^, just return with operator
   //-----------------------------------------------------------------
   if (opStr1 == wxT("^"))
   {      
      bool opFound1;      
      op = GetOperatorName(opStr1, opFound1);
      
      left = str.substr(0, index);
      right = str.substr(index+1, str.npos);
      FillItems(items, op, left, right);
      
      #if DEBUG_PARENTHESIS
      WriteItems(wxT("MathParser::ParseParenthesis(): lowest ^ found.")
                 wxT(" returning "), items);
      #endif
      
      return items;
   }
   
   //-----------------------------------------------------------------
   // if lowest operator is ', just return with operator
   //-----------------------------------------------------------------
   if (opStr1 == wxT("'"))
   {      
      bool opFound1;      
      op = GetOperatorName(opStr1, opFound1);
      
      left = str.substr(0, index);
      right = str.substr(index+1, str.npos);
      FillItems(items, op, left, right);
      
      #if DEBUG_PARENTHESIS
      WriteItems(wxT("MathParser::ParseParenthesis(): lowest ' found.")
                 wxT(" returning "), items);
      #endif
      
      return items;
   }
   
   //-----------------------------------------------------------------
   // if ( is part of fuction, just return first parenthesis
   //-----------------------------------------------------------------
   wxString strBeforeParen = str.substr(0, index1);
   
   #if DEBUG_PARENTHESIS
   MessageInterface::ShowMessage
      (wxT("   ParseParenthesis() strBeforeParen=%s\n"), strBeforeParen.c_str());
   #endif
   
   if (IsParenPartOfFunction(strBeforeParen))
   {
      // find matching closing parenthesis
      UnsignedInt index2 = FindMatchingParen(str, index1);
      
      #if DEBUG_PARENTHESIS
      MessageInterface::ShowMessage
         (wxT("   ParseParenthesis() Parenthesis is part of function. str=%s, size=%u, ")
          wxT("index1=%u, index2=%u\n"), str.c_str(), str.size(), index1, index2);
      #endif
      
      // if last char is ')'
      if (index2 == str.size()-1)
      {
         #if DEBUG_PARENTHESIS
         MessageInterface::ShowMessage
            (wxT("   ParseParenthesis() last char is ) so find function name\n"));
         #endif
         // find math function
         op = GetFunctionName(GMAT_FUNCTION, str, left);
         if (op == wxT(""))
            op = GetFunctionName(MATH_FUNCTION, str, left);
         if (op == wxT(""))
            op = GetFunctionName(MATRIX_FUNCTION, str, left);
         if (op == wxT(""))
            op = GetFunctionName(UNIT_CONVERSION, str, left);
      }
      
      #if DEBUG_PARENTHESIS
      MessageInterface::ShowMessage(wxT("   ParseParenthesis() op='%s'\n"), op.c_str());
      #endif
      
      // See if there is an operator before this function
      wxString op1, left1, right1;
      op1 = FindOperatorFrom(str, 0, left1, right1, opIndex);
      if (op1 != wxT("") && opIndex != str.npos)
      {
         if (opIndex < index1)
         {
            // return blank for next parse
            #if DEBUG_PARENTHESIS
            MessageInterface::ShowMessage(wxT("found operator before function\n"));
            #endif
            
            FillItems(items, wxT(""), wxT(""), wxT(""));
            return items;
         }
      }
      
      // Handle special atan2(y,x) function
      if (op == wxT("atan2"))
      {
         StringArray parts = GmatStringUtil::SeparateByComma(str);

         #ifdef DEBUG_ATAN2
         for (UnsignedInt i = 0; i < parts.size(); i++)
            MessageInterface::ShowMessage(wxT("..... (1) %d '%s'\n"), i, parts[i].c_str());
         #endif
         
         bool parsingFailed = true;
         
         if (parts.size() == 1)
         {
            wxString str1 = str.substr(index1+1, index2-index1-1);
            parts = GmatStringUtil::SeparateByComma(str1);

            #ifdef DEBUG_ATAN2
            for (UnsignedInt i = 0; i < parts.size(); i++)
               MessageInterface::ShowMessage(wxT("..... (2) %d '%s'\n"), i, parts[i].c_str());
            #endif
            
            if (parts.size() == 2)
            {
               left = parts[0];
               right = parts[1];
               if (left != wxT("") && right != wxT(""))
                  parsingFailed = false;
            }
         }
         
         if (parsingFailed)
            throw MathException(wxT("Atan2() - Missing or invalid input arguments"));
      }
      else
      {
         left = str.substr(index1+1, index2-index1-1);
      }
      
      FillItems(items, op, left, right);
      
      #if DEBUG_PARENTHESIS
      WriteItems(wxT("MathParser::ParseParenthesis() - parenthesis is part ")
                 wxT("of function. returning "), items);
      #endif
      
      return items;
   }
   
   
   //-----------------------------------------------------------------
   // If it is ^(-1), handle it later in DecomposeMatrixOps()
   //-----------------------------------------------------------------
   if (str.find(wxT("^(-1)")) != str.npos)
   {
      #if DEBUG_INVERSE_OP
      MessageInterface::ShowMessage
         (wxT("MathParser::ParseParenthesis() found ^(-1) str=%s\n"), str.c_str());
      #endif
      
      FillItems(items, op, left, right);
      
      #if DEBUG_PARENTHESIS
      WriteItems(wxT("MathParser::ParseParenthesis() found ^(-1)returning "), items);
      #endif
      
      return items;
   }
   
   
   //-----------------------------------------------------------------
   // if enclosed with parenthesis
   //-----------------------------------------------------------------
   if (GmatStringUtil::IsEnclosedWithExtraParen(str, false))
   {   
      left = str.substr(1, str.size()-2);
      FillItems(items, op, left, right);
      
      #if DEBUG_PARENTHESIS
      WriteItems(wxT("MathParser::ParseParenthesis() complete parenthesis found.")
                 wxT(" returning "), items);
      #endif
      
      return items;
   }
   
   
   Integer index2;
   //-----------------------------------------------------------------
   // find the lowest operator
   //-----------------------------------------------------------------
   wxString opStr = FindLowestOperator(str, index2);
   if (opStr != wxT(""))
   {
      bool opFound;      
      op = GetOperatorName(opStr, opFound);
      
      if (opFound)
      {
         left = str.substr(0, index2);
         right = str.substr(index2+1, str.npos);
         
         if (op == wxT("Subtract") && left == wxT(""))
         {
            op = wxT("Negate");
            left = right;
            right = wxT("");
         }
         
         FillItems(items, op, left, right);
         
         #if DEBUG_PARENTHESIS
         WriteItems(wxT("MathParser::ParseParenthesis() found lowest operator ")
                    wxT("returning "), items);
         #endif
         return items;
      }
   }
   else
   {
      #if DEBUG_PARENTHESIS
      MessageInterface::ShowMessage
         (wxT("   ParseParenthesis() No operator found\n"));
      #endif
      
      //--------------------------------------------------------------
      // to be continued - will continue later (loj: 2008.09.03)
      //--------------------------------------------------------------
//       // Check if name before first parenthesis is GmatFunction
//       if (IsGmatFunction(strBeforeParen))
//       {
//          FillItems(items, strBeforeParen, str.substr(index1+1), wxT(""));
         
//          #if DEBUG_PARENTHESIS
//          WriteItems(wxT("MathParser::ParseParenthesis() found lowest operator ")
//                     wxT("returning "), items);
//          #endif
//          return items;
//       }
      //--------------------------------------------------------------
   }
   
   
   FillItems(items, op, left, right);
   
   #if DEBUG_PARENTHESIS
   WriteItems(wxT("MathParser::ParseParenthesis() returning "), items);
   #endif
   
   return items;
}


//------------------------------------------------------------------------------
// wxString FindOperatorFrom(const wxString &str, wxString::size_type start,
//                              wxString &left, wxString &right,
//                              wxString::size_type &opIndex)
//------------------------------------------------------------------------------
wxString MathParser::FindOperatorFrom(const wxString &str, wxString::size_type start,
                                         wxString &left, wxString &right,
                                         wxString::size_type &opIndex)
{
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("MathParser::FindOperatorFrom() entered, str=%s, start=%u\n"), str.c_str(), start);
   #endif
   
   StringArray items;
   wxString op;
   wxString::size_type index;
   wxString::size_type index1 = str.find(wxT("+"), start);
   wxString::size_type index2 = str.find(wxT("-"), start);
   
   if (index1 == str.npos && index2 == str.npos)
   {
      index1 = str.find(wxT("*"), start);
      index2 = str.find(wxT("/"), start);
      
      if (index1 == str.npos && index2 == str.npos)
         index1 = str.find(wxT("^"), start);
      
      if (index1 != str.npos)
      {
         #if DEBUG_INVERSE_OP
         MessageInterface::ShowMessage
            (wxT("FindOperatorFrom() found ^ str=%s, index1=%u\n"),
             str.c_str(), index1);
         #endif
         
         // try for ^(-1) for inverse
         if (str.substr(index1, 5) == wxT("^(-1)"))
            return wxT(""); // handle it later in DecomposeMatrixOps()
      }
   }
   
   index = index1 == str.npos ? index2 : index1;
   
   if (index != str.npos)
   {
      bool opFound;
      op = str.substr(index, 1);
      op = GetOperatorName(op, opFound);
      left = str.substr(0, index);
      right = str.substr(index+1, str.size()-index);
   }
   
   opIndex = index;
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("MathParser::FindOperatorFrom() returning op=%s, left=%s, right=%s, opIndex=%u\n"),
       op.c_str(), left.c_str(), right.c_str(), opIndex);
   #endif
   
   return op;
   
}


//------------------------------------------------------------------------------
// wxString GetOperatorName(const wxString &op, bool &opFound)
//------------------------------------------------------------------------------
wxString MathParser::GetOperatorName(const wxString &op, bool &opFound)
{
   #if DEBUG_PARENTHESIS
   MessageInterface::ShowMessage
      (wxT("MathParser::GetOperatorName() op=%s\n"), op.c_str());
   #endif
   
   wxString opName = wxT("<") + op + wxT("> :Unknown Operator");
   opFound = true;
   
   if (op == wxT("+"))
      opName = wxT("Add");
   else if (op == wxT("-"))
      opName = wxT("Subtract");
   else if (op == wxT("*"))
      opName = wxT("Multiply");
   else if (op == wxT("/"))
      opName = wxT("Divide");
   else if (op == wxT("^"))
      opName = wxT("Power");
   else if (op == wxT("'"))
      opName = wxT("Transpose");
   else
      opFound = false;
   
   #if DEBUG_PARENTHESIS
   MessageInterface::ShowMessage
      (wxT("MathParser::GetOperatorName() opFound=%u, opName=%s\n"), opFound,
       opName.c_str());
   #endif
   
   return opName;
}


//------------------------------------------------------------------------------
// wxString FindOperator(const wxString &str, Integer &opIndex)
//------------------------------------------------------------------------------
/*
 * Finds the right most lowest operator from the input string.
 *
 * Precedence of operators is (lowest to highest)
 *    +, -
 *    *, /
 *    unary -
 *    ', ^
 *    ()
 *
 * Single operators are +, -, *, /, ^, '
 * Double operators are ++, --, +-, -+, *-, /-, ^+, ^-
 *
 * 
 * @param  str  Input string
 * @param  opIndex  Index of operator
 *
 * @return  Single operator,
 *          First operator, if double operator found
 *          wxT(""), if operator not found
 */
//------------------------------------------------------------------------------
wxString MathParser::FindOperator(const wxString &str, Integer &opIndex)
{
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("MathParser::FindOperator() entered, str=[ %s ]\n"), str.c_str());
   #endif
   
   wxString str1 = str;
   
   // Replace scientific notation e- E- e+ E+
   str1 = GmatStringUtil::ReplaceNumber(str1, wxT("e-"), wxT("e#"));
   str1 = GmatStringUtil::ReplaceNumber(str1, wxT("e+"), wxT("e#"));
   str1 = GmatStringUtil::ReplaceNumber(str1, wxT("E-"), wxT("e#"));
   str1 = GmatStringUtil::ReplaceNumber(str1, wxT("E+"), wxT("e#"));
   
   StringArray items;
   wxString op;
   wxString::size_type index;
   wxString::size_type index1 = str1.find_last_of(wxT("+"));
   wxString::size_type index2 = str1.find_last_of(wxT("-"));
   wxString::size_type index3 = str1.find_first_of(wxT("-"));
   bool unaryMinusFound = false;
   bool checkNext = true;
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("FindOperator() for +,- index1=%u, index2=%u, index3=%u\n"), index1, index2, index3);
   #endif
   
   if (index1 != str1.npos || index2 != str1.npos)
      checkNext = false;
   
   // Check for unary - operator
   if (index3 != str1.npos)
   {
      #if DEBUG_FIND_OPERATOR
      MessageInterface::ShowMessage(wxT("   Found unary minus operator\n"));
      #endif
      unaryMinusFound = true;
      if (index1 != str1.npos || index2 != str1.npos)
      {
         wxString::size_type index4;
         if (index1 != str1.npos && index2 != str1.npos)
            index4 = index1 > index2 ? index1 : index2;
         else
            index4 = index1 == str1.npos ? index2 : index1;
         
         #if DEBUG_FIND_OPERATOR
         MessageInterface::ShowMessage(wxT("   index4=%u\n"), index4);
         #endif
         
         if ((index4 > 0) &&
             (str[index4-1] == wxT('*') || str[index4-1] == wxT('/')))
            checkNext = true;
      }
   }
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage(wxT("FindOperator() %s *,/\n"), checkNext ? wxT("check") : wxT("skip"));
   #endif
   
   if (checkNext)
   {
      index1 = str1.find_last_of(wxT("*"));
      index2 = str1.find_last_of(wxT("/"));
      
      #if DEBUG_FIND_OPERATOR
      MessageInterface::ShowMessage
         (wxT("FindOperator() for *,/ index1=%u, index2=%u\n"), index1, index2);
      #endif
      
      if (index1 == str1.npos && index2 == str1.npos)
      {
         if (unaryMinusFound)
         {
            op = wxT("-");
            opIndex = 0;
            
            #if DEBUG_FIND_OPERATOR
            MessageInterface::ShowMessage
               (wxT("FindOperator() returning op=%s, opIndex=%u\n"), op.c_str(), opIndex);
            #endif
            
            return op;            
         }
         
         index1 = str1.find_last_of(wxT("^"));
         
         #if DEBUG_FIND_OPERATOR
         MessageInterface::ShowMessage
            (wxT("FindOperator() for ^ index1=%u\n"), index1);
         #endif
         
         if (index1 != str1.npos)
         {
            #if DEBUG_FIND_OPERATOR
            MessageInterface::ShowMessage
               (wxT("FindOperator() found ^ str=%s, index1=%u\n"),
                str1.c_str(), index1);
            #endif
            
            // try for ^(-1) for inverse
            if (str1.substr(index1, 5) == wxT("^(-1)"))
            {
               #if DEBUG_FIND_OPERATOR
               MessageInterface::ShowMessage
                  (wxT("MathParser::FindOperator() found ^(-1) so returning \"\""));
               #endif
               return wxT("");
            }
         }
         else
         {
            // Find ' for transpose
            index1 = str1.find_last_of(wxT("'"));
            
            if (index1 != str1.npos)
            {
               #if DEBUG_FIND_OPERATOR
               MessageInterface::ShowMessage
                  (wxT("FindOperator() found ' str=%s, index1=%u\n"), str1.c_str(), index1);
               #endif
            }
            else
            {
               #if DEBUG_FIND_OPERATOR
               MessageInterface::ShowMessage
                  (wxT("MathParser::FindOperator() returning \"\""));
               #endif
               return wxT("");
            }
         }
      }
   }
   
   // if both operators found, assign to greater position
   if (index1 != str1.npos && index2 != str1.npos)
      index = index1 > index2 ? index1 : index2;
   else
      index = index1 == str1.npos ? index2 : index1;
   
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("FindOperator() Checking for double operator, index=%u, index1=%u, ")
       wxT("index2=%u\n"), index, index1, index2);
   #endif
   
   // check for double operator such as *-, /-
   
   if (index == 0)
   {
      op = str1.substr(index, 1);
      opIndex = index;
   }
   else if (str[index-1] == wxT('+') || str[index-1] == wxT('-') ||
            str[index-1] == wxT('*') || str[index-1] == wxT('/') ||
            str[index-1] == wxT('^'))
   {
      op = str1.substr(index-1, 1);
      opIndex = index-1;
   }
   else if (index != str1.npos)
   {
      op = str1.substr(index, 1);
      opIndex = index;
   }
   else
   {
      op = wxT("");
      opIndex = -1;
   }
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("FindOperator() returning op=%s, opIndex=%u\n"), op.c_str(), opIndex);
   #endif
   
   return op;
   
}


//------------------------------------------------------------------------------
// wxString GetOperator(const IntegerMap::iterator &pos1,
//                         const IntegerMap::iterator &pos2,
//                         const IntegerMap &opIndexMap,
//                         Integer &opIndex)
//------------------------------------------------------------------------------
wxString MathParser::GetOperator(const IntegerMap::iterator &pos1,
                                    const IntegerMap::iterator &pos2,
                                    const IntegerMap &opIndexMap,
                                    Integer &opIndex)
{
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage(wxT("GetOperator() entered\n"));
   if (pos1 != opIndexMap.end())
      MessageInterface::ShowMessage
         (wxT("   pos1=<%s><%d>\n"), pos1->first.c_str(), pos1->second);
   else
      MessageInterface::ShowMessage(wxT("   pos1 is NULL\n"));
   
   if (pos2 != opIndexMap.end())
      MessageInterface::ShowMessage
         (wxT("   pos2=<%s><%d>\n"), pos2->first.c_str(), pos2->second);
   else
      MessageInterface::ShowMessage(wxT("   pos2 is NULL\n"));
   #endif
   
   wxString opStr;
   Integer index = -1;
   
   if (pos1 != opIndexMap.end() || pos2 != opIndexMap.end())
   {
      if (pos2 == opIndexMap.end())
      {
         opStr = pos1->first;
         index = pos1->second;
      }
      else if (pos1 == opIndexMap.end())
      {
         opStr = pos2->first;
         index = pos2->second;
      }
      else
      {
         // if operators on the same level of precedence are evaluated
         // from left to right
         if (pos1->second > pos2->second)
         {
            opStr = pos1->first;
            index = pos1->second;
         }
         else
         {
            opStr = pos2->first;
            index = pos2->second;
         }
      }
   }
   
   opIndex = index;
   
   #if DEBUG_FIND_OPERATOR
   MessageInterface::ShowMessage
      (wxT("GetOperator() return opStr='%s', opIndex=%d\n"), opStr.c_str(), opIndex);
   #endif
   
   return opStr;
}


//------------------------------------------------------------------------------
// UnsignedInt FindSubtract(const wxString &str, wxString::size_type start)
//------------------------------------------------------------------------------
wxString::size_type MathParser::FindSubtract(const wxString &str,
                                                wxString::size_type start)
{
   #if DEBUG_INVERSE_OP
   MessageInterface::ShowMessage
      (wxT("MathParser::FindSubtract() str=%s, start=%u\n"), str.c_str(), start);
   #endif
   
   wxString::size_type index2 = str.find(wxT('-'), start);
   wxString::size_type index3 = str.find(wxT("^(-1)"), start);
   
   #if DEBUG_INVERSE_OP
   MessageInterface::ShowMessage
      (wxT("MathParser::FindSubtract() index2=%u, index3=%u\n"), index2, index3);
   #endif
   
   // found no ^(-1)
   if (index2 != str.npos && index3 == str.npos)
   {
      #if DEBUG_INVERSE_OP
      MessageInterface::ShowMessage
         (wxT("MathParser::FindSubtract() found no ^(-1) returning index2=%u\n"),
          index2);
      #endif
      return index2;
   }
   
   // found - inside of ^(-1)
   if (index2 > index3 && index3 + 5 == str.size())
   {
      #if DEBUG_INVERSE_OP
      MessageInterface::ShowMessage
         (wxT("MathParser::FindSubtract() found - inside of ^(-1) ")
          wxT("returning str.size()=%u\n"), str.size());
      #endif
      return str.size();
   }

   // found - and ^(-1)
   if (index2 < index3)
   {
      #if DEBUG_INVERSE_OP
      MessageInterface::ShowMessage
         (wxT("MathParser::FindSubtract() found - and ^(-1) ")
          wxT("returning index2=%u\n"), index2);
      #endif
      return index2;
   }

   
   if (index3 != str.npos)
   {
      // If it has only wxT("^(-1)"), handle it later in DecomposeMatrixOps()
      if (index3+5 == str.size())
      {
         #if DEBUG_INVERSE_OP
         MessageInterface::ShowMessage
            (wxT("MathParser::FindSubtract() found ^(-1) str=%s\n"),
             str.c_str());
         MessageInterface::ShowMessage
            (wxT("MathParser::FindSubtract() returning str.size()=%u\n"), str.size());
         #endif
         
         return str.size();
      }
      else
      {
         wxString::size_type index = FindSubtract(str, index3+5);
         
         #if DEBUG_INVERSE_OP
         MessageInterface::ShowMessage
            (wxT("MathParser::FindSubtract() index=%u, after FindSubtract()\n"),
             index);
         #endif
         
         // if found first - not in ^(-1)
         if (index != str.npos && index != str.size())
            return index;
      }
   }
   
   #if DEBUG_INVERSE_OP
   MessageInterface::ShowMessage
      (wxT("MathParser::FindSubtract() returning str.size()=%u\n"), str.size());
   #endif
   
   return str.size();
}


//------------------------------------------------------------------------------
// StringArray ParseAddSubtract(const wxString &str)
//------------------------------------------------------------------------------
StringArray MathParser::ParseAddSubtract(const wxString &str)
{
   #if DEBUG_ADD_SUBTRACT
   MessageInterface::ShowMessage
      (wxT("MathParser::ParseAddSubtract() str=%s, size=%d\n"), str.c_str(), str.size());
   #endif
   
   //-----------------------------------------------------------------
   // Operators of equal precedence evaluate from left to right.
   // 10 - 50 + 1 + 30 - 25 should produce
   // (((10 - 50) + 1) + 30) - 25
   //-----------------------------------------------------------------
   
   StringArray items;
   wxString op = wxT("");
   wxString left;
   wxString right;

   // find last - or +
   wxString::size_type index1 = str.find_last_of(wxT('+'));
   wxString::size_type index2 = str.find_last_of(wxT('-'));
   
   #if DEBUG_ADD_SUBTRACT
   MessageInterface::ShowMessage
      (wxT("ParseAddSubtract() index1=%u, index2=%u\n"), index1, index2);
   #endif
   
   //-------------------------------------------------------
   // no + or - found
   //-------------------------------------------------------
   if (index1 == str.npos && index2 == str.npos)
   {
      FillItems(items, wxT(""), wxT(""), wxT(""));
      #if DEBUG_ADD_SUBTRACT
      WriteItems(wxT("ParseAddSubtract(): '+' or '-' not found"), items);
      #endif
      return items;
   }
   
   //-------------------------------------------------------
   // find lowest operator, expecting + or -
   //-------------------------------------------------------
   Integer index;
   wxString opStr = FindLowestOperator(str, index);
   
   #if DEBUG_ADD_SUBTRACT
   MessageInterface::ShowMessage
      (wxT("after FindLowestOperator() opStr=%s, index=%d\n"),
       opStr.c_str(), index);
   #endif
   
   //-------------------------------------------------------
   // lowest operator is not + or -
   //-------------------------------------------------------
   if (opStr != wxT("+") && opStr != wxT("-"))
   {
      // Check for scientific number
      if (GmatStringUtil::IsNumber(str))
         FillItems(items, wxT("number"), str, wxT(""));
      else
         FillItems(items, wxT(""), wxT(""), wxT(""));
      
      #if DEBUG_ADD_SUBTRACT
      WriteItems(wxT("ParseAddSubtract(): lowest op is not '+' or '-'"), items);
      #endif
      return items;
   }
   
   //-------------------------------------------------------
   // If unary operator, handle it later in ParseUnary()
   //-------------------------------------------------------
   if (index2 == 0 && index1 == str.npos)
   {
      #if DEBUG_INVERSE_OP
      MessageInterface::ShowMessage
         (wxT("MathParser::ParseAddSubtract() found unary str=%s\n"), str.c_str());
      #endif
      
      FillItems(items, op, left, right);
      return items;
   }
   
   
   UnsignedInt indexLeft = index;
   UnsignedInt indexRight = index + 1;
   
   bool opFound;      
   op = GetOperatorName(opStr, opFound);
   
   // if double operator +- or -+ found
   if (str[index+1] == wxT('+') || str[index+1] == wxT('-'))
   {
      #if DEBUG_ADD_SUBTRACT
      MessageInterface::ShowMessage
         (wxT("ParseAddSubtract() double operator found, %s\n"),
          str.substr(index, 2).c_str());
      #endif
      
      if (opStr == wxT("+") && str[index+1] == wxT('+'))
         op = wxT("Add");
      else if (opStr == wxT("+") && str[index+1] == wxT('-'))
         op = wxT("Subtract");
      else if (opStr == wxT("-") && str[index+1] == wxT('-'))
         op = wxT("Add");
      else if (opStr == wxT("-") && str[index+1] == wxT('+'))
         op = wxT("Subtract");
      
      indexRight = indexRight + 1;
   }
   
   #if DEBUG_ADD_SUBTRACT
   MessageInterface::ShowMessage
      (wxT("ParseAddSubtract() indexLeft=%u, indexRight=%u, op=%s\n"),
       indexLeft, indexRight, op.c_str());
   #endif
   
   left = str.substr(0, indexLeft);
   right = str.substr(indexRight, str.npos);
   
   #if DEBUG_ADD_SUBTRACT
   MessageInterface::ShowMessage
      (wxT("ParseAddSubtract() op=%s, left=%s, right=%s\n"), op.c_str(),
       left.c_str(), right.c_str());
   #endif
   
   if (right == wxT(""))
      throw MathException(wxT("Need right side of \"") + op + wxT("\""));
   
   FillItems(items, op, left, right);
   
   #if DEBUG_ADD_SUBTRACT
   WriteItems(wxT("After ParseAddSubtract()"), items);
   #endif
   
   return items;
}


//------------------------------------------------------------------------------
// StringArray ParseMultDivide(const wxString &str)
//------------------------------------------------------------------------------
StringArray MathParser::ParseMultDivide(const wxString &str)
{
   #if DEBUG_MULT_DIVIDE
   MessageInterface::ShowMessage
      (wxT("MathParser::ParseMultDivide() str=%s\n"), str.c_str());
   #endif
   
   StringArray items;
   wxString op = wxT("");

   //-----------------------------------------------------------------
   // find last * or /
   // because we want to evaluate * or / in the order it appears
   // a * b / c * d
   //-----------------------------------------------------------------
   
   wxString::size_type index1 = str.find_last_of(wxT('*'));
   wxString::size_type index2 = str.find_last_of(wxT('/'));
   
   if (index1 == str.npos && index2 == str.npos)
   {
      FillItems(items, wxT(""), wxT(""), wxT(""));
      #if DEBUG_MULT_DIVIDE
      WriteItems(wxT("ParseMultDivide(): No * or / found"), items);
      #endif
      return items;
   }
   
   //-------------------------------------------------------
   // find lowest operator, expecting * or /
   //-------------------------------------------------------
   Integer index;
   wxString opStr = FindLowestOperator(str, index);
   
   #if DEBUG_MULT_DIVIDE
   MessageInterface::ShowMessage
      (wxT("after FindLowestOperator() opStr=%s, index=%d\n"),
       opStr.c_str(), index);
   #endif
   
   //-------------------------------------------------------
   // lowest operator is not / or *
   //-------------------------------------------------------
   if (opStr != wxT("/") && opStr != wxT("*"))
   {
      FillItems(items, wxT(""), wxT(""), wxT(""));
      #if DEBUG_MULT_DIVIDE
      WriteItems(wxT("ParseMultDivide(): lowest op is not '/' or '*'"), items);
      #endif
      return items;
   }
   
   bool opFound;
   op = GetOperatorName(opStr, opFound);
   wxString left = str.substr(0, index);
   wxString right = str.substr(index+1, str.npos);
   
   //-------------------------------------------------------
   // find double operator *+, *-, /+, /-
   //-------------------------------------------------------
   if (str[index+1] == wxT('+') || str[index+1] == wxT('-'))
   {
      wxString right = str.substr(index+2, str.npos);
      
      #if DEBUG_MULT_DIVIDE
      MessageInterface::ShowMessage
         (wxT("combined operator found, %s\n"), str.substr(index, 2).c_str());
      #endif
   }
   
   if (left == wxT(""))
      throw MathException(wxT("Need left side of the operator \"") + op + wxT("\""));
   
   if (right == wxT(""))
      throw MathException(wxT("Need right side of the operator \"") + op + wxT("\""));
   
   FillItems(items, op, left, right);

   #if DEBUG_MULT_DIVIDE
   WriteItems(wxT("After ParseMultDivide()"), items);
   #endif
   
   return items;
   
}


//------------------------------------------------------------------------------
// StringArray ParsePower(const wxString &str)
//------------------------------------------------------------------------------
StringArray MathParser::ParsePower(const wxString &str)
{
   #if DEBUG_POWER
   MessageInterface::ShowMessage
      (wxT("MathParser::ParsePower() str=%s\n"), str.c_str());
   #endif
   
   StringArray items;
   wxString op = wxT("");
   
   // We should find last ^ insted of first ^ to fix bug 2176 (LOJ: 2010.10.29)
   wxString::size_type index1 = str.find_last_of(wxT('^'));
   
   if (index1 == str.npos)
   {
      FillItems(items, wxT(""), wxT(""), wxT(""));
      return items;
   }
   
   // If it is ^(-1), handle it later in DecomposeMatrixOps()
   if (str.find(wxT("^(-1)")) != str.npos)
   {
      #if DEBUG_INVERSE_OP
      MessageInterface::ShowMessage
         (wxT("MathParser::ParsePower() found ^(-1) str=%s\n"), str.c_str());
      #endif
      
      FillItems(items, wxT(""), wxT(""), wxT(""));
      return items;
   }
   
   
   // If first unary operator found, handle it later in ParseUnary()
   if (str.find(wxT("-")) != str.npos)
   {
      UnsignedInt index3 = str.find(wxT("-"));
      if (index3 == 0)
      {
         #if DEBUG_UNARY
         MessageInterface::ShowMessage
            (wxT("MathParser::ParsePower() found - unary str=%s\n"), str.c_str());
         #endif
      
         FillItems(items, wxT(""), wxT(""), wxT(""));
         return items;
      }
   }

   
   wxString::size_type index = str.npos;
   if (index1 != str.npos)
   {
      op = wxT("Power");
      index = index1;
   }
   
   wxString left = str.substr(0, index);
   wxString right = str.substr(index+1, str.npos);

   if (left == wxT(""))
      throw MathException(wxT("Need left side of the operator \"") + op + wxT("\""));
   
   if (right == wxT(""))
      throw MathException(wxT("Need right side of the operator \"") + op + wxT("\""));
   
   FillItems(items, op, left, right);
   
   #if DEBUG_MATH_PARSER > 1
   WriteItems(wxT("After ParsePower()"), items);
   #endif
   
   return items;
}


//------------------------------------------------------------------------------
// StringArray ParseUnary(const wxString &str)
//------------------------------------------------------------------------------
StringArray MathParser::ParseUnary(const wxString &str)
{
   #if DEBUG_UNARY
   MessageInterface::ShowMessage
      (wxT("MathParser::ParseUnary() str=%s\n"), str.c_str());
   #endif
   
   StringArray items;
   wxString op = wxT("");
   
   // If it is ^(-1), handle it later in DecomposeMatrixOps()
   if (str.find(wxT("^(-1)")) != str.npos)
   {
      #if DEBUG_INVERSE_OP
      MessageInterface::ShowMessage
         (wxT("MathParser::ParseUnary() found ^(-1) str=%s\n"), str.c_str());
      #endif
      
      FillItems(items, wxT(""), wxT(""), wxT(""));
      return items;
   }
   
   // find  - or -
   wxString::size_type index1 = str.find(wxT('-'));
   wxString::size_type index2 = str.find(wxT('+'));
   
   if (index1 == str.npos && index2 == str.npos)
   {
      FillItems(items, wxT(""), wxT(""), wxT(""));
      return items;
   }
   
   if (index1 != str.npos)
      op = wxT("Negate");
   else
      op = wxT("None");
   
   wxString left;
   
   // If power ^ found
   if (str.find(wxT("^")) != str.npos)
   {
      #if DEBUG_POWER
      MessageInterface::ShowMessage
         (wxT("MathParser::ParseUnary() found ^ str=%s\n"), str.c_str());
      #endif
      
      left = str.substr(index1+1);
   }
   else
   {
      left = str.substr(index1+1, str.npos);
   }
   
   FillItems(items, op, left, wxT(""));
   
   #if DEBUG_UNARY
   WriteItems(wxT("After ParseUnary()"), items);
   #endif
   
   return items;
}


//------------------------------------------------------------------------------
// StringArray ParseMathFunctions(const wxString &str)
//------------------------------------------------------------------------------
StringArray MathParser::ParseMathFunctions(const wxString &str)
{
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathParser::ParseMathFunctions() str=%s\n"), str.c_str());
   #endif
   
   StringArray items;
   wxString left;
   
   // find first math function
   wxString fnName = GetFunctionName(MATH_FUNCTION, str, left);
   
   if (fnName == wxT(""))
   {
      // let's try GmatFunction name
      fnName = GetFunctionName(GMAT_FUNCTION, str, left);
      
      if (fnName == wxT(""))
      {
         FillItems(items, wxT(""), wxT(""), wxT(""));
         return items;
      }
   }
   
   if (left == wxT(""))
      throw MathException(wxT("Need an argument of the function \"") + fnName + wxT("\""));
   
   FillItems(items, fnName, left, wxT(""));
   
   #if DEBUG_FUNCTION > 1
   WriteItems(wxT("After ParseMathFunction()"), items);
   #endif
   
   return items;
}


//------------------------------------------------------------------------------
// StringArray ParseMatrixOps(const wxString &str)
//------------------------------------------------------------------------------
StringArray MathParser::ParseMatrixOps(const wxString &str)
{
   #if DEBUG_MATRIX_OPS
   MessageInterface::ShowMessage
      (wxT("MathParser::ParseMatrixOps() str=%s\n"), str.c_str());
   #endif
   
   StringArray items;
   wxString left;
   
   // find matrix function
   wxString fnName = GetFunctionName(MATRIX_FUNCTION, str, left);
   
   // Check for matrix operator symbol, such as ' for transpose and ^(-1) for inverse
   if (fnName == wxT(""))
   {
      // try matrix op ' for transpose
      wxString::size_type index1 = str.find(wxT("'"));
      
      #if DEBUG_MATRIX_OPS
      MessageInterface::ShowMessage
         (wxT("MathParser::ParseMatrixOps() find ' for transpose, index1=%u\n"), index1);
      #endif
      
      // if transpose ' not found
      if (index1 == str.npos)
      {
         // try matrix op ^(-1) for inverse
         index1 = str.find(wxT("^(-1)"));
         
         if (index1 == str.npos)
         {
            FillItems(items, wxT(""), wxT(""), wxT(""));
         }
         else // ^(-1) found
         {
            left = str.substr(0, index1);
            fnName = wxT("Inv");
            FillItems(items, fnName, left, wxT(""));
         }
      }
      else // ' found
      {
         left = str.substr(0, index1);
         fnName = wxT("Transpose");
         FillItems(items, fnName, left, wxT(""));
      }
      
      // Check for invalid operators after matrix ops
      if (fnName != wxT(""))
      {
         wxString::size_type index2 = index1 + 1;
         
         if (fnName == wxT("Inv"))
            index2 = index1 + 4;
         
         if (str.size() > index2)
         {
            wxString nextOp = str.substr(index2+1, 1);
            #if DEBUG_MATRIX_OPS
            MessageInterface::ShowMessage(wxT("   nextOp='%s'\n"), nextOp.c_str());
            #endif
            if (nextOp != wxT("") && !IsValidOperator(nextOp))
                throw MathException(wxT("Invalid math operator \"") + nextOp + wxT("\" found"));
         }
      }
   }
   else // matrix function name found
   {
      FillItems(items, fnName, left, wxT(""));
   }
   
   #if DEBUG_MATRIX_OPS
   WriteItems(wxT("After ParseMatrixOps()"), items);
   #endif
   
   return items;
}


//------------------------------------------------------------------------------
// StringArray ParseUnitConversion(const wxString &str)
//------------------------------------------------------------------------------
StringArray MathParser::ParseUnitConversion(const wxString &str)
{
   #if DEBUG_MATH_PARSER > 1
   MessageInterface::ShowMessage
      (wxT("MathParser::ParseUnitConversion() str=%s\n"), str.c_str());
   #endif
   
   StringArray items;
   wxString left;
   
   // find first math function
   wxString fnName = GetFunctionName(UNIT_CONVERSION, str, left);

   if (fnName == wxT(""))
      FillItems(items, wxT(""), wxT(""), wxT(""));
   else
      FillItems(items, fnName, left, wxT(""));
   
   #if DEBUG_MATH_PARSER > 1
   WriteItems(wxT("After ParseUnitConversion()"), items);
   #endif
   
   return items;
}


//------------------------------------------------------------------------------
// bool IsMathFunction(const wxString &str)
//------------------------------------------------------------------------------
/**
 * Tests if input string is any of built-in math functions
 */
//------------------------------------------------------------------------------
bool MathParser::IsMathFunction(const wxString &str)
{
   if (HasFunctionName(str, realFuncList) ||
       HasFunctionName(str, matrixFuncList) ||
       HasFunctionName(str, unitConvList))
      return true;
   
   return false;
}


//------------------------------------------------------------------------------
// bool HasFunctionName(const wxString &str, const StringArray &fnList)
//------------------------------------------------------------------------------
bool MathParser::HasFunctionName(const wxString &str, const StringArray &fnList)
{
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathParser::HasFunctionName() str=%s\n"), str.c_str());
   #endif
   
   // Find name from the input function list as is
   if (find(fnList.begin(), fnList.end(), str) != fnList.end())
   {
      #if DEBUG_FUNCTION
      MessageInterface::ShowMessage
         (wxT("MathParser::HasFunctionName() returning true\n"));
      #endif
      return true;
   }
   
   // Try lower the first letter and find
   wxString str1 = GmatStringUtil::ToLower(str, true);
   if (find(fnList.begin(), fnList.end(), str1) != fnList.end())
   {
      #if DEBUG_FUNCTION
      MessageInterface::ShowMessage
         (wxT("MathParser::HasFunctionName() returning true\n"));
      #endif
      return true;
   }
   
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathParser::HasFunctionName() returning false\n"));
   #endif
   
   return false;
}


//------------------------------------------------------------------------------
// bool IsParenPartOfFunction(const wxString &str)
//------------------------------------------------------------------------------
bool MathParser::IsParenPartOfFunction(const wxString &str)
{
   // Check function name in the GmatFunction list first (loj: 2008.08.20)
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathParser::IsParenPartOfFunction() checking for function name '%s'\n"),
       str.c_str());
   #endif
   
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("   Checking GmatFunction list...\n"));
   #endif
   if (HasFunctionName(str, gmatFuncList))
   {
      #if DEBUG_FUNCTION
      MessageInterface::ShowMessage
         (wxT("MathParser::IsParenPartOfFunction() returning true, GmatFunction found\n"));
      #endif
      return true;
   }
   
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("   Checking internal MathFunction list...\n"));
   #endif
   if (HasFunctionName(str, realFuncList))
   {
      #if DEBUG_FUNCTION
      MessageInterface::ShowMessage
         (wxT("MathParser::IsParenPartOfFunction() returning true, MathFunction found\n"));
      #endif
      return true;
   }
   
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("   Checking internal MatrixFunction list...\n"));
   #endif
   if (HasFunctionName(str, matrixFuncList))
   {
      #if DEBUG_FUNCTION
      MessageInterface::ShowMessage
         (wxT("MathParser::IsParenPartOfFunction() returning true, MatrixFunction found\n"));
      #endif
      return true;
   }
   
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("   Checking internal UnitConversionFunction list...\n"));
   #endif
   if (HasFunctionName(str, unitConvList))
   {
      #if DEBUG_FUNCTION
      MessageInterface::ShowMessage
         (wxT("MathParser::IsParenPartOfFunction() returning true, UnitConversionFunction found\n"));
      #endif
      return true;
   }
   
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathParser::IsParenPartOfFunction() returning false, no function found\n"));
   #endif
   return false;
}


//------------------------------------------------------------------------------
// bool IsGmatFunction(const wxString &name)
//------------------------------------------------------------------------------
bool MathParser::IsGmatFunction(const wxString &name)
{
   // if name has open parenthesis, get it up to
   wxString name1 = name;
   wxString::size_type index = name1.find(wxT("("));
   if (index != name1.npos)
      name1 = name1.substr(0, index);
   
   for (int i=0; i<theGmatFuncCount; i++)
      if (name1 == gmatFuncList[i])
         return true;
   
   return false;
}


//------------------------------------------------------------------------------
// bool IsValidOperator(const wxString &str)
//------------------------------------------------------------------------------
bool MathParser::IsValidOperator(const wxString &str)
{
   wxChar op = str[0];
   if (op == wxT('+') || op == wxT('-') || op == wxT('*') || op == wxT('/') || op == wxT('^') || op == wxT('\''))
      return true;
   else
      return false;
}


//------------------------------------------------------------------------------
// wxString GetFunctionName(UnsignedInt functionType, const wxString &str,
//                             wxString &left)
//------------------------------------------------------------------------------
wxString MathParser::GetFunctionName(UnsignedInt functionType,
                                        const wxString &str,
                                        wxString &left)
{
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathParser::GetFunctionName() entered, functionType=%u, str='%s'\n"),
       functionType, str.c_str());
   #endif

   left = wxT("");
   wxString fnName = wxT("");
   
   // if string does not start with letter, just return
   if (!isalpha(str[0]))
      return fnName;
   
   switch (functionType)
   {
   case MATH_FUNCTION:
      {
         BuildFunction(str, realFuncList, fnName, left);
         break;
      }
   case MATRIX_FUNCTION:
      {
         BuildFunction(str, matrixFuncList, fnName, left);
         break;
      }
   case UNIT_CONVERSION:
      {
         BuildFunction(str, unitConvList, fnName, left);
         break;
      }
   case GMAT_FUNCTION:
      {
         BuildFunction(str, gmatFuncList, fnName, left);
         break;
      }
   default:
      break;
   }
   
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathParser::GetFunctionName() leaving, fnName='%s', left='%s'\n"),
       fnName.c_str(), left.c_str());
   #endif
   
   return fnName;
}


//------------------------------------------------------------------------------
// void BuildGmatFunctionList(const wxString &str)
//------------------------------------------------------------------------------
/*
 * Builds GmatFunction list found in the GmatFunction path.
 */
//------------------------------------------------------------------------------
void MathParser::BuildGmatFunctionList(const wxString &str)
{
   #if DEBUG_GMAT_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathParser::BuildGmatFunctionList() entered, str='%s'\n"), str.c_str());
   #endif
   
   StringArray names = GmatStringUtil::GetVarNames(str);
   FileManager *fm = FileManager::Instance();
   
   for (UnsignedInt i=0; i<names.size(); i++)
   {
      #if DEBUG_GMAT_FUNCTION > 1
      MessageInterface::ShowMessage
         (wxT("   BuildGmatFunctionList() checking '%s'\n"), names[i].c_str());
      #endif
      
      if (fm->GetGmatFunctionPath(names[i]) != wxT(""))
      {         
         // if GmatFunction not registered, add to the list
         if (find(gmatFuncList.begin(), gmatFuncList.end(), names[i]) == gmatFuncList.end())
            gmatFuncList.push_back(names[i]);
      }
   }
   
   theGmatFuncCount = gmatFuncList.size();
   
   #if DEBUG_GMAT_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathParser::BuildGmatFunctionList() found total of %d GmatFunction(s)\n"),
       theGmatFuncCount);
   for (Integer i=0; i<theGmatFuncCount; i++)
      MessageInterface::ShowMessage(wxT("   '%s'\n"), gmatFuncList[i].c_str());
   #endif
}


//------------------------------------------------------------------------------
// void BuildFunction(const wxString &str, const StringArray &fnList,
//                    wxString &fnName, wxString &left)
//------------------------------------------------------------------------------
void MathParser::BuildFunction(const wxString &str, const StringArray &fnList,
                               wxString &fnName, wxString &left)
{
   UnsignedInt count = fnList.size();
   fnName = wxT("");
   left = wxT("");
   
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathParser::BuildFunction() entered, str='%s', function count=%d\n"),
       str.c_str(), count);
   #endif
   
   if (count == 0)
      return;
   
   wxString::size_type functionIndex = str.npos;
   
   // Check if function name is in the function list
   wxString fname = GmatStringUtil::ParseFunctionName(str);
   
   #if DEBUG_FUNCTION > 1
   MessageInterface::ShowMessage(wxT("==> fname = '%s'\n"), fname.c_str());
   #endif
   
   if (find(fnList.begin(), fnList.end(), fname) != fnList.end())
   {
      fnName = fname;
      functionIndex = str.find(fname + wxT("("));
   }
   else
   {
      // Let's try lower case of the first letter
      if (isalpha(fname[0]) && isupper(fname[0]))
      {
         // MSVC++ failes to do: fname[0] = tolower(fname[0])
         wxString fname1 = fname;
         fname1[0] = tolower(fname[0]);
         
         #if DEBUG_FUNCTION > 1
         MessageInterface::ShowMessage
            (wxT("   function name '%s' not found so trying lower case '%s'\n"),
             fname.c_str(), fname1.c_str());
         #endif
         
         if (find(fnList.begin(), fnList.end(), fname1) != fnList.end())
         {
            fnName = fname1;
            functionIndex = str.find(fname + wxT("("));
         }
      }
   }
   
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathParser::BuildFunction() fnName='%s', functionIndex=%u\n"),
       fnName.c_str(), functionIndex);
   #endif
   
   if (fnName != wxT(""))
   {
      wxString::size_type index1 = str.find(wxT("("), functionIndex);
      
      #if DEBUG_FUNCTION
      MessageInterface::ShowMessage
         (wxT("MathParser::BuildFunction() calling FindMatchingParen() with start ")
          wxT("index=%u\n"), index1);
      #endif
      
      UnsignedInt index2 = FindMatchingParen(str, index1);
      
      #if DEBUG_FUNCTION
      MessageInterface::ShowMessage
         (wxT("MathParser::BuildFunction() matching ) found at %u\n"), index2);
      #endif
      
      left = str.substr(index1+1, index2-index1-1);
   }
   
   #if DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathParser::BuildFunction() leaving, fnName=%s, left=%s\n"),
       fnName.c_str(), left.c_str());
   #endif
}


//------------------------------------------------------------------------------
// wxString::size_type FindMatchingParen(const wxString &str, size_type start)
//------------------------------------------------------------------------------
wxString::size_type MathParser::FindMatchingParen(const wxString &str,
                                                     wxString::size_type start)
{
   #if DEBUG_MATH_PARSER > 1
   MessageInterface::ShowMessage
      (wxT("MathParser::FindMatchingParen() entered, str='%s', str.size()=%u, start=%u\n"),
       str.c_str(), str.size(), start);
   #endif
   
   int leftCounter = 0;
   int rightCounter = 0;
   
   for (UnsignedInt i = start; i < str.size(); i++)
   {
      if (str[i] == wxT('('))
         leftCounter++;

      if (str[i] == wxT(')'))
         rightCounter++;
      
      if (leftCounter == rightCounter)
         return i;
   }
   
   #if DEBUG_MATH_PARSER
   MessageInterface::ShowMessage
      (wxT("**** ERROR ****  MathParser::FindMatchingParen() Unmatching parenthesis found\n"));
   #endif
   
   throw MathException(wxT("Unmatching parenthesis found"));
}


//------------------------------------------------------------------------------
// void FillItems(StringArray &items, const wxString &op,
//                const wxString &left, const wxString &right)
//------------------------------------------------------------------------------
void MathParser::FillItems(StringArray &items, const wxString &op,
                           const wxString &left, const wxString &right)
{
   items.push_back(op);
   items.push_back(left);
   items.push_back(right);
}


//------------------------------------------------------------------------------
// void WriteItems(const wxString &msg, StringArray &items)
//------------------------------------------------------------------------------
void MathParser::WriteItems(const wxString &msg, StringArray &items)
{
   MessageInterface::ShowMessage
      (wxT("%s items = <%s> <%s> <%s>\n"), msg.c_str(),
       items[0].c_str(), items[1].c_str(), items[2].c_str());
}


//------------------------------------------------------------------------------
// void WriteNode(MathNode *node, UnsignedInt level)
//------------------------------------------------------------------------------
void MathParser::WriteNode(MathNode *node, UnsignedInt level)
{
#if DEBUG_PARSE
   
   if (node == NULL)
      return;

   level++;

   if (!node->IsFunction())
   {
      for (UnsignedInt i=0; i<level; i++)
         MessageInterface::ShowMessage(wxT("...."));
      
      MessageInterface::ShowMessage
         (wxT("node=%s: %s\n"), node->GetTypeName().c_str(), node->GetName().c_str());
   }
   else
   {
      for (UnsignedInt i=0; i<level; i++)
         MessageInterface::ShowMessage(wxT("...."));

      if (node->IsOfType(wxT("FunctionRunner")))
         MessageInterface::ShowMessage
            (wxT("node=%s: %s\n"), node->GetTypeName().c_str(), node->GetName().c_str());
      else
         MessageInterface::ShowMessage
            (wxT("node=%s: %s\n"), node->GetTypeName().c_str(), node->GetName().c_str());
      
      if (node->GetLeft())
      {
         for (UnsignedInt i=0; i<level; i++)
            MessageInterface::ShowMessage(wxT("...."));
         
         MessageInterface::ShowMessage
            (wxT("left=%s: %s\n"), node->GetLeft()->GetTypeName().c_str(),
             node->GetLeft()->GetName().c_str());
         
         WriteNode(node->GetLeft(), level);
      }
      
      
      if (node->GetRight())
      {
         for (UnsignedInt i=0; i<level; i++)
            MessageInterface::ShowMessage(wxT("...."));
         
         MessageInterface::ShowMessage
            (wxT("right=%s: %s\n"), node->GetRight()->GetTypeName().c_str(),
             node->GetRight()->GetName().c_str());
         
         WriteNode(node->GetRight(), level);
      }
   }
#endif
}


//------------------------------------------------------------------------------
// void BuildAllFunctionList()
//------------------------------------------------------------------------------
/*
 * Builds function list.
 *
 * @note
 * We cannot use the list from the Moderator::GetListOfFactoryItems(Gmat::MATH_TREE)
 * since it needs to be grouped into the order of parsing.
 *    ParseParenthesis()
 *    ParseAddSubtract()
 *    ParseMultDivide()
 *    ParsePower()
 *    ParseUnary()
 *    ParseMathFunctions()
 *    ParseMatrixOps()
 *    ParseUnitConversion()
 *
 */
//------------------------------------------------------------------------------
void MathParser::BuildAllFunctionList()
{
   //@todo We should get this list from the MathFactory.
   // Why power (^) is not here? (LOJ: 2010.11.04)
   // Real Function List
   realFuncList.push_back(wxT("asin"));
   realFuncList.push_back(wxT("sin"));
   realFuncList.push_back(wxT("acos"));
   realFuncList.push_back(wxT("cos"));
   realFuncList.push_back(wxT("atan2"));
   realFuncList.push_back(wxT("atan"));
   realFuncList.push_back(wxT("tan"));
   realFuncList.push_back(wxT("log10"));
   realFuncList.push_back(wxT("log"));
   realFuncList.push_back(wxT("exp"));
   realFuncList.push_back(wxT("sqrt"));
   realFuncList.push_back(wxT("abs"));
   
   // Matrix Function List
   matrixFuncList.push_back(wxT("transpose"));
   matrixFuncList.push_back(wxT("det"));
   matrixFuncList.push_back(wxT("inv"));
   matrixFuncList.push_back(wxT("norm"));
   
   // Unit Conversion List
   unitConvList.push_back(wxT("degToRad"));
   unitConvList.push_back(wxT("radToDeg"));
   unitConvList.push_back(wxT("deg2Rad"));
   unitConvList.push_back(wxT("rad2Deg"));
   
   // Matrix Operator List
   matrixOpList.push_back(wxT("'"));
   matrixOpList.push_back(wxT("^(-1)"));
   
}

