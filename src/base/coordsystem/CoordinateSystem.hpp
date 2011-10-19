//$Id: CoordinateSystem.hpp 9761 2011-08-15 18:48:03Z wendys-dev $
//------------------------------------------------------------------------------
//                                  CoordinateSystem
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
// Created: 2004/12/22
//
/**
 * Definition of the CoordinateSystem class.
 *
 */
//------------------------------------------------------------------------------

#ifndef CoordinateSystem_hpp
#define CoordinateSystem_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "CoordinateBase.hpp"
#include "AxisSystem.hpp"
#include "Rvector.hpp"
#include "A1Mjd.hpp"


class GMAT_API CoordinateSystem : public CoordinateBase
{
public:
   // default constructor
   CoordinateSystem(const wxString &itsType,
                    const wxString &itsName = wxT(""));
   // copy constructor
   CoordinateSystem(const CoordinateSystem &coordSys);
   // operator = for assignment
   const CoordinateSystem& operator=(const CoordinateSystem &coordSys);
   // operator == for equality testing
   const bool operator==(const CoordinateSystem &coordSys);
   // destructor
   virtual ~CoordinateSystem();

   // virtual methods to check to see how/if an AxisSystem uses 
   // a particular parameter (calls through to its AxisSystem)
   virtual GmatCoordinate::ParameterUsage UsesEopFile() const;
   virtual GmatCoordinate::ParameterUsage UsesItrfFile() const;
   virtual GmatCoordinate::ParameterUsage UsesEpoch() const;
   virtual GmatCoordinate::ParameterUsage UsesPrimary() const;
   virtual GmatCoordinate::ParameterUsage UsesSecondary() const;
   virtual GmatCoordinate::ParameterUsage UsesXAxis() const;
   virtual GmatCoordinate::ParameterUsage UsesYAxis() const;
   virtual GmatCoordinate::ParameterUsage UsesZAxis() const;
   virtual GmatCoordinate::ParameterUsage UsesNutationUpdateInterval() const;
   virtual bool                           UsesSpacecraft() const;

   // methods to set parameters for the AxisSystems
   virtual void                  SetPrimaryObject(SpacePoint *prim);
   virtual void                  SetSecondaryObject(SpacePoint *second);
   virtual void                  SetEpoch(const A1Mjd &toEpoch);
   virtual void                  SetXAxis(const wxString &toValue);
   virtual void                  SetYAxis(const wxString &toValue);
   virtual void                  SetZAxis(const wxString &toValue);
   virtual void                  SetEopFile(EopFile *eopF);
   virtual void                  SetCoefficientsFile(
                                    ItrfCoefficientsFile *itrfF);

   virtual SpacePoint*           GetPrimaryObject() const;
   virtual SpacePoint*           GetSecondaryObject() const;
   virtual A1Mjd                 GetEpoch() const;
   virtual wxString           GetXAxis() const;
   virtual wxString           GetYAxis() const;
   virtual wxString           GetZAxis() const;
   virtual EopFile*              GetEopFile() const;
   virtual ItrfCoefficientsFile* GetItrfCoefficientsFile();
   virtual Rmatrix33             GetLastRotationMatrix() const;
   virtual void                  GetLastRotationMatrix(Real *mat) const;
   virtual Rmatrix33             GetLastRotationDotMatrix() const;
   virtual void                  GetLastRotationDotMatrix(Real *mat) const;
   virtual bool                  AreAxesOfType(const wxString &ofType) const;
   
   
   // initializes the CoordinateSystem
   virtual bool Initialize();
   
   // methods to convert between this CoordinateSystem and MJ2000Eq
   virtual Rvector ToMJ2000Eq(const A1Mjd &epoch, const Rvector &inState, 
                              bool coincident = false,
                              bool forceComputation = false); 

   virtual void    ToMJ2000Eq(const A1Mjd &epoch, const Real *inState, 
                              Real *outState, bool coincident = false,
                              bool forceComputation = false); 
   
   virtual Rvector FromMJ2000Eq(const A1Mjd &epoch, const Rvector &inState, 
                                bool coincident = false,
                                bool forceComputation = false); 
   virtual void    FromMJ2000Eq(const A1Mjd &epoch, const Real *inState, 
                                Real *outState, bool coincident = false,
                                bool forceComputation = false); 
   
   // all classes derived from GmatBase must supply this Clone method;
   // this must be implemented in the 'leaf' classes
   virtual GmatBase*    Clone() const;
   virtual void         Copy(const GmatBase* orig);
   
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   
   // Parameter access methods - overridden from GmatBase 
   virtual bool         IsParameterReadOnly(const Integer id) const;
   virtual bool         IsParameterReadOnly(const wxString &label) const;
   virtual wxString  GetParameterText(const Integer id) const;     
   virtual Integer      GetParameterID(const wxString &str) const; 
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   virtual Real         GetRealParameter(const Integer id) const;
   virtual Real         SetRealParameter(const Integer id,
                                         const Real value);
   virtual Real         GetRealParameter(const wxString &label) const;
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value);
   wxString          GetStringParameter(const Integer id) const;
   wxString          GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value);
   virtual bool         SetStringParameter(const wxString &label, 
                                           const wxString &value);
   virtual bool         GetBooleanParameter(const Integer id) const; 
   virtual bool         GetBooleanParameter(const wxString &label) const; 
   virtual bool         SetBooleanParameter(const Integer id,
                                            const bool value); 
   virtual bool         SetBooleanParameter(const wxString &label,
                                            const bool value);
   
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name);
   virtual bool         IsOwnedObject(Integer id) const;
   virtual GmatBase*    GetOwnedObject(Integer whichOne);
   virtual bool         HasRefObjectTypeArray();
   virtual const ObjectTypeArray&
                        GetRefObjectTypeArray();
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name = wxT(""));
   
   static CoordinateSystem* CreateLocalCoordinateSystem(
                            const wxString &csName, const wxString &axesType,
                            SpacePoint *origin, SpacePoint *primary,
                            SpacePoint *secondary, SpacePoint *j2000Body,
                            SolarSystem *solarSystem);
   
protected:

   enum
   {
      AXES = CoordinateBaseParamCount,
      UPDATE_INTERVAL,
      OVERRIDE_ORIGIN_INTERVAL,
      //INTERNAL_STATE,   // currently, no access allowed

      // owned object parameters
      EPOCH,
      CoordinateSystemParamCount
   };
   
   static const wxString PARAMETER_TEXT[
      CoordinateSystemParamCount - CoordinateBaseParamCount];
   
   static const Gmat::ParameterType PARAMETER_TYPE[
      CoordinateSystemParamCount - CoordinateBaseParamCount];
   
   virtual bool TranslateToMJ2000Eq(const A1Mjd &epoch, const Rvector &inState, 
                                    Rvector &outState);
   virtual bool TranslateToMJ2000Eq(const A1Mjd &epoch, const Real *inState, 
                                    Real *outState);
   // Rvector &outState, SpacePoint *j2000Body);
   
   virtual bool TranslateFromMJ2000Eq(const A1Mjd &epoch, const Rvector &inState, 
                                      Rvector &outState);
   virtual bool TranslateFromMJ2000Eq(const A1Mjd &epoch, const Real *inState, 
                                      Real *outState);
   // Rvector &outState, SpacePoint *j2000Body);
   
   /// axis system
   AxisSystem    *axes;
   
};
#endif // CoordinateSystem_hpp
