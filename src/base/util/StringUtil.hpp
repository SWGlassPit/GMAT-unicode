//$Id: StringUtil.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                 StringUtil
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
// Created: 2006/1/6
//
/**
 * This file provides string utility.
 */
//------------------------------------------------------------------------------
#ifndef StringUtil_hpp
#define StringUtil_hpp

#include "gmatdefs.hpp"
#include "GmatGlobal.hpp"

namespace GmatStringUtil
{
   enum StripType
   {
      LEADING = 1,
      TRAILING = 2,
      BOTH = 3,
   };
   
   GMAT_API wxString RemoveAll(const wxString &str, char ch, Integer start = 0);
   GMAT_API wxString RemoveLastNumber(const wxString &str, Integer &lastNumber);
   GMAT_API wxString RemoveLastString(const wxString &str, const wxString &lastStr,
                                bool removeAll = false);
   
   GMAT_API wxString RemoveSpaceInBrackets(const wxString &str,
                                     const wxString &bracketPair);
   GMAT_API wxString Trim(const wxString &str, StripType stype = BOTH,
                    bool removeSemicolon = false, bool removeEol = false);
   GMAT_API wxString Strip(const wxString &str, StripType stype = BOTH);
   GMAT_API wxString ToUpper(const wxString &str, bool firstLetterOnly = false);
   GMAT_API wxString ToLower(const wxString &str, bool firstLetterOnly = false);
   GMAT_API wxString Capitalize(const wxString &str);
   GMAT_API wxString Replace(const wxString &str, const wxString &from,
                       const wxString &to);
   GMAT_API wxString ReplaceName(const wxString &str, const wxString &from,
                           const wxString &to);
   GMAT_API wxString ReplaceNumber(const wxString &str, const wxString &from,
                           const wxString &to);
   
   GMAT_API wxString ToString(const bool &val);
   GMAT_API wxString ToString(const Real &val, Integer precision, bool showPoint = false,
                        Integer width = 1);
   GMAT_API wxString ToString(const Integer &val, Integer width);
   
   GMAT_API wxString ToString(const Real &val, bool useCurrentFormat = true,
                        bool scientific = false, bool showPoint = true, 
                        Integer precision = GmatGlobal::DATA_PRECISION,
                        Integer width = GmatGlobal::DATA_WIDTH);
   GMAT_API wxString ToString(const Integer &val, bool useCurrentFormat = true,
                        Integer width = GmatGlobal::INTEGER_WIDTH);
   
   GMAT_API wxString RemoveExtraParen(const wxString &str);
   GMAT_API wxString RemoveOuterString(const wxString &str, const wxString &start,
                                 const wxString &end);
   GMAT_API wxString RemoveEnclosingString(const wxString &str, const wxString &enStr);
   GMAT_API wxString RemoveInlineComment(const wxString &str, const wxString &cmStr);
   GMAT_API wxString ParseFunctionName(const wxString &str);
   GMAT_API wxString AddEnclosingString(const wxString &str, const wxString &enStr);
   GMAT_API wxString GetInvalidNameMessageFormat();
   
   GMAT_API char GetClosingBracket(const char &openBracket);
   
   GMAT_API StringArray SeparateBy(const wxString &str, const wxString &delim,
                          bool putBracketsTogether = false, bool insertDelim = false,
                          bool insertComma = true);
   GMAT_API StringArray SeparateByComma(const wxString &str);
   GMAT_API StringArray SeparateDots(const wxString &str);
   GMAT_API StringArray DecomposeBy(const wxString &str, const wxString &delim);
   
   GMAT_API bool IsNumber(const wxString &str);
   GMAT_API bool ToReal(const wxString &str, Real *value, bool trimParens = false);
   GMAT_API bool ToReal(const wxString &str, Real &value, bool trimParens = false);
   GMAT_API bool ToInteger(const wxString &str, Integer *value, bool trimParens = false);
   GMAT_API bool ToInteger(const wxString &str, Integer &value, bool trimParens = false);
   GMAT_API bool ToBoolean(const wxString &str, bool *value, bool trimParens = false);
   GMAT_API bool ToBoolean(const wxString &str, bool &value, bool trimParens = false);
   
