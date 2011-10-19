//$Id: ObjectReferencedAxes.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  ObjectReferencedAxes
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under 
// MOMS Task order 124.
//
// Author: Wendy C. Shoan/GSFC/MAB
// Created: 2005/03/01
//
/**
 * Definition of the ObjectReferencedAxes class.
 *
 */
//------------------------------------------------------------------------------

#ifndef ObjectReferencedAxes_hpp
#define ObjectReferencedAxes_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "AxisSystem.hpp"
#include "DynamicAxes.hpp"

class GMAT_API ObjectReferencedAxes : public DynamicAxes
{
public:

   // default constructor
   ObjectReferencedAxes(const wxString &itsName = wxT(""));
   // another constructor - for derived classes to call
   ObjectReferencedAxes(const wxString &itsType,
                        const wxString &itsName);
   // copy constructor
   ObjectReferencedAxes(const ObjectReferencedAxes &orAxes);
   // operator = for assignment
   const ObjectReferencedAxes& operator=(const ObjectReferencedAxes &orAxes);
   // destructor
   virtual ~ObjectReferencedAxes();

   // methods to set parameters for the AxisSystems
   // (inherited from CoordinateBase)
   virtual void SetPrimaryObject(SpacePoint *prim);
   virtual void SetSecondaryObject(SpacePoint *second);
   virtual void SetXAxis(const wxString &toValue);
   virtual void SetYAxis(const wxString &toValue);
   virtual void SetZAxis(const wxString &toValue);   
   virtual SpacePoint* GetPrimaryObject() const;
   virtual SpacePoint* GetSecondaryObject() const;
   virtual wxString GetXAxis() const;
   virtual wxString GetYAxis() const;
   virtual wxString GetZAxis() const;
   virtual void        ResetAxes();
   
   virtual GmatCoordinate::ParameterUsage UsesPrimary() const;
   virtual GmatCoordinate::ParameterUsage UsesSecondary() const;
   virtual GmatCoordinate::ParameterUsage UsesXAxis() const;
   virtual GmatCoordinate::ParameterUsage UsesYAxis() const;
   virtual GmatCoordinate::ParameterUsage UsesZAxis() const;
   
   // method to initialize the data
   virtual bool Initialize();
   
   // all classes derived from GmatBase must supply this Clone method;
   // this must be implemented in the 'leaf' classes
   virtual GmatBase*       Clone() const;
   
   virtual bool RenameRefObject(const Gmat::ObjectType type,
                                const wxString &oldName,
                                const wxString &newName);
   
   // Parameter access methods - overridden from GmatBase
   virtual wxString     GetParameterText(const Integer id) const;     
   virtual Integer         GetParameterID(const wxString &str) const; 
   virtual Gmat::ParameterType
                           GetParameterType(const Integer id) const;
   virtual wxString     GetParameterTypeString(const Integer id) const;
   virtual wxString     GetStringParameter(const Integer id) const;
   virtual bool            SetStringParameter(const Integer id, 
                                              const wxString &value);
   virtual wxString     GetStringParameter(const wxString &label) const;
   virtual bool            SetStringParameter(const wxString &label, 
                                              const wxString &value);
   virtual GmatBase*       GetRefObject(const Gmat::ObjectType type,
                                        const wxString &name);
   const StringArray&      GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool            SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                        const wxString &name = wxT(""));
protected:

   // types and data
   enum
   {
      X_AXIS = DynamicAxesParamCount,
      Y_AXIS,
      Z_AXIS,
      PRIMARY_OBJECT_NAME,
      SECONDARY_OBJECT_NAME,
      ObjectReferencedAxesParamCount
   };
   
   static const wxString PARAMETER_TEXT[ObjectReferencedAxesParamCount - 
                                           DynamicAxesParamCount];
   
   static const Gmat::ParameterType PARAMETER_TYPE[ObjectReferencedAxesParamCount - 
                                                   DynamicAxesParamCount];
   wxString primaryName;
   wxString secondaryName;
   SpacePoint  *primary;
   SpacePoint  *secondary;
   
   wxString xAxis;
   wxString yAxis;
   wxString zAxis;

   // methods
   // methods to compute the rotation matrix (and its derivative) to transform
   // from this ObjectReferencedAxes system to MJ2000EqAxes system; inherited
   // from AxisSystem
   virtual void CalculateRotationMatrix(const A1Mjd &atEpoch,
                                        bool forceComputation = false);


};
#endif // ObjectReferencedAxes_hpp
