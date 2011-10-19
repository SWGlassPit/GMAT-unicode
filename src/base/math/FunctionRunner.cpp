//$Id: FunctionRunner.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  FunctionRunner
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CCA54C
//
// Author: Wendy Shoan
// Created: 2008.04.21
//
/**
 * Implements FunctionRunner class.
 */
//------------------------------------------------------------------------------

#include "FunctionRunner.hpp"
#include "MessageInterface.hpp"
#include "NumberWrapper.hpp"
#include "Variable.hpp"

//#define DEBUG_FUNCTION
//#define DEBUG_INPUT_OUTPUT
//#define DEBUG_EVALUATE
//#define DEBUG_FINALIZE

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif
//#ifndef DEBUG_PERFORMANCE
//#define DEBUG_PERFORMANCE
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif
#ifdef DEBUG_PERFORMANCE
#include <ctime>                 // for clock()
#endif

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// FunctionRunner()
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
FunctionRunner::FunctionRunner(const wxString &nomme)
   : MathFunction(wxT("FunctionRunner"), nomme)
{
   objectTypeNames.push_back(wxT("FunctionRunner"));
   theObjectMap = NULL;
   theGlobalObjectMap = NULL;
   theFunction = NULL;
   callingFunction = NULL;
   internalCS = NULL;
}


//------------------------------------------------------------------------------
// ~FunctionRunner()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
FunctionRunner::~FunctionRunner()
{
}


//------------------------------------------------------------------------------
//  FunctionRunner(const FunctionRunner &copy)
//------------------------------------------------------------------------------
/**
 * Constructs the FunctionRunner object (copy constructor).
 * 
 * @param <copy> Object that is copied
 */
//------------------------------------------------------------------------------
FunctionRunner::FunctionRunner(const FunctionRunner &copy) :
   MathFunction(copy)
{
   theObjectMap = copy.theObjectMap;
   theGlobalObjectMap = copy.theGlobalObjectMap;
   theFunction = copy.theFunction;
   callingFunction = copy.callingFunction;
   internalCS = copy.internalCS;
}


//------------------------------------------------------------------------------
// void SetFunctionName(const wxString &fname)
//------------------------------------------------------------------------------
/*
 * Sets function name to the FunctionManager.
 *
 * @param  fname  The function name to set
 */
//------------------------------------------------------------------------------
void FunctionRunner::SetFunctionName(const wxString &fname)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::SetFunctionName() passingArgs='%s', fname='%s'\n"),
       GetName().c_str(), fname.c_str());
   #endif
   
   theFunctionName = fname;
   theFunctionManager.SetFunctionName(fname);
}


//------------------------------------------------------------------------------
// void SetFunction(Function *function)
//------------------------------------------------------------------------------
/*
 * Sets function pointer to the FunctionManager.
 *
 * @param  function  The function pointer to set
 */
//------------------------------------------------------------------------------
void FunctionRunner::SetFunction(Function *function)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::SetFunction() passingArgs='%s', function=%p, name='%s'\n"),
       GetName().c_str(), function, function->GetName().c_str());
   #endif
   
   if (theFunctionManager.GetFunctionName() == function->GetName())
      theFunctionManager.SetFunction(function);
}


//------------------------------------------------------------------------------
// void AddFunctionInput(const wxString &name)
//------------------------------------------------------------------------------
/*
 * Adds function input argument name to the input names.
 *
 * @param  name  The name to add the list
 */
//------------------------------------------------------------------------------
void FunctionRunner::AddFunctionInput(const wxString &name)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::AddFunctionInput() passingArgs='%s', adding name='%s'\n"),
       GetName().c_str(), name.c_str());
   #endif
   
   theInputNames.push_back(name);
}


//------------------------------------------------------------------------------
// void SetFunctionOutputs()
//------------------------------------------------------------------------------
/*
 * Sets function output list to the FunctionManager.
 */
//------------------------------------------------------------------------------
void FunctionRunner::SetFunctionOutputs()
{
   theFunctionManager.SetOutputs(theOutputNames);
}


//------------------------------------------------------------------------------
// const StringArray& GetInputs()
//------------------------------------------------------------------------------
/*
 * @return the input list
 */
//------------------------------------------------------------------------------
const StringArray& FunctionRunner::GetInputs()
{
   return theInputNames;
}


