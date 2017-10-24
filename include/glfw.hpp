#define GLFW_INCLUDE_NONE
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <string>
//#include <pair>

class glfw {
public:
	static auto requiredVulkanExtensions()
	{
				unsigned int extensionCount;
				auto glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
				std::vector<char const*> extensions;
				for (unsigned int i = 0; i < extensionCount; ++i) extensions.push_back(glfwExtensions[i]);
				return extensions;
				//return std::make_pair(extensionCount, glfwExtensions);
	}
	class window {
	public:
		window(
			int width, int height,
			const std::string& title,
			GLFWmonitor * monitor = nullptr,
			GLFWwindow * share = nullptr) :
			handle_(glfwCreateWindow(width, height, title.c_str(), monitor, share))
		{
		    if (not handle_) throw "No glfw window?!?";
		}
		~window()
		{
			glfwDestroyWindow(handle_);
		}
		void makeContextCurrent()
		{
			glfwMakeContextCurrent(handle_);
		}
		void setKeyCallback(void(*func)(GLFWwindow*, int, int, int, int))
		{
			glfwSetKeyCallback(handle_, func);
		}
		void show() const
		{
			glfwShowWindow(handle_);
		}
		bool shouldClose() const
		{
			return glfwWindowShouldClose(handle_);
		}
		void shouldClose(bool state) const
		{
			return glfwSetWindowShouldClose(handle_, state);
		}
		void swapBuffers()
		{
			glfwSwapBuffers(handle_);
		}
		auto createSurface(vk::Instance& instance)
		{
			VkSurfaceKHR surface;
			glfwCreateWindowSurface(instance, handle_, nullptr, &surface);
			return vk::SurfaceKHR(surface);
		}
	private:
		GLFWwindow * const handle_;
	};
private:
	glfw()
	{
		if(not glfwInit()) throw std::runtime_error("glfwInit() failed");
	}
	~glfw()
	{
		glfwTerminate();
	}
	static glfw singleton;
};

glfw glfw::singleton;
