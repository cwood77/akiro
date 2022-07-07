#include "../cmn/crypto.hpp"
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
   //fileDb fDb(c);
   autoCryptoContext cCtxt;

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

         md5Hasher hasher(cCtxt);
         hasher.addFile(jit->first);

         md5Hash h;
         hasher.get(h);
         jit->second = h.toString();

         getWorkerLog()
            << L"   hash is "
            << jit->second
            << std::endl;
      }

      //treeDb tDb(c,key);
      //tDb.add(it->backupTime,listing);
   }
}
