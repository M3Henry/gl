# gl
*gl is a temporary name*

This is a little side project to see what I can do with Vulkan.

## Compiling
To compile, you will need:
- The [meson build](http://mesonbuild.com/) system.
- [glslangValidator](https://cvs.khronos.org/svn/repos/ogl/trunk/ecosystem/public/sdk/tools/glslang/Install/) in `/usr/local/bin`
- A recent version of GCC.

The command to **initially** configure it:
```bash
meson build && cd build
```
To [re]build, reconfigure, just run **`ninja exec`** from the `build` directory.
Obviously you will need graphics drivers and a graphics card which support the Vulkan API.
