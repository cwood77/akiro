#define WIN32_LEAN_AND_MEAN
#include "treeDb.hpp"
#include "windows.h"

void treeListing::elaborate(const std::wstring& basePath)
{
   WIN32_FIND_DATAW fData;
   HANDLE hFind = ::FindFirstFileW((basePath + L"\\*").c_str(),&fData);
   if(hFind == INVALID_HANDLE_VALUE)
      return;
   do
   {
      if(fData.cFileName == std::wstring(L"."))
         continue;
      if(fData.cFileName == std::wstring(L".."))
         continue;

      std::wstring fullPath = basePath + L"\\" + fData.cFileName;

      if(fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
         elaborate(fullPath);
      else
         files[fullPath];
   }
   while(::FindNextFileW(hFind,&fData));
   ::FindClose(hFind);
}
