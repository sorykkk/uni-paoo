#!/bin/bash

# Build the project if needed
if [ ! -f "build/bin/doc_analytics_demo" ] || [ ! -f "build/bin/unsafe_doc_analytics_demo" ]; then
    echo "Building the project..."
    mkdir -p build
    cd build
    cmake ..
    make
    cd ..
fi

# Run the safe demo
echo "========================================="
echo "Running doc_analytics_demo (Safe Version)"
echo "========================================="
./build/bin/doc_analytics_demo

echo ""
echo ""

# Run the unsafe demo
echo "=================================================="
echo "Running unsafe_doc_analytics_demo (Unsafe Version)"
echo "=================================================="
./build/bin/unsafe_doc_analytics_demo

