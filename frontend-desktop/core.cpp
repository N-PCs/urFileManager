#include "core.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <cstdio>

const std::vector<Theme> g_themes = {
    { "Midnight Dark",     0x0f0f11, 0x18181b, 0x27272a, 0x18181b, 0xffffff, 0xa1a1aa, 0xe4e4e7, 0x6366f1, 0x4f46e5, 0x4338ca },
    { "Minimalist Light",  0xf4f4f5, 0xffffff, 0xf4f4f5, 0xfafafa, 0x09090b, 0x71717a, 0x27272a, 0x4f46e5, 0x4338ca, 0x3730a3 },
    { "Nordic Frost",      0x2e3440, 0x3b4252, 0x4c566a, 0x3b4252, 0xeceff4, 0xd8dee9, 0xe5e9f0, 0x88c0d0, 0x8fbcbb, 0x81a1c1 },
    { "Forest Emerald",    0x141c18, 0x1c2822, 0x28362e, 0x1c2822, 0xf0f7f4, 0xa3baaf, 0xdae7e0, 0x10b981, 0x059669, 0x047857 },
    { "Neon Cyberpunk",    0x0a0a0f, 0x140f1e, 0x231932, 0x140f1e, 0x00fff0, 0xff007f, 0xffffff, 0xff007f, 0xff3399, 0xcc0066 },
};

int g_currentThemeIdx = 0;

