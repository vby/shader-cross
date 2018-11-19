#include <shader_cross/shader_cross.h>
#include <string>
#include <fstream>
#include <iostream>

std::string loadFileToString(const char* filename) {
    std::ifstream fs(filename);
    fs.exceptions(std::ios::failbit);
    fs.seekg(0, std::ios::end);
    std::size_t size = std::size_t(fs.tellg());
    fs.seekg(0, std::ios::beg);
    std::string content;
    content.resize(size);
    fs.read(&content[0], size);
    return content;
}

//TODO
int main(int argc, char** argv) {
    std::string glsl = loadFileToString("test.vert");

    std::cout << "GLSL:\n" << glsl << std::endl;

    shader_cross::GLSLOptions glslOpts;
    glslOpts.Stage = shader_cross::Stage::Vertex;
    glslOpts.Version = 450;
    glslOpts.EntryPoint = "main";
    std::string log;
    shader_cross::GLSLAST glslAST;
    glslAST.Parse(glsl, glslOpts, &log);

    std::vector<std::uint32_t> spirv;
    {
        shader_cross::SPIRVOptions opts;
        opts.Version = shader_cross::SPIRVVersion::V1_3;
        std::string log;
        bool ok = glslAST.ToSPIRV(&spirv, opts, &log);
        std::cout << "GLSL->SPIRV: " << std::boolalpha << ok << std::endl << "Log: " << log << std::endl << "SPIRV:\n" << spirv.size() << std::endl;
    }

    std::cout << std::endl << std::endl;

    shader_cross::SPIRVIR spirvIR;
    spirvIR.Parse(spirv.data(), spirv.size(), &log);

    {
        std::string glsl;
        shader_cross::GLSLOptions opts;
        opts.Version = 450;
        std::string log;
        bool ok = spirvIR.ToGLSL(&glsl, opts, &log);
        std::cout << "SPIRV->GLSL: " << std::boolalpha << ok << std::endl << "Log: " << log << std::endl << "GLSL:\n" << glsl << std::endl;
    }

    {
        std::string hlsl;
        shader_cross::HLSLOptions opts;
        opts.Model = 60;
        std::string log;
        bool ok = spirvIR.ToHLSL(&hlsl, opts, &log);
        std::cout << "SPIRV->HLSL: " << std::boolalpha << ok << std::endl << "Log: " << log << std::endl << "HLSL:\n" << hlsl << std::endl;
    }

    {
        std::string msl;
        shader_cross::MSLOptions opts;
        std::string log;
        bool ok = spirvIR.ToMSL(&msl, opts, &log);
        std::cout << "SPIRV->MSL: " << std::boolalpha << ok << std::endl << "Log: " << log << std::endl << "MSL:\n" << msl << std::endl;
    }

    return 0;
}
