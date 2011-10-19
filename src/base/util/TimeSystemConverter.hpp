//$Id: TimeSystemConverter.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                 TimeSystemConverter
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
 * Definition of the TimeSystemConverter class
 */
//------------------------------------------------------------------------------

#ifndef TimeSystemConverter_hpp
#define TimeSystemConverter_hpp

#include <iostream>
#include <iomanip>
#include <sstream>
#include "BaseException.hpp"
#include "EopFile.hpp"
#include "LeapSecsFileReader.hpp"
#include "GmatConstants.hpp"

// struct TimeSystemConverterExceptions
// {
   class GMAT_API UnimplementedException : public BaseException
   {
      public:
         UnimplementedException(const wxString &message =
         wxT("TimeSystemConverter: Conversion not implemented: "))
         : BaseException(message) {};
   };
   class GMAT_API TimeFileException : public BaseException
   {
      public:
         TimeFileException(const wxString &message =
         wxT("TimeSystemConverter: File is unknown: "))
         : BaseException(message) {};
   };
   class GMAT_API TimeFormatException : public BaseException
   {
      public:
         TimeFormatException(const wxString &message =
         wxT("TimeSystemConverter: Requested format not implemented: "))
         : BaseException(message) {};
   };
   class GMAT_API InvalidTimeException : public BaseException
   {
      public:
         InvalidTimeException(const wxString &message =
         wxT("TimeSystemConverter: Requested time is invalid: "))
         : BaseException(message) {};
   };
// };

namespace TimeConverterUtil
{
   // Specified in Math Spec section 2.3
   static const Real TDB_COEFF1                    = 0.001658;
   static const Real TDB_COEFF2                    = 0.00001385;
   static const Real M_E_OFFSET                    = 357.5277233;
   static const Real M_E_COEFF1                    = 35999.05034;
   static const Real T_TT_OFFSET                   = GmatTimeConstants::JD_OF_J2000;
   static const Real T_TT_COEFF1                   = GmatTimeConstants::DAYS_PER_JULIAN_CENTURY;
   static const Real L_B                           = 1.550505e-8;
   static const Real TCB_JD_MJD_OFFSET             = 2443144.5;
   static const Real NUM_SECS                      = GmatTimeConstants::SECS_PER_DAY;

   enum TimeSystemTypes
   {
      A1MJD = 0,
      TAIMJD,
      UTCMJD,
      UT1MJD,
      TDBMJD,
      TCBMJD,
      TTMJD,
      A1,
      TAI,
      UTC,
      UT1,
      TDB,
      TCB,
      TT,
      TimeSystemCount
   };

   static const wxString TIME_SYSTEM_TEXT[TimeSystemCount] =
   {
      wxT("A1Mjd"),
      wxT("TaiMjd"),
      wxT("UtcMjd"),
      wxT("Ut1Mjd"),
      wxT("TdbMjd"),
      wxT("TcbMjd"),
      wxT("TtMjd"),
      // New entries added by DJC
      wxT("A1"),
      wxT("TAI"),
      wxT("UTC"),
      wxT("UT1"),
      wxT("TDB"),
      wxT("TCB"),
      wxT("TT"),
   };

/*   Real Convert(const Real origValue,
                      const wxString &fromType,
                      const wxString &toType,
                      Real refJd = GmatTimeConstants::JD_NOV_17_1858);

   Real ConvertToTaiMjd(wxString fromType, Real origValue,
      Real refJd= GmatTimeConstants::JD_NOV_17_1858);
   Real ConvertFromTaiMjd(wxString toType, Real origValue,
      Real refJd= GmatTimeConstants::JD_NOV_17_1858);
 */
   
   Integer GMAT_API GetTimeTypeID(wxString &str);
      
   Real GMAT_API Convert(const Real origValue,
                const Integer fromType,
                const Integer toType,
                Real refJd);
   
   Real GMAT_API ConvertToTaiMjd(Integer fromType, Real origValue,
      Real refJd= GmatTimeConstants::JD_NOV_17_1858);
   Real GMAT_API ConvertFromTaiMjd(Integer toType, Real origValue,
      Real refJd= GmatTimeConstants::JD_NOV_17_1858);

   void GMAT_API SetEopFile(EopFile *eopFile);
   void GMAT_API SetLeapSecsFileReader(LeapSecsFileReader *leapSecsFileReader);
   
   void GMAT_API GetTimeSystemAndFormat(const wxString &type, wxString &system,
                               wxString &format);
   
   wxString GMAT_API ConvertMjdToGregorian(const Real mjd, Integer format = 1);   
   Real GMAT_API ConvertGregorianToMjd(const wxString &greg);
   void GMAT_API Convert(const wxString &fromType, Real fromMjd,
                const wxString &fromStr, const wxString &toType,
                Real &toMjd, wxString &toStr, Integer format = 1);
   
   bool GMAT_API ValidateTimeSystem(wxString sys);   
   bool GMAT_API ValidateTimeFormat(const wxString &format, const wxString &value,
                           bool checkValue = true);
   StringArray GMAT_API GetValidTimeRepresentations();
}

#endif // TimeSystemConverter_hpp
