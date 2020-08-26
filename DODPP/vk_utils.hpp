#ifndef VK_UTILS_HPP
#define VK_UTILS_HPP

#include "vulkan_interface.hpp"

AGE_RESULT vk_create_buffer (const size_t size, const VkBufferUsageFlags usage, VkBuffer* out_buffer);

AGE_RESULT vk_allocate_bind_buffer_memory (const VkBuffer buffer, const uint32_t required_types, VkDeviceMemory* out_buffer_memory);

AGE_RESULT vk_map_buffer_memory (const VkDeviceMemory memory, const size_t offset, const size_t size, void** mapped_memory_ptr);

void vk_copy_data_to_memory_mapped_ptr (size_t offset, void* data, size_t size, void* mapped_memory_ptr);

AGE_RESULT vk_allocate_command_buffers (const VkCommandPool cmd_pool, VkCommandBufferLevel level, size_t num_cmd_buffers, VkCommandBuffer* out_cmd_buffers);

AGE_RESULT vk_begin_cmd_buffer (const VkCommandBuffer cmd_buffer, const VkCommandBufferUsageFlags flags);

AGE_RESULT vk_end_cmd_buffer (const VkCommandBuffer cmd_buffer);

AGE_RESULT vk_submit_cmd_buffer (const VkCommandBuffer cmd_buffer, const VkQueue queue);

AGE_RESULT vk_copy_buffer_to_buffer (const VkBuffer src_buffer, const VkBuffer dst_buffer, const size_t offste, const size_t size);

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
);

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
);

AGE_RESULT vk_allocate_bind_image_memory (VkImage** images, const size_t images_count, const uint32_t required_types, VkDeviceMemory* out_memory);

AGE_RESULT vk_copy_buffer_to_images (
	const VkBuffer src_buffer, 
	VkImage** dst_images, 
	const VkExtent3D* images_extents, 
	const VkDeviceSize* buffer_offsets, 
	const size_t images_count
);

AGE_RESULT vk_create_image_views (
	VkImage** images, 
	const size_t images_count, 
	const VkImageViewType type, 
	const VkFormat format, 
	VkImageView** out_image_views
);

AGE_RESULT vk_create_shader_module (
	const uint32_t* shader_code, 
	const uint32_t shader_code_size,
	const VkShaderStageFlags shader_stage, 
	VkPipelineShaderStageCreateInfo* out_shader_stage_create_info, 
	VkShaderModule* out_shader_module
);

#endif