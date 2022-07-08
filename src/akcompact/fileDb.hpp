#pragma once
#include "../cmn/crypto.hpp"

namespace inmem { class config; }

class fileDb {
public:
   explicit fileDb(inmem::config& c);

   // returns hash *and* adds to db
   std::wstring addOrFetch(const std::wstring& path);

private:
   std::wstring computeHash(const std::wstring& path);

   std::wstring m_dbRootPath;
   autoCryptoContext m_ctxt;
};
