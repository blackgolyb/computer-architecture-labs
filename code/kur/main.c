#include <ctype.h>
#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Частота системного генератора імпульсів
#define SYSTEM_CLOCK 1193180

// Адреси портів
#define TIMER_CHANNEL_2 0x42
#define TIMER_CONTROL 0x43
#define SPEAKER_PORT 0x61

// Адреси портів DMA
#define DMA_BASE_ADDRESS 0xC0
#define DMA_PAGE_REGISTER 0x8F


typedef double (*slide_function_t)(double, double, double);

typedef enum {
    NOTE,
    SLIDE,
    PAUSE,
} play_types_t;

typedef enum {
    C = 0,
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
    // Cb = -1,
    // C,
    // Db = 1,
    // D,
} note_declaration_t;

// Структура для опису ноти
typedef struct {
    char octave;
    note_declaration_t note;
} note_t;

// Структура для опису звуку
typedef struct {
    play_types_t type;
    union {
        note_t note;
        struct {
           note_t start;
           note_t end;
           slide_function_t function;
        } slide;
    } data;
    unsigned int duration;   // Тривалість у мс
} sound_t;


double
linear(double start, double end, double t) {
    return (end - start) * t + start;
}

double
ease_in_out(double start, double end, double t) {
    t = t * t * (3 - 2 * t); // Рівномірне уповільнення на початку та наприкінці
    return (end - start) * t + start;
}

double
ease_out(double start, double end, double t) {
    t = 1 - pow(1 - t, 2); // Прискорення на початку, потім уповільнення
    return (end - start) * t + start;
}

double
ease_in(double start, double end, double t) {
    t = pow(t, 2); // Уповільнення на початку, потім прискорення
    return (end - start) * t + start;
}

typedef struct {
    char symbol;
    slide_function_t function;
} slide_settings_t;

slide_settings_t slides[] = {
    {'-', linear},
    {'~', ease_in_out},
    {'(', ease_out},
    {')', ease_in},
};
unsigned int divisor;
unsigned char speaker_state;


unsigned long
get_time_in_milliseconds(void) {
    unsigned int low, high;
    unsigned long ticks;

    asm {
        mov ax, 0x40        // Сегмент BIOS Data Area
        mov es, ax          // Завантажуємо сегмент у ES
        mov bx, 0x6C        // Зміщення таймера
        mov ax, word ptr es:[bx]   // Читаємо молодші 16 біт
        mov word ptr low, ax
        mov ax, word ptr es:[bx + 2] // Читаємо старші 16 біт
        mov word ptr high, ax
    }

    // Об'єднання молодших і старших 16 біт
    ticks = ((unsigned long)high << 16) | low;

    return ticks * 55; // Конвертуємо тики у мілісекунди
}

char
note_to_char(note_declaration_t note) {
    switch (note) {
        case 0: return 'C';
        case 1: return 'D';
        case 2: return 'E';
        case 3: return 'F';
        case 4: return 'G';
        case 5: return 'A';
        case 6: return 'B';
        default: return '!';
    }
}

void
print_note(note_t note) {
    char note_c;
    int dies = 0;

    if (note.note <= E) {
        note_c = note_to_char(note.note / 2);
        dies = note.note % 2;
    } else {
        note_c = note_to_char((note.note + 1) / 2);
        dies = (note.note + 1) % 2;
    }

    if (dies) {
        printf("%c#%d", note_c, note.octave);
    } else {
        printf("%c%d", note_c, note.octave);
    }
}

void
print_sound(const sound_t *sound) {
    switch (sound->type) {
        case NOTE:
            printf("NOTE: ");
            print_note(sound->data.note);
            break;
        case SLIDE:
            printf("SLIDE: ");
            print_note(sound->data.slide.start);
            printf(" ~ ");
            print_note(sound->data.slide.end);
            break;
        case PAUSE:
            printf("PAUSE");
            break;
    }
    printf(", Duration: %u ms\n", sound->duration);
}

void
print_sounds(const sound_t sounds[], size_t sound_count) {
    size_t i;
    for (i = 0; i < sound_count; i++) {
        print_sound(&sounds[i]);
    }
}


int
parse_note(const char *token, note_t *note) {
    char note_char;
    if (!token || !note) return 0;
    // printf("%s\n", token);

    note_char = token[0];

    switch (note_char) {
        case 'C': note->note = C; break;
        case 'D': note->note = D; break;
        case 'E': note->note = E; break;
        case 'F': note->note = F; break;
        case 'G': note->note = G; break;
        case 'A': note->note = A; break;
        case 'B': note->note = B; break;
        default: return 0;
    }

    if (token[1] == '#') { // Якщо є дієз
        note->octave = token[2] - '0';
        note->note += 1;
    } else {
        note->octave = token[1] - '0';
    }
    // print_note(*note);
    // printf("\n");
    // getchar();

    return 1;
}

