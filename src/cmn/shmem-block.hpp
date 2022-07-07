#pragma once
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <string>

#define kNumMonitors 10

namespace inmem {

namespace states {
   enum type {
      kStatus_Dead,
      kStatus_Ready,
      kStatus_Staging,
      kStatus_Compacting,

      kCmd_Status,
      kCmd_Die,
      kCmd_Compact,
      kCmd_Timestamps,
      kCmd_Restore,
      kCmd_PauseMonitor,
      kCmd_UnpauseMonitor,
   };
};

struct heartbeatComms {
   unsigned long servicingProcessId;
   long state;
   time_t lastAction;
   long heartbeat;
};

struct monitorConfig : public heartbeatComms{
   wchar_t absolutePath[MAX_PATH];
   size_t frequencyInMinutes;
};

struct backupConfig : public heartbeatComms {
   wchar_t absolutePath[MAX_PATH];
   size_t oldestVersionToKeepInDays;

   wchar_t args[3][MAX_PATH];
   wchar_t actionLogFile[MAX_PATH];
};

struct config {
   monitorConfig monitors[kNumMonitors];
   backupConfig backup;
};

inline std::string getMasterShmemName() { return "cdwe_akiro_shmem"; }
inline std::string getStagingOperationLockName() { return "cdwe_akiro_shlock"; }
extern std::string getServicingProcessTxSignalName(unsigned long pid);
extern std::string getServicingProcessRxSignalName(unsigned long pid);
extern std::string getServicingProcessHeartbeatSignalName(unsigned long pid);

extern bool waitForState(long *state, long desired, unsigned long timeout);

} // namespace inmem
