//$Id: CallFunction.hpp 9513 2011-04-30 21:23:06Z djcinsb $
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
#ifndef CallFunction_hpp
#define CallFunction_hpp

#include "GmatCommand.hpp"
#include "Function.hpp"
#include "FunctionManager.hpp"
#include "Parameter.hpp"
#include "Array.hpp"
#include "StringVar.hpp"

// Forward references for GMAT core objects
class Publisher;

class GMAT_API CallFunction : public GmatCommand
{
public:
   CallFunction(const wxString &type);
   virtual ~CallFunction();
   
   CallFunction(const CallFunction& cf);
   CallFunction&        operator=(const CallFunction& cf);
   
   wxString          FormEvalString();
   bool                 AddInputParameter(const wxString &paramName, Integer index);
   bool                 AddOutputParameter(const wxString &paramName, Integer index);
   
   // override GmatCommand methods
   virtual bool         Initialize();
   virtual bool         Execute();
   virtual void         RunComplete();
   virtual void         SetInternalCoordSystem(CoordinateSystem *cs);
   
   // override these to set on FunctionManager (and find function object in GOS)
   virtual void         SetPublisher(Publisher *pub);
   virtual void         SetObjectMap(std::map<wxString, GmatBase *> *map);
   virtual void         SetGlobalObjectMap(std::map<wxString, GmatBase *> *map);
   virtual bool         HasAFunction();
   virtual bool         IsMatlabFunctionCall();
   
   // override GmatBase methods
   virtual GmatBase*    Clone() const;
   virtual const wxString&
                        GetGeneratingString(Gmat::WriteMode mode,
                                            const wxString &prefix = wxT(""),
                                            const wxString &useName = wxT(""));
   
   virtual bool         TakeAction(const wxString &action,
                                   const wxString &actionData = wxT(""));
   
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name = wxT(""));
   virtual ObjectArray& GetRefObjectArray(const Gmat::ObjectType type);
   
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const Integer id, const wxString &value);
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value);
   virtual bool         SetStringParameter(const Integer id, const wxString &value,
                                           const Integer index);
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value,
                                           const Integer index);
   virtual const StringArray&
                        GetStringArrayParameter(const Integer id) const;
   virtual const StringArray&
                        GetStringArrayParameter(const wxString &label) const;

protected:

   ObjectArray objectArray;
   std::vector<Parameter*> mInputList;
   std::vector<Parameter*> mOutputList;
   GmatCommand *callcmds;
   
   StringArray mInputNames;
   StringArray mOutputNames;
   
   Integer mNumInputParams;
   Integer mNumOutputParams;
   
   Function *mFunction;
   wxString mFunctionName;
   wxString mFunctionPathAndName;
   
   /// the manager for the Function
   FunctionManager fm;
   
   bool isGmatFunction;
   bool isMatlabFunction;
   
   void ClearInputParameters();
   void ClearOutputParameters();
   
   enum
   {
      FUNCTION_NAME = GmatCommandParamCount,
      ADD_INPUT,
      ADD_OUTPUT,
      COMMAND_STREAM,
      CallFunctionParamCount  /// Count of the parameters for this class
   };
   
   
   static const wxString
      PARAMETER_TEXT[CallFunctionParamCount - GmatCommandParamCount];
   static const Gmat::ParameterType
      PARAMETER_TYPE[CallFunctionParamCount - GmatCommandParamCount];
};
#endif // CallFunction_hpp

