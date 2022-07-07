#pragma once

namespace inmem { class config; }

class fileDb {
public:
   explicit fileDb(inmem::config& c);

   std::wstring addOrFetch(const std::wstring& path);
};
