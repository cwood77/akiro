#pragma once
#include <map>
#include <set>
#include <string>

namespace inmem { class config; }

class treeListing {
public:
   std::map<std::wstring,std::wstring> files;

   void elaborate(const std::wstring& basePath);
};

class treeDb {
public:
   treeDb(inmem::config& c, size_t key);

   void add(time_t timestamp, treeListing& l);

private:
   std::wstring m_rootPath;
   std::set<std::wstring> m_timestamps;
};
