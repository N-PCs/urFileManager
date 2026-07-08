// gui_app.cpp - Graphical User Interface for urFileManager (Windows)
//
// Builds into the GUI launcher (ufmgr.exe / gui_app.exe). Contains the Win32
// window procedure, themed custom-drawn controls, the organization worker
// thread, the revert (undo) logic and the report dialog.
//
// Shared logic (config parsing, PDF report, revert) lives in urfm_common.cpp.
#include "urfm_common.h"

// ---------------------------------------------------------------------------
//  Theme + control definitions
// ---------------------------------------------------------------------------
struct Theme {
    std::wstring name;
    COLORREF windowBg, cardBg, editBg, logBg;
    COLORREF textTitle, textSub, textNormal;
    COLORREF accent, accentHover, accentPressed;
    COLORREF sBg, sBorder, sHoverBg, sHoverBorder, sPressedBg, sPressedBorder;
    COLORREF eNorm, eHover, eFocus;
};

const std::vector<Theme> g_themes = {
    { L"Midnight Dark",     RGB(15,15,17), RGB(24,24,27), RGB(39,39,42), RGB(24,24,27),
      RGB(255,255,255), RGB(161,161,170), RGB(228,228,231),
      RGB(99,102,241), RGB(79,70,229), RGB(67,56,202),
      RGB(39,39,42), RGB(39,39,42), RGB(39,39,42), RGB(63,63,70), RGB(39,39,42), RGB(82,82,91),
      RGB(39,39,42), RGB(63,63,70), RGB(99,102,241) },
    { L"Minimalist Light",  RGB(244,244,245), RGB(255,255,255), RGB(244,244,245), RGB(250,250,250),
      RGB(9,9,11), RGB(113,113,122), RGB(39,39,42),
      RGB(79,70,229), RGB(67,56,202), RGB(55,48,163),
      RGB(244,244,245), RGB(228,228,231), RGB(228,228,231), RGB(212,212,216), RGB(212,212,216), RGB(161,161,170),
      RGB(228,228,231), RGB(161,161,170), RGB(79,70,229) },
    { L"Red Sakura",        RGB(255,255,255), RGB(255,232,239), RGB(255,245,247), RGB(255,232,239),
      RGB(51,51,51), RGB(102,102,102), RGB(34,34,34),
      RGB(255,183,197), RGB(255,155,176), RGB(222,49,99),
      RGB(255,245,247), RGB(255,183,197), RGB(255,183,197), RGB(222,49,99), RGB(255,183,197), RGB(222,49,99),
      RGB(255,245,247), RGB(255,183,197), RGB(222,49,99) },
    { L"Forest Emerald",    RGB(20,28,24), RGB(28,40,34), RGB(40,54,46), RGB(28,40,34),
      RGB(240,247,244), RGB(163,186,175), RGB(218,231,224),
      RGB(16,185,129), RGB(5,150,105), RGB(4,120,87),
      RGB(40,54,46), RGB(40,54,46), RGB(40,54,46), RGB(16,185,129), RGB(40,54,46), RGB(5,150,105),
      RGB(40,54,46), RGB(5,150,105), RGB(16,185,129) },
    { L"Neon Cyberpunk",    RGB(10,10,15), RGB(20,15,30), RGB(35,25,50), RGB(20,15,30),
      RGB(0,255,240), RGB(255,0,127), RGB(255,255,255),
      RGB(255,0,127), RGB(255,51,153), RGB(204,0,102),
      RGB(35,25,50), RGB(0,255,240), RGB(35,25,50), RGB(255,0,127), RGB(35,25,50), RGB(255,51,153),
      RGB(35,25,50), RGB(0,255,240), RGB(255,0,127) },
    { L"Obsidian Volt",     RGB(10,10,12), RGB(18,18,20), RGB(26,26,30), RGB(18,18,20),
      RGB(251,251,251), RGB(203,205,213), RGB(224,224,224),
      RGB(220,243,101), RGB(203,226,78), RGB(184,209,69),
      RGB(26,26,30), RGB(220,243,101), RGB(26,26,30), RGB(220,243,101), RGB(26,26,30), RGB(203,226,78),
      RGB(26,26,30), RGB(220,243,101), RGB(203,226,78) },
};

// Message / control IDs
#define WM_APP_LOG      (WM_USER + 1)
#define WM_APP_PROGRESS (WM_USER + 2)
#define WM_APP_STATUS   (WM_USER + 3)
#define WM_APP_DONE     (WM_USER + 4)
#define WM_APP_UNDO_DONE (WM_USER + 5)

#define IDC_PATH_EDIT       1001
#define IDC_BROWSE_BTN      1002
#define IDC_DRY_RUN_CHECK   1003
#define IDC_EDIT_CONFIG_BTN 1004
#define IDC_VIEW_LOG_BTN    1005
#define IDC_ACTION_BTN      1006
#define IDC_LOG_CONSOLE     1007
#define IDC_THEME_COMBO     1008
#define IDC_VIEW_REPORT_BTN 1009
#define IDC_REPORT_LIST     1010
#define IDC_SAVE_PDF_BTN    1011
#define IDC_CLOSE_REPORT_BTN 1012
#define IDC_UNDO_BTN        1013

// ---------------------------------------------------------------------------
//  Globals + small helpers (GUI only)
// ---------------------------------------------------------------------------
struct OrganizeParams { HWND hwnd; std::wstring sourceDirectory; bool dryRun; std::map<std::wstring,std::vector<std::wstring>> fileTypeMap; };
struct MovedFileInfo { std::wstring fileName, category, status; uint64_t fileSize; };
struct ReportData { std::vector<MovedFileInfo>* files; std::wstring* srcDir; bool dryRun; };
struct UndoRecord { std::wstring originalPath; std::wstring movedPath; };

HWND g_hMain = NULL, g_hPath = NULL, g_hBrowse = NULL, g_hDryRun = NULL, g_hCfg = NULL, g_hLogBtn = NULL, g_hAction = NULL, g_hLog = NULL, g_hTheme = NULL, g_hReport = NULL, g_hUndo = NULL;
int g_themeIdx = 0;
HFONT g_fTitle, g_fSub, g_fNormal, g_fBold, g_fLog;
HBRUSH g_bWindow, g_bCard, g_bEdit, g_bLog;
std::atomic<bool> g_running(false), g_cancel(false);
std::map<std::wstring,std::vector<std::wstring>> g_fileTypeMap;
size_t g_processed = 0, g_total = 0;
std::wstring g_status = L"Ready";
std::vector<MovedFileInfo> g_movedFiles;
std::wstring g_srcDir;
bool g_lastDryRun = false;
double g_dpi = 1.0;
int SX(int x) { return (int)(x * g_dpi); }
int SY(int y) { return (int)(y * g_dpi); }

