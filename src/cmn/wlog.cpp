#include "wlog.hpp"

workerLogBinding *workerLogBinding::gSelf = NULL;

workerLogBinding::workerLogBinding(std::wostream& s)
: m_s(s)
{
   if(gSelf)
      throw std::runtime_error("multiple workerLogBinding");
   gSelf = this;
}

std::wostream& workerLogBinding::getLog()
{
   if(!gSelf)
      throw std::runtime_error("worker log used without binding");
   return gSelf->m_s;
}
