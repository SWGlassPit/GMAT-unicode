//$Id: OrbitDesignerTime.hpp 9801 2011-08-26 17:38:38Z lindajun $
//------------------------------------------------------------------------------
//                           OrbitDesignerTime      
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
// Author:  Evelyn Hull
// Created: 2011/07/25
//
/**
 * Definition of the Orbit Designer time class
 */
//------------------------------------------------------------------------------

#ifndef OrbitDesignerTime_hpp
#define OrbitDesignerTime_hpp

#include "StringUtil.hpp"

class GMAT_API OrbitDesignerTime
{
public:
   OrbitDesignerTime(wxString epochFormat = wxT("UTCGregorian"), 
                     wxString epoch = wxT("01 Jan 2000 11:59:28.000"), 
                     Real RAAN = 306.6148021947984100,
                     wxString startTime = wxT("12:00:00.0"));
   OrbitDesignerTime(wxString epoch, wxString epochFormatStr, 
                     bool raanVal, Real RAAN, bool startTimeVal, 
                     wxString startTime);
   ~OrbitDesignerTime();

   Real FindRAAN();
   wxString FindStartTime(bool flag = false, Real lon = 0);

   void SetEpoch(wxString val);
   void SetStart(wxString val);
   void SetRAAN(Real val);

   //accessor functions
   Real GetRAAN();
   wxString GetStartTime();
   wxString GetEpoch();
   wxString GetEpochFormat();

   bool IsError();
   wxString GetError();

private:
   wxString epoch;
   wxString epochFormat;
   Real RAAN;
   wxString startTime;
   wxString errormsg;
   bool isError;
};

#endif

