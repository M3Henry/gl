#define GLFW_INCLUDE_NONE
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>

class glfw {
public:
	static auto requiredVulkanExtensions()
	{
		unsigned int extensionCount;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
		std::vector<char const*> extensions;
		for (unsigned int i = 0; i < extensionCount; ++i)
			extensions.push_back(glfwExtensions[i]);
		return extensions;
	}
	class window
	{
	public:
		window(int width, int height, const std::string& title, GLFWmonitor * monitor = nullptr, GLFWwindow * share = nullptr)
		:	handle_(glfwCreateWindow(width, height, title.c_str(), monitor, share), glfwDestroyWindow)
		{
			if (not handle_)
				throw "No glfw window?!?";
			glfwSetWindowUserPointer(handle_.get(), this);
			glfwSetKeyCallback(handle_.get(), keyCallback);
			glfwSetWindowSizeCallback(handle_.get(), resizeCallback);
		}
		window()
		:	handle_(nullptr, glfwDestroyWindow)
		{}
		window(window const&) = delete;
		window& operator =(window const&) = delete;
		window(window&& old) :
			handle_(std::move(old.handle_)),
			keyFunction_(old.keyFunction_),
			resizeFunction_(old.resizeFunction_)
		{
			glfwSetWindowUserPointer(handle_.get(), this);
		}
		window& operator =(window&& old)
		{
			handle_ = std::move(old.handle_);
			keyFunction_ = old.keyFunction_;
			resizeFunction_ = old.resizeFunction_;
			glfwSetWindowUserPointer(handle_.get(), this);
			return *this;
		}
		void makeContextCurrent()
		{
			glfwMakeContextCurrent(handle_.get());
		}
		void setKeyCallback(void(*func)(window&, int, int, int, int))
		{
			keyFunction_ = func;
		}
		void setResizeCallback(void(*func)(window&, int, int))
		{
			resizeFunction_ = func;
		}
		void show() const
		{
			glfwShowWindow(handle_.get());
		}
		bool shouldClose() const
		{
			return glfwWindowShouldClose(handle_.get());
		}
		void shouldClose(bool state) const
		{
			return glfwSetWindowShouldClose(handle_.get(), state);
		}
		void swapBuffers()
		{
			glfwSwapBuffers(handle_.get());
		}
		std::pair<int, int> getFramebufferSize() const
		{
			std::pair<int, int> size;
			glfwGetFramebufferSize(handle_.get(), &size.first, &size.second);
			return size;
		}
		auto createSurface(vk::Instance& instance)
		{
			VkSurfaceKHR surface;
			glfwCreateWindowSurface(instance, handle_.get(), nullptr, &surface);
			return vk::SurfaceKHR(surface);
		}
	private:
		static void keyCallback(GLFWwindow* handle, int a, int b, int c, int d)
		{
			auto & window = *reinterpret_cast<glfw::window*>(glfwGetWindowUserPointer(handle));
			if (window.keyFunction_)
				window.keyFunction_(window, a, b, c, d);
		}
		static void resizeCallback(GLFWwindow* handle, int a, int b)
		{
			auto & window = *reinterpret_cast<glfw::window*>(glfwGetWindowUserPointer(handle));
			if (window.resizeFunction_)
				window.resizeFunction_(window, a, b);
		}
	private:
		std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)> handle_;
		void(*keyFunction_)(window&, int, int, int, int) = nullptr;
		void(*resizeFunction_)(window&, int, int) = nullptr;
	};
private:
	glfw()
	{
		if(not glfwInit())
			throw std::runtime_error("glfwInit() failed");
	}
	~glfw()
	{
		glfwTerminate();
	}
	static glfw singleton;
};

glfw glfw::singleton;
