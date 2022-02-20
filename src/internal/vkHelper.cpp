/*
* @author:	Zachary Tay
* @date:	20/02/21
* @brief:	vulkan midterm
*/

#include "vkHelper.h"

#include <iostream>
#include <fstream>
#include <set>
#include <assert.h>
#include <algorithm>

#include "wndHelper.h"

namespace vkHelper
{
	namespace Create
	{
		bool vkInstance ( char const* name , VkInstance& instance , int flags )
		{
			// get and check validation and render doc layers
			bool enable_validation = flags & static_cast< int >( Get::VKLAYER::KHRONOS_VALIDATION );
			bool enable_renderdoc = flags & static_cast< int >( Get::VKLAYER::RENDERDOC_CAPTURE );

			std::vector<const char*> vk_layers;
			if ( enable_validation )
			{
				vk_layers.emplace_back ( Get::vkLayer ( Get::VKLAYER::KHRONOS_VALIDATION ) );

				if ( enable_renderdoc )
				{
					vk_layers.emplace_back ( Get::vkLayer ( Get::VKLAYER::RENDERDOC_CAPTURE ) );
				}
			}
			if ( !Check::vkLayersSupport ( vk_layers ) )
			{
				std::cerr << "### Create::vkInstance failed! Vulkan layers requested not supported!" << std::endl;
			}

			// get and check instance extensions support
			std::vector<const char*> instance_extensions = Get::InstanceExtensions ( enable_validation );

			if ( !Check::InstanceExtensionsSupport ( enable_validation ) )
			{
				std::cerr << "### Create::vkInstance failed! Extensions requested not supported!" << std::endl;
			}

			// app info
			VkApplicationInfo app_info {};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pApplicationName = name;
			app_info.applicationVersion = VK_MAKE_VERSION ( 1 , 0 , 0 );
			app_info.pEngineName = "Engine";
			app_info.engineVersion = VK_MAKE_VERSION ( 1 , 0 , 0 );
			app_info.apiVersion = VK_API_VERSION_1_0;

			// create info for app info
			VkInstanceCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			create_info.pApplicationInfo = &app_info;

			// validation layer settings
			create_info.enabledLayerCount = static_cast< uint32_t >( vk_layers.size () );
			create_info.ppEnabledLayerNames = vk_layers.size () > 0 ? vk_layers.data () : nullptr;

			create_info.enabledExtensionCount = static_cast< uint32_t >( instance_extensions.size () );
			create_info.ppEnabledExtensionNames = instance_extensions.size () > 0 ? instance_extensions.data () : nullptr;

			create_info.pNext = nullptr;

			if ( enable_validation )
			{
				// set debug info
				VkDebugUtilsMessengerCreateInfoEXT debug_create_info {};
				Debug::PopulateDebugMessengerCreateInfo ( debug_create_info );
				create_info.pNext = ( VkDebugUtilsMessengerCreateInfoEXT* ) &debug_create_info;
			}

			if ( vkCreateInstance ( &create_info , nullptr , &instance ) != VK_SUCCESS )
			{
				std::cerr << "### Create::vkInstance failed to create VkInstance!" << std::endl;
				return false;
			}

			return true;
		}

		bool vkDebugMessenger ( VkInstance instance , VkDebugUtilsMessengerEXT& debugMessenger )
		{
			VkDebugUtilsMessengerCreateInfoEXT debug_create_info {};
			Debug::PopulateDebugMessengerCreateInfo ( debug_create_info );

			if ( Debug::CreateDebugUtilsMessengerEXT ( instance , &debug_create_info , nullptr , &debugMessenger ) != VK_SUCCESS )
			{
				std::cerr << "### Create::vkDebugMessenger Failed to set up debug messenger!" << std::endl;
				return false;
			}

			return true;
		}

		bool vkSurfaceWin32 ( VkInstance instance , HWND hWnd , VkSurfaceKHR& surface )
		{
			// get surface creation extension
			auto vkCreateWin32Surface = ( PFN_vkCreateWin32SurfaceKHR ) vkGetInstanceProcAddr ( instance , "vkCreateWin32SurfaceKHR" );
			if ( vkCreateWin32Surface == nullptr )
			{
				std::cerr << "### vkHelper::Create::vkSurfaceWin32 failed! Surface creation extension not loaded!" << std::endl;
				return false;
			}

			// surface create info
			VkWin32SurfaceCreateInfoKHR surface_create_info;
			surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surface_create_info.pNext = nullptr;
			surface_create_info.flags = 0;
			surface_create_info.hinstance = GetModuleHandle ( NULL );
			surface_create_info.hwnd = hWnd;

			// create the surface
			if ( vkCreateWin32Surface != nullptr )
			{
				if ( auto error = vkCreateWin32Surface ( instance , &surface_create_info , nullptr , &surface ) )
				{
					std::cerr << "### vkHelper::Create::vkSurfaceWin32 failed! Failed to create win32 surface" << std::endl;
					return false;
				}
			}

			return true;
		}

