//$Id: AxisSystem.hpp 9761 2011-08-15 18:48:03Z wendys-dev $
//------------------------------------------------------------------------------
//                                  AxisSystem
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
// Created: 2004/12/20
//
/**
 * Definition of the AxisSystem class.  This is the base class for the 
 * InertialAxes and DynamicAxes classes.
 *
 */
//------------------------------------------------------------------------------

#ifndef AxisSystem_hpp
#define AxisSystem_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "CoordinateBase.hpp"
#include "Rmatrix33.hpp"
#include "A1Mjd.hpp"
#include "EopFile.hpp"
#include "ItrfCoefficientsFile.hpp"

class GMAT_API AxisSystem : public CoordinateBase
{
public:

   // default constructor
   AxisSystem(const wxString &itsType,
              const wxString &itsName = wxT(""));
   // copy constructor
   AxisSystem(const AxisSystem &axisSys);
   // operator = for assignment
   const AxisSystem& operator=(const AxisSystem &axisSys);
   // destructor
   virtual ~AxisSystem();

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
   // methods to set the files to use - for those AxisSystems that 
   // need all or part of the FK5 reduction
   virtual void                  SetEopFile(EopFile *eopF);
   virtual void                  SetCoefficientsFile(ItrfCoefficientsFile *itrfF);
   virtual void                  SetEpochFormat(const wxString &fmt);  // for GUI
   virtual SpacePoint*           GetPrimaryObject() const;
   virtual SpacePoint*           GetSecondaryObject() const;
   virtual A1Mjd                 GetEpoch() const;
   virtual wxString           GetXAxis() const;
   virtual wxString           GetYAxis() const;
   virtual wxString           GetZAxis() const;
   virtual EopFile*              GetEopFile() const;
   virtual ItrfCoefficientsFile* GetItrfCoefficientsFile();
   virtual wxString           GetEpochFormat() const; // for GUI
   virtual Rmatrix33             GetLastRotationMatrix() const;
   virtual void                  GetLastRotationMatrix(Real *mat) const;
   virtual Rmatrix33             GetLastRotationDotMatrix() const;
   virtual void                  GetLastRotationDotMatrix(Real *mat) const;
   
   virtual void                  SetCoordinateSystemName(const wxString &csName);

   // initializes the AxisSystem
   virtual bool Initialize();
   
   // methods to convert to/from the MJ2000 Equatorial axis system
   virtual bool RotateToMJ2000Eq(const A1Mjd &epoch, const Rvector &inState,
                                 Rvector &outState, 
                                 bool forceComputation = false); 
   virtual bool RotateToMJ2000Eq(const A1Mjd &epoch, const Real *inState,
                                 Real *outState,
                                 bool forceComputation = false); 
   virtual bool RotateFromMJ2000Eq(const A1Mjd &epoch, const Rvector &inState,
                                   Rvector &outState,
                                   bool forceComputation = false); 
   virtual bool RotateFromMJ2000Eq(const A1Mjd &epoch, const Real *inState,
                                   Real *outState,
                                   bool forceComputation = false); 
   
   
   // all classes derived from GmatBase must supply this Clone method;
   // this must be implemented in the 'leaf' classes
   //virtual GmatBase*       Clone(void) const;

   // Parameter access methods - overridden from GmatBase - may need these later??
   virtual wxString     GetParameterText(const Integer id) const;     
   virtual Integer         GetParameterID(const wxString &str) const; 
   virtual Gmat::ParameterType
                           GetParameterType(const Integer id) const;
   virtual wxString     GetParameterTypeString(const Integer id) const;
   virtual bool            IsParameterReadOnly(const Integer id) const;
   virtual Real            GetRealParameter(const Integer id) const;
   virtual Real            SetRealParameter(const Integer id,
                                            const Real value);
   virtual Real            GetRealParameter(const wxString &label) const;
   virtual Real            SetRealParameter(const wxString &label,
                                            const Real value);
   virtual bool            GetBooleanParameter(const Integer id) const; 
   virtual bool            GetBooleanParameter(const wxString &label) const; 
   virtual bool            SetBooleanParameter(const Integer id,
                                               const bool value); 
   virtual bool            SetBooleanParameter(const wxString &label,
                                               const bool value);
   
   // currently, no access to RotMatrix and RotDotMatrix allowed
   
protected:

