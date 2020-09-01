#include "vulkan_interface.hpp"
#include "utils.hpp"

#include <cstdio>
#include <algorithm>
#include <vulkan/vulkan_win32.h>

bool is_validation_needed = false;
VkDebugUtilsMessengerEXT debug_utils_messenger = VK_NULL_HANDLE;
VkSurfaceKHR surface = VK_NULL_HANDLE;
VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
VkPresentModeKHR chosen_present_mode = VK_PRESENT_MODE_FIFO_KHR;

VkInstance instance;
VkPhysicalDevice physical_device;
VkDevice device;
uint32_t graphics_queue_family_index;
uint32_t compute_queue_family_index;
uint32_t transfer_queue_family_index;
VkQueue graphics_queue;
VkQueue compute_queue;
VkQueue transfer_queue;
VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
VkPhysicalDeviceLimits physical_device_limits;
VkExtent2D surface_extent;
VkSwapchainKHR swapchain;
std::vector<VkImage> swapchain_images;
std::vector<VkImageView> swapchain_image_views;
uint32_t swapchain_image_count;
VkExtent2D current_extent;
VkSurfaceFormatKHR chosen_surface_format;
VkCommandPool graphics_command_pool;
VkSampler common_sampler;

VkResult create_instance_debug_utils_messenger (VkInstance instance,
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

void destroy_instance_debug_utils_messenger (VkInstance instance,
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

VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback (
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_types,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* pUserData)
{
	if (callback_data)
	{
		printf ("Debug Callback Message: %s\n\n", callback_data->pMessage);
	}

	return false;
}

AGE_RESULT create_instance ()
{
	AGE_RESULT age_result = AGE_RESULT::SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	std::vector<const char*> instance_extensions;
	instance_extensions.push_back (VK_KHR_SURFACE_EXTENSION_NAME);
	instance_extensions.push_back (VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

	VkApplicationInfo application_info = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"Asteroids",
		VK_MAKE_VERSION (1, 0, 0),
		"AGE",
		VK_MAKE_VERSION (1, 0, 0),
		VK_API_VERSION_1_1
	};

	std::vector<const char*> instance_layers;

	if (is_validation_needed) {
		instance_extensions.push_back (VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		instance_layers.push_back ("VK_LAYER_KHRONOS_validation");
	}

	if (is_validation_needed)
	{
		instance_extensions.push_back (VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		instance_layers.push_back ("VK_LAYER_KHRONOS_validation");
		VkValidationFeatureEnableEXT enables[] = {
			VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
			VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT
		};

		VkValidationFeaturesEXT features = {
			VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
			nullptr,
			1,
			enables,
			0,
			nullptr
		};

		VkInstanceCreateInfo instance_create_info = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			&features,
			0,
			&application_info,
			(uint32_t)instance_layers.size (),
			instance_layers.data (),
			(uint32_t)instance_extensions.size (),
			instance_extensions.data ()
		};

		vk_result = vkCreateInstance (&instance_create_info, nullptr, &instance);
		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_INSTANCE;
		}
	}
	else
	{
		VkInstanceCreateInfo instance_create_info = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			nullptr,
			0,
			&application_info,
			0,
			nullptr,
			(uint32_t)instance_extensions.size (),
			instance_extensions.data ()
		};

		vk_result = vkCreateInstance (&instance_create_info, nullptr, &instance);
		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_INSTANCE;
		}
	}

	return age_result;
}

AGE_RESULT create_debug_utils_messenger ()
{
	VkResult vk_result = VK_SUCCESS;

	VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info = {
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		nullptr,
		0,
		/*VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |*/
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
		&debug_utils_messenger_callback,
		nullptr
	};

	vk_result = create_instance_debug_utils_messenger (instance, &debug_utils_messenger_create_info, nullptr, &debug_utils_messenger);

	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_CREATE_DEBUG_UTILS_MESSENGER;
	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT create_surface (HINSTANCE h_instance, HWND h_wnd)
{
	AGE_RESULT age_result = AGE_RESULT::SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	VkWin32SurfaceCreateInfoKHR surface_create_info = { 
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, 
		nullptr, 
		0, 
		h_instance, 
		h_wnd 
	};

	vk_result = vkCreateWin32SurfaceKHR (instance, &surface_create_info, nullptr, &surface);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_SURFACE;
	}

	return age_result;
}