		VkPhysicalDevice vkPhysicalDevice ( VkInstance instance , VkSurfaceKHR surface )
		{
			// pick a physical device
			uint32_t device_count { 0 };
			vkEnumeratePhysicalDevices ( instance , &device_count , nullptr );
			if ( device_count == 0 )
			{
				std::cerr << "### vkHelper::Create::vkPhysicalDevice failed! Failed to find GPU with vulkan support." << std::endl;
				return VK_NULL_HANDLE;
			}
			std::vector<VkPhysicalDevice> devices ( device_count );
			vkEnumeratePhysicalDevices ( instance , &device_count , devices.data () );

			// print all devices
			std::cout << "### All physical devices:" << std::endl;
			for ( auto const& physical_device : devices )
			{
				VkPhysicalDeviceProperties device_properties;
				vkGetPhysicalDeviceProperties ( physical_device , &device_properties );
				std::cout << "\t- " << device_properties.deviceName << std::endl;
			}

			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
			for ( auto const& physical_device : devices )
			{
				if ( Check::PhysicalDeviceSuitable ( physical_device , surface ) )
				{
					// device found
					std::cout << "### Suitable Device Found:" << std::endl;
					physicalDevice = physical_device;
					VkPhysicalDeviceProperties device_properties;
					vkGetPhysicalDeviceProperties ( physicalDevice , &device_properties );
					std::cout << "\t- " << device_properties.deviceName << std::endl;
					break;
				}
			}

			if ( physicalDevice == VK_NULL_HANDLE )
			{
				std::cout << "\t- " << "none" << std::endl;
				std::cerr << "### vkHelper::Create::vkPhysicalDevice failed! Failed to find a suitable GPU for selected operations." << std::endl;
				return VK_NULL_HANDLE;
			}

			return physicalDevice;
		}

		VkDevice vkLogicalDevice ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , int flags )
		{
			Get::QueueFamilyIndices indices = Get::QueueFamilies ( physicalDevice , surface );

			// create set of queue families to guarantee unique key
			std::set<uint32_t> unique_queue_families = { indices.graphics_family_.value (), indices.present_family_.value () };

			float queue_priority { 1.0f };

			// iterate over families and create queue info
			std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
			for ( auto const& queue_family : unique_queue_families )
			{
				VkDeviceQueueCreateInfo queue_create_info {};
				queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queue_create_info.queueFamilyIndex = queue_family;
				queue_create_info.queueCount = 1;
				queue_create_info.pQueuePriorities = &queue_priority;

				queue_create_infos.push_back ( queue_create_info );
			}

			// device features for logical device
			VkPhysicalDeviceFeatures device_features {};

			// create logical device
			bool enable_validation = flags & static_cast< int >( Get::VKLAYER::KHRONOS_VALIDATION );
			bool enable_renderdoc = flags & static_cast< int >( Get::VKLAYER::RENDERDOC_CAPTURE );

			// get device extensions
			std::vector<const char*> device_extensions = Get::DeviceExtensions ();

			// get validation layers
			std::vector<const char*> vk_layers;
			if ( enable_validation )
			{
				vk_layers.emplace_back ( Get::vkLayer ( Get::VKLAYER::KHRONOS_VALIDATION ) );

				if ( enable_renderdoc )
				{
					vk_layers.emplace_back ( Get::vkLayer ( Get::VKLAYER::RENDERDOC_CAPTURE ) );
				}
			}

			VkDeviceCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			create_info.pQueueCreateInfos = queue_create_infos.data ();
			create_info.queueCreateInfoCount = static_cast< uint32_t >( queue_create_infos.size () );
			create_info.pEnabledFeatures = &device_features;
			create_info.enabledExtensionCount = static_cast< uint32_t >( device_extensions.size () );
			create_info.ppEnabledExtensionNames = device_extensions.data ();

			if ( enable_validation )
			{
				create_info.enabledLayerCount = static_cast< uint32_t >( vk_layers.size () );
				create_info.ppEnabledLayerNames = vk_layers.data ();
			}
			else
			{
				create_info.enabledLayerCount = 0;
			}

			VkDevice logical_device { VK_NULL_HANDLE };
			if ( vkCreateDevice ( physicalDevice , &create_info , nullptr , &logical_device ) != VK_SUCCESS )
			{
				std::cerr << "### vkHelper::Create::vkLogicalDevice failed! Failed to create a logical device." << std::endl;
				return VK_NULL_HANDLE;
			}

			return logical_device;
		}

