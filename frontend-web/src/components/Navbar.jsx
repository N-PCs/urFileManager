import { useState, useEffect } from 'react'
import './Navbar.css'
import { IconHome, IconMenu, IconX } from './Icons'

export default function Navbar({ currentRoute }) {
  const [open, setOpen] = useState(false)

  useEffect(() => {
    if (open) {
      document.body.style.overflow = 'hidden'
    } else {
      document.body.style.overflow = ''
    }
    return () => { document.body.style.overflow = '' }
  }, [open])

  const isDocs = currentRoute === '#docs'

  return (
    <>
      <header className="navbar">
        <div className="navbar-inner">
          <img src='./logo.png' className="navbar-logo" alt="urFileManager logo"></img>
          <a href="#" className="navbar-brand">

            urFileManager
          </a>

          <nav className="navbar-links" aria-label="Main navigation">
            <a href="#home" className="navbar-home" aria-label="Home">
              <IconHome />
            </a>
            <a href="#features">Features</a>
            <a href="#how-it-works">How it works</a>
            <a href="#themes">Themes</a>
            <a href="#docs" style={{ color: isDocs ? 'var(--primary)' : 'var(--text-secondary)', fontWeight: isDocs ? '700' : '500' }}>Docs</a>
            <a href="#download" className="btn btn-primary navbar-cta" style={{ color: 'black', background: 'var(--primary)' }}>Download</a>
          </nav>

          <button
            className="navbar-burger"
            onClick={() => setOpen(true)}
            aria-label="Open menu"
          >
            <IconMenu />
          </button>
        </div>
      </header>

      <div className={`sidebar-backdrop ${open ? 'open' : ''}`} onClick={() => setOpen(false)} />
      <aside className={`sidebar ${open ? 'open' : ''}`}>
        <div className="sidebar-header">
          <span className="sidebar-brand">urFM</span>
          <button className="sidebar-close" onClick={() => setOpen(false)} aria-label="Close menu">
            <IconX />
          </button>
        </div>
        <nav className="sidebar-nav" aria-label="Mobile navigation">
          <a href="#home" onClick={() => setOpen(false)}>Home</a>
          <a href="#features" onClick={() => setOpen(false)}>Features</a>
          <a href="#how-it-works" onClick={() => setOpen(false)}>How it works</a>
          <a href="#themes" onClick={() => setOpen(false)}>Themes</a>
          <a href="#docs" style={{ color: isDocs ? 'var(--primary)' : 'inherit', fontWeight: isDocs ? '700' : 'inherit' }} onClick={() => setOpen(false)}>Docs</a>
          <a href="#download" className="sidebar-cta" onClick={() => setOpen(false)}>Download</a>
        </nav>
      </aside>
    </>
  )
}
