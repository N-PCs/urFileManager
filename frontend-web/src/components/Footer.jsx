import './Footer.css'

export default function Footer() {
  return (
    <footer className="footer">
      <div className="container footer-inner">
        <div className="footer-brand">
          <span className="footer-logo">urFM</span>
          <p className="footer-tagline">Smart file organization for all platforms.</p>
        </div>

        <div className="footer-links">
          <a href="#features">Features</a>
          <a href="#how-it-works">How it works</a>
          <a href="#themes">Themes</a>
          <a href="#download">Download</a>
        </div>

        <p className="footer-copy">
          © {new Date().getFullYear()} urFileManager. Native C++, fully offline, no tracking.                                        <span justify content='right' className='footer-copy' style={{color:"blue", alignItems:"right"}}>made with ❤️ by <a href="https://github.com/N-PCs" color='blue'>N-PCs</a></span>
        </p>        

      </div>
    </footer>
  )
}
