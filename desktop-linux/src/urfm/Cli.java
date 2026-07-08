package urfm;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.Map;

/**
 * Command-line interface for urFileManager.
 *
 * Parses CLI arguments, renders colored status output and delegates the actual
 * file-organizing work to {@link OrganizerEngine} and {@link ConfigParser}.
 */
public final class Cli {

    public static final String VERSION = Main.VERSION;

    private Cli() {}

    /**
     * Prints the full help / usage text with colors.
     */
    public static void printHelp() {
        String title = Console.bold(Console.volt("urFileManager (urFM)") + " — Java CLI v" + VERSION);
        System.out.println(title);
        System.out.println(Console.dim("Bulk file separator & PDF report generator."));
        System.out.println();

        System.out.println(Console.bold("USAGE"));
        System.out.println("  " + Console.white("urfm") + " " + Console.green("<directory>")
                + " [" + Console.volt("--dry-run") + "] [" + Console.volt("--revert") + "]");
        System.out.println("  " + Console.white("urfm") + " " + Console.volt("--gui"));
        System.out.println("  " + Console.white("urfm") + " " + Console.volt("--help") + " | " + Console.volt("--version"));
        System.out.println();

        System.out.println(Console.bold("COMMANDS"));
        System.out.println("  " + Console.green("<directory>")
                + Console.dim("              The folder of files to organize (required for CLI mode).")
                + Console.dim(""));
        System.out.println("  " + Console.volt("--dry-run")
                + Console.dim("             Preview moves & generate a preview PDF without touching files."));
        System.out.println("  " + Console.volt("--revert")
                + Console.dim("             Undo the last organization using the saved undo log."));
        System.out.println("  " + Console.volt("--gui")
                + Console.dim("                Launch the Java Swing graphical interface."));
        System.out.println("  " + Console.volt("--version")
                + Console.dim("             Print version information and exit."));
        System.out.println("  " + Console.volt("--help") + ", " + Console.volt("-h")
                + Console.dim("           Show this help and exit."));
        System.out.println();

        System.out.println(Console.bold("EXAMPLES"));
        System.out.println("  " + Console.dim("$ ") + Console.white("urfm") + " " + Console.green("~/Downloads")
                + "  " + Console.dim("# live run"));
        System.out.println("  " + Console.dim("$ ") + Console.white("urfm") + " " + Console.green("~/Downloads") + " " + Console.volt("--dry-run")
                + "  " + Console.dim("# safe preview"));
        System.out.println("  " + Console.dim("$ ") + Console.white("urfm") + " " + Console.green("~/Downloads") + " " + Console.volt("--revert")
                + "  " + Console.dim("# undo last run"));
        System.out.println("  " + Console.dim("$ ") + Console.white("urfm") + " " + Console.volt("--gui")
                + "  " + Console.dim("# open the GUI"));
        System.out.println();

        System.out.println(Console.bold("NOTES"));
        System.out.println(Console.dim("  • Rules are read from " + Console.white("config.json")
                + " next to the executable, then its parent, then the current dir."));
        System.out.println(Console.dim("  • A PDF report (organization_report.pdf / _preview.pdf) is written to the target folder."));
        System.out.println(Console.dim("  • Colors are disabled automatically when output is not a terminal (set NO_COLOR to force off)."));
    }

    /**
     * Entry point for CLI mode. Returns a process exit code.
     */
    public static int run(String[] args) {
        boolean dryRun = false;
        boolean revert = false;
        boolean showVersion = false;
        String dirArg = null;

        for (String arg : args) {
            switch (arg) {
                case "--dry-run":
                    dryRun = true;
                    break;
                case "--revert":
                    revert = true;
                    break;
                case "--version":
                    showVersion = true;
                    break;
                case "--help":
                case "-h":
                    printHelp();
                    return 0;
                default:
                    if (arg.startsWith("-")) {
                        System.err.println(Console.red("Unknown option: " + arg));
                        printHelp();
                        return 1;
                    }
                    dirArg = arg;
            }
        }

        if (showVersion) {
            System.out.println(Console.bold("urFileManager") + " Java CLI v" + VERSION);
            return 0;
        }

        if (dirArg == null) {
            System.err.println(Console.red("Missing required <directory> argument."));
            System.err.println(Console.dim("Run 'urfm --help' for usage."));
            return 1;
        }

        Path targetDir = Path.of(dirArg);
        if (!Files.exists(targetDir) || !Files.isDirectory(targetDir)) {
            System.err.println(Console.red("[ERROR]") + " The path '" + dirArg + "' is not a valid directory.");
            return 1;
        }

        Path configPath = Main.getConfigPath();
        Map<String, List<String>> config = null;
        try {
            config = ConfigParser.parse(configPath);
        } catch (IOException e) {
            System.err.println(Console.red("[ERROR]") + " Failed to load config from: " + configPath);
            System.err.println(Console.dim("Details: " + e.getMessage()));
            return 1;
        }

        printBanner(targetDir, dryRun, revert);

        ColoredLogger logger = new ColoredLogger();
        int count;
        if (revert) {
            OrganizerEngine.revert(targetDir, logger);
            count = -1; // revert prints its own summary
        } else {
            List<Map<String, Object>> files = OrganizerEngine.organize(targetDir, dryRun, config, logger);
            count = files.size();
        }

        printFooter(targetDir, dryRun, revert, count);
        return 0;
    }

    private static void printBanner(Path targetDir, boolean dryRun, boolean revert) {
        String mode;
        if (revert) {
            mode = Console.volt(Console.bold("[ REVERT MODE ]"));
        } else if (dryRun) {
            mode = Console.volt(Console.bold("[ DRY RUN — NO FILES WILL BE MOVED ]"));
        } else {
            mode = Console.volt(Console.bold("[ LIVE RUN — FILE SYSTEM WILL BE MODIFIED ]"));
        }
        System.out.println();
        System.out.println(Console.dim("────────────────────────────────────────────────────────────"));
        System.out.println(" " + mode);
        System.out.println(" " + Console.dim("Target : ") + Console.white(targetDir.toAbsolutePath().toString()));
        System.out.println(Console.dim("────────────────────────────────────────────────────────────"));
        System.out.println();
    }

    private static void printFooter(Path targetDir, boolean dryRun, boolean revert, int count) {
        System.out.println();
        System.out.println(Console.dim("────────────────────────────────────────────────────────────"));
        if (revert) {
            System.out.println(" " + Console.volt("Revert operation finished."));
        } else if (dryRun) {
            System.out.println(" " + Console.volt("Dry run complete. ")
                    + Console.white(count + " file(s)") + Console.dim(" analysed — no changes made."));
            System.out.println(" " + Console.dim("Preview report: ")
                    + Console.white(targetDir.resolve("organization_report_preview.pdf").toString()));
        } else {
            System.out.println(" " + Console.green("Live run complete. ")
                    + Console.white(count + " file(s)") + Console.dim(" processed."));
            System.out.println(" " + Console.dim("Report: ")
                    + Console.white(targetDir.resolve("organization_report.pdf").toString()));
        }
        System.out.println(Console.dim("────────────────────────────────────────────────────────────"));
        System.out.println();
    }
}
