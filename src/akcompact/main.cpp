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
}

implWorkerMain(myMain)
