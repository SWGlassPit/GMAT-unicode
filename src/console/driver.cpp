//$Id: driver.cpp 9517 2011-04-30 21:57:41Z djcinsb $
//------------------------------------------------------------------------------
//                           TestScriptInterpreter driver
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Darrel J. Conway
// Created: 2003/08/28
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
/**
 * Program entry point for TestScriptInterpreter.
 */
//------------------------------------------------------------------------------


#include "driver.hpp" 

#include <fstream>

#include "BaseException.hpp"
#include "ConsoleAppException.hpp"
#include "ConsoleMessageReceiver.hpp"
#include "Moderator.hpp"

#include "CommandFactory.hpp"
#include "PointMassForce.hpp"
#include "PrintUtility.hpp"


//------------------------------------------------------------------------------
//  void ShowHelp()
//------------------------------------------------------------------------------
/**
 * Lists the commands available for the application.
 */
//------------------------------------------------------------------------------
void ShowHelp()
{
   std::cout << _T("Usage: One of the following\n")  
             << _T("   TestScriptInterpreter\n")
             << _T("   TestScriptInterpreter ScriptFileName\n")
             << _T("   TestScriptInterpreter <option> <string>\n\n")
             << _T("The first selection runs an interactive session.\n")
             << _T("The second runs the input script once and then exits.\n")
             << _T("The third selection executes specific testing scenarios.\n\n") 
             << _T("Valid options are:\n")
             << _T("   --help               Shows available options\n")
             << _T("   --save               Saves current script (interactive ")
             << _T("mode only)\n")
             << _T("   --summary            Writes command summary (interactive ")
             << _T("mode only)\n")
             << _T("   --batch <filename>   ")
             << _T("Runs multiple scripts listed in specified file\n")
             << _T("   --verbose <on/off>   ")
             << _T("Toggles display of command sequence prior to a run\n")
             << _T("                        This option is set on the startup line\n")
             << _T("                        (default is on)\n")
             << std::endl;   
}


//------------------------------------------------------------------------------
// void RunScriptInterpreter(wxString script, int verbosity, bool batchmode)
//------------------------------------------------------------------------------
/**
 * Executes a script.
 * 
 * @param <script> The script file that is run.
 * @param <verbosity> Toggles the display of the command list (from the script)
 *                    on or off.  Likely to become more robust over time.
 * @param <batchmode> Flag indicating if the script is part of a batch or a
 *                    single script.
 */
//------------------------------------------------------------------------------
void RunScriptInterpreter(wxString script, int verbosity, bool batchmode)
{
   static bool moderatorInitialized = false;
   
   std::ifstream fin(script.c_str());
   if (!(fin)) {
      wxString errstr = _T("Script file ");
      errstr += script;
      errstr += _T(" does not exist");
      if (!batchmode) {
         std::cout << errstr << std::endl;
         return;
      }
      else
         throw  ConsoleAppException(errstr);
   }
   else
      fin.close();

   ConsoleMessageReceiver *theMessageReceiver = 
      ConsoleMessageReceiver::Instance();
   MessageInterface::SetMessageReceiver(theMessageReceiver);
   
   Moderator *mod = Moderator::Instance();
   
   if (!moderatorInitialized) {
      if (!mod->Initialize()) {
         throw ConsoleAppException(_T("Moderator failed to initialize!"));
      }
      
      moderatorInitialized = true;
   }
   
   try
   {
      if (!mod->InterpretScript(script)) {
         if (!batchmode) {
            std::cout << _T("\n***Could not read script.***\n\n");
            ShowHelp();
         }
         else
            throw ConsoleAppException(_T("Script file did not parse"));
         return;
      }
   }
   catch (BaseException &oops)
   {
      std::cout << _T("ERROR!!!!!! ---- ") << oops.GetFullMessage();
   }
   // print out the sequence
   GmatCommand *top = mod->GetFirstCommand();
   if (verbosity != 0)
   {
      PrintUtility* pu = PrintUtility::Instance();
      pu->PrintEntireSequence(top);
   }
   
   // And now run it
   if (mod->RunMission() != 1)
      throw ConsoleAppException(_T("Moderator::RunMission failed"));
   
   // Success!
   if (!batchmode)
      std::cout << _T("\n\n*** GMAT Integration test ")
                << _T("(Console version) successful! ***")
                << _T("\n\n");
}