//------------------------------------------------------------------------------
// void AddInputNode(MathNode *node)
//------------------------------------------------------------------------------
/*
 * Adds input MathNode to the list
 *
 * @param  node  input MathNode to add to the list
 */
//------------------------------------------------------------------------------
void FunctionRunner::AddInputNode(MathNode *node)
{
   if (node != NULL)
      theInputNodes.push_back(node);
}


//------------------------------------------------------------------------------
// void SetFunctionInputs()
//------------------------------------------------------------------------------
/*
 * Sets function input list to the FunctionManager.
 */
//------------------------------------------------------------------------------
void FunctionRunner::SetFunctionInputs()
{
   theFunctionManager.SetInputs(theInputNames);
}


//------------------------------------------------------------------------------
// void AddFunctionOutput(const wxString &name)
//------------------------------------------------------------------------------
/*
 * Adds function output argument name to the output names.
 *
 * @param  name  The name to add the list
 */
//------------------------------------------------------------------------------
void FunctionRunner::AddFunctionOutput(const wxString &name)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::AddFunctionOutput() passingArgs='%s', adding name='%s'\n"),
       GetName().c_str(), name.c_str());
   #endif
   
   theOutputNames.push_back(name);
}


//------------------------------------------------------------------------------
// void SetCallingFunction();
//------------------------------------------------------------------------------
void FunctionRunner::SetCallingFunction(FunctionManager *fm)
{
   callingFunction = fm;
}


//------------------------------------------------------------------------------
//  void SetObjectMap(ObjectMap *map)
//------------------------------------------------------------------------------
/**
 * Called by the MathTree to set the local asset store used by the GmatCommand
 * 
 * @param <map> Pointer to the local object map
 */
//------------------------------------------------------------------------------
void FunctionRunner::SetObjectMap(ObjectMap *map)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::SetObjectMap() entered, theFunctionName='%s', ")
       wxT("map=<%p>\n"), theFunctionName.c_str(), map);
   #endif

   theObjectMap = map;
   theFunctionManager.SetObjectMap(map);
}


//------------------------------------------------------------------------------
//  void SetGlobalObjectMap(ObjectMap *map)
//------------------------------------------------------------------------------
/**
 * Called by the MathTree to set the global asset store used by the GmatCommand
 * 
 * @param <map> Pointer to the global object map
 */
