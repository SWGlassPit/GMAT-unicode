//$Id: TimeSystemConverter.cpp 9513 2011-04-30 21:23:06Z djcinsb $ 
//------------------------------------------------------------------------------
//                              TimeSystemConverter
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
// Author:  Allison Greene
// Created: 2005/02/03
//
/**
 * Implements the TimeSystemConverter class
 */
//------------------------------------------------------------------------------

#include "TimeSystemConverter.hpp"
#include "A1Mjd.hpp"
#include "GregorianDate.hpp"
#include "DateUtil.hpp"            // for ModifiedJulianDate()
#include "StringUtil.hpp"          // for ToReal()
#include "GmatGlobal.hpp"          // for TIME_PRECISION
#include "MessageInterface.hpp"

//#define DEBUG_FIRST_CALL
//#define DEBUG_TIMECONVERTER_DETAILS
//#define DEBUG_GREGORIAN
//#define DEBUG_TIME_CONVERT

#ifdef DEBUG_FIRST_CALL
   static bool firstCallFired = false;
   static Real lastValue = -9999999.999999e99;
#endif


using namespace GmatMathUtil;

static EopFile *theEopFile;
static LeapSecsFileReader *theLeapSecsFileReader;

//------------------------------------------------------------------------------
// Integer GetTimeTypeID(wxString &str) 
//------------------------------------------------------------------------------
Integer TimeConverterUtil::GetTimeTypeID(wxString &str) 
{
    for (Integer i = 0; i < TimeSystemCount; i++)
    {
        if (str == TIME_SYSTEM_TEXT[i])
            return i;
    }

    return -1;
}

//---------------------------------------------------------------------------
//  Real TimeConverterUtil::Convert(const Real origValue,
//                                  const wxString &fromType,
//                                  const wxString &toType)
//---------------------------------------------------------------------------
/**
 * Assignment operator for TimeConverter structures.
 *
 * @param <origValue>            Given Time
 * @param <fromType>  Time which is converted from date format
 * @param <toType>    Tiem which is converted to date format
 *
 * @return Converted time from the specific data format 
 */
//---------------------------------------------------------------------------
Real TimeConverterUtil::Convert(const Real origValue,
                                const Integer fromType,
                                const Integer toType,
                                Real refJd)
{   
   #ifdef DEBUG_FIRST_CALL
      if ((firstCallFired == true) && (origValue < (lastValue-0.25)))
      {
         firstCallFired = false;
         MessageInterface::ShowMessage(
            wxT("Reset firstCallFired, orig == '%.12lf', last = '%.12lf'\n"),
            origValue, lastValue);
      }
      lastValue = origValue;
   #endif
   
   #ifdef DEBUG_TIMECONVERTER_DETAILS
      MessageInterface::ShowMessage(
         wxT("      TimeConverterUtil::Converting %.18lf in %s to %s; refJD = %.18lf\n"), origValue, 
         TIME_SYSTEM_TEXT[fromType].c_str(), TIME_SYSTEM_TEXT[toType].c_str(), refJd);
   #endif
      
   Real newTime =
      TimeConverterUtil::ConvertToTaiMjd(fromType, origValue, refJd);

   #ifdef DEBUG_TIMECONVERTER_DETAILS
      MessageInterface::ShowMessage(wxT("      TAI time =  %.18lf\n"), newTime);
   #endif
   
   Real returnTime =
      TimeConverterUtil::ConvertFromTaiMjd(toType, newTime, refJd);

   #ifdef DEBUG_FIRST_CALL
//       if (toType == wxT("TtMjd"))
      if (toType == TTMJD)
         firstCallFired = true;
   #endif

   #ifdef DEBUG_TIMECONVERTER_DETAILS
      MessageInterface::ShowMessage(wxT("      %s time =  %.18lf\n"), TIME_SYSTEM_TEXT[toType].c_str(), 
         returnTime);
   #endif

   return returnTime;
}

