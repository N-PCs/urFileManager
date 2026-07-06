import { useState, useEffect, useRef, useCallback } from 'react'
import './Themes.css'

const STORAGE_KEY = 'urfm-theme'

const safeStorage = {
  get: (key) => { try { return localStorage.getItem(key) } catch { return null } },
  set: (key, value) => { try { localStorage.setItem(key, value) } catch { /* empty */ } },
  remove: (key) => { try { localStorage.removeItem(key) } catch { /* empty */ } },
}

const themes = [

  {
    name: 'Minimalist Light',
    colors: ['#f8fafc', '#e2e8f0', '#94a3b8'],
    vars: {
      '--bg': '#f8fafc',
      '--bg-white': '#ffffff',
      '--bg-muted': '#f1f5f9',
      '--border': '#e2e8f0',
      '--border-strong': '#cbd5e1',
      '--text': '#1a202c',
      '--text-secondary': '#64748b',
      '--text-muted': '#94a3b8',
      '--primary': '#334155',
      '--primary-hover': '#1e293b',
      '--secondary': '#1e3a8a',
      '--accent': '#9fa8da',
      '--accent-bg': '#eef2ff',
      '--accent-soft': '#e0e7ff',
      '--btn-text': '#ffffff',
    },
  },
  {
    name: 'Midnight Dark',
    colors: ['#0f172a', '#334155', '#64748b'],
    vars: {
      '--bg': '#0f172a',
      '--bg-white': '#1e293b',
      '--bg-muted': '#334155',
      '--border': '#334155',
      '--border-strong': '#475569',
      '--text': '#f1f5f9',
      '--text-secondary': '#94a3b8',
      '--text-muted': '#64748b',
      '--primary': '#64748b',
      '--primary-hover': '#94a3b8',
      '--secondary': '#60a5fa',
      '--accent': '#818cf8',
      '--accent-bg': '#1e1b4b',
      '--accent-soft': '#1e293b',
      '--btn-text': '#ffffff',
    },
  },
  {
    name: 'Red Sakura',
    colors: ['#ffffff', '#ffb7c5', '#de3163'],
    vars: {
      '--bg': '#ffffff',
      '--bg-white': '#fff5f7',
      '--bg-muted': '#ffe8ef',
      '--border': '#ffd6e0',
      '--border-strong': '#ffb7c5',
      '--text': '#333333',
      '--text-secondary': '#666666',
      '--text-muted': '#999999',
      '--primary': '#de3163',
      '--primary-hover': '#c42958',
      '--secondary': '#ff6b8a',
      '--accent': '#ffb7c5',
      '--accent-bg': '#fff0f3',
      '--accent-soft': '#ffe0ea',
      '--btn-text': '#ffffff',
    },
  },
  {
    name: 'Forest Emerald',
    colors: ['#0d1f17', '#134e4a', '#059669'],
    vars: {
      '--bg': '#0d1f17',
      '--bg-white': '#134e4a',
      '--bg-muted': '#1a3c34',
      '--border': '#1a3c34',
      '--border-strong': '#2d6a4f',
      '--text': '#d1fae5',
      '--text-secondary': '#a7f3d0',
      '--text-muted': '#6ee7b7',
      '--primary': '#34d399',
      '--primary-hover': '#10b981',
      '--secondary': '#059669',
      '--accent': '#047857',
      '--accent-bg': '#022c22',
      '--accent-soft': '#064e3b',
      '--btn-text': '#ffffff',
    },
  },
  {
    name: 'Neon Cyberpunk',
    colors: ['#0f172a', '#312e81', '#6366f1'],
    vars: {
      '--bg': '#0f172a',
      '--bg-white': '#1e1b4b',
      '--bg-muted': '#312e81',
      '--border': '#3730a3',
      '--border-strong': '#4f46e5',
      '--text': '#e0e7ff',
      '--text-secondary': '#a5b4fc',
      '--text-muted': '#818cf8',
      '--primary': '#6366f1',
      '--primary-hover': '#4f46e5',
      '--secondary': '#818cf8',
      '--accent': '#a78bfa',
      '--accent-bg': '#2e1065',
      '--accent-soft': '#3b0764',
      '--btn-text': '#ffffff',
    },
  },
  {
    name: 'Obsidian Volt',
    colors: ['#0a0a0c', '#121214', '#dcf365'],
    vars: {
      '--bg': '#0a0a0c',
      '--bg-white': '#121214',
      '--bg-muted': '#1a1a1e',
      '--border': '#1e1e24',
      '--border-strong': '#2c2c35',
      '--text': '#fbfbfb',
      '--text-secondary': '#cbcdd5',
      '--text-muted': '#8d8d8f',
      '--primary': '#dcf365',
      '--primary-hover': '#cbe24e',
      '--secondary': '#fbfbfb',
      '--accent': '#e8ff6b',
      '--accent-bg': '#1c1d15',
      '--accent-soft': '#282a1a',
      '--btn-text': '#000000',
    },
  },
]

