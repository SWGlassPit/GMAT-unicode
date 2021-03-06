//$Id: GmatApp.cpp 9945 2011-10-06 12:47:21Z wendys-dev $
//------------------------------------------------------------------------------
//                              GmatApp
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
// Author: Linda Jun
// Created: 2003/08/08
//
// 11/24/2003 - D. Conway, Thinking Systems, Inc.
// Changes:
//  - Added test for Unix envirnment, and set the size if the main frame in that
//    environment, so that the Linux build (and, presumably, Unix builds) would
//    appear reasonably sized.
//
/**
 * This class contains GMAT main application. Program starts here.
 */
//------------------------------------------------------------------------------

#include "gmatwxdefs.hpp"
#include "GmatApp.hpp"
#include "GmatMainFrame.hpp"
#include "ViewTextFrame.hpp"
#include "GmatAppData.hpp"
#include "Moderator.hpp"
#include <wx/datetime.h>
#include <wx/splash.h>
#include <wx/image.h>
#include <wx/config.h>

#include "MessageInterface.hpp"
#include "PlotInterface.hpp"
#include "GuiMessageReceiver.hpp"
#include "GuiPlotReceiver.hpp"
#include "GuiInterpreter.hpp"
#include "FileUtil.hpp"

//#define DEBUG_GMATAPP
//#define DEBUG_CMD_LINE

// In single window mode, don't have any child windows; use
// main window.

// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. GmatApp and
// not wxApp)

IMPLEMENT_APP(GmatApp)
    
//------------------------------------------------------------------------------
// GmatApp()
//------------------------------------------------------------------------------
GmatApp::GmatApp()
{
   GuiMessageReceiver *theMessageReceiver = GuiMessageReceiver::Instance();
   MessageInterface::SetMessageReceiver(theMessageReceiver);
   
   GuiPlotReceiver *thePlotReceiver = GuiPlotReceiver::Instance();
   PlotInterface::SetPlotReceiver(thePlotReceiver);
   
   theModerator = (Moderator *)NULL;
   scriptToRun = wxT("");
   showMainFrame = true;
   runScript = false;
   runBatch = false;
   startMatlabServer = false;
}


//------------------------------------------------------------------------------
// OnInit()
//------------------------------------------------------------------------------
/**
 * The execution of Main program starts here.
 */