//------------------------------------------------------------------------------
// Integer RunBatch(wxString& batchfilename)
//------------------------------------------------------------------------------
/**
 * Executes a collection of scripts.
 * 
 * @param <batchfilename> The file containing the list of script files to run.
 * 
 * @return The number of lines parsed from teh batch file.
 */
//------------------------------------------------------------------------------
Integer RunBatch(wxString& batchfilename)
{
   Integer count = 0, successful = 0, failed = 0, skipped = 0;
   wxString script;
   StringArray failedScripts;
   StringArray skippedScripts;
      
   std::cout << _T("Running batch file \"") << batchfilename << _T("\"") << std::endl;
   std::ifstream batchfile(batchfilename.c_str());
   
   if (!(batchfile)) {
      wxString errstr = _T("Batch file ");
      errstr += batchfilename;
      errstr += _T(" does not exist");
      std::cout << errstr << std::endl;
         return 0;
   }
   
   batchfile >> script;

   while (!batchfile.eof()) {
      if (script == _T("--summary"))
      {
         ShowCommandSummary();
      }
      else
      {
         ++count;
         if (script[0] != '%') {
            std::cout << _T("\n*************************************************\n*** ") 
                      << count << _T(": \"") << script 
                      << _T("\"\n*************************************************\n") 
                      << std::endl;
            try {
               RunScriptInterpreter(script, 0, true);
               ++successful;
            }
            catch (BaseException &ex) {
               std::cout << _T("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n")
                         << _T("!!!\n")
                         << _T("!!! Exception in script \"") << script << _T("\"\n")
                         << _T("!!!    \"")
                         << ex.GetFullMessage() << _T("\"\n") 
                         << _T("!!!\n")
                         << _T("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n")
                         << std::endl;
                         
               ++failed;
               failedScripts.push_back(script);
            }
            catch (...)
            {
               std::cout << _T("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n")
                         << _T("!!!\n")
                         << _T("!!! Unhandled Exception in script \"") << script << _T("\"\n")
                         << _T("!!!\n")
                         << _T("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n")
                         << std::endl;
                         
               ++failed;
               failedScripts.push_back(script);
            }
         }
         else {
            std::cout << _T("\n*************************************************\n*** ") 
                      << count << _T(": Skipping script \"") << script.substr(1)
                      << _T("\"\n*************************************************\n") 
                      << std::endl;
            skippedScripts.push_back(script.substr(1));
            ++skipped;
         }
      }
      batchfile >> script;
   }
   
   std::cout << _T("\n\n**************************************\n*** ")
             << _T("Batch Run Statistics:")
             <<               _T("\n***   Successful scripts:  ") 
             << successful << _T("\n***   Failed Scripts:      ")
             << failed     << _T("\n***   Skipped Scripts:     ")
             << skipped    << _T("\n**************************************\n");
             
   if (failed > 0) {
      std::cout << _T("\n**************************************\n") 
                << _T("***   Scripts that failed:\n");
      for (StringArray::iterator i = failedScripts.begin(); 
           i != failedScripts.end(); ++i)
         std::cout << _T("***      ") << *i << _T("\n");
      std::cout << _T("**************************************\n");
   }
   if (skipped > 0) {
      std::cout << _T("\n**************************************\n")<< _T("***   Scripts that were skipped:\n");
      for (StringArray::iterator i = skippedScripts.begin(); 
           i != skippedScripts.end(); ++i)
         std::cout << _T("***      ") << *i << _T("\n");
      std::cout << _T("**************************************\n\n");
   }

   return count;
}


