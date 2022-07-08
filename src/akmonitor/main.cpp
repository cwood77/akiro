#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
#include "../cmn/wlog.hpp"
#include "../cmn/worker.hpp"
#include "cmdStage.hpp"
#include <fstream>
#include <memory>
#include <sstream>
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

   bool once = true;
   while(true)
   {
      bool timedout;
      evt.waitWithTimeout(monitorCfg.frequencyInMinutes*60*1000,timedout);
      inmem::setState(&monitorCfg.heartbeatAwk,monitorCfg.heartbeat);

      if(once)
      {
         timedout = true;
         once = false;
      }
      else
         timedout = false;

      if(timedout)
      {
         if(inmem::setStateWhen(&monitorCfg.state,inmem::states::kStatus_Ready,
            inmem::states::kStatus_Staging,10))
         {
            std::unique_ptr<std::wostream> pStream;
            if(monitorCfg.lastStageLogAbsolutePath[0])
               pStream.reset(new std::wofstream(monitorCfg.lastStageLogAbsolutePath));
            else
               pStream.reset(new std::wstringstream());
            workerLogBinding _wb(*pStream.get());

            cmdStage(*pShmem,monitorCfg);
         }
      }

      if(monitorCfg.state == inmem::states::kCmd_Die)
         break;

      inmem::setState(&monitorCfg.state,inmem::states::kStatus_Ready);
   }

   monitorCfg.servicingProcessId = 0;
   inmem::setState(&monitorCfg.state,inmem::states::kStatus_Dead);
}

implWorkerMain(myMain)

