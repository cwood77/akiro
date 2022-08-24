#include "../cmn/wlog.hpp"
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

folderWatch::folderWatch(const std::wstring& folder, size_t maxFrequencyInMinutes)
: m_maxFrequencyInMinutes(maxFrequencyInMinutes)
, m_lastFired(0)
, m_lastIgnored(0)
{
   m_hFind = ::FindFirstChangeNotificationW(
      folder.c_str(),
      TRUE, // bWatchSubtree,
      FILE_NOTIFY_CHANGE_FILE_NAME |
      FILE_NOTIFY_CHANGE_DIR_NAME |
      FILE_NOTIFY_CHANGE_ATTRIBUTES |
      FILE_NOTIFY_CHANGE_SIZE |
      FILE_NOTIFY_CHANGE_LAST_WRITE);

   if(m_hFind == INVALID_HANDLE_VALUE)
      throw std::runtime_error("failed to start file watch");
}

folderWatch::~folderWatch()
{
   ::FindCloseChangeNotification(m_hFind);
}

bool folderWatch::waitUntilFolderChange(osEvent& otherEvt)
{
   const auto maxFreqInSecs = m_maxFrequencyInMinutes * 60;

   while(true)
   {
      getWorkerLog() << L"[watch] waiting" << std::endl;
      DWORD timeoutInMSec = INFINITE;
      if(m_lastIgnored)
      {
         getWorkerLog() << L"[watch]    adjusting timeout for ignored change" << std::endl;
         auto waitTime = ::time(NULL);
         size_t elapsedSecs = waitTime - m_lastFired;
         if(elapsedSecs >= maxFreqInSecs)
            timeoutInMSec = 0;
         else
            timeoutInMSec = (maxFreqInSecs - elapsedSecs) * 1000;
      }
      HANDLE hans[2];
      hans[0] = m_hFind;
      hans[1] = otherEvt._getHandle();
      DWORD reason = ::WaitForMultipleObjects(2,hans,/*any*/FALSE,timeoutInMSec);
      getWorkerLog() << L"[watch] awoke" << std::endl;
      if(reason == WAIT_TIMEOUT)
      {
         if(timeoutInMSec == INFINITE)
            throw std::runtime_error("insanity from Wait function");

         // this notification should be handled
         getWorkerLog() << L"[watch] handling DEFERRED change" << std::endl;
         m_lastFired = ::time(NULL);
         m_lastIgnored = 0;
         return true;
      }
      else if(reason == WAIT_OBJECT_0)
      {
         getWorkerLog() << L"[watch] folder changed" << std::endl;
         BOOL success = ::FindNextChangeNotification(m_hFind);
         if(!success)
            throw std::runtime_error("insanity from Wait function");

         auto signalTime = ::time(NULL);
         size_t elapsedSecs = signalTime - m_lastFired;
         if(m_lastFired!=0 && (elapsedSecs < maxFreqInSecs))
         {
            // too soon; ignore this notification
            getWorkerLog() << L"[watch] ignoring change" << std::endl;
            m_lastIgnored = signalTime;
            continue;
         }

         // this notification should be handled
         getWorkerLog() << L"[watch] handling change" << std::endl;
         m_lastFired = signalTime;
         m_lastIgnored = 0;
         return true;
      }
      else if(reason == WAIT_OBJECT_0 + 1)
      {
         getWorkerLog() << L"[watch] something else changed" << std::endl;
         return false;
      }
      else
         throw std::runtime_error("insanity from Wait function");
   }
}
