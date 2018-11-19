#include "spirv.hpp"
#include <spirv_hlsl.hpp>

namespace shader_cross {

bool SPIRVIR::ToHLSL(std::string* hlsl, const HLSLOptions& opts, std::string* log) const {
    auto initFn = [&opts](spirv_cross::CompilerHLSL& compiler) {
        spirv_cross::CompilerHLSL::Options hlslOpts = compiler.get_hlsl_options();
        hlslOpts.shader_model = opts.Model;
        compiler.set_hlsl_options(hlslOpts);
    };
    return spirvCompile<spirv_cross::CompilerHLSL>(initFn, this->parser->get_parsed_ir(), hlsl, log);
}

} // namespace shader_cross
