import React from 'react'
import { Search, Filter, Music } from 'lucide-react'

interface EmptyStateProps {
  searchQuery: string
  selectedCategory: string
}

export default function EmptyState({ searchQuery, selectedCategory }: EmptyStateProps) {
  const hasFilters = searchQuery || selectedCategory

  return (
    <div className="text-center py-12">
      <div className="flex justify-center mb-4">
        {hasFilters ? (
          <Search className="w-16 h-16 text-gray-300" />
        ) : (
          <Music className="w-16 h-16 text-gray-300" />
        )}
      </div>
      
      <h3 className="text-lg font-medium text-gray-900 mb-2">
        {hasFilters ? 'No presets found' : 'No presets available'}
      </h3>
      
      <p className="text-gray-500 mb-6 max-w-md mx-auto">
        {hasFilters ? (
          <>
            No presets match your current filters.
            {searchQuery && (
              <>
                <br />
                Try adjusting your search query or category filter.
              </>
            )}
          </>
        ) : (
          'No presets have been loaded yet. Please check your configuration.'
        )}
      </p>

      {hasFilters && (
        <div className="flex flex-col sm:flex-row gap-3 justify-center">
          {searchQuery && (
            <button
              onClick={() => {
                // Clear search query
                const searchInput = document.querySelector('input[type="text"]') as HTMLInputElement
                if (searchInput) {
                  searchInput.value = ''
                  searchInput.dispatchEvent(new Event('input', { bubbles: true }))
                }
              }}
              className="btn btn-outline btn-sm"
            >
              <Search className="w-4 h-4 mr-2" />
              Clear Search
            </button>
          )}
          
          {selectedCategory && (
            <button
              onClick={() => {
                // Clear category filter
                const categorySelect = document.querySelector('select') as HTMLSelectElement
                if (categorySelect) {
                  categorySelect.value = ''
                  categorySelect.dispatchEvent(new Event('change', { bubbles: true }))
                }
              }}
              className="btn btn-outline btn-sm"
            >
              <Filter className="w-4 h-4 mr-2" />
              Clear Filter
            </button>
          )}
        </div>
      )}
    </div>
  )
}