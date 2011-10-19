//$Id: Factory.hpp 9838 2011-09-03 00:40:26Z djcinsb $
//------------------------------------------------------------------------------
//                             Factory
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
// Author: Wendy Shoan
// Created: 2003/08/28
//
/**
 *  This class is the base class for the factories.  Derived classes will be
 *  responsible for creating objects of a specific type.
 */
//------------------------------------------------------------------------------
#ifndef Factory_hpp
#define Factory_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"

// Forward references
class Spacecraft;
class SpaceObject;
class Parameter;
class Burn;
class Propagator;
class ODEModel;
class PhysicalModel;
class PropSetup;
class StopCondition;
class CalculatedPoint;
class CelestialBody;
class SolarSystem;
class Solver;
class Subscriber;
class EphemerisFile;
class GmatCommand;
class AtmosphereModel;
class Function;
class Hardware;
class AxisSystem;
class CoordinateSystem;
class MathNode;
class Attitude;
class SpacePoint;
class Event;
class EventLocator;
class Interface;

class MeasurementModel;
class CoreMeasurement;
class DataFile;
class ObType;
class TrackingSystem;
class TrackingData;


/// @todo Find a clever way to allow user types here when we don't know them

class GMAT_API Factory
{
public:
   // method to return objects as generic type
   virtual GmatBase*        CreateObject(const wxString &ofType,
                                         const wxString &withName = wxT(""));        

   // methods to return objects of specified types
   virtual SpaceObject*     CreateSpacecraft(const wxString &ofType,
                                             const wxString &withName = wxT(""));
   virtual SpacePoint*      CreateSpacePoint(const wxString &ofType,
                                             const wxString &withName = wxT(""));        
   virtual Propagator*      CreatePropagator(const wxString &ofType,
                                             const wxString &withName = wxT(""));
   virtual ODEModel*        CreateODEModel(const wxString &ofType,
                                             const wxString &withName = wxT(""));
   virtual PhysicalModel*   CreatePhysicalModel(const wxString &ofType,
                                                const wxString &withName = wxT(""));
   virtual PropSetup*       CreatePropSetup(const wxString &ofType,
                                            const wxString &withName = wxT(""));
   virtual Parameter*       CreateParameter(const wxString &ofType,
                                            const wxString &withName = wxT(""));
   virtual Burn*            CreateBurn(const wxString &ofType,
                                       const wxString &withName = wxT(""));
   virtual StopCondition*   CreateStopCondition(const wxString &ofType,
                                                const wxString &withName = wxT(""));
   virtual CalculatedPoint* CreateCalculatedPoint(const wxString &ofType,
                                                  const wxString &withName = wxT(""));
   virtual CelestialBody*   CreateCelestialBody(const wxString &ofType,
                                                const wxString &withName = wxT(""));
   virtual SolarSystem*     CreateSolarSystem(const wxString &ofType,
                                              const wxString &withName = wxT(""));
   virtual Solver*          CreateSolver(const wxString &ofType,
                                         const wxString &withName = wxT("")); 
   virtual Subscriber*      CreateSubscriber(const wxString &ofType,
                                             const wxString &withName = wxT(""),
                                             const wxString &fileName = wxT(""));
   virtual EphemerisFile*   CreateEphemerisFile(const wxString &ofType,
                                                const wxString &withName = wxT(""));
   virtual GmatCommand*     CreateCommand(const wxString &ofType,
                                          const wxString &withName = wxT(""));
   virtual AtmosphereModel* CreateAtmosphereModel(const wxString &ofType,
                                                  const wxString &withName = wxT(""),
                                                  const wxString &forBody = wxT("Earth"));
   virtual Function*        CreateFunction(const wxString &ofType,
                                           const wxString &withName = wxT(""));
   virtual Hardware*        CreateHardware(const wxString &ofType,
                                           const wxString &withName = wxT(""));
   virtual AxisSystem*      CreateAxisSystem(const wxString &ofType,
                                             const wxString &withName = wxT(""));
   virtual CoordinateSystem* CreateCoordinateSystem(const wxString &ofType,
                                                    const wxString &withName = wxT(""));
   virtual MathNode*        CreateMathNode(const wxString &ofType,
                                           const wxString &withName = wxT(""));
   virtual Attitude*        CreateAttitude(const wxString &ofType,
                                           const wxString &withName = wxT(""));
   virtual MeasurementModel*
                            CreateMeasurementModel(const wxString &ofType,
                                           const wxString &withName = wxT(""));
   virtual CoreMeasurement* CreateMeasurement(const wxString &ofType,
                                           const wxString &withName = wxT(""));
   virtual DataFile*        CreateDataFile(const wxString &ofType,
                                           const wxString &withName = wxT(""));
   virtual ObType*          CreateObType(const wxString &ofType,
                                         const wxString &withName = wxT(""));
   virtual TrackingSystem*  CreateTrackingSystem(const wxString &ofType,
                                           const wxString &withName = wxT(""));
   virtual TrackingData*    CreateTrackingData(const wxString &ofType,
                                           const wxString &withName = wxT(""));
   virtual Event*           CreateEvent(const wxString &ofType,
                                        const wxString &withName = wxT(""));
   virtual EventLocator*    CreateEventLocator(const wxString &ofType,
                                        const wxString &withName = wxT(""));
   virtual Interface*       CreateInterface(const wxString &ofType,
                                            const wxString &withName = wxT(""));
   
   // method to return list of types of objects that this factory can create
   virtual StringArray      GetListOfCreatableObjects(
                                  const wxString &qualifier = wxT("")) const;
   // method to check if a createable object type matches a subtype
   virtual bool             DoesObjectTypeMatchSubtype(
                                  const wxString &theType,
                                  const wxString &theSubtype);
   // method to return list of objects that can be viewed via GUI of this factory
   StringArray              GetListOfViewableObjects();
   // method to return list of objects that cannot be viewed via GUI of this factory
   StringArray              GetListOfUnviewableObjects() const;
   // method to set the types of objects that this factory can create
   bool                     SetListOfCreatableObjects(StringArray newList);
   // method to add types of objects that this factory can create
   bool                     AddCreatableObjects(StringArray newList);
   
   // method to return the type of factory this is
   Gmat::ObjectType         GetFactoryType() const;  
   bool                     IsTypeCaseSensitive() const;
   
   // destructor
   virtual ~Factory();

protected:
   // constructor specifying the type of objects creatable by the factory
   Factory(Gmat::ObjectType ofType = Gmat::UNKNOWN_OBJECT);
   // constructor specifying the type of objects creatable by the factory and the
   // specific types that this factory can create
   Factory(StringArray createList, Gmat::ObjectType ofType = Gmat::UNKNOWN_OBJECT);
   // copy constructor
   Factory(const Factory& fact);
   // assignment operator
   Factory& operator= (const Factory& fact);

   // protected data
   // the type of the factory (i.e. what type of objects it can create)
   Gmat::ObjectType         itsType;
   // a list of all of the specific types of objects (of type itsType) that
   // can be created by this factory.
   StringArray              creatables;
   // a list of qualified creatable objects thatcan be created by this factory.
   StringArray              qualifiedCreatables;
   // a list of all of the types of objects that can be viewed from the GUI
   // (This is automacally generated)
   StringArray              viewables;
   // a list of all of the types of objects that cannot be viewed from the GUI
   StringArray              unviewables;
   // is type name case sensitive
   bool                     isCaseSensitive;

private:
    
};

#endif // Factory_hpp
