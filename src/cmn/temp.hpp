#pragma once
#include <string>

std::wstring reserveTempFilePath(const std::wstring& hint);
void dumpAndDestroyTempFile(const std::wstring& path);
