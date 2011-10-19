//$Id: TimeConverter.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              TimeConverter
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
// Created: 2004/04/15
//
/**
 * Implements the TimeConverter class
 */
//------------------------------------------------------------------------------

#include "TimeConverter.hpp"

#include <cstdlib>			// Required for GCC 4.3

//-------------------------------------
// public methods
//-------------------------------------

//---------------------------------------------------------------------------
//  TimeConverter()
//---------------------------------------------------------------------------
/**
 * Creates default constructor.
 *
 */
TimeConverter::TimeConverter() :
   Converter(wxT("ModifiedJulian"))
{
}

//---------------------------------------------------------------------------
//  TimeConverter(const wxString &type)
//---------------------------------------------------------------------------
/**
 * Creates constructors with parameters.
 *
 * @param <typeStr> GMAT script string associated with this type of object.
 *
 */
TimeConverter::TimeConverter(const wxString &type) :
   Converter(type)
{
}

//---------------------------------------------------------------------------
//  TimeConverter(const TimeConverter &timeConverter)
//---------------------------------------------------------------------------
/**
 * Copy Constructor for base TimeConverter structures.
 *
 * @param <stateConverter> The original that is being copied.
 */
TimeConverter::TimeConverter(const TimeConverter &timeConverter) :
    Converter (timeConverter.type)
{
}

//---------------------------------------------------------------------------
//  ~TimeConverter()
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
TimeConverter::~TimeConverter()
{
}

//---------------------------------------------------------------------------
//  TimeConverter& operator=(const TimeConverter &timeConverter)
//---------------------------------------------------------------------------
/**
 * Assignment operator for TimeConverter structures.
 *
 * @param <converter> The original that is being copied.
 *
 * @return Reference to this object
 */
TimeConverter& TimeConverter::operator=(const TimeConverter &timeConverter)
{
    // Don't do anything if copying self
    if (&timeConverter == this)
        return *this;

    // Will fix it later
    return *this;
}

//---------------------------------------------------------------------------
//  Real TimeConverter::Convert(const Real time,
//                              const wxString &fromDateFormat,
//                              const wxString &toDateFormat)
//---------------------------------------------------------------------------
/**
 * Assignment operator for TimeConverter structures.
 *
 * @param <time>            Given Time
 * @param <fromDateFormat>  Time which is converted from date format
 * @param <toDateFormat>    Tiem which is converted to date format
 *
 * @return Converted time from the specific data format
 */
wxString TimeConverter::Convert(const wxString &time,
                                   const wxString &fromDateFormat,
                                   const wxString &toDateFormat)
{
   Real inTime, outTime;
   time.ToDouble(&inTime);
   wxString newTime = time;

   wxString timeBuffer;

   // Determine the input of date format
   if (fromDateFormat == wxT("TAIModJulian") && toDateFormat != wxT("TAIModJulian"))
   {
      if (toDateFormat == wxT("TAIGregorian"))
         return ModJulianToGregorian(inTime);

    // 20.02.06 - arg: changed to use enum types instead of strings
//      outTime = TimeConverterUtil::Convert(inTime, wxT("A1Mjd"), wxT("UtcMjd"),
//         GmatTimeConstants::JD_JAN_5_1941);
      outTime = TimeConverterUtil::Convert(inTime, TimeConverterUtil::A1MJD,
         TimeConverterUtil::UTCMJD, GmatTimeConstants::JD_JAN_5_1941);

      if (toDateFormat == wxT("UTCModJulian"))
      {
         timeBuffer << outTime;
         newTime = timeBuffer;
      }
      else if (toDateFormat == wxT("UTCGregorian"))
         newTime = ModJulianToGregorian(outTime);
   }
   else if (fromDateFormat == wxT("TAIGregorian") && toDateFormat != wxT("TAIGregorian"))
   {
      inTime = GregorianToModJulian(time);
      if (toDateFormat == wxT("TAIModJulian"))
      {
         timeBuffer << inTime;
         return(timeBuffer);
      }

    // 20.02.06 - arg: changed to use enum types instead of strings
//      outTime = TimeConverterUtil::Convert(inTime, wxT("A1Mjd"), wxT("UtcMjd"),
//         GmatTimeConstants::JD_JAN_5_1941);
      outTime = TimeConverterUtil::Convert(inTime, TimeConverterUtil::A1MJD,
         TimeConverterUtil::UTCMJD, GmatTimeConstants::JD_JAN_5_1941);

      if (toDateFormat == wxT("UTCGregorian"))
         newTime = ModJulianToGregorian(outTime);
      else if (toDateFormat == wxT("UTCModJulian"))
      {
         timeBuffer << outTime;
         return(timeBuffer);
      }
   }
   else if (fromDateFormat == wxT("UTCModJulian") && toDateFormat != wxT("UTCModJulian"))
   {
      if (toDateFormat == wxT("UTCGregorian"))
         return ModJulianToGregorian(inTime);

    // 20.02.06 - arg: changed to use enum types instead of strings
//      outTime = TimeConverterUtil::Convert(inTime, wxT("UtcMjd"), wxT("A1Mjd"),
//         GmatTimeConstants::JD_JAN_5_1941);
      outTime = TimeConverterUtil::Convert(inTime, TimeConverterUtil::UTCMJD,
         TimeConverterUtil::A1MJD, GmatTimeConstants::JD_JAN_5_1941);

      if (toDateFormat == wxT("TAIGregorian"))
         newTime = ModJulianToGregorian(outTime);
      else if (toDateFormat == wxT("TAIModJulian"))
      {
         timeBuffer << outTime;
         return timeBuffer;
      }
   }
   else if (fromDateFormat == wxT("UTCGregorian") && toDateFormat != wxT("UTCGregorian"))
   {
      inTime = GregorianToModJulian(time);
      if (toDateFormat == wxT("UTCModJulian"))
      {
         timeBuffer << inTime;
         return timeBuffer;
      }

    // 20.02.06 - arg: changed to use enum types instead of strings
//      outTime = TimeConverterUtil::Convert(inTime, wxT("UtcMjd"), wxT("A1Mjd"),
//         GmatTimeConstants::JD_JAN_5_1941);
      outTime = TimeConverterUtil::Convert(inTime, TimeConverterUtil::UTCMJD,
         TimeConverterUtil::A1MJD, GmatTimeConstants::JD_JAN_5_1941);

      if (toDateFormat == wxT("TAIModJulian"))
      {
         timeBuffer << outTime;
         return timeBuffer;
      }
      else if (toDateFormat == wxT("TAIGregorian"))
         newTime = ModJulianToGregorian(outTime);
   }

   return newTime;
}


wxString TimeConverter::ModJulianToGregorian(const Real mjTime)
{
   A1Mjd a1Mjd(mjTime);
   A1Date a1Date = a1Mjd.ToA1Date();
   GregorianDate gregorianDate(&a1Date);
   return gregorianDate.GetDate();
}


Real TimeConverter::GregorianToModJulian(const wxString greg)
{
   GregorianDate gregorianDate(greg);
   Real jules;

   if (!gregorianDate.IsValid())
      throw TimeConverterException();

   try
   {
      A1Date a1Date(gregorianDate.GetYMDHMS());
      jules = ModifiedJulianDate(a1Date.GetYear(),a1Date.GetMonth(),
                                 a1Date.GetDay(),a1Date.GetHour(),
                                 a1Date.GetMinute(),a1Date.GetSecond());
   }
   catch (Date::TimeRangeError&)
   {
      throw TimeConverterException();
   }

   return jules;
}
