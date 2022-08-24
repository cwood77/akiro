#pragma once
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "wincrypt.h"
#include <string>
#include <vector>

class autoCryptoContext {
public:
   autoCryptoContext();
   ~autoCryptoContext();

   HCRYPTPROV getHandle() { return m_han; }

private:
   HCRYPTPROV m_han;
};

class fileHasher {
public:
   ~fileHasher();

   void addFile(const std::wstring& path);

protected:
   explicit fileHasher(autoCryptoContext& c, ALG_ID alg);

   HCRYPTPROV m_hHash;
};

class md5Hash {
public:
   md5Hash();

   size_t getSize() { return m_hash.size(); }

   std::wstring toString();

   BYTE& getBuffer() { return m_hash[0]; }

private:
   std::vector<BYTE> m_hash;
};

class md5Hasher : public fileHasher {
public:
   explicit md5Hasher(autoCryptoContext& c) : fileHasher(c,CALG_MD5) {}

   void get(md5Hash& h);
};
