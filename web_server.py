#!/usr/bin/env python3
"""
Flask Web Server for AI Audio Generator
Provides REST API and serves the web interface for sound preset playback
"""

import json
import os
import sys
import argparse
from typing import Dict, List, Any, Optional
from flask import Flask, jsonify, request, render_template, send_from_directory
from flask_cors import CORS
import threading
import time

# Import our audio interface
from audio_interface import AudioInterface, AudioEngine
import base64
import numpy as np
try:
    import cpp_engine
except Exception:
    cpp_engine = None

app = Flask(__name__)
CORS(app)  # Enable CORS for web interface

# Global audio interface
audio_interface: Optional[AudioInterface] = None

@app.route('/')
def index():
    """Serve the main web interface"""
    return render_template('index.html')

@app.route('/api/presets')
def get_presets():
    """Get all available presets"""
    if not audio_interface:
        return jsonify({'error': 'Audio interface not initialized'}), 500
        
    presets = audio_interface.list_presets()
    return jsonify({
        'presets': presets,
        'total': len(presets)
    })

@app.route('/api/presets/<preset_name>')
def get_preset_info(preset_name: str):
    """Get detailed information about a specific preset"""
    if not audio_interface:
        return jsonify({'error': 'Audio interface not initialized'}), 500
        
    info = audio_interface.get_preset_info(preset_name)
    if not info:
        return jsonify({'error': 'Preset not found'}), 404
        
    return jsonify(info)

@app.route('/api/presets/<preset_name>/play', methods=['POST'])
def play_preset(preset_name: str):
    """Play a sound preset"""
    if not audio_interface:
        return jsonify({'error': 'Audio interface not initialized'}), 500
        
    data = request.get_json() or {}
    duration = data.get('duration', 2.0)
    
    success = audio_interface.play_preset(preset_name, duration)
    if success:
        return jsonify({
            'status': 'playing',
            'preset': preset_name,
            'duration': duration
        })
    else:
        return jsonify({'error': 'Failed to play preset'}), 400

@app.route('/api/presets/<preset_name>/stop', methods=['POST'])
def stop_preset(preset_name: str):
    """Stop playing a sound preset"""
    if not audio_interface:
        return jsonify({'error': 'Audio interface not initialized'}), 500
        
    success = audio_interface.stop_preset(preset_name)
    if success:
        return jsonify({
            'status': 'stopped',
            'preset': preset_name
        })
    else:
        return jsonify({'error': 'Preset not playing'}), 400

@app.route('/api/stop-all', methods=['POST'])
def stop_all():
    """Stop all playing sounds"""
    if not audio_interface:
        return jsonify({'error': 'Audio interface not initialized'}), 500
        
    audio_interface.stop_all()
    return jsonify({'status': 'all_stopped'})

@app.route('/api/status')
def get_status():
    """Get system status"""
    if not audio_interface:
        return jsonify({'error': 'Audio interface not initialized'}), 500
        
    playing_sounds = list(audio_interface.playing_sounds.keys())
    return jsonify({
        'status': 'ready',
        'engine': audio_interface.engine.value,
        'playing_sounds': playing_sounds,
        'total_presets': len(audio_interface.presets),
        'cpp_available': bool(cpp_engine and cpp_engine.is_available())
    })

