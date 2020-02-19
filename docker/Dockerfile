FROM ubuntu
ENV OPT /opt
ENV LINUX /opt/linux
ENV LINUX_BUILD /builds/linux
ENV CROSS_TOOL_NG /opt/crosstool-ng
ENV BUILDROOT /opt/buildroot
ENV BUILDROOT_BUILD /builds/buildroot
COPY ./linux.config /
COPY ./buildroot.config /
COPY ./init /
RUN apt update &&\
    apt install -y qemu qemu-system bc build-essential ccache libncurses-dev libssl-dev u-boot-tools libelf-dev\
                   git gcc g++ gperf bison flex texinfo help2man make libncurses5-dev python3-dev cpio rsync\
                   autoconf automake libtool libtool-bin gawk wget bzip2 xz-utils unzip patch libstdc++6 python &&\
    apt clean &&\
    mkdir -p $LINUX_BUILD &&\
    mkdir -p $BUILDROOT_BUILD &&\
    wget https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.20.8.tar.xz &&\
    tar xf linux-*.tar.xz -C $OPT &&\
    rm linux-*tar.xz &&\
    mv $OPT/linux* $LINUX &&\
    cd $LINUX &&\
    make O=$LINUX_BUILD allnoconfig &&\
    cd $LINUX_BUILD &&\
    mv /linux.config .config &&\
    make &&\
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
    CT_ALLOW_BUILD_AS_ROOT_SURE=y ct-ng build.2 &&\
    wget https://buildroot.org/downloads/buildroot-2019.02.9.tar.gz &&\
    tar xf buildroot-*.tar.gz -C $OPT &&\
    rm buildroot-*.tar.gz &&\
    mv $OPT/buildroot-* $BUILDROOT &&\
    cd $BUILDROOT_BUILD &&\
    touch Config.in external.mk &&\
    echo 'name: linux' > external.desc &&\
    mkdir configs overlay &&\
    cd $BUILDROOT &&\
    make O=$BUILDROOT_BUILD BR2_EXTERNAL=$BUILDROOT_BUILD qemu_x86_64_defconfig &&\
    cd $BUILDROOT_BUILD &&\
    mv /buildroot.config .config &&\
    make savedefconfig &&\
    mv /init overlay/init &&\
    chmod +x overlay/init &&\
    make
