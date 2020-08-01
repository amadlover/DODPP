#include "graphics.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <iostream>
#include <array>
#include <vector>

bool is_validation_needed = false;
VkInstance instance = VK_NULL_HANDLE;
VkDebugUtilsMessengerEXT debug_utils_messenger = VK_NULL_HANDLE;
VkPhysicalDevice physical_device = VK_NULL_HANDLE;

uint32_t graphics_queue_family_index = -1;
uint32_t compute_queue_family_index = -1;
uint32_t transfer_queue_family_index = -1;

VkPhysicalDeviceMemoryProperties physical_device_memory_properties = {0};
VkPhysicalDeviceLimits physical_device_limits = {0};
VkSurfaceKHR surface = VK_NULL_HANDLE;
VkSurfaceCapabilitiesKHR surface_capabilities = {0};
VkSurfaceFormatKHR chosen_surface_format = {};
VkPresentModeKHR chosen_present_mode;
VkDevice graphics_device = VK_NULL_HANDLE;
VkSwapchainKHR swapchain = VK_NULL_HANDLE;

VkQueue graphics_queue = VK_NULL_HANDLE;
VkQueue compute_queue = VK_NULL_HANDLE;
VkQueue transfer_queue = VK_NULL_HANDLE;

std::vector<VkImage> swapchain_images;
std::vector<VkImageView> swapchain_image_views;
std::vector<VkFramebuffer> swapchain_framebuffers;
size_t swapchain_image_count = 0;

VkRenderPass render_pass = VK_NULL_HANDLE;

VkCommandPool swapchain_command_pool = VK_NULL_HANDLE;
std::vector<VkCommandBuffer> swapchain_command_buffers;

VkSemaphore wait_semaphore = VK_NULL_HANDLE;
std::vector<VkSemaphore> swapchain_signal_semaphores;
std::vector<VkFence> swapchain_fences;

VkBuffer vertex_index_buffer = VK_NULL_HANDLE;
VkDeviceMemory vertex_index_memory = VK_NULL_HANDLE;

VkPipelineLayout graphics_pipeline_layout = VK_NULL_HANDLE;
VkPipeline graphics_pipeline = VK_NULL_HANDLE;
VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
size_t descriptor_set_count = 0;

VkBuffer transforms_buffer = VK_NULL_HANDLE;
VkDeviceMemory transforms_buffer_memory = VK_NULL_HANDLE;

size_t aligned_size_per_transform = 0;
size_t total_transforms_size = 0;

void* transforms_aligned_data = NULL;
void* transforms_mapped_data = NULL;

float background_positions[12] = { -1,-1,1, 1,-1,1, 1,1,1, -1,1,1 };
float background_colors[12] = {1,0,0, 0,1,0, 0,0,1, 1,1,1};
size_t background_positions_size = sizeof (background_positions);
size_t background_colors_size = sizeof (background_colors);
size_t background_indices[6] = { 0,1,2, 0,2,3 };
size_t background_indices_size = sizeof (background_indices);
size_t background_index_count = 6;

float actor_positions[9] = { -0.1f,0,0.5f, 0.1f,0,0.5f, 0,0.1f,0.5f };
float actor_colors[9] = {1,0,0, 0,1,0, 0,0,1 };
size_t actor_positions_size = sizeof (actor_positions);
size_t actor_colors_size = sizeof (actor_colors);
size_t actor_indices[3] = { 0,1,2 };
size_t actor_indices_size = sizeof (actor_indices);
size_t actor_index_count = 3;


VkResult create_debug_utils_messenger (VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* debug_utils_messenger_create_info,
	const VkAllocationCallbacks* allocation_callbacks,
	VkDebugUtilsMessengerEXT* debug_utils_messenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkCreateDebugUtilsMessengerEXT");

	if (func)
	{
		return func (instance, debug_utils_messenger_create_info, allocation_callbacks, debug_utils_messenger);
	}
	else
	{
		return VK_ERROR_INITIALIZATION_FAILED;
	}
}

