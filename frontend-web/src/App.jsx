import ErrorBoundary from './components/ErrorBoundary'
import Navbar from './components/Navbar'
import Hero from './components/Hero'
import Features from './components/Features'
import HowItWorks from './components/HowItWorks'
import Themes from './components/Themes'
import Download from './components/Download'
import Footer from './components/Footer'

export default function App() {
  return (
    <ErrorBoundary>
      <Navbar />
      <main>
        <Hero />
        <Features />
        <HowItWorks />
        <Themes />
        <Download />
      </main>
      <Footer />
    </ErrorBoundary>
  )
}
