#include "shmem-block.hpp"

namespace inmem {

bool waitForState(long *state, long desired, unsigned long timeout)
{
   time_t startTime = ::time(NULL);
   while(true)
   {
      LONG orig = ::InterlockedCompareExchange(state,desired,desired);
      if(orig == desired)
         return true;

      if(!timeout)
         continue;

      time_t now = ::time(NULL);
      if(now >= (startTime + timeout))
         return false;
   }
}

} // namespace inmem
