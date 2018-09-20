#include <stddef.h>
#include <uart.h>

#define BUFFER_SZB      64

char buffer[BUFFER_SZB];
int buffer_index;

char *arg_list[BUFFER_SZB];
int arg_count;

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

            // TODO - copy the buffer before passing argc and argv
            for (i = 0, arg_count = 0; buffer[i] != '\0'; ) {
                arg_list[arg_count] = &(buffer[i]);
                ++arg_count;
                for (; buffer[i] != '\0' && buffer[i] != ' '; ++i);
                if (buffer[i] == ' ') {
                    buffer[i] = '\0';
                    ++i;
                    for (; buffer[i] == ' '; ++i);
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
