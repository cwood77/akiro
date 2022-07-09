#pragma once
#include <map>
#include <set>
#include <string>

namespace inmem { class config; }

class treeListing {
public:
   std::map<std::wstring,std::wstring> files;
   std::wstring basePath;

   void save(std::wostream& s);
   void load(std::wistream& s);

   void elaborate(const std::wstring& basePath);
};

class treeDb {
public:
   treeDb(inmem::config& c, size_t key);

   void add(time_t timestamp, treeListing& l);
   void dump(std::wostream& s);
   void load(const std::wstring& timestamp, treeListing& l);

private:
   static std::wstring format(time_t timestamp);

   std::wstring m_rootPath;
   std::set<std::wstring> m_timestamps;
};
