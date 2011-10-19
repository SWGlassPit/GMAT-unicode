//$Id: Anomaly.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                             Anomaly 
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
// Author:  Joey Gurganus 
// Created: 2005/02/17
//
/**
 * Implements Anomaly class to compute the true, mean, and eccentric anomaly 
 * using the semi-major axis and eccentricity.
 *
 */
//------------------------------------------------------------------------------

#include "Anomaly.hpp"
#include "MessageInterface.hpp"
#include "Keplerian.hpp"
#include "RealUtilities.hpp"
#include "GmatConstants.hpp"
#include "UtilityException.hpp"
#include "StringUtil.hpp"
#include <sstream>

//#define DEBUG_ANOMALY 1

using namespace GmatMathUtil;
using namespace GmatMathConstants;

//---------------------------------
// static data
//---------------------------------
const wxString Anomaly::ANOMALY_LONG_TEXT[AnomalyTypeCount] =
{
   wxT("True Anomaly"), wxT("Mean Anomaly"), wxT("Eccentric Anomaly"), wxT("Hyperbolic Anomaly"),
};

const wxString Anomaly::ANOMALY_SHORT_TEXT[AnomalyTypeCount] =
{
   wxT("TA"), wxT("MA"), wxT("EA"), wxT("HA"),
};


//---------------------------------
//  public methods
//---------------------------------

//------------------------------------------------------------------------------
//  Anomaly() 
//------------------------------------------------------------------------------
/**
 * Default constructor.
 */
//------------------------------------------------------------------------------
Anomaly::Anomaly() :
   mSma(0.0), mEcc(0.0), mAnomalyInRad (0.0), mType(TA)
{
}


//------------------------------------------------------------------------------
// Anomaly(Real sma, Real ecc, Real value, AnomalyType type = TA,
//         bool valueInRadians = false)
//------------------------------------------------------------------------------
/**
 * Constructor with parameters.
 * 
 * @param <sma>    Semimajor axis.
 * @param <ecc>    Eccentricity.
 * @param <value>  Anomaly value. 
 * @param <type>   Anomaly type.
 * @param <valueInRadians>  true if value is in radians
 */
//------------------------------------------------------------------------------
Anomaly::Anomaly(Real sma, Real ecc, Real value, AnomalyType type,
                 bool valueInRadians) :
   mSma(sma), mEcc(ecc), mAnomalyInRad (value), mType(type)
{
   if (!valueInRadians)
      mAnomalyInRad = value * RAD_PER_DEG;
   
   #ifdef DEBUG_ANOMALY
   MessageInterface::ShowMessage
      (wxT("Anomaly::Anomaly() mEcc=%f, mAnomalyInRad=%f, mType=%d (%s)\n"),
       mEcc, mAnomalyInRad, mType, GetTypeString().c_str());
   #endif
}


//------------------------------------------------------------------------------
// Anomaly(Real sma, Real ecc, Real value, const wxString &type = wxT("TA"),
//         bool valueInRadians = false)
//------------------------------------------------------------------------------
Anomaly::Anomaly(Real sma, Real ecc, Real value, const wxString &type,
                 bool valueInRadians)
{
   Anomaly(sma, ecc, value, GetType(type), valueInRadians);
}


//------------------------------------------------------------------------------
//   Anomaly(const Anomaly &anomaly)
//------------------------------------------------------------------------------
/**
 * Copy Constructor for base Anomaly structures.
 *
 * @param <anomaly> The original that is being copied.
 */
//------------------------------------------------------------------------------
Anomaly::Anomaly(const Anomaly &anomaly) :
   mSma          (anomaly.mSma),
   mEcc          (anomaly.mEcc),
   mAnomalyInRad (anomaly.mAnomalyInRad),
   mType         (anomaly.mType)
{
}


