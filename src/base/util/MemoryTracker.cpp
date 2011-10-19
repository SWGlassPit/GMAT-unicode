//$Id$
//------------------------------------------------------------------------------
//                                 MemoryTracker
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
// Created: 2008/11/21
//
/**
 * Implements MemoryTracker class which tracks memory usage. It is a singleton class -
 * only one instance of this class can be created.
 */
//------------------------------------------------------------------------------

#include "MemoryTracker.hpp"
#include "MessageInterface.hpp"

//--------------------------------------
//  initialize static variables
//--------------------------------------
MemoryTracker* MemoryTracker::instance = NULL;

//------------------------------------------------------------------------------
// MemoryTracker* Instance()
//------------------------------------------------------------------------------
MemoryTracker* MemoryTracker::Instance()
{
   if (instance == NULL)
      instance = new MemoryTracker;
   
   return instance;
}


//------------------------------------------------------------------------------
// void SetScript(const wxString &script)
//------------------------------------------------------------------------------
void MemoryTracker::SetScript(const wxString &script)
{
   scriptFile = script;
}


//------------------------------------------------------------------------------
// void SetShowTrace(bool show)
//------------------------------------------------------------------------------
void MemoryTracker::SetShowTrace(bool show)
{
   showTrace = show;
}


//------------------------------------------------------------------------------
// void Add(void *addr, const wxString &objName, const wxString &funName,
//          const wxString &note, void *from)
//------------------------------------------------------------------------------
void MemoryTracker::Add(void *addr, const wxString &objName,
                        const wxString &funName, const wxString &note,
                        void *from)
{
   if (showTrace)
   {
      if (from == NULL)
      {
         MessageInterface::ShowMessage
            (wxT("+++ Creating <%p> %-20s in %s  %s\n"), addr, objName.c_str(),
             funName.c_str(), note.c_str());
      }
      else
      {
         MessageInterface::ShowMessage
            (wxT("+++ Creating <%p> %-20s in %s  %s from <%p>\n"), addr, objName.c_str(),
             funName.c_str(), note.c_str(), from);
      }
   }
   
   TrackType track(wxT("+++"), addr, objName, funName, note, scriptFile);
   memoryTracks.push_back(track);
}


//------------------------------------------------------------------------------
// void Remove(void *addr, const wxString &objName, const wxString &funName,
//             const wxString &note, void *from)
//------------------------------------------------------------------------------
void MemoryTracker::Remove(void *addr, const wxString &objName,
                           const wxString &funName, const wxString &note,
                           void *from)
{
   if (showTrace)
   {
      if (from == NULL)
      {
         MessageInterface::ShowMessage
            (wxT("--- Deleting <%p> %-20s in %s %s\n"), addr, objName.c_str(),
             funName.c_str(), note.c_str());
      }
      else
      {
         MessageInterface::ShowMessage
            (wxT("--- Deleting <%p> %-20s in %s %s from <%p>\n"), addr, objName.c_str(),
             funName.c_str(), note.c_str(), from);
      }
   }
   
   bool trackFound = false;
   std::vector<TrackType>::iterator track = memoryTracks.begin();
   while (track != memoryTracks.end())
   {
      if ((*track).address == addr)
      {
         memoryTracks.erase(track);
         trackFound = true;
         break;
      }
      
      ++track;
   }
   
   if (!trackFound)
   {
      TrackType track(wxT("---"), addr, objName, funName, note, scriptFile);
      memoryTracks.push_back(track);
   }
   
}


//------------------------------------------------------------------------------
// UnsignedInt GetNumberOfTracks()
//------------------------------------------------------------------------------
UnsignedInt MemoryTracker::GetNumberOfTracks()
{
   return memoryTracks.size();
}


//------------------------------------------------------------------------------
// StringArray& GetTracks(bool clearTracks, bool justGetSize, bool writeScriptName)
//------------------------------------------------------------------------------
/**
 * Returns memory tracks
 *
 * @param  clearTracks  Clears track if set to true [true]
 * @param  writeScriptName  Adds script name to track record
 */
//------------------------------------------------------------------------------
StringArray& MemoryTracker::GetTracks(bool clearTracks, bool writeScriptName)
{
   allTracks.clear();
   std::vector<TrackType>::iterator track = memoryTracks.begin();
   static wxString text;
   Integer trackCount = 0;

   #ifdef DEBUG_GET_TRACKS
   MessageInterface::ShowMessage
      (wxT("===> MemoryTracker::GetTracks() There are %d memory tracks\n"),
       memoryTracks.size());
   #endif
   
   while (track != memoryTracks.end() && trackCount < 1000)
   {
      trackCount++;
      wxString script = wxT("");
      if (writeScriptName)
         script = (*track).scriptName;
      text.Printf(wxT("%s <%p> %-20s %-s  %s %s"), ((*track).preface).c_str(), (*track).address,
              ((*track).objectName).c_str(), ((*track).functionName).c_str(),
              ((*track).remark).c_str(), script.c_str());
      allTracks.push_back(text);
      ++track;
   }
   
   if (clearTracks)
      memoryTracks.clear();
   
   #ifdef DEBUG_GET_TRACKS
   MessageInterface::ShowMessage
      (wxT("===> MemoryTracker::GetTracks() returning %d memory tracks\n"), allTracks.size());
   #endif
   
   return allTracks;
}


//------------------------------------------------------------------------------
//  MemoryTracker()
//------------------------------------------------------------------------------
MemoryTracker::MemoryTracker()
{
   showTrace = false;
}

//------------------------------------------------------------------------------
//  ~MemoryTracker()
//------------------------------------------------------------------------------
MemoryTracker::~MemoryTracker()
{
   memoryTracks.clear();
}