//---------------------------------------------------------------------------
// Real ConvertToTaiMjd(wxString fromType, Real origValue, Real refJd)
//---------------------------------------------------------------------------
Real TimeConverterUtil::ConvertToTaiMjd(Integer fromType, Real origValue,
                                        Real refJd)
{
   #ifdef DEBUG_FIRST_CALL
      if (!firstCallFired)
         MessageInterface::ShowMessage(
            wxT("   Converting %.11lf to TAI from %s\n"), origValue, TIME_SYSTEM_TEXT[fromType].c_str());
   #endif

   #ifdef DEBUG_TIMECONVERTER_DETAILS
      MessageInterface::ShowMessage( 
         wxT("      ***Converting %.18lf to TAI from %s\n"), origValue, 
         TIME_SYSTEM_TEXT[fromType].c_str());
   #endif
   
   switch(fromType)
   {
    case TimeConverterUtil::A1MJD:
    case TimeConverterUtil::A1:
        return (origValue -
             (GmatTimeConstants::A1_TAI_OFFSET/GmatTimeConstants::SECS_PER_DAY));
    case TimeConverterUtil::TAIMJD:
    case TimeConverterUtil::TAI:
        return origValue;
    case TimeConverterUtil::UTCMJD:
    case TimeConverterUtil::UTC:
    {
        Real offsetValue = 0;
    
        if (refJd != GmatTimeConstants::JD_NOV_17_1858)
        {
           // DJC: 6/16/05 Reversed order of difference so future times are positive
           // offsetValue = GmatTimeConstants::JD_NOV_17_1858 - refJd;
           offsetValue = refJd - GmatTimeConstants::JD_NOV_17_1858;
        }
    
        //loj: 4/12/05 Added
        if (theLeapSecsFileReader == NULL)
           throw TimeFileException
              (wxT("theLeapSecsFileReader is unknown\n"));
          
        // look up leap secs from file
        Real numLeapSecs =
           theLeapSecsFileReader->NumberOfLeapSecondsFrom(origValue + offsetValue);
  
        #ifdef DEBUG_TIMECONVERTER_DETAILS
           MessageInterface::ShowMessage(
              wxT("      CVT to TAI, Leap secs count = %.14lf\n"), numLeapSecs);
        #endif
    
        return (origValue + (numLeapSecs/GmatTimeConstants::SECS_PER_DAY));
    }
    case TimeConverterUtil::UT1MJD:
    case TimeConverterUtil::UT1:
    {
        if (theEopFile == NULL)
           throw TimeFileException(wxT("EopFile is unknown\n"));
        
        Real offsetValue = 0;
        
        if (refJd != GmatTimeConstants::JD_NOV_17_1858)
        {
           offsetValue = GmatTimeConstants::JD_NOV_17_1858 - refJd;
        }
    
        Real ut1Offset = theEopFile->GetUt1UtcOffset(origValue + offsetValue);
        Real utcOffset = theEopFile->GetUt1UtcOffset((origValue + offsetValue)
            - (ut1Offset/GmatTimeConstants::SECS_PER_DAY));
        
        return (TimeConverterUtil::ConvertToTaiMjd(TimeConverterUtil::UTCMJD,
                 (origValue - (utcOffset/GmatTimeConstants::SECS_PER_DAY)), refJd));
    }
    case TimeConverterUtil::TDBMJD:
    case TimeConverterUtil::TDB:
    {
       // This cleans up round-off error from differencing large numbers
       Real tttOffset = T_TT_OFFSET - refJd;

       // An approximation valid to the difference between TDB and TT; 1st term
       // here should be in TT rather than the input TDB, but we do not know TT
       Real t_TT = (origValue - tttOffset) / T_TT_COEFF1;
       Real m_E = (M_E_OFFSET + (M_E_COEFF1 * t_TT)) *
             GmatMathConstants::RAD_PER_DEG;

       Real offset = ((TDB_COEFF1 *Sin(m_E)) + (TDB_COEFF2 * Sin(2 * m_E))) /
             GmatTimeConstants::SECS_PER_DAY ;

       Real ttJd = origValue - offset;
       Real taiJd = TimeConverterUtil::ConvertToTaiMjd(TimeConverterUtil::TTMJD,
             ttJd, 0.0);

      #ifdef DEBUG_TDB
          MessageInterface::ShowMessage(wxT("TDB -> MJD\n"));
          MessageInterface::ShowMessage(wxT("   T_TT_COEFF1 = %.15le\n"), T_TT_COEFF1);
          MessageInterface::ShowMessage(wxT("   T_TT_OFFSET = %.15le\n"), T_TT_OFFSET);
          MessageInterface::ShowMessage(wxT("   M_E_OFFSET  = %.15le\n"), M_E_OFFSET);
          MessageInterface::ShowMessage(wxT("   M_E_COEFF1  = %.15le\n"), M_E_COEFF1);
          MessageInterface::ShowMessage(wxT("   TDB_COEFF1  = %.15le\n"), TDB_COEFF1);
          MessageInterface::ShowMessage(wxT("   TDB_COEFF2  = %.15le\n"), TDB_COEFF2);
          MessageInterface::ShowMessage(wxT("   origValue   = %.15le\n"), origValue);
          MessageInterface::ShowMessage(wxT("   refJd       = %.15le\n"), refJd);
          MessageInterface::ShowMessage(wxT("   t_TT        = %.15le\n"), t_TT);
          MessageInterface::ShowMessage(wxT("   m_E         = %.15le\n"), m_E);
          MessageInterface::ShowMessage(wxT("   offset      = %.15le\n"), offset);
          MessageInterface::ShowMessage(wxT("   ttJd        = %.15le\n"), ttJd);
          MessageInterface::ShowMessage(wxT("   taiJd       = %.15le\n"), taiJd);
          MessageInterface::ShowMessage(wxT("   tai         = %.15le\n"), taiJd - refJd);
      #endif

       return taiJd;
    }
    case TimeConverterUtil::TCBMJD:
    case TimeConverterUtil::TCB:
          throw UnimplementedException(
               wxT("Not Implemented - TCB to TAI"));
//      Real tdbMjd;
//      return ConvertToTaiMjd(wxT("TdbMjd"), tdbMjd);    
    case TimeConverterUtil::TTMJD:
    case TimeConverterUtil::TT:
          return (origValue -
             (GmatTimeConstants::TT_TAI_OFFSET/GmatTimeConstants::SECS_PER_DAY));
    default:
         ;
   }
   return 0.0;

}


