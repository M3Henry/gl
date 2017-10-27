#pragma once
#include "glfw.hpp"
#include "utils.hpp"

class vulkan {
public:
	static auto& instance()
	{
		return singleton.instance_;
	}
private:
	auto createInfo()
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
	}
	vulkan() : instance_(createInfo())
	{}
	~vulkan()
	{}
	static vulkan singleton;
	vk::Instance instance_;
};

vulkan vulkan::singleton;
