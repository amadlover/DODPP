#include "graphics.hpp"
#include "utils.hpp"
#include "error.hpp"
#include "actor_vert.hpp"
#include "actor_frag.hpp"
#include "vulkan_interface.hpp"
#include "vk_utils.hpp"

#include <cstdio>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <stb_image.h>


VkRenderPass render_pass = VK_NULL_HANDLE;
VkFramebuffer* swapchain_framebuffers = nullptr;

VkCommandPool swapchain_command_pool = VK_NULL_HANDLE;
VkCommandBuffer* swapchain_command_buffers = nullptr;

VkSemaphore wait_semaphore = VK_NULL_HANDLE;
VkSemaphore* swapchain_signal_semaphores = nullptr;
VkFence* swapchain_fences = nullptr;

VkBuffer vertex_index_buffer = VK_NULL_HANDLE;
VkDeviceMemory vertex_index_buffer_memory = VK_NULL_HANDLE;

VkPipelineLayout graphics_pipeline_layout = VK_NULL_HANDLE;
VkPipeline graphics_pipeline = VK_NULL_HANDLE;
VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
VkDescriptorSetLayout transform_descriptor_set_layout = VK_NULL_HANDLE;
VkDescriptorSet transform_descriptor_set = VK_NULL_HANDLE;
VkDescriptorSetLayout texture_descriptor_set_layout = VK_NULL_HANDLE;
VkDescriptorSet texture_descriptor_set = VK_NULL_HANDLE;

VkBuffer transforms_buffer = VK_NULL_HANDLE;
VkDeviceMemory transforms_buffer_memory = VK_NULL_HANDLE;

size_t aligned_size_per_transform = 0;
size_t total_transforms_size = 0;

void* transforms_aligned_data = nullptr;
void* transforms_mapped_data = nullptr;

float background_positions[12] = { -1,-1,0.9f, 1,-1,0.9f, 1,1,0.9f, -1,1,0.9f };
size_t background_positions_size = sizeof (background_positions);
float background_uvs[8] = { 0,0, 1,0, 1,1, 0,1 };
size_t background_uvs_size = sizeof (background_uvs);
size_t background_indices[6] = { 0,1,2, 0,2,3 };
size_t background_indices_size = sizeof (background_indices);
size_t background_index_count = 6;


float actor_positions[12] = { -0.1f,-0.1f,0.5f, 0.1f,-0.1f,0.5f, 0.1f,0.1f,0.5f, -0.1f,0.1f,0.5f };
size_t actor_positions_size = sizeof (actor_positions);
float actor_uvs[8] = { 1,1, 0,1, 0,0, 1,0 };
size_t actor_uvs_size = sizeof (actor_uvs);
size_t actor_indices[6] = { 0,1,2, 0,2,3 };
size_t actor_indices_size = sizeof (actor_indices);
size_t actor_index_count = 6;


VkImage background_image = VK_NULL_HANDLE;
VkImageView background_image_view = VK_NULL_HANDLE;
size_t background_image_pixels_size = 0;
VkImage player_image = VK_NULL_HANDLE;
VkImageView player_image_view = VK_NULL_HANDLE;
size_t player_image_pixels_size = 0;
VkImage asteroid_image = VK_NULL_HANDLE;
VkImageView asteroid_image_view = VK_NULL_HANDLE;
size_t asteroid_image_pixels_size = 0;
VkImage bullet_image = VK_NULL_HANDLE;
VkImageView bullet_image_view = VK_NULL_HANDLE;
size_t bullet_image_pixels_size = 0;

VkDeviceMemory images_memory = VK_NULL_HANDLE;

AGE_RESULT graphics_create_geometry_buffers ()
{
	AGE_RESULT age_result = AGE_RESULT::SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	VkBuffer staging_vertex_index_buffer = VK_NULL_HANDLE;
	VkDeviceMemory staging_vertex_index_memory = VK_NULL_HANDLE;
	VkBufferCopy buffer_copy = {};

	size_t size = (VkDeviceSize)background_positions_size +
		(VkDeviceSize)background_uvs_size +
		(VkDeviceSize)background_indices_size +
		(VkDeviceSize)actor_positions_size +
		(VkDeviceSize)actor_uvs_size +
		(VkDeviceSize)actor_indices_size;

	age_result = vk_create_buffer (size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &staging_vertex_index_buffer);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	age_result = vk_allocate_bind_buffer_memory (staging_vertex_index_buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &staging_vertex_index_memory);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	void* mapped_memory_ptr = nullptr;

	age_result = vk_map_buffer_memory (
		staging_vertex_index_memory, 
		0, 
		(VkDeviceSize)background_positions_size +
		(VkDeviceSize)background_uvs_size +
		(VkDeviceSize)background_indices_size +
		(VkDeviceSize)actor_positions_size +
		(VkDeviceSize)actor_uvs_size +
		(VkDeviceSize)actor_indices_size, 
		&mapped_memory_ptr
	);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	vk_copy_data_to_memory_mapped_ptr (0, background_positions, background_positions_size, mapped_memory_ptr);
	vk_copy_data_to_memory_mapped_ptr (background_positions_size, background_uvs, background_uvs_size, mapped_memory_ptr);
	vk_copy_data_to_memory_mapped_ptr (background_positions_size + background_uvs_size, background_indices, background_indices_size, mapped_memory_ptr);
	vk_copy_data_to_memory_mapped_ptr (background_positions_size + background_uvs_size + background_indices_size, actor_positions, actor_positions_size, mapped_memory_ptr);
	vk_copy_data_to_memory_mapped_ptr (background_positions_size + background_uvs_size + background_indices_size + actor_positions_size, actor_uvs, actor_uvs_size, mapped_memory_ptr);
	vk_copy_data_to_memory_mapped_ptr (background_positions_size + background_uvs_size + background_indices_size + actor_positions_size + actor_uvs_size, actor_indices, actor_indices_size, mapped_memory_ptr);

	vkUnmapMemory (device, staging_vertex_index_memory);

	age_result = vk_create_buffer (size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &vertex_index_buffer);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	age_result = vk_allocate_bind_buffer_memory (vertex_index_buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertex_index_buffer_memory);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	VkCommandBuffer copy_cmd_buffer;
	age_result = vk_allocate_command_buffers (graphics_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &copy_cmd_buffer);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	age_result = vk_begin_cmd_buffer (copy_cmd_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	buffer_copy = {
		0,
		0,
		(VkDeviceSize)background_positions_size +
		(VkDeviceSize)background_uvs_size +
		(VkDeviceSize)background_indices_size +
		(VkDeviceSize)actor_positions_size +
		(VkDeviceSize)actor_uvs_size +
		(VkDeviceSize)actor_indices_size,
	};

	vkCmdCopyBuffer (copy_cmd_buffer, staging_vertex_index_buffer, vertex_index_buffer, 1, &buffer_copy);

	age_result = vk_end_cmd_buffer (copy_cmd_buffer);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	age_result = vk_submit_cmd_buffer (copy_cmd_buffer, graphics_queue);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}
	

exit:
	if (staging_vertex_index_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (device, staging_vertex_index_buffer, nullptr);
	}

	if (staging_vertex_index_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (device, staging_vertex_index_memory, nullptr);
	}

	if (copy_cmd_buffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers (device, graphics_command_pool, 1, &copy_cmd_buffer);
	}

	return age_result;
}

