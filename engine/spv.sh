#!/bin/bash

# spv.sh - Compile GLSL shader using glslc
# Usage: ./spv.sh <shader_path> [output_path]

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_usage() {
  echo "Usage: $0 <shader_path> [output_path]"
  echo ""
  echo "Arguments:"
  echo "  shader_path    Path to the GLSL shader file"
  echo "  output_path    Optional output path for the SPIR-V binary"
  echo "                 Defaults to input filename with .spv extension"
  echo ""
  echo "Examples:"
  echo "  $0 shader.vert"
  echo "  $0 shader.frag compiled/shader.frag.spv"
  echo "  $0 --help"
}

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
    *.glsl) echo "unknown" ;;
    *) echo "unknown" ;;
  esac
}

if [[ "$1" == "--help" || "$1" == "-h" ]]; then
  print_usage
  exit 0
fi

if [ $# -lt 1 ]; then
  echo -e "${RED}Error: Shader path is required${NC}"
  print_usage
  exit 1
fi

SHADER_PATH="$1"

if [ ! -f "$SHADER_PATH" ]; then
  echo -e "${RED}Error: Shader file not found: $SHADER_PATH${NC}"
  exit 1
fi

if ! command -v glslc &> /dev/null; then
  echo -e "${RED}Error: glslc not found${NC}"
  exit 1
fi

if [ $# -ge 2 ]; then
  OUTPUT_PATH="$2"
else
  OUTPUT_PATH="${SHADER_PATH%.*}.spv"
fi

OUTPUT_DIR="$(dirname "$OUTPUT_PATH")"

if [ "$OUTPUT_DIR" != "." ] && [ ! -d "$OUTPUT_DIR" ]; then
  mkdir -p "$OUTPUT_DIR"
  echo -e "${YELLOW}Created directory: $OUTPUT_DIR${NC}"
fi

SHADER_STAGE=$(detect_shader_stage "$SHADER_PATH")

echo -e "${GREEN}Compiling shader:${NC} $SHADER_PATH"
echo -e "${GREEN}Stage:${NC} $SHADER_STAGE"
echo -e "${GREEN}Output:${NC} $OUTPUT_PATH"

if glslc \
  --target-env=vulkan1.3 \
  "$SHADER_PATH" \
  -o "$OUTPUT_PATH"
then
  if [ -f "$OUTPUT_PATH" ]; then
    FILE_SIZE=$(du -h "$OUTPUT_PATH" | cut -f1)

    echo -e "${GREEN}Compilation successful${NC}"
    echo -e "${GREEN}Output size:${NC} $FILE_SIZE"

    if command -v spirv-dis &> /dev/null; then
      echo -e "${YELLOW}View SPIR-V:${NC} spirv-dis \"$OUTPUT_PATH\""
    fi
  else
    echo -e "${RED}Compilation failed: output file was not created${NC}"
    exit 1
  fi
else
  echo -e "${RED}Compilation failed${NC}"
  exit 1
fi