@app.route('/api/render', methods=['POST'])
def render_audio():
    """Render audio via C++ engine if available, else Python preview.
    Request JSON: { prompt, role?, duration?, return_audio? }
    """
    if not audio_interface:
        return jsonify({'error': 'Audio interface not initialized'}), 500

    data = request.get_json() or {}
    prompt = data.get('prompt')
    role = data.get('role')
    duration = float(data.get('duration', 2.0))
    return_audio = bool(data.get('return_audio', False))

    if not prompt:
        return jsonify({'error': 'prompt is required'}), 400

    # Try C++ engine first
    used_cpp = False
    audio = None
    meta = {}
    if cpp_engine and cpp_engine.is_available():
        try:
            result = cpp_engine.render_sync(prompt, role=role)
            if result is not None:
                audio, meta = result
                used_cpp = True
        except Exception as e:
            meta = {'warning': f'CPP render failed: {e}'}

    # Fallback to Python tone synthesis
    if audio is None:
        try:
            audio = audio_interface.generate_audio_from_preset(prompt if prompt in audio_interface.presets else list(audio_interface.presets.keys())[0], duration)
            meta.setdefault('fallback', True)
        except Exception as e:
            return jsonify({'error': f'Python render failed: {e}'}), 500

    response = {
        'used_cpp': used_cpp,
        'sample_rate': getattr(audio_interface, 'sample_rate', 44100),
        'num_samples': int(audio.shape[0]) if isinstance(audio, np.ndarray) else 0,
        'meta': meta,
    }

    if return_audio and isinstance(audio, np.ndarray):
        # Return base64-encoded float32 little-endian PCM
        pcm_bytes = audio.astype(np.float32).tobytes()
        response['audio_base64'] = base64.b64encode(pcm_bytes).decode('ascii')

    return jsonify(response)

@app.route('/api/categories')
def get_categories():
    """Get all preset categories"""
    if not audio_interface:
        return jsonify({'error': 'Audio interface not initialized'}), 500
        
    categories = {}
    for preset in audio_interface.presets.values():
        category = preset.category
        if category not in categories:
            categories[category] = []
        categories[category].append({
            'name': preset.name,
            'description': preset.description
        })
        
    return jsonify({
        'categories': categories,
        'category_count': len(categories)
    })

@app.route('/api/presets/<preset_name>/parameters')
def get_preset_parameters(preset_name: str):
    """Get detailed parameters for a preset"""
    if not audio_interface:
        return jsonify({'error': 'Audio interface not initialized'}), 500
        
    info = audio_interface.get_preset_info(preset_name)
    if not info:
        return jsonify({'error': 'Preset not found'}), 404
        
    return jsonify({
        'name': preset_name,
        'parameters': info['parameters']
    })

def initialize_audio_interface(json_files: List[str], engine: Optional[AudioEngine] = None):
    """Initialize the audio interface with JSON datasets"""
    global audio_interface
    
    print(f"Initializing audio interface with files: {json_files}")
    audio_interface = AudioInterface(engine)
    
    # Load JSON datasets
    loaded_count = audio_interface.load_json_datasets(json_files)
    print(f"Loaded {loaded_count} presets")
    
    return loaded_count

