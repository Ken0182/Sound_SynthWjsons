# AI Audio Generator - Web Interface

Modern React/Vite web interface for the AI Audio Generator.

## Features

- 🎵 One-click sound preview
- 🔍 Real-time search and filtering
- 📊 Quality metrics display
- 🎨 Modern, responsive UI
- ⚡ Low-latency audio playback
- 🌓 Dark mode support

## Development

```bash
# Install dependencies
npm install

# Start dev server (with API proxy)
npm run dev

# Build for production
npm run build

# Preview production build
npm run preview
```

## Architecture

```
┌─────────────────────────────────┐
│     React/Vite Frontend         │
│  - Preset Grid                  │
│  - Search & Filter              │
│  - Real-time Playback Status    │
└───────────┬─────────────────────┘
            │ HTTP API
┌───────────▼─────────────────────┐
│     Flask Backend               │
│  - REST API                     │
│  - Audio Interface              │
│  - C++ Engine Bridge            │
└─────────────────────────────────┘
```

## API Endpoints

- `GET /api/presets` - Get all presets
- `GET /api/presets/:name` - Get preset details
- `POST /api/presets/:name/play` - Play preset
- `POST /api/presets/:name/stop` - Stop preset
- `POST /api/render` - Render audio with C++ engine
- `GET /api/engine/status` - Get C++ engine status
- `GET /api/quality/:name` - Get preset quality metrics

## Tech Stack

- React 18
- Vite 5
- Modern CSS (no frameworks)
- Native Web Audio API

## Browser Support

- Chrome/Edge 90+
- Firefox 88+
- Safari 14+

## Performance

- Initial load: < 1s
- Click-to-sound: < 100ms
- Search latency: < 50ms
- 60 FPS animations