		VkQueue vkGraphicsQueue ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , VkDevice logicalDevice )
		{
			assert ( physicalDevice != VK_NULL_HANDLE &&
				surface != VK_NULL_HANDLE &&
				logicalDevice != VK_NULL_HANDLE );

			Get::QueueFamilyIndices indices = Get::QueueFamilies ( physicalDevice , surface );
			VkQueue graphics_queue;
			vkGetDeviceQueue ( logicalDevice , indices.graphics_family_.value () , 0 , &graphics_queue );
			return graphics_queue;
		}

		VkQueue vkPresentQueue ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , VkDevice logicalDevice )
		{
			assert ( physicalDevice != VK_NULL_HANDLE &&
				surface != VK_NULL_HANDLE &&
				logicalDevice != VK_NULL_HANDLE );

			Get::QueueFamilyIndices indices = Get::QueueFamilies ( physicalDevice , surface );
			VkQueue present_queue;
			vkGetDeviceQueue ( logicalDevice , indices.present_family_.value () , 0 , &present_queue );
			return present_queue;
		}

		vkSwapChainData vkSwapChain ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , VkDevice logicalDevice )
		{
			vkSwapChainData swapchain_data;

			Get::SwapChainSupportDetails swapchain_support = Get::SwapChainSupportDetails_f ( physicalDevice , surface );

			// get swap chain formats
			VkSurfaceFormatKHR surface_format = Get::vkSwapChainSurfaceFormat ( physicalDevice , surface );
			swapchain_data.format_ = surface_format.format;

			// get swap chain present modes
			VkPresentModeKHR present_mode = Get::vkSwapChainPresentMode ( physicalDevice , surface );

			// get swap chain extent from capabilities
			swapchain_data.extent_ = Get::vkSwapChainExtent2D ( physicalDevice , surface );

			uint32_t image_count = swapchain_support.capabilities_.minImageCount + 1;

			if ( swapchain_support.capabilities_.maxImageCount > 0 && image_count > swapchain_support.capabilities_.maxImageCount )
			{
				image_count = swapchain_support.capabilities_.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo {};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface;
			createInfo.minImageCount = image_count;
			createInfo.imageFormat = surface_format.format;
			createInfo.imageColorSpace = surface_format.colorSpace;
			createInfo.imageExtent = swapchain_data.extent_;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			createInfo.presentMode = present_mode;
			// transform of the image in the swap chain, e.g. rotation
			createInfo.preTransform = swapchain_support.capabilities_.currentTransform;
			// how the image blends with other windows
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			// if true, pixels blocked by other windows are clipped
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = VK_NULL_HANDLE;

			// queue handling
			Get::QueueFamilyIndices indices = Get::QueueFamilies ( physicalDevice , surface );
			uint32_t queueFamilyIndices[] = { indices.graphics_family_.value (), indices.present_family_.value () };
			if ( indices.graphics_family_ != indices.present_family_ )
			{
				// any queue can access the image even from a different queue
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else
			{
				// only the owning queue can access the swap chain image, more efficient
				// most hardware have the same graphics and presentation queue family,
				// so exclusive if the most used case
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				createInfo.queueFamilyIndexCount = 1;
				createInfo.pQueueFamilyIndices = nullptr;
			}

			if ( vkCreateSwapchainKHR ( logicalDevice , &createInfo , nullptr , &swapchain_data.swapchain_ ) != VK_SUCCESS )
			{
				std::cerr << "### vkHelper::Create::vkSwapChain failed! Failed to create swap chain." << std::endl;
			}

			swapchain_data.images_ = Get::vkSwapChainImages ( logicalDevice , swapchain_data.swapchain_ );
			swapchain_data.image_views_ = Get::vkSwapChainImageViews ( logicalDevice , swapchain_data.images_ , swapchain_data.format_ );

			return swapchain_data;
		}

		VkRenderPass vkRenderPass ( VkDevice logicalDevice , VkFormat imageFormat )
		{
			// single color buffer attachment from one of the images from the swap chain
			VkAttachmentDescription colorAttachment {};
			colorAttachment.format = imageFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			// subpasses and attachment references, for postprocessing
			VkAttachmentReference colorAttachmentRef {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			VkSubpassDependency dependency {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			// create render pass
			VkRenderPassCreateInfo renderPassInfo {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			VkRenderPass render_pass { VK_NULL_HANDLE };
			if ( vkCreateRenderPass ( logicalDevice , &renderPassInfo , nullptr , &render_pass ) != VK_SUCCESS )
			{
				std::cerr << "### vkHelper::Create::vkRenderPass failed! Failed to create render pass." << std::endl;
				return VK_NULL_HANDLE;
			}

			return render_pass;
		}

		vkPipelineData vkGraphicsPipeline ( VkDevice logicalDevice , vkSwapChainData swapChainData , VkRenderPass renderPass )
		{
			vkPipelineData pipeline_data;

			auto vertShaderCode = IO::ReadFile ( "shaders/vert.spv" );
			auto fragShaderCode = IO::ReadFile ( "shaders/frag.spv" );

			std::cout << "size of vert read : " << vertShaderCode.size () << std::endl;
			std::cout << "size of frag read : " << fragShaderCode.size () << std::endl;

			VkShaderModule vertShaderModule = IO::CreateShaderModule ( logicalDevice , vertShaderCode );
			VkShaderModule fragShaderModule = IO::CreateShaderModule ( logicalDevice , fragShaderCode );

			// vertex shader stage creation
			VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = vertShaderModule;
			vertShaderStageInfo.pName = "main";
			vertShaderStageInfo.pSpecializationInfo = nullptr;  // used to optimize constant variables

			// fragment shader stage creation
			VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";
			fragShaderStageInfo.pSpecializationInfo = nullptr;  // used to optimize constant variables

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

			// fixed function pipeline setup - vertex input, no vertex data for now
			VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 0;
			vertexInputInfo.pVertexBindingDescriptions = nullptr;
			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.pVertexAttributeDescriptions = nullptr;

			// fixed function pipeline setup - input assembly
			VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			// viewport
			VkViewport viewport {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = ( float ) swapChainData.extent_.width;
			viewport.height = ( float ) swapChainData.extent_.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			// scizzor rectangle
			VkRect2D scissor {};
			scissor.offset = { 0,0 };
			scissor.extent = swapChainData.extent_;

			// combine viewport and scizzor rectangle into a viewport state
			VkPipelineViewportStateCreateInfo viewportState {};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			// rasterizer
			VkPipelineRasterizationStateCreateInfo rasterizer {};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;
			rasterizer.depthBiasConstantFactor = 0.0f;
			rasterizer.depthBiasClamp = 0.0f;
			rasterizer.depthBiasSlopeFactor = 0.0f;

			// multisampling
			VkPipelineMultisampleStateCreateInfo multisampling {};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.0f;
			multisampling.pSampleMask = nullptr;
			multisampling.alphaToCoverageEnable = VK_FALSE;
			multisampling.alphaToOneEnable = VK_FALSE;

			// color blending
			VkPipelineColorBlendAttachmentState colorBlendAttachment {};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

			// non
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

			// alpha blend
			/*colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;*/

			// color blend state
			VkPipelineColorBlendStateCreateInfo colorBlending {};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			colorBlending.blendConstants[ 0 ] = 0.0f;
			colorBlending.blendConstants[ 1 ] = 0.0f;
			colorBlending.blendConstants[ 2 ] = 0.0f;
			colorBlending.blendConstants[ 3 ] = 0.0f;

			// setting dynamic states of the pipeline to modify it without recreating entire pipeline
			VkDynamicState dynamicStates[] = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_LINE_WIDTH
			};

			VkPipelineDynamicStateCreateInfo dynamicState {};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = 2;
			dynamicState.pDynamicStates = dynamicStates;

			// uniform variables in shaders, pipeline layout

			VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 0;
			pipelineLayoutInfo.pSetLayouts = nullptr;
			pipelineLayoutInfo.pushConstantRangeCount = 0;
			pipelineLayoutInfo.pPushConstantRanges = nullptr;

			if ( vkCreatePipelineLayout ( logicalDevice , &pipelineLayoutInfo , nullptr , &pipeline_data.layout_ ) != VK_SUCCESS )
			{
				std::cerr << "vkHelper::Create::vkGraphicsPipeline failed! Failed to create pipeline layout." << std::endl;
			}

			// creating pipeline
			VkGraphicsPipelineCreateInfo pipelineInfo {};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

			// shader stages
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;

			// fixed function
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = nullptr; // Optional
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = nullptr;

			// pipeline layout
			pipelineInfo.layout = pipeline_data.layout_;

			// render and sub pass
			pipelineInfo.renderPass = renderPass;
			pipelineInfo.subpass = 0;

			// base pipeline to aid efficient creation of pipeline based on existing
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineInfo.basePipelineIndex = -1;

			if ( vkCreateGraphicsPipelines ( logicalDevice , VK_NULL_HANDLE , 1 , &pipelineInfo , nullptr , &pipeline_data.pipeline_ ) != VK_SUCCESS )
			{
				std::cerr << "vkHelper::Create::vkGraphicsPipeline failed! Failed to create graphics pipeline." << std::endl;
			}

			// clean up local shader modules after compiling and linking
			vkDestroyShaderModule ( logicalDevice , fragShaderModule , nullptr );
			vkDestroyShaderModule ( logicalDevice , vertShaderModule , nullptr );

			return pipeline_data;
		}

		bool vkFramebuffers ( VkDevice logicalDevice , vkSwapChainData& swapChainData , VkRenderPass renderPass , std::vector<VkFramebuffer>& framebuffers )
		{
			framebuffers.resize ( swapChainData.image_views_.size () );

			// iterate image views and create framebuffers from them
			for ( size_t i = 0; i < swapChainData.image_views_.size (); ++i )
			{
				VkImageView attachments[] = {
					swapChainData.image_views_[ i ]
				};

				VkFramebufferCreateInfo framebufferInfo {};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderPass;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = swapChainData.extent_.width;
				framebufferInfo.height = swapChainData.extent_.height;
				framebufferInfo.layers = 1;

				if ( vkCreateFramebuffer ( logicalDevice , &framebufferInfo , nullptr , &framebuffers[ i ] ) != VK_SUCCESS )
				{
					std::cerr << "vkHelper::Create::vkFramebuffers failed! Failed to create framebuffer " << i << "." << std::endl;
					return false;
				}
			}
			return true;
		}

		VkCommandPool vkCommandPool ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , VkDevice logicalDevice )
		{
			Get::QueueFamilyIndices queueFamilyIndices = Get::QueueFamilies ( physicalDevice , surface );

			VkCommandPoolCreateInfo poolInfo {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = queueFamilyIndices.graphics_family_.value ();
			poolInfo.flags = 0;

			VkCommandPool command_pool;
			if ( vkCreateCommandPool ( logicalDevice , &poolInfo , nullptr , &command_pool ) != VK_SUCCESS )
			{
				std::cerr << "vkHelper::Create::vkCommandPool failed! Failed to create command pool." << std::endl;
				return VK_NULL_HANDLE;
			}
			return command_pool;
		}

		bool vkCommandBuffers ( VkDevice logicalDevice , vkSwapChainData swapChain , VkRenderPass renderPass , vkPipelineData graphicsPipeline , std::vector<VkFramebuffer>& framebuffers , VkCommandPool commandPool , std::vector<VkCommandBuffer>& commandBuffers )
		{
			commandBuffers.resize ( framebuffers.size () );

			VkCommandBufferAllocateInfo allocInfo {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = ( uint32_t ) commandBuffers.size ();

			if ( vkAllocateCommandBuffers ( logicalDevice , &allocInfo , commandBuffers.data () ) != VK_SUCCESS )
			{
				std::cerr << "vkHelper::Create::vkCommandBuffers failed! Failed to allocate command buffers." << std::endl;
				return false;
			}

			for ( size_t i = 0; i < commandBuffers.size (); ++i )
			{
				// begin command buffer
				VkCommandBufferBeginInfo beginInfo {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = 0;
				beginInfo.pInheritanceInfo = nullptr;

				if ( vkBeginCommandBuffer ( commandBuffers[ i ] , &beginInfo ) != VK_SUCCESS )
				{
					std::cerr << "vkHelper::Create::vkCommandBuffers failed! Failed to begin command buffer." << std::endl;
					return false;
				}

				// assign render pass to command buffer and begin render pass
				VkRenderPassBeginInfo renderPassInfo {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = renderPass;
				renderPassInfo.framebuffer = framebuffers[ i ];
				renderPassInfo.renderArea.offset = { 0,0 };
				renderPassInfo.renderArea.extent = swapChain.extent_;

				VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearColor;

				vkCmdBeginRenderPass ( commandBuffers[ i ] , &renderPassInfo , VK_SUBPASS_CONTENTS_INLINE );

				// bind graphics pipeline
				vkCmdBindPipeline ( commandBuffers[ i ] , VK_PIPELINE_BIND_POINT_GRAPHICS , graphicsPipeline.pipeline_ );

				// bind draw command
				// param
				// 1. command buffer
				// 2. vertex count
				// 3. first vertex
				// 4. first instance
				vkCmdDraw ( commandBuffers[ i ] , 3 , 1 , 0 , 0 );

				// end render pass
				vkCmdEndRenderPass ( commandBuffers[ i ] );

				// end command buffer
				if ( vkEndCommandBuffer ( commandBuffers[ i ] ) != VK_SUCCESS )
				{
					std::cerr << "vkHelper::Create::vkEndCommandBuffer failed! Failed to end command buffer." << std::endl;
					return false;
				}
			}
			return true;
		}

		bool SyncObjects ( VkDevice logicalDevice , vkSwapChainData swapChain , vkSyncObjects& syncObjects )
		{
			syncObjects.available_semaphores_.resize ( MAX_FRAMES_IN_FLIGHT );
			syncObjects.finished_semaphores_.resize ( MAX_FRAMES_IN_FLIGHT );
			syncObjects.in_flight_fences_.resize ( MAX_FRAMES_IN_FLIGHT );
			syncObjects.images_in_flight_.resize ( swapChain.images_.size () , VK_NULL_HANDLE );

			VkSemaphoreCreateInfo semaphoreInfo {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
			{
				if ( vkCreateSemaphore ( logicalDevice , &semaphoreInfo , nullptr , &syncObjects.available_semaphores_[ i ] ) != VK_SUCCESS ||
					vkCreateSemaphore ( logicalDevice , &semaphoreInfo , nullptr , &syncObjects.finished_semaphores_[ i ] ) != VK_SUCCESS ||
					vkCreateFence ( logicalDevice , &fenceInfo , nullptr , &syncObjects.in_flight_fences_[ i ] ) != VK_SUCCESS )
				{
					std::cerr << "vkHelper::Create::SyncObjects failed! Failed to create semaphore for a frame." << std::endl;
					return false;
				}
			}
			return true;
		}
	}

	namespace Check
	{
		bool vkLayersSupport ( std::vector<char const*> requestedLayers )
		{
			std::cout << "### Requested Layers:" << std::endl;
			for ( const auto& layer : requestedLayers )
			{
				std::cout << "\t- " << layer << std::endl;
			}

			// get available layers
			uint32_t layer_count;
			vkEnumerateInstanceLayerProperties ( &layer_count , nullptr );
			std::vector<VkLayerProperties> available_layers ( layer_count );
			vkEnumerateInstanceLayerProperties ( &layer_count , available_layers.data () );

			std::cout << "### Available Layers:" << std::endl;;
			for ( const auto& layer : available_layers )
			{
				std::cout << "\t- " << layer.layerName << std::endl;
			}

			for ( const auto& requested_layer : requestedLayers )
			{
				bool layer_found = false;
				for ( const auto& layerProperty : available_layers )
				{
					if ( !strcmp ( requested_layer , layerProperty.layerName ) )
					{
						layer_found = true;
					}
				}
				if ( !layer_found )
				{
					std::cerr << "### vkHelper::Check::vkLayersSupported failed! Requested layer not supported." << std::endl;
					std::cerr << "###\t- " << requested_layer << std::endl;
					return false;
				}
			}

			std::cout << "### All requested layers supported!" << std::endl;
			return true;
		}

		bool CompareExtensionsList ( std::vector<char const*> requestedExtensions , std::vector<VkExtensionProperties> availableExtensions , char const* name )
		{
			std::set<std::string> required_extensions ( requestedExtensions.begin () , requestedExtensions.end () );

			// print out requested extensions
			std::cout << "### Requested " << name << " extensions:" << std::endl;
			for ( const auto& extension : required_extensions )
			{
				std::cout << "\t- " << extension << std::endl;
			}

			// print out available extensions
			std::cout << "### Available " << name << " instance extensions:" << std::endl;
			for ( const auto& extension : availableExtensions )
			{
				std::cout << "\t- " << extension.extensionName << std::endl;
			}

			// check extensions
			for ( auto const& extension : availableExtensions )
			{
				required_extensions.erase ( extension.extensionName );
			}
			if ( !required_extensions.empty () )
			{
				std::cerr << "### vkHelper::CheckCompareExtensionsList Failed! Requested " << name << " extensions not supported." << std::endl;
				for ( auto const& extension : required_extensions )
				{
					std::cerr << "\t- " << extension << std::endl;
				}
				return false;
			}
			std::cout << "### All requested " << name << " extensions supported." << std::endl;
			return true;
		}

		bool InstanceExtensionsSupport ( bool debug )
		{
			// get available extensions
			uint32_t extension_count { 0 };
			//  - get number of supported extensions
			vkEnumerateInstanceExtensionProperties ( nullptr , &extension_count , nullptr );
			//  - allocate container for storing the extensions
			std::vector<VkExtensionProperties> available_extensions ( extension_count );
			//  - query supported extensions details
			vkEnumerateInstanceExtensionProperties ( nullptr , &extension_count , available_extensions.data () );

			return CompareExtensionsList ( Get::InstanceExtensions ( debug ) , available_extensions , "instance" );
		}

		bool DeviceExtensionsSupport ( VkPhysicalDevice physicalDevice )
		{
			VkPhysicalDeviceProperties device_properties;
			vkGetPhysicalDeviceProperties ( physicalDevice , &device_properties );
			std::cout << "### Checking device extensions of: " << device_properties.deviceName << std::endl;

			uint32_t extension_count;
			vkEnumerateDeviceExtensionProperties ( physicalDevice , nullptr , &extension_count , nullptr );
			std::vector<VkExtensionProperties> available_extensions ( extension_count );
			vkEnumerateDeviceExtensionProperties ( physicalDevice , nullptr , &extension_count , available_extensions.data () );

			return CompareExtensionsList ( Get::DeviceExtensions () , available_extensions , "device" );
		}

		bool SwapChainSupport ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface )
		{
			Get::SwapChainSupportDetails details = Get::SwapChainSupportDetails_f ( physicalDevice , surface );
			return !details.formats_.empty () && !details.present_modes_.empty ();
		}

		bool PhysicalDeviceSuitable ( VkPhysicalDevice device , VkSurfaceKHR surface )
		{
			return Get::QueueFamilies ( device , surface ).IsComplete () &&
				Check::DeviceExtensionsSupport ( device ) &&
				Check::SwapChainSupport ( device , surface );
		}
	}

	namespace Get
	{
		char const* vkLayer ( VKLAYER layer )
		{
			assert ( vk_layers_.find ( layer ) != vk_layers_.end () );
			return vk_layers_[ layer ];
		}

		std::vector<const char*> ValidationLayers ( int flags )
		{
			std::vector<const char*> validation_layers;
			if ( flags & static_cast< int >( VKLAYER::KHRONOS_VALIDATION ) )
			{
				validation_layers.emplace_back ( Get::vkLayer ( Get::VKLAYER::KHRONOS_VALIDATION ) );
			}
			if ( flags & static_cast< int >( VKLAYER::RENDERDOC_CAPTURE ) )
			{
				validation_layers.emplace_back ( Get::vkLayer ( Get::VKLAYER::RENDERDOC_CAPTURE ) );
			}
			return validation_layers;
		}

		std::vector<char const*> InstanceExtensions ( bool debug )
		{
			std::vector<char const*> instance_extensions {
				"VK_KHR_surface",
				"VK_KHR_win32_surface"
			};

			if ( debug )
			{
				instance_extensions.emplace_back ( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
			}
			return instance_extensions;
		}

		std::vector<char const*> DeviceExtensions ()
		{
			return {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
		}

		QueueFamilyIndices QueueFamilies ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface )
		{
			QueueFamilyIndices indices;

			// get all device queue families
			uint32_t qfp_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties ( physicalDevice , &qfp_count , nullptr );
			std::vector<VkQueueFamilyProperties> queue_families_properties ( qfp_count );
			vkGetPhysicalDeviceQueueFamilyProperties ( physicalDevice , &qfp_count , queue_families_properties.data () );

			// store them in self made queue family struct, i.e. QueueFamilyIndices
			// graphics and present family share the same index
			int i = 0;
			for ( const auto& qfp : queue_families_properties )
			{
				// look for graphics bit
				if ( qfp.queueFlags & VK_QUEUE_GRAPHICS_BIT )
				{
					indices.graphics_family_ = i;
				}

				// look for present support
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR ( physicalDevice , i , surface , &presentSupport );
				if ( presentSupport )
				{
					indices.present_family_ = i;
				}

				// if all families filled, early exit from queue
				if ( indices.IsComplete () )
				{
					break;
				}
				++i;
			}

			return indices;
		}

		SwapChainSupportDetails SwapChainSupportDetails_f ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface )
		{
			SwapChainSupportDetails details;

			// check surface capabilities
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( physicalDevice , surface , &details.capabilities_ );

			// check surface formats
			uint32_t format_count;
			vkGetPhysicalDeviceSurfaceFormatsKHR ( physicalDevice , surface , &format_count , nullptr );
			if ( format_count != 0 )
			{
				details.formats_.resize ( format_count );
				vkGetPhysicalDeviceSurfaceFormatsKHR ( physicalDevice , surface , &format_count , details.formats_.data () );
			}

			// check surface present modes
			uint32_t present_modes_count;
			vkGetPhysicalDeviceSurfacePresentModesKHR ( physicalDevice , surface , &present_modes_count , nullptr );
			if ( present_modes_count != 0 )
			{
				details.present_modes_.resize ( present_modes_count );
				vkGetPhysicalDeviceSurfacePresentModesKHR ( physicalDevice , surface , &present_modes_count , details.present_modes_.data () );
			}

			return details;
		}

		VkSurfaceFormatKHR vkSwapChainSurfaceFormat ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface )
		{
			std::vector<VkSurfaceFormatKHR> available_formats = SwapChainSupportDetails_f ( physicalDevice , surface ).formats_;
			// if format specified found 
			for ( auto const& available_format : available_formats )
			{
				if ( available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
				{
					return available_format;
				}
			}
			// else return first one
			return available_formats[ 0 ];
		}

		VkPresentModeKHR vkSwapChainPresentMode ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface )
		{
			std::vector<VkPresentModeKHR> available_present_modes = SwapChainSupportDetails_f ( physicalDevice , surface ).present_modes_;
			// if present mode specified found 
			for ( auto const& available_present_mode : available_present_modes )
			{
				if ( available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR )
				{
					return available_present_mode;
				}
			}
			// else return first in first out
			return VK_PRESENT_MODE_FIFO_KHR;
		}

		float aspect_ratio { 0.5625f };
		VkExtent2D vkSwapChainExtent2D ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface )
		{
			VkSurfaceCapabilitiesKHR capabilities = SwapChainSupportDetails_f ( physicalDevice , surface ).capabilities_;
			if ( capabilities.currentExtent.width != UINT32_MAX )
			{
				return capabilities.currentExtent;
			}
			else
			{
				VkExtent2D actual_extent = { static_cast< uint32_t >( wndHelper::g_width ), static_cast< uint32_t >( wndHelper::g_height ) };

				actual_extent.width = std::clamp ( actual_extent.width , capabilities.minImageExtent.width , capabilities.maxImageExtent.width );
				actual_extent.height = std::clamp ( actual_extent.height , capabilities.minImageExtent.height , capabilities.maxImageExtent.height );

				return actual_extent;
			}
		}

		std::vector<VkImage> vkSwapChainImages ( VkDevice logicalDevice , VkSwapchainKHR swapChain )
		{
			std::vector<VkImage> images;
			uint32_t image_count { 0 };
			vkGetSwapchainImagesKHR ( logicalDevice , swapChain , &image_count , nullptr );
			images.resize ( image_count );
			vkGetSwapchainImagesKHR ( logicalDevice , swapChain , &image_count , images.data () );
			return images;
		}

		std::vector<VkImageView> vkSwapChainImageViews ( VkDevice logicalDevice , std::vector<VkImage> const& swapChainImages , VkFormat swapChainImageFormat )
		{
			std::vector<VkImageView> image_views;
			image_views.resize ( swapChainImages.size () );

			for ( size_t i = 0; i < swapChainImages.size (); ++i )
			{
				VkImageViewCreateInfo createInfo {};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = swapChainImages[ i ];
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = swapChainImageFormat;
				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				if ( vkCreateImageView ( logicalDevice , &createInfo , nullptr , &image_views[ i ] ) != VK_SUCCESS )
				{
					std::cerr << "### vkHelper::Get::vkSwapChainImageViews failed! Failed to create image view." << std::endl;
				}
			}

			return image_views;
		}
	}

	namespace Debug
	{
#define JZVK_ALL_LAYER_MESSAGES
		VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback ( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity , VkDebugUtilsMessageTypeFlagsEXT messageType , const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData , void* pUserData )
		{
			if ( messageSeverity & VK_DEBUG_REPORT_ERROR_BIT_EXT )
			{
				std::cerr << "[ERROR]\n"
					<< "\t[CODE: " << pCallbackData->messageIdNumber << "]\n"
					<< "\t[MESSAGE: " << pCallbackData->pMessage << "]" << std::endl;
			}
			else if ( messageSeverity & VK_DEBUG_REPORT_WARNING_BIT_EXT )
			{
				std::cerr << "[WARNING]\n"
					<< "\t[CODE: " << pCallbackData->messageIdNumber << "]\n"
					<< "\t[MESSAGE: " << pCallbackData->pMessage << "]" << std::endl;
			}
			else
			{
#ifdef JZVK_ALL_LAYER_MESSAGES
				std::cerr << "[INFO]\n"
					<< "\t[CODE: " << pCallbackData->messageIdNumber << "]\n"
					<< "\t[MESSAGE: " << pCallbackData->pMessage << "]" << std::endl;
#endif
			}
			// should always return false, i.e. not abort function call that triggered this callback
			return VK_FALSE;
		}

		void PopulateDebugMessengerCreateInfo ( VkDebugUtilsMessengerCreateInfoEXT& createInfo )
		{
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			// flag possible problems
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

			// debug call back function defined above 
			createInfo.pfnUserCallback = DebugCallback;

			createInfo.pUserData = nullptr;
		}

		VkResult CreateDebugUtilsMessengerEXT ( VkInstance instance , const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo , const VkAllocationCallbacks* pAllocator , VkDebugUtilsMessengerEXT* pDebugMessenger )
		{
			auto func = ( PFN_vkCreateDebugUtilsMessengerEXT ) vkGetInstanceProcAddr ( instance , "vkCreateDebugUtilsMessengerEXT" );
			if ( func != nullptr )
			{
				return func ( instance , pCreateInfo , pAllocator , pDebugMessenger );
			}
			else
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		void DestroyDebugUtilsMessengerEXT ( VkInstance instance , VkDebugUtilsMessengerEXT debugMessenger , const VkAllocationCallbacks* pAllocator )
		{
			auto func = ( PFN_vkDestroyDebugUtilsMessengerEXT ) vkGetInstanceProcAddr ( instance , "vkDestroyDebugUtilsMessengerEXT" );
			if ( func != nullptr )
			{
				func ( instance , debugMessenger , pAllocator );
			}
			else
			{
				std::cerr << "### vkHelper::Debug::DestroyUtilsMessengerEXT failed! vkDestroyDebugUtilsMessengerEXT func not loaded!" << std::endl;
			}
		}
	}

	namespace IO
	{
		std::vector<char> ReadFile ( std::string const& filename )
		{
			std::ifstream file ( filename , std::ios::ate | std::ios::binary );

			if ( !file.is_open () )
			{
				throw std::runtime_error ( "failed to open file!" );
			}

			size_t fileSize = ( size_t ) file.tellg ();
			std::vector<char> buffer ( fileSize );

			file.seekg ( 0 );
			file.read ( buffer.data () , fileSize );

			file.close ();

			return buffer;
		}

		VkShaderModule CreateShaderModule ( VkDevice logicalDevice , std::vector<char> const& code )
		{
			VkShaderModuleCreateInfo createInfo {};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size ();
			createInfo.pCode = reinterpret_cast< uint32_t const* >( code.data () );

			VkShaderModule shaderModule;
			if ( vkCreateShaderModule ( logicalDevice , &createInfo , nullptr , &shaderModule ) != VK_SUCCESS )
			{
				throw std::runtime_error ( "failed to create shader module!" );
			}

			return shaderModule;
		}
	}

	namespace Misc
	{

		void DrawFrame ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , VkDevice logicalDevice , VkQueue graphicsQueue , VkQueue presentQueue , vkSwapChainData& swapChain , VkRenderPass& renderPass , vkPipelineData& graphicsPipeline ,
			std::vector<VkFramebuffer>& framebuffers , VkCommandPool commandPool , std::vector<VkCommandBuffer>& commandBuffers , vkSyncObjects& syncObjects , size_t& currentFrame )
		{

			// wait for frame to be finished before drawing next frame
			vkWaitForFences ( logicalDevice , 1 , &syncObjects.in_flight_fences_[ currentFrame ] , VK_TRUE , UINT64_MAX );

			uint32_t imageIndex;
			VkResult result = vkAcquireNextImageKHR ( logicalDevice , swapChain.swapchain_ , UINT64_MAX , syncObjects.available_semaphores_[ currentFrame ] , VK_NULL_HANDLE , &imageIndex );
			if ( result == VK_ERROR_OUT_OF_DATE_KHR )
			{
				Misc::RecreateSwapChain ( physicalDevice , surface , logicalDevice , swapChain , renderPass , graphicsPipeline , framebuffers , commandPool , commandBuffers );
				return;
			}
			else if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
			{
				throw std::runtime_error ( "failed to acquire swap chain image!" );
			}

			// check if the previous frame is using this image
			if ( syncObjects.images_in_flight_[ imageIndex ] != VK_NULL_HANDLE )
			{
				vkWaitForFences ( logicalDevice , 1 , &syncObjects.images_in_flight_[ imageIndex ] , VK_TRUE , UINT64_MAX );
			}

			// mark image as now being used by this frame
			syncObjects.images_in_flight_[ imageIndex ] = syncObjects.in_flight_fences_[ currentFrame ];

			// queue submission and synchronization
			VkSubmitInfo submitInfo {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphore[] = { syncObjects.available_semaphores_[ currentFrame ] };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphore;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffers[ imageIndex ];

			VkSemaphore signalSemaphores[] = { syncObjects.finished_semaphores_[ currentFrame ] };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			vkResetFences ( logicalDevice , 1 , &syncObjects.in_flight_fences_[ currentFrame ] );

			if ( vkQueueSubmit ( graphicsQueue , 1 , &submitInfo , syncObjects.in_flight_fences_[ currentFrame ] ) != VK_SUCCESS )
			{
				throw std::runtime_error ( "failed to submit draw command buffer!" );
			}

			VkPresentInfoKHR presentInfo {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapChains[] = { swapChain.swapchain_ };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &imageIndex;

			presentInfo.pResults = nullptr;

			result = vkQueuePresentKHR ( presentQueue , &presentInfo );

			if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR )
			{
				Misc::RecreateSwapChain ( physicalDevice , surface , logicalDevice , swapChain , renderPass , graphicsPipeline , framebuffers , commandPool , commandBuffers );
			}
			else if ( result != VK_SUCCESS )
			{
				throw std::runtime_error ( "failed to present swap chain image!" );
			}

			currentFrame = ( currentFrame + 1 ) % Create::MAX_FRAMES_IN_FLIGHT;
		}

		void RecreateSwapChain ( VkPhysicalDevice physicalDevice , VkSurfaceKHR surface , VkDevice logicalDevice , vkSwapChainData& swapChain , VkRenderPass& renderPass , vkPipelineData& graphicsPipeline ,
			std::vector<VkFramebuffer>& framebuffers , VkCommandPool commandPool , std::vector<VkCommandBuffer>& commandBuffers )
		{
			vkDeviceWaitIdle ( logicalDevice );

			CleanUpSwapChain ( logicalDevice , swapChain , renderPass , graphicsPipeline , framebuffers , commandPool , commandBuffers );

			swapChain = Create::vkSwapChain ( physicalDevice , surface , logicalDevice );
			renderPass = Create::vkRenderPass ( logicalDevice , swapChain.format_ );
			graphicsPipeline = Create::vkGraphicsPipeline ( logicalDevice , swapChain , renderPass );
			std::vector<VkFramebuffer> new_framebuffers;
			Create::vkFramebuffers ( logicalDevice , swapChain , renderPass , new_framebuffers );
			framebuffers = new_framebuffers;
			std::vector<VkCommandBuffer> new_commandbuffers;
			Create::vkCommandBuffers ( logicalDevice , swapChain , renderPass , graphicsPipeline , framebuffers , commandPool , new_commandbuffers );
			commandBuffers = new_commandbuffers;
		}

		void CleanUpSwapChain ( VkDevice logicalDevice , vkSwapChainData& swapChain , VkRenderPass renderPass , vkPipelineData& graphicsPipeline , std::vector<VkFramebuffer>& framebuffers , VkCommandPool commandPool , std::vector<VkCommandBuffer>& commandBuffers )
		{
			for ( size_t i = 0; i < framebuffers.size (); i++ )
			{
				vkDestroyFramebuffer ( logicalDevice , framebuffers[ i ] , nullptr );
			}

			vkFreeCommandBuffers ( logicalDevice , commandPool , static_cast< uint32_t >( commandBuffers.size () ) , commandBuffers.data () );

			vkDestroyPipeline ( logicalDevice , graphicsPipeline.pipeline_ , nullptr );
			vkDestroyPipelineLayout ( logicalDevice , graphicsPipeline.layout_ , nullptr );
			vkDestroyRenderPass ( logicalDevice , renderPass , nullptr );

			for ( size_t i = 0; i < swapChain.image_views_.size (); i++ )
			{
				vkDestroyImageView ( logicalDevice , swapChain.image_views_[ i ] , nullptr );
			}

			vkDestroySwapchainKHR ( logicalDevice , swapChain.swapchain_ , nullptr );
		}
	}
}