AGE_RESULT graphics_create_image_buffers ()
{
	AGE_RESULT age_result = AGE_RESULT::SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	int background_image_width = 0;
	int background_image_height = 0;
	int background_image_bpp = 0;
	uint8_t* background_image_pixels = nullptr;
	utils_import_texture ("background.png", &background_image_width, &background_image_height, &background_image_bpp, &background_image_pixels);

	background_image_pixels_size = background_image_width * background_image_height * background_image_bpp * sizeof (uint8_t);

	int player_image_width = 0;
	int player_image_height = 0;
	int player_image_bpp = 0;
	uint8_t* player_image_pixels = nullptr;
	utils_import_texture ("player.png", &player_image_width, &player_image_height, &player_image_bpp, &player_image_pixels);

	player_image_pixels_size = player_image_width * player_image_height * player_image_bpp * sizeof (uint8_t);

	int asteroid_image_width = 0;
	int asteroid_image_height = 0;
	int asteroid_image_bpp = 0;
	uint8_t* asteroid_image_pixels = nullptr;
	utils_import_texture ("asteroid.png", &asteroid_image_width, &asteroid_image_height, &asteroid_image_bpp, &asteroid_image_pixels);

	asteroid_image_pixels_size = asteroid_image_width * asteroid_image_height * asteroid_image_bpp * sizeof (uint8_t);

	int bullet_image_width = 0;
	int bullet_image_height = 0;
	int bullet_image_bpp = 0;
	uint8_t* bullet_image_pixels = nullptr;
	utils_import_texture ("bullet.png", &bullet_image_width, &bullet_image_height, &bullet_image_bpp, &bullet_image_pixels);

	bullet_image_pixels_size = bullet_image_width * bullet_image_height * bullet_image_bpp * sizeof (uint8_t);

	VkBuffer staging_image_buffer = VK_NULL_HANDLE;
	age_result = vk_create_buffer (
		(VkDeviceSize)background_image_pixels_size +
		(VkDeviceSize)player_image_pixels_size +
		(VkDeviceSize)asteroid_image_pixels_size +
		(VkDeviceSize)bullet_image_pixels_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		&staging_image_buffer
	);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	VkDeviceMemory staging_image_memory = VK_NULL_HANDLE;
	age_result = vk_allocate_bind_buffer_memory (staging_image_buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &staging_image_memory);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	void* mapped_memory_ptr = nullptr;
	age_result = vk_map_buffer_memory (
		staging_image_memory,
		0,
		(VkDeviceSize)background_image_pixels_size +
		(VkDeviceSize)player_image_pixels_size +
		(VkDeviceSize)asteroid_image_pixels_size +
		(VkDeviceSize)bullet_image_pixels_size,
		&mapped_memory_ptr
	);
	if (age_result!=AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	vk_copy_data_to_memory_mapped_ptr (0, background_image_pixels, background_image_pixels_size, mapped_memory_ptr);
	vk_copy_data_to_memory_mapped_ptr (background_image_pixels_size, player_image_pixels, player_image_pixels_size, mapped_memory_ptr);
	vk_copy_data_to_memory_mapped_ptr (background_image_pixels_size + player_image_pixels_size, asteroid_image_pixels, asteroid_image_pixels_size, mapped_memory_ptr);
	vk_copy_data_to_memory_mapped_ptr (background_image_pixels_size + player_image_pixels_size + asteroid_image_pixels_size, bullet_image_pixels, bullet_image_pixels_size, mapped_memory_ptr);

	vkUnmapMemory (device, staging_image_memory);

	VkImage* images[] = {
		&background_image,
		&player_image,
		&asteroid_image,
		&bullet_image
	};

	VkExtent3D images_extents[] = {
		{(uint32_t)background_image_width, (uint32_t)background_image_height, 1},
		{(uint32_t)player_image_width, (uint32_t)player_image_height, 1},
		{(uint32_t)asteroid_image_width, (uint32_t)asteroid_image_height, 1},
		{(uint32_t)bullet_image_width, (uint32_t)bullet_image_height, 1}
	};

	age_result = vk_create_images (
		VK_IMAGE_TYPE_2D,
		VK_FORMAT_R8G8B8A8_UNORM,
		1,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		4,
		images_extents,
		images
	);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	age_result = vk_allocate_bind_image_memory (images, 4, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &images_memory);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	age_result = vk_change_images_layout (
		images,
		4,
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		graphics_queue_family_index,
		graphics_queue_family_index,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT
	);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	VkDeviceSize buffer_offsets[] =
	{
		0,
		(VkDeviceSize)background_image_pixels_size,
		(VkDeviceSize)background_image_pixels_size + (VkDeviceSize)player_image_pixels_size,
		(VkDeviceSize)background_image_pixels_size + (VkDeviceSize)player_image_pixels_size + (VkDeviceSize)asteroid_image_pixels_size,
		(VkDeviceSize)background_image_pixels_size + (VkDeviceSize)player_image_pixels_size + (VkDeviceSize)asteroid_image_pixels_size + (VkDeviceSize)bullet_image_pixels_size
	};

	age_result = vk_copy_buffer_to_images (staging_image_buffer, images, images_extents, buffer_offsets, 4);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	age_result = vk_change_images_layout (
		images,
		4,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		graphics_queue_family_index,
		graphics_queue_family_index,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
	);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	VkImageView* image_views[] = {
		&background_image_view,
		&player_image_view,
		&asteroid_image_view,
		&bullet_image_view
	};

	age_result = vk_create_image_views (
		images,
		4,
		VK_IMAGE_VIEW_TYPE_2D,
		VK_FORMAT_R8G8B8A8_UNORM,
		image_views
	);

	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

exit:
	
	if (staging_image_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (device, staging_image_buffer, nullptr);
	}

	if (staging_image_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (device, staging_image_memory, nullptr);
	}

	stbi_image_free (background_image_pixels);
	stbi_image_free (player_image_pixels);
	stbi_image_free (asteroid_image_pixels);
	stbi_image_free (bullet_image_pixels);

	return age_result;
}

AGE_RESULT graphics_create_pipeline ()
{
	AGE_RESULT age_result = AGE_RESULT::SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	VkShaderModule vertex_shader_module = VK_NULL_HANDLE;
	VkShaderModule fragment_shader_module = VK_NULL_HANDLE;
	VkPipelineShaderStageCreateInfo shader_stage_create_infos[2] = {
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT,
			VK_NULL_HANDLE,
			"main",
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_NULL_HANDLE,
			"main",
			nullptr
		}
	};

	VkVertexInputBindingDescription vertex_input_binding_descriptions[2] = {
		{
			0,
			sizeof (float) * 3,
			VK_VERTEX_INPUT_RATE_VERTEX
		},
		{
			1,
			sizeof (float) * 2,
			VK_VERTEX_INPUT_RATE_VERTEX
		}
	};

	VkVertexInputAttributeDescription vertex_input_attribute_descriptions[2] = {
		{
			0,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			0
		},
		{
			1,
			1,
			VK_FORMAT_R32G32_SFLOAT,
			0
		}
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		2,
		vertex_input_binding_descriptions,
		2,
		vertex_input_attribute_descriptions,
	};

	VkPipelineInputAssemblyStateCreateInfo vertex_input_assembly_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_FALSE
	};

	VkViewport viewport = {
		0,
		(float)current_extent.height,
		(float)current_extent.width,
		-(float)current_extent.height,
		0,
		1
	};
	VkRect2D scissor = {
		{0,0},
		current_extent
	};

	VkPipelineViewportStateCreateInfo viewport_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		1,
		&viewport,
		1,
		&scissor
	};

	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE,
		VK_FALSE,
		VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FALSE,
		0,
		0,
		0,
		1
	};

	VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_SAMPLE_COUNT_1_BIT
	};

	VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
		VK_TRUE,
		VK_BLEND_FACTOR_ONE,
		VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		VK_BLEND_OP_ADD,
		VK_BLEND_FACTOR_ONE,
		VK_BLEND_FACTOR_ZERO,
		VK_BLEND_OP_ADD,
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE,
		VK_LOGIC_OP_NO_OP,
		1,
		&color_blend_attachment_state,
		{1,1,1,1}
	};

	VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		nullptr,
		0,
		2,
		nullptr,
		&vertex_input_state_create_info,
		&vertex_input_assembly_state_create_info,
		nullptr,
		&viewport_state_create_info,
		&rasterization_state_create_info,
		&multisample_state_create_info,
		nullptr,
		&color_blend_state_create_info,
		nullptr,
		graphics_pipeline_layout,
		render_pass,
		0,
		VK_NULL_HANDLE,
		0
	};

	age_result = vk_create_shader_module (actor_vert, sizeof (actor_vert), VK_SHADER_STAGE_VERTEX_BIT, &shader_stage_create_infos[0], &vertex_shader_module);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	age_result = vk_create_shader_module (actor_frag, sizeof (actor_frag), VK_SHADER_STAGE_FRAGMENT_BIT, &shader_stage_create_infos[1], &fragment_shader_module);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		goto exit;
	}

	graphics_pipeline_create_info.pStages = shader_stage_create_infos;

	vk_result = vkCreateGraphicsPipelines (device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &graphics_pipeline);
	if (vk_result)
	{
		age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_GRAPHICS_PIPELINE;
		goto exit;
	}