//------------------------------------------------------------------------------
//  Anomaly& operator=(const Anomaly &anomaly)
//------------------------------------------------------------------------------
/**
 * Assignment operator for Anomaly structures.
 *
 * @param <anomaly> The original that is being copied.
 *
 * @return Reference to this object.
 */
//------------------------------------------------------------------------------
Anomaly& Anomaly::operator=(const Anomaly &anomaly)
{
   if (&anomaly != this)
   {
      mSma           = anomaly.mSma;
      mEcc           = anomaly.mEcc;
      mAnomalyInRad  = anomaly.mAnomalyInRad;
      mType          = anomaly.mType;
   }
   
   return *this;
}


//------------------------------------------------------------------------------
//  ~Anomaly() 
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
Anomaly::~Anomaly() 
{
}


//------------------------------------------------------------------------------
// void Set(Real sma, Real ecc, Real value, AnomalyType type,
//          bool valueInRadians = false)
//------------------------------------------------------------------------------
/**
 * Set the whole anomaly with parameters.
 * 
 * @param <sma>    Semimajor axis
 * @param <ecc>    Eccentricity
 * @param <value>  Anomaly value
 * @param <type>   Anomaly type
 * @param <valueInRadians>  true if value is in radians
 */
//------------------------------------------------------------------------------
void Anomaly::Set(Real sma, Real ecc, Real value, AnomalyType type,
                  bool valueInRadians)
{
   mSma   = sma;
   mEcc   = ecc;
   mType  = type;
   
   if (valueInRadians)
      mAnomalyInRad = value;
   else
      mAnomalyInRad = value * RAD_PER_DEG;
}


//------------------------------------------------------------------------------
// void Set(Real sma, Real ecc, Real value, const wxString &type,
//          bool valueInRadians = false)
//------------------------------------------------------------------------------
/**
 * Set the whole anomaly with parameters.
 * 
 * @param <sma>    Semimajor axis
 * @param <ecc>    Eccentricity
 * @param <value>  Anomaly value
 * @param <type>   Anomaly type string
 * @param <valueInRadians>  true if value is in radians
 */
//------------------------------------------------------------------------------
void Anomaly::Set(Real sma, Real ecc, Real value, const wxString &type,
                  bool valueInRadians)
{
   #ifdef DEBUG_ANOMALY
      MessageInterface::ShowMessage(wxT("Anomaly::Set called with AnomalyType = %s\n"),
            type.c_str());
   #endif
   Set(sma, ecc, value, GetType(type), valueInRadians);
}


//------------------------------------------------------------------------------
// Real GetValue(bool inRadians = false) const
//------------------------------------------------------------------------------
/** 
 * Gets anomaly value.
 * 
 * @param <inRadians>  true if output value in radians is requested
 *
 * @return Anomaly value.
 */
//------------------------------------------------------------------------------
Real Anomaly::GetValue(bool inRadians) const
{
   Real value = mAnomalyInRad;
   
   if (!inRadians)
      value = mAnomalyInRad * DEG_PER_RAD;

   #ifdef DEBUG_ANOMALY
   MessageInterface::ShowMessage
      (wxT("Anomaly::GetValue() returning %.18f\n"), value);
   #endif
   
   return value;
}


//------------------------------------------------------------------------------
// Real GetValue(AnomalyType type, bool inRadians = false) const
//------------------------------------------------------------------------------
/** 
 * Gets anomaly value from the given anomaly type.
 * 
 * @param <type> type of anomaly
 * @param <inRadians>  true if output value in radians is requested
 * @return Anomaly value.
 */
//------------------------------------------------------------------------------
Real Anomaly::GetValue(AnomalyType type, bool inRadians) const
{
   #ifdef DEBUG_ANOMALY
   MessageInterface::ShowMessage
      (wxT("Anomaly::GetValue() type=%d, inRadians=%d, mType=%d, mAnomalyInRad=%f\n"),
       type, inRadians, mType, mAnomalyInRad);
   #endif
   
   return Convert(type, inRadians);
}


