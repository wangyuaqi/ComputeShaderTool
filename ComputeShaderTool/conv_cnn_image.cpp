#include"conv_cnn_image.h"
namespace ImageProcess {
	OpConvImage::OpConvImage() {
		support_vulkan_ = false;
		//完成初始化步骤
		config_.local_size_x = 256;
		config_.local_size_y = 1;
		config_.local_size_z = 1;
		config_.block_depth = 1;
		config_.block_height = 1;
		config_.block_width = 1;
	}

	void OpConvImage::InitOp(const int output_channels, const bool has_bias,
		const int *filter_size, const int *pad, const int *stride,
		const int *dilation, const int activation, const int group,
		const int padding_mode)
	{
		output_channels_ = output_channels;
		has_bias_ = has_bias;
		kernel_width_ = filter_size[0];
		kernel_height_ = filter_size[1];

		padding_left_ = pad[0];
		padding_top_ = pad[1];

		stride_w_ = stride[0];
		stride_h_ = stride[1];

		dilation_w_ = dilation[0];
		dilation_h_ = dilation[1];

		activation_ = activation;

		group_ = group;
		padding_mode_ = (PaddingMode)padding_mode;
		//完成Vulkan 初始化
		std::string shader_path = "D://vulkan_project//ComputeShaderTool//ComputeShaderTool//conv_op_1.comp.spv";
		int buffer_num = 4;
		std::string application_name = "ConvOp";
		descriptor_type_.resize(buffer_num);
		descriptor_type_[0] = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_type_[1] = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_type_[2] = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_type_[3] = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		InitVulkan(shader_path, application_name, buffer_num, descriptor_type_);
	}
	bool OpConvImage::Forward(std::vector<Tensor>& input_tensor,
		std::vector<Tensor>& blobs,
		std::vector<Tensor>& out_tensor,
		std::shared_ptr<VulkanUtil::VulkanMemoryPool> use_memory_pool)
	{
		std::vector<int> shape = { 1 };
		Tensor bias(0, shape, my_device_);
		if (has_bias_) {
			assert(blobs.size() == 2);
			bias = blobs[1];
		}
		return Forward(input_tensor[0], blobs[0], bias, out_tensor[0],use_memory_pool);

	}
	bool OpConvImage::Forward(Tensor& input_tensor, Tensor& filter_weights, Tensor& bias, Tensor& output_tensor,
		std::shared_ptr<VulkanUtil::VulkanMemoryPool> use_memory_pool)
	{
		Shape input_shape = input_tensor.getShape();
		Shape output_shape = output_tensor.getShape();

		batch_ = input_shape[ShapeIdxBatch];
		input_data_height_ = input_shape[ShapeIdxHeight];
		input_data_width_ = input_shape[ShapeIdxWidth];
		input_data_channels_ = input_shape[ShapeIdxChannel];

		output_height_ = output_shape[ShapeIdxHeight];
		output_width_ = output_shape[ShapeIdxWidth];
		output_channels_ = output_shape[ShapeIdxChannel];
		int M = output_height_*output_width_;
		int K = kernel_height_*kernel_width_*input_data_channels_;
		int N = output_channels_;

		ConvParam conv_param = {};
		conv_param.basic_shader_batch_idx = 0;
		conv_param.basic_shader_partition_idx = 0;
		conv_param.basic_shader_partition_size = 0;
		conv_param.batch = batch_;
		conv_param.channels = input_data_channels_;
		conv_param.dilation_h = dilation_h_;
		conv_param.dilation_w = dilation_w_;
		conv_param.filter_h = kernel_height_;
		conv_param.filter_w = kernel_width_;
		conv_param.has_bias = has_bias_;
		conv_param.in_h = input_data_height_;
		conv_param.in_w = input_data_width_;
		conv_param.K = K;
		conv_param.M = M;
		conv_param.N = N;
		conv_param.out_h = output_height_;
		conv_param.out_w = output_width_;
		conv_param.pad_h = padding_top_;
		conv_param.pad_w = padding_left_;
		conv_param.stride_h = stride_h_;
		conv_param.stride_w = stride_w_;

		//计算ComputeShader的Group参数
		ComputeGroupSize();
		my_shader_->SetComputeParam(group_x_, group_y_, group_z_);
		VkSpecializationInfo spec_info = {};
		my_shader_->CreateComputePipeline(sizeof(ConvParam), spec_info);

		//完成数据与DescriptorSet的绑定
		size_t input_data_size = input_tensor.size();
		size_t out_data_size = output_tensor.size();
		size_t kernel_size = filter_weights.size();
		size_t bias_size = bias.size();
		
		std::vector<int> input_tensor_shape = input_tensor.getShape();
		std::vector<int> bias_shape = bias.getShape();
		std::vector<int> filter_weights_shape = filter_weights.getShape();
		std::vector<int> output_tensor_shape = output_tensor.getShape();

		//std::vector<int> input_image_shape()
		/*将tensor 转为image*/

		//VkFormat image_format = VK_FORMAT_R32G32B32A32_SFLOAT;
		VkFormat image_format = VK_FORMAT_R32_SFLOAT;
		std::shared_ptr<VulkanUtil::VulkanImage> input_image = std::make_shared<VulkanUtil::VulkanImage>(my_device_, 
			use_memory_pool, true,input_tensor.getShape(), image_format);
		
		std::shared_ptr<VulkanUtil::VulkanImage> filter_weights_image = std::make_shared<VulkanUtil::VulkanImage>(my_device_,
			use_memory_pool, true, filter_weights.getShape(), image_format);
		std::shared_ptr<VulkanUtil::VulkanImage> bias_image = std::make_shared<VulkanUtil::VulkanImage>(my_device_,
			use_memory_pool, true,bias.getShape(), image_format);
		std::shared_ptr<VulkanUtil::VulkanImage> output_image = std::make_shared<VulkanUtil::VulkanImage>(my_device_,
			use_memory_pool, true,output_tensor.getShape(), image_format);

		
		VkImageLayout copy_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		VkImageLayout read_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkImageLayout write_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkImageLayout update_layout = VK_IMAGE_LAYOUT_GENERAL;
		//VkImageLayout update_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		
		//转换内存layout,可以拷贝数据。
		input_image->TransitionImageLayout(copy_layout);
		filter_weights_image->TransitionImageLayout(copy_layout);
		bias_image->TransitionImageLayout(copy_layout);
		output_image->TransitionImageLayout(copy_layout);

		input_image->CopyBufferToImage(input_tensor.getBuffer());
		filter_weights_image->CopyBufferToImage(filter_weights.getBuffer());
		bias_image->CopyBufferToImage(bias.getBuffer());
		//output_image->CopyBufferToImage(output_tensor.getBuffer());
		
		input_image->TransitionImageLayout(update_layout);
		filter_weights_image->TransitionImageLayout(update_layout);
		bias_image->TransitionImageLayout(update_layout);
		output_image->TransitionImageLayout(update_layout);
		/*input_image->TransitionImageLayout(read_layout);
		filter_weights_image->TransitionImageLayout(read_layout);
		bias_image->TransitionImageLayout(read_layout);
		output_image->TransitionImageLayout(write_layout);*/

		//VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		std::shared_ptr<VulkanUtil::VulkanSampler> my_sampler = std::make_shared<VulkanUtil::VulkanSampler>(my_device_);
		my_shader_->BindImage(input_image, 0,input_image->GetImageLayout(), descriptor_type_[0], my_sampler);
		my_shader_->BindImage(bias_image, 1, bias_image->GetImageLayout(), descriptor_type_[1], my_sampler);
		my_shader_->BindImage(filter_weights_image, 2, filter_weights_image->GetImageLayout(), descriptor_type_[2], my_sampler);
		my_shader_->BindImage(output_image, 3, output_image->GetImageLayout(), descriptor_type_[3], my_sampler);

		/*input_image->TransitionImageLayout(read_layout);
		filter_weights_image->TransitionImageLayout(read_layout);
		bias_image->TransitionImageLayout(read_layout);
		output_image->TransitionImageLayout(write_layout);*/
		/*my_shader_->BindBuffer(input_tensor.getBuffer(), 0);
		my_shader_->BindBuffer(bias.getBuffer(), 1);
		my_shader_->BindBuffer(filter_weights.getBuffer(), 2);
		my_shader_->BindBuffer(output_tensor.getBuffer(), 3);*/
		my_shader_->RecordCommandBuffer((void*)&conv_param, sizeof(ConvParam));
		//my_shader_->RunCommandBuffer();
		clock_t start, end;
		start = clock();
		//执行计算
		my_shader_->RunCommandBuffer();
		end = clock();
		double time_cost = double(end - start) / CLOCKS_PER_SEC;
		std::cout << "Time Cost::" << time_cost << std::endl;//0.018ms
															 //获取结果
		VkImageLayout src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		output_image->TransitionImageLayout(src_layout);
		output_image->CopyImageToBuffer(output_tensor.getBuffer());
		/*input_image->TransitionImageLayout(src_layout);
		input_image->CopyImageToBuffer(input_tensor.getBuffer());*/
		float *out_data = (float*)output_tensor.getBuffer()->Map();
		//float *out_data = (float*)input_tensor.getBuffer()->Map();
		int begin_index = 0;
		int end_index = output_shape[0] * output_shape[1] * output_shape[2] * output_shape[3];
		
		int sum_count = 0;
		for (int count = begin_index; count < end_index; count++)
		{
			if (out_data[count] > 1)
				sum_count++;
				//std::cout << out_data[count]<<","<<count << std::endl;
		}
		std::cout << sum_count << std::endl;
		output_tensor.getBuffer()->UnMap();
		return true;
	}


	bool OpConvImage::ComputeGroupSize()
	{
		//计算得到compute group
		group_x_ = AlignSize(output_width_*output_height_, config_.local_size_x) / config_.local_size_x;
		group_y_ = AlignSize(output_channels_, config_.local_size_y) / config_.local_size_y;
		/*float GFLOPS = (2.0 * kernel_height_ * kernel_width_ * input_data_channels_ + 1) *
			(output_channels_ * output_height_ * output_width_) / 1000 / 1000 / 1000;
		const int max_group_count_y = 65535, max_compute_gflops = 10;
		group_y_ = std::min(max_group_count_y, (int)floor(max_compute_gflops / (GFLOPS / output_channels_)));

		//group_x_ = 1;
		group_y_ = 1;*/
		group_z_ = 1;

		return true;
	}


}