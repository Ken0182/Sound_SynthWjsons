import React from 'react'
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom'
import { Toaster } from 'react-hot-toast'
import { AudioProvider } from './contexts/AudioContext'
import { PresetProvider } from './contexts/PresetContext'
import Header from './components/Header'
import PresetGrid from './components/PresetGrid'
import PresetDetail from './components/PresetDetail'
import StatusBar from './components/StatusBar'
import LoadingSpinner from './components/LoadingSpinner'
import ErrorBoundary from './components/ErrorBoundary'

function App() {
  return (
    <ErrorBoundary>
      <AudioProvider>
        <PresetProvider>
          <Router>
            <div className="min-h-screen bg-gray-50">
              <Header />
              <main className="container mx-auto px-4 py-8">
                <Routes>
                  <Route path="/" element={<PresetGrid />} />
                  <Route path="/preset/:id" element={<PresetDetail />} />
                </Routes>
              </main>
              <StatusBar />
              <Toaster
                position="top-right"
                toastOptions={{
                  duration: 4000,
                  style: {
                    background: '#363636',
                    color: '#fff',
                  },
                  success: {
                    duration: 3000,
                    iconTheme: {
                      primary: '#22c55e',
                      secondary: '#fff',
                    },
                  },
                  error: {
                    duration: 5000,
                    iconTheme: {
                      primary: '#ef4444',
                      secondary: '#fff',
                    },
                  },
                }}
              />
            </div>
          </Router>
        </PresetProvider>
      </AudioProvider>
    </ErrorBoundary>
  )
}

export default App