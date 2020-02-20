#include"vulkan_device.h"
#include<assert.h>
namespace VulkanUtil {
	VulkanDevice::VulkanDevice(std::shared_ptr<VulkanInstance>& vulkan_instance,
		std::shared_ptr<VulkanValidLayer>& vulkan_valid_layer) : my_valid_layer_(vulkan_valid_layer)
	{
		is_present_ = false;
		my_instance_ = vulkan_instance;
		//my_valid_layer_ = vulkan_valid_layer;

		my_physical_device_ = VK_NULL_HANDLE;
		my_deivce_ = VK_NULL_HANDLE;

		//��ȡ�����豸�б��Լ����Ӧ������
		uint32_t gpu_count = 0;
		log_util_.PrintResult("EnumeratePhysicalDevices", my_instance_->EnumeratePhysicalDevices(gpu_count, nullptr));
		if (gpu_count == 0) {
			log_util_.PrintLog("Non Device is avaliable");
		}
		assert(gpu_count != 0);
		std::vector<VkPhysicalDevice> tmp_gpus(gpu_count);
		log_util_.PrintResult("EnumeratePhysicalDevices", my_instance_->EnumeratePhysicalDevices(gpu_count, tmp_gpus.data()));
		assert(gpu_count != 0);
		for (auto tmp_gpu : tmp_gpus)
		{
			if (DeviceSuitable(tmp_gpu))
			{
				//��ȡ��һ�����㵱ǰ�ĳ���Ҫ���physical �豸
				my_physical_device_ = tmp_gpu;
				break;
			}
		}
		faimily_indices_ = FindQueueFamilies(my_physical_device_);
		std::cout << faimily_indices_.graghic_family << std::endl;
		CreateDevice(faimily_indices_);
		//��ȡ���߼��豸����Ҫ��ö�Ӧ�Ķ���

	}
	VulkanDevice::~VulkanDevice() {
		vkDestroyDevice(my_deivce_, nullptr);//���ٴ������߼�device
	}

