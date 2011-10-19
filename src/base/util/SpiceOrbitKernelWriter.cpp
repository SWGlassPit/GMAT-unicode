//$Id:$
//------------------------------------------------------------------------------
//                              SpiceOrbitKernelWriter
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under
// FDSS Task order 28.
//
// Author: Wendy C. Shoan
// Created: 2009.12.07
//
/**
 * Implementation of the SpiceOrbitKernelWriter, which writes SPICE data (kernel) files.
 *
 * This code creates a temporary text file, required in order to include META-Data
 * (commentary) in the SPK file.  The file is deleted from the system after the
 * commentary is added to the SPK file.  The name of this temporary text file
 * takes the form
 *       GMATtmpSPKcmmnt<objName>.txt
 * where <objName> is the name of the object for whom the SPK file is created
 *
 * If the code is unable to create the temporary file (e.g., because of a permission
 * problem), the SPK file will still be generated but will contain no META-data.
 *
 */
//------------------------------------------------------------------------------
#include <stdio.h>
#include <sstream>

#include "SpiceOrbitKernelWriter.hpp"
#include "MessageInterface.hpp"
#include "StringUtil.hpp"
#include "TimeTypes.hpp"
#include "TimeSystemConverter.hpp"
#include "UtilityException.hpp"
#include "RealUtilities.hpp"

//#define DEBUG_SPK_WRITING

//---------------------------------
// static data
//---------------------------------


wxString   SpiceOrbitKernelWriter::TMP_TXT_FILE_NAME = wxT("GMATtmpSPKcmmnt");
const Integer SpiceOrbitKernelWriter::MAX_FILE_RENAMES  = 1000; // currently unused
//---------------------------------
// public methods
//---------------------------------
//------------------------------------------------------------------------------
//  SpiceOrbitKernelWriter(const wxString       &objName,
//                         const wxString       &centerName,
//                         Integer                 objNAIFId,
//                         Integer                 centerNAIFId,
//                         const wxString       &fileName,
//                         Integer                 deg,
//                         const wxString       &frame)
//------------------------------------------------------------------------------
/**
 * This method constructs a SpiceKernelWriter instance.
 * (constructor)
 *
 * @param    objName       name of the object for which to write the SPK kernel
 * @param    centerName    name of he central body of the object
 * @param    objNAIFId     NAIF ID for the object
 * @param    centerNAIFId  NAIF ID for the central body
 * @param    fileName      name of the kernel to generate
 * @param    deg           degree of interpolating polynomials
 * @param    frame         reference frame (default = wxT("J2000"))
 *
 */
