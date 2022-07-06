// shmemm block format
//
// processes:
//   cmdline
//   backup
//   compact
//
// communication directions:
//   pause/unpause          cmdline -> [all]
//   on backup done         backup* -> compact
//   timestamps/restore     cmdling -> [all]
//
// maybe this is easier than I'm making it
//

// general transitions
//
// timestamp/restore:
//   pause all monitors
//      wait for each monitor to be ready
//         issue pause command
//            wait for awk
//   wait for backup to be ready
//     issue command
//        wait for awk
//        latch log path
//   unpause all monitors
//   dump log contents
//   destroy log file
//
// stop
//    kill everyone
//       wait for each process to be ready
//          issue die command
//             wait for dead
//
// start
//    verify I own the shmem
//    load the config into the shmem
//    start the backup
//       wait for the ready status
//    for each monitor in the config
//       start a monitor process
//          wait for the ready status

#pragma once

#define kNumMonitors 10

namespace inmem {

namespace cmds {
   enum type {
      kDead,
      kReady,
      kStaging,
      kCompacting,

      kDie,
      kCompact,
      kTimestamps,
      kRestore,
      kPauseMonitor,
      kUnpauseMonitor,
   };
};

struct heartbeatComms {
   unsigned long servicingProcessId;
   size_t cmd;
   time_t lastAction;
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

extern std::string getMasterShmemName();
//extern std::string getMasterShmemLockName();
extern std::string getStagingOperationLockName();
extern std::string getServicingProcessTxSignalName(unsigned long pid);
extern std::string getServicingProcessRxSignalName(unsigned long pid);

} // namespace inmem