//------------------------------------------------------------------------------
bool GmatApp::OnInit()
{
   bool status = false;
   
   // Moved from TrajPlotCanvas.cpp (loj: 2009.01.08)
   wxInitAllImageHandlers();
   
   // set application name
   SetAppName(wxT("GMAT"));
   
#if wxUSE_PRINTING_ARCHITECTURE
   // initialize print data and setup
   globalPrintData = new wxPrintData;
   globalPageSetupData = new wxPageSetupDialogData;
#endif // wxUSE_PRINTING_ARCHITECTURE
   
   try
   {
      GmatAppData *gmatAppData = GmatAppData::Instance();
      FileManager *fm = FileManager::Instance();
      wxString startupFile = fm->GetFullStartupFilePath();
      
      // continue work on this (loj: 2008.12.04)
      //@todo: add all files contains gmat_startup_file in
      // startup up directory, and show user to select one
      //---------------------------------------------------------
      #ifdef __GET_STARTUP_FILE_FROM_USER__
      //---------------------------------------------------------
      bool readOtherStartupFile = false;
      
      wxArrayString choices;
      choices.Add(startupFile);
      choices.Add(wxT("Read other startup file"));
      wxString msg = wxT("Please select GMAT startup file to read");
      int result =
         wxGetSingleChoiceIndex(msg, wxT("GMAT Startup File"), 
                                choices, NULL, -1, -1, true, 150, 200);
      if (result == 1)
         readOtherStartupFile = true;
      
      // reading other startup file, save current directory and set it back
      if (readOtherStartupFile)
      {
         wxString filename = 
            ::wxFileSelector(wxT("Choose GMAT startup file"), wxT(""),
                             wxT("gmat_startup_file.txt"), wxT("txt"), wxT("*.*"));
         if (filename != wxT(""))
            startupFile = filename;
      }
      //---------------------------------------------------------      
      #endif
      //---------------------------------------------------------
      
      // create the Moderator - GMAT executive
      theModerator = Moderator::Instance();
      
      // initialize the moderator
      if (theModerator->Initialize(startupFile.c_str(), true))
      {
         GuiInterpreter *guiInterp = GuiInterpreter::Instance();
         theModerator->SetUiInterpreter(guiInterp);
         theModerator->SetInterpreterMapAndSS(guiInterp);
         guiInterp->BuildCreatableObjectMaps();
         
         // get GuiInterpreter
         gmatAppData->SetGuiInterpreter(
               (GuiInterpreter *)theModerator->GetUiInterpreter());
         
         // set default size
         wxSize size = wxSize(800, 600);
         
         // for Windows
         #ifdef __WXMSW__
         size = wxSize(800, 600);
         #endif
         
         // The code above broke the Linux scaling.  This is a temporary hack to 
         // repair it.  PLEASE don't use UNIX macros to detect the Mac code!!!
         #ifdef __LINUX__
            size = wxSize(1024, 768);
         #endif
         // Mac doesn't look right, either
         #ifdef __WXMAC__
            size = wxSize(235,900);
         #endif
            
         
         ProcessCommandLineOptions();
         
         if (!showMainFrame)
            return false;
         
         
         if (GmatGlobal::Instance()->GetGuiMode() != GmatGlobal::MINIMIZED_GUI)
         {
            // Initializes all available image handlers.
            ::wxInitAllImageHandlers();
            
            //show the splash screen
            wxString splashFile = theModerator->GetFileName(wxT("SPLASH_FILE"));
            if (GmatFileUtil::DoesFileExist(splashFile))
            {
               wxImage::AddHandler(new wxTIFFHandler);
               wxBitmap *bitmap = new wxBitmap(splashFile, wxBITMAP_TYPE_TIF);
               
               // Changed to time out in 4 sec (LOJ: 2009.10.07)
               new wxSplashScreen(*bitmap,
                                  wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,
                                  4000, NULL, -1, wxDefaultPosition, wxSize(100, 100),
                                  wxSIMPLE_BORDER|wxSTAY_ON_TOP);
            }
            else
            {
               MessageInterface::ShowMessage
                  (wxT("*** WARNING *** Can't load SPLASH_FILE from '%s'\n"), splashFile.c_str());
            }
         }
         
         wxYield();
         
         theMainFrame =
            new GmatMainFrame((wxFrame *)NULL, -1,
                              wxT("GMAT - General Mission Analysis Tool"),
                              wxDefaultPosition, size,
                              wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL);
         
         wxDateTime now = wxDateTime::Now();
         wxString wxNowStr = now.FormatISODate() + wxT(" ") + now.FormatISOTime() + wxT(" ");
         wxString nowStr = wxNowStr.c_str();
         
         MessageInterface::LogMessage(nowStr + wxT("GMAT GUI successfully launched.\n"));
         
         // Show any errors occured during initialization
         wxString savedMsg = MessageInterface::GetQueuedMessage();
         if (savedMsg != wxT(""))
            MessageInterface::ShowMessage(savedMsg);
         
         #ifdef DEBUG_GMATAPP
         MessageInterface::ShowMessage
            (wxT("GmatApp::OnInit() size=%dx%d\n"), size.GetWidth(), size.GetHeight());
         #endif
         
         // Mac user rather smaller frame and top left corner and show it.
         // (the frames, unlike simple controls, are not shown when created
         // initially)
#ifndef __WXMAC__
         theMainFrame->Maximize();
         theMainFrame->CenterOnScreen(wxBOTH);
#endif

         if (startMatlabServer)
            theMainFrame->StartMatlabServer();
         
         if (GmatGlobal::Instance()->GetGuiMode() == GmatGlobal::MINIMIZED_GUI)
            theMainFrame->Show(false);
         else
         {
            theMainFrame->Show(true);
            theMainFrame->ManageMissionTree();
         }

         if (runScript)
         {
            if (GmatGlobal::Instance()->GetGuiMode() == GmatGlobal::MINIMIZED_GUI)
               theMainFrame->Iconize(true);
            
            theMainFrame->BuildAndRunScript(scriptToRun);
            
            if (GmatGlobal::Instance()->GetRunMode() == GmatGlobal::EXIT_AFTER_RUN)
            {
               theMainFrame->Close();
               #ifdef __LINUX__
                  // Linux needs this to complete shutdown
                  MessageInterface::ShowMessage(wxT("\n"));
               #endif
            }
         }
         else if (runBatch)
         {
            RunBatch();
         }
         status = true;
      }
      else
      {
         // show error messages
         {
            wxBusyCursor bc;
            wxLogWarning(wxT("The Moderator failed to initialize."));
            
            // and if ~wxBusyCursor doesn't do it, then call it manually
            wxYield();
         }
         
         wxLogError(wxT("The error occurred during the initialization.  GMAT will exit"));
         wxLog::FlushActive();
         status = false;
      }
      
      // success: wxApp::OnRun() will be called which will enter the main message
      // loop and the application will run. If we returned FALSE here, the
      // application would exit immediately.
      
      return status;
   }
   catch (BaseException &e)
   {
      wxDateTime now = wxDateTime::Now();
      wxString wxNowStr = now.FormatISODate() + wxT(" ") + now.FormatISOTime() + wxT(" ");
      wxString nowStr = wxNowStr.c_str();
      
      MessageInterface::LogMessage
         (nowStr + wxT("Error encountered while launching GMAT GUI.\n\n"));
      
      MessageInterface::LogMessage(e.GetFullMessage());
      return false;
   }
   catch (...)
   {
      wxDateTime now = wxDateTime::Now();
      wxString wxNowStr = now.FormatISODate() + wxT(" ") + now.FormatISOTime() + wxT(" ");
      wxString nowStr = wxNowStr.c_str();
      
      MessageInterface::LogMessage
         (nowStr + wxT("Unknown error encountered while launching GMAT GUI.\n\n"));
      return false;
   }
}


