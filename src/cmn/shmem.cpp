#include "shmem-block.hpp"
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
      name.length() ? name.c_str() : NULL);
}

osEvent::~osEvent()
{
   ::CloseHandle(m_e);
}

void osEvent::wait()
{
   ::WaitForSingleObject(m_e,INFINITE);
}

void osEvent::waitWithTimeout(DWORD timeoutInMs, bool& timedout)
{
   DWORD rval = ::WaitForSingleObject(m_e,timeoutInMs);
   timedout = (rval == WAIT_TIMEOUT);
}

void osEvent::raise()
{
   ::SetEvent(m_e);
}

mutex::mutex(const std::string& name)
{
   m_han = ::CreateMutexA(NULL,FALSE,name.c_str());
}

mutex::~mutex()
{
   ::CloseHandle(m_han);
}

bool mutex::lock(DWORD timeoutInMs)
{
   DWORD rval = ::WaitForSingleObject(m_han,timeoutInMs);
   return rval == WAIT_OBJECT_0;
}

void mutex::unlock()
{
   ::ReleaseMutex(m_han);
}

heartbeatThread::heartbeatThread(inmem::heartbeatComms& cfg, osEvent& signal)
: m_cfg(cfg)
, m_shmemSignal(inmem::getServicingProcessTxSignalName(cfg.servicingProcessId))
, m_signal(signal)
, m_hThread(INVALID_HANDLE_VALUE)
{
}

void heartbeatThread::start()
{
   m_hThread = ::CreateThread(
      NULL, // lpThreadAttributes,
      0,    // dwStackSize,
      &_threadProcThunk,
      this,
      0,    // dwCreationFlags,
      NULL  // lpThreadId
   );
}

void heartbeatThread::join()
{
   ::WaitForSingleObject(m_hThread,INFINITE);
   ::CloseHandle(m_hThread);
}

DWORD heartbeatThread::_threadProcThunk(LPVOID pParam)
{
   reinterpret_cast<heartbeatThread*>(pParam)->_threadProc();
   return 0;
}

void heartbeatThread::_threadProc()
{
   while(true)
   {
      m_shmemSignal.wait();

      m_signal.raise();

      inmem::setState(&m_cfg.heartbeatAwk,m_cfg.heartbeat);

      if(m_cfg.state == inmem::states::kCmd_Die)
         return;
   }
}
