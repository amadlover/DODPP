#include "vk_utils.hpp"
#include "utils.hpp"


AGE_RESULT vk_create_buffer (const size_t size, const VkBufferUsageFlags usage, VkBuffer* out_buffer)
{
	VkBufferCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.size = size;
	create_info.usage = usage;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult vk_result = vkCreateBuffer (device, &create_info, NULL, out_buffer);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_CREATE_BUFFER;
	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT vk_allocate_bind_buffer_memory (const VkBuffer buffer, const uint32_t required_types, VkDeviceMemory* out_buffer_memory)
{
	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements (device, buffer, &memory_requirements);

	uint32_t required_memory_type_index = 0;

	for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
	{
		if (memory_requirements.memoryTypeBits & (1 << i) && required_types & physical_device_memory_properties.memoryTypes[i].propertyFlags)
		{
			required_memory_type_index = i;
			break;
		}
	}

	VkMemoryAllocateInfo memory_allocate_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		NULL,
		memory_requirements.size,
		required_memory_type_index
	};

	VkResult vk_result = vkAllocateMemory (device, &memory_allocate_info, NULL, out_buffer_memory);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	vk_result = vkBindBufferMemory (device, buffer, *out_buffer_memory, 0);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT vk_map_buffer_memory (const VkDeviceMemory memory, const size_t offset, const size_t size, void** mapped_memory_ptr)
{
	VkResult vk_result = vkMapMemory (device, memory, offset, size, 0, mapped_memory_ptr);

	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_MAP_MEMORY;
	}

	return AGE_RESULT::SUCCESS;
}

void vk_copy_data_to_memory_mapped_ptr (size_t offset, void* data, size_t size, void* mapped_memory_ptr)
{
	std::memcpy ((char*)mapped_memory_ptr + offset, data, size);
}

