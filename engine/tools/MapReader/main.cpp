#include "map_parser.h"

#include <cstdint>
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;

static constexpr double MAP_SCALE = 1.0 / 32.0;  // map units -> metres

// Not rendered (but still exported for collision)
static bool skip_texture( const std::string& t )
{
    return t == "clip" || t == "trigger" || t == "skip";
}

// Map is Z-up, glTF is Y-up: (x, y, z) -> (x, z, -y). A pure rotation, so winding is preserved.
static Vec3 to_gl( Vec3 v ) { return { v.x, v.z, -v.y }; }

// ---------- texture sizes ----------
static bool read_size( const fs::path& p, double& w, double& h )
{
    std::ifstream f( p, std::ios::binary );
    if (!f) return false;
    unsigned char b[28] = {};
    f.read( (char*)b, 28 );
    if (f.gcount() < 28) return false;

    auto be = [&]( int o )
        {
            return ((uint32_t)b[o] << 24) | ((uint32_t)b[o + 1] << 16) | ((uint32_t)b[o + 2] << 8) | (uint32_t)b[o + 3];
        };
    auto le = [&]( int o )
        {
            return ((uint32_t)b[o + 3] << 24) | ((uint32_t)b[o + 2] << 16) | ((uint32_t)b[o + 1] << 8) | (uint32_t)b[o];
        };

    if (b[0] == 0x89 && b[1] == 'P' && b[2] == 'N' && b[3] == 'G') { w = be( 16 ); h = be( 20 ); return true; }
    if (b[0] == 0xAB && b[1] == 'K' && b[2] == 'T' && b[3] == 'X') { w = le( 20 ); h = le( 24 ); return true; }
    return false;
}

class TexSizes
{
public:
    explicit TexSizes( fs::path d ) : dir( std::move( d ) ) {}

    std::pair<double, double> operator()( const std::string& name )
    {
        auto it = cache.find( name );
        if (it != cache.end()) return it->second;

        std::pair<double, double> sz{ 64.0, 64.0 };
        bool found = false;
        for (const char* ext : { ".png", ".ktx2" })
            if (read_size( dir / (name + ext), sz.first, sz.second )) { found = true; break; }

        if (!found)
            std::fprintf( stderr, "warning: no texture file for '%s', assuming 64x64\n", name.c_str() );
        return cache[name] = sz;
    }

private:
    fs::path dir;
    std::map<std::string, std::pair<double, double>> cache;
};

// ---------- helpers ----------
static std::string json_escape( const std::string& s )
{
    std::string o;
    for (unsigned char c : s)
    {
        if (c == '"' || c == '\\') { o += '\\'; o += (char)c; }
        else if (c < 0x20) { char buf[8]; std::snprintf( buf, sizeof buf, "\\u%04x", c ); o += buf; }
        else o += (char)c;
    }
    return o;
}

static std::string join( const std::vector<std::string>& v )
{
    std::string o;
    for (size_t i = 0; i < v.size(); ++i) { if (i) o += ','; o += v[i]; }
    return o;
}

// ---------- geometry batches ----------
struct Batch
{
    std::vector<float> pos, nrm, uv;
    std::vector<uint32_t> idx;
};

static void add_polygon( Batch& b, const Polygon& p )
{
    uint32_t base = (uint32_t)(b.pos.size() / 3);
    Vec3 n = to_gl( p.normal );
    for (const Vertex& v : p.verts)
    {
        Vec3 g = to_gl( v.pos );
        b.pos.push_back( (float)(g.x * MAP_SCALE) );
        b.pos.push_back( (float)(g.y * MAP_SCALE) );
        b.pos.push_back( (float)(g.z * MAP_SCALE) );
        b.nrm.push_back( (float)n.x );
        b.nrm.push_back( (float)n.y );
        b.nrm.push_back( (float)n.z );
        b.uv.push_back( (float)v.u );
        b.uv.push_back( (float)v.v );
    }
    for (uint32_t i = 1; i + 1 < p.verts.size(); ++i)
    {
        b.idx.push_back( base );
        b.idx.push_back( base + i );
        b.idx.push_back( base + i + 1 );
    }
}

// ---------- glb builder ----------
class Glb
{
public:
    // One node (with one mesh) per call; one primitive per texture batch.
    void add_node( const std::string& name, const std::map<std::string, Batch>& batches )
    {
        std::vector<std::string> prims;
        for (const auto& [tex, b] : batches)
            prims.push_back( primitive( tex, b ) );

        std::string n = json_escape( name );
        meshes.push_back( "{\"name\":\"" + n + "\",\"primitives\":[" + join( prims ) + "]}" );
        nodes.push_back( "{\"name\":\"" + n + "\",\"mesh\":" + std::to_string( meshes.size() - 1 ) + "}" );
    }

