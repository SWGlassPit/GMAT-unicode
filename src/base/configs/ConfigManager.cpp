//$Id: ConfigManager.cpp 9513 2011-04-30 21:23:06Z djcinsb $
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


#include "ConfigManager.hpp"
#include "ConfigManagerException.hpp"
#include "StringUtil.hpp"               // for GmatStringUtil::

//#define DEBUG_RENAME 1
//#define DEBUG_CONFIG 1
//#define DEBUG_CONFIG_SS 1
//#define DEBUG_CONFIG_ADD_CLONE 1
//#define DEBUG_CONFIG_REMOVE
//#define DEBUG_CONFIG_REMOVE_MORE
//#define DEBUG_CONFIG_OBJ_USING

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//---------------------------------
// static members
//---------------------------------


ConfigManager* ConfigManager::theConfigManager = NULL;


//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// ConfigManager* Instance()
//------------------------------------------------------------------------------
/**
 * Accessor method used to obtain the singleton.
 *
 * @return the singleton instance of the configuration manager.
 */
//------------------------------------------------------------------------------
ConfigManager* ConfigManager::Instance()
{
   if (!theConfigManager)
      theConfigManager = new ConfigManager;
        
   return theConfigManager;
}


//------------------------------------------------------------------------------
// ConfigManager()
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
ConfigManager::ConfigManager() :
   objectChanged     (false)
{
}

// class destructor
//------------------------------------------------------------------------------
// ~ConfigManager()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
ConfigManager::~ConfigManager()
{
   RemoveAllItems();
}


//------------------------------------------------------------------------------
// wxString GetNewName(const wxString &name, Integer startCount)
//------------------------------------------------------------------------------
/*
 * It gives new name by adding counter to the input name.
 *
 * @param <name> Base name to used to generate new name
 * @param <startCount> Starting counter
 * @return new name
 */
//------------------------------------------------------------------------------
wxString ConfigManager::GetNewName(const wxString &name, Integer startCount)
{
   #if DEBUG_CONFIG_NEW_NAME
   MessageInterface::ShowMessage
      (wxT("ConfigManager::GetNewName() name = %s\n"), name.c_str());
   #endif
   
   if (name == wxT(""))
      return wxT("");
   
   // get initial new name
   Integer counter = 0;
   wxString baseName = GmatStringUtil::RemoveLastNumber(name, counter);
   if (counter == 0)
      counter = startCount;
   
   wxString newName = baseName + GmatStringUtil::ToString(counter, 1);
   
   // construct new name while it exists
   while (GetItem(newName))
   {
      ++counter;
      newName = baseName + GmatStringUtil::ToString(counter,1);
   }
   
   #if DEBUG_CONFIG_NEW_NAME
   MessageInterface::ShowMessage
      (wxT("ConfigManager::GetNewName() newName = %s\n"), newName.c_str());
   #endif
   
   return newName;
}


//------------------------------------------------------------------------------
// void AddObject(Gmat::ObjectType objType, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Adds an object to the configuration.
 *
 * @param obj Pointer to the object instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddObject(Gmat::ObjectType objType, GmatBase *obj)
{
   #ifdef DEBUG_ADD_OBJECT
   MessageInterface::ShowMessage
      (wxT("ConfigManager::AddObject() entered, objType=%d, obj=<%p>\n"), objType, obj);
   #endif
   wxString name = obj->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));
   
   if (!obj->IsOfType(objType))
      throw ConfigManagerException(name + wxT(" is not a valid object type"));
   
   AddObject(obj);
}

//------------------------------------------------------------------------------
// wxString AddClone(const wxString &name)
//------------------------------------------------------------------------------
/*
 * Adds the clone of the named object to configuration.
 * It gives new name by adding counter to the name to be cloned.
 *
 * return new name if object was cloned and added to configuration,
 *        blank otherwise
 */
//------------------------------------------------------------------------------
wxString ConfigManager::AddClone(const wxString &name)
{
   if (name == wxT(""))
      return wxT("");
   
   GmatBase *obj1 = GetItem(name);
   wxString newName = GetNewName(name, 2);
   
   GmatBase* obj2 = obj1->Clone();
   obj2->SetName(newName, newName);
   AddObject(obj2);
   
   #if DEBUG_CONFIG_ADD_CLONE
   MessageInterface::ShowMessage
      (wxT("ConfigManager::AddClone() newName = %s, type = %s\n"), obj2->GetName().c_str(),
       obj2->GetTypeName().c_str());
   #endif
   
   return newName;
}


//------------------------------------------------------------------------------
// void AddPhysicalModel(PhysicalModel *pm)
//------------------------------------------------------------------------------
/**
 * Adds a PhysicalModel to the configuration.
 *
 * @param pm Pointer to the PhysicalModel instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddPhysicalModel(PhysicalModel *pm)
{
   wxString name = pm->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!pm->IsOfType(Gmat::PHYSICAL_MODEL))
      throw ConfigManagerException(name + wxT(" is not a PhysicalModel"));

   AddObject(pm);
}

//------------------------------------------------------------------------------
// void ConfigManager::AddPropagator(Propagator *prop)
//------------------------------------------------------------------------------
/**
 * Adds a Propagator to the configuration.
 *
 * @param prop Pointer to the Propagator instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddPropagator(Propagator *prop)
{
   wxString name = prop->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!prop->IsOfType(Gmat::PROPAGATOR))
      throw ConfigManagerException(name + wxT(" is not a Propagator"));

   AddObject(prop);
}


//------------------------------------------------------------------------------
// void AddForceModel(ForceModel *fm)
//------------------------------------------------------------------------------
/**
 * Adds a ForceModel to the configuration.
 *
 * @param fm Pointer to the ForceModel instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddODEModel(ODEModel *fm)
{
   wxString name = fm->GetName();

   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!fm->IsOfType(Gmat::ODE_MODEL))
      throw ConfigManagerException(name + wxT(" is not a ForceModel"));

   AddObject(fm);
}


//------------------------------------------------------------------------------
// void AddSubscriber(Subscriber *subs)
//------------------------------------------------------------------------------
/**
 * Adds a Subscriber to the configuration.
 *
 * @param subs Pointer to the Subscriber.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddSubscriber(Subscriber *subs)
{
   wxString name = subs->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!subs->IsOfType(Gmat::SUBSCRIBER))
      throw ConfigManagerException(name + wxT(" is not a Subscriber"));

   AddObject(subs);
}


//------------------------------------------------------------------------------
// void AddSolarSystem(SolarSystem *solarSys)
//------------------------------------------------------------------------------
/**
 * Adds a SolarSystem to the configuration.
 *
 * @param solarSys Pointer to the SolarSystem instance.
 *
 * @todo Add solar systems to the ConfigManager
 */
