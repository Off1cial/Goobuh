#!/bin/bash
# build.sh

# Default values
BUILD_TYPE="Debug"
RUN_ENGINE=false
CLEAN=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        run)
            RUN_ENGINE=true
            shift
            ;;
        clean)
            CLEAN=true
            shift
            ;;
        Debug|Release|RelWithDebInfo|MinSizeRel)
            BUILD_TYPE="$1"
            shift
            ;;
        -h|--help)
            echo "Usage: ./build.sh [BUILD_TYPE] [run] [clean]"
            echo ""
            echo "BUILD_TYPE: Debug (default), Release, RelWithDebInfo, MinSizeRel"
            echo "run:        Run the engine after building"
            echo "clean:      Clean build directory before building"
            echo ""
            echo "Examples:"
            echo "  ./build.sh              # Build Debug"
            echo "  ./build.sh run          # Build Debug and run"
            echo "  ./build.sh Release      # Build Release"
            echo "  ./build.sh Release run  # Build Release and run"
            echo "  ./build.sh clean        # Clean and build Debug"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use ./build.sh --help for usage"
            exit 1
            ;;
    esac
done

echo "Building with type: $BUILD_TYPE"

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo "Cleaning build directory..."
    rm -rf build
fi

# Configure and build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=$BUILD_TYPE
ninja -C build

# Check if build succeeded
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

# Run the engine if requested
if [ "$RUN_ENGINE" = true ]; then
    echo ""
    echo "========================================"
    echo "Running engine..."
    echo "========================================"
    echo ""
    ./build/engine
fi