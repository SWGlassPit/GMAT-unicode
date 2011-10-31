//$Id: BeginScript.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              BeginScript
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Darrel J. Conway
// Created: 2004/02/25
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under MOMS Task
// Order 124.
//
/**
 * Script tag used to indicate a block of script that shows up verbatim in a
 * ScriptEvent panel on the GUI.
 */
//------------------------------------------------------------------------------


#include "BeginScript.hpp" 
#include "TextParser.hpp"     // for DecomposeBlock()
#include "CommandUtil.hpp"    // for GetNextCommand()
#include <sstream>

#include "MessageInterface.hpp"

//#define DEBUG_BEGINSCRIPT
//#define DBGLVL_GEN_STRING 1

//------------------------------------------------------------------------------
//  BeginScript()
//------------------------------------------------------------------------------
/**
 * Constructs the BeginScript command (default constructor).
 */
//------------------------------------------------------------------------------
BeginScript::BeginScript() :
   GmatCommand(wxT("BeginScript"))
{
   generatingString = wxT("BeginScript");
}


//------------------------------------------------------------------------------
//  ~BeginScript()
//------------------------------------------------------------------------------
/**
 * Destroys the BeginScript command (default constructor).
 */
//------------------------------------------------------------------------------
BeginScript::~BeginScript()
{
}


//------------------------------------------------------------------------------
//  BeginScript(const BeginScript& noop)
//------------------------------------------------------------------------------
/**
 * Makes a copy of the BeginScript command (copy constructor).
 */
//------------------------------------------------------------------------------
BeginScript::BeginScript(const BeginScript& noop) :
    GmatCommand       (noop)
{
        generatingString = noop.generatingString;
}


//------------------------------------------------------------------------------
//  BeginScript& operator=(const BeginScript&)
//------------------------------------------------------------------------------
/**
 * Sets this BeginScript to match another one (assignment operator).
 * 
 * @param noop The original used to set parameters for this one.
 *
 * @return this object.
 */
//------------------------------------------------------------------------------
BeginScript& BeginScript::operator=(const BeginScript& noop)
{
   if (this == &noop)
      return *this;

   GmatCommand::operator=(noop);
   generatingString = noop.generatingString;
   
   return *this;
}


//------------------------------------------------------------------------------
//  bool Execute()
//------------------------------------------------------------------------------
/**
 * Executes the BeginScript command (copy constructor).
 * 
 * During mission execution, BeginScript is a null operation -- nothing is done
 * in this command.  It functions as a marker in the script, indicating to the
 * GUI where a block of commands starts that should all be grouped together on a
 * ScriptEvent panel.
 *
 * @return true on success.
 */
//------------------------------------------------------------------------------
bool BeginScript::Execute()
{
   BuildCommandSummary(true);
   return true;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the BeginScript.
 *
 * @return clone of the BeginScript.
 */
//------------------------------------------------------------------------------
GmatBase* BeginScript::Clone() const
{
   return (new BeginScript(*this));
}


//------------------------------------------------------------------------------
// const wxString& GetGeneratingString(Gmat::WriteMode mode,
//                                        const wxString &prefix,
//                                        const wxString &useName)
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the BeginScript.
 *
 * @param <mode>    Specifies the type of serialization requested. (Not yet
 *                  used in commands)
 * @param <prefix>  Optional prefix appended to the object's name.  (Not yet
 *                  used in commands)
 * @param <useName> Name that replaces the object's name.  (Not yet used in
 *                  commands)
 *
 * @return The string that reproduces this command.
 */
