#include <uart.h>

#define BUFFER_SZB      64

int buffer_index;
char buffer[BUFFER_SZB];
int arg_count;
char const *arg_list[BUFFER_SZB];


#include <stddef.h>
#include <vmem32.h>

void *const malloc(size_t const szb) {
    sysjob_t volatile sysjob_malloc;
    uint32_t size_bytes;

    size_bytes = (uint32_t const)szb;

    sysjob_malloc->job_id = 0;
    sysjob_malloc->complete = 0;
    sysjob_malloc->input  = &size_bytes;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&sysjob_malloc) : "a0", "memory");
    while (sysjob_malloc->complete == 0);

    return sysjob_malloc->output;
}
void free(void *const buffer) {
    vmem32_free(buffer);
}
int const execute(int const argc, char const *const *const argv,
                  char const *const buffer) {
    char *str_buffer;
    void *arg_buffer;
    char const **arg_vector;
    int i;

    arg_buffer = malloc(BUFFER_SZB + BUFFER_SZB * sizeof(char *));
    //printf("buffer = %X\n", (uint32_t const)arg_buffer);

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

    // printf("\nExecuting from the arg list (count = %i)\n", argc);
    for (i = 0; i < argc; ++i) {
        printf("'%s'\n", argv[i]);
    }

    free(arg_buffer);
    return 1;
}

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