//------------------------------------------------------------------------------
// void SaveScript(wxString filename)
//------------------------------------------------------------------------------
/**
 * Saves the current script to a file
 * 
 * @param <filename> The name of the script file.
 */
//------------------------------------------------------------------------------
void SaveScript(wxString filename)
{
    Moderator *mod = Moderator::Instance();
    mod->SaveScript(filename);
    std::cout << _T("\n\n");
}


//------------------------------------------------------------------------------
// void ShowCommandSummary(wxString filename)
//------------------------------------------------------------------------------
/**
 * Displays the command summary, either on screen or writing to a file
 * 
 * @param <filename> The name of the summary file.
 */
//------------------------------------------------------------------------------
void ShowCommandSummary(wxString filename)
{
   Moderator *mod = Moderator::Instance();
   GmatCommand *cmd = mod->GetFirstCommand();
   if (cmd->GetTypeName() == _T("NoOp"))
      cmd = cmd->GetNext();
   
   if (cmd == NULL)
   {
      std::cout << _T("Command stream is empty.\n\n");
      return;
   }
   
   if (filename == _T(""))
   {
      std::cout << _T("\n\n");
      wxString summary = cmd->GetStringParameter(_T("MissionSummary"));
      std::cout << summary << _T("\n\n");
   }
   else
   {
      std::cout << _T("File output for command summaries is not yet available\n\n");
   }
}


//------------------------------------------------------------------------------
// void TestSyncModeAccess(wxString filename)
//------------------------------------------------------------------------------
/**
 * Tests the propsync script.
 * 
 * @note This looks like old code that should be removed.
 * 
 * @param <filename> The name of the script file.  (Not used)
 */
