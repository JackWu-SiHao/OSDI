# Makefile for the simple example kernel.
AS86	=as86 -0 -a
LD86	=ld86 -0
CC	=gcc
AS	=as
LD	=ld
LDFLAGS =-m elf_i386 -Ttext 0 -e startup_32 -M  #-s -x
CFLAGS  = -g -m32 -fno-builtin -fno-stack-protector -fomit-frame-pointer -fstrength-reduce

all:	Image

Image: boot system
	dd bs=32 if=boot of=Image skip=1
	objcopy -O binary system head
	cat head >> Image
	dd bs=512 if=/dev/zero count=1 >> Image

disk: Image
	dd bs=8192 if=Image of=/dev/fd0
	sync;sync;sync

head.o:
	$(AS) -g head.s -o head.o
main.o:
	$(CC) $(CFLAGS) -c main.c -o main.o

system:	head.o main.o
	$(LD) $(LDFLAGS) head.o main.o -o system > System.map


boot:	boot.s
	$(AS86) -o boot.o boot.s
	$(LD86) -s -o boot boot.o

clean:
	rm -f Image System.map core boot head *.o system

run:
	qemu -m 16M -boot a -fda Image

debug:
	qemu -m 16M -boot a -fda Image -s -S -serial stdio
