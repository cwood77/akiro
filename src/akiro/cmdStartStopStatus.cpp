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

void cmdStart(inmem::config& c)
{
   std::wifstream file(exeAdjacentPath(L"akiro.txt").c_str());
   if(!file.good())
      throw std::runtime_error("config file doesn't exist or can't be opened");

   configParser::load(file,c);

   ensurePathExists(std::wstring(c.backup.absolutePath) + L"\\s"); // staging
   ensurePathExists(std::wstring(c.backup.absolutePath) + L"\\t"); // treeDB
   ensurePathExists(std::wstring(c.backup.absolutePath) + L"\\f"); // fileDB

   std::wcout << L"starting workers..." << std::endl;
   launchProcess(exeAdjacentPath(L"akcompact.exe -r"));
   inmem::waitForState(&c.backup.state,inmem::states::kStatus_Ready,
      10,"timeout waiting for akcompact EXE");
}

void cmdStop(inmem::config& c)
{
   std::wcout << L"shutting down backup..." << std::endl;
   inmem::setStateWhen(&c.backup.state,inmem::states::kStatus_Ready,
      inmem::states::kCmd_Die,
      10,"timeout telling backup to die");
   osEvent(inmem::getServicingProcessTxSignalName(c.backup.servicingProcessId))
      .raise();
   inmem::waitForState(&c.backup.state,inmem::states::kStatus_Dead,
      10,"timeout waiting for akcompact EXE");
}

void cmdStatus(inmem::config& c)
{
   long ans = ::InterlockedIncrement(&c.backup.heartbeat);
   osEvent(inmem::getServicingProcessTxSignalName(c.backup.servicingProcessId))
      .raise();
   inmem::waitForState(&c.backup.heartbeatAwk,ans,10,"backup doesn't seem responsive");
   std::wcout << L"backup process is responsive" << std::endl;
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
