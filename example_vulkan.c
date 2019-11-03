#ifdef _MSC_VER
#include <windows.h>
#endif
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#define IMDD_IMPLEMENTATION
#include "imdd.h"
#include "imdd_draw_vulkan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "example_common.h"
#include <time.h>

#define VERIFY(STMT)									\
	do { 												\
		if (!(STMT)) {									\
			fprintf(stderr, "failed: %s\n", #STMT);		\
			exit(-1);									\
		}												\
	} while (0)

#define VK_VERIFY(STMT)											\
	do { 														\
		VkResult res = STMT;									\
		if (res != VK_SUCCESS) {								\
			fprintf(stderr, "%s failed: %d\n", #STMT, res);		\
			exit(-1);											\
		}														\
	} while (0)

static
void vk_verify(VkResult imdd_vk_result)
{
	VK_VERIFY(imdd_vk_result);
}

#define EXAMPLE_COMMAND_BUFFER_COUNT	2
#define EXAMPLE_SWAPCHAIN_FORMAT		VK_FORMAT_B8G8R8A8_SRGB
#define EXAMPLE_SWAPCHAIN_COLORSPACE	VK_COLOR_SPACE_SRGB_NONLINEAR_KHR

#define EXAMPLE_DEPTH_FORMAT			VK_FORMAT_D16_UNORM

typedef struct example_t {
	VkInstance instance;
	VkPhysicalDevice physical_device;
	uint32_t queue_family_index;
	VkSurfaceKHR surface;

	VkDevice device;
	VkQueue queue;

	VkCommandPool command_pool;
	VkCommandBuffer command_buffers[EXAMPLE_COMMAND_BUFFER_COUNT];
	VkFence fences[EXAMPLE_COMMAND_BUFFER_COUNT];
	uint32_t command_buffer_index;

	VkSemaphore image_available_semaphore;
	VkSemaphore rendering_finished_semaphore;
	VkRenderPass render_pass;
} example_t;

typedef struct swapchain_t {
	VkExtent2D extent;
	VkSwapchainKHR self;
	uint32_t image_count;
	VkImage *images;
	VkDeviceMemory depth_device_memory;
	VkImage depth_image;
	VkImageView depth_image_view;
	VkImageView *image_views;
	VkFramebuffer *framebuffers;
	bool must_recreate;
} swapchain_t;

void example_init(
	example_t *ex,
	VkInstance instance,
	VkPhysicalDevice physical_device,
	uint32_t queue_family_index,
	VkSurfaceKHR surface)
{
	memset(ex, 0, sizeof(example_t));
	ex->instance = instance;
	ex->physical_device = physical_device;
	ex->queue_family_index = queue_family_index;
	ex->surface = surface;

	uint32_t surface_format_count = 0;
	VK_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, NULL));
	VERIFY(surface_format_count > 0);
	VkSurfaceFormatKHR *const surface_formats = calloc(surface_format_count, sizeof(VkSurfaceFormatKHR));
	VK_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_formats));

	float const queue_priority = 1.f;
	VkDeviceQueueCreateInfo device_queue_create_info;
	IMDD_VULKAN_SET_ZERO(device_queue_create_info);
	device_queue_create_info.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueFamilyIndex	= queue_family_index;
	device_queue_create_info.queueCount			= 1;
	device_queue_create_info.pQueuePriorities	= &queue_priority;
	uint32_t device_extension_count = 1;
	char const *const device_extensions[1] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	VkDeviceCreateInfo device_create_info;
	IMDD_VULKAN_SET_ZERO(device_create_info);
	device_create_info.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount		= 1;
	device_create_info.pQueueCreateInfos		= &device_queue_create_info;
	device_create_info.enabledExtensionCount	= device_extension_count;
	device_create_info.ppEnabledExtensionNames	= device_extensions;
	VK_VERIFY(vkCreateDevice(physical_device, &device_create_info, NULL, &ex->device));

	vkGetDeviceQueue(ex->device, queue_family_index, 0, &ex->queue);

	VkCommandPoolCreateInfo command_pool_create_info;
	IMDD_VULKAN_SET_ZERO(command_pool_create_info);
	command_pool_create_info.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_create_info.queueFamilyIndex	= queue_family_index;
	VK_VERIFY(vkCreateCommandPool(ex->device, &command_pool_create_info, NULL, &ex->command_pool));

	VkCommandBufferAllocateInfo command_buffer_allocate_info;
	IMDD_VULKAN_SET_ZERO(command_buffer_allocate_info);
	command_buffer_allocate_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool		= ex->command_pool;
	command_buffer_allocate_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount	= EXAMPLE_COMMAND_BUFFER_COUNT;
	VK_VERIFY(vkAllocateCommandBuffers(ex->device, &command_buffer_allocate_info, ex->command_buffers));

	for (uint32_t i = 0; i < EXAMPLE_COMMAND_BUFFER_COUNT; ++i) {
		VkFenceCreateInfo fence_create_info;
		IMDD_VULKAN_SET_ZERO(fence_create_info);
		fence_create_info.sType	= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags	= VK_FENCE_CREATE_SIGNALED_BIT;
		VK_VERIFY(vkCreateFence(ex->device, &fence_create_info, NULL, &ex->fences[i]));
	}

	VkSemaphoreCreateInfo semaphore_create_info;
	IMDD_VULKAN_SET_ZERO(semaphore_create_info);
	semaphore_create_info.sType	= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_create_info.flags	= 0;
	VK_VERIFY(vkCreateSemaphore(ex->device, &semaphore_create_info, NULL, &ex->image_available_semaphore));
	VK_VERIFY(vkCreateSemaphore(ex->device, &semaphore_create_info, NULL, &ex->rendering_finished_semaphore));

	VkAttachmentDescription attachments[2];
	IMDD_VULKAN_SET_ZERO(attachments);
	attachments[0].format			= EXAMPLE_SWAPCHAIN_FORMAT;
	attachments[0].samples			= VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachments[1].format			= EXAMPLE_DEPTH_FORMAT;
	attachments[1].samples			= VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_attachment_ref;
	IMDD_VULKAN_SET_ZERO(color_attachment_ref);
	color_attachment_ref.attachment		= 0;
	color_attachment_ref.layout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref;
	IMDD_VULKAN_SET_ZERO(depth_attachment_ref);
	depth_attachment_ref.attachment		= 1;
	depth_attachment_ref.layout			= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass;
	IMDD_VULKAN_SET_ZERO(subpass);
	subpass.pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount		= 1;
	subpass.pColorAttachments			= &color_attachment_ref;
	subpass.pDepthStencilAttachment		= &depth_attachment_ref;

	VkRenderPassCreateInfo render_pass_create_info;
	IMDD_VULKAN_SET_ZERO(render_pass_create_info);
	render_pass_create_info.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount		= 2;
	render_pass_create_info.pAttachments		= attachments;
	render_pass_create_info.subpassCount		= 1;
	render_pass_create_info.pSubpasses			= &subpass;
	VK_VERIFY(vkCreateRenderPass(ex->device, &render_pass_create_info, NULL, &ex->render_pass));
}

