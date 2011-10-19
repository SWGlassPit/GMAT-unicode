//$Id: EphemerisFile.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  EphemerisFile
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Linda Jun / NASA
// Created: 2009/09/02
//
/**
 * Writes a spacecraft orbit states or attitude to an ephemeris file either
 * CCSDS or SPK format.
 */
//------------------------------------------------------------------------------
#ifndef EphemerisFile_hpp
#define EphemerisFile_hpp

#include "Subscriber.hpp"
#include "Spacecraft.hpp"
#include "CoordinateSystem.hpp"
#include "CoordinateConverter.hpp"
#include "Interpolator.hpp"
#include <iostream>
#include <fstream>

class SpiceOrbitKernelWriter;

class GMAT_API EphemerisFile : public Subscriber
{
public:
   EphemerisFile(const wxString &name, const wxString &type = wxT("EphemerisFile"));
   virtual ~EphemerisFile();
   EphemerisFile(const EphemerisFile &);
   EphemerisFile& operator=(const EphemerisFile&);
   
   // methods for this class
   wxString          GetFileName();
   virtual void         ValidateParameters();
   
   // methods inherited from Subscriber
   virtual bool         Initialize();
   virtual void         SetProvider(GmatBase *provider);
   
   // methods inherited from GmatBase
   virtual GmatBase*    Clone(void) const;
   virtual void         Copy(const GmatBase* orig);
   
   virtual bool         TakeAction(const wxString &action,
                                   const wxString &actionData = wxT(""));
   
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   virtual bool         IsParameterReadOnly(const Integer id) const;
   
   virtual Gmat::ObjectType
                        GetPropertyObjectType(const Integer id) const;
   virtual const StringArray&
                        GetPropertyEnumStrings(const Integer id) const;
   
   virtual bool         GetBooleanParameter(const Integer id) const;
   virtual bool         SetBooleanParameter(const Integer id,
                                            const bool value);

   virtual Integer      GetIntegerParameter(const Integer id) const;
   virtual Integer      SetIntegerParameter(const Integer id,
                                            const Integer value);
   
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value);
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value);
   
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                     const wxString &name);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                     const wxString &name = wxT(""));
   
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   
   
