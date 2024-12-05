#ifndef LAB_TYPES_H
#define LAB_TYPES_H

#include <stddef.h> // Для size_t і ptrdiff_t

typedef unsigned char bool;
#define true 1
#define false 0

#define loop while (1)

// Аналоги для беззнакових цілих чисел
typedef unsigned char u8;       // Аналог Rust `u8`
typedef unsigned short u16;     // Аналог Rust `u16`

// Визначаємо `u32`, враховуючи можливі обмеження компілятора
#if sizeof(int) == 4
typedef unsigned int u32;       // Використовуємо `int` для 32-бітного числа
#else
typedef unsigned long u32;      // Використовуємо `long` для 32-бітного числа
#endif

// Визначаємо `u64`, враховуючи можливі обмеження компілятора
#if defined(__BORLANDC__) || (sizeof(long long) < 8)
typedef struct {
    unsigned long low;          // Нижні 32 біти
    unsigned long high;         // Верхні 32 біти
} u64;                          // Емуляція 64-бітного числа
#else
typedef unsigned long long u64; // Використовуємо `long long`, якщо доступно
#endif

// Визначаємо `usize`
typedef size_t usize;

// Аналоги для підписаних цілих чисел
typedef signed char i8;         // Аналог Rust `i8`
typedef signed short i16;       // Аналог Rust `i16`

// Визначаємо `i32`, враховуючи можливі обмеження компілятора
#if sizeof(int) == 4
typedef signed int i32;         // Використовуємо `int` для 32-бітного числа
#else
typedef signed long i32;        // Використовуємо `long` для 32-бітного числа
#endif

// Визначаємо `i64`, враховуючи можливі обмеження компілятора
#if defined(__BORLANDC__) || (sizeof(long long) < 8)
typedef struct {
    signed long low;            // Нижні 32 біти
    signed long high;           // Верхні 32 біти
} i64;                          // Емуляція 64-бітного числа
#else
typedef signed long long i64;   // Використовуємо `long long`, якщо доступно
#endif

// Визначаємо `isize`
typedef ptrdiff_t isize;

// Аналоги для чисел із рухомою комою
typedef float f32;              // Аналог Rust `f32`
typedef double f64;             // Аналог Rust `f64`

// Додаткові типи для адрес і розмірів
typedef size_t usize_c;         // Аналог `usize` через стандартний `size_t`
typedef ptrdiff_t isize_c;      // Аналог `isize` через стандартний `ptrdiff_t`

#endif // LAB_TYPES_H
