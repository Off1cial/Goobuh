#pragma once#

#include <filesystem>
#include "common/common.h"
#include "common/hash.h"


#include "public/engine/iassetmanager.hpp"

#define ASSETS_MAX_STR 512
#define ASSETS_MAX_PATHS 5


enum 
{
    ASSET_TYPE_MODEL,
    ASSET_TYPE_SOUND,
    ASSET_TYPE_IMAGE,
};

enum 
{
    ASSET_STATE_UNLOADED,
    ASSET_STATE_LOADING,
    ASSET_STATE_LOADED,
};

struct assethandle_t
{
    int state;
    uint32_t hash;
    union {
        struct {
            u32 hash_bucket;
            int index; // index in bucket
        }  loaded;
        struct {
            std::filesystem::path path; // O_O
        } unloaded; // and loading
    };
    struct assethandle_t* next; // for directory list
    assethandle_t( std::string name ){ hash = Hash_String(name.data()); state = ASSET_STATE_UNLOADED; }
};

struct assetdir_t
{
    std::filesystem::path syspath;
    assethandle_t* handle;
};

typedef struct asset_s
{
    std::string name;
    assethandle_t handle; 
    uint32_t hash;
    int type;
    struct asset_s* next;
} asset_t;


class CAssetManager : public IAssetManager
{
public:
    CAssetManager( void ) = default;
    bool Init(const char* first_path ... ); // Initialise with search paths
    bool AddSearchPath( std::string relative_dir ) override;
    void RemoveSearchPath( std::string relative_dir ) override;

    assethandle_t GetHandle( std::string name );
    assethandle_t FindAsset( std::string name );
private:

    static constexpr int ASSETS_MAX_BUCKETS = 40;

    std::string m_paths[ASSETS_MAX_PATHS];
    int m_numpaths = 0;
    
    asset_t* m_htable[ASSETS_MAX_BUCKETS];

    asset_t* find_loaded_asset( const char* name );
    std::filesystem::path find_unloaded_asset( const char* name );


    asset_t* LoadAsset( assethandle_t handle );
};
