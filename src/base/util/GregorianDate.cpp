//$Id: GregorianDate.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                             GregorianDate 
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
// Created: 2004/07/14
//
/**
 * Definition of the Gregorian class base
 */
//------------------------------------------------------------------------------

#include "GregorianDate.hpp"
#include "TimeTypes.hpp"
#include "DateUtil.hpp"         // for IsValidTime()
#include "MessageInterface.hpp"
#include <algorithm>
#include <wx/sstream.h>
#include <wx/txtstrm.h>

//-------------------------------------
// public methods
//-------------------------------------

//---------------------------------------------------------------------------
//  GregorianDate()
//---------------------------------------------------------------------------
/**
 * Creates default constructor.
 */
//---------------------------------------------------------------------------
GregorianDate::GregorianDate()
{
   SetDate(wxT("01 Jan 2000 12:00:00.000"));
}

//---------------------------------------------------------------------------
//  GregorianDate(const wxString &str)
//---------------------------------------------------------------------------
/**
 * Creates constructor with parameters.
 *
 * @param <str>   Given String of Date
 */
//---------------------------------------------------------------------------
GregorianDate::GregorianDate(const wxString &str)
{
   SetDate(str);   
}

//---------------------------------------------------------------------------
//  GregorianDate(Date *newDate, Integer format = 1)
//---------------------------------------------------------------------------
/**
 * Creates default constructor with new Date.
 */
//---------------------------------------------------------------------------
GregorianDate::GregorianDate(Date *newDate, Integer format)
{
   outFormat = format;
   Initialize(wxT(""));
   SetDate(newDate, format);
}


//---------------------------------------------------------------------------
//  ~GregorianDate()
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
//---------------------------------------------------------------------------
GregorianDate::~GregorianDate()
{
}

    
//---------------------------------------------------------------------------
//  wxString GetDate() const
//---------------------------------------------------------------------------
/**
 * Get the date in string.
 *
 * @return the date in string. 
 */
//---------------------------------------------------------------------------
wxString GregorianDate::GetDate() const
{
   return stringDate;
}

//---------------------------------------------------------------------------
//  bool SetDate(const wxString &str) 
//---------------------------------------------------------------------------
/**
 * Set the date in string. 
 *
 * @return        return flag indicator (true = successful; otherwise, false) 
 */
//---------------------------------------------------------------------------
bool GregorianDate::SetDate(const wxString &str) 
{
    Initialize(str); // for now....
    ParseOut(str);

    return true;
}


//---------------------------------------------------------------------------
//  bool SetDate(Date *newDate, Integer format = 1) 
//---------------------------------------------------------------------------
/**
 * Set the new date in Date
 *
 * @param   format    1 = wxT("01 Jan 2000 11:59:28.000")
 *                    2 = wxT("2000-01-01T11:59:28.000")
 * @return  return flag indicator (true = successful; otherwise, false) 
 * 
 */
//---------------------------------------------------------------------------
bool GregorianDate::SetDate(Date *newDate, Integer format) 
{
   // Check validity on date first then convert to string
   if (!newDate->IsValid())
   { 
      MessageInterface::ShowMessage(wxT("Warning:  Can't set date to string\n"));
      return (isValid = false); 
   }  
   
   wxString temp;
   
   // Convert the date format in string
   if (format == 2)
   {
      temp += NumToString(newDate->GetYear()) + wxT("-");
      temp += NumToString(newDate->GetMonth()) + wxT("-");
      temp += NumToString(newDate->GetDay()) + wxT("T");
      temp += NumToString(newDate->GetHour()) + wxT(":");
      temp += NumToString(newDate->GetMinute()) + wxT(":");
      temp += NumToString(newDate->GetSecond());
      stringDate = temp;
   }
   else
   {
      temp = NumToString(newDate->GetDay());
      temp += wxT(" ") + GetMonthName(newDate->GetMonth()) + wxT(" "); 
      temp += NumToString(newDate->GetYear()) + wxT(" ");
      temp += NumToString(newDate->GetHour()) + wxT(":");
      temp += NumToString(newDate->GetMinute()) + wxT(":");
      temp += NumToString(newDate->GetSecond());
      stringDate = temp;
   }
   
   stringYMDHMS = newDate->ToPackedCalendarString();
   type = wxT("Gregorian");
   
   return (isValid = true);
}

//---------------------------------------------------------------------------
//  wxString GetYMDHMS() const
//---------------------------------------------------------------------------
/**
 * Get YYYYMMDD.HHMMSSmmm from Gregorian format in string 
 *
 * @return    string in YYYYMMDD.HHMMSSmmm 
 * 
 */
