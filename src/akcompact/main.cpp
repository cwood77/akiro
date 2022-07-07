#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
#include "../cmn/worker.hpp"
#include <stdexcept>

void myMain()
{
   autoShmem<inmem::config> pShmem(inmem::getMasterShmemName());
   if(!pShmem.didExist())
      throw std::runtime_error("shmem not configured!");

   pShmem->backup.servicingProcessId = ::GetCurrentProcessId();
   ::InterlockedExchange(&pShmem->backup.state,inmem::states::kStatus_Ready);

   osEvent evt(inmem::getServicingProcessTxSignalName(pShmem->backup.servicingProcessId));
   while(true)
   {
      evt.wait();
      inmem::setState(&pShmem->backup.heartbeatAwk,pShmem->backup.heartbeat);

      if(pShmem->backup.state == inmem::states::kCmd_Die)
         break;
   }

   pShmem->backup.servicingProcessId = 0;
   inmem::setState(&pShmem->backup.state,inmem::states::kStatus_Dead);
}

implWorkerMain(myMain)
