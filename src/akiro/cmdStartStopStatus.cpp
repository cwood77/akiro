#include "../cmn/path.hpp"
#include "../cmn/shmem-block.hpp"
#include "../cmn/shmem.hpp"
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

   std::cout << "starting workers..." << std::endl;
   launchProcess(exeAdjacentPath(L"akcompact.exe -r"));
   inmem::waitForState(&c.backup.state,inmem::states::kStatus_Ready,
      10,"timeout waiting for akcompact EXE");
}

void cmdStop(inmem::config& c)
{
   std::cout << "shutting down backup..." << std::endl;
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
   std::cout << "backup process is responsive" << std::endl;
}
