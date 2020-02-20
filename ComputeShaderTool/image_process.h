#pragma once
#include"vulkan_shader.h"
#include"vulkan_buffer.h"
#include"vulkan_device.h"
#include"tensor.h"
#include<opencv2\opencv.hpp>
namespace ImageProcess {
	class ImageProcessBase {
	public:
		enum PaddingMode
		{
			PaddingModeValid,
			PaddingModeSame,
			PaddingModeCaffe,
		};
		/*virtual void InitVulkan(const std::string shader_path, std::string application_name, const int buffer_num);*/
		virtual void InitVulkan(const std::string shader_path, std::string application_name,
			const int buffer_num, std::vector<VkDescriptorType> my_descriptor_type);
		/*virtual void Process() = 0;
		virtual void ComputeConvOutputShape(PaddingMode& padding_mode, int& padding_top, int& padding_left, const int& in_h, const int& in_w,
			const int& filter_h, const int& filter_w, const int& dilation_h, const int& dilation_w, const int& stride_h, const int& stride_w,
			int& out_h, int& out_w);
		virtual bool Forward(std::vector<Tensor>& input_tensor,
			std::vector<Tensor>& blobs,
			std::vector<Tensor>& out_tensor) = 0;

		int AlignSize(int width, int local_size) {
			return width + local_size - 1;
		}*/
	protected:
		std::shared_ptr<VulkanUtil::VulkanValidLayer> my_valid_layer_;
		std::shared_ptr<VulkanUtil::VulkanInstance> my_instance_;
		std::shared_ptr<VulkanUtil::VulkanDevice> my_device_;
		std::shared_ptr<VulkanUtil::VulkanShader> my_shader_;

		std::shared_ptr<VulkanUtil::VulkanBuffer> input_buffer_;
		std::shared_ptr<VulkanUtil::VulkanBuffer> output_buffer_;
		bool support_vulkan_;

		int image_width_;
		int image_height_;

		int input_data_width_;
		int input_data_height_;
		int input_data_channels_;

		int output_width_;
		int output_height_;
		int output_channels_;

		int padding_top_;
		int padding_left_;

		int kernel_width_;
		int kernel_height_;

		int dilation_h_;
		int dilation_w_;

		int stride_h_;
		int stride_w_;

		int batch_;

		int group_x_;
		int group_y_;
		int group_z_;
	};
}