//------------------------------------------------------------------------------
// Real GetValue(const wxString &type, bool valueInRadians = false) const
//------------------------------------------------------------------------------
/** 
 * Gets anomaly value from the given anomaly type.
 * 
 * @param <type> type of anomaly
 * @param <inRadians>  true if output value in radians is requested
 * @return Anomaly value.
 */
//------------------------------------------------------------------------------
Real Anomaly::GetValue(const wxString &type, bool valueInRadians) const
{
   return GetValue(GetType(type), valueInRadians);
}


//------------------------------------------------------------------------------
// void SetValue(Real value, bool valueInRadians = false)
//------------------------------------------------------------------------------
/** 
 * Sets anomaly value.
 * 
 * @param <value> Anomaly value.
 *
 */
//------------------------------------------------------------------------------
void Anomaly::SetValue(Real value, bool valueInRadians)
{
   if (valueInRadians)
      mAnomalyInRad = value;
   else
      mAnomalyInRad = value * RAD_PER_DEG;
}


//------------------------------------------------------------------------------
// AnomalyType GetType(const wxString &typeStr) const
//------------------------------------------------------------------------------
/** 
 * @return  AnomalyType of input type string.
 */
//------------------------------------------------------------------------------
Anomaly::AnomalyType Anomaly::GetType(const wxString &typeStr) const
{
   return GetAnomalyType(typeStr);
}


//------------------------------------------------------------------------------
// wxString GetTypeString() const
//------------------------------------------------------------------------------
/*
 * Returns internal type in string
 */
//------------------------------------------------------------------------------
wxString Anomaly::GetTypeString() const
{
   #ifdef DEBUG_ANOMALY
      MessageInterface::ShowMessage(wxT("Entering GetTypeString and mType = %d\n"),
            (Integer) mType);
   #endif
   return ANOMALY_SHORT_TEXT[mType];
}


//------------------------------------------------------------------------------
// void SetType(const wxString &type)
//------------------------------------------------------------------------------
/** 
 * Sets anomaly type.
 * 
 * @param <type> Anomaly type string.
 */
//------------------------------------------------------------------------------
void Anomaly::SetType(const wxString &type)
{
   SetType(GetType(type));
}


//------------------------------------------------------------------------------
// Real GetTrueAnomaly(bool inRadians = false) const
//------------------------------------------------------------------------------
/** 
 * Gets true anomaly.
 *
 * @param <inRadians>  true if output value in radians is requested
 * @return  value of true anomaly.
 */
//------------------------------------------------------------------------------
Real Anomaly::GetTrueAnomaly(bool inRadians) const
{
   #if DEBUG_ANOMALY
   MessageInterface::ShowMessage
      (wxT("Anomaly::GetTrueAnomaly() mEcc=%f, mAnomalyInRad=%f\n"), mEcc, mAnomalyInRad);
   #endif
   
   Real ta = mAnomalyInRad;
   
   if (mType == MA)
   {
      try
      {
         ta = Keplerian::MeanToTrueAnomaly(mAnomalyInRad * DEG_PER_RAD, mEcc) * RAD_PER_DEG;
      }
      catch(UtilityException &ue)
      {
         wxString msg = wxT("Anomaly::GetTrueAnomaly - ") + ue.GetFullMessage();
         throw UtilityException(msg); 
      }
   }
   else if (mType == EA || mType == HA)
   {
      if (mSma >= 0.0 && mEcc <= 1.0)
      {
         wxString typeStr = GetTypeString();
         
         throw UtilityException
            (wxT("Anomaly Type: \"") + typeStr + wxT("\", SMA: \"") +
             GmatStringUtil::ToString(mSma) + wxT("\",  and ECC: \"") +
             GmatStringUtil::ToString(mEcc) + wxT("\" are incompatible."));
      }
      
      ta = Keplerian::MeanToTrueAnomaly(mAnomalyInRad * DEG_PER_RAD, mEcc) * RAD_PER_DEG;
   }
   
   if (!inRadians)
      ta = ta * DEG_PER_RAD;
   
   #if DEBUG_ANOMALY
   MessageInterface::ShowMessage(wxT("Anomaly::GetTrueAnomaly() returning %f\n"), ta);
   #endif
   
   return ta;
}


