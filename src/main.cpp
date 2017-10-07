#include <iostream>
#include <GL/glew.h>
#include <chrono>
#include <thread>
#include <unistd.h>
#include "glfw.hpp"

float color = 0.f;
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
    mainWindow.show();
	glewExperimental = GL_TRUE;
	glewInit();

	while(not mainWindow.shouldClose()) {
        glClearColor(color, color, color, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		mainWindow.swapBuffers();
		::usleep(100000);
		glfwWaitEvents();
	}
	return 0;
}
