#pragma once
#include"vulkan_tool.h"
#include"vulkan_device.h"
#include"vulkan_buffer.h"
#include"vulkan_image.h"
namespace VulkanUtil {
	class VulkanShader:public VulKanTool {
	public:
		VulkanShader(std::shared_ptr<VulkanDevice>& use_device,std::string shader_path);
		//考虑到多线程提交，需要将创建Pipeline的部分划，将描述符与每一个Shader绑定
		void SetComputeParam(uint32_t group_x, uint32_t group_y, uint32_t group_z);

		void InitShader(uint32_t buffer_num, std::vector<VkDescriptorType> my_descriptor_type);
		//void CreateDescritorSetLayouot(uint32_t buffer_num);
		//void CreateDescriptorSet(uint32_t buffer_num);
		void CreateComputePipeline(size_t push_constants_size,VkSpecializationInfo& shader_stages);
		void BindBuffer(std::shared_ptr<VulkanBuffer> use_buffer, int binding);
		void BindImage(std::shared_ptr<VulkanImage> use_image,int binding,
			VkImageLayout image_layout, VkDescriptorType descriptor_type,std::shared_ptr<VulkanSampler> use_sampler = nullptr);
		//void BindSampler(std::shared_ptr<VulkanSampler> use_sampler, int binding);

		//void CreateCommandBuffer();
		void RecordCommandBuffer(void *push_constants, size_t push_constant_size);
		void RunCommandBuffer();
	private:
		std::shared_ptr<VulkanDevice> now_device_;
		//VkShaderModule compute_shader_module_:
		VkShaderModule compute_shader_;

		VkDescriptorSetLayout my_descriptor_layout_;
		std::vector<VkDescriptorSetLayout> my_descriptor_layouts_;
		
		//std::map<VkDescriptorType, VkDescriptorPool > my_descriptor_pool_;
		VkDescriptorPool my_descriptor_pool_;
		VkDescriptorSet my_descriptor_set_;
		//std::vector<VkDescriptorSet> my_descriptor_sets_;
		VkPipelineLayout my_pipeline_layout_;

		VkPipeline my_compute_pipeline_;
		VkCommandBuffer my_command_buffer_;

		std::vector<VkDescriptorType> my_descriptor_type_;

		uint32_t group_x_, group_y_, group_z_;
	private:
		VkShaderModule& CreateShaderModule(const std::string file_name);
		
		void CreateDescritorSetLayout(uint32_t buffer_num);
		void CreateDescriptorSet(uint32_t buffer_num, std::vector<VkDescriptorType> descriptor_type);
		void CreateCommandBuffer();
		
		//记录传输指令到指令缓冲
		VkCommandBuffer BeginSingleTimeCommands();
		//结束传输指令
		void EndSingleTimeCommands(VkCommandBuffer command_buffer);

	};
}