//------------------------------------------------------------------------------
void ConfigManager::AddSolarSystem(SolarSystem *solarSys)
{
   throw ConfigManagerException(wxT("SolarSystem objects are not yet managed"));
}


//------------------------------------------------------------------------------
// void AddPropSetup(PropSetup* propSetup)
//------------------------------------------------------------------------------
/**
 * Adds a PropSetup to the configuration.
 *
 * @param propSetup Pointer to the PropSetup instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddPropSetup(PropSetup* propSetup)
{
   wxString name = propSetup->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!propSetup->IsOfType(Gmat::PROP_SETUP))
      throw ConfigManagerException(name + wxT(" is not a PropSetup"));

   AddObject(propSetup);
}


//------------------------------------------------------------------------------
// void AddSpacecraft(SpaceObject *sc)
//------------------------------------------------------------------------------
/**
 * Adds a Spacecraft and formations to the configuration.
 *
 * @param sc Pointer to the Spacecraft/Formation instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddSpacecraft(SpaceObject *sc)
{
   wxString name = sc->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!sc->IsOfType(Gmat::SPACEOBJECT))
      throw ConfigManagerException(name + wxT(" is not a SpaceObject"));

   AddObject(sc);
}


//------------------------------------------------------------------------------
// void AddSpacePoint(SpacePoint *sp)
//------------------------------------------------------------------------------
/**
 * Adds a SpacePoint to the configuration.
 * 
 * NOTE: Spacecraft and Formations are handled in the AddSpacecraft method. 
 *
 * @param sp Pointer to the SpacePoint instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddSpacePoint(SpacePoint *sp)
{
   wxString name = sp->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!sp->IsOfType(Gmat::SPACE_POINT))
      throw ConfigManagerException(name + wxT(" is not a SpacePoint"));

   AddObject(sp);
}


//------------------------------------------------------------------------------
// void AddHardware(Hardware *hw)
//------------------------------------------------------------------------------
/**
 * Adds a Hardware object to the configuration.
 *
 * @param hw Pointer to the Hardware object.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddHardware(Hardware *hw)
{
   wxString name = hw->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!hw->IsOfType(Gmat::HARDWARE))
      throw ConfigManagerException(name + wxT(" is not Hardware"));

   AddObject(hw);
}


//------------------------------------------------------------------------------
// void AddStopCondition(StopCondition* stopCond)
//------------------------------------------------------------------------------
/**
 * Adds a StopCondition to the configuration.
 *
 * @param stopCond Pointer to the StopCondition instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddStopCondition(StopCondition* stopCond)
{
   wxString name = stopCond->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!stopCond->IsOfType(Gmat::STOP_CONDITION))
      throw ConfigManagerException(name + wxT(" is not a StopCondition"));

   AddObject(stopCond);
}


//------------------------------------------------------------------------------
// void AddParameter(Parameter* parameter)
//------------------------------------------------------------------------------
/**
 * Adds a Parameter to the configuration.
 *
 * @param parameter Pointer to the Parameter instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddParameter(Parameter* parameter)
{
   wxString name = parameter->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!parameter->IsOfType(Gmat::PARAMETER))
      throw ConfigManagerException(name + wxT(" is not a Parameter"));

   AddObject(parameter);
}


//------------------------------------------------------------------------------
// void AddBurn(Burn* burn)
//------------------------------------------------------------------------------
/**
 * Adds a Burn to the configuration.
 *
 * @param burn Pointer to the Burn instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddBurn(Burn* burn)
{
   wxString name = burn->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!burn->IsOfType(Gmat::BURN))
      throw ConfigManagerException(name + wxT(" is not a Burn"));

   AddObject(burn);
}

//------------------------------------------------------------------------------
// void AddSolver(Solver* solver)
//------------------------------------------------------------------------------
/**
 * Adds a Solver to the configuration.
 *
 * @param solver Pointer to the Solver instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddSolver(Solver* solver)
{
   wxString name = solver->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!solver->IsOfType(Gmat::SOLVER))
      throw ConfigManagerException(name + wxT(" is not a Solver"));

   AddObject(solver);
}

//------------------------------------------------------------------------------
// void AddAtmosphereModel(AtmosphereModel* atmosModel)
//------------------------------------------------------------------------------
/**
 * Adds an AtmosphereModel to the configuration.
 *
 * @param pm atmosModel to the AtmosphereModel instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddAtmosphereModel(AtmosphereModel* atmosModel)
{
   wxString name = atmosModel->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!atmosModel->IsOfType(Gmat::ATMOSPHERE))
      throw ConfigManagerException(name + wxT(" is not an AtmosphereModel"));

   AddObject(atmosModel);
}

//------------------------------------------------------------------------------
// void AddFunction(Function* function)
//------------------------------------------------------------------------------
/**
 * Adds a Function to the configuration.
 *
 * @param function Pointer to the Function instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddFunction(Function* function)
{
   wxString name = function->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));
   
   if (!function->IsOfType(Gmat::FUNCTION))
      throw ConfigManagerException(name + wxT(" is not a Function"));

   AddObject(function);
}

//------------------------------------------------------------------------------
// void AddCoordinateSystem(CoordinateSystem *cs)
//------------------------------------------------------------------------------
/**
 * Adds a CoordinateSystem to the configuration.
 *
 * @param cs Pointer to the CoordinateSystem instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddCoordinateSystem(CoordinateSystem *cs)
{
   wxString name = cs->GetName();

   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!cs->IsOfType(Gmat::COORDINATE_SYSTEM))
      throw ConfigManagerException(name + wxT(" is not a CoordinateSystem"));

   AddObject(cs);
}

//------------------------------------------------------------------------------
// void AddCalculatedPoint(CalculatedPoint *cs)
//------------------------------------------------------------------------------
/**
 * Adds a CalculatedPoint to the configuration.
 *
 * @param cs Pointer to the CalculatedPoint instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddCalculatedPoint(CalculatedPoint *cs)
{
   wxString name = cs->GetName();

   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!cs->IsOfType(Gmat::CALCULATED_POINT))
      throw ConfigManagerException(name + wxT(" is not a CalculatedPoint"));

   AddObject(cs);
}

//------------------------------------------------------------------------------
// void AddMeasurementModel(MeasurementModel *mModel)
//------------------------------------------------------------------------------
/**
 * Adds a MeasurementModel to the configuration.
 *
 * @param mModel Pointer to the MeasurementModel instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddMeasurementModel(MeasurementModel *mModel)
{
   GmatBase *obj = (GmatBase*)mModel;

   wxString name = obj->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!obj->IsOfType(Gmat::MEASUREMENT_MODEL))
      throw ConfigManagerException(name + wxT(" is not a MeasurementModel"));

   AddObject(obj);
}

//------------------------------------------------------------------------------
// void AddTrackingSystem(TrackingSystem *ts)
//------------------------------------------------------------------------------
/**
 * Adds a TrackingSystem to the configuration.
 *
 * @param ts Pointer to the TrackingSystem instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddTrackingSystem(TrackingSystem *ts)
{
   GmatBase *obj = (GmatBase*)ts;

   wxString name = obj->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!obj->IsOfType(Gmat::TRACKING_SYSTEM))
      throw ConfigManagerException(name + wxT(" is not a TrackingSystem"));

   AddObject(obj);
}

//------------------------------------------------------------------------------
// void AddTrackingData(TrackingData *td)
//------------------------------------------------------------------------------
/**
 * Adds a TrackingData object to the configuration.
 *
 * @param td Pointer to the TrackingData instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddTrackingData(TrackingData *td)
{
   GmatBase *obj = (GmatBase*)td;

   wxString name = obj->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!obj->IsOfType(Gmat::TRACKING_DATA))
      throw ConfigManagerException(name + wxT(" is not a TrackingData object"));

   AddObject(obj);
}


//------------------------------------------------------------------------------
// void AddMeasurementModel(MeasurementModel *mModel)
//------------------------------------------------------------------------------
/**
 * Adds a MeasurementModel to the configuration.
 *
 * @param mModel Pointer to the MeasurementModel instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddMeasurement(CoreMeasurement *meas)
{
   GmatBase *obj = (GmatBase*)meas;
   wxString name = obj->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!obj->IsOfType(Gmat::CORE_MEASUREMENT))
      throw ConfigManagerException(name + wxT(" is not a Measurement"));

   MessageInterface::ShowMessage(wxT("Warning: Core measurement %s configured; it ")
         wxT("should be hidden inside of a MeasurementModel"), name.c_str());
   AddObject(obj);
}


//------------------------------------------------------------------------------
// void AddDataFile(DataFile *df)
//------------------------------------------------------------------------------
/**
 * Adds a DataFile to the configuration
 *
 * @param df The DataFile object that is being added to the configuration
 */
