package urfm;

import javax.swing.*;
import javax.swing.border.LineBorder;
import javax.swing.border.TitledBorder;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.JTableHeader;
import java.awt.*;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.Map;

public class UrfmGUI extends JFrame {

    private JTextField pathField;
    private JCheckBox dryRunCheck;
    private JComboBox<String> themeCombo;
    private JButton organizeBtn;
    private JButton revertBtn;
    private JLabel statusLabel;
    private JTable reportTable;
    private DefaultTableModel tableModel;
    private JButton openPdfBtn;
    private JTextArea logArea;
    private JTabbedPane tabbedPane;

    private Color currentBg;
    private Color currentFg;
    private Color currentCompBg;
    private Color currentBorderCol;
    
    private Path currentTargetFolder;
    private boolean lastDryRun = true;

    private Map<String, List<String>> fileTypeMap;

    public UrfmGUI(Map<String, List<String>> fileTypeMap) {
        this.fileTypeMap = fileTypeMap;
        setTitle("urFileManager — Bulk File Organizer");
        setSize(750, 520);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null);
        
        // Main Font (Monospaced for terminal vibe)
        Font monoFont = new Font("Monospaced", Font.PLAIN, 13);
        UIManager.put("Label.font", monoFont);
        UIManager.put("Button.font", monoFont);
        UIManager.put("TextField.font", monoFont);
        UIManager.put("TextArea.font", monoFont);
        UIManager.put("Table.font", monoFont);
        UIManager.put("TableHeader.font", monoFont);
        UIManager.put("ComboBox.font", monoFont);
        UIManager.put("CheckBox.font", monoFont);
        UIManager.put("TabbedPane.font", monoFont);

        initComponents();
        
