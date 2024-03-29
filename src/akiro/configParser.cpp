#include "../cmn/shmem-block.hpp"
#include "configParser.hpp"
#include <iostream>

void configParser::load(std::wistream& s, inmem::config& c)
{
   configParser self(c);
   do
   {
      std::wstring line;
      std::getline(s,line);
      self.parseLine(line);
   } while(s.good());
   self.closeState();

   if(::wcslen(c.backup.absolutePath)==0)
      throw std::runtime_error("at least a backup path must be indicated in the config file");
}

void configParser::parseLine(const std::wstring& line)
{
   if(line.empty()) return; // discard empties
   const wchar_t *pThumb = line.c_str();

   for(;*pThumb==L' ';++pThumb); // eat leading whitespace

   if(*pThumb=='#') return; // discard comments

   if(m_state == kRPolicy)
   {
      if(startsWithAndAdvance(pThumb,L"older-than-in-days: "))
         ::swscanf(pThumb,L"%lld",
            &m_config.monitors[m_iMonitor].rpolicy[m_iRPolicy].olderThanInDays);
      else if(startsWithAndAdvance(pThumb,L"keep-at-most-timestamps: "))
         ::swscanf(pThumb,L"%lld",
            &m_config.monitors[m_iMonitor].rpolicy[m_iRPolicy].keepMax);
      else
         closeState();
   }

   if(m_state == kMonitor)
   {
      if(startsWithAndAdvance(pThumb,L"frequency-in-minutes: "))
         ::swscanf(pThumb,L"%lld",&m_config.monitors[m_iMonitor].frequencyInMinutes);
      else if(startsWithAndAdvance(pThumb,L"last-stage-log-absolute-path: "))
         ::wcscpy(m_config.monitors[m_iMonitor].lastStageLogAbsolutePath,pThumb);
      else if(startsWithAndAdvance(pThumb,L"enabled: true"))
         m_config.monitors[m_iMonitor].enabled = true;
      else if(startsWithAndAdvance(pThumb,L"enabled: false"))
         m_config.monitors[m_iMonitor].enabled = false;
      else if(::wcscmp(pThumb,L"retention-policy:")==0)
      {
         m_state = kRPolicy;
         m_iRPolicy = 0;
      }
      else if(::wcscmp(pThumb,L"retention-policy: same-as-last-monitor")==0)
      {
         if(m_iMonitor == 0)
            throw std::runtime_error("used 'same-as-last-monitor' on first monitor!");
         ::memcpy(
            m_config.monitors[m_iMonitor].rpolicy,
            m_config.monitors[m_iMonitor-1].rpolicy,
            sizeof(inmem::retentionPolicy)*10);
      }
      else
         closeState();
   }
   else if(m_state == kBackup)
   {
      if(startsWithAndAdvance(pThumb,L"absolute-path: "))
         ::wcscpy(m_config.backup.absolutePath,pThumb);
      else if(startsWithAndAdvance(pThumb,L"enforce-retention-frequency-in-days: "))
         ::swscanf(pThumb,L"%lld",&m_config.backup.retentionFrequencyInDays);
      else if(startsWithAndAdvance(pThumb,L"last-compact-log-absolute-path: "))
         ::wcscpy(m_config.backup.lastCompactLogAbsolutePath,pThumb);
      else if(startsWithAndAdvance(pThumb,L"last-cull-log-absolute-path: "))
         ::wcscpy(m_config.backup.lastCullLogAbsolutePath,pThumb);
      else if(startsWithAndAdvance(pThumb,L"last-prune-log-absolute-path: "))
         ::wcscpy(m_config.backup.lastPruneLogAbsolutePath,pThumb);
      else
         closeState();
   }

   if(m_state == kNone)
   {
      if(startsWithAndAdvance(pThumb,L"monitor-absolute-path: "))
      {
         if(m_iMonitor >= kNumMonitors)
            throw std::runtime_error("too many monitors indicated in config file; only a fixed number is supported at this time :(");
         m_state = kMonitor;

         ::wcscpy(m_config.monitors[m_iMonitor].absolutePath,pThumb);
      }
      else if(::wcscmp(pThumb,L"backup:")==0)
         m_state = kBackup;
      else
      {
         std::wcout << L"error on: '" << pThumb << L"'" << std::endl;
         throw std::runtime_error("bad line in config file");
      }
   }
}

bool configParser::startsWithAndAdvance(const wchar_t*& pThumb, const wchar_t *pPattern)
{
   size_t l = ::wcslen(pPattern);
   if(::wcsncmp(pThumb,pPattern,l)==0)
   {
      pThumb += l;
      return true;
   }
   return false;
}

void configParser::closeState()
{
   if(m_state == kRPolicy)
   {
      if(m_config.monitors[m_iMonitor].rpolicy[m_iRPolicy].olderThanInDays == 0)
         throw std::runtime_error("older-than-in-days must be nonzero in config file");
      m_iRPolicy++;
      m_state = kMonitor;
      return;
   }

   if(m_state == kMonitor)
   {
      if(m_config.monitors[m_iMonitor].frequencyInMinutes == 0)
         throw std::runtime_error("frequency must be nonzero in config file");
      m_iMonitor++;
   }
   m_state = kNone;
}