VkCommandBuffer example_begin_command_buffer(example_t *ex)
{
	ex->command_buffer_index = (1 + ex->command_buffer_index) % EXAMPLE_COMMAND_BUFFER_COUNT;

	VkCommandBuffer const command_buffer = ex->command_buffers[ex->command_buffer_index];
	VkFence const fence = ex->fences[ex->command_buffer_index];

	for (;;) {
		uint64_t const timeout_ns = 1000*1000*1000;
		VkResult const res = vkWaitForFences(ex->device, 1, &fence, VK_TRUE, timeout_ns);
		VERIFY(res == VK_SUCCESS || res == VK_TIMEOUT);
		if (res == VK_SUCCESS) {
			break;
		}
	}

	VK_VERIFY(vkResetFences(ex->device, 1, &fence));

	VkCommandBufferBeginInfo const command_buffer_begin_info = {
		.sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags	= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	VK_VERIFY(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

	return command_buffer;
}

void swapchain_create(swapchain_t *swapchain, example_t const *ex)
{
	memset(swapchain, 0, sizeof(swapchain_t));

	VkSurfaceCapabilitiesKHR surface_capabilities;
	IMDD_VULKAN_SET_ZERO(surface_capabilities);
	VK_VERIFY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ex->physical_device, ex->surface, &surface_capabilities));
	swapchain->extent = surface_capabilities.currentExtent;
	uint32_t const min_image_count = surface_capabilities.minImageCount;

	VkBool32 surface_supported = VK_FALSE;
	VK_VERIFY(vkGetPhysicalDeviceSurfaceSupportKHR(ex->physical_device, ex->queue_family_index, ex->surface, &surface_supported));
	VERIFY(surface_supported);

	VkSwapchainCreateInfoKHR swapchain_create_info;
	IMDD_VULKAN_SET_ZERO(swapchain_create_info);
	swapchain_create_info.sType					= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface				= ex->surface;
	swapchain_create_info.minImageCount			= min_image_count;
	swapchain_create_info.imageFormat			= EXAMPLE_SWAPCHAIN_FORMAT;
	swapchain_create_info.imageColorSpace		= EXAMPLE_SWAPCHAIN_COLORSPACE;
	swapchain_create_info.imageExtent			= swapchain->extent;
	swapchain_create_info.imageArrayLayers		= 1;
	swapchain_create_info.imageUsage			= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_create_info.imageSharingMode		= VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.queueFamilyIndexCount	= 1;
	swapchain_create_info.pQueueFamilyIndices	= &ex->queue_family_index;
	swapchain_create_info.preTransform			= VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchain_create_info.compositeAlpha		= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode			= VK_PRESENT_MODE_FIFO_KHR;
	swapchain_create_info.clipped				= VK_TRUE;
	VK_VERIFY(vkCreateSwapchainKHR(ex->device, &swapchain_create_info, NULL, &swapchain->self));

	VK_VERIFY(vkGetSwapchainImagesKHR(ex->device, swapchain->self, &swapchain->image_count, NULL));
	VERIFY(swapchain->image_count > 0);
	swapchain->images = calloc(swapchain->image_count, sizeof(VkImage));
	VK_VERIFY(vkGetSwapchainImagesKHR(ex->device, swapchain->self, &swapchain->image_count, swapchain->images));

	{
		VkImageCreateInfo image_create_info;
		IMDD_VULKAN_SET_ZERO(image_create_info);
		image_create_info.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.imageType		= VK_IMAGE_TYPE_2D;
		image_create_info.format		= EXAMPLE_DEPTH_FORMAT;
		image_create_info.extent.width	= swapchain->extent.width;
		image_create_info.extent.height	= swapchain->extent.height;
		image_create_info.extent.depth	= 1;
		image_create_info.mipLevels		= 1;
		image_create_info.arrayLayers	= 1;
		image_create_info.samples		= VK_SAMPLE_COUNT_1_BIT;
		image_create_info.usage			= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		VK_VERIFY(vkCreateImage(ex->device, &image_create_info, NULL, &swapchain->depth_image));

		VkMemoryRequirements mem_req;
		IMDD_VULKAN_SET_ZERO(mem_req);
		vkGetImageMemoryRequirements(ex->device, swapchain->depth_image, &mem_req);

		VkPhysicalDeviceMemoryProperties memory_properties;
		IMDD_VULKAN_SET_ZERO(memory_properties);
		vkGetPhysicalDeviceMemoryProperties(ex->physical_device, &memory_properties);

		uint32_t memory_type_index = 0;
		for (; memory_type_index < memory_properties.memoryTypeCount; ++memory_type_index) {
			if ((mem_req.memoryTypeBits & (1 << memory_type_index)) == 0) {
				continue;
			}
			VkMemoryPropertyFlags const property_flags = memory_properties.memoryTypes[memory_type_index].propertyFlags;
			if (property_flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
				break;
			}
		}

		VkMemoryAllocateInfo allocate_info;
		IMDD_VULKAN_SET_ZERO(allocate_info);
		allocate_info.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocate_info.allocationSize	= mem_req.size;
		allocate_info.memoryTypeIndex	= memory_type_index;
		VK_VERIFY(vkAllocateMemory(ex->device, &allocate_info, NULL, &swapchain->depth_device_memory));

		VK_VERIFY(vkBindImageMemory(ex->device, swapchain->depth_image, swapchain->depth_device_memory, 0));

		VkImageViewCreateInfo image_view_create_info;
		IMDD_VULKAN_SET_ZERO(image_view_create_info);
		image_view_create_info.sType						= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image						= swapchain->depth_image;
		image_view_create_info.viewType						= VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format						= EXAMPLE_DEPTH_FORMAT;
		image_view_create_info.subresourceRange.aspectMask	= VK_IMAGE_ASPECT_DEPTH_BIT;
		image_view_create_info.subresourceRange.levelCount	= 1;
		image_view_create_info.subresourceRange.layerCount	= 1;
		VK_VERIFY(vkCreateImageView(ex->device, &image_view_create_info, NULL, &swapchain->depth_image_view));
	}

	swapchain->image_views = calloc(swapchain->image_count, sizeof(VkImageView));
	swapchain->framebuffers = calloc(swapchain->image_count, sizeof(VkFramebuffer));
	for (uint32_t i = 0; i < swapchain->image_count; ++i) {
		VkImageViewCreateInfo image_view_create_info;
		IMDD_VULKAN_SET_ZERO(image_view_create_info);
		image_view_create_info.sType						= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image						= swapchain->images[i];
		image_view_create_info.viewType						= VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format						= EXAMPLE_SWAPCHAIN_FORMAT;
		image_view_create_info.subresourceRange.aspectMask	= VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.levelCount	= 1;
		image_view_create_info.subresourceRange.layerCount	= 1;
		VK_VERIFY(vkCreateImageView(ex->device, &image_view_create_info, NULL, &swapchain->image_views[i]));

		VkImageView attachments[2];
		IMDD_VULKAN_SET_ZERO(attachments);
		attachments[0] = swapchain->image_views[i];
		attachments[1] = swapchain->depth_image_view;

		VkFramebufferCreateInfo framebuffer_create_info;
		IMDD_VULKAN_SET_ZERO(framebuffer_create_info);
		framebuffer_create_info.sType				= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.renderPass			= ex->render_pass;
		framebuffer_create_info.attachmentCount		= 2;
		framebuffer_create_info.pAttachments		= attachments;
		framebuffer_create_info.width				= swapchain->extent.width;
		framebuffer_create_info.height				= swapchain->extent.height;
		framebuffer_create_info.layers				= 1;
		VK_VERIFY(vkCreateFramebuffer(ex->device, &framebuffer_create_info, NULL, &swapchain->framebuffers[i]));
	}
}

