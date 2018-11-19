#include <shader_cross/shader_cross.h>

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/ResourceLimits.h>

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
    case Stage::Vertex:
        return EShLangVertex;
    case Stage::Fragment:
        return EShLangFragment;
    case Stage::Geometry:
        return EShLangGeometry;
    case Stage::Compute:
        return EShLangCompute;
    }
    return EShLangCount;
}

void GLSLAST::TShaderDeleter::operator()(glslang::TShader* shader) {
    delete shader;
}

bool GLSLAST::Parse(const char* data, std::size_t size, const GLSLOptions& opts, std::string* log) {
    shader.reset(new glslang::TShader(stageToEShLang(opts.Stage)));
    const char* shaderStrings[] = { data };
    int shaderStringLens[] = { int(size) };
    shader->setStringsWithLengths(shaderStrings, shaderStringLens, 1);

    if (opts.EntryPoint) {
        shader->setEntryPoint(opts.EntryPoint);
    }

    bool parsed = shader->parse(&glslang::DefaultTBuiltInResource, opts.Version, opts.ForwardCompatible, EShMsgDefault);
    if (log) {
        log->append(shader->getInfoLog());
    }
    //glslang::TProgram program;
    //program.addShader(&shader);
    //program.link(messages);
    //program.mapIO();
    return parsed;
}

bool GLSLAST::Parse(const std::string& s, const GLSLOptions& opts, std::string* log) {
    return this->Parse(s.data(), s.size(), opts, log);
}

bool GLSLAST::ToSPIRV(std::vector<std::uint32_t>* spirv, const SPIRVOptions& opts, std::string* log) const {
    glslang::TIntermediate* intermediate = shader->getIntermediate();
    glslang::SpvVersion spvVersion = intermediate->getSpv();
    spvVersion.spv = unsigned int(opts.Version);
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

bool GLSLAST::ToSPIRVIR(SPIRVIR* spirvIR, const SPIRVOptions& opts, std::string* log) const {
    std::vector<std::uint32_t> spirv;
    if (!this->ToSPIRV(&spirv, opts, log)) {
        return false;
    }
    return spirvIR->Parse(spirv, log);
}

bool GLSLAST::ToHLSL(std::string* hlsl, const HLSLOptions& opts, std::string* log) const {
    SPIRVIR spirvIR;
    if (!this->ToSPIRVIR(&spirvIR, SPIRVOptions{}, log)) {
        return false;
    }
    return spirvIR.ToHLSL(hlsl, opts, log);
}

bool GLSLAST::ToMSL(std::string* msl, const MSLOptions& opts, std::string* log) const {
    SPIRVIR spirvIR;
    if (!this->ToSPIRVIR(&spirvIR, SPIRVOptions{}, log)) {
        return false;
    }
    return spirvIR.ToMSL(msl, opts, log);
}

} // namespace shader_cross
