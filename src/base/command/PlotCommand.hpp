//$Id$
//------------------------------------------------------------------------------
//                                PlotCommand
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Linda Jun
// Created: 2011/09/08
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
/**
 * Class declation for the PlotCommand.
 */
//------------------------------------------------------------------------------


#ifndef PlotCommand_hpp
#define PlotCommand_hpp

#include "GmatCommand.hpp"


/**
 * Command used to operate on plots
 */
class GMAT_API PlotCommand : public GmatCommand
{
public:
   PlotCommand(const wxString &plotTypeName);
   virtual          ~PlotCommand();
   PlotCommand(const PlotCommand &c);
   PlotCommand&      operator=(const PlotCommand &c);
      
   virtual const ObjectTypeArray&
                        GetRefObjectTypeArray();
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   
   // Access methods derived classes can override
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value);
   virtual wxString  GetStringParameter(const Integer id,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value,
                                           const Integer index);
   
   virtual const wxString&  
                        GetGeneratingString(
                           Gmat::WriteMode mode = Gmat::SCRIPTING,
                           const wxString &prefix = wxT(""),
                           const wxString &useName = wxT(""));
   virtual bool         InterpretAction();
   
   virtual bool         Initialize();
   
   enum
   {
      SUBSCRIBER = GmatCommandParamCount,
      PlotCommandParamCount  /// Count of the parameters for this class
   };
   
protected:
   StringArray              plotNameList;   
   std::vector<Subscriber*> thePlotList;
   
   static const wxString
      PARAMETER_TEXT[PlotCommandParamCount - GmatCommandParamCount];
   static const Gmat::ParameterType
      PARAMETER_TYPE[PlotCommandParamCount - GmatCommandParamCount];
};

#endif /* PlotCommand_hpp */
