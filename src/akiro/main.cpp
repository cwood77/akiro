#include "../cmn/path.hpp"
#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
#include "cmdStartStopStatus.hpp"
#include <iostream>

int main(int argc, const char *argv[])
{
   try
   {
      std::wcout << L"akiro - https://github.com/cwood77/akiro" << std::endl;
      std::wcout << std::endl;

      if(argc == 1)
      {
         std::wcout << L"usage: akiro [verb]" << std::endl;
         std::wcout << std::endl;
         std::wcout << L"examples:" << std::endl;
         std::wcout << L"  akiro start - start up monitoring" << std::endl;
         std::wcout << L"  akiro stop - shutdown monitoring" << std::endl;
         std::wcout << L"  akiro status - report monitoring status" << std::endl;
         std::wcout << L"  akiro timestamps <dir> - list captured timestamps of dir" << std::endl;
         return 0;
      }

      autoShmem<inmem::config> pShmem(inmem::getMasterShmemName());

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
      else if(argc == 2 && argv[1] == std::string("compact"))
      {
         if(!pShmem.didExist())
            throw std::runtime_error("processes aren't running");
         cmdCompact(*pShmem);
      }
      else if(argc == 3 && argv[1] == std::string("timestamps"))
      {
         if(!pShmem.didExist())
            throw std::runtime_error("processes aren't running");
         cmdTimestamps(*pShmem,widen(argv[2]));
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
