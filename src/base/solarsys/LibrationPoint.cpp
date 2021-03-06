//$Id: LibrationPoint.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  LibrationPoint
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Wendy C. Shoan
// Created: 2005/04/04
//
/**
 * Implementation of the LibrationPoint class.
 *
 * @note This is an abstract class.
 */
//------------------------------------------------------------------------------

#include <vector>
#include "gmatdefs.hpp"
#include "CalculatedPoint.hpp"
#include "LibrationPoint.hpp"
#include "Barycenter.hpp"
#include "SolarSystemException.hpp"
#include "CelestialBody.hpp"
#include "RealUtilities.hpp"
#include "Rvector3.hpp"
#include "Rvector6.hpp"
#include "Rmatrix33.hpp"
#include "MessageInterface.hpp"

#include <iostream>


//#define DEBUG_CHECK_BODIES
//#define DEBUG_LP_OBJECT

//---------------------------------
// static data
//---------------------------------
const Real LibrationPoint::CONVERGENCE_TOLERANCE = 1.0e-8;
const Real LibrationPoint::MAX_ITERATIONS        = 2000;

const wxString
LibrationPoint::PARAMETER_TEXT[LibrationPointParamCount - CalculatedPointParamCount] =
{
   wxT("Primary"),
   wxT("Secondary"),
   wxT("Point"),
};

const Gmat::ParameterType
LibrationPoint::PARAMETER_TYPE[LibrationPointParamCount - CalculatedPointParamCount] =
{
   Gmat::OBJECT_TYPE,
   Gmat::OBJECT_TYPE,
   Gmat::ENUMERATION_TYPE,
};

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  LibrationPoint(const wxString &itsName)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the LibrationPoint class
 * (default constructor).
 *
 * @param <ptType>  string representation of its body type
 * @param <itsName> parameter indicating the name of the LibrationPoint.
 */
//------------------------------------------------------------------------------
LibrationPoint::LibrationPoint(const wxString &itsName) :
CalculatedPoint(wxT("LibrationPoint"), itsName),
primaryBodyName     (wxT("")),
secondaryBodyName   (wxT("")),
whichPoint          (wxT("")),
primaryBody         (NULL),
secondaryBody       (NULL)
{
   objectTypes.push_back(Gmat::LIBRATION_POINT);
   objectTypeNames.push_back(wxT("LibrationPoint"));
   parameterCount = LibrationPointParamCount;
}

//------------------------------------------------------------------------------
//  LibrationPoint(const LibrationPoint &lp)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the LibrationPoint class as a copy of the
 * specified LibrationPoint class (copy constructor).
 *
 * @param <lp> LibrationPoint object to copy.
 */
//------------------------------------------------------------------------------
LibrationPoint::LibrationPoint(const LibrationPoint &lp) :
CalculatedPoint          (lp),
primaryBodyName          (lp.primaryBodyName),
secondaryBodyName        (lp.secondaryBodyName),
whichPoint               (lp.whichPoint),
primaryBody              (lp.primaryBody),
secondaryBody            (lp.secondaryBody)
{
}

//------------------------------------------------------------------------------
//  LibrationPoint& operator= (const LibrationPoint& lp)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the LibrationPoint class.
 *
 * @param <lp> the LibrationPoint object whose data to assign to wxT("this")
 *             calculated point.
 *
 * @return wxT("this") LibrationPoint with data of input LibrationPoint lp.
 */
//------------------------------------------------------------------------------
LibrationPoint& LibrationPoint::operator=(const LibrationPoint &lp)
{
   if (&lp == this)
      return *this;

   CalculatedPoint::operator=(lp);
   primaryBodyName     = lp.primaryBodyName;
   secondaryBodyName   = lp.secondaryBodyName;
   whichPoint          = lp.whichPoint;
   primaryBody         = lp.primaryBody;
   secondaryBody       = lp.secondaryBody;
   return *this;
}

//------------------------------------------------------------------------------
//  ~LibrationPoint()
//------------------------------------------------------------------------------
/**
 * Destructor for the LibrationPoint class.
 */
//------------------------------------------------------------------------------
LibrationPoint::~LibrationPoint()
{
}


//---------------------------------------------------------------------------
//  const Rvector6 GetMJ2000State(const A1Mjd &atTime)
//---------------------------------------------------------------------------
/**
 * Method returning the MJ2000 state of the Barycenter at the time atTime.
 *
 * @param <atTime> Time for which the state is requested.
 *
 * @return state of the Barycenter at time atTime.
 */
