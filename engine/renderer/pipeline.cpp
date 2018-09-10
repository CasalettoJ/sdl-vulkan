#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <iostream>

#include "pipeline.h"
#include "../systems/fileio.h"

Pipeline::ConstructedPipeline Pipeline::CreateGraphicsPipeline(const VkDevice &logicalDevice, const VkExtent2D &extent, const VkFormat &format)
{
    // Pipeline Steps:
    // 1. Shader Modules -- Programmable Shaders
    // 2. Pipeline Shader Stage -- Shader combinations
    // 3. Vertex Input -- Outside data structs input to shaders
    // 4. Input Assembly -- Interpretation of vertex input
    // 5. Viewport / Scissor -- Render sizing
    // 6. Rasterizer -- Translation from Vertices (vertex shader output) to Fragments as input to the fragment shader
    // 7. Multisampling -- Antialiasing technique
    // 8. Depth and stencil testing -- https://en.wikipedia.org/wiki/Stencil_buffer
    // 9. Color Blending -- Color blending.
    // 10. Dynamic State -- Handling dynamic structs in pipeline (like viewport)
    // 11. Pipeline Layout -- uniform values need to be specified during pipeline creation by creating a VkPipelineLayout object
    // 12. Render Pass --  how many color and depth buffers there will be, how many samples to use for each of them and how their contents should be handled throughout the rendering operations.
    Pipeline::ConstructedPipeline constructedPipeline;

    // 1 Shader Modules
    std::vector<char> vertShaderData = FileIOSystem::ReadFileToVector("./assets/shaders/vert.spv");
    std::vector<char> fragShaderData = FileIOSystem::ReadFileToVector("./assets/shaders/frag.spv");

    VkShaderModule vertShader = Pipeline::CreateShaderModule(logicalDevice, vertShaderData);
    VkShaderModule fragShader = Pipeline::CreateShaderModule(logicalDevice, fragShaderData);

    VkPipelineShaderStageCreateInfo vertCreateInfo = {};
    vertCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertCreateInfo.module = vertShader;
    vertCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragCreateInfo = {};
    fragCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragCreateInfo.module = fragShader;
    fragCreateInfo.pName = "main";

    // 2 Pipeline Shader Stage
    VkPipelineShaderStageCreateInfo pipelineShaderSteps[] = {vertCreateInfo, fragCreateInfo};

    // 3 Vertex Input
    // https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    // 4 Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // 5 Viewport / Scissor
    // Now this viewport and scissor rectangle need to be combined into a viewport state using the VkPipelineViewportStateCreateInfo struct.
    // It is possible to use multiple viewports and scissor rectangles on some graphics cards, so its members reference an array of them.
    // Using multiple requires enabling a GPU feature (see logical device creation).
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // 6 Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // 7 Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // 8 Depth and stencil testing - skipped

    // 9 Color Blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // 10 Dynamic State - skipped

    // 11 Pipeline Layout
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout.");
    }
    constructedPipeline.layout = pipelineLayout;

    // 12 Render Pass
    constructedPipeline.renderPass = Pipeline::CreateRenderPass(logicalDevice, format);

    vkDestroyShaderModule(logicalDevice, vertShader, nullptr);
    vkDestroyShaderModule(logicalDevice, fragShader, nullptr);
    return constructedPipeline;
}

VkShaderModule Pipeline::CreateShaderModule(const VkDevice &logicalDevice, const std::vector<char> &source)
{
    VkShaderModuleCreateInfo createShaderInfo = {};
    createShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createShaderInfo.codeSize = source.size();
    createShaderInfo.pCode = reinterpret_cast<const uint32_t *>(source.data());

    VkShaderModule shader;
    if (vkCreateShaderModule(logicalDevice, &createShaderInfo, nullptr, &shader) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module.");
    }
    return shader;
}

VkRenderPass Pipeline::CreateRenderPass(const VkDevice &logicalDevice, const VkFormat &format)
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // Reference for the description above for subpasses to use in their creation.
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // performance type.

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Always do this unless doing compute instead
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;


    VkRenderPass renderPass;
    if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }

    return renderPass;
}