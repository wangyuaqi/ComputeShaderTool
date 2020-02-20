#pragma once
#include "vulkan_tool.h"
#include "vulkan_valid_layer.h"
namespace VulkanUtil {
	/*
	封装与Instance相关操作
	*/
	class VulkanInstance : public VulKanTool {
	public:
		VulkanInstance(const char *application_name, const char *engin_name,
			std::shared_ptr<VulkanValidLayer>& vulkan_validation_layer);
		~VulkanInstance();
		const VkResult EnumeratePhysicalDevices(uint32_t& physical_device_count, VkPhysicalDevice* physical_devices) const;
		const bool SupportVulkan() const;//检查当前的设备是否支持vulkan
		VkInstance Get() {
			return my_instance_;
		}
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		}
	private:
		bool my_owner_;
		VkInstance my_instance_;
		std::vector<VkExtensionProperties> vulkan_extensions_;
		bool enable_validation_layers_;
		VkDebugUtilsMessengerEXT debug_messenger_;
	private:

		void GetVulkanExtensions();
		void SetupDebugMessenger();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info);

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info,
			const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debug_messenger);

	};

}