//------------------------------------------------------------------------------
SpiceOrbitKernelWriter::SpiceOrbitKernelWriter(const wxString      &objName,   const wxString &centerName,
                                              Integer                 objNAIFId,  Integer           centerNAIFId,
                                              const wxString       &fileName,  Integer           deg,
                                              const wxString       &frame) :
   SpiceKernelWriter(),
   objectName      (objName),
   centralBodyName (centerName),
   kernelFileName  (fileName),
   frameName       (frame),
   fileOpen        (false),
   tmpTxtFile      (NULL),
   fm              (NULL)
{
   #ifdef DEBUG_SPK_WRITING
      MessageInterface::ShowMessage(wxT("Entering constructor for SPKOrbitWriter with fileName = %s, objectName = %s\n"),
            fileName.c_str(), objName.c_str());
   #endif
   if (GmatMathUtil::IsEven(deg)) // degree must be odd for Data Type 13
   {
      wxString errmsg = wxT("Error creating SpiceOrbitKernelWriter: degree must be odd for Data Type 13\n");
      throw UtilityException(errmsg);
   }
   // Check for the default NAIF ID
   if (objNAIFId == SpiceInterface::DEFAULT_NAIF_ID)
   {
      MessageInterface::ShowMessage(
            wxT("*** WARNING *** NAIF ID for object %s is set to the default NAIF ID (%d).  Resulting SPK file will contain that value as the object's ID.\n"),
            objectName.c_str(), objNAIFId);
   }

   // Get the FileManage pointer
   fm = FileManager::Instance();
   // Create the temporary text file to hold the meta data
   tmpTxtFileName = fm->GetAbsPathname(FileManager::OUTPUT_PATH);
   tmpTxtFileName += TMP_TXT_FILE_NAME + objectName + wxT(".txt");
   #ifdef DEBUG_SPK_WRITING
      MessageInterface::ShowMessage(wxT("temporary SPICE file name is: %s\n"), tmpTxtFileName.c_str());
   #endif
   tmpTxtFile = fopen(tmpTxtFileName.char_str(), "w");

   if (!tmpTxtFile)
   {
      wxString errmsg = wxT("Error creating or opening temporary text file for SPK meta data, for object \"");
      errmsg += objectName + wxT("\".  No meta data will be added to the file.\n");
      MessageInterface::PopupMessage(Gmat::WARNING_, errmsg);
      tmpFileOK = false;
   }
   else
   {
      fclose(tmpTxtFile);
      // remove the temporary text file
      remove(tmpTxtFileName.char_str());
      tmpFileOK = true;
   }

   /// set up CSPICE data
   objectNAIFId      = objNAIFId;
   if (centerNAIFId == 0) // need to find the NAIF Id for the central body  @todo - for object too??
      centralBodyNAIFId = GetNaifID(centralBodyName);
   else
      centralBodyNAIFId = centerNAIFId;
   kernelNameSPICE   = kernelFileName.char_str();
   degree            = deg;
   referenceFrame    = frameName.char_str();
   handle            = -999;

   // @todo - do we need to call boddef_c here to set the NAIF ID for the spacecraft????

   // get a file handle here
   SpiceInt        maxChar = MAX_CHAR_COMMENT;
   wxString     internalFileName = wxT("GMAT-generated SPK file for ") + objectName;
   ConstSpiceChar  *internalSPKName  = internalFileName.char_str();
   #ifdef DEBUG_SPK_WRITING
      MessageInterface::ShowMessage(wxT("... attempting to open SPK file with  fileName = %s\n"),
            fileName.c_str());
   #endif
   spkopn_c(kernelNameSPICE, internalSPKName, maxChar, &handle); // CSPICE method to create and open an SPK kernel
   if (failed_c()) // CSPICE method to detect failure of previous call to CSPICE
   {
      // first, try to rename the existing file, as SPICE will not overwrite or
      // append to an existing file - this is the most common error returned from spkopn
      Integer     fileCounter = 0;
      bool        done        = false;
      wxString fileWithBSP = fileName;
      wxString fileNoBSP   = fileWithBSP.erase(fileWithBSP.rfind(wxT(".bsp")));
      wxString fileRename(wxT(""));
      Integer     retCode = 0;
      while (!done)
      {
         fileRename.Clear();
         fileRename << fileNoBSP << wxT("__") << fileCounter << wxT(".bsp");
         if (fm->RenameFile(kernelFileName, fileRename, retCode))
         {
            done = true;
         }
         else
         {
            if (retCode == 0) // if no error from system, but not allowed to overwrite
            {
//               if (fileCounter < MAX_FILE_RENAMES) // no MAX for now
                  fileCounter++;
//               else
//               {
//                  reset_c(); // reset failure flag in SPICE
//                  wxString errmsg = wxT("Error renaming existing SPK file  \"");
//                  errmsg += kernelFileName + wxT("\".  Maximum number of renames exceeded.\n");
//                  throw UtilityException(errmsg);
//               }
            }
            else
            {
               reset_c(); // reset failure flag in SPICE
               wxString errmsg =
                     wxT("Unknown system error occurred when attempting to rename existing SPK file \"");
               errmsg += kernelFileName + wxT("\".\n");
               throw UtilityException(errmsg);
            }
         }

      }
      reset_c(); // reset failure flag in SPICE
      // then try to open the file again (should create a new one)
      spkopn_c(kernelNameSPICE, internalSPKName, maxChar, &handle); // CSPICE method to create and open an SPK kernel
      if (failed_c()) // CSPICE method to detect failure of previous call to CSPICE
      {
         ConstSpiceChar option[]   = "LONG"; // retrieve long error message
         SpiceInt       numErrChar = MAX_LONG_MESSAGE_VALUE;
         //SpiceChar      err[MAX_LONG_MESSAGE_VALUE];
         SpiceChar      *err = new SpiceChar[MAX_LONG_MESSAGE_VALUE];
         getmsg_c(option, numErrChar, err);
         wxString errStr(wxString::FromAscii(err));
         wxString errmsg = wxT("Error getting file handle for SPK file \"");
         errmsg += kernelFileName + wxT("\".  Message received from CSPICE is: ");
         errmsg += errStr + wxT("\n");
         reset_c();
         delete [] err;
         throw UtilityException(errmsg);
      }
   }
   fileOpen = true;
   // set up the wxT("basic") meta data here ...
   SetBasicMetaData();

   // make sure that the NAIF Id is associated with the object name  @todo - need to set center's Id as well sometimes?
   ConstSpiceChar *itsName = objectName.char_str();
   boddef_c(itsName, objectNAIFId);        // CSPICE method to set NAIF ID for an object
   if (failed_c())
   {
      ConstSpiceChar option[]   = "LONG"; // retrieve long error message
      SpiceInt       numErrChar = MAX_LONG_MESSAGE_VALUE;
      //SpiceChar      err[MAX_LONG_MESSAGE_VALUE];
      SpiceChar      *err = new SpiceChar[MAX_LONG_MESSAGE_VALUE];
      getmsg_c(option, numErrChar, err);
      wxString errStr(wxString::FromAscii(err));
      wxString ss(wxT(""));
      ss << wxT("Unable to set NAIF Id for object \"") << objectName << wxT("\" to the value ");
      ss << objNAIFId << wxT(".  Message received from CSPICE is: ");
      ss << errStr << wxT("\n");
      reset_c();
      delete [] err;
      throw UtilityException(ss);
   }
}

