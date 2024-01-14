obj-m += myled.o

all: myled.ko test

myled.ko: myled.c
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

test: test.c
	$(CC) -o $@ $<

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	-rm test
