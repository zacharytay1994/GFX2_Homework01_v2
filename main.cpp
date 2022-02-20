/*
* @author:	Zachary Tay
* @date:	20/02/21
* @brief:	vulkan midterm
*/

#include <vulkan/vulkan.h>
#include <cstring>
#include <iostream>
#include <exception>

#include "src/internal/vkHelper.h"
#include "src/internal/wndHelper.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

int main ( int argc , char* argv[] )
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode ( _CRT_WARN , _CRTDBG_MODE_DEBUG );
#endif

	bool enable_validation_ { false };
	bool enable_renderdoc_ { false };

	for ( int i = 0; i < argc; ++i )
	{
		if ( !strcmp ( argv[ i ] , "-d" ) )
		{
			enable_validation_ = true;
		}
		else if ( !strcmp ( argv[ i ] , "-r" ) )
		{
			enable_renderdoc_ = true;
		}
	}

	char ans;
	std::cout << "Validation layers? (Y/N)" << std::endl;
	std::cin >> ans;
	if ( ans == 'Y' || ans == 'y' )
	{
		enable_validation_ = true;
	}
	else
	{
		enable_validation_ = false;
	}

	std::cout << "RenderDoc? (Y/N)" << std::endl;
	std::cin >> ans;
	if ( ans == 'Y' || ans == 'y' )
	{
		enable_renderdoc_ = true;
	}
	else
	{
		enable_renderdoc_ = false;
	}

	// create vulkan instance
	VkInstance vk_instance { nullptr };
	int flags { 0 };
	if ( enable_validation_ )
	{
		flags |= static_cast< int >( vkHelper::Get::VKLAYER::KHRONOS_VALIDATION );
	}
	if ( enable_renderdoc_ )
	{
		flags |= static_cast< int >( vkHelper::Get::VKLAYER::RENDERDOC_CAPTURE );
	}
	if ( !vkHelper::Create::vkInstance ( "Homework1" , vk_instance , flags ) )
	{
		throw std::runtime_error ( "Failed to create vkinstance!" );
	}
	std::cout << "### VkInstance created successfully." << std::endl;

	// create debug messenger
	VkDebugUtilsMessengerEXT vk_debug_messenger { VK_NULL_HANDLE };
	if ( enable_validation_ )
	{
		if ( !vkHelper::Create::vkDebugMessenger ( vk_instance , vk_debug_messenger ) )
		{
			throw std::runtime_error ( "Failed to create debug messenger!" );
		}
	}
	std::cout << "### VkDebugMessenger created successfully." << std::endl;

	// create window
	wndHelper::Window window;
	if ( !window.Initialize ( { 500, 300, false } ) )
	{
		throw std::runtime_error ( "Failed to initialize window!" );
	}
	std::cout << "### Window created successfully." << std::endl;

	// create surface
	VkSurfaceKHR vk_surface { VK_NULL_HANDLE };
	if ( !vkHelper::Create::vkSurfaceWin32 ( vk_instance , window.GetHandle () , vk_surface ) )
	{
		throw std::runtime_error ( "Failed to create VkSurfaceKHR!" );
	}
	std::cout << "### VkSurface created successfully." << std::endl;

	// create physical device
	VkPhysicalDevice vk_physical_device { VK_NULL_HANDLE };
	if ( ( vk_physical_device = vkHelper::Create::vkPhysicalDevice ( vk_instance , vk_surface ) ) == VK_NULL_HANDLE )
	{
		throw std::runtime_error ( "Failed to create VkPhysicalDevice!" );
	}
	std::cout << "### VkPhysicalDevice created successfully." << std::endl;

	// create logical device
	VkDevice vk_logical_device { VK_NULL_HANDLE };
	if ( ( vk_logical_device = vkHelper::Create::vkLogicalDevice ( vk_physical_device , vk_surface , flags ) ) == VK_NULL_HANDLE )
	{
		throw std::runtime_error ( "Failed to create VkDevice logical!" );
	}
	std::cout << "### VkDevice logical created successfully." << std::endl;

	// create graphics queue
	VkQueue vk_graphics_queue { VK_NULL_HANDLE };
	if ( ( vk_graphics_queue = vkHelper::Create::vkGraphicsQueue ( vk_physical_device , vk_surface , vk_logical_device ) ) == VK_NULL_HANDLE )
	{
		throw std::runtime_error ( "Failed to create graphics queue!" );
	}
	std::cout << "### VkQueue graphics created successfully." << std::endl;

	// create present queue
	VkQueue vk_present_queue { VK_NULL_HANDLE };
	if ( ( vk_present_queue = vkHelper::Create::vkPresentQueue ( vk_physical_device , vk_surface , vk_logical_device ) ) == VK_NULL_HANDLE )
	{
		throw std::runtime_error ( "Failed to create present queue!" );
	}
	std::cout << "### VkQueue present created successfully." << std::endl;

	// create swap chain
	vkHelper::vkSwapChainData vk_swapchain_data;
	if ( ( vk_swapchain_data = vkHelper::Create::vkSwapChain ( vk_physical_device , vk_surface , vk_logical_device ) ).swapchain_ == VK_NULL_HANDLE )
	{
		throw std::runtime_error ( "Failed to create VkSwapchain" );
	}
	std::cout << "### VkSwapchain created successfully." << std::endl;

	// create render pass
	VkRenderPass vk_render_pass { VK_NULL_HANDLE };
	if ( ( vk_render_pass = vkHelper::Create::vkRenderPass ( vk_logical_device , vk_swapchain_data.format_ ) ) == VK_NULL_HANDLE )
	{
		throw std::runtime_error ( "Failed to create VkRenderPass" );
	}
	std::cout << "### VkRenderPass created successfully." << std::endl;

	// create graphics pipeline
	vkHelper::vkPipelineData vk_graphics_pipeline;
	if ( ( vk_graphics_pipeline = vkHelper::Create::vkGraphicsPipeline ( vk_logical_device , vk_swapchain_data , vk_render_pass ) ).pipeline_ == VK_NULL_HANDLE )
	{
		throw std::runtime_error ( "Failed to create VkPipeline" );
	}
	std::cout << "### VkGraphicsPipeline created successfully." << std::endl;

	// create swap chain framebuffers
	std::vector<VkFramebuffer> vk_framebuffers;
	if ( !vkHelper::Create::vkFramebuffers ( vk_logical_device , vk_swapchain_data , vk_render_pass , vk_framebuffers ) )
	{
		throw std::runtime_error ( "Failed to create VkFramebuffers" );
	}
	std::cout << "### VkFramebuffers created successfully." << std::endl;

	// create command pool
	VkCommandPool vk_command_pool;
	if ( ( vk_command_pool = vkHelper::Create::vkCommandPool ( vk_physical_device , vk_surface , vk_logical_device ) ) == VK_NULL_HANDLE )
	{
		throw std::runtime_error ( "Failed to create VkCommandPool" );
	}
	std::cout << "### VkCommandPool created successfully." << std::endl;

	// create command buffers
	std::vector<VkCommandBuffer> vk_command_buffers;
	if ( !vkHelper::Create::vkCommandBuffers ( vk_logical_device , vk_swapchain_data , vk_render_pass , vk_graphics_pipeline , vk_framebuffers , vk_command_pool , vk_command_buffers ) )
	{
		throw std::runtime_error ( "Failed to create command buffers" );
	}
	std::cout << "### VkCommandBuffers created successfully." << std::endl;

	// create sync objects
	vkHelper::vkSyncObjects vk_sync_objects;
	if ( !vkHelper::Create::SyncObjects ( vk_logical_device , vk_swapchain_data , vk_sync_objects ) )
	{
		throw std::runtime_error ( "Failed to create vkSyncObjects" );
	}
	std::cout << "### vkSyncObjects created successfully." << std::endl;

	std::cout << "### Setup complete.\n### Press any key to continue!" << std::endl;

	size_t current_frame { 0 };
	while ( !window.WindowShouldClose () )
	{
		window.PollEvents ();
		if ( window.WindowShouldClose () )
		{
			break;
		}

		// process vulkan draw logic
		vkHelper::Misc::DrawFrame (
			vk_physical_device ,
			vk_surface ,
			vk_logical_device ,
			vk_graphics_queue ,
			vk_present_queue ,
			vk_swapchain_data ,
			vk_render_pass ,
			vk_graphics_pipeline ,
			vk_framebuffers ,
			vk_command_pool ,
			vk_command_buffers ,
			vk_sync_objects ,
			current_frame );
	}

	vkDeviceWaitIdle ( vk_logical_device );

	// clean up code
	vkHelper::Misc::CleanUpSwapChain (
		vk_logical_device, 
		vk_swapchain_data, 
		vk_render_pass, 
		vk_graphics_pipeline, 
		vk_framebuffers, 
		vk_command_pool, 
		vk_command_buffers
	);

	vkDestroyCommandPool ( vk_logical_device , vk_command_pool , nullptr );

	for ( size_t i = 0; i < vkHelper::Create::MAX_FRAMES_IN_FLIGHT; i++ )
	{
		vkDestroySemaphore ( vk_logical_device , vk_sync_objects.finished_semaphores_[ i ] , nullptr );
		vkDestroySemaphore ( vk_logical_device , vk_sync_objects.available_semaphores_[ i ] , nullptr );
		vkDestroyFence ( vk_logical_device , vk_sync_objects.in_flight_fences_[ i ] , nullptr );
	}

	vkDestroyDevice ( vk_logical_device , nullptr );

	// destroy debug messenger
	if ( enable_validation_ )
	{
		vkHelper::Debug::DestroyDebugUtilsMessengerEXT ( vk_instance , vk_debug_messenger , nullptr );
	}

	// destroy surface, happens before destroy instance
	vkDestroySurfaceKHR ( vk_instance , vk_surface , nullptr );

	// destroy vkinstance before program exits
	vkDestroyInstance ( vk_instance , nullptr );

	return 1;
}