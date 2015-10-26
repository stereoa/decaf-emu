#include <fstream>
#include <spdlog/spdlog.h>
#include <Windows.h>

#include "gx2_texture.h"
#include "gx2_shaders.h"
#include "gpu/latte.h"
#include "gpu/hlsl/hlsl.h"
#include "gpu/hlsl/hlsl_generator.h"
#include "memory_translate.h"

static void
GX2CreateDumpDirectory()
{
   CreateDirectory(TEXT("dump"), NULL);
}

static std::string
GX2EnumAsString(GX2SurfaceDim::Dim dim)
{
   switch (dim) {
   case GX2SurfaceDim::Texture2D:
      return "Texture2D";
   case GX2SurfaceDim::Texture2DMSAA:
      return "Texture2DMSAA";
   case GX2SurfaceDim::Texture2DMSAAArray:
      return "Texture2DMSAAArray";
   default:
      return std::to_string(static_cast<int>(dim));
   }
}

static std::string
GX2EnumAsString(GX2SurfaceFormat::Format format)
{
   switch (format) {
   case GX2SurfaceFormat::UNORM_R8G8B8A8:
      return "R8G8B8A8";
   default:
      return std::to_string(static_cast<int>(format));
   }
}

static std::string
GX2EnumAsString(GX2AAMode::Mode mode)
{
   switch (mode) {
   case GX2AAMode::Mode1X:
      return "Mode1X";
   default:
      return std::to_string(static_cast<int>(mode));
   }
}

static std::string
GX2EnumAsString(GX2SurfaceUse::Use use)
{
   switch (use) {
   case GX2SurfaceUse::Texture:
      return "Texture";
   case GX2SurfaceUse::ColorBuffer:
      return "ColorBuffer";
   case GX2SurfaceUse::DepthBuffer:
      return "DepthBuffer";
   default:
      return std::to_string(static_cast<int>(use));
   }
}

static std::string
GX2EnumAsString(GX2TileMode::Mode mode)
{
   switch (mode) {
   case GX2TileMode::Default:
      return "Default";
   case GX2TileMode::LinearSpecial:
      return "LinearSpecial";
   default:
      return std::to_string(static_cast<int>(mode));
   }
}

static std::string
GX2EnumAsString(GX2ShaderMode::Mode mode)
{
   switch (mode) {
   case GX2ShaderMode::UniformRegister:
      return "UniformRegister";
   case GX2ShaderMode::UniformBlock:
      return "UniformBlock";
   case GX2ShaderMode::GeometryShader:
      return "GeometryShader";
   default:
      return std::to_string(static_cast<int>(mode));
   }
}

static std::string
GX2EnumAsString(GX2UniformType::Type type)
{
   switch (type) {
   case GX2UniformType::Int:
      return "Int";
   case GX2UniformType::Int2:
      return "Int2";
   case GX2UniformType::Int3:
      return "Int3";
   case GX2UniformType::Int4:
      return "Int4";
   case GX2UniformType::Float:
      return "Float";
   case GX2UniformType::Float2:
      return "Float2";
   case GX2UniformType::Float3:
      return "Float3";
   case GX2UniformType::Float4:
      return "Float4";
   case GX2UniformType::Matrix4x4:
      return "Matrix4x4";
   default:
      return std::to_string(static_cast<int>(type));
   }
}

static std::string
GX2PointerAsString(const void *pointer)
{
   fmt::MemoryWriter format;
   format.write("{:08X}", memory_untranslate(pointer));
   return format.str();
}

static void
GX2DumpData(const std::string &filename, const void *data, size_t size)
{
   auto file = std::ofstream { filename, std::ofstream::out | std::ofstream::binary };
   file.write(static_cast<const char *>(data), size);
}

static void
GX2DumpData(std::ofstream &file, const void *data, size_t size)
{
   file.write(reinterpret_cast<const char *>(data), size);
}

