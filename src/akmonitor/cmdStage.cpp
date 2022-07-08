#include "../cmn/file.hpp"
#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
#include "../cmn/staging.hpp"
#include "../cmn/wlog.hpp"
#include "cmdStage.hpp"

static void signalCompaction(inmem::config& c)
{
   bool success = inmem::setStateWhen(&c.backup.state,inmem::states::kStatus_Ready,
      inmem::states::kCmd_Compact,10);
   if(success)
   {
      osEvent(inmem::getServicingProcessTxSignalName(c.backup.servicingProcessId))
         .raise();
      getWorkerLog() << L"successfully signaled the backup" << std::endl;
   }
   else
      getWorkerLog() << L"failed to signal the backup; is it busy?" << std::endl;
}

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

   signalCompaction(c);
}
