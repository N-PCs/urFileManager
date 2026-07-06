// gui_fltk.cpp - Cross-platform GUI for urFileManager (Linux)
// Build: g++ -std=c++17 -O2 gui_fltk.cpp core.cpp -o urfm $(fltk-config --cxxflags --ldflags) -lpthread
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Progress.H>
#include <FL/fl_ask.H>
#include <filesystem>
#include <fstream>
#include <thread>
#include <atomic>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>

#include "core.h"

// Global state
Fl_Double_Window *g_win = nullptr;
Fl_Input *g_pathInput = nullptr;
Fl_Button *g_browseBtn = nullptr;
Fl_Check_Button *g_dryRunCheck = nullptr;
Fl_Button *g_actionBtn = nullptr;
Fl_Choice *g_themeChoice = nullptr;
Fl_Text_Display *g_logDisplay = nullptr;
Fl_Text_Buffer *g_logBuf = nullptr;
Fl_Progress *g_progress = nullptr;
Fl_Box *g_statusBox = nullptr;

std::atomic<bool> g_running(false);
std::atomic<bool> g_cancel(false);
std::map<std::string, std::vector<std::string>> g_fileTypeMap;
std::vector<MovedFileInfo> g_movedFiles;
std::string g_srcDir;
bool g_lastDryRun = false;

void applyTheme(int idx) {
    if (idx < 0 || idx >= (int)g_themes.size()) idx = 0;
    const auto& t = g_themes[idx];
    Fl::background((t.windowBg>>16)&0xFF, (t.windowBg>>8)&0xFF, t.windowBg&0xFF);
    Fl::foreground((t.textTitle>>16)&0xFF, (t.textTitle>>8)&0xFF, t.textTitle&0xFF);
    if (g_win) g_win->redraw();
}

void appendLog(const std::string& msg) {
    if (g_logBuf) {
        std::string line = msg + "\n";
        g_logBuf->append(line.c_str());
        if (g_logDisplay) {
            g_logDisplay->scroll(g_logBuf->count_lines(0, g_logBuf->length()), 0);
        }
    }
}

static void browse_cb(Fl_Widget*, void*) {
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Select a folder to organize");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    if (fnfc.show() == 0) {
        g_pathInput->value(fnfc.filename());
    }
}