    bool empty() const { return nodes.empty(); }

    bool write( const fs::path& path )
    {
        std::string scene_nodes;
        for (size_t i = 0; i < nodes.size(); ++i) { if (i) scene_nodes += ','; scene_nodes += std::to_string( i ); }

        std::string json =
            "{\"asset\":{\"version\":\"2.0\",\"generator\":\"map_reader\"},"
            "\"scene\":0,\"scenes\":[{\"nodes\":[" + scene_nodes + "]}],"
            "\"nodes\":[" + join( nodes ) + "],"
            "\"meshes\":[" + join( meshes ) + "],"
            "\"materials\":[" + join( mats ) + "],"
            "\"accessors\":[" + join( accs ) + "],"
            "\"bufferViews\":[" + join( views ) + "],"
            "\"buffers\":[{\"byteLength\":" + std::to_string( bin.size() ) + "}]}";

        while (json.size() % 4) json += ' ';
        const size_t bin_padded = (bin.size() + 3) & ~size_t( 3 );
        const uint32_t total = 12 + 8 + (uint32_t)json.size() + 8 + (uint32_t)bin_padded;

        std::ofstream f( path, std::ios::binary );
        if (!f) return false;
        auto u32 = [&]( uint32_t v ) { f.write( (const char*)&v, 4 ); };
        u32( 0x46546C67 ); u32( 2 ); u32( total );
        u32( (uint32_t)json.size() ); u32( 0x4E4F534A );
        f.write( json.data(), (std::streamsize)json.size() );
        u32( (uint32_t)bin_padded ); u32( 0x004E4942 );
        f.write( (const char*)bin.data(), (std::streamsize)bin.size() );
        for (size_t i = bin.size(); i < bin_padded; ++i) f.put( '\0' );
        return (bool)f;
    }

private:
    std::vector<uint8_t> bin;
    std::vector<std::string> views, accs, mats, meshes, nodes;
    std::map<std::string, int> mat_index;

    int view( const void* data, size_t bytes, int target )
    {
        size_t off = bin.size();
        const uint8_t* p = (const uint8_t*)data;
        bin.insert( bin.end(), p, p + bytes );
        views.push_back( "{\"buffer\":0,\"byteOffset\":" + std::to_string( off ) +
                         ",\"byteLength\":" + std::to_string( bytes ) +
                         ",\"target\":" + std::to_string( target ) + "}" );
        return (int)views.size() - 1;
    }

    int accessor( int v, int comp, size_t count, const char* type, const std::string& extra = "" )
    {
        accs.push_back( "{\"bufferView\":" + std::to_string( v ) + ",\"componentType\":" + std::to_string( comp ) +
                        ",\"count\":" + std::to_string( count ) + ",\"type\":\"" + type + "\"" + extra + "}" );
        return (int)accs.size() - 1;
    }

    int material( const std::string& name )
    {
        auto it = mat_index.find( name );
        if (it != mat_index.end()) return it->second;
        mats.push_back( "{\"name\":\"" + json_escape( name ) +
                        "\",\"pbrMetallicRoughness\":{\"metallicFactor\":0,\"roughnessFactor\":1}}" );
        return mat_index[name] = (int)mats.size() - 1;
    }

    std::string primitive( const std::string& tex, const Batch& b )
    {
        const size_t vcount = b.pos.size() / 3;
        float mn[3] = { 1e30f, 1e30f, 1e30f }, mx[3] = { -1e30f, -1e30f, -1e30f };
        for (size_t i = 0; i < vcount; ++i)
            for (int a = 0; a < 3; ++a)
            {
                mn[a] = std::min( mn[a], b.pos[i * 3 + a] );
                mx[a] = std::max( mx[a], b.pos[i * 3 + a] );
            }

        std::ostringstream mm;
        mm << std::setprecision( 9 ) << ",\"min\":[" << mn[0] << "," << mn[1] << "," << mn[2]
            << "],\"max\":[" << mx[0] << "," << mx[1] << "," << mx[2] << "]";

        int a_pos = accessor( view( b.pos.data(), b.pos.size() * 4, 34962 ), 5126, vcount, "VEC3", mm.str() );
        int a_nrm = accessor( view( b.nrm.data(), b.nrm.size() * 4, 34962 ), 5126, vcount, "VEC3" );
        int a_uv = accessor( view( b.uv.data(), b.uv.size() * 4, 34962 ), 5126, vcount, "VEC2" );
        int a_idx = accessor( view( b.idx.data(), b.idx.size() * 4, 34963 ), 5125, b.idx.size(), "SCALAR" );

        return "{\"attributes\":{\"POSITION\":" + std::to_string( a_pos ) + ",\"NORMAL\":" + std::to_string( a_nrm ) +
            ",\"TEXCOORD_0\":" + std::to_string( a_uv ) + "},\"indices\":" + std::to_string( a_idx ) +
            ",\"material\":" + std::to_string( material( tex ) ) + "}";
    }
};