exit:
	if (vertex_shader_module != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule (device, vertex_shader_module, nullptr);
	}

	if (fragment_shader_module != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule (device, fragment_shader_module, nullptr);
	}

	return age_result;
}

AGE_RESULT graphics_create_swapchain_framebuffers ()
{
	swapchain_framebuffers = (VkFramebuffer*)utils_calloc (swapchain_image_count, sizeof (VkFramebuffer));

	VkFramebufferCreateInfo framebuffer_create_info = {
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		nullptr,
		0,
		render_pass,
		1,
		nullptr,
		current_extent.width,
		current_extent.height,
		1
	};

	VkResult vk_result = VK_SUCCESS;

	for (size_t i = 0; i < swapchain_image_count; ++i)
	{
		framebuffer_create_info.pAttachments = &swapchain_image_views[i];

		vk_result = vkCreateFramebuffer (device, &framebuffer_create_info, nullptr, swapchain_framebuffers + i);

		if (vk_result != VK_SUCCESS)
		{
			return AGE_RESULT::ERROR_GRAPHICS_CREATE_FRAMEBUFFER;
		}
	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT graphics_create_swapchain_render_pass ()
{
	VkAttachmentDescription color_attachment_description = {
	0,
	chosen_surface_format.format,
	VK_SAMPLE_COUNT_1_BIT,
	VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	VK_ATTACHMENT_STORE_OP_STORE,
	VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	VK_ATTACHMENT_STORE_OP_DONT_CARE,
	VK_IMAGE_LAYOUT_UNDEFINED,
	VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentReference color_attachment_reference = {
		0,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription color_subpass_description = {
		0,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		0,
		nullptr,
		1,
		&color_attachment_reference,
		nullptr,
		nullptr,
		0,
		nullptr
	};

	VkRenderPassCreateInfo render_pass_create_info = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
		1,
		&color_attachment_description,
		1,
		&color_subpass_description,
		0,
		nullptr
	};

	VkResult vk_result = vkCreateRenderPass (device, &render_pass_create_info, nullptr, &render_pass);

	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_CREATE_RENDER_PASS;
		
	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT graphics_create_swapchain_command_pool_buffers ()
{
	VkCommandPoolCreateInfo cmd_pool_create_info = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,
		0,
		graphics_queue_family_index
	};

	VkResult vk_result = vkCreateCommandPool (device, &cmd_pool_create_info, nullptr, &swapchain_command_pool);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_CREATE_COMMAND_POOL;
	}

	swapchain_command_buffers = (VkCommandBuffer*)utils_malloc(sizeof (VkCommandBuffer) * swapchain_image_count);
	AGE_RESULT age_result = vk_allocate_command_buffers (swapchain_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, swapchain_image_count, swapchain_command_buffers);
	
	return age_result;
}

AGE_RESULT graphics_create_swapchain_semaphores_fences ()
{
	AGE_RESULT age_result = AGE_RESULT::SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	VkSemaphoreCreateInfo semaphore_create_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
	VkFenceCreateInfo fence_create_info = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr,
		0
	};

	vk_result = vkCreateSemaphore (device, &semaphore_create_info, nullptr, &wait_semaphore);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_SEMAPHORE;
		goto exit;
	}

	swapchain_signal_semaphores = (VkSemaphore*)utils_malloc (sizeof (VkSemaphore) * swapchain_image_count);
	for (size_t i = 0; i < swapchain_image_count; ++i)
	{
		vk_result = vkCreateSemaphore (device, &semaphore_create_info, nullptr, swapchain_signal_semaphores + i);

		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_SEMAPHORE;
			goto exit;
		}
	}

	swapchain_fences = (VkFence*)utils_malloc (sizeof (VkFence) * swapchain_image_count);

	for (size_t i = 0; i < swapchain_image_count; ++i)
	{
		vk_result = vkCreateFence (device, &fence_create_info, nullptr, swapchain_fences + i);
		
		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_FENCE;
			goto exit;
		}
	}

