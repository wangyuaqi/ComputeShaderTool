#include"vulkan_image.h"

namespace VulkanUtil {
	VulkanSampler::VulkanSampler(std::shared_ptr<VulkanDevice> use_device, VkFilter filter,
		VkSamplerAddressMode mode) {
		use_device->CreateSampler(my_sampler_, filter, mode);
		now_device_ = use_device;
	}
	VulkanSampler::~VulkanSampler() {
		now_device_->DestroySampler(my_sampler_);
	}
	VulkanImage::VulkanImage(std::shared_ptr<VulkanDevice> use_device,std::shared_ptr<VulkanMemoryPool> use_pool, 
		bool separate, std::vector<int> dims, VkFormat my_format):my_release_(false){
		log_util_.LogAssert(dims.size() == 4);
		//log_util_.LogAssert(dims.size() >= 1 && dims.size() <= 4);
		now_device_ = use_device;
		now_pool_ = use_pool;
		auto image_type = VK_IMAGE_TYPE_1D;
		auto view_type = VK_IMAGE_VIEW_TYPE_1D;

		image_dims_ = dims;
		image_width_ = dims[2];
		/*image_height_ = 1;
		image_depth_ = 1;*/
		image_height_ = dims[3];
		image_depth_ = dims[1];

		/*if (dims.size() > 2)
		{
			image_height_ = dims[2];
			image_type = VK_IMAGE_TYPE_2D;
			view_type = VK_IMAGE_VIEW_TYPE_2D;
		}
		if (dims.size() > 3) {
			image_depth_ = dims[3];
			image_type = VK_IMAGE_TYPE_3D;
			view_type = VK_IMAGE_VIEW_TYPE_3D;
		}*/
		if (image_depth_ <= 1)
		{
			image_type = VK_IMAGE_TYPE_2D;
			view_type = VK_IMAGE_VIEW_TYPE_2D;
		}
		else {
			image_type = VK_IMAGE_TYPE_3D;
			view_type = VK_IMAGE_VIEW_TYPE_3D;
		}
		//使用fp16作为输入,这里可以设置一个变量选择，用于后续的不同的数据类型的选择
		my_format_ = my_format;
		my_layout_ = VK_IMAGE_LAYOUT_UNDEFINED;
		//创建一个VkImage资源
		use_device->CreateImage(my_image_, image_type, image_width_, image_height_, image_depth_, my_format_);

		VkMemoryRequirements memory_reqiurements;
		use_device->GetImageMemoryReqirements(my_image_, memory_reqiurements);
		//分配memory
		my_memory = now_pool_->AllocateMemory(memory_reqiurements, 0, separate);
		//将Image与对应的Memory进行绑定
		now_device_->BindImageMemory(my_image_, my_memory->get());
		//创建ImageView
		log_util_.PrintResult("Create Image View", now_device_->CreateImageView(my_image_view_, my_image_, view_type,
			my_format_));
	}
	VulkanImage::~VulkanImage() {
		//清除创建的image与imageview
		now_device_->DestroyImage(my_image_);
		now_device_->DestroyImageView(my_image_view_);
		if(!my_release_)
			now_pool_->ReturnMemory(my_memory);
	}

	//将数据从Buffer拷贝至Image中
	void VulkanImage::CopyBufferToImage(std::shared_ptr<VulkanBuffer> image_buffer)
	{
		VkCommandBuffer command_buffer = BeginSingleTimeCommands();
		//使用BufferCopy数据将数据复制到图像的部分区域信息
		VkBufferImageCopy region = {};
		region.bufferOffset = 0;//指定数据的偏移位置
		region.bufferRowLength = 0;//均设置为0，默认数据是紧凑排放的
		region.bufferImageHeight = 0;
		//指定缓冲被复制到数据的哪一部分
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0,0,0 };
		region.imageExtent = {
			static_cast<uint32_t>(image_width_),static_cast<uint32_t>(image_height_),static_cast<uint32_t>(image_depth_)
		};

		//执行Buffer的拷贝操作
		vkCmdCopyBufferToImage(command_buffer, (image_buffer->GetVkBuffer()), my_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		EndSingleTimeCommands(command_buffer);
	}