//------------------------------------------------------------------------------
void ConfigManager::AddDataFile(DataFile *df)
{
   GmatBase *obj = (GmatBase*)df;

   wxString name = obj->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if ((!obj->IsOfType(Gmat::DATA_FILE)) && (!obj->IsOfType(Gmat::DATASTREAM)))
      throw ConfigManagerException(name + wxT(" is not a DataFile or DataStream"));

   AddObject(obj);
}


//------------------------------------------------------------------------------
// void AddObType(ObType *ot)
//------------------------------------------------------------------------------
/**
 * Adds a named ObType to the configuration
 *
 * @param ot The ObType that is being added
 */
//------------------------------------------------------------------------------
void ConfigManager::AddObType(ObType *ot)
{
   GmatBase *obj = (GmatBase*)ot;

   wxString name = obj->GetName();
   if (name == wxT(""))
      throw ConfigManagerException(wxT("Unnamed objects cannot be managed"));

   if (!obj->IsOfType(Gmat::OBTYPE))
      throw ConfigManagerException(name + wxT(" is not an ObType"));

   MessageInterface::ShowMessage(wxT("Warning: ObType %s configured; it ")
         wxT("should be hidden inside of a DataFile"), name.c_str());
   AddObject(obj);
}


//------------------------------------------------------------------------------
// void AddObject(GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Adds a CoordinateSystem to the configuration.
 *
 * @param cs Pointer to the CoordinateSystem instance.
 */
//------------------------------------------------------------------------------
void ConfigManager::AddObject(GmatBase *obj)
{
   wxString name = obj->GetName();
   
   #ifdef DEBUG_CONFIG_ADD
   MessageInterface::ShowMessage
      (wxT("ConfigManager::AddObject() adding <%p><%s> '%s'\n"), obj,
       obj->GetTypeName().c_str(), name.c_str());
   #endif
   
   if (mapping.find(name) != mapping.end())
   {
      name += wxT(" is already in the configuration table");
      throw ConfigManagerException(name);
   }
   else
   {
      objects.push_back(obj);
      mapping[name] = obj;
   }
   
   // Until we can add TextEphemFile to resource tree, we don't want to
   // write to script file on save script. (loj: 2007.04.07)
   
   if (obj->GetTypeName() != wxT("TextEphemFile"))
      objectChanged = true;
}


//------------------------------------------------------------------------------
// void SetDefaultSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 * Sets the default SolarSystem.
 *
 * @param ss The SolarSystem object pointer.
 *
 */
//------------------------------------------------------------------------------
void ConfigManager::SetDefaultSolarSystem(SolarSystem *ss)
{
   defaultSolarSystem = ss;
}


//------------------------------------------------------------------------------
// void SetSolarSystemInUse(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 * Sets the current SolarSystem.
 *
 * @param ss The SolarSystem object pointer.
 *
 */
//------------------------------------------------------------------------------
void ConfigManager::SetSolarSystemInUse(SolarSystem *ss)
{
   #if DEBUG_CONFIG_SS
   MessageInterface::ShowMessage
      (wxT("ConfigManager::SetSolarSystemInUse() name=%s\n"), ss->GetName().c_str());
   #endif
   
   solarSystemInUse = ss;
}


//------------------------------------------------------------------------------
// bool SetSolarSystemInUse(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Sets the name for the current SolarSystem.
 *
 * @param name The SolarSystem name.
 *
 * @note This method is not yet used in GMAT.
 */
//------------------------------------------------------------------------------
bool ConfigManager::SetSolarSystemInUse(const wxString &name)
{
   throw ConfigManagerException
      (wxT("ConfigManager::SetSolarSystemInUse(name) has not been implemented.\n"));
}


//------------------------------------------------------------------------------
// const StringArray& GetListOfAllItems()
//------------------------------------------------------------------------------
/**
 * Retrieves a list of all configured objects.
 *
 * @return The list of objects.
 */
