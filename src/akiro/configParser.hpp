#pragma once
#include <istream>

namespace inmem { class config; }

class configParser {
public:
   static void load(std::wistream& s, inmem::config& c);

private:
   explicit configParser(inmem::config& c)
   : m_config(c), m_iMonitor(0), m_state(kNone) {}

   void parseLine(const std::wstring& line);
   static bool startsWithAndAdvance(const wchar_t*& pThumb, const wchar_t *pPattern);

   inmem::config& m_config;
   size_t m_iMonitor;
   enum {
      kNone,
      kMonitor,
      kBackup
   } m_state;

   void closeState();
};