//------------------------------------------------------------------------------
//  SpiceOrbitKernelWriter(const SpiceOrbitKernelWriter &copy)
//------------------------------------------------------------------------------
/**
 * This method constructs a SpiceKernelWriter instance, copying data from the
 * input instance.
 * (copy constructor)
 *
 * @param    copy       object to copy
 *
 */
//------------------------------------------------------------------------------
SpiceOrbitKernelWriter::SpiceOrbitKernelWriter(const SpiceOrbitKernelWriter &copy) :
   SpiceKernelWriter(copy),
   objectName        (copy.objectName),
   centralBodyName   (copy.centralBodyName),
   kernelFileName    (copy.kernelFileName),
   frameName         (copy.frameName),
   objectNAIFId      (copy.objectNAIFId),
   centralBodyNAIFId (copy.centralBodyNAIFId),
   degree            (copy.degree),
   handle            (copy.handle),
   basicMetaData     (copy.basicMetaData),
   addedMetaData     (copy.addedMetaData),
   fileOpen          (copy.fileOpen),    // ??
   tmpTxtFile        (copy.tmpTxtFile),
   fm                (copy.fm)// ??
{
   kernelNameSPICE  = kernelFileName.char_str();
   referenceFrame   = frameName.char_str();
}

//------------------------------------------------------------------------------
//  SpiceOrbitKernelWriter& operator=(const SpiceOrbitKernelWriter &copy)
//------------------------------------------------------------------------------
/**
 * This method copies data from the input SpiceKernelWriter instance to
 * wxT("this") instance.
 *
 * @param    copy       object whose data to copy
 *
 */
