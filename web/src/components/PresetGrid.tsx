import React from 'react'
import { Search, Filter, Grid, List } from 'lucide-react'
import { usePresets, useAudio } from '../contexts'
import PresetTile from './PresetTile'
import LoadingSpinner from './LoadingSpinner'
import EmptyState from './EmptyState'

export default function PresetGrid() {
  const { state, setSearchQuery, setSelectedCategory } = usePresets()
  const { isPresetPlaying } = useAudio()

  const handleSearchChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    setSearchQuery(e.target.value)
  }

  const handleCategoryChange = (e: React.ChangeEvent<HTMLSelectElement>) => {
    setSelectedCategory(e.target.value)
  }

  if (state.isLoading) {
    return <LoadingSpinner />
  }

  if (state.error) {
    return (
      <div className="text-center py-12">
        <div className="text-red-500 text-lg font-medium mb-2">Error Loading Presets</div>
        <div className="text-gray-500 mb-4">{state.error}</div>
        <button
          onClick={() => window.location.reload()}
          className="btn btn-primary"
        >
          Retry
        </button>
      </div>
    )
  }

  return (
    <div className="space-y-6">
      {/* Search and Filter Controls */}
      <div className="bg-white rounded-lg shadow-sm border border-gray-200 p-6">
        <div className="flex flex-col sm:flex-row gap-4">
          {/* Search Input */}
          <div className="flex-1 relative">
            <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-gray-400 w-5 h-5" />
            <input
              type="text"
              placeholder="Search presets..."
              value={state.searchQuery}
              onChange={handleSearchChange}
              className="input pl-10 w-full"
            />
          </div>

          {/* Category Filter */}
          <div className="sm:w-64">
            <div className="relative">
              <Filter className="absolute left-3 top-1/2 transform -translate-y-1/2 text-gray-400 w-5 h-5" />
              <select
                value={state.selectedCategory}
                onChange={handleCategoryChange}
                className="input pl-10 w-full appearance-none"
              >
                <option value="">All Categories</option>
                {state.categories.map(category => (
                  <option key={category} value={category}>
                    {category}
                  </option>
                ))}
              </select>
            </div>
          </div>

          {/* View Toggle */}
          <div className="flex items-center space-x-2">
            <button className="p-2 rounded-lg bg-primary-100 text-primary-600">
              <Grid className="w-5 h-5" />
            </button>
            <button className="p-2 rounded-lg hover:bg-gray-100 text-gray-600">
              <List className="w-5 h-5" />
            </button>
          </div>
        </div>

        {/* Results Summary */}
        <div className="mt-4 text-sm text-gray-500">
          Showing {state.filteredPresets.length} of {state.presets.length} presets
          {state.searchQuery && ` matching "${state.searchQuery}"`}
          {state.selectedCategory && ` in ${state.selectedCategory}`}
        </div>
      </div>

      {/* Presets Grid */}
      {state.filteredPresets.length === 0 ? (
        <EmptyState
          searchQuery={state.searchQuery}
          selectedCategory={state.selectedCategory}
        />
      ) : (
        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-6">
          {state.filteredPresets.map(preset => (
            <PresetTile
              key={preset.name}
              preset={preset}
              isPlaying={isPresetPlaying(preset.name)}
            />
          ))}
        </div>
      )}
    </div>
  )
}