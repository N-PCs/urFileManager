export const VERSION = '1.0.0'

export const platforms = [
  {
    id: 'windows',
    name: 'Windows',
    zipName: 'urfm-windows.zip',
    archiveType: 'zip',
    description: 'Native Win32 GUI — no dependencies, no admin rights required.',
    contents: [
      'ufmgr.exe — Native Windows application',
      'config.json — Customizable sorting rules',
      'run.bat — Launch the GUI instantly',
      'ufmgr.bat — CLI wrapper for scripting',
      'README.txt — Quick start guide',
    ],
    note: 'Portable ZIP · Windows 10 or later · x64',
    sysReqs: {
      OS: 'Windows 10 / 11',
      Architecture: '64-bit (x64)',
      Dependencies: 'None',
      'Install type': 'Portable',
    },
    usageCli: ['.\\run.bat', '.\\ufmgr.bat C:\\Downloads --dry-run'],
  },
  {
    id: 'linux',
    name: 'Linux',
    zipName: 'urfm-linux.tar.gz',
    archiveType: 'tarball',
    description: 'Java Swing GUI with a terminal-inspired aesthetic. Ships as a ready-to-run JAR.',
    contents: [
      'urfm.jar — Executable Java archive (Swing GUI + CLI)',
      'urfm — Convenience launcher script',
      'config.json — Customizable sorting rules',
      'README.txt — Quick start guide',
    ],
    note: 'Java JAR · Fedora / Ubuntu / Arch · x64',
    sysReqs: {
      OS: 'Fedora / Ubuntu / Arch',
      Architecture: '64-bit (x64)',
      Dependencies: 'Java Runtime 17+',
      'Install type': 'Download & run',
    },
    usageCli: [
      'sudo apt install openjdk-17-jre   # Ubuntu',
      'sudo dnf install java-17-openjdk      # Fedora',
      'chmod +x urfm && ./urfm ~/Downloads --dry-run',
    ],
  },
]

export function formatBytes(bytes) {
  if (!bytes || bytes <= 0) return null
  if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)} KB`
  return `${(bytes / (1024 * 1024)).toFixed(2)} MB`
}

export function getDownloadUrl(zipName, origin = '') {
  const base = origin || (typeof window !== 'undefined' ? window.location.origin : '')
  return `${base}/${zipName}`
}

export function getCliCommands(zipName, origin = '') {
  const url = getDownloadUrl(zipName, origin)
  return {
    powershell: `Invoke-WebRequest -Uri "${url}" -OutFile "${zipName}"`,
    cmd: `curl -L -o "${zipName}" "${url}"`,
    bash: `curl -LO "${url}"`,
    wget: `wget -O "${zipName}" "${url}"`,
  }
}
