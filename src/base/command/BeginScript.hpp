//$Id: BeginScript.hpp 9513 2011-04-30 21:23:06Z djcinsb $
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



#ifndef BeginScript_hpp
#define BeginScript_hpp

#include "GmatCommand.hpp" // inheriting class's header file

/**
 * Default command used to initialize the command sequence lists in the 
 * Moderator
 */
class GMAT_API BeginScript : public GmatCommand
{
public:
   BeginScript();
   virtual ~BeginScript();
   BeginScript(const BeginScript& noop);
   BeginScript&         operator=(const BeginScript&);
   
   bool                 Execute();
   
   // inherited from GmatBase
   virtual GmatBase*    Clone() const;
   virtual const wxString&
                        GetGeneratingString(
                           Gmat::WriteMode mode = Gmat::SCRIPTING,
                           const wxString &prefix = wxT(""),
                           const wxString &useName = wxT(""));
   
   const wxString    GetChildString(const wxString &prefix,
                                       GmatCommand *child, GmatCommand *parent);
   
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
protected:
   
   void IndentChildString(wxString &gen, GmatCommand* cmd, 
                          wxString &indent, Gmat::WriteMode mode,
                          const wxString &prefix, const wxString &useName,
                          bool indentCommentOnly);
   void IndentComment(wxString &gen, wxString &comment,
                      const wxString &prefix);
};

#endif // BeginScript_hpp
