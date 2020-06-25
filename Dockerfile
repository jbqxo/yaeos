FROM archlinux

RUN pacman --noconfirm -Syu base-devel curl lib32-glibc make python python-pip
RUN pip install compiledb
# Required to build GCC
RUN pacman --noconfirm -Syu gmp libmpc mpfr
# Required to build the project
RUN pacman --noconfirm -Syu nasm clang grub xorriso mtools

# Fetch gcc sources and binutils sources.
RUN curl -O https://ftp.gnu.org/gnu/binutils/binutils-2.34.tar.xz && \
    curl -O https://ftp.gnu.org/gnu/gcc/gcc-10.1.0/gcc-10.1.0.tar.xz && \
    mkdir -p /tools/src && \
    tar -xf binutils-2.34.tar.xz -C /tools/src && \
    tar -xf gcc-10.1.0.tar.xz -C /tools/src && \
    rm binutils-2.34.tar.xz && \
    rm gcc-10.1.0.tar.xz

ENV PATH="/tools/i686/bin:$PATH"

# Build binutils cross-toolchain.
RUN mkdir -p /tools/build/binutils
WORKDIR /tools/build/binutils
RUN /tools/src/binutils-2.34/configure \
 --target=i686-elf \
 --prefix="/tools/i686" \
 --with-sysroot \
 --disable-nls \
 --disable-werror && \
 make && \
 make install

# Build GCC cross-compiler.
# GCC_ROOT is required to help build scripts find crtbegin.o and crtend.o
RUN mkdir -p /tools/build/gcc
WORKDIR /tools/build/gcc
RUN /tools/src/gcc-10.1.0/configure \
 --target=i686-elf \
 --prefix="/tools/i686" \
 --disable-nls \
 --enable-languages=c,c++ \
 --without-headers && \
 make all-gcc all-target-libgcc && \
 make install-gcc install-target-libgcc

# Cleanup
RUN rm -rf /tools/build /tools/src

# The directory where sources will be located inside a container
ARG SOURCE=/src
# The directory where build files will be located inside a container
ARG BUILD=/build

RUN mkdir -p ${SOURCE} ${BUILD}
VOLUME [ ${SOURCE}, ${BUILD} ]
ENV BUILDDIR_BUILD=${BUILD}

WORKDIR ${SOURCE}
CMD [ "make", "grub-iso" ]
