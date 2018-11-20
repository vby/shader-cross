#include <shader_cross/shader_cross.hpp>
#include <cxxopts.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>

static cxxopts::Options options("shaderx", "Shader languages cross compiler");

void initOptions() {
    auto addOpt = options.add_options();
    addOpt("S,stage", "Shader stage: vs, fs, gs, cs", cxxopts::value<std::string>()->default_value(""), "<stage>");
    addOpt("I,input", "Read input from <file>", cxxopts::value<std::string>()->default_value("-"), "<file>");
    addOpt("O,output", "Write output to <file>", cxxopts::value<std::string>()->default_value("-"), "<file>");
    addOpt("F,from", "From language: glsl, spirv", cxxopts::value<std::string>()->default_value("glsl"), "<lang>");
    addOpt("T,target", "Target language: spirv, glsl, essl, hlsl, msl", cxxopts::value<std::string>()->default_value("spirv"), "<lang>");
    addOpt("V,version", "Target language version", cxxopts::value<std::string>()->default_value(""), "<ver>");
    //addOpt("I,include", "Add directory to include search path", cxxopts::value<std::vector<std::string>>(), "<dir>");
    addOpt("h,help", "Display available options");
}

int showHelp(const char* reason = nullptr) {
    if (reason) {
        std::cout << reason << std::endl;
    }
    std::cout << options.help() << std::endl;
    return 0;
}

int printError(const std::string& err) {
    std::cerr << err << std::endl;
    return 1;
}

void printLog(const std::string& log) {
    if (!log.empty()) {
        std::cout << log << std::endl;
    }
}

shader_cross::Stage extToStage(const std::string& ext) {
    if (ext == "vert" || ext == "vs") {
        return shader_cross::Stage::Vertex;
    } else if (ext == "tesc" || ext == "tcs") {
        return shader_cross::Stage::TessControl;
    } else if (ext == "tese" || ext == "tes") {
        return shader_cross::Stage::TessEvaluation;
    } else if (ext == "geom" || ext == "gs") {
        return shader_cross::Stage::Geometry;
    } else if (ext == "frag" || ext == "fs") {
        return shader_cross::Stage::Fragment;
    } else if (ext == "comp" || ext == "cs") {
        return shader_cross::Stage::Compute;
    }
    return shader_cross::Stage::None;
}

shader_cross::Stage toStage(const std::string& stage, const std::string& input) {
    if (stage == "") {
        auto pos = input.rfind('.');
        if (pos != std::string::npos) {
            return extToStage(input.substr(pos+1));
        }
        return shader_cross::Stage::None;
    } else if (stage == "vs") {
        return shader_cross::Stage::Vertex;
    } else if (stage == "fs") {
        return shader_cross::Stage::Fragment;
    } else if (stage == "gs") {
        return shader_cross::Stage::Geometry;
    } else if (stage == "cs") {
        return shader_cross::Stage::Compute;
    }
    return shader_cross::Stage::None;
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
        result.append(buf.data(), is.gcount());
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
    writeSPIRV(os, reinterpret_cast<const std::uint32_t*>(str.data()), str.size());
}

void writeString(std::ostream* os, const std::string& str) {
    os->write(str.data(), str.size());
}

int main(int argc, char** argv) {
    initOptions();
    auto opts = options.parse(argc, argv);
    if (opts.count("help")) {
        return showHelp();
    }

    std::string stageStr = opts["stage"].as<std::string>();
    std::string input = opts["input"].as<std::string>();
    std::string output = opts["output"].as<std::string>();
    std::string from = opts["from"].as<std::string>();
    std::string target = opts["target"].as<std::string>();
    std::string versionStr = opts["version"].as<std::string>();

    int version = toVersion(versionStr);

    std::string inputContent;
    if (input == "-") {
        inputContent = readToString(std::cin);
    } else {
        std::ifstream ifs(input);
        if (!ifs) {
            std::cerr << "Open file " << input << " error" << std::endl;
            return 1;
        }
        inputContent = readToString(ifs);
    }
    std::ostream* outputStream;
    std::ofstream ofs;
    if (output == "-") {
        outputStream = &std::cout;
    } else {
        ofs.open(output);
        if (!ofs) {
            std::cerr << "Open file " << output << " error" << std::endl;
            return 1;
        }
        outputStream = &ofs;
    }

    shader_cross::GLSLAST glslAST;
    shader_cross::SPIRVIR spirvIR;

    if (from == "glsl") {
        {
            shader_cross::Stage stage = toStage(stageStr, input);
            if (stage == shader_cross::Stage::None) {
                return printError("Unknown stage");
            }
            shader_cross::GLSLAST::Options opts;
            opts.Stage = stage;
            opts.Filename = input.c_str();
            std::string log;
            if (!glslAST.Parse(inputContent, opts, &log)) {
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
        if (target == "spirv") {
            writeSPIRVString(outputStream, inputContent);
            return 0;
        }
        std::string log;
        if (!spirvIR.Parse(reinterpret_cast<const std::uint32_t*>(inputContent.data()), inputContent.size(), &log)) {
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

