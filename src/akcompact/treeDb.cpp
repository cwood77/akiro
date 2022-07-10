#define WIN32_LEAN_AND_MEAN
#include "../cmn/file.hpp"
#include "../cmn/shmem-block.hpp"
#include "../cmn/wlog.hpp"
#include "fileDb.hpp"
#include "treeDb.hpp"
#include "windows.h"
#include <fstream>
#include <sstream>

void treeListing::save(std::wostream& s)
{
   for(auto it=files.begin();it!=files.end();++it)
      s << it->second << L":" << (it->first.c_str()+basePath.length()) << std::endl;
}

void treeListing::load(std::wistream& s)
{
   while(s.good())
   {
      std::wstring line;
      std::getline(s,line);
      if(line.empty())
         continue;

      auto iColon = line.find(L':');
      if(iColon == std::wstring::npos)
         throw std::runtime_error("bad line format in timestamp file");

      std::wstring hash(line.c_str(),iColon);
      std::wstring path(line.c_str()+iColon+1);
      files[basePath + path] = hash;
   }
}

void treeListing::elaborate(const std::wstring& bPath)
{
   WIN32_FIND_DATAW fData;
   HANDLE hFind = ::FindFirstFileW((bPath + L"\\*").c_str(),&fData);
   if(hFind == INVALID_HANDLE_VALUE)
      return;
   do
   {
      if(fData.cFileName == std::wstring(L"."))
         continue;
      if(fData.cFileName == std::wstring(L".."))
         continue;

      std::wstring fullPath = bPath + L"\\" + fData.cFileName;

      if(fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
         elaborate(fullPath);
      else
         files[fullPath];
   }
   while(::FindNextFileW(hFind,&fData));
   ::FindClose(hFind);
}

void timestampBucket::configure(time_t now, inmem::retentionPolicy& c)
{
   time_t cuttOff = now - c.olderThanInDays*24*60*60;
   m_timestamp = treeDb::format(cuttOff);
   m_toKeep = c.keepMax;
}

bool timestampBucket::addIf(const std::wstring& timestamp)
{
   bool mine = timestamp < m_timestamp;
   if(mine)
      m_matches.insert(timestamp);
   return mine;
}

void timestampBucket::cull(const std::wstring& basePath)
{
   getWorkerLog() << L"----bucket has " << m_matches.size() << L" candidate(s) and should keep " << m_toKeep << std::endl;
   auto lToKeep = m_toKeep;

   if(lToKeep && m_matches.size())
   {
      getWorkerLog() << L" keeping " << *m_matches.begin() << L" b/c it's the first" << std::endl;
      m_keepers.insert(*m_matches.begin());
      lToKeep--;
   }
   if(lToKeep && (m_matches.size() > 1))
   {
      getWorkerLog() << L" keeping " << *(--m_matches.end()) << L" b/c it's the last" << std::endl;
      m_keepers.insert(*(--m_matches.end()));
      lToKeep--;
   }
   if(lToKeep && (m_matches.size() > 2))
      keepMore(*m_matches.begin(),*(--m_matches.end()),lToKeep);

   getWorkerLog() << L"keeping "
      << m_keepers.size() << L" out of " << m_matches.size()
      << L" when requested to keep " << m_toKeep << std::endl;

   for(auto timestamp : m_matches)
   {
      if(m_keepers.find(timestamp)==m_keepers.end())
      {
         getWorkerLog() << L"deleting unretained timestamp " << timestamp << std::endl;
         deleteFolderAndAllContents(basePath + L"\\" + timestamp);
      }
   }
}

void timestampBucket::rebuild(std::set<std::wstring>& s)
{
   for(auto ts : m_keepers)
      s.insert(ts);
}

void timestampBucket::keepMore(const std::wstring& top, const std::wstring& bottom, size_t nKeep)
{
   if(nKeep == 0) return;

   // calculate range
   time_t topTime = treeDb::scan(top);
   time_t bottomTime = treeDb::scan(bottom);

   // calculate midpoint
   auto midpoint = treeDb::format((bottomTime - topTime) / 2 + topTime);

   // search for midpoint
   size_t sizeBefore = m_matches.size();
   m_matches.insert(midpoint);
   bool midpointAbsent = (sizeBefore != m_matches.size());
   auto it = m_matches.find(midpoint);

   // if not found, move down until found
   if(midpointAbsent)
      ++it;

   // if still not found, move up until found
   if(*it == bottom)
   {
      it = m_matches.find(midpoint);
      --it;
   }

   // if still not found, give up and return
   if(*it == top)
   {
      if(midpointAbsent)
         m_matches.erase(midpoint);
      return;
   }
   auto midpointAdjusted = *it;
   if(midpointAbsent)
      m_matches.erase(midpoint);

   // otherwise, add found point and nKeep--;
   getWorkerLog() << L"adding 'midpoint' " << midpointAdjusted << L", but ideally would have gotten " << midpoint << std::endl;
   m_keepers.insert(midpointAdjusted);
   nKeep--;

   // if nKeep == 0, done and return
   if(nKeep == 0)
   {
      return;
   }

   // otherwise bifurcate and recurse
   size_t upperRecurse = nKeep / 2;
   keepMore(top,midpointAdjusted,upperRecurse);
   keepMore(midpointAdjusted,bottom,nKeep-upperRecurse);
}

timestampBucketer::timestampBucketer(inmem::monitorConfig& c, const std::wstring& basePath)
: m_basePath(basePath)
{
   time_t now = ::time(NULL);
   for(size_t i=0;i<10;i++)
   {
      auto& rpol = c.rpolicy[i];
      if(rpol.olderThanInDays == 0)
         return;

      m_buckets.push_back(timestampBucket());
      auto& last = *(--m_buckets.end());
      last.configure(now,rpol);
   }
}

void timestampBucketer::add(std::set<std::wstring>& s)
{
   for(auto value : s)
   {
      bool placed = false;
      for(auto& bucket : m_buckets)
      {
         placed = bucket.addIf(value);
         if(placed)
            break;
      }
      if(!placed)
         getWorkerLog() << L"timestamp " << value << L" claimed by no bucket; not considered for cull." << std::endl;
   }
}

void timestampBucketer::cull()
{
   for(auto& bucket : m_buckets)
      bucket.cull(m_basePath);
}

void timestampBucketer::rebuild(std::set<std::wstring>& s)
{
   for(auto& bucket : m_buckets)
      bucket.rebuild(s);
}

std::wstring treeDb::format(time_t timestamp)
{
   struct tm *pLt = ::localtime(&timestamp);

   wchar_t buffer[MAX_PATH];
   ::wcsftime(
      buffer,
      MAX_PATH,
      L"%Y%m%d-%H%M%S",
      pLt);

   return buffer;
}

time_t treeDb::scan(const std::wstring& timestamp)
{
   struct tm myTime;
   ::memset(&myTime,0,sizeof(struct tm));
   myTime.tm_isdst = -1;

   ::swscanf(timestamp.c_str(),
      L"%04d%02d%02d-%02d%02d%02d",
      &myTime.tm_year,&myTime.tm_mon,&myTime.tm_mday,
      &myTime.tm_hour,&myTime.tm_min,&myTime.tm_sec);

   myTime.tm_year -= 1900;
   myTime.tm_mon--;

   return mktime(&myTime);
}

treeDb::treeDb(inmem::config& c, size_t key)
{
   std::wstringstream stream;
   stream << c.backup.absolutePath << L"\\t\\" << key;
   m_rootPath = stream.str();

   WIN32_FIND_DATAW fData;
   HANDLE hFind = ::FindFirstFileW((m_rootPath + L"\\*").c_str(),&fData);
   if(hFind == INVALID_HANDLE_VALUE)
      return;
   do
   {
      if(fData.cFileName == std::wstring(L".")) continue;
      if(fData.cFileName == std::wstring(L"..")) continue;

      m_timestamps.insert(fData.cFileName);
   }
   while(::FindNextFileW(hFind,&fData));
   ::FindClose(hFind);
}

void treeDb::add(time_t timestamp, treeListing& l)
{
   std::wstring name = format(timestamp);
   if(m_timestamps.find(name)!=m_timestamps.end())
      throw std::runtime_error("timestamp already exists!");

   std::wstring fileName = m_rootPath + L"\\" + name;
   ensurePathForFileExists(fileName);
   std::wofstream file(fileName.c_str());
   if(!file.good())
      throw std::runtime_error("failed to write tree listing");
   l.basePath = m_rootPath;
   l.save(file);
}

void treeDb::dump(std::wostream& s)
{
   s << L"the following timestamps are available = [" << std::endl;
   for(auto timestamp : m_timestamps)
      s << L"   " << timestamp << std::endl;
   s << L"]" << std::endl;
}

void treeDb::load(const std::wstring& timestamp, treeListing& l)
{
   std::wifstream file((m_rootPath + L"\\" + timestamp).c_str());
   if(!file.good())
      throw std::runtime_error("timestamp file not found");
   l.basePath = m_rootPath;
   l.load(file);
}

void treeDb::deleteUnusedTrees(inmem::config& c, const std::set<size_t>& referencedKeys)
{
   std::wstringstream stream;
   stream << c.backup.absolutePath << L"\\t";
   std::wstring rootPath = stream.str();

   WIN32_FIND_DATAW fData;
   HANDLE hFind = ::FindFirstFileW((rootPath + L"\\*").c_str(),&fData);
   if(hFind == INVALID_HANDLE_VALUE)
   {
      getWorkerLog() << L"no trees at all!" << std::endl;
      return;
   }
   do
   {
      if(fData.cFileName == std::wstring(L".")) continue;
      if(fData.cFileName == std::wstring(L"..")) continue;

      size_t key = 0;
      ::swscanf(fData.cFileName,L"%lld",&key);
      if(referencedKeys.find(key) == referencedKeys.end())
      {
         getWorkerLog() << L"deleting all trees for key " << key << std::endl;
         deleteFolderAndAllContents(rootPath + L"\\" + fData.cFileName);
      }
   }
   while(::FindNextFileW(hFind,&fData));
   ::FindClose(hFind);
}

void treeDb::collectReferencedFiles(referencedHashList& keepers)
{
   for(auto ts : m_timestamps)
   {
      treeListing l;
      load(ts,l);
      for(auto it=l.files.begin();it!=l.files.end();++it)
         keepers.add(it->second);
   }
}

void treeDb::cull(inmem::monitorConfig& c)
{
   timestampBucketer bucker(c,m_rootPath);
   bucker.add(m_timestamps);
   bucker.cull();
   m_timestamps.clear();
   bucker.rebuild(m_timestamps);

   getWorkerLog() << std::endl;
   getWorkerLog() << L"after cull" << std::endl;
   dump(getWorkerLog());
}