//------------------------------------------------------------------------------
SpiceOrbitKernelWriter& SpiceOrbitKernelWriter::operator=(const SpiceOrbitKernelWriter &copy)
{
   if (&copy != this)
   {
      SpiceKernelWriter::operator=(copy);
      objectName        = copy.objectName;
      centralBodyName   = copy.centralBodyName;
      kernelFileName    = copy.kernelFileName;
      frameName         = copy.frameName;
      objectNAIFId      = copy.objectNAIFId;
      centralBodyNAIFId = copy.centralBodyNAIFId;
      degree            = copy.degree;
      handle            = copy.handle;
      basicMetaData     = copy.basicMetaData;
      addedMetaData     = copy.addedMetaData;
      fileOpen          = copy.fileOpen; // ??
      tmpTxtFile        = copy.tmpTxtFile; // ??
      fm                = copy.fm;

      kernelNameSPICE   = kernelFileName.char_str();
      referenceFrame    = frameName.char_str();
   }

   return *this;
}

//------------------------------------------------------------------------------
//  ~SpiceOrbitKernelWriter()
//------------------------------------------------------------------------------
/**
 * This method deletes wxT("this") SpiceKernelWriter instance.
 * (destructor)
 *
 */
//------------------------------------------------------------------------------
SpiceOrbitKernelWriter::~SpiceOrbitKernelWriter()
{
   if (fileOpen) FinalizeKernel();
}


//------------------------------------------------------------------------------
//  SpiceOrbitKernelWriter* Clone(void) const
//------------------------------------------------------------------------------
/**
 * This method clones the object.
 *
 * @return new object, cloned from wxT("this") object.
 *
 */
//------------------------------------------------------------------------------
SpiceOrbitKernelWriter* SpiceOrbitKernelWriter::Clone(void) const
{
   SpiceOrbitKernelWriter * clonedSKW = new SpiceOrbitKernelWriter(*this);

   return clonedSKW;
}

//------------------------------------------------------------------------------
//  void WriteSegment(const A1Mjd &start, const A1Mjd &end,
//                    const StateArray &states, const EpochArray &epochs)
//------------------------------------------------------------------------------
/**
 * This method writes a segment to the SPK kernel.
 *
 * @param   start    start time of the segment data
 * @param   end      end time of the segment data
 * @param   states   array of states to write to the segment
 * @param   epochs   array or corresponding epochs
 *
 */
//------------------------------------------------------------------------------
void SpiceOrbitKernelWriter::WriteSegment(const A1Mjd &start, const A1Mjd &end,
                                     const StateArray &states, const EpochArray &epochs)
{
   SpiceInt numStates = states.size();
   if ((Integer) epochs.size() != (Integer) numStates)
   {
      wxString errmsg = wxT("Error writing segment to SPK file \"");
      errmsg += kernelFileName + wxT("\" - size of epoch array does not match size of state array.\n");
      throw UtilityException(errmsg);
   }
   // do time conversions here, for start, end, and all epochs
   SpiceDouble  startSPICE = A1ToSpiceTime(start.Get());

   SpiceDouble  endSPICE = A1ToSpiceTime(end.Get());

   #ifdef DEBUG_SPK_WRITING
      MessageInterface::ShowMessage(wxT("In WriteSegment, epochs are:\n"));
   #endif
   SpiceDouble  *epochArray;     // (deleted at end of method)
   epochArray = new SpiceDouble[numStates];
   for (Integer ii = 0; ii < numStates; ii++)
   {
      epochArray[ii] = A1ToSpiceTime(epochs.at(ii)->Get());
      #ifdef DEBUG_SPK_WRITING
         MessageInterface::ShowMessage(wxT("epochArray[%d] = %12.10f\n"), ii, (Real) epochArray[ii]);
      #endif
   }

   // put states into SpiceDouble arrays
   SpiceDouble  *stateArray;
   stateArray = new SpiceDouble[numStates * 6];

   for (Integer ii = 0; ii < numStates; ii++)
   {
      for (Integer jj = 0; jj < 6; jj++)
      {
         stateArray[(ii*6) + jj] = ((states.at(ii))->GetDataVector())[jj];
      }
   }

   // create a segment ID
   wxString         segmentID = wxT("SPK_SEGMENT");
   ConstSpiceChar      *segmentIDSPICE = segmentID.char_str();

   // pass data to CSPICE method that writes a segment to a Data Type 13 kernel
   spkw13_c(handle, objectNAIFId, centralBodyNAIFId, referenceFrame, startSPICE,
            endSPICE, segmentIDSPICE, degree, numStates, stateArray, epochArray);

   if(failed_c())
   {
      ConstSpiceChar option[]   = "LONG"; // retrieve long error message
      SpiceInt       numErrChar = MAX_LONG_MESSAGE_VALUE;
      //SpiceChar      err[MAX_LONG_MESSAGE_VALUE];
      SpiceChar      *err = new SpiceChar[MAX_LONG_MESSAGE_VALUE];
      getmsg_c(option, numErrChar, err);
      wxString errStr(wxString::FromAscii(err));
      wxString errmsg = wxT("Error writing ephemeris data to SPK file \"");
      errmsg += kernelFileName + wxT("\".  Message received from CSPICE is: ");
      errmsg += errStr + wxT("\n");
      reset_c();
      delete [] err;
      throw UtilityException(errmsg);
   }
   delete [] epochArray;
   delete [] stateArray;
}