//------------------------------------------------------------------------------
const wxString& BeginScript::GetGeneratingString(Gmat::WriteMode mode,
                                                    const wxString &prefix,
                                                    const wxString &useName)
{
   //Note: This method is called only once from the ScriptInterpreter::WriteScript()
   // So all nested ScriptEvent generating string should be handled here
   
   wxString gen;
   wxString indent;
   wxString commentLine = GetCommentLine();
   wxString inlineComment = GetInlineComment();
   wxString beginPrefix = prefix;

   if (mode != Gmat::GUI_EDITOR)
   {
      if (mode == Gmat::NO_COMMENTS)
      {
         gen << prefix << wxT("BeginScript") << wxT("\n");
      }
      else
      {
         IndentComment(gen, commentLine, prefix);
         gen << prefix << wxT("BeginScript");   
         
         if (inlineComment != wxT(""))
            gen << inlineComment << wxT("\n");
         else
            gen << wxT("\n");
      }
   }
   
   #if DBGLVL_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("BeginScript::GetGeneratingString() this=(%p)%s, mode=%d, prefix='%s', ")
       wxT("useName='%s'\n"), this, this->GetTypeName().c_str(), mode, prefix.c_str(),
       useName.c_str());
   #endif
   
   if (mode == Gmat::GUI_EDITOR)
      indent = wxT("");
   else
      indent = wxT("   ");
   
   GmatCommand *current = next;
   while (current != NULL)
   {      
      #if DBGLVL_GEN_STRING > 1
      MessageInterface::ShowMessage
         (wxT("BeginScript::GetGeneratingString() current=(%p)%s\n"), current,
          current->GetTypeName().c_str());
      #endif
      
      if (current->GetTypeName() != wxT("EndScript"))
      {
         // Indent whole block within Begin/EndScript
         IndentChildString(gen, current, indent, mode, prefix, useName, false);
         
         // Get command after EndScript
         current = GmatCommandUtil::GetNextCommand(current);
         
         if (current == NULL)
            IndentChildString(gen, current, indent, mode, beginPrefix, useName, true);
      }
      else
      {
         if (mode != Gmat::GUI_EDITOR)
         {
            // Indent whole block within Begin/EndScript
            IndentChildString(gen, current, indent, mode, beginPrefix, useName, true);
         }
         current = NULL;
      }
   }
   
   generatingString = gen;
   
   #if DBGLVL_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("BeginScript::GetGeneratingString() returnning generatingString\n"));
   MessageInterface::ShowMessage(wxT("<<<\n%s>>>\n\n"), generatingString.c_str());
   #endif
   
   return generatingString;
}


//------------------------------------------------------------------------------
// const wxString BeginScript::GetChildString(const wxString &prefix,
//                                               GmatCommand *cmd,
//                                               GmatCommand *parent)
//------------------------------------------------------------------------------
/**
 * Iteratively recurses through the command tree, building the strings to build
 * the child commands.
 *
 * @param <prefix> Prefix appended to the child command's string (typically used
 *                 to indent the child command strings).
 * @param <cmd>    The first child command in the current nesting level.
 * @param <parent> The command that has this command as a child.
 *
 * @return The string that reproduces the commands at the child's level.
 */
//------------------------------------------------------------------------------
const wxString BeginScript::GetChildString(const wxString &prefix,
                                              GmatCommand *cmd,
                                              GmatCommand *parent)
{
   #ifdef DEBUG_BEGINSCRIPT
      MessageInterface::ShowMessage(wxT("BeginScript::GetChildString entered\n"));
   #endif
   
   wxString sstr;
   wxString cmdstr;
   Integer whichOne, start;
   GmatCommand *current = cmd;
   
   while ((current != parent) && (current != NULL))
   {
      cmdstr = current->GetGeneratingString();
      start = 0;
      while (cmdstr[start] == wxT(' '))
         ++start;
      cmdstr = cmdstr.substr(start);
      sstr << prefix << cmdstr << wxT("\n");
      whichOne = 0;
      GmatCommand* child = current->GetChildCommand(whichOne);
      while ((child != NULL) && (child != cmd))
      {
         sstr << GetChildString(prefix + wxT("   "), child, current);
         ++whichOne;
         child = current->GetChildCommand(whichOne);
      }
      current = current->GetNext();
   }
   
   return sstr;
}


