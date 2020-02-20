#include"vulkan_shader.h"

namespace VulkanUtil {

	VulkanShader::VulkanShader(std::shared_ptr<VulkanDevice>& use_device,std::string shader_path)
	{
		now_device_ = use_device;

		compute_shader_ = CreateShaderModule(shader_path);

		group_x_ = 1;
		group_y_ = 1;
		group_z_ = 1;

		//my_descriptor_type_ = 
	}

	void VulkanShader::InitShader(uint32_t buffer_num, std::vector<VkDescriptorType> descriptor_type)
	{
		//完成对shader的初始化
		my_descriptor_type_ = descriptor_type;
		CreateDescritorSetLayout(buffer_num);
		CreateDescriptorSet(buffer_num, my_descriptor_type_);
		CreateCommandBuffer();
	}
	//创建ComputeShader
	VkShaderModule& VulkanShader::CreateShaderModule(const std::string file_name) {
		std::ifstream spv_file(file_name, std::ios::ate | std::ios::binary);
		if (!spv_file.is_open())
			throw std::runtime_error("failed to open file");

		size_t file_size = (size_t)spv_file.tellg();
		std::vector<char> buffer(file_size);
		std::cout << "file size" << file_size << std::endl;
		spv_file.seekg(0);
		spv_file.read(buffer.data(), file_size);
		//spv_file.close();
		VkShaderModule shader_module;
		VkShaderModuleCreateInfo shader_info = {};
		shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_info.codeSize = buffer.size();
		shader_info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

		VkResult vertex_shader_result = vkCreateShaderModule(now_device_->GetDevice(), &shader_info, nullptr, &shader_module);
		log_util_.PrintResult("Create Vertex Shader Module", vertex_shader_result);
		if (vertex_shader_result != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module");
		}

		buffer.clear();
		return shader_module;
	}

	void VulkanShader::SetComputeParam(uint32_t group_x, uint32_t group_y, uint32_t group_z)
	{
		group_x_ = group_x;
		group_y_ = group_y;
		group_z_ = group_z;
	}
	void VulkanShader::CreateDescritorSetLayout(uint32_t buffer_num)
	{
		if (buffer_num <= 0)
			return;
		std::vector<VkDescriptorSetLayoutBinding> bindings(buffer_num);
		for (int count = 0; count < buffer_num; count++) {
			bindings[count].binding = count;
			//bindinggs[count].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[count].descriptorType = my_descriptor_type_[count];
			bindings[count].descriptorCount = 1;
			bindings[count].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[count].pImmutableSamplers = NULL;
		}
		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = buffer_num;
		info.pBindings = bindings.data();
		//创建DescriptorLayout
		log_util_.PrintResult("Create Descriptor Layout Result", vkCreateDescriptorSetLayout(now_device_->GetDevice(), &info, NULL, &my_descriptor_layout_));

	}
	//每一个shader都有对应的描述符绑定
	void VulkanShader::CreateDescriptorSet(uint32_t buffer_num, std::vector<VkDescriptorType> descriptor_type)
	{
		std::map<VkDescriptorType, uint32_t> descriptor_mount;
		//my_descriptor_sets_.resize(buffer_num);
		for (auto des_type : descriptor_type) {
			if (descriptor_mount.count(des_type) < 0)
			{
				descriptor_mount[des_type] = 1;
			}
			else {
				descriptor_mount[des_type] += 1;
			}
		}
		//需要首先遍历获取当前所有的descriptorType
		std::vector<VkDescriptorPoolSize> pool_size_set;
		for (auto des_type : descriptor_mount)
		{
			VkDescriptorPoolSize pool_size = {};
			//pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			pool_size.type = (des_type).first;
			pool_size.descriptorCount = (des_type).second;
			pool_size_set.push_back(pool_size);
		}
		/*for (auto des_type : descriptor_mount) {
			VkDescriptorPoolSize pool_size = {};
			//pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			pool_size.type = (des_type).first;
			pool_size.descriptorCount = (des_type).second;
		}*/
		//VkDescriptorPool descriptor_pool;
		VkDescriptorPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.maxSets = 1;
		info.poolSizeCount = 1;
		info.pPoolSizes = pool_size_set.data();
		//创建描述符Pool
		log_util_.PrintResult("Create Descriptor Pool", vkCreateDescriptorPool(now_device_->GetDevice(), &info, NULL, &my_descriptor_pool_));

		//my_descriptor_pool_[(des_type).first] = descriptor_pool;
		
		//for (int buffer_count = 0; buffer_count < buffer_num; buffer_count++)
		{
			VkDescriptorSetAllocateInfo allocate_info = {};
			allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocate_info.descriptorPool = my_descriptor_pool_;
			allocate_info.descriptorSetCount = 1;
			allocate_info.pSetLayouts = &my_descriptor_layout_;
			//为描述符分配内存
			log_util_.PrintResult("Create Allocate Info", vkAllocateDescriptorSets(now_device_->GetDevice(), &allocate_info, &(my_descriptor_set_)));
		}

		/*VkDescriptorPoolSize pool_size = {};
		//pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_size.type = descriptor_type;
		pool_size.descriptorCount = buffer_num;
		
		VkDescriptorPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.maxSets = 1;
		info.poolSizeCount = 1;
		info.pPoolSizes = &pool_size;
		//创建描述符Pool
		log_util_.PrintResult("Create Descriptor Pool", vkCreateDescriptorPool(now_device_->GetDevice(), &info, NULL, &my_descriptor_pool_));
		VkDescriptorSetAllocateInfo allocate_info = {};
		allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocate_info.descriptorPool = my_descriptor_pool_;
		allocate_info.descriptorSetCount = 1;
		allocate_info.pSetLayouts = &my_descriptor_layout_;
		//为描述符分配内存
		log_util_.PrintResult("Create Allocate Info", vkAllocateDescriptorSets(now_device_->GetDevice(), &allocate_info, &my_descriptor_set_));*/

	}

