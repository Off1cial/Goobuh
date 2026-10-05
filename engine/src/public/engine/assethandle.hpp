#pragma once

#include <cstdint>

// Lightweight, trivially copyable handle to an asset owned by CAssetManager.
// Lives in its own header so interfaces (e.g. IRenderer) can take it by value
// without including assetmanager.hpp (which would create an include cycle).
struct AssetHandle
{
    static constexpr uint32_t INVALID_INDEX = UINT32_MAX;
    uint32_t index = INVALID_INDEX;

    bool IsValid() const { return index != INVALID_INDEX; }
    explicit operator bool() const { return IsValid(); }
    friend bool operator==( AssetHandle a, AssetHandle b ) { return a.index == b.index; }
    friend bool operator!=( AssetHandle a, AssetHandle b ) { return a.index != b.index; }
};