void destroy_debug_utils_messenger (VkInstance instance,
	VkDebugUtilsMessengerEXT debug_utils_messenger,
	const VkAllocationCallbacks* allocation_callbacks)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func)
	{
		func (instance, debug_utils_messenger, allocation_callbacks);
	}
	else
	{
		printf ("Could not destroy debug utils messenger\n");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback (
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_types,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* pUserData)
{
	if (callback_data)
	{
		std::cout << "Debug Callback Message: " << callback_data->pMessage << "\n\n";
	}

	return false;
}

AGE_RESULT graphics_common_graphics_init (
	HINSTANCE h_instance, 
	HWND h_wnd)
{
#ifdef _DEBUG 
	is_validation_needed = true;
#elif DEBUG
	is_validation_needed = true;
#else
	is_validation_needed = false;
#endif

    std::vector<const char*> requested_instance_layers;
    std::vector<const char*> requested_instance_extensions;

	if (is_validation_needed)
	{
		size_t layer_count = 0;
		vkEnumerateInstanceLayerProperties (&layer_count, NULL);
        std::vector<VkLayerProperties> layer_properties (layer_count);
		vkEnumerateInstanceLayerProperties (&layer_count, layer_properties.data ());

		auto layer_iter = std::find_if (layer_properties.begin (), 
										layer_properties.end (), 
										[&](const VkLayerProperties& layer_property) 
										{ 
											return (strcmp (layer_property.layerName, "VK_LAYER_KHRONOS_validation") == 0); 
										});
		if (layer_iter != layer_properties.end ())
		{
			requested_instance_layers.push_back (layer_iter->layerName);
		}
	}

	size_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties (NULL, &extension_count, NULL);

	std::vector<VkExtensionProperties> extension_properties (extension_count);
	vkEnumerateInstanceExtensionProperties (NULL, &extension_count, extension_properties.data ());

	for (const auto& extension_property : extension_properties)
	{
		if (strcmp (extension_property.extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			requested_instance_extensions.push_back (extension_property.extensionName);
		}
		else if (strcmp (extension_property.extensionName, "VK_KHR_win32_surface") == 0)
		{
			requested_instance_extensions.push_back (extension_property.extensionName);
		}

		if (is_validation_needed)
		{
			if (strcmp (extension_property.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
			{
                requested_instance_extensions.push_back (extension_property.extensionName);
			}
		}
	}

	VkApplicationInfo application_info = { 
		VK_STRUCTURE_TYPE_APPLICATION_INFO, 
		NULL, 
		"Asteroids", 
		VK_MAKE_VERSION (1, 0, 0), 
		"AGE", 
		VK_MAKE_VERSION (1, 0, 0), 
		VK_API_VERSION_1_2 
	};
	
	AGE_RESULT age_result = AGE_RESULT::SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	if (is_validation_needed)
	{
		VkValidationFeatureEnableEXT enables[] = { 
			VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
			VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT
		};
		
		VkValidationFeaturesEXT features = {
			VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
			NULL,
			1,
			enables,
			0,
			NULL
		};

		VkInstanceCreateInfo instance_create_info = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			&features,
			0,
			&application_info,
			requested_instance_layers.size (),
			requested_instance_layers.data (),
			requested_instance_extensions.size (),
			requested_instance_extensions.data ()
		};

		vk_result = vkCreateInstance (&instance_create_info, NULL, &instance);
		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_INSTANCE;
			goto exit;
		}

		VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info = {
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			NULL,
			0,
			/*VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |*/
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
			debug_messenger_callback,
			NULL
		};

		vk_result = create_debug_utils_messenger (instance, &debug_utils_messenger_create_info, NULL, &debug_utils_messenger);

		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_DEBUG_UTILS_MESSENGER;
			goto exit;
		}
	}
	else
	{
		VkInstanceCreateInfo instance_create_info = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			NULL,
			0,
			&application_info,
			requested_instance_layers.size (),
			requested_instance_layers.data (),
			requested_instance_extensions.size (),
			requested_instance_extensions.data ()
		};

		AGE_RESULT age_result = AGE_RESULT::SUCCESS;
		VkResult vk_result = vkCreateInstance (&instance_create_info, NULL, &instance);
		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_INSTANCE;
			goto exit;
		}
	}

	VkPhysicalDeviceFeatures device_features;

	{
		size_t physical_device_count = 0;
		vkEnumeratePhysicalDevices (instance, &physical_device_count, NULL);

		if (physical_device_count == 0)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_GET_PHYSICAL_DEVICE;
			goto exit;
		}

		std::vector<VkPhysicalDevice> physical_devices (physical_device_count);
		vkEnumeratePhysicalDevices (instance, &physical_device_count, physical_devices.data ());

		physical_device = physical_devices[0];
		vkGetPhysicalDeviceFeatures (physical_device, &device_features);

		size_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &queue_family_count, NULL);
		std::vector<VkQueueFamilyProperties> queue_family_properties (queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &queue_family_count, queue_family_properties.data ());

		graphics_queue_family_index = std::distance (
			queue_family_properties.begin (),
			std::find_if (queue_family_properties.begin (), 
				queue_family_properties.end (), 
				[&](const VkQueueFamilyProperties& family_property) 
				{ 
					return (family_property.queueFlags & VK_QUEUE_GRAPHICS_BIT); 
				}
			)
		);
		auto compute_family_index_iter = std::find_if (queue_family_properties.begin (), 
													queue_family_properties.end (), 
													[&](const VkQueueFamilyProperties& family_property) 
													{ 
														return (family_property.queueFlags & VK_QUEUE_COMPUTE_BIT) && (!(family_property.queueFlags & VK_QUEUE_GRAPHICS_BIT));
													});
		
		if (compute_family_index_iter != queue_family_properties.end ())
		{
			compute_queue_family_index = std::distance (queue_family_properties.begin (), compute_family_index_iter);
		}
		else
		{
			compute_queue_family_index = std::distance (queue_family_properties.begin (), 
														std::find_if (queue_family_properties.begin (), 
																	queue_family_properties.end (), 
																	[&](const VkQueueFamilyProperties& family_property) 
																	{ 
																		return (family_property.queueFlags & VK_QUEUE_COMPUTE_BIT); 
																	}
																	)
														);
		}

		auto transfer_family_index_iter = std::find_if (queue_family_properties.begin (), 
														queue_family_properties.end (), 
														[&](const VkQueueFamilyProperties& family_property) 
														{ 
															return (family_property.queueFlags & VK_QUEUE_TRANSFER_BIT) && 
																(!(family_property.queueFlags & VK_QUEUE_GRAPHICS_BIT)) && 
																(!(family_property.queueFlags & VK_QUEUE_COMPUTE_BIT)); 
														}
														);
		if (transfer_family_index_iter != queue_family_properties.end ())
		{
			transfer_queue_family_index = std::distance (queue_family_properties.begin (), transfer_family_index_iter);
		}
		else
		{
			transfer_queue_family_index = std::distance (queue_family_properties.begin (), 
														std::find_if (queue_family_properties.begin (), 
																	queue_family_properties.end (), 
																	[&](const VkQueueFamilyProperties& family_property) 
																	{ 
																		return (family_property.queueFlags & VK_QUEUE_TRANSFER_BIT); 
																	}
																	)
														);
		}
	}
	
	{
		vkGetPhysicalDeviceMemoryProperties (physical_device, &physical_device_memory_properties);

		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties (physical_device, &device_properties);
		physical_device_limits = device_properties.limits;

		VkWin32SurfaceCreateInfoKHR surface_create_info = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, NULL, 0, h_instance, h_wnd };

		vk_result = vkCreateWin32SurfaceKHR (instance, &surface_create_info, NULL, &surface);

		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_SURFACE;
			goto exit;
		}

		VkBool32 is_surface_supported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR (physical_device, graphics_queue_family_index, surface, &is_surface_supported);

		if (!is_surface_supported)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_SURFACE_SUPPORT;
			goto exit;
		}

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR (physical_device, surface, &surface_capabilities);

		size_t surface_format_count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &surface_format_count, NULL);

		std::vector<VkSurfaceFormatKHR> surface_formats (surface_format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &surface_format_count, surface_formats.data ());

		for (const auto& surface_format : surface_formats)
		{
			if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				chosen_surface_format = surface_format;
				break;
			}
		}

		size_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &present_mode_count, NULL);

		std::vector<VkPresentModeKHR> present_modes (present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &present_mode_count, present_modes.data ());

		for (const auto& present_mode : present_modes)
		{
			if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				chosen_present_mode = present_mode;
				break;
			}
		}

		if (chosen_present_mode == -1)
		{
			chosen_present_mode = VK_PRESENT_MODE_FIFO_KHR;
		}
	}

	{
		std::vector<const char*> requested_device_extensions;

		extension_count = 0;
		vkEnumerateDeviceExtensionProperties (physical_device, NULL, &extension_count, NULL);

		extension_properties.clear ();
		extension_properties.resize (extension_count);
		vkEnumerateDeviceExtensionProperties (physical_device, NULL, &extension_count, extension_properties.data ());

		for (auto const& extension_property : extension_properties)
		{
			if (strcmp (extension_property.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
			{
				requested_device_extensions.push_back (VK_KHR_SWAPCHAIN_EXTENSION_NAME);
				break;
			}
		}

		extension_properties.clear ();

		float priorities[3] = { 1.f, 1.f, 1.f };

		VkDeviceQueueCreateInfo queue_create_infos[3];
		size_t unique_queue_family_indices[3] = { 0,0,0 };
		size_t unique_queue_count[3] = { 1,1,1 };
		size_t unique_queue_family_index_count = 0;

		if (graphics_queue_family_index == compute_queue_family_index)
		{
			unique_queue_family_indices[0] = graphics_queue_family_index;
			++unique_queue_family_index_count;
			++unique_queue_count[0];
		}
		else
		{
			unique_queue_family_indices[0] = graphics_queue_family_index;
			unique_queue_family_indices[1] = compute_queue_family_index;
			unique_queue_family_index_count += 2;
		}

		if (graphics_queue_family_index != transfer_queue_family_index)
		{
			unique_queue_family_indices[unique_queue_family_index_count] = transfer_queue_family_index;
			++unique_queue_family_index_count;
		}

		for (size_t ui = 0; ui < unique_queue_family_index_count; ++ui)
		{
			queue_create_infos[ui].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_infos[ui].pNext = NULL;
			queue_create_infos[ui].pQueuePriorities = priorities;
			queue_create_infos[ui].queueCount = unique_queue_count[ui];
			queue_create_infos[ui].queueFamilyIndex = unique_queue_family_indices[ui];
			queue_create_infos[ui].flags = 0;
		}

		VkDeviceCreateInfo device_create_info = {
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			NULL,
			0,
			unique_queue_family_index_count,
			queue_create_infos,
			0,
			NULL,
			requested_device_extensions.size (),
			requested_device_extensions.data (),
			&device_features
		};

		vk_result = vkCreateDevice (physical_device, &device_create_info, NULL, &graphics_device);

		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_GRAPHICS_DEVICE;
			goto exit;
		}
	}

	{
		VkSwapchainCreateInfoKHR swapchain_create_info = {
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			NULL,
			0,
			surface,
			surface_capabilities.minImageCount + 1,
			chosen_surface_format.format,
			chosen_surface_format.colorSpace,
			surface_capabilities.currentExtent,
			1,
			surface_capabilities.supportedUsageFlags,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			NULL,
			surface_capabilities.currentTransform,
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			chosen_present_mode,
			1,
			VK_NULL_HANDLE
		};

		vk_result = vkCreateSwapchainKHR (graphics_device, &swapchain_create_info, NULL, &swapchain);

		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_SWAPCHAIN;
			goto exit;
		}
	}

	{
		vkGetSwapchainImagesKHR (graphics_device, swapchain, &swapchain_image_count, NULL);
		swapchain_images.resize (swapchain_image_count);
		vkGetSwapchainImagesKHR (graphics_device, swapchain, &swapchain_image_count, swapchain_images.data ());
		swapchain_image_views.resize (swapchain_image_count);

		VkImageSubresourceRange subresource_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		VkImageViewCreateInfo image_view_create_info = {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			NULL,
			0,
			VK_NULL_HANDLE,
			VK_IMAGE_VIEW_TYPE_2D,
			chosen_surface_format.format,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};

		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			image_view_create_info.image = swapchain_images[i];
			vk_result = vkCreateImageView (graphics_device, &image_view_create_info, NULL, &swapchain_image_views[i]);

			if (vk_result != VK_SUCCESS)
			{
				age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
				goto exit;
			}
		}
	}

	size_t graphics_queue_index = 0;
	size_t compute_queue_index = graphics_queue_family_index == compute_queue_family_index ? 1 : 0;
	size_t transfer_queue_index = transfer_queue_family_index == compute_queue_family_index ? compute_queue_index + 1 : 0;

	vkGetDeviceQueue (graphics_device, graphics_queue_family_index, graphics_queue_index, &graphics_queue);
	vkGetDeviceQueue (graphics_device, compute_queue_family_index, compute_queue_index, &compute_queue);
	vkGetDeviceQueue (graphics_device, transfer_queue_family_index, transfer_queue_index, &transfer_queue);


	{
		VkDescriptorPoolSize descriptor_pool_size = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			1
		};

		VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			NULL,
			VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			1,
			1,
			&descriptor_pool_size
		};
		vk_result = vkCreateDescriptorPool (graphics_device, &descriptor_pool_create_info, NULL, &descriptor_pool);

		VkDescriptorSetLayoutBinding descriptor_layout_binding = {
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			1,
			VK_SHADER_STAGE_VERTEX_BIT,
			NULL
		};

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			NULL,
			0,
			1,
			&descriptor_layout_binding
		};

		vk_result = vkCreateDescriptorSetLayout (graphics_device, &descriptor_set_layout_create_info, NULL, &descriptor_set_layout);
		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT;
			goto exit;
		}
	}

	{
		VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			NULL,
			descriptor_pool,
			1,
			&descriptor_set_layout
		};

		vk_result = vkAllocateDescriptorSets (graphics_device, &descriptor_set_allocate_info, &descriptor_set);
		if (vk_result)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SETS;
			goto exit;
		}
	}

	{
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			NULL,
			0,
			1,
			&descriptor_set_layout,
			0,
			NULL
		};

		vk_result = vkCreatePipelineLayout (graphics_device, &pipeline_layout_create_info, NULL, &graphics_pipeline_layout);
		if (vk_result)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT;
			goto exit;
		}
	}

