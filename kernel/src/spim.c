#include <spim.h>

#include <common.h>

#define SPIM_DATA_OFFSET        0
#define SPIM_STATUS_OFFSET      4
#define SPIM_FLUSH_OFFSET       8
#define SPIM_CFG_OFFSET         12
//
#define SPIM_STATUS_TXE_MASK    0x10
#define SPIM_STATUS_TXF_MASK    0x08
#define SPIM_STATUS_RXE_MASK    0x04
#define SPIM_STATUS_RXF_MASK    0x02
#define SPIM_STATUS_CS_MASK     0x01
#define SPIM_FLUSH_TX_MASK      0x2
#define SPIM_FLUSH_RX_MASK      0x1
#define SPIM_CFG_CPOL_MASK      0x20000
#define SPIM_CFG_CPHA_MASK      0x10000
#define SPIM_CFG_CPHB_MASK      0x0ffff

int const spim_init(uint32_t const dev_base_addr, unsigned const cphb, int const cpol, int const cpha) {
    uint32_t cfg;

    write_reg(dev_base_addr + SPIM_STATUS_OFFSET, ~SPIM_STATUS_CS_MASK);
    cfg  = (cpol ? SPIM_CFG_CPOL_MASK : 0);
    cfg |= (cpha ? SPIM_CFG_CPHA_MASK : 0);
    cfg |= cphb;
    write_reg(dev_base_addr + SPIM_CFG_OFFSET, cfg);

    return (cphb & ~SPIM_CFG_CPHB_MASK ? 1 : 0);
}
int const spim_set_cphb(uint32_t const dev_base_addr, unsigned const cphb) {
    uint32_t cfg;

    cfg  = read_reg(dev_base_addr + SPIM_CFG_OFFSET) & ~SPIM_CFG_CPHB_MASK;
    cfg |= cphb;
    write_reg(dev_base_addr + SPIM_CFG_OFFSET, cfg);

    return (cphb & ~SPIM_CFG_CPHB_MASK ? 1 : 0);
}
void spim_cs_assert(uint32_t const dev_base_addr) {
    write_reg(dev_base_addr + SPIM_STATUS_OFFSET, SPIM_STATUS_CS_MASK);
}
void spim_cs_deassert(uint32_t const dev_base_addr) {
    write_reg(dev_base_addr + SPIM_STATUS_OFFSET, ~SPIM_STATUS_CS_MASK);
}
void spim_block_exchange(uint32_t const dev_base_addr, uint8_t *const dest_buffer,
                         uint8_t const *const src_buffer, unsigned const count) {
    unsigned i, j;

    for (i = 0, j = 0; i < count; ++i) {
        while (read_reg(dev_base_addr + SPIM_STATUS_OFFSET) & SPIM_STATUS_TXF_MASK);
        write_reg(dev_base_addr + SPIM_DATA_OFFSET, src_buffer[i]);

        if (!(read_reg(dev_base_addr + SPIM_STATUS_OFFSET) & SPIM_STATUS_RXE_MASK)) {
            dest_buffer[j] = (uint8_t const)read_reg(dev_base_addr + SPIM_DATA_OFFSET);
            ++j;
        }
    }
    for (; j < count; ++j) {
        while (read_reg(dev_base_addr + SPIM_STATUS_OFFSET) & SPIM_STATUS_RXE_MASK);
        dest_buffer[j] = (uint8_t const)read_reg(dev_base_addr + SPIM_DATA_OFFSET);
    }
}
void spim_block_read(uint32_t const dev_base_addr,
                     uint8_t *const dest_buffer, unsigned const count) {
    unsigned i, j;

    for (i = 0, j = 0; i < count; ++i) {
        while (read_reg(dev_base_addr + SPIM_STATUS_OFFSET) & SPIM_STATUS_TXF_MASK);
        write_reg(dev_base_addr + SPIM_DATA_OFFSET, 0xff);

        if (!(read_reg(dev_base_addr + SPIM_STATUS_OFFSET) & SPIM_STATUS_RXE_MASK)) {
            dest_buffer[j++] = (uint8_t const)read_reg(dev_base_addr + SPIM_DATA_OFFSET);
            ++j;
        }
    }
    for (; j < count; ++j) {
        while (read_reg(dev_base_addr + SPIM_STATUS_OFFSET) & SPIM_STATUS_RXE_MASK);
        dest_buffer[j] = (uint8_t const)read_reg(dev_base_addr + SPIM_DATA_OFFSET);
    }
}
void spim_block_write(uint32_t const dev_base_addr,
                      uint8_t const *const src_buffer, unsigned const count) {
    unsigned i, j;

    for (i = 0, j = 0; i < count; ++i) {
        while (read_reg(dev_base_addr + SPIM_STATUS_OFFSET) & SPIM_STATUS_TXF_MASK);
        write_reg(dev_base_addr + SPIM_DATA_OFFSET, src_buffer[i]);

        if (!(read_reg(dev_base_addr + SPIM_STATUS_OFFSET) & SPIM_STATUS_RXE_MASK)) {
            read_reg(dev_base_addr + SPIM_DATA_OFFSET);
            ++j;
        }
    }
    for (; j < count; ++j) {
        while (read_reg(dev_base_addr + SPIM_STATUS_OFFSET) & SPIM_STATUS_RXE_MASK);
        read_reg(dev_base_addr + SPIM_DATA_OFFSET);
    }
}
uint8_t const spim_byte_exchange(uint32_t const dev_base_addr, uint8_t const tx_data) {
    while (read_reg(dev_base_addr + SPIM_STATUS_OFFSET) & SPIM_STATUS_TXF_MASK);
    write_reg(dev_base_addr + SPIM_DATA_OFFSET, (uint32_t const)tx_data);
    while (read_reg(dev_base_addr + SPIM_STATUS_OFFSET) & SPIM_STATUS_RXE_MASK);
    return (uint8_t const)read_reg(dev_base_addr + SPIM_DATA_OFFSET);
}
