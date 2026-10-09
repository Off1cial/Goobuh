#pragma once

#include <string>
#include <string_view>
#include "public/engine/assethandle.hpp"

// Examples
//
// relative_dir:  "models/"
// relative_path: "models/cone.glb"
// name:          "cone.glb"


// Forward Declaration
class IRenderer;

class IAssetManager
{
public:
    virtual ~IAssetManager() = default;
    virtual bool AddSearchPath( std::string relative_dir ) = 0;
    virtual void RemoveSearchPath( std::string relative_dir ) = 0;

    virtual AssetHandle GetHandle( std::string_view name ) = 0;
    virtual bool Load( IRenderer* renderer, AssetHandle handle ) = 0;
    virtual void Unload( AssetHandle handle ) = 0;
};
