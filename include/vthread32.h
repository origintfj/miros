#ifndef VTHREAD32_H
#define VTHREAD32_H

int const vthread32_init(void *const(*thread)(void *const),
                         void *const arg, unsigned const stack_szw,
                         unsigned const kernel_stack_szw);
void vthread32_switch(void);

#endif
