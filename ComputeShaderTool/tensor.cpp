#include"tensor.h"
namespace ImageProcess {
	Tensor::Tensor(Format fmt) : size_in_byte_(0), format_(fmt)
	{
		//createContext();
		//device_ = kDevice;
	}

	Tensor::Tensor(const char* data, std::vector<int>& shape, std::shared_ptr<VulkanUtil::VulkanDevice> kDevice,
		Format fmt)
		: size_in_byte_(0), format_(fmt)
	{
		//createContext();
		device_ = kDevice;
		reshape(data, shape);
	}

	void* Tensor::map()
	{
		void *p;

		log_util_.PrintResult("Map Memory",vkMapMemory(device_->GetDevice(), buffer_->GetVkMemory(),
			0, size_in_byte_, 0, (void **)&p));

		return p;
	}

	void Tensor::unMap()
	{
		vkUnmapMemory(device_->GetDevice(), buffer_->GetVkMemory());
	}

	Shape Tensor::getShape() const
	{
		return shape_;
	}

	int Tensor::count(const int start_axis, const int end_axis) const
	{
		return shapeCount(shape_, start_axis, end_axis);
	}

	int Tensor::dimSize(const int axis) const
	{
		assert(axis >= 0);
		assert(axis < shape_.size());

		return shape_[axis];
	}

	int Tensor::dimNum() const
	{
		return shape_.size();
	}

	Tensor Tensor::reshape(const char* data, const std::vector<int>& shape, bool alloc, Format fmt)
	{
		if (device_->GetDevice() == VK_NULL_HANDLE)
		{
			//CV_Error(Error::StsError, "device is NULL");
			return *this;
		}

		log_util_.LogAssert(shape.size() > 0 && shape.size() <= 6);

		if (shape_ != shape) shape_ = shape;
		if (checkFormat(fmt) && fmt != format_) format_ = fmt;

		size_t new_size = shapeCount(shape_) * elementSize(format_);
		if (alloc || new_size > size_in_byte_)
			alloc = true;
		size_in_byte_ = new_size;

		if (alloc)
		{
			buffer_ = std::make_shared<VulkanUtil::VulkanBuffer>(device_, size_in_byte_, data);
			//buffer_.reset(new VulkanUtil::VulkanBuffer(device_, size_in_byte_, data));
		}
		else if (data)
		{
			void* p = map();
			memcpy(p, data, size_in_byte_);
			unMap();
		}

		return *this;
	}

	void Tensor::setTo(float val)
	{
		if (device_->GetDevice() == VK_NULL_HANDLE)
		{
			//CV_Error(Error::StsError, "device is NULL");
			return;
		}

		log_util_.LogAssert(format_ == kFormatFp32);

		float* p = (float *)map();
		int cnt = count();
		for (int i = 0; i < cnt; i++)
			*p++ = val;
		unMap();
	}

	int Tensor::getFormat() const
	{
		return format_;
	}

	void Tensor::copyTo(Tensor& dst)
	{
		void* p = map();
		dst.reshape((const char*)p, shape_, format_);
		unMap();
	}
}