package urfm;

/**
 * Minimal ANSI terminal color helper for the CLI.
 *
 * Brand identity matches the frontend-web "Volt" theme (three colours):
 *   - background : obsidian  #0a0a0c
 *   - foreground : off-white #fbfbfb
 *   - accent     : volt      #dcf365
 *
 * Colours are auto-disabled when output is not a real terminal or NO_COLOR is set.
 * The volt accent is emitted as true-colour (24-bit) with a bright-yellow fallback.
 */
public final class Console {

    public static final String RESET   = "\033[0m";
    public static final String BOLD    = "\033[1m";
    public static final String DIM     = "\033[2m";
    public static final String ITALIC  = "\033[3m";
    public static final String UNDERLINE = "\033[4m";

    // 16-colour fallbacks
    public static final String BLACK   = "\033[30m";
    public static final String RED     = "\033[31m";
    public static final String GREEN   = "\033[32m";
    public static final String YELLOW  = "\033[33m";
    public static final String BLUE    = "\033[34m";
    public static final String MAGENTA = "\033[35m";
    public static final String CYAN    = "\033[36m";
    public static final String WHITE   = "\033[37m";
    public static final String BRIGHT_YELLOW = "\033[93m";
    public static final String BRIGHT_GREEN  = "\033[92m";

    // Brand colours (true-colour where a 16-colour code is insufficient)
    public static final String BG_DARK = rgb(10, 10, 12);     // #0a0a0c obsidian
    public static final String FG_LIGHT = rgb(251, 251, 251);  // #fbfbfb off-white
    public static final String VOLT    = rgb(220, 243, 101);   // #dcf365 volt accent
    public static final String DANGER  = rgb(239, 68, 68);     // #ef4444
    public static final String SUCCESS = rgb(16, 185, 129);    // #10b981

    private static boolean enabled = detect();

    private Console() {}

    private static boolean detect() {
        if (Boolean.getBoolean("urfm.noColor")) return false;
        if (System.getenv("NO_COLOR") != null) return false;
        return System.console() != null || Boolean.getBoolean("urfm.forceColor");
    }

    public static void setEnabled(boolean on) {
        enabled = on;
    }

    public static boolean isEnabled() {
        return enabled;
    }

    /** True-colour escape for an RGB triple (graceful: works in modern terminals). */
    public static String rgb(int r, int g, int b) {
        return "\033[38;2;" + r + ";" + g + ";" + b + "m";
    }

    public static String c(String code, String text) {
        return enabled ? code + text + RESET : text;
    }

    public static String bold(String t)    { return c(BOLD, t); }
    public static String dim(String t)     { return c(DIM, t); }
    public static String red(String t)     { return c(DANGER, t); }
    public static String green(String t)   { return c(SUCCESS, t); }
    public static String yellow(String t)  { return c(YELLOW, t); }
    public static String blue(String t)    { return c(BLUE, t); }
    public static String cyan(String t)    { return c(CYAN, t); }
    public static String white(String t)   { return c(FG_LIGHT, t); }

    /** Volt accent (true-colour, bright-yellow fallback). */
    public static String volt(String t) {
        return enabled ? VOLT + t + RESET : t;
    }
}
