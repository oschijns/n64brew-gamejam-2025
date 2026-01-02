#!/usr/bin/env python3

import os
import SCons
from SCons.Builder import Builder
from SCons.Action  import Action
from SCons.Script  import GetOption
#from pathlib       import Path

# N64 toolchain configuration
N64_INST = os.environ.get('N64_INST', '/opt/libdragon')

# Assets
assets_tracks = 'assets/tracks/*.bin'
assets_models = 'assets/models/*.glb'

# Source files
source_files = [
    # compile C files at the root
    'src/*.c',
    'src/*.cpp',

    # Compile C files in sub directories
    'src/**/*.c',
    'src/**/*.cpp',
]

# Include paths
include_paths = [
    f'{N64_INST}/mips64-elf/include',
    'libraries/libdragon/include',
    'libraries/tiny3d/src',
    'libraries/flecs/include',
    'src'
]

# Libraries to link against
libraries = [
    # Project and third-party libraries first

    # libdragon
    'dragon',
    'dragonsys',

    # Tiny3D
    't3d',

    # Flecs ECS
    'flecs_static',

    # System/runtime libraries last (searched after objects and project libs)
    'stdc++',
    'm',
    'c',
    'gcc'
]

# N64 specific compiler flags
n64_cflags = [
    # ABI and kernel TLS include required by libdragon toolchain
    '-mabi=o64',
    '-march=vr4300',
    '-mtune=vr4300',
    '-falign-functions=32',
    '-ffunction-sections',
    '-fdata-sections',
    '-g',
    '-ffast-math',
    '-ftrapping-math',
    '-fno-associative-math',
    '-DN64',
    '-O2',
    '-Wall',
    '-Werror',
    '-Wno-error=deprecated-declarations',
    '-Wno-error=unused-variable',
    '-Wno-error=unused-but-set-variable',
    '-Wno-error=unused-function',
    '-Wno-error=unused-parameter',
    '-Wno-error=unused-but-set-parameter',
    '-Wno-error=unused-label',
    '-Wno-error=unused-local-typedefs',
    '-Wno-error=unused-const-variable',
    '-Wno-error=invalid-constexpr',
    '-ftrivial-auto-var-init=pattern'
]

# Create environment with N64 cross-compilation tools
env = SCons.Environment.Environment(
    # path to the executables
    CC     = f'{N64_INST}/bin/mips64-elf-gcc',
    CXX    = f'{N64_INST}/bin/mips64-elf-g++',
    AR     = f'{N64_INST}/bin/mips64-elf-gcc-ar',
    RANLIB = f'{N64_INST}/bin/mips64-elf-gcc-ranlib',
    AS     = f'{N64_INST}/bin/mips64-elf-as',
    STRIP  = f'{N64_INST}/bin/mips64-elf-strip',
    # Use the compiler driver for linking so it pulls in the correct startup objects
    # and automatically links libgcc/libstdc++/libc where appropriate.
    LINK   = f'{N64_INST}/bin/mips64-elf-g++',

    # flags
    CCFLAGS   = n64_cflags,
    CXXFLAGS  = n64_cflags,
    ASFLAGS   = n64_cflags,
    LINKFLAGS = [
        '-g',

        # libdragon
        f'-L{N64_INST}/mips64-elf/lib',
        '-Llibraries/libdragon',

        # Tiny3D
        '-Llibraries/tiny3d/build',

        # Flecs ECS
        '-Llibraries/flecs',

        # Use the libdragon N64 linker script and wrap global ctor handling.
        f'-Wl,-T,{N64_INST}/mips64-elf/lib/n64.ld',
        '-Wl,--wrap,__do_global_ctors',
        # pass gc-sections and other linker options through -Wl,
        # so they are forwarded by the compiler driver to the linker.
        '-Wl,--gc-sections',
    ],

    CPPPATH = include_paths,
    LIBS    = libraries,

    # Set environment variables for commands
    ENV = os.environ
)

# Number of jobs to use
jobs: int = GetOption('num_jobs') or 1

# define path to library builds
lib_libdragon = [
    'libraries/libdragon/libdragon.a', 
    'libraries/libdragon/libdragonsys.a',
]
lib_tiny3d = [
    'libraries/tiny3d/build/libt3d.a'
]
lib_flecs = [
    'libraries/flecs/libflecs_static.a'
]

# Define rules to build libraries
env.Command(
    target = lib_libdragon, 
    source = [],
    action = Action(chdir = 'libraries/libdragon', act = f'make -j{jobs} all')
)
env.Command(
    target = lib_tiny3d, 
    source = [],
    action = Action(chdir = 'libraries/tiny3d', act = f'make -j{jobs} all')
)
env.Command(
    target = lib_flecs, 
    source = [], 
    action = [
        Action(chdir = 'libraries/flecs', act = f'cmake -DN64_INST={N64_INST} -DCMAKE_TOOLCHAIN_FILE=../n64_flecs.cmake .'),
        Action(chdir = 'libraries/flecs', act = f'make -j{jobs} flecs_static')
    ]
)


