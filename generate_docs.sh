#!/bin/bash

# Generate documentation using Doxygen
# This script generates HTML documentation from C++ source code comments

echo "🔧 Generating C++ Documentation with Doxygen..."

# Check if Doxygen is installed
if ! command -v doxygen &> /dev/null; then
    echo "❌ Doxygen is not installed!"
    echo "📦 Install it using:"
    echo "   macOS: brew install doxygen"
    echo "   Ubuntu: sudo apt-get install doxygen"
    echo "   Windows: Download from https://www.doxygen.nl/download.html"
    exit 1
fi

# Check if Doxyfile exists
if [ ! -f "Doxyfile" ]; then
    echo "❌ Doxyfile not found!"
    echo "Make sure you're running this script from the project root directory."
    exit 1
fi

# Create docs directory if it doesn't exist
mkdir -p docs

# Generate documentation
echo "📚 Running Doxygen..."
doxygen Doxyfile

# Check if generation was successful
if [ $? -eq 0 ]; then
    echo "✅ Documentation generated successfully!"
    echo "📖 Open docs/html/index.html in your browser to view the documentation"
    echo ""
    echo "🌐 To open automatically:"
    echo "   macOS: open docs/html/index.html"
    echo "   Linux: xdg-open docs/html/index.html"
    echo "   Windows: start docs/html/index.html"
    echo ""
    echo "📁 Documentation files are in: docs/html/"
else
    echo "❌ Documentation generation failed!"
    echo "Check the output above for errors."
    exit 1
fi
