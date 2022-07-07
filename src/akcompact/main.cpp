#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
#include "../cmn/worker.hpp"
#include <stdexcept>

#include "../cmn/temp.hpp"
#include <fstream>

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

      // kCmd_Compact
      // kCmd_Timestamps
      // kCmd_Restore
      // kCmd_Cull

      if(pShmem->backup.state == inmem::states::kCmd_Die)
      {
         // try out temp files
         auto path = reserveTempFilePath(L"compact");
         ::wcscpy(pShmem->backup.actionLogFile,path.c_str());
         std::wofstream writer(path.c_str());
         writer << L"test 1, 2, 3" << std::endl;

         break;
      }
   }

   pShmem->backup.servicingProcessId = 0;
   inmem::setState(&pShmem->backup.state,inmem::states::kStatus_Dead);
}

implWorkerMain(myMain)
