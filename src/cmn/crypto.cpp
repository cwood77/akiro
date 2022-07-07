#include "crypto.hpp"
#include <sstream>
#include <stdexcept>

// inspired by MSDN Example C Program: Creating an MD5 Hash from File Content
// https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program--creating-an-md-5-hash-from-file-content

autoCryptoContext::autoCryptoContext()
{
   BOOL success = ::CryptAcquireContext(&m_han,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT);
   if(!success)
      throw std::runtime_error("failed to open crypto context");
}

autoCryptoContext::~autoCryptoContext()
{
   ::CryptReleaseContext(m_han,0);
}

fileHasher::~fileHasher()
{
   ::CryptDestroyHash(m_hHash);
}

void fileHasher::addFile(const std::wstring& path)
{
   HANDLE hFile = ::CreateFileW(
      path.c_str(),
      GENERIC_READ,
      FILE_SHARE_READ,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_SEQUENTIAL_SCAN,
      NULL);
   if(hFile == INVALID_HANDLE_VALUE)
      throw std::runtime_error("can't open file for hashing");

   BYTE buffer[1024];
   DWORD cbRead = 0;
   while(::ReadFile(hFile,buffer,1024,&cbRead,NULL))
   {
      if(cbRead==0) break; // end of file

      BOOL success = ::CryptHashData(m_hHash,buffer,cbRead,0);
      if(!success)
         throw std::runtime_error("error hashing file data");
   }

   ::CloseHandle(hFile);
}

fileHasher::fileHasher(autoCryptoContext& c, ALG_ID alg)
{
   BOOL success = ::CryptCreateHash(c.getHandle(),alg,0,0,&m_hHash);
   if(!success)
      throw std::runtime_error("failed to create hasher");
}

md5Hash::md5Hash()
{
   m_hash.resize(16); // MD5 hash length
}

std::wstring md5Hash::toString()
{
   wchar_t rgbDigits[] = L"0123456789abcdef";
   std::wstringstream stream;
   for(size_t i=0;i<m_hash.size();i++)
   {
      stream << rgbDigits[m_hash[i] >> 4];
      stream << rgbDigits[m_hash[i] & 0xf];
   }
   return stream.str();
}

void md5Hasher::get(md5Hash& h)
{
   DWORD cbHash = h.getSize();
   BOOL success = ::CryptGetHashParam(m_hHash, HP_HASHVAL, &h.getBuffer(), &cbHash, 0);
   if(!success)
      throw std::runtime_error("error getting hash");
}
