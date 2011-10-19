//$Id: ExternalOptimizer.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                         ExternalOptimizer
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P
//
// Author: Wendy C. Shoan/GSFC
// Created: 2006.07.149
//
/**
 * Implementation for the external optimizer base class. 
 *
 */
//------------------------------------------------------------------------------


#include "ExternalOptimizer.hpp"
#include "FileManager.hpp"
#include "MessageInterface.hpp"



//------------------------------------------------------------------------------
// static data
//------------------------------------------------------------------------------

const wxString
ExternalOptimizer::PARAMETER_TEXT[ExternalOptimizerParamCount -OptimizerParamCount] =
{
   wxT("FunctionPath"),
   wxT("SourceType"),
};

const Gmat::ParameterType
ExternalOptimizer::PARAMETER_TYPE[ExternalOptimizerParamCount - OptimizerParamCount] =
{
   Gmat::STRING_TYPE,
   Gmat::STRING_TYPE,
};

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// ExternalOptimizer(wxString type, wxString name)
//------------------------------------------------------------------------------
ExternalOptimizer::ExternalOptimizer(wxString type, wxString name) :
   Optimizer               (type, name), 
   functionPath            (wxT("")),
   sourceType              (wxT("MATLAB")),
   sourceReady             (false),
   inSource                (NULL),
   inSourceServer          (NULL)
 {
   objectTypeNames.push_back(wxT("ExternalOptimizer"));
   parameterCount = ExternalOptimizerParamCount;
   
   isInternal = false;
}


//------------------------------------------------------------------------------
// ~ExternalOptimizer()
//------------------------------------------------------------------------------
ExternalOptimizer::~ExternalOptimizer()
{
   //FreeArrays();
}

//------------------------------------------------------------------------------
// ExternalOptimizer(const ExternalOptimizer &opt) :
//------------------------------------------------------------------------------
ExternalOptimizer::ExternalOptimizer(const ExternalOptimizer &opt) :
   Optimizer               (opt),
   functionPath            (opt.functionPath),
   sourceType              (opt.sourceType),
   sourceReady             (false),
   inSource                (opt.inSource),
   inSourceServer          (opt.inSourceServer)
{
   parameterCount   = opt.parameterCount;
}


//------------------------------------------------------------------------------
// ExternalOptimizer& operator=(const ExternalOptimizer& opt)
//------------------------------------------------------------------------------
ExternalOptimizer& ExternalOptimizer::operator=(const ExternalOptimizer& opt)
{
   if (&opt == this)
      return *this;
   
   Optimizer::operator=(opt);
   
   functionPath        = opt.functionPath;
   sourceType          = opt.sourceType;
   sourceReady         = opt.sourceReady;
   inSource            = opt.inSource;
   inSourceServer      = opt.inSourceServer;
   parameterCount      = opt.parameterCount;
   
   return *this;
}

//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
bool ExternalOptimizer::Initialize()
{
   Optimizer::Initialize();

   // function path
   FileManager *fm = FileManager::Instance();
   wxString pathname;
   
   try
   {
      if (functionPath == wxT(""))
      {
         // matlab uses directory path
         if (sourceType == wxT("MATLAB"))
            pathname = fm->GetFullPathname(wxT("MATLAB_FUNCTION_PATH"));
         
         functionPath = pathname;
      }
   }
   catch (GmatBaseException &)
   {
      try
      {
         // see if there is FUNCTION_PATH
         pathname = fm->GetFullPathname(wxT("FUNCTION_PATH"));
         functionPath = pathname;
      }
      catch (GmatBaseException &)
      {
         throw;  // for now, at least
      }
   }
   
   #ifdef DEBUG_MATLAB_PATH
   MessageInterface::ShowMessage
      (wxT("ExternalOptimizer::Initialize() functionPath='%s'\n"), functionPath.c_str());
   #endif
   return true;
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
 */
//------------------------------------------------------------------------------
wxString ExternalOptimizer::GetParameterText(const Integer id) const
{
   if ((id >= OptimizerParamCount) && (id < ExternalOptimizerParamCount))
      return PARAMETER_TEXT[id - OptimizerParamCount];
   return Optimizer::GetParameterText(id);
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
 */
//------------------------------------------------------------------------------
Integer ExternalOptimizer::GetParameterID(const wxString &str) const
{
   for (Integer i = OptimizerParamCount; i < ExternalOptimizerParamCount; ++i)
   {
      if (str == PARAMETER_TEXT[i - OptimizerParamCount])
         return i;
   }

   return Optimizer::GetParameterID(str);
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
 */
//------------------------------------------------------------------------------
Gmat::ParameterType ExternalOptimizer::GetParameterType(
                                              const Integer id) const
{
   if ((id >= OptimizerParamCount) && (id < ExternalOptimizerParamCount))
      return PARAMETER_TYPE[id - OptimizerParamCount];

   return Optimizer::GetParameterType(id);
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
 */
//------------------------------------------------------------------------------
wxString ExternalOptimizer::GetParameterTypeString(
                                      const Integer id) const
{
   return Optimizer::PARAM_TYPE_STRING[GetParameterType(id)];
}

//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return  string value of the requested parameter.
 */
//------------------------------------------------------------------------------
wxString ExternalOptimizer::GetStringParameter(const Integer id) const
{
    if (id == FUNCTION_PATH)
        return functionPath;
    if (id == SOURCE_TYPE)
        return sourceType;
        
    return Optimizer::GetStringParameter(id);
}


//------------------------------------------------------------------------------
//  Integer SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
/**
 * This method sets a string or string array parameter value, given the input
 * parameter ID.
 *
 * @param <id>    ID for the requested parameter.
 * @param <value> string value for the parameter.
 *
 * @return  The value of the parameter at the completion of the call.
 */
//------------------------------------------------------------------------------
bool ExternalOptimizer::SetStringParameter(const Integer id,
                                           const wxString &value)
{
    if (id == FUNCTION_PATH) 
    {
        functionPath = value; 
        return true;
    }
    if (id == SOURCE_TYPE) 
    {
        sourceType = value;  // currently, only MATLAB allowed
        return true;
    }
        
    
    return Optimizer::SetStringParameter(id, value);
}
// compiler complained again - so here they are ....
wxString ExternalOptimizer::GetStringParameter(const wxString& label) const
{
   return Optimizer::GetStringParameter(label);
}
bool ExternalOptimizer::SetStringParameter(const wxString& label,
                                           const wxString &value)
{
   return Optimizer::SetStringParameter(label, value);
}
wxString ExternalOptimizer::GetStringParameter(const Integer id,
                                                  const Integer index) const
{
   return Optimizer::GetStringParameter(id, index);
}

bool ExternalOptimizer::SetStringParameter(const Integer id, 
                                           const wxString &value,
                                           const Integer index)
{
   return Optimizer::SetStringParameter(id, value, index);
}

wxString ExternalOptimizer::GetStringParameter(const wxString &label,
                                                  const Integer index) const
{
   return Optimizer::GetStringParameter(label, index);
}

bool ExternalOptimizer::SetStringParameter(const wxString &label, 
                                           const wxString &value,
                                           const Integer index)
{
   return Optimizer::SetStringParameter(label, value, index);
}


//------------------------------------------------------------------------------
//  wxString  GetStringArrayParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return  StringArray value of the requested parameter.
 */
//------------------------------------------------------------------------------
/*
const StringArray& ExternalOptimizer::GetStringArrayParameter(
                                                        const Integer id) const
{
        
    if (id == PARAMETER_LIST)
        return parmList;
        
    return Optimizer::GetStringArrayParameter(id);
}
*/

