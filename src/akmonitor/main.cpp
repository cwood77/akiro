#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
#include "../cmn/worker.hpp"
#include <stdexcept>

void myMain()
{
   autoShmem<inmem::config> pShmem(inmem::getMasterShmemName());
   if(!pShmem.didExist())
      throw std::runtime_error("shmem not configured!");

   size_t myIndex;
   for(myIndex=0;myIndex<kNumMonitors;myIndex++)
   {
      auto& candidate = pShmem->monitors[myIndex];
      if(candidate.frequencyInMinutes == 0)
         throw std::runtime_error("couldn't find monitor to service");
      if(candidate.servicingProcessId == 0)
         break; // win!
   }
   auto& monitorCfg = pShmem->monitors[myIndex];

   monitorCfg.servicingProcessId = ::GetCurrentProcessId();
   osEvent evt(inmem::getServicingProcessTxSignalName(monitorCfg.servicingProcessId));
   ::InterlockedExchange(&monitorCfg.state,inmem::states::kStatus_Ready);

   while(true)
   {
      evt.wait();
      inmem::setState(&monitorCfg.heartbeatAwk,monitorCfg.heartbeat);

      if(monitorCfg.state == inmem::states::kCmd_Die)
         break;

      inmem::setState(&monitorCfg.state,inmem::states::kStatus_Ready);
   }

   monitorCfg.servicingProcessId = 0;
   inmem::setState(&monitorCfg.state,inmem::states::kStatus_Dead);
}

implWorkerMain(myMain)

