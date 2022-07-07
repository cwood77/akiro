#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
#include "cmdStartStopStatus.hpp"
#include <iostream>

int main(int argc, const char *argv[])
{
   try
   {
      std::cout << "akiro - https://github.com/cwood77/akiro" << std::endl;
      std::cout << std::endl;

      if(argc == 1)
      {
         std::cout << "usage: akiro [verb]" << std::endl;
         std::cout << std::endl;
         std::cout << "examples:" << std::endl;
         std::cout << "  akiro start - start up monitoring" << std::endl;
         std::cout << "  akiro stop - shutdown monitoring" << std::endl;
         std::cout << "  akiro status - report monitoring status" << std::endl;
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