	void VulkanDevice::CreateCommandPool() {
		/*
		����ָ��ض������ڹ���ָ�����ʹ�õ��ڴ�
		*/
		VkCommandPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = faimily_indices_.graghic_family;
		pool_info.flags = 0;

		VkResult create_command_pool_result = vkCreateCommandPool(my_deivce_, &pool_info, nullptr, &my_command_pool_);
		log_util_.PrintResult("Create Command Pool", create_command_pool_result);
		if (create_command_pool_result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool");
		}
	}
	void VulkanDevice::BindPipeline(uint32_t image_view_size, VkRenderPass& render_pass,
		std::vector<VkFramebuffer>& swap_frame_buffer, VkPipeline my_graphics_pipeline,
		VkExtent2D swap_chain_extent, VkBuffer& vertex_buffer, VkBuffer& indices_buffer, uint32_t vertex_data_size
		, uint32_t indices_size, std::vector<VkDescriptorSet>& descriptorSets, VkPipelineLayout& pipeline_layout)
	{


		//����ָ������
		my_command_buffer_.resize(image_view_size);

		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = my_command_pool_;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = (uint32_t)my_command_buffer_.size();

		VkResult create_command_buffer_result = vkAllocateCommandBuffers(my_deivce_, &alloc_info, my_command_buffer_.data());
		log_util_.PrintResult("Create Command Buffers", create_command_buffer_result);
		if (create_command_buffer_result != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}
		//��¼ָ�ָ���
		uint32_t command_size = my_command_buffer_.size();
		for (uint32_t command_count = 0; command_count < command_size; command_count++) {
			VkCommandBufferBeginInfo begin_info = {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;//ָ���������һ֡��û����Ⱦ��ɣ��Ϳ��Լ������ύ����߶��̲߳��жȡ�
			begin_info.pInheritanceInfo = nullptr;
			//����ָ������ϸ��
			VkResult begin_command_buffer_result = vkBeginCommandBuffer(my_command_buffer_[command_count], &begin_info);
			log_util_.PrintResult("Begin Command Buffer result", begin_command_buffer_result);
			if (begin_command_buffer_result != VK_SUCCESS)
				throw std::runtime_error("Begin Command Buffer Fail");

			VkRenderPassBeginInfo render_pass_info = {};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_info.renderPass = render_pass;
			render_pass_info.framebuffer = swap_frame_buffer[command_count];
			//ָ��֡�������
			render_pass_info.renderArea.offset = { 0,0 };
			render_pass_info.renderArea.extent = swap_chain_extent;

			//ָ����Ⱦ����
			VkClearValue clear_color = { 0.0f,0.0f,0.0f,1.0f };//ʹ�ú�ɫ��Ϊ���ֵ
			render_pass_info.clearValueCount = 1;
			render_pass_info.pClearValues = &clear_color;
			//���������Ҫ���д�����
			vkCmdBeginRenderPass(my_command_buffer_[command_count], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(my_command_buffer_[command_count], VK_PIPELINE_BIND_POINT_GRAPHICS, my_graphics_pipeline);
			/*
			��Ӷ���������֧��
			*/
			VkBuffer vertex_buffers[] = { vertex_buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(my_command_buffer_[command_count], 0, 1, vertex_buffers, offsets);
			//������������
			vkCmdBindIndexBuffer(my_command_buffer_[command_count], indices_buffer, 0, VK_INDEX_TYPE_UINT16);

			//3��ʾָ�����������㣬1��ʾ������ʵ����Ⱦ��0����ʾ��һ������ɫ��vertexIndex��ֵ��InstanceIndex��ʾ��ɫ������
			//vkCmdDraw(my_command_buffer_[command_count], 3, 1, 0, 0
			//vkCmdDraw(my_command_buffer_[command_count], vertex_data_size, 1, 0, 0);
			//��Ӷ�������֧��
			vkCmdBindDescriptorSets(my_command_buffer_[command_count], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptorSets[command_count], 0, nullptr);

			//���ڻ��ƹ��������ľ���
			vkCmdDrawIndexed(my_command_buffer_[command_count], indices_size, 1, 0, 0, 0);
			//������Ⱦ����
			vkCmdEndRenderPass(my_command_buffer_[command_count]);
			//������¼ָ�ָ���
			if (vkEndCommandBuffer(my_command_buffer_[command_count]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer");
			}

		}
	}


	void VulkanDevice::CreateSemaphores(uint32_t frame_mount, uint32_t swap_frame_size) {

		frame_mount_ = frame_mount;
		VkSemaphoreCreateInfo semaphore_info = {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;//����Ϊ�ѷ����ź�

		image_available_semaphore_.resize(frame_mount);
		render_finished_semaphore_.resize(frame_mount);
		inflight_fences_.resize(frame_mount);
		images_inflight_.resize(swap_frame_size, VK_NULL_HANDLE);
		for (uint32_t frame_count = 0; frame_count < frame_mount; frame_count++) {
			VkResult available_result = vkCreateSemaphore(my_deivce_, &semaphore_info, nullptr, &image_available_semaphore_[frame_count]);
			log_util_.PrintResult("Create Available Result", available_result);
			VkResult render_result = vkCreateSemaphore(my_deivce_, &semaphore_info, nullptr, &render_finished_semaphore_[frame_count]);
			log_util_.PrintResult("Render Semaphore Result", render_result);

			if (available_result != VK_SUCCESS) {
				throw std::runtime_error("Create available smaphore fail");
			}

			if (render_result != VK_SUCCESS)
			{
				throw std::runtime_error("Create render semaphore fail");
			}

			/*
			����դ��
			*/
			VkResult create_inflight_result = vkCreateFence(my_deivce_, &fence_info, nullptr, &inflight_fences_[frame_count]);
			log_util_.PrintResult("Create Inflight result", create_inflight_result);
			if (create_inflight_result != VK_SUCCESS)
			{
				throw std::runtime_error("Create fence fail");
			}
		}
	}
	uint32_t VulkanDevice::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
	{
		//��ѯ�����豸���õ��ڴ�����
		VkPhysicalDeviceMemoryProperties mem_properties;
		//memoryHeaps����ڴ���Դ��
		//�����������Ҫ��properties�Ƿ���ȫ��ͬ
		vkGetPhysicalDeviceMemoryProperties(my_physical_device_, &mem_properties);
		for (uint32_t count = 0; count < mem_properties.memoryTypeCount; count++) {
			if ((type_filter&(1 << count)) && (mem_properties.memoryTypes[count].propertyFlags&properties) == properties)
				return count;
		}
	}

	void VulkanDevice::GetImageMemoryReqirements(const VkImage& image, VkMemoryRequirements& memory_requirements)
	{
		//��ȡ��Ҫmemory��Դ
		vkGetImageMemoryRequirements(my_deivce_, image, &memory_requirements);
	}

	void VulkanDevice::FreeMemory(VkDeviceMemory& memory, const VkAllocationCallbacks* allocator) {
		vkFreeMemory(my_deivce_, memory, allocator);
	}
	void VulkanDevice::AllocaMemory(VkDeviceMemory& memory, const VkMemoryAllocateInfo& allocate_info,
		const VkAllocationCallbacks* allocator)
	{
		//���÷���Memory��Դ
		log_util_.PrintResult("Allocate Memory", vkAllocateMemory(my_deivce_, &allocate_info, allocator, &memory));
	}

	void VulkanDevice::CreateSampler(VkSampler& now_sampler, const VkFilter filter,
		const VkSamplerAddressMode& mode , const VkAllocationCallbacks* allocator)
	{
		VkSamplerCreateInfo samplerInfo;
		::memset(&samplerInfo, 0, sizeof(samplerInfo));
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = filter;
		samplerInfo.minFilter = filter;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.addressModeU = mode;
		samplerInfo.addressModeV = mode;
		samplerInfo.addressModeW = mode;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		log_util_.PrintResult("Create Sampler Result",vkCreateSampler(my_deivce_, &samplerInfo, allocator, &now_sampler));
	}

	void VulkanDevice::DestroySampler(VkSampler& now_sampler)
	{
		vkDestroySampler(my_deivce_, now_sampler,nullptr);
	}

	//��VkImage��ص�
	void  VulkanDevice::CreateImage(VkImage& now_image, const VkImageType image_type, const uint32_t image_width, uint32_t image_height,
		const uint32_t image_depth, const VkFormat image_format,
		const VkAllocationCallbacks* allocator)
	{
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = image_type;
		info.extent.width = image_width;
		info.extent.height = image_height;
		info.extent.depth = image_depth;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.format = image_format;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		info.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT| VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.pNext = nullptr;
		log_util_.PrintResult("Create Image",vkCreateImage(my_deivce_, &info, allocator, &now_image));
	}

	void VulkanDevice::BindImageMemory(VkImage& use_image, const VkDeviceMemory& memory,
		const VkDeviceSize& memory_offset)
	{
		//���ð󶨵�Image���Ѿ������Memory
		vkBindImageMemory(my_deivce_, use_image, memory, memory_offset);
	}

	const VkResult VulkanDevice::CreateImageView(VkImageView& view, const VkImage& image, const VkImageViewType& view_type,
		const VkFormat& format, const VkAllocationCallbacks* allocator) const
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = image;
		info.viewType = view_type;
		info.format = format;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;

		return vkCreateImageView(my_deivce_, &info, allocator, &view);
	}

	void VulkanDevice::DestroyImage(VkImage& image) {
		vkDestroyImage(my_deivce_, image, nullptr);
	}

	void VulkanDevice::DestroyImageView(VkImageView image_view) {
		vkDestroyImageView(my_deivce_, image_view, nullptr);
	}
	/*
	Private Function
	*/
	bool VulkanDevice::DeviceSuitable(VkPhysicalDevice now_device) {
		VkPhysicalDeviceProperties device_properties;//�����豸��properties
		VkPhysicalDeviceFeatures device_features;//�����豸��features

		vkGetPhysicalDeviceProperties(now_device, &device_properties);
		vkGetPhysicalDeviceFeatures(now_device, &device_features);
		//�жϵ�ǰ���Կ��Ƿ�֧����ɫ��

		QueueFamilyIndices indices = FindQueueFamilies(now_device);
		return indices.IsComplete(false) && device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&device_features.geometryShader;

	}

	int VulkanDevice::RateDeviceSuitability(VkPhysicalDevice now_device) {

		int device_score = 0;
		VkPhysicalDeviceProperties device_properties;//�����豸��properties
		VkPhysicalDeviceFeatures device_features;//�����豸��features

		vkGetPhysicalDeviceProperties(now_device, &device_properties);
		vkGetPhysicalDeviceFeatures(now_device, &device_features);

		if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			device_score += 1000;
		if (!device_features.geometryShader)
			device_score = 0;

		return device_score;
	}

	VulkanDevice::QueueFamilyIndices VulkanDevice::FindQueueFamilies(VkPhysicalDevice now_device)
	{
		QueueFamilyIndices indices;
		uint32_t queue_family_mount = 0;
		//��ȡ֧�ֵ�queue family����Ŀ
		vkGetPhysicalDeviceQueueFamilyProperties(now_device, &queue_family_mount, nullptr);
		std::vector<VkQueueFamilyProperties> queue_familes(queue_family_mount);
		vkGetPhysicalDeviceQueueFamilyProperties(now_device, &queue_family_mount, queue_familes.data());


		int queue_mount = 0;
		for (const auto& queue_family : queue_familes) {
			if (queue_family.queueCount > 0 && queue_family.queueFlags&VK_QUEUE_GRAPHICS_BIT) {
				indices.graghic_family = queue_mount;
			}
			/*if (is_present_) {
				//�ж��Ƿ�֧��present_support
				VkBool32 present_support;
				vkGetPhysicalDeviceSurfaceSupportKHR(now_device, queue_mount, my_surface_->Get(), &present_support);
				if (present_support) {
					indices.present_family = queue_mount;
				}

			}*/

			if (indices.IsComplete(is_present_))
				break;
			queue_mount++;
		}
		return indices;
	}

	void VulkanDevice::CreateDevice(QueueFamilyIndices queue_family_indices) {
		//ָ����Ҫ�����Ķ��У����п���һ�������ǿ��Զ��߳��ύcommand buffer
		//ָ��queue���ȼ�
		float queue_priority = 1.0f;
		std::vector<VkDeviceQueueCreateInfo> queue_create_info_list;
		/*queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.flags = 0;
		queue_create_info.pNext = nullptr;
		queue_create_info.queueFamilyIndex = queue_family_indices.graghic_family;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queue_priority;

		std::vector<VkDeviceQueueCreateInfo> queue_create_info_list;*/

		std::set<int> unique_queue_families;
		unique_queue_families.insert(queue_family_indices.graghic_family);
		if (is_present_)
			unique_queue_families.insert(queue_family_indices.present_family);
		for (int queue_family : unique_queue_families) {
			VkDeviceQueueCreateInfo queue_create_info;
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.flags = 0;
			queue_create_info.pNext = nullptr;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;

			queue_create_info_list.push_back(queue_create_info);
		}

		//ָ��������device������
		VkPhysicalDeviceFeatures device_features = {};

		//�����߼��豸
		VkDeviceCreateInfo create_info = {};//��Ҫ�ȷ����ڴ����
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		//create_info.pNext = nullptr;
		//create_info.flags = 0;
		create_info.pQueueCreateInfos = queue_create_info_list.data();
		create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_info_list.size());
		create_info.pEnabledFeatures = &device_features;
		//create_info.flags = 1;
		std::vector<const char*> deviceExtensionNames;
		deviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		create_info.enabledExtensionCount = 1;
		create_info.ppEnabledExtensionNames = deviceExtensionNames.data();
		if (my_valid_layer_->is_enable_ && my_valid_layer_->is_supported_) {
			//auto valid_layers = my_valid_layer_->GetValidationLayer();
			//create_info.enabledLayerCount = static_cast<uint32_t>(valid_layers.size());
			//create_info.ppEnabledLayerNames = valid_layers.data();
			create_info.enabledLayerCount = 0;
			create_info.ppEnabledLayerNames = nullptr;
		}
		else
		{
			create_info.enabledLayerCount = 0;
			//create_info.ppEnabledLayerNames = nullptr;
		}

		//�����߼�device
		VkResult device_result = vkCreateDevice(my_physical_device_, &create_info, nullptr, &my_deivce_);
		log_util_.PrintResult("CreateDevice", device_result);
		//��ȡ������Ⱦ��queue���о��
		vkGetDeviceQueue(my_deivce_, queue_family_indices.graghic_family, 0, &my_graphics_queue_);
		//��ȡ������ʾqueue
		if (is_present_)
			vkGetDeviceQueue(my_deivce_, queue_family_indices.present_family, 0, &my_present_queue_);
	}
}