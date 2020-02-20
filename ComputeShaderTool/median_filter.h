#pragma once
#include"vulkan_shader.h"
#include"vulkan_buffer.h"
#include"vulkan_device.h"
#include"image_process.h"
#include<opencv2\opencv.hpp>
namespace ImageProcess {

	class MedianFilter:public ImageProcessBase {
	public:

		struct FilterParam {
			int Width;
			int Height;
			int filter_radius;
		};
		MedianFilter(std::string image_path);
		void Process();//执行处理操作
	private:
		std::string input_path_;
		int workgroup_size_;
	private:
		/*std::shared_ptr<VulkanUtil::VulkanValidLayer> my_valid_layer_;
		std::shared_ptr<VulkanUtil::VulkanInstance> my_instance_;
		std::shared_ptr<VulkanUtil::VulkanDevice> my_device_;
		std::shared_ptr<VulkanUtil::VulkanShader> my_shader_;

		std::shared_ptr<VulkanUtil::VulkanBuffer> input_buffer_;
		std::shared_ptr<VulkanUtil::VulkanBuffer> output_buffer_;
		bool support_vulkan_;

		int image_width_;
		int image_height_;*/
		std::vector<VkDescriptorType> descriptor_type_;
	private:
		//void InitVulkan(const std::string shader_path,std::string application_name,const int buffer_num);
		void LoadData(cv::Mat& input_image,cv::Mat& output_image);
		void GetOutputData(cv::Mat& result_image);
	};
}