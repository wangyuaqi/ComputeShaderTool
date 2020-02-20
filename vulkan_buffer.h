#pragma once
#include"vulkan_tool.h"
#include"vulkan_device.h"
namespace VulkanUtil {
	class VulkanBuffer :public VulKanTool {

	public:
		VulkanBuffer(std::shared_ptr<VulkanDevice> use_device);

		VulkanBuffer(std::shared_ptr<VulkanDevice> use_device, size_t size_in_bytes, const char *data);

		~VulkanBuffer();

		VkDeviceMemory& GetVkMemory() { return now_buffer_memory_; }
		VkBuffer& GetVkBuffer() { return now_buffer_; }
	private:
	    std::shared_ptr<VulkanDevice> my_device_;
		VkBuffer now_buffer_;
		VkDeviceMemory now_buffer_memory_;

	private:
		VulkanBuffer();
		bool Init(size_t size_in_bytes, const char *data);


	};
}