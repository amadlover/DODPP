#pragma once

#include "error.hpp"

#include <vulkan/vulkan.h>
#include <Windows.h>

static VkInstance instance;
static VkPhysicalDevice physical_device;
static VkDevice device;
static uint32_t graphics_queue_family_index;
static uint32_t compute_queue_family_index;
static uint32_t transfer_queue_family_index;
static VkQueue graphics_queue;
static VkQueue compute_queue;
static VkQueue transfer_queue;
static VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
static VkPhysicalDeviceLimits physical_device_limits;
static VkExtent2D surface_extent;
static VkSwapchainKHR swapchain;
static VkImage* swapchain_images;
static VkImageView* swapchain_image_views;
static size_t swapchain_image_count;
static VkExtent2D current_extent;
static VkSurfaceFormatKHR chosen_surface_format;

AGE_RESULT vulkan_interface_init (HINSTANCE h_instance, HWND h_wnd);
void vulkan_interface_shutdown ();