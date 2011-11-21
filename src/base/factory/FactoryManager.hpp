//$Id: FactoryManager.hpp 9840 2011-09-07 00:20:57Z djcinsb $
//------------------------------------------------------------------------------
//                             FactoryManager
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
// Created: 2003/08/27
//
/**
 * This class is the interface between the Moderator and Factories.  It is a
 * singleton - only one instance of this class can be created.
 */
//------------------------------------------------------------------------------
#ifndef FactoryManager_hpp
#define FactoryManager_hpp

#include <list>
#include <string>
#include "gmatdefs.hpp"
#include "Factory.hpp"
#include "Spacecraft.hpp"
#include "Parameter.hpp"
#include "Propagator.hpp"
#include "ODEModel.hpp"
#include "PhysicalModel.hpp"
#include "PropSetup.hpp"
#include "StopCondition.hpp"
#include "CalculatedPoint.hpp"
#include "CelestialBody.hpp"
#include "SolarSystem.hpp"
#include "Solver.hpp"
#include "Subscriber.hpp"
#include "GmatCommand.hpp"
#include "Burn.hpp"
#include "AtmosphereModel.hpp"
#include "Function.hpp"
#include "AxisSystem.hpp"
#include "CoordinateSystem.hpp"
#include "MathNode.hpp"
#include "Attitude.hpp"

class MeasurementModel;
class CoreMeasurement;
class DataFile;
class ObType;
class TrackingSystem;
class TrackingData;
class EphemerisFile;
class Interface;
class EventLocator;

/**
 * GMAT Factory Manager Class, the interface between the Moderator and the
 * factories.
 *
 * The Factory Manager manages all of the factories that are needed for
 * execution of the system for a particular run.  Each factory must be
 * registered with the FactoryManager in order for its objects to be able to be
 * created.  The Moderator will register all of the predefined factories;
 * user-defined factories must be registered as they are added to the system.
 */
class GMAT_API FactoryManager
{
public:
   // class instance method (this is a singleton class)
   static FactoryManager* Instance();
   // all factories must be registered via this method
   bool                   RegisterFactory(Factory* fact);

   // Generic method to create an object
   GmatBase*              CreateObject(const Gmat::ObjectType generalType,
                                       const wxString &ofType,
                                       const wxString &withName = wxT(""));
   
   // methods to create and return objects of the various types
   SpaceObject*           CreateSpacecraft(const wxString &ofType,
                                           const wxString &withName = wxT(""));
   Parameter*             CreateParameter(const wxString &ofType,
                                          const wxString &withName = wxT(""));
   Propagator*            CreatePropagator(const wxString &ofType,
                                           const wxString &withName = wxT(""));
   PhysicalModel*         CreatePhysicalModel(const wxString &ofType,
                                              const wxString &withName = wxT(""));
   StopCondition*         CreateStopCondition(const wxString &ofType,
                                              const wxString &withName = wxT(""));
   CalculatedPoint*       CreateCalculatedPoint(const wxString &ofType,
                                                const wxString &withName = wxT(""));
   CelestialBody*         CreateCelestialBody(const wxString &ofType,
                                              const wxString &withName = wxT(""));
   Solver*                CreateSolver(const wxString &ofType,
                                       const wxString &withName = wxT(""));
   Subscriber*            CreateSubscriber(const wxString &ofType,
                                           const wxString &withName = wxT(""),
                                           const wxString &fileName = wxT(""));
   EphemerisFile*         CreateEphemerisFile(const wxString &ofType,
                                              const wxString &withName = wxT(""));
   GmatCommand*           CreateCommand(const wxString &ofType,
                                        const wxString &withName = wxT(""));
   Burn*                  CreateBurn(const wxString &ofType,
                                     const wxString &withName = wxT(""));
   AtmosphereModel*       CreateAtmosphereModel(const wxString &ofType,
                                                const wxString &withName = wxT(""),
                                                const wxString &forBody = wxT("Earth"));
   Function*              CreateFunction(const wxString &ofType,
                                         const wxString &withName = wxT(""));
   Hardware*              CreateHardware(const wxString &ofType,
                                         const wxString &withName = wxT(""));
   AxisSystem*            CreateAxisSystem(const wxString &ofType,
                                           const wxString &withName = wxT(""));
   MathNode*              CreateMathNode(const wxString &ofType,
                                         const wxString &withName = wxT(""));
   Attitude*              CreateAttitude(const wxString &ofType,
                                         const wxString &withName = wxT(""));
   SpacePoint*            CreateSpacePoint(const wxString &ofType,
                                           const wxString &withName = wxT(""));

   CoreMeasurement*       CreateMeasurement(const wxString &ofType,
                                            const wxString &withName = wxT(""));

   ObType*                CreateObType(const wxString &ofType,
                                       const wxString &withName = wxT(""));
   
   Interface*             CreateInterface(const wxString &ofType,
                                          const wxString &withName = wxT(""));
   
   //----- Just container
   SolarSystem*           CreateSolarSystem(const wxString &withName = wxT(""));
   PropSetup*             CreatePropSetup(const wxString &withName = wxT(""));
   ODEModel*              CreateODEModel(const wxString &ofType,
                                         const wxString &withName = wxT(""));
   CoordinateSystem*      CreateCoordinateSystem(const wxString &withName = wxT(""));

   MeasurementModel*      CreateMeasurementModel(const wxString &withName);
   DataFile*              CreateDataFile(const wxString &ofType,
                                         const wxString &withName);
   TrackingSystem*        CreateTrackingSystem(const wxString &ofType,
                                               const wxString &withName);
   TrackingData*          CreateTrackingData(const wxString &withName = wxT(""));
   EventLocator*          CreateEventLocator(const wxString &ofType,
                                             const wxString &withName = wxT(""));

   // method to return a list of strings representing the objects of the input
   // type that may be created in the system
   const StringArray&     GetListOfItems(Gmat::ObjectType byType, 
                                const wxString &withQualifier = wxT(""));
   const StringArray&     GetListOfAllItems();
   const StringArray&     GetListOfAllItemsExcept(const ObjectTypeArray &types);
   const StringArray&     GetListOfViewableItems(Gmat::ObjectType byType);
   const StringArray&     GetListOfUnviewableItems(Gmat::ObjectType byType);
   
   bool                   DoesObjectTypeMatchSubtype(
                                const Gmat::ObjectType coreType,
                                const wxString &theType,
                                const wxString &theSubtype);

   // method to return the base type for the input string
   Gmat::ObjectType       GetBaseTypeOf(const wxString &typeName);
   
protected:
   StringArray            entireList;
   
private:

   // private class data
   /// the list of factories that have been registered and which are available
   /// to create objects
   std::list<Factory*> factoryList;
   /// the list of object types that factory can create
   std::list<Gmat::ObjectType> factoryTypeList;
   /// pointer to the only instance allowed for this singleton class
   static FactoryManager* onlyInstance;
   
   // private methods 
   Factory*               FindFactory(Gmat::ObjectType ofType, const wxString &forType);
   const StringArray&     GetList(Gmat::ObjectType ofType, const wxString &withQualifier);
   const StringArray&     GetListOfViewables(Gmat::ObjectType ofType);
   const StringArray&     GetListOfUnviewables(Gmat::ObjectType ofType);
   
   // Hide the default constructor and destructor to preserve singleton status
   // default constructor
   FactoryManager();
   // class destructor
   ~FactoryManager();
};

#endif // FactoryManager_hpp
