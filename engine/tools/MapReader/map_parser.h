#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// ---------- math ----------
struct Vec3 { double x = 0, y = 0, z = 0; };
static Vec3 operator+( Vec3 a, Vec3 b ) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
static Vec3 operator-( Vec3 a, Vec3 b ) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static Vec3 operator*( Vec3 a, double s ) { return { a.x * s, a.y * s, a.z * s }; }
static double dot( Vec3 a, Vec3 b ) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static Vec3 cross( Vec3 a, Vec3 b )
{
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
static Vec3 normalize( Vec3 a )
{
    double l = std::sqrt( dot( a, a ) );
    return l > 0 ? a * (1.0 / l) : a;
}

// ---------- map data ----------
struct Face
{
    Vec3 p[3];
    std::string tex;
    Vec3 uaxis, vaxis;
    double uoff = 0, voff = 0, rot = 0, xscale = 1, yscale = 1;
    Vec3 normal;
    double dist = 0;
};

struct Brush { std::vector<Face> faces; };

struct Entity
{
    std::map<std::string, std::string> props;
    std::vector<Brush> brushes;
};

// ---------- tokenizer ----------
enum class Tok { Punct, String, Word };
struct Token { Tok kind; std::string text; };

static bool is_punct( char c )
{
    return c == '{' || c == '}' || c == '(' || c == ')' || c == '[' || c == ']';
}

static std::vector<Token> tokenize( const std::string& s )
{
    std::vector<Token> out;
    size_t i = 0;
    while (i < s.size())
    {
        char c = s[i];
        if (std::isspace( (unsigned char)c )) { ++i; continue; }
        if (c == '/' && i + 1 < s.size() && s[i + 1] == '/')
        {
            while (i < s.size() && s[i] != '\n') ++i;
            continue;
        }
        if (is_punct( c )) { out.push_back( { Tok::Punct, std::string( 1, c ) } ); ++i; continue; }
        if (c == '"')
        {
            size_t start = ++i;
            while (i < s.size() && s[i] != '"') ++i;
            if (i >= s.size()) throw std::runtime_error( "unterminated string" );
            out.push_back( { Tok::String, s.substr( start, i - start ) } );
            ++i;
            continue;
        }
        size_t start = i;
        while (i < s.size() && !std::isspace( (unsigned char)s[i] ) && !is_punct( s[i] )) ++i;
        out.push_back( { Tok::Word, s.substr( start, i - start ) } );
    }
    return out;
}

// ---------- parser ----------
class Parser
{
public:
    explicit Parser( std::vector<Token> t ) : toks( std::move( t ) ) {}

    std::vector<Entity> parse()
    {
        std::vector<Entity> out;
        while (pos < toks.size()) out.push_back( entity() );
        return out;
    }

private:
    std::vector<Token> toks;
    size_t pos = 0;

    [[noreturn]] void fail( const std::string& msg ) const
    {
        throw std::runtime_error( msg + " (token " + std::to_string( pos ) + ")" );
    }
    Token take()
    {
        if (pos >= toks.size()) fail( "unexpected end of file" );
        return toks[pos++];
    }
    bool at( const char* p ) const
    {
        return pos < toks.size() && toks[pos].kind == Tok::Punct && toks[pos].text == p;
    }
    void expect( const char* p )
    {
        if (!at( p )) fail( std::string( "expected '" ) + p + "'" );
        ++pos;
    }
    double number()
    {
        Token t = take();
        if (t.kind != Tok::Word) fail( "expected number" );
        char* end = nullptr;
        double v = std::strtod( t.text.c_str(), &end );
        if (*end != '\0') fail( "bad number '" + t.text + "'" );
        return v;
    }
    Vec3 vec3()
    {
        Vec3 v;
        v.x = number(); v.y = number(); v.z = number();
        return v;
    }

    Entity entity()
    {
        Entity e;
        expect( "{" );
        while (!at( "}" ))
        {
            if (at( "{" )) { e.brushes.push_back( brush() ); continue; }
            Token k = take(), v = take();
            if (k.kind != Tok::String || v.kind != Tok::String) fail( "expected key/value" );
            e.props[k.text] = v.text;
        }
        expect( "}" );
        return e;
    }

    Brush brush()
    {
        Brush b;
        expect( "{" );
        while (!at( "}" )) b.faces.push_back( face() );
        expect( "}" );
        return b;
    }

    Face face()
    {
        Face f;
        for (Vec3& p : f.p) { expect( "(" ); p = vec3(); expect( ")" ); }
        f.tex = take().text;

        if (!at( "[" )) fail( "expected Valve 220 texture axes; is the map in Valve format?" );
        expect( "[" ); f.uaxis = vec3(); f.uoff = number(); expect( "]" );
        expect( "[" ); f.vaxis = vec3(); f.voff = number(); expect( "]" );
        f.rot = number(); f.xscale = number(); f.yscale = number();

        // Quake2 (Valve) appends contents/flags/value; skip them if present
        while (pos < toks.size() && toks[pos].kind == Tok::Word) ++pos;

        f.normal = normalize( cross( f.p[2] - f.p[0], f.p[1] - f.p[0] ) );
        f.dist = dot( f.normal, f.p[0] );
        return f;
    }
};

// ---------- brush -> polygons ----------
struct Vertex { Vec3 pos; double u, v; };
struct Polygon { std::string tex; Vec3 normal; std::vector<Vertex> verts; };

using TexSizeFn = std::function<std::pair<double, double>( const std::string& )>;

static std::vector<Vec3> clip( const std::vector<Vec3>& poly, Vec3 n, double d, double eps )
{
    std::vector<Vec3> out;
    for (size_t i = 0; i < poly.size(); ++i)
    {
        Vec3 a = poly[i], b = poly[(i + 1) % poly.size()];
        double da = dot( n, a ) - d, db = dot( n, b ) - d;
        bool ain = da <= eps, bin = db <= eps;
        if (ain) out.push_back( a );
        if (ain != bin) out.push_back( a + (b - a) * (da / (da - db)) );
    }
    return out;
}

static std::vector<Polygon> build_polygons( const Brush& b, const TexSizeFn& tex_size )
{
    const double eps = 1e-4, big = 100000.0;
    std::vector<Polygon> result;

    for (size_t i = 0; i < b.faces.size(); ++i)
    {
        const Face& f = b.faces[i];
        auto [tex_w, tex_h] = tex_size( f.tex );

        // Huge quad lying on this face's plane, wound counter-clockwise from outside
        Vec3 ref = std::fabs( f.normal.z ) < 0.9 ? Vec3{ 0, 0, 1 } : Vec3{ 1, 0, 0 };
        Vec3 u = normalize( cross( f.normal, ref ) );
        Vec3 v = cross( f.normal, u );
        Vec3 c = f.normal * f.dist;
        std::vector<Vec3> poly = {
            c - u * big - v * big, c + u * big - v * big,
            c + u * big + v * big, c - u * big + v * big,
        };

        // Cut it down by every other plane of the brush
        for (size_t j = 0; j < b.faces.size() && poly.size() >= 3; ++j)
            if (j != i) poly = clip( poly, b.faces[j].normal, b.faces[j].dist, eps );

        if (poly.size() < 3) continue;

        Polygon out{ f.tex, f.normal, {} };
        for (Vec3 p : poly)
        {
            double tu = (dot( p, f.uaxis ) / f.xscale + f.uoff) / tex_w;
            double tv = (dot( p, f.vaxis ) / f.yscale + f.voff) / tex_h;
            out.verts.push_back( { p, tu, tv } );
        }
        result.push_back( std::move( out ) );
    }
    return result;
}