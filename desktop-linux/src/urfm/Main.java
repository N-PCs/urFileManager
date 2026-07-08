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
        // Top-level dispatch flags
        boolean guiMode = false;
        boolean help = false;
        boolean version = false;
        String dirArg = null;

        for (String arg : args) {
            if ("--gui".equals(arg)) {
                guiMode = true;
            } else if ("--help".equals(arg) || "-h".equals(arg)) {
                help = true;
            } else if ("--version".equals(arg)) {
                version = true;
            } else if (!arg.startsWith("-")) {
                dirArg = arg;
            }
        }

        // --help works in any context
        if (help) {
            Cli.printHelp();
            return;
        }

        // --version works in any context
        if (version) {
            System.out.println("urFileManager Java CLI v" + VERSION);
            return;
        }

        // --gui (or no directory at all) launches the Swing GUI
        if (guiMode || dirArg == null) {
            launchGui();
            return;
        }

        // Otherwise, run the command-line interface
        int exitCode = Cli.run(args);
        System.exit(exitCode);
    }

    /**
     * Loads config and launches the Swing GUI, with a clear error if headless.
     */
    private static void launchGui() {
        if (GraphicsEnvironment.isHeadless()) {
            System.err.println(Console.red("[ERROR]") + " The GUI cannot start: no graphical environment was detected.");
            System.err.println(Console.dim("Java reports headless — this usually means one of:"));
            System.err.println(Console.dim("  • You are on a server/SSH session without a display."));
            System.err.println(Console.dim("  • The JVM in use is the headless package (e.g. java-*-openjdk-headless)."));
            System.err.println(Console.dim("  • No X11/XWayland display is available (set DISPLAY, e.g. export DISPLAY=:0)."));
            System.err.println();
            System.err.println(Console.dim("Fixes:"));
            System.err.println(Console.dim("  • Install a FULL JDK/JRE (not -headless):  sudo dnf install java-latest-openjdk"));
            System.err.println(Console.dim("  • On Wayland ensure XWayland is running; on SSH use:  ssh -X user@host"));
            System.err.println(Console.dim("  • Otherwise use CLI mode, e.g.:  urfm <directory> --dry-run"));
            System.exit(1);
        }

        Path configPath = getConfigPath();
        Map<String, List<String>> config = new HashMap<>();
        try {
            if (Files.exists(configPath)) {
                config = ConfigParser.parse(configPath);
            } else {
                System.err.println(Console.yellow("[WARN]")
                        + " config.json not found at " + configPath + ". Using empty config.");
            }
        } catch (IOException e) {
            System.err.println(Console.yellow("[WARN]") + " Failed to parse config.json: " + e.getMessage());
        }

        Map<String, List<String>> finalConfig = config;
        try {
            javax.swing.SwingUtilities.invokeLater(() -> {
                UrfmGUI gui = new UrfmGUI(finalConfig);
                gui.setVisible(true);
            });
        } catch (Throwable t) {
            System.err.println(Console.red("[ERROR]") + " Failed to start the GUI: " + t.getMessage());
            System.err.println(Console.dim("Use CLI mode instead, e.g.: urfm <directory> --dry-run"));
            System.exit(1);
        }
    }

    static Path getConfigPath() {
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
}
