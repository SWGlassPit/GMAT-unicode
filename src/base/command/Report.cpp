//$Id: Report.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                            Report
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under MOMS Purchase
// order MOMS418823
//
// Author: Darrel Conway, Thinking Systems, Inc.
// Created: 2005/07/06
//
/**
 *  Class implementation for the Report command.
 */
//------------------------------------------------------------------------------


#include "Report.hpp"
#include "MessageInterface.hpp"
#include "StringUtil.hpp"       // for GetArrayIndex()
#include <sstream>

//#define DEBUG_REPORT_OBJ
//#define DEBUG_REPORT_SET
//#define DEBUG_REPORT_INIT
//#define DEBUG_REPORT_EXEC
//#define DEBUG_WRAPPER_CODE
//#define DEBUG_OBJECT_MAP
//#define DEBUG_RENAME

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//---------------------------------
// static data
//---------------------------------
const wxString
Report::PARAMETER_TEXT[ReportParamCount - GmatCommandParamCount] =
{
   wxT("ReportFile"),
   wxT("Add"),
};

const Gmat::ParameterType
Report::PARAMETER_TYPE[ReportParamCount - GmatCommandParamCount] =
{
   Gmat::STRING_TYPE,        // "ReportFile",
   Gmat::OBJECTARRAY_TYPE,   // "Add",
};


//---------------------------------
// public members
//---------------------------------

//------------------------------------------------------------------------------
//  Report()
//------------------------------------------------------------------------------
/**
 * Constructs the Report Command (default constructor).
 */
//------------------------------------------------------------------------------
Report::Report() :
   GmatCommand  (wxT("Report")),
   rfName       (wxT("")),
   reporter     (NULL),
   reportID     (-1),
   numParams    (0),
   needsHeaders (true)
{
   // GmatBase data
   objectTypeNames.push_back(wxT("Report"));
}


//------------------------------------------------------------------------------
//  ~Report()
//------------------------------------------------------------------------------
/**
 * Destroys the Report Command (destructor).
 */
//------------------------------------------------------------------------------
Report::~Report()
{
   DeleteParameters();
}


//------------------------------------------------------------------------------
//  Report(const Report &rep)
//------------------------------------------------------------------------------
/**
 * Constructs the Report Command based on another instance (copy constructor).
 * 
 * @param rep The Report that is copied.
 */
//------------------------------------------------------------------------------
Report::Report(const Report &rep) :
   GmatCommand    (rep),
   rfName         (rep.rfName),
   reporter       (NULL),
   reportID       (-1),
   needsHeaders   (rep.needsHeaders)
{
   parmNames = rep.parmNames;
   actualParmNames = rep.actualParmNames;
   parms.clear();
   parmRows.clear();
   parmCols.clear();
   parmWrappers.clear();
}


//------------------------------------------------------------------------------
//  Report& operator=(const Report &rep)
//------------------------------------------------------------------------------
/**
 * Sets this Report Command to match another instance (Assignment operator).
 * 
 * @param rep The Report that is copied.
 * 
 * @return This instance, configured to match the other and ready for 
 *         initialization.
 */
