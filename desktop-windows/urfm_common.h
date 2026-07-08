// urfm_common.h
// Shared declarations for the urFileManager Windows builds.
// Included by both the GUI (gui_app.cpp) and the CLI (cli.cpp) so the two
// versions share the exact same PDF report and config-parsing logic.
#pragma once

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <thread>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <iomanip>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "shell32.lib")

#ifndef WS_EX_NOREDIRECTIONBITMAP
#define WS_EX_NOREDIRECTIONBITMAP 0x00200000L
#endif

// --- Shared config / file helpers (defined in urfm_common.cpp) ---

// Read a (UTF-8) file and return its contents as a wide string.
std::wstring ReadFile(const std::wstring& path);

// Parse the config.json category -> extensions map.
std::map<std::wstring, std::vector<std::wstring>> ParseCfg(const std::wstring& s);

// --- Shared PDF report (defined in urfm_common.cpp) ---
// Plain-string record consumed by the PDF generator.
struct MovedFileInfoStr {
    std::string fileName, category, status;
    uint64_t fileSize;
};

// Generate a standalone PDF report of an organization run into outputPath.
void GeneratePDFReportStr(const std::string& outputPath, const std::string& targetFolder,
                          const std::vector<MovedFileInfoStr>& movedFiles, bool dryRun);

// --- Revert helper shared by GUI undo and the CLI --revert command ---
// Move every file from category sub-folders back into the root folder, remove
// the now-empty category folders, and delete the generated PDF report(s).
void RevertFolder(const std::filesystem::path& src);