//------------------------------------------------------------------------------
const StringArray& ConfigManager::GetListOfAllItems()
{
   listOfItems.erase(listOfItems.begin(), listOfItems.end());
    
   std::vector<GmatBase*>::iterator current =
      (std::vector<GmatBase*>::iterator)(objects.begin());
   
   while (current != (std::vector<GmatBase*>::iterator)(objects.end()))
   {
      listOfItems.push_back((*current)->GetName());
      ++current;
   }
   return listOfItems;
}


//------------------------------------------------------------------------------
// const StringArray& GetListOfItemsHas(Gmat::ObjectType type, const wxString &name,
//                                      bool includeSysParam)
//------------------------------------------------------------------------------
/**
 * Checks a specific item used in anywhere.
 *
 * @param type The type of the object that is being checked.
 * @param name The name of the object.
 * @param includeSysParam True if system parameter to be included
 *
 * @return array of item names where the name is used.
 */
//------------------------------------------------------------------------------
const StringArray& ConfigManager::GetListOfItemsHas(Gmat::ObjectType type,
                                                    const wxString &name,
                                                    bool includeSysParam)
{
   StringArray items = GetListOfAllItems();
   wxString::size_type pos;
   GmatBase *obj;
   wxString objName;
   wxString objString;
   StringArray genStringArray;
   static StringArray itemList;
   itemList.clear();
   Gmat::ObjectType objType;
   
   #if DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("ConfigManager::GetListOfItemsHas() type=%d, name='%s', includeSysParam=%d\n"),
       type, name.c_str(), includeSysParam);
   #endif
   
   try
   {
      for (UnsignedInt itemIndex=0; itemIndex<items.size(); itemIndex++)
      {
         obj = GetItem(items[itemIndex]);
         objType = obj->GetType();
         
         #if DEBUG_RENAME
         MessageInterface::ShowMessage
            (wxT("===> obj[%d]=%s, %s\n"), itemIndex, obj->GetTypeName().c_str(),
             obj->GetName().c_str());
         #endif
         
         // if same type and name, skip
         // Added to check for the same name since FuelTank and Thruster are
         // both HARDWARE type to fix bug 2314 (LOJ: 2011.01.19)
         if (obj->IsOfType(type) && obj->GetName() == name)
            continue;
         
         bool isSystemParam = false;
         if (obj->IsOfType(Gmat::PARAMETER))
            if (((Parameter*)obj)->GetKey() == GmatParam::SYSTEM_PARAM)
               isSystemParam = true;
         
         // if system parameter not to be included, skip
         if (!includeSysParam && isSystemParam)
            continue;
         
         objName = obj->GetName();
         
         // We need to check names in RHS of equal sign so use GetGeneratingStringArray
         // This fixes bug 2222 (LOJ: 2010.12.01) 
         //objString = obj->GetGeneratingString();
         //pos = objString.find(name);
         
         genStringArray = obj->GetGeneratingStringArray();
         
         #if DEBUG_RENAME
         MessageInterface::ShowMessage(wxT("   genStringArray.size()=%d\n"), genStringArray.size());
         for (UnsignedInt ii = 0; ii < genStringArray.size(); ii++)
            MessageInterface::ShowMessage
               (wxT("      genStringArray[%d]='%s'\n"), ii, genStringArray[ii].c_str());
         #endif
         
         if (genStringArray.empty())
         {
            // Add Parameters to list (LOJ: 2011.01.11 - to fix bug 2321)
            if (obj->IsOfType(Gmat::PARAMETER))
            {
               objString = obj->GetGeneratingString();
               #if DEBUG_RENAME
               MessageInterface::ShowMessage
                  (wxT("   objString='%s'\n"), objString.c_str());
               #endif
               pos = objString.find(name);
               if (pos != objString.npos)
               {
                  #if DEBUG_RENAME
                  MessageInterface::ShowMessage
                     (wxT("   '%s' found in Parameter, so adding '%s'\n"), name.c_str(),
                      objName.c_str());
                  #endif
                  itemList.push_back(objName);
               }
            }
         }
         else
         {
            wxString rhsString;
            for (StringArray::iterator iter = genStringArray.begin();
                 iter < genStringArray.end(); iter++)
            {
               objString = *iter;
               StringArray parts = GmatStringUtil::SeparateBy(objString, wxT("="));
               
               #if DEBUG_RENAME
               MessageInterface::ShowMessage(wxT("   parts.size()=%d\n"), parts.size());
               for (UnsignedInt j = 0; j < parts.size(); j++)
                  MessageInterface::ShowMessage
                     (wxT("      parts[%d]='%s'\n"), j, parts[j].c_str());
               #endif
               
               if (parts.size() > 1)
               {
                  rhsString = parts[1];
                  rhsString = GmatStringUtil::Trim(rhsString, GmatStringUtil::BOTH, true, true);
                  
                  #if DEBUG_RENAME
                  MessageInterface::ShowMessage
                     (wxT("   objString='%s', rhsString='%s'\n"), objString.c_str(),
                      rhsString.c_str());
                  #endif
                  
                  pos = rhsString.find(name);
                  if (pos != rhsString.npos)
                  {
                     // Add to list if name not found in string enclosed with single quote
                     if (!GmatStringUtil::IsEnclosedWith(rhsString, wxT("'")))
                     {
                        #if DEBUG_RENAME
                        MessageInterface::ShowMessage
                           (wxT("   '%s' found in RHS, so adding '%s'\n"), name.c_str(), objName.c_str());
                        #endif
                        itemList.push_back(objName);
                     }
                  }
               }
            }
         }
      }
   }
   catch (BaseException &e)
   {
      MessageInterface::ShowMessage(wxT("*** Error: %s\n"), e.GetFullMessage().c_str());
   }
   
   return itemList;
}


//------------------------------------------------------------------------------
// const StringArray& GetListOfItems(Gmat::ObjectType itemType)
//------------------------------------------------------------------------------
/**
 * Retrieves a list of all configured objects of a given type.
 *
 * @param itemType The type of object requested.
 *
 * @return The list of objects.
 */
//------------------------------------------------------------------------------
const StringArray& ConfigManager::GetListOfItems(Gmat::ObjectType itemType)
{
   listOfItems.erase(listOfItems.begin(), listOfItems.end());
    
   std::vector<GmatBase*>::iterator current =
      (std::vector<GmatBase*>::iterator)(objects.begin());
   while (current != (std::vector<GmatBase*>::iterator)(objects.end()))
   {
      if ((*current)->IsOfType(itemType))
         listOfItems.push_back((*current)->GetName());
      ++current;
   }
   return listOfItems;
}


