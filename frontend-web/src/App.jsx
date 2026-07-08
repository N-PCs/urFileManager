import { useState, useEffect } from 'react'
import ErrorBoundary from './components/ErrorBoundary'
import Navbar from './components/Navbar'
import Hero from './components/Hero'
import Features from './components/Features'
import HowItWorks from './components/HowItWorks'
import Themes from './components/Themes'
import Download from './components/Download'
import Docs from './components/Docs'
import Footer from './components/Footer'

export default function App() {
  const [route, setRoute] = useState(window.location.hash)

  useEffect(() => {
    const handleHashChange = () => {
      setRoute(window.location.hash)
    }
    window.addEventListener('hashchange', handleHashChange)
    return () => window.removeEventListener('hashchange', handleHashChange)
  }, [])

  const isDocs = route === '#docs'

  return (
    <ErrorBoundary>
      <Navbar currentRoute={route} />
      <main>
        {isDocs ? (
          <Docs />
        ) : (
          <>
            <Hero />
            <Features />
            <HowItWorks />
            <Themes />
            <Download />
          </>
        )}
      </main>
      <Footer />
    </ErrorBoundary>
  )
}
