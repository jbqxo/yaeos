FROM archlinux

# Target arch
ARG ARCH
# Target triplet
ARG TRIPLET
# The directory where sources will be located inside a container
ARG SOURCE
# The directory where build files will be located inside a container
ARG BUILD
# The number of jobs for make to run
ARG JOBS

RUN pacman --noconfirm -Syu base-devel curl lib32-glibc make python
# Required to build GCC
RUN pacman --noconfirm -S gmp libmpc mpfr
# Required to build the project
RUN pacman --noconfirm -S clang llvm grub xorriso mtools

# Fetch gcc sources and binutils sources.
RUN curl -O https://ftp.gnu.org/gnu/binutils/binutils-2.34.tar.xz && \
	curl -O https://ftp.gnu.org/gnu/gcc/gcc-10.1.0/gcc-10.1.0.tar.xz && \
	curl -O https://ftp.gnu.org/gnu/gdb/gdb-9.2.tar.xz && \
	mkdir -p /tools/src && \
	tar -xf binutils-2.34.tar.xz -C /tools/src && \
	tar -xf gcc-10.1.0.tar.xz -C /tools/src && \
	tar -xf gdb-9.2.tar.xz -C /tools/src && \
	rm binutils-2.34.tar.xz && \
	rm gcc-10.1.0.tar.xz && \
	rm gdb-9.2.tar.xz

ENV PATH="/tools/${TRIPLET}/bin:$PATH"

# Build binutils cross-toolchain.
RUN mkdir -p /tools/build/binutils
WORKDIR /tools/build/binutils
RUN /tools/src/binutils-2.34/configure \
	--target=${TRIPLET} \
	--prefix="/tools/${TRIPLET}" \
	--with-sysroot \
	--disable-nls \
	--disable-werror && \
	make -j${JOBS} && \
	make install

# Build GCC cross-compiler.
# GCC_ROOT is required to help build scripts find crtbegin.o and crtend.o
RUN mkdir -p /tools/build/gcc
WORKDIR /tools/build/gcc
RUN /tools/src/gcc-10.1.0/configure \
	--target=${TRIPLET} \
	--prefix="/tools/${TRIPLET}" \
	--disable-nls \
	--enable-languages=c,c++ \
	--without-headers && \
	make -j${JOBS} all-gcc all-target-libgcc && \
	make install-gcc install-target-libgcc

# Build GDB Server
RUN mkdir -p /tools/build/gdb
WORKDIR /tools/build/gdb
RUN /tools/src/gdb-9.2/gdb/gdbserver/configure \
	--prefix="/tools/${TRIPLET}" && \
	make -j${JOBS} && \
	make install

# Cleanup
RUN rm -rf /tools/build /tools/src

RUN mkdir -p ${SOURCE} ${BUILD}
VOLUME [ ${SOURCE}, ${BUILD} ]

WORKDIR ${SOURCE}
# Port for gdbserver
EXPOSE 1234
ENV TARGET_ARCH="${ARCH}"
CMD "sh"