//---------------------------------------------------------------------------
wxString GregorianDate::GetYMDHMS() const
{
   return stringYMDHMS;
}

//---------------------------------------------------------------------------
//  bool IsValid() const 
//---------------------------------------------------------------------------
/**
 * Determines if the date is valid or not.
 *
 * @return        return flag indicator (true = valid; otherwise, false) 
 */
//---------------------------------------------------------------------------
bool GregorianDate::IsValid() const
{
    return isValid;
}

//---------------------------------------------------------------------------
//  static bool IsValid(const wxString &greg)
//---------------------------------------------------------------------------
/**
 * Determines if input date is valid or not.
 *   Valid format is dd mmm yyyy hh:mm:ss.mmm.
 *   For example, 01 Jan 2000 12:00:00.000
 *
 * @param  greg  input gregorian string
 *
 * @return true if time is in valid Gregorian format; otherwise, false
 */
//---------------------------------------------------------------------------
bool GregorianDate::IsValid(const wxString &greg)
{
   return DateUtil::IsValidGregorian(greg);
}

//-------------------------------------
// private methods
//-------------------------------------

//---------------------------------------------------------------------------
//  void Initialize(const wxString &str) 
//---------------------------------------------------------------------------
/**
 * Initializes values of data method
 */
//---------------------------------------------------------------------------
void GregorianDate::Initialize(const wxString &str)
{
   stringDate = str;
   stringYMDHMS = wxT("");
   type = wxT("Gregorian");
   isValid = false;
}


//---------------------------------------------------------------------------
//  void ParseOut(const wxString &str) 
//---------------------------------------------------------------------------
/**
 * Parse out the string and check validity
 */
//---------------------------------------------------------------------------
void GregorianDate::ParseOut(const wxString &str) 
{
   #if DEBUG_GREGORIAN_VALIDATE
   //MessageInterface::ShowMessage(wxT("==> GregorianDate::ParseOut() str=%s\n"), str.c_str());
   #endif
   
   // Check if non-empty string then parse out; otherwise, nothing. 
   if (str != wxT(""))
   {
      StringTokenizer dateToken(str, wxT(" "));

      if (dateToken.CountTokens() == 4)
      {
         wxString issString = (dateToken.GetToken(0));
         wxStringInputStream issStringStream(issString);
         wxTextInputStream iss(issStringStream);
         Integer dayNum, yearNum;

         // Get the number
         iss >> dayNum;

         // Get the year
         yearNum = ToInteger(dateToken.GetToken(2)); 

         // Check validity for year
         if (dateToken.GetToken(2).length() != 4 || yearNum < 1950)
            return;
//            throw GregorianDateException();

         // Check validity for month 
//          std::vector<wxString>::iterator pos;
//          pos = find(monthName.begin(),monthName.end(),
//                     dateToken.GetToken(1));

//          if (pos == monthName.end())
//             return;
// //            throw GregorianDateException();

         bool monthFound = false;
         Integer monthNum = 0;
         for (int i=0; i<12; i++)
         {
            if (GmatTimeConstants::MONTH_NAME_TEXT[i] == dateToken.GetToken(1))
            {
               monthFound = true;
               monthNum = i+1;
               break;
            }
         }
         
         if (!monthFound)
            return;

//          Integer monthNum;
//          monthNum = (Integer) distance(monthName.begin(),pos) + 1;

         wxString tempYMD;
         tempYMD = dateToken.GetToken(2) + NumToString(monthNum); 
         if (dateToken.GetToken(0).length() == 1)
            tempYMD += wxT("0");
         tempYMD += dateToken.GetToken(0) + wxT("."); 

         // Start with time
         StringTokenizer timeToken(dateToken.GetToken(3),wxT(":"));  

         if (timeToken.CountTokens() == 3)
         {
//            // Check length of time format
//            if (timeToken.GetToken(0).length() != 2 ||
//                timeToken.GetToken(1).length() != 2 ||
//                timeToken.GetToken(2).length() != 6)
//            {
//               MessageInterface::ShowMessage(
//                  wxT("\nWarning: invalid Gregorian format with time")); 
//               return;
//            }

            // Check length of the hour format
            if (timeToken.GetToken(0).length() != 2)
            {
               MessageInterface::ShowMessage(
                  wxT("\nWarning: invalid Gregorian time for hours format(HH)\n")); 
               return;
            }
            // Check length of the minute format
            if (timeToken.GetToken(1).length() != 2)
            {
               MessageInterface::ShowMessage(
                  wxT("\nWarning: invalid Gregorian time for minutes format(MM)\n")); 
               return;
            }
            if (timeToken.GetToken(2).length() != 6)
            {
               MessageInterface::ShowMessage(
                  wxT("\nWarning: invalid Gregorian time for seconds format(SS.mmm)\n")); 
               return;
            }

            // Get hour and minute
            Integer hour, minute;
            hour = ToInteger(timeToken.GetToken(0)); 
            minute = ToInteger(timeToken.GetToken(1)); 
           
            tempYMD += timeToken.GetToken(0) + timeToken.GetToken(1);

            // Finally start with seconds
            wxString strSeconds = timeToken.GetToken(2);
            timeToken.Set(strSeconds,wxT(".")); 

            // Check time format in second
            if (timeToken.CountTokens() != 2 || 
                timeToken.GetToken(0).length() != 2 ||
                timeToken.GetToken(1).length() != 3)
            {
               MessageInterface::ShowMessage(
                  wxT("\nWarning: invalid Gregorian format with seconds")); 
               return;
            }
            
            tempYMD += timeToken.GetToken(0) + timeToken.GetToken(1);

            // Get real number in seconds
            Real second = ToReal(strSeconds); 
   #if DEBUG_GREGORIAN_VALIDATE
            //MessageInterface::ShowMessage
            //   (wxT("==> GregorianDate::ParseOut() second=%.10f\n"), second);
            #endif
            
            // Finally check validity for the date  
            if (!IsValidTime(yearNum,monthNum,dayNum,hour,minute,second))
            {
               MessageInterface::ShowMessage(
                  wxT("\nWarning: invalid Gregorian format from DateUtil")); 
               return;
            } 
            
            stringYMDHMS = tempYMD;
            //MessageInterface::ShowMessage
            //   (wxT("==> GregorianDate::ParseOut() stringYMDHMS=%s\n"),
            //    stringYMDHMS.c_str());
         }                    
         isValid = true;
      }
      else
      {
         MessageInterface::ShowMessage(
             wxT("\nWarning: invalid Gregorian format: %s"),str.c_str());  
      }
   }
   else
   {
       MessageInterface::ShowMessage(
         wxT("\nWarning: invalid Gregorian format since there is no value."));  
   } 

}

