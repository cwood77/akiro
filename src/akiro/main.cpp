#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
#include "configParser.hpp"
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
         std::cout << "shmem block is empty; loading config..." << std::endl;

         wchar_t buffer[MAX_PATH];
         ::GetModuleFileNameW(NULL,buffer,MAX_PATH-1);
         std::wifstream file((std::wstring(buffer) + L"\\..\\akiro.txt").c_str());
         if(!file.good())
            throw std::runtime_error("config file doesn't exist or can't be opened");

         configParser::load(file,*pShmem);
      }
   }
   catch(std::exception& x)
   {
      std::cout << "ERROR: " << x.what() << std::endl;
   }

   return 0;
}