exit:
	return age_result;
}

AGE_RESULT graphics_create_descriptor_sets_pipeline_layout ()
{
	AGE_RESULT age_result = AGE_RESULT::SUCCESS;
	VkResult vk_result = VK_SUCCESS;
	
	VkDescriptorPoolSize descriptor_pool_sizes[] = {
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			1
		},
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			4
		}
	};

	VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,
		VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		2,
		2,
		descriptor_pool_sizes
	};
	vk_result = vkCreateDescriptorPool (device, &descriptor_pool_create_info, nullptr, &descriptor_pool);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_CREATE_DESCRIPTOR_POOL;
	}

	VkDescriptorSetLayoutBinding descriptor_layout_bindings[] = {
		{
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			1,
			VK_SHADER_STAGE_VERTEX_BIT,
			nullptr
		},
		{
			0,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			4,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		}
	};

	VkDescriptorSetLayoutCreateInfo transform_descriptor_set_layout_create_info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		1,
		descriptor_layout_bindings
	};
	transform_descriptor_set_layout_create_info.pBindings = descriptor_layout_bindings;

	vk_result = vkCreateDescriptorSetLayout (device, &transform_descriptor_set_layout_create_info, nullptr, &transform_descriptor_set_layout);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT;
	}

	VkDescriptorSetLayoutCreateInfo texture_descriptor_set_layout_create_info = transform_descriptor_set_layout_create_info;
	texture_descriptor_set_layout_create_info.pBindings = descriptor_layout_bindings + 1;

	vk_result = vkCreateDescriptorSetLayout (device, &texture_descriptor_set_layout_create_info, nullptr, &texture_descriptor_set_layout);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT;
	}

	VkDescriptorSetAllocateInfo transform_descriptor_set_allocate_info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		nullptr,
		descriptor_pool,
		1,
		&transform_descriptor_set_layout
	};

	vk_result = vkAllocateDescriptorSets (device, &transform_descriptor_set_allocate_info, &transform_descriptor_set);
	if (vk_result)
	{
		return AGE_RESULT::ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SETS;
	}

	VkDescriptorSetAllocateInfo texture_descriptor_set_allocate_info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		nullptr,
		descriptor_pool,
		1,
		&texture_descriptor_set_layout
	};

	vk_result = vkAllocateDescriptorSets (device, &texture_descriptor_set_allocate_info, &texture_descriptor_set);
	if (vk_result)
	{
		return AGE_RESULT::ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SETS;
	}

	VkDescriptorSetLayout descriptor_set_layouts[] = {
		transform_descriptor_set_layout,
		texture_descriptor_set_layout
	};

	VkDescriptorImageInfo image_infos[4] = {
		{
			common_sampler,
			background_image_view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		},
		{
			common_sampler,
			player_image_view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		},
		{
			common_sampler,
			asteroid_image_view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		},
		{
			common_sampler,
			bullet_image_view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		}
	};

	VkWriteDescriptorSet texture_descriptor_set_write = {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		nullptr,
		texture_descriptor_set,
		0,
		0,
		4,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		image_infos,
		nullptr,
		nullptr
	};

	vkUpdateDescriptorSets (device, 1, &texture_descriptor_set_write, 0, nullptr);

	VkPushConstantRange push_constant_ranges[] = {
		{
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof (float)
		},
		{
			VK_SHADER_STAGE_FRAGMENT_BIT,
			sizeof (float),
			sizeof (uint32_t)
		}
	};

	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		2,
		descriptor_set_layouts,
		2,
		push_constant_ranges
	};

	vk_result = vkCreatePipelineLayout (device, &pipeline_layout_create_info, nullptr, &graphics_pipeline_layout);
	if (vk_result)
	{
		return AGE_RESULT::ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT;
	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT graphics_create_transforms_buffer (
	const size_t game_large_asteroids_current_max_count,
	const size_t game_small_asteroids_current_max_count,
	const size_t game_bullet_current_max_count
)
{
	if (transforms_mapped_data != nullptr)
	{
		vkUnmapMemory (device, transforms_buffer_memory);
	}

	if (transforms_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (device, transforms_buffer, nullptr);
	}

	if (transforms_buffer_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (device, transforms_buffer_memory, nullptr);
	}

	VkResult vk_result = VK_SUCCESS;

	size_t raw_size_per_transform = sizeof (float2) + sizeof (float2) + sizeof (float2);
	aligned_size_per_transform = (raw_size_per_transform + (size_t)physical_device_limits.minUniformBufferOffsetAlignment - 1) & ~((size_t)physical_device_limits.minUniformBufferOffsetAlignment - 1);

	total_transforms_size = aligned_size_per_transform * (game_large_asteroids_current_max_count + game_small_asteroids_current_max_count + game_bullet_current_max_count + 2);
	if (transforms_aligned_data == nullptr)
	{
		transforms_aligned_data = utils_malloc (total_transforms_size);
	}
	else 
	{
		transforms_aligned_data = utils_realloc (transforms_aligned_data, total_transforms_size);
	}

	VkBufferCreateInfo transforms_buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
		total_transforms_size,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr
	};

	vk_result = vkCreateBuffer (device, &transforms_buffer_create_info, nullptr, &transforms_buffer);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_CREATE_BUFFER;
		
	}

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements (device, transforms_buffer, &memory_requirements);

	uint32_t required_memory_types = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	uint32_t required_memory_type_index = 0;

	for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
	{
		if (memory_requirements.memoryTypeBits & (1 << i) && required_memory_types & physical_device_memory_properties.memoryTypes[i].propertyFlags)
		{
			required_memory_type_index = i;
			break;
		}
	}

	VkMemoryAllocateInfo memory_allocate_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		memory_requirements.size,
		required_memory_type_index
	};

	vk_result = vkAllocateMemory (device, &memory_allocate_info, nullptr, &transforms_buffer_memory);
	if (vk_result != VK_SUCCESS) 
	{
		return AGE_RESULT::ERROR_SYSTEM_ALLOCATE_MEMORY;
	}

	vk_result = vkBindBufferMemory (device, transforms_buffer, transforms_buffer_memory, 0);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	vk_result = vkMapMemory (device, transforms_buffer_memory, 0, memory_requirements.size, 0, &transforms_mapped_data);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_MAP_MEMORY;
	}

	VkDescriptorBufferInfo buffer_info = {
		transforms_buffer,
		0,
		VK_WHOLE_SIZE
	};

	VkWriteDescriptorSet descriptor_write = {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		nullptr,
		transform_descriptor_set,
		0,
		0,
		1,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		nullptr,
		&buffer_info,
		nullptr
	};

	vkUpdateDescriptorSets (device, 1, &descriptor_write, 0, nullptr);

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT graphics_update_command_buffers (
	const size_t game_large_asteroids_live_count, 
	const size_t game_small_asteroids_live_count,
	const size_t game_bullet_live_count,
	const float screen_aspect_ratio
)
{
	AGE_RESULT age_result = AGE_RESULT::SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	VkCommandBufferBeginInfo command_buffer_begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		0,
		nullptr
	};

	VkRenderPassBeginInfo render_pass_begin_info = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		nullptr,
		render_pass,
		VK_NULL_HANDLE,
		{ {0,0}, current_extent },
		0,
		nullptr
	};

	vk_result = vkResetCommandPool (device, swapchain_command_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_RESET_COMMAND_POOL;
	}

	for (size_t i = 0; i < swapchain_image_count; ++i)
	{
		vk_result = vkBeginCommandBuffer (swapchain_command_buffers[i], &command_buffer_begin_info);
		if (vk_result != VK_SUCCESS)
		{
			return AGE_RESULT::ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
		}

		render_pass_begin_info.framebuffer = swapchain_framebuffers[i];
		VkDeviceSize vertex_index_buffer_offsets[6] = {
			0, 
			(VkDeviceSize)background_positions_size,
			(VkDeviceSize)background_positions_size + (VkDeviceSize)background_uvs_size,
			(VkDeviceSize)background_positions_size + (VkDeviceSize)background_uvs_size + (VkDeviceSize) background_indices_size,
			(VkDeviceSize)background_positions_size + (VkDeviceSize)background_uvs_size + (VkDeviceSize) background_indices_size + (VkDeviceSize) actor_positions_size,
			(VkDeviceSize)background_positions_size + (VkDeviceSize)background_uvs_size + (VkDeviceSize) background_indices_size + (VkDeviceSize) actor_positions_size + (VkDeviceSize) actor_uvs_size
		};

		vkCmdBeginRenderPass (swapchain_command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		
		vkCmdBindPipeline (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

		float aspect_adjust = 1.f / screen_aspect_ratio;
		vkCmdPushConstants (swapchain_command_buffers[i], graphics_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof (float), &aspect_adjust);

		uint32_t dynamic_offset = 0;
		vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 0, 1, &transform_descriptor_set, 1, &dynamic_offset);
		vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 1, 1, &texture_descriptor_set, 0, nullptr);
		uint32_t texture_index = 0;
		vkCmdPushConstants (swapchain_command_buffers[i], graphics_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof (float), sizeof (uint32_t), &texture_index);
		vkCmdBindVertexBuffers (swapchain_command_buffers[i], 0, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[0]);
		vkCmdBindVertexBuffers (swapchain_command_buffers[i], 1, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[1]);
		vkCmdBindIndexBuffer (swapchain_command_buffers[i], vertex_index_buffer, vertex_index_buffer_offsets[2], VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed (swapchain_command_buffers[i], background_index_count, 1, 0, 0, 0);

		dynamic_offset = aligned_size_per_transform;
		vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 0, 1, &transform_descriptor_set, 1, &dynamic_offset);
		vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 1, 1, &texture_descriptor_set, 0, nullptr); texture_index = 1;
		vkCmdPushConstants (swapchain_command_buffers[i], graphics_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof (float), sizeof (uint32_t), &texture_index);
		vkCmdBindVertexBuffers (swapchain_command_buffers[i], 0, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[3]);
		vkCmdBindVertexBuffers (swapchain_command_buffers[i], 1, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[4]);
		vkCmdBindIndexBuffer (swapchain_command_buffers[i], vertex_index_buffer, vertex_index_buffer_offsets[5], VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed (swapchain_command_buffers[i], actor_index_count, 1, 0, 0, 0);

		texture_index = 2;
		vkCmdPushConstants (swapchain_command_buffers[i], graphics_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof (float), sizeof (uint32_t), &texture_index);

		for (size_t a = 0; a < game_large_asteroids_live_count; ++a)
		{
			dynamic_offset = aligned_size_per_transform * (a + 2);
			vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 0, 1, &transform_descriptor_set, 1, &dynamic_offset);
			vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 1, 1, &texture_descriptor_set, 0, nullptr); 
			vkCmdBindVertexBuffers (swapchain_command_buffers[i], 0, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[3]);
			vkCmdBindVertexBuffers (swapchain_command_buffers[i], 1, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[4]);
			vkCmdBindIndexBuffer (swapchain_command_buffers[i], vertex_index_buffer, vertex_index_buffer_offsets[5], VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed (swapchain_command_buffers[i], actor_index_count, 1, 0, 0, 0);
		}

		for (size_t a = 0; a < game_small_asteroids_live_count; ++a)
		{
			dynamic_offset = aligned_size_per_transform * (game_large_asteroids_live_count + a + 2);
			vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 0, 1, &transform_descriptor_set, 1, &dynamic_offset);
			vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 1, 1, &texture_descriptor_set, 0, nullptr); 
			vkCmdBindVertexBuffers (swapchain_command_buffers[i], 0, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[3]);
			vkCmdBindVertexBuffers (swapchain_command_buffers[i], 1, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[4]);
			vkCmdBindIndexBuffer (swapchain_command_buffers[i], vertex_index_buffer, vertex_index_buffer_offsets[5], VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed (swapchain_command_buffers[i], actor_index_count, 1, 0, 0, 0);
		}

		texture_index = 3;
		vkCmdPushConstants (swapchain_command_buffers[i], graphics_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof (float), sizeof (uint32_t), &texture_index);
		for (size_t b = 0; b < game_bullet_live_count; ++b)
		{
			dynamic_offset = aligned_size_per_transform * (game_large_asteroids_live_count + game_small_asteroids_live_count + b + 2);
			vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 0, 1, &transform_descriptor_set, 1, &dynamic_offset);
			vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 1, 1, &texture_descriptor_set, 0, nullptr);
			vkCmdBindVertexBuffers (swapchain_command_buffers[i], 0, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[3]);
			vkCmdBindVertexBuffers (swapchain_command_buffers[i], 1, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[4]);
			vkCmdBindIndexBuffer (swapchain_command_buffers[i], vertex_index_buffer, vertex_index_buffer_offsets[5], VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed (swapchain_command_buffers[i], actor_index_count, 1, 0, 0, 0);
		}

		vkCmdEndRenderPass (swapchain_command_buffers[i]);

		vk_result = vkEndCommandBuffer (swapchain_command_buffers[i]);
		if (vk_result != VK_SUCCESS)
		{
			return AGE_RESULT::ERROR_GRAPHICS_END_COMMAND_BUFFER;
		}
	}

	return age_result;
}

