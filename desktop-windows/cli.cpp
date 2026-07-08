// cli.cpp - Command Line Interface for urFileManager (Windows)
//
// Builds into ufmgr-cli.exe. Provides the same organization / revert / PDF
// behaviour as the GUI but driven entirely from the command line, with
// coloured, easy-to-read output. See windows_usage.md for runnable examples.
//
// Shared logic (config parsing, PDF report, revert) lives in urfm_common.cpp.
#include "urfm_common.h"
#include <conio.h>

// ---------------------------------------------------------------------------
//  Console helpers (coloured, easy-to-read CLI output)
// ---------------------------------------------------------------------------
enum class CCol { Def, Bold, Cyan, Green, Yellow, Red, BoldCyan, BoldYellow, BoldGreen };

WORD ColorAttr(CCol c) {
    switch (c) {
        case CCol::Bold:       return FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        case CCol::Cyan:       return FOREGROUND_GREEN | FOREGROUND_BLUE;
        case CCol::Green:      return FOREGROUND_GREEN;
        case CCol::Yellow:     return FOREGROUND_RED | FOREGROUND_GREEN;
        case CCol::Red:        return FOREGROUND_RED;
        case CCol::BoldCyan:   return FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE;
        case CCol::BoldYellow: return FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN;
        case CCol::BoldGreen:  return FOREGROUND_INTENSITY | FOREGROUND_GREEN;
        default:               return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
}

// Print a coloured piece of text, then restore the previous colour so later
// output is unaffected. Falls back gracefully when output is redirected.
void COut(CCol c, const std::wstring& s) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO ci; WORD old = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    if (GetConsoleScreenBufferInfo(h, &ci)) old = ci.wAttributes;
    SetConsoleTextAttribute(h, ColorAttr(c));
    std::wcout << s;
    SetConsoleTextAttribute(h, old);
}

// std::wstring -> std::string helper (used when feeding data to the PDF writer).
std::string W2S(const std::wstring& w) { return std::string(w.begin(), w.end()); }

// Make sure we have a usable console and return true only when we had to
// create a NEW one (so the caller can pause before exiting). If a console is
// already present (e.g. a -mconsole build, or a console launched from a
// terminal) we just use it and never call AllocConsole, which would fail.
// Make sure we have a writable stdout/stderr. For a console-subsystem build
// (the CLI is compiled with -mconsole) the terminal is already attached, so
// this is a no-op. When launched some other way we try to attach to the parent
// console. We deliberately never call AllocConsole() because doing so on a
// console-subsystem process is what crashed earlier.
bool EnsureConsole() {
    if (AttachConsole(ATTACH_PARENT_PROCESS)) return false; // attached to parent terminal
    return false;                                            // no parent console; -mconsole will provide one
}

