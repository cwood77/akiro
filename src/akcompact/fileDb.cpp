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

std::wstring fileDb::computeHash(const std::wstring& path)
{
   md5Hasher hasher(m_ctxt);
   hasher.addFile(path);
   md5Hash h;
   hasher.get(h);
   return h.toString();
}
