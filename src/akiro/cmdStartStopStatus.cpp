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
      if(monitorCfg.frequencyInMinutes == 0) break;

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

static void status(inmem::heartbeatComms& c, const std::wstring& userName)
{
   std::wcout << userName << " - ";
   long ans = ::InterlockedIncrement(&c.heartbeat);
   osEvent(inmem::getServicingProcessTxSignalName(c.servicingProcessId))
      .raise();
   if(!inmem::waitForState(&c.heartbeatAwk,ans,10))
      std::wcout << L"doesn't seem responsive!!!!" << std::endl;
   else
      std::wcout << L"responsive" << std::endl;
}

void cmdStatus(inmem::config& c)
{
   for(size_t i=0;i<kNumMonitors;i++)
   {
      auto& monitorCfg = c.monitors[i];
      if(monitorCfg.frequencyInMinutes == 0) break;
      std::wstring name = L"monitor (";
      name += monitorCfg.absolutePath;
      name += L")";
      status(monitorCfg,name);
   }

   status(c.backup,L"compactor");
}

void cmdCompact(inmem::config& c)
{
   inmem::setStateWhen(&c.backup.state,inmem::states::kStatus_Ready,
      inmem::states::kCmd_Compact,
      10,"timeout telling backup to compact");
   osEvent(inmem::getServicingProcessTxSignalName(c.backup.servicingProcessId))
      .raise();
   inmem::waitForState(&c.backup.state,inmem::states::kStatus_Ready,
      10,"timeout waiting for akcompact EXE");

   dumpAndDestroyTempFile(c.backup.actionLogFile);
}
