#include "assetmanager.hpp"

#include <stdarg.h>
#include <filesystem>

#include "common/common.h"
#include "common/hash.h"

bool CAssetManager::Init(const char* first_path, ... )
{
    va_list args;

    va_start(args, first_path);


    const char* path = first_path;
    while (path != nullptr)
    {
        printf("%s\n", path);
        AddSearchPath(path);
        path = va_arg( args, const char* );
    }

    // Add each arg as a search path
    va_end( args );
    for (int i = 0; i < ASSETS_MAX_PATHS; i++){
        printf("Path: %s\n", m_paths[i].data());
    }

    printf("Cone: %s\n", find_unloaded_asset("cone.glb").string().data());

    return true;
} 

bool CAssetManager::AddSearchPath( std::string relative_path ){
   if (m_numpaths >= ASSETS_MAX_PATHS) return false; 
   relative_path.push_back('/');
   m_paths[m_numpaths].clear();
   m_paths[m_numpaths++] = relative_path;
   return true;
}


asset_t* CAssetManager::LoadAsset( assethandle_t handle ){
    if (handle.state == ASSET_STATE_LOADED){
        return NULL;
    }
    
    

    return NULL;
}

void CAssetManager::RemoveSearchPath( std::string relative_path ){
    return;
}

asset_t* CAssetManager::find_loaded_asset( const char* name )
{
    uint32_t hash = Hash_String(name);
    asset_t* current = nullptr;
    uint32_t bucket = hash % ASSETS_MAX_BUCKETS;
    for (current = m_htable[bucket]; current; current = current->next){
        if (current->hash == hash){
            return current;
        }
#ifdef ASSETS_SAFE_CHECK
        if (current->name == name){
            return current;
        }
#endif
    }
    return nullptr;
}

std::filesystem::path CAssetManager::find_unloaded_asset( const char* name )
{
    for ( std::string& path : m_paths){
        std::filesystem::path dir = path.append(name);

        if (std::filesystem::exists(dir) && std::filesystem::is_regular_file(dir)){
            return dir;
        }
    }
    return {};
}
