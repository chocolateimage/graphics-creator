// clang-format off
#include <volk.h>
#include "gpu/vk_mem_alloc.h"
// clang-format on
#include <QColorSpace>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QThread>

#define VK_CHECK_ERROR(result)                                                 \
    {                                                                          \
        if (result != 0) {                                                     \
            qInfo() << "Vulkan result" << result << "at" << __FILE__ << "line" \
                    << __LINE__;                                               \
            abort();                                                           \
        }                                                                      \
    }

int main(int argc, char **argv) {
    qInfo() << "Init";
    if (argc != 2) {
        return 1;
    }

    float radius = std::atof(argv[1]);
    if (radius < 1)
        radius = 1;
    if (radius > 500)
        radius = 500;

    volkInitialize();

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Testing Stuff";
    appInfo.applicationVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_3;

    std::vector<const char *> layerNames = {"VK_LAYER_KHRONOS_validation"};

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = layerNames.size();
    instanceCreateInfo.ppEnabledLayerNames = layerNames.data();

    VkInstance instance;
    VK_CHECK_ERROR(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));

    volkLoadInstance(instance);
    qInfo() << "Instance created";

    uint32_t physicalDeviceCount;
    VK_CHECK_ERROR(
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VK_CHECK_ERROR(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount,
                                              physicalDevices.data()));

    VkPhysicalDevice physicalDevice;
    uint32_t queueFamilyIndex = 0;

    for (auto device : physicalDevices) {
        qInfo() << "Device";
        VkPhysicalDeviceProperties2 properties{};
        properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceProperties2(device, &properties);
        qInfo() << "  Device ID" << properties.properties.deviceID;
        qInfo() << "  Device Name" << properties.properties.deviceName;

        uint32_t queueFamilyPropertyCount;
        vkGetPhysicalDeviceQueueFamilyProperties2(
            device, &queueFamilyPropertyCount, nullptr);
        std::vector<VkQueueFamilyProperties2> queueFamilyProperties;
        vkGetPhysicalDeviceQueueFamilyProperties2(
            device, &queueFamilyPropertyCount, queueFamilyProperties.data());

        for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
            if (queueFamilyProperties[i].queueFamilyProperties.queueFlags &
                VK_QUEUE_COMPUTE_BIT) {
                queueFamilyIndex = i;
                break;
            }
        }
    }
    physicalDevice = physicalDevices[0];

    std::vector<float> priorities = {.5};

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.pQueuePriorities = priorities.data();

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

    qInfo() << "Creating device";

    VkDevice device;
    VK_CHECK_ERROR(
        vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));
    volkLoadDevice(device);

    qInfo() << "Device created";

    qInfo() << "Creating allocator";

    VmaVulkanFunctions vulkanFunctions{};

    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions);

    VmaAllocator allocator;
    VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &allocator);
    if (result != 0) {
        qInfo() << "could not create allocator" << result;
        return 1;
    }

    qInfo() << "Allocator created";

    VkQueue queue;
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    VkCommandPool commandPool;
    VK_CHECK_ERROR(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr,
                                       &commandPool));

    VkCommandBufferAllocateInfo cmdBufAllocInfo{};
    cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocInfo.commandPool = commandPool;
    cmdBufAllocInfo.commandBufferCount = 1;
    cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    std::vector<VkCommandBuffer> cmdBuffers(1);
    VK_CHECK_ERROR(
        vkAllocateCommandBuffers(device, &cmdBufAllocInfo, cmdBuffers.data()));

    qInfo() << "Create shader";
    QFile file("testshader.comp.spv");
    if (!file.open(QFile::ReadOnly)) {
        qInfo() << "File not found";
        return 1;
    }
    QByteArray shaderCode = file.readAll();

    VkShaderModuleCreateInfo shaderCreateInfo{};
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.codeSize = shaderCode.size();
    shaderCreateInfo.pCode = (uint32_t *)shaderCode.data();
    VkShaderModule shaderModule;
    VK_CHECK_ERROR(vkCreateShaderModule(device, &shaderCreateInfo, nullptr,
                                        &shaderModule));

    VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
    pipelineCacheCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    QByteArray loadedPipelineCacheData;
    {
        QFile file("pipeline_cache.bin");
        if (file.open(QFile::ReadOnly)) {
            loadedPipelineCacheData = file.readAll();
            qInfo() << "Loading pipeline cache with size"
                    << loadedPipelineCacheData.size();
            pipelineCacheCreateInfo.initialDataSize =
                loadedPipelineCacheData.size();
            pipelineCacheCreateInfo.pInitialData =
                loadedPipelineCacheData.data();
        }
    }
    VkPipelineCache pipelineCache;
    VK_CHECK_ERROR(vkCreatePipelineCache(device, &pipelineCacheCreateInfo,
                                         nullptr, &pipelineCache));

    qInfo() << "Creating pipeline";

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings(2);
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
    descriptorSetLayoutCreateInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = layoutBindings.size();
    descriptorSetLayoutCreateInfo.pBindings = layoutBindings.data();
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(1);
    VK_CHECK_ERROR(
        vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo,
                                    nullptr, descriptorSetLayouts.data()));

    VkPushConstantRange range{};
    range.offset = 0;
    range.size = sizeof(float);
    range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkPipelineLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
    layoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
    layoutCreateInfo.pushConstantRangeCount = 1;
    layoutCreateInfo.pPushConstantRanges = &range;
    VkPipelineLayout pipelineLayout;
    VK_CHECK_ERROR(vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr,
                                          &pipelineLayout));

    VkComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.basePipelineIndex = -1;
    pipelineCreateInfo.basePipelineHandle = nullptr;
    pipelineCreateInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineCreateInfo.stage.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineCreateInfo.stage.module = shaderModule;
    pipelineCreateInfo.stage.pName = "main";
    pipelineCreateInfo.layout = pipelineLayout;

    VkPipeline pipeline;
    vkCreateComputePipelines(device, pipelineCache, 1, &pipelineCreateInfo,
                             nullptr, &pipeline);

    qInfo() << "Saving cache";
    {
        size_t savedSize;
        VK_CHECK_ERROR(
            vkGetPipelineCacheData(device, pipelineCache, &savedSize, nullptr));
        char *savedData = new char[savedSize];
        VK_CHECK_ERROR(vkGetPipelineCacheData(device, pipelineCache, &savedSize,
                                              savedData));
        qInfo() << "Pipeline cache size" << savedSize;
        QFile file("pipeline_cache.bin");
        if (file.open(QFile::WriteOnly)) {
            file.write(savedData, savedSize);
        } else {
            qInfo() << "Could not write pipeline cache file";
        }
        delete[] savedData;
    }

    qInfo() << "Creating descriptor set";
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes(2);
    descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorPoolSizes[0].descriptorCount = 1;
    descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSizes[1].descriptorCount = 1;
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.maxSets = 1;
    descriptorPoolInfo.poolSizeCount = descriptorPoolSizes.size();
    descriptorPoolInfo.pPoolSizes = descriptorPoolSizes.data();
    VkDescriptorPool descriptorPool;
    VK_CHECK_ERROR(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr,
                                          &descriptorPool));

    VkDescriptorSetAllocateInfo descriptorSetInfo{};
    descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetInfo.descriptorSetCount = descriptorSetLayouts.size();
    descriptorSetInfo.pSetLayouts = descriptorSetLayouts.data();
    descriptorSetInfo.descriptorPool = descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets(descriptorSetLayouts.size());
    VK_CHECK_ERROR(vkAllocateDescriptorSets(device, &descriptorSetInfo,
                                            descriptorSets.data()));

    qInfo() << "Creating images";

    constexpr uint64_t width = 1920;
    constexpr uint64_t height = 1080;
    // constexpr uint64_t bufferSize = width * height * 4;
    // constexpr uint64_t bufferSizeBytes = bufferSize * sizeof(float);
    // qInfo() << "bufferSizeBytes" << bufferSizeBytes;

    VkImage inputImg;
    VkImage outputImg;
    VkBuffer stagingBuffer;
    VmaAllocation inputImgAlloc;
    VmaAllocation outputImgAlloc;
    VmaAllocation stagingBufferAlloc;

    {
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageCreateInfo.usage =
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.pQueueFamilyIndices = &queueFamilyIndex;
        imageCreateInfo.queueFamilyIndexCount = 1;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        result =
            vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo,
                           &outputImg, &outputImgAlloc, nullptr);
        if (result != 0) {
            qInfo() << "vmaCreateImage Error" << result;
            return 1;
        }
    }

    {
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageCreateInfo.usage =
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.queueFamilyIndexCount = 1;
        imageCreateInfo.pQueueFamilyIndices = &queueFamilyIndex;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        result =
            vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo,
                           &inputImg, &inputImgAlloc, nullptr);
        if (result != 0) {
            qInfo() << "vmaCreateImage Error" << result;
            return 1;
        }
    }

    {
        VkBufferCreateInfo stagingBufferInfo{};
        stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingBufferInfo.size = width * height * 4;
        stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        stagingBufferInfo.usage =
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        allocationCreateInfo.flags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        result = vmaCreateBuffer(allocator, &stagingBufferInfo,
                                 &allocationCreateInfo, &stagingBuffer,
                                 &stagingBufferAlloc, nullptr);
        if (result != 0) {
            qInfo() << "vmaCreateBuffer Error" << result;
            return 1;
        }
    }

    qInfo() << "creating image views";
    VkImageView outputImageView;
    {
        VkImageViewCreateInfo imageViewCreateInfoOutput{};
        imageViewCreateInfoOutput.sType =
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfoOutput.image = outputImg;
        imageViewCreateInfoOutput.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfoOutput.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageViewCreateInfoOutput.subresourceRange.aspectMask =
            VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfoOutput.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfoOutput.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfoOutput.subresourceRange.layerCount = 1;
        imageViewCreateInfoOutput.subresourceRange.levelCount = 1;
        vkCreateImageView(device, &imageViewCreateInfoOutput, nullptr,
                          &outputImageView);
    }

    VkImageView inputImageView;
    {
        VkImageViewCreateInfo imageViewCreateInfoInput{};
        imageViewCreateInfoInput.sType =
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfoInput.image = inputImg;
        imageViewCreateInfoInput.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfoInput.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageViewCreateInfoInput.subresourceRange.aspectMask =
            VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfoInput.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfoInput.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfoInput.subresourceRange.layerCount = 1;
        imageViewCreateInfoInput.subresourceRange.levelCount = 1;
        VK_CHECK_ERROR(vkCreateImageView(device, &imageViewCreateInfoInput,
                                         nullptr, &inputImageView));
    }

    VkSampler sampler;
    {
        VkSamplerCreateInfo sampleInfo{};
        sampleInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampleInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampleInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampleInfo.unnormalizedCoordinates = true;
        VK_CHECK_ERROR(vkCreateSampler(device, &sampleInfo, nullptr, &sampler));
    }

    std::vector<VkDescriptorImageInfo> descriptorImageInfos(2);
    descriptorImageInfos[0].sampler = nullptr;
    descriptorImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    descriptorImageInfos[0].imageView = outputImageView;

    descriptorImageInfos[1].sampler = sampler;
    descriptorImageInfos[1].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    descriptorImageInfos[1].imageView = inputImageView;

    qInfo() << "Update descriptor sets";
    std::vector<VkWriteDescriptorSet> writeDescriptorSets(2);
    {
        writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[0].descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writeDescriptorSets[0].descriptorCount = 1;
        writeDescriptorSets[0].dstSet = descriptorSets[0];
        writeDescriptorSets[0].dstBinding = 0;
        writeDescriptorSets[0].pImageInfo = &descriptorImageInfos[0];
    }
    {
        writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[1].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSets[1].descriptorCount = 1;
        writeDescriptorSets[1].dstSet = descriptorSets[0];
        writeDescriptorSets[1].dstBinding = 1;
        writeDescriptorSets[1].pImageInfo = &descriptorImageInfos[1];
    }
    vkUpdateDescriptorSets(device, writeDescriptorSets.size(),
                           writeDescriptorSets.data(), 0, nullptr);

    qInfo() << "Mapping memory";
    uint8_t *imageData;
    result = vmaMapMemory(allocator, stagingBufferAlloc, (void **)&imageData);
    if (result != 0) {
        qInfo() << "vmaMapMemory Error" << result;
        return 1;
    }

    {
        // copy from file to staging buffer
        QImage img("original.png");
        img = img.convertedToColorSpace(QColorSpace::SRgb);
        for (uint64_t y = 0; y < height; y++) {
            for (uint64_t x = 0; x < width; x++) {
                uint8_t *val = imageData + ((y * width + x) * 4);
                QColor c = img.pixelColor(x, y);
                val[0] = c.red();
                val[1] = c.green();
                val[2] = c.blue();
                val[3] = c.alpha();
            }
        }
    }

    vmaFlushAllocation(allocator, stagingBufferAlloc, 0, VK_WHOLE_SIZE);

    qInfo() << "Recording";
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(cmdBuffers[0], &beginInfo);

        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = inputImg;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.srcAccessMask = VK_ACCESS_NONE;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            vkCmdPipelineBarrier(cmdBuffers[0],
                                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
                                 0, nullptr, 1, &barrier);
        }

        {
            VkBufferImageCopy bufferImageCopy{};
            bufferImageCopy.imageSubresource.aspectMask =
                VK_IMAGE_ASPECT_COLOR_BIT;
            bufferImageCopy.imageSubresource.baseArrayLayer = 0;
            bufferImageCopy.imageSubresource.layerCount = 1;
            bufferImageCopy.imageSubresource.mipLevel = 0;
            bufferImageCopy.imageExtent = VkExtent3D{1920, 1080, 1};
            bufferImageCopy.bufferRowLength = 1920;
            bufferImageCopy.bufferImageHeight = 1080;
            vkCmdCopyBufferToImage(cmdBuffers[0], stagingBuffer, inputImg,
                                   VK_IMAGE_LAYOUT_GENERAL, 1,
                                   &bufferImageCopy);
        }

        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = outputImg;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.srcAccessMask = VK_ACCESS_NONE;
            barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;

            VkImageMemoryBarrier barrier2{};
            barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier2.image = inputImg;
            barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier2.subresourceRange.baseArrayLayer = 0;
            barrier2.subresourceRange.baseMipLevel = 0;
            barrier2.subresourceRange.layerCount = 1;
            barrier2.subresourceRange.levelCount = 1;
            barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier2.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier2.newLayout = VK_IMAGE_LAYOUT_GENERAL;

            std::vector<VkImageMemoryBarrier> barriers{barrier, barrier2};

            vkCmdPipelineBarrier(cmdBuffers[0], VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0,
                                 nullptr, 0, nullptr, barriers.size(),
                                 barriers.data());
        }

        vkCmdBindPipeline(cmdBuffers[0], VK_PIPELINE_BIND_POINT_COMPUTE,
                          pipeline);
        vkCmdBindDescriptorSets(cmdBuffers[0], VK_PIPELINE_BIND_POINT_COMPUTE,
                                pipelineLayout, 0, descriptorSets.size(),
                                descriptorSets.data(), 0, nullptr);
        vkCmdPushConstants(cmdBuffers[0], pipelineLayout,
                           VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(float),
                           &radius);

        vkCmdDispatch(cmdBuffers[0], std::ceil(1920. / 8.),
                      std::ceil(1080. / 8.), 1);

        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = outputImg;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            vkCmdPipelineBarrier(cmdBuffers[0],
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
                                 0, nullptr, 1, &barrier);
        }

        {
            VkBufferImageCopy bufferImageCopy{};
            bufferImageCopy.imageSubresource.aspectMask =
                VK_IMAGE_ASPECT_COLOR_BIT;
            bufferImageCopy.imageSubresource.baseArrayLayer = 0;
            bufferImageCopy.imageSubresource.layerCount = 1;
            bufferImageCopy.imageSubresource.mipLevel = 0;
            bufferImageCopy.imageExtent = VkExtent3D{1920, 1080, 1};
            bufferImageCopy.bufferRowLength = 1920;
            bufferImageCopy.bufferImageHeight = 1080;
            vkCmdCopyImageToBuffer(cmdBuffers[0], outputImg,
                                   VK_IMAGE_LAYOUT_GENERAL, stagingBuffer, 1,
                                   &bufferImageCopy);
        }

        vkEndCommandBuffer(cmdBuffers[0]);
    }

    qInfo() << "Submitting";

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = cmdBuffers.size();
    submitInfo.pCommandBuffers = cmdBuffers.data();
    // submitInfo.setSignalSemaphores(sem);
    // vk::Fence f;
    vkQueueSubmit(queue, 1, &submitInfo, nullptr);

    qInfo() << "Waiting";
    vkQueueWaitIdle(queue);

    qInfo() << "Writing";
    vmaInvalidateAllocation(allocator, stagingBufferAlloc, 0, VK_WHOLE_SIZE);

    {
        QImage img(width, height, QImage::Format_RGBA8888_Premultiplied);
        for (uint64_t y = 0; y < height; y++) {
            for (uint64_t x = 0; x < width; x++) {
                uint8_t *val = imageData + ((y * width + x) * 4);
                img.setPixelColor(x, y, QColor(val[0], val[1], val[2], val[3]));
            }
        }
        img.save("test.png");
    }

    // qInfo() << imageData[0];
    // qInfo() << imageData[1];
    // qInfo() << imageData[2];
    // qInfo() << imageData[3];

    qInfo() << "Freeing";
    vmaUnmapMemory(allocator, stagingBufferAlloc);

    vkDestroySampler(device, sampler, nullptr);
    vkDestroyImageView(device, inputImageView, nullptr);
    vkDestroyImageView(device, outputImageView, nullptr);
    vmaDestroyImage(allocator, inputImg, inputImgAlloc);
    vmaDestroyImage(allocator, outputImg, outputImgAlloc);
    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAlloc);
    vmaDestroyAllocator(allocator);

    vkDestroyPipelineCache(device, pipelineCache, nullptr);
    for (auto layout : descriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(device, layout, nullptr);
    }
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyShaderModule(device, shaderModule, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);

    qInfo() << "Finish";
    return 0;
}
