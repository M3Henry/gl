#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>
#include <unistd.h>

int main()
{
	std::cout << "Hello, world!" << std::endl;
	if(not glfwInit()) throw "glfwInit() failed";
    //glfwWindowHint(GLFW_VISIBLE, false);
    GLFWwindow* window = glfwCreateWindow(720, 720, "Hello Windows", NULL, NULL);
    if (not window) throw "No glfw window?!?";

    glfwSwapInterval(1);
    glfwMakeContextCurrent(window);
	glfwSetKeyCallback
	(
		window,
		[](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			std::cout << '\n' << key << std::endl;
			if (action == GLFW_PRESS) {
		        switch (key) {
		        case GLFW_KEY_ESCAPE:
		        case GLFW_KEY_Q:
		        case GLFW_KEY_F12:
		            glfwSetWindowShouldClose(window, true);
					return;
				default:
					return;
				}
			}
			return;
		}
	);
    glfwShowWindow(window);
	glewExperimental = GL_TRUE;
	glewInit();

	while(not glfwWindowShouldClose(window)) {
		std::cout << '.';
		std::cout.flush();
		::usleep(100000);
	}
	glfwDestroyWindow(window);
	char x;
	std::cin >> x;
	std::cout << "Done." << std::endl;
	return 0;
}
/*
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
        case GLFW_KEY_F12:
            glfwSetWindowShouldClose(window, true);
*/
