#include <lab.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

#define LIGHT CAPS_LOCK
#define FREQUENCY 0x1F
#define DELAY 0x00
#define LOOP_DELAY 100
#define KEY 0x05
#define BREAK_KEY 0x1


u8 light_data;

typedef enum {
    SCROLL_LOCK = 0,
    NUM_LOCK = 1,
    CAPS_LOCK = 2,
} light_t;


void
change_bit(u8 *data, usize pos, bool b) {
    int mask = 1 << pos;
    *data = ((*data & ~mask) | (b << pos));
}

void
setup_auto_repeat(u8 frequency, u8 delay) {
    outp(0x60, 0xF3);
    outp(0x60, frequency | (delay << 5));
}

void
switch_light(light_t light, bool state) {
    change_bit(&light_data, light, state);
    outp(0x60, 0xED);
    outp(0x60, light_data);
}

void
off_lights(void) {
    light_data = 0;
    outp(0x60, 0xED);
    outp(0x60, 0x00);
}

void
reset_to_bios(void) {
    outp(0x60, 0xF6);
}

void
task() {
    u8 check;
    bool state = false;
    bool on = false;
    setup_auto_repeat(FREQUENCY, DELAY);

    loop {
        check = inp(0x60);
        if (check == BREAK_KEY || check == BREAK_KEY + 128) {
            break;
        } else if (check == KEY) {
            if (on) {
                printf("Key with scan code 0x%X in auto-repeat\n", KEY);
            } else {
                printf("Key with scan code 0x%X pressed relese scancode(0x%X)\n", KEY, KEY + 128);
            }
            on = true;
            state = true;
        } else if (check == KEY + 128) {
            printf("Key with scan code 0x%X released\n", check);
            on = false;
            switch_light(LIGHT, false);
        }
        if (on) {
            state = !state;
            switch_light(LIGHT, state);
        }

        delay(LOOP_DELAY);
    }
}

int
main() {
    // Init
    off_lights();

    task();

    // Clean up
    off_lights();
    reset_to_bios();

    return 0;
}
