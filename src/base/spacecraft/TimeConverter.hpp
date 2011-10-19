//$Id: TimeConverter.hpp 9513 2011-04-30 21:23:06Z djcinsb $ 
//------------------------------------------------------------------------------
//                                 TimeConverter
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
// Created: 2004/03/22
//
/**
 * Definition of the TimeConverter class
 */
//------------------------------------------------------------------------------

#ifndef TimeConverter_hpp
#define TimeConverter_hpp

#include <iostream>
#include <iomanip>
#include <sstream>
#include "BaseException.hpp"
#include "Converter.hpp"
#include "A1Date.hpp"
#include "A1Mjd.hpp"
#include "UtcDate.hpp"
#include "DateUtil.hpp"
#include "GregorianDate.hpp"

#include "TimeSystemConverter.hpp"

class GMAT_API TimeConverter : public Converter
{
public:
    // Implements exception
    class TimeConverterException : public BaseException
    {
       public: 
          TimeConverterException(const wxString &message = 
           wxT("TimeConverterException: Can't convert due to invalid date"))
           : BaseException(message) {};
    };

    // Default constructor
    TimeConverter();
    TimeConverter(const wxString &name);
    TimeConverter(const wxString &typeStr, const wxString &name);
    // Copy constructor
    TimeConverter(const TimeConverter &timeConverter);
    // Assignment operator
    TimeConverter& operator=(const TimeConverter &timeConverter);

    // Destructor
    virtual ~TimeConverter();

    // public method 
    wxString Convert(const wxString &time, 
                        const wxString &fromDateFormat,
                        const wxString &toDateFormat);

protected:
   wxString          ModJulianToGregorian(const Real mjTime);
   Real                 GregorianToModJulian(const wxString greg);

private:

};

#endif // TimeConverter_hpp
