#include <shader_cross/shader_cross.hpp>

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/ResourceLimits.h>
#include <StandAlone/DirStackFileIncluder.h>

namespace shader_cross {

struct GlslangInitializer {
    GlslangInitializer() {
        glslang::InitializeProcess();
    }
    ~GlslangInitializer() {
        glslang::FinalizeProcess();
    }
};

static GlslangInitializer glslangInitializer;

static inline EShLanguage stageToEShLang(Stage stage) {
    switch (stage) {
    case Stage::None:
        return EShLangCount;
    case Stage::Vertex:
        return EShLangVertex;
    case Stage::TessControl:
        return EShLangTessControl;
    case Stage::TessEvaluation:
        return EShLangTessEvaluation;
    case Stage::Fragment:
        return EShLangFragment;
    case Stage::Geometry:
        return EShLangGeometry;
    case Stage::Compute:
        return EShLangCompute;
    }
}

void GLSLAST::TShaderDeleter::operator()(glslang::TShader* shader) {
    delete shader;
}

bool GLSLAST::Parse(const char* data, std::size_t size, const Options& opts, std::string* log) {
    shader.reset(new glslang::TShader(stageToEShLang(opts.Stage)));
    const char* shaderStrings[] = { data };
    int shaderStringLens[] = { int(size) };
    const char* names[] = { opts.Filename };
    shader->setStringsWithLengthsAndNames(shaderStrings, shaderStringLens, names, 1);
    if (opts.EntryPoint) {
        shader->setEntryPoint(opts.EntryPoint);
    }
    shader->setPreamble("#extension GL_GOOGLE_include_directive : require\n");

    const TBuiltInResource* resources = &glslang::DefaultTBuiltInResource;
    EShMessages messages = EShMsgDefault;

    DirStackFileIncluder includer;
    for (auto& dir : opts.IncludeDirectories) {
        includer.pushExternalLocalDirectory(dir);
    }
    bool parsed = shader->parse(resources, opts.DefaultVersion, false, messages, includer);
    if (log) {
        log->append(shader->getInfoLog());
    }
    //glslang::TProgram program;
    //program.addShader(&shader);
    //program.link(messages);
    //program.mapIO();
    return parsed;
}

bool GLSLAST::Parse(const std::string& s, const Options& opts, std::string* log) {
    return this->Parse(s.data(), s.size(), opts, log);
}

unsigned int toSpvVersion(int version) {
    return (unsigned int)((version/10) << 16) | ((version % 10) << 8);
}

bool GLSLAST::ToSPIRV(std::vector<std::uint32_t>* spirv, const SPIRVOptions& opts, std::string* log) const {
    glslang::TIntermediate* intermediate = shader->getIntermediate();
    glslang::SpvVersion spvVersion = intermediate->getSpv();
    spvVersion.spv = toSpvVersion(opts.Version);
    intermediate->setSpv(spvVersion);

    spv::SpvBuildLogger logger;
    glslang::SpvOptions spvOptions;
    spvOptions.generateDebugInfo = true;
    spvOptions.disableOptimizer = false;
    spvOptions.optimizeSize = false;
    spvOptions.disassemble = false;
    spvOptions.validate = true;
    glslang::GlslangToSpv(*intermediate, *spirv, &logger, &spvOptions);
    if (log) {
        log->append(logger.getAllMessages());
    }
    return true;
}

} // namespace shader_cross
