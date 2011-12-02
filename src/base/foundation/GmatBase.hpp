//$Id: GmatBase.hpp 9969 2011-10-21 22:56:49Z djcinsb $
//------------------------------------------------------------------------------
//                                  GmatBase
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Darrel J. Conway
// Created: 2003/09/25
//
//
// Modification History:
//
// 11/9/2003 D. Conway
//   Made GetParameterCount virtual so PropSetup can override it, returning the
//   count for the member ForceModel, Forces, and Propagator.
/**
 * Definition for the base class for all GMAT extensible objects
 */
//------------------------------------------------------------------------------


#ifndef GmatBase_hpp
#define GmatBase_hpp

#include "gmatdefs.hpp"
#include "GmatBaseException.hpp"
#include "Rvector.hpp"
#include "Rmatrix.hpp"
#include "Covariance.hpp"

#include <algorithm>                    // Required by GCC 4.3 for find


// Forward reference
class SolarSystem;
class CoordinateSystem;

// The allocation size used to construct estimation object parameter IDs
#define ESTIMATION_TYPE_ALLOCATION  250


//------------------------------------------------------------------------------
/**
 * Definition for the base class for all GMAT extensible objects
 *
 * The following class hierarchy trees use this class as their basis:
 *
 *     SpacePoint (hence Spacecraft and Formation, and all CelestialBody's)
 *     Propagator
 *     PhysicalModel (hence Force and ForceModel)
 *     PropConfig
 *     Parameter
 *     GmatCommand
 *
 * Every class that users can use to derive new classes, or that need to be
 * accessed through generic pointers, should be derived from this class to
 * ensure consistent interfaces accessed by the GMAT control systems (i.e. the
 * Moderator, FactoryManager, Configuration, Interpreter, and Sandbox, along
 * with the GUIInterpreter).
 */
//------------------------------------------------------------------------------
class GMAT_API GmatBase
{
public:
   // The usual suspects
   GmatBase(Gmat::ObjectType typeId, const wxString &typeStr,
            const wxString &nomme = wxT(""));
   virtual ~GmatBase() = 0;
   GmatBase(const GmatBase &a);
   GmatBase&            operator=(const GmatBase &a);

   // Access methods called on the base class
   virtual Gmat::ObjectType
                        GetType() const;
   inline wxString   GetTypeName() const;
   inline wxString   GetName() const;
   virtual bool         SetName(const wxString &who,
                                const wxString &oldName = wxT(""));
   virtual Integer      GetParameterCount() const;

   bool                 IsOfType(Gmat::ObjectType ofType) const;
   bool                 IsOfType(wxString typeDescription) const;

   void                 SetShowPrefaceComment(bool show = true);
   void                 SetShowInlineComment(bool show = true);
   bool                 GetShowPrefaceComment();
   bool                 GetShowInlineComment();

   // Access methods derived classes can override on comments
   virtual const wxString
                        GetCommentLine() const;
   virtual void         SetCommentLine(const wxString &comment);

   virtual const wxString
                        GetInlineComment() const;
   virtual void         SetInlineComment(const wxString &comment);

   virtual const wxString
                        GetAttributeCommentLine(Integer index);
   virtual void         SetAttributeCommentLine(Integer index,
                                                const wxString &comment);

   virtual const wxString
                        GetInlineAttributeComment(Integer index);
   virtual void         SetInlineAttributeComment(Integer index,
                                                  const wxString &comment);

