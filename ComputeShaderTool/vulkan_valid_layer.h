#pragma once
#include"vulkan_tool.h"
namespace VulkanUtil {
	/*
	����ʵ����vulkan��У�����صĲ���
	*/
	class VulkanValidLayer :public VulKanTool {
	public:
		bool is_enable_;//�Ƿ���У���ʹ��
		bool is_supported_;//�Ƿ�֧��
	public:
		VulkanValidLayer();
		void CheckValidLayerAvaliable(const std::vector<const char*>& validation_layers);//���Բ�ʹ��
		void SetIsEnable(bool is_enable) {
			is_enable_ = is_enable;
		}
		const bool GetSupported() const {
			return is_supported_;
		}
		std::vector<const char*> GetRequiredExtensions();//����extension���
		std::vector<const char*> GetValidationLayer() { return validation_layers_; };
	private:
		uint32_t layer_mount_;
		std::vector<VkLayerProperties> available_layers_;
		std::vector<const char*> validation_layers_;


	};
}