   GMAT_API RealArray ToRealArray(const wxString &str);
   GMAT_API IntegerArray ToIntegerArray(const wxString &str);
   GMAT_API UnsignedIntArray ToUnsignedIntArray(const wxString &str);
   GMAT_API StringArray ToStringArray(const wxString &str);
   GMAT_API BooleanArray ToBooleanArray(const wxString &str);
   
   GMAT_API void ParseParameter(const wxString &str, wxString &type,
                                wxString &owner, wxString &dep);
   GMAT_API void GetArrayCommaIndex(const wxString &str, Integer &comma,
                           const wxString &bracketPair = wxT("()"));
   GMAT_API void GetArrayIndex(const wxString &str, Integer &row, Integer &col,
                      wxString &name, const wxString &bracketPair = wxT("()"));
   GMAT_API void GetArrayIndex(const wxString &str, wxString &rowStr,
                      wxString &colStr, Integer &row, Integer &col,
                      wxString &name, const wxString &bracketPair = wxT("()"));
   GMAT_API void GetArrayIndexVar(const wxString &str, wxString &rowStr,
                         wxString &colStr, wxString &name,
                         const wxString &bracketPair = wxT("()"));
   GMAT_API void FindFirstAndLast(const wxString &str, char ch, Integer &first,
                         Integer &last);
   GMAT_API void FindParenMatch(const wxString &str, Integer &open, Integer &close,
                       bool &isOuterParen);
   GMAT_API void FindMatchingParen(const wxString &str, Integer &openParen,
                          Integer &closeParen, bool &isOuterParen,
                          Integer start = 0);
   GMAT_API void FindMatchingBracket(const wxString &str, Integer &openBracket,
                            Integer &closeBracket, bool &isOuterBracket,
                            const wxString &bracket, Integer start = 0);
   GMAT_API void FindLastParenMatch(const wxString &str, Integer &openParen,
                           Integer &closeParen, Integer start = 0);
   
   GMAT_API bool IsEnclosedWith(const wxString &str, const wxString &enclosingStr);
   GMAT_API bool IsEnclosedWithExtraParen(const wxString &str, bool checkOps = true);
   GMAT_API bool IsEnclosedWithBraces(const wxString &str);
   GMAT_API bool IsEnclosedWithBrackets(const wxString &str);
   GMAT_API bool IsBracketBalanced(const wxString &str, const wxString &bracketPair);
   GMAT_API bool IsParenBalanced(const wxString &str);
   GMAT_API bool AreAllBracketsBalanced(const wxString &str, const wxString &allPairs);
   GMAT_API bool IsOuterParen(const wxString &str);
   GMAT_API bool IsCommaPartOfArray(const wxString &str, Integer start = 0);
   GMAT_API bool IsBracketPartOfArray(const wxString &str, const wxString &bracketPairs,
                             bool checkOnlyFirst);
   GMAT_API bool IsParenPartOfArray(const wxString &str);
   GMAT_API bool IsThereEqualSign(const wxString &str);
   GMAT_API bool IsThereMathSymbol(const wxString &str);
   GMAT_API bool HasNoBrackets(const wxString &str, bool parensForArraysAllowed = true);
   GMAT_API bool IsSingleItem(const wxString &str);
   GMAT_API bool StartsWith(const wxString &str, const wxString &value);
   GMAT_API bool EndsWith(const wxString &str, const wxString &value);
   GMAT_API bool EndsWithPathSeparator(const wxString &str); 
   GMAT_API bool IsValidNumber(const wxString &str);
   GMAT_API bool IsValidName(const wxString &str, bool ignoreBracket = false);
   GMAT_API bool IsBlank(const wxString &str, bool ignoreEol = false);
   GMAT_API bool HasMissingQuote(const wxString &str, const wxString &quote);
   GMAT_API bool IsMathEquation(const wxString &str);
   
   GMAT_API Integer NumberOfOccurrences(const wxString &str, const char c);
   
   GMAT_API StringArray GetVarNames(const wxString &str);
   GMAT_API void WriteStringArray(const StringArray &strArray,
                                  const wxString &desc = wxT(""),
                                  const wxString &prefix = wxT(""));
   
}

#endif // StringUtil_hpp