//------------------------------------------------------------------------------
//  void AddMetaData(const wxString &line, bool done)
//------------------------------------------------------------------------------
/**
 * This method writes meta data (comments) to the SPK kernel.
 *
 * @param   line    line of comments to add.
 * @param   bool    indicates whether or not this is the last line to add
 *                  (if so, file can be finalized)
 */
//------------------------------------------------------------------------------
void SpiceOrbitKernelWriter::AddMetaData(const wxString &line, bool done)
{
   if (!fileOpen)
   {
      wxString errmsg = wxT("Unable to add meta data to SPK kernel \"");
      errmsg += kernelFileName + wxT("\".  File has been finalized and closed.\n");
      throw UtilityException(errmsg);
   }
   addedMetaData.push_back(line);

   if (done)
      FinalizeKernel();
}

//------------------------------------------------------------------------------
//  void AddMetaData(const StringArray &lines, bool done)
//------------------------------------------------------------------------------
/**
 * This method writes meta data (comments) to the SPK kernel.
 *
 * @param   lines   lines of comments to add.
 * @param   bool    indicates whether or not this is the last line to add
 *                  (if so, file can be finalized)
 */
//------------------------------------------------------------------------------
void SpiceOrbitKernelWriter::AddMetaData(const StringArray &lines, bool done)
{
   if (!fileOpen)
   {
      wxString errmsg = wxT("Unable to add meta data to SPK kernel \"");
      errmsg += kernelFileName + wxT("\".  File has been finalized and closed.\n");
      throw UtilityException(errmsg);
   }
   unsigned int sz = lines.size();
   for (unsigned int ii = 0; ii < sz; ii++)
      addedMetaData.push_back(lines.at(ii));

   if (done)
      FinalizeKernel();
}

//---------------------------------
// protected methods
//---------------------------------
// none at this time

//---------------------------------
// private methods
//---------------------------------

//------------------------------------------------------------------------------
//  SetBasicMetaData()
//------------------------------------------------------------------------------
/**
 * This method sets the 'basic' (i.e. written to every kernel)  meta data
 * (comments).
 *
 */
//------------------------------------------------------------------------------
void SpiceOrbitKernelWriter::SetBasicMetaData()
{
   basicMetaData.clear();
   wxString metaDataLine = wxT("--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---\n");
   basicMetaData.push_back(metaDataLine);
   metaDataLine = (wxT("SPK EPHEMERIS kernel for object ") + objectName) + wxT("\n");
   basicMetaData.push_back(metaDataLine);
   metaDataLine = wxT("Generated on ");
   metaDataLine += GmatTimeUtil::FormatCurrentTime();
   metaDataLine += wxT("\n");
   basicMetaData.push_back(metaDataLine);
   metaDataLine = wxT("Generated by the General Mission Analysis Tool (GMAT) [Build ");
   metaDataLine += wxT(__DATE__);
   metaDataLine += wxT(" at ");
   metaDataLine += wxT(__TIME__);
   metaDataLine += wxT("]\n");
   basicMetaData.push_back(metaDataLine);
   metaDataLine = wxT("--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---\n");
   basicMetaData.push_back(metaDataLine);
}

