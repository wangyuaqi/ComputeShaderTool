#pragma once
#include"vulkan_tool.h"
#include"vulkan_device.h"
#include"vulkan_memory.h"
#include"vulkan_buffer.h"
namespace VulkanUtil {

	class VulkanSampler :public VulKanTool {
	public:
		VulkanSampler(std::shared_ptr<VulkanDevice> use_device, VkFilter = VK_FILTER_NEAREST,
			VkSamplerAddressMode mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
		~VulkanSampler();
		VkSampler& GetSampler() { return my_sampler_; }
	private:
		VkSampler my_sampler_;
		std::shared_ptr<VulkanDevice> now_device_;

	};
	class VulkanImage :public VulKanTool {
	public:
		VulkanImage(std::shared_ptr<VulkanDevice> use_device,std::shared_ptr<VulkanMemoryPool> use_pool,
			bool separate, std::vector<int> image_dim, VkFormat my_format);
		//释放创建的memory资源
		void Release();
		void CopyBufferToImage(std::shared_ptr<VulkanBuffer> image_buffer);
		void TransitionImageLayout(VkImageLayout new_layout);
		void CopyImageToBuffer(std::shared_ptr<VulkanBuffer>& image_buffer);
		VkImageView& GetImageView() { return my_image_view_; }
		VkImageLayout GetImageLayout() { return my_layout_; }

		~VulkanImage();
	private:
		std::shared_ptr<VulkanDevice> now_device_;
		std::shared_ptr<VulkanMemoryPool> now_pool_;
		std::vector<int> image_dims_;
		int image_width_;
		int image_height_;
		int image_depth_;
		VkImage my_image_;
		VulkanMemory *my_memory;
		VkImageView my_image_view_;
		bool my_release_;
		VkFormat my_format_;
		VkImageLayout my_layout_;
	private:
		//记录传输指令到指令缓冲
		VkCommandBuffer BeginSingleTimeCommands();
		//结束传输指令
		void EndSingleTimeCommands(VkCommandBuffer command_buffer);

		
	};
}