//------------------------------------------------------------------------------
void FunctionRunner::SetGlobalObjectMap(ObjectMap *map)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::SetGlobalObjectMap() entered, theFunctionName='%s', ")
       wxT("map=<%p>\n"), theFunctionName.c_str(), map);
   #endif
   
   theGlobalObjectMap = map;
   
   // Now, find the function object
   GmatBase *mapObj = FindObject(theFunctionName);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   => found the function: <%p><%s>\n"), mapObj,
       mapObj ? mapObj->GetName().c_str() : wxT("NULL"));
   #endif
   
   if (mapObj == NULL)
   {
      throw MathException(wxT("FunctionRunner::SetGlobalObjectMap cannot find the Function \"") +
                          theFunctionName);
   }
   else
   {
      theFunction = (Function *)mapObj;
      theFunctionManager.SetFunction(theFunction);
   }
   
   theFunctionManager.SetGlobalObjectMap(map);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("FunctionRunner::SetGlobalObjectMap() exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
//  void SetSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 * Called by the MathTree to set the SolarSystem used by the GmatCommand
 * 
 * @param <ss> Pointer to the SolarSystem
 */
//------------------------------------------------------------------------------
void FunctionRunner::SetSolarSystem(SolarSystem *ss)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::SetSolarSystem() entered, theFunctionName='%s', ")
       wxT("ss=<%p>\n"), theFunctionName.c_str(), ss);
   #endif
   
   theFunctionManager.SetSolarSystem(ss);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("FunctionRunner::SetSolarSystem() exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
//  void SetInternalCoordSystem(CoordinateSystem *cs)
//------------------------------------------------------------------------------
/**
 * Called by the MathTree to set the CoordinateSystem used by the GmatCommand and
 * function objects.
 * 
 * @param <cs> Pointer to the CoordinateSystem
 */
//------------------------------------------------------------------------------
void FunctionRunner::SetInternalCoordSystem(CoordinateSystem *cs)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::SetInternalCoordSystem() entered, theFunctionName='%s', ")
       wxT("cs=<%p>\n"), theFunctionName.c_str(), cs);
   #endif
   
   internalCS = cs;
   theFunctionManager.SetInternalCoordinateSystem(cs);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("FunctionRunner::SetInternalCoordSystem() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
//  void SetTransientForces(std::vector<PhysicalModel*> *tf)
//------------------------------------------------------------------------------
/**
 * Called by the MathTree to set the transient forces used by the GmatCommand
 * 
 * @param <map> Pointer to the vector of transient forces
 */
//------------------------------------------------------------------------------
void FunctionRunner::SetTransientForces(std::vector<PhysicalModel*> *tf)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::SetTransientForces() entered, theFunctionName='%s', ")
       wxT("tf=<%p>\n"), theFunctionName.c_str(), tf);
   #endif
   
   theFunctionManager.SetTransientForces(tf);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("FunctionRunner::SetTransientForces() exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
//  void SetPublisher(Publisher *pub)
//------------------------------------------------------------------------------
/**
 * Called by the MathTree to set the Publisher used by the GmatCommand
 * 
 * @param <pub> Pointer to the Publisher
 */
//------------------------------------------------------------------------------
void FunctionRunner::SetPublisher(Publisher *pub)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::SetPublisher() entered, theFunctionName='%s', ")
       wxT("pub=<%p>\n"), theFunctionName.c_str(), pub);
   #endif
   
   theFunctionManager.SetPublisher(pub);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("FunctionRunner::SetPublisher() exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
// void GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
//------------------------------------------------------------------------------
void FunctionRunner::GetOutputInfo(Integer &type,
                                   Integer &rowCount, Integer &colCount)
{
   Function *function = theFunctionManager.GetFunction();
   if (function == NULL)
      throw MathException(wxT("FunctionRunner::GetOutputInfo() function is NULL"));
   
   #ifdef DEBUG_INPUT_OUTPUT
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::GetOutputInfo() entered, this=<%p><%s>, function=<%s><%p>\n"),
       this, GetName().c_str(), function->GetName().c_str(), function);
   #endif
   
   // check for function output count
   IntegerArray rowCounts, colCounts;
   WrapperTypeArray outputTypes = function->GetOutputTypes(rowCounts, colCounts);
   wxString errMsg;
   
   if (outputTypes.size() == 0)
   {
      errMsg = wxT("The function \"") + function->GetName() + wxT("\" does not return")
         wxT(" any value");
   }
   else if (outputTypes.size() > 1)
   {
      errMsg = wxT("The function \"") + function->GetName() + wxT("\" returns more than ")
         wxT(" one value");
   }
   else
   {
      if (outputTypes[0] == Gmat::VARIABLE_WT)
      {
         type = Gmat::REAL_TYPE;
         rowCount = 1;
         colCount = 1;
      }
      else if (outputTypes[0] == Gmat::ARRAY_WT)
      {
         type = Gmat::RMATRIX_TYPE;
         rowCount = rowCounts[0];
         colCount = colCounts[0];
         matrix.SetSize(rowCount, colCount);
      }
   }
   
   elementType = type;
   
   if (errMsg != wxT(""))
      throw MathException(wxT("FunctionRunner::GetOutputInfo() ") + errMsg);
   
   //======================================================================
   #ifdef __ALLOW_MATH_EXP_NODE__
   //======================================================================
   
   if (leftNode == NULL)
      throw MathException(wxT("FunctionRunner::GetOutputInfo() The left node is NULL"));
   
   Integer type1, row1, col1; // Left node
   
   // Get the type(Real or Matrix), # rows and # columns of the left node
   leftNode->GetOutputInfo(type1, row1, col1);
   
   // Check output type and assign
   
   if (outputTypes[0] == Gmat::VARIABLE_WT)
   {
      if (type1 != Gmat::REAL_TYPE)
         throw MathException
            (wxT("FunctionRunner::GetOutputInfo() The GmatFunction \"%s\" expecting ")
             wxT("output type of Real"));
      
      type = Gmat::REAL_TYPE;
      rowCount = 1;
      colCount = 1;
   }
   else if (outputTypes[0] == Gmat::ARRAY_WT)
   {
      if (type1 != Gmat::RMATRIX_TYPE)
         throw MathException
            (wxT("FunctionRunner::GetOutputInfo() The GmatFunction \"%s\" expecting ")
             wxT("output type of Rmatrix"));
      
      type = Gmat::RMATRIX_TYPE;
      rowCount = rowCounts[0];
      colCount = colCounts[0];
      matrix.SetSize(rowCount, colCount);
   }
   
   //======================================================================
   #endif
   //======================================================================
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::GetOutputInfo() returning type=%d, rowCount=%d, colCount=%d\n"),
       type, rowCount, colCount);
   #endif
}


