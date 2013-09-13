KERNEL_BUILD := /home/showp1984/WORK/kernel/3.1.0-modbuild-kernel-tegra-aosp/

obj-m += kcontrol_gpu_tegra.o

all:
ifneq ($(wildcard mach-tegra),)
	rm mach-tegra
endif
	ln -s $(KERNEL_BUILD)arch/arm/mach-tegra/ mach-tegra
	make -C $(KERNEL_BUILD) M=$(PWD) modules
	$(CROSS_COMPILE)strip --strip-debug kcontrol_gpu_tegra.ko

clean:
ifneq ($(wildcard mach-tegra),)
	rm mach-tegra
endif
	make -C $(KERNEL_BUILD) M=$(PWD) clean 2> /dev/null
	rm -f modules.order *~ *.o
