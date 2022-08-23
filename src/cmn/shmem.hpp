#pragma once
#define WIN32_LEAN_AND_MEAN
#include <string>
#include <windows.h>

namespace inmem { class heartbeatComms; }

class autoShmemBase {
public:
   autoShmemBase(size_t size, const std::string& name);
   ~autoShmemBase();

   bool didExist() const { return m_existed; }

protected:
   void *m_pPtr;

private:
   HANDLE m_hMap;
   bool m_existed;
};

template<class T>
class autoShmem : public autoShmemBase {
public:
   explicit autoShmem(const std::string& name)
   : autoShmemBase(sizeof(T),name) {}

   T *operator->() { return reinterpret_cast<T*>(m_pPtr); }
   T& operator*() { return *reinterpret_cast<T*>(m_pPtr); }
};

class osEvent {
public:
   explicit osEvent(const std::string& name);
   ~osEvent();

   void wait();
   void waitWithTimeout(DWORD timeoutInMs, bool& timedout);
   void raise();

   HANDLE _getHandle() { return m_e; }

private:
   HANDLE m_e;
};

class mutex {
public:
   explicit mutex(const std::string& name);
   ~mutex();

   bool lock(DWORD timeoutInMs);
   void unlock();

private:
   HANDLE m_han;
};

class autoLock {
public:
   explicit autoLock(mutex& m) : m_m(m) { m_m.lock(INFINITE); }
   ~autoLock() { m_m.unlock(); }

private:
   mutex& m_m;
};

class heartbeatThread {
public:
   heartbeatThread(inmem::heartbeatComms& cfg, osEvent& signal);

   void start();
   void join();

private:
   static DWORD _threadProcThunk(LPVOID pParam);
   void _threadProc();

   inmem::heartbeatComms& m_cfg;
   osEvent m_shmemSignal;
   osEvent& m_signal;
   HANDLE m_hThread;
};

class folderWatch {
public:
   folderWatch(const std::wstring& folder, size_t maxFrequencyInMinutes);
   ~folderWatch();

   bool waitUntilFolderChange(osEvent& otherEvt);

private:
   const size_t m_maxFrequencyInMinutes;
   HANDLE m_hFind;
   time_t m_lastFired;
};