//---------------------------------------------------------------------------
// Real ConvertFromTaiMjd(wxString toType, Real origValue, Real refJd)
//---------------------------------------------------------------------------
Real TimeConverterUtil::ConvertFromTaiMjd(Integer toType, Real origValue,
                                          Real refJd)
{
   #ifdef DEBUG_FIRST_CALL
      if (!firstCallFired)
         MessageInterface::ShowMessage(
            wxT("   Converting %.11lf from TAI to %s\n"), origValue,
            TIME_SYSTEM_TEXT[toType].c_str());
   #endif
   #ifdef DEBUG_TIMECONVERTER_DETAILS
      MessageInterface::ShowMessage(
         wxT("      ** Converting %.18lf from TAI to %s\n"), origValue, 
         TIME_SYSTEM_TEXT[toType].c_str());
   #endif
   
   switch (toType)
   {
      case TimeConverterUtil::A1MJD:
      case TimeConverterUtil::A1:
      {
          #ifdef DEBUG_TIMECONVERTER_DETAILS
             MessageInterface::ShowMessage(wxT("      In the 'a1' block\n"));
          #endif
          return (origValue +
                 (GmatTimeConstants::A1_TAI_OFFSET/GmatTimeConstants::SECS_PER_DAY));
      }
      case TimeConverterUtil::TAIMJD:
      case TimeConverterUtil::TAI:
      {
          #ifdef DEBUG_TIMECONVERTER_DETAILS
             MessageInterface::ShowMessage(wxT("      In the 'tai' block\n"));
          #endif
          // already in tai
          return origValue;
      }
      case TimeConverterUtil::UTCMJD:
      case TimeConverterUtil::UTC:
       {
          #ifdef DEBUG_TIMECONVERTER_DETAILS
             MessageInterface::ShowMessage(wxT("      In the 'utc' block\n"));
          #endif
          Real offsetValue = 0;
    
          if (refJd != GmatTimeConstants::JD_NOV_17_1858)
          {
             //offsetValue = GmatTimeConstants::JD_NOV_17_1858 - refJd;
             offsetValue = refJd - GmatTimeConstants::JD_NOV_17_1858;
          }
    
          //loj: 4/12/05 Added
          if (theLeapSecsFileReader == NULL)
             throw TimeFileException
                (wxT("theLeapSecsFileReader is unknown\n"));
          
          Real taiLeapSecs =
             theLeapSecsFileReader->NumberOfLeapSecondsFrom(origValue + offsetValue);
          
          Real utcLeapSecs =
             theLeapSecsFileReader->
             NumberOfLeapSecondsFrom((origValue + offsetValue)
                                     - (taiLeapSecs/GmatTimeConstants::SECS_PER_DAY));
    
          #ifdef DEBUG_TIMECONVERTER_DETAILS
             MessageInterface::ShowMessage(wxT("      offsetValue = %.17lf\n"),
                offsetValue);
             MessageInterface::ShowMessage(wxT("      Leap secs: tai = %.17lf, utc = %.17lf\n"),
                taiLeapSecs, utcLeapSecs);
          #endif
    
          if (utcLeapSecs == taiLeapSecs)
             return (origValue - (taiLeapSecs/GmatTimeConstants::SECS_PER_DAY));
          else
             return (origValue - (utcLeapSecs/GmatTimeConstants::SECS_PER_DAY));
       }
      case TimeConverterUtil::UT1MJD:
      case TimeConverterUtil::UT1:
       {
          #ifdef DEBUG_TIMECONVERTER_DETAILS
             MessageInterface::ShowMessage(wxT("      In the 'ut1' block\n"));
          #endif
          if (theEopFile == NULL)
             throw TimeFileException(
                   wxT("EopFile is unknown"));
    
          Real offsetValue = 0;
    
          if (refJd != GmatTimeConstants::JD_NOV_17_1858)
          {
             //offsetValue = GmatTimeConstants::JD_NOV_17_1858 - refJd;
             offsetValue = refJd - GmatTimeConstants::JD_NOV_17_1858;
          }
          // convert origValue to utc
          Real utcMjd = TimeConverterUtil::ConvertFromTaiMjd(TimeConverterUtil::UTCMJD, 
                    origValue, refJd);
          Real numOffset = 0;
    
          numOffset = theEopFile->GetUt1UtcOffset(utcMjd + offsetValue);
    
          #ifdef DEBUG_TIMECONVERTER_DETAILS
             MessageInterface::ShowMessage(wxT("   offsetValue = %.17lf\n"),
                offsetValue);
             MessageInterface::ShowMessage(
                wxT("   utcMjd = %.20lf, numOffset = %.20lf\n"), utcMjd, numOffset);
          #endif
          #ifdef DEBUG_FIRST_CALL
             if (!firstCallFired || (numOffset == 0.0))
                MessageInterface::ShowMessage(
                   wxT("   utcMjd = %.20lf, numOffset = %.20lf\n"), utcMjd, numOffset);
          #endif
    
          // add delta ut1 read from eop file
          //return (utcMjd + numOffset);
          return (utcMjd + (numOffset/GmatTimeConstants::SECS_PER_DAY));
       }
      case TimeConverterUtil::TDBMJD:
      case TimeConverterUtil::TDB:      
          {
             #ifdef DEBUG_TIMECONVERTER_DETAILS
                MessageInterface::ShowMessage(wxT("      In the 'tdb' block\n"));
             #endif
             // convert time to tt
             Real ttJd = TimeConverterUtil::ConvertFromTaiMjd(
                   TimeConverterUtil::TTMJD, origValue, refJd);

             // compute T_TT
             // This cleans up round-off error from differencing large numbers
             Real tttOffset = T_TT_OFFSET - refJd;
             Real t_TT = (origValue - tttOffset) / T_TT_COEFF1;

             // compute M_E
             Real m_E = (M_E_OFFSET + (M_E_COEFF1 * t_TT)) *
                   GmatMathConstants::RAD_PER_DEG;
             Real offset = ((TDB_COEFF1 *Sin(m_E)) +
                   (TDB_COEFF2 * Sin(2 * m_E))) / GmatTimeConstants::SECS_PER_DAY;
             Real tdbJd = ttJd + offset;
             return tdbJd;
          }
       case TimeConverterUtil::TCBMJD:
       case TimeConverterUtil::TCB:
       {
          #ifdef DEBUG_TIMECONVERTER_DETAILS
             MessageInterface::ShowMessage(wxT("      In the 'tcb' block\n"));
          #endif
          // convert time to tdb
          Real tdbMjd = TimeConverterUtil::ConvertFromTaiMjd(TimeConverterUtil::TDBMJD, 
                origValue, refJd);
          //Real jdValue = origValue;  // but this is TAI
          Real jdValue = tdbMjd;
          //Real offset = L_B * ((jdValue + refJd) - TCB_JD_MJD_OFFSET) * GmatTimeConstants::SECS_PER_DAY;
          Real offset = L_B * ((jdValue + refJd) - TCB_JD_MJD_OFFSET);
          // units of offset are in seconds, so convert to fraction of days
          //return ((offset / GmatTimeConstants::SECS_PER_DAY) + tdbMjd);
          return (offset + tdbMjd);
       }
       case TimeConverterUtil::TTMJD:
       case TimeConverterUtil::TT:
       {
          #ifdef DEBUG_TIMECONVERTER_DETAILS
             MessageInterface::ShowMessage(wxT("      In the 'tt' block\n"));
          #endif
          return (origValue +
                 (GmatTimeConstants::TT_TAI_OFFSET/GmatTimeConstants::SECS_PER_DAY));
       }       
       default:
        ;
   }

   return 0;
}


