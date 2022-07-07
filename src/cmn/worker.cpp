#include "worker.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>

int _workerMain(int argc, const char *argv[], void (*pFunc)())
{
   try
   {
      if(argc != 2 || ::strcmp(argv[1],"-r")!=0)
      {
         std::cout << "akiro - https://github.com/cwood77/akiro" << std::endl;
         std::cout << argv[0] << std::endl;
         std::cout << std::endl;
         throw std::runtime_error("this EXE should not be called directly");
      }

      pFunc();

      return 0;
   }
   catch(std::exception& x)
   {
      std::cerr << "ERROR: " << x.what() << std::endl;
      return -1;
   }
}
