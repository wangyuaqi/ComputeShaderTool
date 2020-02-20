#pragma once
#include"env.h"
#define WIN
#include<iostream>
#include<string>
namespace VulkanUtil {
	class LogUtil {
	public:
		void PrintResult(std::string log_str, VkResult operate_result) {
			if (operate_result != VK_SUCCESS) {
#ifdef WIN
				//std::cout << "Vulkan Error " << operate_result << ",File " << __FILE__ << ",Line " << __LINE__ << std::endl;
				std::cout << log_str << ": Vulkan Error " << operate_result << std::endl;
#elif ANDROID

#endif
			}
			else {
#ifdef WIN
				//std::cout << "Vulkan Error " << operate_result << ",File " << __FILE__ << ",Line " << __LINE__ << std::endl;
				std::cout << log_str << ": Vulkan SUCCESS " << operate_result << std::endl;
#elif ANDROID

#endif
			}
		}
		void PrintLog(std::string log_str) const {
#ifdef WIN
			std::cout << log_str << std::endl;
			//std::cout << ""+log_str << std::endl;
#elif ANDROID
#endif
		}
	};
}
