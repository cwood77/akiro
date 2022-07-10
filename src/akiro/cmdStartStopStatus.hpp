#pragma once
#include <string>

namespace inmem { class config; }

void cmdStart(inmem::config& c);
void cmdStop(inmem::config& c);
void cmdStatus(inmem::config& c);

void cmdTimestamps(inmem::config& c, const std::wstring& dir);
void cmdRestore(inmem::config& c, const std::wstring& dir, const std::wstring& timestamp, const std::wstring& dest);

void cmdStage(inmem::config& c);
void cmdCompact(inmem::config& c);
