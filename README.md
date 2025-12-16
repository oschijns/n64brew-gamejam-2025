# N64 starting project


## Requirements

To build this project, you need
- *git*
- The *libdragon* toolchain
- *SCons*


## Initialize

This project is composed of git submodules.
To clone the project, use the following command:
```sh
git clone --recursive git@github.com:oschijns/n64brew-gamejam-2025.git
```

If you did a regular `git clone`,
you can recover the submodules using the following commands:
```sh
git submodule init
git submodule update
```

Once you have pulled the source, you can build the whole project with `SConstruct`.
The build script will compile the libraries into `.a` files, the main sources 
and convert the binaries into a ROM.
Simply run:
```sh
scons
```


## Libraries

The list of libraries usable in this project
- [libdragon](https://github.com/DragonMinded/libdragon.git)
- [Tiny3D](https://github.com/HailToDodongo/tiny3d.git)
- [Flecs](https://github.com/SanderMertens/flecs.git)

