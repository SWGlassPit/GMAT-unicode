//$Id$
//------------------------------------------------------------------------------
//                            BeginFunction
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under MOMS Task
// order 124.
//
// Author: Darrel Conway, Thinking Systems, Inc.
// Created: 2005/08/30
//
/**
 * Definition code for the BeginFunction command, a wrapper that manages the
 * commands in a GMAT function.
 */
//------------------------------------------------------------------------------

#ifndef BeginFunction_hpp
#define BeginFunction_hpp



#include "GmatCommand.hpp"
#include "GmatFunction.hpp"



class GMAT_API BeginFunction : public GmatCommand
{
public:
   BeginFunction();
   virtual ~BeginFunction();
   BeginFunction(const BeginFunction& bf);
   BeginFunction&       operator=(const BeginFunction& bf);


   virtual GmatBase*    Clone() const;
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                    const wxString &name = wxT(""));
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);


   // Access methods inherited from GmatBase
   virtual wxString GetParameterText(const Integer id) const;
   virtual Integer     GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                       GetParameterType(const Integer id) const;
   virtual wxString GetParameterTypeString(const Integer id) const;

   virtual bool        IsParameterReadOnly(const Integer id) const;
   virtual bool        IsParameterReadOnly(const wxString &label) const;

   virtual wxString GetStringParameter(const Integer id) const;
   virtual wxString GetStringParameter(const Integer id,
                                          const Integer index) const;
   virtual const StringArray&
                       GetStringArrayParameter(const Integer id) const;
   virtual bool        SetStringParameter(const Integer id,
                                          const wxString &value);
   virtual bool        SetStringParameter(const Integer id,
                                          const wxString &value,
                                          const Integer index);
   virtual wxString GetStringParameter(const wxString &label) const;
   virtual wxString GetStringParameter(const wxString &label,
                                          const Integer index) const;
   virtual const StringArray&
                       GetStringArrayParameter(const wxString &label) const;
   virtual bool        SetStringParameter(const wxString &label,
                                          const wxString &value);
   virtual bool        SetStringParameter(const wxString &label,
                                          const wxString &value,
                                          const Integer index);

   virtual bool        TakeAction(const wxString &action,
                                  const wxString &actionData = wxT(""));


   virtual bool         Initialize();
   virtual bool         Execute();
   void                 ClearReturnObjects();
   virtual void         SetTransientForces(std::vector<PhysicalModel*> *tf);
   void                 InitializeInternalObjects();
   void                 BuildReferences(GmatBase *obj);
   void                 SetRefFromName(GmatBase *obj, const wxString &oName);
   virtual void         SetInternalCoordSystem(CoordinateSystem *cs);
   void                 ClearInputMap();


protected:
   /// Name of the function
   wxString          functionName;
   /// The actual GMAT function
   GmatFunction         *gfun;
   /// List of function parameters defined in the function file
   StringArray          inputs;
   /// List of output elements defined in the function file
   StringArray          outputs;
   /// Names of the function parameters used in the CallFunction
   StringArray          inputObjects;
   /// Names of output elements expected by the CallFunction
   StringArray          outputObjects;
   /// Mapping of parameter names to local vars and clones of the input objects
   std::map <wxString, GmatBase *>
                        localMap;
   /// Vector of the return objects
   ObjectArray          returnObjects;
   /// Transient force container, in case finite burns are active
   std::vector<PhysicalModel*>
                        *transientForces;
   /// CoordinateSystem used internally
   CoordinateSystem     *internalCoordSys;


   SpacePoint*          FindSpacePoint(const wxString &spName);
   wxString          trimIt( wxString s );


   /// Published parameters for functions
   enum
   {
      FUNCTION_NAME = GmatCommandParamCount,
      INPUTS,
      OUTPUTS,
      INPUT_OBJECT_NAMES,
      OUTPUT_OBJECT_NAMES,
      BeginFunctionParamCount
   };

   /// burn parameter labels
   static const wxString
      PARAMETER_TEXT[BeginFunctionParamCount - GmatCommandParamCount];
   /// burn parameter types
   static const Gmat::ParameterType
      PARAMETER_TYPE[BeginFunctionParamCount - GmatCommandParamCount];

};



#endif /* BeginFunction_hpp */