std::vector<UndoRecord> g_undoHistory;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK BtnSubclass(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
LRESULT CALLBACK EditSubclass(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
LRESULT CALLBACK ReportProc(HWND, UINT, WPARAM, LPARAM);

int GetScreenDPI() { HDC hdc = GetDC(NULL); int d = 96; if (hdc) { d = GetDeviceCaps(hdc, LOGPIXELSX); ReleaseDC(NULL, hdc); } return d; }
int GetWindowDPI(HWND h) { typedef UINT(WINAPI *G)(HWND); HMODULE m = GetModuleHandleW(L"user32.dll"); if (m) { G p = (G)GetProcAddress(m, "GetDpiForWindow"); if (p) return p(h); } return GetScreenDPI(); }

std::wstring FormatSizeW(uint64_t bytes) {
    double size = (double)bytes;
    const wchar_t* unit = L"B";
    if (size >= 1024) { size /= 1024; unit = L"KB"; }
    if (size >= 1024) { size /= 1024; unit = L"MB"; }
    if (size >= 1024) { size /= 1024; unit = L"GB"; }
    wchar_t buf[32]; swprintf_s(buf, L"%.2f %s", size, unit);
    return buf;
}

void UpdateBrushes() {
    if (g_bWindow) DeleteObject(g_bWindow); if (g_bCard) DeleteObject(g_bCard);
    if (g_bEdit) DeleteObject(g_bEdit); if (g_bLog) DeleteObject(g_bLog);
    const auto& t = g_themes[g_themeIdx];
    g_bWindow = CreateSolidBrush(t.windowBg); g_bCard = CreateSolidBrush(t.cardBg);
    g_bEdit = CreateSolidBrush(t.editBg); g_bLog = CreateSolidBrush(t.logBg);
}

void AppendLog(const std::wstring& msg) {
    int len = GetWindowTextLengthW(g_hLog);
    SendMessage(g_hLog, EM_SETSEL, len, len);
    SendMessage(g_hLog, EM_REPLACESEL, FALSE, (LPARAM)(msg + L"\r\n").c_str());
    SendMessage(g_hLog, EM_SCROLLCARET, 0, 0);
}

std::wstring Browse(HWND h) {
    std::wstring p;
    IFileOpenDialog* d = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, (void**)&d))) {
        DWORD o; d->GetOptions(&o); d->SetOptions(o | FOS_PICKFOLDERS);
        if (SUCCEEDED(d->Show(h))) { IShellItem* item = nullptr;
            if (SUCCEEDED(d->GetResult(&item))) { PWSTR s = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &s))) { p = s; CoTaskMemFree(s); }
                item->Release(); } }
        d->Release();
    }
    return p;
}

// ---------------------------------------------------------------------------
//  WinMain - launches the GUI
// ---------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow) {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    INITCOMMONCONTROLSEX ix = { sizeof(INITCOMMONCONTROLSEX), ICC_WIN95_CLASSES };
    InitCommonControlsEx(&ix);
    g_dpi = (double)GetScreenDPI() / 96.0;

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc; wc.hInstance = hInst; wc.lpszClassName = L"UrFmWinGUI";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(1));
    RegisterClassW(&wc);

    RECT rc = {0,0,SX(780),SY(620)}; AdjustWindowRect(&rc, WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX, FALSE);
    g_hMain = CreateWindowExW(0, L"UrFmWinGUI", L"urFileManager",
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right-rc.left, rc.bottom-rc.top, NULL, NULL, hInst, NULL);
    if (!g_hMain) return 0;

    BOOL dark = TRUE;
    DwmSetWindowAttribute(g_hMain, 20, &dark, sizeof(dark));
    ShowWindow(g_hMain, nShow); UpdateWindow(g_hMain);
    MSG msg; while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    CoUninitialize(); return 0;
}

