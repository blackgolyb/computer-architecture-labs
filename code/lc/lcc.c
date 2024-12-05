#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>

#define COMMAND_FMT "bcc \
-IC:\\PROGRA~1\\BC\\INCLUDE \
-LC:\\PROGRA~1\\BC\\LIB \
%s"

int
main(int argc, char *argv[]) {
    const char *filename;
    const char *command_fmt;
    char command[256];


    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    filename = argv[1];

    sprintf(command, COMMAND_FMT, filename);


    // printf("%s", command);
    system(command);
    return 0;
}
