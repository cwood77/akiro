#pragma once
#include <list>
#include <map>
#include <set>
#include <string>

namespace inmem { class config; }
class referencedHashList;

class treeListing {
public:
   std::map<std::wstring,std::wstring> files;
   std::wstring basePath;

   void save(std::wostream& s);
   void load(std::wistream& s);

   void elaborate(const std::wstring& basePath);
};

class timestampBucket {
public:
   void configure(time_t now, inmem::retentionPolicy& c);

   bool addIf(const std::wstring& timestamp);
   void cull(const std::wstring& basePath);
   void rebuild(std::set<std::wstring>& s);

private:
   void keepMore(const std::wstring& top, const std::wstring& bottom, size_t nKeep);

   std::wstring m_timestamp;
   size_t m_toKeep;
   std::set<std::wstring> m_matches;
   std::set<std::wstring> m_keepers;
};

class timestampBucketer {
public:
   timestampBucketer(inmem::monitorConfig& c, const std::wstring& basePath);

   void add(std::set<std::wstring>& s);
   void cull();
   void rebuild(std::set<std::wstring>& s);

private:
   std::list<timestampBucket> m_buckets;
   std::wstring m_basePath;
};

class treeDb {
public:
   static std::wstring format(time_t timestamp);
   static time_t scan(const std::wstring& timestamp);

   treeDb(inmem::config& c, size_t key);

   void add(time_t timestamp, treeListing& l);
   void dump(std::wostream& s);
   void load(const std::wstring& timestamp, treeListing& l);

   static void deleteUnusedTrees(inmem::config& c, const std::set<size_t>& referencedKeys);
   void collectReferencedFiles(referencedHashList& keepers);

   void cull(inmem::monitorConfig& c);

private:
   std::wstring m_rootPath;
   std::set<std::wstring> m_timestamps;
};
