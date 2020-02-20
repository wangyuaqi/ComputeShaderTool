#pragma once
#include"op_base.h"
namespace ImageProcess {
	class OpConvImage :public OpBase {
	public:
		struct ConvParam {
			int in_h;
			int in_w;
			int out_h;
			int out_w;
			int stride_h;
			int stride_w;
			int pad_h;
			int pad_w;
			int filter_h;
			int filter_w;
			int dilation_h;
			int dilation_w;
			int channels;
			int batch;
			int has_bias;
			int M;
			int K;
			int N;
			int basic_shader_batch_idx;
			int basic_shader_partition_idx;
			int basic_shader_partition_size;
		};
		struct ConvShaderConfig
		{
			int local_size_x;
			int local_size_y;
			int local_size_z;
			int block_height;
			int block_width;
			int block_depth;
		};
	public:
		OpConvImage();
		void InitOp(const int output_channels, const bool has_bias,
			const int *filter_size, const int *pad, const int *stride,
			const int *dilation, const int activation, const int group,
			const int padding_mode);
		bool Forward(std::vector<Tensor>& input_tensor,
			std::vector<Tensor>& blobs,
			std::vector<Tensor>& out_tensor)
		{
			return false;
		}
		bool Forward(std::vector<Tensor>& input_tensor,
			std::vector<Tensor>& blobs,
			std::vector<Tensor>& out_tensor,
			std::shared_ptr<VulkanUtil::VulkanMemoryPool> use_memory_pool
			);

		bool Forward(Tensor& input_tensor, Tensor& filter_weights, Tensor& bias, Tensor& out,
			std::shared_ptr<VulkanUtil::VulkanMemoryPool> use_memory_pool);

		int GetOutPutWidth() { return output_width_; }
		int GetOutPutHeight() { return output_height_; }

		void GetOutputData(std::vector<float>& output_data);
	private:
		bool has_bias_;
		int activation_;
		PaddingMode padding_mode_;

		int group_;
		ConvShaderConfig config_;

		std::vector<VkDescriptorType> descriptor_type_;
	private:
		//计算用于执行计算的线程的划分
		bool ComputeGroupSize();

	};
}
