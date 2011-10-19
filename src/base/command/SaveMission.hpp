//$Id: SaveMission.hpp 9513 2011-04-30 21:23:06Z djcinsb $
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
 * Class declaration for the SaveMission command
 */
//------------------------------------------------------------------------------
#ifndef SaveMission_hpp
#define SaveMission_hpp

#include "GmatCommand.hpp"

/**
 * Command used to write whole mission to ASCII files.
 */
class GMAT_API SaveMission : public GmatCommand
{
public:
   SaveMission();
   virtual ~SaveMission();
   SaveMission(const SaveMission& sv);
   SaveMission&         operator=(const SaveMission&);
   
   // inherited from GmatCommand
   virtual bool         Execute();
   
   // inherited from GmatBase
   virtual bool         InterpretAction();
   virtual GmatBase*    Clone(void) const;
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   
   // Parameter accessors
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value);
   virtual bool         SetStringParameter(const wxString &label, 
                                           const wxString &value);
   
   // for generating string
   virtual const wxString&
                        GetGeneratingString(
                           Gmat::WriteMode mode = Gmat::SCRIPTING,
                           const wxString &prefix = wxT(""),
                           const wxString &useName = wxT(""));
   
protected:
   // Parameter IDs
   enum  
   {
      FILE_NAME = GmatCommandParamCount,
      SaveMissionParamCount
   };
   
   wxString fileName;
   
   static const wxString
      PARAMETER_TEXT[SaveMissionParamCount - GmatCommandParamCount];
   static const Gmat::ParameterType
      PARAMETER_TYPE[SaveMissionParamCount - GmatCommandParamCount];
};

#endif // SaveMission_hpp
