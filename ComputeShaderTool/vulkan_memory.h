#pragma once
#include"vulkan_tool.h"
#include"vulkan_device.h"
namespace VulkanUtil {
	class VulkanMemory :public VulKanTool {
	public:
		VulkanMemory(std::shared_ptr<VulkanDevice> use_device, const VkMemoryAllocateInfo& info);
		~VulkanMemory();

		VkDeviceMemory get() const {
			return my_memory_;
		}
		uint32_t type() const {
			return my_type_index_;
		}
		VkDeviceSize size() const {
			return my_size_;
		}

	private:
		VkDeviceMemory my_memory_;
		std::shared_ptr<VulkanDevice> my_device_;
		uint32_t my_type_index_;
		VkDeviceSize my_size_;
	};

	class VulkanMemoryPool :public VulKanTool {
	public:
		VulkanMemoryPool(std::shared_ptr<VulkanDevice> use_device);
		~VulkanMemoryPool();
		VulkanMemory* AllocateMemory(const VkMemoryRequirements& requirements,
			VkFlags extra_mask, bool seperate = false);

		void ReturnMemory(VulkanMemory* memory, bool clean = false);

		void Clear();

		std::shared_ptr<VulkanDevice> GetDevice() { return my_deivce_; }
		float ComputeSize() const;
	private:
		std::map<const VulkanMemory*, std::shared_ptr<VulkanMemory>> my_all_buffers_;
		std::vector<std::multimap<VkDeviceSize, VulkanMemory*>> my_free_buffers_;

		VkPhysicalDeviceMemoryProperties my_property_;
		std::shared_ptr<VulkanDevice> my_deivce_;
	};
}