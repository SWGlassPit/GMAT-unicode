//$Id: GregorianDate.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              GregorianDate     
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
// Created: 2004/05/19
//
/**
 * Definition of the GregorianDate class base
 */
//------------------------------------------------------------------------------

#ifndef GregorianDate_hpp
#define GregorianDate_hpp

#include <iostream>
#include <sstream>
#include "gmatdefs.hpp"
#include "Date.hpp"
#include "DateUtil.hpp"
#include "StringTokenizer.hpp"
#include "MessageInterface.hpp"

class GMAT_API GregorianDate
{
public:
   // Implement exception
   class GregorianDateException : public BaseException
   { 
      public: 
         GregorianDateException(const wxString& message =
          wxT("GregorianDateException:  Invalid date format"))
         : BaseException(message) {};
   };
   
   GregorianDate();
   GregorianDate(const wxString &str);
   GregorianDate(Date *newDate, Integer format = 1);
   ~GregorianDate();
   
   wxString      GetDate() const;
   bool             SetDate(const wxString &str);
   bool             SetDate(Date *newDate, Integer format = 1);
   
   wxString      GetType() const;
   bool             SetType(const wxString &str);
   
   wxString      GetYMDHMS() const;
   
   bool             IsValid() const;
   static bool      IsValid(const wxString &greg);
   
private:
   // function method
   void           Initialize(const wxString &str);
   void           ParseOut(const wxString &str);
   
   wxString    NumToString(const Integer num);
   wxString    NumToString(const Real num);
   Integer        ToInteger(const wxString &str);
   Real           ToReal(const wxString &str);
   
   wxString    GetMonthName(const Integer month);
   
   // data method
   wxString      stringDate;
   wxString      stringYMDHMS;
   wxString      type;
   Integer          outFormat;
   bool             isValid;
   bool             initialized;
};

#endif // GregorianDate_hpp