void swapchain_destroy(swapchain_t *swapchain, example_t const *ex)
{
	for (uint32_t i = 0; i < swapchain->image_count; ++i) {
		vkDestroyFramebuffer(ex->device, swapchain->framebuffers[i], NULL);
		vkDestroyImageView(ex->device, swapchain->image_views[i], NULL);
	}
	vkDestroySwapchainKHR(ex->device, swapchain->self, NULL);
	free(swapchain->framebuffers);
	free(swapchain->image_views);
	free(swapchain->images);
}

uint32_t swapchain_acquire(swapchain_t *swapchain, example_t const *ex)
{
	for (;;) {
		if (swapchain->must_recreate) {
			vkDeviceWaitIdle(ex->device);
			swapchain_destroy(swapchain, ex);
			swapchain_create(swapchain, ex);
		}

		uint32_t image_index = 0;
		VkResult const res = vkAcquireNextImageKHR(ex->device, swapchain->self, UINT64_MAX, ex->image_available_semaphore, VK_NULL_HANDLE, &image_index);
		switch (res) {
			case VK_ERROR_OUT_OF_DATE_KHR:
				swapchain->must_recreate = true;
				break;

			case VK_SUCCESS:
			case VK_SUBOPTIMAL_KHR:
				return image_index;

			default:
				fprintf(stderr, "failed: vkAcquireNextImageKHR returned %d\n", res);
				exit(-1);
		}
		return image_index;
	}
}

