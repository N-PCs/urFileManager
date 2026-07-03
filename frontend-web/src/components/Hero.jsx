import './Hero.css'
import { IconWindows } from './Icons'

export default function Hero() {
  return (
    <section className="hero">
      <div className="container hero-layout">
        <div className="hero-copy">
          <p className="hero-eyebrow">Cross-platform file organizer</p>
          <h1 className="hero-title">
            Turn a messy folder into{' '}
            <span className="hero-gradient">organized categories</span>
          </h1>
          <p className="hero-description">
            Point urFileManager at any cluttered directory — like Downloads — and it moves
            every loose file into subfolders by type. Images, Documents, Audio, Video,
            and Archives are created automatically. Preview with dry-run before anything moves.
          </p>
          <div className="hero-actions">
            <a href="#download" className="btn btn-primary btn-lg">
              Download urFM
            </a>
            <a href="#download" className="btn btn-secondary btn-lg">
              CLI install commands
            </a>
          </div>
        </div>
      </div>
    </section>
  )
}
