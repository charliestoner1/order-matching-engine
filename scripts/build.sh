#!/bin/bash

# Build script for Order Matching Engine

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[BUILD]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Check if cmake is installed
if ! command -v cmake &> /dev/null; then
    print_error "CMake is not installed. Please install CMake first."
    exit 1
fi

# Parse command line arguments
BUILD_TYPE="Release"
CLEAN_BUILD=0
RUN_BENCHMARK=0

while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=1
            shift
            ;;
        -b|--benchmark)
            RUN_BENCHMARK=1
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  -d, --debug      Build in debug mode"
            echo "  -c, --clean      Clean build (remove build directory)"
            echo "  -b, --benchmark  Run benchmarks after building"
            echo "  -h, --help       Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Create build directory name based on build type
BUILD_DIR="cmake-build-$(echo ${BUILD_TYPE} | tr '[:upper:]' '[:lower:]')"

# Clean build if requested
if [ $CLEAN_BUILD -eq 1 ]; then
    print_status "Cleaning build directory: $BUILD_DIR"
    rm -rf $BUILD_DIR
fi

# Create build directory
print_status "Creating build directory: $BUILD_DIR"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure with CMake
print_status "Configuring with CMake (${BUILD_TYPE} mode)..."
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
if [ $? -ne 0 ]; then
    print_error "CMake configuration failed"
    exit 1
fi

# Determine number of CPU cores for parallel build
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    CORES=$(nproc)
elif [[ "$OSTYPE" == "darwin"* ]]; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=1
fi

# Build the project
print_status "Building with $CORES cores..."
make -j$CORES
if [ $? -ne 0 ]; then
    print_error "Build failed"
    exit 1
fi

print_status "Build completed successfully!"

# Run benchmark if requested
if [ $RUN_BENCHMARK -eq 1 ]; then
    print_status "Running benchmarks..."
    ./benchmark
fi

# Print build artifacts location
echo ""
print_status "Build artifacts location: $(pwd)"
print_status "Executables:"
echo "  - Main program: ./ordermatching"
echo "  - Benchmark:    ./benchmark"