// ---------------------------------------------------------------------------
//  Main window procedure
// ---------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        int dpi = GetWindowDPI(hWnd); g_dpi = (double)dpi / 96.0;
        g_fTitle = CreateFontW(-SY(28),0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Segoe UI");
        g_fSub = CreateFontW(-SY(12),0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Segoe UI");
        g_fNormal = CreateFontW(-SY(14),0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Segoe UI");
        g_fBold = CreateFontW(-SY(14),0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Segoe UI");
        g_fLog = CreateFontW(-SY(12),0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,0,FIXED_PITCH,L"Consolas");
        UpdateBrushes();

        int bx = 35;

        // Path edit
        g_hPath = CreateWindowExW(0, L"EDIT", L"", ES_AUTOHSCROLL|WS_CHILD|WS_VISIBLE|ES_LEFT,
            SX(45),SY(122),SX(528),SY(20), hWnd, (HMENU)IDC_PATH_EDIT, GetModuleHandle(NULL), NULL);
        SendMessage(g_hPath, WM_SETFONT, (WPARAM)g_fNormal, TRUE);
        SetWindowSubclass(g_hPath, EditSubclass, 0, 0);

        // Browse button
        g_hBrowse = CreateWindowExW(0, L"BUTTON", L"Browse",
            WS_CHILD|WS_VISIBLE, SX(585),SY(116),SX(110),SY(32), hWnd, (HMENU)IDC_BROWSE_BTN, GetModuleHandle(NULL), NULL);
        SendMessage(g_hBrowse, WM_SETFONT, (WPARAM)g_fBold, TRUE);
        SetWindowSubclass(g_hBrowse, BtnSubclass, IDC_BROWSE_BTN, 0);

        // Dry-run checkbox
        g_hDryRun = CreateWindowExW(0, L"BUTTON", L" Dry Run (Preview only - uncheck to execute)",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, SX(35),SY(162),SX(700),SY(22), hWnd, (HMENU)IDC_DRY_RUN_CHECK, GetModuleHandle(NULL), NULL);
        SendMessage(g_hDryRun, WM_SETFONT, (WPARAM)g_fNormal, TRUE);
        SendMessage(g_hDryRun, BM_SETCHECK, BST_CHECKED, 0);

        // Action buttons row
        g_hCfg = CreateWindowExW(0, L"BUTTON", L"Edit Config",
            WS_CHILD|WS_VISIBLE, SX(bx),SY(200),SX(120),SY(32), hWnd, (HMENU)IDC_EDIT_CONFIG_BTN, GetModuleHandle(NULL), NULL);
        SendMessage(g_hCfg, WM_SETFONT, (WPARAM)g_fBold, TRUE);
        SetWindowSubclass(g_hCfg, BtnSubclass, IDC_EDIT_CONFIG_BTN, 0);

        g_hLogBtn = CreateWindowExW(0, L"BUTTON", L"View Log",
            WS_CHILD|WS_VISIBLE, SX(bx+132),SY(200),SX(120),SY(32), hWnd, (HMENU)IDC_VIEW_LOG_BTN, GetModuleHandle(NULL), NULL);
        SendMessage(g_hLogBtn, WM_SETFONT, (WPARAM)g_fBold, TRUE);
        SetWindowSubclass(g_hLogBtn, BtnSubclass, IDC_VIEW_LOG_BTN, 0);

        g_hReport = CreateWindowExW(0, L"BUTTON", L"View Report",
            WS_CHILD|WS_VISIBLE, SX(bx+264),SY(200),SX(120),SY(32), hWnd, (HMENU)IDC_VIEW_REPORT_BTN, GetModuleHandle(NULL), NULL);
        SendMessage(g_hReport, WM_SETFONT, (WPARAM)g_fBold, TRUE);
        SetWindowSubclass(g_hReport, BtnSubclass, IDC_VIEW_REPORT_BTN, 0);
        ShowWindow(g_hReport, SW_HIDE);

        // Theme combo
        g_hTheme = CreateWindowExW(0, L"COMBOBOX", L"",
            WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL, SX(520),SY(203),SX(220),SY(150), hWnd, (HMENU)IDC_THEME_COMBO, GetModuleHandle(NULL), NULL);
        SendMessage(g_hTheme, WM_SETFONT, (WPARAM)g_fNormal, TRUE);
        for (const auto& t : g_themes) SendMessage(g_hTheme, CB_ADDSTRING, 0, (LPARAM)t.name.c_str());
        SendMessage(g_hTheme, CB_SETCURSEL, g_themeIdx, 0);

        // Start button
        g_hAction = CreateWindowExW(0, L"BUTTON", L"Start Organizing",
            WS_CHILD|WS_VISIBLE, SX(35),SY(248),SX(220),SY(40), hWnd, (HMENU)IDC_ACTION_BTN, GetModuleHandle(NULL), NULL);
        SendMessage(g_hAction, WM_SETFONT, (WPARAM)g_fBold, TRUE);
        SetWindowSubclass(g_hAction, BtnSubclass, IDC_ACTION_BTN, 1);

        // Undo button
        g_hUndo = CreateWindowExW(0, L"BUTTON", L"Undo Last Organize",
            WS_CHILD|WS_VISIBLE, SX(268),SY(248),SX(220),SY(40), hWnd, (HMENU)IDC_UNDO_BTN, GetModuleHandle(NULL), NULL);
        SendMessage(g_hUndo, WM_SETFONT, (WPARAM)g_fBold, TRUE);
        SetWindowSubclass(g_hUndo, BtnSubclass, IDC_UNDO_BTN, 2);
        EnableWindow(g_hUndo, FALSE);

        // Log console
        g_hLog = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY,
            SX(35),SY(360),SX(710),SY(215), hWnd, (HMENU)IDC_LOG_CONSOLE, GetModuleHandle(NULL), NULL);
        SendMessage(g_hLog, WM_SETFONT, (WPARAM)g_fLog, TRUE);
        SendMessage(g_hLog, EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELPARAM(6,6));

        wchar_t exe[MAX_PATH]; GetModuleFileNameW(NULL, exe, MAX_PATH);
        std::wstring json = ReadFile(std::filesystem::path(exe).parent_path() / "config.json");
        g_fileTypeMap = ParseCfg(json);
        if (g_fileTypeMap.empty()) {
            AppendLog(L"[ERROR] config.json not found or invalid. Place it next to ufmgr.exe.");
            EnableWindow(g_hAction, FALSE);
        } else {
            AppendLog(L"urFileManager v1.0");
            AppendLog(L"Loaded configuration from config.json");
            AppendLog(L"Select a folder and click \"Start Organizing\" to begin.");
            AppendLog(L"Dry-run mode is ON by default for safety.");
        }
        break;
    }

    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam; HWND h = (HWND)lParam;
        const auto& t = g_themes[g_themeIdx];
        if (h == g_hLog) { SetTextColor(hdc, t.textNormal); SetBkColor(hdc, t.logBg); return (INT_PTR)g_bLog; }
        if (h == g_hPath) { SetTextColor(hdc, t.textTitle); SetBkColor(hdc, t.editBg); return (INT_PTR)g_bEdit; }
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam; HWND h = (HWND)lParam;
        const auto& t = g_themes[g_themeIdx];
        if (h == g_hDryRun) { SetTextColor(hdc, t.accent); SetBkColor(hdc, t.cardBg); return (INT_PTR)g_bCard; }
        SetTextColor(hdc, t.textNormal); SetBkMode(hdc, TRANSPARENT); return (INT_PTR)GetStockObject(NULL_BRUSH);
    }
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam; HWND h = (HWND)lParam;
        const auto& t = g_themes[g_themeIdx];
        if (h == g_hDryRun) { SetTextColor(hdc, t.accent); SetBkColor(hdc, t.cardBg); return (INT_PTR)g_bCard; }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    case WM_CTLCOLORLISTBOX: {
        HDC hdc = (HDC)wParam; const auto& t = g_themes[g_themeIdx];
        SetTextColor(hdc, t.textNormal); SetBkColor(hdc, t.logBg); return (INT_PTR)g_bLog;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam), notif = HIWORD(wParam);
        if (notif == BN_CLICKED) {
            switch (id) {
            case IDC_BROWSE_BTN: {
                std::wstring p = Browse(hWnd);
                if (!p.empty()) { SetWindowTextW(g_hPath, p.c_str()); InvalidateRect(hWnd, NULL, FALSE); }
                break;
            }
            case IDC_VIEW_REPORT_BTN: {
                if (!g_movedFiles.empty()) {
                    WNDCLASSW wc={}; wc.lpfnWndProc=ReportProc; wc.hInstance=GetModuleHandle(NULL);
                    wc.lpszClassName=L"UrFmReport"; wc.hCursor=LoadCursor(NULL,IDC_ARROW);                     wc.hIcon=LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(1));
                    wc.hbrBackground = g_bCard;
                    RegisterClassW(&wc);
                    int d=GetWindowDPI(hWnd); double sc=(double)d/96.0;
                    ReportData rd = { &g_movedFiles, &g_srcDir, g_lastDryRun };
                    HWND hDlg = CreateWindowExW(0, L"UrFmReport", L"Organization Report",
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,CW_USEDEFAULT,(int)(660*sc),(int)(500*sc),
                        hWnd, NULL, GetModuleHandle(NULL), &rd);
                    if (hDlg) ShowWindow(hDlg, SW_SHOW);
                }
                break;
            }
            case IDC_EDIT_CONFIG_BTN: ShellExecuteW(hWnd, L"open", L"config.json", NULL, NULL, SW_SHOW); break;
            case IDC_VIEW_LOG_BTN: ShellExecuteW(hWnd, L"open", L"organizer.log", NULL, NULL, SW_SHOW); break;
            case IDC_ACTION_BTN: {
                if (g_running) {
                    g_cancel = true; SetWindowTextW(g_hAction, L"Cancelling..."); EnableWindow(g_hAction, FALSE); AppendLog(L"Cancellation requested...");
                } else {
                    int len = GetWindowTextLengthW(g_hPath);
                    if (len == 0) { MessageBoxW(hWnd, L"Please select a folder first.", L"Path Required", MB_OK|MB_ICONWARNING); break; }
                    std::wstring ps(len+1,0); GetWindowTextW(g_hPath, ps.data(), len+1); ps.resize(len);
                    if (!std::filesystem::exists(ps) || !std::filesystem::is_directory(ps)) {
                        MessageBoxW(hWnd, L"The path is not a valid directory.", L"Invalid", MB_OK|MB_ICONERROR); break;
                    }
                    bool dry = SendMessage(g_hDryRun, BM_GETCHECK, 0, 0) == BST_CHECKED;
                    g_running = true; g_cancel = false; g_processed = 0; g_total = 0; g_status = L"Scanning...";
                    g_movedFiles.clear(); g_srcDir.clear(); g_lastDryRun = false;
                    g_undoHistory.clear();
                    ShowWindow(g_hReport, SW_HIDE); EnableWindow(g_hUndo, FALSE);
                    EnableWindow(g_hPath, FALSE); EnableWindow(g_hBrowse, FALSE); EnableWindow(g_hDryRun, FALSE);
                    EnableWindow(g_hCfg, FALSE); EnableWindow(g_hLogBtn, FALSE);
                    SetWindowTextW(g_hAction, L"Cancel");
                    OrganizeParams* p = new OrganizeParams{ hWnd, ps, dry, g_fileTypeMap };
                    std::thread([](OrganizeParams* p) {
                        HWND h = p->hwnd; std::wstring src = p->sourceDirectory; bool dry = p->dryRun; auto map = p->fileTypeMap;
                        std::filesystem::path sp(src);
                        AppendLog(L"Starting organization in: " + src);
                        if (dry) AppendLog(L"DRY RUN MODE - no files will be moved.");
                        std::vector<std::filesystem::path> f;
                        try { for (const auto& e : std::filesystem::directory_iterator(sp)) if (e.is_regular_file()) f.push_back(e.path()); }
                        catch (const std::exception& e) { std::string s=e.what(); AppendLog(L"Error: "+std::wstring(s.begin(),s.end())); PostMessage(h,WM_APP_DONE,0,0); delete p; return; }
                        size_t total = f.size(); PostMessage(h, WM_APP_PROGRESS, 0, (LPARAM)total);
                        if (total == 0) { AppendLog(L"No files found."); PostMessage(h, WM_APP_DONE, 0, 0); delete p; return; }
                        std::wofstream log(L"organizer.log", std::ios::app);
                        size_t ok = 0; std::vector<MovedFileInfo> moved; std::vector<UndoRecord> undo;
                        for (const auto& fp : f) {
                            if (g_cancel) { AppendLog(L"Cancelled by user."); break; }
                            std::wstring ext = fp.extension().wstring();
                            for (auto& c : ext) c = std::tolower(c);
                            std::wstring dest = L"Other";
                            for (const auto& [cat, exts] : map) { bool found=false;
                                for (const auto& e : exts) { if (ext == e) { dest=cat; found=true; break; } }
                                if (found) break; }
                            std::filesystem::path dd = sp / dest;
                            std::wstring fn = fp.filename().wstring();
                            uint64_t sz=0; try { sz = std::filesystem::file_size(fp); } catch (...) {}
                            if (dry) {
                                AppendLog(L"  [DRY-RUN] " + fn + L" -> " + dest + L"/");
                                if (log.is_open()) log << L"DRY-RUN: " << fn << L"\n";
                                ok++; moved.push_back({fn, dest, L"Dry Run Preview", sz});
                            } else {
                                try {
                                    std::filesystem::create_directories(dd);
                                    std::filesystem::path df = dd / fn;
                                    int ctr = 1; std::wstring stem = fp.stem().wstring();
                                    while (std::filesystem::exists(df)) { df = dd / (stem + L" (" + std::to_wstring(ctr++) + L")" + ext); }
                                    std::filesystem::rename(fp, df);
                                    AppendLog(L"  Moved: " + fn + L" -> " + dest + L"/");
                                    if (log.is_open()) log << L"MOVED: " << fn << L"\n";
                                    ok++; moved.push_back({fn, dest, ctr>1?L"Renamed":L"Moved", sz});
                                    undo.push_back({fp.wstring(), df.wstring()});
                                } catch (const std::exception& e) {
                                    std::string es=e.what(); std::wstring we(es.begin(),es.end());
                                    AppendLog(L"  ERROR: " + fn + L" - " + we);
                                    moved.push_back({fn, dest, L"Error: "+we, sz});
                                }
                            }
                            g_processed = ok; PostMessage(h, WM_APP_PROGRESS, ok, total);
                        }
                        if (log.is_open()) log.close();
                        g_movedFiles = moved; g_srcDir = src; g_lastDryRun = dry; g_undoHistory = undo;
                        if (!dry && !undo.empty()) {
                            AppendLog(L"Undo history saved. " + std::to_wstring(undo.size()) + L" file(s) can be reverted.");
                        }
                        // Generate PDF report (shared helper)
                        if (!moved.empty()) {
                            std::wstring reportName = dry ? L"organization_report_preview.pdf" : L"organization_report.pdf";
                            std::wstring outPath = (std::filesystem::path(src) / reportName).wstring();
                            try {
                                std::string savePath(outPath.begin(), outPath.end());
                                std::string targetFolder(src.begin(), src.end());
                                std::vector<MovedFileInfoStr> converted;
                                converted.reserve(moved.size());
                                for (const auto& m : moved) {
                                    MovedFileInfoStr rec;
                                    rec.fileName = std::string(m.fileName.begin(), m.fileName.end());
                                    rec.category = std::string(m.category.begin(), m.category.end());
                                    rec.fileSize = m.fileSize;
                                    rec.status = std::string(m.status.begin(), m.status.end());
                                    converted.push_back(rec);
                                }
                                GeneratePDFReportStr(savePath, targetFolder, converted, dry);
                                AppendLog(L"PDF report generated: " + reportName);
                            } catch (const std::exception& e) {
                                std::string es = e.what();
                                AppendLog(L"Failed to generate PDF: " + std::wstring(es.begin(), es.end()));
                            }
                        }
                        AppendLog(L"Done. Processed " + std::to_wstring(ok) + L"/" + std::to_wstring(total) + L" files.");
                        PostMessage(h, WM_APP_DONE, 0, 0); delete p;
                    }, p).detach();
                }
                break;
            }
            case IDC_UNDO_BTN: {
                if (g_undoHistory.empty()) { MessageBoxW(hWnd, L"No undo history available.", L"Undo", MB_OK|MB_ICONINFORMATION); break; }
                if (g_running) { MessageBoxW(hWnd, L"Please wait for the current operation to finish.", L"Busy", MB_OK|MB_ICONWARNING); break; }
                int confirm = MessageBoxW(hWnd,
                    (L"Revert " + std::to_wstring(g_undoHistory.size()) + L" moved file(s) back to their original locations?").c_str(),
                    L"Confirm Undo", MB_YESNO|MB_ICONQUESTION);
                if (confirm != IDYES) break;
                g_running = true; g_cancel = false; g_status = L"Reverting...";
                EnableWindow(g_hAction, FALSE); EnableWindow(g_hUndo, FALSE);
                SetWindowTextW(g_hAction, L"Cancel");
                EnableWindow(g_hPath, FALSE); EnableWindow(g_hBrowse, FALSE); EnableWindow(g_hDryRun, FALSE);
                std::thread([](HWND h) {
                    AppendLog(L"Starting undo - reverting moved files...");
                    size_t ok = 0, err = 0;
                    // Track every category folder a file was reverted from so we
                    // can delete it afterwards if it ends up empty.
                    std::set<std::wstring> categoryFolders;
                    for (const auto& rec : g_undoHistory) {
                        if (g_cancel) { AppendLog(L"Undo cancelled."); break; }
                        // Remember the category folder for later cleanup.
                        categoryFolders.insert(std::filesystem::path(rec.movedPath).parent_path().wstring());
                        try {
                            if (std::filesystem::exists(rec.movedPath)) {
                                std::filesystem::path origDir = std::filesystem::path(rec.originalPath).parent_path();
                                if (!std::filesystem::exists(origDir)) std::filesystem::create_directories(origDir);
                                std::filesystem::rename(rec.movedPath, std::filesystem::path(rec.originalPath));
                                AppendLog(L"  Reverted: " + std::filesystem::path(rec.movedPath).filename().wstring());
                                ok++;
                            } else {
                                AppendLog(L"  Skipped (not found): " + std::filesystem::path(rec.movedPath).filename().wstring());
                            }
                        } catch (const std::exception& e) {
                            std::string es = e.what();
                            std::wstring we(es.begin(), es.end());
                            AppendLog(L"  Undo error: " + std::filesystem::path(rec.movedPath).filename().wstring() + L" - " + we);
                            err++;
                        }
                    }
                    // Cleanup #1: delete every category folder that became empty
                    // after the files were moved back to their original location.
                    for (const auto& cf : categoryFolders) {
                        std::error_code ec;
                        if (std::filesystem::exists(cf) && std::filesystem::is_empty(cf, ec)) {
                            std::filesystem::remove(cf, ec);
                            if (!ec) AppendLog(L"  Removed empty folder: " + std::filesystem::path(cf).filename().wstring());
                        }
                    }
                    // Cleanup #2: delete the PDF report(s) created during organization.
                    for (const auto& rpt : { std::wstring(L"organization_report.pdf"), std::wstring(L"organization_report_preview.pdf") }) {
                        std::filesystem::path p = std::filesystem::path(g_srcDir) / rpt;
                        std::error_code ec;
                        if (std::filesystem::exists(p)) {
                            std::filesystem::remove(p, ec);
                            if (!ec) AppendLog(L"  Deleted report: " + rpt);
                        }
                    }
                    AppendLog(L"Undo complete. Reverted " + std::to_wstring(ok) + L" file(s)." + (err > 0 ? (L" Errors: " + std::to_wstring(err)) : L""));
                    g_undoHistory.clear();
                    PostMessage(h, WM_APP_UNDO_DONE, 0, 0);
                }, hWnd).detach();
                break;
            }
            }
        }
        if (id == IDC_THEME_COMBO && notif == CBN_SELCHANGE) {
            int idx = (int)SendMessage(g_hTheme, CB_GETCURSEL, 0, 0);
            if (idx != CB_ERR && idx != g_themeIdx) {
                g_themeIdx = idx; UpdateBrushes(); InvalidateRect(hWnd, NULL, TRUE);
                InvalidateRect(g_hPath, NULL, TRUE); InvalidateRect(g_hBrowse, NULL, TRUE);
                InvalidateRect(g_hDryRun, NULL, TRUE); InvalidateRect(g_hCfg, NULL, TRUE);
                InvalidateRect(g_hLogBtn, NULL, TRUE); InvalidateRect(g_hAction, NULL, TRUE);
                InvalidateRect(g_hReport, NULL, TRUE); InvalidateRect(g_hLog, NULL, TRUE);
                InvalidateRect(g_hTheme, NULL, TRUE); InvalidateRect(g_hUndo, NULL, TRUE);
            }
        }
        break;
    }

    case WM_APP_LOG: {
        std::wstring* m = (std::wstring*)lParam; if (m) { AppendLog(*m); delete m; } break;
    }
    case WM_APP_PROGRESS: {
        g_processed = (size_t)wParam; g_total = (size_t)lParam;
        RECT r = {SX(35),SY(300),SX(730),SY(320)}; InvalidateRect(hWnd, &r, FALSE); break;
    }
    case WM_APP_STATUS: {
        std::wstring* s = (std::wstring*)lParam; if (s) { g_status=*s; delete s; RECT r={SX(250),SY(250),SX(700),SY(280)}; InvalidateRect(hWnd,&r,FALSE); } break;
    }
    case WM_APP_DONE: {
        g_running = false; g_cancel = false;
        g_status = L"Ready";
        EnableWindow(g_hPath, TRUE); EnableWindow(g_hBrowse, TRUE); EnableWindow(g_hDryRun, TRUE);
        EnableWindow(g_hCfg, TRUE); EnableWindow(g_hLogBtn, TRUE);
        SetWindowTextW(g_hAction, L"Start Organizing"); EnableWindow(g_hAction, TRUE);
        if (!g_undoHistory.empty()) EnableWindow(g_hUndo, TRUE);
        if (!g_movedFiles.empty()) { ShowWindow(g_hReport, SW_SHOW); EnableWindow(g_hReport, TRUE); }
        InvalidateRect(hWnd, NULL, FALSE); break;
    }
    case WM_APP_UNDO_DONE: {
        g_running = false; g_cancel = false;
        g_status = L"Ready";
        EnableWindow(g_hPath, TRUE); EnableWindow(g_hBrowse, TRUE); EnableWindow(g_hDryRun, TRUE);
        EnableWindow(g_hCfg, TRUE); EnableWindow(g_hLogBtn, TRUE);
        SetWindowTextW(g_hAction, L"Start Organizing"); EnableWindow(g_hAction, TRUE);
        EnableWindow(g_hUndo, FALSE);
        InvalidateRect(hWnd, NULL, FALSE); break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc; GetClientRect(hWnd, &rc);
        int w = rc.right-rc.left, h = rc.bottom-rc.top;
        HDC mdc = CreateCompatibleDC(hdc); HBITMAP mb = CreateCompatibleBitmap(hdc, w, h);
        HBITMAP ob = (HBITMAP)SelectObject(mdc, mb);
        const auto& t = g_themes[g_themeIdx];
        FillRect(mdc, &rc, g_bWindow);

        // === Material App Bar ===
        RECT appBar = {0, 0, w, SY(70)};
        HBRUSH hAppBar = CreateSolidBrush(t.accent);
        FillRect(mdc, &appBar, hAppBar); DeleteObject(hAppBar);
        for (int i = 0; i < 4; i++) {
            int alpha = 5 - i;
            RECT sh = {0, SY(70)+i, w, SY(70)+i+1};
            HBRUSH hs = CreateSolidBrush(RGB(0,0,0));
            FillRect(mdc, &sh, hs); DeleteObject(hs);
        }
        SetBkMode(mdc, TRANSPARENT);
        SetTextColor(mdc, RGB(255,255,255)); SelectObject(mdc, g_fTitle);
        TextOutW(mdc, SX(24), SY(16), L"urFM", 4);
        SetTextColor(mdc, RGB(255,255,255)); SelectObject(mdc, g_fSub);
        TextOutW(mdc, SX(102), SY(26), L"urFileManager", 13);
        SetTextColor(mdc, RGB(255,255,255)); SelectObject(mdc, g_fSub);
        TextOutW(mdc, SX(24), SY(48), L"Organize files into categorized folders", 40);

        // === Material Card with shadow ===
        for (int i = 1; i <= 4; i++) {
            RECT shRc = {SX(20)+i, SY(80)+i, SX(745)+i, SY(305)+i};
            HBRUSH hSh = CreateSolidBrush(RGB(
                (GetRValue(t.windowBg)*3)/4, (GetGValue(t.windowBg)*3)/4, (GetBValue(t.windowBg)*3)/4));
            HPEN hShPen = CreatePen(PS_NULL, 0, 0);
            auto obSh = SelectObject(mdc, hSh); auto opSh = SelectObject(mdc, hShPen);
            RoundRect(mdc, shRc.left, shRc.top, shRc.right, shRc.bottom, SX(12), SX(12));
            SelectObject(mdc, obSh); SelectObject(mdc, opSh);
            DeleteObject(hSh); DeleteObject(hShPen);
        }
        RECT cardRc = {SX(20), SY(80), SX(745), SY(305)};
        HBRUSH hCard = CreateSolidBrush(RGB(
            GetRValue(t.cardBg), GetGValue(t.cardBg), GetBValue(t.cardBg)));
        HPEN hPen = CreatePen(PS_SOLID, 1, t.eNorm);
        auto ob2 = SelectObject(mdc, hCard); auto op2 = SelectObject(mdc, hPen);
        RoundRect(mdc, cardRc.left, cardRc.top, cardRc.right, cardRc.bottom, SX(12), SX(12));
        SelectObject(mdc, ob2); SelectObject(mdc, op2); DeleteObject(hPen); DeleteObject(hCard);

        SetTextColor(mdc, t.textNormal); SelectObject(mdc, g_fBold);
        TextOutW(mdc, SX(35), SY(96), L"Target Directory", 16);

        bool eHov = GetPropW(g_hPath, L"hov") != NULL;
        bool eFoc = GetFocus() == g_hPath;
        HPEN hEp = CreatePen(PS_SOLID, eFoc ? 2 : 1, eFoc ? t.eFocus : (eHov ? t.eHover : t.eNorm));
        RECT er = {SX(35), SY(114), SX(575), SY(144)};
        auto op3 = SelectObject(mdc, g_bEdit); auto op4 = SelectObject(mdc, hEp);
        RoundRect(mdc, er.left, er.top, er.right, er.bottom, SX(6), SX(6));
        SelectObject(mdc, op3); SelectObject(mdc, op4); DeleteObject(hEp);
        if (eFoc) {
            HPEN hAc = CreatePen(PS_SOLID, 2, t.accent);
            auto opAc = SelectObject(mdc, hAc);
            MoveToEx(mdc, SX(37), SY(141), NULL); LineTo(mdc, SX(573), SY(141));
            SelectObject(mdc, opAc); DeleteObject(hAc);
        }

        SetTextColor(mdc, t.textNormal); SelectObject(mdc, g_fBold);
        TextOutW(mdc, SX(465), SY(209), L"Theme", 5);

        SetTextColor(mdc, t.textSub); SelectObject(mdc, g_fSub);
        std::wstring st = g_status;
        if (g_total > 0 && !g_running) {
            st = L"Processed " + std::to_wstring(g_processed) + L" / " + std::to_wstring(g_total) + L" files";
        }
        TextOutW(mdc, SX(500), SY(260), st.c_str(), (int)st.length());

        RECT pb = {SX(35), SY(308), SX(730), SY(316)};
        HBRUSH hp = CreateSolidBrush(t.editBg);
        FillRect(mdc, &pb, hp); DeleteObject(hp);
        if (g_total > 0) {
            double pct = (double)g_processed / g_total;
            int fw = (int)((pb.right-pb.left) * pct);
            if (fw > 0) {
                RECT pf = {pb.left, pb.top, pb.left+fw, pb.bottom};
                HBRUSH hf = CreateSolidBrush(t.accent); FillRect(mdc, &pf, hf); DeleteObject(hf);
            }
        }

        SetTextColor(mdc, t.textNormal); SelectObject(mdc, g_fBold);
        TextOutW(mdc, SX(35), SY(332), L"Live Log", 8);

        HPEN hSp = CreatePen(PS_SOLID, 1, t.eNorm);
        auto op5 = SelectObject(mdc, hSp);
        MoveToEx(mdc, SX(35), SY(350), NULL); LineTo(mdc, SX(745), SY(350));
        SelectObject(mdc, op5); DeleteObject(hSp);

        BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, SRCCOPY);
        SelectObject(mdc, ob); DeleteObject(mb); DeleteDC(mdc);
        EndPaint(hWnd, &ps); return 0;
    }

    case WM_DESTROY: {
        DeleteObject(g_fTitle); DeleteObject(g_fSub); DeleteObject(g_fNormal);
        DeleteObject(g_fBold); DeleteObject(g_fLog);
        DeleteObject(g_bWindow); DeleteObject(g_bCard); DeleteObject(g_bEdit); DeleteObject(g_bLog);
        PostQuitMessage(0); return 0;
    }
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ---------------------------------------------------------------------------
//  Custom-drawn button subclass (material style)
// ---------------------------------------------------------------------------
LRESULT CALLBACK BtnSubclass(HWND h, UINT msg, WPARAM w, LPARAM l, UINT_PTR, DWORD_PTR ref) {
    bool prim = (ref == 1);
    bool undo = (ref == 2);
    bool hov = GetPropW(h, L"hov") != NULL;
    bool prs = GetPropW(h, L"prs") != NULL;
    switch (msg) {
    case WM_DESTROY: RemovePropW(h, L"hov"); RemovePropW(h, L"prs"); break;
    case WM_MOUSEMOVE:
        if (!hov) { SetPropW(h, L"hov", (HANDLE)TRUE); TRACKMOUSEEVENT t={sizeof(t),TME_LEAVE,h,0}; TrackMouseEvent(&t); InvalidateRect(h,NULL,FALSE); }
        break;
    case WM_MOUSELEAVE: SetPropW(h, L"hov", (HANDLE)FALSE); InvalidateRect(h,NULL,FALSE); break;
    case WM_LBUTTONDOWN: SetPropW(h, L"prs", (HANDLE)TRUE); SetCapture(h); InvalidateRect(h,NULL,FALSE); break;
    case WM_LBUTTONUP:
        if (prs) { SetPropW(h, L"prs", (HANDLE)FALSE); ReleaseCapture(); InvalidateRect(h,NULL,FALSE); SendMessage(GetParent(h),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(h),BN_CLICKED),(LPARAM)h); }
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);
        int cw = rc.right-rc.left, ch = rc.bottom-rc.top;
        HDC mdc = CreateCompatibleDC(dc); HBITMAP mb = CreateCompatibleBitmap(dc, cw, ch); HBITMAP ob = (HBITMAP)SelectObject(mdc, mb);
        FillRect(mdc, &rc, g_bCard);
        const auto& t = g_themes[g_themeIdx];
        COLORREF bg, bd, tx;
        if (prim) {
            tx = RGB(255,255,255);
            if (!IsWindowEnabled(h)) { bg = t.cardBg; bd = t.eNorm; tx = t.textSub; }
            else if (prs) { bg = t.accentPressed; bd = t.accentPressed; }
            else if (hov) { bg = t.accentHover; bd = t.accentHover; }
            else { bg = t.accent; bd = t.accent; }
        } else if (undo) {
            tx = t.textTitle;
            if (!IsWindowEnabled(h)) { bg = t.cardBg; bd = t.eNorm; tx = t.textSub; }
            else if (prs) { bg = RGB(180,80,80); bd = RGB(180,80,80); tx = RGB(255,255,255); }
            else if (hov) { bg = RGB(140,50,50); bd = RGB(140,50,50); tx = RGB(255,255,255); }
            else { bg = RGB(120,40,40); bd = RGB(160,60,60); tx = RGB(255,200,200); }
        } else {
            tx = t.textNormal;
            if (!IsWindowEnabled(h)) { bg = t.cardBg; bd = t.eNorm; tx = t.textSub; }
            else if (prs) { bg = t.cardBg; bd = t.accent; }
            else if (hov) { bg = t.editBg; bd = t.accentHover; }
            else { bg = t.editBg; bd = t.eNorm; }
        }
        HBRUSH hb = CreateSolidBrush(bg); HPEN hp = CreatePen(PS_SOLID, 1, bd);
        auto ob1 = SelectObject(mdc, hb); auto op1 = SelectObject(mdc, hp);
        RoundRect(mdc, 0, 0, cw, ch, SX(10), SX(10));
        SelectObject(mdc, ob1); SelectObject(mdc, op1); DeleteObject(hb); DeleteObject(hp);
        SetBkMode(mdc, TRANSPARENT); SetTextColor(mdc, tx);
        int len = GetWindowTextLengthW(h); std::wstring txt(len+1,0); GetWindowTextW(h, txt.data(), len+1);
        HFONT f = (HFONT)SendMessage(h, WM_GETFONT, 0, 0); if (!f) f = g_fBold;
        SelectObject(mdc, f);
        DrawTextW(mdc, txt.c_str(), -1, &rc, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
        BitBlt(dc, 0, 0, cw, ch, mdc, 0, 0, SRCCOPY);
        SelectObject(mdc, ob); DeleteObject(mb); DeleteDC(mdc);
        EndPaint(h, &ps); return 0;
    }
    }
    return DefSubclassProc(h, msg, w, l);
}

