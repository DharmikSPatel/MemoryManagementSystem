CC = gcc
CFLAGS = -g -m32 -lm #if on 64 bit machine, remove -m32 and -lm
TESTFLAGS = $(CFLAGS) -std=c99 -Wall -fsanitize=address,undefined 
AR = ar -rc
RANLIB = ranlib
vm_test: my_vm.a
	$(CC) $(TESTFLAGS) -o vm_test vm_test.c my_vm.a

bitmap_test: bitmap_test.c bitmap.o
	$(CC) $(TESTFLAGS) -o bitmap_test bitmap_test.c bitmap.o 

my_vm.a: my_vm.o bitmap.o
	$(AR) $@ $^
	$(RANLIB) $@

clean:
	rm -rf *.o *.a output* my_vm bitmap bitmap_test vm_test my_vm.dSYM vm_test.dSYM bitmap_test.dSYM