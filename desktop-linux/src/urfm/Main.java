package urfm;

import java.awt.GraphicsEnvironment;
import java.io.IOException;
import java.net.URISyntaxException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.*;

public class Main {
    public static final String VERSION = "1.0.0";

    public static void main(String[] args) {
        // Parse Arguments
        boolean dryRun = false;
        boolean revert = false;
        boolean showVersion = false;
        boolean guiMode = false;
        String dirArg = null;

        for (String arg : args) {
            if ("--dry-run".equals(arg)) {
                dryRun = true;
            } else if ("--revert".equals(arg)) {
                revert = true;
            } else if ("--version".equals(arg)) {
                showVersion = true;
            } else if ("--gui".equals(arg)) {
                guiMode = true;
            } else if (arg.startsWith("-")) {
                printUsage();
                System.exit(1);
            } else {
                dirArg = arg;
            }
        }

        if (showVersion) {
            System.out.println("urFileManager Java CLI v" + VERSION);
            System.exit(0);
        }

        // Determine Config Path
        Path configPath = getConfigPath();

        // 1. CLI Mode (skip if --gui was passed)
        if (dirArg != null && !guiMode) {
            Path targetDir = Path.of(dirArg);
            if (!Files.exists(targetDir) || !Files.isDirectory(targetDir)) {
                System.err.println("[ERROR] The path '" + dirArg + "' is not a valid directory.");
                System.exit(1);
            }

            // Parse config for sorting rules
            Map<String, List<String>> config = null;
            try {
                config = ConfigParser.parse(configPath);
            } catch (IOException e) {
                System.err.println("[ERROR] Failed to load config from: " + configPath);
                System.err.println("Details: " + e.getMessage());
                System.exit(1);
            }

            if (revert) {
                OrganizerEngine.revert(targetDir, null);
            } else {
                OrganizerEngine.organize(targetDir, dryRun, config, null);
            }
            System.exit(0);
        }

        // 2. GUI Mode
        if (GraphicsEnvironment.isHeadless()) {
            System.out.println("Headless environment detected. Launching in CLI mode requires a folder path.");
            printUsage();
            System.exit(1);
        }

        // Parse config for GUI, fallback to default map if missing
        Map<String, List<String>> config = new HashMap<>();
        try {
            if (Files.exists(configPath)) {
                config = ConfigParser.parse(configPath);
            } else {
                System.err.println("[WARN] config.json not found at " + configPath + ". Using empty config.");
            }
        } catch (IOException e) {
            System.err.println("[WARN] Failed to parse config.json: " + e.getMessage());
        }

        Map<String, List<String>> finalConfig = config;
        javax.swing.SwingUtilities.invokeLater(() -> {
            UrfmGUI gui = new UrfmGUI(finalConfig);
            gui.setVisible(true);
        });
    }

    private static Path getConfigPath() {
        try {
            // Find path relative to the running jar/class location
            Path jarPath = Path.of(Main.class.getProtectionDomain().getCodeSource().getLocation().toURI());
            Path jarDir = Files.isDirectory(jarPath) ? jarPath : jarPath.getParent();
            Path config = jarDir.resolve("config.json");
            if (Files.exists(config)) {
                return config;
            }
            
            // Check parent of jarDir (useful in build/ development layouts)
            Path parentConfig = jarDir.getParent().resolve("config.json");
            if (Files.exists(parentConfig)) {
                return parentConfig;
            }
        } catch (URISyntaxException | IllegalArgumentException ignored) {}

        // Fallback to current directory config.json
        return Path.of("config.json");
    }

    private static void printUsage() {
        System.out.println("usage: urfm <directory> [--dry-run] [--revert] [--version]");
        System.out.println();
        System.out.println("Options:");
        System.out.println("  --dry-run    Preview moves without modifying file system (CLI)");
        System.out.println("  --revert     Undo the last organization on the target directory");
        System.out.println("  --version    Show version info");
        System.out.println("  --gui        Force launch the Swing GUI");
        System.out.println("  (no args)    Launches the Swing GUI");
    }
}
