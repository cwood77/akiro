#include "../cmn/shmem-block.hpp"
#include "../cmn/staging.hpp"
#include "../cmn/wlog.hpp"
#include "cmdCompact.hpp"
#include "treeDb.hpp"

void cmdCompact(inmem::config& c)
{
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

      treeListing listing;
      listing.elaborate(it->pathRoot);
      for(auto jit=listing.files.begin();jit!=listing.files.end();++jit)
      {
         getWorkerLog()
            << jit->first
            << std::endl;
      }
   }
}