AGE_RESULT graphics_update_transforms_buffer_data (
	const float2* game_player_output_position, const float2* game_player_output_rotation, const float2* game_player_output_scale,
	const float2* game_large_asteroids_outputs_positions, const float2* game_large_asteroids_outputs_rotations, const float2* game_large_asteroids_outputs_scales,
	const size_t game_large_asteroids_live_count, const size_t game_large_asteroids_current_max_count, 
	const float2* game_small_asteroids_outputs_positions, const float2* game_small_asteroids_outputs_rotations, const float2* game_small_asteroids_outputs_scales,
	const size_t game_small_asteroids_live_count, const size_t game_small_asteroids_current_max_count, 
	const float2* game_bullets_outputs_positions, const float2* game_bullets_outputs_rotations, const float2* game_bullets_outputs_scales, 
	const size_t game_bullet_live_count, const size_t game_bullets_current_max_count
)
{
	AGE_RESULT age_result = AGE_RESULT::SUCCESS;

	float background_transform[] = { 0,0,0,0,1,1 };
	std::memcpy (transforms_aligned_data, background_transform, sizeof (background_transform));

	float player_transform[] = {
		game_player_output_position->x,
		game_player_output_position->y,
		game_player_output_rotation->x,
		game_player_output_rotation->y,
		game_player_output_scale->x,
		game_player_output_scale->y
	};

	std::memcpy ((char*)transforms_aligned_data + aligned_size_per_transform, player_transform, sizeof (player_transform));

	for (size_t a = 0; a < game_large_asteroids_live_count; ++a)
	{
		std::memcpy ((char*)transforms_aligned_data + (aligned_size_per_transform * (a + 2)), game_large_asteroids_outputs_positions + a, sizeof (float2));
	}

	for (size_t a = 0; a < game_large_asteroids_live_count; ++a)
	{
		std::memcpy ((char*)transforms_aligned_data + (aligned_size_per_transform * (a + 2) + sizeof (float2)), game_large_asteroids_outputs_rotations + a, sizeof (float2));
	}

	for (size_t a = 0; a < game_large_asteroids_live_count; ++a)
	{
		std::memcpy ((char*)transforms_aligned_data + (aligned_size_per_transform * (a + 2) + sizeof (float2) + sizeof (float2)), game_large_asteroids_outputs_scales + a, sizeof (float2));
	}

	for (size_t a = 0; a < game_small_asteroids_live_count; ++a)
	{
		std::memcpy ((char*)transforms_aligned_data + (aligned_size_per_transform * (game_large_asteroids_live_count + a + 2)), game_small_asteroids_outputs_positions + a, sizeof (float2));
	}

	for (size_t a = 0; a < game_small_asteroids_live_count; ++a)
	{
		std::memcpy ((char*)transforms_aligned_data + (aligned_size_per_transform * (game_large_asteroids_live_count + a + 2) + sizeof (float2)), game_small_asteroids_outputs_rotations + a, sizeof (float2));
	}

	for (size_t a = 0; a < game_small_asteroids_live_count; ++a)
	{
		std::memcpy ((char*)transforms_aligned_data + (aligned_size_per_transform * (game_large_asteroids_live_count + a + 2) + sizeof (float2) + sizeof (float2)), game_small_asteroids_outputs_scales + a, sizeof (float2));
	}

	for (size_t b = 0; b < game_bullet_live_count; ++b)
	{
		std::memcpy ((char*)transforms_aligned_data + (aligned_size_per_transform * (game_large_asteroids_live_count + game_small_asteroids_live_count + b + 2)), game_bullets_outputs_positions + b, sizeof (float2));
	}

	for (size_t b = 0; b < game_bullet_live_count; ++b)
	{
		std::memcpy ((char*)transforms_aligned_data + (aligned_size_per_transform * (game_large_asteroids_live_count + game_small_asteroids_live_count + b + 2) + sizeof (float2)), game_bullets_outputs_rotations + b, sizeof (float2));
	}

	for (size_t b = 0; b < game_bullet_live_count; ++b)
	{
		std::memcpy ((char*)transforms_aligned_data + (aligned_size_per_transform * (game_large_asteroids_live_count + game_small_asteroids_live_count + b + 2) + sizeof (float2) + sizeof (float2)), game_bullets_outputs_scales + b, sizeof (float2));
	}

	std::memcpy (transforms_mapped_data, transforms_aligned_data, total_transforms_size);

	return age_result;
}

