import React from 'react'
import { Volume2, Wifi, WifiOff } from 'lucide-react'
import { useAudio } from '../contexts/AudioContext'

export default function StatusBar() {
  const { state } = useAudio()
  const [isOnline, setIsOnline] = React.useState(navigator.onLine)

  React.useEffect(() => {
    const handleOnline = () => setIsOnline(true)
    const handleOffline = () => setIsOnline(false)

    window.addEventListener('online', handleOnline)
    window.addEventListener('offline', handleOffline)

    return () => {
      window.removeEventListener('online', handleOnline)
      window.removeEventListener('offline', handleOffline)
    }
  }, [])

  return (
    <div className="fixed bottom-0 left-0 right-0 bg-white border-t border-gray-200 px-4 py-2">
      <div className="container mx-auto flex items-center justify-between text-sm text-gray-500">
        {/* Left side - Audio status */}
        <div className="flex items-center space-x-4">
          {state.playingPresets.size > 0 ? (
            <div className="flex items-center space-x-2">
              <Volume2 className="w-4 h-4 text-success-500" />
              <span>
                Playing {state.playingPresets.size} sound{state.playingPresets.size !== 1 ? 's' : ''}
              </span>
            </div>
          ) : (
            <div className="flex items-center space-x-2">
              <Volume2 className="w-4 h-4 text-gray-400" />
              <span>No sounds playing</span>
            </div>
          )}
        </div>

        {/* Right side - Connection status */}
        <div className="flex items-center space-x-4">
          <div className="flex items-center space-x-2">
            {isOnline ? (
              <Wifi className="w-4 h-4 text-success-500" />
            ) : (
              <WifiOff className="w-4 h-4 text-danger-500" />
            )}
            <span>{isOnline ? 'Connected' : 'Offline'}</span>
          </div>
          
          {state.isLoading && (
            <div className="flex items-center space-x-2">
              <div className="w-2 h-2 bg-primary-500 rounded-full animate-pulse"></div>
              <span>Loading...</span>
            </div>
          )}
        </div>
      </div>
    </div>
  )
}