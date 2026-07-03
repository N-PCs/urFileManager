import './Themes.css'

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
    },
  },
  {
    name: 'Nordic Frost',
    colors: ['#eceff4', '#d8dee9', '#5e81ac'],
    vars: {
      '--bg': '#eef0f4',
      '--bg-white': '#ffffff',
      '--bg-muted': '#e2e5eb',
      '--border': '#d8dee9',
      '--border-strong': '#c0c7d4',
      '--text': '#2e3440',
      '--text-secondary': '#5e686f',
      '--text-muted': '#8f9ba8',
      '--primary': '#4c566a',
      '--primary-hover': '#3b4252',
      '--secondary': '#5e81ac',
      '--accent': '#81a1c1',
      '--accent-bg': '#e5edf5',
      '--accent-soft': '#d8dee9',
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
    },
  },
]

export default function Themes() {
  const handleEnter = (vars) => {
    const root = document.documentElement
    Object.entries(vars).forEach(([key, value]) => {
      root.style.setProperty(key, value)
    })
  }

  const handleLeave = () => {
    const root = document.documentElement
    const defaultVars = themes.find((t) => t.default)?.vars || themes[1].vars
    Object.keys(defaultVars).forEach((key) => {
      root.style.removeProperty(key)
    })
  }

  return (
    <section id="themes" className="section themes">
      <div className="container">
        <div className="themes-layout">
          <div className="themes-text">
            <span className="section-label">Themes</span>
            <h2 className="section-title">Five interfaces, one engine</h2>
            <p className="section-subtitle">
              Hover over any theme to preview it on the website. Switch between color
              schemes designed for different workspaces — from professional dark mode
              to clean light layouts.
            </p>
          </div>

          <div className="themes-showcase">
            {themes.map((theme) => (
              <div
                key={theme.name}
                className="theme-card card"
                onMouseEnter={() => handleEnter(theme.vars)}
                onMouseLeave={handleLeave}
              >
                <div className="theme-preview">
                  {theme.colors.map((color) => (
                    <span key={color} style={{ background: color }} />
                  ))}
                </div>
                <div className="theme-meta">
                  <span className="theme-name">{theme.name}</span>
                </div>
              </div>
            ))}
          </div>
        </div>
      </div>
    </section>
  )
}
