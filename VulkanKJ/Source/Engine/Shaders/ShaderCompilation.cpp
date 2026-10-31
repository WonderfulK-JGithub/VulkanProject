#include "ShaderCompilation.h"

#include <filesystem>
#include <fstream>
#include <shaderc/shaderc.hpp>

#include "Rendering/RenderStage.h"
#include "Util/Logging/Logging.h"

namespace Vulkan
{
    static std::unordered_map<Filepath,vk::ShaderStageFlagBits> localExtensionToShaderStage{
        {".vert",vk::ShaderStageFlagBits::eVertex},
        {".frag",vk::ShaderStageFlagBits::eFragment},
    };
    
    class GLSLIncluder : public shaderc::CompileOptions::IncluderInterface
    {
    public:
        shaderc_include_result* GetInclude(
            const char* aRequestedSource,
            shaderc_include_type aType,
            const char* aRequestingSource,
            size_t /* includeDepth */) override
        {
            namespace fs = std::filesystem;

            fs::path requestedPath = aRequestedSource;

            if (aType == shaderc_include_type_relative)
            {
                requestedPath = fs::path(aRequestingSource).parent_path()
                              / requestedPath;
            }

            requestedPath = fs::absolute(requestedPath).lexically_normal();

            auto* result = new shaderc_include_result{};
            auto* userData = new std::string[2];

            userData[0] = requestedPath.string();

            std::ifstream file(requestedPath, std::ios::binary);

            if (!file)
            {
                userData[1] = "Failed to open include file: "
                            + requestedPath.string();
            }
            else
            {
                userData[1].assign(
                    std::istreambuf_iterator<char>(file),
                    std::istreambuf_iterator<char>());
            }

            result->source_name = userData[0].c_str();
            result->source_name_length = userData[0].size();
            result->content = userData[1].c_str();
            result->content_length = userData[1].size();
            result->user_data = userData;

            return result;
        }

        void ReleaseInclude(shaderc_include_result* result) override
        {
            delete[] static_cast<std::string*>(result->user_data);
            delete result;
        }
    };
    
    bool CompileShader(
    const Filepath& aInputFile,
    const Filepath& aOutputFile,
    shaderc_shader_kind aShaderKind)
    {
        // Read GLSL source.
        std::ifstream input(aInputFile, std::ios::binary);

        if (!input)
        {
            LOG_ERROR("Failed to open shader %s", aInputFile.c_str());
            return false;
        }

        std::string source{ std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>()};

        // Create compiler and options.
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
    
        options.SetIncluder(std::make_unique<GLSLIncluder>());

        // Match the Vulkan environment supported by your engine.
        options.SetTargetEnvironment(
            shaderc_target_env_vulkan,
            shaderc_env_version_vulkan_1_0);

        // Compile GLSL into SPIR-V.
        shaderc::SpvCompilationResult result =
            compiler.CompileGlslToSpv(
                source,
                aShaderKind,
                aInputFile.string().c_str(),
                options);

        if (result.GetCompilationStatus() !=
            shaderc_compilation_status_success)
        {
            LOG_ERROR("Shader compilation failed: %s\n%s",aInputFile.string().c_str(),result.GetErrorMessage().c_str());
            return false;
        }

        // Write SPIR-V binary.
        std::ofstream output(
            aOutputFile,
            std::ios::binary | std::ios::trunc);

        if (!output)
        {
            LOG_ERROR("Failed to create SPIR-V file: %s", aOutputFile.c_str());
            return false;
        }

        for (uint32_t word : result)
        {
            output.write(
                reinterpret_cast<const char*>(&word),
                sizeof(word));
        }

        if (!output)
        {
            LOG_ERROR("Failed to create SPIR-V file: %s", aOutputFile.c_str());
            return false;
        }

        LOG_INFO("Compiled shader %s:", aInputFile.c_str());

        return true;
    }

    shaderc_shader_kind GetShaderKind(const Filepath& aFilename)
    {
        Filepath extension = aFilename.extension();
        if (extension == ".frag")
        {
            return shaderc_shader_kind::shaderc_glsl_default_fragment_shader;
        }
        if (extension == ".vert")
        {
            return shaderc_shader_kind::shaderc_glsl_default_vertex_shader;
        }
    
        return {};
    }

    vk::ShaderStageFlagBits GetShaderStage(const Filepath& aExtension)
    {
        auto it = localExtensionToShaderStage.find(aExtension);
        if (it != localExtensionToShaderStage.end())
        {
            return it->second;
        }
        
        return {};
    }
}