// ---------------------------------------------------------------------------
//  PrintUsage
//  Displays the command-line help text with clear, runnable examples.
// ---------------------------------------------------------------------------
void PrintUsage() {
    COut(CCol::BoldCyan, L"\n  urFileManager - Command Line Usage\n");
    COut(CCol::Def,       L"  ===================================\n\n");
    std::wcout << L"  No arguments  -> Launch the graphical interface (.exe launcher)\n";
    std::wcout << L"      ufmgr.exe\n\n";
    std::wcout << L"  Organize a folder (PREVIEW ONLY by default - nothing is moved):\n";
    std::wcout << L"      ufmgr.exe \"C:\\Users\\YourName\\Downloads\"\n\n";
    std::wcout << L"  Actually move the files (no preview):\n";
    std::wcout << L"      ufmgr.exe \"C:\\Users\\YourName\\Downloads\" --no-dry-run\n\n";
    std::wcout << L"  Reverse a previous organization. Moves the files back to the\n";
    std::wcout << L"  folder root, deletes the now-empty category folders and removes\n";
    std::wcout << L"  the generated PDF report:\n";
    std::wcout << L"      ufmgr.exe --revert \"C:\\Users\\YourName\\Downloads\"\n\n";
    std::wcout << L"  Force the graphical interface even when a folder is given:\n";
    std::wcout << L"      ufmgr.exe --gui\n\n";
    std::wcout << L"  Options:\n";
    std::wcout << L"      --dry-run       Preview moves, change nothing (CLI default)\n";
    std::wcout << L"      --no-dry-run    Perform the actual file moves\n";
    std::wcout << L"      --revert <dir>  Undo a previous organization of <dir>\n";
    std::wcout << L"      --gui           Open the graphical interface\n";
    std::wcout << L"      -h, --help      Show this help message\n\n";
    std::wcout << L"  Example folder locations:\n";
    std::wcout << L"      \"C:\\Users\\YourName\\Downloads\"\n";
    std::wcout << L"      \"D:\\Photos 2024\"\n";
    std::wcout << L"      \"E:\\Work\\Inbox\"\n\n";
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int nShow) {
    LPWSTR* args; int n; args = CommandLineToArgvW(GetCommandLineW(), &n);
    bool cli = false, forceGui = false, revert = false, showHelp = false;
    std::wstring dir; bool dry = true;   // CLI defaults to a safe dry-run preview
    if (args && n > 1) {
        cli = true;
        for (int i = 1; i < n; i++) {
            std::wstring a = args[i];
            // Accept both "--flag" and "-flag" styles.
            if (a.size() > 2 && a[0] == L'-' && a[1] == L'-') a = a.substr(1);
            if (a == L"-dry-run") dry = true;
            else if (a == L"-no-dry-run") dry = false;
            else if (a == L"-gui") forceGui = true;
            else if (a == L"-revert") revert = true;
            else if (a == L"-help" || a == L"-h" || a == L"/?") showHelp = true;
            else if (a[0] != L'-' && dir.empty()) dir = a;   // first non-flag arg = folder
        }
        LocalFree(args);
    }

    // --help / -h: show the usage text and exit.
    if (showHelp) {
        bool alloc = EnsureConsole();
        PrintUsage();
        if (alloc) { std::wcout << L"\n  Press any key to exit . . . "; _getwch(); std::wcout << L"\n"; }
        return 0;
    }

    // --gui: this is the CLI binary, so if the user explicitly asks for the GUI
    // we just point them to the separate GUI launcher.
    if (forceGui) {
        bool alloc = EnsureConsole();
        COut(CCol::Yellow, L"  This is the CLI build. To use the graphical interface,\n  run the GUI launcher (ufmgr.exe / gui_app.exe) instead.\n");
        if (alloc) { std::wcout << L"\n  Press any key to exit . . . "; _getwch(); std::wcout << L"\n"; }
        return 0;
    }

    if (cli) {
        bool alloc = EnsureConsole();
        std::wcout << L"\n";
        COut(CCol::BoldCyan, L"  urFileManager - CLI Organizer\n");
        COut(CCol::Def,       L"  ================================\n");

        // --revert <folder>: undo a previous organization of that folder.
        if (revert) {
            if (dir.empty()) {
                COut(CCol::Red, L"  Usage: ufmgr-cli.exe --revert <directory>\n");
                if (alloc) { std::wcout << L"\n  Press any key to exit . . . "; _getwch(); std::wcout << L"\n"; }
                return 1;
            }
            std::filesystem::path src(dir);
            if (!std::filesystem::exists(src) || !std::filesystem::is_directory(src)) {
                COut(CCol::Red, L"  Invalid directory: " + src.wstring() + L"\n");
                if (alloc) { std::wcout << L"\n  Press any key to exit . . . "; _getwch(); std::wcout << L"\n"; }
                return 1;
            }
            RevertFolder(src);
            if (alloc) { std::wcout << L"\n  Press any key to exit . . . "; _getwch(); std::wcout << L"\n"; }
            return 0;
        }

        if (dir.empty()) {
            PrintUsage();
            if (alloc) { std::wcout << L"\n  Press any key to exit . . . "; _getwch(); std::wcout << L"\n"; }
            return 1;
        }
        std::filesystem::path src(dir);
        if (!std::filesystem::exists(src) || !std::filesystem::is_directory(src)) {
            COut(CCol::Red, L"  Invalid directory: " + src.wstring() + L"\n");
            if (alloc) { std::wcout << L"\n  Press any key to exit . . . "; _getwch(); std::wcout << L"\n"; }
            return 1;
        }
        wchar_t exe[MAX_PATH]; GetModuleFileNameW(NULL, exe, MAX_PATH);
        std::wstring json = ReadFile(std::filesystem::path(exe).parent_path() / "config.json");
        auto cfg = ParseCfg(json); if (cfg.empty()) {
            COut(CCol::Red, L"  Failed to load config.json\n");
            if (alloc) { std::wcout << L"\n  Press any key to exit . . . "; _getwch(); std::wcout << L"\n"; }
            return 1;
        }
        std::vector<std::filesystem::path> files;
        for (const auto& e : std::filesystem::directory_iterator(src)) if (e.is_regular_file()) files.push_back(e.path());
        size_t total = files.size();
        COut(CCol::Def, L"  Folder : ");
        COut(CCol::Bold, src.wstring() + L"\n");
        COut(CCol::Def, L"  Found  : ");
        COut(CCol::Bold, std::to_wstring(total) + L" file(s)\n\n");

        size_t ok = 0;
        std::wofstream log(L"organizer.log", std::ios::app);
        std::vector<MovedFileInfoStr> moved;
        for (const auto& fp : files) {
            std::wstring ext = fp.extension().wstring();
            for (auto& c : ext) c = std::tolower(c);
            std::wstring dest = L"Other";
            for (const auto& [cat, exts] : cfg) { for (const auto& e : exts) { if (ext == e) { dest = cat; break; } } if (dest != L"Other") break; }
            std::filesystem::path dd = src / dest;
            std::wstring fn = fp.filename().wstring();
            uint64_t sz = 0;
            try { sz = std::filesystem::file_size(fp); } catch (...) {}
            if (dry) {
                COut(CCol::Yellow, L"  [DRY-RUN] ");
                COut(CCol::Def, fn + L"  ->  " + dest + L"/\n");
                if (log.is_open()) log << L"DRY-RUN: " << fn << L" -> " << dest << L"/\n";
                moved.push_back({ W2S(fn), W2S(dest), "Dry Run Preview", sz });
                ok++;
            } else {
                try {
                    std::filesystem::create_directories(dd);
                    std::filesystem::path df = dd / fn;
                    int ctr = 1; std::wstring stem = fp.stem().wstring();
                    while (std::filesystem::exists(df)) { df = dd / (stem + L" (" + std::to_wstring(ctr) + L")" + ext); ctr++; }
                    std::filesystem::rename(fp, df);
                    COut(CCol::Green, L"  [MOVED]   ");
                    COut(CCol::Def, fn + L"  ->  " + dest + L"/\n");
                    if (log.is_open()) log << L"MOVED: " << fn << L" -> " << dest << L"/\n";
                    moved.push_back({ W2S(fn), W2S(dest), ctr > 1 ? "Renamed" : "Moved", sz });
                    ok++;
                } catch (const std::exception& e) {
                    std::wstring we(e.what(), e.what() + strlen(e.what()));
                    COut(CCol::Red, L"  [ERROR]   ");
                    COut(CCol::Def, fn + L" - " + we + L"\n");
                    moved.push_back({ W2S(fn), W2S(dest), "Error", sz });
                }
            }
        }
        if (log.is_open()) log.close();

        // Generate the same PDF report the GUI produces (saved in the folder).
        if (!moved.empty()) {
            std::wstring reportName = dry ? L"organization_report_preview.pdf" : L"organization_report.pdf";
            std::filesystem::path outPath = src / reportName;
            std::string sp = outPath.string();
            std::string tf = src.string();
            try { GeneratePDFReportStr(sp, tf, moved, dry); }
            catch (...) {}
            COut(CCol::Def, L"\n  Report : ");
            COut(CCol::Cyan, outPath.wstring() + L"\n");
        }

        // Clear, GUI-like summary.
        if (dry) {
            COut(CCol::BoldYellow, L"\n  ============================================\n");
            COut(CCol::BoldYellow, L"   DRY RUN - NO FILES WERE MOVED\n");
            COut(CCol::BoldYellow, L"  ============================================\n");
            COut(CCol::Def, L"   This was only a preview. To actually organize,\n   run the same command again with ");
            COut(CCol::Bold, L"--no-dry-run\n");
        } else {
            COut(CCol::BoldGreen, L"\n  ============================================\n");
            COut(CCol::BoldGreen, L"   DONE - Organized " + std::to_wstring(ok) + L"/" + std::to_wstring(total) + L" files\n");
            COut(CCol::BoldGreen, L"  ============================================\n");
        }
        if (alloc) { std::wcout << L"\n  Press any key to exit . . . "; _getwch(); std::wcout << L"\n"; }
        return 0;
    }

    // No arguments: this is the CLI binary, so tell the user how to launch the GUI.
    {
        bool alloc = EnsureConsole();
        COut(CCol::Yellow, L"  This is the CLI build. To use the graphical interface,\n  run the GUI launcher (ufmgr.exe / gui_app.exe) instead.\n  Or pass a folder to organize it, e.g.:\n");
        COut(CCol::Bold, L"      ufmgr-cli.exe \"C:\\Users\\YourName\\Downloads\"\n");
        if (alloc) { std::wcout << L"\n  Press any key to exit . . . "; _getwch(); std::wcout << L"\n"; }
        return 0;
    }
}
