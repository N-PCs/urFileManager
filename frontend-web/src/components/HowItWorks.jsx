import { useState, useEffect } from 'react'
import './HowItWorks.css'
import {
  IconFolder,
  IconFile,
  IconShield,
  IconSort,
  IconReport,
  IconImage,
  IconDocuments,
  IconAudio,
  IconVideo,
} from './Icons'

const looseFiles = [
  { name: 'photo.jpg', Icon: IconImage },
  { name: 'report.pdf', Icon: IconDocuments },
  { name: 'track.mp3', Icon: IconAudio },
  { name: 'movie.mp4', Icon: IconVideo },
  { name: 'backup.zip', Icon: IconFile },
]

const sortedFolders = [
  { name: 'Images/', count: 1, Icon: IconImage },
  { name: 'Documents/', count: 1, Icon: IconDocuments },
  { name: 'Audio/', count: 1, Icon: IconAudio },
  { name: 'Video/', count: 1, Icon: IconVideo },
  { name: 'Archives/', count: 1, Icon: IconFile },
]

// Single horizontal row — nodes centered on the mid-line (y=50 in a 0-100 canvas)
// Big nodes (with folder previews): y=12, h=76  → spans 12→88, centred at 50
// Small nodes (scan/dryrun/report):  y=37, h=26  → spans 37→63, centred at 50
// Classify (has chips):              y=30, h=40  → spans 30→70, centred at 50
// x spacing: 4% gap between each node
// folder: x=1  w=13 → right=14 | gap4 | scan: x=18 w=12 → right=30 | gap4
// classify: x=34 w=13 → right=47 | gap4 | dryrun: x=51 w=12 → right=63 | gap4
// organize: x=67 w=13 → right=80 | gap4 | report: x=84 w=12 → right=96
const workflowNodes = [
  {
    id: 'folder',
    title: 'Pick folder',
    subtitle: 'Downloads/',
    Icon: IconFolder,
    x: 1,
    y: 12,
    w: 13,
    h: 110,
    accent: '#6366f1',
    type: 'trigger',
    preview: 'before',
  },
  {
    id: 'scan',
    title: 'Scan files',
    subtitle: '5 loose items',
    Icon: IconFile,
    x: 18,
    y: 37,
    w: 12,
    h: 26,
    accent: '#8b5cf6',
    type: 'action',
  },
  {
    id: 'classify',
    title: 'Classify',
    subtitle: 'By extension',
    Icon: IconSort,
    x: 34,
    y: 30,
    w: 13,
    h: 45,
    accent: '#a855f7',
    type: 'action',
  },
  {
    id: 'dryrun',
    title: 'Dry-run',
    subtitle: 'Preview moves',
    Icon: IconShield,
    x: 51,
    y: 37,
    w: 12,
    h: 26,
    accent: '#059669',
    type: 'action',
  },
  {
    id: 'organize',
    title: 'Organize',
    subtitle: 'Move to folders',
    Icon: IconFolder,
    x: 67,
    y: 12,
    w: 13,
    h: 110,
    accent: '#2563eb',
    type: 'action',
    preview: 'after',
  },
  {
    id: 'report',
    title: 'PDF report',
    subtitle: 'Save summary',
    Icon: IconReport,
    x: 84,
    y: 37,
    w: 12,
    h: 26,
    accent: '#dc2626',
    type: 'output',
  },
]

const workflowEdges = [
  { from: 'folder',   fromSide: 'right', to: 'scan',     toSide: 'left' },
  { from: 'scan',     fromSide: 'right', to: 'classify', toSide: 'left' },
  { from: 'classify', fromSide: 'right', to: 'dryrun',   toSide: 'left' },
  { from: 'dryrun',   fromSide: 'right', to: 'organize', toSide: 'left' },
  { from: 'organize', fromSide: 'right', to: 'report',   toSide: 'left' },
]

const categoryChips = [
  { label: 'Images', ext: '.jpg', Icon: IconImage, color: '#ec4899' },
  { label: 'Documents', ext: '.pdf', Icon: IconDocuments, color: '#f59e0b' },
  { label: 'Audio', ext: '.mp3', Icon: IconAudio, color: '#14b8a6' },
  { label: 'Video', ext: '.mp4', Icon: IconVideo, color: '#3b82f6' },
]

const CANVAS_H = 145

function getPort(node, side) {
  switch (side) {
    case 'right':
      return { x: node.x + node.w, y: node.y + node.h / 2 }
    case 'left':
      return { x: node.x, y: node.y + node.h / 2 }
    case 'bottom':
      return { x: node.x + node.w / 2, y: node.y + node.h }
    case 'top':
      return { x: node.x + node.w / 2, y: node.y }
    default:
      return { x: node.x + node.w / 2, y: node.y + node.h / 2 }
  }
}