static void action_cb(Fl_Widget*, void*) {
    if (g_running) {
        g_cancel = true;
        g_actionBtn->label("Cancelling...");
        g_actionBtn->deactivate();
        appendLog("[INFO] Cancellation requested...");
        return;
    }

    const char* path = g_pathInput->value();
    if (!path || strlen(path) == 0) {
        fl_alert("Please select a folder first.");
        return;
    }
    std::filesystem::path sp(path);
    if (!std::filesystem::exists(sp) || !std::filesystem::is_directory(sp)) {
        fl_alert("The selected path is not a valid directory.");
        return;
    }

    bool dryRun = (g_dryRunCheck->value() == 1);
    g_running = true;
    g_cancel = false;
    g_movedFiles.clear();
    g_srcDir = path;
    g_progress->value(0);
    g_actionBtn->label("Cancel");

    // Disable inputs during processing
    g_pathInput->deactivate();
    g_browseBtn->deactivate();
    g_dryRunCheck->deactivate();

    std::thread([dryRun, path]() {
        std::filesystem::path sp(path);
        appendLog("[INFO] Starting organization in: " + std::string(path));
        if (dryRun) appendLog("[INFO] DRY RUN MODE — no files will be moved.");

        std::vector<std::filesystem::path> files;
        try {
            for (const auto& e : std::filesystem::directory_iterator(sp))
                if (e.is_regular_file()) files.push_back(e.path());
        } catch (const std::exception& e) {
            appendLog("[ERROR] " + std::string(e.what()));
            g_running = false;
            Fl::awake([](){ if(g_actionBtn){g_actionBtn->label("Start Organizing");g_actionBtn->activate();}
                if(g_pathInput)g_pathInput->activate();if(g_browseBtn)g_browseBtn->activate();
                if(g_dryRunCheck)g_dryRunCheck->activate(); });
            return;
        }

        size_t total = files.size();
        if (total == 0) {
            appendLog("[INFO] No files found to organize.");
            g_running = false;
            Fl::awake([](){ if(g_actionBtn){g_actionBtn->label("Start Organizing");g_actionBtn->activate();}
                if(g_pathInput)g_pathInput->activate();if(g_browseBtn)g_browseBtn->activate();
                if(g_dryRunCheck)g_dryRunCheck->activate(); });
            return;
        }

        std::ofstream log("organizer.log", std::ios::app);
        size_t processed = 0;
        size_t ok = 0;
        std::vector<MovedFileInfo> moved;

        for (const auto& fp : files) {
            if (g_cancel) { appendLog("[INFO] Cancelled by user."); break; }

            std::string ext = fp.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            std::string dest = "Other";
            for (const auto& [cat, exts] : g_fileTypeMap) {
                if (std::find(exts.begin(), exts.end(), ext) != exts.end()) { dest = cat; break; }
            }

            std::filesystem::path dd = sp / dest;
            std::string fn = fp.filename().string();
            uint64_t sz = 0;
            try { sz = std::filesystem::file_size(fp); } catch (...) {}

            if (dryRun) {
                appendLog("  [DRY-RUN] " + fn + " -> " + dest + "/");
                if (log.is_open()) log << "DRY-RUN: " << fn << "\n";
                ok++;
                moved.push_back({fn, dest, sz, "Dry Run Preview"});
            } else {
                try {
                    std::filesystem::create_directories(dd);
                    std::filesystem::path df = dd / fn;
                    int ctr = 1;
                    std::string stem = fp.stem().string();
                    while (std::filesystem::exists(df)) {
                        df = dd / (stem + " (" + std::to_string(ctr++) + ")" + ext);
                    }
                    std::filesystem::rename(fp, df);
                    appendLog("  Moved: " + fn + " -> " + dest + "/");
                    if (log.is_open()) log << "MOVED: " << fn << "\n";
                    ok++;
                    moved.push_back({fn, dest, sz, ctr > 1 ? "Renamed" : "Moved"});
                } catch (const std::exception& e) {
                    appendLog("  ERROR: " + fn + " - " + e.what());
                    moved.push_back({fn, dest, sz, "Error: " + std::string(e.what())});
                }
            }

            processed++;
            double pct = (double)processed / total;
            Fl::awake([pct](){ if(g_progress) g_progress->value(pct); });
        }

        if (log.is_open()) log.close();
        g_movedFiles = moved;
        g_srcDir = path;
        g_lastDryRun = dryRun;

        // Generate PDF report
        if (!moved.empty()) {
            std::string reportName = dryRun ? "organization_report_preview.pdf" : "organization_report.pdf";
            std::string outPath = (sp / reportName).string();
            try {
                GeneratePDFReport(outPath, path, moved, dryRun);
                appendLog("[INFO] PDF report generated: " + reportName);
            } catch (const std::exception& e) {
                appendLog("[ERROR] Failed to generate PDF: " + std::string(e.what()));
            }
        }

        appendLog("[DONE] Processed " + std::to_string(ok) + "/" + std::to_string(total) + " files.");

        Fl::awake([](){
            g_running = false;
            if(g_actionBtn){g_actionBtn->label("Start Organizing");g_actionBtn->activate();}
            if(g_pathInput)g_pathInput->activate();
            if(g_browseBtn)g_browseBtn->activate();
            if(g_dryRunCheck)g_dryRunCheck->activate();
        });
    }).detach();
}

static void theme_cb(Fl_Widget*, void*) {
    if (g_themeChoice) {
        int idx = g_themeChoice->value();
        g_currentThemeIdx = idx;
        applyTheme(idx);
    }
}

