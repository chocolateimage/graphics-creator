#include "gpu/vk_mem_alloc.h"
#include "math.hpp"
#include <QColorSpace>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QThread>
#include <string>
#include <vulkan/vulkan.hpp>

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

    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = "Testing Stuff";
    appInfo.applicationVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    std::vector<const char *> layerNames = {"VK_LAYER_KHRONOS_validation"};

    vk::InstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.setPEnabledLayerNames(layerNames);

    vk::Instance instance = vk::createInstance(instanceCreateInfo);
    qInfo() << "Instance created";

    std::vector<vk::PhysicalDevice> physicalDevices =
        instance.enumeratePhysicalDevices();

    vk::PhysicalDevice firstDevice;
    uint32_t queueFamilyIndex = 1;

    for (auto device : physicalDevices) {
        qInfo() << "Device";
        auto properties = device.getProperties();
        qInfo() << "  Device ID" << properties.deviceID;
        qInfo() << "  Device Name" << properties.deviceName.data();

        auto queueFamilyProperties = device.getQueueFamilyProperties();

        // for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
        //     if (queueFamilyProperties[i].queueFlags &
        //         vk::QueueFlagBits::eCompute) {
        //         queueFamilyIndex = i;
        //         break;
        //     }
        // }

        firstDevice = device;
        break;
    }

    std::vector<float> priorities = {.5};

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.pQueuePriorities = priorities.data();
    queueCreateInfos.push_back(queueCreateInfo);

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.setQueueCreateInfos(queueCreateInfos);

    qInfo() << "Creating device";

    auto device = firstDevice.createDevice(deviceCreateInfo);

    qInfo() << "Device created";

    qInfo() << "Creating allocator";

    VmaVulkanFunctions vulkanFunctions{};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.physicalDevice = firstDevice;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    VmaAllocator allocator;
    VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &allocator);
    if (result != 0) {
        qInfo() << "could not create allocator" << result;
        return 1;
    }

    qInfo() << "Allocator created";

    auto queue = device.getQueue(queueFamilyIndex, 0);

    vk::CommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    auto commandPool = device.createCommandPool(commandPoolCreateInfo);

    vk::CommandBufferAllocateInfo cmdBufAllocInfo{};
    cmdBufAllocInfo.commandPool = commandPool;
    cmdBufAllocInfo.commandBufferCount = 1;
    cmdBufAllocInfo.level = vk::CommandBufferLevel::ePrimary;

    auto cmdBuffers = device.allocateCommandBuffers(cmdBufAllocInfo);

    qInfo() << "Create shader";
    QFile file("testshader.comp.spv");
    if (!file.open(QFile::ReadOnly)) {
        qInfo() << "File not found";
        return 1;
    }
    QByteArray shaderCode = file.readAll();

    vk::ShaderModuleCreateInfo shaderCreateInfo{};
    shaderCreateInfo.codeSize = shaderCode.size();
    shaderCreateInfo.pCode = (uint32_t *)shaderCode.data();
    auto shaderModule = device.createShaderModule(shaderCreateInfo);

    vk::PipelineCacheCreateInfo pipelineCacheCreateInfo;
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
    vk::PipelineCache pipelineCache =
        device.createPipelineCache(pipelineCacheCreateInfo);

    qInfo() << "Creating pipeline";

    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(2);
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].descriptorType = vk::DescriptorType::eStorageImage;
    layoutBindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].descriptorType =
        vk::DescriptorType::eCombinedImageSampler;
    layoutBindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
    descriptorSetLayoutCreateInfo.setBindings(layoutBindings);
    auto descriptorSetLayouts =
        device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

    vk::PushConstantRange range;
    range.offset = 0;
    range.size = sizeof(float);
    range.stageFlags = vk::ShaderStageFlagBits::eCompute;

    vk::PipelineLayoutCreateInfo layoutCreateInfo;
    layoutCreateInfo.setSetLayouts(descriptorSetLayouts);
    layoutCreateInfo.setPushConstantRanges(range);
    auto pipelineLayout = device.createPipelineLayout(layoutCreateInfo);

    vk::ComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.basePipelineIndex = -1;
    pipelineCreateInfo.basePipelineHandle = nullptr;
    pipelineCreateInfo.stage.stage = vk::ShaderStageFlagBits::eCompute;
    pipelineCreateInfo.stage.module = shaderModule;
    pipelineCreateInfo.stage.pName = "main";
    pipelineCreateInfo.layout = pipelineLayout;

    vk::Pipeline pipeline =
        device.createComputePipeline(pipelineCache, pipelineCreateInfo).value;

    qInfo() << "Saving cache";
    {
        auto savedPipelineCacheData =
            device.getPipelineCacheData(pipelineCache);
        qInfo() << "Pipeline cache size" << savedPipelineCacheData.size();
        QFile file("pipeline_cache.bin");
        if (file.open(QFile::WriteOnly)) {
            file.write((char *)savedPipelineCacheData.data(),
                       savedPipelineCacheData.size());
        } else {
            qInfo() << "Could not write pipeline cache file";
        }
    }

    qInfo() << "Creating descriptor set";
    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes(2);
    descriptorPoolSizes[0].type = vk::DescriptorType::eStorageImage;
    descriptorPoolSizes[0].descriptorCount = 1;
    descriptorPoolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    descriptorPoolSizes[1].descriptorCount = 1;
    vk::DescriptorPoolCreateInfo descriptorPoolInfo;
    descriptorPoolInfo.maxSets = 1;
    descriptorPoolInfo.setPoolSizes(descriptorPoolSizes);
    vk::DescriptorPool descriptorPool =
        device.createDescriptorPool(descriptorPoolInfo);

    vk::DescriptorSetAllocateInfo descriptorSetInfo;
    descriptorSetInfo.setSetLayouts(descriptorSetLayouts);
    descriptorSetInfo.descriptorPool = descriptorPool;
    auto descriptorSets = device.allocateDescriptorSets(descriptorSetInfo);

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
        vk::ImageCreateInfo imageCreateInfo{};
        imageCreateInfo.imageType = vk::ImageType::e2D;
        imageCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
        imageCreateInfo.usage = vk::ImageUsageFlagBits::eStorage |
                                vk::ImageUsageFlagBits::eTransferSrc;
        imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
        imageCreateInfo.setQueueFamilyIndices(queueFamilyIndex);

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
        result =
            vmaCreateImage(allocator, imageCreateInfo, &allocationCreateInfo,
                           &outputImg, &outputImgAlloc, nullptr);
        if (result != 0) {
            qInfo() << "vmaCreateImage Error" << result;
            return 1;
        }
    }

    {
        vk::ImageCreateInfo imageCreateInfo{};
        imageCreateInfo.imageType = vk::ImageType::e2D;
        imageCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
        imageCreateInfo.usage = vk::ImageUsageFlagBits::eSampled |
                                vk::ImageUsageFlagBits::eTransferDst;
        imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
        imageCreateInfo.setQueueFamilyIndices(queueFamilyIndex);

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
        result =
            vmaCreateImage(allocator, imageCreateInfo, &allocationCreateInfo,
                           &inputImg, &inputImgAlloc, nullptr);
        if (result != 0) {
            qInfo() << "vmaCreateImage Error" << result;
            return 1;
        }
    }

    {
        vk::BufferCreateInfo stagingBufferInfo;
        stagingBufferInfo.size = width * height * 4;
        stagingBufferInfo.sharingMode = vk::SharingMode::eExclusive;
        stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc |
                                  vk::BufferUsageFlagBits::eTransferDst;
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        allocationCreateInfo.flags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        result =
            vmaCreateBuffer(allocator, stagingBufferInfo, &allocationCreateInfo,
                            &stagingBuffer, &stagingBufferAlloc, nullptr);
        if (result != 0) {
            qInfo() << "vmaCreateBuffer Error" << result;
            return 1;
        }
    }

    // float *bufferData1;

    // result = vmaMapMemory(allocator, allocation1, (void **)&bufferData1);
    // if (result != 0) {
    //     qInfo() << "vmaMapMemory Error" << result;
    //     return 1;
    // }

    // qInfo() << "writing data";
    // for (uint64_t i = 0; i < bufferSize; i++) {
    //     bufferData1[i] = i;
    //     bufferData2[i] = i * 2;
    // }

    qInfo() << "creating image views";
    vk::ImageViewCreateInfo imageViewCreateInfoOutput;
    imageViewCreateInfoOutput.image = outputImg;
    imageViewCreateInfoOutput.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfoOutput.format = vk::Format::eR8G8B8A8Unorm;
    imageViewCreateInfoOutput.subresourceRange.aspectMask =
        vk::ImageAspectFlagBits::eColor;
    imageViewCreateInfoOutput.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfoOutput.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfoOutput.subresourceRange.layerCount = 1;
    imageViewCreateInfoOutput.subresourceRange.levelCount = 1;
    vk::ImageView outputImageView =
        device.createImageView(imageViewCreateInfoOutput);

    vk::ImageViewCreateInfo imageViewCreateInfoInput;
    imageViewCreateInfoInput.image = inputImg;
    imageViewCreateInfoInput.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfoInput.format = vk::Format::eR8G8B8A8Unorm;
    imageViewCreateInfoInput.subresourceRange.aspectMask =
        vk::ImageAspectFlagBits::eColor;
    imageViewCreateInfoInput.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfoInput.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfoInput.subresourceRange.layerCount = 1;
    imageViewCreateInfoInput.subresourceRange.levelCount = 1;
    vk::ImageView inputImageView =
        device.createImageView(imageViewCreateInfoInput);

    vk::SamplerCreateInfo sampleInfo;
    sampleInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    sampleInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    sampleInfo.unnormalizedCoordinates = true;
    vk::Sampler sampler = device.createSampler(sampleInfo);

    std::vector<vk::DescriptorImageInfo> descriptorImageInfos(2);
    descriptorImageInfos[0].sampler = nullptr;
    descriptorImageInfos[0].imageLayout = vk::ImageLayout::eGeneral;
    descriptorImageInfos[0].imageView = outputImageView;

    descriptorImageInfos[1].sampler = sampler;
    descriptorImageInfos[1].imageLayout = vk::ImageLayout::eGeneral;
    descriptorImageInfos[1].imageView = inputImageView;

    qInfo() << "Update descriptor sets";
    std::vector<vk::WriteDescriptorSet> writeDescriptorSets(2);
    {
        writeDescriptorSets[0].descriptorType =
            vk::DescriptorType::eStorageImage;
        writeDescriptorSets[0].descriptorCount = 1;
        writeDescriptorSets[0].dstSet = descriptorSets[0];
        writeDescriptorSets[0].dstBinding = 0;
        writeDescriptorSets[0].pImageInfo = &descriptorImageInfos[0];
    }
    {
        writeDescriptorSets[1].descriptorType =
            vk::DescriptorType::eCombinedImageSampler;
        writeDescriptorSets[1].descriptorCount = 1;
        writeDescriptorSets[1].dstSet = descriptorSets[0];
        writeDescriptorSets[1].dstBinding = 1;
        writeDescriptorSets[1].pImageInfo = &descriptorImageInfos[1];
    }
    device.updateDescriptorSets(writeDescriptorSets, {});

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
    vk::CommandBufferBeginInfo beginInfo;
    {
        cmdBuffers[0].begin(beginInfo);

        {
            vk::ImageMemoryBarrier barrier;
            barrier.image = inputImg;
            barrier.subresourceRange =
                imageViewCreateInfoInput.subresourceRange;
            barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
            barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
            barrier.srcAccessMask = vk::AccessFlagBits::eNone;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.oldLayout = vk::ImageLayout::eUndefined;
            barrier.newLayout = vk::ImageLayout::eGeneral;
            cmdBuffers[0].pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                          vk::PipelineStageFlagBits::eTransfer,
                                          {}, {}, {}, barrier);
        }

        {
            vk::BufferImageCopy bufferImageCopy;
            bufferImageCopy.imageSubresource.aspectMask =
                vk::ImageAspectFlagBits::eColor;
            bufferImageCopy.imageSubresource.baseArrayLayer = 0;
            bufferImageCopy.imageSubresource.layerCount = 1;
            bufferImageCopy.imageSubresource.mipLevel = 0;
            bufferImageCopy.imageExtent = vk::Extent3D{1920, 1080, 1};
            bufferImageCopy.bufferRowLength = 1920;
            bufferImageCopy.bufferImageHeight = 1080;
            cmdBuffers[0].copyBufferToImage(stagingBuffer, inputImg,
                                            vk::ImageLayout::eGeneral,
                                            bufferImageCopy);
        }

        {
            vk::ImageMemoryBarrier barrier;
            barrier.image = outputImg;
            barrier.subresourceRange =
                imageViewCreateInfoOutput.subresourceRange;
            barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
            barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
            barrier.srcAccessMask = vk::AccessFlagBits::eNone;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
            barrier.oldLayout = vk::ImageLayout::eUndefined;
            barrier.newLayout = vk::ImageLayout::eGeneral;

            vk::ImageMemoryBarrier barrier2;
            barrier2.image = inputImg;
            barrier2.subresourceRange =
                imageViewCreateInfoInput.subresourceRange;
            barrier2.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
            barrier2.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
            barrier2.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier2.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            barrier2.oldLayout = vk::ImageLayout::eGeneral;
            barrier2.newLayout = vk::ImageLayout::eGeneral;

            cmdBuffers[0].pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eComputeShader, {}, {}, {},
                {barrier, barrier2});
        }

        cmdBuffers[0].bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
        cmdBuffers[0].bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                         pipelineLayout, 0, descriptorSets, {});
        cmdBuffers[0].pushConstants(pipelineLayout,
                                    vk::ShaderStageFlagBits::eCompute, 0,
                                    sizeof(float), &radius);

        cmdBuffers[0].dispatch(std::ceil(1920. / 8.), std::ceil(1080. / 8.), 1);

        {
            vk::ImageMemoryBarrier barrier;
            barrier.image = outputImg;
            barrier.subresourceRange =
                imageViewCreateInfoOutput.subresourceRange;
            barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
            barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
            barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.oldLayout = vk::ImageLayout::eGeneral;
            barrier.newLayout = vk::ImageLayout::eGeneral;
            cmdBuffers[0].pipelineBarrier(
                vk::PipelineStageFlagBits::eComputeShader,
                vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);
        }

        {
            vk::BufferImageCopy bufferImageCopy;
            bufferImageCopy.imageSubresource.aspectMask =
                vk::ImageAspectFlagBits::eColor;
            bufferImageCopy.imageSubresource.baseArrayLayer = 0;
            bufferImageCopy.imageSubresource.layerCount = 1;
            bufferImageCopy.imageSubresource.mipLevel = 0;
            bufferImageCopy.imageExtent = vk::Extent3D{1920, 1080, 1};
            bufferImageCopy.bufferRowLength = 1920;
            bufferImageCopy.bufferImageHeight = 1080;
            cmdBuffers[0].copyImageToBuffer(outputImg,
                                            vk::ImageLayout::eGeneral,
                                            stagingBuffer, bufferImageCopy);
        }

        cmdBuffers[0].end();
    }

    qInfo() << "Submitting";

    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(cmdBuffers);
    // submitInfo.setSignalSemaphores(sem);
    // vk::Fence f;
    queue.submit(submitInfo);

    qInfo() << "Waiting";
    queue.waitIdle();

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

    device.destroySampler(sampler);
    device.destroyImageView(inputImageView);
    device.destroyImageView(outputImageView);
    vmaDestroyImage(allocator, inputImg, inputImgAlloc);
    vmaDestroyImage(allocator, outputImg, outputImgAlloc);
    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAlloc);
    vmaDestroyAllocator(allocator);

    device.destroyPipelineCache(pipelineCache);
    device.destroyDescriptorSetLayout(descriptorSetLayouts);
    device.destroyDescriptorPool(descriptorPool);
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyPipeline(pipeline);
    device.destroyShaderModule(shaderModule);
    device.destroyCommandPool(commandPool);
    device.destroy();
    instance.destroy();

    qInfo() << "Finish";
    return 0;
}
