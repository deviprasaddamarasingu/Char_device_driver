#specify the output file 
obj-m := char_driver_udev.o

#setup the variables
KERN_SRC = /lib/modules/$(shell uname -r)/build 
PWD = $(shell pwd)

#make target for compiling the modules
all :
	make -C $(KERN_SRC) M=$(PWD) modules

#make target for cleaning up the space
clean :
#	make -C $(KERN_SRC) m=$(PWD) clean
	rm *.mod* *.symvers *.order *.o 



