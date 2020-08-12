#pragma once

#include "error.hpp"

#include <Windows.h>
#include <vulkan/vulkan.h>


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
VkImage* swapchain_images;
VkImageView* swapchain_image_views;
size_t swapchain_image_count;
VkExtent2D current_extent;
VkSurfaceFormatKHR chosen_surface_format;
VkCommandPool transfer_command_pool;
VkSampler common_sampler;

AGE_RESULT vulkan_interface_init (HINSTANCE h_instance, HWND h_wnd);
void vulkan_interface_shutdown (void);