        // Apply the "Volt" theme (matches the frontend-web brand) by default
        applyTheme("Volt");
    }

    private void initComponents() {
        tabbedPane = new JTabbedPane();

        // ---------------- TAB 1: ORGANIZE ----------------
        JPanel organizeTab = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(10, 10, 10, 10);
        gbc.fill = GridBagConstraints.HORIZONTAL;

        // Path Selector Panel
        JPanel pathPanel = new JPanel(new BorderLayout(10, 0));
        pathPanel.setBorder(BorderFactory.createTitledBorder(
                BorderFactory.createLineBorder(Color.GRAY), "1. Target Directory",
                TitledBorder.LEFT, TitledBorder.TOP, null, null
        ));
        pathField = new JTextField();
        JButton browseBtn = createStyledButton("Browse");
        browseBtn.addActionListener(e -> {
            JFileChooser chooser = new JFileChooser();
            chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
            int ret = chooser.showOpenDialog(this);
            if (ret == JFileChooser.APPROVE_OPTION) {
                pathField.setText(chooser.getSelectedFile().getAbsolutePath());
            }
        });
        pathPanel.add(pathField, BorderLayout.CENTER);
        pathPanel.add(browseBtn, BorderLayout.EAST);

        gbc.gridx = 0;
        gbc.gridy = 0;
        gbc.weightx = 1.0;
        organizeTab.add(pathPanel, gbc);

        // Options Panel
        JPanel optionsPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 15, 10));
        optionsPanel.setBorder(BorderFactory.createTitledBorder(
                BorderFactory.createLineBorder(Color.GRAY), "2. Options",
                TitledBorder.LEFT, TitledBorder.TOP, null, null
        ));
        dryRunCheck = new JCheckBox("Dry-Run Preview (simulates file moves safely)", true);
        
        themeCombo = new JComboBox<>(new String[]{
                "Volt", "Amber Monitor", "White on Dark", "Blue Matrix", "Red Alert"
        });
        themeCombo.addActionListener(e -> {
            String theme = (String) themeCombo.getSelectedItem();
            applyTheme(theme);
        });
        
        optionsPanel.add(dryRunCheck);
        optionsPanel.add(new JLabel("Theme:"));
        optionsPanel.add(themeCombo);

        gbc.gridx = 0;
        gbc.gridy = 1;
        organizeTab.add(optionsPanel, gbc);

        // Actions Panel
        JPanel actionPanel = new JPanel(new GridLayout(1, 2, 20, 0));
        organizeBtn = createStyledButton("EXECUTE RUN");
        revertBtn = createStyledButton("REVERT LAST RUN");
        actionPanel.add(organizeBtn);
        actionPanel.add(revertBtn);

        gbc.gridx = 0;
        gbc.gridy = 2;
        organizeTab.add(actionPanel, gbc);

        // Status Label Panel
        statusLabel = new JLabel("$ status: IDLE (waiting for command)");
        JPanel statusPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        statusPanel.setBorder(BorderFactory.createLineBorder(Color.GRAY));
        statusPanel.add(statusLabel);

        gbc.gridx = 0;
        gbc.gridy = 3;
        organizeTab.add(statusPanel, gbc);

        // ---------------- TAB 2: REPORT ----------------
        JPanel reportTab = new JPanel(new BorderLayout(0, 10));
        tableModel = new DefaultTableModel(
                new Object[][]{},
                new String[]{"File Name", "Category", "Size", "Status"}
        ) {
            @Override
            public boolean isCellEditable(int row, int column) {
                return false;
            }
        };
        reportTable = new JTable(tableModel);
        reportTable.setRowHeight(22);
        reportTable.setShowGrid(true);
        reportTable.setGridColor(Color.GRAY);
        reportTable.setDefaultRenderer(Object.class, new StatusCellRenderer());

        JScrollPane tableScroll = new JScrollPane(reportTable);
        reportTab.add(tableScroll, BorderLayout.CENTER);

        openPdfBtn = createStyledButton("OPEN PDF REPORT");
        openPdfBtn.setEnabled(false);
        openPdfBtn.addActionListener(e -> {
            if (currentTargetFolder != null) {
                String reportName = lastDryRun ? "organization_report_preview.pdf" : "organization_report.pdf";
                Path reportPath = currentTargetFolder.resolve(reportName);
                if (Files.exists(reportPath)) {
                    try {
                        Desktop.getDesktop().open(reportPath.toFile());
                    } catch (IOException ex) {
                        appendLog("[ERROR] Could not open PDF report: " + ex.getMessage());
                    }
                } else {
                    appendLog("[ERROR] Report file not found: " + reportPath);
                }
            }
        });
        reportTab.add(openPdfBtn, BorderLayout.SOUTH);

        // ---------------- TAB 3: LOG ----------------
        JPanel logTab = new JPanel(new BorderLayout());
        logArea = new JTextArea();
        logArea.setEditable(false);
        logArea.setLineWrap(true);
        logArea.setWrapStyleWord(true);
        JScrollPane logScroll = new JScrollPane(logArea);
        logTab.add(logScroll, BorderLayout.CENTER);

        tabbedPane.addTab("  ORGANIZE  ", organizeTab);
        tabbedPane.addTab("  PREVIEW / REPORT  ", reportTab);
        tabbedPane.addTab("  CONSOLE LOG  ", logTab);

        add(tabbedPane);

        JLabel footerLabel = new JLabel("github.com/N-PCs/urFileManager");
        footerLabel.setHorizontalAlignment(SwingConstants.CENTER);
        footerLabel.setFont(new Font("Monospaced", Font.PLAIN, 10));
        add(footerLabel, BorderLayout.SOUTH);

        // Setup event handlers
        organizeBtn.addActionListener(e -> startOrganization());
        revertBtn.addActionListener(e -> startRevert());
    }

    private JButton createStyledButton(String text) {
        JButton btn = new JButton(text);
        btn.setFocusPainted(false);
        btn.setContentAreaFilled(false);
        btn.setOpaque(true);
        btn.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseEntered(MouseEvent e) {
                if (btn.isEnabled()) {
                    btn.setBackground(currentCompBg.brighter());
                }
            }

            @Override
            public void mouseExited(MouseEvent e) {
                if (btn.isEnabled()) {
                    btn.setBackground(currentCompBg);
                }
            }
        });
        return btn;
    }

    private void appendLog(String message) {
        logArea.append(message + "\n");
        logArea.setCaretPosition(logArea.getDocument().getLength());
    }

    private void startOrganization() {
        String pathStr = pathField.getText().trim();
        if (pathStr.isEmpty()) {
            JOptionPane.showMessageDialog(this, "Please enter or select a directory.", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }

        Path sourceDir = Path.of(pathStr);
        if (!Files.exists(sourceDir) || !Files.isDirectory(sourceDir)) {
            JOptionPane.showMessageDialog(this, "The specified path is not a valid directory.", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }

        currentTargetFolder = sourceDir;
        lastDryRun = dryRunCheck.isSelected();

        // Clear previous report
        tableModel.setRowCount(0);
        logArea.setText("");
        openPdfBtn.setEnabled(false);

        organizeBtn.setEnabled(false);
        revertBtn.setEnabled(false);
        statusLabel.setText("$ status: RUNNING...");
        
        tabbedPane.setSelectedIndex(2); // Switch to Log tab to show progress

        // Run organization in background
        new Thread(() -> {
            List<Map<String, Object>> files = OrganizerEngine.organize(
                    sourceDir,
                    lastDryRun,
                    fileTypeMap,
                    this::appendLog
            );

            SwingUtilities.invokeLater(() -> {
                // Populate Table
                for (Map<String, Object> file : files) {
                    long size = ((Number) file.get("fileSize")).longValue();
                    String sizeStr = formatSize(size);
                    tableModel.addRow(new Object[]{
                            file.get("fileName"),
                            file.get("category"),
                            sizeStr,
                            file.get("status")
                    });
                }

                organizeBtn.setEnabled(true);
                revertBtn.setEnabled(true);
                
                String reportName = lastDryRun ? "organization_report_preview.pdf" : "organization_report.pdf";
                Path reportPath = sourceDir.resolve(reportName);
                if (Files.exists(reportPath)) {
                    openPdfBtn.setEnabled(true);
                }

                if (lastDryRun) {
                    statusLabel.setText("$ status: DRY RUN COMPLETE (preview generated)");
                    tabbedPane.setSelectedIndex(1); // Switch to report preview
                } else {
                    statusLabel.setText("$ status: LIVE RUN COMPLETE (files organized)");
                    tabbedPane.setSelectedIndex(1);
                }
            });
        }).start();
    }

    private void startRevert() {
        String pathStr = pathField.getText().trim();
        if (pathStr.isEmpty()) {
            JOptionPane.showMessageDialog(this, "Please enter or select a directory.", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }

        Path sourceDir = Path.of(pathStr);
        if (!Files.exists(sourceDir) || !Files.isDirectory(sourceDir)) {
            JOptionPane.showMessageDialog(this, "The specified path is not a valid directory.", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }

        organizeBtn.setEnabled(false);
        revertBtn.setEnabled(false);
        statusLabel.setText("$ status: REVERTING...");
        
        tabbedPane.setSelectedIndex(2);

        new Thread(() -> {
            OrganizerEngine.revert(sourceDir, this::appendLog);
            SwingUtilities.invokeLater(() -> {
                organizeBtn.setEnabled(true);
                revertBtn.setEnabled(true);
                statusLabel.setText("$ status: REVERT OPERATION COMPLETE");
            });
        }).start();
    }

    private String formatSize(long bytesVal) {
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
        return String.format(java.util.Locale.US, "%.2f %s", size, unit);
    }

    private void applyTheme(String themeName) {
        if ("Amber Monitor".equals(themeName)) {
            currentBg = new Color(0x14, 0x0d, 0x02);
            currentFg = new Color(0xff, 0xb0, 0x00);
            currentCompBg = new Color(0x24, 0x18, 0x04);
            currentBorderCol = new Color(0xff, 0xb0, 0x00);
        } else if ("White on Dark".equals(themeName)) {
            currentBg = new Color(0x12, 0x12, 0x12);
            currentFg = new Color(0xff, 0xff, 0xff);
            currentCompBg = new Color(0x1e, 0x1e, 0x1e);
            currentBorderCol = new Color(0x88, 0x88, 0x88);
        } else if ("Blue Matrix".equals(themeName)) {
            currentBg = new Color(0x08, 0x10, 0x1a);
            currentFg = new Color(0x00, 0xa8, 0xff);
            currentCompBg = new Color(0x11, 0x22, 0x33);
            currentBorderCol = new Color(0x00, 0xa8, 0xff);
        } else if ("Red Alert".equals(themeName)) {
            currentBg = new Color(0x15, 0x05, 0x05);
            currentFg = new Color(0xff, 0x33, 0x33);
            currentCompBg = new Color(0x25, 0x0a, 0x0a);
            currentBorderCol = new Color(0xff, 0x33, 0x33);
        } else { // Volt (default) — matches frontend-web brand: obsidian, off-white, volt accent
            currentBg = new Color(0x0a, 0x0a, 0x0c);   // #0a0a0c obsidian
            currentFg = new Color(0xfb, 0xfb, 0xfb);   // #fbfbfb off-white
            currentCompBg = new Color(0x12, 0x12, 0x14); // #121214 elevated panel
            currentBorderCol = new Color(0xdc, 0xf3, 0x65); // #dcf365 volt accent
        }

        // Apply theme recursively
        styleComponent(getRootPane());
        
        // Repaint components
        SwingUtilities.updateComponentTreeUI(this);
    }

    private void styleComponent(Component c) {
        c.setBackground(currentBg);
        c.setForeground(currentFg);

        if (c instanceof JPanel) {
            JPanel panel = (JPanel) c;
            panel.setBackground(currentBg);
            if (panel.getBorder() instanceof TitledBorder) {
                TitledBorder tb = (TitledBorder) panel.getBorder();
                tb.setTitleColor(currentFg);
                tb.setBorder(new LineBorder(currentBorderCol, 1));
            }
        } else if (c instanceof JButton) {
            JButton btn = (JButton) c;
            btn.setBackground(currentCompBg);
            btn.setForeground(currentFg);
            btn.setBorder(new LineBorder(currentBorderCol, 1));
        } else if (c instanceof JTextField) {
            JTextField tf = (JTextField) c;
            tf.setBackground(currentCompBg);
            tf.setForeground(currentFg);
            tf.setCaretColor(currentFg);
            tf.setBorder(new LineBorder(currentBorderCol, 1));
        } else if (c instanceof JTextArea) {
            JTextArea ta = (JTextArea) c;
            ta.setBackground(currentCompBg);
            ta.setForeground(currentFg);
            ta.setCaretColor(currentFg);
            ta.setBorder(new LineBorder(currentBorderCol, 1));
        } else if (c instanceof JComboBox) {
            JComboBox<?> cb = (JComboBox<?>) c;
            cb.setBackground(currentCompBg);
            cb.setForeground(currentFg);
            cb.setBorder(new LineBorder(currentBorderCol, 1));
        } else if (c instanceof JCheckBox) {
            JCheckBox cb = (JCheckBox) c;
            cb.setBackground(currentBg);
            cb.setForeground(currentFg);
        } else if (c instanceof JTable) {
            JTable tbl = (JTable) c;
            tbl.setBackground(currentCompBg);
            tbl.setForeground(currentFg);
            tbl.setGridColor(currentBorderCol);
            tbl.setSelectionBackground(currentFg);
            tbl.setSelectionForeground(currentBg);
            
            JTableHeader header = tbl.getTableHeader();
            header.setBackground(currentCompBg);
            header.setForeground(currentFg);
            header.setBorder(new LineBorder(currentBorderCol, 1));
        } else if (c instanceof JTabbedPane) {
            JTabbedPane jtp = (JTabbedPane) c;
            jtp.setBackground(currentCompBg);
            jtp.setForeground(currentFg);
        } else if (c instanceof JLabel) {
            c.setForeground(currentFg);
        }

        if (c instanceof Container) {
            Container container = (Container) c;
            for (Component child : container.getComponents()) {
                styleComponent(child);
            }
        }
    }

    private class StatusCellRenderer extends DefaultTableCellRenderer {
        @Override
        public Component getTableCellRendererComponent(
                JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column
        ) {
            Component c = super.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, column);
            c.setBackground(currentCompBg);
            c.setForeground(currentFg);

            if (column == 3 && value != null) {
                String val = value.toString();
                if ("Moved".equals(val)) {
                    c.setForeground(new Color(0x00, 0xbb, 0x00));
                } else if ("Renamed".equals(val) || val.contains("Conflict")) {
                    c.setForeground(new Color(0xff, 0xa5, 0x00));
                } else if (val.contains("Error") || val.contains("Permission")) {
                    c.setForeground(new Color(0xff, 0x33, 0x33));
                } else if ("Dry Run Preview".equals(val)) {
                    c.setForeground(new Color(0x00, 0xa8, 0xff));
                }
            }

            if (isSelected) {
                c.setBackground(currentFg);
                c.setForeground(currentBg);
            }
            return c;
        }
    }
}
