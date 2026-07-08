package urfm;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.*;

public class ConfigParser {
    /**
     * Parses the config.json file into a map of Categories to List of File Extensions.
     * Implements a simple, dependency-free JSON parser.
     */
    public static Map<String, List<String>> parse(Path configPath) throws IOException {
        String content = Files.readString(configPath);
        Map<String, List<String>> map = new LinkedHashMap<>();

        content = content.trim();
        if (!content.startsWith("{") || !content.endsWith("}")) {
            throw new IOException("Invalid JSON config format: must start with { and end with }");
        }

        String body = content.substring(1, content.length() - 1);
        int i = 0;
        while (i < body.length()) {
            char c = body.charAt(i);
            if (c == '"') {
                // Read Category Key
                int start = i + 1;
                int end = body.indexOf('"', start);
                if (end == -1) break;
                String key = body.substring(start, end);
                i = end + 1;

                // Find colon ':'
                while (i < body.length() && body.charAt(i) != ':') {
                    i++;
                }
                i++; // skip ':'

                // Find array start '['
                while (i < body.length() && body.charAt(i) != '[') {
                    i++;
                }
                i++; // skip '['

                List<String> list = new ArrayList<>();
                // Read extensions inside the array
                while (i < body.length()) {
                    while (i < body.length() && body.charAt(i) != '"' && body.charAt(i) != ']') {
                        i++;
                    }
                    if (i >= body.length()) break;
                    if (body.charAt(i) == ']') {
                        i++;
                        break; // end of array
                    }
                    if (body.charAt(i) == '"') {
                        int extStart = i + 1;
                        int extEnd = body.indexOf('"', extStart);
                        if (extEnd == -1) break;
                        list.add(body.substring(extStart, extEnd).toLowerCase());
                        i = extEnd + 1;
                    }
                }
                map.put(key, list);
            } else {
                i++;
            }
        }
        return map;
    }
}