//------------------------------------------------------------------------------
// const StringArray& GetListOfItems(const wxString &typeName)
//------------------------------------------------------------------------------
/**
 * Retrieves a list of all configured objects of a given type name.
 *
 * @param typeName The type name of object requested.
 *
 * @return The list of objects.
 */
//------------------------------------------------------------------------------
const StringArray& ConfigManager::GetListOfItems(const wxString &typeName)
{
   listOfItems.clear();
   
   std::vector<GmatBase*>::iterator current =
      (std::vector<GmatBase*>::iterator)(objects.begin());
   while (current != (std::vector<GmatBase*>::iterator)(objects.end()))
   {
      if ((*current)->IsOfType(typeName))
         listOfItems.push_back((*current)->GetName());
      ++current;
   }
   return listOfItems;
}


//------------------------------------------------------------------------------
// GmatBase* GetFirstItemUsing(Gmat::ObjectType type, const wxString &name,
//                            bool includeSysParam = true);
//------------------------------------------------------------------------------
/**
 * Retrives first object that uses given object type and name.
 *
 * @param type The type of the object that is being checked.
 * @param name The name of the object.
 * @param includeSysParam True if system parameter to be included
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
GmatBase* ConfigManager::GetFirstItemUsing(Gmat::ObjectType type,
                                           const wxString &name,
                                           bool includeSysParam)
{
   #ifdef DEBUG_CONFIG_OBJ_USING
   MessageInterface::ShowMessage
      (wxT("ConfigManager::GetFirstItemUsing() type=%d, name='%s', includeSysParam=%d\n"),
       type, name.c_str(), includeSysParam);
   #endif
   
   StringArray objList = GetListOfItemsHas(type, name, includeSysParam);
   
   #ifdef DEBUG_CONFIG_OBJ_USING
   MessageInterface::ShowMessage
      (wxT("   There are %d objects using '%s'\n"), objList.size(), name.c_str());
   #endif
   
   GmatBase *obj = NULL;
   
   for (UnsignedInt i = 0; i < objList.size(); i++)
   {
      obj = GetItem(objList[i]);
      
      #ifdef DEBUG_CONFIG_OBJ_USING
      MessageInterface::ShowMessage
         (wxT("      obj = <%p><%s>'%s'\n"), obj, obj->GetTypeName().c_str(),
          obj->GetName().c_str());
      #endif
      
      if (obj->GetName() != name)
         break;
   }
   
   #ifdef DEBUG_CONFIG_OBJ_USING
   MessageInterface::ShowMessage
      (wxT("ConfigManager::GetFirstItemUsing() returning <%p>\n"), obj);
   #endif
   
   return obj;
}


//------------------------------------------------------------------------------
// GmatBase* GetItem(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves an object by name.
 *
 * @param name The name of the object requested.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
GmatBase* ConfigManager::GetItem(const wxString &name)
{
   #ifdef DEBUG_CONFIG_GET_ITEM
   MessageInterface::ShowMessage(wxT("ConfigManager::GetItem() name='%s'\n"), name.c_str());
   #endif
   
   GmatBase *obj = NULL;
   
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->GetName() == name)
      {
         obj = mapping[name];
      }
   }
   
   #ifdef DEBUG_CONFIG_GET_ITEM
   MessageInterface::ShowMessage(wxT("===> ConfigManager::GetItem() returning <%p>\n"), obj);
   #endif
   
   return obj;
}


//------------------------------------------------------------------------------
// bool RenameItem(Gmat::ObjectType type, const wxString &oldName,
//                 const wxString &newName)
//------------------------------------------------------------------------------
/**
 * Changes the name for a configured object.
 *
 * @param type The type of object that is renamed.
 * @param oldName The current name for the object.
 * @param newName The new name for the object.
 *
 * @return true if the object was renamed, false if not.
 */
