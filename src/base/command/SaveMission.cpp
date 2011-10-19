//$Id: SaveMission.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  SaveMission
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Linda Jun (NASA/GSFC)
// Created: 2010/08/03
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
/**
 * Class implementation for the SaveMission command
 */
//------------------------------------------------------------------------------

#include "SaveMission.hpp"
#include "CommandUtil.hpp"         // for GetLastCommand()
#include "FileManager.hpp"         // for GetPathname()
#include "FileUtil.hpp"            // for ParseFileExtension()
#include "StringUtil.hpp"          // for RemoveEnclosingString()
#include "Moderator.hpp"           // for GetScript()
#include "MessageInterface.hpp"    // for ShowMessage()
#include <fstream>                 // for fopen(), <<

//#define DEBUG_SAVEMISSION_IA
//#define DEBUG_SAVEMISSION_EXE

//---------------------------------
//  static data
//---------------------------------
const wxString
SaveMission::PARAMETER_TEXT[SaveMissionParamCount - GmatCommandParamCount] = 
{
   wxT("FileName"),
};

const Gmat::ParameterType
SaveMission::PARAMETER_TYPE[SaveMissionParamCount - GmatCommandParamCount] =
{
   Gmat::STRING_TYPE,   // "FileName",
};


//------------------------------------------------------------------------------
//  SaveMission()
//------------------------------------------------------------------------------
/**
 * Constructs the SaveMission command (default constructor).
 */
//------------------------------------------------------------------------------
SaveMission::SaveMission() :
    GmatCommand(wxT("SaveMission"))
{
}


//------------------------------------------------------------------------------
//  ~SaveMission()
//------------------------------------------------------------------------------
/**
 * Destroys the SaveMission command (default constructor).
 */
//------------------------------------------------------------------------------
SaveMission::~SaveMission()
{
}


//------------------------------------------------------------------------------
//  SaveMission(const SaveMission& noop)
//------------------------------------------------------------------------------
/**
 * Makes a copy of the SaveMission command (copy constructor).
 *
 * @param noop The original used to set parameters for this one.
 */
//------------------------------------------------------------------------------
SaveMission::SaveMission(const SaveMission& noop) :
    GmatCommand (noop)
{
}


//------------------------------------------------------------------------------
//  SaveMission& operator=(const SaveMission&)
//------------------------------------------------------------------------------
/**
 * Sets this SaveMission to match another one (assignment operator).
 * 
 * @param noop The original used to set parameters for this one.
 *
 * @return this instance.
 */
//------------------------------------------------------------------------------
SaveMission& SaveMission::operator=(const SaveMission& noop)
{
   return *this;
}


//------------------------------------------------------------------------------
//  bool Execute()
//------------------------------------------------------------------------------
/**
 * Executes the SaveMission command (copy constructor).
 * 
 * Like the name implies, SaveMission is a null operation -- nothing is done in this 
 * command.  It functions as a place holder, and as the starting command in the 
 * command sequence managed by the Moderator.
 *
 * @return true always.
 */
//------------------------------------------------------------------------------
bool SaveMission::Execute()
{
   // If this command is not in a function and is the last command, write
   // whole mission to a file
   if (currentFunction == NULL && GmatCommandUtil::GetLastCommand(this) == this)
   {
      FileManager *fm = FileManager::Instance();
      wxString outPath = fm->GetAbsPathname(FileManager::OUTPUT_PATH);
      wxString fname = fileName;
      
      // add output path if there is no path
      if (fileName.find(wxT("/")) == fileName.npos &&
          fileName.find(wxT("\\")) == fileName.npos)
      {
         fname = outPath + fileName;
      }
      
      // If file name has no extension, add .script
      if (GmatFileUtil::ParseFileExtension(fname) == wxT(""))
         fname = fname + wxT(".script");
      
      // Call Moderator::GetScript();
      wxString script = Moderator::Instance()->GetScript();
      
      #ifdef DEBUG_SAVEMISSION_EXE
      MessageInterface::ShowMessage
         (wxT("SaveMission::Execute() '%s' should save mission to a file\n"),
          GetGeneratingString(Gmat::NO_COMMENTS).c_str());
      MessageInterface::ShowMessage(wxT("   fileName='%s'\n"), fname.c_str());
      //MessageInterface::ShowMessage(script);
      #endif
      
      std::ofstream dstream;
      dstream.open(fname.char_str());
      dstream << script;
   }
   
   BuildCommandSummary(true);
   return true;
}


