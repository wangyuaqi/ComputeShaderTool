#include"vulkan_tool.h"

namespace VulkanUtil {
	uint32_t VulkanUtil::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) {
		//��ѯ�����豸���õ��ڴ�����
		VkPhysicalDeviceMemoryProperties mem_properties;
		//memoryHeaps����ڴ���Դ��
		//�����������Ҫ��properties�Ƿ���ȫ��ͬ
		vkGetPhysicalDeviceMemoryProperties(now_device_->GetPhysicalDevice(), &mem_properties);
		for (uint32_t count = 0; count < mem_properties.memoryTypeCount; count++) {
			if ((type_filter&(1 << count)) && (mem_properties.memoryTypes[count].propertyFlags&properties) == properties)
				return count;
		}
	}
}