function edgePath(fromNode, fromSide, toNode, toSide) {
  const a = getPort(fromNode, fromSide)
  const b = getPort(toNode, toSide)

  if (fromSide === 'right' && toSide === 'left') {
    const offset = Math.max(Math.abs(b.x - a.x) * 0.45, 3)
    return `M ${a.x} ${a.y} C ${a.x + offset} ${a.y}, ${b.x - offset} ${b.y}, ${b.x} ${b.y}`
  }

  if (fromSide === 'bottom' && toSide === 'top') {
    const offset = Math.max(Math.abs(b.y - a.y) * 0.45, 3)
    return `M ${a.x} ${a.y} C ${a.x} ${a.y + offset}, ${b.x} ${b.y - offset}, ${b.x} ${b.y}`
  }

  if (fromSide === 'right' && toSide === 'top') {
    const offsetX = Math.max(Math.abs(b.x - a.x) * 0.35, 3)
    const offsetY = Math.max(Math.abs(b.y - a.y) * 0.35, 3)
    return `M ${a.x} ${a.y} C ${a.x + offsetX} ${a.y}, ${b.x} ${b.y - offsetY}, ${b.x} ${b.y}`
  }

  const mx = (a.x + b.x) / 2
  const my = (a.y + b.y) / 2
  return `M ${a.x} ${a.y} Q ${mx} ${my}, ${b.x} ${b.y}`
}

function FolderPreview({ type, activeStep }) {
  if (type === 'before') {
    return (
      <div className="workflow-folder-preview workflow-folder-before">
        <div className="workflow-preview-badge workflow-preview-badge-before">Before</div>
        <div className="workflow-folder-path">
          <IconFolder />
          <span>Downloads/</span>
        </div>
        <ul className="workflow-mini-list">
          {looseFiles.map((file) => {
            const FileIcon = file.Icon
            const dimmed = activeStep > 0
            return (
              <li key={file.name} className={dimmed ? 'workflow-mini-dimmed' : ''}>
                <FileIcon />
                <span>{file.name}</span>
              </li>
            )
          })}
        </ul>
      </div>
    )
  }

  return (
    <div className="workflow-folder-preview workflow-folder-after">
      <div className="workflow-preview-badge workflow-preview-badge-after">After</div>
      <div className="workflow-folder-path">
        <IconFolder />
        <span>Downloads/</span>
      </div>
      <ul className="workflow-mini-list">
        {sortedFolders.map((folder) => {
          const FolderIcon = folder.Icon
          const lit = activeStep >= 4
          return (
            <li key={folder.name} className={lit ? 'workflow-mini-lit' : ''}>
              <FolderIcon />
              <span>{folder.name}</span>
              <span className="workflow-mini-count">{lit ? folder.count : '—'}</span>
            </li>
          )
        })}
      </ul>
    </div>
  )
}

const steps = [
  {
    num: '01',
    title: 'Pick a folder',
    description: 'Launch urFileManager and select any cluttered directory — Downloads, Desktop, or a project folder.',
    code: 'run.bat  ·  ./urfm  ·  organizer.bat <folder>',
  },
  {
    num: '02',
    title: 'Preview with dry-run',
    description: 'Review the planned moves in the live log console. Disable dry-run when you are ready to commit.',
    code: '--dry-run  (default: ON in GUI)',
  },
  {
    num: '03',
    title: 'Organize and report',
    description: 'Start organizing. Files sort into subfolders instantly. View the in-app summary or export a PDF report.',
    code: 'organization_report.pdf',
  },
]