   // Access methods derived classes can override on reference objects
   virtual wxString  GetRefObjectName(const Gmat::ObjectType type) const;
   virtual bool         HasRefObjectTypeArray();
   virtual const ObjectTypeArray&
                        GetRefObjectTypeArray();
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool         SetRefObjectName(const Gmat::ObjectType type,
                                         const wxString &name);
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name);
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

   virtual bool         IsOwnedObject(Integer id) const;
   virtual Integer      GetOwnedObjectCount();
   virtual GmatBase*    GetOwnedObject(Integer whichOne);
   virtual bool         SetIsGlobal(bool globalFlag);
   virtual bool         GetIsGlobal() const;
   virtual bool         SetIsLocal(bool localFlag);
   virtual bool         IsLocal() const;
   virtual bool         IsObjectCloaked() const;
   virtual bool         SaveAllAsDefault();
   virtual bool         SaveParameterAsDefault(const Integer id);
   virtual bool         SaveParameterAsDefault(const wxString &label);
   /// method to determine if a parameter value has been changed from the default - 
   // should be implemented in leaf classes that need to monitor changes to
   // parameter values (currently, SolarSystem and celestial bodies)
   
   virtual bool         ExecuteCallback();
   virtual bool         IsCallbackExecuting();
   virtual bool         PutCallbackData(wxString &data);
   virtual wxString  GetCallbackResults();

   // required method for all subclasses
   virtual GmatBase*    Clone() const = 0;

   // required method for all subclasses that can be copied in a script
   virtual void         Copy(const GmatBase*);

   virtual bool         Validate();
   virtual bool         Initialize();
   virtual void         SetSolarSystem(SolarSystem *ss);
   virtual void         SetInternalCoordSystem(CoordinateSystem *cs);

   virtual bool         RequiresJ2000Body();

   // Access methods derived classes can override
   virtual wxString  GetParameterText(const Integer id) const;
   virtual wxString  GetParameterUnit(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;

   virtual bool         IsParameterReadOnly(const Integer id) const;
   virtual bool         IsParameterReadOnly(const wxString &label) const;
   virtual bool         IsParameterEnabled(const Integer id) const;
   virtual bool         IsParameterEnabled(const wxString &label) const;
   virtual bool         IsParameterCloaked(const Integer id) const;
   virtual bool         IsParameterCloaked(const wxString &label) const;
   virtual bool         IsParameterEqualToDefault(const Integer id) const;
   virtual bool         IsParameterEqualToDefault(const wxString &label) const;
   virtual bool         IsParameterVisible(const Integer id) const;
   virtual bool         IsParameterVisible(const wxString &label) const;

   virtual bool         ParameterAffectsDynamics(const Integer id) const;
   virtual bool         ParameterDvInitializesNonzero(const Integer id,
                              const Integer r = 0, const Integer c = 0) const;
   virtual Real         ParameterDvInitialValue(const Integer id,
                              const Integer r = 0, const Integer c = 0) const;
   virtual bool         ParameterUpdatesAfterSuperposition(const Integer id) const;

   virtual Gmat::ObjectType
                        GetPropertyObjectType(const Integer id) const;
   virtual const StringArray&
                        GetPropertyEnumStrings(const Integer id) const;
   virtual const StringArray&
                        GetPropertyEnumStrings(const wxString &label) const;
   
   virtual Real         GetRealParameter(const Integer id) const;
   virtual Real         SetRealParameter(const Integer id,
                                         const Real value);
   virtual Real         GetRealParameter(const Integer id,
                                         const Integer index) const;
   virtual Real         GetRealParameter(const Integer id, const Integer row,
                                         const Integer col) const;
   virtual Real         SetRealParameter(const Integer id,
                                         const Real value,
                                         const Integer index);
   virtual Real         SetRealParameter(const Integer id, const Real value,
                                         const Integer row, const Integer col);

   virtual Integer      GetIntegerParameter(const Integer id) const;
   virtual Integer      SetIntegerParameter(const Integer id,
                                            const Integer value);
   virtual Integer      GetIntegerParameter(const Integer id,
                                            const Integer index) const;
   virtual Integer      SetIntegerParameter(const Integer id,
                                            const Integer value,
                                            const Integer index);

   virtual UnsignedInt  GetUnsignedIntParameter(const Integer id) const;
   virtual UnsignedInt  SetUnsignedIntParameter(const Integer id,
                                                const UnsignedInt value);
   virtual UnsignedInt  GetUnsignedIntParameter(const Integer id,
                                                const Integer index) const;
   virtual UnsignedInt  SetUnsignedIntParameter(const Integer id,
                                                const UnsignedInt value,
                                                const Integer index);
   virtual const UnsignedIntArray&
                        GetUnsignedIntArrayParameter(const Integer id) const;

   virtual const IntegerArray&
                        GetIntegerArrayParameter(const Integer id) const;
   virtual const IntegerArray&
                        GetIntegerArrayParameter(const Integer id,
                                                 const Integer index) const;

   virtual const Rvector&
                        GetRvectorParameter(const Integer id) const;
   virtual const Rvector&
                        SetRvectorParameter(const Integer id,
                                            const Rvector &value);

   virtual const Rmatrix&
                        GetRmatrixParameter(const Integer id) const;
   virtual const Rmatrix&
                        SetRmatrixParameter(const Integer id,
                                            const Rmatrix &value);

   virtual wxString  GetStringParameter(const Integer id) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value);
   virtual wxString  GetStringParameter(const Integer id,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value,
                                           const Integer index);
   virtual const StringArray&
                        GetStringArrayParameter(const Integer id) const;
   virtual const StringArray&
                        GetStringArrayParameter(const Integer id,
                                                const Integer index) const;

   virtual wxString  GetOnOffParameter(const Integer id) const;
   virtual bool         SetOnOffParameter(const Integer id,
                                         const wxString &value);

   virtual bool         GetBooleanParameter(const Integer id) const;
   virtual bool         SetBooleanParameter(const Integer id,
                                            const bool value);
   virtual bool         GetBooleanParameter(const Integer id,
                                            const Integer index) const;
   virtual bool         SetBooleanParameter(const Integer id,
                                            const bool value,
                                            const Integer index);

   virtual Real         GetRealParameter(const wxString &label) const;
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value);
   virtual Real         GetRealParameter(const wxString &label,
                                         const Integer index) const;
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value,
                                         const Integer index);
   virtual Real         GetRealParameter(const wxString &label,
                                         const Integer row,
                                         const Integer col) const;
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value, const Integer row,
                                         const Integer col);

   virtual Integer      GetIntegerParameter(const wxString &label) const;
   virtual Integer      SetIntegerParameter(const wxString &label,
                                            const Integer value);
   virtual Integer      GetIntegerParameter(const wxString &label,
                                            const Integer index) const;
   virtual Integer      SetIntegerParameter(const wxString &label,
                                            const Integer value,
                                            const Integer index);

   virtual UnsignedInt  GetUnsignedIntParameter(const wxString &label) const;
   virtual UnsignedInt  SetUnsignedIntParameter(const wxString &label,
                                                const UnsignedInt value);
   virtual UnsignedInt  GetUnsignedIntParameter(const wxString &label,
                                                const Integer index) const;
   virtual UnsignedInt  SetUnsignedIntParameter(const wxString &label,
                                                const UnsignedInt value,
                                                const Integer index);
   virtual const UnsignedIntArray&
                        GetUnsignedIntArrayParameter(const wxString &label) const;

   virtual const Rvector&
                        GetRvectorParameter(const wxString &label) const;
   virtual const Rvector&
                        SetRvectorParameter(const wxString &label,
                                            const Rvector &value);

   virtual const Rmatrix&
                        GetRmatrixParameter(const wxString &label) const;
   virtual const Rmatrix&
                        SetRmatrixParameter(const wxString &label,
                                            const Rmatrix &value);

   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value);
   virtual wxString  GetStringParameter(const wxString &label,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value,
                                           const Integer index);

   virtual const StringArray&
                        GetStringArrayParameter(const wxString &label) const;
   virtual const StringArray&
                        GetStringArrayParameter(const wxString &label,
                                                const Integer index) const;

   virtual bool         GetBooleanParameter(const wxString &label) const;
   virtual bool         SetBooleanParameter(const wxString &label,
                                            const bool value);
   virtual bool         GetBooleanParameter(const wxString &label,
                                            const Integer index) const;
   virtual bool         SetBooleanParameter(const wxString &label,
                                            const bool value,
                                            const Integer index);
   virtual const BooleanArray&
                        GetBooleanArrayParameter(const Integer id) const;
   virtual const BooleanArray&
                        GetBooleanArrayParameter(const wxString &label) const;
   virtual bool         SetBooleanArrayParameter(const Integer id,
                                                 const BooleanArray &valueArray);
   virtual bool         SetBooleanArrayParameter(const wxString &label,
                                                 const BooleanArray &valueArray);
   
   virtual wxString  GetOnOffParameter(const wxString &label) const;
   virtual bool         SetOnOffParameter(const wxString &label,
                                          const wxString &value);

   virtual bool         TakeAction(const wxString &action,
                                   const wxString &actionData = wxT(""));
   virtual bool         TakeRequiredAction(const Integer id);
   virtual bool         TakeRequiredAction(const wxString &label);

   virtual const wxString&
                        GetGeneratingString(
                           Gmat::WriteMode mode = Gmat::SCRIPTING,
                           const wxString &prefix = wxT(""),
                           const wxString &useName = wxT(""));
   virtual StringArray  GetGeneratingStringArray(
                           Gmat::WriteMode mode = Gmat::SCRIPTING,
                           const wxString &prefix = wxT(""),
                           const wxString &useName = wxT(""));
   virtual wxString  BuildPropertyName(GmatBase *ownedObj);
   virtual void         FinalizeCreation();

   virtual wxString  GetLastErrorMessage();
   virtual wxString  GetErrorMessageFormat();
   virtual void         SetErrorMessageFormat(const wxString &fmt);

   /// Return value used if the parameter is not accessible as a Real
   static const Real         REAL_PARAMETER_UNDEFINED;
   /// Return value used if the parameter is not accessible as an Integer
   static const Integer      INTEGER_PARAMETER_UNDEFINED;
   /// Return value used if the parameter is not accessible as an UnsignedInt
   static const UnsignedInt  UNSIGNED_INT_PARAMETER_UNDEFINED;
   /// Return value used if the parameter is not accessible as a String
   static const wxString  STRING_PARAMETER_UNDEFINED;
   /// Return value used if the parameter is not accessible as a StringArray
   static const StringArray  STRINGARRAY_PARAMETER_UNDEFINED;
   /// Return value used if the parameter is not accessible as a IntegerArray
   static const IntegerArray INTEGERARRAY_PARAMETER_UNDEFINED;
   /// Return value used if the parameter is not accessible as a UnsignedIntArray
   static const UnsignedIntArray UNSIGNED_INTARRAY_PARAMETER_UNDEFINED;
   /// Return value used if the parameter is not accessible as a Rvector
   static const Rvector      RVECTOR_PARAMETER_UNDEFINED;
   /// Return value used if the parameter is not accessible as a Rmatrix
   static const Rmatrix      RMATRIX_PARAMETER_UNDEFINED;
   /// String mappings for the GMAT data types
   static const wxString  PARAM_TYPE_STRING[Gmat::TypeCount];
   /// String mappings for the GMAT object types
   static const wxString  OBJECT_TYPE_STRING[Gmat::UNKNOWN_OBJECT - Gmat::SPACECRAFT+1];
   static const bool         AUTOMATIC_GLOBAL_FLAGS[Gmat::UNKNOWN_OBJECT - Gmat::SPACECRAFT+1];

   /// Method to return the current number of instantiated objects
   static Integer          GetInstanceCount();
   /// Method for getting GMAT object type
   static Gmat::ObjectType GetObjectType(const wxString &typeString);
   /// Method for getting GMAT object type string
   static wxString      GetObjectTypeString(Gmat::ObjectType type);
   /// Method for getting data precision
   static Integer          GetDataPrecision();
   /// Method for getting time precision
   static Integer          GetTimePrecision();

   virtual Integer         GetPropItemID(const wxString &whichItem);
   virtual Integer         SetPropItem(const wxString &propItem);
   virtual StringArray     GetDefaultPropItems();
   virtual Real*           GetPropItem(const Integer item);
   virtual Integer         GetPropItemSize(const Integer item);
   virtual bool            PropItemNeedsFinalUpdate(const Integer item);
   virtual bool            HasAssociatedStateObjects();
   virtual wxString     GetAssociateName(UnsignedInt val = 0);

   virtual Integer         GetEstimationParameterID(const wxString &param);
   virtual Integer         SetEstimationParameter(const wxString &param);
   virtual bool            IsEstimationParameterValid(const Integer id);
   virtual Integer         GetEstimationParameterSize(const Integer id);
   virtual Real*           GetEstimationParameterValue(const Integer id);

   virtual bool            HasDynamicParameterSTM(Integer parameterId);
   virtual Rmatrix*        GetParameterSTM(Integer parameterId);

   // Covariance handling code
   virtual Integer         HasParameterCovariances(Integer parameterId);
   virtual Rmatrix*        GetParameterCovariances(Integer parameterId = -1);
   virtual Covariance*     GetCovariance();

