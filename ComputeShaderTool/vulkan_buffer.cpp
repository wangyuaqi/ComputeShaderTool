#include"vulkan_buffer.h"

namespace VulkanUtil {
	VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> use_device) {
		my_device_ = use_device;
	}
	VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> use_device, size_t size_in_bytes, const char *data) {
		my_device_ = use_device;
		now_buffer_ = VK_NULL_HANDLE;
		now_buffer_memory_ = VK_NULL_HANDLE;
		//用于初始化data信息
		clock_t start, end;
		start = clock();
		Init(size_in_bytes, data);
		end = clock();
		double time_cost = double(end - start) / CLOCKS_PER_SEC;
		std::cout << "Copy Data to GPU Time Cost:" << time_cost << std::endl;
	}

	void* VulkanBuffer::Map()
	{
		void *p;
		log_util_.PrintResult("Map Memory", vkMapMemory(my_device_->GetDevice(), now_buffer_memory_, 0, byte_size_, 0, (void**)&p));

		return p;
	}
	void VulkanBuffer::UnMap() {
		vkUnmapMemory(my_device_->GetDevice(), now_buffer_memory_);
	}
	bool VulkanBuffer::Init(size_t size_in_bytes, const char *data)
	{
		if (now_buffer_ != VK_NULL_HANDLE)
		{
			printf("Warn: Buffer object already inited\n");
			return false;
		}
		byte_size_ = size_in_bytes;
		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size_in_bytes;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_SRC_BIT| VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		log_util_.PrintResult("Create Buffer",vkCreateBuffer(my_device_->GetDevice(), &bufferCreateInfo, NULL, &now_buffer_));

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(my_device_->GetDevice(), now_buffer_, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = my_device_->FindMemoryType(memoryRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		log_util_.PrintResult("Allocate Memory",vkAllocateMemory(my_device_->GetDevice(), &allocateInfo, NULL, &now_buffer_memory_));

		if (data)
		{
			char* dst;
			log_util_.PrintResult("Map Memory",vkMapMemory(my_device_->GetDevice(), now_buffer_memory_, 0, size_in_bytes, 0, (void **)&dst));
			memcpy(dst, data, size_in_bytes);
			vkUnmapMemory(my_device_->GetDevice(), now_buffer_memory_);
		}
		log_util_.PrintResult("Bind Buffer Memory",vkBindBufferMemory(my_device_->GetDevice(), now_buffer_, now_buffer_memory_, 0));
		return true;
	}
}