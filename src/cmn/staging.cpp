#include "../cmn/shmem-block.hpp"
#include "file.hpp"
#include "staging.hpp"
#include <fstream>

#include "wlog.hpp"

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
