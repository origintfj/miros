#include <miros.h>

#include <uart.h>

#define ARG_BUFFER_SZB      256

int arg_buffer_index;
char arg_buffer_temp[ARG_BUFFER_SZB];
char arg_buffer[ARG_BUFFER_SZB];
int arg_count;
char arg0_buffer[ARG_BUFFER_SZB]; // stores strcat of path and argv[0]
char const *arg_list[ARG_BUFFER_SZB];

#define PATH_SZB            256

char path[PATH_SZB];

#define HIS_BUFFER_SZB      1024

unsigned history_next_id;
unsigned history_entries;
char history_buffer[HIS_BUFFER_SZB];
unsigned history_buffer_rd_index;
unsigned history_buffer_wr_index;

void history_init(void) {
    history_next_id = 1;
    history_entries = 0;

    history_buffer_rd_index = 0;
    history_buffer_wr_index = 0;
}
void history_print(void) {
    unsigned entry_number = 0;
    int next_entry = 1;
    int i = 0;
    //printf("entry count = %u\n", history_entries);
    //printf("history_buffer_rd_index = %u\n", history_buffer_rd_index);
    //printf("history_buffer_wr_index = %u\n", history_buffer_wr_index);
    for (i = history_buffer_rd_index;
         i != history_buffer_wr_index;
         i = (i == HIS_BUFFER_SZB - 1 ? i = 0 : i + 1)) {

        if (next_entry && entry_number <= history_entries) {
            printf(" %u ", history_next_id - history_entries + entry_number);
            ++entry_number;
            next_entry = 0;
        }

        if (history_buffer[i] == '\0') {
            printf("\n");
            next_entry = 1;
        } else {
            printf("%c", history_buffer[i]);
        }
    }
}
void history_fix_pointers(void) {
    if (history_buffer_wr_index == HIS_BUFFER_SZB) {
        history_buffer_wr_index = 0;
    }
    if (history_buffer_wr_index == history_buffer_rd_index) {
        if (history_buffer[history_buffer_rd_index] == '\0') {
            --history_entries;
        }
        ++history_buffer_rd_index;
        if (history_buffer_rd_index == HIS_BUFFER_SZB) {
            history_buffer_rd_index = 0;
        }

        while (history_buffer[history_buffer_rd_index] != '\0') {
            ++history_buffer_rd_index;
            if (history_buffer_rd_index == HIS_BUFFER_SZB) {
                history_buffer_rd_index = 0;
            }
        }
        --history_entries;
        ++history_buffer_rd_index;
        if (history_buffer_rd_index == HIS_BUFFER_SZB) {
            history_buffer_rd_index = 0;
        }
    }
}
void history_append(int const argc, char const *const *const argv) {
    int i, j;
    for (i = 0; i < argc; ++i) {
        for (j = 0; argv[i][j] != '\0'; ++j) {
            history_buffer[history_buffer_wr_index++] = argv[i][j];
            history_fix_pointers();
        }
        if (i < argc - 1) {
            history_buffer[history_buffer_wr_index++] = ' ';
            history_fix_pointers();
        }
    }
    history_buffer[history_buffer_wr_index++] = '\0';
    history_fix_pointers();
    ++history_next_id;
    ++history_entries;
}
char const *const history_get_entry_at(int const i) {
    return &history_buffer[i];
}
int const history_get_last(void) {
    if (history_buffer_wr_index == history_buffer_rd_index) {
        return -1;
    }

    int i = history_buffer_wr_index - 2;
    for (; i < 0; i += HIS_BUFFER_SZB);
    for (; i != history_buffer_rd_index && history_buffer[i] != '\0';
           i = (i == 0 ? HIS_BUFFER_SZB - 1 : i - 1));

    if (history_buffer_rd_index == i) {
        return i;
    }

    ++i;
    if (i == HIS_BUFFER_SZB) {
        i = 0;
    }

    return i;
}
int const history_get_previous(int const current) {
    if (current == history_buffer_rd_index) {
        return current;
    }

    int i = current - 2;
    for (; i < 0; i += HIS_BUFFER_SZB);
    if (history_buffer_rd_index == i) {
        return i;
    }

    for (; i != history_buffer_rd_index && history_buffer[i] != '\0';
           i = (i == 0 ? HIS_BUFFER_SZB - 1 : i - 1));

    if (history_buffer_rd_index == i) {
        return i;
    }

    ++i;
    if (i == HIS_BUFFER_SZB) {
        i = 0;
    }

    return i;
}
int const history_get_next(int const current) {
    int i;
    for (i = current; i != history_buffer_wr_index && history_buffer[i] != '\0';
                      i = (i == HIS_BUFFER_SZB - 1 ? 0 : i + 1));

    if (i == history_buffer_wr_index) {
        return current;
    }

    ++i;
    if (i == HIS_BUFFER_SZB) {
        i = 0;
    }

    if (i == history_buffer_wr_index) {
        return current;
    }

    return i;
}
int const history_is_empty() {
    return history_buffer_wr_index == history_buffer_rd_index;
}

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
int const execute(int const argc, char const *const *const argv) {
    int status = 0;

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
                }
                strcat(temp_path, "/");
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
    } else if (!strcmp(argv[0], "history")) {
        printf("\n");
        history_print();
    } else if (!strcmp(argv[0], "p")) {
        int his = history_get_last();
        do {
            printf("\n%u '%s'\n", his, history_get_entry_at(his));
        } while ((his = history_get_previous(his)) != -1);
    } else if (!strcmp(argv[0], "exit!")) {
        status = -1;
    } else {
        status = 1;
    }

    return status;
}
int const run(char *const app, int const argc, char const *const *argv) {
    uint64_t proc_id = proc_start(app, argc, argv);

    if (proc_id == 0) {
        return 1;
    }

    void *rtn_val;
    if (vthread_join(proc_id, &rtn_val)) {
        printf("ERROR!");
    } else {
        //printf("\nExit status (%i)", (int const)rtn_val);
    }

    return 0;
}
void print_prompt() {
    printf("shell:");
    printf("%c[36m", 0x1b);
    printf("%s", path);
    printf("%c[0m", 0x1b);
    printf("# ");
}
void *const shell(void *const arg) {
//int const main(int const argc, char const *const argv) {
    char c;

    history_init();
    int history_index = -1;

    strcpy(path, "/");

    printf("%c[0m", 0x1b);
    printf("\n");
    print_prompt();

    arg_buffer_index = 0;

    int exit = 0;
    while (!exit) {
        c = uart_getc();
        if (arg_buffer_index < ARG_BUFFER_SZB - 1 && c >= 32 && c <= 126) {
            uart_putc(c);
            arg_buffer[arg_buffer_index] = c;
            ++arg_buffer_index;
        } else if (arg_buffer_index < ARG_BUFFER_SZB && c == 0x1b) { // escape
            c = uart_getc();
            if (c == '[') {
                c = uart_getc();
                if (c == 'A') { // up
                    if (!history_is_empty() && history_index < 0) {
                        history_index = history_get_last();
                    }
                    if (history_index >= 0) {
                        printf("%c[2K\r", 0x1b);
                        print_prompt();
                        strcpy(arg_buffer, history_get_entry_at(history_index));
                        arg_buffer_index = strlen(arg_buffer);
                        history_index = history_get_previous(history_index);
                        printf("%s", arg_buffer);
                    }
                } else if (c == 'B') { // down
                    if (history_index >= 0) {
                        history_index = history_get_next(history_index);
                        printf("%c[2K\r", 0x1b);
                        print_prompt();
                        strcpy(arg_buffer, history_get_entry_at(history_index));
                        arg_buffer_index = strlen(arg_buffer);
                        printf("%s", arg_buffer);
                    }
                } else if (c == 'D') { // left
/*
                    printf("%c[2K\r", 0x1b);
                    print_prompt();
                    strcpy(arg_buffer_temp, arg_buffer);
                    arg_buffer_temp[arg_buffer_index - 1] = '\0';
                    printf("%s", arg_buffer_temp);
                    printf("%c[D", 0x1b);
*/
                } else if (c == 'C') { // right
/*
                    printf("%c[2K\r", 0x1b);
                    print_prompt();
                    printf("%c[C", 0x1b);
*/
                }
            }
        } else if (arg_buffer_index < ARG_BUFFER_SZB && c == 0x0d) { // enter
            history_index = -1;
            arg_buffer[arg_buffer_index] = '\0';

            // populate the arg_list and set arg_count
            int i;
            for (i = 0, arg_count = 0; arg_buffer[i] != '\0'; ) {
                if (arg_buffer[i] == ' ') {
                    arg_buffer[i] = '\0';
                    ++i;
                    for (; arg_buffer[i] == ' '; ++i);
                }
                if (arg_buffer[i] != '\0') {
                    arg_list[arg_count] = &(arg_buffer[i]);
                    ++arg_count;
                }
                for (; arg_buffer[i] != '\0' && arg_buffer[i] != ' '; ++i);
            }
            if (arg_count > 0) {
                int status;
                char const *arg0_temp;
                status = execute(arg_count, arg_list);
                if (status == -1) {
                    exit = 1;
                } else if (status > 0) {
                    char const *const app_name = arg_list[0];
                    char app_path[PATH_SZB + ARG_BUFFER_SZB];
                    strcpy(app_path, "/bin/");
                    strcat(app_path, app_name);
                    strcpy(arg0_buffer, path);
                    strcat(arg0_buffer, app_name);
                    arg0_temp = arg_list[0];
                    arg_list[0] = arg0_buffer;
                    //printf("\n'%s', '%s', '%s'", app_path, app_name, arg0_buffer);
                    strupr(app_path, app_path);
                    status = run(app_path, arg_count, arg_list);
                    if (status) {
                        printf("\n%s: command not found", app_name);
                    }
                }
                arg_list[0] = arg0_temp;
                if (status == 0) {
                    history_append(arg_count, arg_list);
                }
            }

            arg_buffer_index = 0;
            printf("\n");
            print_prompt();
        } else if (arg_buffer_index > 0 && c == 0x08) { // backspace
            --arg_buffer_index;
            arg_buffer[arg_buffer_index] = '\0';
            printf("\r");
            print_prompt();
            printf("%s ", arg_buffer);
            printf("\r");
            print_prompt();
            printf("%s", arg_buffer);
        }
    }

    //return 0;
    return NULL;
}
