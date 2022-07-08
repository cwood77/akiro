#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
#include "../cmn/temp.hpp"
#include "../cmn/wlog.hpp"
#include "../cmn/worker.hpp"
#include "cmdCompact.hpp"
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>

void myMain()
{
   autoShmem<inmem::config> pShmem(inmem::getMasterShmemName());
   if(!pShmem.didExist())
      throw std::runtime_error("shmem not configured!");

   pShmem->backup.servicingProcessId = ::GetCurrentProcessId();
   osEvent myEvt("");
   heartbeatThread hbeat(pShmem->backup,myEvt);
   hbeat.start();
   ::InterlockedExchange(&pShmem->backup.state,inmem::states::kStatus_Ready);

   while(true)
   {
      myEvt.wait();
      inmem::setState(&pShmem->backup.heartbeatAwk,pShmem->backup.heartbeat);

      if(pShmem->backup.state == inmem::states::kCmd_Compact)
      {
         pShmem->backup.lastAction = ::time(NULL);

         std::unique_ptr<std::wostream> pStream;
         if(pShmem->backup.lastCompactLogAbsolutePath[0])
            pStream.reset(new std::wofstream(pShmem->backup.lastCompactLogAbsolutePath));
         else
            pStream.reset(new std::wstringstream());
         workerLogBinding _wb(*pStream.get());

         try
         {
            cmdCompact(*pShmem);
         }
         catch(std::exception& x)
         {
            getWorkerLog() << L"ERROR:" << x.what() << std::endl;
         }
         getWorkerLog() << L"done" << std::endl;
      }
      else if(pShmem->backup.state == inmem::states::kCmd_Timestamps)
      {
         auto path = reserveTempFilePath(L"timestamps");
         ::wcscpy(pShmem->backup.actionLogFile,path.c_str());
         std::wofstream writer(path.c_str());
         workerLogBinding _wb(writer);

         try
         {
            cmdTimestamps(*pShmem,pShmem->backup.args[0]);
         }
         catch(std::exception& x)
         {
            getWorkerLog() << L"ERROR:" << x.what() << std::endl;
         }
         getWorkerLog() << L"done" << std::endl;
      }
      else if(pShmem->backup.state == inmem::states::kCmd_Restore)
      {
         auto path = reserveTempFilePath(L"restore");
         ::wcscpy(pShmem->backup.actionLogFile,path.c_str());
         std::wofstream writer(path.c_str());
         workerLogBinding _wb(writer);

         try
         {
            cmdRestore(*pShmem,pShmem->backup.args[0],pShmem->backup.args[1],pShmem->backup.args[2]);
         }
         catch(std::exception& x)
         {
            getWorkerLog() << L"ERROR:" << x.what() << std::endl;
         }
         getWorkerLog() << L"done" << std::endl;
      }

      if(pShmem->backup.state == inmem::states::kCmd_Die)
         break;

      inmem::setState(&pShmem->backup.state,inmem::states::kStatus_Ready);
   }

   hbeat.join();
   pShmem->backup.servicingProcessId = 0;
   inmem::setState(&pShmem->backup.state,inmem::states::kStatus_Dead);
}

implWorkerMain(myMain)
