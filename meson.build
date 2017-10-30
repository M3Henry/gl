project(
	'gl',
	'cpp',
	default_options: ['cpp_std=c++1z'])

inc_dirs = include_directories(['include', '/usr/include'])
sources = ['src/main.cpp']

VULKdep = dependency('vulkan', version : '>=1.0.42')
GLFWdep = dependency('glfw3', version : '>=3.2.0')

glslang = find_program('glslangValidator')

ld = find_program('ld')

glcmd = [glslang, '-V', '-o', '@OUTPUT@', '@INPUT@']

glcompiler = generator(glslang,
	arguments: ['-V', '-o', '@OUTPUT@', '@INPUT@'],
	output: '@PLAINNAME@.spv')

embed_file = generator(ld,
	arguments: ['-r', '-b', 'binary', '-o', '@OUTPUT@', '@INPUT@'],
	output: '@PLAINNAME@.o')

embed = find_program('embed.sh')

shaders = custom_target('shaders',
	output: 'shader.o',
	input: [glcompiler.process('src/shader.frag'), glcompiler.process('src/shader.vert')],
	command: [embed, '@OUTPUT@', '@INPUT@'],
	build_by_default: true)

exe = executable(
	'exe',
	shaders,
	sources: sources,
	include_directories: inc_dirs,
	dependencies: [GLFWdep, VULKdep])

run_target('exec',
	command: exe)
