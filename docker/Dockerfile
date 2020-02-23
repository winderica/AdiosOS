FROM ubuntu

ENV OPT=/opt
ENV BUILDS=/builds
ENV LINUX=$OPT/linux
ENV LINUX_BUILD=$BUILDS/linux
ENV CROSS_TOOL_NG=$OPT/crosstool-ng
ENV BUILDROOT=$OPT/buildroot
ENV BUILDROOT_BUILD=$BUILDS/buildroot
ENV CONFIG=$BUILDS/config
ENV APPS=$BUILDS/apps
ENV MODULES=$BUILDS/modules
ENV TOOLCHAIN=/root/x-tools/x86_64-unknown-linux-gnu/bin

RUN mkdir -p $CONFIG $APPS $LINUX_BUILD $BUILDROOT_BUILD $MODULES

COPY ../config /builds/config/
COPY ../test /builds/apps/
COPY ../3 /builds/modules/
COPY ../2/copy.c /

RUN apt update &&\
    apt install -y qemu-system-x86 bc build-essential ccache libncurses-dev libssl-dev u-boot-tools libelf-dev \
                   git gperf bison flex texinfo help2man libncurses5-dev python3-dev cpio rsync kmod \
                   autoconf automake libtool libtool-bin gawk wget bzip2 unzip patch libstdc++6 python &&\
    apt clean &&\
    rm -r /var/lib/apt/lists/* &&\
    wget https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.20.8.tar.xz &&\
    tar xf linux-*.tar.xz -C $OPT &&\
    rm linux-*tar.xz &&\
    mv $OPT/linux* $LINUX &&\
    cd $LINUX &&\
    printf '556\tcommon\tcopy\t\t\t__x64_sys_copy\n' >> arch/x86/entry/syscalls/syscall_64.tbl &&\
    sed -i '$ i\asmlinkage long sys_copy(char*, int, char*, int);' include/linux/syscalls.h &&\
    mv /copy.c kernel/ &&\
    echo 'obj-y += copy.o' >> kernel/Makefile &&\
    make O=$LINUX_BUILD allnoconfig &&\
    cd $LINUX_BUILD &&\
    cp $CONFIG/linux.config .config &&\
    make &&\
    make modules &&\
    wget http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-1.24.0.tar.xz &&\
    tar xf crosstool-ng-*.tar.xz -C $OPT &&\
    rm crosstool-ng-*.tar.xz &&\
    mv $OPT/crosstool-ng-* $CROSS_TOOL_NG &&\
    cd $CROSS_TOOL_NG &&\
    ./configure --prefix=$CROSS_TOOL_NG &&\
    make &&\
    make install &&\
    export PATH="${PATH}:$CROSS_TOOL_NG/bin" &&\
    ct-ng x86_64-unknown-linux-gnu &&\
    CT_ALLOW_BUILD_AS_ROOT_SURE=y ct-ng build &&\
    rm -r $CROSS_TOOL_NG &&\
    wget https://buildroot.org/downloads/buildroot-2019.02.9.tar.gz &&\
    tar xf buildroot-*.tar.gz -C $OPT &&\
    rm buildroot-*.tar.gz &&\
    mv $OPT/buildroot-* $BUILDROOT &&\
    cd $BUILDROOT_BUILD &&\
    touch Config.in external.mk &&\
    echo 'name: linux' > external.desc &&\
    mkdir configs overlay &&\
    cd $LINUX_BUILD &&\
    make modules_install INSTALL_MOD_PATH=$BUILDROOT_BUILD/overlay &&\
    cd $APPS &&\
    make &&\
    cp copy_test lkm_test $BUILDROOT_BUILD/overlay &&\
    cd $MODULES &&\
    make &&\
    make modules_install INSTALL_MOD_PATH=$BUILDROOT_BUILD/overlay &&\
    cd $BUILDROOT &&\
    make O=$BUILDROOT_BUILD BR2_EXTERNAL=$BUILDROOT_BUILD qemu_x86_64_defconfig &&\
    cd $BUILDROOT_BUILD &&\
    cp $CONFIG/buildroot.config .config &&\
    make savedefconfig &&\
    printf '#!/bin/sh\n\
/bin/mount -t devtmpfs devtmpfs /dev\n\
/bin/mount -t proc none /proc\n\
/bin/mount -t sysfs none /sys\n\
exec 0</dev/console\n\
exec 1>/dev/console\n\
exec 2>/dev/console\n\
exec /bin/sh +m\n' > overlay/init &&\
    chmod +x overlay/init &&\
    make &&\
    printf '#!/bin/sh\n\
qemu-system-x86_64 \
-kernel $LINUX_BUILD/arch/x86_64/boot/bzImage \
-initrd $BUILDROOT_BUILD/images/rootfs.cpio.gz \
-nographic -append "console=ttyS0"' > /start.sh &&\
    chmod +x /start.sh
CMD bash