std::string ReadFileToString(const std::string& path) {
    std::ifstream fs(path, std::ios::binary);
    if (!fs.is_open()) return "";
    return std::string((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
}

std::map<std::string, std::vector<std::string>> ParseConfig(const std::string& jsonStr) {
    std::map<std::string, std::vector<std::string>> config;
    size_t i = 0, len = jsonStr.length();

    auto skip = [&]() {
        while (i < len && (jsonStr[i] == ' ' || jsonStr[i] == '\t' || jsonStr[i] == '\r' || jsonStr[i] == '\n')) i++;
    };
    auto parseStr = [&]() -> std::string {
        std::string s;
        if (i < len && jsonStr[i] == '"') {
            i++;
            while (i < len && jsonStr[i] != '"') {
                if (jsonStr[i] == '\\' && i + 1 < len) { i++; s += jsonStr[i]; }
                else { s += jsonStr[i]; }
                i++;
            }
            if (i < len) i++;
        }
        return s;
    };

    skip();
    if (i < len && jsonStr[i] == '{') {
        i++;
        while (i < len) {
            skip();
            if (jsonStr[i] == '}') break;
            if (jsonStr[i] == '"') {
                std::string cat = parseStr();
                skip();
                if (i < len && jsonStr[i] == ':') {
                    i++; skip();
                    if (i < len && jsonStr[i] == '[') {
                        i++;
                        std::vector<std::string> exts;
                        while (i < len) {
                            skip();
                            if (jsonStr[i] == ']') { i++; break; }
                            if (jsonStr[i] == '"') {
                                std::string ext = parseStr();
                                for (auto& c : ext) c = std::tolower(c);
                                exts.push_back(ext);
                            } else if (jsonStr[i] == ',') { i++; }
                            else { i++; }
                        }
                        config[cat] = exts;
                    }
                }
            } else if (jsonStr[i] == ',') { i++; }
            else { i++; }
        }
    }
    return config;
}

std::string FormatSize(uint64_t bytes) {
    double size = (double)bytes;
    const char* unit = "B";
    if (size >= 1024) { size /= 1024; unit = "KB"; }
    if (size >= 1024) { size /= 1024; unit = "MB"; }
    if (size >= 1024) { size /= 1024; unit = "GB"; }
    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f %s", size, unit);
    return buf;
}

std::string EscapePDFText(const std::string& text) {
    std::string s;
    for (char c : text) {
        if ((unsigned char)c < 128) {
            if (c == '(' || c == ')' || c == '\\') s += '\\';
            s += c;
        } else {
            s += '?';
        }
    }
    return s;
}

std::string TruncateText(const std::string& text, size_t maxLen) {
    if (text.length() <= maxLen) return text;
    return text.substr(0, maxLen - 3) + "...";
}

void GeneratePDFReport(const std::string& outputPath, const std::string& targetFolder,
                       const std::vector<MovedFileInfo>& movedFiles, bool dryRun) {
    if (movedFiles.empty()) return;

    std::vector<MovedFileInfo> sortedFiles = movedFiles;
    std::sort(sortedFiles.begin(), sortedFiles.end(), [](const MovedFileInfo& a, const MovedFileInfo& b) {
        if (a.category != b.category) return a.category < b.category;
        return a.fileName < b.fileName;
    });

    uint64_t totalSize = 0;
    for (const auto& f : sortedFiles)
        if (f.status.find("Error") == std::string::npos)
            totalSize += f.fileSize;
    std::string totalSizeStr = FormatSize(totalSize);

    const size_t rowsPerPageFirst = 25, rowsPerPageSubsequent = 32;
    size_t total = sortedFiles.size(), pageCount = 1;
    if (total > rowsPerPageFirst)
        pageCount = 1 + (total - rowsPerPageFirst + rowsPerPageSubsequent - 1) / rowsPerPageSubsequent;

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &in_time_t);
#else
    localtime_r(&in_time_t, &tm);
#endif
    char timeBuf[64];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tm);

    std::vector<std::string> contents;
    size_t fileIdx = 0;

    for (size_t p = 0; p < pageCount; ++p) {
        std::stringstream ss;
        ss << "0.9 0.9 0.9 RG 0.5 w 30 30 535 782 re S\n";

        size_t startY;
        if (p == 0) {
            ss << "0.95 0.95 0.98 rg 40 700 515 100 re f\n0.85 0.85 0.9 RG 1 w 40 700 515 100 re S\n";
            ss << "BT /F2 18 Tf 0.1 0.1 0.25 rg 55 765 Td ("
               << (dryRun ? "urFileManager - DRY RUN REPORT" : "urFileManager - File Transfer Report")
               << ") Tj ET\n";
            ss << "BT /F1 9 Tf 0.35 0.35 0.45 rg 55 745 Td (Report generated on: " << EscapePDFText(timeBuf) << ") Tj ET\n";
            ss << "BT /F1 9 Tf 0.35 0.35 0.45 rg 55 730 Td (Target Folder: " << EscapePDFText(targetFolder) << ") Tj ET\n";
            ss << "BT /F2 10 Tf 0.15 0.5 0.15 rg 55 712 Td (Status: "
               << (dryRun ? "PREVIEW ONLY" : "COMPLETED")
               << "  |  Total Files: " << total
               << "  |  Total Space: " << EscapePDFText(totalSizeStr) << ") Tj ET\n";
            ss << "0.9 0.9 0.92 rg 40 655 515 22 re f\n0.7 0.7 0.75 RG 1 w 40 655 515 22 re S\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 48 662 Td (File Name) Tj ET\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 268 662 Td (Category) Tj ET\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 368 662 Td (Size) Tj ET\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 458 662 Td (Status) Tj ET\n";
            startY = 655;
        } else {
            ss << "0.9 0.9 0.92 rg 40 770 515 22 re f\n0.7 0.7 0.75 RG 1 w 40 770 515 22 re S\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 48 777 Td (File Name) Tj ET\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 268 777 Td (Category) Tj ET\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 368 777 Td (Size) Tj ET\n";
            ss << "BT /F2 9 Tf 0.15 0.15 0.2 rg 458 777 Td (Status) Tj ET\n";
            startY = 770;
        }

        size_t items = (p == 0) ? rowsPerPageFirst : rowsPerPageSubsequent;
        size_t drawn = 0;
        for (size_t r = 0; r < items && fileIdx < total; ++r, ++fileIdx) {
            const auto& f = sortedFiles[fileIdx];
            size_t rowY = startY - 20 - (r * 20);
            drawn++;
            if (r % 2 == 1) ss << "0.97 0.97 0.99 rg 40 " << rowY << " 515 20 re f\n";
            ss << "0.9 0.9 0.9 RG 0.5 w 40 " << rowY << " m 555 " << rowY << " l S\n";

            size_t textY = rowY + 6;
            ss << "BT /F1 9 Tf 0.15 0.15 0.15 rg 48 " << textY << " Td (" << EscapePDFText(TruncateText(f.fileName, 38)) << ") Tj ET\n";
            ss << "BT /F1 9 Tf 0.15 0.15 0.15 rg 268 " << textY << " Td (" << EscapePDFText(TruncateText(f.category, 15)) << ") Tj ET\n";
            ss << "BT /F1 9 Tf 0.15 0.15 0.15 rg 368 " << textY << " Td (" << EscapePDFText(FormatSize(f.fileSize)) << ") Tj ET\n";

            if (f.status == "Moved")
                ss << "BT /F2 9 Tf 0.1 0.5 0.1 rg 458 " << textY << " Td (Moved) Tj ET\n";
            else if (f.status.find("Renamed") != std::string::npos || f.status.find("Conflict") != std::string::npos)
                ss << "BT /F2 9 Tf 0.8 0.4 0.0 rg 458 " << textY << " Td (Renamed) Tj ET\n";
            else if (f.status.find("Error") != std::string::npos)
                ss << "BT /F2 9 Tf 0.8 0.1 0.1 rg 458 " << textY << " Td (Error) Tj ET\n";
            else
                ss << "BT /F1 9 Tf 0.2 0.2 0.7 rg 458 " << textY << " Td (" << EscapePDFText(f.status) << ") Tj ET\n";
        }

        size_t finalY = startY - 20 - (drawn * 20);
        ss << "0.85 0.85 0.85 RG 0.5 w 40 " << finalY << " m 40 " << startY << " l S\n";
        ss << "0.85 0.85 0.85 RG 0.5 w 260 " << finalY << " m 260 " << startY << " l S\n";
        ss << "0.85 0.85 0.85 RG 0.5 w 360 " << finalY << " m 360 " << startY << " l S\n";
        ss << "0.85 0.85 0.85 RG 0.5 w 450 " << finalY << " m 450 " << startY << " l S\n";
        ss << "0.85 0.85 0.85 RG 0.5 w 555 " << finalY << " m 555 " << startY << " l S\n";
        ss << "0.7 0.7 0.75 RG 1 w 40 " << finalY << " m 555 " << finalY << " l S\n";
        ss << "BT /F1 8 Tf 0.5 0.5 0.5 rg 270 45 Td (Page " << (p + 1) << " of " << pageCount << ") Tj ET\n";
        contents.push_back(ss.str());
    }

    std::vector<std::string> pdfObjects;
    pdfObjects.push_back("<< /Type /Catalog /Pages 2 0 R >>");
    std::string kids;
    for (size_t p = 0; p < pageCount; ++p) kids += std::to_string(5 + p) + " 0 R ";
    pdfObjects.push_back("<< /Type /Pages /Kids [" + kids + "] /Count " + std::to_string(pageCount) + " >>");
    pdfObjects.push_back("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>");
    pdfObjects.push_back("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold >>");
    for (size_t p = 0; p < pageCount; ++p)
        pdfObjects.push_back("<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842]"
                             " /Resources << /Font << /F1 3 0 R /F2 4 0 R >> >>"
                             " /Contents " + std::to_string(5 + pageCount + p) + " 0 R >>");
    for (size_t p = 0; p < pageCount; ++p) {
        const auto& c = contents[p];
        pdfObjects.push_back("<< /Length " + std::to_string(c.length()) + " >>\nstream\n" + c + "\nendstream");
    }

    std::ofstream out(std::filesystem::path(outputPath), std::ios::binary);
    if (!out.is_open()) return;
    out << "%PDF-1.4\n";
    std::vector<size_t> offsets;
    size_t offset = 9;
    for (size_t i = 0; i < pdfObjects.size(); ++i) {
        offsets.push_back(offset);
        std::string obj = std::to_string(i + 1) + " 0 obj\n" + pdfObjects[i] + "\nendobj\n";
        out.write(obj.c_str(), obj.length());
        offset += obj.length();
    }
    size_t xref = offset;
    out << "xref\n0 " << (pdfObjects.size() + 1) << "\n";
    char buf[64];
    snprintf(buf, sizeof(buf), "%010d 65535 f\r\n", 0);
    out.write(buf, strlen(buf));
    for (size_t o : offsets) {
        snprintf(buf, sizeof(buf), "%010zu 00000 n\r\n", o);
        out.write(buf, strlen(buf));
    }
    out << "trailer\n<< /Size " << (pdfObjects.size() + 1) << " /Root 1 0 R >>\nstartxref\n" << xref << "\n%%EOF\n";
    out.close();
}
