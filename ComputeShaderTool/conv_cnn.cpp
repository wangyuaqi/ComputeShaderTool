#include"conv_cnn.h"
namespace ImageProcess {
	OpConv::OpConv() {
		support_vulkan_ = false;
		//完成初始化步骤
		config_.local_size_x = 256;
		config_.local_size_y = 1;
		config_.local_size_z = 1;
		config_.block_depth = 1;
		config_.block_height = 1;
		config_.block_width = 1;
	}

	void OpConv::InitOp(const int output_channels, const bool has_bias,
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
		std::string shader_path = "D://vulkan_project//ComputeShaderTool//ComputeShaderTool//conv_op.comp.spv";
		int buffer_num = 4;
		std::string application_name = "ConvOp";
		std::vector<VkDescriptorType> descriptor_type(buffer_num,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		InitVulkan(shader_path, application_name, buffer_num,descriptor_type);
	}
	bool OpConv::Forward(std::vector<Tensor>& input_tensor,
		std::vector<Tensor>& blobs,
		std::vector<Tensor>& out_tensor)
	{
		std::vector<int> shape = { 1 };
		Tensor bias(0, shape,my_device_);
		if (has_bias_) {
			assert(blobs.size() == 2);
			bias = blobs[1];
		}
		return Forward(input_tensor[0], blobs[0], bias, out_tensor[0]);

	}
	bool OpConv::Forward(Tensor& input_tensor, Tensor& filter_weights, Tensor& bias, Tensor& output_tensor)
	{
		Shape input_shape = input_tensor.getShape();
		Shape output_shape = output_tensor.getShape();

		batch_ = input_shape[ShapeIdxBatch];
		input_data_height_ = input_shape[ShapeIdxHeight];
		input_data_width_ = input_shape[ShapeIdxWidth];
		input_data_channels_ = input_shape[ShapeIdxChannel];
		
		output_height_ = output_shape[ShapeIdxHeight];
		output_width_ = output_shape[ShapeIdxWidth];

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
		my_shader_->BindBuffer(input_tensor.getBuffer(), 0);
		my_shader_->BindBuffer(bias.getBuffer(), 1);
		my_shader_->BindBuffer(filter_weights.getBuffer(), 2);
		my_shader_->BindBuffer(output_tensor.getBuffer(), 3);
		my_shader_->RecordCommandBuffer((void*)&conv_param, sizeof(ConvParam));
		//my_shader_->RunCommandBuffer();
		clock_t start, end;
		start = clock();
		//执行计算
		my_shader_->RunCommandBuffer();
		end = clock();
		double time_cost = double(end - start) / CLOCKS_PER_SEC;
		//std::cout << "Time Cost::" << time_cost << std::endl;//0.018ms
		//获取结果
		float *out_data = (float*)output_tensor.getBuffer()->Map();
		int begin_index = (output_height_-300) * output_width_;
		int end_index = begin_index + 30;
		for(int count = begin_index;count<end_index;count++)
			std::cout << out_data[count] << std::endl;
		output_tensor.getBuffer()->UnMap();
		return true;
	}

	
	bool OpConv::ComputeGroupSize()
	{
		//计算得到compute group
		group_x_ = AlignSize(output_width_*output_height_, config_.local_size_x) / config_.local_size_x;
		float GFLOPS = (2.0 * kernel_height_ * kernel_width_ * input_data_channels_ + 1) *
			(output_channels_ * output_height_ * output_width_) / 1000 / 1000 / 1000;
		const int max_group_count_y = 65535,max_compute_gflops=10;
		group_y_ = std::min(max_group_count_y, (int)floor(max_compute_gflops / (GFLOPS / output_channels_)));

		group_x_ = 1;
		group_y_ = 1;
		group_z_ = 1;

		return true;
	}
	
	
}