	void VulkanShader::CreateComputePipeline(size_t push_constants_size, VkSpecializationInfo& specialization_info)
	{
		
		//创建PipelineCache
		/*VkPipelineCacheCreateFlags flags = {};
		VkPipelineCacheCreateInfo pipeline_cache_info;
		pipeline_cache_info.pNext = NULL;
		pipeline_cache_info.initialDataSize = 0;
		pipeline_cache_info.pInitialData = nullptr;
		pipeline_cache_info.flags = flags;

		VkPipelineCache pipeline_cache;
		log_util_.PrintResult("Create Pipeline Cahce",
			vkCreatePipelineCache(now_device_->GetDevice(), &pipeline_cache_info, nullptr, &pipeline_cache));*/
		//********************************
		VkPipelineShaderStageCreateInfo stage_info = {};
		stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;//指定在哪一个阶段使用

		stage_info.module = compute_shader_;
		stage_info.pName = "main";//指定调用的着色器函数，对应void main()
		//if (specialization_info.empty())
		//	stage_info.pSpecializationInfo = nullptr;
		//else
		stage_info.pSpecializationInfo = nullptr;
		
		VkPushConstantRange push_constant_ranges= {};
		push_constant_ranges.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		push_constant_ranges.offset = 0;
		push_constant_ranges.size = push_constants_size;

		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		if (push_constants_size != 0) {
			pipeline_layout_create_info.pushConstantRangeCount = 1;
			pipeline_layout_create_info.pPushConstantRanges = &push_constant_ranges;
		}
		pipeline_layout_create_info.setLayoutCount = 1;
		pipeline_layout_create_info.pSetLayouts = &my_descriptor_layout_;

		log_util_.PrintResult("CreatePipelineLayout", vkCreatePipelineLayout(now_device_->GetDevice(), &pipeline_layout_create_info,
			nullptr, &my_pipeline_layout_));

		//创建Pipeline
		VkComputePipelineCreateInfo pipeline_create_info = {};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipeline_create_info.stage = stage_info;
		pipeline_create_info.layout = my_pipeline_layout_;

		log_util_.PrintResult("Create Pipeline", vkCreateComputePipelines(now_device_->GetDevice(), VK_NULL_HANDLE,1,&pipeline_create_info,NULL,&my_compute_pipeline_));
		//完成创建pipeline

	}

