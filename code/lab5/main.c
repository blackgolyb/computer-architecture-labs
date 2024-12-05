#include <lab.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <sys/types.h>

#define BUFFER_ADDR 0x400000
#define BUFFER_START 0x1E
#define BUFFER_END 0x3C
#define KEY 101
#define BREAK_KEY 1


typedef struct {
    char key;
    u8 scan_code;
} key_data_t;


key_data_t
get_key_data() {
    key_data_t data;

    u8 far *buffer;
    u8 far *p_start = (u8 far *)0x40001A;
    u8 start = *p_start;

    buffer = (u8 far *)(BUFFER_ADDR + start);
    data.key = *buffer;
    data.scan_code = *(buffer + 1);

    return data;
}

void
display_key_data(key_data_t data) {
    if (data.key == KEY)
        printf("Target ");

    printf("Button: '%c'\tASCII: %d\tScan code: %02X\n", data.key, data.key, data.scan_code);
}

void
propagate_buffer() {
    u8 far *p_start = (u8 far *)0x40001A;

    if (*p_start == BUFFER_END) {
        *p_start = BUFFER_START;
    } else {
        *p_start += 2;
    }
}

bool
skip_unpressed() {
    u8 far *p_start = (u8 far *)0x40001A;
    u8 far *p_end = (u8 far *)0x40001C;

    return *p_start == *p_end;
}

bool
need_to_break(key_data_t data) {
    return data.scan_code == BREAK_KEY;
}

int
main() {
    key_data_t data;

    do {
        if (skip_unpressed())
            continue;

        data = get_key_data();
        display_key_data(data);
        propagate_buffer();
    } while(!need_to_break(data));

    return 0;
}
