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

bool SPIRVIR::ToESSL(std::string* glsl, const ESSLOptions& opts, std::string* log) const {
    auto initFn = [&opts](spirv_cross::CompilerGLSL& compiler) {
        spirv_cross::CompilerGLSL::Options glslOpts = compiler.get_common_options();
        glslOpts.es = true;
        glslOpts.version = opts.Version;
        compiler.set_common_options(glslOpts);
    };
    return spirvCompile<spirv_cross::CompilerGLSL>(initFn, this->parser->get_parsed_ir(), glsl, log);
}

} // namespace shader_cross
