#!/bin/bash
set -e

echo "=== Building React Web Application ==="

# Check if we're in the right directory
if [ ! -f "web/package.json" ]; then
    echo "Error: web/package.json not found. Please run from the project root."
    exit 1
fi

# Install dependencies if node_modules doesn't exist
if [ ! -d "web/node_modules" ]; then
    echo "Installing web dependencies..."
    cd web
    npm install
    cd ..
fi

# Build the React app
echo "Building React application..."
cd web
npm run build
cd ..

echo "✓ React application built successfully"
echo "Static files are now in the static/ directory"
echo "You can now run the web server with: python3 web_server.py"