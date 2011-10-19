//$Id: CommandUtil.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                 CommandUtil
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
// Author: Linda Jun
// Created: 2005/11/28
//
/**
 * This file provides methods to get whole mission sequence string.
 */
//------------------------------------------------------------------------------
#ifndef CommandUtil_hpp
#define CommandUtil_hpp

#include "gmatdefs.hpp"
#include "GmatCommand.hpp"

namespace GmatCommandUtil
{
   GmatCommand GMAT_API *GetFirstCommand(GmatCommand *cmd);
   GmatCommand GMAT_API *GetLastCommand(GmatCommand *cmd);
   GmatCommand GMAT_API *GetNextCommand(GmatCommand *cmd);
   GmatCommand GMAT_API *GetPreviousCommand(GmatCommand *from, GmatCommand *cmd);
   GmatCommand GMAT_API *GetMatchingEnd(GmatCommand *cmd);
   GmatCommand GMAT_API *GetParentCommand(GmatCommand *top, GmatCommand *cmd);
   GmatCommand GMAT_API *GetSubParent(GmatCommand *brCmd, GmatCommand *cmd);
   GmatCommand GMAT_API *RemoveCommand(GmatCommand *seq, GmatCommand *cmd);
   bool GMAT_API ClearCommandSeq(GmatCommand *seq, bool leaveFirstCmd = true,
                        bool callRunComplete = true);
   bool GMAT_API IsAfter(GmatCommand *cmd1, GmatCommand *cmd2);
   bool GMAT_API FindObject(GmatCommand *cmd, Gmat::ObjectType objType,
                   const wxString &objName, wxString &cmdName);
   bool GMAT_API FindObjectFromSubCommands(GmatCommand *brCmd, Integer level,
                                  Gmat::ObjectType objType,
                                  const wxString &objName, wxString &cmdName);
   wxString GMAT_API GetCommandSeqString(GmatCommand *cmd, bool showAddr = true,
                                   bool showGenStr = false);
   void GMAT_API GetSubCommandString(GmatCommand* brCmd, Integer level,
                            wxString &cmdseq, bool showAddr = true,
                            bool showGenStr = false);
   void GMAT_API ShowCommand(const wxString &title1, GmatCommand *cmd1,
                    const wxString &title2 = wxT(""), GmatCommand *cmd2 = NULL);
}

#endif // CommandUtil_hpp
