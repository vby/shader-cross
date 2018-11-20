#include "spirv.hpp"
#include <spirv_msl.hpp>

namespace shader_cross {

bool SPIRVIR::ToMSL(std::string* msl, const MSLOptions& opts, std::string* log) const {
    auto initFn = [&opts](spirv_cross::CompilerMSL& compiler) {
        spirv_cross::CompilerMSL::Options mslOpts = compiler.get_msl_options();
        mslOpts.platform = spirv_cross::CompilerMSL::Options::Platform(opts.Platform);
        mslOpts.set_msl_version(opts.Version/100, (opts.Version/10)%10, opts.Version%10);
        compiler.set_msl_options(mslOpts);
    };
    return spirvCompile<spirv_cross::CompilerMSL>(initFn, this->parser->get_parsed_ir(), msl, log);
}

} // namespace shader_cross