//------------------------------------------------------------------------------
// bool ValidateInputs()
//------------------------------------------------------------------------------
/**
 * This method calls its subnodes and checks to be sure that the subnodes return
 * compatible data for the function.
 */
//------------------------------------------------------------------------------
bool FunctionRunner::ValidateInputs()
{
   Function *function = theFunctionManager.GetFunction();
   if (function == NULL)
      throw MathException(wxT("FunctionRunner::ValidateInputs() function is NULL"));
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::ValidateInputs() entered, this=<%p><%s>, function=<%s><%p>\n"),
       this, GetName().c_str(), function->GetName().c_str(), function);
   #endif
   
   //@todo
   //How can we validate input here? Just return true for now.
   return true;
}


//------------------------------------------------------------------------------
// Real Evaluate()
//------------------------------------------------------------------------------
/**
 * @return the FunctionRunner of left node
 *
 */
//------------------------------------------------------------------------------
Real FunctionRunner::Evaluate()
{
   Function *function = theFunctionManager.GetFunction();
   if (function == NULL)
      throw MathException(wxT("FunctionRunner::Evaluate() function is NULL"));
   
   #ifdef DEBUG_PERFORMANCE
   static Integer callCount = 0;
   callCount++;      
   clock_t t1 = clock();
   MessageInterface::ShowMessage
      (wxT("=== FunctionRunner::Evaluate() entered, '%s' Count = %d\n"),
       GetName().c_str(), callCount);
   #endif
   
   #ifdef DEBUG_EVALUATE
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::Evaluate() entered, this=<%p><%s>, function=<%s><%p>, ")
       wxT("internalCS=<%p>\n"), this, GetName().c_str(), function->GetName().c_str(),
       function, internalCS);
   #endif
   
   if (elementType == Gmat::RMATRIX_TYPE)
      throw MathException
         (wxT("The function \"") + function->GetName() + wxT("\" returns matrix value"));
   
   //=======================================================
   #ifdef __ALLOW_MATH_EXP_NODE__
   HandlePassingMathExp(function);
   #endif
   //=======================================================
   
   #ifdef DEBUG_EVALUATE
   MessageInterface::ShowMessage
      (wxT("Calling FunctionManager::Evaluate()\n")
       wxT("===================================================================\n"));
   #endif
   
   Real result = -9999.9999;
   
   /// as temp fix, try setting InternalCoordinateSystem on FunctionManager
   theFunctionManager.SetInternalCoordinateSystem(internalCS);
   result = theFunctionManager.Evaluate(callingFunction);
   
   WrapperArray wrappersToDelete = theFunctionManager.GetWrappersToDelete();
   #ifdef DEBUG_EVALUATE
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::Evaluate(), there are %d wrappers to delete\n"),
       wrappersToDelete.size());
   #endif
   
   // Delete old output wrappers (loj: 2008.11.24)
   for (WrapperArray::iterator ewi = wrappersToDelete.begin();
        ewi < wrappersToDelete.end(); ewi++)
   {
      if ((*ewi) != NULL)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            ((*ewi), (*ewi)->GetDescription(), wxT("FunctionRunner::Evaluate()"),
             wxT(" deleting output wrapper"));
         #endif
         delete (*ewi);
         (*ewi) = NULL;
      }
   }
   
   #ifdef DEBUG_EVALUATE
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::Evaluate() returning %f\n"), result);
   #endif
   
   #ifdef DEBUG_PERFORMANCE
   clock_t t2 = clock();
   MessageInterface::ShowMessage
      (wxT("=== FunctionRunner::Evaluate() exiting, '%s' Count = %d, Run Time: %f seconds\n"),
       GetName().c_str(), callCount, (Real)(t2-t1)/CLOCKS_PER_SEC);
   #endif
   return result;
}