int
parse_sound(const char *token, sound_t *sound) {
    char *slide = NULL;
    int slides_n = sizeof(slides) / sizeof(slide_settings_t);
    int i;
    slide_function_t slide_function;
    if (!token || !sound) return 0;

    if (token[0] == '_') {
        sound->type = PAUSE;
        return 1;
    }

    for (i = 0; i < slides_n && !slide; ++i) {
        slide = strchr(token, slides[i].symbol);
        slide_function = slides[i].function;
    }

    if (slide) {
        sound->type = SLIDE;
        sound->data.slide.function = slide_function;
        *slide = '\0';
        if (!parse_note(token, &sound->data.slide.start) || !parse_note(slide + 1, &sound->data.slide.end)) {
            return 0;
        }
    } else {
        sound->type = NOTE;
        if (!parse_note(token, &sound->data.note)) {
            return 0;
        }
    }

    return 1;
}

int
parse_melody_file(const char *filename, sound_t **sounds_p, size_t *sound_count) {
    FILE *file = fopen(filename, "r");
    char buffer[256];
    size_t count = 0;
    size_t cap = 10;
    sound_t *sounds;

    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    sounds = malloc(sizeof(sound_t) * cap);

    while (fscanf(file, "%s", buffer) != EOF) {
        if (count >= cap) {
            cap *= 2;
            sounds = realloc(sounds, sizeof(sound_t) * cap);
            if (sounds == NULL) {
                printf("Fault\n");
                return 1;
            }
        }

        if (isdigit(buffer[0])) {
            sounds[count - 1].duration = atoi(buffer);
        } else {
            if (!parse_sound(buffer, &sounds[count])) {
                fprintf(stderr, "Failed to parse sound: %s\n", buffer);
                fclose(file);
                return 1;
            }
            count++;
        }
    }

    *sound_count = count;
    *sounds_p = sounds;
    fclose(file);

    return 0;
}

void
init_speacker(void) {
    outp(TIMER_CONTROL, 0xB6);  // 10110110: Канал 2, режим 3 (генератор меандру)
}

unsigned char
on_speaker(void) {
    speaker_state = inp(SPEAKER_PORT);
    outp(SPEAKER_PORT, speaker_state | 0x03);  // Увімкнути біти 0 і 1
    return speaker_state;
}

void
off_speaker(void) {
    outp(SPEAKER_PORT, speaker_state & ~0x03);  // Вимкнути біти 0 і 1
}

// Функція для встановлення частоти ноти
void
setup_frequency(double frequency) {
    divisor = SYSTEM_CLOCK / frequency;
    outp(TIMER_CHANNEL_2, divisor & 0xFF);        // Молодший байт
    outp(TIMER_CHANNEL_2, (divisor >> 8) & 0xFF); // Старший байт
}


double
convert_note_to_frequency(note_t note) {
    const double a = 1.05946309436;  // 2^(1/12)
    const char base_octave = 4;
    const double base_note_freq = 440.0;  // A4
    const note_declaration_t base_note = A;  // A4

    sound_t sound;
    int delta = note.note - base_note + 12 * (note.octave - base_octave);

    return base_note_freq * pow(a, (double) delta);
}


void
play_note(note_t note, unsigned int duration) {
    setup_frequency(convert_note_to_frequency(note));
    on_speaker();
    delay(duration);
    off_speaker();
}


/**
frequency updates every 55 millisecconds
*/
void
play_slide(note_t start, note_t end, unsigned int duration, slide_function_t slide_function) {
    double start_f, end_f, current_f;
    unsigned long current_time, start_time;

    start_f = convert_note_to_frequency(start);
    end_f = convert_note_to_frequency(end);

    on_speaker();
    start_time = get_time_in_milliseconds();
    current_time = start_time;
    while ((current_time - start_time) < duration) {
        current_f = slide_function(start_f, end_f, ((double)(current_time - start_time) / duration));
        current_time = get_time_in_milliseconds();
        setup_frequency(current_f);
    }
    off_speaker();
}

void
play_sound(sound_t sound) {
    switch (sound.type) {
        case NOTE:
            play_note(sound.data.note, sound.duration);
            break;
        case SLIDE:
            play_slide(sound.data.slide.start, sound.data.slide.end, sound.duration, sound.data.slide.function);
            break;
        case PAUSE:
            delay(sound.duration);
            break;
    }
}


void
play_melody(sound_t *melody, unsigned int length) {
    unsigned int i;
    for (i = 0; i < length; ++i) {
        play_sound(melody[i]);
    }
}


int
main(int argc, char *argv[]) {
    const char *file;
    size_t melody_length;
    sound_t *melody;

    if (argc < 2) {
        printf("Usage: %s <file: *.mel> with melody \n", argv[0]);
        return 1;
    }

    file = argv[1];
    parse_melody_file(file, &melody, &melody_length);
    // print_sounds(melody, melody_length);

    init_speacker();
    play_melody(melody, melody_length);

    free(melody);
    return 0;
}