//------------------------------------------------------------------------------
bool ConfigManager::RenameItem(Gmat::ObjectType type,
                               const wxString &oldName,
                               const wxString &newName)
{
   #if DEBUG_RENAME
      MessageInterface::ShowMessage
         (wxT("ConfigManager::RenameItem() type=%d, oldName='%s', newName='%s'\n"),
          type, oldName.c_str(), newName.c_str());
   #endif
   
   bool renamed = false;
   
   //--------------------------------------------------
   // change mapping name
   //--------------------------------------------------
   GmatBase *mapObj = NULL;
   if (mapping.find(oldName) != mapping.end())
   {
      mapObj = mapping[oldName];
      if (mapObj->IsOfType(type))
      {
         // if newName does not exist, change name
         if (mapping.find(newName) == mapping.end())
         {
            mapping.erase(oldName);
            mapping[newName] = mapObj;
            mapObj->SetName(newName);
            renamed = true;
            #if DEBUG_RENAME
            MessageInterface::ShowMessage
               (wxT("   Rename mapping mapObj=%s\n"), mapObj->GetName().c_str());
            #endif
         }
         else
         {
            MessageInterface::PopupMessage
               (Gmat::WARNING_, wxT("%s already exist, Please enter different name.\n"),
                newName.c_str());
         }
      }
      else
      {
         MessageInterface::ShowMessage
            (wxT("ConfigManager::RenameItem() oldName has different type:%d\n"),
             mapObj->GetType());
      }
   }
   
   if (!renamed)
   {
      #if DEBUG_RENAME
         MessageInterface::ShowMessage
            (wxT("ConfigManager::RenameItem() Unable to rename: oldName not found.\n"));
      #endif
      return false;
   }
   
   //----------------------------------------------------
   // Rename ref. objects
   //----------------------------------------------------
   GmatBase *obj = NULL;
   StringArray itemList = GetListOfItemsHas(type, oldName);
   #if DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("   =====> There are %d items has '%s'\n"), itemList.size(), oldName.c_str());
   #endif
   for (UnsignedInt i=0; i<itemList.size(); i++)
   {
      obj = GetItem(itemList[i]);
      if (obj)
      {
         #if DEBUG_RENAME
         MessageInterface::ShowMessage
            (wxT("      calling  %s->RenameRefObject()\n"), obj->GetName().c_str());
         #endif
         
         renamed = obj->RenameRefObject(type, oldName, newName);
      }
   }
   
   //----------------------------------------------------
   // Rename owned object ODEModel in the PropSetup.
   //----------------------------------------------------
   if (type == Gmat::PROP_SETUP)
   {
      // Change _ForceMode name if _ForceModel is configured
      wxString oldFmName = oldName + wxT("_ForceModel");
      wxString newFmName = newName + wxT("_ForceModel");
      if (mapping.find(oldFmName) != mapping.end())
      {
         mapObj = mapping[oldFmName];
         // if newName does not exist, change name
         if (mapping.find(newFmName) == mapping.end())
         {
            mapping.erase(oldFmName);
            mapping[newFmName] = mapObj;
            mapObj->SetName(newFmName);         
            
            #if DEBUG_RENAME
            MessageInterface::ShowMessage
               (wxT("   Rename mapping mapObj=%s\n"), mapObj->GetName().c_str());
            #endif
         }
      }
      
      #if DEBUG_RENAME
      MessageInterface::ShowMessage(wxT("   Calling PropSetup::RenameRefObject()\n"));
      #endif
      mapObj->RenameRefObject(type, oldName, newName);
   }
   
   //----------------------------------------------------
   // Rename owned tanks in the thrusters.
   // Tank is ReadOnly parameter so it does show in
   // GeneratingString()
   //----------------------------------------------------
   
   if (type == Gmat::HARDWARE)
   {
      itemList = GetListOfItems(Gmat::HARDWARE);
      
      for (unsigned int i=0; i<itemList.size(); i++)
      {
         obj = GetItem(itemList[i]);
         if (obj->IsOfType(wxT("Thruster")))
             obj->RenameRefObject(type, oldName, newName);
      }
   }
   
   //----------------------------------------------------
   // Rename system parameters and expression of variables
   //----------------------------------------------------
   // Since new hardware Parameters were added, chcek for the Hardware also.
   
   if (type == Gmat::SPACECRAFT || type == Gmat::COORDINATE_SYSTEM ||
       type == Gmat::CALCULATED_POINT || type == Gmat::BURN ||
       type == Gmat::IMPULSIVE_BURN || type == Gmat::HARDWARE ||
       type == Gmat::THRUSTER || type == Gmat::FUEL_TANK)
   {
      StringArray params = GetListOfItems(Gmat::PARAMETER);
      wxString oldParamName, newParamName;
      Parameter *param;
      wxString::size_type pos;
      
      for (unsigned int i=0; i<params.size(); i++)
      {
         #if DEBUG_RENAME
         MessageInterface::ShowMessage(wxT("params[%d]=%s\n"), i, params[i].c_str());
         #endif
         
         param = GetParameter(params[i]);
         
         // if system parameter, change it's own name
         if (param->GetKey() == GmatParam::SYSTEM_PARAM)
         {
            oldParamName = param->GetName();
            pos = oldParamName.find(oldName);
            
            // rename actual parameter name
            if (pos != oldParamName.npos)
            {
               newParamName = oldParamName;
               newParamName.replace(pos, oldName.size(), newName);
               
               #if DEBUG_RENAME
               MessageInterface::ShowMessage
                  (wxT("===> oldParamName=%s, newParamName=%s\n"),
                   oldParamName.c_str(), newParamName.c_str());
               #endif
               
               // change parameter mapping name
               if (mapping.find(oldParamName) != mapping.end())
               {
                  mapping.erase(oldParamName);
                  mapping[newParamName] = (GmatBase*)param;
                  param->SetName(newParamName);
                  renamed = true;
               }
            }
         }
         // if variable, need to change expression
         else if (param->GetTypeName() == wxT("Variable"))
         {
            param->RenameRefObject(Gmat::PARAMETER, oldName, newName);
         }
      }
   }
   
   #if DEBUG_RENAME
   StringArray allItems = GetListOfAllItems();
   MessageInterface::ShowMessage(wxT("===> After rename\n"));
   for (UnsignedInt i=0; i<allItems.size(); i++)
      MessageInterface::ShowMessage(wxT("   item[%d] = %s\n"), i, allItems[i].c_str());
   #endif
   
   objectChanged = true;
   
   return renamed;
} // RenameItem()


//------------------------------------------------------------------------------
// bool RemoveAllItems()
//------------------------------------------------------------------------------
/**
 * Removes all configured objects from memory
 *
 * @return true on success, false on failure.
 */
//------------------------------------------------------------------------------
bool ConfigManager::RemoveAllItems()
{
   // delete objects
   #ifdef DEBUG_CONFIG_REMOVE
   MessageInterface::ShowMessage
      (wxT("ConfigManager::RemoveAllItems() Deleting %d objects\n"), objects.size());
   #endif
   
   for (unsigned int i=0; i<objects.size(); i++)
   {
      wxString objName = objects[i]->GetName();
      
      #ifdef DEBUG_CONFIG_REMOVE_MORE
      MessageInterface::ShowMessage
         (wxT("   deleting <%p><%s>'%s'\n"), objects[i], objects[i]->GetTypeName().c_str(),
          objects[i]->GetName().c_str());
      #endif
      
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (objects[i], objects[i]->GetName(), wxT("ConfigManager::RemoveAllItems()"),
          wxT(" deleting configured obj"));
      #endif
      
      delete objects[i];
      objects[i] = NULL;
   }
   
   #ifdef DEBUG_CONFIG_REMOVE
   MessageInterface::ShowMessage(wxT("Deleting %d new objects\n"), newObjects.size());
   #endif
   // delete objects that were reconfigured, ie, just object pointer reset in the map
   for (unsigned int i=0; i<newObjects.size(); i++)
   {
      wxString objName = newObjects[i]->GetName();
      
      #ifdef DEBUG_CONFIG_REMOVE_MORE
      MessageInterface::ShowMessage
         (wxT("   deleting <%p><%s>'%s'\n"), newObjects[i], newObjects[i]->GetTypeName().c_str(),
          newObjects[i]->GetName().c_str());
      #endif
      
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (newObjects[i], newObjects[i]->GetName(), wxT("ConfigManager::RemoveAllItems()"),
          wxT(" deleting configured obj"));
      #endif
      
      delete newObjects[i];
      newObjects[i] = NULL;
   }
   
   objects.clear();
   newObjects.clear();
   mapping.clear();
   
   return true;
}


//------------------------------------------------------------------------------
// bool RemoveItem(Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Removes a specific item from memory.
 *
 * @param type The type of the object that is being removed.
 * @param name The name of the object.
 *
 * @return true on success, false on failure.
 */
