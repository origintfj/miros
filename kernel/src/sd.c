#include <sd.h>

#include <spim.h>
#include <vmem32.h>

#define SD_POSTAMBLE_SZB    10//8

#define SD_CMD_SZB          6

//uint8_t const sd__cmd01[] = { 0x40 |  1, 0x00, 0x00, 0x00, 0x00, 0x00 | 0x01 }; // TODO - use
//uint8_t const sd__cmd17[] = { 0x40 | 17, 0x00, 0x00, 0x00, 0x00, 0xff | 0x01 };

#include <uart.h>

static void sd_send_dummy_bytes(uint32_t const spim_base_addr, unsigned const n) {
    unsigned i;

    for (i = 0; i < n; ++i) {
        spim_byte_exchange(spim_base_addr, 0xff);
    }
}
static uint32_t const sd_get_r1(uint32_t const spim_base_addr, unsigned const try_count) {
    unsigned i;
    uint8_t rx_data;

    for (i = 0; (try_count == 0 || i < try_count) &&
                (rx_data = spim_byte_exchange(spim_base_addr, 0xff)) & 0x80; ++i);
    return (uint32_t const)rx_data;
}
/*
static uint32_t const sd_get_r3(uint32_t const spim_base_addr, unsigned const try_count,
                                uint32_t *const r3_response) {
    unsigned i;
    uint8_t rx_data;
    uint32_t r1_response;

    for (i = 0; (try_count == 0 || i < try_count) &&
                (rx_data = spim_byte_exchange(spim_base_addr, 0xff)) & 0x80; ++i);
    r1_response = (uint32_t const)rx_data;
    *r3_response  = spim_byte_exchange(spim_base_addr, 0xff) << 24;
    *r3_response |= spim_byte_exchange(spim_base_addr, 0xff) << 16;
    *r3_response |= spim_byte_exchange(spim_base_addr, 0xff) <<  8;
    *r3_response |= spim_byte_exchange(spim_base_addr, 0xff) <<  0;
    return r1_response;
}
static uint32_t const sd_send__cmd_r1(uint32_t const spim_base_addr, uint8_t const *const cmd) {
    uint32_t r1;

    sd_send_dummy_bytes(spim_base_addr, 1);
    spim_cs_assert(spim_base_addr);
    spim_block_write(spim_base_addr, cmd, SD_CMD_SZB);
    r1 = sd_get_r1(spim_base_addr, 0);
    spim_cs_deassert(spim_base_addr);
    sd_send_dummy_bytes(spim_base_addr, SD_POSTAMBLE_SZB);

    return r1;
}
static uint32_t const sd_send__cmd_r3(uint32_t const spim_base_addr, uint8_t const *const cmd,
                                      uint32_t *const rsp) {
    uint32_t r1;

    sd_send_dummy_bytes(spim_base_addr, 1);
    spim_cs_assert(spim_base_addr);
    spim_block_write(spim_base_addr, cmd, SD_CMD_SZB);
    r1 = sd_get_r3(spim_base_addr, 0, rsp);
    spim_cs_deassert(spim_base_addr);
    sd_send_dummy_bytes(spim_base_addr, SD_POSTAMBLE_SZB);

    return r1;
}
*/
static uint8_t const sd_send_cmd(uint32_t const spim_base_addr,
                                 uint8_t const cmd, uint32_t const arg, uint8_t const crc,
                                 void *const rsp, size_t const rsp_szb) {
    sd_send_dummy_bytes(spim_base_addr, 1);

    spim_cs_assert(spim_base_addr);

    spim_byte_exchange(spim_base_addr, 0x40 | cmd);
    spim_byte_exchange(spim_base_addr, (uint8_t const)(arg >> 24));
    spim_byte_exchange(spim_base_addr, (uint8_t const)(arg >> 16));
    spim_byte_exchange(spim_base_addr, (uint8_t const)(arg >>  8));
    spim_byte_exchange(spim_base_addr, (uint8_t const)(arg >>  0));
    spim_byte_exchange(spim_base_addr, 0x01 | crc);

    uint8_t rsp0;
    while ((rsp0 = spim_byte_exchange(spim_base_addr, 0xff)) & 0x80); // TODO timeout?

    unsigned i;
    for (i = 0; i < (unsigned const)rsp_szb; ++i) {
        ((uint8_t *const)rsp)[i] = spim_byte_exchange(spim_base_addr, 0xff);
    }

    spim_cs_deassert(spim_base_addr);

    sd_send_dummy_bytes(spim_base_addr, SD_POSTAMBLE_SZB);

    return rsp0;
}
static int const sd_init(uint32_t const spim_base_addr, unsigned const cphb_slow, unsigned const cphb_fast) {
    uint8_t r1;
    uint32_t vinfo, ocr;

    //printf("SD Card init:\n", r1);
    spim_init(spim_base_addr, cphb_slow, 0, 0);

    sd_send_dummy_bytes(spim_base_addr, 20);

    r1 = sd_send_cmd(spim_base_addr, 0, 0x00000000, 0x95, NULL, 0);
    r1 = sd_send_cmd(spim_base_addr, 8, 0x000001aa, 0x87, &vinfo, 4);
    if (r1 == 0x01) { // version 2 card
    } else { // version 1 card ?
    }
    r1 = sd_send_cmd(spim_base_addr, 58, 0x00000000, 0x00, &ocr, 4);
    //printf(" CMD58: R1=%X, OCR=0x%X\n", r1, ocr);

    for (r1 = 1; r1 != 0; ) {
        r1 = sd_send_cmd(spim_base_addr, 55, 0x00000000, 0x00, NULL, 0);
        r1 = sd_send_cmd(spim_base_addr, 41, 0x41000000, 0x00, NULL, 0);
    }

    r1 = sd_send_cmd(spim_base_addr, 58, 0x00000000, 0x00, &ocr, 4);
    //printf(" CMD58: R1=%X, OCR=0x%X\n", r1, ocr);

    spim_set_cphb(spim_base_addr, cphb_fast);

    return 0;
}
static int const sd_block_read(uint32_t const spim_base_addr, void *const buffer, uint32_t const sector) {
    uint8_t rx_byte;
    unsigned i;

    sd_send_dummy_bytes(spim_base_addr, 1);
    spim_cs_assert(spim_base_addr);
    spim_byte_exchange(spim_base_addr, 0x40 | 17);
    spim_byte_exchange(spim_base_addr, sector >> 24);
    spim_byte_exchange(spim_base_addr, sector >> 16);
    spim_byte_exchange(spim_base_addr, sector >>  8);
    spim_byte_exchange(spim_base_addr, sector >>  0);
    spim_byte_exchange(spim_base_addr, 0xff | 0x01);

    while ((rx_byte = spim_byte_exchange(spim_base_addr, 0xff)) & 0x80); // TODO timeout?

    if (rx_byte) {
        return 1;
    }

    do {
        rx_byte = spim_byte_exchange(spim_base_addr, 0xff);
    } while (rx_byte != 0xfe);

    for (i = 0; i < 512; ++i) {
        ((uint8_t *const)buffer)[i] = spim_byte_exchange(spim_base_addr, 0xff);
    }

    spim_cs_deassert(spim_base_addr);
    sd_send_dummy_bytes(spim_base_addr, SD_POSTAMBLE_SZB);

    return 0;
}

