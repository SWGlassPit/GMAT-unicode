//$Id: TextParser.hpp 9518 2011-04-30 22:32:04Z djcinsb $
//------------------------------------------------------------------------------
//                              TextParser
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
// Created: 2006/08/10
//
/**
 * Declares the methods to parse text into parts.
 */
//------------------------------------------------------------------------------
#ifndef TextParser_hpp
#define TextParser_hpp

#include "gmatdefs.hpp"

namespace Gmat
{
   enum BlockType
   {
      COMMENT_BLOCK,
      DEFINITION_BLOCK,
      COMMAND_BLOCK,
      ASSIGNMENT_BLOCK,
      FUNCTION_BLOCK,
   };
};


class GMAT_API TextParser
{
public:

   TextParser();
   ~TextParser();

   // inline methods
   wxString GetPrefaceComment() { return prefaceComment; }
   wxString GetInlineComment() { return inlineComment; }
   wxString GetInstruction() { return theInstruction; }
   
   void Initialize(const StringArray &commandList);
   StringArray& GetChunks() { return theChunks; }
   bool IsFunctionCall() { return isFunctionCall; }
   
   void Reset();
   
   // for parsing
   Gmat::BlockType EvaluateBlock(const wxString &logicalBlock);
   StringArray DecomposeBlock(const wxString &logicalBlock);
   StringArray ChunkLine();
   
   StringArray Decompose(const wxString &chunk,
                         const wxString &bracketPair,
                         bool checkForArray = true,
                         bool removeOuterBracket = false);
   StringArray SeparateBrackets(const wxString &chunk,
                                const wxString &bracketPair,
                                const wxString &delim,
                                bool checkOuterBracket = true);
   StringArray SeparateAllBrackets(const wxString &chunk,
                                   const wxString &bracketPair);
   StringArray SeparateSpaces(const wxString &chunk);
   StringArray SeparateDots(const wxString &chunk);
   StringArray SeparateBy(const wxString &chunk, const wxString &delim);
   
   
protected:
      
   bool IsCommand(const wxString &str);
   char GetClosingBracket(const char &openBracket);
   
private:
   
   wxString prefaceComment;
   wxString inlineComment;
   wxString theInstruction;
   wxString whiteSpace;
   StringArray theChunks;
   StringArray theCommandList;
   Gmat::BlockType theBlockType;
   bool isFunctionCall;
   wxString errorMsg;
};


#endif // TextParser_hpp

