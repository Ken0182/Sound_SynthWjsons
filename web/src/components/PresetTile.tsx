import React from 'react'
import { Play, Pause, Square, Info, Volume2 } from 'lucide-react'
import { motion } from 'framer-motion'
import { useAudio } from '../contexts/AudioContext'
import { Preset } from '../contexts/PresetContext'

interface PresetTileProps {
  preset: Preset
  isPlaying: boolean
}

export default function PresetTile({ preset, isPlaying }: PresetTileProps) {
  const { playPreset, stopPreset, state } = useAudio()

  const handlePlayStop = async () => {
    if (isPlaying) {
      await stopPreset(preset.name)
    } else {
      await playPreset(preset.name)
    }
  }

  const handleInfoClick = (e: React.MouseEvent) => {
    e.stopPropagation()
    // TODO: Open preset detail modal
    console.log('Show preset info:', preset.name)
  }

  return (
    <motion.div
      initial={{ opacity: 0, y: 20 }}
      animate={{ opacity: 1, y: 0 }}
      exit={{ opacity: 0, y: -20 }}
      transition={{ duration: 0.2 }}
      className={`preset-tile ${isPlaying ? 'playing' : ''} ${state.isLoading ? 'loading' : ''}`}
      onClick={handlePlayStop}
    >
      {/* Preset Header */}
      <div className="flex items-start justify-between mb-4">
        <div className="flex-1 min-w-0">
          <h3 className="text-lg font-semibold text-gray-900 truncate">
            {preset.name}
          </h3>
          <div className="flex items-center space-x-2 mt-1">
            <span className="inline-flex items-center px-2 py-1 rounded-full text-xs font-medium bg-primary-100 text-primary-800">
              {preset.category}
            </span>
            {isPlaying && (
              <span className="inline-flex items-center px-2 py-1 rounded-full text-xs font-medium bg-success-100 text-success-800">
                <Volume2 className="w-3 h-3 mr-1" />
                Playing
              </span>
            )}
          </div>
        </div>
        
        <button
          onClick={handleInfoClick}
          className="p-1 rounded-lg hover:bg-gray-100 transition-colors"
          title="Show details"
        >
          <Info className="w-4 h-4 text-gray-400" />
        </button>
      </div>

      {/* Preset Description */}
      <p className="text-sm text-gray-600 mb-4 line-clamp-2">
        {preset.description}
      </p>

      {/* Preset Controls */}
      <div className="flex items-center justify-between">
        <button
          onClick={handlePlayStop}
          disabled={state.isLoading}
          className={`btn btn-sm flex-1 mr-2 ${
            isPlaying 
              ? 'btn-danger' 
              : 'btn-primary'
          }`}
        >
          {isPlaying ? (
            <>
              <Square className="w-4 h-4 mr-2" />
              Stop
            </>
          ) : (
            <>
              <Play className="w-4 h-4 mr-2" />
              Play
            </>
          )}
        </button>

        {/* Additional Controls */}
        <div className="flex items-center space-x-1">
          {/* Volume indicator */}
          {isPlaying && (
            <div className="flex space-x-1">
              <div className="w-1 h-3 bg-success-500 rounded-full animate-pulse"></div>
              <div className="w-1 h-3 bg-success-500 rounded-full animate-pulse" style={{ animationDelay: '0.1s' }}></div>
              <div className="w-1 h-3 bg-success-500 rounded-full animate-pulse" style={{ animationDelay: '0.2s' }}></div>
            </div>
          )}
        </div>
      </div>

      {/* Preset Parameters Preview */}
      {preset.parameters && Object.keys(preset.parameters).length > 0 && (
        <div className="mt-4 pt-4 border-t border-gray-100">
          <div className="text-xs text-gray-500 mb-2">Parameters:</div>
          <div className="flex flex-wrap gap-1">
            {Object.entries(preset.parameters)
              .slice(0, 3)
              .map(([key, value]) => (
                <span
                  key={key}
                  className="inline-flex items-center px-2 py-1 rounded text-xs bg-gray-100 text-gray-600"
                >
                  {key}: {typeof value === 'number' ? value.toFixed(2) : String(value)}
                </span>
              ))}
            {Object.keys(preset.parameters).length > 3 && (
              <span className="inline-flex items-center px-2 py-1 rounded text-xs bg-gray-100 text-gray-600">
                +{Object.keys(preset.parameters).length - 3} more
              </span>
            )}
          </div>
        </div>
      )}
    </motion.div>
  )
}