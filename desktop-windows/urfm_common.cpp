// urfm_common.cpp
// Shared implementations used by both the GUI (gui_app.cpp) and the CLI
// (cli.cpp): config loading, the PDF report generator, and the revert helper.
#include "urfm_common.h"

std::wstring ReadFile(const std::wstring& path) {
    std::ifstream fs(std::filesystem::path(path), std::ios::binary);
    if (!fs.is_open()) return L"";
    std::string s((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), NULL, 0);
    std::wstring w(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}

std::map<std::wstring, std::vector<std::wstring>> ParseCfg(const std::wstring& s) {
    std::map<std::wstring, std::vector<std::wstring>> c;
    size_t i = 0, l = s.length();
    auto sk = [&]() { while (i < l && (s[i] == L' ' || s[i] == L'\t' || s[i] == L'\r' || s[i] == L'\n')) i++; };
    auto ps = [&]() -> std::wstring { std::wstring r; if (i < l && s[i] == L'"') { i++; while (i < l && s[i] != L'"') { if (s[i] == L'\\' && i + 1 < l) { i++; r += s[i]; } else r += s[i]; i++; } if (i < l) i++; } return r; };
    sk(); if (i < l && s[i] == L'{') { i++; while (i < l) { sk(); if (s[i] == L'}') break;
        if (s[i] == L'"') { std::wstring cat = ps(); sk(); if (i < l && s[i] == L':') { i++; sk(); if (i < l && s[i] == L'[') { i++; std::vector<std::wstring> e;
            while (i < l) { sk(); if (s[i] == L']') { i++; break; } if (s[i] == L'"') { std::wstring x = ps(); for (auto& c : x) c = std::tolower(c); e.push_back(x); }
            else if (s[i] == L',') i++; else i++; } c[cat] = e; } } } else if (s[i] == L',') i++; else i++; }
    }
    return c;
}

// ---------------------------------------------------------------------------
//  PDF report generation (shared)
// ---------------------------------------------------------------------------
std::string EscapePDFStr(const std::string& text) {
    std::string s;
    for (char c : text) {
        if ((unsigned char)c < 128) {
            if (c == '(' || c == ')' || c == '\\') s += '\\';
            s += c;
        } else s += '?';
    }
    return s;
}
std::string TruncStr(const std::string& t, size_t m) { return t.length() <= m ? t : t.substr(0, m - 3) + "..."; }
std::string FmtSizeStr(uint64_t b) {
    double sz = (double)b; const char* u = "B";
    if (sz >= 1024) { sz /= 1024; u = "KB"; } if (sz >= 1024) { sz /= 1024; u = "MB"; } if (sz >= 1024) { sz /= 1024; u = "GB"; }
    char buf[32]; snprintf(buf, sizeof(buf), "%.2f %s", sz, u); return buf;
}

void GeneratePDFReportStr(const std::string& outputPath, const std::string& targetFolder,
                          const std::vector<MovedFileInfoStr>& movedFiles, bool dryRun) {
    if (movedFiles.empty()) return;

    std::vector<MovedFileInfoStr> sortedFiles = movedFiles;
    std::sort(sortedFiles.begin(), sortedFiles.end(), [](const MovedFileInfoStr& a, const MovedFileInfoStr& b) {
        if (a.category != b.category) return a.category < b.category;
        return a.fileName < b.fileName;
    });

    uint64_t totalSize = 0;
    for (const auto& f : sortedFiles) if (f.status.find("Error") == std::string::npos) totalSize += f.fileSize;
    std::string totalSizeStr = FmtSizeStr(totalSize);
    const size_t rpf = 25, rps = 32;
    size_t total = sortedFiles.size(), pc = 1;
    if (total > rpf) pc = 1 + (total - rpf + rps - 1) / rps;

    auto now = std::chrono::system_clock::now();
    auto in_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &in_t);
#else
    localtime_r(&in_t, &tm);
#endif
    char tb[64]; strftime(tb, sizeof(tb), "%Y-%m-%d %H:%M:%S", &tm);

    std::vector<std::string> contents;
    size_t fi = 0;
    for (size_t p = 0; p < pc; ++p) {
        std::stringstream ss;
        ss << "0.9 0.9 0.9 RG 0.5 w 30 30 535 782 re S\n";
        size_t sy;
        if (p == 0) {
            ss << "0.95 0.95 0.98 rg 40 700 515 100 re f\n0.85 0.85 0.9 RG 1 w 40 700 515 100 re S\n";
            ss << "BT /F2 18 Tf 0.1 0.1 0.25 rg 55 765 Td (" << (dryRun ? "urFileManager - DRY RUN REPORT" : "urFileManager - File Transfer Report") << ") Tj ET\n";
            ss << "BT /F1 9 Tf 0.35 0.35 0.45 rg 55 745 Td (Report generated: " << EscapePDFStr(tb) << ") Tj ET\n";
            ss << "BT /F1 9 Tf 0.35 0.35 0.45 rg 55 730 Td (Target: " << EscapePDFStr(targetFolder) << ") Tj ET\n";
            ss << "BT /F2 10 Tf 0.15 0.5 0.15 rg 55 712 Td (Status: " << (dryRun ? "PREVIEW" : "COMPLETED") << "  |  Total: " << total << "  |  Size: " << EscapePDFStr(totalSizeStr) << ") Tj ET\n";
            ss << "0.9 0.9 0.92 rg 40 655 515 22 re f\n0.7 0.7 0.75 RG 1 w 40 655 515 22 re S\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 48 662 Td (File Name) Tj ET\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 268 662 Td (Category) Tj ET\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 368 662 Td (Size) Tj ET\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 458 662 Td (Status) Tj ET\n";
            sy = 655;
        } else {
            ss << "0.9 0.9 0.92 rg 40 770 515 22 re f\n0.7 0.7 0.75 RG 1 w 40 770 515 22 re S\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 48 777 Td (File Name) Tj ET\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 268 777 Td (Category) Tj ET\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 368 777 Td (Size) Tj ET\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 458 777 Td (Status) Tj ET\n";
            sy = 770;
        }
        size_t items = (p == 0) ? rpf : rps, drawn = 0;
        for (size_t r = 0; r < items && fi < total; ++r, ++fi) {
            const auto& f = sortedFiles[fi]; size_t ry = sy - 20 - (r * 20); drawn++;
            if (r % 2 == 1) ss << "0.97 0.97 0.99 rg 40 " << ry << " 515 20 re f\n";
            ss << "0.9 0.9 0.9 RG 0.5 w 40 " << ry << " m 555 " << ry << " l S\n";
            size_t ty = ry + 6;
            ss << "BT /F1 9 Tf 0.15 0.15 0.15 rg 48 " << ty << " Td (" << EscapePDFStr(TruncStr(f.fileName, 38)) << ") Tj ET\n";
            ss << "BT /F1 9 Tf 0.15 0.15 0.15 rg 268 " << ty << " Td (" << EscapePDFStr(TruncStr(f.category, 15)) << ") Tj ET\n";
            ss << "BT /F1 9 Tf 0.15 0.15 0.15 rg 368 " << ty << " Td (" << EscapePDFStr(FmtSizeStr(f.fileSize)) << ") Tj ET\n";
            if (f.status == "Moved") ss << "BT /F2 9 Tf 0.1 0.5 0.1 rg 458 " << ty << " Td (Moved) Tj ET\n";
            else if (f.status.find("Renamed") != std::string::npos) ss << "BT /F2 9 Tf 0.8 0.4 0.0 rg 458 " << ty << " Td (Renamed) Tj ET\n";
            else if (f.status.find("Error") != std::string::npos) ss << "BT /F2 9 Tf 0.8 0.1 0.1 rg 458 " << ty << " Td (Error) Tj ET\n";
            else ss << "BT /F1 9 Tf 0.2 0.2 0.7 rg 458 " << ty << " Td (" << EscapePDFStr(f.status) << ") Tj ET\n";
        }
        size_t fy = sy - 20 - (drawn * 20);
        ss << "0.85 0.85 0.85 RG 0.5 w 40 " << fy << " m 40 " << sy << " l S\n";
        ss << "0.85 0.85 0.85 RG 0.5 w 260 " << fy << " m 260 " << sy << " l S\n";
        ss << "0.85 0.85 0.85 RG 0.5 w 360 " << fy << " m 360 " << sy << " l S\n";
        ss << "0.85 0.85 0.85 RG 0.5 w 450 " << fy << " m 450 " << sy << " l S\n";
        ss << "0.85 0.85 0.85 RG 0.5 w 555 " << fy << " m 555 " << sy << " l S\n";
        ss << "0.7 0.7 0.75 RG 1 w 40 " << fy << " m 555 " << fy << " l S\n";
        ss << "BT /F1 8 Tf 0.5 0.5 0.5 rg 270 45 Td (Page " << (p + 1) << " of " << pc << ") Tj ET\n";
        contents.push_back(ss.str());
    }
    std::vector<std::string> objs;
    objs.push_back("<< /Type /Catalog /Pages 2 0 R >>");
    std::string kids; for (size_t p = 0; p < pc; ++p) kids += std::to_string(5 + p) + " 0 R ";
    objs.push_back("<< /Type /Pages /Kids [" + kids + "] /Count " + std::to_string(pc) + " >>");
    objs.push_back("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>");
    objs.push_back("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold >>");
    for (size_t p = 0; p < pc; ++p) objs.push_back("<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Resources << /Font << /F1 3 0 R /F2 4 0 R >> >> /Contents " + std::to_string(5 + pc + p) + " 0 R >>");
    for (size_t p = 0; p < pc; ++p) { const auto& c = contents[p]; objs.push_back("<< /Length " + std::to_string(c.length()) + " >>\nstream\n" + c + "\nendstream"); }
    std::ofstream out(outputPath, std::ios::binary);
    if (!out.is_open()) return;
    out << "%PDF-1.4\n"; std::vector<size_t> off; size_t o = 9;
    for (size_t i = 0; i < objs.size(); ++i) { off.push_back(o); std::string s = std::to_string(i + 1) + " 0 obj\n" + objs[i] + "\nendobj\n"; out.write(s.c_str(), s.length()); o += s.length(); }
    size_t xref = o; out << "xref\n0 " << (objs.size() + 1) << "\n";
    char bf[64]; snprintf(bf, sizeof(bf), "%010d 65535 f\r\n", 0); out.write(bf, strlen(bf));
    for (size_t ox : off) { snprintf(bf, sizeof(bf), "%010zu 00000 n\r\n", ox); out.write(bf, strlen(bf)); }
    out << "trailer\n<< /Size " << (objs.size() + 1) << " /Root 1 0 R >>\nstartxref\n" << xref << "\n%%EOF\n";
}

