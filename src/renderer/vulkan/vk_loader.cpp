#include "renderer/vulkan/vk_loader.hpp"
#include "renderer/vulkan/vk_renderer.hpp"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>

// using namespace VK;

// ------------------------------------------------------------
// Minimal JSON
// ------------------------------------------------------------

namespace VK
{

  struct Json
  {
    enum class Type
    {
      Null,
      Bool,
      Number,
      String,
      Array,
      Object
    };

    Type type = Type::Null;

    bool boolean = false;
    double number = 0.0;
    std::string string;

    std::vector<Json> array;
    std::unordered_map<std::string, Json> object;

    const Json *Get(const char *key) const
    {
      if (type != Type::Object)
        return nullptr;

      auto it = object.find(key);
      if (it == object.end())
        return nullptr;

      return &it->second;
    }

    const Json &operator[](const char *key) const
    {
      static Json empty;
      const Json *value = Get(key);
      return value ? *value : empty;
    }

    bool IsNull() const
    {
      return type == Type::Null;
    }

    int Int() const
    {
      return static_cast<int>(number);
    }

    size_t Size() const
    {
      return array.size();
    }
  };

  class JsonParser
  {
  public:
    explicit JsonParser(std::string_view source)
        : m_source(source)
    {
    }

    std::optional<Json> Parse()
    {
      SkipWhitespace();

      Json result;

      if (!ParseValue(result))
        return {};

      SkipWhitespace();

      if (m_pos != m_source.size())
        return {};

      return result;
    }

  private:
    std::string_view m_source;
    size_t m_pos = 0;

    void SkipWhitespace()
    {
      while (m_pos < m_source.size())
      {
        char c = m_source[m_pos];

        if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
          ++m_pos;
        else
          break;
      }
    }

    bool Consume(char c)
    {
      SkipWhitespace();

      if (m_pos >= m_source.size() || m_source[m_pos] != c)
        return false;

      ++m_pos;
      return true;
    }

    bool ParseValue(Json &out)
    {
      SkipWhitespace();

      if (m_pos >= m_source.size())
        return false;

      switch (m_source[m_pos])
      {
      case '{':
        return ParseObject(out);

      case '[':
        return ParseArray(out);

      case '"':
        out.type = Json::Type::String;
        return ParseString(out.string);

      case 't':
        if (m_source.substr(m_pos, 4) == "true")
        {
          m_pos += 4;
          out.type = Json::Type::Bool;
          out.boolean = true;
          return true;
        }
        return false;

      case 'f':
        if (m_source.substr(m_pos, 5) == "false")
        {
          m_pos += 5;
          out.type = Json::Type::Bool;
          out.boolean = false;
          return true;
        }
        return false;

      case 'n':
        if (m_source.substr(m_pos, 4) == "null")
        {
          m_pos += 4;
          out.type = Json::Type::Null;
          return true;
        }
        return false;

      default:
        return ParseNumber(out);
      }
    }

    bool ParseObject(Json &out)
    {
      if (!Consume('{'))
        return false;

      out.type = Json::Type::Object;
      out.object.clear();

      SkipWhitespace();

      if (m_pos < m_source.size() && m_source[m_pos] == '}')
      {
        ++m_pos;
        return true;
      }

      while (m_pos < m_source.size())
      {
        SkipWhitespace();

        if (m_source[m_pos] != '"')
          return false;

        std::string key;

        if (!ParseString(key))
          return false;

        if (!Consume(':'))
          return false;

        Json value;

        if (!ParseValue(value))
          return false;

        out.object.emplace(std::move(key), std::move(value));

        SkipWhitespace();

        if (m_pos >= m_source.size())
          return false;

        if (m_source[m_pos] == '}')
        {
          ++m_pos;
          return true;
        }

        if (m_source[m_pos] != ',')
          return false;

        ++m_pos;
      }

      return false;
    }

    bool ParseArray(Json &out)
    {
      if (!Consume('['))
        return false;

      out.type = Json::Type::Array;
      out.array.clear();

      SkipWhitespace();

      if (m_pos < m_source.size() && m_source[m_pos] == ']')
      {
        ++m_pos;
        return true;
      }

      while (m_pos < m_source.size())
      {
        Json value;

        if (!ParseValue(value))
          return false;

        out.array.push_back(std::move(value));

        SkipWhitespace();

        if (m_pos >= m_source.size())
          return false;

        if (m_source[m_pos] == ']')
        {
          ++m_pos;
          return true;
        }

        if (m_source[m_pos] != ',')
          return false;

        ++m_pos;
      }

      return false;
    }

