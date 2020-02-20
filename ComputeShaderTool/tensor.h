#pragma once
#include"vulkan_buffer.h"
#include"vk_com.h"

namespace ImageProcess {
	class Tensor {
	public:
		Tensor(Format fmt = kFormatFp32);
		Tensor(const char* data, std::vector<int>& shape, std::shared_ptr<VulkanUtil::VulkanDevice> kDevice,Format fmt = kFormatFp32);
		void* map();
		void unMap();
		Shape getShape() const;
		int dimSize(const int dim) const;
		int dimNum() const;
		int count(const int start_axis = 0, const int end_axis = -1) const;

		inline bool checkFormat(Format fmt)
		{
			return (fmt > -1 && fmt < kFormatNum) ? true : false;
		}

		inline size_t elementSize(Format fmt)
		{
			if (fmt == kFormatFp32 || fmt == kFormatInt32)
			{
				return 4;
			}
			else if (fmt >= 0 && fmt < kFormatNum)
			{
				//CV_LOG_WARNING(NULL, format("Unsupported format %d", fmt));
			}
			else
			{
				//CV_Error(Error::StsError, format("Invalid format %d", fmt));
			}
			return 0;
		}

		inline int shapeCount(const Shape& shape, int start = -1, int end = -1)const
		{
			if (start == -1) start = 0;
			if (end == -1) end = (int)shape.size();

			if (shape.empty())
				return 0;

			int elems = 1;
			assert(start <= (int)shape.size() &&
				end <= (int)shape.size() &&
				start <= end);
			for (int i = start; i < end; i++)
			{
				elems *= shape[i];
			}
			return elems;
		}
		// Change shape and format to as passed in.
		// Copy data if data != NULL
		// Allocate new internal buffer if new size > old size or alloc flag is true
		Tensor reshape(const char* data, const std::vector<int>& shape, bool alloc = false, Format fmt = kFormatInvalid);

		void setTo(float val);
		int getFormat() const;
		size_t size() const { return size_in_byte_; }
		bool isEmpty() { return size_in_byte_ == 0 ? true : false; }
		void copyTo(Tensor& dst);
		std::shared_ptr < VulkanUtil::VulkanBuffer> getBuffer() { return buffer_; }

	private:
		std::shared_ptr<VulkanUtil::VulkanDevice> device_;
		std::vector<int> shape_;
		size_t size_in_byte_;
		std::shared_ptr<VulkanUtil::VulkanBuffer> buffer_;
		Format format_;
		VulkanUtil::LogUtil log_util_;
	};
}