   enum
   {
      EPOCH = CoordinateBaseParamCount,
      UPDATE_INTERVAL, 
      OVERRIDE_ORIGIN_INTERVAL,
      AxisSystemParamCount
   };
   
   static const wxString PARAMETER_TEXT[AxisSystemParamCount - CoordinateBaseParamCount];
   
   static const Gmat::ParameterType PARAMETER_TYPE[AxisSystemParamCount - CoordinateBaseParamCount];
   
   //---------------------------------------------------------------------------
   //  void CalculateRotationMatrix(const A1Mjd &atEpoch)
   //---------------------------------------------------------------------------
   /**
    * This method will compute the rotMatrix and rotDotMatrix used for rotations
    * from/to this AxisSystem to/from the MJ2000EqAxes system.
    *
    * @param atEpoch  epoch at which to compute the roration matrix
    */
   //---------------------------------------------------------------------------
   virtual void CalculateRotationMatrix(const A1Mjd &atEpoch,
                                        bool forceComputation = false)  
                                        = 0;
   
   /// rotation matrix - 
   /// default constructor creates a 3x3 zero-matrix
   Rmatrix33   rotMatrix;
   /// derivative of rotation matrix - 
   /// default constructor creates a 3x3 zero-matrix
   Rmatrix33   rotDotMatrix;
   /// epoch
   A1Mjd epoch;
   /// Name of the coordinate system
   wxString coordName;
   
   
   const Real *rotData;
   const Real *rotDotData;

   // data and methods for those AxisSystems that need all or part of the FK5 
   // reduction
   static const Real  JD_OF_JANUARY_1_1997;
   static const Real  DETERMINANT_TOLERANCE;

   EopFile                   *eop;
   ItrfCoefficientsFile      *itrf;
   
   wxString               epochFormat;
   
   Real                      updateInterval;
   Real                      updateIntervalToUse;
   bool                      overrideOriginInterval;
   A1Mjd                     lastPRECEpoch;
   A1Mjd                     lastNUTEpoch;
   A1Mjd                     lastSTDerivEpoch;
   A1Mjd                     lastPMEpoch;
   Rmatrix33                 lastPREC;
   Rmatrix33                 lastNUT;
   Rmatrix33                 lastSTDeriv;
   Rmatrix33                 lastPM;
      
   Real                      lastDPsi; 
   
   GmatItrf::NutationTerms   nutationSrc;
   GmatItrf::PlanetaryTerms  planetarySrc; 
   
   std::vector<IntegerArray> a, ap;
   Rvector                   A, B, C, D, E, F, Ap, Bp, Cp, Dp;
   
   Integer                   *aVals;
   Integer                   *apVals;
   
   // Performance enhancements
   Rmatrix33 PREC;
   Rmatrix33 NUT;
   Rmatrix33 ST;
   Rmatrix33 STderiv;
   Rmatrix33 PM;
   
   // added for performance
   const Real *precData;
   const Real *nutData;
   const Real *stData;
   const Real *stDerivData;
   const Real *pmData;
   
   
   const Real  *AVals;
   const Real  *BVals;
   const Real  *CVals;
   const Real  *DVals;
   const Real  *EVals;
   const Real  *FVals;
   const Real  *ApVals;
   const Real  *BpVals;
   const Real  *CpVals;
   const Real  *DpVals;
   
   // intermediate quantities needed by more than one method
   
   virtual void      InitializeFK5();

   virtual void ComputePrecessionMatrix(const Real tTDB, A1Mjd atEpoch);
   virtual void ComputeNutationMatrix(const Real tTDB, A1Mjd atEpoch, 
                                           Real &dPsi,
                                           Real &longAscNodeLunar,
                                           Real &cosEpsbar,
                                           bool forceComputation = false);
   virtual void ComputeSiderealTimeRotation(const Real jdTT,
                                                 const Real tUT1,
                                                 Real dPsi,
                                                 Real longAscNodeLunar,
                                                 Real cosEpsbar,
                                                 Real &cosAst,
                                                 Real &sinAst);
   virtual void ComputeSiderealTimeDotRotation(const Real mjdUTC, A1Mjd atEpoch,
                                                    Real cosAst, Real sinAst,
                                                    bool forceComputation = false);
   virtual void ComputePolarMotionRotation(const Real mjdUTC, A1Mjd atEpoch,
                                            bool forceComputation = false);
};
#endif // AxisSystem_hpp
