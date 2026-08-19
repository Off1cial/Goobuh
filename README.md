hi hows it going

Usage: ./build.sh [BUILD_TYPE] [run] [clean]

BUILD_TYPE: Debug (default), Release, RelWithDebInfo, MinSizeRel
run:        Run the engine after building
clean:      Clean build directory before building

Examples:
  ./build.sh              # Build Debug
  ./build.sh run          # Build Debug and run
  ./build.sh Release      # Build Release
  ./build.sh Release run  # Build Release and run
  ./build.sh clean        # Clean and build Debug

Or use the Makefile

make build
make run          # Builds and runs
make clean
make debug
make release
make debug-run
make release-run

Vulkan:


# Ubuntu/Debian
sudo apt-get install vulkan-tools libvulkan-dev vulkan-validationlayers

# Fedora
sudo dnf install vulkan-tools vulkan-devel

# Arch
sudo pacman -S vulkan-icd-loader vulkan-headers vulkan-validation-layers