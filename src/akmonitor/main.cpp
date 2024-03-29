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

   std::unique_ptr<std::wostream> pStream;
   if(monitorCfg.lastStageLogAbsolutePath[0])
      pStream.reset(new std::wofstream(monitorCfg.lastStageLogAbsolutePath));
   else
      pStream.reset(new std::wstringstream());
   workerLogBinding _wb(*pStream.get());

   monitorCfg.servicingProcessId = ::GetCurrentProcessId();
   osEvent myEvt("");
   heartbeatThread hbeat(monitorCfg,myEvt);
   hbeat.start();
   ::InterlockedExchange(&monitorCfg.state,inmem::states::kStatus_Ready);
   folderWatch watch(monitorCfg.absolutePath,monitorCfg.frequencyInMinutes);

   while(true)
   {
      bool shouldStage = watch.waitUntilFolderChange(myEvt);
      if(shouldStage)
         inmem::setStateWhen(&monitorCfg.state,inmem::states::kStatus_Ready,
            inmem::states::kCmd_Stage,10);

      if(monitorCfg.state == inmem::states::kCmd_Stage)
      {
         monitorCfg.lastAction = ::time(NULL);
         cmdStage(*pShmem,monitorCfg);
      }

      if(monitorCfg.state == inmem::states::kCmd_Die)
         break;

      inmem::setState(&monitorCfg.state,inmem::states::kStatus_Ready);
   }

   hbeat.join();
   monitorCfg.servicingProcessId = 0;
   inmem::setState(&monitorCfg.state,inmem::states::kStatus_Dead);
}

implWorkerMain(myMain)
