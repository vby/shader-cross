#include "spirv.hpp"

namespace shader_cross {

void SPIRVIR::ParserDeleter::operator()(spirv_cross::Parser* parser) {
    delete parser;
}

bool SPIRVIR::Parse(const std::uint32_t* data, std::size_t size, std::string* log) {
    try {
        this->parser.reset(new spirv_cross::Parser(data, size));
        this->parser->parse();
    } catch (const spirv_cross::CompilerError& e) {
        if (log) {
            log->append(e.what());
        }
        return false;
    }
    return true;
}

bool SPIRVIR::Parse(const std::vector<std::uint32_t>& spirv, std::string* log) {
    return this->Parse(spirv.data(), spirv.size(), log);
}

} // namespace shader_cross
