#include"op_base.h"
namespace ImageProcess {
	void OpBase::InitVulkan(const std::string shader_path, std::string application_name,
	const int buffer_num,std::vector<VkDescriptorType> my_descriptor_type)
	{
		my_valid_layer_ = std::make_shared<VulkanUtil::VulkanValidLayer>();
		my_valid_layer_->SetIsEnable(true);
		my_instance_ = std::make_shared<VulkanUtil::VulkanInstance>("Test", "Test", my_valid_layer_);
		support_vulkan_ = my_instance_->SupportVulkan();
		if (!support_vulkan_)
			return;
		my_device_ = std::make_shared<VulkanUtil::VulkanDevice>(my_instance_, my_valid_layer_);
		//初始化创建device。
		//std::string shader_path = "D://vulkan_project//ComputeShaderTool//ComputeShaderTool//median_filter.comp.spv";
		my_shader_ = std::make_shared<VulkanUtil::VulkanShader>(my_device_, shader_path);
		my_device_->CreateCommandPool();
		my_shader_->InitShader(buffer_num, my_descriptor_type);
	}

	void OpBase::ComputeConvOutputShape(PaddingMode padding_mode, int& padding_top, int& padding_left, const int& in_h, const int& in_w,
		const int& filter_h, const int& filter_w,
		int& out_h, int& out_w, int dilation_h , int dilation_w, int stride_h, int stride_w)
	{
		if (padding_mode == PaddingModeValid) {
			padding_top = 0;
			padding_left = 0;
			out_h = ceil((in_h - (filter_h - 1)*dilation_h) / stride_h);
			out_w = ceil((in_w - (filter_w - 1)*dilation_w) / stride_w);
		}
		else if (padding_mode == PaddingModeSame)
		{
			padding_top = ((filter_h - 1)*dilation_h + 1) / 2;
			padding_left = ((filter_w - 1)*dilation_w+1)/2;

			out_h = ceil(in_h / stride_h);
			out_w = ceil(in_w / stride_w);
		}
		else if (padding_mode == PaddingModeCaffe) {
			const int filter_actual_h = dilation_h*(filter_h - 1) + 1;
			const int filter_actual_w = dilation_w*(filter_w - 1) + 1;

			out_h = (in_h + 2 * padding_top - filter_actual_h) / stride_h + 1;
			out_w = (in_w + 2 * padding_left - filter_actual_w) / stride_w + 1;
		}
	}
}