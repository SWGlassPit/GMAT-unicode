//$Id: GmatFunction.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  GmatFunction
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
// Author: Allison Greene
// Created: 2004/12/16
//
/**
 * Implementation for the GmatFunction class.
 */
//------------------------------------------------------------------------------

#include "GmatFunction.hpp"
#include "Assignment.hpp"        // for Assignment::GetMathTree()
#include "FileManager.hpp"       // for GetGmatFunctionPath()
#include "FileUtil.hpp"          // for ParseFileName()
#include "StringUtil.hpp"        // for Trim()
#include "CommandUtil.hpp"       // for ClearCommandSeq()
#include "HardwareException.hpp" 
#include "MessageInterface.hpp"

//#define DEBUG_FUNCTION
//#define DEBUG_FUNCTION_SET
//#define DEBUG_FUNCTION_INIT
//#define DEBUG_FUNCTION_EXEC
//#define DEBUG_FUNCTION_FINALIZE
//#define DEBUG_UNUSED_GOL

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
//const wxString
//GmatFunction::PARAMETER_TEXT[GmatFunctionParamCount - FunctionParamCount] =
//{
//};
//
//const Gmat::ParameterType
//GmatFunction::PARAMETER_TYPE[GmatFunctionParamCount - FunctionParamCount] =
//{
//};




//------------------------------------------------------------------------------
// GmatFunction(wxString &name)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> function name
 */
//------------------------------------------------------------------------------
GmatFunction::GmatFunction(const wxString &name) :
   Function(wxT("GmatFunction"), name)
{
   mIsNewFunction = false;
   unusedGlobalObjectList = NULL;
   
   // for initial function path, use FileManager
   FileManager *fm = FileManager::Instance();
   wxString pathname;
   
   try
   {
      // if there is a function name, try to locate it
      if (name != wxT(""))
      {
         // Get path of first it is located
         pathname = fm->GetGmatFunctionPath(name + wxT(".gmf"));
         
         // gmat function uses whole path name
         pathname = pathname + name + wxT(".gmf");         
         functionPath = pathname;
         functionName = GmatFileUtil::ParseFileName(functionPath);
         
         // Remove path and .gmf
         functionName = GmatFileUtil::ParseFileName(functionPath);
         wxString::size_type dotIndex = functionName.find(wxT(".gmf"));
         functionName = functionName.substr(0, dotIndex);
      }
      else
      {
         // gmat function uses whole path name
         functionPath = fm->GetFullPathname(wxT("GMAT_FUNCTION_PATH"));    
      }
   }
   catch (GmatBaseException &e)
   {
      // Use exception to remove Visual C++ warning
      e.GetMessageType();
      #ifdef DEBUG_FUNCTION
      MessageInterface::ShowMessage(e.GetFullMessage());
      #endif
      
      // see if there is FUNCTION_PATH
      try
      {
         pathname = fm->GetFullPathname(wxT("FUNCTION_PATH"));
         functionPath = pathname;
      }
      catch (GmatBaseException &e)
      {
         // Use exception to remove Visual C++ warning
         e.GetMessageType();
         #ifdef DEBUG_FUNCTION
         MessageInterface::ShowMessage(e.GetFullMessage());
         #endif
      }
   }
   
   objectTypeNames.push_back(wxT("GmatFunction"));

   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   Gmat functionPath=<%s>\n"), functionPath.c_str());
   MessageInterface::ShowMessage
      (wxT("   Gmat functionName=<%s>\n"), functionName.c_str());
   #endif
}


