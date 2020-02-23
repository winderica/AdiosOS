ifneq ($(KERNELRELEASE),)
obj-m := lkm.o
else
KDIR ?= $(LINUX_BUILD)
default:
	$(MAKE) -C $(KDIR) M=$$PWD
modules_install:
	$(MAKE) -C $(KDIR) M=$$PWD $@
clean:
	rm -rf *.o .*.cmd *.ko *.mod.c modules.order Module.symvers .tmp_versions
endif