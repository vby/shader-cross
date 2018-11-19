#include "spirv.hpp"
#include <spirv_glsl.hpp>

namespace shader_cross {

bool SPIRVIR::ToGLSL(std::string* glsl, const GLSLOptions& opts, std::string* log) const {
    auto initFn = [&opts](spirv_cross::CompilerGLSL& compiler) {
        spirv_cross::CompilerGLSL::Options glslOpts = compiler.get_common_options();
        glslOpts.version = opts.Version;
        compiler.set_common_options(glslOpts);
    };
    return spirvCompile<spirv_cross::CompilerGLSL>(initFn, this->parser->get_parsed_ir(), glsl, log);
}

bool SPIRVIR::ToGLSLAST(GLSLAST* glslAST, const GLSLOptions& opts, std::string* log) const {
    std::string glsl;
    if (!this->ToGLSL(&glsl, opts, log)) {
        return false;
    }
    return glslAST->Parse(glsl, opts, log);
}

} // namespace shader_cross