//---------------------------------------------------------------------------
const Rvector6 LibrationPoint::GetMJ2000State(const A1Mjd &atTime)
{
   #ifdef DEBUG_GET_STATE
   MessageInterface::ShowMessage
      (wxT("LibrationPoint::GetMJ2000State() '%s' entered, atTime=%f, ")
       wxT("primaryBody=<%p> '%s', secondaryBody=<%p> '%s'\n"), GetName().c_str(),
       atTime.GetReal(), primaryBody, primaryBody->GetName().c_str(),
       secondaryBody, secondaryBody->GetName().c_str());
   #endif
   
   CheckBodies();
   // Compute position and velocity from primary to secondary
   Rvector6 primaryState = primaryBody->GetMJ2000State(atTime);
   Rvector6 secondaryState = secondaryBody->GetMJ2000State(atTime);
   
   #ifdef DEBUG_GET_STATE
   MessageInterface::ShowMessage
      (wxT("   primaryState =\n   %s\n"), primaryState.ToString().c_str());
   MessageInterface::ShowMessage
      (wxT("   secondaryState =\n   %s\n"), secondaryState.ToString().c_str());
   #endif
   
   Rvector6 pToS = (secondaryBody->GetMJ2000State(atTime)) - primaryState;
   Rvector3 r    = pToS.GetR();
   Rvector3 v    = pToS.GetV();
   Rvector3 a    = (secondaryBody->GetMJ2000Acceleration(atTime)) -
                   (primaryBody->GetMJ2000Acceleration(atTime));
   
   Real     massPrimary, massSecondary;
   if ((primaryBody->GetType()) == Gmat::CELESTIAL_BODY)
      massPrimary = ((CelestialBody*) primaryBody)->GetMass();
   else  // Barycenter
      massPrimary = ((Barycenter*) primaryBody)->GetMass();
   if ((secondaryBody->GetType()) == Gmat::CELESTIAL_BODY)
      massSecondary = ((CelestialBody*) secondaryBody)->GetMass();
   else  // Barycenter
      massSecondary = ((Barycenter*) secondaryBody)->GetMass();
   if ((massPrimary == 0.0) && (massSecondary == 0.0))
      throw SolarSystemException(
            wxT("Primary and secondary bodies for LibrationPoint are massless"));
   Real muStar = massSecondary / (massPrimary + massSecondary);
   #ifdef DEBUG_GET_STATE
   MessageInterface::ShowMessage
      (wxT("   Mass of the primary is %f\n"), massPrimary);
   MessageInterface::ShowMessage
      (wxT("   Mass of the secondary is %f\n"), massSecondary);
   #endif
   
   Real gamma = 0.0;
   Real gamma2 = 0.0, gamma3 = 0.0, gamma4 = 0.0, gamma5 = 0.0, gammaPrev = 0.0;
   Real F = 0.0, Fdot = 0.0;
   if ((whichPoint == wxT("L1")) || (whichPoint == wxT("L2")) ||
       (whichPoint == wxT("L3")))
   {
      // Determine initial gamma
      if (whichPoint == wxT("L3"))  gamma = 1.0;
      else  gamma = GmatMathUtil::Pow((muStar / (3.0 * (1.0 - muStar))),
                                      (1.0 / 3.0));
      

      Integer counter = 0;
      Real diff = 999.99;
      while (diff > CONVERGENCE_TOLERANCE)
      {
         if (counter > MAX_ITERATIONS)
            throw SolarSystemException(
                  wxT("Libration point gamma not converging."));
         gamma2 = gamma  * gamma;
         gamma3 = gamma2 * gamma;
         gamma4 = gamma3 * gamma;
         gamma5 = gamma4 * gamma;
         if (whichPoint == wxT("L1"))
         {
            F = gamma5 - ((3.0 - muStar) * gamma4) + 
                ((3.0 - 2.0 * muStar) * gamma3) - 
                (muStar * gamma2) + (2.0 * muStar * gamma) - muStar;
            Fdot = (5.0 * gamma4) - (4.0 * (3.0 - muStar) * gamma3) + 
                   (3.0 * (3.0 - 2.0 * muStar) * gamma2) - 
                   (2.0 * muStar * gamma) + (2.0 * muStar);
         }
         else if (whichPoint == wxT("L2"))
         {
            F = gamma5 + ((3.0 - muStar) * gamma4) + 
                ((3.0 - 2.0 * muStar) * gamma3) - 
                (muStar * gamma2) - (2.0 * muStar * gamma) - muStar;
            Fdot = (5.0 * gamma4) + (4.0 * (3.0 - muStar) * gamma3) + 
               (3.0 * (3.0 - 2.0 * muStar) * gamma2) - (2.0 * muStar * gamma) - 
               (2.0 * muStar);
         }
         else  // whichPoint == wxT("L3")
         {
            F = gamma5 + ((2.0 + muStar) * gamma4) + 
                ((1.0 + 2.0 * muStar) * gamma3) -
                ((1.0 - muStar) * gamma2) - (2.0 * (1.0 - muStar) * gamma) - 
                (1.0 - muStar);
            Fdot = (5.0 * gamma4) + (4.0 * (2.0 +  muStar) * gamma3) + 
               (3.0 * (1.0 +  2.0 * muStar) * gamma2) - 
               (2.0 * (1.0 - muStar) * gamma) - (2.0 * (1.0 - muStar));
         }
         counter++;
         gammaPrev = gamma;
         gamma     = gammaPrev - (F / Fdot);
         diff      = GmatMathUtil::Abs(gamma - gammaPrev);
      }
   }
   Real x = 0.0;
   Real y = 0.0;
   if (whichPoint == wxT("L1")) 
   {
      x = 1.0 - gamma;
      y = 0.0;
   }
   else if (whichPoint == wxT("L2"))
   {
      x = 1.0 + gamma;
      y = 0.0;
   }
   else if (whichPoint == wxT("L3"))
   {
      x = - gamma;
      y = 0.0;
   }
   else if (whichPoint == wxT("L4"))
   {
      x = 0.5;
      y = GmatMathUtil::Sqrt(3.0) / 2.0;
   }
   else if (whichPoint == wxT("L5"))
   {
      x = 0.5;
      y = - GmatMathUtil::Sqrt(3.0) / 2.0;
   }
   else // ERROR
      throw SolarSystemException
         (wxT("\"") + whichPoint + wxT("\" is illegal value for libration point."));
   
   // Express position and velocity of the libration point in the rotating
   // system with the origin centered on the primary body
   Rvector3 ri(x, y, 0.0);
   Rvector3 vi(x, y, 0.0);
   Real rMag  = r.GetMagnitude();
   ri         = rMag * ri;
   Real vMult = (v * r) / rMag;
   vi         = vMult * vi;
   // Determine the rotation matrix and its derivative
   Rvector3 xHat    = r / r.GetMagnitude();  // unit vector
   Rvector3 zHat    = (Cross(r,v)).GetUnitVector();
   Rvector3 yHat    = Cross(zHat, xHat);
   Rvector3 xDotHat = (v / rMag) - (xHat / rMag) * (xHat * v);
   Rvector3 ra      = Cross(r,a);
   Rvector3 rv      = Cross(r,v);
   Real     rvMag   = rv.GetMagnitude();
   Rvector3 zDotHat = ra / rvMag - (zHat / rvMag) * ((ra * zHat));
   Rvector3 yDotHat = Cross(zDotHat, xHat) + Cross(zHat, xDotHat);
   Rmatrix33 R;
   R(0,0) = xHat(0);
   R(0,1) = yHat(0);
   R(0,2) = zHat(0);
   R(1,0) = xHat(1);
   R(1,1) = yHat(1);
   R(1,2) = zHat(1);
   R(2,0) = xHat(2);
   R(2,1) = yHat(2);
   R(2,2) = zHat(2);
   
   Rmatrix33 RDot;
   RDot(0,0) = xDotHat(0);
   RDot(0,1) = yDotHat(0);
   RDot(0,2) = zDotHat(0);
   RDot(1,0) = xDotHat(1);
   RDot(1,1) = yDotHat(1);
   RDot(1,2) = zDotHat(1);
   RDot(2,0) = xDotHat(2);
   RDot(2,1) = yDotHat(2);
   RDot(2,2) = zDotHat(2);
   
   Rvector3 rLi = R * ri;
   Rvector3 vLi = RDot * ri + R * vi;
   
   Rvector6 rvFK5(rLi(0), rLi(1), rLi(2), vLi(0), vLi(1), vLi(2));
   
   // Translate so that the origin is at the j2000Body
   Rvector6 rvResult = rvFK5 + primaryState;
   
   #ifdef DEBUG_GET_STATE
   MessageInterface::ShowMessage
      (wxT("LibrationPoint::GetMJ2000State() returning\n   %s\n"),
       rvResult.ToString().c_str());
   #endif
   return rvResult;
}

