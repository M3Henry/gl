project(
	'gl',
	'cpp',
	default_options: ['cpp_std=c++1z'])

inc_dirs = include_directories(['include', '/usr/include'])
sources = ['src/main.cpp']

VULKdep = dependency('vulkan', version : '>=1.0.42')
#GLEWdep = dependency('glew', version : '>=2.0.0')
GLFWdep = dependency('glfw3', version : '>=3.2.0')

exe = executable(
	'exe',
	sources: sources,
	include_directories: inc_dirs,
	dependencies: [GLFWdep, VULKdep])

comp = find_program('glslangValidator')

vertex_target = custom_target('vertex-shader',
	output : 'vert.spv',
	input : 'src/shader.vert',
	command : [comp, '-V', '-o', '@OUTPUT@', '@INPUT@'],
	install : true,
	install_dir : 'shaders')

fragment_target = custom_target('fragment-shader',
	output : 'frag.spv',
	input : 'src/shader.frag',
	command : [comp, '-V', '-o', '@OUTPUT@', '@INPUT@'],
	install : true,
	install_dir : 'shaders')
