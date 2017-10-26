#include <iostream>
#include <chrono>
#include <thread>
#include <unistd.h>
#include "glfw.hpp"
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>
#include <cstddef>
#include <array>
#include "utils.hpp"


constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
float color = 0.f;
int main()
{
	std::cout << std::boolalpha << "Hello, world!\n" << std::endl;

	glfwWindowHint(GLFW_VISIBLE, false);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfw::window mainWindow(WIDTH, HEIGHT, "Hello GLFW");
	glfwSwapInterval(1);
	mainWindow.makeContextCurrent();
	mainWindow.setKeyCallback
	(
		[](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (action == GLFW_PRESS) {
				std::cout << char(key);
				std::cout.flush();
		        switch (key) {
		        case GLFW_KEY_ESCAPE:
		            glfwSetWindowShouldClose(window, true);
					return;
				case GLFW_KEY_C:
					color = color < .5f ? 1.f : 0.f;
					return;
				default:
					return;
				}
			}
			return;
		}
	);
	// vulkan
	try
	{
		auto instance = []()
		{
			auto requiredExtensions = glfw::requiredVulkanExtensions();
			{
				auto available = vk::enumerateInstanceExtensionProperties();
				std::cout << "Vulkan extensions:\n";
				if (not hasRequiredExtensions(requiredExtensions, available)) throw std::runtime_error("Missing required vulkan extension[s]");
			}
			const std::vector<char const*> requiredLayers = {"VK_LAYER_LUNARG_standard_validation"};
			{
				auto available = vk::enumerateInstanceLayerProperties();
				std::cout << "Vulkan layers:\n";
				if (not hasRequiredLayers(requiredLayers, available)) throw std::runtime_error("Missing required vulkan layer[s]");
			}
			auto appInfo = vk::ApplicationInfo
			(
				"Hello, vulkan", VK_MAKE_VERSION(1, 0, 0),
				"no engine", VK_MAKE_VERSION(1, 0, 0),
				VK_API_VERSION_1_0
			);
			auto ici = vk::InstanceCreateInfo
			(
				vk::InstanceCreateFlags(),
				&appInfo,
				requiredLayers.size(), requiredLayers.data(),
				requiredExtensions.size(), requiredExtensions.data()
			);

			return createInstance(ici);
		}();

		auto surface = mainWindow.createSurface(instance);

		auto physicalDevice = [&instance, &surface]()
		{
			auto devices = instance.enumeratePhysicalDevices();
			std::cout << "Found " << devices.size() << " physical devices." << std::endl;
			for (auto it = devices.begin(); it != devices.end();)
			{
				auto & d = *it;
				bool failed = false;
				{
					auto properties = d.getProperties();
					failed |= properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu;

					auto features = d.getFeatures();
					failed |= not features.geometryShader;

					std::cout << "    deviceType = " << to_string(properties.deviceType) << ",   geometryShader = " << (bool)features.geometryShader << '\n';
				}
				{
					std::vector<char const*> required =
					{
						VK_KHR_SWAPCHAIN_EXTENSION_NAME,
					};
					failed |= not hasRequiredExtensions(required, d.enumerateDeviceExtensionProperties() );
				}
				if (failed)
				{
					it = devices.erase(it);
					continue;
				}

				auto surfaceFormats = d.getSurfaceFormatsKHR(surface);
				failed |= surfaceFormats.empty();
				auto surfacePresentModes = d.getSurfacePresentModesKHR(surface);
				failed |= surfacePresentModes.empty();
				std::cout << "    # surfaceFormats = " << surfaceFormats.size() << "  # surfacePresentModes = " << surfacePresentModes.size() << '\n';
				if (failed)
				{
					it = devices.erase(it);
				} else
				{
					++it;
				}
			}
			std::cout << std::endl;
			if (devices.size()) return devices.front();
			throw std::runtime_error("No suitable physical devices found!");
		}();

		auto queueFamily = [&physicalDevice, &surface]()
		{
			int graphicsFamily = -1;
			int presentationFamily = -1;
			auto queueProperties = physicalDevice.getQueueFamilyProperties();
			int i = 0;
			for (auto &q : queueProperties)
			{
				if (q.queueCount > 0)
				{
					if (q.queueFlags & vk::QueueFlagBits::eGraphics) graphicsFamily = i;
					if (physicalDevice.getSurfaceSupportKHR(i, surface)) presentationFamily = i;
				}
				if (graphicsFamily >= 0 && presentationFamily >= 0) break;
				++i;
			}
			std::cout << "graphicsFamily = " << graphicsFamily << std::endl;
			std::cout << "presentationFamily = " << presentationFamily << std::endl;
			if (graphicsFamily < 0 || presentationFamily < 0) throw std::runtime_error("a queue family is missing");
			return std::make_pair<uint32_t, uint32_t>(graphicsFamily, presentationFamily);
		}();

		auto logicalDevice = [&physicalDevice, &queueFamily]()
		{
			std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueFamilies = {queueFamily.first, queueFamily.second};
			float queuePriority = 1.f;
			for (auto qFam : uniqueFamilies)
			{
				queueCreateInfos.emplace_back(
					vk::DeviceQueueCreateFlags(),
					qFam, 1, &queuePriority
				);
			}
			const std::vector<const char*> deviceExtensions = {
			    VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
			auto deviceFeatures = physicalDevice.getFeatures();

			auto dci = vk::DeviceCreateInfo
			(
				vk::DeviceCreateFlags(),
				queueCreateInfos.size(), queueCreateInfos.data(),
				0, nullptr, // validation layers
				deviceExtensions.size(), deviceExtensions.data(), // enabled extensions,
				&deviceFeatures
			);
			return physicalDevice.createDevice(dci);
		}();

		auto graphicsQueue = logicalDevice.getQueue(queueFamily.first, 0); // 0 = queue index

		auto presentationQueue = logicalDevice.getQueue(queueFamily.second, 0); // 0 = queue index

		auto surfaceFormat = [&physicalDevice, &surface]() -> vk::SurfaceFormatKHR
		{
			auto availableFormats = physicalDevice.getSurfaceFormatsKHR(surface);
			auto desiredFormat = vk::Format::eB8G8R8A8Unorm;
			auto desiredColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
			if (availableFormats.size() == 1 && availableFormats.front().format == vk::Format::eUndefined)
			{
				return {desiredFormat, desiredColorSpace};
			}
			for (const auto & f : availableFormats)
			{
				if (f.format == desiredFormat && f.colorSpace == desiredColorSpace) return f;
			}
			return availableFormats.front();
		}();
		std::cout << "SurfaceFormat = " << to_string(surfaceFormat.format) << "  Colourspace = " << to_string(surfaceFormat.colorSpace) << '\n';

		vk::Extent2D swapExtent;
		auto swapChain = [&swapExtent, &physicalDevice, &logicalDevice, &surface, &surfaceFormat, &queueFamily]()
		{
			auto presentationMode = [&physicalDevice, &surface]()
			{
				std::cout << "Available presentation modes:\n";
				auto availableModes = physicalDevice.getSurfacePresentModesKHR(surface);
				for (auto & m : availableModes)
				{
					std::cout << "    " << to_string(m) << '\n';
				}
				return vk::PresentModeKHR::eFifo;
			}();

			auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

			swapExtent = [&swapExtent, &capabilities]() -> vk::Extent2D
			{
				if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			        return capabilities.currentExtent;
			    } else {
					return {
						std::max(
							capabilities.minImageExtent.width,
							std::min(
								capabilities.maxImageExtent.width,
								WIDTH
							)
						),
						std::max(
							capabilities.minImageExtent.height,
							std::min(
								capabilities.maxImageExtent.height,
								HEIGHT
							)
						)
					};
				}
			}();

			auto swapDepth = capabilities.minImageCount;
			if (0 == swapDepth) swapDepth = 2;
			std::cout << "Selected swap extent = " << swapExtent.width << 'x' << swapExtent.height << " : " << swapDepth << '\n';

			auto exclusiveMode = queueFamily.first == queueFamily.second;
			if (not exclusiveMode) throw std::runtime_error("Concurrent mode unsupported atm.");
			std::vector<uint32_t> queueFamilyArray = {queueFamily.first, queueFamily.second};

			auto scci = vk::SwapchainCreateInfoKHR
			(
				vk::SwapchainCreateFlagsKHR(),
				surface,
				swapDepth,
				surfaceFormat.format, surfaceFormat.colorSpace,
				swapExtent,
				1, // image layers higher for stereoscopic
				vk::ImageUsageFlagBits::eColorAttachment,
				exclusiveMode ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent,
				queueFamilyArray.size(),
				queueFamilyArray.data(),
				capabilities.currentTransform,
				vk::CompositeAlphaFlagBitsKHR::eOpaque,
				presentationMode,
				true,
				vk::SwapchainKHR()
			);

			return logicalDevice.createSwapchainKHR(scci);
		}();

		auto swapImages = logicalDevice.getSwapchainImagesKHR(swapChain);
		std::cout << "Created swap chain with length " << swapImages.size() << '\n';

		auto imageViews = [&logicalDevice, &swapImages, &surfaceFormat]()
		{
			std::vector<vk::ImageView> imageViews;
			for (auto & i : swapImages)
			{
				auto ivci = vk::ImageViewCreateInfo
				(
					vk::ImageViewCreateFlags(),
					i,
					vk::ImageViewType::e2D,
					surfaceFormat.format,
					vk::ComponentMapping(),
					vk::ImageSubresourceRange
					(
						vk::ImageAspectFlagBits::eColor,
						0,1, 	// mipmap levels
						0,1		// array layers (higher for stereographic)
					)
				);
				imageViews.push_back(logicalDevice.createImageView(ivci));
			}
			std::cout << "Created " << imageViews.size() << " image views." << std::endl;
			return imageViews;
		}();

		auto pipelineLayout = [&logicalDevice]()
		{
			auto plci = vk::PipelineLayoutCreateInfo
			(
				vk::PipelineLayoutCreateFlags(),
				0, nullptr,	// layouts
				0, nullptr	// push constant ranges
			);
			return logicalDevice.createPipelineLayout(plci);
		}();

		auto renderPass = [&logicalDevice, &surfaceFormat]()
		{
			auto colourAttachment = vk::AttachmentDescription
			(
				vk::AttachmentDescriptionFlags(),
				surfaceFormat.format,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,	// render data
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,	// stencil data
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
			);
			auto colourAttachmentRef = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);

			auto subpass = vk::SubpassDescription
			(
				vk::SubpassDescriptionFlags(),
				vk::PipelineBindPoint::eGraphics,
				0, nullptr,	// input
				1, &colourAttachmentRef // color
				// etc
			);
			auto rpci = vk::RenderPassCreateInfo
			(
				vk::RenderPassCreateFlags(),
				1, &colourAttachment,
				1, &subpass,
				0, nullptr // dependencies
			);
			return logicalDevice.createRenderPass(rpci);
		}();

		auto graphicsPipeline = [&pipelineLayout, &renderPass, &logicalDevice, &swapExtent]()
		{
			auto fileReader = [&logicalDevice](const std::string& filename)
			{
				std::ifstream file(filename, std::ios::ate | std::ios::binary);
				if (not file) throw std::runtime_error("failed to open " + filename);
				std::cout << "Loading '" << filename << '\'';
				size_t size = file.tellg();
				std::vector<short> buffer(size / 2);
				file.read(buffer.data(), size);
				file.close();
				std::cout << ' ' << size << " bytes" << std::endl;
				auto smci = vk::ShaderModuleCreateInfo
				(
					vk::ShaderModuleCreateFlags(),
					size,	reinterpret_cast<const uint32_t*>(buffer.data())
				);
				return logicalDevice.createShaderModule(smci);
				//return buffer;
			};
			auto vertShaderCode = fileReader("vert.spv");
			auto fragShaderCode = fileReader("frag.spv");
			std::vector<vk::PipelineShaderStageCreateInfo> stages;
			stages.emplace_back
			(
				vk::PipelineShaderStageCreateFlags(),
				vk::ShaderStageFlagBits::eVertex,
				vertShaderCode,
				"main",
				nullptr	// specialization info (for constants)
			);
			stages.emplace_back
			(
				vk::PipelineShaderStageCreateFlags(),
				vk::ShaderStageFlagBits::eFragment,
				fragShaderCode,
				"main",
				nullptr	// specialization info (for constants)
			);
			auto pvisci = vk::PipelineVertexInputStateCreateInfo
			(
				vk::PipelineVertexInputStateCreateFlags(),
				0, nullptr,
				0, nullptr
			);
			auto piasci = vk::PipelineInputAssemblyStateCreateInfo
			(
				vk::PipelineInputAssemblyStateCreateFlags(),
				vk::PrimitiveTopology::eTriangleList,
				false
			);
			auto viewport = vk::Viewport
			(
				0.f, 0.f,	// xy centre
				swapExtent.width, swapExtent.height,
				0.f, 1.f	// z extent
			);
			auto scissor = vk::Rect2D({0, 0}, swapExtent);

			auto pvsci= vk::PipelineViewportStateCreateInfo
			(
				vk::PipelineViewportStateCreateFlags(),
				1, &viewport,
				1, &scissor
			);
			auto rasterizer = vk::PipelineRasterizationStateCreateInfo
			(
				vk::PipelineRasterizationStateCreateFlags(),
				false,	// depth clamp
				false,	// discard all
				vk::PolygonMode::eFill,
				vk::CullModeFlagBits::eBack,
				vk::FrontFace::eClockwise,
				false, 0.f, 0.f, 0.f,	// depth bias
				1.f	// line width
			);
			auto multisampler = vk::PipelineMultisampleStateCreateInfo
			(
				vk::PipelineMultisampleStateCreateFlags(),
				vk::SampleCountFlagBits::e1,
				false,	// enable
				1.f,	// min sample shading
				nullptr,// sample mask
				false,	// alpha to coverage
				false	// alpha to one
			);
			auto depthStencil = vk::PipelineDepthStencilStateCreateInfo
			();
			auto colourBlendAttachment = vk::PipelineColorBlendAttachmentState
			(
				false, // enable
				vk::BlendFactor::eOne,	// colour
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd,
				vk::BlendFactor::eOne,	// alpha
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd,
				(
					vk::ColorComponentFlagBits::eR
					| vk::ColorComponentFlagBits::eG
					| vk::ColorComponentFlagBits::eB
					| vk::ColorComponentFlagBits::eA
				)
			);
			auto colourBlending = vk::PipelineColorBlendStateCreateInfo
			(
				vk::PipelineColorBlendStateCreateFlags(),
				false,	// logic operations
				vk::LogicOp::eCopy,
				1, &colourBlendAttachment,
				{0, 0, 0, 0} // blend constants
			);
			auto dynamicState = vk::PipelineDynamicStateCreateInfo
			();
			auto gpci = vk::GraphicsPipelineCreateInfo
			(
				vk::PipelineCreateFlags(),
				2, stages.data(),
				&pvisci,
				&piasci,
				nullptr,	// assembly state
				&pvsci,
				&rasterizer,
				&multisampler,
				nullptr,	//&depthStencil,
				&colourBlending,
				nullptr,	//&dynamicState,
				pipelineLayout,
				renderPass,
				0,	// subpass
				vk::Pipeline(), -1 // base pipeline
			);
			return logicalDevice.createGraphicsPipeline(vk::PipelineCache(), gpci);
		}();

	}
	catch (std::exception &exception)
	{
		std::cerr << exception.what() << std::endl;
		return EXIT_FAILURE;
	}
	// loop time
    mainWindow.show();
	while(not mainWindow.shouldClose())
	{
		mainWindow.swapBuffers();
		::usleep(100000);
		glfwWaitEvents();
	}
	return EXIT_SUCCESS;
}
