import { useState, useEffect, useCallback } from 'react'
import './Download.css'
import { IconWindows, IconLinux, IconCheck, IconDownload, IconTerminal, IconFedora, IconUbuntu } from './Icons'
import {
  VERSION,
  platforms,
  formatBytes,
  getDownloadUrl,
  getCliCommands,
} from '../data/downloads'

const platformIcons = {
  windows: IconWindows,
  linux: IconLinux,
}

const formatIcons = {
  rpm: IconFedora,
  deb: IconUbuntu,
  tarball: IconLinux,
}

function CopyButton({ text, label }) {
  const [copied, setCopied] = useState(false)

  const handleCopy = useCallback(async () => {
    try {
      await navigator.clipboard.writeText(text)
      setCopied(true)
      setTimeout(() => setCopied(false), 2000)
    } catch {
      /* clipboard may be blocked */
    }
  }, [text])

  return (
    <button type="button" className="copy-btn" onClick={handleCopy} title={`Copy ${label}`}>
      {copied ? 'Copied!' : 'Copy'}
    </button>
  )
}

function WindowsCard({ platform, manifest, origin, onSelect }) {
  const Icon = platformIcons[platform.icon]
  const size = manifest?.files?.find((f) => f.name === platform.zipName)?.size ?? null
  const href = getDownloadUrl(platform.zipName, origin)

  return (
    <div className="download-platform-card card">
      <div className="platform-top">
        <Icon className="platform-icon" />
        <h3 className="platform-name">{platform.name}</h3>
        {size != null && <span className="platform-size">{formatBytes(size)}</span>}
      </div>
      <p className="platform-desc">{platform.description}</p>

      <ul className="package-list">
        {platform.contents.map((item) => (
          <li key={item}>
            <IconCheck className="check-icon" />
            {item}
          </li>
        ))}
      </ul>

      <a
        href={href}
        download={platform.zipName}
        className="btn btn-primary btn-lg download-btn"
        onClick={() => onSelect(platform.id)}
      >
        <IconDownload className="btn-icon" />
        Download v{VERSION}
        {size != null && <span className="btn-size"> ({formatBytes(size)})</span>}
      </a>
      <p className="download-note">{platform.note}</p>

      <details className="platform-details">
        <summary>System requirements & usage</summary>
        <div className="sys-req card">
          <dl>
            {Object.entries(platform.sysReqs).map(([k, v]) => (
              <div key={k}>
                <dt>{k}</dt>
                <dd>{v}</dd>
              </div>
            ))}
          </dl>
        </div>
        <div className="cli-preview card">
          <h4>After extracting</h4>
          <pre>
            <code>{platform.usageCli.join('\n')}</code>
          </pre>
        </div>
      </details>
    </div>
  )
}

function LinuxCard({ platform, manifest, origin, onSelect }) {
  const [selectedFormat, setSelectedFormat] = useState('tarball')
  const Icon = platformIcons[platform.icon]
  const format = platform.formats.find((f) => f.id === selectedFormat) ?? platform.formats[0]
  const FormatIcon = formatIcons[format.id] || IconLinux
  const size = manifest?.files?.find((f) => f.name === format.zipName)?.size ?? null
  const href = getDownloadUrl(format.zipName, origin)

  return (
    <div className="download-platform-card card linux-card">
      <div className="platform-top">
        <Icon className="platform-icon" />
        <h3 className="platform-name">{platform.name}</h3>
      </div>
      <p className="platform-desc">{platform.description}</p>

      {/* Format Selector */}
      <div className="linux-format-selector">
        <span className="format-label">Package format</span>
        <div className="format-tabs">
          {platform.formats.map((f) => {
            const FIcon = formatIcons[f.id] || IconLinux
            return (
              <button
                key={f.id}
                type="button"
                className={`format-tab ${selectedFormat === f.id ? 'active' : ''}`}
                onClick={() => setSelectedFormat(f.id)}
              >
                <FIcon className="format-tab-icon" />
                {f.label}
              </button>
            )
          })}
        </div>
      </div>

      {/* Format Details */}
      <div className="format-detail-area">
        <div className="format-distro-badge">
          <FormatIcon className="format-distro-icon" />
          <span>{format.distro}</span>
          {size != null && <span className="platform-size">{formatBytes(size)}</span>}
        </div>

        <ul className="package-list">
          {format.contents.map((item) => (
            <li key={item}>
              <IconCheck className="check-icon" />
              {item}
            </li>
          ))}
        </ul>

        <a
          href={href}
          download={format.zipName}
          className="btn btn-primary btn-lg download-btn"
          onClick={() => onSelect('linux')}
        >
          <IconDownload className="btn-icon" />
          Download v{VERSION}
          {size != null && <span className="btn-size"> ({formatBytes(size)})</span>}
        </a>
        <p className="download-note">{format.note}</p>

        <details className="platform-details">
          <summary>System requirements & usage</summary>
          <div className="sys-req card">
            <dl>
              {Object.entries(format.sysReqs).map(([k, v]) => (
                <div key={k}>
                  <dt>{k}</dt>
                  <dd>{v}</dd>
                </div>
              ))}
            </dl>
          </div>
          <div className="cli-preview card">
            <h4>After installing</h4>
            <pre>
              <code>{format.usageCli.join('\n')}</code>
            </pre>
          </div>
        </details>
      </div>
    </div>
  )
}

