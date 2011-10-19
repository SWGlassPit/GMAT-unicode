//$Id$
//------------------------------------------------------------------------------
//                           HarmonicGravityGrv
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: John P. Downing/GSFC/595
// Created: 2010.10.28
// (modified for style, etc. by Wendy Shoan/GSFC/583 2011.06.04)
//
/**
 * This is the class that loads data from a GRV type gravity file.
 */
//------------------------------------------------------------------------------
#include "HarmonicGravityGrv.hpp"
#include "ODEModelException.hpp"
#include "UtilityException.hpp"
#include "StringUtil.hpp"       
#include <wx/wfstream.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <stdlib.h>           // For atoi

//------------------------------------------------------------------------------
// static data
//------------------------------------------------------------------------------
// n/a

//------------------------------------------------------------------------------
HarmonicGravityGrv::HarmonicGravityGrv(const wxString& filename,
                                       const Real& radius, const Real& mukm) :
   HarmonicGravity (filename)
{
   bodyRadius = radius;
   factor     = -mukm;
   Load();
}

//------------------------------------------------------------------------------
HarmonicGravityGrv::~HarmonicGravityGrv()
{
}

//------------------------------------------------------------------------------
void HarmonicGravityGrv::Load()
{
   wxFFileInputStream inStreamFile(gravityFilename);
   wxTextInputStream inStream(inStreamFile);
   if (!inStreamFile.IsOk())
      throw GravityFileException(wxT("Cannot open GRV gravity file \"") + gravityFilename + wxT("\""));

   #ifdef DEBUG_GRAVITY_GRV_FILE
   MessageInterface::ShowMessage(wxT("Entered GravityFile::ReadGrvFile\n"));
   #endif

   wxString isNormalized = wxT("");
   wxString line;
   wxString firstStr;

   // Read header line
   line = inStream.ReadLine();

   while (!inStreamFile.Eof())
   {
      line = inStream.ReadLine();

      if (line == wxT(""))
         continue;

      wxStringInputStream lineStreamString(line);
      wxTextInputStream lineStream(lineStreamString);

      // ignore comment lines
      if (line[0] != '#')
      {
         lineStream >> firstStr;
         if (firstStr == wxT("END")) break;

         wxString upperString = GmatStringUtil::ToUpper(firstStr);

         // ignore the stk version and blank lines
         if ((upperString == wxT("MODEL")) ||
             (upperString ==wxT("BEGIN")))
         {
            // do nothing - we don't need to know this
         }
         else if (upperString == wxT("DEGREE"))
         {
            lineStream >> NN;
         }
         else if (upperString == wxT("ORDER"))
         {
            lineStream >> MM;
         }
         else if (upperString == wxT("GM"))
         {
            Real tmpMu = 0.0;
            lineStream >> tmpMu;
            if (tmpMu != 0.0)
               factor = -tmpMu / 1.0e09;     // -> Km^3/sec^2
         }
         else if (upperString == wxT("REFDISTANCE"))
         {
            Real tmpA  = 0.0;
            lineStream >> tmpA;
            if (tmpA != 0.0)
               bodyRadius = tmpA / 1000.0;  // -> Km
         }
         else if (upperString == wxT("NORMALIZED"))
         {
            lineStream >> isNormalized;
         }
         else
         {
            Allocate ();

            Integer n = -1;
            Integer m = -1;
            Real cnm = 0.0;
            Real snm = 0.0;
            // Ensure that m and n fall in the allowed ranges
            //n = (Integer) atoi(firstStr.c_str());
            long tmp;
            firstStr.ToLong(&tmp);
            n = (Integer) (tmp);
            if ((n > 0) && (n < NN))
            {
               lineStream >> m;
               if ((m >= 0) && (m <= n))
               {
                  lineStream >> cnm >> snm;
                  if (isNormalized == wxT("No"))
                     {
                     cnm *= V[n][m];
                     snm *= V[n][m];
                     }
                  C[n][m] = (Real)cnm;
                  S[n][m] = (Real)snm;
               }
            }
         }
      }
   }

   #ifdef DEBUG_GRAVITY_GRV_FILE
   if (loadcoef)
   {
      MessageInterface::ShowMessage(wxT("Leaving GravityFile::ReadGrvFile\n"));
      MessageInterface::ShowMessage(wxT("   cbar[ 2][ 0] = %le   sbar[ 2][ 0] = %le   \n"),
                                    cbar[2][0], sbar[2][0]);
      MessageInterface::ShowMessage(wxT("   cbar[20][20] = %le   sbar[20][20] = %le   \n"),
                                    cbar[20][20], sbar[20][20]);
   }
   #endif
}
