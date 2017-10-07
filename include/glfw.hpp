
#include <GLFW/glfw3.h>

namespace glfw {
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
}