//---------------------------------------------------------------------------
//  const Rvector3 GetMJ2000Position(const A1Mjd &atTime)
//---------------------------------------------------------------------------
/**
 * Method returning the MJ2000 position of the Barycenter at the time atTime.
 *
 * @param <atTime> Time for which the position is requested.
 *
 * @return position of the Barycenter at time atTime.
 */
//---------------------------------------------------------------------------
const Rvector3 LibrationPoint::GetMJ2000Position(const A1Mjd &atTime)
{
   Rvector6 tmp = GetMJ2000State(atTime);
   return (tmp.GetR());
}

//---------------------------------------------------------------------------
//  const Rvector3 GetMJ2000Velocity(const A1Mjd &atTime)
//---------------------------------------------------------------------------
/**
 * Method returning the MJ2000 velocity of the Barycenter at the time atTime.
 *
 * @param <atTime> Time for which the velocity is requested.
 *
 * @return velocity of the Barycenter at time atTime.
 */
//---------------------------------------------------------------------------
const Rvector3 LibrationPoint::GetMJ2000Velocity(const A1Mjd &atTime)
{
   Rvector6 tmp = GetMJ2000State(atTime);
   return (tmp.GetV());
}


//------------------------------------------------------------------------------
//  wxString  GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter text, given the input parameter ID.
 *
 * @param <id> Id for the requested parameter text.
 *
 * @return parameter text for the requested parameter.
 *
 */
