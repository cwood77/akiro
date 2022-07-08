#define WIN32_LEAN_AND_MEAN
#include "file.hpp"
#include "windows.h"
#include <stdexcept>

bool fileExists(const std::wstring& path)
{
   WIN32_FIND_DATAW fData;
   HANDLE hFind = ::FindFirstFileW(path.c_str(),&fData);
   if(hFind != INVALID_HANDLE_VALUE)
   {
      ::CloseHandle(hFind);
      return true;
   }
   else
      return false;
}

void copyFileSd(const std::wstring& source, const std::wstring& dest)
{
   BOOL success = ::CopyFileW(
      source.c_str(),
      dest.c_str(),
      /* bFailIfExists */TRUE);
   if(!success)
      throw std::runtime_error("copy failed");
}

std::wstring stripPath(const std::wstring& path)
{
   std::size_t pos = path.rfind(L"\\");
   if(pos == std::wstring::npos)
      return L"";
   return std::wstring(path.c_str(),pos);
}

void ensurePathExists(const std::wstring& path)
{
   if(fileExists(path)) return;
   std::wstring parent = stripPath(path);
   std::wstring leaf(path.c_str()+parent.length()+1);
   ensurePathExists(parent);
   BOOL success = ::CreateDirectoryW(path.c_str(),NULL);
   if(!success)
      throw std::runtime_error("error creating directory");
}

void ensurePathForFileExists(const std::wstring& path)
{
   ensurePathExists(stripPath(path));
}