LRESULT CALLBACK EditSubclass(HWND h, UINT msg, WPARAM w, LPARAM l, UINT_PTR, DWORD_PTR) {
    switch (msg) {
    case WM_DESTROY: RemovePropW(h, L"hov"); break;
    case WM_MOUSEMOVE:
        if (!GetPropW(h, L"hov")) { SetPropW(h, L"hov", (HANDLE)TRUE); TRACKMOUSEEVENT t={sizeof(t),TME_LEAVE,h,0}; TrackMouseEvent(&t); RECT r={SX(35),SY(114),SX(575),SY(144)}; InvalidateRect(GetParent(h),&r,FALSE); }
        break;
    case WM_MOUSELEAVE: {
        SetPropW(h, L"hov", (HANDLE)FALSE); RECT r={SX(35),SY(114),SX(575),SY(144)}; InvalidateRect(GetParent(h),&r,FALSE); break;
    }
    case WM_SETFOCUS:
    case WM_KILLFOCUS: {
        RECT r={SX(35),SY(114),SX(575),SY(144)}; InvalidateRect(GetParent(h),&r,FALSE); break;
    }
    }
    return DefSubclassProc(h, msg, w, l);
}

// ---------------------------------------------------------------------------
//  Report dialog (list view + "Save as PDF")
// ---------------------------------------------------------------------------
LRESULT CALLBACK ReportProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCT* cs = (CREATESTRUCT*)l;
        ReportData* rd = (ReportData*)cs->lpCreateParams;
        int dpi = GetWindowDPI(h); double s = (double)dpi / 96.0;
        const auto& t = g_themes[g_themeIdx];

        std::wstring summary;
        if (rd && rd->files && rd->srcDir) {
            size_t ok=0, err=0, dry=0;
            for (const auto& f : *rd->files) {
                if (f.status == L"Dry Run Preview") dry++;
                else if (f.status.find(L"Error") != std::wstring::npos) err++;
                else ok++;
            }
            summary = L"Source: " + *rd->srcDir;
            summary += L"  |  Total: " + std::to_wstring(rd->files->size());
            summary += L"  |  OK: " + std::to_wstring(ok);
            summary += L"  |  Errors: " + std::to_wstring(err);
            summary += L"  |  Dry-Run: " + std::to_wstring(dry);
        } else {
            summary = L"No report data available.";
        }

        HFONT hTitleFont = CreateFontW(-(int)(20*s),0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Segoe UI");
        HFONT hBodyFont = CreateFontW(-(int)(13*s),0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Segoe UI");
        HFONT hBoldFont = CreateFontW(-(int)(13*s),0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Segoe UI");
        SetPropW(h, L"hTitleFont", (HANDLE)hTitleFont);
        SetPropW(h, L"hBodyFont", (HANDLE)hBodyFont);
        SetPropW(h, L"hBoldFont", (HANDLE)hBoldFont);

        HWND hTitle = CreateWindowExW(0, L"STATIC", L"Organization Report",
            WS_CHILD|WS_VISIBLE|SS_LEFT, (int)(15*s),(int)(10*s),(int)(500*s),(int)(25*s),
            h, NULL, GetModuleHandle(NULL), NULL);
        SendMessage(hTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

        HWND hSummary = CreateWindowExW(0, L"STATIC", summary.c_str(),
            WS_CHILD|WS_VISIBLE|SS_LEFT, (int)(15*s),(int)(38*s),(int)(600*s),(int)(30*s),
            h, NULL, GetModuleHandle(NULL), NULL);
        SendMessage(hSummary, WM_SETFONT, (WPARAM)hBodyFont, TRUE);
        SetPropW(h, L"hSummary", (HANDLE)hSummary);

        HWND hl = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
            WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS,
            (int)(15*s),(int)(72*s),(int)(600*s),(int)(340*s),
            h, (HMENU)IDC_REPORT_LIST, GetModuleHandle(NULL), NULL);
        SendMessage(hl, WM_SETFONT, (WPARAM)hBodyFont, TRUE);

        ListView_SetExtendedListViewStyle(hl, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_DOUBLEBUFFER);

        LVCOLUMNW lv={}; lv.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM|LVCF_FMT;
        lv.cx=(int)(210*s); lv.pszText=(LPWSTR)L"File Name"; lv.fmt=LVCFMT_LEFT;  ListView_InsertColumn(hl,0,&lv);
        lv.cx=(int)(100*s); lv.pszText=(LPWSTR)L"Category"; lv.fmt=LVCFMT_LEFT;  ListView_InsertColumn(hl,1,&lv);
        lv.cx=(int)(80*s);  lv.pszText=(LPWSTR)L"Size";     lv.fmt=LVCFMT_RIGHT; ListView_InsertColumn(hl,2,&lv);
        lv.cx=(int)(120*s); lv.pszText=(LPWSTR)L"Status";   lv.fmt=LVCFMT_LEFT;  ListView_InsertColumn(hl,3,&lv);

        if (rd && rd->files) {
            for (size_t i = 0; i < rd->files->size(); ++i) {
                const auto& fi = (*rd->files)[i];
                LVITEMW li={}; li.mask=LVIF_TEXT|LVIF_PARAM; li.iItem=(int)i; li.iSubItem=0; li.lParam=(LPARAM)i;
                li.pszText=const_cast<LPWSTR>(fi.fileName.c_str());
                ListView_InsertItem(hl, &li);
                ListView_SetItemText(hl,(int)i,1,const_cast<LPWSTR>(fi.category.c_str()));
                std::wstring sz = FormatSizeW(fi.fileSize);
                ListView_SetItemText(hl,(int)i,2,const_cast<LPWSTR>(sz.c_str()));
                ListView_SetItemText(hl,(int)i,3,const_cast<LPWSTR>(fi.status.c_str()));
            }
        }

        SetPropW(h, L"files", (HANDLE)rd->files);
        SetPropW(h, L"src", (HANDLE)rd->srcDir);
        SetPropW(h, L"dryRun", (HANDLE)(INT_PTR)(rd->dryRun ? 1 : 0));

        HWND hSave = CreateWindowExW(0, L"BUTTON", L"Save as PDF...",
            WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, (int)(15*s),(int)(425*s),(int)(130*s),(int)(30*s),
            h, (HMENU)IDC_SAVE_PDF_BTN, GetModuleHandle(NULL), NULL);
        SendMessage(hSave, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
        SetWindowSubclass(hSave, BtnSubclass, IDC_SAVE_PDF_BTN, 1);

        HWND hClose = CreateWindowExW(0, L"BUTTON", L"Close",
            WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, (int)(155*s),(int)(425*s),(int)(100*s),(int)(30*s),
            h, (HMENU)IDC_CLOSE_REPORT_BTN, GetModuleHandle(NULL), NULL);
        SendMessage(hClose, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
        SetWindowSubclass(hClose, BtnSubclass, IDC_CLOSE_REPORT_BTN, 0);

        break;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)w; HWND hCtrl = (HWND)l;
        const auto& t = g_themes[g_themeIdx];
        SetTextColor(hdc, t.textNormal);
        SetBkColor(hdc, t.cardBg);
        return (INT_PTR)g_bCard;
    }
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)w; HWND hCtrl = (HWND)l;
        const auto& t = g_themes[g_themeIdx];
        SetTextColor(hdc, t.textNormal);
        SetBkColor(hdc, t.cardBg);
        return (INT_PTR)g_bCard;
    }
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)w;
        const auto& t = g_themes[g_themeIdx];
        SetTextColor(hdc, t.textNormal);
        SetBkColor(hdc, t.logBg);
        return (INT_PTR)g_bLog;
    }

    case WM_COMMAND: {
        int id = LOWORD(w);
        if (id == IDC_CLOSE_REPORT_BTN) {
            DestroyWindow(h);
        } else if (id == IDC_SAVE_PDF_BTN) {
            auto* f = (std::vector<MovedFileInfo>*)GetPropW(h, L"files");
            auto* sd = (std::wstring*)GetPropW(h, L"src");
            bool dry = (INT_PTR)GetPropW(h, L"dryRun") == 1;
            if (f && sd && !f->empty()) {
                wchar_t p[MAX_PATH]={}; OPENFILENAMEW ofn={}; ofn.lStructSize=sizeof(ofn); ofn.hwndOwner=h;
                ofn.lpstrFilter=L"PDF Files\0*.pdf\0All Files\0*.*\0"; ofn.lpstrFile=p; ofn.nMaxFile=MAX_PATH;
                ofn.lpstrDefExt=L"pdf"; ofn.Flags=OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;
                std::wstring defName = dry ? L"organization_report_preview.pdf" : L"organization_report.pdf";
                wcscpy_s(p, defName.c_str());
                if (GetSaveFileNameW(&ofn)) {
                    std::string savePath(p, p + wcslen(p));
                    std::string targetFolder((*sd).begin(), (*sd).end());
                    std::vector<MovedFileInfoStr> converted;
                    converted.reserve(f->size());
                    for (const auto& m : *f) {
                        MovedFileInfoStr rec;
                        rec.fileName = std::string(m.fileName.begin(), m.fileName.end());
                        rec.category = std::string(m.category.begin(), m.category.end());
                        rec.fileSize = m.fileSize;
                        rec.status = std::string(m.status.begin(), m.status.end());
                        converted.push_back(rec);
                    }
                    GeneratePDFReportStr(savePath, targetFolder, converted, dry);
                    MessageBoxW(h, L"PDF report saved successfully.", L"Export Complete", MB_OK|MB_ICONINFORMATION);
                }
            }
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);
        int w = rc.right-rc.left, ch = rc.bottom-rc.top;
        const auto& t = g_themes[g_themeIdx];
        int dpi = GetWindowDPI(h); double sc = (double)dpi / 96.0;

        HDC mdc = CreateCompatibleDC(hdc); HBITMAP mb = CreateCompatibleBitmap(hdc, w, ch); HBITMAP ob = (HBITMAP)SelectObject(mdc, mb);
        FillRect(mdc, &rc, g_bCard);

        RECT accentLine = {0, 0, w, (int)(3*sc)};
        HBRUSH hAccent = CreateSolidBrush(t.accent);
        FillRect(mdc, &accentLine, hAccent); DeleteObject(hAccent);

        BitBlt(hdc, 0, 0, w, ch, mdc, 0, 0, SRCCOPY);
        SelectObject(mdc, ob); DeleteObject(mb); DeleteDC(mdc);
        EndPaint(h, &ps); return 0;
    }

    case WM_CLOSE: DestroyWindow(h); break;

    case WM_DESTROY: {
        HFONT hf1=(HFONT)RemovePropW(h,L"hTitleFont");
        HFONT hf2=(HFONT)RemovePropW(h,L"hBodyFont");
        HFONT hf3=(HFONT)RemovePropW(h,L"hBoldFont");
        if(hf1)DeleteObject(hf1); if(hf2)DeleteObject(hf2); if(hf3)DeleteObject(hf3);
        RemovePropW(h,L"files"); RemovePropW(h,L"src"); RemovePropW(h,L"dryRun"); RemovePropW(h,L"hSummary");
        break;
    }
    }
    return DefWindowProcW(h, msg, w, l);
}
