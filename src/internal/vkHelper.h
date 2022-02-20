/*
* @author:	Zachary Tay
* @date:	20/02/21
* @brief:	vulkan midterm
*/

#pragma once
#define VK_USE_PLATFORM_WIN32_KHR 1
#include <vulkan/vulkan.h>
#include <vector>
#include <Windows.h>
#include <optional>
#include <array>
#include <unordered_map>

namespace vkHelper
{
	/*!
	 * @brief holds all relevant swap chain objects
	*/
	struct vkSwapChainData
	{
		VkSwapchainKHR				swapchain_ { VK_NULL_HANDLE };
		VkExtent2D					extent_;
		VkFormat					format_;
		std::vector<VkImage>		images_;
		std::vector<VkImageView>	image_views_;
	};

	/*!
	 * @brief holds all pipeline objects
	*/
	struct vkPipelineData
	{
		VkPipeline			pipeline_;
		VkPipelineLayout	layout_;
	};

	/*!
	 * @brief holds all sync objects
	*/
	struct vkSyncObjects
	{
		std::vector<VkSemaphore>	available_semaphores_;
		std::vector<VkSemaphore>	finished_semaphores_;
		std::vector<VkFence>		in_flight_fences_;
		std::vector<VkFence>		images_in_flight_;
	};

	namespace Create
	{
		/*!
		 * @brief creates a vkinstance
		*/
		bool				vkInstance ( char const* name , VkInstance& instance , int flags );

		/*!
		 * @brief creates a vkDebugMessenger
		*/
		bool				vkDebugMessenger ( VkInstance instance , VkDebugUtilsMessengerEXT& debugMessenger );

		/*!
		 * @brief creates a vkSurfaceWin32
		*/
		bool				vkSurfaceWin32 ( VkInstance instance , HWND hWnd , VkSurfaceKHR& surface );

		/*!
		 * @brief creates a vkPhysicalDevice
		*/
		VkPhysicalDevice	vkPhysicalDevice ( VkInstance instance , VkSurfaceKHR surface );

		/*!
		 * @brief creates a vkLogicalDevice
		*/
		VkDevice			vkLogicalDevice ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , int flags );