//------------------------------------------------------------------------------
void TestSyncModeAccess(wxString filwwename)
{
    Moderator *mod = Moderator::Instance();
    
    // First load up the Moderator with the propsync script
    RunScriptInterpreter(_T("propsync.script"), 1);
    std::cout << _T("\n\n");
    
    // Find the command entry point
    GmatCommand *cmd = mod->GetFirstCommand();
    StringArray props, sats;
    
    while (cmd) {
       if (cmd->GetTypeName() == _T("Propagate")) {
          std::cout << _T("Found \"") << cmd->GetGeneratingString() << _T("\"\n");
          std::cout << _T("Current propagation mode is \"") 
                    << cmd->GetStringParameter(_T("PropagateMode"))
                    << _T("\"\n");
          props = cmd->GetStringArrayParameter(_T("Propagator"));
          for (Integer i = 0; i < (Integer) props.size(); ++i) {
             std::cout << _T("  Propagator: ") << props[i] << _T("\n");
             sats = cmd->GetStringArrayParameter(_T("Spacecraft"), i);
             for (Integer i = 0; i < (Integer) sats.size(); ++i) {
                std::cout << _T("    SpaceObject: ") << sats[i] << _T("\n");
             }
          }
          
          // Now try clearing this puppy
          std::cout << _T("*** Testing the \"Clear\" action\n");
          cmd->TakeAction(_T("Clear"));
          std::cout << _T("Current propagation mode is \"") 
                    << cmd->GetStringParameter(_T("PropagateMode"))
                    << _T("\"\n");
          props = cmd->GetStringArrayParameter(_T("Propagator"));
          for (Integer i = 0; i < (Integer) props.size(); ++i) {
             std::cout << _T("  Propagator: ") << props[i] << _T("\n");
             sats = cmd->GetStringArrayParameter(_T("Spacecraft"), i);
             for (Integer i = 0; i < (Integer) sats.size(); ++i) {
                std::cout << _T("    SpaceObject: ") << sats[i] << _T("\n");
             }
          }
          
          // Now add in some bogus data
          std::cout << _T("*** Testing the \"SetString\" method: \"\", ")
                    << _T("\"Bogus\", \"Synchronized\"\n");
          cmd->SetStringParameter(_T("PropagateMode"), _T(""));
          std::cout << _T("Current propagation mode is \"") 
                    << cmd->GetStringParameter(_T("PropagateMode"))
                    << _T("\"\n");
//          cmd->SetStringParameter(_T("PropagateMode"), _T("Bogus"));
          std::cout << _T("Current propagation mode is \"") 
                    << cmd->GetStringParameter(_T("PropagateMode"))
                    << _T("\"\n");
          cmd->SetStringParameter(_T("PropagateMode"), _T("Synchronized"));
          std::cout << _T("Current propagation mode is \"") 
                    << cmd->GetStringParameter(_T("PropagateMode"))
                    << _T("\"\n");

          std::cout << _T("Setting the stooges as the PropSetups\n");
          cmd->SetStringParameter(_T("Propagator"), _T("Moe"));
          cmd->SetStringParameter(_T("Propagator"), _T("Curly"));
          cmd->SetStringParameter(_T("Propagator"), _T("Larry"));
          std::cout << _T("Setting the dwarfs as the Spacecraft\n");
          cmd->SetStringParameter(_T("Spacecraft"), _T("Dopey"), 0);
          cmd->SetStringParameter(_T("Spacecraft"), _T("Sleepy"), 1);
          cmd->SetStringParameter(_T("Spacecraft"), _T("Doc"), 2);
          cmd->SetStringParameter(_T("Spacecraft"), _T("Happy"), 0);
          cmd->SetStringParameter(_T("Spacecraft"), _T("Grumpy"), 1);
          cmd->SetStringParameter(_T("Spacecraft"), _T("Bashful"), 2);
          cmd->SetStringParameter(_T("Spacecraft"), _T("Sneezy"), 0);
          
          props = cmd->GetStringArrayParameter(_T("Propagator"));
          for (Integer i = 0; i < (Integer) props.size(); ++i) {
             std::cout << _T("  Propagator: ") << props[i] << _T("\n");
             sats = cmd->GetStringArrayParameter(_T("Spacecraft"), i);
             for (Integer i = 0; i < (Integer) sats.size(); ++i) {
                std::cout << _T("    SpaceObject: ") << sats[i] << _T("\n");
             }
          }
       }
       
       cmd = cmd->GetNext();
    }
    
    std::cout << _T("\n\n");
}


//------------------------------------------------------------------------------
// void DumpDEData(double secsToStep)
//------------------------------------------------------------------------------
/**
 * Writes out the Earth and Moon position and velocity data for a set span to
 * the file EarthMoonDe.txt
 *
 * @param secsToStep The timestep to use
 * @param spanInSecs The time span in seconds
 */
//------------------------------------------------------------------------------
void DumpDEData(double secsToStep, double spanInSecs)
{
   double baseEpoch = 21545.0, currentEpoch = 21545.0;
   long step = 0;
   std::ofstream data(_T("EarthMoonDe.txt"));

   Moderator *mod = Moderator::Instance();
   if (!mod->Initialize()) {
      throw ConsoleAppException(_T("Moderator failed to initialize!"));
   }

   SolarSystem *sol = mod->GetSolarSystemInUse();
   if (sol == NULL)
      MessageInterface::ShowMessage(_T("Oh no, the solar system is NULL!"));

   CelestialBody *earth = sol->GetBody(_T("Earth"));
   CelestialBody *moon  = sol->GetBody(_T("Luna"));

   data << _T("Earth and Moon Position and Velocity from the DE file\n\n");

   data.precision(17);
   double targetEpoch = currentEpoch + spanInSecs / 86400.0;
   while (currentEpoch <= targetEpoch)
   {
      currentEpoch = baseEpoch + step * secsToStep / 86400.0;

      Rvector6 earthRV = earth->GetMJ2000State(currentEpoch);
      Rvector6 moonRV  = moon->GetMJ2000State(currentEpoch);
      Rvector3 moonAcc = moon->GetMJ2000Acceleration(currentEpoch);

      data << currentEpoch << _T(" ") << (step * secsToStep) << _T(" ")
//           << earthRV[0] << _T(" ") << earthRV[1] << _T(" ") << earthRV[2] << _T(" ")
//           << earthRV[3] << _T(" ") << earthRV[4] << _T(" ") << earthRV[5] << _T(" | ")
           << moonRV[0] << _T(" ") << moonRV[1] << _T(" ") << moonRV[2] << _T(" ")
           << moonRV[3] << _T(" ") << moonRV[4] << _T(" ") << moonRV[5]
           << moonAcc[0] << _T(" ") << moonAcc[1] << _T(" ") << moonAcc[2]
           << _T("\n");

      ++step;
   }
   data << std::endl;
}

