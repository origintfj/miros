#ifndef SOC_H
#define SOC_H

#define CLK_FREQ                    25000000 // 25 MHz

#define ROM_BASE_ADDR               0x00000000
#define SRAM_BASE_ADDR              0x40000000
#define SRAM_END_ADDR               0x40040000

#define UART_BASE_ADDR              0x80000000
#define UART_DATA_OFFSET            0
#define UART_CPHB_OFFSET            4
#define UART_STATUS_OFFSET          8
#define UART_INT_EN_OFFSET          12
#define UART_INT_STATE_OFFSET       16
//
#define UART_STATUS_RXDAV_MASK      8
#define UART_STATUS_RXFULL_MASK     4
#define UART_STATUS_TXBUSY_MASK     2
#define UART_STATUS_TXFULL_MASK     1

#define GPIO_BASE_ADDR              0x90000000
#define GPIO_IO_OFFSET              0
#define GPIO_DIR_OFFSET             4

#define TIMER0_BASE_ADDR            0xA0000000
#define TIMER0_TIME_OFFSET          0
#define TIMER0_MAX_OFFSET           4
#define TIMER0_CTRL_OFFSET          8
#define TIMER0_INT_OFFSET           12
//
#define TIMER0_CTRL_INT_EN_MASK     2
#define TIMER0_CTRL_EN_MASK         1

#endif