//------------------------------------------------------------------------------
wxString LibrationPoint::GetParameterText(const Integer id) const
{
   if (id >= CalculatedPointParamCount && id < LibrationPointParamCount)
      return PARAMETER_TEXT[id - CalculatedPointParamCount];
   return CalculatedPoint::GetParameterText(id);
}

//------------------------------------------------------------------------------
//  Integer  GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter ID, given the input parameter string.
 *
 * @param <str> string for the requested parameter.
 *
 * @return ID for the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Integer     LibrationPoint::GetParameterID(const wxString &str) const
{
   for (Integer i = CalculatedPointParamCount; i < LibrationPointParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - CalculatedPointParamCount])
         return i;
   }
   
   return CalculatedPoint::GetParameterID(str);
}

//------------------------------------------------------------------------------
//  Gmat::ParameterType  GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type, given the input parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return parameter type of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Gmat::ParameterType LibrationPoint::GetParameterType(const Integer id) const
{
   if (id >= CalculatedPointParamCount && id < LibrationPointParamCount)
      return PARAMETER_TYPE[id - CalculatedPointParamCount];
      
   return CalculatedPoint::GetParameterType(id);
}

//------------------------------------------------------------------------------
//  wxString  GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type string, given the input parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return parameter type string of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
wxString LibrationPoint::GetParameterTypeString(const Integer id) const
{
   return CalculatedPoint::PARAM_TYPE_STRING[GetParameterType(id)];
}

//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter ID.
 *
 * @param <id>    ID for the requested parameter.
 *
 * @return  string value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
wxString LibrationPoint::GetStringParameter(const Integer id) const
{
   if (id == PRIMARY_BODY_NAME)             
   {
      if (primaryBody)  return primaryBody->GetName();
      return primaryBodyName;
   }
   if (id == SECONDARY_BODY_NAME)             
   {
      if (secondaryBody)  return secondaryBody->GetName();
      return secondaryBodyName;
   }
   if (id == WHICH_POINT)             
   {
      return whichPoint;
   }
   
   return CalculatedPoint::GetStringParameter(id);
}


//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter label.
 *
 * @param <label> label for the requested parameter.
 *
 * @return  string value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
wxString LibrationPoint::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}

//------------------------------------------------------------------------------
//  wxString  SetStringParameter(const Integer id, const wxString value)
//------------------------------------------------------------------------------
/**
 * This method sets the string parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 * @param <value> string value for the requested parameter.
 *
 * @return  success flag.
 *
 */
