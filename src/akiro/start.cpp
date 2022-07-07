#define WIN32_LEAN_AND_MEAN
#include "start.hpp"
#include <stdexcept>
#include <windows.h>

void launchProcess(const std::wstring& path)
{
   STARTUPINFOW si;
   ::memset(&si,0,sizeof(STARTUPINFOW));
   si.cb = sizeof(STARTUPINFOW);

   PROCESS_INFORMATION pi;
   ::memset(&pi,0,sizeof(PROCESS_INFORMATION));

   // Start the child process. 
   BOOL success = ::CreateProcessW(
      NULL,                               // No module name (use command line)
      const_cast<wchar_t*>(path.c_str()), // Command line
      NULL,                               // Process handle not inheritable
      NULL,                               // Thread handle not inheritable
      FALSE,                              // Set handle inheritance to FALSE
      CREATE_NO_WINDOW,                   // No creation flags
      NULL,                               // Use parent's environment block
      NULL,                               // Use parent's starting directory 
      &si,                                // Pointer to STARTUPINFO structure
      &pi)                                // Pointer to PROCESS_INFORMATION structure
   ;
   if(!success)
      throw std::runtime_error("CreateProcess failed");

   // Close process and thread handles. 
   ::CloseHandle(pi.hProcess);
   ::CloseHandle(pi.hThread);
}