//---------------------------------------------------------------------------
// void SetEopFile(EopFile *eopFile)
//---------------------------------------------------------------------------
void TimeConverterUtil::SetEopFile(EopFile *eopFile)
{
   theEopFile = eopFile;
}


//---------------------------------------------------------------------------
// void SetLeapSecsFileReader(LeapSecsFileReader *leapSecsFileReader)
//---------------------------------------------------------------------------
void TimeConverterUtil::SetLeapSecsFileReader(LeapSecsFileReader *leapSecsFileReader)
{
   theLeapSecsFileReader = leapSecsFileReader;
}


//---------------------------------------------------------------------------
// void GetTimeSystemAndFormat(const wxString &type, wxString &system,
//                             wxString &format)
//---------------------------------------------------------------------------
/*
 * Returns time system and format from the input format.
 * For example, TAIModJulian gives TAI and ModJulian.
 *
 * @param  type    input time type
 * @param  system  output time type
 * @param  format  output time format
 *
 * @exception <TimeFormatException> thrown if input type is invalid
 */
//---------------------------------------------------------------------------
void TimeConverterUtil::GetTimeSystemAndFormat(const wxString &type,
                                               wxString &system,
                                               wxString &format)
{
   //-------------------------------------------------------
   // Get from time system and format
   //-------------------------------------------------------
   wxString::size_type loc = type.find(wxT("ModJulian"), 0);
   if (loc == type.npos)
      loc = type.find(wxT("Gregorian"), 0);
   
   if (loc == type.npos)
      throw TimeFormatException
         (wxT("\"") + type + wxT("\" is not a valid time format.\n")
          wxT("The allowed values are: [A1ModJulian, TAIModJulian, UTCModJulian, ")
          wxT("TTModJulian, A1Gregorian, TAIGregorian, UTCGregorian, TTGregorian]"));
   
   system = type.substr(0, loc);
   format = type.substr(loc);
}