//------------------------------------------------------------------------------
//  void InterpretAction()
//------------------------------------------------------------------------------
/**
 * Parses the command string and builds the corresponding command structures.
 *
 * The SaveMission command has the following syntax:
 *
 *     SaveMission 'file name'
 */
//------------------------------------------------------------------------------
bool SaveMission::InterpretAction()
{
   #ifdef DEBUG_SAVEMISSION_IA
   MessageInterface::ShowMessage
      (wxT("SaveMission::InterpretAction(), generatingString = %s\n"),
       generatingString.c_str());
   #endif
   
   StringArray chunks = InterpretPreface();
   
   if (chunks.size() < 2)
      throw CommandException(wxT("Missing information for MissionSave command.\n"));
   
   fileName = chunks[1];
   
   // Remove single quotes
   fileName = GmatStringUtil::RemoveEnclosingString(chunks[1], wxT("'"));
   
   return true;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the SaveMission.
 *
 * @return clone of the SaveMission.
 */
//------------------------------------------------------------------------------
GmatBase* SaveMission::Clone() const
{
   return (new SaveMission(*this));
}


//------------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
bool SaveMission::RenameRefObject(const Gmat::ObjectType type,
                                  const wxString &oldName,
                                  const wxString &newName)
{
   // There are no objects to be renamed here
   return true;
}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
wxString SaveMission::GetParameterText(const Integer id) const
{
   if (id >= GmatCommandParamCount && id < SaveMissionParamCount)
      return PARAMETER_TEXT[id - GmatCommandParamCount];
   else
      return GmatCommand::GetParameterText(id);
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
Integer SaveMission::GetParameterID(const wxString &str) const
{
   for (int i=GmatCommandParamCount; i<SaveMissionParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatCommandParamCount])
         return i;
   }
   
   return GmatCommand::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
Gmat::ParameterType SaveMission::GetParameterType(const Integer id) const
{
   if (id >= GmatCommandParamCount && id < SaveMissionParamCount)
      return PARAMETER_TYPE[id - GmatCommandParamCount];
   else
      return GmatCommand::GetParameterType(id);
}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
wxString SaveMission::GetParameterTypeString(const Integer id) const
{
   if (id >= GmatCommandParamCount && id < SaveMissionParamCount)
      return GmatBase::PARAM_TYPE_STRING[GetParameterType(id)];
   else
      return GmatCommand::GetParameterTypeString(id);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
wxString SaveMission::GetStringParameter(const Integer id) const
{
   switch (id)
   {
   case FILE_NAME:
      return fileName;
   default:
      return GmatCommand::GetStringParameter(id);
   }
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString SaveMission::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString value)
//------------------------------------------------------------------------------
bool SaveMission::SetStringParameter(const Integer id, const wxString &value)
{
   switch (id)
   {
   case FILE_NAME:
      fileName = value;
      return true;
   default:
      return GmatCommand::SetStringParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
bool SaveMission::SetStringParameter(const wxString &label, const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// const wxString& GetGeneratingString(Gmat::WriteMode mode = Gmat::SCRIPTING,
//                    const wxString &prefix = "",
//                    const wxString &useName = "");
//------------------------------------------------------------------------------
const wxString& SaveMission::GetGeneratingString(Gmat::WriteMode mode,
                                                    const wxString &prefix,
                                                    const wxString &useName)
{
   // Build the local string
   generatingString = prefix + wxT("SaveMission");
   generatingString += wxT(" '") + fileName + wxT("';");
   
   // Then call the base class method for comments
   return GmatCommand::GetGeneratingString(mode, prefix, useName);
}

