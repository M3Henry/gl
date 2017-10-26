project(
	'gl',
	'cpp',
	default_options: ['cpp_std=c++1z'])

inc_dirs = include_directories(['include', '/usr/include'])
sources = ['src/main.cpp']

VULKdep = dependency('vulkan', version : '>=1.0.42')
#GLEWdep = dependency('glew', version : '>=2.0.0')
GLFWdep = dependency('glfw3', version : '>=3.2.0')

glslang = find_program('glslangValidator')

ld = find_program('ld')

vertex_shader_code = custom_target('vertex-shader',
	output : 'vert.spv',
	input : 'src/shader.vert',
	command : [glslang, '-V', '-o', '@OUTPUT@', '@INPUT@'],
	install : true,
	install_dir : 'shaders')

vertex_object = custom_target('vertex-object',
	output: 'vert.o',
	input: vertex_shader_code,
	command: [ld, '-r', '-b', 'binary', '-o', '@OUTPUT@', '@INPUT@'],
	install: true,
	install_dir: 'shaders')

fragment_shader_code = custom_target('fragment-shader',
	output : 'frag.spv',
	input : 'src/shader.frag',
	command : [glslang, '-V', '-o', '@OUTPUT@', '@INPUT@'],
	install : true,
	install_dir : 'shaders')

fragment_object = custom_target('fragment-object',
	output: 'frag.o',
	input: fragment_shader_code,
	command: [ld, '-r', '-b', 'binary', '-o', '@OUTPUT@', '@INPUT@'],
	install: true,
	install_dir: 'shaders')

exe = executable(
	'exe',
	vertex_object, fragment_object,
	sources: sources,
	include_directories: inc_dirs,
	dependencies: [GLFWdep, VULKdep])