		/*!
		 * @brief creates a vkGraphicsQueue
		*/
		VkQueue				vkGraphicsQueue ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , VkDevice logicalDevice );

		/*!
		 * @brief creates a vkPresentQueue
		*/
		VkQueue				vkPresentQueue ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , VkDevice logicalDevice );

		/*!
		 * @brief creates a vkSwapChain
		*/
		vkSwapChainData		vkSwapChain ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , VkDevice logicalDevice );

		/*!
		 * @brief creates a vkRenderPass
		*/
		VkRenderPass		vkRenderPass ( VkDevice logicalDevice , VkFormat imageFormat );

		/*!
		 * @brief creates a vkGraphicsPipeline
		*/
		vkPipelineData		vkGraphicsPipeline ( VkDevice logicalDevice , vkSwapChainData swapChainData , VkRenderPass renderPass );

		/*!
		 * @brief creates a vkFramebuffers
		*/
		bool				vkFramebuffers ( VkDevice logicalDevice , vkSwapChainData& swapChainData , VkRenderPass renderPass , std::vector<VkFramebuffer>& framebuffers );

		/*!
		 * @brief creates a vkCommandPool
		*/
		VkCommandPool		vkCommandPool ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , VkDevice logicalDevice );

		/*!
		 * @brief creates a vkCommandBuffers
		*/
		bool				vkCommandBuffers ( VkDevice logicalDevice , vkSwapChainData swapChain , VkRenderPass renderPass , vkPipelineData graphicsPipeline , std::vector<VkFramebuffer>& framebuffers , VkCommandPool commandPool , std::vector<VkCommandBuffer>& commandBuffers );

		/*!
		 * @brief creates a SyncObjects
		*/
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
		bool				SyncObjects ( VkDevice logicalDevice , vkSwapChainData swapChain , vkSyncObjects& syncObjects );
	}

	namespace Check
	{
		/*!
		 * @brief checks vkLayersSupport
		*/
		bool vkLayersSupport ( std::vector<char const*> requestedLayers );

		/*!
		 * @brief checks InstanceExtensionsSupport
		*/
		bool InstanceExtensionsSupport ( bool debug );

		/*!
		 * @brief checks DeviceExtensionsSupport
		*/
		bool DeviceExtensionsSupport ( VkPhysicalDevice device );

		/*!
		 * @brief checks SwapChainSupport
		*/
		bool SwapChainSupport ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface );

		/*!
		 * @brief checks PhysicalDeviceSuitable
		*/
		bool PhysicalDeviceSuitable ( VkPhysicalDevice device , VkSurfaceKHR surface );
	}

	namespace Get
	{
		/*!
		 * @brief vulkan instance extension flags
		*/
		enum class VKLAYER
		{
			KHRONOS_VALIDATION = 1 ,
			RENDERDOC_CAPTURE = 2
		};

		static std::unordered_map<VKLAYER , char const*> vk_layers_
		{
			{ VKLAYER::KHRONOS_VALIDATION , "VK_LAYER_KHRONOS_validation" },
			{ VKLAYER::RENDERDOC_CAPTURE , "VK_LAYER_RENDERDOC_Capture" }
		};

		/*!
		 * @brief get a vkLayer
		*/
		char const* vkLayer ( VKLAYER layer );

		/*!
		 * @brief get all validation layers based on flags
		*/
		std::vector<const char*> ValidationLayers ( int flags );

		/*!
		 * @brief get all instance extensions based on debug mode
		*/
		std::vector<char const*> InstanceExtensions ( bool debug );

		/*!
		 * @brief get all device extensions
		*/
		std::vector<char const*> DeviceExtensions ();

		/*!
		 * @brief object that checks if all queue families are ready
		*/
		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphics_family_;
			std::optional<uint32_t> present_family_;

			bool IsComplete ()
			{
				return graphics_family_.has_value ()
					&& present_family_.has_value ();
			}
		};
		QueueFamilyIndices QueueFamilies ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface );

		/*!
		 * @brief object that holds all swap chain support details
		*/
		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities_;
			std::vector<VkSurfaceFormatKHR> formats_;
			std::vector<VkPresentModeKHR> present_modes_;
		};

		/*!
		 * @brief gets swap chain support details
		*/
		SwapChainSupportDetails		SwapChainSupportDetails_f ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface );

		/*!
		 * @brief gets swap chain surface format
		*/
		VkSurfaceFormatKHR			vkSwapChainSurfaceFormat ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface );

		/*!
		 * @brief gets swap chain present mode
		*/
		VkPresentModeKHR			vkSwapChainPresentMode ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface );

		/*!
		 * @brief gets swap chain extent2D
		*/
		VkExtent2D					vkSwapChainExtent2D ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface );

		/*!
		 * @brief gets swap chain images
		*/
		std::vector<VkImage>		vkSwapChainImages ( VkDevice logicalDevice , VkSwapchainKHR swapChain );

		/*!
		 * @brief gets swap chain image views
		*/
		std::vector<VkImageView>	vkSwapChainImageViews ( VkDevice logicalDevice , std::vector<VkImage> const& swapChainImages , VkFormat swapChainImageFormat );
	}

	namespace Debug
	{
		/*!
		 * @brief debug callback
		*/
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback ( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity ,
			VkDebugUtilsMessageTypeFlagsEXT messageType ,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData ,
			void* pUserData );

		/*!
		 * @brief populates debug messenger create info
		*/
		void		PopulateDebugMessengerCreateInfo ( VkDebugUtilsMessengerCreateInfoEXT& createInfo );

		/*!
		 * @brief creates debug utils messenger
		*/
		VkResult	CreateDebugUtilsMessengerEXT ( VkInstance instance ,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo ,
			const VkAllocationCallbacks* pAllocator ,
			VkDebugUtilsMessengerEXT* pDebugMessenger );

		/*!
		 * @brief destroys debug messenger
		*/
		void		DestroyDebugUtilsMessengerEXT ( VkInstance instance ,
			VkDebugUtilsMessengerEXT debugMessenger ,
			const VkAllocationCallbacks* pAllocator );
	}

	namespace IO
	{
		/*!
		 * @brief reads a txt file for shader code 
		*/
		std::vector<char>	ReadFile ( std::string const& filename );

		/*!
		 * @brief compiles the shader code into a shader module
		*/
		VkShaderModule		CreateShaderModule ( VkDevice logicalDevice , std::vector<char> const& code );
	}

	namespace Misc
	{
		/*!
		 * @brief draws a vulkan frame 
		*/
		void DrawFrame ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , VkDevice logicalDevice , VkQueue graphicsQueue , VkQueue presentQueue , vkSwapChainData& swapChain , VkRenderPass& renderPass , vkPipelineData& graphicsPipeline ,
			std::vector<VkFramebuffer>& framebuffers , VkCommandPool commandPool , std::vector<VkCommandBuffer>& commandBuffers , vkSyncObjects& syncObjects , size_t& currentFrame );

		/*!
		 * @brief recreates the swap chain
		*/
		void RecreateSwapChain ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , VkDevice logicalDevice , vkSwapChainData& swapChain , VkRenderPass& renderPass , vkPipelineData& graphicsPipeline ,
			std::vector<VkFramebuffer>& framebuffers , VkCommandPool commandPool , std::vector<VkCommandBuffer>& commandBuffers );

		/*!
		 * @brief clean up the swap chain
		*/
		void CleanUpSwapChain ( VkDevice logicalDevice , vkSwapChainData& swapChain , VkRenderPass renderPass , vkPipelineData& graphicsPipeline , std::vector<VkFramebuffer>& framebuffers , VkCommandPool commandPool , std::vector<VkCommandBuffer>& commandBuffers );
	}
}