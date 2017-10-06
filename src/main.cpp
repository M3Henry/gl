#include <iostream>
#include <GL/glew.h>
#include <chrono>
#include <thread>
#include <unistd.h>
#include "glfw.hpp"

int main()
{
	std::cout << "Hello, world!" << std::endl;
	if(not glfwInit()) throw "glfwInit() failed";
    //glfwWindowHint(GLFW_VISIBLE, false);
    glfw::window mainWindow(720, 720, "Hello GLFW");

    glfwSwapInterval(1);
    mainWindow.makeContextCurrent();
	mainWindow.setKeyCallback
	(
		[](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (action == GLFW_PRESS) {
				std::cout << char(key);
		        switch (key) {
		        case GLFW_KEY_ESCAPE:
		            glfwSetWindowShouldClose(window, true);
					return;
				default:
					return;
				}
			}
			return;
		}
	);
    mainWindow.show();
	glewExperimental = GL_TRUE;
	glewInit();

	while(not mainWindow.shouldClose()) {
		glfwWaitEvents();
	}
	return 0;
}