//------------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//------------------------------------------------------------------------------
/**
 * Renames referenced objects.
 *
 * @param type Type of the object that is renamed.
 * @param oldName The current name for the object.
 * @param newName The name the object has when this operation is complete.
 *
 * @return true on success.
 */
//------------------------------------------------------------------------------
bool BeginScript::RenameRefObject(const Gmat::ObjectType type,
                                  const wxString &oldName,
                                  const wxString &newName)
{   
   GmatCommand *current = next;
   
   while (current != NULL)
   {
      #if DEBUG_RENAME
      MessageInterface::ShowMessage
         (wxT("BeginScript::RenameRefObject() current=%s\n"),
          current->GetTypeName().c_str());
      #endif
      
      if (current->GetTypeName() != wxT("EndScript"))
      {
         current->RenameRefObject(type, oldName, newName);
         current = current->GetNext();
      }
      else
      {
         current = NULL;
      }
   }
   
   return true;
}


//------------------------------------------------------------------------------
//void IndentChildString(wxStringstream &gen, GmatCommand* cmd, 
//                       wxString &indent, Gmat::WriteMode mode,
//                       const wxString &prefix, const wxString &useName,
//                       bool indentCommentOnly)
//------------------------------------------------------------------------------
void BeginScript::IndentChildString(wxString &gen, GmatCommand* cmd, 
                                    wxString &indent, Gmat::WriteMode mode,
                                    const wxString &prefix,
                                    const wxString &useName,
                                    bool indentCommentOnly)
{
   TextParser tp;
   
   #if DBGLVL_GEN_STRING
   ShowCommand(wxT(""), wxT("BeginScript::IndentChildString() cmd = "), cmd);
   MessageInterface::ShowMessage
      (wxT("BeginScript::IndentChildString() indent='%s', mode=%d, prefix='%s', ")
       wxT("useName='%s', indentCommentOnly=%d\n"), indent.c_str(), mode, prefix.c_str(),
       useName.c_str(), indentCommentOnly);
   #endif
   
   wxString cmdstr;
   if (indentCommentOnly)
      cmdstr = cmd->GetCommentLine();
   else
      cmdstr = cmd->GetGeneratingString(mode, prefix, useName);
   
   StringArray textArray = tp.DecomposeBlock(cmdstr);
   UnsignedInt size = textArray.size();
   
   #if DBGLVL_GEN_STRING
   MessageInterface::ShowMessage(wxT("   There are %d text lines\n"), size);
   #endif
   
   if (size > 0 && textArray[0] != wxT(""))
   {
      for (UnsignedInt i=0; i<size; i++)
      {
         if (indentCommentOnly)
            gen << indent << prefix << textArray[i];
         else
            gen << indent << textArray[i];
         
         if (textArray[i].find(wxT("\n")) == cmdstr.npos &&
             textArray[i].find(wxT("\r")) == cmdstr.npos)
         {
            gen << wxT("\n");
         }
      }
   }
   
   if (indentCommentOnly)
      gen << prefix << cmd->GetTypeName() << wxT(";");
}


//------------------------------------------------------------------------------
//void IndentComment(wxStringstream &gen, wxString &comment,
//                   const wxString &prefix)
//------------------------------------------------------------------------------
void BeginScript::IndentComment(wxString &gen, wxString &comment,
                                const wxString &prefix)
{
   TextParser tp;
   
   #if DBGLVL_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("BeginScript::IndentComment() comment='%s', prefix='%s'\n"),
       comment.c_str(), prefix.c_str());
   #endif
   
   StringArray textArray = tp.DecomposeBlock(comment);
   UnsignedInt size = textArray.size();
   
   if (size > 0 && textArray[0] != wxT(""))
   {
      for (UnsignedInt i=0; i<size; i++)
      {
         gen << prefix << textArray[i];
         
         if (textArray[i].find(wxT("\n")) == comment.npos &&
             textArray[i].find(wxT("\r")) == comment.npos)
         {
            gen << wxT("\n");
         }
      }
   }
}

