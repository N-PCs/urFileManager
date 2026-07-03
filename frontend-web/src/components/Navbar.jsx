import './Navbar.css'

export default function Navbar() {
  return (
    <header className="navbar">
      <div className="navbar-inner">
        <a href="#" className="navbar-brand">
          urFileManager
        </a>

        <nav className="navbar-links" aria-label="Main navigation">
          <a href="#features">Features</a>
          <a href="#how-it-works">How it works</a>
          <a href="#themes">Themes</a>
        </nav>

        <a href="#download" className="btn btn-primary navbar-cta" style={{ color: "black" }}>
          Download
        </a>
      </div>
    </header>
  )
}
