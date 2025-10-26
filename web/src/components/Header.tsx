import React from 'react'
import { Music, Volume2, VolumeX, Settings } from 'lucide-react'
import { useAudio } from '../contexts/AudioContext'

export default function Header() {
  const { state, setMuted, stopAll } = useAudio()

  return (
    <header className="bg-white shadow-sm border-b border-gray-200">
      <div className="container mx-auto px-4 py-4">
        <div className="flex items-center justify-between">
          {/* Logo and Title */}
          <div className="flex items-center space-x-3">
            <div className="flex items-center justify-center w-10 h-10 bg-primary-600 rounded-lg">
              <Music className="w-6 h-6 text-white" />
            </div>
            <div>
              <h1 className="text-2xl font-bold text-gray-900">AI Audio Generator</h1>
              <p className="text-sm text-gray-500">One-Click Sound Generation</p>
            </div>
          </div>

          {/* Audio Controls */}
          <div className="flex items-center space-x-4">
            {/* Volume Control */}
            <div className="flex items-center space-x-2">
              <button
                onClick={() => setMuted(!state.isMuted)}
                className="p-2 rounded-lg hover:bg-gray-100 transition-colors"
                title={state.isMuted ? 'Unmute' : 'Mute'}
              >
                {state.isMuted ? (
                  <VolumeX className="w-5 h-5 text-gray-500" />
                ) : (
                  <Volume2 className="w-5 h-5 text-gray-700" />
                )}
              </button>
              
              <input
                type="range"
                min="0"
                max="1"
                step="0.1"
                value={state.volume}
                onChange={(e) => {
                  const volume = parseFloat(e.target.value)
                  setMuted(volume === 0)
                }}
                className="w-20 h-2 bg-gray-200 rounded-lg appearance-none cursor-pointer"
                disabled={state.isMuted}
              />
            </div>

            {/* Stop All Button */}
            {state.playingPresets.size > 0 && (
              <button
                onClick={stopAll}
                className="btn btn-danger btn-sm"
                title="Stop all sounds"
              >
                Stop All
              </button>
            )}

            {/* Settings Button */}
            <button
              className="p-2 rounded-lg hover:bg-gray-100 transition-colors"
              title="Settings"
            >
              <Settings className="w-5 h-5 text-gray-700" />
            </button>
          </div>
        </div>

        {/* Status Bar */}
        {state.playingPresets.size > 0 && (
          <div className="mt-4 p-3 bg-success-50 border border-success-200 rounded-lg">
            <div className="flex items-center space-x-2">
              <div className="w-2 h-2 bg-success-500 rounded-full animate-pulse"></div>
              <span className="text-sm text-success-700">
                Playing {state.playingPresets.size} sound{state.playingPresets.size !== 1 ? 's' : ''}:{' '}
                {Array.from(state.playingPresets).join(', ')}
              </span>
            </div>
          </div>
        )}
      </div>
    </header>
  )
}