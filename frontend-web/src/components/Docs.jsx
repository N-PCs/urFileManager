import { useEffect, useState } from 'react'
import './Docs.css'
import { IconWindows, IconLinux, IconFedora, IconUbuntu, IconBook } from './Icons'

const docSections = [
  { id: 'introduction', title: 'Introduction' },
  { id: 'cli-download', title: 'CLI Download Guide' },
  { id: 'package-managers', title: 'Package Manager Install' },
  { id: 'running-app', title: 'Running & CLI Usage' },
  { id: 'configuration', title: 'Custom Configuration' },
]

export default function Docs() {
  const [activeSection, setActiveSection] = useState('introduction')

  useEffect(() => {
    window.scrollTo(0, 0)

    const handleScroll = () => {
      const scrollPosition = window.scrollY + 120
      for (const section of docSections) {
        const el = document.getElementById(section.id)
        if (el) {
          const top = el.offsetTop
          const height = el.offsetHeight
          if (scrollPosition >= top && scrollPosition < top + height) {
            setActiveSection(section.id)
            break
          }
        }
      }
    }

    window.addEventListener('scroll', handleScroll)
    return () => window.removeEventListener('scroll', handleScroll)
  }, [])

  const handleNavClick = (id) => {
    setActiveSection(id)
    const el = document.getElementById(id)
    if (el) {
      const top = el.offsetTop - 90
      window.scrollTo({ top, behavior: 'smooth' })
    }
  }

  return (
    <div className="docs-page">
      <div className="container docs-container">
        
        {/* Navigation Sidebar / Page Markers */}
        <aside className="docs-sidebar card">
          <div className="sidebar-title-area">
            <IconBook className="sidebar-title-icon" />
            <h3>Documentation</h3>
          </div>
          <nav className="docs-nav" aria-label="Table of contents">
            <ul>
              {docSections.map((sec) => (
                <li key={sec.id}>
                  <button
                    type="button"
                    className={`docs-nav-link ${activeSection === sec.id ? 'active' : ''}`}
                    onClick={() => handleNavClick(sec.id)}
                  >
                    {sec.title}
                  </button>
                </li>
              ))}
            </ul>
          </nav>
        </aside>

        {/* Documentation Content Area */}
        <main className="docs-content">
          
          {/* Section: Introduction */}
          <section id="introduction" className="docs-sec">
            <h1 className="docs-title">urFileManager Guide</h1>
            <p className="docs-lead">
              urFileManager (urFM) is a lightning-fast bulk file organizer. It declutters folders (like Downloads or Desktop) by sorting files into well-organized categories like Documents, Images, Audio, Video, and Archives. 
            </p>
            <p className="docs-paragraph">
              This page provides clear instructions on downloading via the command line, installing package formats, and configuring sorting rules.
            </p>
            <div className="platform-badges-row">
              <span className="platform-badge"><IconWindows className="badge-icon" /> Windows (C++ GUI)</span>
              <span className="platform-badge"><IconLinux className="badge-icon" /> Linux (Java Swing GUI)</span>
            </div>
          </section>

          {/* Section: CLI Download */}
          <section id="cli-download" className="docs-sec">
            <h2 className="docs-section-title">1. CLI Download Guide</h2>
            <p className="docs-paragraph">
              You can download urFileManager releases directly from your terminal. Choose the script command for your platform:
            </p>

            <div className="docs-sub-sec">
              <h3>Windows (CMD / PowerShell)</h3>
              <p className="docs-paragraph">Download the Windows release zip without opening a browser:</p>
              
              <div className="code-block-wrapper">
                <span className="code-title">PowerShell</span>
                <pre>
                  <code>Invoke-WebRequest -Uri "https://urfilemanager.dev/urfm-windows.zip" -OutFile "urfm-windows.zip"</code>
                </pre>
              </div>

              <div className="code-block-wrapper">
                <span className="code-title">Command Prompt (CMD)</span>
                <pre>
                  <code>curl -L -o urfm-windows.zip "https://urfilemanager.dev/urfm-windows.zip"</code>
                </pre>
              </div>
            </div>

            <div className="docs-sub-sec">
              <h3>Linux (Universal Tarball)</h3>
              <p className="docs-paragraph">Download the Java GUI tarball on Ubuntu, Debian, Fedora, Arch, etc.:</p>
              
              <div className="code-block-wrapper">
                <span className="code-title">Bash Terminal</span>
                <pre>
                  <code>curl -LO "https://urfilemanager.dev/urfm-linux.tar.gz"</code>
                </pre>
              </div>
            </div>
          </section>

          {/* Section: Package Manager Install */}
          <section id="package-managers" className="docs-sec">
            <h2 className="docs-section-title">2. Package Manager Install</h2>
            <p className="docs-paragraph">
              For native system integration, desktop launch shortcuts, and automatic Java dependency handling, download and install the package matching your Linux distribution:
            </p>

            <div className="docs-card-grid">
              <div className="docs-info-card card">
                <div className="card-top">
                  <IconFedora className="info-card-icon" />
                  <h4>Fedora / RHEL (.rpm)</h4>
                </div>
                <p>Installs globally as a system package. Handles OpenJDK installation automatically.</p>
                <pre className="inline-pre">
                  <code>sudo dnf install ./urfm-1.0.0-1.noarch.rpm</code>
                </pre>
              </div>

              <div className="docs-info-card card">
                <div className="card-top">
                  <IconUbuntu className="info-card-icon" />
                  <h4>Ubuntu / Debian (.deb)</h4>
                </div>
                <p>Installs globally on Ubuntu and Debian distributions.</p>
                <pre className="inline-pre">
                  <code>sudo apt install ./urfm_1.0.0_all.deb</code>
                </pre>
              </div>
            </div>
          </section>

          {/* Section: Running the Application */}
          <section id="running-app" className="docs-sec">
            <h2 className="docs-section-title">3. Running & CLI Usage</h2>
            
            <div className="docs-sub-sec">
              <h3>Running the GUI</h3>
              <ul>
                <li><strong>Windows:</strong> Extract the ZIP and double-click <code>run.bat</code> or run <code>ufmgr.exe</code>.</li>
                <li><strong>Linux (Tarball):</strong> Extract, run <code>chmod +x urfm</code>, then run <code>./urfm</code>.</li>
                <li><strong>Linux (RPM/DEB):</strong> Launch "urFileManager" from your system app menu, or run <code>urfm</code> in a terminal.</li>
              </ul>
            </div>

            <div className="docs-sub-sec">
              <h3>Command-line Interface (CLI)</h3>
              <p className="docs-paragraph">
                Organize directories from the command line by supplying the target path. Use flags for advanced controls:
              </p>
              
              <div className="table-responsive">
                <table className="docs-table">
                  <thead>
                    <tr>
                      <th>Flag</th>
                      <th>Description</th>
                      <th>Example Command</th>
                    </tr>
                  </thead>
                  <tbody>
                    <tr>
                      <td><code>--dry-run</code></td>
                      <td>Runs simulation. Outputs files that would be moved without editing the filesystem.</td>
                      <td><code>urfm ~/Downloads --dry-run</code></td>
                    </tr>
                    <tr>
                      <td><code>--revert</code></td>
                      <td>Undoes the last file reorganization on the folder by moving files back to their original places.</td>
                      <td><code>urfm ~/Downloads --revert</code></td>
                    </tr>
                    <tr>
                      <td><code>--version</code></td>
                      <td>Prints the app version.</td>
                      <td><code>urfm --version</code></td>
                    </tr>
                  </tbody>
                </table>
              </div>
            </div>
          </section>

          {/* Section: Custom Configuration */}
          <section id="configuration" className="docs-sec">
            <h2 className="docs-section-title">4. Custom Configuration</h2>
            <p className="docs-paragraph">
              Both the C++ and Java versions read sorting definitions from <code>config.json</code> (placed in the same folder as `urfm.jar` or in `/opt/urfm/`). You can customize destination folders and file extension mappings.
            </p>
            
            <div className="code-block-wrapper">
              <span className="code-title">config.json (Example)</span>
              <pre>
                <code>{`{
  "categories": {
    "Images": [".jpg", ".jpeg", ".png", ".gif", ".webp", ".svg"],
    "Documents": [".pdf", ".docx", ".doc", ".txt", ".xlsx", ".pptx"],
    "Audio": [".mp3", ".wav", ".aac", ".flac", ".m4a"],
    "Video": [".mp4", ".mkv", ".mov", ".avi", ".webm"],
    "Archives": [".zip", ".tar.gz", ".rar", ".7z", ".tar"]
  },
  "default_folder": "Other"
}`}</code>
              </pre>
            </div>
            <div className="docs-tip">
              <strong>Tip:</strong> If you add custom file extensions, restart the application or re-run the CLI command to reload the rules instantly!
            </div>
          </section>

        </main>
      </div>
    </div>
  )
}