protected:
   /// Parameter IDs
   enum
   {
      COVARIANCE = 0,
      GmatBaseParamCount,
   };

   /// Spacecraft parameter types
   static const Gmat::ParameterType PARAMETER_TYPE[GmatBaseParamCount];
   /// Spacecraft parameter labels
   static const wxString PARAMETER_LABEL[GmatBaseParamCount];


   /// count of the number of GmatBase objects currently instantiated
   static Integer      instanceCount;

   /// Count of the accessible parameters
   Integer             parameterCount;
   /// Script string used or this class
   wxString         typeName;
   /// Name of the object -- empty if it is nameless
   wxString         instanceName;
   /// Enumerated base type of the object
   Gmat::ObjectType    type;
   /// Number of owned objects that belong to this instance
   Integer             ownedObjectCount;
   /// Script string used to build the object
   wxString         generatingString;
   /// The list of generic types that this class extends.
   ObjectTypeArray     objectTypes;
   /// The list types that this class extends, by name
   StringArray         objectTypeNames;

   /// The list of object types referenced by this class
   ObjectTypeArray     refObjectTypes;
   /// The list of object names referenced by this class
   StringArray         refObjectNames;
   /// flag indicating whether or not the object is Global
   bool                isGlobal;
   /// flag indicating whether or not the object is local inside a function
   bool                isLocal;
   
   /// flag indicating whether or not a Callback method is currently executing
   bool                callbackExecuting;

   /// error message and formats
   wxString         lastErrorMessage;
   wxString         errorMessageFormat;
   wxString         errorMessageFormatUnnamed;
   wxString         deprecatedMessageFormat;
   
   /// flag used to deterine if the current write is in Matlab mode
   bool                inMatlabMode;

   /// Integer array used to hold the parameter write order
   /// This array is automatically created if array is empty
   IntegerArray         parameterWriteOrder;
   /// String used to hold the comment line
   wxString         commentLine;
   /// String used to hold inline comment
   wxString         inlineComment;
   /// String array used to hold the attribute comments
   StringArray         attributeCommentLines;
   /// String array used to hold the attribute inline comments
   StringArray         attributeInlineComments;
   /// Flag to indicating whether to show preface comment
   bool                showPrefaceComment;
   /// Flag to indicating whether to show inline comment
   bool                showInlineComment;
   /// flag indicating whether or not to omit the wxT("Create") line when writing the script
   bool                cloaking;

   // Ordered list of parameters that have covariances
   StringArray         covarianceList;
   // Ordered list of parameter IDs that have covariances
   IntegerArray        covarianceIds;
   // Size of the covariance element
   IntegerArray        covarianceSizes;
   // Covariance matrix for parameters identified in covarianceList
   Covariance          covariance;

   // Scripting interfaces
   void                CopyParameters(const GmatBase &a);
   virtual void        WriteParameters(Gmat::WriteMode mode,
                                       wxString &prefix,
                                       wxString &stream);
   void                WriteParameterValue(Integer id,
                                           wxString &stream);

private:

   virtual void PrepCommentTables();
};


//------------------------------------------------------------------------------
//  wxString GetTypeName() const
//------------------------------------------------------------------------------
/**
 * Retrieves the type name (i.e. the type used in scripting) for the object.
 *
 * @return The type name.
 */
wxString GmatBase::GetTypeName() const
{
   return typeName;
}


//------------------------------------------------------------------------------
//  wxString GetName() const
//------------------------------------------------------------------------------
/**
 * Retrieves the object's name.
 *
 * @return The name.
 */
wxString GmatBase::GetName() const
{
   return instanceName;
}
#endif // GmatBase_hpp

