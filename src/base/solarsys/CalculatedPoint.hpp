//$Id: CalculatedPoint.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  CalculatedPoint
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
// Author: Wendy C. Shoan
// Created: 2005.04.01 (no foolin')
//
/**
 * This is the base class for calculated points.
 *
 * @note This is an abstract class.
 */
//------------------------------------------------------------------------------

#ifndef CalculatedPoint_hpp
#define CalculatedPoint_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "SpacePoint.hpp"
#include "A1Mjd.hpp"
#include "Rmatrix.hpp"
#include "Rvector6.hpp"
#include "TimeTypes.hpp"

/**
 * CalculatedPoint base class, from which all types of calculated points 
 * will derive.
 *
 * The CalculatedPoint class is primarily an intermediate base class, from which
 * all types of calculated points will derive.  CalculatedPoint itself derives 
 * from SpacePoint.
 *
 * @note Bodies are sent to an (sub)object of this class via the SetRefObject
 *       method.
 */
class GMAT_API CalculatedPoint : public SpacePoint
{
public:
   // default constructor, specifying calculated point type and name
   CalculatedPoint(const wxString &ptType = wxT("LibrationPoint"), 
                   const wxString &itsName = wxT(""));
   // copy constructor
   CalculatedPoint(const CalculatedPoint &cp);
   // operator=
   CalculatedPoint& operator=(const CalculatedPoint &cp);
   // destructor
   virtual ~CalculatedPoint();
   
   // methods inherited from SpacePoint, that must be implemented here (and/or
   // in the derived classes (--> in derived classes)
   //virtual const Rvector6           GetMJ2000State(const A1Mjd &atTime);
   //virtual const Rvector3           GetMJ2000Position(const A1Mjd &atTime);
   //virtual const Rvector3           GetMJ2000Velocity(const A1Mjd &atTime);   
   
   // Parameter access methods - overridden from GmatBase
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const; 
   virtual Gmat::ParameterType 
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   virtual bool         IsParameterReadOnly(const Integer id) const;
   virtual bool         IsParameterReadOnly(const wxString &label) const;
   
   virtual Integer      GetIntegerParameter(const Integer id) const;
   virtual Integer      GetIntegerParameter(const wxString &label) const;
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual wxString  GetStringParameter(const Integer id,
                                           const Integer index) const;
   virtual wxString  GetStringParameter(const wxString &label,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value);
   virtual bool         SetStringParameter(const wxString &label, 
                                           const wxString &value);
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value,
                                           const Integer index);
   virtual bool         SetStringParameter(const wxString &label, 
                                           const wxString &value,
                                           const Integer index);
   
   virtual const StringArray&  
                        GetStringArrayParameter(const Integer id) const;
   virtual const StringArray&  
                        GetStringArrayParameter(const wxString &label) const;
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name,
                                     const Integer index);
   virtual bool         SetRefObject(GmatBase *obj, 
                                     const Gmat::ObjectType type,
                                     const wxString &name = wxT(""));
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   virtual bool         HasRefObjectTypeArray();
   virtual const ObjectTypeArray&
                        GetRefObjectTypeArray();
   virtual const StringArray& 
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool         TakeAction(const wxString &action,  
                                   const wxString &actionData = wxT(""));
   virtual bool         TakeRequiredAction(const Integer id);

   
   virtual void         SetDefaultBody(const wxString &defBody);
   virtual const StringArray&
                        GetDefaultBodies() const;

protected:
   
   enum
   {
      NUMBER_OF_BODIES = SpacePointParamCount, 
      BODY_NAMES,
      CalculatedPointParamCount
   };
   
   static const wxString
      PARAMETER_TEXT[CalculatedPointParamCount - SpacePointParamCount];
   static const Gmat::ParameterType
      PARAMETER_TYPE[CalculatedPointParamCount - SpacePointParamCount];
   
   /// number of bodies
   Integer                  numberOfBodies;
   /// list of bodies
   std::vector<SpacePoint*> bodyList;
   /// list of body names
   StringArray              bodyNames;

   // names of the default bodies to use
   StringArray defaultBodies;
    
private:
      
   //------------------------------------------------------------------------------
   //  void CheckBodies()
   //------------------------------------------------------------------------------
   /**
    * Abstract method for derived classes that checks to make sure the bodyList has
    * been defined appropriately (i.e. all are of a tyep that makes sense).
    *
    */
    //------------------------------------------------------------------------------
   virtual void CheckBodies() = 0;      
};
#endif // CalculatedPoint_hpp

