//$Id: CallFunction.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                 CallFunction
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P
//
// Author: Allison Greene
// Created: 2004/09/22
//
/**
 * Definition for the CallFunction command class
 */
//------------------------------------------------------------------------------
#include "CallFunction.hpp"
#include "BeginFunction.hpp"
#include "StringTokenizer.hpp"
#include "StringUtil.hpp"          // for Replace()
#include "FileUtil.hpp"            // for ParseFileName()
#include "FileManager.hpp"         // for GetAllMatlabFunctionPaths()
#include "MessageInterface.hpp"
#include <sstream>


//#define DEBUG_CALL_FUNCTION_PARAM
//#define DEBUG_CALL_FUNCTION_INIT
//#define DEBUG_CALL_FUNCTION_EXEC
//#define DEBUG_SEND_PARAM
//#define DEBUG_UPDATE_VAR
//#define DEBUG_UPDATE_OBJECT
//#define DEBUG_SHOW_ARRAY
//#define DEBUG_GMAT_FUNCTION_INIT
//#define DEBUG_GET_OUTPUT
//#define DEBUG_OBJECT_MAP
//#define DEBUG_GLOBAL_OBJECT_MAP
//#define DEBUG_RUN_COMPLETE

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif
//#ifndef DEBUG_TRACE
//#define DEBUG_TRACE
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif
#ifdef DEBUG_TRACE
#include <ctime>                 // for clock()
#endif

//---------------------------------
// static data
//---------------------------------
const wxString
CallFunction::PARAMETER_TEXT[CallFunctionParamCount - GmatCommandParamCount] =
{
   wxT("FunctionName"),
   wxT("AddInput"),
   wxT("AddOutput"),
   wxT("CommandStream"),
};


const Gmat::ParameterType
CallFunction::PARAMETER_TYPE[CallFunctionParamCount - GmatCommandParamCount] =
{
   Gmat::STRING_TYPE,
   Gmat::STRINGARRAY_TYPE,
   Gmat::STRINGARRAY_TYPE,
   Gmat::OBJECT_TYPE,
};


//------------------------------------------------------------------------------
// CallFunction::CallFunction(const wxString &type)
//------------------------------------------------------------------------------
CallFunction::CallFunction(const wxString &type) :
   GmatCommand          (type),
   callcmds             (NULL),
   mFunction            (NULL),
   mFunctionName        (wxT("")),
   mFunctionPathAndName (wxT("")),
   isGmatFunction       (false),
   isMatlabFunction     (false)
{
   mNumInputParams = 0;
   mNumOutputParams = 0;
   
   parameterCount = CallFunctionParamCount;
   objectTypeNames.push_back(wxT("CallFunction"));
}


//------------------------------------------------------------------------------
// ~CallFunction()
//------------------------------------------------------------------------------
CallFunction::~CallFunction()
{
   if (callcmds)
      delete callcmds;
}


//------------------------------------------------------------------------------
// CallFunction(const CallFunction& cf) :
//------------------------------------------------------------------------------
CallFunction::CallFunction(const CallFunction& cf) :
   GmatCommand           (cf),
   callcmds              (NULL),
   mFunction             (cf.mFunction),
   mFunctionName         (cf.mFunctionName),
   mFunctionPathAndName  (cf.mFunctionPathAndName),
   fm                    (cf.fm)
{
   mNumInputParams = cf.mNumInputParams;
   mNumOutputParams = cf.mNumOutputParams;
   
   objectArray = cf.objectArray;
   mInputList = cf.mInputList;
   mOutputList = cf.mOutputList;
   callcmds = NULL;           // Commands must be reinitialized
   isGmatFunction = cf.isGmatFunction;
   isMatlabFunction = cf.isMatlabFunction;
   
   mInputNames  = cf.mInputNames;
   mOutputNames = cf.mOutputNames;
   
   parameterCount = CallFunctionParamCount;
}


