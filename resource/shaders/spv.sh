#!/bin/bash

# spv.sh - Compile GLSL shader using glslc
# Usage: ./spv.sh <shader_path> [output_path]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print usage
print_usage() {
    echo "Usage: $0 <shader_path> [output_path]"
    echo ""
    echo "Arguments:"
    echo "  shader_path    Path to the GLSL shader file"
    echo "  output_path    (Optional) Output path for the compiled SPIR-V binary"
    echo "                 If not provided, will use input filename with .spv extension"
    echo ""
    echo "Examples:"
    echo "  $0 shader.vert"
    echo "  $0 shader.frag compiled/spirv/shader.frag.spv"
    echo "  $0 --help     Show this help message"
}

# Function to detect shader stage from file extension
detect_shader_stage() {
    local file="$1"
    case "$file" in
        *.vert) echo "vertex" ;;
        *.frag) echo "fragment" ;;
        *.comp) echo "compute" ;;
        *.geom) echo "geometry" ;;
        *.tesc) echo "tessellation control" ;;
        *.tese) echo "tessellation evaluation" ;;
        *.mesh) echo "mesh" ;;
        *.task) echo "task" ;;
        *.rgen) echo "ray generation" ;;
        *.rint) echo "ray intersection" ;;
        *.rahit) echo "ray any-hit" ;;
        *.rchit) echo "ray closest-hit" ;;
        *.rmiss) echo "ray miss" ;;
        *.rcall) echo "ray callable" ;;
        *.glsl) echo "unknown (will try auto-detect)" ;;
        *) echo "unknown" ;;
    esac
}

# Check for help flag
if [[ "$1" == "--help" ]] || [[ "$1" == "-h" ]]; then
    print_usage
    exit 0
fi

# Check if shader path is provided
if [ $# -lt 1 ]; then
    echo -e "${RED}Error: Shader path is required${NC}"
    print_usage
    exit 1
fi

# Get shader path
SHADER_PATH="$1"

# Check if shader file exists
if [ ! -f "$SHADER_PATH" ]; then
    echo -e "${RED}Error: Shader file not found: $SHADER_PATH${NC}"
    exit 1
fi

# Check if glslc is installed
if ! command -v glslc &> /dev/null; then
    echo -e "${RED}Error: glslc not found. Please install Vulkan SDK.${NC}"
    echo "  Ubuntu/Debian: sudo apt install vulkan-sdk"
    echo "  macOS: brew install vulkan-sdk"
    echo "  Or download from: https://vulkan.lunarg.com/"
    exit 1
fi

# Determine output path
if [ $# -ge 2 ]; then
    OUTPUT_PATH="$2"
else
    # Remove extension and add .spv
    BASENAME="${SHADER_PATH%.*}"
    OUTPUT_PATH="${BASENAME}.spv"
fi

# Create output directory if it doesn't exist
OUTPUT_DIR="$(dirname "$OUTPUT_PATH")"
if [ ! -d "$OUTPUT_DIR" ] && [ "$OUTPUT_DIR" != "." ]; then
    mkdir -p "$OUTPUT_DIR"
    echo -e "${YELLOW}Created directory: $OUTPUT_DIR${NC}"
fi

# Detect shader stage
SHADER_STAGE=$(detect_shader_stage "$SHADER_PATH")

# Print compilation info
echo -e "${GREEN}Compiling shader:${NC} $SHADER_PATH"
echo -e "${GREEN}Stage:${NC} $SHADER_STAGE"
echo -e "${GREEN}Output:${NC} $OUTPUT_PATH"

# Compile the shader
# Add -mfmt=bin for binary SPIR-V output (default)
# Add -g for debug information (optional)
if glslc "$SHADER_PATH" -o "$OUTPUT_PATH" 2>/dev/null; then
    # Check if output file was created
    if [ -f "$OUTPUT_PATH" ]; then
        FILE_SIZE=$(du -h "$OUTPUT_PATH" | cut -f1)
        echo -e "${GREEN}✓ Compilation successful!${NC}"
        echo -e "${GREEN}Output size:${NC} $FILE_SIZE"
        
        # Optionally display SPIR-V info using spirv-dis if available
        if command -v spirv-dis &> /dev/null; then
            echo -e "${YELLOW}To view SPIR-V assembly: spirv-dis $OUTPUT_PATH${NC}"
        fi
    else
        echo -e "${RED}✗ Compilation failed${NC}"
        exit 1
    fi
else
    echo -e "${RED}✗ Compilation failed${NC}"
    echo -e "${YELLOW}Try running glslc directly to see full error:${NC}"
    echo "  glslc \"$SHADER_PATH\" -o \"$OUTPUT_PATH\""
    exit 1
fi

# Show additional usage info
echo ""
echo -e "${YELLOW}Extra options you can add to glslc:${NC}"
echo "  --target-env=vulkan1.2  Set Vulkan environment"
echo "  -O                      Optimize (0=disable, 1=size, 2=performance)"
echo "  -g                      Include debug information"
echo "  -std=450                Set GLSL standard (100, 310, 320, 330, 400, 410, 420, 430, 440, 450)"
echo ""
echo -e "${GREEN}Example with options:${NC}"
echo "  glslc \"$SHADER_PATH\" -o \"$OUTPUT_PATH\" 