AGE_RESULT graphics_init (
	const size_t game_large_asteroids_current_max_count,
	const size_t game_large_asteroids_live_count,
	const size_t game_small_asteroids_current_max_count,
	const size_t game_small_asteroids_live_count,
	const size_t game_bullets_current_max_count,
	const size_t game_bullet_live_count,
	const float screen_aspect_ratio
)
{
	AGE_RESULT age_result = AGE_RESULT::SUCCESS;

	age_result = graphics_create_geometry_buffers ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = graphics_create_image_buffers ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = graphics_create_descriptor_sets_pipeline_layout ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = graphics_create_swapchain_render_pass ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = graphics_create_swapchain_framebuffers ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = graphics_create_pipeline ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = graphics_create_swapchain_command_pool_buffers ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = graphics_create_swapchain_semaphores_fences ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = graphics_create_transforms_buffer (
		game_large_asteroids_current_max_count, 
		game_small_asteroids_current_max_count, 
		game_bullets_current_max_count
	);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = graphics_update_command_buffers (
		game_large_asteroids_live_count, 
		game_small_asteroids_live_count, 
		game_bullet_live_count,
		screen_aspect_ratio
	);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	return age_result;
}

AGE_RESULT graphics_submit_present ()
{
	size_t image_index = 0;
	VkResult vk_result = vkAcquireNextImageKHR (device, swapchain, UINT64_MAX, wait_semaphore, VK_NULL_HANDLE, &image_index);

	if (vk_result != VK_SUCCESS)
	{
		if (vk_result == VK_SUBOPTIMAL_KHR ||
			vk_result == VK_ERROR_OUT_OF_DATE_KHR ||
			vk_result == VK_TIMEOUT ||
			vk_result == VK_NOT_READY)
		{
			return AGE_RESULT::SUCCESS;
		}
		else
		{
			return AGE_RESULT::ERROR_GRAPHICS_ACQUIRE_NEXT_IMAGE;
		}
	}

	VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit_info = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		1,
		&wait_semaphore,
		&wait_stage_mask,
		1,
		swapchain_command_buffers + image_index,
		1,
		swapchain_signal_semaphores + image_index,
	};

	vk_result = vkQueueSubmit (graphics_queue, 1, &submit_info, swapchain_fences[image_index]);

	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	VkPresentInfoKHR present_info = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr,
		1,
		swapchain_signal_semaphores + image_index,
		1,
		&swapchain,
		&image_index,
		nullptr
	};

	vk_result = vkQueuePresentKHR (graphics_queue, &present_info);

	if (vk_result != VK_SUCCESS)
	{
		if (vk_result == VK_ERROR_OUT_OF_HOST_MEMORY || vk_result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
		{
			return AGE_RESULT::ERROR_GRAPHICS_QUEUE_PRESENT;
		}
	}

	vk_result = vkWaitForFences (device, 1, swapchain_fences + image_index, VK_TRUE, UINT64_MAX);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_WAIT_FOR_FENCES;
	}

	vk_result = vkResetFences (device, 1, swapchain_fences + image_index);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_RESET_FENCES;
	}

	return AGE_RESULT::SUCCESS;
}