AGE_RESULT vk_allocate_command_buffers (const VkCommandPool cmd_pool, VkCommandBufferLevel level, size_t num_cmd_buffers, VkCommandBuffer* out_cmd_buffers)
{
	VkCommandBufferAllocateInfo copy_cmd_buffer_allocate_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		cmd_pool,
		level,
		num_cmd_buffers
	};

	VkResult vk_result = vkAllocateCommandBuffers (device, &copy_cmd_buffer_allocate_info, out_cmd_buffers);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT vk_begin_cmd_buffer (const VkCommandBuffer cmd_buffer, const VkCommandBufferUsageFlags flags)
{

	VkCommandBufferBeginInfo copy_cmd_buffer_begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		flags,
		NULL
	};

	VkResult vk_result = vkBeginCommandBuffer (cmd_buffer, &copy_cmd_buffer_begin_info);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;

	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT vk_end_cmd_buffer (const VkCommandBuffer cmd_buffer)
{
	VkResult vk_result = vkEndCommandBuffer (cmd_buffer);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_END_COMMAND_BUFFER;

	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT vk_submit_cmd_buffer (const VkCommandBuffer cmd_buffer, const VkQueue queue)
{
	VkSubmitInfo submit_info = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		NULL,
		0,
		NULL,
		0,
		1,
		&cmd_buffer,
		0,
		NULL
	};

	VkResult vk_result = vkQueueSubmit (graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	vkQueueWaitIdle (graphics_queue);

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT vk_copy_buffer_to_buffer (const VkBuffer src_buffer, const VkBuffer dst_buffer, const size_t offste, const size_t size)
{
	return AGE_RESULT::SUCCESS;
}

AGE_RESULT vk_create_images (
	const VkImageType image_type,
	const VkFormat format,
	const uint32_t mip_levels,
	const uint32_t array_layers,
	const VkSampleCountFlagBits samples,
	const VkImageTiling tiling,
	const VkImageUsageFlags usage,
	const size_t images_count,
	const VkExtent3D* extents,
	VkImage** out_images
)
{
	VkImageCreateInfo create_info = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0,
		image_type,
		format,
		{0,0,0},
		mip_levels,
		array_layers,
		samples,
		tiling,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL,
		VK_IMAGE_LAYOUT_UNDEFINED
	};

	for (size_t i = 0; i < images_count; ++i)
	{
		create_info.extent = extents[i];
		VkResult vk_result = vkCreateImage (device, &create_info, NULL, *(out_images + i));
		if (vk_result != VK_SUCCESS)
		{
			return AGE_RESULT::ERROR_GRAPHICS_CREATE_IMAGE;
		}
	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT vk_change_images_layout (
	VkImage** images,
	const size_t images_count,
	const VkAccessFlags src_access,
	const VkAccessFlags dst_access,
	const VkImageLayout src_layout,
	const VkImageLayout dst_layout,
	const uint32_t src_queue_famliy_index,
	const uint32_t dst_queue_family_index,
	const VkPipelineStageFlags src_pipeline_stage,
	const VkPipelineStageFlags dst_pipeline_stage
)
{
	VkImageSubresourceRange subresource_range = {
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1
	};

	VkImageMemoryBarrier image_memory_barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		src_access,
		dst_access,
		src_layout,
		dst_layout,
		src_queue_famliy_index,
		dst_queue_family_index,
		VK_NULL_HANDLE,
		subresource_range
	};

	VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;
	AGE_RESULT age_result = vk_allocate_command_buffers (graphics_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &cmd_buffer);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = vk_begin_cmd_buffer (cmd_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	VkImageMemoryBarrier* barriers = (VkImageMemoryBarrier*)utils_malloc (sizeof (VkImageMemoryBarrier) * images_count);

	for (size_t i = 0; i < images_count; ++i)
	{
		image_memory_barrier.image = **(images + i);
		barriers[i] = image_memory_barrier;
	}

	vkCmdPipelineBarrier (
		cmd_buffer,
		src_pipeline_stage,
		dst_pipeline_stage,
		0,
		0,
		NULL,
		0,
		NULL,
		images_count,
		barriers
	);

	age_result = vk_end_cmd_buffer (cmd_buffer);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	age_result = vk_submit_cmd_buffer (cmd_buffer, graphics_queue);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	vkQueueWaitIdle (graphics_queue);

	if (cmd_buffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers (device, graphics_command_pool, 1, &cmd_buffer);
	}

exit:
	utils_free (barriers);

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT vk_allocate_bind_image_memory (VkImage** images, const size_t images_count, const uint32_t required_types, VkDeviceMemory* out_memory)
{
	VkDeviceSize total_vk_images_size = 0;
	VkMemoryRequirements memory_requirements = { 0 };

	VkDeviceSize* image_data_offsets = (VkDeviceSize*)utils_malloc (sizeof (VkDeviceSize) * images_count);

	for (size_t i = 0; i < images_count; ++i)
	{
		image_data_offsets[i] = total_vk_images_size;
		vkGetImageMemoryRequirements (device, **(images + i), &memory_requirements);
		total_vk_images_size += memory_requirements.size;
	}

	memory_requirements.size = total_vk_images_size;
	uint32_t required_memory_type_index = 0;

	for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
	{
		if (memory_requirements.memoryTypeBits & (1 << i) && required_types & physical_device_memory_properties.memoryTypes[i].propertyFlags)
		{
			required_memory_type_index = i;
			break;
		}
	}

	VkMemoryAllocateInfo memory_allocate_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		NULL,
		memory_requirements.size,
		required_memory_type_index
	};

	VkResult vk_result = vkAllocateMemory (device, &memory_allocate_info, NULL, out_memory);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	for (size_t i = 0; i < images_count; ++i)
	{
		vk_result = vkBindImageMemory (device, **(images + i), *out_memory, image_data_offsets[i]);
		if (vk_result != VK_SUCCESS)
		{
			return AGE_RESULT::ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
		}
	}

	utils_free (image_data_offsets);

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT vk_copy_buffer_to_images (const VkBuffer src_buffer, VkImage** dst_images, const VkExtent3D* images_extents, const VkDeviceSize* buffer_offsets, const size_t images_count)
{
	VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;
	VkOffset3D img_offset = { 0,0,0 };

	VkImageSubresourceLayers subresource_layers = {
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		0,
		1
	};

	VkBufferImageCopy buffer_image_copy = {
		0,
		0,
		0,
		subresource_layers,
		img_offset,
		{0,0,0}
	};

	AGE_RESULT age_result = vk_allocate_command_buffers (graphics_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &cmd_buffer);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	age_result = vk_begin_cmd_buffer (cmd_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	for (size_t i = 0; i < images_count; ++i)
	{
		buffer_image_copy.bufferOffset = buffer_offsets[i];
		buffer_image_copy.imageExtent = images_extents[i];

		vkCmdCopyBufferToImage (cmd_buffer, src_buffer, **(dst_images + i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);
	}

	age_result = vk_end_cmd_buffer (cmd_buffer);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}
	
	age_result = vk_submit_cmd_buffer (cmd_buffer, graphics_queue);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	if (cmd_buffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers (device, graphics_command_pool, 1, &cmd_buffer);
	}

exit:
	return age_result;
}

AGE_RESULT vk_create_image_views (VkImage** images, const size_t images_count, const VkImageViewType type, const VkFormat format, VkImageView** out_image_views)
{
	VkImageSubresourceRange subresource_range = {
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1
	};

	VkImageViewCreateInfo create_info = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0,
		VK_NULL_HANDLE,
		type,
		format,
		{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
		subresource_range
	};

	for (size_t i = 0; i < images_count; ++i)
	{
		create_info.image = **(images + i);
		VkResult vk_result = vkCreateImageView (device, &create_info, NULL, *(out_image_views + i));
		if (vk_result != VK_SUCCESS)
		{
			return AGE_RESULT::ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
		}
	}

	return AGE_RESULT::SUCCESS;
}
