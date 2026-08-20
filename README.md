
Hi, how's it going?

## Usage

```
./build.sh [BUILD_TYPE] [run] [clean]
```

### Build Types
- `Debug` (default)
- `Release`


### Examples
```
./build.sh              # Build Debug
./build.sh run          # Build Debug and run
./build.sh Release      # Build Release
./build.sh Release run  # Build Release and run
./build.sh clean        # Clean and build Debug
```

## Makefile Alternative

You can also use the provided Makefile:

```
make build
make run          # Builds and runs
make clean
make debug
make release
make debug-run
make release-run
```

## Dependencies

### Build Tools
- CMake (required)

### Vulkan
Install Vulkan dependencies for your distribution:

#### Ubuntu/Debian
```
sudo apt-get install vulkan-tools libvulkan-dev vulkan-validationlayers
```

#### Fedora
```
sudo dnf install vulkan-tools vulkan-devel
```

#### Arch
```
sudo pacman -S vulkan-icd-loader vulkan-headers vulkan-validation-layers
```

### SDL3
Install SDL3 development libraries:

#### Ubuntu/Debian
```
sudo apt-get install libsdl3-dev
```

#### Fedora
```
sudo dnf install SDL3-devel
```

#### Arch
```
sudo pacman -S sdl3
```

### SDL_image (TTF and Image support)
Install SDL_image development libraries:

#### Ubuntu/Debian
```
sudo apt-get install libsdl3-image-dev
```

#### Fedora
```
sudo dnf install SDL3_image-devel
```

#### Arch
```
sudo pacman -S sdl3_image
```

### SDL_ttf (Font support)
Install SDL_ttf development libraries:

#### Ubuntu/Debian
```
sudo apt-get install libsdl3-ttf-dev
```

#### Fedora
```
sudo dnf install SDL3_ttf-devel
```

#### Arch
```
sudo pacman -S sdl3_ttf
```

## Full Installation (All Dependencies)

### Ubuntu/Debian
```
sudo apt-get install cmake vulkan-tools libvulkan-dev vulkan-validationlayers libsdl3-dev libsdl3-image-dev libsdl3-ttf-dev
```

### Fedora
```
sudo dnf install cmake vulkan-tools vulkan-devel SDL3-devel SDL3_image-devel SDL3_ttf-devel
```

### Arch
```
sudo pacman -S cmake vulkan-icd-loader vulkan-headers vulkan-validation-layers sdl3 sdl3_image sdl3_ttf
```

## Building

1. Ensure all dependencies are installed
2. Run the build script or use the Makefile
