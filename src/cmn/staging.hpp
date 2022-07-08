#pragma once
#include <list>
#include <string>

// the staging area is N writers / 1 reader
//
// s\XX\dir\...
// s\XX.txt
//
// a lock protects writers from hitting each other; creating the folder reserves the number
// the reader is protected from writers b/c the file is written last

namespace inmem { class config; }

class stagingEntry {
public:
   std::wstring monitorPath;
   time_t backupTime;
   std::wstring pathRoot;

   void save();
   void eraseOnDisk();
};

std::list<stagingEntry> readStagingEntries(inmem::config& c);
stagingEntry reserveStagingEntry(inmem::config& c);
