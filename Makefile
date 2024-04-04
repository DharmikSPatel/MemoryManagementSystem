CC = gcc
CFLAGS = -g 
TESTFLAGS = -g -std=c99 -Wall -fsanitize=address,undefined 
AR = ar -rc
RANLIB = ranlib
#TODO: run on ilab
vm_test: my_vm.a
	$(CC) $(TESTFLAGS) -o vm_test vm_test.c my_vm.a

bitmap_test: bitmap_test.c bitmap.o
	$(CC) $(TESTFLAGS) -o bitmap_test bitmap_test.c bitmap.o

my_vm.a: my_vm.o bitmap.o
	$(AR) $@ $^
	$(RANLIB) $@

clean:
	rm -rf *.o *.a my_vm bitmap bitmap_test vm_test TLB