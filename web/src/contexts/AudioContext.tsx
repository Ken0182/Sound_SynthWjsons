import React, { createContext, useContext, useReducer, useCallback, useEffect } from 'react'
import { toast } from 'react-hot-toast'

export interface AudioState {
  isPlaying: boolean
  currentPreset: string | null
  volume: number
  isMuted: boolean
  isLoading: boolean
  error: string | null
  playingPresets: Set<string>
}

type AudioAction =
  | { type: 'SET_LOADING'; payload: boolean }
  | { type: 'SET_ERROR'; payload: string | null }
  | { type: 'SET_VOLUME'; payload: number }
  | { type: 'SET_MUTED'; payload: boolean }
  | { type: 'PLAY_PRESET'; payload: string }
  | { type: 'STOP_PRESET'; payload: string }
  | { type: 'STOP_ALL' }
  | { type: 'SET_PLAYING_PRESETS'; payload: Set<string> }

const initialState: AudioState = {
  isPlaying: false,
  currentPreset: null,
  volume: 1.0,
  isMuted: false,
  isLoading: false,
  error: null,
  playingPresets: new Set(),
}

function audioReducer(state: AudioState, action: AudioAction): AudioState {
  switch (action.type) {
    case 'SET_LOADING':
      return { ...state, isLoading: action.payload }
    case 'SET_ERROR':
      return { ...state, error: action.payload, isLoading: false }
    case 'SET_VOLUME':
      return { ...state, volume: Math.max(0, Math.min(1, action.payload)) }
    case 'SET_MUTED':
      return { ...state, isMuted: action.payload }
    case 'PLAY_PRESET': {
      const newPlayingPresets = new Set(state.playingPresets)
      newPlayingPresets.add(action.payload)
      return {
        ...state,
        playingPresets: newPlayingPresets,
        currentPreset: action.payload,
        isPlaying: true,
        error: null,
      }
    }
    case 'STOP_PRESET': {
      const newPlayingPresets = new Set(state.playingPresets)
      newPlayingPresets.delete(action.payload)
      return {
        ...state,
        playingPresets: newPlayingPresets,
        currentPreset: newPlayingPresets.size > 0 ? Array.from(newPlayingPresets)[0] : null,
        isPlaying: newPlayingPresets.size > 0,
      }
    }
    case 'STOP_ALL':
      return {
        ...state,
        playingPresets: new Set(),
        currentPreset: null,
        isPlaying: false,
      }
    case 'SET_PLAYING_PRESETS':
      return {
        ...state,
        playingPresets: action.payload,
        isPlaying: action.payload.size > 0,
        currentPreset: action.payload.size > 0 ? Array.from(action.payload)[0] : null,
      }
    default:
      return state
  }
}

interface AudioContextType {
  state: AudioState
  playPreset: (presetId: string) => Promise<void>
  stopPreset: (presetId: string) => Promise<void>
  stopAll: () => Promise<void>
  setVolume: (volume: number) => void
  setMuted: (muted: boolean) => void
  isPresetPlaying: (presetId: string) => boolean
}

const AudioContext = createContext<AudioContextType | undefined>(undefined)

export function AudioProvider({ children }: { children: React.ReactNode }) {
  const [state, dispatch] = useReducer(audioReducer, initialState)

  const playPreset = useCallback(async (presetId: string) => {
    if (state.playingPresets.has(presetId)) {
      return
    }

    dispatch({ type: 'SET_LOADING', payload: true })
    dispatch({ type: 'SET_ERROR', payload: null })

    try {
      const response = await fetch(`/api/presets/${encodeURIComponent(presetId)}/play`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ duration: 2.0 }),
      })

      if (!response.ok) {
        const error = await response.json()
        throw new Error(error.error || 'Failed to play preset')
      }

      dispatch({ type: 'PLAY_PRESET', payload: presetId })
      toast.success(`Playing ${presetId}`)
    } catch (error) {
      const errorMessage = error instanceof Error ? error.message : 'Unknown error'
      dispatch({ type: 'SET_ERROR', payload: errorMessage })
      toast.error(`Error playing preset: ${errorMessage}`)
    } finally {
      dispatch({ type: 'SET_LOADING', payload: false })
    }
  }, [state.playingPresets])

  const stopPreset = useCallback(async (presetId: string) => {
    if (!state.playingPresets.has(presetId)) {
      return
    }

    try {
      const response = await fetch(`/api/presets/${encodeURIComponent(presetId)}/stop`, {
        method: 'POST',
      })

      if (!response.ok) {
        const error = await response.json()
        throw new Error(error.error || 'Failed to stop preset')
      }

      dispatch({ type: 'STOP_PRESET', payload: presetId })
      toast.success(`Stopped ${presetId}`)
    } catch (error) {
      const errorMessage = error instanceof Error ? error.message : 'Unknown error'
      toast.error(`Error stopping preset: ${errorMessage}`)
    }
  }, [state.playingPresets])

  const stopAll = useCallback(async () => {
    try {
      const response = await fetch('/api/stop-all', {
        method: 'POST',
      })

      if (!response.ok) {
        throw new Error('Failed to stop all presets')
      }

      dispatch({ type: 'STOP_ALL' })
      toast.success('Stopped all presets')
    } catch (error) {
      const errorMessage = error instanceof Error ? error.message : 'Unknown error'
      toast.error(`Error stopping all presets: ${errorMessage}`)
    }
  }, [])

  const setVolume = useCallback((volume: number) => {
    dispatch({ type: 'SET_VOLUME', payload: volume })
  }, [])

  const setMuted = useCallback((muted: boolean) => {
    dispatch({ type: 'SET_MUTED', payload: muted })
  }, [])

  const isPresetPlaying = useCallback((presetId: string) => {
    return state.playingPresets.has(presetId)
  }, [state.playingPresets])

  // Poll for status updates
  useEffect(() => {
    const pollStatus = async () => {
      try {
        const response = await fetch('/api/status')
        if (response.ok) {
          const data = await response.json()
          const playingPresets = new Set(data.playing_sounds || [])
          dispatch({ type: 'SET_PLAYING_PRESETS', payload: playingPresets })
        }
      } catch (error) {
        // Silently fail for status polling
      }
    }

    const interval = setInterval(pollStatus, 1000)
    return () => clearInterval(interval)
  }, [])

  const value: AudioContextType = {
    state,
    playPreset,
    stopPreset,
    stopAll,
    setVolume,
    setMuted,
    isPresetPlaying,
  }

  return (
    <AudioContext.Provider value={value}>
      {children}
    </AudioContext.Provider>
  )
}

export function useAudio() {
  const context = useContext(AudioContext)
  if (context === undefined) {
    throw new Error('useAudio must be used within an AudioProvider')
  }
  return context
}