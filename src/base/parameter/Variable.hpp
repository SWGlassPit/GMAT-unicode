//$Id: Variable.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                Variable
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
// Created: 2004/09/15
//
/**
 * Declares Variable class.
 */
//------------------------------------------------------------------------------
#ifndef Variable_hpp
#define Variable_hpp

#include "gmatdefs.hpp"
#include "RealVar.hpp"
#include "ParameterDatabase.hpp"
#include "ExpressionParser.hpp"

class GMAT_API Variable : public RealVar
{
public:

   Variable(const wxString &name, const wxString &valStr = wxT(""),
            const wxString &desc = wxT(""), const wxString &unit = wxT(""));
   Variable(const Variable &copy);
   Variable& operator=(const Variable &right);
   virtual ~Variable();

   // methods inherited from RealVar
   virtual Real GetReal() const;
   virtual Real EvaluateReal();
   
   // methods inherited from GmatBase
   virtual GmatBase* Clone() const;
   virtual void Copy(const GmatBase*);
   
   virtual bool SetStringParameter(const Integer id, const wxString &value);
   virtual bool SetStringParameter(const wxString &label,
                                   const wxString &value);
   
   virtual bool RenameRefObject(const Gmat::ObjectType type,
                                const wxString &oldName,
                                const wxString &newName);
   
   virtual wxString GetRefObjectName(const Gmat::ObjectType type) const;
   virtual bool SetRefObjectName(const Gmat::ObjectType type,
                                 const wxString &name);
   virtual GmatBase* GetRefObject(const Gmat::ObjectType type,
                                  const wxString &name);
   virtual bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                             const wxString &name = wxT(""));
   
   virtual const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type);
   
   virtual const wxString& GetGeneratingString(Gmat::WriteMode mode,
                                                  const wxString &prefix,
                                                  const wxString &useName);
protected:
   
   // Currently simple math expression (+-*/^) is no longer allowed in object mode.
   // Math equations are allowed in only command mode.
   // Save this for future? (LOJ: 2010.11.24)
   #ifdef __ALLOW_SIMPLE_VAR_EXP__
   ParameterDatabase *mParamDb;
   ExpressionParser *mExpParser;
   void CreateSimpleExpression();
   void CopySimpleExpression(const Variable &copy);
   void AssignSimpleExpression(const Variable &right);
   void DeleteSimpleExpression();
   #endif
};

#endif // Variable_hpp
