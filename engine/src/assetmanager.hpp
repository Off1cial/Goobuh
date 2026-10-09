#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>
#include <SDL3/SDL_audio.h>


#include "public/engine/iassetmanager.hpp"
#include "public/engine/assethandle.hpp"
#include "renderer/vulkan/vk_mesh_loader.h" // VKMesh must be a complete type (held by value)

class IRenderer; // forward declared; irenderer.hpp is only needed in the .cpp

constexpr size_t ASSETS_MAX_PATHS = 5;

enum class AssetType : uint8_t
{
    None,   // Unknown extension
    Model,
    Sound,
    Image,
};

enum class AssetState : uint8_t
{
    Unloaded,
    Loading,
    Loaded,
    Failed,
};

struct ModelData { VKMesh vk_mesh{}; vec3_t halfs; };
struct SoundData { SDL_AudioSpec spec{}; std::vector<uint8_t> data; };
struct TextureData { int w = 0, h = 0; std::vector<uint8_t> data; int vk_index; };

// What the manager owns for each asset. Payload is a variant instead of a raw union,
// so non-trivial members are constructed/destroyed correctly.
struct Asset
{
    std::string           name;   // normalized lookup key, e.g. "models/cone.glb"
    std::filesystem::path path;   // resolved on-disk path
    AssetType             type = AssetType::None;
    AssetState            state = AssetState::Unloaded;

    std::variant<std::monostate, ModelData, SoundData, TextureData> payload;

    ModelData ModelByValue() { return std::get<ModelData>( payload ); }
    TextureData TextureByValue() { return std::get<TextureData>( payload ); }
    SoundData SoundByValue() { return std::get<SoundData>( payload ); }

    ModelData* Model() { return std::get_if<ModelData>( &payload ); }
    const ModelData* Model()   const { return std::get_if<ModelData>( &payload ); }
    const SoundData* Sound()   const { return std::get_if<SoundData>( &payload ); }
    const TextureData* Texture() const { return std::get_if<TextureData>( &payload ); }
};

class CAssetManager : public IAssetManager
{
public:
    CAssetManager() = default;
    CAssetManager( const CAssetManager& ) = delete;
    CAssetManager& operator=( const CAssetManager& ) = delete;

    ~CAssetManager() { Shutdown(); }
    // e.g. Init({ "assets", "mods/foo/assets" });
    bool Init( std::initializer_list<const char*> search_paths );

    bool AddSearchPath( std::string relative_dir ) override;
    void RemoveSearchPath( std::string relative_dir ) override;

    // Finds (or registers) an asset by name. Does not load it.
    // Returns an invalid handle if the file can't be found in any search path.
    AssetHandle GetHandle( std::string_view name ) override;

    // Loads the asset's data. Returns true if it is loaded on return.
    bool Load( IRenderer* renderer, AssetHandle handle ) override;
    void Unload( AssetHandle handle ) override;

    // nullptr if the handle is invalid.
    Asset* Get( AssetHandle handle );
    const Asset* Get( AssetHandle handle ) const;

    // Specific asset getters
    bool GetModelData(  AssetHandle h, ModelData& data_out );
    bool GetTextureData( AssetHandle h, TextureData& data_out );

private:

    void Shutdown( void );
    void UnloadAsset( Asset* asset );

    static AssetType TypeFromExtension( const std::filesystem::path& p );
    static bool      IsSafeRelativeName( const std::filesystem::path& p );

    std::filesystem::path FindOnDisk( const std::filesystem::path& name ) const;

    std::vector<std::filesystem::path> m_paths;

    // deque: growing never invalidates references to existing Assets.
    std::deque<Asset>                          m_assets;
    std::unordered_map<std::string, uint32_t>  m_lookup; // name -> index into m_assets
};

extern CAssetManager* g_AssetManager;