void
GX2DumpTexture(const GX2Texture *texture)
{
   GX2CreateDumpDirectory();

   // Write text dump of GX2Texture structure to texture_X.txt
   auto filename = "texture_" + GX2PointerAsString(texture);
   auto file = std::ofstream { "dump/" + filename + ".txt", std::ofstream::out };
   auto format = fmt::MemoryWriter {};

   format
      << "surface.dim = " << GX2EnumAsString(texture->surface.dim) << '\n'
      << "surface.width = " << texture->surface.width << '\n'
      << "surface.height = " << texture->surface.height << '\n'
      << "surface.depth = " << texture->surface.depth << '\n'
      << "surface.mipLevels = " << texture->surface.mipLevels << '\n'
      << "surface.format = " << GX2EnumAsString(texture->surface.format) << '\n'
      << "surface.aa = " << GX2EnumAsString(texture->surface.aa) << '\n'
      << "surface.use = " << GX2EnumAsString(texture->surface.use) << '\n'
      << "surface.resourceFlags = " << texture->surface.resourceFlags << '\n'
      << "surface.imageSize = " << texture->surface.imageSize << '\n'
      << "surface.image = " << GX2PointerAsString(texture->surface.image) << '\n'
      << "surface.mipmapSize = " << texture->surface.mipmapSize << '\n'
      << "surface.mipmaps = " << GX2PointerAsString(texture->surface.mipmaps) << '\n'
      << "surface.tileMode = " << GX2EnumAsString(texture->surface.tileMode) << '\n'
      << "surface.swizzle = " << texture->surface.swizzle << '\n'
      << "surface.alignment = " << texture->surface.alignment << '\n'
      << "surface.pitch = " << texture->surface.pitch << '\n'
      << "viewFirstMip = " << texture->viewFirstMip << '\n'
      << "viewNumMips = " << texture->viewNumMips << '\n'
      << "viewFirstSlice = " << texture->viewFirstSlice << '\n'
      << "viewNumSlices = " << texture->viewNumSlices << '\n';

   file << format.str();

   // Write binary dump of image data to texture_X.img
   if (texture->surface.imageSize) {
      GX2DumpData("dump/" + filename + ".img", texture->surface.image, texture->surface.imageSize);
   }

   // Write binary dump of mipmap data to texture_X.mip
   if (texture->surface.mipmapSize) {
      GX2DumpData("dump/" + filename + ".mip", texture->surface.mipmaps, texture->surface.mipmapSize);
   }

   // Write combined binary dump of GX2Texture + Image + Mipmap to texture_X.gx2tex
   auto binary = std::ofstream { "dump/" + filename + ".gx2tex", std::ofstream::out | std::ofstream::binary };
   GX2DumpData(binary, texture, sizeof(GX2Texture));
   GX2DumpData(binary, texture->surface.image, texture->surface.imageSize);
   GX2DumpData(binary, texture->surface.mipmaps, texture->surface.mipmapSize);
}

static void
GX2DumpShader(const std::string &filename, const std::string &info, uint8_t *data, size_t size)
{
   std::string output;

   // Write binary of shader data to shader_pixel_X.bin
   GX2DumpData("dump/" + filename + ".bin", data, size);

   // Write text of shader to shader_pixel_X.txt
   auto file = std::ofstream { "dump/" + filename + ".txt", std::ofstream::out };

   // Disassemble
   latte::disassemble(output, { data, size });

   file
      << info << std::endl
      << "Disassembly:" << std::endl
      << output << std::endl;

   output.clear();

   // Decompiled
   auto decompiled = latte::Shader { };
   latte::decode(decompiled, latte::Shader::Vertex, { data, size });
   hlsl::generateBody(decompiled, output);

   file
      << "Decompiled:" << std::endl
      << output << std::endl;
}

static void
formatUniformBlocks(fmt::MemoryWriter &out, uint32_t count, GX2UniformBlock *blocks)
{
   out << "  uniformBlockCount: " << count << "\n";

   for (auto i = 0u; i < count; ++i) {
      out << "    Block " << i << "\n"
         << "      name: " << blocks[i].name.get() << "\n"
         << "      offset: " << blocks[i].offset << "\n"
         << "      size: " << blocks[i].size << "\n";
   }
}

static void
formatUniformVars(fmt::MemoryWriter &out, uint32_t count, GX2UniformVar *vars)
{
   out << "  uniformVarCount: " << count << "\n";

   for (auto i = 0u; i < count; ++i) {
      out << "    Var " << i << "\n"
         << "      name: " << vars[i].name.get() << "\n"
         << "      type: " << GX2EnumAsString(vars[i].type) << "\n"
         << "      count: " << vars[i].count << "\n"
         << "      offset: " << vars[i].offset << "\n"
         << "      block: " << vars[i].block << "\n";
   }
}

void
GX2DumpShader(GX2PixelShader *shader)
{
   fmt::MemoryWriter out;
   out << "GX2PixelShader:\n"
      << "  size: " << shader->size << "\n"
      << "  mode: " << GX2EnumAsString(shader->mode) << "\n";

   formatUniformBlocks(out, shader->uniformBlockCount, shader->uniformBlocks);
   formatUniformVars(out, shader->uniformVarCount, shader->uniformVars);

   out << "  numUnk1: " << shader->numUnk1 << "\n";
   out << "  numUnk2: " << shader->numUnk2 << "\n";
   out << "  samplerVarCount: " << shader->samplerVarCount << "\n";
   out << "  unk3: " << shader->unk3 << "\n";
   out << "  unk4: " << shader->unk4 << "\n";
   out << "  unk5: " << shader->unk5 << "\n";
   out << "  unk6: " << shader->unk6 << "\n";
   GX2DumpShader("shader_pixel_" + GX2PointerAsString(shader), out.str(), shader->data, shader->size);
}

void
GX2DumpShader(GX2VertexShader *shader)
{
   fmt::MemoryWriter out;
   out << "GX2VertexShader:\n"
      << "  size: " << shader->size << "\n"
      << "  mode: " << GX2EnumAsString(shader->mode) << "\n";

   formatUniformBlocks(out, shader->uniformBlockCount, shader->uniformBlocks);
   formatUniformVars(out, shader->uniformVarCount, shader->uniformVars);

   out << "  numUnk1: " << shader->numUnk1 << "\n";
   out << "  numUnk2: " << shader->numUnk2 << "\n";
   out << "  samplerVarCount: " << shader->samplerVarCount << "\n";
   out << "  numUnk3: " << shader->numUnk3 << "\n";

   GX2DumpShader("shader_vertex_" + GX2PointerAsString(shader), out.str(), shader->data, shader->size);
}
