#ifndef SHADER_CROSS_H
#define SHADER_CROSS_H

#include <cstdint>
#include <vector>
#include <string>

namespace glslang {
class TShader;
} // namespace glslang

namespace spirv_cross {
class Parser;
} // namespace spirv_cross

namespace shader_cross {

enum class Stage {
    Vertex = 0,
    Fragment,
    Geometry,
    Compute,
};

struct GLSLOptions {
    Stage Stage = Stage::Vertex;
    const char* EntryPoint = nullptr;
    int Version = 450;
    bool ForwardCompatible = false;
};

enum class SPIRVVersion {
    V1_0 = (1 << 16),
    V1_1 = (1 << 16) | (1 << 8),
    V1_2 = (1 << 16) | (2 << 8),
    V1_3 = (1 << 16) | (3 << 8),
    //V1_4 = (1 << 16) | (4 << 8),
};

struct SPIRVOptions {
    SPIRVVersion Version = SPIRVVersion::V1_3;
};

struct HLSLOptions {
    int Model = 60;
};

enum class MSLPlatform {
    IOS = 0,
    OSX,
};

//TODO
struct MSLOptions {
    MSLPlatform Platform = MSLPlatform::OSX;
    int Major = 1;
    int Minor = 2;
    int Patch = 0;
};

class SPIRVIR;

class GLSLAST {
public:
    bool Parse(const char* data, std::size_t size, const GLSLOptions& opts, std::string* log);

    bool Parse(const std::string& s, const GLSLOptions& opts, std::string* log);

    bool ToSPIRV(std::vector<std::uint32_t>* spirv, const SPIRVOptions& opts, std::string* log) const;

    bool ToSPIRVIR(SPIRVIR* spirvIR, const SPIRVOptions& opts, std::string* log) const;

    bool ToHLSL(std::string* hlsl, const HLSLOptions& opts, std::string* log) const;

    bool ToMSL(std::string* msl, const MSLOptions& opts, std::string* log) const;

private:
    struct TShaderDeleter {
        void operator()(glslang::TShader* shader);
    };
    std::unique_ptr<glslang::TShader, TShaderDeleter> shader;
};

class SPIRVIR {
public:
    bool Parse(const std::uint32_t* data, std::size_t size, std::string* log);

    bool Parse(const std::vector<std::uint32_t>& spirv, std::string* log);

    bool ToGLSL(std::string* glsl, const GLSLOptions& opts, std::string* log) const;

    bool ToGLSLAST(GLSLAST* glslAST, const GLSLOptions& opts, std::string* log) const;

    bool ToHLSL(std::string* hlsl, const HLSLOptions& opts, std::string* log) const;

    bool ToMSL(std::string* msl, const MSLOptions& opts, std::string* log) const;

private:
    struct ParserDeleter {
        void operator()(spirv_cross::Parser* parser);
    };
    std::unique_ptr<spirv_cross::Parser, ParserDeleter> parser;
};

} // shader_cross

#endif // SHADER_CROSS_H
