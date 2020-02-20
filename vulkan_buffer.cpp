#include"vulkan_buffer.h"

namespace VulkanUtil {
	VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> use_device) {
		my_device_ = use_device;
	}
	VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> use_device, size_t size_in_bytes, const char *data) {

	}

	bool VulkanBuffer::Init(size_t size_in_bytes, const char *data)
	{
		if (now_buffer_ != VK_NULL_HANDLE)
		{
			printf("Warn: Buffer object already inited\n");
			return false;
		}

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size_in_bytes;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		log_util_.PrintResult("Create Buffer",vkCreateBuffer(my_device_->GetDevice(), &bufferCreateInfo, NULL, &now_buffer_));

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(my_device_->GetDevice(), now_buffer_, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(device_, &allocateInfo, NULL, &memory_));

		if (data)
		{
			char* dst;
			VK_CHECK_RESULT(vkMapMemory(device_, memory_, 0, size_in_bytes, 0, (void **)&dst));
			memcpy(dst, data, size_in_bytes);
			vkUnmapMemory(device_, memory_);
		}

		VK_CHECK_RESULT(vkBindBufferMemory(device_, buffer_, memory_, 0));
		return true;
	}
}