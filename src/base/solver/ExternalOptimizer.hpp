//$Id: ExternalOptimizer.hpp 9513 2011-04-30 21:23:06Z djcinsb $
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
// Author: Wendy C. Shoan, GSFC
// Created: 2006.07.13
//
/**
 * Definition for the ExternalOptimizer base class. 
 */
//------------------------------------------------------------------------------


#ifndef ExternalOptimizer_hpp
#define ExternalOptimizer_hpp

#include "Optimizer.hpp"
//#include wxT("GmatServer.hpp")  
#include "GmatInterface.hpp"  // a singleton

class GmatServer;             // Forward refefence the server


class GMAT_API ExternalOptimizer : public Optimizer
{
public:
   ExternalOptimizer(wxString type, wxString name);
   virtual ~ExternalOptimizer();
   ExternalOptimizer(const ExternalOptimizer &opt);
   ExternalOptimizer&      operator=(const ExternalOptimizer& opt);

   virtual bool        Initialize();

   // Access methods overriden from the base class
   
   virtual wxString GetParameterText(const Integer id) const;
   virtual Integer     GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                       GetParameterType(const Integer id) const;
   virtual wxString GetParameterTypeString(const Integer id) const;

   //virtual Integer     GetIntegerParameter(const Integer id) const;
   //virtual Integer     SetIntegerParameter(const Integer id,
   //                                        const Integer value);
   //virtual bool        GetBooleanParameter(const Integer id) const;
   //virtual bool        SetBooleanParameter(const Integer id,
   //                                        const bool value);
   virtual wxString GetStringParameter(const Integer id) const;
   virtual bool        SetStringParameter(const Integer id,
                                          const wxString &value);
   // compiler complained again - so here they are ....
   virtual wxString GetStringParameter(const wxString &label) const;
   virtual bool        SetStringParameter(const wxString &label,
                                          const wxString &value);
   virtual wxString GetStringParameter(const Integer id,
                                          const Integer index) const;
   virtual bool        SetStringParameter(const Integer id, 
                                          const wxString &value,
                                          const Integer index);
   virtual wxString GetStringParameter(const wxString &label,
                                          const Integer index) const;
   virtual bool        SetStringParameter(const wxString &label, 
                                          const wxString &value,
                                          const Integer index);
   //virtual const StringArray&
   //                    GetStringArrayParameter(const Integer id) const;
   //virtual bool        TakeAction(const wxString &action,
   //                               const wxString &actionData = wxT(""));

//------------------------------------------------------------------------------
//  bool  Optimize()    <pure virtual> <from Optimizer>
//------------------------------------------------------------------------------
/**
 * This method performs the optimization.
 *
 * @return success flag.
 */
//------------------------------------------------------------------------------
   virtual bool        Optimize() = 0;


protected:
   // Parameter IDs
   enum
   {
      FUNCTION_PATH = OptimizerParamCount,
      SOURCE_TYPE,
      ExternalOptimizerParamCount
   };
   

   /// Path for function script
   wxString         functionPath;
   /// array of values for the optimizer - value of the objective funcion;
   /// elements of the gradient (if calculated); values of the constraints
   //std::vector<Real>   costConstraintArray;
   /// type of external interface used (as of 2006.07.13, only MATLAB is 
   /// supported)
   wxString         sourceType;
   /// flag indicating whether or not the interface was opened successfully
   /// and the supporting structures needed by the interface were found
   bool                sourceReady;
   /// Pointer to GmatInterface
   GmatInterface       *inSource;
   /// Pointer to GmatServer
   GmatServer          *inSourceServer;
   
   static const wxString    PARAMETER_TEXT[ExternalOptimizerParamCount -
                                              OptimizerParamCount];
   static const Gmat::ParameterType
                               PARAMETER_TYPE[ExternalOptimizerParamCount -
                                              OptimizerParamCount];
   
   // Methods from Solver
   //virtual void                RunNominal();
   //virtual void                RunPerturbation();
   //virtual void                CalculateParameters();
   //virtual void                CheckCompletion();
   //virtual void                RunComplete();

   // Methods used to perform optimization
   //void                        CalculateJacobian();
   //void                        InvertJacobian();

   //void                        FreeArrays();
   //virtual wxString         GetProgressString();
   //virtual void                WriteToTextFile();
   
   virtual bool                  OpenConnection() = 0;
   virtual void                  CloseConnection() = 0;
};

#endif // ExternalOptimizer_hpp
