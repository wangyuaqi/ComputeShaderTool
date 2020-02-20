#include"vulkan_tool.h"

namespace VulkanUtil {
	uint32_t VulkanUtil::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) {
		//查询物理设备可用的内存类型
		VkPhysicalDeviceMemoryProperties mem_properties;
		//memoryHeaps存放内存来源，
		//检测与我们需要的properties是否完全相同
		vkGetPhysicalDeviceMemoryProperties(now_device_->GetPhysicalDevice(), &mem_properties);
		for (uint32_t count = 0; count < mem_properties.memoryTypeCount; count++) {
			if ((type_filter&(1 << count)) && (mem_properties.memoryTypes[count].propertyFlags&properties) == properties)
				return count;
		}
	}
}