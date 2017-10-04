project(
	'gl',
	'cpp',
	default_options: ['cpp_std=c++1z'])

inc_dirs = include_directories(['/usr/include'])
sources = ['src/main.cpp']

GLEWdep = dependency('glew', version : '>=2.0.0')
GLFWdep = dependency('glfw3', version : '>=3.2.0')

exe = executable(
	'exe',
	sources: sources,
	include_directories: inc_dirs,
	dependencies: [GLEWdep, GLFWdep])