//------------------------------------------------------------------------------
//  FinalizeKernel()
//------------------------------------------------------------------------------
/**
 * This method writes the meta data (comments) to the kernel and then closes it.
 *
 */
//------------------------------------------------------------------------------
void SpiceOrbitKernelWriter::FinalizeKernel()
{
   // write all the meta data to the file
   if (tmpFileOK) WriteMetaData();
   basicMetaData.clear();
   addedMetaData.clear();
   // close the SPK file
   spkcls_c(handle);
   fileOpen = false;
}


//------------------------------------------------------------------------------
//  WriteMetaData()
//------------------------------------------------------------------------------
/**
 * This method writes the meta data (comments) to the kernel.
 *
 */
//------------------------------------------------------------------------------
void SpiceOrbitKernelWriter::WriteMetaData()
{
   // open the temporary file for writing the metadata
   tmpTxtFile = fopen(tmpTxtFileName.char_str(), "w");

   // write the meta data to the temporary file (according to SPICE documentation, must use regular C routines)
   // close the temporary file, when done
   unsigned int basicSize = basicMetaData.size();
   unsigned int addedSize = addedMetaData.size();
   for (unsigned int ii = 0; ii < basicSize; ii++)
      fprintf(tmpTxtFile, "%s", (char *)(basicMetaData[ii]).char_str());
   fprintf(tmpTxtFile,"\n");
   for (unsigned int ii = 0; ii < addedSize; ii++)
      fprintf(tmpTxtFile, "%s", (char *)(addedMetaData[ii]).char_str());
   fprintf(tmpTxtFile,"\n");
   fflush(tmpTxtFile);
   fclose(tmpTxtFile);

   // write the meta data to the SPK file comment area by telling it to read the
   // temporary text file
   Integer     txtLength = tmpTxtFileName.length();
//   char        tmpTxt[txtLength+1]; // MSVC doesn't like this allocation
   char        *tmpTxt;    // (deleted at end of method)
   tmpTxt = new char[txtLength + 1];
   for (Integer jj = 0; jj < txtLength; jj++)
      tmpTxt[jj] = tmpTxtFileName.at(jj);
   tmpTxt[txtLength] = '\0';
   integer     unit;
   char        blank[2] = " ";
   ftnlen      txtLen = txtLength + 1;
   txtopr_(tmpTxt, &unit, txtLen);         // CSPICE method to open text file for reading
   spcac_(&handle, &unit, blank, blank, 1, 1); // CSPICE method to write comments to kernel
   if (failed_c())
   {
      ConstSpiceChar option[]   = "LONG"; // retrieve long error message
      SpiceInt       numErrChar = MAX_LONG_MESSAGE_VALUE;
      //SpiceChar      err[MAX_LONG_MESSAGE_VALUE];
      SpiceChar      *err = new SpiceChar[MAX_LONG_MESSAGE_VALUE];
      getmsg_c(option, numErrChar, err);
      wxString errStr(wxString::FromAscii(err));
      wxString errmsg = wxT("Error writing meta data to SPK file \"");
      errmsg += kernelFileName + wxT("\".  Message received from CSPICE is: ");
      errmsg += errStr + wxT("\n");
      reset_c();
      delete [] err;
      throw UtilityException(errmsg);
   }
   // close the text file
   ftncls_c(unit);                         // CSPICE method to close the text file
   // remove the temporary text file
   remove(tmpTxtFileName.char_str());
   delete [] tmpTxt;
}

//---------------------------------
// private methods
//---------------------------------

//------------------------------------------------------------------------------
//  SpiceOrbitKernelWriter()
//------------------------------------------------------------------------------
/**
 * This method constructs an instance of SpiceOrbitKernelWriter.
 * (default constructor)
 *
 */
//------------------------------------------------------------------------------
SpiceOrbitKernelWriter::SpiceOrbitKernelWriter()
{
};