//---------------------------------------------------------------------------
// wxString ConvertMjdToGregorian(const Real mjd, Integer format = 1)
//---------------------------------------------------------------------------
/**
 * Converts MJD to Gregorian date format.
 *
 * @param  format    1 = wxT("01 Jan 2000 11:59:28.000")
 *                   2 = wxT("2000-01-01T11:59:28.000")
 */
//---------------------------------------------------------------------------
wxString TimeConverterUtil::ConvertMjdToGregorian(const Real mjd,
                                                     Integer format)
{
   A1Mjd a1Mjd(mjd);
   A1Date a1Date = a1Mjd.ToA1Date();
   GregorianDate gregorianDate(&a1Date, format);
   #ifdef DEBUG_GREGORIAN
       MessageInterface::ShowMessage(wxT("------ In ConvertMjdToGregorian\n"));
       MessageInterface::ShowMessage(wxT("------ input mjd     = %.18lf\n"), mjd);
       MessageInterface::ShowMessage(wxT("------ A1Date        = %s\n"), 
          (a1Date.ToPackedCalendarString()).c_str());
       MessageInterface::ShowMessage(wxT("------ GregorianDate = %s\n"), 
          (gregorianDate.GetDate()).c_str());
   #endif
   return gregorianDate.GetDate();
}


