//$Id: Epoch.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                               Epoch
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
// Created: 2005/06/20
//
/**
 * Definition of the Epoch class
 */
//------------------------------------------------------------------------------

#ifndef Epoch_hpp
#define Epoch_hpp

#include "gmatdefs.hpp"
#include "TimeConverter.hpp"

class GMAT_API Epoch
{
public:
    // Implements exception
    class EpochException : public BaseException
    {
       public:
          EpochException(const wxString &message =
           wxT("EpochException: Can't convert due to invalid date format"))
           : BaseException(message) {};
    };


   // Default constructor
   Epoch();
   Epoch(const wxString &mFormat);
   Epoch(const wxString &mFormat, const wxString &mValue);

   // Copy constructor
   Epoch(const Epoch &e);
   // Assignment operator
   Epoch& operator=(const Epoch &e);

   // Destructor
   virtual ~Epoch(void);

   // public methods
   wxString GetValue() const;
   wxString GetValue(const wxString &mFormat) const;
   bool        SetValue(const wxString &mFormat);
   bool        SetValue(const wxString &mFormat, const wxString &mValue);
   bool        SetValue(const Real e);

   Real        GetRealValue();
   Real        GetRealValue(const wxString &mFormat);
  
   bool        UpdateValue(Real e);

   wxString GetFormat() const;
   bool        SetFormat(const wxString &mFormat);

   bool        IsValidFormat(const wxString &label) const;

   wxString GetLabel() const;

protected:
   // Epoch's date format list
   enum DateFormat
   {
        TAI_MJD, TAI_GREGORIAN, UTC_MJD, UTC_GREGORIAN, DateFormatCount
   };
          
   static const wxString FORMAT[DateFormatCount];
   
private:
   wxString             format;
   wxString             value;
   mutable TimeConverter   timeConverter;

   // private methods
   void        DefineDefault();
   wxString GetFormatTrim(const wxString &mFormat) const;
};

#endif // Epoch_hpp
