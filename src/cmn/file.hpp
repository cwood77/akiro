#pragma once
#include <string>

bool fileExists(const std::wstring& path);
void copyFileSd(const std::wstring& source, const std::wstring& dest);
void ensurePathExists(const std::wstring& path);
void ensurePathForFileExists(const std::wstring& path);
void deleteFolderAndAllContents(const std::wstring& path);
void copyDiskTree(const std::wstring& baseSrcPath, const std::wstring& baseDestPath, bool allowErrors);
