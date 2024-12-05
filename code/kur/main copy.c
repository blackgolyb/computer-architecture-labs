#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

// Частота системного генератора імпульсів
#define SYSTEM_CLOCK 1193180

// Адреси портів
#define TIMER_CHANNEL_2 0x42
#define TIMER_CONTROL 0x43
#define SPEAKER_PORT 0x61

// Адреси портів DMA
#define DMA_BASE_ADDRESS 0xC0
#define DMA_PAGE_REGISTER 0x8F


typedef enum {
    PAUSE = -1,
    C,
    Cd,
    D,
    Dd,
    E,
    F,
    Fd,
    G,
    Gd,
    A,
    Ad,
    B,
} note_declaration_t;


// Структура для опису ноти
typedef struct {
    char octave;
    note_declaration_t note;
    unsigned int duration;   // Тривалість у мс
} note_t;

// Структура для опису звуку
typedef struct {
    unsigned int frequency;  // Частота в Гц
    unsigned int duration;   // Тривалість у мс
} sound_t;

note_t melody[] = {
    {4, G, 3000}, {4, E, 3000}, {4, D, 4000},
};


unsigned int read_dma_register(unsigned int channel) {
    unsigned int address_port = DMA_BASE_ADDRESS + (channel - 4) * 2;
    unsigned int count_port = DMA_BASE_ADDRESS + (channel - 4) * 2 + 1;

    // Зчитуємо початкову адресу
    unsigned char low = inp(address_port);       // Молодший байт
    unsigned char high = inp(address_port + 1); // Старший байт
    unsigned int address = (high << 8) | low;

    // Зчитуємо початковий лічильник
    unsigned char count_low = inp(count_port);       // Молодший байт
    unsigned char count_high = inp(count_port + 1);  // Старший байт
    unsigned int count = (count_high << 8) | count_low;

    printf("DMA channels %u: Starting Address = 0x%X, Starting Counter = %u\n", channel, address, count);

    return address;
}


// Функція для встановлення частоти ноти
void play_note(unsigned int frequency, unsigned int duration_ms) {
    unsigned char speaker_state;
    unsigned int divisor;

    if (frequency == 0) {  // Для пауз
        delay(duration_ms);
        return;
    }
    divisor = SYSTEM_CLOCK / frequency;

    outp(TIMER_CONTROL, 0xB6);  // 10110110: Канал 2, режим 3 (генератор меандру)
    outp(TIMER_CHANNEL_2, divisor & 0xFF);        // Молодший байт
    outp(TIMER_CHANNEL_2, (divisor >> 8) & 0xFF); // Старший байт

    speaker_state = inp(SPEAKER_PORT);
    outp(SPEAKER_PORT, speaker_state | 0x03);  // Увімкнути біти 0 і 1

    delay(duration_ms);

    outp(SPEAKER_PORT, speaker_state & ~0x03);  // Вимкнути біти 0 і 1
}


// Функція для відтворення мелодії
void play_melody(note_t *melody, unsigned int length) {
    unsigned int i;
    for (i = 0; i < length; ++i) {
        play_note(melody[i].frequency, melody[i].duration);
    }
}


int main() {
    unsigned int melody_length = sizeof(melody) / sizeof(melody[0]);
    unsigned int i = 4;

    printf("Reading DMA registers for channels 4-7:\n");
    for (i = 4; i <= 7; ++i) {
        read_dma_register(i);
    }

    play_melody(melody, melody_length);

    return 0;
}
