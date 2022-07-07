#include "../cmn/path.hpp"
#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
#include "configParser.hpp"
#include "start.hpp"
#include <fstream>
#include <iostream>

int main(int argc, const char *argv[])
{
   try
   {
      std::cout << "akiro - https://github.com/cwood77/akiro" << std::endl;
      std::cout << std::endl;

      autoShmem<inmem::config> pShmem(inmem::getMasterShmemName());

      if(!pShmem.didExist())
      {
         std::cout << "shmem block is empty!; loading config..." << std::endl;

         std::wifstream file(exeAdjacentPath(L"akiro.txt").c_str());
         if(!file.good())
            throw std::runtime_error("config file doesn't exist or can't be opened");

         configParser::load(file,*pShmem);

         std::cout << "starting workers..." << std::endl;
         launchProcess(exeAdjacentPath(L"akcompact.exe -r"));
         inmem::waitForState(&pShmem->backup.state,inmem::states::kStatus_Ready,
            10,"timeout waiting for akcompact EXE");
      }

      std::cout << "shutting down backup..." << std::endl;
      inmem::setStateWhen(&pShmem->backup.state,inmem::states::kStatus_Ready,
         inmem::states::kCmd_Die,
         10,"timeout telling backup to die");
      osEvent(inmem::getServicingProcessTxSignalName(pShmem->backup.servicingProcessId))
         .raise();
      inmem::waitForState(&pShmem->backup.state,inmem::states::kStatus_Dead,
         10,"timeout waiting for akcompact EXE");

      return 0;
   }
   catch(std::exception& x)
   {
      std::cerr << "ERROR: " << x.what() << std::endl;
      return -1;
   }
}
