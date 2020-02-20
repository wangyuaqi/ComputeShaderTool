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
		//ʹ��fp16��Ϊ����,�����������һ������ѡ�����ں����Ĳ�ͬ���������͵�ѡ��
		my_format_ = my_format;
		my_layout_ = VK_IMAGE_LAYOUT_UNDEFINED;
		//����һ��VkImage��Դ
		use_device->CreateImage(my_image_, image_type, image_width_, image_height_, image_depth_, my_format_);

		VkMemoryRequirements memory_reqiurements;
		use_device->GetImageMemoryReqirements(my_image_, memory_reqiurements);
		//����memory
		my_memory = now_pool_->AllocateMemory(memory_reqiurements, 0, separate);
		//��Image���Ӧ��Memory���а�
		now_device_->BindImageMemory(my_image_, my_memory->get());
		//����ImageView
		log_util_.PrintResult("Create Image View", now_device_->CreateImageView(my_image_view_, my_image_, view_type,
			my_format_));
	}
	VulkanImage::~VulkanImage() {
		//���������image��imageview
		now_device_->DestroyImage(my_image_);
		now_device_->DestroyImageView(my_image_view_);
		if(!my_release_)
			now_pool_->ReturnMemory(my_memory);
	}

	//�����ݴ�Buffer������Image��
	void VulkanImage::CopyBufferToImage(std::shared_ptr<VulkanBuffer> image_buffer)
	{
		VkCommandBuffer command_buffer = BeginSingleTimeCommands();
		//ʹ��BufferCopy���ݽ����ݸ��Ƶ�ͼ��Ĳ���������Ϣ
		VkBufferImageCopy region = {};
		region.bufferOffset = 0;//ָ�����ݵ�ƫ��λ��
		region.bufferRowLength = 0;//������Ϊ0��Ĭ�������ǽ����ŷŵ�
		region.bufferImageHeight = 0;
		//ָ�����屻���Ƶ����ݵ���һ����
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0,0,0 };
		region.imageExtent = {
			static_cast<uint32_t>(image_width_),static_cast<uint32_t>(image_height_),static_cast<uint32_t>(image_depth_)
		};

		//ִ��Buffer�Ŀ�������
		vkCmdCopyBufferToImage(command_buffer, (image_buffer->GetVkBuffer()), my_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		EndSingleTimeCommands(command_buffer);
	}

	void VulkanImage::CopyImageToBuffer(std::shared_ptr<VulkanBuffer>& image_buffer)
	{
		VkCommandBuffer command_buffer = BeginSingleTimeCommands();
		//ʹ��BufferCopy���ݽ����ݸ��Ƶ�ͼ��Ĳ���������Ϣ
		VkBufferImageCopy region = {};
		region.bufferOffset = 0;//ָ�����ݵ�ƫ��λ��
		region.bufferRowLength = 0;//������Ϊ0��Ĭ�������ǽ����ŷŵ�
		region.bufferImageHeight = 0;
		//ָ�����屻���Ƶ����ݵ���һ����
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0,0,0 };
		region.imageExtent = {
			static_cast<uint32_t>(image_width_),static_cast<uint32_t>(image_height_),static_cast<uint32_t>(image_depth_)
		};

		//ִ��Buffer�Ŀ�������
		
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
		//ִ�п�������ָ��
		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = now_device_->GetCommandPool();//һ����õ��Ǵ������õ�command pool��Ϊ�������ڴ洫��ָ���command pool
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(now_device_->GetDevice(), &alloc_info, &command_buffer);

		//��¼�ڴ洫��ָ��
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;//���ָ���ֻʹ��һ�Σ�����ѭ������

		vkBeginCommandBuffer(command_buffer, &begin_info);

		return command_buffer;
	}

	void VulkanImage::EndSingleTimeCommands(VkCommandBuffer command_buffer)
	{
		//��������commandbufferʹ��
		vkEndCommandBuffer(command_buffer);

		VkFence fence;
		VkFenceCreateInfo fence_create_info = {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = 0;

		//�ύָ��
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;
		//log_util_.PrintResult("End CommnadBuffer QueueSubmit", vkQueueSubmit(now_device_->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE));
		log_util_.PrintResult("End CommnadBuffer Create Fence", vkCreateFence(now_device_->GetDevice(), &fence_create_info, nullptr, &fence));
		log_util_.PrintResult("End CommnadBuffer QueueSubmit", vkQueueSubmit(now_device_->GetGraphicsQueue(), 1, &submit_info, fence));
		//log_util_.PrintResult("End CommnadBuffer vkQueueWaitIdle", vkQueueWaitIdle(now_device_->GetGraphicsQueue()));//��ɿ���ָ����ύ
		log_util_.PrintResult("End CommnadBuffer Wait Fence", vkWaitForFences(now_device_->GetDevice(), 1, &fence, VK_TRUE, 100000000000));
		vkDestroyFence(now_device_->GetDevice(), fence, nullptr);
		vkFreeCommandBuffers(now_device_->GetDevice(), now_device_->GetCommandPool(), 1, &command_buffer);//���ָ������
	}

	void VulkanImage::TransitionImageLayout(VkImageLayout new_layout)
	{
		VkCommandBuffer command_buffer = BeginSingleTimeCommands();
		//ͨ��ͼ���ڴ�����ʵ��ͼ�񲼾ֱ任
		//��������ʵ�ֵ���Դͬ�����Լ�ʵ��ͼ�񲼾�ת��

		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags destination_stage;

		VkImageMemoryBarrier barrier = {};

		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;//�����ڴ����ϣ���Ϊʵ��
		barrier.oldLayout = my_layout_;
		barrier.newLayout = new_layout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;//�����ݶ�������Ȩ

		barrier.image = my_image_;//����ͼ�����
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;//������ϸ�ֵļ���
		barrier.subresourceRange.layerCount = 1;

		if (my_layout_ == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;//ָ��������ֵĹ��߽׶�
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;//��һ��α�׶Σ������ڴ�����㷢����ʱ��
		}
		else if (my_layout_ == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			//ͼƬ������Ҫ��Ƭ����ɫ���׶α�ʹ��
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;//Ƭ����ɫ���׶�
		}
		else if (new_layout == VK_IMAGE_LAYOUT_GENERAL)
		{
			/*barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			destination_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;//Ƭ����ɫ���׶�*/

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destination_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;//Ƭ����ɫ���׶�
		}
		else if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;//ָ��������ֵĹ��߽׶�
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;//��һ��α�׶Σ������ڴ�����㷢����ʱ��
		}
		else
		{
			log_util_.PrintLog("UnSupported Layout Transition");
			throw std::runtime_error("Unsupported layoout transition");
		}
		//barrier.srcAccessMask = 0;//ָ����Դ��������
		//barrier.dstAccessMask = 0;
		//�ύPipeline ���ϣ�ʵ�ֶ�ͼ���ڴ��Ų��ı任
		vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		my_layout_ = new_layout;
		EndSingleTimeCommands(command_buffer);

	}
}