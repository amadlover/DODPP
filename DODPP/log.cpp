#include "log.hpp"

#include <iostream>

AGE_RESULT log_error (const AGE_RESULT result)
{
	switch (result)
	{
	case AGE_RESULT::ERROR_GRAPHICS_POPULATE_INSTANCE_LAYERS_AND_EXTENSIONS:
		std::cout << "Graphics Error: Populating instance Layers and Extensions\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_INSTANCE:
		std::cout << "Graphics Error: Create instance\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_SETUP_DEBUG_UTILS_MESSENGER:
		std::cout << "Graphics Error: Setup Debug Utils Messenger\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_DEBUG_UTILS_MESSENGER:
		std::cout << "Graphics Error: Create Debug Utils Messenger\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_DESTROY_DEBUG_UTILS_MESSENGER:
		std::cout << "Graphics Error: Destroy Debud Utils Messenger\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_SURFACE:
		std::cout << "Graphics Error: Create surface\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_GET_PHYSICAL_DEVICE:
		std::cout << "Graphics Error: Get Physical Device\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_POPULATE_DEVICE_LAYERS_AND_EXTENSIONS:
		std::cout << "Graphics Error: Populate Device Layers and Extensions\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_GRAPHICS_DEVICE:
		std::cout << "Graphics Error: Create Graphics Device\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_SURFACE_SUPPORT:
		std::cout << "Graphics Error: surface Support\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_SWAPCHAIN:
		std::cout << "Graphics Error: Create swapchain\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_IMAGE_VIEW:
		std::cout << "Graphics Error: Create Image View\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_BUFFER:
		std::cout << "Graphics Error: Create buffer\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_ALLOCATE_MEMORY:
		std::cout << "Graphics Error: Allocate Memory\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_BIND_BUFFER_MEMORY:
		std::cout << "Graphics Error: Bind buffer Memory\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_IMAGE:
		std::cout << "Graphics Error: Create Image\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_BIND_IMAGE_MEMORY:
		std::cout << "Graphics Error: Bind Image Memory\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_MAP_MEMORY:
		std::cout << "Graphics Error: Map Image Memory\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT:
		std::cout << "Graphics Error: Create Descriptor Set Layout\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT:
		std::cout << "Graphics Error: Create Pipeline Layout\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_DESCRIPTOR_POOL:
		std::cout << "Graphics Error: Create Descriptor Pool\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SETS:
		std::cout << "Graphics Error: Allocate Descriptor Set\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_RENDER_PASS:
		std::cout << "Graphics Error: Create Render Pass\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_SHADER_MODULE:
		std::cout << "Graphics Error: Create Shader Module\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_FRAMEBUFFER:
		std::cout << "Graphics Error: Create Framebuffer\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER:
		std::cout << "Graphics Error: Begin Command buffer\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_END_COMMAND_BUFFER:
		std::cout << "Graphics Error: End Command buffer\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_COMMAND_POOL:
		std::cout << "Graphics Error: Create Command Pool\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_RESET_COMMAND_POOL:
		std::cout << "Graphics Error: Reset Command Pool\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER:
		std::cout << "Graphics Error: Allocate Command buffer\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_GRAPHICS_PIPELINE:
		std::cout << "Graphics Error: Create Graphics Pipeline\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_SEMAPHORE:
		std::cout << "Graphics Error: Create Semaphore\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_ACQUIRE_NEXT_IMAGE:
		std::cout << "Graphics Error: Acquire Next Image\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_WAIT_FOR_FENCES:
		std::cout << "Graphics Error: Wait for Fence\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_FENCE:
		std::cout << "Graphics Error: Create Fence\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_RESET_FENCES:
		std::cout << "Graphics Error: Reset Fence\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_QUEUE_SUBMIT:
		std::cout << "Graphics Error: Queue Submit\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_QUEUE_PRESENT:
		std::cout << "Graphics Error: Queue Present\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_UPDATE_UNIFORM_BUFFER:
		std::cout << "Graphics Error: Update Uniform buffer\n";
		break;

	case AGE_RESULT::ERROR_GRAPHICS_CREATE_SAMPLER:
		std::cout << "Graphics Error: Create Sampler\n";
		break;

	case AGE_RESULT::ERROR_GLTF_IMPORT:
		std::cout << "GLTF Error: Import GLTF File\n";
		break;

	case AGE_RESULT::ERROR_SYSTEM_ALLOCATE_MEMORY:
		std::cout << "System Error: Allocate Memory\n";
		break;

	case AGE_RESULT::ERROR_SYSTEM_TMP_FILE:
		std::cout << "System Error: Create TMP File\n";
		break;

	default:
		break;
	}

	return AGE_RESULT::SUCCESS;
}