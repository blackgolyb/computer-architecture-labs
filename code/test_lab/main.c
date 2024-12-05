#include <lab.h>
#include <stdio.h>


void
main () {
    printf("u8: %d\n", sizeof(u8));
    printf("u16: %d\n", sizeof(u16));
    printf("u32: %d\n", sizeof(u32));
    printf("u64: %d\n", sizeof(u64));
    printf("usize: %d\n", sizeof(usize));

    printf("i8: %d\n", sizeof(i8));
    printf("i16: %d\n", sizeof(i16));
    printf("i32: %d\n", sizeof(i32));
    printf("i64: %d\n", sizeof(i64));
    printf("isize: %d\n", sizeof(isize));

    printf("f32: %d\n", sizeof(f32));
    printf("f64: %d\n", sizeof(f64));

    printf("usize_c (size_t): %d\n", sizeof(usize_c));
    printf("isize_c (ptrdiff_t): %d\n", sizeof(isize_c));
}