//------------------------------------------------------------------------------
// ~GmatFunction()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
GmatFunction::~GmatFunction()
{
   #ifdef DEBUG_GMATFUNCTION
   MessageInterface::ShowMessage
      (wxT("GmatFunction() destructor entered, this=<%p> '%s', fcs=<%p>\n"), this,
       GetName().c_str(), fcs);
   #endif
   
   // delete function sequence including NoOp (loj: 2008.12.22)
   if (fcs != NULL)
      GmatCommandUtil::ClearCommandSeq(fcs, false);
   
   if (unusedGlobalObjectList != NULL)
      delete unusedGlobalObjectList;
   
   #ifdef DEBUG_GMATFUNCTION
   MessageInterface::ShowMessage(wxT("GmatFunction() destructor exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
// GmatFunction(const GmatFunction &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> object being copied
 */
//------------------------------------------------------------------------------
GmatFunction::GmatFunction(const GmatFunction &copy) :
   Function(copy)
{
   mIsNewFunction = false;
   unusedGlobalObjectList = NULL;
}


//------------------------------------------------------------------------------
// GmatFunction& GmatFunction::operator=(const GmatFunction& right)
//------------------------------------------------------------------------------
/**
 * The assignment operator
 *
 * @param <right> object being copied
 *
 * @return reference to this object
 */
//------------------------------------------------------------------------------
GmatFunction& GmatFunction::operator=(const GmatFunction& right)
{
   if (this == &right)
      return *this;
   
   Function::operator=(right);
   mIsNewFunction = false;
   unusedGlobalObjectList = NULL;
   
   return *this;
}


//------------------------------------------------------------------------------
// bool IsNewFunction()
//------------------------------------------------------------------------------
/**
 * Return true if function was created but not saved to file.
 * FunctionSetupPanel uses this flag to open new editor or load existing function.
 */
//------------------------------------------------------------------------------
bool GmatFunction::IsNewFunction()
{
   return mIsNewFunction;
}


//------------------------------------------------------------------------------
// void SetNewFunction(bool flag)
//------------------------------------------------------------------------------
void GmatFunction::SetNewFunction(bool flag)
{
   mIsNewFunction = flag;
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
bool GmatFunction::Initialize()
{
   #ifdef DEBUG_TRACE
   static Integer callCount = 0;
   callCount++;      
   clock_t t1 = clock();
   ShowTrace(callCount, t1, wxT("GmatFunction::Initialize() entered"));
   #endif
   
   #ifdef DEBUG_FUNCTION_INIT
      MessageInterface::ShowMessage
         (wxT("======================================================================\n")
          wxT("GmatFunction::Initialize() entered for function '%s'\n"), functionName.c_str());
      MessageInterface::ShowMessage(wxT("   and FCS is %s set.\n"), (fcs? wxT("correctly") : wxT("NOT")));
      MessageInterface::ShowMessage(wxT("   Pointer for FCS is %p\n"), fcs);
      MessageInterface::ShowMessage(wxT("   First command in fcs is %s\n"),
            (fcs->GetTypeName()).c_str());
      MessageInterface::ShowMessage(wxT("   internalCS is %p\n"), internalCoordSys);
   #endif
   if (!fcs) return false;
   
   Function::Initialize();
   
   // Initialize the Validator - I think I need to do this each time - or do I?
   validator->SetFunction(this);
   validator->SetSolarSystem(solarSys);
   std::map<wxString, GmatBase *>::iterator omi;
   
   // add automatic objects such as sat.X to the FOS (well, actually, clones of them)
   for (omi = automaticObjectMap.begin(); omi != automaticObjectMap.end(); ++omi)
   {
      wxString autoObjName = omi->first;
      
      // if name not found, clone it and add to map (loj: 2008.12.15)
      if (objectStore->find(autoObjName) == objectStore->end())
      {
         GmatBase *autoObj = (omi->second)->Clone();
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Add
            (autoObj, autoObjName, wxT("GmatFunction::Initialize()"),
             wxT("autoObj = (omi->second)->Clone()"));
         #endif
         
         #ifdef DEBUG_FUNCTION_INIT
         try
         {
            MessageInterface::ShowMessage
               (wxT("   autoObj->EvaluateReal() = %f\n"), autoObj->GetRealParameter(wxT("Value")));
         }
         catch (BaseException &e)
         {
            MessageInterface::ShowMessage(e.GetFullMessage());             
         }
         #endif
         
         autoObj->SetIsLocal(true);
         objectStore->insert(std::make_pair(autoObjName, autoObj));
      }
   }
   
   // first, send all the commands the object store, solar system, etc
   GmatCommand *current = fcs;
   
   while (current)
   {
      #ifdef DEBUG_FUNCTION_INIT
         if (!current)  MessageInterface::ShowMessage(wxT("Current is NULL!!!\n"));
         else MessageInterface::ShowMessage(wxT("   =====> Current command is %s <%s>\n"),
                 (current->GetTypeName()).c_str(),
                 current->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
      #endif
      current->SetObjectMap(objectStore);
      current->SetGlobalObjectMap(globalObjectStore);
      current->SetSolarSystem(solarSys);
      current->SetInternalCoordSystem(internalCoordSys);
      current->SetTransientForces(forces);
      #ifdef DEBUG_FUNCTION_INIT
         MessageInterface::ShowMessage
            (wxT("   Now about to set object map of type %s to Validator\n"),
             (current->GetTypeName()).c_str());      
      #endif
      // (Re)set object map on Validator (necessary because objects may have been added to the 
      // Local Object Store or Global Object Store during initialization of previous commands)
      validatorStore.clear();
      for (omi = objectStore->begin(); omi != objectStore->end(); ++omi)
         validatorStore.insert(std::make_pair(omi->first, omi->second));
      for (omi = globalObjectStore->begin(); omi != globalObjectStore->end(); ++omi)
         validatorStore.insert(std::make_pair(omi->first, omi->second));
      validator->SetObjectMap(&validatorStore);
      
      #ifdef DEBUG_FUNCTION_INIT
      MessageInterface::ShowMessage
         (wxT("   Now about to call Validator->ValidateCommand() of type %s\n"),
          current->GetTypeName().c_str());
      #endif
      
      // Let's try to ValidateCommand here, this will validate the command
      // and create wrappers also
      if (!validator->ValidateCommand(current, false, 2))
      {
         // get error message (loj: 2008.06.04)
         StringArray errList = validator->GetErrorList();
         wxString msg; // Check for empty errList (loj: 2009.03.17)
         if (errList.empty())
            msg = wxT("Error occurred");
         else
            msg = errList[0];
         
         throw FunctionException(msg + wxT(" in the function \"") + functionPath + wxT("\""));
      }
      
      #ifdef DEBUG_FUNCTION_INIT
      MessageInterface::ShowMessage
         (wxT("   Now about to initialize command of type %s\n"), current->GetTypeName().c_str());
      #endif
      
      // catch exception and add function name to message (loj: 2008.09.23)
      try
      {
         if (!(current->Initialize()))
         {
            #ifdef DEBUG_FUNCTION_INIT
            MessageInterface::ShowMessage
               (wxT("Exiting  GmatFunction::Initialize for function '%s' with false\n"),
                functionName.c_str());
            #endif
            return false;
         }
      }
      catch (BaseException &e)
      {
         throw FunctionException(wxT("Cannot continue due to ") + e.GetFullMessage() +
                                 wxT(" in the function \"") + functionPath + wxT("\""));
      }
      
      // Check to see if the command needs a server startup (loj: 2008.07.25)
      if (current->NeedsServerStartup())
         if (validator->StartMatlabServer(current) == false)
            throw FunctionException(wxT("Unable to start the server needed by the ") +
                                   (current->GetTypeName()) + wxT(" command"));
      
      current = current->GetNext();
   }
   
   // Get automatic global object list and check if they are used in the function
   // command sequence so that when any global object is declared in the main script
   // but not used in the function, they can be ignored during function local object
   // initialization. (LOJ: 2009.12.18)
   BuildUnusedGlobalObjectList();
   
   fcsFinalized = false;
   #ifdef DEBUG_FUNCTION_INIT
   MessageInterface::ShowMessage
      (wxT("GmatFunction::Initialize() exiting for function '%s' with true\n"),
       functionName.c_str());
   #endif
   
   #ifdef DEBUG_TRACE
   ShowTrace(callCount, t1, wxT("GmatFunction::Initialize() exiting"), true);
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// bool GmatFunction::Execute(ObjectInitializer *objInit, bool reinitialize)
//------------------------------------------------------------------------------
bool GmatFunction::Execute(ObjectInitializer *objInit, bool reinitialize)
{
   if (!fcs) return false;
   if (!objInit) return false;
   
   #ifdef DEBUG_TRACE
   static Integer callCount = 0;
   callCount++;      
   clock_t t1 = clock();
   ShowTrace(callCount, t1, wxT("GmatFunction::Execute() entered"));
   #endif
   
   #ifdef DEBUG_FUNCTION_EXEC
   MessageInterface::ShowMessage
      (wxT("======================================================================\n")
       wxT("GmatFunction::Execute() entered for '%s'\n   internalCS is <%p>, ")
       wxT("reinitialize = %d\n"), functionName.c_str(), internalCoordSys, reinitialize);
   #endif
   
   GmatCommand *current = fcs;
   GmatCommand *last = NULL;
   
   // We want to initialize local objects with new object map,
   // so do it everytime (loj: 2008.09.26)
   // This causes to slow down function execution, so initialize if necessary
   if (reinitialize)
      objectsInitialized = false;
   
   // Reinitialize CoordinateSystem to fix bug 1599 (LOJ: 2009.11.05)
   // Reinitialize Parameters to fix bug 1519 (LOJ: 2009.09.16)
   if (objectsInitialized)
   {
      if (!objInit->InitializeObjects(true, Gmat::COORDINATE_SYSTEM))
         throw FunctionException
            (wxT("Failed to re-initialize Parameters in the \"") + functionName + wxT("\""));
      if (!objInit->InitializeObjects(true, Gmat::PARAMETER))
         throw FunctionException
            (wxT("Failed to re-initialize Parameters in the \"") + functionName + wxT("\""));
   }
   
   // Go through each command in the sequence and execute.
   // Once it gets to a real command, initialize local and automatic objects.
   while (current)
   {
      // Call to IsNextAFunction is necessary for branch commands in particular
      #ifdef DEBUG_FUNCTION_EXEC
      MessageInterface::ShowMessage
         (wxT("......Function executing <%p><%s> [%s]\n"), current, current->GetTypeName().c_str(),
          current->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
      MessageInterface::ShowMessage(wxT("      objectsInitialized=%d\n"), objectsInitialized);
      #endif
      
      last = current;
      
      if (!objectsInitialized)
      {
         // Since we don't know where actual mission sequence starts, just check
         // for command that is not NoOp, Create, Global, and GMAT with equation.
         // Can we have simple command indicating beginning of the sequence,
         // such as BeginSequence? (loj: 2008.06.19)
         // @todo: Now we have BeginMissionSequence, but not all functions have it,
         // so check it first otherwise do in the old way. (loj: 2010.07.16)
         Function *func = current->GetCurrentFunction();
         bool isEquation = false;
         wxString cmdType = current->GetTypeName();
         if (func && cmdType == wxT("GMAT"))
            if (((Assignment*)current)->GetMathTree() != NULL)
               isEquation = true;
         
         if (cmdType != wxT("NoOp") && cmdType != wxT("Create") && cmdType != wxT("Global"))
         {
            bool beginInit = true;            
            if (cmdType == wxT("GMAT") && !isEquation)
               beginInit = false;

            if (cmdType == wxT("BeginMissionSequence") || cmdType == wxT("BeginScript"))
               beginInit = true;
            
            if (beginInit)
            {
               objectsInitialized = true;
               validator->HandleCcsdsEphemerisFile(objectStore, true);
               #ifdef DEBUG_FUNCTION_EXEC
               MessageInterface::ShowMessage
                  (wxT("============================ Initializing LocalObjects at current\n")
                   wxT("%s\n"), current->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
               #endif
               InitializeLocalObjects(objInit, current, true);
            }
         }
      }
      
      // Now execute the function sequence
      try
      {
         #ifdef DEBUG_FUNCTION_EXEC
         MessageInterface::ShowMessage
            (wxT("Now calling <%p>[%s]->Execute()\n"), current->GetTypeName().c_str(),
             current->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
         #endif
         
         if (!(current->Execute()))
            return false;
      }
      catch (BaseException &e)
      {
         // If it is user interrupt, rethrow (loj: 2008.10.16)
         // How can we tell if it is thrown by Stop command?
         // For now just find the phrase wxT("interrupted by Stop command")
         wxString msg = e.GetFullMessage();
         if (msg.find(wxT("interrupted by Stop command")) != msg.npos)
         {
            #ifdef DEBUG_FUNCTION_EXEC
            MessageInterface::ShowMessage
               (wxT("*** Interrupted by Stop commaned, so re-throwing...\n"));
            #endif
            throw;
         }
         
         if (e.IsFatal())
         {
            #ifdef DEBUG_FUNCTION_EXEC
            MessageInterface::ShowMessage
               (wxT("*** The exception is fatal, so re-throwing...\n"));
            #endif
            // Add command line to error message (LOJ: 2010.04.13)
            throw FunctionException
               (wxT("In ") + current->GetGeneratingString(Gmat::NO_COMMENTS) + wxT(", ") +
                e.GetFullMessage());
            //throw;
         }
         
         // Let's try initialzing local objects here again (2008.10.14)
         try
         {
            #ifdef DEBUG_FUNCTION_EXEC
            MessageInterface::ShowMessage
               (wxT("============================ Reinitializing LocalObjects at current\n")
                wxT("%s\n"), current->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
            #endif
            
            InitializeLocalObjects(objInit, current, false);
            
            #ifdef DEBUG_FUNCTION_EXEC
            MessageInterface::ShowMessage
               (wxT("......Function re-executing <%p><%s> [%s]\n"), current,
                current->GetTypeName().c_str(),
                current->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
            #endif
            
            if (!(current->Execute()))
               return false;
         }
         catch (HardwareException &he)
         {
            // Ignore for hardware exception since spacecraft is associated with Thruster
            // but Thruster binds with Tank later in the fcs
         }
         catch (BaseException &be)
         {
            throw FunctionException
               (wxT("During initialization of local objects before \"") +
                current->GetGeneratingString(Gmat::NO_COMMENTS) + wxT("\", ") +
                e.GetFullMessage());
         }
      }
      
      // If current command is BranchCommand and still executing, continue to next
      // command in the branch (LOJ: 2009.03.24)
      if (current->IsOfType(wxT("BranchCommand")) && current->IsExecuting())
      {
         #ifdef DEBUG_FUNCTION_EXEC
         MessageInterface::ShowMessage
            (wxT("In Function '%s', still executing current command is <%p><%s>\n"),
             functionName.c_str(), current, current ? current->GetTypeName().c_str() : wxT("NULL"));
         #endif
         
         continue;
      }
      
      current = current->GetNext();
      
      #ifdef DEBUG_FUNCTION_EXEC
      MessageInterface::ShowMessage
         (wxT("In Function '%s', the next command is <%p><%s>\n"), functionName.c_str(),
          current, current ? current->GetTypeName().c_str() : wxT("NULL"));
      #endif
   }
   
   // Set ObjectMap from the last command to Validator in order to create
   // valid output wrappers (loj: 2008.11.12)
   validator->SetObjectMap(last->GetObjectMap());
   
   #ifdef DEBUG_FUNCTION_EXEC
   MessageInterface::ShowMessage
      (wxT("   Now about to create %d output wrapper(s) to set results, objectsInitialized=%d\n"),
       outputNames.size(), objectsInitialized);
   #endif
   
   // create output wrappers and put into map
   GmatBase *obj;
   wrappersToDelete.clear();
   for (unsigned int jj = 0; jj < outputNames.size(); jj++)
   {
      if (!(obj = FindObject(outputNames.at(jj))))
      {
         wxString errMsg = wxT("Function: Output \"") + outputNames.at(jj);
         errMsg += wxT(" not found for function \"") + functionName + wxT("\"");
         throw FunctionException(errMsg);
      }
      wxString outName = outputNames.at(jj);
      ElementWrapper *outWrapper =
         validator->CreateElementWrapper(outName, false, false);
      #ifdef DEBUG_MORE_MEMORY
      MessageInterface::ShowMessage
         (wxT("+++ GmatFunction::Execute() *outWrapper = validator->")
          wxT("CreateElementWrapper(%s), <%p> '%s'\n"), outName.c_str(), outWrapper,
          outWrapper->GetDescription().c_str());
      #endif
      
      outWrapper->SetRefObject(obj);
      
      // nested CallFunction crashes if old outWrappers are deleted here. (loj: 2008.11.24)
      // so collect here and delete when FunctionRunner completes.
      wrappersToDelete.push_back(outWrapper);         
      
      // Set new outWrapper
      outputArgMap[outName] = outWrapper;
      #ifdef DEBUG_FUNCTION_EXEC // --------------------------------------------------- debug ---
         MessageInterface::ShowMessage(wxT("GmatFunction: Output wrapper created for %s\n"),
                                       (outputNames.at(jj)).c_str());
      #endif // -------------------------------------------------------------- end debug ---
   }
   
   #ifdef DEBUG_FUNCTION_EXEC
   MessageInterface::ShowMessage
      (wxT("GmatFunction::Execute() exiting true for '%s'\n"), functionName.c_str());
   #endif
   
   #ifdef DEBUG_TRACE
   ShowTrace(callCount, t1, wxT("GmatFunction::Execute() exiting"), true);
   #endif
   
   return true; 
}


//------------------------------------------------------------------------------
// void Finalize()
//------------------------------------------------------------------------------
void GmatFunction::Finalize()
{
   #ifdef DEBUG_TRACE
   static Integer callCount = 0;
   callCount++;      
   clock_t t1 = clock();
   ShowTrace(callCount, t1, wxT("GmatFunction::Finalize() entered"));
   #endif
   
   #ifdef DEBUG_FUNCTION_FINALIZE
   MessageInterface::ShowMessage
      (wxT("======================================================================\n")
       wxT("GmatFunction::Finalize() entered for '%s', FCS %s\n"),
       functionName.c_str(), fcsFinalized ? wxT("already finalized, so skp fcs") :
       wxT("NOT finalized, so call fcs->RunComplete"));
   #endif
   
   // Call RunComplete on each command in fcs
   if (!fcsFinalized)
   {
      fcsFinalized = true;
      GmatCommand *current = fcs;
      while (current)
      {
         #ifdef DEBUG_FUNCTION_FINALIZE
            if (!current)
               MessageInterface::ShowMessage
                  (wxT("   GmatFunction:Finalize() Current is NULL!!!\n"));
            else
               MessageInterface::ShowMessage
                  (wxT("   GmatFunction:Finalize() Now about to finalize ")
                   wxT("(call RunComplete on) command %s\n"),
                   (current->GetTypeName()).c_str());
         #endif
         current->RunComplete();
         current = current->GetNext();
      }
   }
   
   Function::Finalize();
   
   #ifdef DEBUG_FUNCTION_FINALIZE
   MessageInterface::ShowMessage(wxT("GmatFunction::Finalize() leaving\n"));
   #endif
   
   #ifdef DEBUG_TRACE
   ShowTrace(callCount, t1, wxT("GmatFunction::Finalize() exiting"), true, true);
   #endif
   
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Clone of the GmatFunction.
 *
 * @return clone of the GmatFunction.
 *
 */
//------------------------------------------------------------------------------
GmatBase* GmatFunction::Clone() const
{
   return (new GmatFunction(*this));
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
void GmatFunction::Copy(const GmatBase* orig)
{
   operator=(*((GmatFunction *)(orig)));
}


//------------------------------------------------------------------------------
//  bool SetStringParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * Sets the value for a wxString parameter.
 * 
 * @param <id> Integer ID of the parameter.
 * @param <value> New value for the parameter.
 * 
 * @return If value of the parameter was set.
 */
//------------------------------------------------------------------------------
bool GmatFunction::SetStringParameter(const Integer id, const wxString &value)
{
   #ifdef DEBUG_FUNCTION_SET
   MessageInterface::ShowMessage
      (wxT("GmatFunction::SetStringParameter() entered, id=%d, value=%s\n"), id, value.c_str());
   #endif
   
   switch (id)
   {
   case FUNCTION_PATH:
      {
         FileManager *fm = FileManager::Instance();
         
         // Compose full path if it has relative path.
         // Assuming if first char has '.', it has relative path.
         wxString temp = GmatStringUtil::Trim(value);
         if (temp[0] == wxT('.'))
         {
            wxString currPath = fm->GetCurrentPath();
            
            #ifdef DEBUG_FUNCTION_SET
            MessageInterface::ShowMessage(wxT("   currPath=%s\n"), currPath.c_str());
            #endif
            
            functionPath = currPath + temp.substr(1);
         }
         else
         {
            functionPath = value;
         }
         
         // Add to GmatFunction path
         fm->AddGmatFunctionPath(functionPath);
         
         // Remove path
         functionName = GmatFileUtil::ParseFileName(functionPath);
         
         // Remove .gmf if GmatGmatFunction
         wxString::size_type dotIndex = functionName.find(wxT(".gmf"));
         functionName = functionName.substr(0, dotIndex);
         
         #ifdef DEBUG_FUNCTION_SET
         MessageInterface::ShowMessage
            (wxT("   functionPath=<%s>\n"), functionPath.c_str());
         MessageInterface::ShowMessage
            (wxT("   functionName=<%s>\n"), functionName.c_str());
         #endif
         
         return true;
      }
   case FUNCTION_NAME:
      {
         // Remove path if it has one
         functionName = GmatFileUtil::ParseFileName(functionPath);
         
         // Remove .gmf if GmatGmatFunction
         wxString::size_type dotIndex = functionName.find(wxT(".gmf"));
         functionName = functionName.substr(0, dotIndex);
         
         return true;
      }
   default:
      return Function::SetStringParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
bool GmatFunction::SetStringParameter(const wxString &label,
                                      const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// void ShowTrace(Integer count, Integer t1, const wxString &label = wxT(""),
//                bool showMemoryTracks = false, bool addEol = false)
//------------------------------------------------------------------------------
void GmatFunction::ShowTrace(Integer count, Integer t1, const wxString &label,
                             bool showMemoryTracks, bool addEol)
{
   // To locally control debug output
   bool showTrace = false;
   bool showTracks = true;
   
   showTracks = showTracks & showMemoryTracks;
   
   if (showTrace)
   {
      #ifdef DEBUG_TRACE
      clock_t t2 = clock();
      MessageInterface::ShowMessage
         (wxT("=== %s, '%s' Count = %d, elapsed time: %f sec\n"), label.c_str(),
          functionName.c_str(), count, (Real)(t2-t1)/CLOCKS_PER_SEC);
      #endif
   }
   
   if (showTracks)
   {
      #ifdef DEBUG_MEMORY
      StringArray tracks = MemoryTracker::Instance()->GetTracks(false, false);
      if (showTrace)
         MessageInterface::ShowMessage
            (wxT("    ==> There are %d memory tracks\n"), tracks.size());
      else
         MessageInterface::ShowMessage
            (wxT("=== There are %d memory tracks when %s, '%s'\n"), tracks.size(),
             label.c_str(), functionName.c_str());
      
      if (addEol)
         MessageInterface::ShowMessage(wxT("\n"));
      #endif
   }
}


//------------------------------------------------------------------------------
// bool InitializeLocalObjects(ObjectInitializer *objInit,
//                             GmatCommand *current, bool ignoreException)
//------------------------------------------------------------------------------
bool GmatFunction::InitializeLocalObjects(ObjectInitializer *objInit,
                                          GmatCommand *current,
                                          bool ignoreException)
{
   #ifdef DEBUG_FUNCTION_EXEC
   MessageInterface::ShowMessage
      (wxT("\n============================ Begin initialization of local objects in '%s'\n"),
       functionName.c_str());
   MessageInterface::ShowMessage
      (wxT("Now at command \"%s\"\n"),
       current->GetGeneratingString(Gmat::NO_COMMENTS).c_str());
   #endif
   
   // Why internal coordinate system is empty in ObjectInitializer?
   // Set internal coordinate system (added (loj: 2008.10.07)
   objInit->SetInternalCoordinateSystem(internalCoordSys);
   
   // Let's try initializing local objects using ObjectInitializer (2008.06.19)
   // We need to add subscribers to publisher, so pass true
   try
   {
      if (!objInit->InitializeObjects(true, Gmat::UNKNOWN_OBJECT,
                                      unusedGlobalObjectList))
         // Should we throw an exception instead?
         return false;
   }
   catch (BaseException &e)
   {
      // We need to ignore exception thrown for the case Object is
      // created after it is used, such as
      // GMAT DefaultOpenGL.ViewPointReference = EarthSunL1;
      // Create LibrationPoint EarthSunL1;
      if (!ignoreException || (ignoreException && e.IsFatal()))
      {
         #ifdef DEBUG_FUNCTION_EXEC
         MessageInterface::ShowMessage
            (wxT("objInit->InitializeObjects() threw a fatal exception:\n'%s'\n")
             wxT("   So rethrow...\n"), e.GetFullMessage().c_str());
         #endif
         throw;
      }
      else
      {
         #ifdef DEBUG_FUNCTION_EXEC
         MessageInterface::ShowMessage
            (wxT("objInit->InitializeObjects() threw an exception:\n'%s'\n")
             wxT("   So ignoring...\n"), e.GetFullMessage().c_str());
         #endif
      }
   }
   
   #ifdef DEBUG_FUNCTION_EXEC
   MessageInterface::ShowMessage
      (wxT("============================ End   initialization of local objects\n"));
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// void BuildUnusedGlobalObjectList()
//------------------------------------------------------------------------------
/*
 * Builds unused global object list which is used in the ObjectInitializer
 * for ignoring undefined ref objects. For now it adds automatic global
 * CoordinateSystem if it's ref objects is Spacecraft and it is not used in
 * the function sequence. Since Spacecraft is not an automatic object it is
 * not automatically added to GOS.
 */
//------------------------------------------------------------------------------
void GmatFunction::BuildUnusedGlobalObjectList()
{
   #ifdef DEBUG_UNUSED_GOL
   MessageInterface::ShowMessage
      (wxT("BuildUnusedGlobalObjectList() entered. There are %d global objects\n"),
       globalObjectStore->size());
   #endif
   
   if (unusedGlobalObjectList != NULL)
      delete unusedGlobalObjectList;
   
   unusedGlobalObjectList = new StringArray;
   
   // Check global object store
   wxString cmdUsed;
   std::map<wxString, GmatBase *>::iterator omi;
   for (omi = globalObjectStore->begin(); omi != globalObjectStore->end(); ++omi)
   {
      GmatBase *obj = omi->second;
      if (!GmatCommandUtil::FindObject(fcs, (omi->second)->GetType(), omi->first,
                                       cmdUsed))
      {
         // Add unused global CoordinateSystem with Spacecraft origin,  primary,
         // or secondary, since Spacecraft is not an automatic global object and
         // we don't want to throw an exception for unexisting Spacecraft in the GOS.
         if (obj->IsOfType(Gmat::COORDINATE_SYSTEM))
         {
            GmatBase *origin = obj->GetRefObject(Gmat::SPACE_POINT, wxT("_GFOrigin_"));
            GmatBase *primary = obj->GetRefObject(Gmat::SPACE_POINT, wxT("_GFPrimary_"));
            GmatBase *secondary = obj->GetRefObject(Gmat::SPACE_POINT, wxT("_GFSecondary_"));
            
            if ((origin != NULL && origin->IsOfType(Gmat::SPACECRAFT)) ||
                (primary != NULL && primary->IsOfType(Gmat::SPACECRAFT)) ||
                (secondary != NULL && secondary->IsOfType(Gmat::SPACECRAFT)))
            {
               #ifdef DEBUG_UNUSED_GOL
               MessageInterface::ShowMessage
                  (wxT("==> Adding '%s' to unusedGOL\n"), (omi->first).c_str());
               #endif
               unusedGlobalObjectList->push_back(omi->first);
            }
         }
      }
   }
   #ifdef DEBUG_UNUSED_GOL
   MessageInterface::ShowMessage
      (wxT("BuildUnusedGlobalObjectList() leaving, There are %d unused global objects\n"),
       unusedGlobalObjectList->size());
   #endif
}
