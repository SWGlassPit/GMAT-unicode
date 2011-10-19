//$Id: Formation.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              Formation
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CI63P
//
// Author: Darrel J. Conway, Thinking Systems, Inc.
// Created: 2004/7/24
//
/**
 * Defines the class used for formations. 
 */
//------------------------------------------------------------------------------


#ifndef Formation_hpp
#define Formation_hpp

#include "SpaceObject.hpp"

class GMAT_API Formation : public SpaceObject
{
public:
   Formation(Gmat::ObjectType typeId, const wxString &typeStr, 
             const wxString &instName);
   virtual ~Formation();
   Formation(const Formation& orig);
   Formation&           operator=(const Formation& orig);
   
   virtual const Rvector6 GetMJ2000State(const A1Mjd &atTime);
   
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   virtual GmatBase*    Clone() const;
   virtual void         Copy(const GmatBase* orig);
   virtual void         ParametersHaveChanged(bool flag);
   
   // Access methods derived classes can override
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   virtual bool         IsParameterReadOnly(const Integer id) const;
   
   virtual bool         GetBooleanParameter(const Integer id) const;
   virtual bool         GetBooleanParameter(const wxString &label) const;
   virtual bool         SetBooleanParameter(const Integer id, const bool value);
   virtual bool         SetBooleanParameter(const wxString &label,
                                            const bool value);
   
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual wxString  GetStringParameter(const Integer id,
                                           const Integer index) const;
   
   
   virtual Real         GetRealParameter(const Integer id) const;
   virtual Real         GetRealParameter(const wxString &label) const;
   virtual Real         SetRealParameter(const Integer id, const Real value);
   virtual Real         SetRealParameter(const wxString &label, 
                                         const Real value);
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value);
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value,
                                           const Integer index);
   virtual const StringArray& 
                        GetStringArrayParameter(const Integer id) const;
   virtual const StringArray& 
                        GetStringArrayParameter(const wxString &label) const;
   
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name,
                                     const Integer index);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name = wxT(""));
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name,
                                     const Integer index);
   virtual ObjectArray& GetRefObjectArray(const Gmat::ObjectType type);
   virtual ObjectArray& GetRefObjectArray(const wxString& typeString);
   
   virtual void         BuildState();
   virtual void         UpdateElements();
   virtual void         UpdateState();
   virtual bool         TakeAction(const wxString &action,
                                   const wxString &actionData = wxT(""));
   virtual void         ClearLastStopTriggered();
   // virtual void SetLastStopTriggered(const wxString &stopCondName);
   // virtual bool WasLastStopTriggered(const wxString &stopCondName);
   
//   virtual Integer         GetPropItemID(const wxString &whichItem);
   virtual Integer         SetPropItem(const wxString &propItem);
   virtual StringArray     GetDefaultPropItems();
   virtual Real*           GetPropItem(const Integer item);
   virtual Integer         GetPropItemSize(const Integer item);
   
protected:
   /// List of the object names used in the formation
   StringArray                      componentNames;
   /// Pointers to the formation members
   std::vector <SpaceObject *>      components;
   /// Size of the state vector used in propagation
   Integer                          dimension;
   /// Number of spacecraft in the state
   UnsignedInt                      satCount;
   
   /// Enumerated parameter IDs   
   enum
   {
      ADDED_SPACECRAFT = SpaceObjectParamCount,
      REMOVED_SPACECRAFT,
      CLEAR_NAMES,
      FORMATION_STM,
      FORMATION_CARTESIAN_STATE,
      FormationParamCount
   };
   
   /// Array of supported parameters
   static const wxString
      PARAMETER_TEXT[FormationParamCount - SpaceObjectParamCount];
   /// Array of parameter types
   static const Gmat::ParameterType
      PARAMETER_TYPE[FormationParamCount - SpaceObjectParamCount];
   
   bool                 ClearSpacecraftList();
   bool                 RemoveSpacecraft(const wxString &name);
};

#endif // Formation_hpp
