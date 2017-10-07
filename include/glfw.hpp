#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class glfw {
public:
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