AGE_RESULT get_physical_device ()
{
	uint32_t physical_device_count = 0;
	vkEnumeratePhysicalDevices (instance, &physical_device_count, nullptr);

	if (physical_device_count == 0)
	{
		return AGE_RESULT::ERROR_GRAPHICS_GET_PHYSICAL_DEVICE;
	}

	auto physical_devices = (VkPhysicalDevice*)utils_malloc (sizeof (VkPhysicalDevice) * physical_device_count);
	vkEnumeratePhysicalDevices (instance, &physical_device_count, physical_devices);

	physical_device = physical_devices[0];

	utils_free (physical_devices);

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT check_physical_device_surface_support ()
{
	VkBool32 is_surface_supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR (physical_device, graphics_queue_family_index, surface, &is_surface_supported);

	if (!is_surface_supported)
	{
		return AGE_RESULT::ERROR_GRAPHICS_SURFACE_SUPPORT;
	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT get_physical_device_queue_families ()
{
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &queue_family_count, nullptr);
	auto queue_family_properties = (VkQueueFamilyProperties*)utils_malloc (sizeof (VkQueueFamilyProperties) * queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &queue_family_count, queue_family_properties);

	for (uint32_t i = 0; i < queue_family_count; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphics_queue_family_index = i;
			break;
		}
	}

	for (uint32_t i = 0; i < queue_family_count; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT && (i != graphics_queue_family_index))
		{
			compute_queue_family_index = i;
			break;
		}
	}

	if (compute_queue_family_index == -1)
	{
		for (uint32_t i = 0; i < queue_family_count; ++i)
		{
			if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				compute_queue_family_index = i;
				break;
			}
		}
	}

	for (uint32_t i = 0; i < queue_family_count; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT && (i != graphics_queue_family_index) && (i != compute_queue_family_index))
		{
			transfer_queue_family_index = i;
			break;
		}
	}

	if (transfer_queue_family_index == -1)
	{
		for (uint32_t i = 0; i < queue_family_count; ++i)
		{
			if (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				transfer_queue_family_index = i;
				break;
			}
		}
	}

	utils_free (queue_family_properties);

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT get_physical_device_properties ()
{
	vkGetPhysicalDeviceMemoryProperties (physical_device, &physical_device_memory_properties);

	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties (physical_device, &device_properties);
	physical_device_limits = device_properties.limits;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR (physical_device, surface, &surface_capabilities);

	current_extent = surface_capabilities.currentExtent;
	uint32_t surface_format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &surface_format_count, nullptr);

	auto surface_formats = (VkSurfaceFormatKHR*)utils_malloc (sizeof (VkSurfaceFormatKHR) * surface_format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &surface_format_count, surface_formats);

	chosen_surface_format = surface_formats[0];

	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &present_mode_count, nullptr);

	auto present_modes = (VkPresentModeKHR*)utils_malloc (sizeof (VkPresentModeKHR) * present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &present_mode_count, present_modes);

	for (uint32_t p = 0; p < present_mode_count; p++)
	{
		if (present_modes[p] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			chosen_present_mode = present_modes[p];
			break;
		}
	}

	utils_free (surface_formats);
	utils_free (present_modes);

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT create_device ()
{
	AGE_RESULT age_result = AGE_RESULT::SUCCESS;

	char* device_extensions[256];
	device_extensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

	float priorities[3] = { 1.f, 1.f, 1.f };

	VkDeviceQueueCreateInfo queue_create_infos[3] = { };
	uint32_t unique_queue_family_indices[3] = { 0,0,0 };
	uint32_t unique_queue_count[3] = { 1,1,1 };
	uint32_t unique_queue_family_index_count = 0;

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

	for (uint32_t ui = 0; ui < unique_queue_family_index_count; ++ui)
	{
		queue_create_infos[ui].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[ui].pNext = nullptr;
		queue_create_infos[ui].pQueuePriorities = priorities;
		queue_create_infos[ui].queueCount = unique_queue_count[ui];
		queue_create_infos[ui].queueFamilyIndex = unique_queue_family_indices[ui];
		queue_create_infos[ui].flags = 0;
	}

	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures (physical_device, &device_features);

	VkDeviceCreateInfo device_create_info = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		unique_queue_family_index_count,
		queue_create_infos,
		0,
		nullptr,
		1,
		device_extensions,
		&device_features
	};


	VkResult vk_result = vkCreateDevice (physical_device, &device_create_info, nullptr, &device);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_GRAPHICS_DEVICE;
	}

	return age_result;
}

AGE_RESULT create_swapchain ()
{
	AGE_RESULT age_result = AGE_RESULT::SUCCESS;

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		surface,
		std::max<uint32_t> (surface_capabilities.minImageCount, 3),
		chosen_surface_format.format,
		chosen_surface_format.colorSpace,
		surface_capabilities.currentExtent,
		1,
		surface_capabilities.supportedUsageFlags,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		surface_capabilities.currentTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		chosen_present_mode,
		1,
		VK_NULL_HANDLE
	};

	VkResult vk_result = vkCreateSwapchainKHR (device, &swapchain_create_info, nullptr, &swapchain);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_RESULT::ERROR_GRAPHICS_CREATE_SWAPCHAIN;
	}

	return age_result;
}