//------------------------------------------------------------------------------
bool LibrationPoint::SetStringParameter(const Integer id, 
                                        const wxString &value)
{     
   if (id == PRIMARY_BODY_NAME)             
   {
      // since we don't know the order of setting, we cannot do the checking
      // of primary and secondary bodies are the same
      primaryBodyName = value;
      return true;
   }
   if (id == SECONDARY_BODY_NAME)             
   {
      // since we don't know the order of setting, we cannot do the checking
      // of primary and secondary bodies are the same
      secondaryBodyName = value;
      return true;
   }
   if (id == WHICH_POINT)             
   {
      if ((value != wxT("L1")) && (value != wxT("L2")) && (value != wxT("L3")) &&
          (value != wxT("L4")) && (value != wxT("L5")))
         throw SolarSystemException(
            wxT("The value of \"") + value + wxT("\" for field \"Libration\"")
            wxT(" on object \"") + instanceName + wxT("\" is not an allowed value.\n")
            wxT("The allowed values are: [ L1, L2, L3, L4, L5 ]. "));
      whichPoint = value;
      return true;
   }
   
   return CalculatedPoint::SetStringParameter(id, value);
}

//------------------------------------------------------------------------------
//  wxString  SetStringParameter(const wxString &label, 
//                                  const wxString value)
//------------------------------------------------------------------------------
/**
 * This method sets the string parameter value, given the input
 * parameter label.
 *
 * @param <label> label for the requested parameter.
 * @param <value> string value for the requested parameter.
 *
 * @return  success flag.
 *
 */
//------------------------------------------------------------------------------
bool LibrationPoint::SetStringParameter(const wxString &label, 
                                        const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}

//------------------------------------------------------------------------------
//  bool  SetStringParameter(const Integer id, const wxString value.
//                           const Integer index)
//------------------------------------------------------------------------------
/**
 * This method sets the string parameter value, given the input
 * parameter ID, and the index.
 *
 * @param <id>    ID for the requested parameter.
 * @param <value> string value for the requested parameter.
 * @param <index> index into the array of strings.
 *
 * @return  success flag.
 *
 * @note ADDED TO SATISFY DUFUS COMPILER
 *
 */
//------------------------------------------------------------------------------
bool  LibrationPoint::SetStringParameter(const Integer id,
                                          const wxString &value,
                                          const Integer index) 
{
   return CalculatedPoint::SetStringParameter(id, value, index);
}

//------------------------------------------------------------------------------
//  bool  SetStringParameter(const wxString &label, const wxString value.
//                           const Integer index)
//------------------------------------------------------------------------------
/**
 * This method sets the string parameter value, given the input
 * parameter label, and the index.
 *
 * @param <label> label for the requested parameter.
 * @param <value> string value for the requested parameter.
 * @param <index> index into the array of strings.
 *
 * @return  success flag.
 *
 * @note ADDED TO SATISFY DUFUS COMPILER
 *
 */
//------------------------------------------------------------------------------
bool  LibrationPoint::SetStringParameter(const wxString &label,
                                          const wxString &value,
                                          const Integer index) 
{
   return SetStringParameter(GetParameterID(label),value,index);
}


//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by this class.
 *
 * @return the list of object types.
 * 
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& LibrationPoint::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::SPACE_POINT);
   return refObjectTypes;
}

//------------------------------------------------------------------------------
//  const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Returns the names of the reference object. 
 *
 * @param <type> reference object type.  Gmat::UnknownObject returns all of the
 *               ref objects.
 *
 * @return The names of the reference object.
 */
//------------------------------------------------------------------------------
const StringArray& LibrationPoint::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::SPACE_POINT)
   {
      static StringArray refs;
      refs.clear();
      refs.push_back(primaryBodyName);
      refs.push_back(secondaryBodyName);
      return refs;
   }
   
   // Not handled here -- invoke the next higher GetRefObjectNameArray call
   return CalculatedPoint::GetRefObjectNameArray(type);
}


//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name)
//------------------------------------------------------------------------------
/**
 * Sets the reference object.
 *
 * @param <obj>   reference object pointer.
 * @param <type>  type of the reference object.
 * @param <name>  name of the reference object.
 *
 * @return success of the operation.
 */
