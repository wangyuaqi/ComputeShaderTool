#pragma once
#include <vector>
#include"vulkan_device.h"
#include"vulkan_buffer.h"
#include"vulkan_instance.h"
#include"vulkan_shader.h"
#include"vulkan_valid_layer.h"
namespace ImageProcess {
	enum Format {
		kFormatInvalid = -1,
		kFormatFp16,
		kFormatFp32,
		kFormatFp64,
		kFormatInt32,
		kFormatNum
	};

	enum OpType {
		kOpTypeConv,
		kOpTypePool,
		kOpTypeDWConv,
		kOpTypeLRN,
		kOpTypeConcat,
		kOpTypeSoftmax,
		kOpTypeReLU,
		kOpTypePriorBox,
		kOpTypePermute,
		kOpTypeNum
	};
	enum ShapeType {
		ShapeIdxBatch,
		ShapeIdxChannel,
		ShapeIdxHeight,
		ShapeIdxWidth
	};

	enum PaddingMode { kPaddingModeSame, kPaddingModeValid, kPaddingModeCaffe, kPaddingModeNum };
	enum FusedActivationType { kNone, kRelu, kRelu1, kRelu6, kActivationNum };
	typedef std::vector<int> Shape;

	//bool isAvailable();
}
