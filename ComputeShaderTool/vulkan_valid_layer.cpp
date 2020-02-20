#include"vulkan_valid_layer.h"
#include<GLFW\glfw3.h>
namespace VulkanUtil {
	VulkanValidLayer::VulkanValidLayer() :is_enable_(false) {
		//检查可用的校验层数量
		vkEnumerateInstanceLayerProperties(&layer_mount_, nullptr);

		available_layers_.resize(layer_mount_);
		//获取校验层
		vkEnumerateInstanceLayerProperties(&layer_mount_, available_layers_.data());
	}

	void VulkanValidLayer::CheckValidLayerAvaliable(const std::vector<const char*>& validation_layers) {
		bool layer_found = false;

		for (const char* layer_name : validation_layers) {
			bool layer_found = false;
			for (const auto& layer_properties : available_layers_) {
				//是否找到
				if (strcmp(layer_name, layer_properties.layerName) == 0)
				{
					layer_found = true;
					break;
				}

			}
			if (!layer_found)
			{
				is_supported_ = false;
				return;
			}
		}

		is_supported_ = true;
		if (is_supported_) {
			validation_layers_ = validation_layers;
		}
	}

	std::vector<const char*> VulkanValidLayer::GetRequiredExtensions() {
		/*uint32_t glfw_extension_count = 0;
		const char** glfw_extensions;
		glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
		//for(auto )
		if (is_enable_&&is_supported_) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;*/

		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions;
		//glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		//std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
		std::vector<const char*> extensions;
		//for(auto )
		//获取Instance支持的拓展
		uint32_t inst_ext_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &inst_ext_count, nullptr);

		// Enumerate the instance extensions
		VkExtensionProperties* inst_exts =
			(VkExtensionProperties *)malloc(inst_ext_count * sizeof(VkExtensionProperties));
		vkEnumerateInstanceExtensionProperties(nullptr, &inst_ext_count, inst_exts);
		uint32_t enabled_inst_ext_count = 0;

		for (uint32_t i = 0; i < inst_ext_count; i++) {
			log_util_.PrintLog(inst_exts[i].extensionName);
			if (strcmp(inst_exts[i].extensionName,
				VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0
				&& is_enable_&&is_supported_) {
				log_util_.PrintLog("Debug Util Extension");
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
		}
		return extensions;
	}

};