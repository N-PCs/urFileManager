import { useState, useEffect, useCallback } from 'react'
import './Download.css'
import { IconWindows, IconCheck, IconDownload, IconTerminal } from './Icons'
import {
  VERSION,
  platforms,
  formatBytes,
  getDownloadUrl,
  getCliCommands,
} from '../data/downloads'

const platformIcons = {
  windows: IconWindows,
  linux: IconDownload,
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

export default function Download() {
  const [manifest, setManifest] = useState(null)
  const [manifestError, setManifestError] = useState(false)
  const [activeCliTab, setActiveCliTab] = useState('powershell')
  const [selectedPlatform, setSelectedPlatform] = useState('windows')

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

  const getFileSize = (zipName) => {
    const entry = manifest?.files?.find((f) => f.name === zipName)
    return entry?.size ?? null
  }

  const isAvailable = (zipName) => {
    if (manifestError) return false
    if (!manifest) return null
    return manifest.files?.some((f) => f.name === zipName)
  }

  const selected = platforms.find((p) => p.id === selectedPlatform) ?? platforms[0]
  const cliCommands = getCliCommands(selected.zipName, origin)
  const cliTabs = [
    { id: 'powershell', label: 'PowerShell' },
    { id: 'cmd', label: 'CMD (curl)' },
    { id: 'bash', label: 'Bash' },
    { id: 'wget', label: 'wget' },
  ]

  return (
    <section id="download" className="section download">
      <div className="container">
        <div className="download-header">
          <span className="section-label" style={{color:"var(--primary)",fontSize:"1.0rem"}}>Download</span>
          <h2 className="section-title">Available for Windows & Linux</h2>
          <p className="section-subtitle">
            Native C++ on Windows, Java Swing on Linux. No runtime required.
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
            <code>.\scripts\package-release.ps1</code> from the repo root, commit the files in{' '}
            <code>frontend-web/public/</code>, then redeploy.
          </div>
        )}

        <div className="download-grid">
          {platforms.map((platform) => {
            const Icon = platformIcons[platform.id]
            const available = isAvailable(platform.zipName)
            const size = getFileSize(platform.zipName)
            const href = getDownloadUrl(platform.zipName, origin)

            return (
              <div key={platform.id} className="download-platform-card card">
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

                {available === false ? (
                  <button type="button" className="btn btn-primary btn-lg download-btn" disabled>
                    Not available on site
                  </button>
                ) : (
                  <a
                    href={href}
                    download={platform.zipName}
                    className="btn btn-primary btn-lg download-btn"
                    onClick={() => setSelectedPlatform(platform.id)}
                  >
                    <IconDownload className="btn-icon" />
                    Download v{VERSION}
                    {size != null && <span className="btn-size"> ({formatBytes(size)})</span>}
                  </a>
                )}
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
          })}
        </div>

        <div className="cli-download-section card">
          <div className="cli-download-header">
            <IconTerminal className="cli-section-icon" />
            <div>
              <h3>Download via command line</h3>
              <p>
                Grab the release from PowerShell, CMD, or any terminal — no browser needed.
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