//------------------------------------------------------------------------------
bool ConfigManager::RemoveItem(Gmat::ObjectType type, const wxString &name)
{
   #ifdef DEBUG_CONFIG_REMOVE
   MessageInterface::ShowMessage
      (wxT("ConfigManager::RemoveItem() entered, type=%d, typeString='%s', ")
       wxT("name='%s'\n"), type, GmatBase::GetObjectTypeString(type).c_str(),
       name.c_str());
   #endif
   
   bool status = false;
   
   // remove from objects
   std::vector<GmatBase*>::iterator currentIter =
      (std::vector<GmatBase*>::iterator)(objects.begin());
   
   #ifdef DEBUG_CONFIG_REMOVE
   MessageInterface::ShowMessage(wxT("   There are %d objects\n"), objects.size());
   #endif
   
   while (currentIter != (std::vector<GmatBase*>::iterator)(objects.end()))
   {
      GmatBase *obj = (*currentIter);
      if (obj->IsOfType(type))
      {
         if (obj->GetName() == name)
         {
            #ifdef DEBUG_CONFIG_REMOVE
            MessageInterface::ShowMessage
               (wxT("   Removing '%s' from objects\n"), name.c_str());
            #endif
            objects.erase(currentIter);
            break;
         }
      }
      ++currentIter;
   }
   
   // remove from mapping
   #ifdef DEBUG_CONFIG_REMOVE
   MessageInterface::ShowMessage(wxT("   There are %d objects in the mapping\n"), mapping.size());
   #endif
   
   if (mapping.find(name) != mapping.end())
   {
      GmatBase *obj = mapping[name];
      if (obj != NULL)
      {
         if (obj->IsOfType(type))
         {
            mapping.erase(name);
            #ifdef DEBUG_CONFIG_REMOVE
            MessageInterface::ShowMessage
               (wxT("   Deleting '%s' from mapping\n"), name.c_str());
            #endif
            
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Remove
               (obj, name, wxT("ConfigManager::RemoveItem()"),
                wxT("deleting object from mapping"));
            #endif
            
            delete obj;
            obj = NULL;
            status = true;
         }
      }
      else
      {
         #ifdef DEBUG_CONFIG_REMOVE
         MessageInterface::ShowMessage
            (wxT("   The obj from the mapping is NULL\n"));
         #endif
      }
   }
   
   objectChanged = true;
   
   #ifdef DEBUG_CONFIG_REMOVE
   MessageInterface::ShowMessage
      (wxT("ConfigManager::RemoveItem() exiting, '%s' %s removed\n"), name.c_str(),
       status ? wxT("was") : wxT("was not"));
   #endif
   return status;
}


//------------------------------------------------------------------------------
// bool ReconfigureItem(GmatBase *newobj, const wxString &name)
//------------------------------------------------------------------------------
/*
 * Sets configured object pointer with new pointer.
 *
 * @param  newobj  New object pointer
 * @param  name  Name of the configured object to be reset
 *
 * @return  true if pointer was reset, false otherwise
 */
//------------------------------------------------------------------------------
bool ConfigManager::ReconfigureItem(GmatBase *newobj, const wxString &name)
{
   GmatBase *obj = NULL;
   
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->GetName() == name)
      {
         obj = mapping[name];
         
         #if DEBUG_CONFIG_RECONFIGURE
         MessageInterface::ShowMessage
            (wxT("ConfigManager::ReconfigureItem() obj=%p newobj=%p\n"), obj,
             newobj);
         #endif
         
         // We want to replace if classsified as the same sub type
         if (newobj->IsOfType(obj->GetTypeName()))
         {
            mapping[name] = newobj;
            newObjects.push_back(newobj);
            return true;
         }
      }
   }
   
   return false;
}


//------------------------------------------------------------------------------
// PhysicalModel* GetPhysicalModel(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a PhysicalModel object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
PhysicalModel* ConfigManager::GetPhysicalModel(const wxString &name)
{
   PhysicalModel *physicalModel = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::PHYSICAL_MODEL))
      {
         physicalModel = (PhysicalModel *)mapping[name];
      }
   }
   return physicalModel;
}

//------------------------------------------------------------------------------
// Propagator* GetPropagator(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a Propagator object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
Propagator* ConfigManager::GetPropagator(const wxString &name)
{
   Propagator *prop = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::PROPAGATOR))
      {
         prop = (Propagator *)mapping[name];
      }
   }
   return prop;
}


//------------------------------------------------------------------------------
// ForceModel* GetForceModel(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a ForceModel object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
ODEModel* ConfigManager::GetODEModel(const wxString &name)
{
   ODEModel *fm = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name] == NULL)
         throw ConfigManagerException
            (wxT("ConfigManager::GetForceModel(name) is finding a NULL object in the mapping.\n"));
     
      if (mapping[name]->IsOfType(Gmat::ODE_MODEL))
      {
         fm = (ODEModel *)mapping[name];
      }
   }
   return fm;
}


//------------------------------------------------------------------------------
// SpaceObject* GetSpacecraft(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a Spacecraft or Formation object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
SpaceObject* ConfigManager::GetSpacecraft(const wxString &name)
{
   SpaceObject *sc = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if ((mapping[name]->IsOfType(Gmat::SPACECRAFT)) ||
          (mapping[name]->IsOfType(Gmat::FORMATION)))
      {
         sc = (SpaceObject *)mapping[name];
      }
   }
   return sc;
}


//------------------------------------------------------------------------------
// SpacePoint* GetSpacePoint(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a SpacePoint object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
SpacePoint* ConfigManager::GetSpacePoint(const wxString &name)
{
   SpaceObject *sp = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::SPACE_POINT))
      {
         sp = (SpaceObject *)mapping[name];
      }
   }
   return sp;
}


//------------------------------------------------------------------------------
// Hardware* GetHardware(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a Hardware object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
Hardware* ConfigManager::GetHardware(const wxString &name)
{
   Hardware *hw = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::HARDWARE))
      {
         hw = (Hardware *)mapping[name];
      }
   }
   return hw;
}


//------------------------------------------------------------------------------
// PropSetup* GetPropSetup(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a PropSetup object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
PropSetup* ConfigManager::GetPropSetup(const wxString &name)
{
   PropSetup *ps = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::PROP_SETUP))
      {
         ps = (PropSetup *)mapping[name];
      }
   }
   return ps;
}


//------------------------------------------------------------------------------
// Subscriber* GetSubscriber(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a Subscriber object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
Subscriber* ConfigManager::GetSubscriber(const wxString &name)
{
   Subscriber *sub = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::SUBSCRIBER))
      {
         sub = (Subscriber *)mapping[name];
      }
   }
   return sub;
}


