#include "../cmn/path.hpp"
#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
#include "cmdStartStopStatus.hpp"
#include <iostream>

int main(int argc, const char *argv[])
{
   try
   {
      std::wcout << L"akiro - simple background backups" << std::endl;
      std::wcout << L"        https://github.com/cwood77/akiro" << std::endl;
      std::wcout << std::endl;

      if(argc == 1)
      {
         std::wcout << L"usage: akiro [verb]" << std::endl;
         std::wcout << std::endl;
         std::wcout << L"common examples:" << std::endl;
         std::wcout << L"  akiro start - start up monitoring" << std::endl;
         std::wcout << L"  akiro stop - shutdown monitoring" << std::endl;
         std::wcout << L"  akiro status - report monitoring status" << std::endl;
         std::wcout << std::endl;
         std::wcout << L"less common commands:" << std::endl;
         std::wcout << L"  akiro timestamps <monitordir> - list captured timestamps of dir" << std::endl;
         std::wcout << L"  akiro restore <monitordir> <timestamp> <dest>" << std::endl;
         std::wcout << L"    reconstruct <monitordir> as it was at <timestamp> in <dest>" << std::endl;
         std::wcout << std::endl;
         std::wcout << L"testing and rare case examples:" << std::endl;
         std::wcout << L"  akiro stage - trigger an immediate stage of all monitors" << std::endl;
         std::wcout << L"  akiro compact - trigger an immediate compaction" << std::endl;
         std::wcout << L"  akiro cull - trigger immediate retention enforcement" << std::endl;
         std::wcout << L"  akiro prune - delete unreferenced archive data based on user" << std::endl;
         std::wcout << L"    changes to the root or tree db" << std::endl;

         std::wcout << std::endl;
         wchar_t buffer[100];
         auto now = ::time(NULL);
         struct tm *pLt = ::localtime(&now);
         ::wcsftime(buffer,MAX_PATH,L"%Y%m%d-%H%M%S",pLt);
         std::wcout << L"btw the current timestamp is " << buffer << std::endl;

         std::wcout << L"btw view/edit configuration at " << exeAdjacentPath(L"akiro.txt") << std::endl;
         return 0;
      }

      autoShmem<inmem::config> pShmem(inmem::getMasterShmemName());

      // common ---------------------
      if(argc == 2 && argv[1] == std::string("start"))
      {
         if(pShmem.didExist())
            throw std::runtime_error("processes already running");
         cmdStart(*pShmem);
      }
      else if(argc == 2 && argv[1] == std::string("stop"))
      {
         if(!pShmem.didExist())
            throw std::runtime_error("processes aren't running");
         cmdStop(*pShmem);
      }
      else if(argc == 2 && argv[1] == std::string("status"))
      {
         if(!pShmem.didExist())
            throw std::runtime_error("processes aren't running");
         cmdStatus(*pShmem);
      }

      // advanced ---------------------
      else if(argc == 3 && argv[1] == std::string("timestamps"))
      {
         if(!pShmem.didExist())
            throw std::runtime_error("processes aren't running");
         cmdTimestamps(*pShmem,widen(argv[2]));
      }
      else if(argc == 5 && argv[1] == std::string("restore"))
      {
         if(!pShmem.didExist())
            throw std::runtime_error("processes aren't running");
         cmdRestore(*pShmem,widen(argv[2]),widen(argv[3]),widen(argv[4]));
      }

      // advanced ---------------------
      else if(argc == 2 && argv[1] == std::string("stage"))
      {
         if(!pShmem.didExist())
            throw std::runtime_error("processes aren't running");
         cmdStage(*pShmem);
      }
      else if(argc == 2 && argv[1] == std::string("compact"))
      {
         if(!pShmem.didExist())
            throw std::runtime_error("processes aren't running");
         cmdCompact(*pShmem);
      }
      else if(argc == 2 && argv[1] == std::string("cull"))
      {
         if(!pShmem.didExist())
            throw std::runtime_error("processes aren't running");
         cmdCull(*pShmem);
      }
      else if(argc == 2 && argv[1] == std::string("prune"))
      {
         if(!pShmem.didExist())
            throw std::runtime_error("processes aren't running");
         cmdPrune(*pShmem);
      }

      else
         throw std::runtime_error("bad usage");

      return 0;
   }
   catch(std::exception& x)
   {
      std::cerr << "ERROR: " << x.what() << std::endl;
      return -1;
   }
}