	void VulkanImage::CopyImageToBuffer(std::shared_ptr<VulkanBuffer>& image_buffer)
	{
		VkCommandBuffer command_buffer = BeginSingleTimeCommands();
		//使用BufferCopy数据将数据复制到图像的部分区域信息
		VkBufferImageCopy region = {};
		region.bufferOffset = 0;//指定数据的偏移位置
		region.bufferRowLength = 0;//均设置为0，默认数据是紧凑排放的
		region.bufferImageHeight = 0;
		//指定缓冲被复制到数据的哪一部分
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0,0,0 };
		region.imageExtent = {
			static_cast<uint32_t>(image_width_),static_cast<uint32_t>(image_height_),static_cast<uint32_t>(image_depth_)
		};

		//执行Buffer的拷贝操作
		
		//vkCmdCopyBufferToImage(command_buffer, image_buffer->GetVkBuffer(), my_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		vkCmdCopyImageToBuffer(command_buffer, my_image_, my_layout_, (image_buffer->GetVkBuffer()), 1, &region);
		EndSingleTimeCommands(command_buffer);
	}
	void VulkanImage::Release() {
		if (my_release_)
			return;
		my_release_ = true;
		now_pool_->ReturnMemory(my_memory);
	}

	VkCommandBuffer VulkanImage::BeginSingleTimeCommands()
	{
		//执行拷贝传输指令
		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = now_device_->GetCommandPool();//一般更好的是创建更好的command pool作为独立的内存传输指令的command pool
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(now_device_->GetDevice(), &alloc_info, &command_buffer);

		//记录内存传输指令
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;//这个指令缓冲只使用一次，不会循环绘制

		vkBeginCommandBuffer(command_buffer, &begin_info);

		return command_buffer;
	}

	void VulkanImage::EndSingleTimeCommands(VkCommandBuffer command_buffer)
	{
		//结束拷贝commandbuffer使用
		vkEndCommandBuffer(command_buffer);

		VkFence fence;
		VkFenceCreateInfo fence_create_info = {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = 0;

		//提交指令
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;
		//log_util_.PrintResult("End CommnadBuffer QueueSubmit", vkQueueSubmit(now_device_->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE));
		log_util_.PrintResult("End CommnadBuffer Create Fence", vkCreateFence(now_device_->GetDevice(), &fence_create_info, nullptr, &fence));
		log_util_.PrintResult("End CommnadBuffer QueueSubmit", vkQueueSubmit(now_device_->GetGraphicsQueue(), 1, &submit_info, fence));
		//log_util_.PrintResult("End CommnadBuffer vkQueueWaitIdle", vkQueueWaitIdle(now_device_->GetGraphicsQueue()));//完成拷贝指令的提交
		log_util_.PrintResult("End CommnadBuffer Wait Fence", vkWaitForFences(now_device_->GetDevice(), 1, &fence, VK_TRUE, 100000000000));
		vkDestroyFence(now_device_->GetDevice(), fence, nullptr);
		vkFreeCommandBuffers(now_device_->GetDevice(), now_device_->GetCommandPool(), 1, &command_buffer);//清除指令缓冲对象
	}

	void VulkanImage::TransitionImageLayout(VkImageLayout new_layout)
	{
		VkCommandBuffer command_buffer = BeginSingleTimeCommands();
		//通过图像内存屏障实现图像布局变换
		//管线屏障实现的资源同步，以及实现图像布局转换

		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags destination_stage;

		VkImageMemoryBarrier barrier = {};

		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;//创建内存屏障，作为实现
		barrier.oldLayout = my_layout_;
		barrier.newLayout = new_layout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;//不传递队列所有权

		barrier.image = my_image_;//设置图像对象
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;//不存在细分的级别
		barrier.subresourceRange.layerCount = 1;

		if (my_layout_ == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;//指定最早出现的管线阶段
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;//是一个伪阶段，出现在传输结算发生的时候
		}
		else if (my_layout_ == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			//图片数据需要在片段着色器阶段被使用
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;//片段着色器阶段
		}
		else if (new_layout == VK_IMAGE_LAYOUT_GENERAL)
		{
			/*barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			destination_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;//片段着色器阶段*/

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destination_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;//片段着色器阶段
		}
		else if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;//指定最早出现的管线阶段
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;//是一个伪阶段，出现在传输结算发生的时候
		}
		else
		{
			log_util_.PrintLog("UnSupported Layout Transition");
			throw std::runtime_error("Unsupported layoout transition");
		}
		//barrier.srcAccessMask = 0;//指定资源操作类型
		//barrier.dstAccessMask = 0;
		//提交Pipeline 屏障，实现对图像内存排布的变换
		vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		my_layout_ = new_layout;
		EndSingleTimeCommands(command_buffer);

	}
}