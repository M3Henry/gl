
#include <GLFW/glfw3.h>

namespace glfw {
	class window {
	public:
		window(
			int width, int height,
			const std::string& title,
			GLFWmonitor * monitor = nullptr,
			GLFWwindow * share = nullptr) :
			window_(glfwCreateWindow(width, height, title.c_str(), monitor, share))
		{
		    if (not window_) throw "No glfw window?!?";
		}
		~window()
		{
			glfwDestroyWindow(window_);
		}
		void makeContextCurrent()
		{
			glfwMakeContextCurrent(window_);
		}
		void setKeyCallback(void(*func)(GLFWwindow*, int, int, int, int))
		{
			glfwSetKeyCallback(window_, func);
		}
		void show() const
		{
			glfwShowWindow(window_);
		}
		bool shouldClose() const
		{
			return glfwWindowShouldClose(window_);
		}
		void shouldClose(bool state) const
		{
			return glfwSetWindowShouldClose(window_, state);
		}
	private:
		GLFWwindow * const window_;
	};
}
