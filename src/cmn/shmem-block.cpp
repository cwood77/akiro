#include "shmem-block.hpp"
#include <sstream>

namespace inmem {

std::string getServicingProcessTxSignalName(unsigned long pid)
{
   std::stringstream stream;
   stream << "cdwe_akiro_txsignal_" << pid;
   return stream.str();
}

void setState(long *state, long desired)
{
   ::InterlockedExchange(state,desired);
}

bool setStateWhen(long *state, long precondition, long desired, unsigned long timeout)
{
   time_t startTime = ::time(NULL);
   while(true)
   {
      LONG orig = ::InterlockedCompareExchange(state,desired,precondition);
      if(orig == desired)
         return true;

      if(!timeout)
         continue;

      time_t now = ::time(NULL);
      if(now >= (startTime + timeout))
         return false;
   }
}

void setStateWhen(long *state, long precondition, long desired, unsigned long timeout, const std::string& timeoutError)
{
   if(!setStateWhen(state,precondition,desired,timeout))
      throw std::runtime_error(timeoutError);
}

bool waitForState(long *state, long desired, unsigned long timeout)
{
   return setStateWhen(state,desired,desired,timeout);
}

void waitForState(long *state, long desired, unsigned long timeout, const std::string& timeoutError)
{
   if(!waitForState(state,desired,timeout))
      throw std::runtime_error(timeoutError);
}

} // namespace inmem
