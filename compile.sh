#!/bin/bash

# EchoHub Project - Compile Script for Linux
# This script compiles the EchoHub project using CMake

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if we're in the project root directory
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: Not in project root directory. Please run from EchoHubProject/ directory.${NC}"
    exit 1
fi

# Create build directory if it doesn't exist
BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${BLUE}Creating build directory...${NC}"
    mkdir -p "$BUILD_DIR"
fi

# Change to build directory
cd "$BUILD_DIR"

echo -e "${BLUE}Configuring project with CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Debug

echo -e "${BLUE}Building project...${NC}"
echo -e "${YELLOW}This may take a few minutes...${NC}"

# Build with all CPU cores
cmake --build . -j$(nproc)

echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${BLUE}Executable: ./echo_hub${NC}"
echo -e "${BLUE}To run: ./echo_hub${NC}"

# Go back to project root
cd ..
