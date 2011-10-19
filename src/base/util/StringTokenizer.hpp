//$Id: StringTokenizer.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              StringTokenizer
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author:  Joey Gurganus
// Created: 2004/05/14
//
/**
 * Definition of the StringTokenizer class base
 */
//------------------------------------------------------------------------------

#ifndef StringTokenizer_hpp
#define StringTokenizer_hpp

#include "gmatdefs.hpp"

class GMAT_API StringTokenizer
{
public:
   StringTokenizer();
   StringTokenizer(const wxString &str, const wxString &delim);
   StringTokenizer(const wxString &str, const wxString &delim,
                   bool insertDelim);
   ~StringTokenizer();
   
   // inline methods
   void SetDelimiters(const wxString &delim) { delimiters = delim; }
   wxString GetDelimiters() { return delimiters; }
   
   Integer CountTokens() const; 
   wxString GetToken(const Integer loc) const;
   const StringArray& GetAllTokens() const;
   
   void Set(const wxString &str, const wxString &delim);
   void Set(const wxString &str, const wxString &delim, bool insertDelim);
   
private:
   
   StringArray  stringTokens;
   wxString  delimiters;
   Integer      countTokens;
   
   void Parse(const wxString &str);
   void Parse(const wxString &str, bool insertDelim);
   
};

#endif // StringTokenizer_hpp
