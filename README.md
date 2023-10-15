![](screenshot.png?raw=true)

# Features

- [X] GLB/GLTF import.
- [X] Frustum culling.
- [X] Diffuse, roughness textures.
- [X] Point lights.
- [X] MSAA.
- [X] Anisotropic filtering.
- [X] Environment mapping.
- [X] Normal mapping.
- [X] Post-processing pass.
- [X] Gamma correction.
- [X] HDR (tone mapping).

# Setup (Build)

Prerequisites:

- [CMake](https://cmake.org/download/)
- [Doxygen](https://doxygen.nl/download.html)
- [LLVM](https://github.com/llvm/llvm-project/releases/latest)

First, clone this repository:

```
git clone <project URL>
cd <project directory name>
git submodule update --init --recursive
```

Then, if you've never used CMake before:

Create a `build` directory next to this file, open created `build` directory and type `cmd` in Explorer's address bar. This will open up a console in which you need to type this:

```
cmake -DCMAKE_BUILD_TYPE=Debug .. // for debug mode
cmake -DCMAKE_BUILD_TYPE=Release .. // for release mode
```

This will generate project files that you will use for development.

In order to run the app you would need to copy the `res` directory next to the built binary (or create a symlink to the `res` directory next to the built binary) so that the app can find its resources.

# Update

To update this repository:

```
git pull
git submodule update --init --recursive
```

# Documentation

In order to generate the documentation you need to have [Doxygen](https://www.doxygen.nl/index.html) installed.

The documentation can be generated by executing the `doxygen` command while being in the `docs` directory. If Doxygen is installed, this will be done automatically on each build.

The generated documentation will be located at `docs/gen/html`, open the `index.html` file from this directory to see the documentation.