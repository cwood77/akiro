#define WIN32_LEAN_AND_MEAN
#include "temp.hpp"
#include "windows.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

std::wstring reserveTempFilePath(const std::wstring& hint)
{
   wchar_t folder[MAX_PATH];
   DWORD len = ::GetTempPathW(MAX_PATH,folder);
   if(!len || len > MAX_PATH)
      throw std::runtime_error("error getting temp path");

   for(size_t i=0;i<1000;i++)
   {
      std::wstringstream pathAttempt;
      pathAttempt << folder << L"akiro_" << hint << L"_" << i;

      WIN32_FIND_DATAW fData;
      HANDLE hFind = ::FindFirstFileW(pathAttempt.str().c_str(),&fData);
      if(hFind != INVALID_HANDLE_VALUE)
         ::CloseHandle(hFind);
      else
         return pathAttempt.str().c_str();
   }

   throw std::runtime_error("gave up picking a temp file");
}

void dumpAndDestroyTempFile(const std::wstring& path)
{
   if(path.empty())
      return;
   {
      std::wifstream file(path.c_str());
      std::wcout << L"[[redirecting from file: " << path << std::endl;
      std::wcout << file.rdbuf();
      std::wcout << L"]]" << std::endl;
   }
   ::DeleteFileW(path.c_str());
}
