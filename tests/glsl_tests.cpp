#include <gtest/gtest.h>
#include <shader_cross/shader_cross.hpp>
#include <string>

class GLSLTest : public testing::Test {
protected:
    void SetUp() override;
    void TearDown() override {}

    shader_cross::GLSLAST glslAST;
    shader_cross::SPIRVIR spirvIR;
};

void GLSLTest::SetUp() {
    std::string transformVSglsl = R"(#version 450
out gl_PerVertex {
    vec4 gl_Position;
};
layout(std140) uniform type_cbVS {
    layout(row_major) mat4 wvp;
} cbVS;
layout(location = 0) in vec4 in_var_POSITION;
void main() {
    gl_Position = cbVS.wvp * in_var_POSITION;
}
)";
    {
        shader_cross::GLSLAST::Options opts;
        opts.Stage = shader_cross::Stage::Vertex;
        std::string log;
        ASSERT_TRUE(glslAST.Parse({ transformVSglsl }, opts, &log)) << log;
    }

    std::vector<std::uint32_t> spirv;
    {
        shader_cross::SPIRVOptions opts;
        opts.Version = 13;
        std::string log;
        ASSERT_TRUE(glslAST.ToSPIRV(&spirv, opts, &log)) << log;
    }

    {
        std::string log;
        ASSERT_TRUE(spirvIR.Parse(spirv, &log)) << log;
    }
}

TEST_F(GLSLTest, ToGLSL) {
    std::string glsl;
    shader_cross::GLSLOptions opts;
    opts.Version = 450;
    std::string log;
    ASSERT_TRUE(spirvIR.ToGLSL(&glsl, opts, &log)) << log;
}

TEST_F(GLSLTest, ToESSL) {
    std::string essl;
    shader_cross::ESSLOptions opts;
    opts.Version = 320;
    std::string log;
    ASSERT_TRUE(spirvIR.ToESSL(&essl, opts, &log)) << log;
}

TEST_F(GLSLTest, ToHLSL) {
    std::string hlsl;
    shader_cross::HLSLOptions opts;
    opts.Model = 60;
    std::string log;
    ASSERT_TRUE(spirvIR.ToHLSL(&hlsl, opts, &log)) << log;
}

TEST_F(GLSLTest, ToMSL) {
    std::string msl;
    shader_cross::MSLOptions opts;
    std::string log;
    ASSERT_TRUE(spirvIR.ToMSL(&msl, opts, &log)) << log;
}
