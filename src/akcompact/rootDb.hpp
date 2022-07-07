#pragma once
#include <map>
#include <set>
#include <string>

namespace inmem { class config; }

class rootDb {
public:
   explicit rootDb(inmem::config& c);
   ~rootDb();

   size_t findOrAddKey(const std::wstring& monitorRoot);

private:
   std::wstring m_path;
   std::map<std::wstring,size_t> m_table;
   std::set<size_t> m_usedIds;
};
