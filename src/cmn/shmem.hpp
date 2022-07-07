#pragma once
#define WIN32_LEAN_AND_MEAN
#include <string>
#include <windows.h>

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
   void raise();

private:
   HANDLE m_e;
};