//------------------------------------------------------------------------------
// CallFunction& operator=(const CallFunction& cf)
//------------------------------------------------------------------------------
CallFunction& CallFunction::operator=(const CallFunction& cf)
{
   if (this == &cf)
      return *this;
   
   GmatCommand::operator=(cf);
   
   mFunction = cf.mFunction;
   mFunctionName = cf.mFunctionName;
   mFunctionPathAndName = cf.mFunctionPathAndName;
   mNumInputParams = cf.mNumInputParams;
   mNumOutputParams = cf.mNumOutputParams;
   
   objectArray = cf.objectArray;
   mInputList = cf.mInputList;
   mOutputList = cf.mOutputList;
   callcmds = NULL;           // Commands must be reinitialized
   isGmatFunction = cf.isGmatFunction;
   isMatlabFunction = cf.isMatlabFunction;
   
   mInputNames  = cf.mInputNames;
   mOutputNames = cf.mOutputNames;
   fm               = cf.fm;
   
   return *this;
}


//------------------------------------------------------------------------------
// wxString FormEvalString()
//  String format
//    [Out1, Out2] = FunctionName(In1, In2, In3);
//------------------------------------------------------------------------------
wxString CallFunction::FormEvalString()
{
   #ifdef DEBUG_MATLAB_EVAL
   MessageInterface::ShowMessage
      (wxT("CallFunction::FormEvalString() entered, mFunction=<%p>'%s'\n"),
       mFunction, mFunction ? mFunction->GetName().c_str() : wxT("NULL"));
   #endif
   wxString evalString = wxT("");
   
   // left hand side of evaluation string and equals (if necessary)
   if (mOutputList.size() > 1)
   {
      evalString = evalString + wxT("[");
      Parameter *param = (Parameter *)mOutputList[0];
      evalString = evalString + param->GetName();
      
      for (unsigned int i=1; i<mOutputList.size(); i++)
      {
         param = (Parameter *)mOutputList[i];
         evalString = evalString +wxT(", ") + param->GetName();
      }
      
      evalString = evalString + wxT("] = ");
   }
   else if (mOutputList.size() == 1)
   {
      Parameter *param = (Parameter *)mOutputList[0];
      evalString = wxT("[") + evalString + param->GetName() + wxT("]");
      evalString = evalString +wxT(" = ");
   }
   else if (mOutputList.size() == 0)
   {
      // no left hand side
   }
   else
   {
      // need to throw an exception here
   }
   
   
   // right hand side of evaluation string
   // function name and left parenthesis
   evalString = evalString + mFunction->GetName().c_str() + wxT("(");


   // input parameters
   if (mInputList.size() > 0)
   {
      Parameter *param = (Parameter *)mInputList[0];
      evalString = evalString + param->GetName();

      for (unsigned int i=1; i<mInputList.size(); i++)
      {
         param = (Parameter *)mInputList[i];
         evalString = evalString + wxT(", ") + param->GetName();
      }
   }
   
   // right parenthesis and semi-colon
   evalString = evalString + wxT(");");
   
   return evalString;
}


//------------------------------------------------------------------------------
// bool AddInputParameter(const wxString &paramName, Integer index)
//------------------------------------------------------------------------------
bool CallFunction::AddInputParameter(const wxString &paramName, Integer index)
{
   if (paramName != wxT("") && index == mNumInputParams)
   {
      mInputNames.push_back(paramName);
      mNumInputParams = mInputNames.size();
      mInputList.push_back(NULL);
      fm.AddInput(paramName);
      return true;
   }
   
   return false;
}


//------------------------------------------------------------------------------
// bool AddOutputParameter(const wxString &paramName, Integer index)
//------------------------------------------------------------------------------
bool CallFunction::AddOutputParameter(const wxString &paramName, Integer index)
{
   if (paramName != wxT("") && index == mNumOutputParams)
   {
      mOutputNames.push_back(paramName);
      mNumOutputParams = mOutputNames.size();
      mOutputList.push_back(NULL);
      fm.AddOutput(paramName);      
      return true;
   }

   return false;
}


//------------------------------------------------------------------------------
//  void SetObjectMap(std::map<wxString, GmatBase *> *map)
//------------------------------------------------------------------------------
/**
 * Called by the Sandbox to set the local asset store used by the GmatCommand
 * 
 * @param <map> Pointer to the local object map
 */
//------------------------------------------------------------------------------
void CallFunction::SetObjectMap(std::map<wxString, GmatBase *> *map)
{
   GmatCommand::SetObjectMap(map);
   fm.SetObjectMap(map);
}


//------------------------------------------------------------------------------
//  void SetGlobalObjectMap(std::map<wxString, GmatBase *> *map)
//------------------------------------------------------------------------------
/**
 * Called by the Sandbox to set the global asset store used by the GmatCommand
 * 
 * @param <map> Pointer to the local object map
 */