export default function Download() {
  const [manifest, setManifest] = useState(null)
  const [manifestError, setManifestError] = useState(false)
  const [activeCliTab, setActiveCliTab] = useState('bash')
  const [selectedPlatform, setSelectedPlatform] = useState('linux')

  const origin = typeof window !== 'undefined' ? window.location.origin : ''

  useEffect(() => {
    fetch('/downloads.json')
      .then((res) => {
        if (!res.ok) throw new Error('manifest missing')
        return res.json()
      })
      .then((data) => setManifest(data))
      .catch(() => setManifestError(true))
  }, [])

  // For the CLI section, get the right zipName based on selected platform
  const getSelectedZipName = () => {
    const plat = platforms.find((p) => p.id === selectedPlatform)
    if (!plat) return 'urfm-linux.tar.gz'
    if (plat.zipName) return plat.zipName
    if (plat.formats) return plat.formats[0].zipName
    return 'urfm-linux.tar.gz'
  }

  const cliCommands = getCliCommands(getSelectedZipName(), origin)
  const cliTabs = [
    { id: 'bash', label: 'Bash' },
    { id: 'wget', label: 'wget' },
    { id: 'powershell', label: 'PowerShell' },
    { id: 'cmd', label: 'CMD (curl)' },
  ]

  const windowsPlatform = platforms.find((p) => p.id === 'windows')
  const linuxPlatform = platforms.find((p) => p.id === 'linux')

  return (
    <section id="download" className="section download">
      <div className="container">
        <div className="download-header">
          <span className="section-label" style={{color:"var(--primary)",fontSize:"1.0rem"}}>Download</span>
          <h2 className="section-title">Available for Windows & Linux</h2>
          <p className="section-subtitle">
            Native C++ on Windows, Java Swing on Linux — Tarball, Fedora RPM, or Ubuntu DEB.
            {manifest?.generated && (
              <span className="manifest-date">
                {' '}
                · Packages built {new Date(manifest.generated).toLocaleDateString()}
              </span>
            )}
          </p>
        </div>

        {manifestError && (
          <div className="download-alert" role="alert">
            Release files are not deployed yet. Run{' '}
            <code>./scripts/package-release.sh</code> from the repo root, commit the files in{' '}
            <code>frontend-web/public/</code>, then redeploy.
          </div>
        )}

        <div className="download-grid">
          {windowsPlatform && (
            <WindowsCard
              platform={windowsPlatform}
              manifest={manifest}
              origin={origin}
              onSelect={setSelectedPlatform}
            />
          )}
          {linuxPlatform && (
            <LinuxCard
              platform={linuxPlatform}
              manifest={manifest}
              origin={origin}
              onSelect={setSelectedPlatform}
            />
          )}
        </div>

        <div className="cli-download-section card">
          <div className="cli-download-header">
            <IconTerminal className="cli-section-icon" />
            <div>
              <h3>Download via command line</h3>
              <p>
                Grab the release from any terminal — no browser needed.
              </p>
            </div>
          </div>

          <div className="cli-platform-picker">
            {platforms.map((p) => (
              <button
                key={p.id}
                type="button"
                className={`cli-platform-btn ${selectedPlatform === p.id ? 'active' : ''}`}
                onClick={() => setSelectedPlatform(p.id)}
              >
                {p.name}
              </button>
            ))}
          </div>

          <div className="cli-tabs" role="tablist">
            {cliTabs.map((tab) => (
              <button
                key={tab.id}
                type="button"
                role="tab"
                aria-selected={activeCliTab === tab.id}
                className={`cli-tab ${activeCliTab === tab.id ? 'active' : ''}`}
                onClick={() => setActiveCliTab(tab.id)}
              >
                {tab.label}
              </button>
            ))}
          </div>

          <div className="cli-command-block">
            <pre>
              <code>{cliCommands[activeCliTab]}</code>
            </pre>
            <CopyButton text={cliCommands[activeCliTab]} label={activeCliTab} />
          </div>

          <p className="cli-hint">
            After download, extract the archive and run the commands shown under each platform card.
          </p>
        </div>
      </div>
    </section>
  )
}
