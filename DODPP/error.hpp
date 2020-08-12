#pragma once

enum class AGE_RESULT
{
	SUCCESS,

	// Vulkan
	ERROR_GRAPHICS_POPULATE_INSTANCE_LAYERS_AND_EXTENSIONS,
	ERROR_GRAPHICS_CREATE_INSTANCE,
	ERROR_GRAPHICS_SETUP_DEBUG_UTILS_MESSENGER,
	ERROR_GRAPHICS_CREATE_DEBUG_UTILS_MESSENGER,
	ERROR_GRAPHICS_DESTROY_DEBUG_UTILS_MESSENGER,
	ERROR_GRAPHICS_CREATE_SURFACE,
	ERROR_GRAPHICS_GET_PHYSICAL_DEVICE,
	ERROR_GRAPHICS_POPULATE_DEVICE_LAYERS_AND_EXTENSIONS,
	ERROR_GRAPHICS_CREATE_GRAPHICS_DEVICE,
	ERROR_GRAPHICS_SURFACE_SUPPORT,
	ERROR_GRAPHICS_CREATE_SWAPCHAIN,
	ERROR_GRAPHICS_CREATE_IMAGE_VIEW,
	ERROR_GRAPHICS_CREATE_BUFFER,
	ERROR_GRAPHICS_ALLOCATE_MEMORY,
	ERROR_GRAPHICS_BIND_BUFFER_MEMORY,
	ERROR_GRAPHICS_CREATE_IMAGE,
	ERROR_GRAPHICS_BIND_IMAGE_MEMORY,
	ERROR_GRAPHICS_MAP_MEMORY,
	ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT,
	ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT,
	ERROR_GRAPHICS_CREATE_DESCRIPTOR_POOL,
	ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SETS,
	ERROR_GRAPHICS_CREATE_RENDER_PASS,
	ERROR_GRAPHICS_CREATE_SHADER_MODULE,
	ERROR_GRAPHICS_CREATE_FRAMEBUFFER,
	ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER,
	ERROR_GRAPHICS_END_COMMAND_BUFFER,
	ERROR_GRAPHICS_RESET_COMMAND_BUFFER,
	ERROR_GRAPHICS_CREATE_COMMAND_POOL,
	ERROR_GRAPHICS_RESET_COMMAND_POOL,
	ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER,
	ERROR_GRAPHICS_CREATE_GRAPHICS_PIPELINE,
	ERROR_GRAPHICS_CREATE_SEMAPHORE,
	ERROR_GRAPHICS_ACQUIRE_NEXT_IMAGE,
	ERROR_GRAPHICS_WAIT_FOR_FENCES,
	ERROR_GRAPHICS_CREATE_FENCE,
	ERROR_GRAPHICS_RESET_FENCES,
	ERROR_GRAPHICS_QUEUE_SUBMIT,
	ERROR_GRAPHICS_QUEUE_PRESENT,
	ERROR_GRAPHICS_UPDATE_UNIFORM_BUFFER,
	ERROR_GRAPHICS_CREATE_SAMPLER,

	// GLTF
	ERROR_GLTF_IMPORT,

	// Network

	// Physics

	// System
	ERROR_SYSTEM_ALLOCATE_MEMORY,
	ERROR_SYSTEM_TMP_FILE

};