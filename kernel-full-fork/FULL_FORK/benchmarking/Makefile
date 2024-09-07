obj-m = fullfork.o

KVERSION = $(shell uname -r)

all: forker
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules


forker: forker.c
	gcc forker.c -o forker

clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
	rm forker