    bool ParseString(std::string &out)
    {
      if (m_pos >= m_source.size() || m_source[m_pos] != '"')
        return false;

      ++m_pos;
      out.clear();

      while (m_pos < m_source.size())
      {
        char c = m_source[m_pos++];

        if (c == '"')
          return true;

        if (c != '\\')
        {
          out.push_back(c);
          continue;
        }

        if (m_pos >= m_source.size())
          return false;

        char escaped = m_source[m_pos++];

        switch (escaped)
        {
        case '"':
          out.push_back('"');
          break;
        case '\\':
          out.push_back('\\');
          break;
        case '/':
          out.push_back('/');
          break;
        case 'b':
          out.push_back('\b');
          break;
        case 'f':
          out.push_back('\f');
          break;
        case 'n':
          out.push_back('\n');
          break;
        case 'r':
          out.push_back('\r');
          break;
        case 't':
          out.push_back('\t');
          break;

        case 'u':
        {
          if (m_pos + 4 > m_source.size())
            return false;

          unsigned value = 0;

          for (int i = 0; i < 4; ++i)
          {
            char h = m_source[m_pos++];

            value <<= 4;

            if (h >= '0' && h <= '9')
              value |= (unsigned int)(h - '0');
            else if (h >= 'a' && h <= 'f')
              value |= (unsigned int)(h - 'a' + 10);
            else if (h >= 'A' && h <= 'F')
              value |= (unsigned int)(h - 'A' + 10);
            else
              return false;
          }

          // Basic UTF-8 conversion.
          if (value <= 0x7F)
          {
            out.push_back(static_cast<char>(value));
          }
          else if (value <= 0x7FF)
          {
            out.push_back(static_cast<char>(
                0xC0 | (value >> 6)));

            out.push_back(static_cast<char>(
                0x80 | (value & 0x3F)));
          }
          else
          {
            out.push_back(static_cast<char>(
                0xE0 | (value >> 12)));

            out.push_back(static_cast<char>(
                0x80 | ((value >> 6) & 0x3F)));

            out.push_back(static_cast<char>(
                0x80 | (value & 0x3F)));
          }

          break;
        }

        default:
          return false;
        }
      }

      return false;
    }

    bool ParseNumber(Json &out)
    {
      SkipWhitespace();

      size_t start = m_pos;

      if (m_pos < m_source.size() && m_source[m_pos] == '-')
        ++m_pos;

      while (m_pos < m_source.size() &&
             std::isdigit(static_cast<unsigned char>(m_source[m_pos])))
      {
        ++m_pos;
      }

      if (m_pos < m_source.size() && m_source[m_pos] == '.')
      {
        ++m_pos;

        while (m_pos < m_source.size() &&
               std::isdigit(static_cast<unsigned char>(m_source[m_pos])))
        {
          ++m_pos;
        }
      }

      if (m_pos < m_source.size() &&
          (m_source[m_pos] == 'e' || m_source[m_pos] == 'E'))
      {
        ++m_pos;

        if (m_pos < m_source.size() &&
            (m_source[m_pos] == '+' || m_source[m_pos] == '-'))
        {
          ++m_pos;
        }

        while (m_pos < m_source.size() &&
               std::isdigit(static_cast<unsigned char>(m_source[m_pos])))
        {
          ++m_pos;
        }
      }

      if (start == m_pos)
        return false;

      std::string number(
          m_source.substr(start, m_pos - start));

      out.type = Json::Type::Number;
      out.number = std::strtod(number.c_str(), nullptr);

      return true;
    }
  };

  // ------------------------------------------------------------
  // Binary helpers
  // ------------------------------------------------------------

  uint32_t ReadU32(const uint8_t *data)
  {
    return static_cast<uint32_t>(data[0]) |
           (static_cast<uint32_t>(data[1]) << 8) |
           (static_cast<uint32_t>(data[2]) << 16) |
           (static_cast<uint32_t>(data[3]) << 24);
  }

  float ReadF32(const uint8_t *data)
  {
    float value;
    std::memcpy(&value, data, sizeof(float));
    return value;
  }

  std::vector<uint8_t> ReadFile(const std::filesystem::path &path)
  {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file)
      return {};

    std::streamsize size = file.tellg();

    if (size <= 0)
      return {};

