#include "../cmn/file.hpp"
#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
#include "../cmn/staging.hpp"
#include "cmdStage.hpp"

void cmdStage(inmem::config& c, inmem::monitorConfig& mc)
{
   stagingEntry e;
   {
      mutex m(inmem::getStagingOperationLockName());
      autoLock _al(m);
      e = reserveStagingEntry(c);
   }

   e.monitorPath = mc.absolutePath;
   e.backupTime = ::time(NULL);
   copyDiskTree(e.monitorPath,e.pathRoot,/*allowErrors*/true);
   e.save();
}
