all: mbrboot.bin

mbrboot.bin: mbrboot.elf
	$(OBJCOPY) -O binary mbrboot.elf mbrboot.bin

mbrboot.elf: mbrboot.o mbrboot.ld
	$(CC) -o mbrboot.elf -Tmbrboot.ld -nostdlib -Wl,-static \
	  -Wl,--gc-sections mbrboot.o 
	size mbrboot.elf

mbrboot.o: mbrboot.c
	$(CC) -c -Wall -Os -ffunction-sections -fdata-sections \
	  -mcpu=iwmmxt -marm -mthumb-interwork -fno-tree-cselim mbrboot.c

clean ::
	rm -f *.elf *.bin *.o
