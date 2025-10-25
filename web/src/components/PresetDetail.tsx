import React from 'react'
import { useParams, useNavigate } from 'react-router-dom'
import { ArrowLeft, Play, Pause, Square, Volume2, Settings } from 'lucide-react'
import { usePresets, useAudio } from '../contexts'
import LoadingSpinner from './LoadingSpinner'

export default function PresetDetail() {
  const { id } = useParams<{ id: string }>()
  const navigate = useNavigate()
  const { getPresetById } = usePresets()
  const { playPreset, stopPreset, isPresetPlaying } = useAudio()

  const preset = id ? getPresetById(id) : undefined
  const isPlaying = preset ? isPresetPlaying(preset.name) : false

  const handlePlayStop = async () => {
    if (!preset) return
    
    if (isPlaying) {
      await stopPreset(preset.name)
    } else {
      await playPreset(preset.name)
    }
  }

  if (!preset) {
    return (
      <div className="text-center py-12">
        <h2 className="text-xl font-semibold text-gray-900 mb-2">Preset Not Found</h2>
        <p className="text-gray-500 mb-4">The requested preset could not be found.</p>
        <button
          onClick={() => navigate('/')}
          className="btn btn-primary"
        >
          <ArrowLeft className="w-4 h-4 mr-2" />
          Back to Presets
        </button>
      </div>
    )
  }

  return (
    <div className="max-w-4xl mx-auto">
      {/* Header */}
      <div className="flex items-center space-x-4 mb-6">
        <button
          onClick={() => navigate('/')}
          className="p-2 rounded-lg hover:bg-gray-100 transition-colors"
        >
          <ArrowLeft className="w-5 h-5 text-gray-600" />
        </button>
        <div>
          <h1 className="text-2xl font-bold text-gray-900">{preset.name}</h1>
          <p className="text-gray-500">{preset.category}</p>
        </div>
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Main Content */}
        <div className="lg:col-span-2 space-y-6">
          {/* Description */}
          <div className="bg-white rounded-lg shadow-sm border border-gray-200 p-6">
            <h2 className="text-lg font-semibold text-gray-900 mb-3">Description</h2>
            <p className="text-gray-600">{preset.description}</p>
          </div>

          {/* Parameters */}
          {preset.parameters && Object.keys(preset.parameters).length > 0 && (
            <div className="bg-white rounded-lg shadow-sm border border-gray-200 p-6">
              <h2 className="text-lg font-semibold text-gray-900 mb-4">Parameters</h2>
              <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
                {Object.entries(preset.parameters).map(([key, value]) => (
                  <div key={key} className="flex justify-between items-center py-2 border-b border-gray-100 last:border-b-0">
                    <span className="text-sm font-medium text-gray-700 capitalize">
                      {key.replace(/_/g, ' ')}
                    </span>
                    <span className="text-sm text-gray-600">
                      {typeof value === 'number' ? value.toFixed(3) : String(value)}
                    </span>
                  </div>
                ))}
              </div>
            </div>
          )}

          {/* Audio Controls */}
          <div className="bg-white rounded-lg shadow-sm border border-gray-200 p-6">
            <h2 className="text-lg font-semibold text-gray-900 mb-4">Audio Controls</h2>
            <div className="flex items-center space-x-4">
              <button
                onClick={handlePlayStop}
                className={`btn btn-lg ${
                  isPlaying ? 'btn-danger' : 'btn-primary'
                }`}
              >
                {isPlaying ? (
                  <>
                    <Square className="w-5 h-5 mr-2" />
                    Stop
                  </>
                ) : (
                  <>
                    <Play className="w-5 h-5 mr-2" />
                    Play
                  </>
                )}
              </button>

              <div className="flex items-center space-x-2">
                <Volume2 className="w-5 h-5 text-gray-400" />
                <span className="text-sm text-gray-500">
                  {isPlaying ? 'Playing...' : 'Ready to play'}
                </span>
              </div>
            </div>
          </div>
        </div>

        {/* Sidebar */}
        <div className="space-y-6">
          {/* Quick Info */}
          <div className="bg-white rounded-lg shadow-sm border border-gray-200 p-6">
            <h3 className="text-lg font-semibold text-gray-900 mb-4">Quick Info</h3>
            <div className="space-y-3">
              <div className="flex justify-between">
                <span className="text-sm text-gray-500">Category</span>
                <span className="text-sm font-medium text-gray-900">{preset.category}</span>
              </div>
              <div className="flex justify-between">
                <span className="text-sm text-gray-500">Status</span>
                <span className={`text-sm font-medium ${
                  isPlaying ? 'text-success-600' : 'text-gray-600'
                }`}>
                  {isPlaying ? 'Playing' : 'Stopped'}
                </span>
              </div>
              <div className="flex justify-between">
                <span className="text-sm text-gray-500">Parameters</span>
                <span className="text-sm font-medium text-gray-900">
                  {preset.parameters ? Object.keys(preset.parameters).length : 0}
                </span>
              </div>
            </div>
          </div>

          {/* Actions */}
          <div className="bg-white rounded-lg shadow-sm border border-gray-200 p-6">
            <h3 className="text-lg font-semibold text-gray-900 mb-4">Actions</h3>
            <div className="space-y-2">
              <button className="btn btn-outline w-full justify-start">
                <Settings className="w-4 h-4 mr-2" />
                Edit Parameters
              </button>
              <button className="btn btn-outline w-full justify-start">
                <Volume2 className="w-4 h-4 mr-2" />
                Export Audio
              </button>
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}