export default function Themes() {
  const [active, setActive] = useState(() => safeStorage.get(STORAGE_KEY))
  const [isMobile, setIsMobile] = useState(() => window.innerWidth <= 768)
  const sectionRef = useRef(null)

  useEffect(() => {
    const onResize = () => setIsMobile(window.innerWidth <= 768)
    window.addEventListener('resize', onResize)
    return () => window.removeEventListener('resize', onResize)
  }, [])

  const clearTheme = useCallback(() => {
    const root = document.documentElement
    const allKeys = Object.keys(themes[0].vars)
    allKeys.forEach((key) => {
      root.style.removeProperty(key)
    })
  }, [])

  // Revert theme on mobile when scrolling away from section
  useEffect(() => {
    const section = sectionRef.current
    if (!section) return

    const observer = new IntersectionObserver(
      ([entry]) => {
        if (isMobile && !entry.isIntersecting && active) {
          setActive(null)
          clearTheme()
          safeStorage.remove(STORAGE_KEY)
        }
      },
      { threshold: 0 }
    )

    observer.observe(section)
    return () => observer.disconnect()
  }, [active, clearTheme, isMobile])

  useEffect(() => {
    if (active) {
      const theme = themes.find((t) => t.name === active)
      if (theme) {
        const root = document.documentElement
        Object.entries(theme.vars).forEach(([key, value]) => {
          root.style.setProperty(key, value)
        })
      }
    }
  }, []) // eslint-disable-line react-hooks/exhaustive-deps

  const applyTheme = (vars) => {
    const root = document.documentElement
    Object.entries(vars).forEach(([key, value]) => {
      root.style.setProperty(key, value)
    })
  }

  const handleClick = (theme) => {
    if (active === theme.name) {
      setActive(null)
      clearTheme()
      safeStorage.remove(STORAGE_KEY)
    } else {
      setActive(theme.name)
      applyTheme(theme.vars)
      safeStorage.set(STORAGE_KEY, theme.name)
    }
  }

  const handleEnter = (vars) => {
    applyTheme(vars)
  }

  const handleLeave = () => {
    if (active) {
      const theme = themes.find((t) => t.name === active)
      if (theme) {
        applyTheme(theme.vars)
        return
      }
    }
    clearTheme()
  }

  const handleCardClick = (theme) => {
    if (!isMobile) return
    handleClick(theme)
  }

  const handleCardEnter = (vars) => {
    if (isMobile) return
    handleEnter(vars)
  }

  const handleCardLeave = () => {
    if (isMobile) return
    handleLeave()
  }

  return (
    <section id="themes" className="section themes" ref={sectionRef}>
      <div className="container">
        <div className="themes-layout">
          <div className="themes-text">
            <span className="section-label" style={{color:"var(--primary)",fontSize:"1.0rem"}}>Themes</span>
            <h2 className="section-title">Six interfaces, one engine</h2>
            <p className="section-subtitle">
              {isMobile
                ? 'Tap any theme to apply it. Scroll away to revert.'
                : 'Hover over any theme to preview it live. Switch between color schemes designed for different workspaces — from professional dark mode to clean light layouts.'}
            </p>
          </div>

          <div className="themes-showcase">
            {themes.map((theme) => (
              <div
                key={theme.name}
                className={`theme-card card${active === theme.name ? ' active' : ''}`}
                onClick={() => handleCardClick(theme)}
                onMouseEnter={() => handleCardEnter(theme.vars)}
                onMouseLeave={handleCardLeave}
              >
                <div className="theme-preview">
                  {theme.colors.map((color) => (
                    <span key={color} style={{ background: color }} />
                  ))}
                </div>
                <div className="theme-meta">
                  <span className="theme-name">{theme.name}</span>
                  {active === theme.name && <span className="theme-active-badge">Active</span>}
                </div>
              </div>
            ))}
          </div>
        </div>
      </div>
    </section>
  )
}
