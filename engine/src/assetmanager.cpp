#include "assetmanager.hpp"

#include "renderer/irenderer.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>

namespace fs = std::filesystem;

CAssetManager* g_AssetManager = new CAssetManager;

// ---------------------------------------------------------------------------
// Search paths
// ---------------------------------------------------------------------------

bool CAssetManager::Init( std::initializer_list<const char*> search_paths )
{
    for (const char* p : search_paths)
    {
        if (p && !AddSearchPath( p ))
            fprintf( stderr, "AssetManager: could not add search path '%s'\n", p );

    }
    return !m_paths.empty();
}

bool CAssetManager::AddSearchPath( std::string relative_dir )
{
    if (m_paths.size() >= ASSETS_MAX_PATHS)
        return false;

    fs::path p = fs::path( relative_dir ).lexically_normal();

    // No duplicates
    if (std::find( m_paths.begin(), m_paths.end(), p ) != m_paths.end())
        return true;

    m_paths.push_back( std::move( p ) );
    return true;
}

void CAssetManager::RemoveSearchPath( std::string relative_dir )
{
    const fs::path p = fs::path( relative_dir ).lexically_normal();
    m_paths.erase( std::remove( m_paths.begin(), m_paths.end(), p ), m_paths.end() );
}

// ---------------------------------------------------------------------------
// Lookup
// ---------------------------------------------------------------------------

AssetHandle CAssetManager::GetHandle( std::string_view name )
{
    const fs::path rel = fs::path( name ).lexically_normal();
    if (!IsSafeRelativeName( rel ))
        return {};

    // Always forward slashes so "a\\b.glb" and "a/b.glb" share one entry.
    const std::string key = rel.generic_string();

    if (auto it = m_lookup.find( key ); it != m_lookup.end())
        return { it->second };

    fs::path found = FindOnDisk( rel );
    if (found.empty())
        return {};

    Asset asset;
    asset.name = key;
    asset.path = std::move( found );
    asset.type = TypeFromExtension( asset.path );

    const uint32_t index = static_cast<uint32_t>(m_assets.size());
    m_assets.push_back( std::move( asset ) );
    m_lookup.emplace( key, index );
    return { index };
}

Asset* CAssetManager::Get( AssetHandle h )
{
    return (h.IsValid() && h.index < m_assets.size()) ? &m_assets[h.index] : nullptr;
}

const Asset* CAssetManager::Get( AssetHandle h ) const
{
    return (h.IsValid() && h.index < m_assets.size()) ? &m_assets[h.index] : nullptr;
}

// ---------------------------------------------------------------------------
// Loading
// ---------------------------------------------------------------------------

bool CAssetManager::Load( IRenderer* renderer, AssetHandle handle )
{
    Asset* asset = Get( handle );
    if (!asset)
        return false;

    if (asset->state == AssetState::Loaded)
        return true;
    if (asset->state == AssetState::Loading || asset->state == AssetState::Failed)
        return false;

    asset->state = AssetState::Loading;

    switch (asset->type)
    {
        case AssetType::Model:
            {
                if (!renderer)
                    break;

                ModelData mdl;
                mdl.vk_mesh = VKMesh_load_gltf( renderer->vk_renderer, asset->path.string().c_str() );
                asset->payload = std::move( mdl );
                asset->state = AssetState::Loaded;
                return true;
            }
        case AssetType::Sound:
        case AssetType::Image:
            fprintf( stderr, "AssetManager: loading '%s' not implemented yet\n", asset->name.c_str() );
            break;
        case AssetType::None:
            fprintf( stderr, "AssetManager: unknown asset type for '%s'\n", asset->name.c_str() );
            break;
    }

    asset->state = AssetState::Failed;
    return false;
}

void CAssetManager::Unload( AssetHandle handle )
{
    Asset* asset = Get( handle );
    if (!asset || asset->state != AssetState::Loaded)
        return;

    // TODO: destroy GPU resources first (e.g. VKMesh_destroy) before dropping the payload.
    asset->payload = std::monostate{};
    asset->state = AssetState::Unloaded;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

fs::path CAssetManager::FindOnDisk( const fs::path& name ) const
{
    std::error_code ec;
    for (const fs::path& dir : m_paths)
    {
        fs::path file = dir / name;
        if (fs::is_regular_file( file, ec ))
            return file;
    }
    return {};
}

AssetType CAssetManager::TypeFromExtension( const fs::path& p )
{
    std::string ext = p.extension().string();
    std::transform( ext.begin(), ext.end(), ext.begin(),
                    []( unsigned char c ) { return static_cast<char>(std::tolower( c )); } );

    if (ext == ".glb" || ext == ".gltf")                          return AssetType::Model;
    if (ext == ".wav" || ext == ".ogg" || ext == ".mp3")          return AssetType::Sound;
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
         ext == ".tga" || ext == ".bmp" || ext == ".ktx" ||
         ext == ".ktx2")                                           return AssetType::Image;
    return AssetType::None;
}

// Rejects empty, absolute, and ".." names so lookups can't escape the search paths.
bool CAssetManager::IsSafeRelativeName( const fs::path& p )
{
    if (p.empty() || p.is_absolute() || p.has_root_name() || p.has_root_directory())
        return false;
    for (const fs::path& part : p)
        if (part == "..")
            return false;
    return true;
}