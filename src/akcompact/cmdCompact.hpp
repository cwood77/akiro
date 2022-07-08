#pragma once
#include <string>

namespace inmem { class config; }

void cmdCompact(inmem::config& c);
void cmdTimestamps(inmem::config& c, const std::wstring& arg);
void cmdRestore(inmem::config& c, const std::wstring& dir, const std::wstring& timestamp, const std::wstring& dest);
