package urfm;

/**
 * A LogListener that renders OrganizerEngine messages with colors in the CLI.
 */
public class ColoredLogger implements OrganizerEngine.LogListener {

    @Override
    public void onLog(String message) {
        System.out.println(colorize(message));
    }

    private String colorize(String m) {
        if (m.startsWith("[ERROR]")) {
            return Console.red(Console.bold("[ERROR]")) + m.substring("[ERROR]".length());
        }
        if (m.startsWith("[WARN]")) {
            return Console.yellow(Console.bold("[WARN]")) + m.substring("[WARN]".length());
        }
        if (m.startsWith("[INFO]")) {
            return Console.white("[INFO]") + m.substring("[INFO]".length());
        }
        if (m.contains("[DRY RUN]")) {
            return Console.volt(m);
        }
        if (m.startsWith("Moved:") || m.startsWith("Reverted:")) {
            return Console.green(m);
        }
        if (m.startsWith("Would move")) {
            return Console.volt(m);
        }
        return m;
    }
}
