#pragma once
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <string>

#define kNumMonitors 20

namespace inmem {

namespace states {
   enum type {
      kStatus_Dead,
      kStatus_Ready,

      kCmd_Status,
      kCmd_Die,
      kCmd_Compact,
      kCmd_Timestamps,
      kCmd_Restore,
      kCmd_Cull,
      kCmd_Prune,
      kCmd_Stage,
      kCmd_PauseMonitor,
      kCmd_UnpauseMonitor,
   };
};

struct heartbeatComms {
   unsigned long servicingProcessId;
   long state;
   time_t lastAction;
   long heartbeat;
   long heartbeatAwk;
};

struct retentionPolicy {
   time_t olderThanInDays;
   size_t keepMax;
};

struct monitorConfig : public heartbeatComms{
   wchar_t absolutePath[MAX_PATH];
   size_t frequencyInMinutes;
   wchar_t lastStageLogAbsolutePath[MAX_PATH];
   bool enabled;
   retentionPolicy rpolicy[10];
};

struct backupConfig : public heartbeatComms {
   wchar_t absolutePath[MAX_PATH];
   size_t retentionFrequencyInDays;
   wchar_t lastCompactLogAbsolutePath[MAX_PATH];
   wchar_t lastCullLogAbsolutePath[MAX_PATH];
   wchar_t lastPruneLogAbsolutePath[MAX_PATH];

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

extern void setState(long *state, long desired);
extern bool setStateWhen(long *state, long precondition, long desired, unsigned long timeout);
extern void setStateWhen(long *state, long precondition, long desired, unsigned long timeout, const std::string& timeoutError);
extern bool waitForState(long *state, long desired, unsigned long timeout);
extern void waitForState(long *state, long desired, unsigned long timeout, const std::string& timeoutError);

} // namespace inmem
