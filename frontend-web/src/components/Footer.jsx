import './Footer.css'

function TinyRawFolder() {
  return (
    <svg viewBox="0 0 24 24" fill="none" className="tiny-folder-svg">
      <rect x="3" y="7" width="18" height="14" rx="2" stroke="currentColor" strokeWidth="1.5" />
      <path d="M3 10h18" stroke="currentColor" strokeWidth="1" opacity="0.3" />
      <rect x="6" y="12" width="4" height="1.5" rx="0.5" fill="currentColor" opacity="0.6" />
      <rect x="6" y="15" width="5" height="1.5" rx="0.5" fill="currentColor" opacity="0.4" />
      <rect x="12" y="13" width="5" height="1.5" rx="0.5" fill="currentColor" opacity="0.35" />
      <rect x="13" y="16" width="4" height="1.5" rx="0.5" fill="currentColor" opacity="0.45" />
      <path d="M5 7l2-2.5h5l2 2.5" stroke="currentColor" strokeWidth="1.5" strokeLinejoin="round" />
    </svg>
  )
}

function TinyOrganizedFolder() {
  return (
    <svg viewBox="0 0 24 24" fill="none" className="tiny-folder-svg">
      <rect x="3" y="7" width="18" height="14" rx="2" stroke="currentColor" strokeWidth="1.5" />
      <path d="M3 10h18" stroke="currentColor" strokeWidth="1" opacity="0.3" />
      <rect x="5" y="11.5" width="3" height="3" rx="1" fill="currentColor" opacity="0.5" />
      <rect x="9.5" y="11.5" width="3" height="3" rx="1" fill="currentColor" opacity="0.4" />
      <rect x="14" y="11.5" width="3" height="3" rx="1" fill="currentColor" opacity="0.35" />
      <rect x="5" y="15.5" width="3" height="3" rx="1" fill="currentColor" opacity="0.45" />
      <rect x="9.5" y="15.5" width="3" height="3" rx="1" fill="currentColor" opacity="0.3" />
      <rect x="14" y="15.5" width="3" height="3" rx="1" fill="currentColor" opacity="0.5" />
      <path d="M5 7l2-2.5h5l2 2.5" stroke="currentColor" strokeWidth="1.5" strokeLinejoin="round" />
    </svg>
  )
}

function TinyWalker() {
  return (
    <svg viewBox="0 0 20 34" fill="none" className="tiny-walker-svg">
      <circle cx="10" cy="4.5" r="3" stroke="currentColor" strokeWidth="1.5" />
      <line x1="10" y1="7.5" x2="10" y2="20" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" />
      <line x1="10" y1="12" x2="4" y2="17" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" className="arm-l" />
      <line x1="10" y1="12" x2="16" y2="17" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" className="arm-r" />
      <line x1="10" y1="20" x2="4" y2="30" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" className="leg-l" />
      <line x1="10" y1="20" x2="16" y2="30" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" className="leg-r" />
      <rect x="14.5" y="13" width="5" height="4" rx="1" stroke="currentColor" strokeWidth="0.8" className="carried-folder" />
      <path d="M15.8 13v-0.8h2.2v0.8" stroke="currentColor" strokeWidth="0.8" strokeLinejoin="round" className="carried-folder" />
    </svg>
  )
}

export default function Footer() {
  return (
    <footer className="footer">
      <div className="container footer-inner">
        <div className="footer-brand">
          <span className="footer-logo" style={{color:"var(--primary)"}}>
            <img src="./logo.png" alt="urFM logo" className="footer-logo-img" />
            urFM
          </span>
          <p className="footer-tagline">Smart file organization for Windows (C++) &amp; Linux (Java).</p>
        </div>

        <div className="footer-links">
          <a href="#features">Features</a>
          <a href="#how-it-works">How it works</a>
          <a href="#themes">Themes</a>
          <a href="#download">Download</a>
        </div>

        <p className="footer-copy">
          © {new Date().getFullYear()} urFileManager. Native C++, fully offline, no tracking.
          <span className="footer-credit">
            {' '}Made with 💛 by{' '}
            <a href="https://github.com/N-PCs" target="_blank" rel="noopener noreferrer">
              N-PCs
            </a>
          </span>

          <span className="fd-inline" aria-hidden="true">
            <span className="fd-inline-node fd-inline-customer">
              <TinyRawFolder />
              <span className="fd-inline-label">Input</span>
              <span className="fd-inline-sub">Raw folder</span>
            </span>

            <span className="fd-inline-road">
              <span className="fd-inline-surface" />
              <span className="fd-inline-dash" />
              <span className="fd-inline-walker">
                <TinyWalker />
              </span>
            </span>

            <span className="fd-inline-node fd-inline-receiver">
              <TinyOrganizedFolder />
              <span className="fd-inline-label">Output</span>
              <span className="fd-inline-sub">Organised folder</span>
            </span>
          </span>
        </p>
      </div>
    </footer>
  )
}
