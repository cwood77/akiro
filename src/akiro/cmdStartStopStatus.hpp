#pragma once
#include <string>

namespace inmem { class config; }

void cmdStart(inmem::config& c);
void cmdStop(inmem::config& c);
void cmdStatus(inmem::config& c);
void cmdCompact(inmem::config& c);
void cmdTimestamps(inmem::config& c, const std::wstring& dir);
