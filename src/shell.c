#include <miros.h>

#include <uart.h>
#include <fat32.h>

#define BUFFER_SZB      64

int buffer_index;
char buffer[BUFFER_SZB];
int arg_count;
char const *arg_list[BUFFER_SZB];

int const run(int const argc, char const *const *argv) {
    int error = 0;

    if (strcmp(argv[0], "tot")) {
        unsigned i;
        uint32_t *thread_list;
        unsigned thread_count;

        printf("\nThreads in flight:\n");
        thread_count = vthread_get_all(&thread_list);
        for (i = 0; i < thread_count; ++i) {
            printf("%X\n", thread_list[i]);
        }
    } else if (strcmp(argv[0], "xd")) {
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
            printf("\nUSAGE:\nxd <hex-address> [ <word-count> ].");
        }
        printf("\n");
    } else if (strcmp(argv[0], "mav")) {
        if (argc != 1) {
            printf("\nToo many arguments.");
        } else {
            printf("\n%u KiB free.", mavailable() >> 10);
        }
        printf("\n");
    } else if (strcmp(argv[0], "ut")) {
        if (argc != 1) {
            printf("\nToo many arguments.");
        } else { // TODO - fix this
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
    printf("# ");
}
void *const shell(void *const arg) {
    char c;
    int i;

    printf("\n");
    print_prompt();
    uart_flush_rx();

    buffer_index = 0;

    while (1) {
        c = uart_getc();
        if (buffer_index < BUFFER_SZB - 1 && c >= 32 && c <= 126) {
            uart_putc(c);
            buffer[buffer_index] = c;
            ++buffer_index;
        } else if (buffer_index < BUFFER_SZB && c == 0x0d) { // enter
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
            uart_flush_rx();
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
