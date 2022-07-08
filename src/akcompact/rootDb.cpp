#include "../cmn/shmem-block.hpp"
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