//------------------------------------------------------------------------------
// Rmatrix MatrixEvaluate()
//------------------------------------------------------------------------------
/**
 * @return the FunctionRunner of left node
 *
 */
//------------------------------------------------------------------------------
Rmatrix FunctionRunner::MatrixEvaluate()
{
   Function *function = theFunctionManager.GetFunction();
   if (function == NULL)
      throw MathException(wxT("FunctionRunner::Evaluate() function is NULL"));
   
   #ifdef DEBUG_PERFORMANCE
   static Integer callCount = 0;
   callCount++;      
   clock_t t1 = clock();
   MessageInterface::ShowMessage
      (wxT("=== FunctionRunner::MatrixEvaluate() entered, '%s' Count = %d\n"),
       GetName().c_str(), callCount);
   #endif
   
   #ifdef DEBUG_EVALUATE
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::MatrixEvaluate() entered, this=<%p><%s>, function=<%s><%p>\n"),
       this, GetName().c_str(), function->GetName().c_str(), function);
   #endif
   
   if (elementType == Gmat::REAL_TYPE)
      throw MathException
         (wxT("The function \"") + function->GetName() + wxT("\" returns Real value"));
   
   Rmatrix rmatResult = theFunctionManager.MatrixEvaluate(callingFunction);
   
   WrapperArray wrappersToDelete = theFunctionManager.GetWrappersToDelete();
   // Delete old output wrappers (loj: 2008.11.24)
   for (WrapperArray::iterator ewi = wrappersToDelete.begin();
        ewi < wrappersToDelete.end(); ewi++)
   {
      if ((*ewi) != NULL)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            ((*ewi), (*ewi)->GetDescription(), wxT("FunctionRunner::MatrixEvaluate()"),
             wxT(" deleting output wrapper"));
         #endif
         delete (*ewi);
         (*ewi) = NULL;
      }
   }
   
   #ifdef DEBUG_PERFORMANCE
   clock_t t2 = clock();
   MessageInterface::ShowMessage
      (wxT("=== FunctionRunner::MatrixEvaluate() exiting, '%s' Count = %d, Run Time: %f seconds\n"),
       GetName().c_str(), callCount, (Real)(t2-t1)/CLOCKS_PER_SEC);
   #endif
   
   return rmatResult;
}


//------------------------------------------------------------------------------
// void Finalize()
//------------------------------------------------------------------------------
void FunctionRunner::Finalize()
{
   #ifdef DEBUG_FINALIZE
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::RunComplete() calling FunctionManager::Finalize()\n"));
   #endif
   theFunctionManager.Finalize();
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Clone of the FunctionRunner operation.
 *
 * @return clone of the FunctionRunner operation.
 *
 */
//------------------------------------------------------------------------------
GmatBase* FunctionRunner::Clone() const
{
   return (new FunctionRunner(*this));
}


//------------------------------------------------------------------------------
// GmatBase* FunctionRunner::FindObject(const wxString &name)
//------------------------------------------------------------------------------
GmatBase* FunctionRunner::FindObject(const wxString &name)
{
   wxString newName = name;
   
   // Ignore array indexing of Array
   wxString::size_type index = name.find('(');
   if (index != name.npos)
      newName = name.substr(0, index);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("FunctionRunner::FindObject() theObjectMap=<%p>, theGlobalObjectMap=<%p>, ")
       wxT("newName='%s'\n"), theObjectMap, theGlobalObjectMap, newName.c_str());
   if (theObjectMap)
   {
      MessageInterface::ShowMessage(wxT("Here is the local object map:\n"));
      for (std::map<wxString, GmatBase *>::iterator i = theObjectMap->begin();
           i != theObjectMap->end(); ++i)
         MessageInterface::ShowMessage(wxT("   %s\n"), i->first.c_str());
   }
   if (theGlobalObjectMap)
   {
      MessageInterface::ShowMessage(wxT("Here is the global object map:\n"));
      for (std::map<wxString, GmatBase *>::iterator i = theGlobalObjectMap->begin();
           i != theGlobalObjectMap->end(); ++i)
         MessageInterface::ShowMessage(wxT("   %s\n"), i->first.c_str());
   }
   #endif
   
   // Check for the object in the Local Object Store (LOS) first
   if (theObjectMap && theObjectMap->find(newName) != theObjectMap->end())
      return (*theObjectMap)[newName];
   
   // If not found in the LOS, check the Global Object Store (GOS)
   if (theGlobalObjectMap &&
       theGlobalObjectMap->find(newName) != theGlobalObjectMap->end())
      return (*theGlobalObjectMap)[newName];
   
   return NULL;
}

