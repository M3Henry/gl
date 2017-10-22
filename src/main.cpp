#include <iostream>
#include <chrono>
#include <thread>
#include <unistd.h>
#include "glfw.hpp"
#include <typeinfo>
#include <tuple>
#include <set>
#include <vector>
#include <limits>
#include <algorithm>
#include <fstream>
#include <cstddef>

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
		auto appInfo = vk::ApplicationInfo(
			"Hello, vulkan", VK_MAKE_VERSION(1, 0, 0),
			"no engine", VK_MAKE_VERSION(1, 0, 0),
			VK_API_VERSION_1_0
		);

		unsigned int glfwExtensionCount;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::cout << "Required glfwExtensions:\n";
		for (unsigned int i = 0; i < glfwExtensionCount; ++i) std::cout << "    " << glfwExtensions[i] << '\n';
		std::cout << std::endl;

		auto ici = vk::InstanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appInfo,
			0, nullptr,
			glfwExtensionCount, glfwExtensions
		);

	    auto instance = createInstance(ici);

		auto surface = mainWindow.createSurface(instance);

		{
			auto extension = vk::enumerateInstanceExtensionProperties();
			std::cout << "Available Vulkan extensions:\n";
			for (auto &x : extension) std::cout << "    " << x.extensionName << '\n';
			std::cout << std::endl;
		}

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
					auto extensions = d.enumerateDeviceExtensionProperties();
					bool hasSwapChains = false;
					for (auto & x : extensions)
					{
						std::cout << "        " << x.extensionName << "  v" << x.specVersion << '\n';
						hasSwapChains |= not strcmp(x.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
					}
					failed |= not hasSwapChains;
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

		auto swapChain = [&physicalDevice, &logicalDevice, &surface, &surfaceFormat, &queueFamily]()
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

			auto swapExtent = [&capabilities]() -> vk::Extent2D
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
						vk::ImageAspectFlags(),
						0,1, 	// mipmap levels
						0,1		// array layers (higher for stereographic)
					)
				);
				imageViews.push_back(logicalDevice.createImageView(ivci));
			}
			std::cout << "Created " << imageViews.size() << " image views." << std::endl;
			return imageViews;
		}();

		auto graphicsPipeline = []()
		{
			auto fileReader = [](const std::string& filename)
			{
				std::ifstream file(filename, std::ios::ate | std::ios::binary);
				if (not file) throw std::runtime_error("failed to open " + filename);
				size_t size = file.tellg();
				std::vector<std::byte> buffer(size);
				file.read(reinterpret_cast<char*>(buffer.data()), size);
				file.close();
				std::cout << filename << " loaded " << size << " bytes." << std::endl;
				return buffer;
			};
			auto vertShaderCode = fileReader("shad.spv");
			auto fragShaderCode = fileReader("frag.spv");
			return false;
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
