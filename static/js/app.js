// AI Audio Generator - Web Interface JavaScript

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
});