protected:

   const static UnsignedInt MAX_SEGMENT_SIZE = 1000;
   
   enum FileType
   {
      CCSDS_OEM, CCSDS_AEM, SPK_ORBIT, SPK_ATTITUDE,
   };
   
   Spacecraft             *spacecraft;
   CoordinateSystem       *outCoordSystem;
   Interpolator           *interpolator; // owned object
   SpiceOrbitKernelWriter *spkWriter;    // owned object
   
   // for buffering ephemeris data
   EpochArray  a1MjdArray;
   StateArray  stateArray;
   
   /// ephemeris output path from the startup file
   wxString oututPath;
   /// ephmeris full file name including the path
   wxString filePath;
   wxString spacecraftName;
   wxString fileName;
   wxString fileFormat;
   wxString epochFormat;
   wxString ccsdsEpochFormat;
   wxString initialEpoch;
   wxString finalEpoch;
   wxString stepSize;
   wxString interpolatorName;
   wxString stateType;
   wxString outCoordSystemName;
   bool writeEphemeris;
   /// for propagator change
   wxString prevPropName;
   wxString currPropName;
   /// for comments
   wxString currComments;
   
   /// for meta data
   wxString metaDataStartStr;
   wxString metaDataStopStr;
   
   Integer     interpolationOrder;
   Integer     initialCount;
   Integer     waitCount;
   
   Real        stepSizeInA1Mjd;
   Real        stepSizeInSecs;
   Real        initialEpochA1Mjd;
   Real        finalEpochA1Mjd;
   Real        nextOutEpoch;
   Real        nextReqEpoch;
   Real        currEpochInDays;
   Real        currEpochInSecs;
   Real        prevEpochInSecs;
   Real        prevProcTime;
   Real        lastEpochWrote;
   Real        attEpoch;
   Real        maneuverEpochInDays;
   Real        currState[6];
   Real        attQuat[4];
   RealArray   epochsOnWaiting;
   
   bool        firstTimeWriting;
   bool        writingNewSegment;
   bool        useStepSize;
   bool        writeOrbit;
   bool        writeAttitude;
   bool        writeDataInDataCS;
   bool        processingLargeStep;
   bool        spkWriteFailed;
   bool        writeCommentAfterData;
   
   Gmat::RunState prevRunState;
   
   CoordinateConverter coordConverter;
   
   FileType    fileType;
   
   /// for maneuver handling
   ObjectArray maneuversHandled;
      
   /// output data stream
   std::ofstream      dstream;
   
   /// Available file format list
   static StringArray fileFormatList;   
   /// Available epoch format list
   static StringArray epochFormatList;   
   /// Available initial epoch list
   static StringArray initialEpochList;
   /// Available final epoch list
   static StringArray finalEpochList;   
   /// Available step size list
   static StringArray stepSizeList;   
   /// Available state type list
   static StringArray stateTypeList;   
   /// Available write ephemeris list
   static StringArray writeEphemerisList;
   /// Available interpolator type list
   static StringArray interpolatorTypeList;
   
   // Initialization
   void         InitializeData();
   void         CreateInterpolator();
   void         CreateSpiceKernelWriter();
   bool         OpenEphemerisFile();
   
   // Time and data
   bool         CheckInitialAndFinalEpoch();
   void         HandleCcsdsOrbitData(bool writeData);
   void         HandleSpkOrbitData(bool writeData);
   
   // Interpolation
   void         RestartInterpolation(const wxString &comments = wxT(""), bool writeAfterData = true);
   bool         IsTimeToWrite(Real epochInSecs, const Real state[6]);
   void         WriteOrbit(Real reqEpochInSecs, const Real state[6]);
   void         WriteOrbitAt(Real reqEpochInSecs, const Real state[6]);
   void         GetAttitude();
   void         WriteAttitude();
   void         FinishUpWriting();
   void         ProcessEpochsOnWaiting(bool checkFinalEpoch = false);
   bool         SetEpoch(Integer id, const wxString &value,
                         const StringArray &allowedValues);
   bool         SetStepSize(Integer id, const wxString &value,
                            const StringArray &allowedValues);
   void         HandleError(Integer id, const wxString &value,
                            const StringArray &allowedValues,
                            const wxString &additionalMsg = wxT(""));
   wxString  ToString(const StringArray &strList);
   
   // General writing
   void         WriteString(const wxString &str);
   void         WriteHeader();
   void         WriteMetaData();
   void         WriteComments(const wxString &comments);
   
   // General data buffering
   void         BufferOrbitData(Real epochInDays, const Real state[6]);
   void         DeleteOrbitData();
   
   // CCSDS file writing for debug and actual
   bool         OpenCcsdsEphemerisFile();
   void         WriteCcsdsHeader();
   void         WriteCcsdsOrbitDataSegment();
   void         WriteCcsdsOemMetaData();
   void         WriteCcsdsOemData(Real reqEpochInSecs, const Real state[6]);
   void         WriteCcsdsAemMetaData();
   void         WriteCcsdsAemData(Real reqEpochInSecs, const Real quat[4]);
   void         WriteCcsdsComments(const wxString &comments);
   
   // CCSDS file actual writing (subclass should overwrite this methods)
   virtual bool OpenRealCcsdsEphemerisFile();
   virtual void WriteRealCcsdsHeader();
   virtual void WriteRealCcsdsOrbitDataSegment();
   virtual void WriteRealCcsdsOemMetaData();
   virtual void WriteRealCcsdsAemMetaData();
   virtual void WriteRealCcsdsAemData(Real reqEpochInSecs, const Real quat[4]);
   virtual void WriteRealCcsdsComments(const wxString &comments);
   
   // SPK file writing
   void         WriteSpkHeader(); // This is for debug
   void         WriteSpkOrbitDataSegment();
   void         WriteSpkOrbitMetaData();
   void         WriteSpkComments(const wxString &comments);
   void         FinalizeSpkFile();
   
   // Epoch handling
   RealArray::iterator
                FindEpochOnWaiting(Real epochInSecs, const wxString &msg);
   void         RemoveEpochAlreadyWritten(Real epochInSecs, const wxString &msg);
   void         AddNextEpochToWrite(Real epochInSecs, const wxString &msg);
   
   // CoordinateSystem conversion
   void         ConvertState(Real epochInDays, const Real inState[6],
                             Real outState[6]);
   
   // for time formatting
   wxString  ToUtcGregorian(Real epoch, bool inDays = false, Integer format = 2);
   
   // for debugging
   void         DebugWriteTime(const wxString &msg, Real epoch, bool inDays = false,
                               Integer format = 2);
   void         DebugWriteOrbit(const wxString &msg, Real epoch, const Real state[6],
                                bool inDays = false, bool logOnly = false);
   void         DebugWriteOrbit(const wxString &msg, A1Mjd *epochInDays,
                                Rvector6 *state, bool logOnly = false);
   void         DebugWriteEpochsOnWaiting(const wxString &msg = wxT(""));
   
   // for deprecated field
   void         WriteDeprecatedMessage(Integer id) const;
   
   // methods inherited from Subscriber
   virtual bool Distribute(Integer len);
   virtual bool Distribute(const Real * dat, Integer len);
   virtual void HandleManeuvering(GmatBase *originator, bool maneuvering, Real epoch,
                                  const StringArray &satNames,
                                  const wxString &desc);
   virtual void HandlePropagatorChange(GmatBase *provider);
   virtual void HandleScPropertyChange(GmatBase *originator, Real epoch,
                                       const wxString &satName,
                                       const wxString &desc);
   enum
   {
      SPACECRAFT = SubscriberParamCount,
      FILENAME,
      FILE_FORMAT,
      EPOCH_FORMAT,
      INITIAL_EPOCH,
      FINAL_EPOCH,
      STEP_SIZE,
      INTERPOLATOR,
      INTERPOLATION_ORDER,
      STATE_TYPE,
      COORDINATE_SYSTEM,
      WRITE_EPHEMERIS,
      FILE_NAME,                // deprecated
      EphemerisFileParamCount   // Count of the parameters for this class
   };
   
   static const wxString
      PARAMETER_TEXT[EphemerisFileParamCount - SubscriberParamCount];
   static const Gmat::ParameterType
      PARAMETER_TYPE[EphemerisFileParamCount - SubscriberParamCount];
   
};

#endif // EphemerisFile_hpp
