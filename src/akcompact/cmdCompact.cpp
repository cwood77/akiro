#include "../cmn/file.hpp"
#include "../cmn/shmem-block.hpp"
#include "../cmn/staging.hpp"
#include "../cmn/wlog.hpp"
#include "cmdCompact.hpp"
#include "fileDb.hpp"
#include "rootDb.hpp"
#include "treeDb.hpp"

void cmdCompact(inmem::config& c)
{
   rootDb rDb(c);
   fileDb fDb(c);

   auto entries = readStagingEntries(c);
   for(auto it=entries.begin();it!=entries.end();++it)
   {
      getWorkerLog()
         << L"handing staging entry: "
         << it->monitorPath
         << L" / "
         << it->backupTime
         << L" => "
         << it->pathRoot
         << std::endl;

      size_t key = rDb.findOrAddKey(it->monitorPath);
      getWorkerLog()
         << L"key=" << key
         << std::endl;

      treeListing listing;
      listing.elaborate(it->pathRoot);
      for(auto jit=listing.files.begin();jit!=listing.files.end();++jit)
      {
         getWorkerLog()
            << jit->first
            << std::endl;

         jit->second = fDb.addOrFetch(jit->first);

         getWorkerLog()
            << L"   hash is "
            << jit->second
            << std::endl;
      }

      treeDb tDb(c,key);
      tDb.add(it->backupTime,listing);

      it->eraseOnDisk();
   }
}

void cmdTimestamps(inmem::config& c, const std::wstring& arg)
{
   rootDb rDb(c);
   size_t key = rDb.lookupKey(arg);
   treeDb tDb(c,key);
   tDb.dump(getWorkerLog());
}

void cmdRestore(inmem::config& c, const std::wstring& dir, const std::wstring& timestamp, const std::wstring& dest)
{
   rootDb rDb(c);
   size_t key = rDb.lookupKey(dir);

   treeDb tDb(c,key);
   treeListing l;
   tDb.load(timestamp,l);
   for(auto it=l.files.begin();it!=l.files.end();++it)
   {
      getWorkerLog() << L"using " << it->second << L" for " << it->first;

      std::wstring src = std::wstring(c.backup.absolutePath) + L"\\f\\" + it->second;
      std::wstring dst = dest + L"\\" + it->first;
      ensurePathForFileExists(dst);
      BOOL success = ::CopyFileW(
         src.c_str(),
         dst.c_str(),
         /* bFailIfExists */TRUE);
      if(!success)
         throw std::runtime_error("copy failed");
   }
}
