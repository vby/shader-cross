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
    None = 0,
    Vertex,
    TessControl,
    TessEvaluation,
    Geometry,
    Fragment,
    Compute,
};

struct GLSLOptions {
    int Version = 450;
};

struct ESSLOptions {
    int Version = 320;
};

struct SPIRVOptions {
    int Version = 13;
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
    int Version = 120;
};

class SPIRVIR;

class GLSLAST {
public:
    struct Options {
        Stage Stage = Stage::None;
        int DefaultVersion = 450;
        std::string EntryPoint = "main";
        bool EnableInclude = true;
        std::vector<std::string> Names;
        std::vector<std::string> IncludeDirectories;
    };

    bool Parse(const char** glsls, const std::size_t* sizes, int num, const Options& opts, std::string* log);

    bool Parse(const std::string* glsls, int num, const Options& opts, std::string* log);

    bool Parse(const std::vector<std::string>& glsls, const Options& opts, std::string* log);

    bool ToSPIRV(std::vector<std::uint32_t>* spirv, const SPIRVOptions& opts, std::string* log) const;

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

    bool ToESSL(std::string* essl, const ESSLOptions& opts, std::string* log) const;

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