//------------------------------------------------------------------------------
Report& Report::operator=(const Report &rep)
{
   if (this != &rep)
   {
      rfName = rep.rfName;
      reporter = NULL;
      reportID = -1;

      parmNames = rep.parmNames;
      actualParmNames = rep.actualParmNames;
      parms.clear();
      parmRows.clear();
      parmCols.clear();
      needsHeaders = rep.needsHeaders;
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
Integer Report::GetParameterID(const wxString &str) const
{
   for (Integer i = GmatCommandParamCount; i < ReportParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatCommandParamCount])
         return i;
   }
   
   return GmatCommand::GetParameterID(str);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
wxString Report::GetStringParameter(const Integer id) const
{
   if (id == REPORTFILE)
      return rfName;
   
   return GmatCommand::GetStringParameter(id);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString Report::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
bool Report::SetStringParameter(const Integer id, const wxString &value)
{
   switch (id)
   {
   case REPORTFILE:
      rfName = value;
      return true;
   case ADD:
      #ifdef DEBUG_REPORT_SET
      MessageInterface::ShowMessage
         (wxT("Report::SetStringParameter() Adding parameter '%s'\n"), value.c_str());
      #endif
      return AddParameter(value, numParams);
   default:
      return GmatCommand::SetStringParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label,
//                         const wxString &value)
//------------------------------------------------------------------------------
bool Report::SetStringParameter(const wxString &label,
                                const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// virtual bool SetStringParameter(const Integer id, const wxString &value,
//                                 const Integer index)
//------------------------------------------------------------------------------
bool Report::SetStringParameter(const Integer id, const wxString &value,
                                const Integer index)
{
   switch (id)
   {
   case ADD:
      return AddParameter(value, index);
   default:
      return GmatCommand::SetStringParameter(id, value, index);
   }
}


//------------------------------------------------------------------------------
// virtual bool SetStringParameter(const wxString &label,
//                                 const wxString &value,
//                                 const Integer index)
//------------------------------------------------------------------------------
bool Report::SetStringParameter(const wxString &label,
                                const wxString &value,
                                const Integer index)
{
   return SetStringParameter(GetParameterID(label), value, index);
}


//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const Integer id) const
//------------------------------------------------------------------------------
const StringArray& Report::GetStringArrayParameter(const Integer id) const
{
   #ifdef DEBUG_REPORTFILE_GET
   MessageInterface::ShowMessage
      (wxT("Report::GetStringArrayParameter() id=%d, actualParmNames.size()=%d, ")
       wxT("numParams=%d\n"), id, actualParmNames.size(), numParams);
   #endif
   
   switch (id)
   {
   case ADD:
      return actualParmNames;
   default:
      return GmatCommand::GetStringArrayParameter(id);
   }
}


//------------------------------------------------------------------------------
// StringArray& GetStringArrayParameter(const wxString &label) const
//------------------------------------------------------------------------------
const StringArray& Report::GetStringArrayParameter(const wxString &label) const
{
   return GetStringArrayParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// const StringArray& GetWrapperObjectNameArray()
//------------------------------------------------------------------------------
const StringArray& Report::GetWrapperObjectNameArray()
{
   wrapperObjectNames.clear();
   wrapperObjectNames.insert(wrapperObjectNames.end(), actualParmNames.begin(),
                             actualParmNames.end());
   return wrapperObjectNames;
}


//------------------------------------------------------------------------------
// bool SetElementWrapper(ElementWrapper *toWrapper, const wxString &withName)
//------------------------------------------------------------------------------
bool Report::SetElementWrapper(ElementWrapper *toWrapper,
                               const wxString &withName)
{
   #ifdef DEBUG_WRAPPER_CODE   
   MessageInterface::ShowMessage
      (wxT("Report::SetElementWrapper() this=<%p> '%s' entered, toWrapper=<%p>, ")
       wxT("withName='%s'\n"), this, GetGeneratingString(Gmat::NO_COMMENTS).c_str(),
       toWrapper, withName.c_str());
   #endif
   
   if (toWrapper == NULL)
      return false;
   
   // Do we need any type checking?
   // CheckDataType(toWrapper, Gmat::REAL_TYPE, "Report", true);
   
   bool retval = false;
   ElementWrapper *ew;
   std::vector<ElementWrapper*> wrappersToDelete;
   
   //-------------------------------------------------------
   // check parameter names
   //-------------------------------------------------------
   #ifdef DEBUG_WRAPPER_CODE   
   MessageInterface::ShowMessage
      (wxT("   Checking %d Parameters\n"), actualParmNames.size());
   for (UnsignedInt i=0; i<actualParmNames.size(); i++)
      MessageInterface::ShowMessage(wxT("      %s\n"), actualParmNames[i].c_str());
   #endif
   
   Integer sz = actualParmNames.size();
   for (Integer i = 0; i < sz; i++)
   {
      if (actualParmNames.at(i) == withName)
      {
         #ifdef DEBUG_WRAPPER_CODE   
         MessageInterface::ShowMessage
            (wxT("   Found wrapper name \"%s\" in actualParmNames\n"), withName.c_str());
         #endif
         if (parmWrappers.at(i) != NULL)
         {
            ew = parmWrappers.at(i);
            parmWrappers.at(i) = toWrapper;
            // if wrapper not found, add to the list to delete
            if (find(wrappersToDelete.begin(), wrappersToDelete.end(), ew) ==
                wrappersToDelete.end())
               wrappersToDelete.push_back(ew);
         }
         else
         {
            parmWrappers.at(i) = toWrapper;
         }
         
         retval = true;
      }
   }
   
   #ifdef DEBUG_WRAPPER_CODE   
   MessageInterface::ShowMessage
      (wxT("   There are %d wrappers to delete\n"), wrappersToDelete.size());
   #endif
   
   // Delete old ElementWrappers (loj: 2008.11.20)
   for (std::vector<ElementWrapper*>::iterator ewi = wrappersToDelete.begin();
        ewi < wrappersToDelete.end(); ewi++)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         ((*ewi), (*ewi)->GetDescription(), wxT("Report::SetElementWrapper()"),
          GetGeneratingString(Gmat::NO_COMMENTS) + wxT(" deleting wrapper"));
      #endif
      delete (*ewi);
      (*ewi) = NULL;
   }
   
   #ifdef DEBUG_WRAPPER_CODE   
   MessageInterface::ShowMessage
      (wxT("Report::SetElementWrapper() exiting with %d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// void ClearWrappers()
//------------------------------------------------------------------------------
void Report::ClearWrappers()
{
   #ifdef DEBUG_WRAPPER_CODE   
   MessageInterface::ShowMessage
      (wxT("Report::ClearWrappers() this=<%p> '%s' entered\n   There are %d wrappers ")
       wxT("allocated, these will be deleted if not NULL\n"), this,
       GetGeneratingString(Gmat::NO_COMMENTS).c_str(), parmWrappers.size());
   #endif
   
   std::vector<ElementWrapper*> wrappersToDelete;
   
   // delete wrappers (loj: 2008.11.20)
   for (std::vector<ElementWrapper*>::iterator ewi = parmWrappers.begin();
        ewi < parmWrappers.end(); ewi++)
   {
      if ((*ewi) == NULL)
         continue;
      
      // if wrapper not found, add to the list to delete
      if (find(wrappersToDelete.begin(), wrappersToDelete.end(), (*ewi)) ==
          wrappersToDelete.end())
         wrappersToDelete.push_back((*ewi));
   }
   
   #ifdef DEBUG_WRAPPER_CODE   
   MessageInterface::ShowMessage
      (wxT("   There are %d wrappers to delete\n"), wrappersToDelete.size());
   #endif
}


//------------------------------------------------------------------------------
// virtual bool TakeAction(const wxString &action,  
//                         const wxString &actionData = "");
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
bool Report::TakeAction(const wxString &action, const wxString &actionData)
{
   #if DEBUG_TAKE_ACTION
   MessageInterface::ShowMessage
      (wxT("Report::TakeAction() action=%s, actionData=%s\n"),
       action.c_str(), actionData.c_str());
   #endif
   
   if (action == wxT("Clear"))
   {
      parmNames.clear();
      actualParmNames.clear();
      parmRows.clear();
      parmCols.clear();

      // I think we also need to clear wrappers here (loj: 2008.11.24)
      ClearWrappers();
      parmWrappers.clear();      
   }
   
   return false;
}


//------------------------------------------------------------------------------
// bool GetRefObjectName(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Retrieves the reference object names.
 * 
 * @param type The type of the reference object.
 * 
 * @return the name of the object.
 */
//------------------------------------------------------------------------------
wxString Report::GetRefObjectName(const Gmat::ObjectType type) const
{
   switch (type)
   {
   case Gmat::SUBSCRIBER:
      return rfName;
      
   case Gmat::PARAMETER:
      if (parmNames.size() == 0)
         return wxT("");
      else
         return parmNames[0];
   default:
      return GmatCommand::GetRefObjectName(type);
   }
}


//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Accesses arrays of names for referenced objects.
 * 
 * @param type Type of object requested.
 * 
 * @return the StringArray containing the referenced object names.
 */
//------------------------------------------------------------------------------
const StringArray& Report::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   static StringArray refObjectNames;
   refObjectNames.clear();
   
   switch (type)
   {
   case Gmat::SUBSCRIBER:
      refObjectNames.push_back(rfName);
      return refObjectNames;
   case Gmat::PARAMETER:
      return parmNames;
   default:
      return GmatCommand::GetRefObjectNameArray(type);
   }
}


//------------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                    const wxString &name, const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets the ReportFile and Parameter objects used bt the Report command.
 * 
 * This method received the global instances of the objects used in the Report
 * command.  It checks their types and stores their names, so that the the 
 * objects can be retrieved from the local store in the Sandbox during 
 * initiialization.  It also tells the ReportFile instance that it will need
 * to be ready to receive data from a ReportCommand, so that the ReportFile does
 * not erroneously inform the user that no data will be written to the 
 * ReportFile.
 * 
 * @param <obj> Pointer to the reference object.
 * @param <type> type of the reference object.
 * @param <name> name of the reference object.
 * @param <index> Index into the object array.
 *
 * @return true if object successfully set, throws otherwise.
 */
//------------------------------------------------------------------------------
bool Report::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                          const wxString &name, const Integer index)
{
   #ifdef DEBUG_REPORT_OBJ
      MessageInterface::ShowMessage(
         wxT("Report::SetRefObject received a %s named '%s', index=%d\n"), 
         obj->GetTypeName().c_str(), obj->GetName().c_str(), index);
   #endif

   if (type == Gmat::SUBSCRIBER)
   {
      if (obj->GetTypeName() != wxT("ReportFile"))
         throw CommandException(wxT("Report command must have a ReportFile name ")
            wxT("as the first parameter.\n"));
      
      rfName = name;
      // Tell the ReportFile object that a command has requested its services
      obj->TakeAction(wxT("PassedToReport"));
      reporter = (ReportFile*)obj;
      
      // Why we need to clear? (loj: 2007.12.19 commented out)
      // We want to preserve parameters to report for ReportFile
      //reporter->TakeAction("Clear");
   }
   else if (type == Gmat::PARAMETER)
   {
      #ifdef DEBUG_REPORT_OBJ
         MessageInterface::ShowMessage(wxT("   Received %s as a Parameter\n"), name.c_str());
      #endif
         
      // All remaining refs should point to Parameter objects
      if (!obj->IsOfType(wxT("Parameter")))
         throw CommandException(wxT("Report command can only have Parameters ")
            wxT("in the list of reported values.\n"));
      
      AddParameter(name, index, (Parameter*)obj);
      
      #ifdef __SHOW_NAMES_IN_REPORTFILE__
      // For compare report column header
      if (reporter)
         reporter->AddParameterForTitleOnly(name);
      else
         throw CommandException(wxT("Report command has undefined ReportFile object.\n"));
      #endif
   }
   
   #ifdef DEBUG_REPORT_OBJ
   MessageInterface::ShowMessage(wxT("Report::SetRefObject() returning true\n"));
   #endif
   
   return true;
}


//---------------------------------------------------------------------------
// bool RenameRefObject(const Gmat::ObjectType type,
//                      const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
/*
 * Renames referenced objects
 *
 * @param <type> type of the reference object.
 * @param <oldName> old name of the reference object.
 * @param <newName> new name of the reference object.
 *
 * @return always true to indicate RenameRefObject() was implemented.
 */
//---------------------------------------------------------------------------
bool Report::RenameRefObject(const Gmat::ObjectType type,
                             const wxString &oldName,
                             const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("Report::RenameRefObject() type=%s, oldName=%s, newName=%s\n"),
       GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif
   
   if (type == Gmat::SUBSCRIBER)
   {
      if (rfName == oldName)
         rfName = newName;
   }
   else if (type == Gmat::PARAMETER)
   {
      for (UnsignedInt i=0; i<parmNames.size(); i++)
         if (parmNames[i] == oldName)
            parmNames[i] = newName;
      
      for (UnsignedInt i=0; i<actualParmNames.size(); i++)
         if (actualParmNames[i] == oldName)
            actualParmNames[i] = newName;
   }
   // Since parameter name is composed of spacecraftName.dep.paramType,
   // spacecraftName.hardwareName.paramType, or burnName.dep.paramType
   // check the type first
   else if (type == Gmat::SPACECRAFT || type == Gmat::BURN ||
            type == Gmat::COORDINATE_SYSTEM || type == Gmat::CALCULATED_POINT ||
            type == Gmat::HARDWARE)
   {
      for (UnsignedInt i=0; i<parmNames.size(); i++)
         if (parmNames[i].find(oldName) != wxString::npos)
            parmNames[i] = GmatStringUtil::Replace(parmNames[i], oldName, newName);
      
      for (UnsignedInt i=0; i<actualParmNames.size(); i++)
         if (actualParmNames[i].find(oldName) != wxString::npos)
            actualParmNames[i] =
               GmatStringUtil::Replace(actualParmNames[i], oldName, newName);
      
      // Go through wrappers
      for (WrapperArray::iterator i = parmWrappers.begin(); i < parmWrappers.end(); i++)
      {
         #ifdef DEBUG_RENAME
         MessageInterface::ShowMessage
            (wxT("   before rename, wrapper desc = '%s'\n"), (*i)->GetDescription().c_str());
         #endif
         
         (*i)->RenameObject(oldName, newName);
         
         #ifdef DEBUG_RENAME
         MessageInterface::ShowMessage
            (wxT("   after  rename, wrapper desc = '%s'\n"), (*i)->GetDescription().c_str());
         #endif
      }
      
      // Go through generating string
      generatingString = GmatStringUtil::Replace(generatingString, oldName, newName);
   }
   
   return true;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of this Report.
 *
 * @return clone of the Report.
 */
//------------------------------------------------------------------------------
GmatBase* Report::Clone() const
{
   return (new Report(*this));
}


//------------------------------------------------------------------------------
// const wxString& GetGeneratingString(Gmat::WriteMode mode,
//                                        const wxString &prefix,
//                                        const wxString &useName)
//------------------------------------------------------------------------------
const wxString& Report::GetGeneratingString(Gmat::WriteMode mode,
                                               const wxString &prefix,
                                               const wxString &useName)
{
   #ifdef DEBUG_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("Report::GetGeneratingString() rfName='%s', has %d parameters to write\n"),
       rfName.c_str(), actualParmNames.size());
   #endif
   
   wxString gen = prefix + wxT("Report ") + rfName + wxT(" ");
   UnsignedInt numElem = actualParmNames.size();
   
   if (numElem > 1)
   {
      for (UnsignedInt i=0; i<numElem-1; i++)
         gen += actualParmNames[i] + wxT(" ");
      
      gen += actualParmNames[numElem-1];
   }
   else if (numElem == 1)
   {
      gen += actualParmNames[0];
   }
   
   generatingString = gen + wxT(";");
   
   #ifdef DEBUG_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("   generatingString=<%s>, \n   now returning GmatCommand::GetGeneratingString()\n"),
       generatingString.c_str());
   #endif
   
   return GmatCommand::GetGeneratingString(mode, prefix, useName);
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 * Performs the initialization needed to run the Report command.
 *
 * @return true if the Report is initialized, false if an error occurs.
 */
//------------------------------------------------------------------------------
bool Report::Initialize()
{
   if (GmatCommand::Initialize() == false)
      return false;

   #ifdef DEBUG_REPORT_INIT
   MessageInterface::ShowMessage
      (wxT("Report::Initialize() entered, has %d parameter names\n"), parmNames.size());
   #endif
   #ifdef DEBUG_OBJECT_MAP
   ShowObjectMaps();
   #endif
   
   parms.clear();
   GmatBase *mapObj = NULL;
   
   if ((mapObj = FindObject(rfName)) == NULL)
      throw CommandException(
         wxT("Report command cannot find ReportFile named \"") + (rfName) +
         wxT("\"\n"));
   
   reporter = (ReportFile *)mapObj;
   if (reporter->GetTypeName() != wxT("ReportFile"))
      throw CommandException(
         wxT("Object named \"") + rfName +
         wxT("\" is not a ReportFile; Report command cannot execute\n"));
   
   // Tell the ReportFile object that a command has requested its services
   // Added this here so that ReportFile::Initialize() doesn't throw exception
   // when there is no paramters to report (loj: 2008.06.11)
   reporter->TakeAction(wxT("PassedToReport"));
   
   needsHeaders =
      reporter->GetOnOffParameter(reporter->GetParameterID(wxT("WriteHeaders"))) == wxT("On");
   
   for (StringArray::iterator i = parmNames.begin(); i != parmNames.end(); ++i)
   {
      #ifdef DEBUG_REPORT_INIT
      MessageInterface::ShowMessage(wxT("   Now find object for '%s'\n"), (*i).c_str());
      #endif
      
      mapObj = FindObject(*i);
      if (mapObj == NULL)
      {
         wxString msg = wxT("Object named \"") + (*i) +
            wxT("\" cannot be found for the Report command '") +
            GetGeneratingString(Gmat::NO_COMMENTS) + wxT("'");
         #ifdef DEBUG_REPORT_INIT
         MessageInterface::ShowMessage(wxT("**** ERROR **** %s\n"), msg.c_str());
         #endif
         //return false;
         throw CommandException(msg);
      }
      
      if (!mapObj->IsOfType(wxT("Parameter")))
         throw CommandException(wxT("Parameter type mismatch for ") + 
            mapObj->GetName());
      parms.push_back((Parameter *)mapObj);
   }
   
   // Set Wrapper references (LOJ: 2009.04.01)
   // We need this to use ReportFile::WriteData() in Execute()
   for (WrapperArray::iterator i = parmWrappers.begin(); i < parmWrappers.end(); i++)
   {
      #ifdef DEBUG_REPORT_INIT
      MessageInterface::ShowMessage
         (wxT("   wrapper desc = '%s'\n"), (*i)->GetDescription().c_str());
      #endif
      
      if (SetWrapperReferences(*(*i)) == false)
         return false;      
   }
   
   #ifdef DEBUG_REPORT_INIT
   MessageInterface::ShowMessage(wxT("Report::Initialize() returning true.\n"));
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// bool Execute()
//------------------------------------------------------------------------------
/**
 * Write the report data to a ReportFile.
 *
 * @return true if the Command runs to completion, false if an error
 *         occurs.
 */
//------------------------------------------------------------------------------
bool Report::Execute()
{
   if (parms.empty())
      throw CommandException(wxT("Report command has no parameters to write\n"));
   if (reporter == NULL)
      throw CommandException(wxT("Reporter is not yet set\n"));
   
   #ifdef DEBUG_REPORT_EXEC
   MessageInterface::ShowMessage
      (wxT("Report::Execute() this=<%p> '%s' entered, reporter <%s> '%s' has %d Parameters\n"),
       this, GetGeneratingString(Gmat::NO_COMMENTS).c_str(),
       reporter->GetName().c_str(), reporter->GetFileName().c_str(), parms.size());
   #endif
   
   // Build the data as a string
   wxString datastream;
   
   // Set the stream to use the settings in the ReportFile
   // Note that this is done here, rather than during initialization, in case
   // the user has changed the values during the run.
   Integer prec = reporter->GetIntegerParameter(reporter->GetParameterID(wxT("Precision")));
   
   bool leftJustify = false;
   if (reporter->GetOnOffParameter(reporter->GetParameterID(wxT("LeftJustify"))) == wxT("On"))
      leftJustify = true;
   
   bool zeroFill = false;
   if (reporter->GetOnOffParameter(reporter->GetParameterID(wxT("ZeroFill"))) == wxT("On"))
      zeroFill = true;
   
   int colWidth = reporter->GetIntegerParameter(reporter->GetParameterID(wxT("ColumnWidth")));
   
   
   if (needsHeaders &&
       reporter->GetOnOffParameter(reporter->GetParameterID(wxT("WriteHeaders"))) == wxT("On"))
      WriteHeaders(datastream, colWidth);
   
   
   // Write to report file using ReportFile::WriateData().
   // This method takes ElementWrapper array to write data to stream
   reporter->TakeAction(wxT("ActivateForReport"), wxT("On"));
   bool retval = reporter->WriteData(parmWrappers);
   reporter->TakeAction(wxT("ActivateForReport"), wxT("Off"));
   BuildCommandSummary(true);
   return retval;   
}


//------------------------------------------------------------------------------
//  void RunComplete()
//------------------------------------------------------------------------------
void Report::RunComplete()
{
   #ifdef DEBUG_RUN_COMPLETE
   MessageInterface::ShowMessage
      (wxT("Report::RunComplete() this=<%p> '%s' entered\n"), this,
       GetGeneratingString(Gmat::NO_COMMENTS).c_str());
   #endif
   
   GmatCommand::RunComplete();
}


//------------------------------------------------------------------------------
// void WriteHeaders(wxStringstream &datastream, Integer colWidth)
//------------------------------------------------------------------------------
void Report::WriteHeaders(wxString &datastream, Integer colWidth)
{
   reporter->TakeAction(wxT("ActivateForReport"), wxT("On"));
   for (StringArray::iterator i = actualParmNames.begin();
        i != actualParmNames.end(); ++i)
   {
      datastream << (*i);
      datastream << wxT("   ");
   }
   
   wxString header = datastream;
   reporter->ReceiveData(header, header.length());
   datastream = wxT("");
   needsHeaders = false;
}


//------------------------------------------------------------------------------
// bool AddParameter(const wxString &paramName, Integer index, Parameter *param)
//------------------------------------------------------------------------------
bool Report::AddParameter(const wxString &paramName, Integer index,
                          Parameter *param)
{
   #ifdef DEBUG_REPORT_SET
   MessageInterface::ShowMessage
      (wxT("Report::AddParameter() this=<%p>, Adding parameter '%s', index=%d, ")
       wxT("param=<%p>, numParams=%d\n"), this, paramName.c_str(), index, param, numParams);
   #endif
   
   if (paramName == wxT(""))
   {
      #ifdef DEBUG_REPORT_SET
      MessageInterface::ShowMessage
         (wxT("Report::AddParameter() returning false, input paramName is blank\n"));
      #endif
      return false;
   }
   
   if (index < 0)
   {
      #ifdef DEBUG_REPORT_SET
      MessageInterface::ShowMessage
         (wxT("Report::AddParameter() returning false, the index %d is less than 0\n"));
      #endif
      return false;
   }
   
   // Since numParam is incremented after adding to arrays, index range varies
   // depends on whether parameter pointer is NULL or not
   if ((param == NULL && index > numParams) ||
       (param != NULL && index >= numParams))
   {
      #ifdef DEBUG_REPORT_SET
      MessageInterface::ShowMessage
         (wxT("Report::AddParameter() returning false, the index %d is out of bounds, ")
          wxT("it must be between 0 and %d\n"), index, param ? numParams + 1 : numParams);
      #endif
      return false;
   }
   
   if (param != NULL)
   {
      #ifdef DEBUG_REPORT_SET
      MessageInterface::ShowMessage
         (wxT("   Set <%p>'%s' to index %d\n"), param, paramName.c_str(), index);
      #endif
      parms[index] = param;
   }
   //if (paramName != "" && index == numParams)
   else
   {
      #ifdef __NO_DUPLICATES__
      // if paramName not found, add
      if (find(actualParmNames.begin(), actualParmNames.end(), paramName) ==
          actualParmNames.end())
      {
      #endif
         // Handle Array indexing
         Integer row, col;
         wxString newName;      
         GmatStringUtil::GetArrayIndex(paramName, row, col, newName);
         
         parmNames.push_back(newName);
         actualParmNames.push_back(paramName);
         parmRows.push_back(row);
         parmCols.push_back(col);
         parms.push_back(param);
         parmWrappers.push_back(NULL);
         numParams = actualParmNames.size();
         
         #ifdef DEBUG_REPORT_SET
         MessageInterface::ShowMessage
            (wxT("   Added '%s', size=%d\n"), paramName.c_str(), numParams);
         #endif
         
         return true;
      
      #ifdef __NO_DUPLICATES__
      }
      #endif
   }
   
   return false;
}


//------------------------------------------------------------------------------
// void DeleteParameters()
//------------------------------------------------------------------------------
void Report::DeleteParameters()
{
   #ifdef DEBUG_WRAPPER_CODE   
   MessageInterface::ShowMessage
      (wxT("Report::DeleteParameters() this=<%p> '%s' entered\n   There are %d wrappers ")
       wxT("allocated, these will be deleted if not NULL\n"), this,
       GetGeneratingString(Gmat::NO_COMMENTS).c_str(), parmWrappers.size());
   #endif
   
   std::vector<ElementWrapper*> wrappersToDelete;
   
   // delete wrappers (loj: 2008.11.20)
   for (std::vector<ElementWrapper*>::iterator ewi = parmWrappers.begin();
        ewi < parmWrappers.end(); ewi++)
   {
      if ((*ewi) == NULL)
         continue;
      
      // if wrapper not found, add to the list to delete
      if (find(wrappersToDelete.begin(), wrappersToDelete.end(), (*ewi)) ==
          wrappersToDelete.end())
         wrappersToDelete.push_back((*ewi));
   }
   
   #ifdef DEBUG_WRAPPER_CODE   
   MessageInterface::ShowMessage
      (wxT("   There are %d wrappers to delete\n"), wrappersToDelete.size());
   #endif
   
   // delete wrappers (loj: 2008.11.20)
   for (std::vector<ElementWrapper*>::iterator ewi = wrappersToDelete.begin();
        ewi < wrappersToDelete.end(); ewi++)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         ((*ewi), (*ewi)->GetDescription(), wxT("Report::DeleteParameters()"),
          GetGeneratingString(Gmat::NO_COMMENTS) + wxT(" deleting wrapper"));
      #endif
      delete (*ewi);
      (*ewi) = NULL;
   }
   
   parmWrappers.clear();   
   actualParmNames.clear();
   parms.clear();
   parmRows.clear();
   parmCols.clear();
}