//------------------------------------------------------------------------------
bool LibrationPoint::SetRefObject(GmatBase *obj, 
                                  const Gmat::ObjectType type,
                                  const wxString &name)
{
   if (obj == NULL)
      return false;
   
   #ifdef DEBUG_LP_OBJECT
   MessageInterface::ShowMessage
      (wxT("LibrationPoint::SetRefObject() this=<%p> '%s', obj=<%p><%s> entered\n"),
       this, GetName().c_str(), obj, obj->GetName().c_str());
   MessageInterface::ShowMessage
      (wxT("   primaryBodyName='%s', primaryBody=<%p>\n   secondaryBodyName='%s', ")
       wxT("secondaryBody=<%p>\n"), primaryBodyName.c_str(), primaryBody,
       secondaryBodyName.c_str(), secondaryBody);
   #endif
   
   if (obj->IsOfType(Gmat::SPACE_POINT))
   {
      if (name == primaryBodyName)
         primaryBody = (SpacePoint*)obj;
      else if (name == secondaryBodyName)
         secondaryBody = (SpacePoint*)obj;
   }
   #ifdef DEBUG_LP_OBJECT
   MessageInterface::ShowMessage
      (wxT("   end of SetRefObject() this=<%p> '%s', obj=<%p><%s> entered\n"),
       this, GetName().c_str(), obj, obj->GetName().c_str());
   MessageInterface::ShowMessage
      (wxT("   primaryBodyName='%s', primaryBody=<%p>\n   secondaryBodyName='%s', ")
       wxT("secondaryBody=<%p>\n"), primaryBodyName.c_str(), primaryBody,
       secondaryBodyName.c_str(), secondaryBody);
   #endif
   
   // Call parent class to add objects to bodyList
   return CalculatedPoint::SetRefObject(obj, type, name);
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the LibrationPoint.
 *
 * @return clone of the LibrationPoint.
 *
 */
//------------------------------------------------------------------------------
GmatBase* LibrationPoint::Clone() const
{
   return (new LibrationPoint(*this));
}


//---------------------------------------------------------------------------
// void Copy(const GmatBase* orig)
//---------------------------------------------------------------------------
/**
 * Sets this object to match another one.
 * 
 * @param orig The original that is being copied.
 */
//---------------------------------------------------------------------------
void LibrationPoint::Copy(const GmatBase* orig)
{
   // We don't want to copy instanceName
   wxString name = instanceName;
   operator=(*((LibrationPoint *)(orig)));
   instanceName = name;
}


//------------------------------------------------------------------------------
// private methods
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//  void CheckBodies()
//------------------------------------------------------------------------------
/**
 * Method for the Barycenter class that checks to make sure the bodyList has
 * been defined appropriately (i.e. all CelestialBody objects).
 *
 */
//------------------------------------------------------------------------------
void LibrationPoint::CheckBodies()
{
   bool foundPrimary   = false;
   bool foundSecondary = false;
   
   #ifdef DEBUG_CHECK_BODIES
   MessageInterface::ShowMessage
      (wxT("LibrationPoint::CheckBodies() this=<%p> '%s' has %d body names, and %d body")
       wxT(" pointers\n"), this, GetName().c_str(), bodyNames.size(), bodyList.size());
   #endif
   for (unsigned int i = 0; i < bodyList.size() ; i++)
   {
      #ifdef DEBUG_CHECK_BODIES
      MessageInterface::ShowMessage
         (wxT("   bodyList[%d] = %s\n"), i,
          (bodyList[i] == NULL ? wxT("NULL") : bodyList[i]->GetName().c_str()));
      #endif
      
      if (((bodyList.at(i))->GetType() != Gmat::CELESTIAL_BODY) &&
          ((bodyList.at(i))->GetTypeName() != wxT("Barycenter")))
         throw SolarSystemException(
             wxT("Bodies for LibrationPoint must be CelestialBodys or Barycenters"));
      if ((bodyList.at(i))->GetName() == primaryBodyName) 
      {
         foundPrimary = true;
         primaryBody  = bodyList.at(i);
      }
      if ((bodyList.at(i))->GetName() == secondaryBodyName) 
      {
         foundSecondary = true;
         secondaryBody  = bodyList.at(i);
      }
   }
   if (!foundPrimary)
      throw SolarSystemException(
            wxT("Primary body \"") + primaryBodyName + wxT("\" not found for LibrationPoint ") +
            wxT("\"") + GetName() + wxT("\""));
   if (!foundSecondary)
      throw SolarSystemException(
            wxT("Secondary body \"") + secondaryBodyName + wxT("\" not found for LibrationPoint ") +
            wxT("\"") + GetName() + wxT("\""));
   if (primaryBody == secondaryBody)
      throw SolarSystemException(
            wxT("Primary body \"") + primaryBodyName + wxT("\" and Secondary body \"") +
            secondaryBodyName + wxT("\" cannot be the same for LibrationPoint ") +
            wxT("\"") + GetName() + wxT("\""));
}


