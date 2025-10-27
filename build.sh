#!/bin/bash

# AI Synthesizer Build Script
# This script provides an easy way to build the project on different platforms

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to detect platform
detect_platform() {
    if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        echo "windows"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    else
        echo "unknown"
    fi
}

# Function to check dependencies
check_dependencies() {
    local platform=$1
    
    print_status "Checking dependencies for $platform..."
    
    # Check CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake not found. Please install CMake 3.20 or higher."
        exit 1
    fi
    
    local cmake_version=$(cmake --version | head -n1 | cut -d' ' -f3)
    print_status "Found CMake version: $cmake_version"
    
    # Check compiler
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        print_error "No C++ compiler found. Please install GCC or Clang."
        exit 1
    fi
    
    # Check yaml-cpp
    if [[ "$platform" == "windows" ]]; then
        if ! pacman -Q mingw-w64-i686-yaml-cpp &> /dev/null; then
            print_warning "yaml-cpp not found. Installing..."
            pacman -S mingw-w64-i686-yaml-cpp --noconfirm
        fi
    elif [[ "$platform" == "linux" ]]; then
        if ! pkg-config --exists yaml-cpp; then
            print_error "yaml-cpp not found. Please install libyaml-cpp-dev"
            exit 1
        fi
    elif [[ "$platform" == "macos" ]]; then
        if ! brew list yaml-cpp &> /dev/null; then
            print_warning "yaml-cpp not found. Installing..."
            brew install yaml-cpp
        fi
    fi
    
    print_success "All dependencies found"
}

# Function to build project
build_project() {
    local platform=$1
    local build_type=$2
    local preset=$3
    
    print_status "Building project for $platform ($build_type)..."
    
    # Create build directory
    local build_dir="build-${platform}-${build_type}"
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    # Configure with CMake
    if [[ "$platform" == "windows" ]]; then
        cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE="$build_type"
    else
        cmake .. -DCMAKE_BUILD_TYPE="$build_type"
    fi
    
    # Build
    if [[ "$platform" == "windows" ]]; then
        mingw32-make -j$(nproc)
    else
        make -j$(nproc)
    fi
    
    cd ..
    print_success "Build completed successfully"
}

# Function to run tests
run_tests() {
    local platform=$1
    local build_type=$2
    
    print_status "Running tests..."
    
    local build_dir="build-${platform}-${build_type}"
    cd "$build_dir"
    
    # Run CTest
    ctest --output-on-failure
    
    cd ..
    print_success "All tests passed"
}

# Function to run demo
run_demo() {
    local platform=$1
    local build_type=$2
    
    print_status "Running demo..."
    
    local build_dir="build-${platform}-${build_type}"
    local executable=""
    
    if [[ "$platform" == "windows" ]]; then
        executable="$build_dir/bin/aiaudio_generator.exe"
    else
        executable="$build_dir/bin/aiaudio_generator"
    fi
    
    if [[ -f "$executable" ]]; then
        "$executable" --demo
        print_success "Demo completed successfully"
    else
        print_error "Executable not found: $executable"
        exit 1
    fi
}

# Function to clean build
clean_build() {
    print_status "Cleaning build directories..."
    rm -rf build-*
    print_success "Build directories cleaned"
}

# Function to show help
show_help() {
    echo "AI Synthesizer Build Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -p, --platform PLATFORM Specify platform (windows, linux, macos, auto)"
    echo "  -t, --type TYPE         Build type (Release, Debug) [default: Release]"
    echo "  -c, --clean             Clean build directories"
    echo "  -b, --build             Build the project"
    echo "  -r, --run-tests         Run tests after building"
    echo "  -d, --demo              Run demo after building"
    echo "  --all                   Build, test, and run demo"
    echo ""
    echo "Examples:"
    echo "  $0 --all                    # Auto-detect platform, build, test, and demo"
    echo "  $0 -p windows -t Debug      # Build Windows debug version"
    echo "  $0 -p linux -b -r           # Build and test on Linux"
    echo "  $0 -c                       # Clean build directories"
}

# Main script
main() {
    local platform="auto"
    local build_type="Release"
    local clean=false
    local build=false
    local run_tests_flag=false
    local demo=false
    local all=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -p|--platform)
                platform="$2"
                shift 2
                ;;
            -t|--type)
                build_type="$2"
                shift 2
                ;;
            -c|--clean)
                clean=true
                shift
                ;;
            -b|--build)
                build=true
                shift
                ;;
            -r|--run-tests)
                run_tests_flag=true
                shift
                ;;
            -d|--demo)
                demo=true
                shift
                ;;
            --all)
                all=true
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # Auto-detect platform if not specified
    if [[ "$platform" == "auto" ]]; then
        platform=$(detect_platform)
        print_status "Auto-detected platform: $platform"
    fi
    
    # Validate platform
    if [[ "$platform" != "windows" && "$platform" != "linux" && "$platform" != "macos" ]]; then
        print_error "Unsupported platform: $platform"
        print_error "Supported platforms: windows, linux, macos"
        exit 1
    fi
    
    # Validate build type
    if [[ "$build_type" != "Release" && "$build_type" != "Debug" ]]; then
        print_error "Unsupported build type: $build_type"
        print_error "Supported build types: Release, Debug"
        exit 1
    fi
    
    # Clean if requested
    if [[ "$clean" == true ]]; then
        clean_build
        if [[ "$build" == false && "$run_tests_flag" == false && "$demo" == false && "$all" == false ]]; then
            exit 0
        fi
    fi
    
    # Check dependencies
    check_dependencies "$platform"
    
    # Build if requested
    if [[ "$build" == true || "$all" == true ]]; then
        build_project "$platform" "$build_type"
    fi
    
    # Run tests if requested
    if [[ "$run_tests_flag" == true || "$all" == true ]]; then
        run_tests "$platform" "$build_type"
    fi
    
    # Run demo if requested
    if [[ "$demo" == true || "$all" == true ]]; then
        run_demo "$platform" "$build_type"
    fi
    
    # If no specific action was requested, show help
    if [[ "$clean" == false && "$build" == false && "$run_tests_flag" == false && "$demo" == false && "$all" == false ]]; then
        show_help
    fi
}

# Run main function with all arguments
main "$@"