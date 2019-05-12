#include <shader_cross/shader_cross.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>

#ifdef _MSC_VER
__pragma(warning(push))
__pragma(warning(disable: 4819))
__pragma(warning(disable: 4018))
__pragma(warning(disable: 4267))
#include <cxxopts.hpp>
__pragma(warning(pop))
#else
#include <cxxopts.hpp>
#endif // _MSC_VER

static cxxopts::Options options("shaderx", "Shader languages cross compiler");

void initOptions() {
    options.parse_positional("inputs");
    options.positional_help("file...");
    auto addOpt = options.add_options();
    addOpt("inputs", "", cxxopts::value<std::vector<std::string>>());
    addOpt("S,stage", "Shader stage: vs, tc, te, fs, gs, cs", cxxopts::value<std::string>()->default_value(""), "<stage>");
    addOpt("O,output", "Write output to <file>", cxxopts::value<std::string>()->default_value("-"), "<file>");
    addOpt("F,from", "From language: glsl, spirv", cxxopts::value<std::string>()->default_value("glsl"), "<lang>");
    addOpt("T,target", "Target language: spirv, glsl, essl, hlsl, msl", cxxopts::value<std::string>()->default_value("spirv"), "<lang>");
    addOpt("V,version", "Target language version", cxxopts::value<std::string>()->default_value(""), "<ver>");
    addOpt("I,include", "Add directory to include search path", cxxopts::value<std::vector<std::string>>(), "<dir>");
    addOpt("h,help", "Display available options");
}

int showHelp(const char* reason = nullptr) {
    if (reason) {
        std::cout << reason << std::endl;
    }
    std::cout << options.help() << std::endl;
    return 0;
}

int printError(const char* err) {
    std::cerr << err << std::endl;
    return 1;
}

int printError(const std::string& err) {
    std::cerr << err << std::endl;
    return 1;
}

int printOpenFileError(const std::string& filename) {
    std::cerr << "Can't open file '" << filename << "'" << std::endl;
    return 1;
}

void printLog(const std::string& log) {
    if (!log.empty()) {
        std::cout << log << std::endl;
    }
}

shader_cross::Stage toStage(const std::string& stage) {
    if (stage == "vs") {
        return shader_cross::Stage::Vertex;
    } else if (stage == "tc") {
        return shader_cross::Stage::TessControl;
    } else if (stage == "te") {
        return shader_cross::Stage::TessEvaluation;
    } else if (stage == "gs") {
        return shader_cross::Stage::Geometry;
    } else if (stage == "fs") {
        return shader_cross::Stage::Fragment;
    } else if (stage == "cs") {
        return shader_cross::Stage::Compute;
    }
    return shader_cross::Stage::None;
}

shader_cross::Stage extToStage(const std::string& ext) {
    if (ext == "vert") {
        return shader_cross::Stage::Vertex;
    } else if (ext == "tesc") {
        return shader_cross::Stage::TessControl;
    } else if (ext == "tese") {
        return shader_cross::Stage::TessEvaluation;
    } else if (ext == "geom") {
        return shader_cross::Stage::Geometry;
    } else if (ext == "frag") {
        return shader_cross::Stage::Fragment;
    } else if (ext == "comp") {
        return shader_cross::Stage::Compute;
    }
    return toStage(ext);
}

shader_cross::Stage filenamesToStage(const std::vector<std::string>& filenames) {
    for (auto& filename : filenames) {
        auto pos = filename.rfind('.');
        if (pos != std::string::npos) {
            auto stage = extToStage(filename.substr(pos + 1));
            if (stage != shader_cross::Stage::None) {
                return stage;
            }
        }
    }
    return shader_cross::Stage::None;
}

shader_cross::Stage toStage(const std::string& stage, const std::vector<std::string>& filenames) {
    if (stage == "") {
        return filenamesToStage(filenames);
    }
    return toStage(stage);
}

int toVersion(const std::string& ver) {
    if (ver.empty()) {
        return 0;
    }
    return std::atoi(ver.c_str());
}

std::string readToString(std::istream& is) {
    std::string result;
    std::vector<char> buf(1024);
    while (!is.eof()) {
        is.read(buf.data(), buf.size());
        result.append(buf.data(), std::size_t(is.gcount()));
    }
    return result;
}

std::size_t readToStrings(std::vector<std::string>* outStrings, const std::vector<std::string>& filenames) {
    for (std::size_t i = 0; i < filenames.size(); ++i) {
        std::ifstream ifs(filenames[i]);
        if (!ifs) {
            return i;
        }
        outStrings->emplace_back(readToString(ifs));
    }
    return filenames.size();
}

std::string joinStrings(const std::vector<std::string>& strings) {
    std::string result;
    for (auto& s : strings) {
        result.append(s);
    }
    return result;
}

void writeSPIRVText(std::ostream* os, const std::uint32_t* spirv, std::size_t size) {
    for (std::size_t i = 0; i < size; ++i) {
        *os << std::hex << std::setfill('0') << std::setw(8) << spirv[i] << " ";
        if (i % 16 == 15) {
            *os << std::endl;
        }
    }
    *os << std::endl;
}

void writeSPIRV(std::ostream* os, const std::uint32_t* spirv, std::size_t size) {
    if (os == &std::cout) {
        return writeSPIRVText(os, spirv, size);
    }
    os->write(reinterpret_cast<const char*>(spirv), size*4);
}

