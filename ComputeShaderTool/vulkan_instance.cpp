#include"vulkan_instance.h"
namespace VulkanUtil {
	VulkanInstance::VulkanInstance(const char *application_name, const char *engine_name,
		std::shared_ptr<VulkanValidLayer>& vulkan_validation_layer) :my_owner_(true), my_instance_(VK_NULL_HANDLE), enable_validation_layers_(false) {

		//获取vulkan支持的扩展列表
		GetVulkanExtensions();

		//创建 app_info
		VkApplicationInfo app_info;

		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = nullptr;
		app_info.pApplicationName = application_name;
		app_info.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
		app_info.pEngineName = engine_name;
		app_info.engineVersion = VK_MAKE_VERSION(1, 1, 0);
		app_info.apiVersion = VK_MAKE_VERSION(1, 1, 0);
		const std::vector<const char*> validation_layers = {
			"VK_LAYER_LUNARG_standard_validation"
		};
		VkInstanceCreateInfo instance_create_info;
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pNext = nullptr;
		instance_create_info.flags = 0;
		instance_create_info.pApplicationInfo = &app_info;

		//目前设备不支持校验层。
		vulkan_validation_layer->CheckValidLayerAvaliable(validation_layers);
		auto extensions = vulkan_validation_layer->GetRequiredExtensions();
		instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instance_create_info.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debug_createinfo;
		if (vulkan_validation_layer->is_enable_&&vulkan_validation_layer->GetSupported()) {
			//instance_create_info.ppEnabledExtensionNames =validation_layers.data();
			instance_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			instance_create_info.ppEnabledLayerNames = validation_layers.data();
			PopulateDebugMessengerCreateInfo(debug_createinfo);
			instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_createinfo;
			enable_validation_layers_ = true;
		}
		else {
			instance_create_info.enabledLayerCount = 0;
			instance_create_info.ppEnabledLayerNames = nullptr;
			//std::vector<const char*> instance_extensions;
			//instance_create_info.enabledExtensionCount = 0;
			//instance_create_info.ppEnabledExtensionNames = nullptr;
			//instance_create_info.ppEnabledExtensionNames = instance_extension.data();
		}


		log_util_.PrintResult("CreateInstance", vkCreateInstance(&instance_create_info, nullptr, &my_instance_));//创建Instance 并打印结果

		SetupDebugMessenger();
		//检查扩展支持
		/*uint32_t extension_count=0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);//获取扩展数量
		std::vector<VkExtensionProperties> instance_extension(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, instance_extension.data());*/
	}

	VulkanInstance::~VulkanInstance() {
		//清除创建的instance 变量
		if (my_owner_ && (my_instance_ != VK_NULL_HANDLE))
		{
			vkDestroyInstance(my_instance_, nullptr);
			my_instance_ = VK_NULL_HANDLE;
		}

	}
	const VkResult VulkanInstance::EnumeratePhysicalDevices(uint32_t& physical_device_count,
		VkPhysicalDevice* physical_devices) const
	{
		//查询当前可用的物理设备的数目以及物理设备的列表
		return vkEnumeratePhysicalDevices(my_instance_, &physical_device_count, physical_devices);
	}
	const bool VulkanInstance::SupportVulkan() const {
		uint32_t gpu_count = 0;

		auto search_result = EnumeratePhysicalDevices(gpu_count, nullptr);
		if (gpu_count == 0 || search_result != VK_SUCCESS) {

			log_util_.PrintLog("Invalid device support vulkan");
			return false;
		}
		else
			return true;

	}
	void VulkanInstance::GetVulkanExtensions() {
		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);//获取扩展数量
		vulkan_extensions_.resize(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, vulkan_extensions_.data());
	}
	void VulkanInstance::SetupDebugMessenger() {
		if (!enable_validation_layers_) return;
		VkDebugUtilsMessengerCreateInfoEXT create_info;
		PopulateDebugMessengerCreateInfo(create_info);
		if (CreateDebugUtilsMessengerEXT(my_instance_, &create_info, nullptr, &debug_messenger_) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger");
		}

	}
	VkResult VulkanInstance::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info,
		const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debug_messenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, create_info, allocator, debug_messenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	void VulkanInstance::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info) {
		create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = debugCallback;
	}
}