struct sd_context {
    uint32_t spim_base_addr;
    uint32_t buffered_sector;
    uint8_t *sector_buffer;
    uint64_t cursor;
};

sd_context_t *const sd_context_create(uint32_t const spim_base_addr, uint32_t const clk_freq_hz) {
    int error;

    error = sd_init(spim_base_addr, clk_freq_hz / 333000 / 2, clk_freq_hz / 25000000 / 2);

    sd_context_t *const sd_context = (sd_context_t *const)vmem32_alloc(sizeof(sd_context_t));
    if (sd_context == NULL) {
        return NULL;
    }
    sd_context->sector_buffer = (uint8_t *const)vmem32_alloc(512 * sizeof(uint8_t));
    if (sd_context->sector_buffer == NULL) {
        return NULL;
    }
    sd_context->spim_base_addr = spim_base_addr;
    sd_context->cursor = 0;

    sd_context->buffered_sector = 0;
    error |= sd_block_read(spim_base_addr, sd_context->sector_buffer, sd_context->buffered_sector);
    if (error) {
        vmem32_free(sd_context->sector_buffer);
        vmem32_free(sd_context);
        return NULL;
    }

    return sd_context;
}
int const sd_context_destroy(sd_context_t *const sd_context) {
    if (sd_context == NULL) {
        return 1;
    }

    vmem32_free(sd_context->sector_buffer);
    vmem32_free(sd_context);

    return 0;
}
int const sd_seek(sd_context_t *const sd_context, uint64_t const offset, int const origin_id) {
    if (origin_id == SD_SEEK_CUR) {
        sd_context->cursor = sd_context->cursor + offset;
    } else if (origin_id == SD_SEEK_SET) {
        sd_context->cursor = offset;
    } else {
        return 1;
    }
    return 0;
}
uint64_t const sd_tell(sd_context_t *const sd_context) {
    return sd_context->cursor;
}
size_t const sd_read(void *const buffer, size_t const size,
                     size_t const count, sd_context_t *const sd_context) {
    uint32_t sector;
    uint32_t offset;

    sector = (uint32_t const)(sd_context->cursor >> 9);
    offset = (uint32_t const)(sd_context->cursor & 0x1ff);

    unsigned i;
    for (i = 0; i < (unsigned const)size * (unsigned const)count; ) {
        if (sector != sd_context->buffered_sector) {
            sd_block_read(sd_context->spim_base_addr, sd_context->sector_buffer, sector);
            sd_context->buffered_sector = sector;
        }
        for (; offset < 512 && i < (unsigned const)size * (unsigned const)count; ++i, ++offset) {
            ((uint8_t *const)buffer)[i] = sd_context->sector_buffer[offset];
        }
        if (offset == 512) {
            ++sector;
            offset = 0;
        }
    }

    sd_context->cursor = ((uint64_t const)sector <<     9)
                       | ((uint64_t const)offset  & 0x1ff)
                       ;

    return (size_t const)count;
}