//------------------------------------------------------------------------------
// OnExit()
//------------------------------------------------------------------------------
int GmatApp::OnExit()
{
   // Moderator destructor is private, so just call Finalize()
   theModerator->Finalize();
   
#if wxUSE_PRINTING_ARCHITECTURE
   // delete global print data and setup
   if (globalPrintData)
      delete globalPrintData;
   if (globalPageSetupData)
      delete globalPageSetupData;
#endif // wxUSE_PRINTING_ARCHITECTURE
   
   wxDateTime now = wxDateTime::Now();
   wxString wxNowStr = now.FormatISODate() + wxT(" ") + now.FormatISOTime() + wxT(" ");
   wxString nowStr = wxNowStr;

   MessageInterface::LogMessage(nowStr + wxT("GMAT GUI exiting.\n"));
   
   return 0;
}


//------------------------------------------------------------------------------
// int FilterEvent(wxEvent& event)
//------------------------------------------------------------------------------
/**
 * Keyboard events go to the component that currently has focus and do not propagate
 * to the parent.
 *
 * This function is called early in event-processing, so we can catch key events
 * globally and do things like F3 for find next.
 */
//------------------------------------------------------------------------------
int GmatApp::FilterEvent(wxEvent& event)
{
   if (theMainFrame != NULL)
   {
      if (event.GetEventType() == wxEVT_KEY_DOWN)
      {
         // Find Next
         if (((wxKeyEvent&)event).GetKeyCode() == WXK_F3)
         {
            theMainFrame->OnFindNext( (wxCommandEvent&)event );
            return true;
         }

         // Find and Replace
         if (((wxKeyEvent&)event).GetKeyCode() == 'H' &&     
             ((wxKeyEvent&)event).GetModifiers() == wxMOD_CONTROL)
         {
            theMainFrame->OnReplaceNext( (wxCommandEvent&)event );
         }
      }
   }
   
   return -1;
}


//------------------------------------------------------------------------------
// void ProcessCommandLineOptions()
//------------------------------------------------------------------------------
void GmatApp::ProcessCommandLineOptions()
{
   wxString commandLineOptions =
      wxT("Valid command line options are:\n")
      wxT("   --help, -h              Shows available options\n")
      wxT("   --version, -v           Shows GMAT build date\n")
      wxT("   --start-server          Starts GMAT server on start-up\n")
      wxT("   --run, -r <scriptname>  Builds and runs the script\n")
      wxT("   --minimize, -m          Minimizes GMAT window\n")
      wxT("   --exit, -x              Exits GMAT after a script is run\n\n");

   #ifdef DEBUG_CMD_LINE
   MessageInterface::ShowMessage(wxT("argc = %d\n"), argc);
   #endif
   
   // Handle any command line arguments
   if (argc > 1)
   {
      for (int i = 1; i < argc; ++i)
      {
         wxString arg = argv[i];
         #ifdef DEBUG_CMD_LINE
         MessageInterface::ShowMessage(wxT("arg = %s\n"), arg.c_str());
         #endif
//         if (arg == "-ms")
         if (arg == wxT("--start-server"))
         {
            startMatlabServer = true;
         }
//         else if (arg == "-date")
         else if ((arg == wxT("--version")) || (arg == wxT("-v")))
         {
            wxString buildDate;
            buildDate.Printf(wxT("Build Date: %s %s\n"), wxT(__DATE__), wxT(__TIME__));
            MessageInterface::ShowMessage(buildDate.c_str());
         }
//         else if (arg == "-br")
         else if ((arg == wxT("--run")) || (arg == wxT("-r")))
         {
            if (argc < 3)
            {
               MessageInterface::ShowMessage
                  (wxT("Please enter script file name to run\n"));
            }
            else
            {
               scriptToRun = argv[i+1];
               // Replace single quotes
               scriptToRun.Replace(wxT("'"), wxT(""), true);
               runScript = true;
               ++i;
               #ifdef DEBUG_CMD_LINE
               MessageInterface::ShowMessage(wxT("%s\n"), scriptToRun.c_str());
               #endif
            }
         }
//         else if (arg == "-help")
         else if ((arg == wxT("--help")) || (arg == wxT("-h")))
         {
            MessageInterface::ShowMessage(commandLineOptions.c_str());
         }
//         else if (arg == "-exit")
         else if ((arg == wxT("--exit")) || (arg == wxT("-x")))
         {
            GmatGlobal::Instance()->SetRunMode(GmatGlobal::EXIT_AFTER_RUN);
         }
//         else if (arg == "-minimize")
         else if ((arg == wxT("--minimize")) || (arg == wxT("-m")))
         {
            GmatGlobal::Instance()->SetGuiMode(GmatGlobal::MINIMIZED_GUI);
         }
         else
         {
            MessageInterface::ShowMessage(wxT("The option \"%s\" is not valid.\n"), arg.c_str());
            MessageInterface::ShowMessage(commandLineOptions.c_str());
            break;
         }
      }
   }
}


//------------------------------------------------------------------------------
// void RunBatch()
//------------------------------------------------------------------------------
void GmatApp::RunBatch()
{
}


