#include <iostream>
#include <vulkan/vulkan.hpp>
#include <chrono>
#include <thread>
#include <unistd.h>
#include "glfw.hpp"

const int WIDTH = 800;
const int HEIGHT = 600;
float color = 0.f;
int main()
{
	std::cout << "Hello, world!" << std::endl;
	//if(not glfwInit()) throw "glfwInit() failed";
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

		unsigned int glfwExtensionCount = 0;

		vk::InstanceCreateInfo instanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appInfo,
			0, nullptr,
			glfwExtensionCount, glfwGetRequiredInstanceExtensions(&glfwExtensionCount)
		);

	    auto instance = createInstance(instanceCreateInfo);
	}
	catch (std::runtime_error &exception)
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
