#include "../cmn/file.hpp"
#include "../cmn/path.hpp"
#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
#include "../cmn/temp.hpp"
#include "cmdStartStopStatus.hpp"
#include "configParser.hpp"
#include "start.hpp"
#include <fstream>
#include <iostream>

static void start(inmem::heartbeatComms& c, const std::wstring& procName)
{
   launchProcess(exeAdjacentPath(procName));
   inmem::waitForState(&c.state,inmem::states::kStatus_Ready,
      10,"timeout waiting for subprocess ready");
}

void cmdStart(inmem::config& c)
{
   std::wifstream file(exeAdjacentPath(L"akiro.txt").c_str());
   if(!file.good())
      throw std::runtime_error("config file doesn't exist or can't be opened");

   configParser::load(file,c);

   ensurePathExists(std::wstring(c.backup.absolutePath) + L"\\s"); // staging
   ensurePathExists(std::wstring(c.backup.absolutePath) + L"\\t"); // treeDB
   ensurePathExists(std::wstring(c.backup.absolutePath) + L"\\f"); // fileDB

   std::wcout << L"starting compactor..." << std::endl;
   start(c.backup,L"akcompact.exe -r");

   for(size_t i=0;i<kNumMonitors;i++)
   {
      auto& monitorCfg = c.monitors[i];
      if(monitorCfg.frequencyInMinutes == 0) break;

      std::wcout << L"starting monitor " << i
         << L": "
         << monitorCfg.absolutePath
         << L"..."
         << std::endl;

      if(!fileExists(monitorCfg.absolutePath))
         std::wcout << L"   ERROR - monitored path doesn't exist" << std::endl;
      else
         start(monitorCfg,L"akmonitor.exe -r");
   }
}

static void stop(inmem::heartbeatComms& c)
{
   inmem::setStateWhen(&c.state,inmem::states::kStatus_Ready,
      inmem::states::kCmd_Die,
      10,"timeout waiting to ask subprocess to die");
   osEvent(inmem::getServicingProcessTxSignalName(c.servicingProcessId))
      .raise();
   inmem::waitForState(&c.state,inmem::states::kStatus_Dead,
      10,"timeout waiting for subprocess to die");
}

void cmdStop(inmem::config& c)
{
   for(size_t i=0;i<kNumMonitors;i++)
   {
      auto& monitorCfg = c.monitors[i];
      if(monitorCfg.servicingProcessId == 0) break;

      std::wcout << L"stopping monitor " << i
         << L": "
         << monitorCfg.absolutePath
         << L"..."
         << std::endl;
      stop(monitorCfg);
   }

   std::wcout << L"shutting down backup..." << std::endl;
   stop(c.backup);
}

static const wchar_t *gStatusNames[] = {
   L"dead         ",
   L"ready        ",

   L"status       ",
   L"shutting down",
   L"compacting   ",
   L"timestamps   ",
   L"restoring    ",
   L"culling      ",
   L"pruning      ",
   L"staging      ",
   L"paused       ",
   L"unpaused     ",
};

static void status(inmem::heartbeatComms& c, const std::wstring& userName)
{
   std::wcout << L"[" << gStatusNames[c.state] << L"] ";

   long ans = ::InterlockedIncrement(&c.heartbeat);
   osEvent(inmem::getServicingProcessTxSignalName(c.servicingProcessId))
      .raise();
   if(!inmem::waitForState(&c.heartbeatAwk,ans,10))
      std::wcout << L"UNRESPONSIVE!! - ";
   else
      std::wcout << L"responsive     - ";

   struct tm *pLt = ::localtime(&c.lastAction);
   wchar_t buffer[] = L"   -- never --   ";
   if(c.lastAction)
      ::wcsftime(
         buffer,
         MAX_PATH,
         L"%m/%d/%y %H:%M:%S",
         pLt);

   std::wcout << buffer << L" - " << userName << std::endl;
}

void cmdStatus(inmem::config& c)
{
   for(size_t i=0;i<kNumMonitors;i++)
   {
      auto& monitorCfg = c.monitors[i];
      if(monitorCfg.servicingProcessId == 0) break;
      std::wstring name = L"monitor (";
      name += monitorCfg.absolutePath;
      name += L")";
      status(monitorCfg,name);
   }

   status(c.backup,L"compactor");
}

void cmdTimestamps(inmem::config& c, const std::wstring& dir)
{
   inmem::setStateWhen(&c.backup.state,inmem::states::kStatus_Ready,
      inmem::states::kCmd_Timestamps,
      10,"waiting for backup to idle");
   ::wcscpy(c.backup.args[0],dir.c_str());
   osEvent(inmem::getServicingProcessTxSignalName(c.backup.servicingProcessId))
      .raise();
   inmem::waitForState(&c.backup.state,inmem::states::kStatus_Ready,
      10,"timeout waiting for akcompact EXE");

   dumpAndDestroyTempFile(c.backup.actionLogFile);
}

void cmdRestore(inmem::config& c, const std::wstring& dir, const std::wstring& timestamp, const std::wstring& dest)
{
   inmem::setStateWhen(&c.backup.state,inmem::states::kStatus_Ready,
      inmem::states::kCmd_Restore,
      10,"waiting for backup to idle");
   ::wcscpy(c.backup.args[0],dir.c_str());
   ::wcscpy(c.backup.args[1],timestamp.c_str());
   ::wcscpy(c.backup.args[2],dest.c_str());
   osEvent(inmem::getServicingProcessTxSignalName(c.backup.servicingProcessId))
      .raise();
   std::wcout << L"this could take awhile" << std::endl;
   inmem::waitForState(&c.backup.state,inmem::states::kStatus_Ready,
      7*60,"timeout waiting for akcompact EXE");

   dumpAndDestroyTempFile(c.backup.actionLogFile);
}

static void stage(inmem::heartbeatComms& c)
{
   inmem::setStateWhen(&c.state,inmem::states::kStatus_Ready,
      inmem::states::kCmd_Stage,
      10,"waiting for backup to idle");
   osEvent(inmem::getServicingProcessTxSignalName(c.servicingProcessId))
      .raise();
}

void cmdStage(inmem::config& c)
{
   for(size_t i=0;i<kNumMonitors;i++)
   {
      auto& monitorCfg = c.monitors[i];
      if(monitorCfg.servicingProcessId == 0) break;
      stage(monitorCfg);
   }
}

void cmdCompact(inmem::config& c)
{
   inmem::setStateWhen(&c.backup.state,inmem::states::kStatus_Ready,
      inmem::states::kCmd_Compact,
      10,"waiting for backup to idle");
   osEvent(inmem::getServicingProcessTxSignalName(c.backup.servicingProcessId))
      .raise();
}