//------------------------------------------------------------------------------
// Real GetMeanAnomaly(bool inRadians = false) const
//------------------------------------------------------------------------------
/** 
 * Gets mean anomaly.
 * 
 * @param <inRadians>  true if output value in radians is requested
 * @return  value of mean anomaly.
 *
 */
//------------------------------------------------------------------------------
Real Anomaly::GetMeanAnomaly(bool inRadians) const
{
   Real ma = mAnomalyInRad;
   
   if (mType == TA || mType == EA || mType == HA)
   {
      Real ta = GetTrueAnomaly(true);   
      ma = Keplerian::TrueToMeanAnomaly(ta, mEcc);
   }
   
   if (!inRadians)
      ma = ma * DEG_PER_RAD;
   
   #if DEBUG_ANOMALY
   MessageInterface::ShowMessage(wxT("Anomaly::GetMeanAnomaly() returning %f\n"), ma);
   #endif

   return ma;
}


//------------------------------------------------------------------------------
// Real GetEccentricAnomaly(bool inRadians = false) const
//------------------------------------------------------------------------------
/** 
 * Gets eccentric anomaly.
 * 
 * @param <inRadians>  true if output value in radians is requested
 * @return  value of eccentric anomaly.
 *
 */
//------------------------------------------------------------------------------
Real Anomaly::GetEccentricAnomaly(bool inRadians) const
{
   Real ea = mAnomalyInRad;
   
   if (mType == TA || mType == MA || mType == HA)
   {
      Real ta = GetTrueAnomaly(true);   
      ea = Keplerian::TrueToEccentricAnomaly(ta, mEcc);
   }
   
   if (!inRadians)
      ea = ea * DEG_PER_RAD;
   
   #if DEBUG_ANOMALY
   MessageInterface::ShowMessage(wxT("Anomaly::GetEccentricAnomaly() returning %f\n"), ea);
   #endif
   
   return ea;
}


//------------------------------------------------------------------------------
// Real GetHyperbolicAnomaly(bool inRadians = false) const
//------------------------------------------------------------------------------
/** 
 * Gets hyperbolic anomaly.
 * 
 * @param <inRadians>  true if output value in radians is requested
 * @return  value of hyperbolic anomaly.
 */
//------------------------------------------------------------------------------
Real Anomaly::GetHyperbolicAnomaly(bool inRadians) const
{
   Real ha = mAnomalyInRad;
   
   if (mType == MA || mType == EA || mType == TA)
   {
      Real ta = GetTrueAnomaly(true);
      ha = Keplerian::TrueToHyperbolicAnomaly(ta, mEcc);
   }
   
   if (!inRadians)
      ha = ha * DEG_PER_RAD;
   
   #if DEBUG_ANOMALY
   MessageInterface::ShowMessage(wxT("Anomaly::GetHyperbolicAnomaly() returning %f\n"), ha);
   #endif
   
   return ha;
}


//------------------------------------------------------------------------------
// bool IsInvalid(const wxString &typeStr) const
//------------------------------------------------------------------------------
/** 
 * Determines if the anomlay type is invalid.
 * 
 * @return   true if invalid, false it is valid.
 */
//------------------------------------------------------------------------------
bool Anomaly::IsInvalid(const wxString &typeStr) const
{
   for (int i=0; i<AnomalyTypeCount; i++)
   {
      if (typeStr == ANOMALY_LONG_TEXT[i])
         return false;
   }
   
   for (int i=0; i<AnomalyTypeCount; i++)
   {
      if (typeStr == ANOMALY_SHORT_TEXT[i])
         return false;
   }
   
   return true;
}


