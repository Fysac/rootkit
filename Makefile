obj-m += rootkit.o
rootkit-objs := src/rootkit.o src/device.o src/hide.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