void swapchain_submit_and_present(swapchain_t *swapchain, example_t const *ex, uint32_t image_index)
{
	VkCommandBuffer const command_buffer = ex->command_buffers[ex->command_buffer_index];
	VkFence const fence = ex->fences[ex->command_buffer_index];

	VkPipelineStageFlags const wait_dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submit_info;
	IMDD_VULKAN_SET_ZERO(submit_info);
	submit_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount		= 1;
	submit_info.pWaitSemaphores			= &ex->image_available_semaphore;
	submit_info.pWaitDstStageMask		= &wait_dst_stage_mask;
	submit_info.commandBufferCount		= 1;
	submit_info.pCommandBuffers			= &command_buffer;
	submit_info.signalSemaphoreCount	= 1;
	submit_info.pSignalSemaphores		= &ex->rendering_finished_semaphore;
	VK_VERIFY(vkQueueSubmit(ex->queue, 1, &submit_info, fence));

	VkPresentInfoKHR present_info;
	IMDD_VULKAN_SET_ZERO(present_info);
	present_info.sType					= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount		= 1;
	present_info.pWaitSemaphores		= &ex->rendering_finished_semaphore;
	present_info.swapchainCount			= 1;
	present_info.pSwapchains			= &swapchain->self;
	present_info.pImageIndices			= &image_index;
	VkResult const res = vkQueuePresentKHR(ex->queue, &present_info);
	switch (res) {
		case VK_ERROR_OUT_OF_DATE_KHR:
			swapchain->must_recreate = true;
			break;

		case VK_SUCCESS:
		case VK_SUBOPTIMAL_KHR:
			// success
			break;

		default:
			fprintf(stderr, "failed: vkQueuePresentKHR returned %d\n", res);
			exit(-1);
	}
}