//---------------------------------------------------------------------------
// Real ConvertGregorianToMjd(const wxString &greg)
//---------------------------------------------------------------------------
Real TimeConverterUtil::ConvertGregorianToMjd(const wxString &greg)
{
   GregorianDate gregorianDate(greg);
   Real jules;

   if (!gregorianDate.IsValid())
      throw TimeFormatException(
         wxT("Gregorian date '") + greg + wxT("' is not valid."));

   //MessageInterface::ShowMessage
   //   (wxT("==> TimeConverterUtil::ConvertGregorianToMjd() greg=%s\n"), greg.c_str());
   
   try
   {
      A1Date a1Date(gregorianDate.GetYMDHMS());
      
      #ifdef DEBUG_TIMECONVERTER_DETAILS
         MessageInterface::ShowMessage(wxT("Gregorian: %s\n"), 
            gregorianDate.GetYMDHMS().c_str());
         MessageInterface::ShowMessage(wxT("YMDHMS:    %d  %d  %d  %d  %d  %.11lf\n"), 
            a1Date.GetYear(),a1Date.GetMonth(), a1Date.GetDay(),
            a1Date.GetHour(), a1Date.GetMinute(),a1Date.GetSecond());
      #endif
      
      jules = ModifiedJulianDate(a1Date.GetYear(),a1Date.GetMonth(),
                                 a1Date.GetDay(),a1Date.GetHour(),
                                 a1Date.GetMinute(),a1Date.GetSecond());
    #ifdef DEBUG_GREGORIAN
       MessageInterface::ShowMessage(wxT("------ In ConvertGregorianToMjd\n"));
       MessageInterface::ShowMessage(wxT("------ input greg = %s\n"), greg.c_str());
       MessageInterface::ShowMessage(wxT("------ Gregorian  = %s\n"), 
            gregorianDate.GetYMDHMS().c_str());
       MessageInterface::ShowMessage(wxT("------ A1Date     = %s\n"), 
          (a1Date.ToPackedCalendarString()).c_str());
       MessageInterface::ShowMessage(wxT("------ YMDHMS:    =     %d  %d  %d  %d  %d  %.17lf\n"), 
            a1Date.GetYear(),a1Date.GetMonth(), a1Date.GetDay(),
            a1Date.GetHour(), a1Date.GetMinute(),a1Date.GetSecond());
       MessageInterface::ShowMessage(wxT("------ jules      = %s\n"), 
          (gregorianDate.GetDate()).c_str());
       MessageInterface::ShowMessage(wxT("------ jules (as Real)      = %.12f\n"), 
          jules);
    #endif
   }
   catch (Date::TimeRangeError& )
   {
      throw TimeFormatException(
         wxT("Gregorian date '") + greg +wxT("' appears to be out of range."));
   }

   //MessageInterface::ShowMessage
   //   (wxT("==> TimeConverterUtil::ConvertGregorianToMjd() jules=%.11f\n"), jules);
   
   return jules;
}