AGE_RESULT create_swapchain_image_views ()
{
	VkResult vk_result = VK_SUCCESS;

	vkGetSwapchainImagesKHR (device, swapchain, &swapchain_image_count, nullptr);
	swapchain_images.resize (swapchain_image_count);
	vkGetSwapchainImagesKHR (device, swapchain, &swapchain_image_count, swapchain_images.data ());
	swapchain_image_views.resize (swapchain_image_count);

	VkImageSubresourceRange subresource_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	VkComponentMapping component_mapping = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	VkImageViewCreateInfo image_view_create_info = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,
		0,
		VK_NULL_HANDLE,
		VK_IMAGE_VIEW_TYPE_2D,
		chosen_surface_format.format,
		component_mapping,
		subresource_range
	};

	for (uint32_t i = 0; i < swapchain_image_count; ++i)
	{
		image_view_create_info.image = swapchain_images[i];
		vk_result = vkCreateImageView (device, &image_view_create_info, nullptr, &swapchain_image_views[i]);

		if (vk_result != VK_SUCCESS)
		{
			return AGE_RESULT::ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
		}
	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT get_device_queues ()
{
	uint32_t graphics_queue_index = 0;
	uint32_t compute_queue_index = graphics_queue_family_index == compute_queue_family_index ? 1 : 0;
	uint32_t transfer_queue_index = transfer_queue_family_index == compute_queue_family_index ? compute_queue_index + 1 : 0;

	vkGetDeviceQueue (device, graphics_queue_family_index, graphics_queue_index, &graphics_queue);
	vkGetDeviceQueue (device, compute_queue_family_index, compute_queue_index, &compute_queue);
	vkGetDeviceQueue (device, transfer_queue_family_index, transfer_queue_index, &transfer_queue);

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT create_graphics_command_pool ()
{
	VkCommandPoolCreateInfo graphics_command_pool_create_info = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		graphics_queue_family_index
	};

	VkResult vk_result = vkCreateCommandPool (device, &graphics_command_pool_create_info, nullptr, &graphics_command_pool);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_CREATE_COMMAND_POOL;
	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT create_common_sampler ()
{
	VkSamplerCreateInfo sampler_create_info = {
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		nullptr,
		0,
		VK_FILTER_LINEAR,
		VK_FILTER_LINEAR,
		VK_SAMPLER_MIPMAP_MODE_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		0,
		VK_FALSE,
		0,
		VK_FALSE,
		VK_COMPARE_OP_NEVER,
		0,
		0,
		VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		VK_FALSE
	};

	VkResult vk_result = vkCreateSampler (device, &sampler_create_info, nullptr, &common_sampler);
	if (vk_result != VK_SUCCESS)
	{
		return AGE_RESULT::ERROR_GRAPHICS_CREATE_SAMPLER;
	}

	return AGE_RESULT::SUCCESS;
}

AGE_RESULT vulkan_interface_init (HINSTANCE h_instance, HWND h_wnd)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

#ifdef _DEBUG 
	is_validation_needed = true;
#elif DEBUG
	is_validation_needed = true;
#else
	is_validation_needed = false;
#endif

	age_result = create_instance ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	if (is_validation_needed)
	{
		age_result = create_debug_utils_messenger ();

		if (age_result != AGE_RESULT::SUCCESS)
		{
			return age_result;
		}
	}

	age_result = create_surface (h_instance, h_wnd);
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = get_physical_device ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = check_physical_device_surface_support ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = get_physical_device_queue_families ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = get_physical_device_properties ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}
	
	age_result = get_physical_device_properties ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = create_device ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = create_swapchain ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}
	
	age_result = create_swapchain_image_views ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}
	
	age_result = get_device_queues ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = create_graphics_command_pool ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	age_result = create_common_sampler ();
	if (age_result != AGE_RESULT::SUCCESS)
	{
		return age_result;
	}

	return age_result;
}

void vulkan_interface_shutdown ()
{
	if (graphics_command_pool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool (device, graphics_command_pool, nullptr);
	}

	if (common_sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler (device, common_sampler, nullptr);
	}

	for (auto& swapchain_image_view : swapchain_image_views)
	{
		if (swapchain_image_view != VK_NULL_HANDLE)
		{
			vkDestroyImageView (device, swapchain_image_view, nullptr);
		}
	}
	swapchain_image_views.clear ();

	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR (device, swapchain, nullptr);
	}

	swapchain_images.clear ();

	if (device != VK_NULL_HANDLE)
	{
		vkDestroyDevice (device, nullptr);
	}

	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR (instance, surface, nullptr);
	}

	if (is_validation_needed)
	{
		if (debug_utils_messenger != VK_NULL_HANDLE)
		{
			destroy_instance_debug_utils_messenger (instance, debug_utils_messenger, nullptr);
		}
	}

    if (instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance (instance, nullptr);
    }
}
