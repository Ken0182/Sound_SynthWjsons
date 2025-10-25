#!/usr/bin/env python3
"""
Demo script for AI Audio Generator
Tests the complete system including JSON loading, audio generation, and web interface
"""

import os
import sys
import time
import subprocess
import webbrowser
from pathlib import Path

def check_dependencies():
    """Check if required dependencies are available"""
    print("Checking dependencies...")
    
    # Check Python packages
    try:
        import flask
        import pygame
        import sounddevice
        import numpy
        print("✓ Python dependencies available")
    except ImportError as e:
        print(f"✗ Missing Python dependency: {e}")
        print("Run: pip install -r requirements.txt")
        return False
    
    # Check JSON files
    json_files = ['electronic_track.json', 'guitar.json', 'group.json']
    missing_files = [f for f in json_files if not os.path.exists(f)]
    
    if missing_files:
        print(f"✗ Missing JSON files: {missing_files}")
        return False
    else:
        print("✓ JSON files found")
    
    return True

def test_audio_interface():
    """Test the audio interface"""
    print("\nTesting audio interface...")
    
    try:
        from audio_interface import AudioInterface, AudioEngine
        
        # Create audio interface
        audio_interface = AudioInterface(AudioEngine.MOCK)  # Use mock for testing
        
        # Load JSON datasets
        json_files = ['electronic_track.json', 'guitar.json', 'group.json']
        loaded_count = audio_interface.load_json_datasets(json_files)
        
        print(f"✓ Loaded {loaded_count} presets")
        
        # List some presets
        presets = audio_interface.list_presets()
        if presets:
            print(f"✓ Sample presets:")
            for i, preset in enumerate(presets[:5]):
                print(f"  - {preset['category']}: {preset['name']}")
            if len(presets) > 5:
                print(f"  ... and {len(presets) - 5} more")
        
        # Test audio generation (mock mode)
        if presets:
            first_preset = presets[0]['name']
            print(f"✓ Testing audio generation for: {first_preset}")
            audio_data = audio_interface.generate_audio_from_preset(first_preset, duration=0.1)
            if audio_data is not None:
                print(f"✓ Generated audio: {audio_data.shape} samples")
            else:
                print("✗ Failed to generate audio")
        
        return True
        
    except Exception as e:
        print(f"✗ Audio interface test failed: {e}")
        return False

def test_web_server():
    """Test the web server startup"""
    print("\nTesting web server...")
    
    try:
        from web_server import create_web_templates
        
        # Create templates
        create_web_templates()
        print("✓ Web templates created")
        
        # Check if templates exist
        if os.path.exists('templates/index.html') and os.path.exists('static/css/style.css'):
            print("✓ Web interface files ready")
            return True
        else:
            print("✗ Web interface files missing")
            return False
            
    except Exception as e:
        print(f"✗ Web server test failed: {e}")
        return False

def run_web_demo():
    """Run the web demo"""
    print("\nStarting web demo...")
    print("The web interface will open in your browser.")
    print("Press Ctrl+C to stop the server.")
    
    try:
        # Start the web server
        from web_server import main as run_web_server
        
        # Run in a separate process to allow for graceful shutdown
        import threading
        import signal
        
        def signal_handler(sig, frame):
            print("\nShutting down web server...")
            sys.exit(0)
        
        signal.signal(signal.SIGINT, signal_handler)
        
        # Start web server
        run_web_server()
        
    except KeyboardInterrupt:
        print("\nWeb demo stopped by user")
    except Exception as e:
        print(f"✗ Web demo failed: {e}")

def run_audio_demo():
    """Run the audio demo"""
    print("\nRunning audio demo...")
    
    try:
        from audio_interface import AudioInterface, AudioEngine
        
        # Create audio interface with real audio engine
        audio_interface = AudioInterface()
        
        # Load presets
        json_files = ['electronic_track.json', 'guitar.json', 'group.json']
        loaded_count = audio_interface.load_json_datasets(json_files)
        
        if loaded_count == 0:
            print("No presets loaded, cannot run audio demo")
            return
        
        print(f"Loaded {loaded_count} presets")
        
        # Get some presets to demo
        presets = audio_interface.list_presets()
        demo_presets = presets[:3]  # Demo first 3 presets
        
        print(f"Playing {len(demo_presets)} presets...")
        
        for i, preset in enumerate(demo_presets):
            print(f"\n{i+1}. Playing: {preset['name']} ({preset['category']})")
            print(f"   Description: {preset['description']}")
            
            # Play the preset
            success = audio_interface.play_preset(preset['name'], duration=2.0)
            
            if success:
                print("   ✓ Playing...")
                time.sleep(2.5)  # Wait for playback
                audio_interface.stop_preset(preset['name'])
                print("   ✓ Stopped")
            else:
                print("   ✗ Failed to play")
        
        print("\nAudio demo completed!")
        
    except Exception as e:
        print(f"✗ Audio demo failed: {e}")

def main():
    """Main demo function"""
    print("AI Audio Generator - Demo Script")
    print("=" * 40)
    
    # Check dependencies
    if not check_dependencies():
        print("\nPlease install missing dependencies and try again.")
        return
    
    # Test components
    if not test_audio_interface():
        print("\nAudio interface test failed. Please check the implementation.")
        return
    
    if not test_web_server():
        print("\nWeb server test failed. Please check the implementation.")
        return
    
    print("\n✓ All tests passed!")
    
    # Ask user what demo to run
    print("\nChoose a demo:")
    print("1. Web interface demo (opens in browser)")
    print("2. Audio demo (plays sounds in terminal)")
    print("3. Both")
    print("4. Exit")
    
    try:
        choice = input("\nEnter your choice (1-4): ").strip()
        
        if choice == '1':
            run_web_demo()
        elif choice == '2':
            run_audio_demo()
        elif choice == '3':
            print("\nRunning audio demo first...")
            run_audio_demo()
            print("\nNow starting web demo...")
            run_web_demo()
        elif choice == '4':
            print("Goodbye!")
        else:
            print("Invalid choice. Please run the demo again.")
            
    except KeyboardInterrupt:
        print("\nDemo interrupted by user. Goodbye!")
    except Exception as e:
        print(f"\nDemo failed: {e}")

if __name__ == '__main__':
    main()