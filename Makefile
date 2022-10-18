# CONFIG_MODULE_SIG=n

obj-m += chardriver.o

KDIR = /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(shell pwd) modules

clean:
	rm -rf .chardriver* .Module* .module* *.mod* *.o *.ko *.symvers *.order