// ---------- main ----------
int main( int argc, char* argv[] )
{
    if (argc < 3)
    {
        std::fprintf( stderr, "usage: map_reader <file.map> <output_dir> [texture_dir]\n" );
        return 1;
    }
    const fs::path map_path = argv[1];
    const fs::path out_dir = argv[2];
    TexSizes sizes( argc > 3 ? fs::path( argv[3] ) : fs::path( "." ) );
    TexSizeFn size_fn = [&]( const std::string& n ) { return sizes( n ); };

    std::ifstream in( map_path, std::ios::binary );
    if (!in) { std::fprintf( stderr, "cannot open %s\n", argv[1] ); return 1; }
    std::stringstream ss;
    ss << in.rdbuf();

    try
    {
        std::vector<Entity> entities = Parser( tokenize( ss.str() ) ).parse();

        Glb glb;
        std::vector<std::string> entity_json;
        size_t tris = 0;

        for (size_t e = 0; e < entities.size(); ++e)
        {
            const Entity& ent = entities[e];
            auto c = ent.props.find( "classname" );
            const std::string cls = c != ent.props.end() ? c->second : "unknown";
            const bool visual = cls.rfind( "trigger", 0 ) != 0;  // trigger_* entities are volumes only

            std::ostringstream ej;
            ej << std::setprecision( 9 );
            ej << "{\"classname\":\"" << json_escape( cls ) << "\"";

            auto o = ent.props.find( "origin" );
            double ox, oy, oz;
            if (o != ent.props.end() && std::sscanf( o->second.c_str(), "%lf %lf %lf", &ox, &oy, &oz ) == 3)
            {
                Vec3 g = to_gl( { ox, oy, oz } );
                ej << ",\"origin\":[" << g.x * MAP_SCALE << "," << g.y * MAP_SCALE << "," << g.z * MAP_SCALE << "]";
            }

            ej << ",\"props\":{";
            bool first = true;
            for (const auto& [k, v] : ent.props)
            {
                if (!first) ej << ",";
                first = false;
                ej << "\"" << json_escape( k ) << "\":\"" << json_escape( v ) << "\"";
            }

            // Collision: every face plane of every brush, in glTF space. Inside = dot(n, p) < d for all planes.
            ej << "},\"brushes\":[";
            std::map<std::string, Batch> batches;
            for (size_t bi = 0; bi < ent.brushes.size(); ++bi)
            {
                const Brush& brush = ent.brushes[bi];
                if (bi) ej << ",";
                ej << "[";
                for (size_t fi = 0; fi < brush.faces.size(); ++fi)
                {
                    const Face& f = brush.faces[fi];
                    Vec3 n = to_gl( f.normal );
                    if (fi) ej << ",";
                    ej << "[" << n.x << "," << n.y << "," << n.z << "," << f.dist * MAP_SCALE << "]";
                }
                ej << "]";

                if (!visual) continue;
                for (const Polygon& poly : build_polygons( brush, size_fn ))
                    if (!skip_texture( poly.tex )) add_polygon( batches[poly.tex], poly );
            }
            ej << "]}";
            entity_json.push_back( ej.str() );

            if (!batches.empty())
            {
                for (const auto& [t, b] : batches) tris += b.idx.size() / 3;
                glb.add_node( cls + "_" + std::to_string( e ), batches );  // node name ends with the entity index
            }
        }

        fs::create_directories( out_dir );
        const std::string stem = map_path.stem().string();

        if (!glb.empty() && !glb.write( out_dir / (stem + ".glb") ))
        {
            std::fprintf( stderr, "failed to write glb\n" );
            return 1;
        }

        std::ofstream js( out_dir / (stem + ".json") );
        js << "{\"scale\":" << MAP_SCALE << ",\"entities\":[" << join( entity_json ) << "]}\n";
        if (!js) { std::fprintf( stderr, "failed to write json\n" ); return 1; }

        std::printf( "%zu entities, %zu triangles -> %s.glb / %s.json\n",
                     entities.size(), tris, (out_dir / stem).string().c_str(), (out_dir / stem).string().c_str() );
    }
    catch (const std::exception& ex)
    {
        std::fprintf( stderr, "parse error: %s\n", ex.what() );
        return 1;
    }
    return 0;
}