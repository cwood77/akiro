#define WIN32_LEAN_AND_MEAN
#include "../cmn/file.hpp"
#include "../cmn/shmem-block.hpp"
#include "treeDb.hpp"
#include "windows.h"
#include <fstream>
#include <sstream>

void treeListing::save(std::wostream& s)
{
   for(auto it=files.begin();it!=files.end();++it)
      s << it->second << L":" << it->first << std::endl;
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
      files[path] = hash;
   }
}

void treeListing::elaborate(const std::wstring& basePath)
{
   WIN32_FIND_DATAW fData;
   HANDLE hFind = ::FindFirstFileW((basePath + L"\\*").c_str(),&fData);
   if(hFind == INVALID_HANDLE_VALUE)
      return;
   do
   {
      if(fData.cFileName == std::wstring(L"."))
         continue;
      if(fData.cFileName == std::wstring(L".."))
         continue;

      std::wstring fullPath = basePath + L"\\" + fData.cFileName;

      if(fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
         elaborate(fullPath);
      else
         files[fullPath];
   }
   while(::FindNextFileW(hFind,&fData));
   ::FindClose(hFind);
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
   l.save(file);
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
   l.load(file);
}