//---------------------------------------------------------------------------
// void Convert(const wxString &fromType, Real fromMjd,
//              const wxString &fromStr, const wxString &toType,
//              Real &toMjd, wxString &toStr, Integer format = 1)
//---------------------------------------------------------------------------
/*
 * Converts input time and time format to output format. If input fromMjd
 * is -999.999 it uses fromStr to get a input value.
 *
 * @param  fromType  input time system and format (A1ModJulian, etc)
 * @param  fromMjd   input time in mjd Real if fromType is ModJulian
 * @param  fromStr   input time in string to be used if fromMjd is -999.999
 * @param  toType    output time system and format (A1ModJulian, etc)
 * @param  toMjd     output time in mjd Real if toType is ModJulian, -999.999 otherwise
 * @param  toStr     output time string in toType format (1)
 * @param  format    1 = wxT("01 Jan 2000 11:59:28.000")
 *                   2 = wxT("2000-01-01T11:59:28.000")
 */
//---------------------------------------------------------------------------
void TimeConverterUtil::Convert(const wxString &fromType, Real fromMjd, 
                                const wxString &fromStr,
                                const wxString &toType, Real &toMjd,
                                wxString &toStr, Integer format)
{
   #ifdef DEBUG_TIME_CONVERT
   MessageInterface::ShowMessage
      (wxT("TimeConverterUtil::Convert() entered fromType=%s, fromMjd=%f, fromStr=%s\n")
       wxT("   toType=%s\n"), fromType.c_str(), fromMjd, fromStr.c_str(), toType.c_str());
   #endif
   
   Real fromMjdVal = fromMjd;
   bool convertToModJulian = false;
   Integer timePrecision = GmatGlobal::Instance()->GetTimePrecision();
   
   if (fromMjd == -999.999)
      convertToModJulian = true;
   
   //-------------------------------------------------------
   // Get from time system and format
   //-------------------------------------------------------
   wxString fromSystem ;
   wxString fromFormat;
   GetTimeSystemAndFormat(fromType, fromSystem, fromFormat);
   
   // Validate from time system
   if (!TimeConverterUtil::ValidateTimeSystem(fromSystem))
      throw TimeFormatException
         (wxT("\"") + fromSystem + wxT("\" is not a valid time system"));
   
   // Validate time format and value
   if (convertToModJulian)
      ValidateTimeFormat(fromFormat, fromStr);
   
   //-------------------------------------------------------
   // Get to time system and format
   //-------------------------------------------------------   
   wxString toSystem;
   wxString toFormat;
   GetTimeSystemAndFormat(toType, toSystem, toFormat);
   
   // Validate to time system
   if (!TimeConverterUtil::ValidateTimeSystem(toSystem))
      throw TimeFormatException
         (wxT("\"") + toSystem + wxT("\" is not a valid time system"));
   
   //-------------------------------------------------------
   // Compute from time in mjd
   //-------------------------------------------------------
   toMjd = -999.999;
   
   if (fromFormat == wxT("ModJulian"))
   {
      if (convertToModJulian)
      {
         wxString str;
         str = fromStr;
         double theVal;
         str.ToDouble(&theVal);
         fromMjdVal = theVal;
      }
   }
   else
   {
      #ifdef DEBUG_TIME_CONVERT
      MessageInterface::ShowMessage(wxT("===> Converting %s from Gregorian to MJD\n"),
      fromStr.c_str());
      #endif
      fromMjdVal = TimeConverterUtil::ConvertGregorianToMjd(fromStr);
   }
   
   #ifdef DEBUG_TIME_CONVERT
   MessageInterface::ShowMessage(wxT("===> fromMjdVal=%.12f\n"), fromMjdVal);
   MessageInterface::ShowMessage(wxT("===> fromType=%s\n"), fromType.c_str());
   MessageInterface::ShowMessage(wxT("===> toType=%s\n"), toType.c_str());
   #endif
   
   //-------------------------------------------------------
   // Compute to time in mjd
   //-------------------------------------------------------
   if (fromType != toType)
   {
      Integer fromId = TimeConverterUtil::GetTimeTypeID(fromSystem);
      Integer toId = TimeConverterUtil::GetTimeTypeID(toSystem);
      toMjd = TimeConverterUtil::Convert(fromMjdVal, fromId, 
                                         toId, GmatTimeConstants::JD_JAN_5_1941);
   }
   else
   {
      //toMjd = fromMjd;  // wcs 2007.05.01 A1ModJulian fix for Bug 807
      toMjd = fromMjdVal;
   }
   
   //-------------------------------------------------------
   // Convert to output format
   //-------------------------------------------------------
   if (toFormat == wxT("ModJulian"))
      toStr = GmatStringUtil::ToString(toMjd, timePrecision);
   else
      toStr = TimeConverterUtil::ConvertMjdToGregorian(toMjd, format);
   
   #ifdef DEBUG_TIME_CONVERT
   MessageInterface::ShowMessage
      (wxT("TimeConverterUtil::Convert() leaving toMjd=%f, toStr=%s\n"), toMjd,
       toStr.c_str());
   #endif
}


