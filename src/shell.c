#include <uart.h>

#define BUFFER_SZB      64

char buffer[BUFFER_SZB];
int buffer_index;

void *arg_buffer;
char *str_buffer;
char const **arg_list;
int arg_count;

#include <stddef.h>
#include <vmem32.h>

void *const malloc(size_t const szb) {
    return vmem32_alloc(szb);
}
void free(void *const buffer) {
    vmem32_free(buffer);
}
void execute(int const argc, char const *const *const argv,
             void *const buffer) {
    int i;

    printf("\nExecuting from the arg list (count = %i)\n", argc);
    for (i = 0; i < argc; ++i) {
        printf("'%s'\n", argv[i]);
    }

    free(buffer);
}

void print_prompt() {
    printf("[]$ ");
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
        if (buffer_index < BUFFER_SZB && c >= 32 && c <= 126) {
            uart_putc(c);
            buffer[buffer_index] = c;
            ++buffer_index;
        } else if (buffer_index < BUFFER_SZB && c == 0x0d) { // enter
            buffer[buffer_index] = '\0';

            // copy the string to the beginning of the arg_buffer
            arg_buffer = malloc(BUFFER_SZB + BUFFER_SZB * sizeof(char *));
            str_buffer = (char *const)arg_buffer;
            for (i = 0; buffer[i] != '\0'; ++i) {
                str_buffer[i] = buffer[i];
            }
            str_buffer[i] = '\0';
            // set the arg_list pointer to that section of the arg_buffer
            arg_list = (char const **const)((char *const)arg_buffer + BUFFER_SZB);
            // populate the arg_list and set arg_count
            for (i = 0, arg_count = 0; str_buffer[i] != '\0'; ) {
                if (str_buffer[i] == ' ') {
                    str_buffer[i] = '\0';
                    ++i;
                    for (; str_buffer[i] == ' '; ++i);
                }
                if (str_buffer[i] != '\0') {
                    arg_list[arg_count] = &(str_buffer[i]);
                    ++arg_count;
                }
                for (; str_buffer[i] != '\0' && str_buffer[i] != ' '; ++i);
            }
            if (arg_count > 0) {
                execute(arg_count, arg_list, arg_buffer);
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
