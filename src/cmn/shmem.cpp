#include "shmem.hpp"
#include <stdexcept>

autoShmemBase::autoShmemBase(size_t size, const std::string& name)
: m_pPtr(NULL)
, m_hMap(INVALID_HANDLE_VALUE)
, m_existed(false)
{
   m_hMap = ::CreateFileMappingA(
      INVALID_HANDLE_VALUE, // use system paging file
      NULL, // default security
      PAGE_READWRITE,
      0,
      size,
      name.c_str());
   m_existed = (::GetLastError() == ERROR_ALREADY_EXISTS);
   if(m_hMap == INVALID_HANDLE_VALUE)
      throw std::runtime_error("shared memory cannot be opened");

   m_pPtr = ::MapViewOfFile(
      m_hMap,
      FILE_MAP_ALL_ACCESS,
      0,
      0,
      0);
   if(!m_pPtr)
      throw std::runtime_error("couldn't map shared memory?!");
}

autoShmemBase::~autoShmemBase()
{
   ::UnmapViewOfFile(m_pPtr);
   ::CloseHandle(m_hMap);
}

osEvent::osEvent(const std::string& name)
: m_e(INVALID_HANDLE_VALUE)
{
   m_e = ::CreateEventA(
      NULL,
      FALSE, // auto
      FALSE, // unsignalled
      name.c_str());
}

osEvent::~osEvent()
{
   ::CloseHandle(m_e);
}

void osEvent::wait()
{
   ::WaitForSingleObject(m_e,INFINITE);
}

void osEvent::raise()
{
   ::SetEvent(m_e);
}
