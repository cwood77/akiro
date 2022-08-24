#pragma once
#include "../cmn/crypto.hpp"
#include <set>

namespace inmem { class config; }

class referencedHashList {
public:
   void add(const std::wstring& hash) { m_keepers.insert(hash); }
   bool isPresent(const std::wstring& hash)
   { return m_keepers.find(hash)!=m_keepers.end(); }

private:
   std::set<std::wstring> m_keepers;
};

class fileDb {
public:
   explicit fileDb(inmem::config& c);

   // returns hash *and* adds to db
   std::wstring addOrFetch(const std::wstring& path);

   void deleteUnusedFiles(referencedHashList& keepers);

private:
   std::wstring computeHash(const std::wstring& path);

   std::wstring m_dbRootPath;
   autoCryptoContext m_ctxt;
};
