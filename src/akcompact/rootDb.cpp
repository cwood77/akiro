#include "../cmn/shmem-block.hpp"
#include "../cmn/wlog.hpp"
#include "rootDb.hpp"
#include <fstream>

rootDb::rootDb(inmem::config& c)
{
   m_path = std::wstring(c.backup.absolutePath) + L"\\roots.txt";

   std::wifstream file(m_path.c_str());
   while(file.good())
   {
      std::wstring line;
      std::getline(file,line);
      if(line.empty())
         continue;
      size_t id;
      ::swscanf(line.c_str(),L"%lld",&id);
      std::getline(file,line);
      m_table[line] = id;
      m_usedIds.insert(id);
   }
}

rootDb::~rootDb()
{
   std::wofstream file(m_path.c_str());
   for(auto it=m_table.begin();it!=m_table.end();++it)
   {
      file << it->second << std::endl;
      file << it->first << std::endl;
      file << std::endl;
   }
}

size_t rootDb::findOrAddKey(const std::wstring& monitorRoot)
{
   auto it = m_table.find(monitorRoot);
   if(it != m_table.end())
      return it->second;

   size_t noob = 0;
   if(m_usedIds.size())
      noob = (*--m_usedIds.end())+1;
   m_table[monitorRoot] = noob;
   m_usedIds.insert(noob);

   return noob;
}

size_t rootDb::lookupKey(const std::wstring& monitorRoot)
{
   auto it = m_table.find(monitorRoot);
   if(it != m_table.end())
      return it->second;
   else
      throw std::runtime_error("bad directory given");
}

void rootDb::deleteUnusedRoots(inmem::config& c)
{
   std::set<std::wstring> keepers;
   for(size_t i=0;i<kNumMonitors;i++)
   {
      auto& monitorCfg = c.monitors[i];
      if(monitorCfg.frequencyInMinutes == 0) break;

      auto it = m_table.find(monitorCfg.absolutePath);
      if(it!=m_table.end())
         keepers.insert(it->first);
   }

   if(keepers.size() == m_table.size())
   {
      getWorkerLog() << L"no unused roots found" << std::endl;
      return;
   }

   auto tableCopy = m_table;
   m_table.clear();
   m_usedIds.clear();
   for(auto it=tableCopy.begin();it!=tableCopy.end();++it)
   {
      if(keepers.find(it->first)!=keepers.end())
      {
         getWorkerLog() << L"retaining root " << it->first << std::endl;
         m_table[it->first] = it->second;
         m_usedIds.insert(it->second);
         continue;
      }

      getWorkerLog()
         << L"root "
         << it->first
         << L" (" << it->second << L")"
         << L" no longer referenced" << std::endl;
   }
}
