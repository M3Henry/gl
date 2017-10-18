#include <iostream>
#include <vulkan/vulkan.hpp>
#include <chrono>
#include <thread>
#include <unistd.h>
#include "glfw.hpp"
#include <typeinfo>

const int WIDTH = 800;
const int HEIGHT = 600;
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
		vk::ApplicationInfo appInfo(
			"Hello, vulkan", VK_MAKE_VERSION(1, 0, 0),
			"no engine", VK_MAKE_VERSION(1, 0, 0),
			VK_API_VERSION_1_0
		);

		unsigned int glfwExtensionCount;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::cout << "Required glfwExtensions:\n";
		for (unsigned int i = 0; i < glfwExtensionCount; ++i) std::cout << "    " << glfwExtensions[i] << '\n';
		std::cout << std::endl;

		vk::InstanceCreateInfo instanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appInfo,
			0, nullptr,
			glfwExtensionCount, glfwExtensions
		);

	    auto instance = createInstance(instanceCreateInfo);

		{
			auto extension = vk::enumerateInstanceExtensionProperties();
			std::cout << "Available Vulkan extensions:\n";
			for (auto &x : extension) std::cout << "    " << x.extensionName << '\n';
			std::cout << std::endl;
		}

		auto device = [&instance](){
			auto devices = instance.enumeratePhysicalDevices();
			std::cout << "Found " << devices.size() << " devices." << std::endl;
			for (auto &d : devices)
			{
				auto properties = d.getProperties();
				auto features = d.getFeatures();
				std::cout << "    deviceType = " << to_string(properties.deviceType) << ",   geometryShader = " << (bool)features.geometryShader << std::endl;
			}
			std::cout << std::endl;
			return devices.front();
		}();

		{
			int graphicsFamily = -1;
			auto queueProperties = device.getQueueFamilyProperties();
			int i = 0;
			for (auto &q : queueProperties)
			{
				if (q.queueCount > 0 && q.queueFlags & vk::QueueFlagBits::eGraphics) graphicsFamily = i;
				if (graphicsFamily >= 0) break;
				++i;
			}
			std::cout << "graphicsFamily = " << graphicsFamily << std::endl;
		}
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
