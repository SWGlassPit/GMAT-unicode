//$Id: ConfigManager.hpp 9840 2011-09-07 00:20:57Z djcinsb $
//------------------------------------------------------------------------------
//                                ConfigManager
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Darrel J. Conway
// Created: 2003/10/27
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
/**
 * Configuration manager used to manage configured (i.e. named) GMAT objects.
 */
//------------------------------------------------------------------------------

#ifndef ConfigManager_hpp
#define ConfigManager_hpp

#include <vector>
#include <map>

#include "ODEModel.hpp"
#include "Subscriber.hpp"
#include "SolarSystem.hpp"
#include "CelestialBody.hpp"
#include "PropSetup.hpp"
#include "Spacecraft.hpp"
#include "StopCondition.hpp"
#include "PhysicalModel.hpp"
#include "Propagator.hpp"
#include "Parameter.hpp"
#include "GmatCommand.hpp"
#include "Burn.hpp"
#include "Solver.hpp"
#include "AtmosphereModel.hpp"
#include "Function.hpp"
#include "Hardware.hpp"
#include "CoordinateSystem.hpp"
#include "CalculatedPoint.hpp"

class MeasurementModel;
class CoreMeasurement;
class DataFile;
class ObType;
class TrackingSystem;
class TrackingData;
class EventLocator;


/**
 * Class used to manage configured objects prior to cloning into the Sandbox.
 */
class GMAT_API ConfigManager
{
public:
   static ConfigManager*   Instance();
   
   wxString         GetNewName(const wxString &name, Integer startCount);
   
   void                AddObject(Gmat::ObjectType objType, GmatBase *obj);
   wxString         AddClone(const wxString &name);
   void                AddPhysicalModel(PhysicalModel *pm);
   void                AddPropagator(Propagator *prop);
   void                AddODEModel(ODEModel *fm);
   void                AddSubscriber(Subscriber *subs);
   void                AddSolarSystem(SolarSystem *solarSys);
   void                AddPropSetup(PropSetup *propSetup);
   void                AddSpacecraft(SpaceObject *sc);
   void                AddSpacePoint(SpacePoint *sp);
   void                AddHardware(Hardware *hw);
   void                AddStopCondition(StopCondition* stopCond);
   void                AddParameter(Parameter* parameter);
   void                AddBurn(Burn* burn);
   void                AddSolver(Solver *solver);
   void                AddAtmosphereModel(AtmosphereModel *atmosModel);
   void                AddFunction(Function *function);
   void                AddCoordinateSystem(CoordinateSystem *cs);
   void                AddCalculatedPoint(CalculatedPoint *cp);
   
   void                SetDefaultSolarSystem(SolarSystem *ss);
   void                SetSolarSystemInUse(SolarSystem *ss);
   bool                SetSolarSystemInUse(const wxString &name);

   void                AddMeasurementModel(MeasurementModel *mModel);
   void                AddMeasurement(CoreMeasurement *meas);
   void                AddDataFile(DataFile *meas);
   void                AddObType(ObType *meas);
   void                AddEventLocator(EventLocator *el);
   void                AddTrackingSystem(TrackingSystem *ts);
   void                AddTrackingData(TrackingData *td);

   const StringArray&  GetListOfAllItems();
   const StringArray&  GetListOfItems(Gmat::ObjectType itemType);
   const StringArray&  GetListOfItems(const wxString &typeName);
   const StringArray&  GetListOfItemsHas(Gmat::ObjectType type,
                                         const wxString &name,
                                         bool includeSysParam = true);
   GmatBase*           GetFirstItemUsing(Gmat::ObjectType type,
                                         const wxString &name,
                                         bool includeSysParam = true);
   GmatBase*           GetItem(const wxString &name);
   
   bool                RenameItem(Gmat::ObjectType itemType,
                                  const wxString &oldName,
                                  const wxString &newName);
   
   bool                RemoveAllItems();
   bool                RemoveItem(Gmat::ObjectType type, const wxString &name);
   bool                ReconfigureItem(GmatBase *newobj, const wxString &name);
   
   
   PhysicalModel*      GetPhysicalModel(const wxString &name);
   Propagator*         GetPropagator(const wxString &name);
   ODEModel*           GetODEModel(const wxString &name);
   SpaceObject*        GetSpacecraft(const wxString &name);
   SpacePoint*         GetSpacePoint(const wxString &name);
   Hardware*           GetHardware(const wxString &name);
   PropSetup*          GetPropSetup(const wxString &name);
   Subscriber*         GetSubscriber(const wxString &name);
   SolarSystem*        GetDefaultSolarSystem();
   SolarSystem*        GetSolarSystemInUse();
   SolarSystem*        GetSolarSystemInUse(const wxString &name);
   StopCondition*      GetStopCondition(const wxString &name);
   Parameter*          GetParameter(const wxString &name);
   Burn*               GetBurn(const wxString &name);
   Solver*             GetSolver(const wxString &name);
   AtmosphereModel*    GetAtmosphereModel(const wxString &name);
   Function*           GetFunction(const wxString &name);
   CoordinateSystem*   GetCoordinateSystem(const wxString &name);
   CalculatedPoint*    GetCalculatedPoint(const wxString &name);
   MeasurementModel*   GetMeasurementModel(const wxString &name);
   TrackingSystem*     GetTrackingSystem(const wxString &name);
   TrackingData*       GetTrackingData(const wxString &name);

   DataFile *          GetDataStream(const wxString &name);

   EventLocator*       GetEventLocator(const wxString &name);

   bool                HasConfigurationChanged();
   void                ConfigurationChanged(bool tf);
   ObjectMap*          GetObjectMap();
   
private:
   
   /// The singleton instance
   static ConfigManager*               theConfigManager;
   /// The managed objects
   std::vector<GmatBase*>              objects;
   std::vector<GmatBase*>              newObjects;
   /// A list of the names of the managed objects
   StringArray                         listOfItems;
   /// Mapping between the object names and their pointers
   std::map<wxString, GmatBase *>   mapping;
   /// Flag indicating that a managed object has been changed by a user
   bool                                objectChanged;
   
   // Treat default and in use solar system separately until we can manage solar 
   // system by name.  All solar system names are "SolarSystem" for now.
   
   /// Default Solar Systems 
   SolarSystem *defaultSolarSystem;
   /// Solar Systems in use
   SolarSystem *solarSystemInUse;
   
   void                AddObject(GmatBase* obj);
   
   // Hide the default constructor and destructor to preserve singleton status
   ConfigManager();
   ~ConfigManager();
   
};


#endif // ConfigManager_hpp

