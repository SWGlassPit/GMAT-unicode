//$Id: CalculatedPoint.cpp 9724 2011-07-20 19:12:31Z wendys-dev $
//------------------------------------------------------------------------------
//                                  CalculatedPoint
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
 * Implementation of the CalculatedPoint class.
 *
 * @note This is an abstract class.
 */
//------------------------------------------------------------------------------

#include <vector>
#include <algorithm>              // for find()
#include "gmatdefs.hpp"
#include "SpacePoint.hpp"
#include "CalculatedPoint.hpp"
#include "SolarSystem.hpp"
#include "SolarSystemException.hpp"
#include "MessageInterface.hpp"
#include "StringUtil.hpp"

//#define DEBUG_CP_OBJECT
//#define DEBUG_CP_BODIES
//#define DEBUG_CP_SET_STRING
//#define DEBUG_CP_ACTION

//---------------------------------
// static data
//---------------------------------
const wxString
CalculatedPoint::PARAMETER_TEXT[CalculatedPointParamCount - SpacePointParamCount] =
{
   wxT("NumberOfBodies"),
   wxT("BodyNames"),
};

const Gmat::ParameterType
CalculatedPoint::PARAMETER_TYPE[CalculatedPointParamCount - SpacePointParamCount] =
{
   Gmat::INTEGER_TYPE,
   Gmat::OBJECTARRAY_TYPE,
};

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  CalculatedPoint(const wxString &ptType, 
//                  const wxString &itsName)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the CalculatedPoint class
 * (default constructor).
 *
 * @param <ptType>  string representation of its body type
 * @param <itsName> parameter indicating the name of the CalculatedPoint.
 */
//------------------------------------------------------------------------------
CalculatedPoint::CalculatedPoint(const wxString &ptType, 
                                 const wxString &itsName) :
SpacePoint(Gmat::CALCULATED_POINT, ptType, itsName),
numberOfBodies  (0)
{
   objectTypes.push_back(Gmat::CALCULATED_POINT);
   objectTypeNames.push_back(wxT("CalculatedPoint"));
   parameterCount = CalculatedPointParamCount;
}

//------------------------------------------------------------------------------
//  CalculatedPoint(const CalculatedPoint &cp)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the CalculatedPoint class as a copy of the
 * specified CalculatedPoint class (copy constructor).
 *
 * @param <cp> CalculatedPoint object to copy.
 */
//------------------------------------------------------------------------------
CalculatedPoint::CalculatedPoint(const CalculatedPoint &cp) :
SpacePoint          (cp)
{
   bodyNames.clear();
   bodyList.clear();
   defaultBodies.clear();
   // copy the list of body pointers
   for (unsigned int i = 0; i < (cp.bodyList).size(); i++)
   {
      bodyList.push_back((cp.bodyList).at(i));
   }
   // copy the list of body names
   for (unsigned int i = 0; i < (cp.bodyNames).size(); i++)
   {
      bodyNames.push_back((cp.bodyNames).at(i));
   }
   numberOfBodies = (Integer) bodyList.size();

   // copy the list of default body names
   for (unsigned int i = 0; i < (cp.defaultBodies).size(); i++)
   {
      defaultBodies.push_back((cp.defaultBodies).at(i));
   }
}

//------------------------------------------------------------------------------
//  CalculatedPoint& operator= (const CalculatedPoint& cp)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the CalculatedPoint class.
 *
 * @param <cp> the CalculatedPoint object whose data to assign to wxT("this")
 *             calculated point.
 *
 * @return wxT("this") CalculatedPoint with data of input CalculatedPoint cp.
 */
//------------------------------------------------------------------------------
CalculatedPoint& CalculatedPoint::operator=(const CalculatedPoint &cp)
{
   if (&cp == this)
      return *this;

   SpacePoint::operator=(cp);
   bodyNames.clear();
   bodyList.clear();
   defaultBodies.clear();
   // copy the list of body pointers
   for (unsigned int i = 0; i < (cp.bodyList).size(); i++)
   {
      bodyList.push_back((cp.bodyList).at(i));
   }
   // copy the list of body names
   for (unsigned int i = 0; i < (cp.bodyNames).size(); i++)
   {
      bodyNames.push_back((cp.bodyNames).at(i));
   }
   numberOfBodies = (Integer) bodyList.size();

   // copy the list of default body names
   for (unsigned int i = 0; i < (cp.defaultBodies).size(); i++)
   {
      defaultBodies.push_back((cp.defaultBodies).at(i));
   }

   return *this;
}

//------------------------------------------------------------------------------
//  ~CalculatedPoint()
//------------------------------------------------------------------------------
/**
 * Destructor for the CalculatedPoint class.
 */
//------------------------------------------------------------------------------
CalculatedPoint::~CalculatedPoint()
{
   bodyNames.clear();
   bodyList.clear();
   defaultBodies.clear();
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
wxString CalculatedPoint::GetParameterText(const Integer id) const
{
   if (id >= SpacePointParamCount && id < CalculatedPointParamCount)
      return PARAMETER_TEXT[id - SpacePointParamCount];
   return SpacePoint::GetParameterText(id);
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
Integer     CalculatedPoint::GetParameterID(const wxString &str) const
{
   for (Integer i = SpacePointParamCount; i < CalculatedPointParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - SpacePointParamCount])
         return i;
   }
   
   // Special handler for wxT("Add") - per Steve 2005.05.18
   if (str == wxT("Add")) return BODY_NAMES;
   
   return SpacePoint::GetParameterID(str);
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
Gmat::ParameterType CalculatedPoint::GetParameterType(const Integer id) const
{
   if (id >= SpacePointParamCount && id < CalculatedPointParamCount)
      return PARAMETER_TYPE[id - SpacePointParamCount];
      
   return SpacePoint::GetParameterType(id);
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
wxString CalculatedPoint::GetParameterTypeString(const Integer id) const
{
   return SpacePoint::PARAM_TYPE_STRING[GetParameterType(id)];
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <id> Description for the parameter.
 *
 * @return true if the parameter is read only, false (the default) if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool CalculatedPoint::IsParameterReadOnly(const Integer id) const
{
   if (id == NUMBER_OF_BODIES)
      return true;

   return SpacePoint::IsParameterReadOnly(id);
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <label> Description for the parameter.
 *
 * @return true if the parameter is read only, false (the default) if not.
 */
//---------------------------------------------------------------------------
bool CalculatedPoint::IsParameterReadOnly(const wxString &label) const
{
   return IsParameterReadOnly(GetParameterID(label));
}


//------------------------------------------------------------------------------
//  Integer  GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the Integer parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return  Integer value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Integer     CalculatedPoint::GetIntegerParameter(const Integer id) const
{
   if (id == NUMBER_OF_BODIES)      return bodyList.size();
   return SpacePoint::GetIntegerParameter(id); 
}

//------------------------------------------------------------------------------
//  Integer  GetIntegerParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * This method returns the Integer parameter value, given the input
 * parameter label.
 *
 * @param <label> label for the requested parameter.
 *
 * @return  Integer value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Integer     CalculatedPoint::GetIntegerParameter(const wxString &label) const
{
   return GetIntegerParameter(GetParameterID(label)); 
}

//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter ID and index.
 *
 * @param <id>    ID for the requested parameter.
 * @param <index> index into the array of strings, for the requested parameter.
 *
 * @return  string value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
wxString CalculatedPoint::GetStringParameter(const Integer id,
                                                const Integer index) const
{
   if (id == BODY_NAMES)             
   {
      try
      {
         return bodyNames.at(index);
      }
      catch (const std::exception &)
      {
         throw SolarSystemException(wxT("CalculatedPoint error: index out-of-range."));
      }
   }

   return SpacePoint::GetStringParameter(id, index);
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
 * @note - This should not be necessary here, but GCC is getting confused 
 *         cbout this method.
 *
 */
//------------------------------------------------------------------------------
wxString CalculatedPoint::GetStringParameter(const Integer id) const
{
   return SpacePoint::GetStringParameter(id);
}

//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const wxString &label, 
//                                  const Integer index) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter label and index.
 *
 * @param <label> label for the requested parameter.
 * @param <index> index into the array of strings, for the requested parameter.
 *
 * @return  string value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
wxString CalculatedPoint::GetStringParameter(const wxString &label,
                                                const Integer index) const
{
   return GetStringParameter(GetParameterID(label), index);
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
 * @note - This should not be necessary here, but GCC is getting confused 
 *         cbout this method.
 *
 */
//------------------------------------------------------------------------------
bool CalculatedPoint::SetStringParameter(const Integer id, 
                                         const wxString &value)
{
   #ifdef DEBUG_CP_SET_STRING
      MessageInterface::ShowMessage(wxT("Entering CalculatedPoint::SetString with id = %d (%s), value = %s\n"),
            id, GetParameterText(id).c_str(), value.c_str());
   #endif
   if (id == BODY_NAMES)
   {
      wxString value1 = GmatStringUtil::Trim(value);
      if (GmatStringUtil::IsEnclosedWithBraces(value1))
      {
//         bodyNames.clear();
         TakeAction(wxT("ClearBodies"));
         bodyNames = GmatStringUtil::ToStringArray(value1);
      }
      else
      {
         if (find(bodyNames.begin(), bodyNames.end(), value) == bodyNames.end())
         {
            #ifdef DEBUG_CP_OBJECT
               MessageInterface::ShowMessage(wxT("Adding %s to body name list for object %s\n"),
                     value.c_str(), instanceName.c_str());
            #endif
            bodyNames.push_back(value);
         }
      }
      
      #ifdef DEBUG_CP_SET_STRING
         MessageInterface::ShowMessage(wxT("Exiting CalculatedPoint::SetString: BodyNames are: \n"));
         for (unsigned int ii = 0; ii < bodyNames.size(); ii++)
            MessageInterface::ShowMessage(wxT("   %d     %s\n"), (Integer) ii, (bodyNames.at(ii)).c_str());
      #endif
      return true;
   }

   return SpacePoint::SetStringParameter(id, value);
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
 * @note - This should not be necessary here, but GCC is getting confused 
 *         cbout this method.
 *
 */
//------------------------------------------------------------------------------
bool CalculatedPoint::SetStringParameter(const wxString &label, 
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
 */
//------------------------------------------------------------------------------
bool  CalculatedPoint::SetStringParameter(const Integer id,
                                          const wxString &value,
                                          const Integer index) 
{
   #ifdef DEBUG_CP_SET_STRING
      MessageInterface::ShowMessage(
            wxT("Entering CalculatedPoint::SetString with id = %d (%s), index = %d, and value = %s\n"),
            id, GetParameterText(id).c_str(), index, value.c_str());
   #endif
   if (id == BODY_NAMES)
   {
      if ((index < 0) || (index > (Integer) bodyNames.size()))
         return false;  // throw an exception here?
      if (index == (Integer) bodyNames.size())
      {
         if (find(bodyNames.begin(), bodyNames.end(), value) == bodyNames.end())
            bodyNames.push_back(value);
         return true;
      }
      else  // replace current name
      {
         bodyNames.at(index) = value; 
         return true;
      }
   }
   
   return SpacePoint::SetStringParameter(id, value, index);
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
 */
//------------------------------------------------------------------------------
bool  CalculatedPoint::SetStringParameter(const wxString &label,
                                          const wxString &value,
                                          const Integer index) 
{
   return SetStringParameter(GetParameterID(label),value,index);
}

//------------------------------------------------------------------------------
//  const StringArray&   GetStringArrayParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
* This method returns the StringArray parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return  StringArray value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
const StringArray& CalculatedPoint::GetStringArrayParameter(const Integer id) const
{
   if (id == BODY_NAMES)
   {
      if (!bodyNames.empty()) return bodyNames;
      else                    return defaultBodies;
   }
   
   return SpacePoint::GetStringArrayParameter(id);
}

//------------------------------------------------------------------------------
//  const StringArray&   GetStringArrayParameter((const wxString &label) 
//                       const
//------------------------------------------------------------------------------
/**
 * This method returns the StringArray parameter value, given the input
 * parameter label.
 *
 * @param <label> label for the requested parameter.
 *
 * @return  StringArray value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
const StringArray& CalculatedPoint::GetStringArrayParameter(
                                    const wxString &label) const
{   
   return GetStringArrayParameter(GetParameterID(label));
}

//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name,
//                        const Integer index)
//------------------------------------------------------------------------------
/**
 * Returns the reference object pointer.
 *
 * @param <type>  type of the reference object.
 * @param <name>  name of the reference object.
 * @param <inde>x Index into the object array.
 * 
 * @return reference object pointer.
 */
//------------------------------------------------------------------------------
GmatBase* CalculatedPoint::GetRefObject(const Gmat::ObjectType type,
                                        const wxString &name,
                                        const Integer index)
{
   if (type == Gmat::SPACE_POINT)
   {
      try
      {
         return bodyList.at(index);
      }
      catch (const std::exception &)
      {
         throw SolarSystemException(
               wxT("CalculatedPoint error: index out-of-range."));
      }
   }
   return SpacePoint::GetRefObject(type, name, index);
}

//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name, const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets the reference object.
 *
 * @param <obj>   reference object pointer.
 * @param <type>  type of the reference object.
 * @param <name>  name of the reference object.
 * @param <index> Index into the object array.
 *
 * @return success of the operation.
 */
//------------------------------------------------------------------------------
bool CalculatedPoint::SetRefObject(GmatBase *obj, 
                                   const Gmat::ObjectType type,
                                   const wxString &name)
{
   if (obj->IsOfType(Gmat::SPACE_POINT))
   {
      // check to see if it's already in the list
      std::vector<SpacePoint*>::iterator pos =
         find(bodyList.begin(), bodyList.end(), obj);
      if (pos != bodyList.end())
      {
         #ifdef DEBUG_CP_OBJECT
         MessageInterface::ShowMessage
            (wxT("CalculatedPoint::SetRefObject() the body <%p> '%s' already exist, so ")
             wxT("returning true\n"), (*pos), name.c_str());
         #endif
         return true;
      }
      
      // If ref object has the same name, reset it (loj: 2008.10.24)      
      pos = bodyList.begin();
      wxString bodyName;
      bool bodyFound = false;
      while (pos != bodyList.end())
      {
         bodyName = (*pos)->GetName();         
         if (bodyName == name)
         {
            #ifdef DEBUG_CP_OBJECT
            MessageInterface::ShowMessage
               (wxT("CalculatedPoint::SetRefObject() resetting the pointer of body '%s' <%p> to ")
                wxT("<%p>\n"), bodyName.c_str(), (*pos), (SpacePoint*)obj);
            #endif
            
            (*pos) = (SpacePoint*)obj;
            bodyFound = true;
         }
         ++pos;
      }
      
      // If ref object not found, add it (loj: 2008.10.24)
      if (!bodyFound)
      {
         #ifdef DEBUG_CP_OBJECT
         MessageInterface::ShowMessage
            (wxT("CalculatedPoint::SetRefObject() this=<%p> '%s', adding <%p> '%s' ")
             wxT("to bodyList for object %s\n"), this, GetName().c_str(), obj, name.c_str(),
             instanceName.c_str());
         #endif
         
         bodyList.push_back((SpacePoint*) obj);         
         numberOfBodies++;
      }
      
      return true;
   }
   
   return SpacePoint::SetRefObject(obj, type, name);
}


//------------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//------------------------------------------------------------------------------
/**
 * Interface used to support user actions.
 *
 * @param <type>    reference object type.
 * @param <oldName> object name to be renamed.
 * @param <newName> new object name.
 * 
 * @return true if object name changed, false if not.
 */
//------------------------------------------------------------------------------
bool CalculatedPoint::RenameRefObject(const Gmat::ObjectType type,
                                      const wxString &oldName,
                                      const wxString &newName)
{
   if ((type == Gmat::SPACE_POINT) || (type == Gmat::CALCULATED_POINT))
   {
      for (unsigned int i=0; i< bodyNames.size(); i++)
      {
         if (bodyNames[i] == oldName)
             bodyNames[i] = newName;
      }
      for (unsigned int i=0; i< defaultBodies.size(); i++)
      {
         if (defaultBodies[i] == oldName)
            defaultBodies[i] = newName;
      }
   }
   return SpacePoint::RenameRefObject(type, oldName, newName);
}


//------------------------------------------------------------------------------
// virtual bool HasRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
bool CalculatedPoint::HasRefObjectTypeArray()
{
   return true;
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
const ObjectTypeArray& CalculatedPoint::GetRefObjectTypeArray()
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
const StringArray& CalculatedPoint::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::SPACE_POINT)
   {
      if (!bodyNames.empty())
         return bodyNames;
      else
         return defaultBodies;
   }
   
   // Not handled here -- invoke the next higher GetRefObject call
   return SpacePoint::GetRefObjectNameArray(type);
}

//------------------------------------------------------------------------------
// bool TakeAction(const wxString &action, const wxString &actionData)
//------------------------------------------------------------------------------
/**
 * Interface used to support user actions.
 *
 * @param <action> The string descriptor for the requested action.
 * @param <actionData> Optional data used for the action.
 *
 * @return true if the action was performed, false if not.
 */
//------------------------------------------------------------------------------
bool CalculatedPoint::TakeAction(const wxString &action,
                                 const wxString &actionData)
{
   #ifdef DEBUG_CP_ACTION
      MessageInterface::ShowMessage(
            wxT("Entering CP::TakeAction with action = \"%s\", actionData = \"%s\"\n"),
            action.c_str(), actionData.c_str());
   #endif
   if (action == wxT("ClearBodies"))
   {
      bodyNames.clear();
      bodyList.clear();
//      defaultBodies.clear();
      numberOfBodies = 0;
      return true; 
   }
   return SpacePoint::TakeAction(action, actionData);
}

//---------------------------------------------------------------------------
//  bool TakeRequiredAction(const Integer id))
//---------------------------------------------------------------------------
/**
 * Tells object to take whatever action it needs to take before the value
 * of the specified parameter is set (e.g. clearing out arrays)
 *
 * @param <id> parameter for which to take prerequisite action.
 *
 * @return true if the action was performed (or none needed), false if not.
 */
//------------------------------------------------------------------------------
bool CalculatedPoint::TakeRequiredAction(const Integer id)
{
   #ifdef DEBUG_CP_ACTION
      MessageInterface::ShowMessage(
            wxT("Entering CP::TakeRequiredAction with id = %d (%s)\n"),
            id, (GetParameterText(id)).c_str());
   #endif
   if (id == BODY_NAMES) return TakeAction(wxT("ClearBodies"));
   return SpacePoint::TakeRequiredAction(id);
}


//---------------------------------------------------------------------------
//  void SetDefaultBody(const wxString &defBody)
//---------------------------------------------------------------------------
/**
 * Method returning the total mass of the celestial bodies included in
 * this Barycenter.
 *
 * @return total mass of the celestial bodies included in this Barycenter.
 */
//---------------------------------------------------------------------------
void CalculatedPoint::SetDefaultBody(const wxString &defBody)
{
   if (find(defaultBodies.begin(), defaultBodies.end(), defBody) == defaultBodies.end())
   {
      #ifdef DEBUG_CP_BODIES
         MessageInterface::ShowMessage(wxT("Adding %s to DEFAULT body name list for object %s\n"),
               defBody.c_str(), instanceName.c_str());
      #endif
      defaultBodies.push_back(defBody);
   }
}

const StringArray& CalculatedPoint::GetDefaultBodies() const
{
   return defaultBodies;
}




//------------------------------------------------------------------------------
// private methods
//------------------------------------------------------------------------------
// none at this time