//------------------------------------------------------------------------------
// Real Convert(AnomalyType toType, bool inRadians = false) const
//------------------------------------------------------------------------------
/** 
 * Converts anomaly value.
 * 
 * @param <toType>   Anomaly type to convert to.
 * @param <inRadians>  true if output value in radians is requested
 *
 * @return Converted anomaly type 
 */
//------------------------------------------------------------------------------
Real Anomaly::Convert(AnomalyType toType, bool inRadians) const
{
   #ifdef DEBUG_ANOMALY
   MessageInterface::ShowMessage
      (wxT("Anomaly::Convert() toType=%d, inRadians=%d, mType=%d, mAnomalyInRad=%f\n"),
       toType, inRadians, mType, mAnomalyInRad);
   #endif
   
   Real value;
   
   if (toType == mType)
      value = mAnomalyInRad;
   else if (toType == TA)
      value = GetTrueAnomaly(true);
   else if (toType == MA) 
      value = GetMeanAnomaly(true);
   else if (toType == EA)
      value = GetEccentricAnomaly(true);
   else if (toType == HA)
      value = GetHyperbolicAnomaly(true);
   else
      throw UtilityException(wxT("Anomaly::Convert() - invalid input type"));
   
   if (!inRadians)
      value = value * DEG_PER_RAD;
   
   #ifdef DEBUG_ANOMALY
   MessageInterface::ShowMessage(wxT("Anomaly::Convert() returning %f\n"), value);
   #endif
   
   return value;
}


//------------------------------------------------------------------------------
// Real Convert(const wxString &toType, bool inRadians = false) const
//------------------------------------------------------------------------------
/** 
 * Converts anomaly value.
 * 
 * @param <toType>   Anomaly type to convert to.
 * @param <inRadians>  true if output value in radians is requested
 *
 * @return Converted anomaly type 
 */
//------------------------------------------------------------------------------
Real Anomaly::Convert(const wxString &toType,  bool inRadians) const
{
   return Convert(GetType(toType), inRadians);
}


//------------------------------------------------------------------------------
// Anomaly ConvertToAnomaly(AnomalyType toType, bool inRadians = false)
//------------------------------------------------------------------------------
/*
 * Converts internal anomaly using toType and returns new Anomaly.
 *
 * @param <toType>     AnomalyType to convert internal anomaly to
 * @param <inRadians>  true if output value in radians is requested
 */
//------------------------------------------------------------------------------
Anomaly Anomaly::ConvertToAnomaly(AnomalyType toType, bool inRadians)
{
   Anomaly temp = *this;
   Real value = temp.Convert(toType, inRadians);
   temp.SetValue(value);
   return temp;
}


//------------------------------------------------------------------------------
// Anomaly ConvertToAnomaly(const wxString &toType, bool inRadians = false)
//------------------------------------------------------------------------------
/*
 * Converts internal anomaly using toType and returns new Anomaly.
 *
 * @param <toType>     AnomalyType to convert internal anomaly to
 * @param <inRadians>  true if output value in radians is requested
 */
//------------------------------------------------------------------------------
Anomaly Anomaly::ConvertToAnomaly(const wxString &toType, bool inRadians)
{
   return ConvertToAnomaly(GetType(toType), inRadians);
}


//------------------------------------------------------------------------------
//  wxString ToString(Integer precision = GmatIO::DATA_PRECISION)
//------------------------------------------------------------------------------
/**
 * @return data value string
 */
//------------------------------------------------------------------------------
wxString Anomaly::ToString(Integer precision)
{
   wxString ss(wxT(""));
   
   ss << wxT("Anomaly Type: ") << GetTypeString();
   ss << wxT(", SMA: ") << mSma;
   ss << wxT(", ECC: ") << mEcc;
   ss << wxT(", Value: ") << GetValue();
      
   return ss;
}