    file.seekg(0);

    std::vector<uint8_t> data(static_cast<size_t>(size));

    if (!file.read(
            reinterpret_cast<char *>(data.data()),
            size))
    {
      return {};
    }

    return data;
  }

  // ------------------------------------------------------------
  // GLB
  // ------------------------------------------------------------

  struct GltfFile
  {
    Json json;
    std::vector<std::vector<uint8_t>> buffers;
  };

  bool LoadGLB(
      const std::vector<uint8_t> &file,
      GltfFile &output)
  {
    if (file.size() < 20)
      return false;

    if (ReadU32(file.data()) != 0x46546C67)
      return false;

    uint32_t version = ReadU32(file.data() + 4);

    if (version != 2)
      return false;

    uint32_t length = ReadU32(file.data() + 8);

    if (length > file.size())
      return false;

    size_t offset = 12;

    std::string jsonText;
    std::vector<uint8_t> binary;

    while (offset + 8 <= length)
    {
      uint32_t chunkLength = ReadU32(file.data() + offset);
      uint32_t chunkType = ReadU32(file.data() + offset + 4);

      offset += 8;

      if (offset + chunkLength > length)
        return false;

      const uint8_t *chunk = file.data() + offset;

      // JSON
      if (chunkType == 0x4E4F534A)
      {
        jsonText.assign(
            reinterpret_cast<const char *>(chunk),
            chunkLength);
      }
      // BIN
      else if (chunkType == 0x004E4942)
      {
        binary.assign(
            chunk,
            chunk + chunkLength);
      }

      offset += chunkLength;
    }

    auto parsed = JsonParser(jsonText).Parse();

    if (!parsed)
      return false;

    output.json = std::move(*parsed);

    const Json &buffers = output.json["buffers"];

    if (buffers.type != Json::Type::Array)
      return false;

    for (size_t i = 0; i < buffers.Size(); ++i)
    {
      const Json &buffer = buffers.array[i];

      // GLB's first buffer normally has no URI.
      if (buffer.Get("uri") == nullptr)
      {
        output.buffers.push_back(std::move(binary));
      }
      else
      {
        output.buffers.emplace_back();
      }
    }

    return true;
  }

  bool LoadGLTF(
      const std::filesystem::path &filepath,
      GltfFile &output)
  {
    std::vector<uint8_t> file = ReadFile(filepath);

    if (file.empty())
      return false;

    // GLB magic.
    if (file.size() >= 4 && ReadU32(file.data()) == 0x46546C67)
      return LoadGLB(file, output);

    std::string jsonText(
        reinterpret_cast<const char *>(file.data()),
        file.size());

    auto parsed = JsonParser(jsonText).Parse();

    if (!parsed)
      return false;

    output.json = std::move(*parsed);

    const Json &buffers = output.json["buffers"];

    if (buffers.type != Json::Type::Array)
      return false;

    for (const Json &buffer : buffers.array)
    {
      const Json *uri = buffer.Get("uri");

      if (!uri || uri->type != Json::Type::String)
        return false;

      // This loader intentionally doesn't implement data: URIs.
      if (uri->string.rfind("data:", 0) == 0)
        return false;

      std::filesystem::path bufferPath =
          filepath.parent_path() / uri->string;

      output.buffers.push_back(ReadFile(bufferPath));

      if (output.buffers.back().empty())
        return false;
    }

    return true;
  }

  // ------------------------------------------------------------
  // Accessor reading
  // ------------------------------------------------------------

  size_t ComponentSize(int componentType)
  {
    switch (componentType)
    {
    case 5120:
      return 1; // BYTE
    case 5121:
      return 1; // UNSIGNED_BYTE
    case 5122:
      return 2; // SHORT
    case 5123:
      return 2; // UNSIGNED_SHORT
    case 5125:
      return 4; // UNSIGNED_INT
    case 5126:
      return 4; // FLOAT
    default:
      return 0;
    }
  }

  size_t ComponentCount(const std::string &type)
  {
    if (type == "SCALAR")
      return 1;
    if (type == "VEC2")
      return 2;
    if (type == "VEC3")
      return 3;
    if (type == "VEC4")
      return 4;
    if (type == "MAT2")
      return 4;
    if (type == "MAT3")
      return 9;
    if (type == "MAT4")
      return 16;

    return 0;
  }

  float ReadComponentAsFloat(
      const uint8_t *data,
      int componentType,
      bool normalized)
  {
    switch (componentType)
    {
    case 5120:
    {
      int8_t value;
      std::memcpy(&value, data, 1);

      if (!normalized)
        return static_cast<float>(value);

      return std::fmax(
          static_cast<float>(value) / 127.0f,
          -1.0f);
    }

    case 5121:
    {
      uint8_t value;
      std::memcpy(&value, data, 1);

      if (!normalized)
        return static_cast<float>(value);

      return static_cast<float>(value) / 255.0f;
    }

    case 5122:
    {
      int16_t value;
      std::memcpy(&value, data, 2);

      if (!normalized)
        return static_cast<float>(value);

      return std::fmax(
          static_cast<float>(value) / 32767.0f,
          -1.0f);
    }

    case 5123:
    {
      uint16_t value;
      std::memcpy(&value, data, 2);

      if (!normalized)
        return static_cast<float>(value);

      return static_cast<float>(value) / 65535.0f;
    }

    case 5125:
    {
      uint32_t value;
      std::memcpy(&value, data, 4);

      return static_cast<float>(value);
    }

    case 5126:
      return ReadF32(data);

    default:
      return 0.0f;
    }
  }

  uint32_t ReadComponentAsIndex(
      const uint8_t *data,
      int componentType)
  {
    switch (componentType)
    {
    case 5121:
      return data[0];

    case 5123:
    {
      uint16_t value;
      std::memcpy(&value, data, 2);
      return value;
    }

    case 5125:
    {
      uint32_t value;
      std::memcpy(&value, data, 4);
      return value;
    }

    default:
      return 0;
    }
  }

  struct AccessorView
  {
    const uint8_t *data = nullptr;

    size_t count = 0;
    size_t stride = 0;

    int componentType = 0;
    size_t components = 0;
    bool normalized = false;
  };

  bool GetAccessor(
      const Json &gltf,
      const std::vector<std::vector<uint8_t>> &buffers,
      size_t accessorIndex,
      AccessorView &out)
  {
    const Json &accessors = gltf["accessors"];
    const Json &views = gltf["bufferViews"];

    if (accessors.type != Json::Type::Array ||
        views.type != Json::Type::Array ||
        accessorIndex >= accessors.Size())
    {
      return false;
    }

    const Json &accessor = accessors.array[accessorIndex];

    const Json *bufferViewValue =
        accessor.Get("bufferView");

    if (!bufferViewValue)
      return false;

    size_t bufferViewIndex =
        static_cast<size_t>(bufferViewValue->Int());

    if (bufferViewIndex >= views.Size())
      return false;

    const Json &view = views.array[bufferViewIndex];

    size_t bufferIndex =
        static_cast<size_t>(view["buffer"].Int());

    if (bufferIndex >= buffers.size())
      return false;

    const auto &buffer = buffers[bufferIndex];

    size_t bufferOffset = 0;

    if (const Json *value = view.Get("byteOffset"))
      bufferOffset = static_cast<size_t>(value->Int());

    size_t accessorOffset = 0;

    if (const Json *value = accessor.Get("byteOffset"))
      accessorOffset = static_cast<size_t>(value->Int());

    size_t componentSize =
        ComponentSize(accessor["componentType"].Int());

    size_t components =
        ComponentCount(accessor["type"].string);

    if (componentSize == 0 || components == 0)
      return false;

    size_t elementSize =
        componentSize * components;

    size_t stride = elementSize;

    if (const Json *value = view.Get("byteStride"))
      stride = static_cast<size_t>(value->Int());

    size_t offset =
        bufferOffset + accessorOffset;

    size_t count =
        static_cast<size_t>(accessor["count"].Int());

    if (offset > buffer.size())
      return false;

    if (count > 0)
    {
      size_t required =
          offset +
          (count - 1) * stride +
          elementSize;

      if (required > buffer.size())
        return false;
    }

    out.data = buffer.data() + offset;
    out.count = count;
    out.stride = stride;
    out.componentType = accessor["componentType"].Int();
    out.components = components;

    const Json *normalized =
        accessor.Get("normalized");

    out.normalized =
        normalized && normalized->boolean;

    return true;
  }

  // ------------------------------------------------------------
  // Attribute lookup
  // ------------------------------------------------------------

  const Json *FindAttribute(
      const Json &primitive,
      const char *name)
  {
    const Json &attributes = primitive["attributes"];

    if (attributes.type != Json::Type::Object)
      return nullptr;

    return attributes.Get(name);
  }

  // ------------------------------------------------------------
  // Main loader
  // ------------------------------------------------------------

  std::optional<std::vector<std::shared_ptr<MeshAsset>>>
  loadGltfMeshes(
      Renderer * renderer,
      std::filesystem::path filepath)
  {
    GltfFile gltf;

    if (!LoadGLTF(filepath, gltf))
    {
      std::printf(
          "Failed to load glTF: %s\n",
          filepath.c_str());

      return {};
    }

    const Json &meshesJson = gltf.json["meshes"];

    if (meshesJson.type != Json::Type::Array)
    {
      std::printf("glTF contains no meshes\n");
      return {};
    }

    std::vector<std::shared_ptr<MeshAsset>> meshes;

    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;

    for (const Json &mesh : meshesJson.array)
    {
      MeshAsset newmesh;

      if (const Json *name = mesh.Get("name"))
        newmesh.name = name->string;

      indices.clear();
      vertices.clear();

      const Json &primitives = mesh["primitives"];

      if (primitives.type != Json::Type::Array)
        continue;

      for (const Json &primitive : primitives.array)
      {
        Surface newSurface;

        newSurface.start_index =
            static_cast<uint32_t>(indices.size());

        const Json *position =
            FindAttribute(primitive, "POSITION");

        if (!position)
          continue;

        AccessorView posAccessor;

        if (!GetAccessor(
                gltf.json,
                gltf.buffers,
                static_cast<size_t>(position->Int()),
                posAccessor))
        {
          std::printf("Invalid POSITION accessor\n");
          return {};
        }

        size_t initialVertex =
            vertices.size();

        vertices.resize(
            vertices.size() +
            posAccessor.count);

        // ------------------------------------------------
        // Positions
        // ------------------------------------------------

        for (size_t i = 0; i < posAccessor.count; ++i)
        {
          const uint8_t *element =
              posAccessor.data +
              i * posAccessor.stride;

          Vertex &vertex =
              vertices[initialVertex + i];

          vertex.pos[0] =
              ReadComponentAsFloat(
                  element,
                  posAccessor.componentType,
                  posAccessor.normalized);

          vertex.pos[1] =
              ReadComponentAsFloat(
                  element +
                      ComponentSize(posAccessor.componentType),
                  posAccessor.componentType,
                  posAccessor.normalized);

          vertex.pos[2] =
              ReadComponentAsFloat(
                  element +
                      ComponentSize(posAccessor.componentType) * 2,
                  posAccessor.componentType,
                  posAccessor.normalized);

          // Default normal.
          vertex.normal[0] = 1.0f;
          vertex.normal[1] = 0.0f;
          vertex.normal[2] = 0.0f;

          // Default colour.
          vertex.col[0] = 1.0f;
          vertex.col[1] = 1.0f;
          vertex.col[2] = 1.0f;
          vertex.col[3] = 1.0f;

          // Default UV.
          vertex.uv[0] = 0.0f;
          vertex.uv[1] = 0.0f;
        }

        // ------------------------------------------------
        // Normals
        // ------------------------------------------------

        if (const Json *normal =
                FindAttribute(primitive, "NORMAL"))
        {
          AccessorView accessor;

          if (!GetAccessor(
                  gltf.json,
                  gltf.buffers,
                  static_cast<size_t>(normal->Int()),
                  accessor))
          {
            return {};
          }

          size_t componentSize =
              ComponentSize(accessor.componentType);

          size_t count =
              std::min(
                  accessor.count,
                  posAccessor.count);

          for (size_t i = 0; i < count; ++i)
          {
            const uint8_t *element =
                accessor.data +
                i * accessor.stride;

            Vertex &vertex =
                vertices[initialVertex + i];

            vertex.normal[0] =
                ReadComponentAsFloat(
                    element,
                    accessor.componentType,
                    accessor.normalized);

            vertex.normal[1] =
                ReadComponentAsFloat(
                    element + componentSize,
                    accessor.componentType,
                    accessor.normalized);

            vertex.normal[2] =
                ReadComponentAsFloat(
                    element + componentSize * 2,
                    accessor.componentType,
                    accessor.normalized);
          }
        }

        // ------------------------------------------------
        // UVs
        // ------------------------------------------------

        if (const Json *uv =
                FindAttribute(primitive, "TEXCOORD_0"))
        {
          AccessorView accessor;

          if (!GetAccessor(
                  gltf.json,
                  gltf.buffers,
                  static_cast<size_t>(uv->Int()),
                  accessor))
          {
            return {};
          }

          size_t componentSize =
              ComponentSize(accessor.componentType);

          size_t count =
              std::min(
                  accessor.count,
                  posAccessor.count);

          for (size_t i = 0; i < count; ++i)
          {
            const uint8_t *element =
                accessor.data +
                i * accessor.stride;

            Vertex &vertex =
                vertices[initialVertex + i];

            vertex.uv[0] =
                ReadComponentAsFloat(
                    element,
                    accessor.componentType,
                    accessor.normalized);

            vertex.uv[1] =
                ReadComponentAsFloat(
                    element + componentSize,
                    accessor.componentType,
                    accessor.normalized);
          }
        }

        // ------------------------------------------------
        // Vertex colours
        // ------------------------------------------------

        if (const Json *colors =
                FindAttribute(primitive, "COLOR_0"))
        {
          AccessorView accessor;

          if (!GetAccessor(
                  gltf.json,
                  gltf.buffers,
                  static_cast<size_t>(colors->Int()),
                  accessor))
          {
            return {};
          }

          size_t componentSize =
              ComponentSize(accessor.componentType);

          size_t count =
              std::min(
                  accessor.count,
                  posAccessor.count);

          for (size_t i = 0; i < count; ++i)
          {
            const uint8_t *element =
                accessor.data +
                i * accessor.stride;

            Vertex &vertex =
                vertices[initialVertex + i];

            vertex.col[0] =
                ReadComponentAsFloat(
                    element,
                    accessor.componentType,
                    accessor.normalized);

            vertex.col[1] =
                ReadComponentAsFloat(
                    element + componentSize,
                    accessor.componentType,
                    accessor.normalized);

            vertex.col[2] =
                ReadComponentAsFloat(
                    element + componentSize * 2,
                    accessor.componentType,
                    accessor.normalized);

            // COLOR_0 can legally be VEC3.
            if (accessor.components >= 4)
            {
              vertex.col[3] =
                  ReadComponentAsFloat(
                      element + componentSize * 3,
                      accessor.componentType,
                      accessor.normalized);
            }
          }
        }

        // ------------------------------------------------
        // Indices
        // ------------------------------------------------

        const Json *indexAccessorIndex =
            primitive.Get("indices");

        if (!indexAccessorIndex)
        {
          // Non-indexed primitive.
          for (uint32_t i = 0;
               i < posAccessor.count;
               ++i)
          {
            indices.push_back(
                static_cast<uint32_t>(
                    initialVertex + i));
          }

          newSurface.count =
              static_cast<uint32_t>(
                  posAccessor.count);
        }
        else
        {
          AccessorView accessor;

          if (!GetAccessor(
                  gltf.json,
                  gltf.buffers,
                  static_cast<size_t>(
                      indexAccessorIndex->Int()),
                  accessor))
          {
            return {};
          }

          size_t componentSize =
              ComponentSize(accessor.componentType);

          if (accessor.components != 1)
          {
            std::printf(
                "Index accessor isn't SCALAR\n");

            return {};
          }

          indices.reserve(
              indices.size() +
              accessor.count);

          for (size_t i = 0;
               i < accessor.count;
               ++i)
          {
            const uint8_t *element =
                accessor.data +
                i * accessor.stride;

            uint32_t index =
                ReadComponentAsIndex(
                    element,
                    accessor.componentType);

            indices.push_back(
                static_cast<uint32_t>(
                    initialVertex) +
                index);
          }

          newSurface.count =
              static_cast<uint32_t>(
                  accessor.count);

          (void)componentSize;
        }

        newmesh.surfaces.push_back(
            newSurface);
      }

      // ----------------------------------------------------
      // Display normals as colours
      // ----------------------------------------------------

      constexpr bool OverrideColors = true;

      if (OverrideColors)
      {
        for (Vertex &vertex : vertices)
        {
          vertex.col[0] =
              vertex.normal[0];

          vertex.col[1] =
              vertex.normal[1];

          vertex.col[2] =
              vertex.normal[2];

          vertex.col[3] = 1.0f;
        }
      }

      newmesh.mesh_buffers =
          renderer->MeshUpload(
              indices,
              vertices);

      meshes.emplace_back(
          std::make_shared<MeshAsset>(
              std::move(newmesh)));
    }

    return meshes;
  }
};