def create_web_templates():
    """Create the web interface templates"""
    templates_dir = 'templates'
    static_dir = 'static'
    
    os.makedirs(templates_dir, exist_ok=True)
    os.makedirs(static_dir, exist_ok=True)
    os.makedirs(os.path.join(static_dir, 'css'), exist_ok=True)
    os.makedirs(os.path.join(static_dir, 'js'), exist_ok=True)
    
    # Create main HTML template
    html_content = '''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>AI Audio Generator - Sound Presets</title>
    <link rel="stylesheet" href="{{ url_for('static', filename='css/style.css') }}">
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
</head>
<body>
    <div class="app">
        <header class="header">
            <div class="container">
                <h1 class="title">
                    <i class="fas fa-music"></i>
                    AI Audio Generator
                </h1>
                <div class="status" id="status">
                    <span class="status-indicator" id="statusIndicator"></span>
                    <span id="statusText">Loading...</span>
                </div>
            </div>
        </header>

        <main class="main">
            <div class="container">
                <div class="controls">
                    <div class="search-box">
                        <i class="fas fa-search"></i>
                        <input type="text" id="searchInput" placeholder="Search presets...">
                    </div>
                    <div class="filter-controls">
                        <select id="categoryFilter">
                            <option value="">All Categories</option>
                        </select>
                        <button id="stopAllBtn" class="btn btn-danger">
                            <i class="fas fa-stop"></i>
                            Stop All
                        </button>
                    </div>
                </div>

                <div class="presets-grid" id="presetsGrid">
                    <!-- Presets will be loaded here -->
                </div>

                <div class="loading" id="loading">
                    <div class="spinner"></div>
                    <p>Loading presets...</p>
                </div>

                <div class="empty-state" id="emptyState" style="display: none;">
                    <i class="fas fa-music"></i>
                    <h3>No presets found</h3>
                    <p>Try adjusting your search or filter criteria</p>
                </div>
            </div>
        </main>

        <div class="playing-indicator" id="playingIndicator">
            <i class="fas fa-volume-up"></i>
            <span id="playingText">No sounds playing</span>
        </div>
    </div>

    <script src="{{ url_for('static', filename='js/app.js') }}"></script>
</body>
</html>'''
    
    with open(os.path.join(templates_dir, 'index.html'), 'w') as f:
        f.write(html_content)
    
    # Create CSS file
    css_content = '''/* AI Audio Generator - Modern Web Interface */

:root {
    --primary-color: #6366f1;
    --primary-dark: #4f46e5;
    --secondary-color: #8b5cf6;
    --success-color: #10b981;
    --danger-color: #ef4444;
    --warning-color: #f59e0b;
    --background: #f8fafc;
    --surface: #ffffff;
    --surface-hover: #f1f5f9;
    --text-primary: #1e293b;
    --text-secondary: #64748b;
    --text-muted: #94a3b8;
    --border: #e2e8f0;
    --border-hover: #cbd5e1;
    --shadow: 0 1px 3px 0 rgba(0, 0, 0, 0.1), 0 1px 2px 0 rgba(0, 0, 0, 0.06);
    --shadow-lg: 0 10px 15px -3px rgba(0, 0, 0, 0.1), 0 4px 6px -2px rgba(0, 0, 0, 0.05);
    --radius: 0.5rem;
    --radius-lg: 0.75rem;
    --transition: all 0.2s ease-in-out;
}

* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
    background: var(--background);
    color: var(--text-primary);
    line-height: 1.6;
}

.container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 0 1rem;
}

/* Header */
.header {
    background: var(--surface);
    border-bottom: 1px solid var(--border);
    padding: 1rem 0;
    position: sticky;
    top: 0;
    z-index: 100;
}

.header .container {
    display: flex;
    justify-content: space-between;
    align-items: center;
}

.title {
    font-size: 1.5rem;
    font-weight: 700;
    color: var(--primary-color);
    display: flex;
    align-items: center;
    gap: 0.5rem;
}

.status {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    font-size: 0.875rem;
    color: var(--text-secondary);
}

.status-indicator {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: var(--warning-color);
    animation: pulse 2s infinite;
}

.status-indicator.ready {
    background: var(--success-color);
    animation: none;
}

.status-indicator.error {
    background: var(--danger-color);
    animation: none;
}

@keyframes pulse {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.5; }
}

/* Main Content */
.main {
    padding: 2rem 0;
    min-height: calc(100vh - 80px);
}

.controls {
    display: flex;
    gap: 1rem;
    margin-bottom: 2rem;
    flex-wrap: wrap;
    align-items: center;
}

.search-box {
    position: relative;
    flex: 1;
    min-width: 200px;
}

.search-box i {
    position: absolute;
    left: 0.75rem;
    top: 50%;
    transform: translateY(-50%);
    color: var(--text-muted);
}

.search-box input {
    width: 100%;
    padding: 0.75rem 0.75rem 0.75rem 2.5rem;
    border: 1px solid var(--border);
    border-radius: var(--radius);
    font-size: 0.875rem;
    transition: var(--transition);
    background: var(--surface);
}

.search-box input:focus {
    outline: none;
    border-color: var(--primary-color);
    box-shadow: 0 0 0 3px rgba(99, 102, 241, 0.1);
}

.filter-controls {
    display: flex;
    gap: 0.75rem;
    align-items: center;
}

.filter-controls select {
    padding: 0.75rem;
    border: 1px solid var(--border);
    border-radius: var(--radius);
    background: var(--surface);
    font-size: 0.875rem;
    cursor: pointer;
    transition: var(--transition);
}

.filter-controls select:focus {
    outline: none;
    border-color: var(--primary-color);
}

/* Buttons */
.btn {
    display: inline-flex;
    align-items: center;
    gap: 0.5rem;
    padding: 0.75rem 1rem;
    border: none;
    border-radius: var(--radius);
    font-size: 0.875rem;
    font-weight: 500;
    cursor: pointer;
    transition: var(--transition);
    text-decoration: none;
}

.btn-primary {
    background: var(--primary-color);
    color: white;
}

.btn-primary:hover {
    background: var(--primary-dark);
    transform: translateY(-1px);
    box-shadow: var(--shadow-lg);
}

.btn-danger {
    background: var(--danger-color);
    color: white;
}

.btn-danger:hover {
    background: #dc2626;
    transform: translateY(-1px);
}

.btn:disabled {
    opacity: 0.5;
    cursor: not-allowed;
    transform: none;
}

/* Presets Grid */
.presets-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
    gap: 1.5rem;
    margin-bottom: 2rem;
}

.preset-tile {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: var(--radius-lg);
    padding: 1.5rem;
    cursor: pointer;
    transition: var(--transition);
    position: relative;
    overflow: hidden;
}

.preset-tile:hover {
    border-color: var(--primary-color);
    box-shadow: var(--shadow-lg);
    transform: translateY(-2px);
}

.preset-tile.playing {
    border-color: var(--success-color);
    background: linear-gradient(135deg, var(--surface) 0%, rgba(16, 185, 129, 0.05) 100%);
}

.preset-tile.playing::before {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    right: 0;
    height: 3px;
    background: linear-gradient(90deg, var(--success-color), var(--primary-color));
}

.preset-header {
    display: flex;
    justify-content: space-between;
    align-items: flex-start;
    margin-bottom: 1rem;
}

.preset-name {
    font-size: 1.125rem;
    font-weight: 600;
    color: var(--text-primary);
    margin-bottom: 0.25rem;
}

.preset-category {
    font-size: 0.75rem;
    color: var(--text-muted);
    background: var(--background);
    padding: 0.25rem 0.5rem;
    border-radius: var(--radius);
    text-transform: uppercase;
    letter-spacing: 0.05em;
}

.preset-description {
    color: var(--text-secondary);
    font-size: 0.875rem;
    line-height: 1.5;
    margin-bottom: 1rem;
}

.preset-controls {
    display: flex;
    gap: 0.5rem;
    align-items: center;
}

.play-btn {
    flex: 1;
    justify-content: center;
    background: var(--primary-color);
    color: white;
    border: none;
    padding: 0.75rem;
    border-radius: var(--radius);
    font-size: 0.875rem;
    font-weight: 500;
    cursor: pointer;
    transition: var(--transition);
}

.play-btn:hover {
    background: var(--primary-dark);
}

.play-btn.playing {
    background: var(--danger-color);
}

.play-btn:disabled {
    opacity: 0.5;
    cursor: not-allowed;
}

.info-btn {
    background: transparent;
    border: 1px solid var(--border);
    color: var(--text-secondary);
    padding: 0.75rem;
    border-radius: var(--radius);
    cursor: pointer;
    transition: var(--transition);
}

.info-btn:hover {
    background: var(--surface-hover);
    border-color: var(--border-hover);
}

/* Loading and Empty States */
.loading {
    text-align: center;
    padding: 3rem 0;
    color: var(--text-secondary);
}

.spinner {
    width: 40px;
    height: 40px;
    border: 3px solid var(--border);
    border-top: 3px solid var(--primary-color);
    border-radius: 50%;
    animation: spin 1s linear infinite;
    margin: 0 auto 1rem;
}

@keyframes spin {
    0% { transform: rotate(0deg); }
    100% { transform: rotate(360deg); }
}

.empty-state {
    text-align: center;
    padding: 3rem 0;
    color: var(--text-muted);
}

.empty-state i {
    font-size: 3rem;
    margin-bottom: 1rem;
    opacity: 0.5;
}

.empty-state h3 {
    font-size: 1.25rem;
    margin-bottom: 0.5rem;
    color: var(--text-secondary);
}

/* Playing Indicator */
.playing-indicator {
    position: fixed;
    bottom: 2rem;
    right: 2rem;
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: var(--radius-lg);
    padding: 1rem 1.5rem;
    box-shadow: var(--shadow-lg);
    display: flex;
    align-items: center;
    gap: 0.75rem;
    font-size: 0.875rem;
    color: var(--text-secondary);
    z-index: 1000;
    transition: var(--transition);
}

.playing-indicator.playing {
    border-color: var(--success-color);
    background: linear-gradient(135deg, var(--surface) 0%, rgba(16, 185, 129, 0.05) 100%);
}

.playing-indicator i {
    color: var(--success-color);
    animation: pulse 2s infinite;
}

/* Responsive Design */
@media (max-width: 768px) {
    .container {
        padding: 0 0.75rem;
    }
    
    .header .container {
        flex-direction: column;
        gap: 1rem;
        text-align: center;
    }
    
    .controls {
        flex-direction: column;
        align-items: stretch;
    }
    
    .search-box {
        min-width: auto;
    }
    
    .filter-controls {
        justify-content: space-between;
    }
    
    .presets-grid {
        grid-template-columns: 1fr;
        gap: 1rem;
    }
    
    .preset-tile {
        padding: 1rem;
    }
    
    .playing-indicator {
        bottom: 1rem;
        right: 1rem;
        left: 1rem;
        text-align: center;
    }
}

/* Accessibility */
@media (prefers-reduced-motion: reduce) {
    * {
        animation-duration: 0.01ms !important;
        animation-iteration-count: 1 !important;
        transition-duration: 0.01ms !important;
    }
}

/* Focus styles for keyboard navigation */
.preset-tile:focus,
.btn:focus,
input:focus,
select:focus {
    outline: 2px solid var(--primary-color);
    outline-offset: 2px;
}

/* High contrast mode support */
@media (prefers-contrast: high) {
    :root {
        --border: #000000;
        --text-secondary: #000000;
    }
}

/* Dark mode support */
@media (prefers-color-scheme: dark) {
    :root {
        --background: #0f172a;
        --surface: #1e293b;
        --surface-hover: #334155;
        --text-primary: #f1f5f9;
        --text-secondary: #cbd5e1;
        --text-muted: #64748b;
        --border: #334155;
        --border-hover: #475569;
    }
}'''
    
    with open(os.path.join(static_dir, 'css', 'style.css'), 'w') as f:
        f.write(css_content)
    
    # Create JavaScript file
    js_content = '''// AI Audio Generator - Web Interface JavaScript

class AudioApp {
    constructor() {
        this.presets = [];
        this.filteredPresets = [];
        this.playingPresets = new Set();
        this.currentCategory = '';
        this.searchQuery = '';
        
        this.init();
    }
    
    async init() {
        this.setupEventListeners();
        await this.loadPresets();
        this.updateStatus('ready', 'Ready');
    }
    
    setupEventListeners() {
        // Search input
        const searchInput = document.getElementById('searchInput');
        searchInput.addEventListener('input', (e) => {
            this.searchQuery = e.target.value.toLowerCase();
            this.filterPresets();
        });
        
        // Category filter
        const categoryFilter = document.getElementById('categoryFilter');
        categoryFilter.addEventListener('change', (e) => {
            this.currentCategory = e.target.value;
            this.filterPresets();
        });
        
        // Stop all button
        const stopAllBtn = document.getElementById('stopAllBtn');
        stopAllBtn.addEventListener('click', () => {
            this.stopAllPresets();
        });
        
        // Keyboard shortcuts
        document.addEventListener('keydown', (e) => {
            if (e.key === 'Escape') {
                this.stopAllPresets();
            }
        });
    }
    
    async loadPresets() {
        try {
            this.updateStatus('loading', 'Loading presets...');
            
            const response = await fetch('/api/presets');
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            
            const data = await response.json();
            this.presets = data.presets;
            this.filteredPresets = [...this.presets];
            
            this.populateCategoryFilter();
            this.renderPresets();
            this.updateStatus('ready', `Ready - ${this.presets.length} presets loaded`);
            
        } catch (error) {
            console.error('Error loading presets:', error);
            this.updateStatus('error', `Error: ${error.message}`);
        }
    }
    
    populateCategoryFilter() {
        const categoryFilter = document.getElementById('categoryFilter');
        const categories = [...new Set(this.presets.map(p => p.category))].sort();
        
        // Clear existing options except "All Categories"
        categoryFilter.innerHTML = '<option value="">All Categories</option>';
        
        categories.forEach(category => {
            const option = document.createElement('option');
            option.value = category;
            option.textContent = category;
            categoryFilter.appendChild(option);
        });
    }
    
    filterPresets() {
        this.filteredPresets = this.presets.filter(preset => {
            const matchesSearch = preset.name.toLowerCase().includes(this.searchQuery) ||
                                preset.description.toLowerCase().includes(this.searchQuery);
            const matchesCategory = !this.currentCategory || preset.category === this.currentCategory;
            
            return matchesSearch && matchesCategory;
        });
        
        this.renderPresets();
    }
    
    renderPresets() {
        const grid = document.getElementById('presetsGrid');
        const loading = document.getElementById('loading');
        const emptyState = document.getElementById('emptyState');
        
        loading.style.display = 'none';
        
        if (this.filteredPresets.length === 0) {
            grid.innerHTML = '';
            emptyState.style.display = 'block';
            return;
        }
        
        emptyState.style.display = 'none';
        
        grid.innerHTML = this.filteredPresets.map(preset => `
            <div class="preset-tile ${this.playingPresets.has(preset.name) ? 'playing' : ''}" 
                 data-preset="${preset.name}">
                <div class="preset-header">
                    <div>
                        <div class="preset-name">${this.escapeHtml(preset.name)}</div>
                        <div class="preset-category">${this.escapeHtml(preset.category)}</div>
                    </div>
                </div>
                
                <div class="preset-description">
                    ${this.escapeHtml(preset.description)}
                </div>
                
                <div class="preset-controls">
                    <button class="play-btn ${this.playingPresets.has(preset.name) ? 'playing' : ''}"
                            data-preset="${preset.name}"
                            ${this.playingPresets.has(preset.name) ? 'disabled' : ''}>
                        <i class="fas ${this.playingPresets.has(preset.name) ? 'fa-stop' : 'fa-play'}"></i>
                        ${this.playingPresets.has(preset.name) ? 'Stop' : 'Play'}
                    </button>
                    <button class="info-btn" data-preset="${preset.name}" title="Show details">
                        <i class="fas fa-info"></i>
                    </button>
                </div>
            </div>
        `).join('');
        
        // Add event listeners to preset tiles
        grid.addEventListener('click', (e) => {
            const presetName = e.target.closest('[data-preset]')?.dataset.preset;
            if (!presetName) return;
            
            if (e.target.classList.contains('play-btn') || e.target.closest('.play-btn')) {
                this.togglePreset(presetName);
            } else if (e.target.classList.contains('info-btn') || e.target.closest('.info-btn')) {
                this.showPresetInfo(presetName);
            } else {
                // Click anywhere else on the tile to play/stop
                this.togglePreset(presetName);
            }
        });
    }
    
    async togglePreset(presetName) {
        if (this.playingPresets.has(presetName)) {
            await this.stopPreset(presetName);
        } else {
            await this.playPreset(presetName);
        }
    }
    
    async playPreset(presetName) {
        try {
            const response = await fetch(`/api/presets/${encodeURIComponent(presetName)}/play`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ duration: 2.0 })
            });
            
            if (!response.ok) {
                const error = await response.json();
                throw new Error(error.error || 'Failed to play preset');
            }
            
            this.playingPresets.add(presetName);
            this.updatePresetUI(presetName);
            this.updatePlayingIndicator();
            
        } catch (error) {
            console.error('Error playing preset:', error);
            alert(`Error playing preset: ${error.message}`);
        }
    }
    
    async stopPreset(presetName) {
        try {
            const response = await fetch(`/api/presets/${encodeURIComponent(presetName)}/stop`, {
                method: 'POST'
            });
            
            if (!response.ok) {
                const error = await response.json();
                throw new Error(error.error || 'Failed to stop preset');
            }
            
            this.playingPresets.delete(presetName);
            this.updatePresetUI(presetName);
            this.updatePlayingIndicator();
            
        } catch (error) {
            console.error('Error stopping preset:', error);
        }
    }
    
    async stopAllPresets() {
        try {
            const response = await fetch('/api/stop-all', {
                method: 'POST'
            });
            
            if (!response.ok) {
                throw new Error('Failed to stop all presets');
            }
            
            this.playingPresets.clear();
            this.renderPresets();
            this.updatePlayingIndicator();
            
        } catch (error) {
            console.error('Error stopping all presets:', error);
        }
    }
    
    updatePresetUI(presetName) {
        const tile = document.querySelector(`[data-preset="${presetName}"]`);
        if (!tile) return;
        
        const isPlaying = this.playingPresets.has(presetName);
        const playBtn = tile.querySelector('.play-btn');
        
        tile.classList.toggle('playing', isPlaying);
        playBtn.classList.toggle('playing', isPlaying);
        playBtn.disabled = isPlaying;
        playBtn.innerHTML = `
            <i class="fas ${isPlaying ? 'fa-stop' : 'fa-play'}"></i>
            ${isPlaying ? 'Stop' : 'Play'}
        `;
    }
    
    updatePlayingIndicator() {
        const indicator = document.getElementById('playingIndicator');
        const text = document.getElementById('playingText');
        
        if (this.playingPresets.size === 0) {
            indicator.classList.remove('playing');
            text.textContent = 'No sounds playing';
        } else {
            indicator.classList.add('playing');
            const presetNames = Array.from(this.playingPresets).join(', ');
            text.textContent = `Playing: ${presetNames}`;
        }
    }
    
    async showPresetInfo(presetName) {
        try {
            const response = await fetch(`/api/presets/${encodeURIComponent(presetName)}`);
            if (!response.ok) {
                throw new Error('Failed to load preset info');
            }
            
            const preset = await response.json();
            
            // Create a simple modal or alert with preset information
            const info = `
Preset: ${preset.name}
Category: ${preset.category}
Description: ${preset.description}

Parameters:
${JSON.stringify(preset.parameters, null, 2)}
            `;
            
            alert(info);
            
        } catch (error) {
            console.error('Error loading preset info:', error);
            alert(`Error loading preset info: ${error.message}`);
        }
    }
    
    updateStatus(type, message) {
        const statusIndicator = document.getElementById('statusIndicator');
        const statusText = document.getElementById('statusText');
        
        statusIndicator.className = `status-indicator ${type}`;
        statusText.textContent = message;
    }
    
    escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }
}

// Initialize the app when the DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new AudioApp();
});

// Handle page visibility changes to pause/resume audio
document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        // Page is hidden, could pause audio here if needed
    } else {
        // Page is visible again
    }
});

// Handle beforeunload to clean up
window.addEventListener('beforeunload', () => {
    // Stop all playing sounds before leaving
    fetch('/api/stop-all', { method: 'POST' }).catch(() => {});
});'''
    
    with open(os.path.join(static_dir, 'js', 'app.js'), 'w') as f:
        f.write(js_content)

