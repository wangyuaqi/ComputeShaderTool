#pragma once
#include"vulkan_tool.h"
#include"vulkan_instance.h"
#include"vulkan_valid_layer.h"
//#include"vulkan_surface.h"
//#include"vulkan_swap_chain.h"
namespace VulkanUtil {
	/*
	实现与vulkan的device相关操作
	*/
	class VulkanDevice : public VulKanTool {
	public:
		struct QueueFamilyIndices {
			int graghic_family = -1;
			int present_family = -1;//添加对于表现的支持，目前阶段不启动
			bool IsComplete(bool is_present) {
				if (is_present)
					return graghic_family >= 0 && present_family >= 0;
				else
					return graghic_family >= 0;
			}
		};
	public:
		VulkanDevice(std::shared_ptr<VulkanInstance>& vulkan_instance,
			std::shared_ptr<VulkanValidLayer>& vulkan_valid_layer);
		~VulkanDevice();

		void CreateCommandPool();

		void BindPipeline(uint32_t image_view_size, VkRenderPass& render_pass,
			std::vector<VkFramebuffer>& swap_frame_buffer, VkPipeline my_graphics_pipeline,
			VkExtent2D swap_chain_extent, VkBuffer& vertex_buffer, VkBuffer& indices_buffer,
			uint32_t data_size, uint32_t indices_size, std::vector<VkDescriptorSet>& descriptorSets,
			VkPipelineLayout& pipeline_layout);

		void CreateSemaphores(uint32_t frame_mount, uint32_t swap_frame_size);

		VkPhysicalDevice& GetPhysicalDevice() { return my_physical_device_; }

		VkDevice& GetDevice() { return my_deivce_; }
		QueueFamilyIndices GetFamiltyIndices() { return faimily_indices_; }
		//std::vector<VkSemaphore>& GetAvailabelSemaphore() { return image_available_semaphore_; }
		//std::vector<VkSemaphore>& GetRenderSemaphore() { return render_finished_semaphore_; }
		VkQueue& GetGraphicsQueue() { return my_graphics_queue_; }
		//VkQueue& GetPresentQueue() { return my_present_queue_; }
		std::vector<VkCommandBuffer>& GetCommandBuffers() { return my_command_buffer_; }

		VkCommandPool& GetCommandPool() { return my_command_pool_; }
		//std::vector<VkFence>& GetInFlightFence() { return inflight_fences_; }
		//std::vector<VkFence>& GetImageInFlight() { return images_inflight_; }
		//void CreateDescritorSetLayouot(uint32_t buffer_num);
		//void CreateDescriptorSet(uint32_t buffer_num);

		uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

		//获取需要多少资源
		void GetImageMemoryReqirements(const VkImage& image, VkMemoryRequirements& memory_requirements);
		//分配资源
		void AllocaMemory(VkDeviceMemory& memory, const VkMemoryAllocateInfo& allocate_info,
			const VkAllocationCallbacks* allocator = nullptr);
		//清除memory
		void FreeMemory(VkDeviceMemory& memory, const VkAllocationCallbacks* allocator = nullptr);

		void GetPhysicalDeviceMemoryProperties(VkPhysicalDeviceMemoryProperties& memory_properties) {
			vkGetPhysicalDeviceMemoryProperties(my_physical_device_, &memory_properties);
		}
		//void AllocateMemory(const )
		//与VulkanSampler相关
		void CreateSampler(VkSampler& now_sampler, const VkFilter filter = VK_FILTER_NEAREST,
			const VkSamplerAddressMode& mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, const VkAllocationCallbacks* allocator = nullptr);
		
		void DestroySampler(VkSampler& now_sampler);

		//与Image相关
		void  CreateImage(VkImage& now_image, const VkImageType image_type, const uint32_t image_width, uint32_t image_height,
			const uint32_t image_depth, const VkFormat image_format,
			const VkAllocationCallbacks* allocator = nullptr);
		//设置绑定内存
		void BindImageMemory(VkImage& use_image, const VkDeviceMemory& memory,
			const VkDeviceSize& memory_offset = 0);
		//创建ImageView
		const VkResult CreateImageView(VkImageView& view, const VkImage& image, const VkImageViewType& view_type,
			const VkFormat& format, const VkAllocationCallbacks* allocator = nullptr) const;

		void DestroyImage(VkImage& image);
		void DestroyImageView(VkImageView image_view);

		bool CleanUp();


	private:
		std::shared_ptr<VulkanInstance> my_instance_;
		std::shared_ptr<VulkanValidLayer> my_valid_layer_;

		VkPhysicalDevice my_physical_device_;
		VkDevice my_deivce_;
		VkQueue my_graphics_queue_;
		VkQueue my_present_queue_;
		VkCommandPool my_command_pool_;

		std::vector<VkSemaphore> image_available_semaphore_;
		std::vector<VkSemaphore> render_finished_semaphore_;
		std::vector<VkFence> inflight_fences_;
		std::vector<VkFence> images_inflight_;

		std::vector<VkCommandBuffer> my_command_buffer_;

		bool is_present_;

		QueueFamilyIndices faimily_indices_;

		uint32_t frame_mount_;



	private:
		//判断当前的physical device是否符合应用的要求
		bool DeviceSuitable(VkPhysicalDevice device);
		//对当前多有获取的physical device打分
		int RateDeviceSuitability(VkPhysicalDevice device);
		//获取physical device 对应的queue,需要支持graphic 
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice now_device);
		void CreateDevice(QueueFamilyIndices queue_family);

	};
};
