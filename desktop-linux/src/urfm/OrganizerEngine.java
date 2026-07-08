package urfm;

import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.AccessDeniedException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class OrganizerEngine {

    public interface LogListener {
        void onLog(String message);
    }

    /**
     * Reverts a previous organization by reading .organize_undo.json and moving files back.
     */
    public static void revert(Path sourceDir, LogListener listener) {
        Path undoPath = sourceDir.resolve(".organize_undo.json");
        if (!Files.exists(undoPath)) {
            log(listener, "[ERROR] No undo log found at: " + undoPath);
            log(listener, "[ERROR] Cannot revert without an undo log.");
            return;
        }

        try {
            String content = Files.readString(undoPath);
            List<Map<String, String>> moves = parseUndoLog(content);

            if (moves.isEmpty()) {
                log(listener, "[INFO] Undo log is empty. Nothing to revert.");
                return;
            }

            log(listener, "[INFO] Reverting " + moves.size() + " file(s)...");
            int reverted = 0;
            int errors = 0;

            for (Map<String, String> move : moves) {
                Path original = Path.of(move.get("original"));
                Path moved = Path.of(move.get("moved"));

                if (!Files.exists(moved)) {
                    log(listener, "[WARN] File not found (skipped): " + moved);
                    continue;
                }

                try {
                    Files.createDirectories(original.getParent());
                    Files.move(moved, original);
                    log(listener, "Reverted: '" + moved.getFileName() + "' -> '" + original + "'");
                    reverted++;
                } catch (Exception e) {
                    log(listener, "[ERROR] Failed to revert '" + moved.getFileName() + "': " + e.getMessage());
                    errors++;
                }
            }

            log(listener, "[INFO] Revert complete. Reverted: " + reverted + ", Errors: " + errors);
            if (errors == 0) {
                Files.delete(undoPath);
                log(listener, "[INFO] Undo log removed.");
            }
        } catch (Exception e) {
            log(listener, "[ERROR] An error occurred during revert: " + e.getMessage());
        }
    }

    /**
     * Orchestrates the file organization process for a given directory.
     */
    public static List<Map<String, Object>> organize(
            Path sourceDir,
            boolean dryRun,
            Map<String, List<String>> fileTypeMap,
            LogListener listener
    ) {
        log(listener, "Starting to organize directory: " + sourceDir);
        if (dryRun) {
            log(listener, "--- DRY RUN MODE ENABLED: No files will be moved. ---");
        } else {
            log(listener, "--- LIVE RUN MODE ENABLED: File system changes will be made. ---");
        }

        List<Map<String, Object>> movedFiles = new ArrayList<>();
        List<Map<String, String>> undoLog = new ArrayList<>();

        try {
            // Find all files in the directory
            Files.list(sourceDir).filter(Files::isRegularFile).forEach(fileItem -> {
                Map<String, Object> res = processFile(fileItem, sourceDir, fileTypeMap, dryRun, undoLog, listener);
                movedFiles.add(res);
            });

            // Write Undo Log in Live Run
            if (!dryRun && !undoLog.isEmpty()) {
                Path undoPath = sourceDir.resolve(".organize_undo.json");
                StringBuilder json = new StringBuilder("{\n  \"moves\": [\n");
                for (int i = 0; i < undoLog.size(); i++) {
                    Map<String, String> move = undoLog.get(i);
                    json.append("    {\n");
                    json.append("      \"original\": \"").append(escapeJson(move.get("original"))).append("\",\n");
                    json.append("      \"moved\": \"").append(escapeJson(move.get("moved"))).append("\"\n");
                    json.append("    }").append(i < undoLog.size() - 1 ? ",\n" : "\n");
                }
                json.append("  ],\n  \"timestamp\": \"").append(new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date())).append("\"\n}");
                Files.writeString(undoPath, json.toString(), StandardCharsets.UTF_8);
                log(listener, "[INFO] Undo log saved to: " + undoPath);
            }

            // Generate PDF Report
            if (!movedFiles.isEmpty()) {
                String reportName = dryRun ? "organization_report_preview.pdf" : "organization_report.pdf";
                Path reportPath = sourceDir.resolve(reportName);
                log(listener, "[INFO] Generating PDF report: " + reportPath);
                generatePdfReport(reportPath, movedFiles, dryRun, sourceDir);
                log(listener, "[INFO] PDF report successfully created at: " + reportPath);
            }

        } catch (Exception e) {
            log(listener, "[ERROR] Organization failed: " + e.getMessage());
        }

        return movedFiles;
    }

    private static Map<String, Object> processFile(
            Path filePath,
            Path sourcePath,
            Map<String, List<String>> fileTypeMap,
            boolean dryRun,
            List<Map<String, String>> undoLog,
            LogListener listener
    ) {
        String fileName = filePath.getFileName().toString();
        String fileExtension = "";
        int dotIdx = fileName.lastIndexOf('.');
        if (dotIdx != -1) {
            fileExtension = fileName.substring(dotIdx).toLowerCase();
        }

        String destinationFolderName = "Other";
        for (Map.Entry<String, List<String>> entry : fileTypeMap.entrySet()) {
            if (entry.getValue().contains(fileExtension)) {
                destinationFolderName = entry.getKey();
                break;
            }
        }

        Path destinationDir = sourcePath.resolve(destinationFolderName);
        long fileSize = 0;
        try {
            fileSize = Files.size(filePath);
        } catch (IOException ignored) {}

        Map<String, Object> result = new HashMap<>();
        result.put("fileName", fileName);
        result.put("category", destinationFolderName);
        result.put("fileSize", fileSize);

        if (dryRun) {
            Path destinationFilePath = destinationDir.resolve(fileName);
            log(listener, "[DRY RUN] Would move '" + fileName + "' -> '" + destinationFilePath + "'");
            result.put("status", "Dry Run Preview");
            return result;
        } else {
            try {
                Files.createDirectories(destinationDir);
                Path destinationFilePath = destinationDir.resolve(fileName);
                int counter = 1;
                String baseName = dotIdx != -1 ? fileName.substring(0, dotIdx) : fileName;
                String ext = dotIdx != -1 ? fileName.substring(dotIdx) : "";

                while (Files.exists(destinationFilePath)) {
                    log(listener, "[WARN] Conflict: '" + destinationFilePath + "' already exists.");
                    String newFilename = baseName + " (" + counter + ")" + ext;
                    destinationFilePath = destinationDir.resolve(newFilename);
                    counter++;
                }

                Files.move(filePath, destinationFilePath);
                log(listener, "Moved: '" + fileName + "' -> '" + destinationFilePath + "'");
                
                Map<String, String> undoEntry = new HashMap<>();
                undoEntry.put("original", filePath.toAbsolutePath().toString());
                undoEntry.put("moved", destinationFilePath.toAbsolutePath().toString());
                undoLog.add(undoEntry);

                result.put("status", counter > 1 ? "Renamed" : "Moved");
            } catch (AccessDeniedException e) {
                log(listener, "[ERROR] Could not move '" + fileName + "'. Permission Denied.");
                result.put("status", "Permission Error");
            } catch (Exception e) {
                log(listener, "[ERROR] An unexpected error occurred while moving '" + fileName + "'. Error: " + e.getMessage());
                result.put("status", "Error: " + e.getMessage());
            }
            return result;
        }
    }

    private static void generatePdfReport(Path reportPath, List<Map<String, Object>> movedFiles, boolean dryRun, Path targetFolder) {
        // Sort files by category and then filename
        movedFiles.sort((a, b) -> {
            String catA = (String) a.getOrDefault("category", "");
            String catB = (String) b.getOrDefault("category", "");
            int comp = catA.compareTo(catB);
            if (comp != 0) return comp;
            String nameA = (String) a.getOrDefault("fileName", "");
            String nameB = (String) b.getOrDefault("fileName", "");
            return nameA.compareTo(nameB);
        });

        long totalSize = 0;
        for (Map<String, Object> f : movedFiles) {
            String st = (String) f.get("status");
            if ("Moved".equals(st) || "Renamed".equals(st) || "Dry Run Preview".equals(st)) {
                totalSize += ((Number) f.get("fileSize")).longValue();
            }
        }
        String totalSizeStr = formatSize(totalSize);
        int totalFiles = movedFiles.size();

        // Calculate pagination
        int rowsPerPageFirst = 25;
        int rowsPerPageSubsequent = 32;
        int pageCount = 0;
        int tempCount = totalFiles;
        if (tempCount <= rowsPerPageFirst) {
            pageCount = 1;
        } else {
            pageCount = 1;
            tempCount -= rowsPerPageFirst;
            pageCount += (tempCount + rowsPerPageSubsequent - 1) / rowsPerPageSubsequent;
        }

        String timeStr = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date());
        String targetFolderStr = targetFolder.toAbsolutePath().toString();

        List<String> contents = new ArrayList<>();
        int fileIdx = 0;

        for (int p = 0; p < pageCount; p++) {
            StringBuilder ss = new StringBuilder();
            // Faint border on page margins
            ss.append("0.9 0.9 0.9 RG 0.5 w 30 30 535 782 re S\n");

            int startY;
            if (p == 0) {
                // Document Header Card Box
                ss.append("0.95 0.95 0.98 rg 40 700 515 100 re f\n");
                ss.append("0.85 0.85 0.9 RG 1 w 40 700 515 100 re S\n");

                // Title Text
                String title = dryRun ? "urFileManager - DRY RUN REPORT" : "urFileManager - File Transfer Report";
                ss.append("BT /F2 18 Tf 0.1 0.1 0.25 rg 55 765 Td (").append(escapePdfText(title)).append(") Tj ET\n");

                // Metadata Lines
                ss.append("BT /F1 9 Tf 0.35 0.35 0.45 rg 55 745 Td (Report generated on: ").append(escapePdfText(timeStr)).append(") Tj ET\n");
                ss.append("BT /F1 9 Tf 0.35 0.35 0.45 rg 55 730 Td (Target Folder: ").append(escapePdfText(targetFolderStr)).append(") Tj ET\n");

                // Summary Info Strip
                String statusText = dryRun ? "PREVIEW ONLY" : "COMPLETED";
                ss.append("BT /F2 10 Tf 0.15 0.5 0.15 rg 55 712 Td (Status: ").append(statusText)
                  .append("   |   Total Files: ").append(totalFiles)
                  .append("   |   Total Space Organised: ").append(escapePdfText(totalSizeStr)).append(") Tj ET\n");

                // Table Header Background and Text
                ss.append("0.9 0.9 0.92 rg 40 655 515 22 re f\n");
                ss.append("0.7 0.7 0.75 RG 1 w 40 655 515 22 re S\n");

                ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 48 662 Td (File Name) Tj ET\n");
                ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 268 662 Td (Category) Tj ET\n");
                ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 368 662 Td (Size) Tj ET\n");
                ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 458 662 Td (Status) Tj ET\n");

                startY = 655;
            } else {
                // Subsequent Page Table Header
                ss.append("0.9 0.9 0.92 rg 40 770 515 22 re f\n");
                ss.append("0.7 0.7 0.75 RG 1 w 40 770 515 22 re S\n");

                ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 48 777 Td (File Name) Tj ET\n");
                ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 268 777 Td (Category) Tj ET\n");
                ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 368 777 Td (Size) Tj ET\n");
                ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 458 777 Td (Status) Tj ET\n");

                startY = 770;
            }

            int itemsOnThisPage = (p == 0) ? rowsPerPageFirst : rowsPerPageSubsequent;
            int rowsDrawn = 0;

            for (int rowIdx = 0; rowIdx < itemsOnThisPage; rowIdx++) {
                if (fileIdx >= totalFiles) break;
                Map<String, Object> file = movedFiles.get(fileIdx);
                int rowY = startY - 20 - (rowIdx * 20);
                rowsDrawn++;
                fileIdx++;

                // Alternate row zebra bg
                if (rowIdx % 2 == 1) {
                    ss.append("0.97 0.97 0.99 rg 40 ").append(rowY).append(" 515 20 re f\n");
                }

                // Horizontal row divider line
                ss.append("0.9 0.9 0.9 RG 0.5 w 40 ").append(rowY).append(" m 555 ").append(rowY).append(" l S\n");

                int textY = rowY + 6;

                // Render text
                ss.append("BT /F1 9 Tf 0.15 0.15 0.15 rg 48 ").append(textY).append(" Td (")
                  .append(escapePdfText(truncateText((String) file.get("fileName"), 38))).append(") Tj ET\n");
                ss.append("BT /F1 9 Tf 0.15 0.15 0.15 rg 268 ").append(textY).append(" Td (")
                  .append(escapePdfText(truncateText((String) file.get("category"), 15))).append(") Tj ET\n");
                ss.append("BT /F1 9 Tf 0.15 0.15 0.15 rg 368 ").append(textY).append(" Td (")
                  .append(escapePdfText(formatSize(((Number) file.get("fileSize")).longValue()))).append(") Tj ET\n");

                // Status coloring
                String st = (String) file.get("status");
                if ("Moved".equals(st)) {
                    ss.append("BT /F2 9 Tf 0.1 0.5 0.1 rg 458 ").append(textY).append(" Td (Moved) Tj ET\n");
                } else if ("Renamed".equals(st) || st.contains("Conflict")) {
                    ss.append("BT /F2 9 Tf 0.8 0.4 0.0 rg 458 ").append(textY).append(" Td (Renamed) Tj ET\n");
                } else if (st.contains("Error") || st.contains("Failed") || st.contains("Could not")) {
                    ss.append("BT /F2 9 Tf 0.8 0.1 0.1 rg 458 ").append(textY).append(" Td (Error) Tj ET\n");
                } else {
                    ss.append("BT /F1 9 Tf 0.2 0.2 0.7 rg 458 ").append(textY).append(" Td (").append(escapePdfText(st)).append(") Tj ET\n");
                }
            }

            // Vertical columns lines
            int finalY = startY - 20 - (rowsDrawn * 20);
            ss.append("0.85 0.85 0.85 RG 0.5 w 40 ").append(finalY).append(" m 40 ").append(startY).append(" l S\n");
            ss.append("0.85 0.85 0.85 RG 0.5 w 260 ").append(finalY).append(" m 260 ").append(startY).append(" l S\n");
            ss.append("0.85 0.85 0.85 RG 0.5 w 360 ").append(finalY).append(" m 360 ").append(startY).append(" l S\n");
            ss.append("0.85 0.85 0.85 RG 0.5 w 450 ").append(finalY).append(" m 450 ").append(startY).append(" l S\n");
            ss.append("0.85 0.85 0.85 RG 0.5 w 555 ").append(finalY).append(" m 555 ").append(startY).append(" l S\n");

            // Table bottom border
            ss.append("0.7 0.7 0.75 RG 1 w 40 ").append(finalY).append(" m 555 ").append(finalY).append(" l S\n");

            // Footer
            ss.append("BT /F1 8 Tf 0.5 0.5 0.5 rg 270 45 Td (Page ").append(p + 1).append(" of ").append(pageCount).append(") Tj ET\n");

            contents.add(ss.toString());
        }

        // PDF object assembly
        List<byte[]> pdfObjects = new ArrayList<>();
        pdfObjects.add("<< /Type /Catalog /Pages 2 0 R >>".getBytes(StandardCharsets.UTF_8));

        StringBuilder kids = new StringBuilder();
        for (int p = 0; p < pageCount; p++) {
            kids.append(5 + p).append(" 0 R ");
        }
        pdfObjects.add(String.format("<< /Type /Pages /Kids [%s] /Count %d >>", kids.toString().trim(), pageCount).getBytes(StandardCharsets.UTF_8));
        pdfObjects.add("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>".getBytes(StandardCharsets.UTF_8));
        pdfObjects.add("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold >>".getBytes(StandardCharsets.UTF_8));

        for (int p = 0; p < pageCount; p++) {
            pdfObjects.add(String.format("<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Resources << /Font << /F1 3 0 R /F2 4 0 R >> >> /Contents %d 0 R >>", 5 + pageCount + p).getBytes(StandardCharsets.UTF_8));
        }

        for (int p = 0; p < pageCount; p++) {
            String cStr = contents.get(p);
            byte[] cBytes = cStr.getBytes(StandardCharsets.UTF_8);
            String streamHeaderStr = String.format("<< /Length %d >>\nstream\n", cBytes.length);
            byte[] streamHeader = streamHeaderStr.getBytes(StandardCharsets.UTF_8);
            byte[] streamFooter = "\nendstream".getBytes(StandardCharsets.UTF_8);

            byte[] fullStream = new byte[streamHeader.length + cBytes.length + streamFooter.length];
            System.arraycopy(streamHeader, 0, fullStream, 0, streamHeader.length);
            System.arraycopy(cBytes, 0, fullStream, streamHeader.length, cBytes.length);
            System.arraycopy(streamFooter, 0, fullStream, streamHeader.length + cBytes.length, streamFooter.length);

            pdfObjects.add(fullStream);
        }

        // Binary writing with offsets
        try (BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(reportPath.toFile()))) {
            bos.write("%PDF-1.4\n".getBytes(StandardCharsets.UTF_8));
            long currentOffset = 9;

            List<Long> offsets = new ArrayList<>();
            for (int i = 0; i < pdfObjects.size(); i++) {
                offsets.add(currentOffset);
                String objHeader = String.format("%d 0 obj\n", i + 1);
                bos.write(objHeader.getBytes(StandardCharsets.UTF_8));
                currentOffset += objHeader.length();

                byte[] objData = pdfObjects.get(i);
                bos.write(objData);
                currentOffset += objData.length;

                String objFooter = "\nendobj\n";
                bos.write(objFooter.getBytes(StandardCharsets.UTF_8));
                currentOffset += objFooter.length();
            }

            long xrefOffset = currentOffset;
            bos.write("xref\n".getBytes(StandardCharsets.UTF_8));
            String xrefRange = String.format("0 %d\n", pdfObjects.size() + 1);
            bos.write(xrefRange.getBytes(StandardCharsets.UTF_8));

            bos.write("0000000000 65535 f\r\n".getBytes(StandardCharsets.UTF_8));
            for (long offset : offsets) {
                String entry = String.format(Locale.US, "%010d 00000 n\r\n", offset);
                bos.write(entry.getBytes(StandardCharsets.UTF_8));
            }

            bos.write("trailer\n".getBytes(StandardCharsets.UTF_8));
            String trailer = String.format("<< /Size %d /Root 1 0 R >>\n", pdfObjects.size() + 1);
            bos.write(trailer.getBytes(StandardCharsets.UTF_8));

            bos.write("startxref\n".getBytes(StandardCharsets.UTF_8));
            bos.write((xrefOffset + "\n").getBytes(StandardCharsets.UTF_8));
            bos.write("%%EOF\n".getBytes(StandardCharsets.UTF_8));
        } catch (Exception e) {
            System.err.println("Failed to generate PDF Report. Error: " + e.getMessage());
        }
    }

    private static String escapePdfText(String text) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            if (c < 128) {
                if (c == '(' || c == ')' || c == '\\') {
                    sb.append('\\');
                }
                sb.append(c);
            } else {
                sb.append('?');
            }
        }
        return sb.toString();
    }

    private static String truncateText(String text, int maxLen) {
        if (text.length() <= maxLen) return text;
        return text.substring(0, maxLen - 3) + "...";
    }

    private static String formatSize(long bytesVal) {
        double size = bytesVal;
        String unit = "B";
        if (size >= 1024) {
            size /= 1024;
            unit = "KB";
        }
        if (size >= 1024) {
            size /= 1024;
            unit = "MB";
        }
        if (size >= 1024) {
            size /= 1024;
            unit = "GB";
        }
        return String.format(Locale.US, "%.2f %s", size, unit);
    }

    private static String escapeJson(String str) {
        if (str == null) return "";
        return str.replace("\\", "\\\\").replace("\"", "\\\"");
    }

    private static List<Map<String, String>> parseUndoLog(String content) {
        List<Map<String, String>> moves = new ArrayList<>();
        int index = content.indexOf("[");
        if (index != -1) {
            int endArray = content.lastIndexOf("]");
            if (endArray != -1) {
                String arrayContent = content.substring(index, endArray);
                String[] blocks = arrayContent.split("\\}");
                for (String block : blocks) {
                    String original = null;
                    String moved = null;

                    int origIdx = block.indexOf("\"original\"");
                    if (origIdx != -1) {
                        int colonIdx = block.indexOf(":", origIdx);
                        int startQuote = block.indexOf("\"", colonIdx);
                        int endQuote = block.indexOf("\"", startQuote + 1);
                        if (startQuote != -1 && endQuote != -1) {
                            original = block.substring(startQuote + 1, endQuote);
                        }
                    }

                    int movedIdx = block.indexOf("\"moved\"");
                    if (movedIdx != -1) {
                        int colonIdx = block.indexOf(":", movedIdx);
                        int startQuote = block.indexOf("\"", colonIdx);
                        int endQuote = block.indexOf("\"", startQuote + 1);
                        if (startQuote != -1 && endQuote != -1) {
                            moved = block.substring(startQuote + 1, endQuote);
                        }
                    }

                    if (original != null && moved != null) {
                        Map<String, String> move = new HashMap<>();
                        // unescape simple JSON escapes
                        move.put("original", original.replace("\\\\", "\\").replace("\\\"", "\""));
                        move.put("moved", moved.replace("\\\\", "\\").replace("\\\"", "\""));
                        moves.add(move);
                    }
                }
            }
        }
        return moves;
    }

    private static void log(LogListener listener, String msg) {
        System.out.println(msg);
        if (listener != null) {
            listener.onLog(msg);
        }
    }
}