export default function HowItWorks() {
  const [activeStep, setActiveStep] = useState(0)
  const totalSteps = workflowNodes.length

  useEffect(() => {
    const timer = setInterval(() => {
      setActiveStep((s) => (s + 1) % totalSteps)
    }, 2000)
    return () => clearInterval(timer)
  }, [totalSteps])

  const activeNodeId = workflowNodes[activeStep]?.id
  const activeEdgeIndex = Math.max(0, activeStep - 1)

  return (
    <section id="how-it-works" className="section how-it-works">
      <div className="container">
        <div className="how-header">
          <span className="section-label">How it works</span>
          <h2 className="section-title">Three steps to a clean folder</h2>
        </div>

                {/* Step cards */}
        <div className="steps">
          {steps.map((step) => (
            <div key={step.num} className="step card">
              <span className="step-num">{step.num}</span>
              <h3 className="step-title">{step.title}</h3>
              <p className="step-desc">{step.description}</p>
              <code className="step-code">{step.code}</code>
            </div>
          ))}
        </div>

        {/* Workflow diagram */}
        <div className="hiw-workflow card" aria-hidden="true">
          <div className="workflow-toolbar">
            <div className="workflow-toolbar-left">
              <span className="workflow-dot" />
              <span className="workflow-dot" />
              <span className="workflow-dot" />
              <span className="workflow-title">Organize files</span>
            </div>
            <span className="workflow-status">
              Step {activeStep + 1}/{totalSteps} · {workflowNodes[activeStep]?.title}
            </span>
          </div>

          <div className="workflow-canvas">
            <svg
              className="workflow-edges"
              viewBox={`0 0 100 ${CANVAS_H}`}
              preserveAspectRatio="none"
            >
              <defs>
                <linearGradient id="hiw-edge-gradient" x1="0%" y1="0%" x2="100%" y2="0%">
                  <stop offset="0%" stopColor="var(--primary)" />
                  <stop offset="100%" stopColor="var(--secondary)" />
                </linearGradient>
                <marker
                  id="hiw-edge-arrow"
                  markerWidth="4"
                  markerHeight="4"
                  refX="3.5"
                  refY="2"
                  orient="auto"
                  markerUnits="strokeWidth"
                >
                  <path d="M0,0 L4,2 L0,4 Z" fill="context-stroke" />
                </marker>
              </defs>
              {workflowEdges.map((edge, i) => {
                const from = workflowNodes.find((n) => n.id === edge.from)
                const to = workflowNodes.find((n) => n.id === edge.to)
                if (!from || !to) return null
                const isActive = i <= activeEdgeIndex
                const isCurrent = i === activeEdgeIndex
                return (
                  <path
                    key={`${edge.from}-${edge.to}`}
                    d={edgePath(from, edge.fromSide, to, edge.toSide)}
                    className={`workflow-edge ${isActive ? 'workflow-edge-done' : ''} ${isCurrent ? 'workflow-edge-active' : ''}`}
                    fill="none"
                    strokeWidth="0.35"
                    vectorEffect="non-scaling-stroke"
                    markerEnd="url(#hiw-edge-arrow)"
                  />
                )
              })}
            </svg>

            {workflowNodes.map((node, index) => {
              const Icon = node.Icon
              const isActive = node.id === activeNodeId
              const isDone = index < activeStep
              const isExpanded = Boolean(node.preview)

              return (
                <div
                  key={node.id}
                  className={`workflow-node workflow-node-${node.type} ${isExpanded ? 'workflow-node-expanded' : ''} ${isActive ? 'workflow-node-active' : ''} ${isDone ? 'workflow-node-done' : ''}`}
                  style={{
                    left: `${node.x}%`,
                    top: `${(node.y / CANVAS_H) * 100}%`,
                    width: `${node.w}%`,
                    height: `${(node.h / CANVAS_H) * 100}%`,
                  }}
                >
                  <span className="workflow-port workflow-port-in" />
                  <div className="workflow-node-body">
                    <div className="workflow-node-header">
                      <div className="workflow-node-icon" style={{ '--node-accent': node.accent }}>
                        <Icon />
                      </div>
                      <div className="workflow-node-text">
                        <span className="workflow-node-title">{node.title}</span>
                        <span className="workflow-node-sub">{node.subtitle}</span>
                      </div>
                    </div>

                    {node.preview && <FolderPreview type={node.preview} activeStep={activeStep} />}

                    {node.id === 'classify' && (
                      <div className="workflow-inline-chips">
                        {categoryChips.map((chip) => {
                          const ChipIcon = chip.Icon
                          const lit = activeStep >= 2
                          return (
                            <span
                              key={chip.label}
                              className={`workflow-chip ${lit ? 'workflow-chip-lit' : ''}`}
                              style={{ '--chip-color': chip.color }}
                            >
                              <ChipIcon />
                              {chip.ext}
                            </span>
                          )
                        })}
                      </div>
                    )}
                  </div>
                  <span className="workflow-port workflow-port-out" />
                  {isActive && <span className="workflow-pulse" />}
                </div>
              )
            })}
          </div>

          <div className="workflow-footer">
            <div className="workflow-progress">
              <div
                className="workflow-progress-fill"
                style={{ width: `${((activeStep + 1) / totalSteps) * 100}%` }}
              />
            </div>
            <span className="workflow-footer-text">
              {activeStep === 0 && 'Messy Downloads/ with 5 loose files'}
              {activeStep === 1 && 'Scanning file extensions…'}
              {activeStep === 2 && 'Matching .jpg .pdf .mp3 .mp4 to categories'}
              {activeStep === 3 && 'Dry-run preview — no files moved yet'}
              {activeStep === 4 && 'Sorted into Images/, Documents/, Audio/, Video/, Archives/'}
              {activeStep === 5 && 'PDF report exported — pipeline complete'}
            </span>
          </div>
        </div>
      </div>
    </section>
  )
}
