import './Features.css'
import {
  IconSort,
  IconShield,
  IconReport,
  IconLog,
  IconMerge,
  IconPalette,
  IconSettings,
  IconTerminal,
} from './Icons'

const features = [
  {
    Icon: IconSort,
    title: 'Smart Extension Sorting',
    description:
      'Moves loose files into category folders — Images, Documents, Audio, Video, Archives — based on customizable rules in config.json.',
    tag: 'Core',
  },
  {
    Icon: IconShield,
    title: 'Dry-Run Preview',
    description:
      'Preview every move before committing. Dry-run is enabled by default in the GUI so you never accidentally reorganize the wrong folder.',
    tag: 'Safety',
  },
  {
    Icon: IconReport,
    title: 'PDF Reports',
    description:
      'Generate detailed organization reports with file names, categories, sizes, and status. Save previews or full run summaries as PDFs.',
    tag: 'Reporting',
  },
  {
    Icon: IconLog,
    title: 'Full Audit Logging',
    description:
      'Every action is recorded in organizer.log with timestamps. Open it directly from the app for a complete history of moves and errors.',
    tag: 'Logging',
  },
  {
    Icon: IconMerge,
    title: 'Conflict Resolution',
    description:
      'Duplicate filenames are renamed to report (1).pdf, report (2).pdf — existing files are never overwritten.',
    tag: 'Smart',
  },
  {
    Icon: IconPalette,
    title: 'Five UI Themes',
    description:
      'Minimalist Light, Midnight Dark, Nordic Frost, Forest Emerald, Neon Cyberpunk. Switch themes from within the app.',
    tag: 'UI',
  },
  {
    Icon: IconSettings,
    title: 'Editable Config',
    description:
      'Add new file types or custom categories by editing config.json — no recompile needed. Open and edit directly from the app.',
    tag: 'Flexible',
  },
  {
    Icon: IconTerminal,
    title: 'GUI + CLI Modes',
    description:
      'Double-click for the graphical interface, or pass folder paths via command line for scripting and automation workflows.',
    tag: 'Dual-mode',
  },
]

export default function Features() {
  return (
    <section id="features" className="section features">
      <div className="container">
        <div className="features-header">
          <span className="section-label" style={{color:"var(--primary)",fontSize:"1.0rem"}}>Features</span>
          <h2 className="section-title">Built for reliable file organization</h2>
          <p className="section-subtitle">
            A native cross-platform utility with no cloud dependency, no subscriptions, and no
            external runtime required on Windows.
          </p>
        </div>

        <div className="features-grid">
          {features.map(({ Icon, title, description, tag }) => (
            <article key={title} className="feature-card card" style={{backgroundColor:"var(--primary)"}}>
              <div className="feature-top" >
                <div className="icon-box" style={{color: "white"}}>
                  <Icon />
                </div>
                <span className="feature-tag">{tag}</span>
              </div>
              <h3 className="feature-title" style={{color:"var(--bg-white)"}}>{title}</h3>
              <p className="feature-desc" style={{color:"var(--bg-white)"}}>{description}</p>
            </article>
          ))}
        </div>
      </div>
    </section>
  )
}
