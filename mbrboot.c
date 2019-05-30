/*
 *     Copyright (C) 2019, Technologic Systems Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define PARTITION(offs, size, type)     \
	0, 0, 0, 0, \
	(type), 0, 0, 0, \
	(unsigned char)(offs), (unsigned char)((offs) >> 8), \
	(unsigned char)((offs) >> 16), (unsigned char)((offs) >> 24), \
	(unsigned char)(size), (unsigned char)((size) >> 8), \
	(unsigned char)((size) >> 16), (unsigned char)((size) >> 24)

const unsigned char ptbl[66] __attribute__((section(".ptbl"), aligned(1))) = {
	PARTITION(1, (0x210000 / 512) - 1, 0xda),
	PARTITION((0x210000 / 512), (0x110000 / 512), 0xda),
	PARTITION((0x540000 / 512), (256 * 0x100000 / 512), 0x83),
	PARTITION(0, 0, 0),
	0x55, 0xaa
};

static unsigned char *atags;
static unsigned char atag_template[20];
static unsigned char atag_script[] = {
  /* ATAG_CORE */
  0x00, 0x05, 0x04, 0x01, 0x06, 0x41, 0xc7, 0x54,

  /* ATAG_INITRD2 */
  0x04, 0x05, 0x06, 0x42, 0x0b, 0x01, 0x0e, 0x40, 0xcf, 0x00
};

static inline void modify_atag_template(int x, unsigned char y) {
	atag_template[x & 0x3f] = y;
}
static inline unsigned char * atags_runscript(unsigned char *pc) {
	unsigned int c, i;

	for (;;) {
		c = *(pc++);
		modify_atag_template(c, *(pc++));
		if (c & 0x80) for (i = 0; i < (atag_template[0] * 4); i++)
		  *(atags++) = atag_template[i];
		if (c & 0x40) break;
	}
	return pc;
}

void mbrboot(void *, void *, void *) __attribute__((noreturn));
void mbrboot(void *read_func, void *ser_puts_func, void *start_func) {
	void (*read)(unsigned int, char *, int) = read_func;
	void (*ser_puts)(char *) = ser_puts_func;
	void (*start)(unsigned int, unsigned int, 
	  unsigned int, unsigned int) __attribute__((noreturn)) = start_func;
	unsigned int locs[2] = {0x8000, 0x1000000};
	const unsigned char *part = &ptbl[0];
	const unsigned char *part_end = part + 64;
	int i;

	ser_puts(".\r\n");

	for (i = 0; part < part_end; part += 16) {
		if (part[0x4] == 0xda) {
			unsigned int start, sz;
			unsigned short *p = (unsigned short *)&part[0x8];
			start = *p++;
			start |= *p++ << 16;
			sz = *p++;
			sz |= *p << 16;
			read(start, (char *)(locs[i++]), sz);
			ser_puts(".\r\n");
		}
	}

	if (i > 1) atag_script[6] = 0x87;
	atags = (unsigned char *)0x100;
	atags_runscript(atag_script);
	*(unsigned int *)atags = 0;
	start(0, 3200, 0x100, locs[0]);
}
