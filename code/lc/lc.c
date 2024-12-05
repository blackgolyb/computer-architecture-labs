#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>

#define COMMAND_FMT "bcc \
-IC:\\PROGRA~1\\BC\\INCLUDE \
-LC:\\PROGRA~1\\BC\\LIB \
-nC:\\CODE\\OUT \
-e%s \
C:\\CODE\\SRC\\%s\\%s"

#define MAIN_FILE "main.c"


int
main(int argc, char *argv[]) {
    const char *project_name;
    const char *command_fmt;
    char *main_file;
    char command[256];

    main_file = MAIN_FILE;

    if (argc < 2) {
        printf("Usage: %s <project_name> with %s as main file\n", argv[0], MAIN_FILE);
        printf("Usage: %s <project_name> <file_name>\n", argv[0]);
        return 1;
    } else if (argc == 3) {
        main_file = argv[2];
    }

    project_name = argv[1];

    sprintf(command, COMMAND_FMT, project_name, project_name, main_file);


    // printf("%s", command);
    system(command);
    return 0;
}
