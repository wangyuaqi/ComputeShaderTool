#include"median_filter.h"
#include"conv_cnn.h"
#include"conv_cnn_image.h"
#include<time.h>
void TestConv();
int main() {
	/*ImageProcess::MedianFilter median_filter("D:\\dataset\\2.jpg");
	clock_t start, end;
	//start = clock();
	median_filter.Process();
	//end = clock();
	//double time_cost_1 = double(end - start) / CLOCKS_PER_SEC;
	
	start = clock();
	cv::Mat input_image = cv::imread("D:\\dataset\\2.jpg", 0);
	cv::Mat median_img;
	cv::blur(input_image, median_img, cv::Size(9,9));
	//cv::medianBlur(input_image, median_img, 9);
	cv::imwrite("result_2.png", median_img);
	end = clock();
	double time_cost_2 = double(end - start) / CLOCKS_PER_SEC;
	std::cout << time_cost_2 << std::endl;*/
	TestConv();
	system("Pause");
}

void InitConvParam(ImageProcess::OpConv& conv_op,int input_width, int input_height, 
	               int input_channels,int stride_dis,int dilation_dis,int filter_w) {
	std::array<int, 2> filter_size = { filter_w,filter_w };
	std::array<int, 2> pad_size = { 1,1 };
	std::array<int, 2> stride_size = { stride_dis,stride_dis };
	std::array<int, 2> dilation_size = { dilation_dis,dilation_dis };
	int activation = 0;
	int group = 0;
	int padding_mode = 0;
	int output_channels = 1;
	//完成参数初始化
	
	conv_op.InitOp(output_channels, true, &filter_size[0], &pad_size[0], &stride_size[0],
		&dilation_size[0], activation, group, padding_mode);

}
void InitConvParam2(ImageProcess::OpConvImage& conv_op, int input_width, int input_height,
	int input_channels, int stride_dis, int dilation_dis, int filter_w) {
	std::array<int, 2> filter_size = { filter_w,filter_w };
	std::array<int, 2> pad_size = { 1,1 };
	std::array<int, 2> stride_size = { stride_dis,stride_dis };
	std::array<int, 2> dilation_size = { dilation_dis,dilation_dis };
	int activation = 0;
	int group = 0;
	int padding_mode = 0;
	int output_channels = 1;
	//完成参数初始化

	conv_op.InitOp(output_channels, true, &filter_size[0], &pad_size[0], &stride_size[0],
		&dilation_size[0], activation, group, padding_mode);

}
void TestConv() {
	int input_width =256;
	int input_height = 256;
	int input_channel = 64;
	int dilation_width = 0;
	int dilation_height = 0;
	int kernel_size = 3;
	//ImageProcess::OpConv conv_op;
	ImageProcess::OpConvImage conv_op;
	InitConvParam2(conv_op, input_width, input_height, input_channel, 1, 1, kernel_size);

	//std::vector<std::vector<std::vector<float>>> input_data(input_channel,std::vector<std::vector<float>>(input_height,std::vector<float>(input_width,1.0f)));
	std::vector<float> input_data(input_channel*input_height*input_width, 5.0f);
	std::vector<int> input_shape = { 1,input_channel,input_height,input_width };

	
	size_t f_size = sizeof(float);
	ImageProcess::Tensor input_tensor(reinterpret_cast<char*>(&input_data[0]), input_shape,conv_op.GetVulkanDevice());
	
	
	int output_width, output_height,output_channel=64;
	int padding_top, padding_left;
	ImageProcess::OpBase::PaddingMode pad_mode= ImageProcess::OpBase::PaddingModeSame;
	conv_op.ComputeConvOutputShape(pad_mode,padding_top, padding_left, input_height, input_width, kernel_size, kernel_size,output_height,output_width);
	
	
	std::cout << output_height << " " << output_width << std::endl;
	//创建输出的tensor
	//std::vector<std::vector<std::vector<float>>> output_data(output_channel, std::vector<std::vector<float>>(output_height, std::vector<float>(output_width, 1.0f)));
	std::vector<float> output_data(output_channel*output_height*output_width, 1.0f);
	std::vector<int> output_shape = { 1,output_channel,output_height,output_width };
	ImageProcess::Tensor output_tensor(reinterpret_cast<char*>(&output_data[0]),output_shape,conv_op.GetVulkanDevice());
	//kernel也是CHW配置
	//std::vector<std::vector<std::vector<std::vector<float>>>> 
	//	filter_param(output_height,std::vector<std::vector<std::vector<float>>>(input_channel, std::vector<std::vector<float>>(kernel_size, std::vector<float>(kernel_size, 1.0f))));
	std::vector<float>filter_param(output_channel*kernel_size*kernel_size, 2.0f);
	std::vector<int> filter_shape = { 1,output_channel,kernel_size,kernel_size };
	//创建weight 对应的tensor
	ImageProcess::Tensor kernel_tensor(reinterpret_cast<char*>(&filter_param[0]), filter_shape,conv_op.GetVulkanDevice());
	//创建bias对应的tensor
	std::vector<float> bias_param(output_channel, 1.0f);
	std::vector<int> bias_shape = { 1,1,1,output_channel };
	ImageProcess::Tensor bias_tensor(reinterpret_cast<char*>(&bias_param[0]), bias_shape,conv_op.GetVulkanDevice());
	
	auto b_shape = bias_tensor.getShape();
	auto n_shape = input_tensor.getShape();
	std::shared_ptr<VulkanUtil::VulkanMemoryPool> my_memory_pool = std::make_shared<VulkanUtil::VulkanMemoryPool>(conv_op.GetVulkanDevice());
	conv_op.Forward(input_tensor, kernel_tensor, bias_tensor, output_tensor,my_memory_pool);
	//conv_op.Forward(input_tensor, kernel_tensor, bias_tensor, output_tensor,my_memory_pool);
}