//------------------------------------------------------------------------------
// void HandlePassingMathExp(Function *function)
//------------------------------------------------------------------------------
/*
 * @todo This method is not complete and will be implemented fully later.
 */
//------------------------------------------------------------------------------
void FunctionRunner::HandlePassingMathExp(Function *function)
{
   if (leftNode == NULL)
      throw MathException
         (wxT("The left node of \"") + function->GetName() + wxT("\" is NULL"));
   
   #ifdef DEBUG_EVALUATE
   MessageInterface::ShowMessage
      (wxT("   leftNode is%s Function input and %sa MathElement\n"),
       leftNode->IsFunctionInput() ? wxT("") : wxT(" not"),
       leftNode->IsOfType(wxT("MathElement")) ? wxT("") : wxT(" not"));
   MessageInterface::ShowMessage
      (wxT("Calling FunctionManager::Initialize()\n")
       wxT("===================================================================\n"));
   #endif
   
   theFunctionManager.PrepareObjectMap();
   theFunctionManager.Initialize();
   
   Integer numInputs = theInputNodes.size();
   MessageInterface::ShowMessage(wxT("..... Has %d inputs\n"), numInputs);
   
   #ifdef DEBUG_EVALUATE
   MessageInterface::ShowMessage
      (wxT("Evaluating Function input nodes\n")
       wxT("===================================================================\n"));
   #endif
   
   // Evaluate input nodes
   for (Integer i=0; i<numInputs; i++)
   {
      Real result = theInputNodes[i]->Evaluate();
      MessageInterface::ShowMessage(wxT("   ..... result=%f\n"), result);
      ElementWrapper *ew = theFunctionManager.GetInputWrapper(i);
      if (ew)
      {
         if (ew->GetDataType() == Gmat::REAL_TYPE)
         {
            MessageInterface::ShowMessage(wxT("..... Just setting value to wrapper\n"));
            ew->SetReal(result);
            NumberWrapper *nw = (NumberWrapper*)(theFunctionManager.GetInputWrapper(i));
            MessageInterface::ShowMessage(wxT("..... got %f form this wrapper\n"), nw->EvaluateReal());
         }
         else
         {
            MessageInterface::ShowMessage
               (wxT("***> Cannot set value to input wrapper, different data type\n"));
            MessageInterface::ShowMessage(wxT("..... Creating new NumberWrapper\n"));
            ElementWrapper *newWrapper = new NumberWrapper();
            
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Add
               (newWrapper, newWrapper->GetDescription(), wxT("FunctionRunner::HandlePassingMathExp()"),
                wxT("*newWrapper = new NumberWrapper()"));
            #endif
            
            newWrapper->SetReal(result);
            theFunctionManager.SetInputWrapper(i, newWrapper);
         }
      }
      else
      {
         MessageInterface::ShowMessage(wxT("..... Creating new NumberWrapper\n"));
         ew = new NumberWrapper();
         
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Add
            (ew, ew->GetDescription(), wxT("FunctionRunner::HandlePassingMathExp()"),
             wxT("ew = new NumberWrapper()"));
         #endif
         
         ew->SetReal(result);
         theFunctionManager.SetInputWrapper(i, ew);
      }
      
      bool inputAdded = false;
      // depends on return type of theInputNodes[i]->Evaluate(), create Variable or Array
      MessageInterface::ShowMessage
         (wxT("..... Creating Variable with '%s'\n"), theInputNodes[i]->GetName().c_str());
      Variable *passingInput = new Variable(theInputNodes[i]->GetName());
      
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Add
         (passingInput, passingInput->GetName(), wxT("FunctionRunner::HandlePassingMathExp()"),
          wxT("*passingInput = new Variable(theInputNodes[i]->GetName())"));
      #endif
      
      passingInput->SetReal(result);
      MessageInterface::ShowMessage(wxT("..... Calling FunctionManager::SetPassedInput()\n"));
      theFunctionManager.SetPassedInput(i, (GmatBase*)passingInput, inputAdded);
      if (!inputAdded)
         delete passingInput;
   }
}