exit: // clean up allocation made within the function

	return age_result;
}

AGE_RESULT graphics_init (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

exit:
    return age_result;
}

AGE_RESULT graphics_update_command_buffers (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

exit:
    return age_result;
}

AGE_RESULT graphics_create_transforms_buffer (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

exit:
    return age_result;
}

AGE_RESULT graphics_update_transforms_buffer (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

exit:
    return age_result;
}

AGE_RESULT graphics_submit_present (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

exit:
    return age_result;
}

void graphics_exit (void)
{
	vkQueueWaitIdle (graphics_queue);

	if (transforms_mapped_data != NULL)
	{
		vkUnmapMemory (graphics_device, transforms_buffer_memory);
	}

	if (descriptor_set != VK_NULL_HANDLE)
	{
		vkFreeDescriptorSets (graphics_device, descriptor_pool, 1, &descriptor_set);
	}
	
	if (descriptor_set_layout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout (graphics_device, descriptor_set_layout, NULL);
	}

	if (descriptor_pool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool (graphics_device, descriptor_pool, NULL);
	}

	for (const auto& swapchain_fence : swapchain_fences)
	{
		vkDestroyFence (graphics_device, swapchain_fence, NULL);
	}
	swapchain_fences.clear ();

	if (graphics_pipeline_layout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout (graphics_device, graphics_pipeline_layout, NULL);
	}

	if (graphics_pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline (graphics_device, graphics_pipeline, NULL);
	}

	if (vertex_index_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (graphics_device, vertex_index_buffer, NULL);
	}

	if (vertex_index_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (graphics_device, vertex_index_memory, NULL);
	}

	if (transforms_buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer (graphics_device, transforms_buffer, NULL);
	}

	if (transforms_buffer_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (graphics_device, transforms_buffer_memory, NULL);
	}

	if (wait_semaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore (graphics_device, wait_semaphore, NULL);
	}

	for (const auto& swapchain_signal_semaphore : swapchain_signal_semaphores)
	{
		vkDestroySemaphore (graphics_device, swapchain_signal_semaphore, NULL);
	}
	swapchain_signal_semaphores.clear ();

	vkFreeCommandBuffers (graphics_device, swapchain_command_pool, swapchain_image_count, swapchain_command_buffers.data ());

	if (swapchain_command_pool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool (graphics_device, swapchain_command_pool, NULL);
	}

	if (render_pass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass (graphics_device, render_pass, NULL);
	}

	for (const auto& swapchain_framebuffer : swapchain_framebuffers)
	{
		if (swapchain_framebuffer != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer (graphics_device, swapchain_framebuffer, NULL);
		}
	}
	swapchain_framebuffers.clear ();

	for (const auto& swapchain_image_view : swapchain_image_views)
	{
		if (swapchain_image_view != VK_NULL_HANDLE)
		{
			vkDestroyImageView (graphics_device, swapchain_image_view, NULL);
		}
	}
	swapchain_image_views.clear ();

	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR (graphics_device, swapchain, NULL);
	}
	swapchain_images.clear ();

	if (graphics_device != VK_NULL_HANDLE)
	{
		vkDestroyDevice (graphics_device, NULL);
	}

	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR (instance, surface, NULL);
	}

	if (is_validation_needed)
	{
		if (debug_utils_messenger != VK_NULL_HANDLE)
		{
			destroy_debug_utils_messenger (instance, debug_utils_messenger, NULL);
		}
	}

    if (instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance (instance, NULL);
    }
}