void error_callback(int error, char const *description)
{
	UNUSED(error);
	fprintf(stderr, "GLFW: %s\n", description);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	UNUSED(scancode);
	UNUSED(mods);
	if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GL_TRUE);;
				break;
		}
	}
}

VkBool32 debug_utils_messenger_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    VkDebugUtilsMessengerCallbackDataEXT const* callback_data,
    void *user_data)
{
	UNUSED(user_data);
	if (callback_data->pMessage) {
		printf("%04x/%01x: %s\n", message_severity, message_types, callback_data->pMessage);
	}
	return VK_FALSE;
}

double time_now(void)
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (double)ts.tv_sec + (1.0/(1000.0*1000.0*1000.0))*(double)ts.tv_nsec;
}

int main(int argc, char *argv[])
{
	bool is_debug = false;
	bool is_perf = false;
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-d") == 0) {
			is_debug = true;
		} else if (strcmp(argv[i], "-p") == 0) {
			is_perf = true;
		} else {
			fprintf(stderr, "unknown argument: %s\n", argv[i]);
			exit(-1);
		}
	}

	glfwSetErrorCallback(&error_callback);
	VERIFY(glfwInit());
	VERIFY(glfwVulkanSupported());

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow *const window = glfwCreateWindow(960, 544, "imdd example", NULL, NULL);
	VERIFY(window);
	glfwSetKeyCallback(window, &key_callback);

	example_t ex;
	{
		uint32_t instance_extension_count = 0;
		char const **glfw_extensions = glfwGetRequiredInstanceExtensions(&instance_extension_count);
		char const **instance_extensions = NULL;
		if (is_debug) {
			instance_extensions = calloc(1 + instance_extension_count, sizeof(char const *));
			for (uint32_t i = 0; i < instance_extension_count; ++i) {
				instance_extensions[i] = glfw_extensions[i];
			}
			instance_extensions[instance_extension_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
			++instance_extension_count;
		} else {
			instance_extensions = glfw_extensions;
		}

		uint32_t layer_count = is_debug ? 1 : 0;
		char const *const layers[1] = {
			"VK_LAYER_KHRONOS_validation"
		};

		VkInstanceCreateInfo instance_create_info;
		IMDD_VULKAN_SET_ZERO(instance_create_info);
		instance_create_info.sType						= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.enabledLayerCount			= layer_count;
		instance_create_info.ppEnabledLayerNames		= layers;
		instance_create_info.enabledExtensionCount		= instance_extension_count;
		instance_create_info.ppEnabledExtensionNames	= instance_extensions;
		VkInstance instance = VK_NULL_HANDLE;
		VK_VERIFY(vkCreateInstance(&instance_create_info, NULL, &instance));

		if (is_debug) {
			PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

			VkDebugUtilsMessengerCreateInfoEXT create_info;
			IMDD_VULKAN_SET_ZERO(create_info);
			create_info.sType			= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			create_info.messageSeverity	= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
										| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
										;
			create_info.messageType		= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
										| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
										| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
										;
			create_info.pfnUserCallback	= &debug_utils_messenger_callback;

			VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
			VK_VERIFY(vkCreateDebugUtilsMessengerEXT(instance, &create_info, NULL, &messenger));
		}

		uint32_t physical_device_count = 0;
		VK_VERIFY(vkEnumeratePhysicalDevices(instance, &physical_device_count, 0));
		VERIFY(physical_device_count > 0);
		VkPhysicalDevice *const physical_devices = calloc(physical_device_count, sizeof(VkPhysicalDevice));
		VK_VERIFY(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices));
		VkPhysicalDevice const physical_device = physical_devices[0];

		uint32_t const queue_family_index = 0;
		VERIFY(glfwGetPhysicalDevicePresentationSupport(instance, physical_device, queue_family_index));

		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VK_VERIFY(glfwCreateWindowSurface(instance, window, NULL, &surface));

		example_init(&ex, instance, physical_device, queue_family_index, surface);
	}

	uint32_t const shape_count = 64*1024;
	uint32_t const shape_mem_size = IMDD_APPROX_SHAPE_SIZE_IN_BYTES*shape_count;
	imdd_shape_store_t *const store = imdd_init(malloc(shape_mem_size), shape_mem_size);

	imdd_vulkan_context_t ctx;
	{
		imdd_vulkan_fp_t fp;
		IMDD_VULKAN_SET_GLOBAL_FP(&fp);

		imdd_vulkan_init(
			&ctx, shape_count, shape_count, shape_count,
			&fp, &vk_verify, ex.physical_device, ex.device, 0);
	}

	swapchain_t swapchain;
	swapchain_create(&swapchain, &ex);

	imdd_vulkan_create_pipelines(
		&ctx,
		ex.device,
		ex.render_pass,
		VK_SAMPLE_COUNT_1_BIT);

	float angle = 0.f;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// reset debug draw and emit test shapes
		imdd_reset(store);

		double const emit_time_start = time_now();
		if (is_perf) {
			imdd_perf_test(store);
		} else {
			imdd_example_test(store);
		}
		double const emit_time_diff = time_now() - emit_time_start;

		// start the vulkan frame
		VkCommandBuffer const command_buffer = example_begin_command_buffer(&ex);
		uint32_t const image_index = swapchain_acquire(&swapchain, &ex);

		// flush shapes to vulkan before the render pass begins
		double const update_time_start = time_now();
		imdd_shape_store_t const *draw_stores = store;
		imdd_vulkan_update(&ctx, &draw_stores, 1, ex.device, command_buffer);
		double const update_time_diff = time_now() - update_time_start;

		// start the render pass
		VkClearValue clear_values[2];
		clear_values[0].color.float32[0]		= .1f;
		clear_values[0].color.float32[1]		= .1f;
		clear_values[0].color.float32[2]		= .1f;
		clear_values[0].color.float32[3] 		= 1.f;
		clear_values[1].depthStencil.depth		= 1.f;
		clear_values[1].depthStencil.stencil	= 0;

		VkRenderPassBeginInfo render_pass_begin_info;
		IMDD_VULKAN_SET_ZERO(render_pass_begin_info);
		render_pass_begin_info.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass			= ex.render_pass;
		render_pass_begin_info.framebuffer			= swapchain.framebuffers[image_index];
		render_pass_begin_info.renderArea.extent	= swapchain.extent;
		render_pass_begin_info.clearValueCount		= 2;
		render_pass_begin_info.pClearValues			= clear_values;
		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		// spin the camera
		angle += .5f*PI/60.f;
		mat4 proj_from_world;
		mat4_identity(&proj_from_world);
		mat4_rotate_y(&proj_from_world, angle);
		mat4_rotate_x(&proj_from_world, PI/8.f);
		mat4_translation(&proj_from_world, 0.f, 0.f, -22.f);
		mat4_perspective_vk(&proj_from_world, PI/8.f, (float)swapchain.extent.width/(float)swapchain.extent.height, .1f, 100.f);

		// set dynamic state expected by imdd
		VkViewport viewport;
		IMDD_VULKAN_SET_ZERO(viewport);
		viewport.width = (float)swapchain.extent.width;
		viewport.height = (float)swapchain.extent.height;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		VkRect2D scissor;
		IMDD_VULKAN_SET_ZERO(scissor);
		scissor.extent = swapchain.extent;
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);

		// draw the shapes
		imdd_vulkan_draw(&ctx, proj_from_world.m[0], ex.device, command_buffer);

		// end the pass, frame and submit it
		vkCmdEndRenderPass(command_buffer);
		VK_VERIFY(vkEndCommandBuffer(command_buffer));
		swapchain_submit_and_present(&swapchain, &ex, image_index);

		// show perf timings
		if (is_perf) {
			printf("emit time: %.1f us, update time: %.1f us\n",
				1000.0*1000.0*emit_time_diff,
				1000.0*1000.0*update_time_diff);
		}
	}
	return 0;
}
