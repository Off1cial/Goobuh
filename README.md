hi hows it going

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
ninja -C build
( I use ninja)
Vulkan:


# Ubuntu/Debian
sudo apt-get install vulkan-tools libvulkan-dev vulkan-validationlayers

# Fedora
sudo dnf install vulkan-tools vulkan-devel

# Arch
sudo pacman -S vulkan-icd-loader vulkan-headers vulkan-validation-layers