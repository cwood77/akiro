#define WIN32_LEAN_AND_MEAN
#include "path.hpp"
#include "windows.h"

std::wstring exeAdjacentPath(const std::wstring& addendum)
{
   wchar_t buffer[MAX_PATH];
   ::GetModuleFileNameW(NULL,buffer,MAX_PATH-1);
   return std::wstring(buffer) + L"\\..\\" + addendum;
}

std::wstring widen(const std::string& nstr)
{
   int len = nstr.size();
   std::wstring wstr(len + 1, 0);
   mbstowcs(&wstr[0], nstr.c_str(), len);
   return wstr;
}
