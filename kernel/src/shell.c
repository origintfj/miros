#include <miros.h>

#include <uart.h>

#define BUFFER_SZB      64

int buffer_index;
char buffer[BUFFER_SZB];
int arg_count;
char const *arg_list[BUFFER_SZB];

#define PATH_SZB        256

char path[PATH_SZB];

unsigned const path_next(char str_next[], char const str_path[], unsigned i) {
    unsigned j;
    for (; str_path[i] == '/'; ++i);
    for (j = 0; str_path[i] != '\0' && str_path[i] != '/'; ++j, ++i) {
        str_next[j] = str_path[i];
    }
    str_next[j] = '\0';
    return i;
}
char *const path_strip_last(char str_path[]) {
    int i;
    // remove trailing '/'
    for (i = strlen(str_path) - 1; i > 0 && str_path[i] == '/'; --i);
    for (; i > 0 && str_path[i] != '/'; --i);
    str_path[i++] = '/';
    str_path[i] = '\0';
    return str_path;
}
int const run(int const argc, char const *const *argv) {
    int error = 0;

    if (!strcmp(argv[0], "ls")) {
        if (argc == 1 || argc == 2) {
            fat32_entry_t fat32_entry;
            char temp_path[FAT32_MAX_PATH_LENGTH];

            strcpy(temp_path, path);
            if (argc == 2) {
                if (argv[1][0] == '/') {
                    strcpy(temp_path, argv[1]);
                } else {
                    strcpy(temp_path, path);
                    strcat(temp_path, argv[1]);
                    strcat(temp_path, "/");
                }
            }
            strupr(temp_path, temp_path);

            if (!fs_dir_get(&fat32_entry, temp_path)) {
                printf("%c[0m", 0x1b);
                while (!fs_get_entry(&fat32_entry)) {
                    if (fat32_entry.attribute & FAT32_ENTRY_ATTRIB_DIR) {
                        printf("%c[36m", 0x1b);
                        printf("\nDIR     ");
                    } else {
                        printf("%c[32m", 0x1b);
                        printf("\n        ");
                    }
                    printf("%s%c", strlwr(fat32_entry.short_name, fat32_entry.short_name),
                                   fat32_entry.attribute & FAT32_ENTRY_ATTRIB_DIR ? '/' : ' ');
                    printf("%c[0m", 0x1b);
                }
            } else {
                printf("\nDirectory not found.");
            }
        } else {
            printf("\n%s: Too many arguments", argv[0]);
        }
    } else if (!strcmp(argv[0], "cat")) {
        if (argc == 2) {
            char temp_path[BUFFER_SZB];
            if (strlen(path) + 1 + strlen(argv[1]) < FAT32_MAX_PATH_LENGTH) {
                if (argv[1][0] == '/') {
                    strcpy(temp_path, argv[1]);
                } else {
                    strcpy(temp_path, path);
                    strcat(temp_path, argv[1]);
                }
                strupr(temp_path, temp_path);

                fat32_file_t *ifile = fopen(temp_path);
                if (ifile != NULL) {
                    fseek(ifile, 0, FAT32_SEEK_END);
                    unsigned ifile_len = ftell(ifile);
                    char *const buffer = (char *const)malloc(ifile_len * sizeof(char) + 1);
                    fseek(ifile, 0, FAT32_SEEK_SET);
                    fread(buffer, sizeof(char), ifile_len, ifile);
                    buffer[ifile_len] = '\0';
                    fclose(ifile);
                    printf("\n%s", buffer);
                    free(buffer);
                } else {
                    printf("\nCannot open file '%s'", temp_path);
                }
            } else {
                printf("\n%s: Path too long", argv[0]);
            }
        } else {
            printf("\nUSAGE:\ncat <file name>");
        }
    } else if (!strcmp(argv[0], "cd")) {
        if (argc == 2) {
            unsigned i;
            fat32_entry_t fat32_entry;
            char temp_path[PATH_SZB];
            char dir[PATH_SZB];

            if (argv[1][0] == '/') {
                strcpy(temp_path, "/");
            } else {
                strcpy(temp_path, path);
            }
            for (i = 0; argv[1][i] != '\0'; ) { // TODO - check path doesn't outgrow buffer
                i = path_next(dir, argv[1], i);
                //printf("next='%s'", dir);
                if (!strcmp(dir, "..")) {
                    //printf("before strip='%s'", temp_path);
                    path_strip_last(temp_path);
                    //printf("after strip='%s'", temp_path);
                } else if (strcmp(dir, ".") && strcmp(dir, "")) {
                    strcat(temp_path, dir);
                    strcat(temp_path, "/");
                    //printf("after cat='%s'", temp_path);
                }
            }
            strupr(temp_path, temp_path);

            //printf("\ncd to '%s'", temp_path);
            if (strlen(temp_path) >= PATH_SZB) {
                printf("\n%s: Path too long", argv[0]);
            } else if (!fs_dir_get(&fat32_entry, temp_path)) {
                strlwr(temp_path, temp_path);
                strcpy(path, temp_path);
            } else {
                printf("\n%s: No such directory", argv[0]);
            }
        } else {
            printf("\nUSAGE:\ncd <path>");
        }
    } else if (!strcmp(argv[0], "tot")) {
        unsigned i;
        uint32_t *thread_list;
        unsigned thread_count;

        printf("\nThreads in flight:\n");
        thread_count = vthread_get_all(&thread_list);
        for (i = 0; i < thread_count; ++i) {
            printf("%X\n", thread_list[i]);
        }
    } else if (!strcmp(argv[0], "xd")) {
        if (argc >= 2 && argc <= 3) {
            unsigned i;
            int count;
            uint32_t addr;

            count = 32;
            if (argc == 3) {
                count = atoi(argv[2]);
                if (count < 0) {
                    count = -count;
                }
            }
            addr = (uint32_t const)(xtoi(argv[1]) & -4);
            printf("\nDumping %i word(s) from address 0x%X:\n", count, addr);
            for (i = 0; i < (unsigned const)count; ++i) {
                if ((i & 3) == 0) {
                    printf("\n%X: ", (addr + i * sizeof(uint32_t)));
                }
                printf("%X ", ((uint32_t const *const)addr)[i]);
            }
        } else {
            printf("\nUSAGE:\nxd <hex-address> [ <word-count> ]");
        }
        printf("\n");
    } else if (!strcmp(argv[0], "mav")) {
        if (argc != 1) {
            printf("\nToo many arguments.");
        } else {
            printf("\n%u KiB free.", mavailable() >> 10);
        }
        printf("\n");
    } else if (!strcmp(argv[0], "free")) { // TODO - clean up
        if (argc == 2) {
            uint8_t *const buffer = (uint8_t *const)xtoi(argv[1]);
            
            free(buffer);
        } else {
            printf("\nUSAGE:\nfree <address of container to be freed>\n");
        }
/*
    } else if (!strcmp(argv[0], "upload")) { // TODO - clean up
        if (argc == 2 || argc == 3) {
            unsigned const size = atoi(argv[1]);
            uint8_t *const buffer = (uint8_t *const)malloc(size);
            
            if (buffer == NULL) {
                printf("\nNot enough memory!");
            } else {
                unsigned i;
                uint8_t c;

                printf("\nPaste the ASCII hex image into the terminal...");
                for (i = 0, c = '\0'; c != 'q' && i < (size << 1); ++i) {
                    uint8_t byte;

                    do {
                        c = (uint8_t const)uart_getc();
                    } while ( !( (c == 'q') ||
                                 (c >= '0' && c <= '9') ||
                                 (c >= 'a' && c <= 'f') ||
                                 (c >= 'A' && c <= 'F') ) );
                    if (c == 'q') {
                        i = 2 * size;
                    } else if (c >= '0' && c <= '9') {
                        c = c - '0';
                    } else if (c >= 'a' && c <= 'f') {
                        c = c - 'a' + 10;
                    } else if (c >= 'A' && c <= 'F') {
                        c = c - 'A' + 10;
                    }
                    byte = byte << 4;
                    byte = byte | c;

                    if (i & 1) {
                        buffer[i >> 1] = byte;
                        //printf("%x=%x\n", &(buffer[i >> 1]), byte);
                    }
                }
                if (c == 'q') {
                    printf("ABORTED!\nUpload canceled", buffer);
                    free(buffer);
                } else {
                    printf("done!\nAddress = %X (HEX)", buffer);
                    if (argc == 3) {
                        if (!strcmp(argv[2], "m")) {
                            if (fs_mount(buffer) != NULL) {
                                strcpy(path, "/");
                            } else {
                                strcpy(path, "");
                                printf("\nUnsupported file system type");
                            }
                        } else {
                            printf("\nBad 3rd argument.  Use 'm' to mount image.");
                        }
                    }
                }
            }
        } else {
            printf("\nUSAGE:\nupload <size (bytes)>\nThen wait for prompt.");
        }
        printf("\n");
*/
    } else if (!strcmp(argv[0], "ut")) { // TODO - fix this
        if (argc != 1) {
            printf("\nToo many arguments.");
        } else {
            uint64_t uptime_us;

            uptime_us = get_up_time_us() >> 6;
            printf("\nBROKEN!\n%u days, %u hours, %u minutes, %u seconds",
                   (uint32_t const)(uptime_us / 15625 / 60 / 60 / 24),
                   (uint32_t const)(uptime_us / 15625 / 60 / 60),
                   (uint32_t const)(uptime_us / 15625 / 60),
                   (uint32_t const)(uptime_us / 15625));
        }
        printf("\n");
    } else {
        error = 1;
    }

    return error;
}
int const start(int const argc, char const *const *const argv) {
    char temp_path[FAT32_MAX_PATH_LENGTH];

    strcpy(temp_path, "/BIN/");

    if (strlen(temp_path) + strlen(argv[0]) < FAT32_MAX_PATH_LENGTH) {
        strcat(temp_path, argv[0]);

        fat32_file_t *ifile = fopen(argv[0]);

        if (ifile != NULL) {
            fseek(ifile, 0, FAT32_SEEK_END);
            unsigned ifile_len = ftell(ifile);
            char *const buffer = (char *const)malloc(ifile_len * sizeof(char));
            fseek(ifile, 0, FAT32_SEEK_SET);
            fread(buffer, sizeof(char), ifile_len, ifile);
            buffer[ifile_len] = '\0';
            fclose(ifile);

            //vthread32_create(buffer, NULL, 1024u, 0x1880);

            free(buffer);
        }
    }
}
int const execute(int const argc, char const *const *const argv,
                  char const *const buffer) {
    char *str_buffer;
    void *arg_buffer;
    char const **arg_vector;
    int i;
    int error;

    arg_buffer = malloc(BUFFER_SZB + BUFFER_SZB * sizeof(char *));
    //printf("\nbuffer = %X\n", (uint32_t const)arg_buffer);

    // set the str_buffer pointer to the start of the arg_buffer
    str_buffer = (char *const)arg_buffer;
    // set the arg_vector pointer to that section of the arg_buffer
    arg_vector = (char const **const)((char *const)arg_buffer + BUFFER_SZB);
    // copy the string to the str_buffer
    for (i = 0; buffer[i] != '\0'; ++i) {
        str_buffer[i] = buffer[i];
    }
    str_buffer[i] = '\0';
    // copy the argv to the argv
    for (i = 0; i < argc; ++i) {
        arg_vector[i] = argv[i] + (arg_vector - argv);
    }

    //printf("\nExecuting from the arg list (count = %i)\n", argc);
    for (i = 0; i < argc; ++i) {
        //printf("'%s'\n", argv[i]);
    }
    error = run(argc, argv);
    if (error) {
        start(argc, argv);
    }

    free(arg_buffer);
    return error;
}
/*
#define HISTORY_BUFFER_SZB      1000

char *history_buffer;
unsigned history_head;
unsigned history_tail;

void history_init(void) {
    history_head = 0;
    history_tail = 0;

    history_buffer = (char *const)malloc(HISTORY_BUFFER_SZB);
}
void history_append(char const *const str) {
    unsigned i;

    for (i = 0; str[i] != '\0'; ++i) {
        history_buffer[history_head + i] = str[i];
    }
    history_head += i + 1;
    history_buffer[history_head++] = '\0';
}
void history_print(void) {
    for (i = history_tail; i < ;
}
*/
void print_prompt() {
    printf("shell:");
    printf("%c[36m", 0x1b);
    printf("%s", path);
    printf("%c[0m", 0x1b);
    printf("# ");
}
void *const shell(void *const arg) {
    char c;

    strcpy(path, "/");

    printf("%c[0m", 0x1b);
    printf("\n");
    print_prompt();

    buffer_index = 0;

    while (1) {
        c = uart_getc();
        if (buffer_index < BUFFER_SZB - 1 && c >= 32 && c <= 126) {
            uart_putc(c);
            buffer[buffer_index] = c;
            ++buffer_index;
        } else if (buffer_index < BUFFER_SZB && c == 0x0d) { // enter
            int i;

            buffer[buffer_index] = '\0';

            // populate the arg_list and set arg_count
            for (i = 0, arg_count = 0; buffer[i] != '\0'; ) {
                if (buffer[i] == ' ') {
                    buffer[i] = '\0';
                    ++i;
                    for (; buffer[i] == ' '; ++i);
                }
                if (buffer[i] != '\0') {
                    arg_list[arg_count] = &(buffer[i]);
                    ++arg_count;
                }
                for (; buffer[i] != '\0' && buffer[i] != ' '; ++i);
            }
            if (arg_count > 0) {
                if (execute(arg_count, arg_list, buffer)) {
                    printf("\n%s: command not found", arg_list[0]);
                }
            }

            buffer_index = 0;
            printf("\n");
            print_prompt();
        } else if (buffer_index > 0 && c == 0x08) { // backspace
            --buffer_index;
            buffer[buffer_index] = '\0';
            printf("\r");
            print_prompt();
            printf("%s ", buffer);
            printf("\r");
            print_prompt();
            printf("%s", buffer);
        }
    }

    return NULL;
}