//------------------------------------------------------------------------------
void CallFunction::SetGlobalObjectMap(std::map<wxString, GmatBase *> *map)
{
   #ifdef DEBUG_GLOBAL_OBJECT_MAP
   MessageInterface::ShowMessage
      (wxT("CallFunction::SetGlobalObjectMap() entered, mFunctionName='%s', ")
       wxT("map=<%p>\n"), mFunctionName.c_str(), map);
   #endif
   
   GmatCommand::SetGlobalObjectMap(map);
   
   // Now, find the function object
   GmatBase *mapObj = FindObject(mFunctionName);
   
   #ifdef DEBUG_GLOBAL_OBJECT_MAP
   MessageInterface::ShowMessage
      (wxT("   mapObj=<%p><%s>'%s'\n"), mapObj,
       mapObj ? mapObj->GetTypeName().c_str() : wxT("NULL"),
       mapObj ? mapObj->GetName().c_str() : wxT("NULL"));
   #endif
   
   if (mapObj == NULL)
   {
      //throw CommandException(wxT("CallFunction command cannot find Function ") +
      //         mFunctionName + wxT("\n"));
      ; // leave NULL for now
   }
   else
   {
      mFunction = (Function *)mapObj;
      
      #ifdef DEBUG_GLOBAL_OBJECT_MAP
      MessageInterface::ShowMessage
         (wxT("   mFunction=<%p><%s>\n"), mFunction, mFunction->GetName().c_str());
      #endif
      
      // Set only GmatFunction to FunctionManager (loj: 2008.09.03)
      if (mapObj->GetTypeName() == wxT("GmatFunction"))
         fm.SetFunction(mFunction);
   }
   fm.SetGlobalObjectMap(map);
   
   #ifdef DEBUG_GLOBAL_OBJECT_MAP
   MessageInterface::ShowMessage(wxT("CallFunction::SetGlobalObjectMap() exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
// bool HasAFunction()
//------------------------------------------------------------------------------
bool CallFunction::HasAFunction()
{
   return true;
}


//------------------------------------------------------------------------------
// virtual bool IsMatlabFunctionCall()
//------------------------------------------------------------------------------
bool CallFunction::IsMatlabFunctionCall()
{
   if (isMatlabFunction)
      return true;
   else
      return false;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the CallFunction.
 *
 * @return clone of the CallFunction.
 *
 */
//------------------------------------------------------------------------------
GmatBase* CallFunction::Clone() const
{
   return (new CallFunction(*this));
}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
wxString CallFunction::GetParameterText(const Integer id) const
{
   #ifdef DEBUG_CALL_FUNCTION_PARAM
      MessageInterface::ShowMessage(wxT("CallFunction::GetParameterText\n"));
   #endif

   if (id >= GmatCommandParamCount && id < CallFunctionParamCount)
      return PARAMETER_TEXT[id - GmatCommandParamCount];
   else
      return GmatCommand::GetParameterText(id);
}


//------------------------------------------------------------------------------
//  const wxString& GetGeneratingString()
//------------------------------------------------------------------------------
/**
 * Method used to retrieve the string that was parsed to build this GmatCommand.
 *
 * This method is used to retrieve the GmatCommand string from the script that
 * was parsed to build the GmatCommand.  It is used to save the script line, so
 * that the script can be written to a file without inverting the steps taken to
 * set up the internal object data.  As a side benefit, the script line is
 * available in the GmatCommand structure for debugging purposes.
 *
 * @param mode    Specifies the type of serialization requested.
 * @param prefix  Optional prefix appended to the object's name.  (Used to
 *                indent commands)
 * @param useName Name that replaces the object's name.  (Not used in
 *                commands)
 *
 * @return The script line that, when interpreted, defines this CallFunction.
 */
//------------------------------------------------------------------------------
const wxString& CallFunction::GetGeneratingString(Gmat::WriteMode mode,
                                                     const wxString &prefix,
                                                     const wxString &useName)
{
   wxString gen;
   
   // Build the local string
   if (mode != Gmat::NO_COMMENTS)
      gen = prefix + wxT("GMAT ");
   
   if (mOutputNames.size() > 0)
   {
      gen += wxT("[");
      for (StringArray::iterator i = mOutputNames.begin();
           i != mOutputNames.end(); ++i)
      {
         if (i != mOutputNames.begin())
            gen += wxT(", ");
         gen += *i;
      }
      gen += wxT("] = ");
   }
   
   gen += mFunctionName;
   
   if (mInputNames.size() > 0)
   {
      gen += wxT("(");
      for (StringArray::iterator i = mInputNames.begin();
           i != mInputNames.end(); ++i)
      {
         if (i != mInputNames.begin())
            gen += wxT(", ");
         gen += *i;
      }
      gen += wxT(")");
   }
   
   generatingString = gen + wxT(";");
   
   if (mode == Gmat::NO_COMMENTS)
      return generatingString;
   
   // Then call the base class method
   return GmatCommand::GetGeneratingString(mode, prefix, useName);
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
Integer CallFunction::GetParameterID(const wxString &str) const
{
   #ifdef DEBUG_CALL_FUNCTION_PARAM
      MessageInterface::ShowMessage(wxT("CallFunction::GetParameterID \n"));
   #endif

   for (int i=GmatCommandParamCount; i<CallFunctionParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatCommandParamCount])
         return i;
   }
   
   return GmatCommand::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
Gmat::ParameterType CallFunction::GetParameterType(const Integer id) const
{
   #ifdef DEBUG_CALL_FUNCTION_PARAM
      MessageInterface::ShowMessage(wxT("CallFunction::GetParameterType\n"));
   #endif

   if (id >= GmatCommandParamCount && id < CallFunctionParamCount)
      return PARAMETER_TYPE[id - GmatCommandParamCount];
   else
      return GmatCommand::GetParameterType(id);
}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
wxString CallFunction::GetParameterTypeString(const Integer id) const
{
   #ifdef DEBUG_CALL_FUNCTION_PARAM
      MessageInterface::ShowMessage(wxT("CallFunction::GetParameterTypeString\n"));
   #endif

   if (id >= GmatCommandParamCount && id < CallFunctionParamCount)
      return GmatBase::PARAM_TYPE_STRING[GetParameterType(id - GmatCommandParamCount)];
   else
      return GmatCommand::GetParameterTypeString(id);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
wxString CallFunction::GetStringParameter(const Integer id) const
{
   #ifdef DEBUG_CALL_FUNCTION_PARAM
      MessageInterface::ShowMessage(wxT("CallFunction::GetStringParameter\n"));
   #endif

   switch (id)
   {
   case FUNCTION_NAME:
      return fm.GetFunctionName();
      //return mFunctionName;
   default:
      return GmatCommand::GetStringParameter(id);
   }
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString CallFunction::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
bool CallFunction::SetStringParameter(const Integer id, const wxString &value)
{
   #ifdef DEBUG_CALL_FUNCTION_PARAM
      MessageInterface::ShowMessage
         (wxT("CallFunction::SetStringParameter with id = %d and value = %s\n"),
          id, value.c_str());
   #endif
      
   switch (id)
   {
   case FUNCTION_NAME:
      mFunctionName = value;
      mFunctionPathAndName = value;
      fm.SetFunctionName(value);
      return true;
   case ADD_INPUT:
      return AddInputParameter(value, mNumInputParams);
   case ADD_OUTPUT:
      return AddOutputParameter(value, mNumOutputParams);
   default:
      return GmatCommand::SetStringParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label,
//                         const wxString &value)
//------------------------------------------------------------------------------
bool CallFunction::SetStringParameter(const wxString &label,
                                const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// virtual bool SetStringParameter(const Integer id, const wxString &value,
//                                 const Integer index)
//------------------------------------------------------------------------------
bool CallFunction::SetStringParameter(const Integer id, const wxString &value,
                                const Integer index)
{
   switch (id)
   {
   case ADD_INPUT:
      return AddInputParameter(value, index);
   case ADD_OUTPUT:
      return AddOutputParameter(value, index);
   default:
      return GmatCommand::SetStringParameter(id, value, index);
   }
}


//------------------------------------------------------------------------------
// virtual bool SetStringParameter(const wxString &label,
//                                 const wxString &value,
//                                 const Integer index)
//------------------------------------------------------------------------------
bool CallFunction::SetStringParameter(const wxString &label,
                                const wxString &value,
                                const Integer index)
{
   return SetStringParameter(GetParameterID(label), value, index);
}


//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const Integer id) const
//------------------------------------------------------------------------------
const StringArray& CallFunction::GetStringArrayParameter(const Integer id) const
{
   switch (id)
   {
   case ADD_INPUT:
      return mInputNames;
   case ADD_OUTPUT:
      return mOutputNames;
   default:
      return GmatCommand::GetStringArrayParameter(id);
   }
}


//------------------------------------------------------------------------------
// StringArray& GetStringArrayParameter(const wxString &label) const
//------------------------------------------------------------------------------
const StringArray& CallFunction::GetStringArrayParameter(const wxString &label) const
{
   return GetStringArrayParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual bool TakeAction(const wxString &action,
//                         const wxString &actionData = wxT(""));
//------------------------------------------------------------------------------
/**
 * This method performs action.
 *
 * @param <action> action to perform
 * @param <actionData> action data associated with action
 * @return true if action successfully performed
 *
 */
//------------------------------------------------------------------------------
bool CallFunction::TakeAction(const wxString &action,
                        const wxString &actionData)
{
   if (action == wxT("ClearInput"))
   {
      ClearInputParameters();
      return true;
   }
   else if (action == wxT("ClearOutput"))
   {
      ClearOutputParameters();
      return true;
   }
   else if (action == wxT("Clear"))
   {
      ClearInputParameters();
      ClearOutputParameters();
      objectArray.clear();
      return true;
   }

   return GmatCommand::TakeAction(action, actionData);
}


//------------------------------------------------------------------------------
// StringArray GetRefObjectNameArray(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
const StringArray& CallFunction::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   refObjectNames.clear();
   
   switch (type) {
      case Gmat::PARAMETER:         // Input/Output
         for (unsigned int i=0; i<mInputNames.size(); i++)
            refObjectNames.push_back(mInputNames[i]);
         for (unsigned int i=0; i<mOutputNames.size(); i++)
            refObjectNames.push_back(mOutputNames[i]);
         return refObjectNames;
      default:
         break;
   }
   
   return refObjectNames;
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
bool CallFunction::RenameRefObject(const Gmat::ObjectType type,
                                   const wxString &oldName,
                                   const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("CallFunction::RenameRefObject() type=%d, oldName='%s', newName='%s'\n"),
       type, oldName.c_str(), newName.c_str());
   #endif
   
   if (type == Gmat::FUNCTION)
   {
      if (mFunctionName == oldName)
         mFunctionName = newName;
   }
   else if (type == Gmat::PARAMETER)
   {
      // parameters - go through input and output
      for (unsigned int i=0; i<mInputNames.size(); i++)
      {
         if (mInputNames[i] == oldName)
         {
            mInputNames[i] = newName;
            break;
         }
      }

      for (unsigned int i=0; i<mOutputNames.size(); i++)
      {
         if (mOutputNames[i] == oldName)
         {
            mOutputNames[i] = newName;
            break;
         }
      }
   }
   // Since parameter name is composed of spacecraftName.dep.paramType or
   // burnName.dep.paramType, check the type first
   else if (type == Gmat::SPACECRAFT || type == Gmat::BURN ||
            type == Gmat::COORDINATE_SYSTEM || type == Gmat::CALCULATED_POINT)
   {
      
      for (UnsignedInt i=0; i<mInputNames.size(); i++)
         if (mInputNames[i].find(oldName) != wxString::npos)
            mInputNames[i] =
               GmatStringUtil::Replace(mInputNames[i], oldName, newName);
      
      for (UnsignedInt i=0; i<mOutputNames.size(); i++)
         if (mOutputNames[i].find(oldName) != wxString::npos)
            mOutputNames[i] =
               GmatStringUtil::Replace(mOutputNames[i], oldName, newName);
   }

   return true;
}


// Reference object accessor methods
//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
GmatBase* CallFunction::GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name)
{
   switch (type)
   {
      case Gmat::PARAMETER:
         for (int i=0; i<mNumInputParams; i++)
         {
            if (mInputNames[i] == name)
               return mInputList[i];
         }
         
         for (int i=0; i<mNumOutputParams; i++)
         {
            if (mOutputNames[i] == name)
               return mOutputList[i];
         }
         
         throw GmatBaseException(wxT("ReportFile::GetRefObject() the object name: ")
                           + name + wxT("not found\n"));
         
      case Gmat::FUNCTION:
         return mFunction;
         
      case Gmat::COMMAND:
         return callcmds;
         
      default:
         break;
   }

   // Not handled here -- invoke the next higher GetRefObject call
   return GmatCommand::GetRefObject(type, name);
}


//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type, ...
//------------------------------------------------------------------------------
/**
 * Sets reference object pointer.
 *
 * @return true if object successfully set, false otherwise
 */
//------------------------------------------------------------------------------
bool CallFunction::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                const wxString &name)
{
   #ifdef DEBUG_CALL_FUNCTION_REF_OBJ
   MessageInterface::ShowMessage
      (wxT("CallFunction::SetRefObject() entered, obj=<%p><%s>'%s', type=%d, name='%s'\n"),
       obj, obj ? obj->GetTypeName().c_str() : wxT("NULL"), obj ? obj->GetName().c_str() : wxT("NULL"),
       type, name.c_str());
   #endif
   
   if (obj == NULL)
      return false;
   
   switch (type)
   {
   case Gmat::PARAMETER:
      for (int i=0; i<mNumInputParams; i++)
      {
         if (mInputNames[i] == name)
         {
            mInputList[i] = (Parameter*)obj;
            return true;
         }
      }

      for (int i=0; i<mNumOutputParams; i++)
      {
         if (mOutputNames[i] == name)
         {
            mOutputList[i] = (Parameter*)obj;
            return true;
         }
      }
      
   case Gmat::FUNCTION:
      if (name == mFunctionName)
      {
         mFunction = (Function *)obj;
         mFunctionPathAndName = mFunction->GetFunctionPathAndName();
         if (mFunction && mFunction->GetTypeName() == wxT("GmatFunction"))
         {
            fm.SetFunction(mFunction);
            isGmatFunction = true;
            isMatlabFunction = false;
         }
      }
      return true;
      
   case Gmat::COMMAND:
      if (callcmds)
         delete callcmds;
      callcmds = (GmatCommand*)obj;
      return true;
      
   default:
      break;
   }
   
   // Not handled here -- invoke the next higher SetRefObject call
   return GmatCommand::SetRefObject(obj, type, name);
}


//------------------------------------------------------------------------------
// virtual ObjectArray& GetRefObjectArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
ObjectArray& CallFunction::GetRefObjectArray(const Gmat::ObjectType type)
{
   switch (type)
   {
   case Gmat::PARAMETER:
      objectArray.clear();

      for (unsigned int i=0; i<mInputList.size(); i++)
         objectArray.push_back(mInputList[i]);
      
      for (unsigned int i=0; i<mOutputList.size(); i++)
         objectArray.push_back(mOutputList[i]);
      
      return objectArray;
      
   default:
      break;
   }

   // Not handled here -- invoke the next higher SetReferenceObject call
   return GmatCommand::GetRefObjectArray(type);
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
bool CallFunction::Initialize()
{
   #ifdef DEBUG_CALL_FUNCTION_INIT
      MessageInterface::ShowMessage
         (wxT("CallFunction::Initialize() this=<%p> entered, command = '%s'\n   ")
          wxT("function type is '%s', callingFunction is '%s'\n"), this,
          GetGeneratingString(Gmat::NO_COMMENTS).c_str(), mFunction->GetTypeName().c_str(),
          callingFunction? (callingFunction->GetFunctionName()).c_str() : wxT("NULL"));
   #endif
   
   GmatCommand::Initialize();
   
   #ifdef DEBUG_OBJECT_MAP
   ShowObjectMaps(wxT("In CallFunction::Initialize()"));
   #endif
   
   isGmatFunction = false;
   isMatlabFunction = false;
   
   bool rv = true;  // Initialization return value
   if (mFunction == NULL)
      throw CommandException(wxT("CallFunction::Initialize() the function pointer is NULL"));
   
   if (mFunction->GetTypeName() == wxT("GmatFunction"))
      isGmatFunction = true;
   else if (mFunction->GetTypeName() == wxT("MatlabFunction"))
      isMatlabFunction = true;
   
   if (!isGmatFunction && !isMatlabFunction)
      throw CommandException
         (wxT("CallFunction::Initialize() the function is neither GmatFunction nor MatlabFunction"));
   
   mFunctionPathAndName = mFunction->GetFunctionPathAndName();
   wxString fname = GmatFileUtil::ParseFileName(mFunctionPathAndName);
   if (fname == wxT(""))
      mFunctionPathAndName += mFunctionName;
   
   #ifdef DEBUG_CALL_FUNCTION_INIT
   MessageInterface::ShowMessage
      (wxT("CallFunction::Initialize() returning %d, fname='%s', mFunctionName='%s', ")
       wxT("mFunctionPathAndName='%s'\n"), rv, fname.c_str(), mFunctionName.c_str(),
       mFunctionPathAndName.c_str());
   #endif
   
   return rv;
}


//------------------------------------------------------------------------------
// bool Execute()
//------------------------------------------------------------------------------
bool CallFunction::Execute()
{
   bool status = false;
   
   if (mFunction == NULL)
      throw CommandException(wxT("Function is not defined for CallFunction"));
   
   #ifdef DEBUG_TRACE
   static Integer callCount = 0;
   callCount++;      
   clock_t t1 = clock();
   MessageInterface::ShowMessage
      (wxT("=== CallFunction::Execute() entered, '%s' Count = %d\n"),
       GetGeneratingString(Gmat::NO_COMMENTS).c_str(), callCount);
   #endif
   
   #ifdef DEBUG_CALL_FUNCTION_EXEC
      MessageInterface::ShowMessage
         (wxT("CallFunction::Execute() this=<%p> entered, command = '%s'\n   ")
          wxT("function type is '%s', callingFunction is '%s'\n"), this,
          GetGeneratingString(Gmat::NO_COMMENTS).c_str(), mFunction->GetTypeName().c_str(),
          callingFunction? (callingFunction->GetFunctionName()).c_str() : wxT("NULL"));
      #ifdef DEBUG_OBJECT_MAP
      ShowObjectMaps(wxT("object maps at the start"));
      #endif
   #endif
      
   return status;
}


//------------------------------------------------------------------------------
// void RunComplete()
//------------------------------------------------------------------------------
void CallFunction::RunComplete()
{
   #ifdef DEBUG_RUN_COMPLETE
   MessageInterface::ShowMessage
      (wxT("CallFunction::RunComplete() entered for this=<%p> '%s',\n   ")
       wxT("FCS %sfinalized\n"), this, GetGeneratingString(Gmat::NO_COMMENTS).c_str(),
       fm.IsFinalized() ? wxT("already ") : wxT("NOT "));
   #endif
   
   if (!fm.IsFinalized())
   {
      #ifdef DEBUG_RUN_COMPLETE
      MessageInterface::ShowMessage(wxT("   calling FunctionManager::Finalize()\n"));
      #endif
      fm.Finalize();
   }
   
   GmatCommand::RunComplete();
}


//------------------------------------------------------------------------------
// void ClearInputParameters()
//------------------------------------------------------------------------------
void CallFunction::ClearInputParameters()
{
   mInputList.clear();
   mInputNames.clear();
   mNumInputParams = 0;
}


//------------------------------------------------------------------------------
// void ClearOutputParameters()
//------------------------------------------------------------------------------
void CallFunction::ClearOutputParameters()
{
   mOutputList.clear();
   mOutputNames.clear();
   mNumOutputParams = 0;
}


//------------------------------------------------------------------------------
// void SetInternalCoordSystem(CoordinateSystem *cs)
//------------------------------------------------------------------------------
/**
 *  Sets the internal coordinate system used by the Sandbox.
 *
 *  @param <cs> The internal coordinate system.
 */
//------------------------------------------------------------------------------
void CallFunction::SetInternalCoordSystem(CoordinateSystem *cs)
{
   /// @todo Check initialization and cloning for the internal CoordinateSystem.
   //internalCoordSys = (CoordinateSystem*)(cs->Clone());
   internalCoordSys = cs;
   fm.SetInternalCoordinateSystem(internalCoordSys);
}


//------------------------------------------------------------------------------
// void SetPublisher(Publisher *pub)
//------------------------------------------------------------------------------
/**
 *  Passes the Publisher used by the Sandbox to FunctionManager
 *
 *  @param <pub> The publisher
 */
//------------------------------------------------------------------------------
void CallFunction::SetPublisher(Publisher *pub)
{
   #ifdef DEBUG_PUBLISHER
   MessageInterface::ShowMessage
      (wxT("CallFunction::SetPublisher() setting publiser <%p> to FunctionManager\n"), pub);
   #endif
   GmatCommand::SetPublisher(pub);
   fm.SetPublisher(pub);
}


