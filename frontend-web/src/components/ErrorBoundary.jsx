import { Component } from 'react'

export default class ErrorBoundary extends Component {
  constructor(props) {
    super(props)
    this.state = { error: null }
  }

  static getDerivedStateFromError(error) {
    return { error }
  }

  render() {
    if (this.state.error) {
      return (
        <section className="section" style={{textAlign:'center',padding:'120px 24px'}}>
          <h2 className="section-title" style={{marginBottom:'12px'}}>Something went wrong</h2>
          <p className="section-subtitle" style={{marginInline:'auto'}}>
            {this.state.error.message}
          </p>
        </section>
      )
    }
    return this.props.children
  }
}