# Build directory
env.VariantDir('build', 'src', duplicate=0)

build_sources = []
for src in source_files:
    build_sources.extend(env.Glob(src))

# Create build directory objects
#build_sources = [f'{build_dir}/{os.path.basename(src)}' for src in source_files]

# Build the ELF executable
elf_target = 'build/Build.elf'
elf = env.Program(target=elf_target, source=build_sources)

# N64 ROM creation tools
n64_tools = {
    'N64SYM'         : f'{N64_INST}/bin/n64sym',
    'N64TOOL'        : f'{N64_INST}/bin/n64tool',
    'N64ELFCOMPRESS' : f'{N64_INST}/bin/n64elfcompress',
    'N64MKASSET'     : f'{N64_INST}/bin/mkasset',
    'N64MKDFS'       : f'{N64_INST}/bin/mkdfs',
    'T3DGLTF'        : f'{N64_INST}/bin/gltf_to_t3d'
}


# Define builders for assets
def build_dfs(target, source, env):
    # Make all the assets
    os.makedirs("filesystem", exist_ok=True)

    for t3d_model in env.Glob(assets_models):
        outs = os.path.basename(str(t3d_model)).replace(".glb", ".t3dm")
        result = env.Execute(
            f"N64_INST={N64_INST} {n64_tools['T3DGLTF']} -v {t3d_model.get_path()} {outs}"
        )
        if result != 0:
            raise RuntimeError("Failed to make the t3d models")

        result = env.Execute(
            f"N64_INST={N64_INST} {n64_tools['N64MKASSET']} -c 3 -o filesystem {outs}"
        )

    for asset in env.Glob(assets_tracks):
        result = env.Execute(
            f"N64_INST={N64_INST} {n64_tools['N64MKASSET']} -c 3 -o filesystem {asset.get_path()}"
        )
        if result != 0:
            raise RuntimeError(f"Couldn't mkasset {result}")

    env.Execute(
        f"N64_INST={N64_INST} {n64_tools['N64MKDFS']} {target[0].get_path()} filesystem"
    )


# Custom builder for N64 ROM
def build_n64_rom(target, source, env):
    """Custom builder to create N64 ROM from ELF"""
    elf_file = str(source[0])
    rom_file = str(target[0])

    # Set N64_INST environment variable for tools
    tool_env = env['ENV'].copy()
    tool_env['N64_INST'] = N64_INST

    # Create symbol file
    sym_file = f'{elf_file}.sym'
    result = env.Execute(f'N64_INST={N64_INST} {n64_tools["N64SYM"]} {elf_file} {sym_file}')
    if result != 0:
        return result

    # Strip the ELF file
    stripped_file = f'{elf_file}.stripped'
    env.Execute(f'cp {elf_file} {stripped_file}')
    env.Execute(f'{env["STRIP"]} -s {stripped_file}')

    # Compress the ELF
    env.Execute(f'N64_INST={N64_INST} {n64_tools["N64ELFCOMPRESS"]} -o build/ -c 1 {stripped_file}')

    # Create the ROM
    cmd = [
        f'N64_INST={N64_INST}',
        n64_tools['N64TOOL'],
        '--toc',
        '--title', 'N64-Demo',
        '--output', rom_file,
        '--align', '256',
        stripped_file,
        sym_file,
        'libraries/libdragon/libdragon.version',
        'build/Build.dfs'
    ]
    env.Execute(' '.join(cmd))

    return 0

# Register the custom builder
dfs_builder = Builder(action=build_dfs    , suffix='.dfs', src_suffix=['.elf', '.bin'])
rom_builder = Builder(action=build_n64_rom, suffix='.z64', src_suffix=['.elf', '.dfs'])
env.Append(BUILDERS={'N64ROM': rom_builder, 'N64DFS': dfs_builder})

# Build the ROM
rom_target = 'n64brew_gamejam.z64'
rom = env.N64ROM(target=rom_target, source=elf)

# Set up dependencies
env.Depends(elf, lib_libdragon + lib_tiny3d + lib_flecs)
env.Depends(rom, env.N64DFS(target="build/Build.dfs", source=elf))
env.Depends(rom, elf)

# Default target
Default(rom)

# Clean target
env.Clean(rom, ['build', rom_target])

# Aliases
env.Alias('elf', elf)
env.Alias('rom', rom)
env.Alias('all', [elf, rom])


# Help text
Help("""
N64brew Game Jamp 2025

Targets:
  scons     - Build ROM (default)
  scons elf - Build ELF executable only
  scons rom - Build ROM only
  scons -c  - Clean build files

Environment Variables:
  N64_INST - Path to libdragon installation (default: /opt/libdragon)
""")
