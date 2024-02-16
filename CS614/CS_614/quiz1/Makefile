obj-m = traphook.o

KVERSION = $(shell uname -r)

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
	gcc -g testcdev.c -o testcdev 
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
	rm -f testcdev

