export const VERSION = '1.0.0'

export const platforms = [
  {
    id: 'windows',
    name: 'Windows',
    icon: 'windows',
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
    icon: 'linux',
    description: 'Java Swing GUI with a terminal-inspired aesthetic. Choose your preferred package format below.',
    formats: [
      {
        id: 'tarball',
        label: 'Tarball (.tar.gz)',
        distro: 'Universal',
        zipName: 'urfm-linux.tar.gz',
        archiveType: 'tarball',
        contents: [
          'urfm.jar — Executable Java archive (Swing GUI + CLI)',
          'urfm — Convenience launcher script',
          'config.json — Customizable sorting rules',
          'README.txt — Quick start guide',
        ],
        note: 'Java JAR · Any Linux distro · x64',
        sysReqs: {
          OS: 'Any Linux distro',
          Architecture: '64-bit (x64)',
          Dependencies: 'Java Runtime 17+',
          'Install type': 'Extract & run',
        },
        usageCli: [
          'tar -xzf urfm-linux.tar.gz',
          'chmod +x urfm && ./urfm ~/Downloads --dry-run',
        ],
      },
      {
        id: 'rpm',
        label: 'Fedora / RHEL (.rpm)',
        distro: 'Fedora',
        zipName: 'urfm-1.0.0-1.noarch.rpm',
        archiveType: 'rpm',
        contents: [
          '/opt/urfm/urfm.jar — Java GUI application',
          '/opt/urfm/config.json — Customizable sorting rules',
          '/usr/local/bin/urfm — Global launcher',
          'urfm.desktop — App menu entry',
          'urfm.svg — Application icon',
        ],
        note: 'RPM Package · Fedora 38+ / RHEL 9+ · noarch',
        sysReqs: {
          OS: 'Fedora 38+ / RHEL 9+',
          Architecture: 'Any (noarch)',
          Dependencies: 'Java 17+ (auto-installed)',
          'Install type': 'System package (dnf)',
        },
        usageCli: [
          'sudo dnf install ./urfm-1.0.0-1.noarch.rpm',
          'urfm ~/Downloads --dry-run',
        ],
      },
      {
        id: 'deb',
        label: 'Ubuntu / Debian (.deb)',
        distro: 'Ubuntu',
        zipName: 'urfm_1.0.0_all.deb',
        archiveType: 'deb',
        contents: [
          '/opt/urfm/urfm.jar — Java GUI application',
          '/opt/urfm/config.json — Customizable sorting rules',
          '/usr/local/bin/urfm — Global launcher',
          'urfm.desktop — App menu entry',
          'urfm.svg — Application icon',
        ],
        note: 'DEB Package · Ubuntu 22.04+ / Debian 12+ · all',
        sysReqs: {
          OS: 'Ubuntu 22.04+ / Debian 12+',
          Architecture: 'Any (all)',
          Dependencies: 'Java 17+ (openjdk-17-jre)',
          'Install type': 'System package (apt)',
        },
        usageCli: [
          'sudo apt install ./urfm_1.0.0_all.deb',
          'urfm ~/Downloads --dry-run',
        ],
      },
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
