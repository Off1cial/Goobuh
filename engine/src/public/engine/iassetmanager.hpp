#pragma once

#include <string>

// Examples
//
// relative_dir:  "models/"
// relative_path: "models/cone.glb"
// name:          "cone.glb"

class IAssetManager
{
public:
    virtual bool AddSearchPath( std::string relative_dir ) = 0;
    virtual void RemoveSearchPath( std::string relative_dir ) = 0;

    
};
