#include "../cmn/shmem-block.hpp"
#include "file.hpp"
#include "staging.hpp"
#include "wlog.hpp"
#include <fstream>
#include <set>
#include <sstream>

void stagingEntry::save()
{
   {
      std::wofstream file((pathRoot + L"._txt").c_str());
      if(!file.good())
         throw std::runtime_error("can't save staging file");
      file << monitorPath << std::endl;
      file << backupTime << std::endl;
   }
   ::MoveFileW(
      (pathRoot + L"._txt").c_str(),
      (pathRoot + L".txt").c_str());
}

void stagingEntry::eraseOnDisk()
{
   getWorkerLog() << L"removing stage " << pathRoot << std::endl;
   {
      BOOL success = ::DeleteFileW((pathRoot + L".txt").c_str());
      if(!success)
         throw std::runtime_error("failed to delete file");
   }
   deleteFolderAndAllContents(pathRoot);
}

std::list<stagingEntry> readStagingEntries(inmem::config& c)
{
   std::list<stagingEntry> rval;

   std::wstring stagingPath = std::wstring(c.backup.absolutePath) + L"\\s";
   WIN32_FIND_DATAW fData;
   HANDLE hFind = ::FindFirstFileW((stagingPath + L"\\*.txt").c_str(),&fData);
   if(hFind == INVALID_HANDLE_VALUE)
      return rval;
   do
   {
      rval.push_back(stagingEntry());
      stagingEntry& e = rval.back();

      std::wstring path = stagingPath + L"\\";
      path += fData.cFileName;
      e.pathRoot = std::wstring(path.c_str(),path.length()-4);

      std::wifstream stream(path.c_str());
      std::getline(stream,e.monitorPath);
      std::wstring timeStr;
      std::getline(stream,timeStr);
      ::swscanf(timeStr.c_str(),L"%ld",&e.backupTime);
   }
   while(::FindNextFileW(hFind,&fData));
   ::FindClose(hFind);

   return rval;
}

stagingEntry reserveStagingEntry(inmem::config& c)
{
   std::set<size_t> usedIds;
   std::wstring stagingPath = std::wstring(c.backup.absolutePath) + L"\\s";
   WIN32_FIND_DATAW fData;
   HANDLE hFind = ::FindFirstFileW((stagingPath + L"\\*").c_str(),&fData);
   if(hFind != INVALID_HANDLE_VALUE)
   {
      do
      {
         if(fData.cFileName == std::wstring(L".")) continue;
         if(fData.cFileName == std::wstring(L"..")) continue;

         if((fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
            continue;

         size_t id = 0;
         ::swscanf(fData.cFileName,L"%lld",&id);
         usedIds.insert(id);
      }
      while(::FindNextFileW(hFind,&fData));
      ::FindClose(hFind);
   }

   size_t myId = 0;
   if(usedIds.size())
      myId = *(--usedIds.end()) + 1;

   std::wstringstream pathRoot;
   pathRoot << stagingPath << L"\\" << myId;
   ensurePathExists(pathRoot.str());
   stagingEntry rval;
   rval.pathRoot = pathRoot.str();
   return rval;
}