//------------------------------------------------------------------------------
// SolarSystem* GetDefaultSolarSystem()
//------------------------------------------------------------------------------
/**
 * Retrieves the default SolarSystem object.
 *
 * @return A pointer to the object.
 *
 * @note This method is not yet used in GMAT.
 */
//------------------------------------------------------------------------------
SolarSystem* ConfigManager::GetDefaultSolarSystem()
{
   return defaultSolarSystem;
}


//------------------------------------------------------------------------------
// SolarSystem* GetSolarSystemInUse()
//------------------------------------------------------------------------------
/**
 * Retrieves the current SolarSystem object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
SolarSystem* ConfigManager::GetSolarSystemInUse()
{
   return solarSystemInUse;
}


//------------------------------------------------------------------------------
// SolarSystem* GetSolarSystemInUse(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves the current SolarSystem object by name
 *
 * @return A pointer to the object.
 *
 * @note This method is not yet used in GMAT.
 */
//------------------------------------------------------------------------------
SolarSystem* ConfigManager::GetSolarSystemInUse(const wxString &name)
{
   #if DEBUG_CONFIG_SS
   MessageInterface::ShowMessage
      (wxT("ConfigManager::GetSolarSystemInUse() name=%s\n"), name.c_str());
   #endif
   
   throw ConfigManagerException
      (wxT("ConfigManager::GetSolarSystemInUse(name) has not been implemented.\n"));
}


//------------------------------------------------------------------------------
// StopCondition* GetStopCondition(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a StopCondition object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
StopCondition* ConfigManager::GetStopCondition(const wxString &name)
{
   StopCondition *sc = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::STOP_CONDITION))
      {
         sc = (StopCondition *)mapping[name];
      }
   }
   return sc;
}


//------------------------------------------------------------------------------
// Parameter* GetParameter(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a Parameter object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
Parameter* ConfigManager::GetParameter(const wxString &name)
{
   Parameter *param = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::PARAMETER))
      {
         param = (Parameter *)mapping[name];
      }
   }
   return param;
}

//------------------------------------------------------------------------------
// Burn* GetBurn(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a Burn object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
Burn* ConfigManager::GetBurn(const wxString &name)
{
   Burn *burn = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::BURN))
      {
         burn = (Burn *)mapping[name];
      }
   }
   return burn;
}

//------------------------------------------------------------------------------
// Solver* GetSolver(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a Solver object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
Solver* ConfigManager::GetSolver(const wxString &name)
{
   Solver *solver = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::SOLVER))
      {
         solver = (Solver *)mapping[name];
      }
   }
   return solver;
}

//------------------------------------------------------------------------------
// AtmosphereModel* GetAtmosphereModel(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves an Atmosphere object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
AtmosphereModel* ConfigManager::GetAtmosphereModel(const wxString &name)
{
   AtmosphereModel *atmosModel = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::ATMOSPHERE))
      {
         atmosModel = (AtmosphereModel *)mapping[name];
      }
   }
   return atmosModel;
}

//------------------------------------------------------------------------------
// Function* GetFunction(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a Function object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
Function* ConfigManager::GetFunction(const wxString &name)
{
   Function *function = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::FUNCTION))
      {
         function = (Function *)mapping[name];
      }
   }
   return function;
}

//------------------------------------------------------------------------------
// CoordinateSystem* GetCoordinateSystem(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a CoordinateSystem object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
CoordinateSystem* ConfigManager::GetCoordinateSystem(const wxString &name)
{
   CoordinateSystem *cs = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::COORDINATE_SYSTEM))
      {
         cs = (CoordinateSystem *)mapping[name];
      }
   }
   return cs;
}


//------------------------------------------------------------------------------
// CalculatedPoint* GetCalculatedPoint(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a CalculatedPoint object.
 *
 * @param name The name of the object.
 *
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
CalculatedPoint* ConfigManager::GetCalculatedPoint(const wxString &name)
{
   CalculatedPoint *cs = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::CALCULATED_POINT))
      {
         cs = (CalculatedPoint *)mapping[name];
      }
   }
   return cs;
}


//------------------------------------------------------------------------------
// MeasurementModel* GetMeasurementModel(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a MeasurementModel from the configuration
 *
 * @param name The name of the MeasurementModel
 *
 * @return A pointer to the MeasurementModel, or NULL if it was not found
 */
//------------------------------------------------------------------------------
MeasurementModel* ConfigManager::GetMeasurementModel(const wxString &name)
{
   MeasurementModel *mm = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::MEASUREMENT_MODEL))
      {
         mm = (MeasurementModel *)mapping[name];
      }
   }
   return mm;
}

//------------------------------------------------------------------------------
// TrackingSystem* GetTrackingSystem(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a TrackingSystem from the configuration
 *
 * @param name The name of the TrackingSystem
 *
 * @return A pointer to the TrackingSystem, or NULL if it was not found
 */
//------------------------------------------------------------------------------
TrackingSystem* ConfigManager::GetTrackingSystem(const wxString &name)
{
   TrackingSystem *obj = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::TRACKING_SYSTEM))
      {
         obj = (TrackingSystem *)mapping[name];
      }
   }
   return obj;
}

//------------------------------------------------------------------------------
// TrackingData* GetTrackingData(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Retrieves a TrackingData object from the configuration
 *
 * @param name The name of the TrackingData object
 *
 * @return A pointer to the TrackingData object, or NULL if it was not found
 */
//------------------------------------------------------------------------------
TrackingData* ConfigManager::GetTrackingData(const wxString &name)
{
   TrackingData *obj = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::TRACKING_DATA))
      {
         obj = (TrackingData *)mapping[name];
      }
   }
   return obj;
}

DataFile* ConfigManager::GetDataStream(const wxString &name)
{
   DataFile *df = NULL;
   if (mapping.find(name) != mapping.end())
   {
      if (mapping[name]->IsOfType(Gmat::DATASTREAM))
      {
         df = (DataFile *)mapping[name];
      }
   }
   return df;
}


//------------------------------------------------------------------------------
// bool HasConfigurationChanged()
//------------------------------------------------------------------------------
bool ConfigManager::HasConfigurationChanged()
{
   return objectChanged;
}


//------------------------------------------------------------------------------
// void ConfigurationChanged(bool tf)
//------------------------------------------------------------------------------
void ConfigManager::ConfigurationChanged(bool tf)
{
   objectChanged = tf;
}


//------------------------------------------------------------------------------
// ObjectMap* GetObjectMap()
//------------------------------------------------------------------------------
ObjectMap* ConfigManager::GetObjectMap()
{
   return &mapping;
}