// ---------------------------------------------------------------------------
//  RevertFolder - shared between the GUI undo and the CLI --revert command
// ---------------------------------------------------------------------------
void RevertFolder(const std::filesystem::path& src) {
    std::wcout << L"Reverting organization in: " << src.wstring() << L"\n";
    size_t movedBack = 0, removedFolders = 0, removedReports = 0;
    std::error_code ec;
    for (auto it = std::filesystem::directory_iterator(src, ec); it != std::filesystem::directory_iterator(); ++it) {
        if (!it->is_directory()) continue;
        std::filesystem::path sub = it->path();
        for (auto fit = std::filesystem::directory_iterator(sub, ec); fit != std::filesystem::directory_iterator(); ++fit) {
            if (!fit->is_regular_file()) continue;
            std::wstring fn = fit->path().filename().wstring();
            std::filesystem::path dest = src / fn;
            int ctr = 1;
            std::wstring stem = fit->path().stem().wstring();
            std::wstring ext = fit->path().extension().wstring();
            // Avoid overwriting an existing file at the destination.
            while (std::filesystem::exists(dest)) { dest = src / (stem + L" (" + std::to_wstring(ctr++) + L")" + ext); }
            try { std::filesystem::rename(fit->path(), dest); movedBack++; }
            catch (const std::exception&) {}
        }
        // Remove the category folder if it is now empty.
        if (std::filesystem::is_empty(sub, ec)) {
            std::filesystem::remove(sub, ec);
            if (!ec) removedFolders++;
        }
    }
    // Delete the PDF report(s) created during organization.
    for (const auto& name : { std::wstring(L"organization_report.pdf"), std::wstring(L"organization_report_preview.pdf") }) {
        std::filesystem::path p = src / name;
        if (std::filesystem::exists(p)) {
            std::filesystem::remove(p, ec);
            if (!ec) removedReports++;
        }
    }
    std::wcout << L"Done. Moved back " << movedBack << L" file(s), removed "
               << removedFolders << L" empty folder(s), deleted "
               << removedReports << L" report(s).\n";
}