void writeSPIRVString(std::ostream* os, const std::string& str) {
    writeSPIRV(os, reinterpret_cast<const std::uint32_t*>(str.data()), str.size()/4);
}

void writeString(std::ostream* os, const std::string& str) {
    os->write(str.data(), str.size());
}

int main(int argc, char** argv) {
    initOptions();

    std::string stageStr;
    std::vector<std::string> inputs;
    std::string output;
    std::string from;
    std::string target;
    int version;
    std::vector<std::string> includes;

    try {
        auto opts = options.parse(argc, argv);
        if (opts.count("help")) {
            return showHelp();
        }
        if (opts.count("inputs")) {
            inputs = opts["inputs"].as<std::vector<std::string>>();
        }
        stageStr = opts["stage"].as<std::string>();
        output = opts["output"].as<std::string>();
        from = opts["from"].as<std::string>();
        target = opts["target"].as<std::string>();
        version = toVersion(opts["version"].as<std::string>());
        if (opts.count("include")) {
            includes = opts["include"].as<std::vector<std::string>>();
        }
    } catch (const cxxopts::missing_argument_exception& e) {
        return printError(e.what());
    } catch (const cxxopts::option_not_exists_exception& e) {
        return printError(e.what());
    }

    shader_cross::Stage stage = shader_cross::Stage::None;
    if (from == "glsl") {
        stage = toStage(stageStr, inputs);
        if (stage == shader_cross::Stage::None) {
            if (!stageStr.empty()) {
                std::cerr << "Unknown stage '" << stageStr << "'" << std::endl;
                return 1;
            }
            return printError("Unknown stage");
        }
    }
    
    bool inputFromStdin = false;
    std::vector<std::string> inputContents;
    if (inputs.empty() || inputs[0] == "-") {
        inputContents.emplace_back(readToString(std::cin));
        inputFromStdin = true;
    } else {
        auto pos = readToStrings(&inputContents, inputs);
        if (pos != inputs.size()) {
            return printOpenFileError(inputs[pos]);
        }
    }

    std::ostream* outputStream;
    std::ofstream ofs;
    if (output == "-") {
        outputStream = &std::cout;
    } else {
        ofs.open(output);
        if (!ofs) {
            return printOpenFileError(output);
        }
        outputStream = &ofs;
    }

    shader_cross::GLSLAST glslAST;
    shader_cross::SPIRVIR spirvIR;

    if (from == "glsl") {
        {
            shader_cross::GLSLAST::Options opts;
            opts.Stage = stage;
            if (!inputFromStdin) {
                opts.Names = inputs;
            }
            opts.IncludeDirectories = includes;
            std::string log;
            if (!glslAST.Parse(inputContents, opts, &log)) {
                return printError(log);
            }
            printLog(log);
        }
        std::vector<std::uint32_t> spirv;
        {
            shader_cross::SPIRVOptions opts;
            if (target == "spirv" && version > 0) {
                opts.Version = version;
            }
            std::string log;
            if (!glslAST.ToSPIRV(&spirv, opts, &log)) {
                return printError(log);
            }
            printLog(log);
        }
        if (target == "spirv") {
            writeSPIRV(outputStream, spirv.data(), spirv.size());
            return 0;
        }
        std::string log;
        if (!spirvIR.Parse(spirv, &log)) {
            return printError(log);
        }
        printLog(log);
    } else if (from == "spirv") {
        auto inputContent = joinStrings(inputContents);
        if (target == "spirv") {
            writeSPIRVString(outputStream, inputContent);
            return 0;
        }
        std::string log;
        if (!spirvIR.Parse(reinterpret_cast<const std::uint32_t*>(inputContents.data()), inputContents.size(), &log)) {
            return printError(log);
        }
        printLog(log);
    } else {
        std::cerr << "Unsupported from language " << from << std::endl;
        return 1;
    }

    if (target == "glsl") {
        std::string glsl;
        shader_cross::GLSLOptions opts;
        if (version > 0) {
            opts.Version = version;
        }
        std::string log;
        if (!spirvIR.ToGLSL(&glsl, opts, &log)) {
            return printError(log);
        }
        printLog(log);
        writeString(outputStream, glsl);
    } else if (target == "essl") {
        std::string essl;
        shader_cross::ESSLOptions opts;
        if (version > 0) {
            opts.Version = version;
        }
        std::string log;
        if (!spirvIR.ToESSL(&essl, opts, &log)) {
            return printError(log);
        }
        printLog(log);
        writeString(outputStream, essl);
    } else if (target == "hlsl") {
        std::string hlsl;
        shader_cross::HLSLOptions opts;
        if (version > 0) {
            opts.Model = version;
        }
        std::string log;
        if (!spirvIR.ToHLSL(&hlsl, opts, &log)) {
            return printError(log);
        }
        printLog(log);
        writeString(outputStream, hlsl);
    } else if (target == "msl") {
        std::string msl;
        shader_cross::MSLOptions opts;
        if (version > 0) {
            opts.Version = version;
        }
        std::string log;
        if (!spirvIR.ToMSL(&msl, opts, &log)) {
            return printError(log);
        }
        printLog(log);
        writeString(outputStream, msl);
    } else {
        std::cerr << "Unsupported target language " << target << std::endl;
        return 1;
    }

    return 0;
}

