#include"median_filter.h"
namespace ImageProcess {
#define ARR_VIEW(x) uint32_t(x.size()), x.data()
	MedianFilter::MedianFilter(std::string image_path) {
		input_path_ = image_path;
		support_vulkan_ = false;
		workgroup_size_ = 16;
	}
	void MedianFilter::Process() {

		const std::string shader_path = "D://vulkan_project//ComputeShaderTool//ComputeShaderTool//median_filter.comp.spv";
		const int buffer_num = 2;
		const std::string application_name = "MedianFilter";
		descriptor_type_.resize(buffer_num);
		descriptor_type_[0] = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptor_type_[1] = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		
		InitVulkan(shader_path,application_name,buffer_num, descriptor_type_);
		
		cv::Mat input_image = cv::imread(input_path_,0);
		image_width_ = input_image.cols;
		image_height_ = input_image.rows;
		input_image.convertTo(input_image, CV_32SC1);
		cv::Mat output_image = cv::Mat(image_height_, image_width_, CV_32SC1);
		int filter_radius = 4;

		FilterParam filter_param = {};
		filter_param.Height = image_height_;
		filter_param.Width = image_width_;
		filter_param.filter_radius = filter_radius;
		auto specValues = std::array<int, 2>{workgroup_size_, workgroup_size_};


		auto specEntries = std::array<VkSpecializationMapEntry, 2>{
			//{ {0, 0, sizeof(int)}, { 1, 1 * sizeof(int), sizeof(int) }}
		};
		specEntries[0].constantID = 0;
		specEntries[0].offset = 0;
		specEntries[0].size = sizeof(int);

		specEntries[1].constantID = 1;
		specEntries[1].offset = 1*sizeof(int);
		specEntries[1].size = sizeof(int);

		//VkSpecializationInfo shader_stages;
		VkSpecializationInfo spec_info = {};
		//spec_info.mapEntryCount = ARR_VIEW(specEntries);
		//spec_info.dataSize = specValues.size() * sizeof(int);
		//spec_info.pData = specValues.data();
		//spec_info.pMapEntries
		//shader_stages.push_back(spec_info);
		my_shader_->SetComputeParam((image_width_ + workgroup_size_ - 1) / workgroup_size_, (image_height_ + workgroup_size_ - 1) / workgroup_size_, 1);
		
		my_shader_->CreateComputePipeline(sizeof(FilterParam), spec_info);
		LoadData(input_image, output_image);
		my_shader_->RecordCommandBuffer((void*)&filter_param, sizeof(FilterParam));

		//cv::Mat input_image = cv::imread(input_path_, 0);
		assert(!input_image.empty());
		clock_t start, end;
		start = clock();
		my_shader_->RunCommandBuffer();
		end = clock();
		double time_cost = double(end - start) / CLOCKS_PER_SEC;
		cv::Mat result_image;
		GetOutputData(result_image);
		cv::imwrite("result.png", result_image);
		std::cout << result_image.at<int>(0, 0) << std::endl;
		//cv::imshow("result", result_image);
		std::cout << "Time cost:" << time_cost <<std::endl;
		cv::waitKey(0);
	}

	/*void MedianFilter::InitVulkan(const std::string shader_path, std::string application_name, const int buffer_num) {
		my_valid_layer_ = std::make_shared<VulkanUtil::VulkanValidLayer>();
		my_valid_layer_->SetIsEnable(true);
		my_instance_ = std::make_shared<VulkanUtil::VulkanInstance>(application_name.c_str(),application_name.c_str(),my_valid_layer_);
		support_vulkan_ = my_instance_->SupportVulkan();
		if (!support_vulkan_)
			return;
		my_device_ = std::make_shared<VulkanUtil::VulkanDevice>(my_instance_, my_valid_layer_);
		//初始化创建device。
		//std::string shader_path = "D://vulkan_project//ComputeShaderTool//ComputeShaderTool//median_filter.comp.spv";
		my_shader_ = std::make_shared<VulkanUtil::VulkanShader>(my_device_, shader_path);
		my_device_->CreateCommandPool();
		my_shader_->InitShader(buffer_num);
	}*/
	void MedianFilter::LoadData(cv::Mat& input_image,cv::Mat& output_image) {
		//image_width_ = input_image.cols;
		//image_height_ = input_image.rows;

		size_t data_size = image_height_*image_width_*4;
		input_buffer_ = std::make_shared<VulkanUtil::VulkanBuffer>(my_device_, data_size, (const char*)input_image.data);
		output_buffer_ = std::make_shared<VulkanUtil::VulkanBuffer>(my_device_, data_size, (const char*)output_image.data);
		//const char* image_data = (const char*)input_image.data;
		//VulkanUtil::VulkanBuffer buffer(my_device_, data_size, image_data);
		my_shader_->BindBuffer(input_buffer_, 1);
		my_shader_->BindBuffer(output_buffer_, 0);
	}

	void MedianFilter::GetOutputData(cv::Mat& result_image) {
		void *out_data = output_buffer_->Map();
		cv::Mat r_image = cv::Mat(image_height_, image_width_, CV_32SC1,out_data);
		r_image.copyTo(result_image);
		output_buffer_->UnMap();


	}
}