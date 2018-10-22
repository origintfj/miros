#include <soc.h>

#include <stddef.h>

soc_device_t soc_device_table[] = {
    { "SRAM",     SRAM_BASE_ADDR,   0x40000 },
    { "UART",     UART_BASE_ADDR,   0 },
    { "TIMER",    TIMER0_BASE_ADDR, 0 },
    { "SPIM_SD",  SPIM_BASE_ADDR,   0 },
    { "",         0,                0 }
};
