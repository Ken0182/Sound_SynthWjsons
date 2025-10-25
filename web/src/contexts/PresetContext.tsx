import React, { createContext, useContext, useReducer, useCallback, useEffect } from 'react'
import { toast } from 'react-hot-toast'

export interface Preset {
  name: string
  category: string
  description: string
  parameters?: Record<string, any>
  is_playing?: boolean
}

export interface PresetState {
  presets: Preset[]
  categories: string[]
  filteredPresets: Preset[]
  searchQuery: string
  selectedCategory: string
  isLoading: boolean
  error: string | null
}

type PresetAction =
  | { type: 'SET_LOADING'; payload: boolean }
  | { type: 'SET_ERROR'; payload: string | null }
  | { type: 'SET_PRESETS'; payload: Preset[] }
  | { type: 'SET_CATEGORIES'; payload: string[] }
  | { type: 'SET_SEARCH_QUERY'; payload: string }
  | { type: 'SET_SELECTED_CATEGORY'; payload: string }
  | { type: 'FILTER_PRESETS' }

const initialState: PresetState = {
  presets: [],
  categories: [],
  filteredPresets: [],
  searchQuery: '',
  selectedCategory: '',
  isLoading: false,
  error: null,
}

function presetReducer(state: PresetState, action: PresetAction): PresetState {
  switch (action.type) {
    case 'SET_LOADING':
      return { ...state, isLoading: action.payload }
    case 'SET_ERROR':
      return { ...state, error: action.payload, isLoading: false }
    case 'SET_PRESETS':
      return { ...state, presets: action.payload, isLoading: false }
    case 'SET_CATEGORIES':
      return { ...state, categories: action.payload }
    case 'SET_SEARCH_QUERY':
      return { ...state, searchQuery: action.payload }
    case 'SET_SELECTED_CATEGORY':
      return { ...state, selectedCategory: action.payload }
    case 'FILTER_PRESETS': {
      const { presets, searchQuery, selectedCategory } = state
      let filtered = presets

      // Filter by category
      if (selectedCategory) {
        filtered = filtered.filter(preset => preset.category === selectedCategory)
      }

      // Filter by search query
      if (searchQuery) {
        const query = searchQuery.toLowerCase()
        filtered = filtered.filter(preset =>
          preset.name.toLowerCase().includes(query) ||
          preset.description.toLowerCase().includes(query) ||
          preset.category.toLowerCase().includes(query)
        )
      }

      return { ...state, filteredPresets: filtered }
    }
    default:
      return state
  }
}

interface PresetContextType {
  state: PresetState
  loadPresets: () => Promise<void>
  setSearchQuery: (query: string) => void
  setSelectedCategory: (category: string) => void
  getPresetById: (id: string) => Preset | undefined
}

const PresetContext = createContext<PresetContextType | undefined>(undefined)

export function PresetProvider({ children }: { children: React.ReactNode }) {
  const [state, dispatch] = useReducer(presetReducer, initialState)

  const loadPresets = useCallback(async () => {
    dispatch({ type: 'SET_LOADING', payload: true })
    dispatch({ type: 'SET_ERROR', payload: null })

    try {
      const response = await fetch('/api/presets')
      if (!response.ok) {
        throw new Error(`HTTP ${response.status}: ${response.statusText}`)
      }

      const data = await response.json()
      const presets = data.presets || []
      
      // Extract unique categories
      const categories = Array.from(new Set(presets.map((preset: Preset) => preset.category))).sort()
      
      dispatch({ type: 'SET_PRESETS', payload: presets })
      dispatch({ type: 'SET_CATEGORIES', payload: categories })
      dispatch({ type: 'FILTER_PRESETS' })
      
      toast.success(`Loaded ${presets.length} presets`)
    } catch (error) {
      const errorMessage = error instanceof Error ? error.message : 'Unknown error'
      dispatch({ type: 'SET_ERROR', payload: errorMessage })
      toast.error(`Error loading presets: ${errorMessage}`)
    }
  }, [])

  const setSearchQuery = useCallback((query: string) => {
    dispatch({ type: 'SET_SEARCH_QUERY', payload: query })
    dispatch({ type: 'FILTER_PRESETS' })
  }, [])

  const setSelectedCategory = useCallback((category: string) => {
    dispatch({ type: 'SET_SELECTED_CATEGORY', payload: category })
    dispatch({ type: 'FILTER_PRESETS' })
  }, [])

  const getPresetById = useCallback((id: string) => {
    return state.presets.find(preset => preset.name === id)
  }, [state.presets])

  // Load presets on mount
  useEffect(() => {
    loadPresets()
  }, [loadPresets])

  // Filter presets when search query or category changes
  useEffect(() => {
    dispatch({ type: 'FILTER_PRESETS' })
  }, [state.searchQuery, state.selectedCategory, state.presets])

  const value: PresetContextType = {
    state,
    loadPresets,
    setSearchQuery,
    setSelectedCategory,
    getPresetById,
  }

  return (
    <PresetContext.Provider value={value}>
      {children}
    </PresetContext.Provider>
  )
}

export function usePresets() {
  const context = useContext(PresetContext)
  if (context === undefined) {
    throw new Error('usePresets must be used within a PresetProvider')
  }
  return context
}