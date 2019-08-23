#ifndef SOC_H
#define SOC_H

#define CLK_FREQ                    25000000 // 25 MHz

#define ROM_BASE_ADDR               0x00000000
#define SRAM_BASE_ADDR              0x40000000
#define SRAM_END_ADDR               0x40040000

#define UART_BASE_ADDR              0x80000000

#define GPIO_BASE_ADDR              0x90000000
#define GPIO_IO_OFFSET              0
#define GPIO_DIR_OFFSET             4

#define TIMER0_BASE_ADDR            0xA0000000
#define SPIM_BASE_ADDR              0xC0000000

#endif
