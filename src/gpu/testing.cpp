#include "gpu/vk_mem_alloc.h"
#include <QDebug>
#include <QFile>
#include <vulkan/vulkan.hpp>

int main(int argc, char **argv) {
    qInfo() << "Init";
    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = "Testing Stuff";
    appInfo.applicationVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_4;

    std::vector<const char *> layerNames = {"VK_LAYER_KHRONOS_validation"};

    vk::InstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.setPEnabledLayerNames(layerNames);

    vk::Instance instance = vk::createInstance(instanceCreateInfo);
    qInfo() << "Instance created";

    std::vector<vk::PhysicalDevice> physicalDevices =
        instance.enumeratePhysicalDevices();

    vk::PhysicalDevice firstDevice;
    uint32_t queueFamilyIndex = 0;

    for (auto device : physicalDevices) {
        qInfo() << "Device";
        auto properties = device.getProperties();
        qInfo() << "  Device ID" << properties.deviceID;
        qInfo() << "  Device Name" << properties.deviceName.data();

        auto queueFamilyProperties = device.getQueueFamilyProperties();

        for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
            if (queueFamilyProperties[i].queueFlags &
                vk::QueueFlagBits::eCompute) {
                queueFamilyIndex = i;
                break;
            }
        }

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

    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(3);
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    layoutBindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    layoutBindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    layoutBindings[2].binding = 2;
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    layoutBindings[2].stageFlags = vk::ShaderStageFlagBits::eCompute;

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
    descriptorSetLayoutCreateInfo.setBindings(layoutBindings);
    auto descriptorSetLayouts =
        device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

    vk::PipelineLayoutCreateInfo layoutCreateInfo;
    layoutCreateInfo.setSetLayouts(descriptorSetLayouts);
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
        if (file.open(QFile::ReadWrite)) {
            file.write((char *)savedPipelineCacheData.data(),
                       savedPipelineCacheData.size());
        } else {
            qInfo() << "Could not write pipeline cache file";
        }
    }

    qInfo() << "Creating descriptor set";
    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes(1);
    descriptorPoolSizes[0].type = vk::DescriptorType::eStorageBuffer;
    descriptorPoolSizes[0].descriptorCount = 3;
    vk::DescriptorPoolCreateInfo descriptorPoolInfo;
    descriptorPoolInfo.maxSets = 1;
    descriptorPoolInfo.setPoolSizes(descriptorPoolSizes);
    vk::DescriptorPool descriptorPool =
        device.createDescriptorPool(descriptorPoolInfo);

    vk::DescriptorSetAllocateInfo descriptorSetInfo;
    descriptorSetInfo.setSetLayouts(descriptorSetLayouts);
    descriptorSetInfo.descriptorPool = descriptorPool;
    auto descriptorSets = device.allocateDescriptorSets(descriptorSetInfo);

    qInfo() << "Creating buffers";
    vk::BufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.size = 8192;
    bufferCreateInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer;
    VkBuffer buffer1;
    VkBuffer buffer2;
    VkBuffer buffer3;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

    VmaAllocation allocation1;
    VmaAllocation allocation2;
    VmaAllocation allocation3;
    result = vmaCreateBuffer(allocator, bufferCreateInfo, &allocationCreateInfo,
                             &buffer1, &allocation1, nullptr);
    if (result != 0) {
        qInfo() << "vmaCreateBuffer Error" << result;
        return 1;
    }
    result = vmaCreateBuffer(allocator, bufferCreateInfo, &allocationCreateInfo,
                             &buffer2, &allocation2, nullptr);
    if (result != 0) {
        qInfo() << "vmaCreateBuffer Error" << result;
        return 1;
    }
    result = vmaCreateBuffer(allocator, bufferCreateInfo, &allocationCreateInfo,
                             &buffer3, &allocation3, nullptr);
    if (result != 0) {
        qInfo() << "vmaCreateBuffer Error" << result;
        return 1;
    }

    float *bufferData1;
    float *bufferData2;
    float *bufferData3;

    result = vmaMapMemory(allocator, allocation1, (void **)&bufferData1);
    if (result != 0) {
        qInfo() << "vmaMapMemory Error" << result;
        return 1;
    }
    result = vmaMapMemory(allocator, allocation2, (void **)&bufferData2);
    if (result != 0) {
        qInfo() << "vmaMapMemory Error" << result;
        return 1;
    }
    result = vmaMapMemory(allocator, allocation3, (void **)&bufferData3);
    if (result != 0) {
        qInfo() << "vmaMapMemory Error" << result;
        return 1;
    }

    bufferData1[0] = 1.5;
    bufferData2[0] = 3.5;
    bufferData3[0] = 555555;

    std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos(3);
    descriptorBufferInfos[0].offset = 0;
    descriptorBufferInfos[0].range = VK_WHOLE_SIZE;
    descriptorBufferInfos[0].buffer = buffer1;
    descriptorBufferInfos[1].offset = 0;
    descriptorBufferInfos[1].range = VK_WHOLE_SIZE;
    descriptorBufferInfos[1].buffer = buffer2;
    descriptorBufferInfos[2].offset = 0;
    descriptorBufferInfos[2].range = VK_WHOLE_SIZE;
    descriptorBufferInfos[2].buffer = buffer3;

    qInfo() << "Update descriptor sets";
    std::vector<vk::WriteDescriptorSet> writeDescriptorSets(3);
    for (int i = 0; i < 3; i++) {
        writeDescriptorSets[i].descriptorType =
            vk::DescriptorType::eStorageBuffer;
        writeDescriptorSets[i].descriptorCount = 1;
        writeDescriptorSets[i].dstSet = descriptorSets[0];
        writeDescriptorSets[i].dstBinding = i;
        writeDescriptorSets[i].pBufferInfo = &descriptorBufferInfos[i];
    }
    device.updateDescriptorSets(writeDescriptorSets, {});

    qInfo() << "Recording";
    vk::CommandBufferBeginInfo beginInfo;
    cmdBuffers[0].begin(beginInfo);

    cmdBuffers[0].bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    cmdBuffers[0].bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                     pipelineLayout, 0, descriptorSets, {});

    // TODO change count
    cmdBuffers[0].dispatch(1, 1, 1);

    cmdBuffers[0].end();

    qInfo() << "Submitting";

    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(cmdBuffers);
    queue.submit(submitInfo);

    qInfo() << "Waiting";
    queue.waitIdle();

    qInfo() << bufferData1[0] << "+" << bufferData2[0] << "=" << bufferData3[0];

    vmaUnmapMemory(allocator, allocation1);
    vmaUnmapMemory(allocator, allocation2);
    vmaUnmapMemory(allocator, allocation1);

    vmaDestroyBuffer(allocator, buffer1, allocation1);
    vmaDestroyBuffer(allocator, buffer2, allocation2);
    vmaDestroyBuffer(allocator, buffer3, allocation3);
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
