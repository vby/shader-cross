#ifndef SHADER_CROSS_SPIRV_H
#define SHADER_CROSS_SPIRV_H

#include <shader_cross/shader_cross.hpp>
#include <spirv_common.hpp>
#include <spirv_parser.hpp>

namespace shader_cross {

template <class Compiler, class InitFn>
bool spirvCompile(InitFn& initFn, const spirv_cross::ParsedIR& spirvIR, std::string* out, std::string* log) {
    try {
        Compiler compiler(spirvIR);
        initFn(compiler);
        *out = compiler.compile();
    } catch (const spirv_cross::CompilerError& e) {
        if (log) {
            log->append(e.what());
        }
        return false;
    }
    return true;
}

} // namespace shader_cross

#endif // SHADER_CROSS_SPIRV_H