//------------------------------------------------------------------------------
// int main(int argc, char *argv[])
//------------------------------------------------------------------------------
/**
 * The program entry point.
 * 
 * @param <argc> The count of the input arguments.
 * @param <argv> The input arguments.
 * 
 * @return 0 on success.
 */
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
   try {
      wxString msg = _T("General Mission Analysis Tool\nConsole Based Version\n");

      msg += _T("Build Date: ");
      msg += __DATE__;
      msg += _T("  ");
      msg += __TIME__;
      
      std::cout << _T("\n********************************************\n")
                << _T("***  GMAT Console Application\n")
                << _T("********************************************\n\n")
                << msg << _T("\n\n")
                << std::endl;
      
      char scriptfile[1024];
      bool runcomplete = false;
      int verbosity = 1;
      wxString optionParm = _T("");
      StringArray parms;
      
      do {
         if (argc < 2) {
            std::cout << _T("Enter a script file, ") 
                      << _T("q to quit, or an option:  ");
            
            std::cin >> scriptfile;
            // Drop the return character -- may be platform dependent
            std::cin.ignore(1);
            
            parms.clear();
            wxString chunk;
            // Integer start = 0, finish;
         }
         else {
            strcpy(scriptfile, argv[1]);
            if (argc == 3)
               optionParm = argv[2];
            if (optionParm != _T(""))
               std::cout << _T("Optional parameter: \"") << optionParm << _T("\"\n");
         }
            
         if ((!strcmp(scriptfile, _T("q"))) || (!strcmp(scriptfile, _T("Q"))))
            runcomplete = true;
            
         if (scriptfile[0] == '-') {
            if (!strcmp(scriptfile, _T("--help"))) {
               ShowHelp();
            }
            else if (!strcmp(scriptfile, _T("--batch"))) {
               RunBatch(optionParm);
            }
            else if (!strcmp(scriptfile, _T("--save"))) {
               SaveScript();
            }
            else if (!strcmp(scriptfile, _T("--summary"))) {
               ShowCommandSummary();
            }
            else if (!strcmp(scriptfile, _T("--sync"))) {
               TestSyncModeAccess();
            }
            else if (!strcmp(scriptfile, _T("--verbose")))
            {
               if (!strcmp(optionParm.c_str(), _T("off")))
                  verbosity = 0;
               std::cout << _T("Verbose mode is ") 
                         << (verbosity == 0 ? _T("off") : _T("on"))
                         << _T("\n");
               argc = 1;
            }
            // Options used for some detailed tests but hidden from casual users
            // (i.e. missing from the help messages)
            else if (!strcmp(scriptfile, _T("--DumpDEData"))) {
               DumpDEData(0.001, 0.2);
            }
            else {
               std::cout << _T("Unrecognized option.\n\n");
               ShowHelp();
            }
         }
         else if (!runcomplete)
            RunScriptInterpreter(scriptfile, verbosity);
         
      } while ((!runcomplete) && (argc < 2));
   }
   catch (BaseException &ex) {
      std::cout << ex.GetFullMessage() << std::endl;
      exit(0);
   }
   
   Moderator::Instance()->Finalize();
   
   return 0;
}

