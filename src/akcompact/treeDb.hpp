#pragma once
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
   void cull();
   void rebuild(std::set<std::wstring>& s);
};

class timestampBucketer {
public:
   timestampBucketer(inmem::monitorConfig& c);

   void add(std::set<std::wstring>& s);
   void cull();
   void rebuild(std::set<std::wstring>& s);
};

class treeDb {
public:
   treeDb(inmem::config& c, size_t key);

   void add(time_t timestamp, treeListing& l);
   void dump(std::wostream& s);
   void load(const std::wstring& timestamp, treeListing& l);

   static void deleteUnusedTrees(inmem::config& c, const std::set<size_t>& referencedKeys);
   void collectReferencedFiles(referencedHashList& keepers);

   void cull(inmem::config&c, const std::wstring& monitorPath);

private:
   static std::wstring format(time_t timestamp);

   std::wstring m_rootPath;
   std::set<std::wstring> m_timestamps;
};
