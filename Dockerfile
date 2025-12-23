FROM ubuntu:latest
WORKDIR /home/ubuntu

# Set environment variables
ENV N64_INST        /opt/libdragon
ENV DEBIAN_FRONTEND noninteractive

# Setup N64 toolchain, libdragon and tiny3d
RUN apt update              &&\
    apt install -yqq        \
        wget                \
        unzip               \
        build-essential     \
        cmake               \
        scons               &&\
    apt clean               &&\
    wget https://github.com/DragonMinded/libdragon/releases/download/toolchain-continuous-prerelease/gcc-toolchain-mips64-x86_64.deb &&\
    dpkg -i gcc-toolchain-mips64-x86_64.deb &&\
    rm      gcc-toolchain-mips64-x86_64.deb &&\
    wget https://github.com/DragonMinded/libdragon/archive/refs/heads/trunk.zip &&\
    unzip  trunk.zip        &&\
    rm     trunk.zip        &&\
    cd     libdragon-trunk  &&\
    ./build.sh              &&\
    cd ..                   &&\
    rm -rf libdragon-trunk  
#   wget https://github.com/HailToDodongo/tiny3d/archive/refs/heads/main.zip &&\
#   unzip  main.zip         &&\
#   rm     main.zip         &&\
#   cd     tiny3d-main      
#   make && make install    &&\
#   cd ..                   &&\
#   rm -rf tiny3d-main
