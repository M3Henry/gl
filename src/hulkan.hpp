#pragma once
#include "glfw.hpp"
#include "utils.hpp"

class hulkan {
public:
	static auto& instance()
	{
		return singleton.instance_;
	}
	static auto initialisePhysicalDevice(vk::SurfaceKHR & surface)
	{
		auto devices = instance().enumeratePhysicalDevices();
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
		if (devices.size())
			return devices.front();
		throw std::runtime_error("No suitable physical devices found!");
	}
private:
	auto initialiseInstance()
	{
		auto requiredExtensions = glfw::requiredVulkanExtensions();
		{
			auto available = vk::enumerateInstanceExtensionProperties();
			std::cout << "Vulkan extensions:\n";
			if (not hasRequiredExtensions(requiredExtensions, available))
				throw std::runtime_error("Missing required vulkan extension[s]");
		}
		const std::vector<char const*> requiredLayers = {/*"VK_LAYER_LUNARG_standard_validation"*/};
		{
			auto available = vk::enumerateInstanceLayerProperties();
			std::cout << "Vulkan layers:\n";
			if (not hasRequiredLayers(requiredLayers, available))
				throw std::runtime_error("Missing required vulkan layer[s]");
		}
		auto appInfo = vk::ApplicationInfo
		(
			"Hello, vulkan", VK_MAKE_VERSION(1, 0, 0),
			"Hulkan", VK_MAKE_VERSION(1, 0, 0),
			VK_API_VERSION_1_0
		);
		auto ici = vk::InstanceCreateInfo
		(
			vk::InstanceCreateFlags(),
			&appInfo,
			requiredLayers.size(), requiredLayers.data(),
			requiredExtensions.size(), requiredExtensions.data()
		);
		return vk::createInstance(ici);
	}
	hulkan() :
		instance_(initialiseInstance())
	{}
	~hulkan()
	{}
	static hulkan singleton;
	vk::Instance instance_;
};

hulkan hulkan::singleton;