	void VulkanShader::BindBuffer(std::shared_ptr<VulkanBuffer> use_buffer, int binding) {
		VkDescriptorBufferInfo desc_info = {};
		desc_info.buffer = (use_buffer->GetVkBuffer());
		desc_info.offset = 0;
		desc_info.range = use_buffer->GetSize();

		VkWriteDescriptorSet write_desscriptor_set = {};
		write_desscriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_desscriptor_set.dstSet = my_descriptor_set_;
		write_desscriptor_set.dstBinding = binding;
		write_desscriptor_set.descriptorCount = 1;
		write_desscriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write_desscriptor_set.pBufferInfo = &desc_info;
		vkUpdateDescriptorSets(now_device_->GetDevice(), 1, &write_desscriptor_set, 0, nullptr);
	}

	//绑定Image对象
	void VulkanShader::BindImage(std::shared_ptr<VulkanImage> use_image, int binding,
		VkImageLayout image_layout,VkDescriptorType descriptor_type, std::shared_ptr<VulkanSampler> use_sampler)
	{
		VkDescriptorImageInfo image_info = {};
		image_info.imageLayout = image_layout;
		image_info.imageView = use_image->GetImageView();
		if (use_sampler != nullptr)
			image_info.sampler = use_sampler->GetSampler();
		else
			image_info.sampler = nullptr;

		VkWriteDescriptorSet write_descriptorset = {};
		write_descriptorset.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//write_descriptorset.dstSet = my_descriptor_sets_[descriptor_index];
		write_descriptorset.dstSet = my_descriptor_set_;
		write_descriptorset.dstBinding = binding;
		write_descriptorset.dstArrayElement = 0;
		write_descriptorset.descriptorType = descriptor_type;
		write_descriptorset.descriptorCount = 1;
		write_descriptorset.pImageInfo = &image_info;
		vkUpdateDescriptorSets(now_device_->GetDevice(), 1, &write_descriptorset, 0, nullptr);
	}
	void VulkanShader::CreateCommandBuffer() 
	{
		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = now_device_->GetCommandPool();
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = 1;
		//一个shader对应一个CommandBuffer
		log_util_.PrintResult("Allocate CommandBuffer", vkAllocateCommandBuffers(now_device_->GetDevice(), &alloc_info, &my_command_buffer_));
	}

	void VulkanShader::RecordCommandBuffer(void *push_constants, size_t push_constant_size)
	{
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;//指令可以在上一帧还没有渲染完成，就可以继续被提交，提高多线程并行度。

		log_util_.PrintResult("Begin Command Buffer", vkBeginCommandBuffer(my_command_buffer_, &begin_info));
		//是否需要需要传入参数
		if (push_constants) {
			vkCmdPushConstants(my_command_buffer_, my_pipeline_layout_, VK_SHADER_STAGE_COMPUTE_BIT, 0, push_constant_size, push_constants);
		}
		vkCmdBindPipeline(my_command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, my_compute_pipeline_);
		vkCmdBindDescriptorSets(my_command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, my_pipeline_layout_, 0, 1,
			&my_descriptor_set_, 0, nullptr);
		vkCmdDispatch(my_command_buffer_, group_x_, group_y_, group_z_);
		log_util_.PrintResult("End Command Buffer", vkEndCommandBuffer(my_command_buffer_));
	}

	void VulkanShader::RunCommandBuffer()
	{
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &my_command_buffer_;

		//创建栅栏
		VkFence fence;
		VkFenceCreateInfo fence_create_info = {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = 0;
		clock_t start, end;
		start = clock();
		log_util_.PrintResult("Create Fence", vkCreateFence(now_device_->GetDevice(), &fence_create_info, nullptr, &fence));
		log_util_.PrintResult("Submite Command Buffer", vkQueueSubmit(now_device_->GetGraphicsQueue(), 1, &submit_info, fence));

		log_util_.PrintResult("Wait Fence",vkWaitForFences(now_device_->GetDevice(), 1, &fence, VK_TRUE, 100000000000));
		end = clock();
		double time_cost = double(end - start) / CLOCKS_PER_SEC;
		std::cout << "Time Cost1"<<time_cost << std::endl;
		vkDestroyFence(now_device_->GetDevice(), fence, nullptr);
	}

}