void graphics_shutdown ()
{
	if (graphics_queue != VK_NULL_HANDLE)
	{
		vkQueueWaitIdle (graphics_queue);
	}

	if (transforms_mapped_data != nullptr && transforms_buffer_memory != VK_NULL_HANDLE)
	{
		vkUnmapMemory (device, transforms_buffer_memory);
	}

	utils_free (transforms_aligned_data);

	VkDescriptorSet descriptor_sets[] = {
		transform_descriptor_set,
		texture_descriptor_set
	};

	if (transform_descriptor_set != VK_NULL_HANDLE && texture_descriptor_set != VK_NULL_HANDLE)
	{
		vkFreeDescriptorSets (device, descriptor_pool, 2, descriptor_sets);
	}

	if (transform_descriptor_set_layout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout (device, transform_descriptor_set_layout, nullptr);
	}
	
	if (texture_descriptor_set_layout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout (device, texture_descriptor_set_layout, nullptr);
	}

	if (descriptor_pool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool (device, descriptor_pool, nullptr);
	}

	if (swapchain_fences)
	{
		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			vkDestroyFence (device, swapchain_fences[i], nullptr);
		}

		utils_free (swapchain_fences);
	}

	if (graphics_pipeline_layout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout (device, graphics_pipeline_layout, nullptr);
	}

	if (graphics_pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline (device, graphics_pipeline, nullptr);
	}

	if (vertex_index_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (device, vertex_index_buffer, nullptr);
	}

	if (vertex_index_buffer_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (device, vertex_index_buffer_memory, nullptr);
	}

	if (background_image != VK_NULL_HANDLE)
	{
		vkDestroyImage (device, background_image, nullptr);
	}

	if (background_image_view != VK_NULL_HANDLE)
	{
		vkDestroyImageView (device, background_image_view, nullptr);
	}

	if (player_image != VK_NULL_HANDLE)
	{
		vkDestroyImage (device, player_image, nullptr);
	}

	if (player_image_view != VK_NULL_HANDLE)
	{
		vkDestroyImageView (device, player_image_view, nullptr);
	}

	if (asteroid_image != VK_NULL_HANDLE)
	{
		vkDestroyImage (device, asteroid_image, nullptr);
	}

	if (asteroid_image_view != VK_NULL_HANDLE)
	{
		vkDestroyImageView (device, asteroid_image_view, nullptr);
	}

	if (bullet_image != VK_NULL_HANDLE)
	{
		vkDestroyImage (device, bullet_image, nullptr);
	}

	if (bullet_image_view != VK_NULL_HANDLE)
	{
		vkDestroyImageView (device, bullet_image_view, nullptr);
	}

	if (images_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (device, images_memory, nullptr);
	}

	if (transforms_buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer (device, transforms_buffer, nullptr);
	}

	if (transforms_buffer_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (device, transforms_buffer_memory, nullptr);
	}

	if (wait_semaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore (device, wait_semaphore, nullptr);
	}

	if (swapchain_signal_semaphores)
	{
		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			vkDestroySemaphore (device, swapchain_signal_semaphores[i], nullptr);
		}
		
		utils_free (swapchain_signal_semaphores);
	}

	if (swapchain_command_buffers)
	{
		vkFreeCommandBuffers (device, swapchain_command_pool, swapchain_image_count, swapchain_command_buffers);

		utils_free (swapchain_command_buffers);
	}

	if (swapchain_command_pool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool (device, swapchain_command_pool, nullptr);
	}

	if (render_pass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass (device, render_pass, nullptr);
	}
	
	if (swapchain_framebuffers)
	{
		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			if (swapchain_framebuffers[i] != VK_NULL_HANDLE)
			{
				vkDestroyFramebuffer (device, swapchain_framebuffers[i], nullptr);
			}
		}

		utils_free (swapchain_framebuffers);
	}
}