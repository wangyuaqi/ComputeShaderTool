#include"image_process.h"
namespace ImageProcess {
	/*void ImageProcessBase::InitVulkan(const std::string shader_path, std::string application_name, const int buffer_num)
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
		//my_shader_->InitShader(buffer_num, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	}*/

	void ImageProcessBase::InitVulkan(const std::string shader_path, std::string application_name,
		const int buffer_num, std::vector<VkDescriptorType> my_descriptor_type)
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
}