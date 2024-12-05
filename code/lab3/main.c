#include <lab.h>
#include <stdio.h>
#include <dos.h>

#define SYS_INT 0x9
#define USR_INT 0x60
#define SIZE_IN_BLOCKS 1024
#define TARGET_KEY 0x1
#define BREAK_KEY 0x4
#define ADRESS_KEY_STATE 0x400017
#define PRESS_TEXT "Target Key(Right Shift) Pressed\n\r"
#define END_TEXT "End Programm\n\r"

typedef void interrupt far (*funcInt)();


funcInt int9, int60;

void
putLine(const char* text) {
	size_t i = 0;
	char current;

	do {
		current = text[i++];
		asm {
			mov ah, 0xE
			mov al, current
			int 0x10
		}
	} while (current != '\0');
}

void interrupt far
intKey() {
	u8 far *pKeyState = (u8 far *)ADRESS_KEY_STATE;
	u8 state = *pKeyState;
	u8 isPress = state & TARGET_KEY;
	u8 isOffing = false;

	if (isPress)
		putLine(PRESS_TEXT);


	isPress = state & BREAK_KEY;
	if (isPress) {
		putLine(END_TEXT);
		isOffing = true;
		setvect(SYS_INT, int9);
		setvect(USR_INT, int60);
		geninterrupt(SYS_INT);
	}

	if (!isOffing)
		geninterrupt(USR_INT);
}

int
makeResident() {
	union REGS rr;

	rr.h.ah = 0x31;
	rr.h.al = 0;
	rr.x.dx = SIZE_IN_BLOCKS;

	int86(0x21, &rr, &rr);
}

int
main() {
	int9 = getvect(SYS_INT);
	int60 = getvect(USR_INT);

	setvect(USR_INT, int9);
	setvect(SYS_INT, intKey);

	makeResident();

	return 0;
}