//---------------------------------------------------------------------------
//  wxString NumToString(const Integer num) 
//---------------------------------------------------------------------------
/**
 * Return the string from number.
 * 
 * @param <num>    Given number of date including day and time
 * 
 * @return     Return the string from the number.
 */
//---------------------------------------------------------------------------
wxString GregorianDate::NumToString(const Integer num)
{
   wxString temp = wxString::Format(wxT("%02d"), num);
   return temp;
}

//---------------------------------------------------------------------------
//  wxString NumToString(const Real num) 
//---------------------------------------------------------------------------
/**
 * Return the string from real number.
 * 
 * @param <num>    Given real number 
 * 
 * @return     Return the string from the real number.
 */
//---------------------------------------------------------------------------
wxString GregorianDate::NumToString(const Real num)
{
   wxString temp;

   if (num < 10.0)
      temp = wxT("0");

   temp << num;

   return temp;
}

//---------------------------------------------------------------------------
//  Integer ToInteger(const wxString &str) 
//---------------------------------------------------------------------------
/**
 * Return the integer number from string.
 * 
 * @param <str>    Given string
 * 
 * @return     Return the integer number from string.
 */
//---------------------------------------------------------------------------
Integer GregorianDate::ToInteger(const wxString &str)
{
   Integer num;
   long thenum;
   str.ToLong(&thenum);
   num = thenum;
   return num;
}

//---------------------------------------------------------------------------
//  Real ToReal(const wxString &str) 
//---------------------------------------------------------------------------
/**
 * Return the integer number from string.
 * 
 * @param <str>    Given string
 * 
 * @return     Return the real number from string.
 */
//---------------------------------------------------------------------------
Real GregorianDate::ToReal(const wxString &str)
{
   Real num;
   double thenum;
   str.ToDouble(&thenum);
   num = thenum;
   return num;
}

//---------------------------------------------------------------------------
//  wxString GetMonthName(const Integer month) 
//---------------------------------------------------------------------------
/**
 * Return the string from number.
 * 
 * @param <month> Given number of and month
 * 
 * @return        Return the string from the number.
 */
//---------------------------------------------------------------------------
wxString GregorianDate::GetMonthName(const Integer month)
{
   if (month < 1 || month > 12) 
   {
      MessageInterface::ShowMessage(wxT("Warning:  bad month!"));
      isValid = false;
      // @todo - will add new exception for throwing
      return wxT("");
   }

   return GmatTimeConstants::MONTH_NAME_TEXT[month-1];
}