int main(int argc, char** argv) {
    // CLI mode check
    if (argc > 1) {
        std::string dir;
        bool dry = false;
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--dry-run") dry = true;
            else if (arg == "--no-dry-run") dry = false;
            else if (arg[0] != '-') dir = arg;
        }

        if (dir.empty()) {
            std::cout << "urFileManager CLI Organizer\n";
            std::cout << "Usage: urfm <directory> [--dry-run]\n";
            return 1;
        }

        std::filesystem::path sp(dir);
        if (!std::filesystem::exists(sp) || !std::filesystem::is_directory(sp)) {
            std::cerr << "Invalid directory: " << dir << "\n";
            return 1;
        }

        std::string configPath = std::filesystem::path(argv[0]).parent_path().string() + "/config.json";
        std::string json = ReadFileToString(configPath);
        auto cfg = ParseConfig(json);
        if (cfg.empty()) { std::cerr << "Failed to load config.json\n"; return 1; }

        std::vector<std::filesystem::path> files;
        for (const auto& e : std::filesystem::directory_iterator(sp))
            if (e.is_regular_file()) files.push_back(e.path());

        size_t total = files.size();
        std::cout << "Found " << total << " files.\n";
        size_t ok = 0;
        std::ofstream log("organizer.log", std::ios::app);
        std::vector<MovedFileInfo> moved;

        for (const auto& fp : files) {
            std::string ext = fp.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            std::string dest = "Other";
            for (const auto& [cat, exts] : cfg)
                if (std::find(exts.begin(), exts.end(), ext) != exts.end()) { dest = cat; break; }

            std::filesystem::path dd = sp / dest;
            std::string fn = fp.filename().string();
            uint64_t sz = 0;
            try { sz = std::filesystem::file_size(fp); } catch (...) {}

            if (dry) {
                std::cout << "[DRY-RUN] " << fn << " -> " << dest << "/\n";
                if (log.is_open()) log << "DRY-RUN: " << fn << "\n";
                ok++;
                moved.push_back({fn, dest, sz, "Dry Run Preview"});
            } else {
                try {
                    std::filesystem::create_directories(dd);
                    std::filesystem::path df = dd / fn;
                    int ctr = 1;
                    std::string stem = fp.stem().string();
                    while (std::filesystem::exists(df))
                        df = dd / (stem + " (" + std::to_string(ctr++) + ")" + ext);
                    std::filesystem::rename(fp, df);
                    std::cout << "Moved: " << fn << "\n";
                    if (log.is_open()) log << "MOVED: " << fn << "\n";
                    ok++;
                    moved.push_back({fn, dest, sz, ctr > 1 ? "Renamed" : "Moved"});
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << fn << " - " << e.what() << "\n";
                    moved.push_back({fn, dest, sz, "Error: " + std::string(e.what())});
                }
            }
            std::cout << "\rProgress: " << ok << "/" << total << std::flush;
        }
        if (log.is_open()) log.close();
        std::cout << "\nDone. Processed " << ok << "/" << total << " files.\n";

        if (!moved.empty()) {
            std::string rpt = dry ? "organization_report_preview.pdf" : "organization_report.pdf";
            GeneratePDFReport((sp / rpt).string(), dir, moved, dry);
            std::cout << "PDF report: " << rpt << "\n";
        }
        return 0;
    }

    // GUI mode
    Fl::scheme("gtk+");

    // Load config
    std::string cfgPath = std::filesystem::path(argv[0]).parent_path().string() + "/config.json";
    std::string json = ReadFileToString(cfgPath);
    g_fileTypeMap = ParseConfig(json);

    g_win = new Fl_Double_Window(740, 560, "urFileManager");
    g_win->box(FL_FLAT_BOX);

    // Header
    Fl_Box *title = new Fl_Box(FL_NO_BOX, 20, 15, 300, 30, "urFM");
    title->labelfont(FL_HELVETICA_BOLD);
    title->labelsize(26);
    Fl_Box *subtitle = new Fl_Box(FL_NO_BOX, 90, 22, 400, 20, "urFileManager");
    subtitle->labelsize(13);
    subtitle->labelcolor(fl_lighter(FL_FOREGROUND_COLOR));
    Fl_Box *desc = new Fl_Box(FL_NO_BOX, 20, 48, 600, 20,
        "Organize loose files into categorized folders in seconds.");
    desc->labelsize(12);
    desc->labelcolor(fl_lighter(FL_FOREGROUND_COLOR));

    // Separator
    Fl_Box *sep = new Fl_Box(FL_NO_BOX, 0, 75, 740, 2, "");
    sep->box(FL_FLAT_BOX);
    sep->color(fl_lighter(FL_FOREGROUND_COLOR));

    // Target directory
    Fl_Box *dirLabel = new Fl_Box(FL_NO_BOX, 35, 95, 200, 20, "Target Directory");
    dirLabel->labelfont(FL_HELVETICA_BOLD);
    dirLabel->labelsize(14);

    g_pathInput = new Fl_Input(35, 118, 530, 28);
    g_pathInput->tooltip("Path to the directory you want to organize");

    g_browseBtn = new Fl_Button(575, 118, 110, 28, "Browse");
    g_browseBtn->callback(browse_cb);

    // Dry-run checkbox
    g_dryRunCheck = new Fl_Check_Button(35, 158, 660, 22, " Dry Run (Preview only - uncheck to execute)");
    g_dryRunCheck->value(1);
    g_dryRunCheck->labelsize(13);

    // Action buttons row
    g_actionBtn = new Fl_Button(35, 200, 200, 36, "Start Organizing");
    g_actionBtn->callback(action_cb);
    g_actionBtn->box(FL_UP_BOX);
    g_actionBtn->labelfont(FL_HELVETICA_BOLD);
    g_actionBtn->labelsize(15);

    // Theme selector
    Fl_Box *themeLabel = new Fl_Box(FL_NO_BOX, 470, 203, 60, 20, "Theme:");
    themeLabel->labelsize(13);
    g_themeChoice = new Fl_Choice(520, 200, 190, 26);
    for (const auto& t : g_themes) g_themeChoice->add(t.name.c_str());
    g_themeChoice->value(0);
    g_themeChoice->callback(theme_cb);

    // Status
    g_statusBox = new Fl_Box(FL_NO_BOX, 250, 245, 400, 20, "Ready");
    g_statusBox->labelsize(12);

    // Progress bar
    g_progress = new Fl_Progress(35, 270, 670, 12);
    g_progress->minimum(0);
    g_progress->maximum(1);
    g_progress->value(0);
    g_progress->selection_color(FL_BLUE);

    // Log label
    Fl_Box *logLabel = new Fl_Box(FL_NO_BOX, 35, 295, 200, 16, "Live Log");
    logLabel->labelfont(FL_HELVETICA_BOLD);
    logLabel->labelsize(13);

    // Log display
    g_logBuf = new Fl_Text_Buffer();
    g_logDisplay = new Fl_Text_Display(35, 315, 670, 210);
    g_logDisplay->buffer(g_logBuf);
    g_logDisplay->textfont(FL_COURIER);
    g_logDisplay->textsize(12);

    g_win->end();
    g_win->resizable(g_logDisplay);
    g_win->show(argc, argv);

    // Initial log
    if (g_fileTypeMap.empty()) {
        appendLog("[ERROR] config.json not found. Place it next to the executable.");
    } else {
        appendLog("urFileManager v1.0");
        appendLog("Configuration loaded from config.json");
        appendLog("Select a folder and click Start Organizing.");
        appendLog("Dry-run mode is ON by default for safety.");
    }

    return Fl::run();
}
