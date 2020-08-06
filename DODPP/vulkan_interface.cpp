#include "vulkan_interface.hpp"
#include "utils.hpp"

#include <stdio.h>
#include <vulkan/vulkan_win32.h>

bool is_validation_needed = false;
VkDebugUtilsMessengerEXT debug_utils_messenger = VK_NULL_HANDLE;
VkSurfaceKHR surface = VK_NULL_HANDLE;
VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
VkPresentModeKHR chosen_present_mode;

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
		printf ("Debug Callback Message: %s\n\n", callback_data->pMessage);
	}

	return false;
}


AGE_RESULT vulkan_interface_init (HINSTANCE h_instance, HWND h_wnd)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;
    VkResult vk_result = VK_SUCCESS;

exit:
    return age_result;
}

void vulkan_interface_shutdown ()
{
	if (swapchain_image_views)
	{
		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			if (swapchain_image_views[i] != VK_NULL_HANDLE)
			{
				vkDestroyImageView (device, swapchain_image_views[i], NULL);
			}
		}

		utils_free (swapchain_image_views);
	}

	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR (device, swapchain, NULL);
	}

	utils_free (swapchain_images);

	if (device != VK_NULL_HANDLE)
	{
		vkDestroyDevice (device, NULL);
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
