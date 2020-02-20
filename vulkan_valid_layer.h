#pragma once
#include"vulkan_tool.h"
namespace VulkanUtil {
	/*
	用来实现与vulkan的校验层相关的操作
	*/
	class VulkanValidLayer :public VulKanTool {
	public:
		bool is_enable_;//是否开启校验层使用
		bool is_supported_;//是否支持
	public:
		VulkanValidLayer();
		void CheckValidLayerAvaliable(const std::vector<const char*>& validation_layers);//可以不使用
		void SetIsEnable(bool is_enable) {
			is_enable_ = is_enable;
		}
		const bool GetSupported() const {
			return is_supported_;
		}
		std::vector<const char*> GetRequiredExtensions();//返回extension结果
		std::vector<const char*> GetValidationLayer() { return validation_layers_; };
	private:
		uint32_t layer_mount_;
		std::vector<VkLayerProperties> available_layers_;
		std::vector<const char*> validation_layers_;


	};
}