//---------------------------------------------------------------------------
// bool TimeConverterUtil::ValidateTimeSystem(wxString sys)
//---------------------------------------------------------------------------
bool TimeConverterUtil::ValidateTimeSystem(wxString sys)
{
   for (Integer i = 0; i < TimeSystemCount; ++i)
      if (TIME_SYSTEM_TEXT[i] == sys)
         return true;
         
   return false;
}


//---------------------------------------------------------------------------
// bool TimeConverterUtil::ValidateTimeFormat(const wxString &format, 
//                                            const wxString &value,
//                                            bool checkValue = true)
//---------------------------------------------------------------------------
bool TimeConverterUtil::ValidateTimeFormat(const wxString &format, 
                                           const wxString &value,
                                           bool checkValue)
{
   bool retval = false;
   
   wxString timeFormat = wxT("ModJulian");
   if (format.find(wxT("Gregorian")) != format.npos)
      timeFormat = wxT("Gregorian");

   #if DEBUG_VALIDATE_TIME
   MessageInterface::ShowMessage
      (wxT("TimeConverterUtil::ValidateTimeFormat() format=%s, timeFormat=%s, ")
       wxT("value=%s\n"), format.c_str(), timeFormat.c_str(), value.c_str());
   #endif
   
   if (timeFormat == wxT("Gregorian"))
   {
      retval = DateUtil::IsValidGregorian(value, false);
      
      if (!retval)
          throw TimeFormatException
             (wxT("Gregorian date \"") + value + wxT("\" is not valid."));

      if (checkValue)
      {
         retval = DateUtil::IsValidGregorian(value, true);

         if (!retval)
             throw TimeFormatException
                (wxT("Gregorian date \"") + value + wxT("\" is not valid - time specified must be \"04 Oct 1957 12:00:00.000\" or later"));
      }

   }
   else if (timeFormat == wxT("ModJulian"))
   {
      Real rval;
      if (GmatStringUtil::ToReal(value, rval))
      {
         // Sputnik launched Oct 4, 1957 = 6116 MJ; don't accept earlier epochs.
//         if (rval >= 6116)
//            retval = true;
         if ((checkValue) && (rval < 6116))
         {
            throw InvalidTimeException
               (wxT("ModJulian Time \"") + value + wxT("\" is not valid - time specified must be >= 6116.00"));
         }
      }
      else
      {
//      if (!retval)
         throw InvalidTimeException
            (wxT("ModJulian Time \"") + value + wxT("\" is not valid."));
      }
   }
   else
   {
      throw TimeFormatException
         (wxT("Invalid Time Format \"") + format + wxT("\" found."));
   }
   
   return true;
}


//---------------------------------------------------------------------------
// StringArray TimeConverterUtil::GetValidTimeRepresentations()
//---------------------------------------------------------------------------
StringArray TimeConverterUtil::GetValidTimeRepresentations()
{
   StringArray systems;
   for (Integer i = A1; i < TimeSystemCount; ++i)
   {
      if ((i != UT1) && (i != TDB) && (i != TCB))
         systems.push_back(TIME_SYSTEM_TEXT[i] + wxT("ModJulian"));
   }
   for (Integer i = A1; i < TimeSystemCount; ++i)
   {
      if ((i != UT1) && (i != TDB) && (i != TCB))
         systems.push_back(TIME_SYSTEM_TEXT[i] + wxT("Gregorian"));
   }
   return systems;
}
