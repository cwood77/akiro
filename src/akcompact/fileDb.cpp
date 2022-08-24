#include "../cmn/file.hpp"
#include "../cmn/shmem-block.hpp"
#include "../cmn/wlog.hpp"
#include "fileDb.hpp"

fileDb::fileDb(inmem::config& c)
: m_dbRootPath(std::wstring(c.backup.absolutePath) + L"\\f")
{
}

std::wstring fileDb::addOrFetch(const std::wstring& path)
{
   std::wstring hash = computeHash(path);
   std::wstring destPath = m_dbRootPath + L"\\" + hash;

   if(fileExists(destPath))
   {
      getWorkerLog() << L"file already in db: " << path << std::endl;
      return hash;
   }

   // add the file to the DB
   getWorkerLog() << L"adding file to db: " << path << L" with hash " << hash << std::endl;
   ensurePathForFileExists(destPath);
   copyFileSd(path,destPath);
   return hash;
}

void fileDb::deleteUnusedFiles(referencedHashList& keepers)
{
   WIN32_FIND_DATAW fData;
   HANDLE hFind = ::FindFirstFileW((m_dbRootPath + L"\\*").c_str(),&fData);
   if(hFind == INVALID_HANDLE_VALUE)
   {
      getWorkerLog() << L"no files?" << std::endl;
   }
   size_t kept = 0;
   size_t killed = 0;
   do
   {
      if(fData.cFileName == std::wstring(L"."))
         continue;
      if(fData.cFileName == std::wstring(L".."))
         continue;

      std::wstring fullPath = m_dbRootPath + L"\\" + fData.cFileName;

      if(!keepers.isPresent(fData.cFileName))
      {
         getWorkerLog() << L"deleting unused file " << fData.cFileName << std::endl;
         killed++;
         BOOL success = ::DeleteFileW(fullPath.c_str());
         if(!success)
            throw std::runtime_error("error deleting file?!");
      }
      else
         kept++;
   }
   while(::FindNextFileW(hFind,&fData));
   ::FindClose(hFind);

   getWorkerLog() << std::endl;
   getWorkerLog() << L"kept " << kept << L" file(s)" << std::endl;
   getWorkerLog() << L"deleted " << killed << L" file(s)" << std::endl;
}

std::wstring fileDb::computeHash(const std::wstring& path)
{
   md5Hasher hasher(m_ctxt);
   hasher.addFile(path);
   md5Hash h;
   hasher.get(h);
   return h.toString();
}