def main():
    """Main function for the web server"""
    parser = argparse.ArgumentParser(description='AI Audio Generator Web Server')
    parser.add_argument('--host', default='127.0.0.1', help='Host to bind to')
    parser.add_argument('--port', type=int, default=8080, help='Port to bind to')
    parser.add_argument('--debug', action='store_true', help='Enable debug mode')
    parser.add_argument('--json-files', nargs='+', 
                       default=['electronic_track.json', 'guitar.json', 'group.json'],
                       help='JSON files to load')
    parser.add_argument('--engine', choices=['pygame', 'sounddevice', 'mock'], 
                       default=None, help='Audio engine to use')
    
    args = parser.parse_args()
    
    # Create web templates
    create_web_templates()
    
    # Initialize audio interface
    engine = AudioEngine(args.engine) if args.engine else None
    loaded_count = initialize_audio_interface(args.json_files, engine)
    
    if loaded_count == 0:
        print("Warning: No presets loaded. Check your JSON files.")
    
    print(f"Starting web server on http://{args.host}:{args.port}")
    print(f"Loaded {loaded_count} presets from {len(args.json_files)} JSON files")
    print("Press Ctrl+C to stop the server")
    
    # Start the Flask server
    app.run(
        host=args.host,
        port=args.port,
        debug=args.debug,
        threaded=True
    )

if __name__ == '__main__':
    main()