//---------------------------------
// static functions
//---------------------------------

//------------------------------------------------------------------------------
// AnomalyType GetAnomalyType(const wxString &typeStr)
//------------------------------------------------------------------------------
/** 
 * @return  AnomalyType of input type string.
 */
//------------------------------------------------------------------------------
Anomaly::AnomalyType Anomaly::GetAnomalyType(const wxString &typeStr)
{
   #ifdef DEBUG_ANOMALY
      MessageInterface::ShowMessage(wxT("GetAnomalyType (%s) called.\n"), typeStr.c_str());
   #endif
   for (int i=0; i<AnomalyTypeCount; i++)
   {
      if (typeStr == ANOMALY_LONG_TEXT[i])
         return (AnomalyType)i;
   }
   
   for (int i=0; i<AnomalyTypeCount; i++)
   {
      if (typeStr == ANOMALY_SHORT_TEXT[i])
         return (AnomalyType)i;
   }
   
   throw UtilityException
      (wxT("Invalid Anomaly Type \"") + typeStr + wxT("\"\nAllowed ")
       wxT("are \"TA\", \"MA\", \"EA\", \"HA\" or \n\"True Anomaly\", ")
       wxT("\"Mean Anomaly\", \"Eccentric Anomaly\", \"Hyperbolic Anomaly\""));
}


//------------------------------------------------------------------------------
// static wxString GetTypeString(const wxString &type)
//------------------------------------------------------------------------------
/*
 * Returns matching short type string of input type string.
 */
//------------------------------------------------------------------------------
wxString Anomaly::GetTypeString(const wxString &type)
{
   if (type == wxT("True Anomaly") || type == wxT("TA"))
      return wxT("TA");
   else if (type == wxT("Mean Anomaly") || type == wxT("MA")) 
      return wxT("MA");
   else if (type == wxT("Eccentric Anomaly") || type == wxT("EA"))
      return wxT("EA");
   else if (type == wxT("Hyperbolic Anomaly") || type == wxT("HA"))
      return wxT("HA");
   else
      throw UtilityException
         (wxT("Invalid Anomaly Type \"") + type + wxT("\"\nAllowed ")
          wxT("are \"TA\", \"MA\", \"EA\", \"HA\" or \n\"True Anomaly\", ")
          wxT("\"Mean Anomaly\", \"Eccentric Anomaly\", \"Hyperbolic Anomaly\""));
   
}


//------------------------------------------------------------------------------
// static wxString GetLongTypeString(const wxString &type)
//------------------------------------------------------------------------------
/*
 * Returns matching long type string of input type string.
 */
//------------------------------------------------------------------------------
wxString Anomaly::GetLongTypeString(const wxString &type)
{
   if (type == wxT("True Anomaly") || type == wxT("TA"))
      return wxT("True Anomaly");
   else if (type == wxT("Mean Anomaly") || type == wxT("MA")) 
      return wxT("Mean Anomaly");
   else if (type == wxT("Eccentric Anomaly") || type == wxT("EA"))
      return wxT("Eccentric Anomaly");
   else if (type == wxT("Hyperbolic Anomaly") || type == wxT("HA"))
      return wxT("Hyperbolic Anomaly");
   else
      throw UtilityException
         (wxT("Invalid Anomaly Type \"") + type + wxT("\"\nAllowed ")
          wxT("are \"TA\", \"MA\", \"EA\", \"HA\" or \n\"True Anomaly\", ")
          wxT("\"Mean Anomaly\", \"Eccentric Anomaly\", \"Hyperbolic Anomaly\""));
   
}


//------------------------------------------------------------------------------
// static const wxString* GetLongTypeNameList()
//------------------------------------------------------------------------------
const wxString* Anomaly::GetLongTypeNameList()
{
   return ANOMALY_LONG_TEXT;
}

