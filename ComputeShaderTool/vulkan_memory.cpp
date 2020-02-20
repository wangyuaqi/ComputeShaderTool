#include"vulkan_memory.h"

namespace VulkanUtil {
	VulkanMemory::VulkanMemory(std::shared_ptr<VulkanDevice> use_device, const VkMemoryAllocateInfo& info) {
		my_device_ = use_device;
		//分配资源
		my_device_->AllocaMemory(my_memory_, info);

		my_type_index_ = info.memoryTypeIndex;
		my_size_ = info.allocationSize;

	}
	VulkanMemory::~VulkanMemory() {
		my_device_->FreeMemory(my_memory_);
	}

	/*
	VulkanMemoryPool
	*/
	VulkanMemoryPool::VulkanMemoryPool(std::shared_ptr<VulkanDevice> use_device) {
		my_deivce_ = use_device;
		my_deivce_->GetPhysicalDeviceMemoryProperties(my_property_);
		my_free_buffers_.resize(my_property_.memoryTypeCount);//不同的memory类型

	}

	VulkanMemoryPool::~VulkanMemoryPool() {

	}

	VulkanMemory* VulkanMemoryPool::AllocateMemory(const VkMemoryRequirements& requirements,
		VkFlags extra_mask, bool seperate) {
		uint32_t index = 0;
		auto typeBits = requirements.memoryTypeBits;
		for (uint32_t i = 0; i < my_property_.memoryTypeCount; i++) {
			if ((typeBits & 1) == 1) {
				// Type is available, does it match user properties?
				if ((my_property_.memoryTypes[i].propertyFlags & extra_mask) == extra_mask) {
					index = i;
					break;
				}
			}
			typeBits >>= 1;
		}
		log_util_.LogAssert(index >= 0);
		if (!seperate) {
			auto freeIter = my_free_buffers_[index].lower_bound(requirements.size);
			if (freeIter != my_free_buffers_[index].end()) {
				VulkanMemory *result = freeIter->second;
				my_free_buffers_[index].erase(freeIter);
				return result;
			}
		}

		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.pNext = nullptr;
		alloc_info.allocationSize = requirements.size;
		alloc_info.memoryTypeIndex = index;
		/*VkMemoryAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
			.allocationSize = requirements.size,
			.memoryTypeIndex = index, // Memory type assigned in the next step
		};*/

		auto memory = std::make_shared<VulkanMemory>(my_deivce_, alloc_info);
		my_all_buffers_.insert(std::make_pair(memory.get(), memory));
		return memory.get();
	}
	void VulkanMemoryPool::ReturnMemory(VulkanMemory* memory, bool clean)
	{
		const VkDeviceSize memory_size = memory->size();
		if (!clean) {
			std::pair<VkDeviceSize,VulkanMemory*> data(memory_size, memory);
			my_free_buffers_[memory->type()].insert(data);
			return;
		}
		auto iter = my_all_buffers_.find(memory);
		if (iter != my_all_buffers_.end())
			my_all_buffers_.erase(iter);
		return;
	}

	void VulkanMemoryPool::Clear()
	{
		for (auto& iter : my_free_buffers_) {
			for (auto& sub_iter : iter) {
				auto erase_iter = my_all_buffers_.find(sub_iter.second);
				if (erase_iter != my_all_buffers_.end())
					my_all_buffers_.erase(erase_iter);
			}
			iter.clear();
		}
	}
	//换算为MB，进行计算
	float VulkanMemoryPool::ComputeSize() const {
		float total_size = 0;
		for (auto& iter : my_all_buffers_)
			total_size += (float)(iter.first->size());

		return total_size / pow(1024.0, 2);
	}
}