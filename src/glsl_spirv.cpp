#include <shader_cross/shader_cross.hpp>

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/ResourceLimits.h>
#include <StandAlone/DirStackFileIncluder.h>

#include <algorithm>

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
    return EShLangCount;
}

void GLSLAST::TShaderDeleter::operator()(glslang::TShader* shader) {
    delete shader;
}

std::vector<const char*> toGlslangStrings(const std::string* strings, std::size_t size) {
    std::vector<const char*> cStrings(strings->size());
    for (std::size_t i = 0; i < strings->size(); ++i) {
        cStrings[i] = strings[i].c_str();
    }
    return cStrings;
}

std::string defaultFilenameOf(int index) {
    std::string name;
    name.append("<");
    name.append(std::to_string(index));
    name.append(">");
    return name;
}

bool GLSLAST::Parse(const char** glsls, const std::size_t* sizes, int num, const Options& opts, std::string* log) {
    const TBuiltInResource* resources = &glslang::DefaultTBuiltInResource;
    EShMessages messages = EShMsgDefault;
    shader.reset(new glslang::TShader(stageToEShLang(opts.Stage)));
    // Sources
    std::vector<int> lens(num);
    for (int i = 0; i < num; ++i) {
        lens[i] = int(sizes[i]);
    }
    std::vector<std::string> names = opts.Names;
    names.resize(num);
    for (int i = int(opts.Names.size()); i < num; ++i) {
        names[i] = defaultFilenameOf(i);
    }
    std::vector<const char*> cNames = toGlslangStrings(names.data(), names.size());
    shader->setStringsWithLengthsAndNames(glsls, lens.data(), cNames.data(), num);
    // EntryPoint
    if (!opts.EntryPoint.empty()) {
        shader->setEntryPoint(opts.EntryPoint.c_str());
    }
    bool parsed = false;
    // Include & Parse
    if (opts.EnableInclude) {
        shader->setPreamble("#extension GL_GOOGLE_include_directive : enable\n");
        DirStackFileIncluder includer;
        for (auto& dir : opts.IncludeDirectories) {
            includer.pushExternalLocalDirectory(dir);
        }
        parsed = shader->parse(resources, opts.DefaultVersion, false, messages, includer);
    } else {
        parsed = shader->parse(resources, opts.DefaultVersion, false, messages);
    }
    if (log) {
        log->append(shader->getInfoLog());
        log->append(shader->getInfoDebugLog());
    }
    //glslang::TProgram program;
    //program.addShader(shader.get());
    //program.link(messages);
    //program.mapIO();
    return parsed;
}

bool GLSLAST::Parse(const std::string* glsls, int num, const Options& opts, std::string* log) {
    std::vector<const char*> cGlsls = toGlslangStrings(glsls, num);
    std::vector<std::size_t> sizes(num);
    for (int i = 0; i < num; ++i) {
        sizes[i] = glsls[i].size();
    }
    return this->Parse(cGlsls.data(), sizes.data(), num, opts, log);
}

bool GLSLAST::Parse(const std::vector<std::string>& glsls, const Options& opts, std::string* log) {
    return this->Parse(glsls.data(), int(glsls.size()), opts, log);
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
