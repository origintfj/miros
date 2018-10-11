#include <sd.h>

#include <spim.h>
#include <vmem32.h>

#define SD_POSTAMBLE_SZB    10//8

#define SD_CMD_SZB          6

uint8_t const sd__cmd00[] = { 0x40 |  0, 0x00, 0x00, 0x00, 0x00, 0x95 | 0x01 };
uint8_t const sd__cmd08[] = { 0x40 |  8, 0x00, 0x00, 0x01, 0xaa, 0x87 | 0x01 };
uint8_t const sd__cmd58[] = { 0x40 | 58, 0x00, 0x00, 0x00, 0x00, 0x00 | 0x01 };
uint8_t const sd__cmd01[] = { 0x40 |  1, 0x00, 0x00, 0x00, 0x00, 0x00 | 0x01 }; // TODO - use

uint8_t const sd__cmd55[] = { 0x40 | 55, 0x00, 0x00, 0x00, 0x00, 0x00 | 0x01 };
uint8_t const sd_acmd41[] = { 0x40 | 41, 0x41, 0x00, 0x00, 0x00, 0x00 | 0x01 };

uint8_t const sd__cmd17[] = { 0x40 | 17, 0x00, 0x00, 0x00, 0x00, 0xff | 0x01 };

#include <uart.h>

static void sd_send_dummy_bytes(uint32_t const spim_base_addr, unsigned const n) {
    unsigned i;

    for (i = 0; i < n; ++i) {
        spim_byte_exchange(spim_base_addr, 0xff);
    }
}
static void sd_send_cmd(uint32_t const spim_base_addr, uint8_t const *const cmd) {
    spim_block_write(spim_base_addr, cmd, SD_CMD_SZB);
}
static uint32_t const sd_get_r1(uint32_t const spim_base_addr, unsigned const try_count) {
    unsigned i;
    uint8_t rx_data;

    for (i = 0; (try_count == 0 || i < try_count) &&
                (rx_data = spim_byte_exchange(spim_base_addr, 0xff)) & 0x80; ++i);
    return (uint32_t const)rx_data;
}
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
//
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
static uint32_t const sd_send__cmd_r3(uint32_t const spim_base_addr, uint8_t const *const cmd, uint32_t *const rsp) {
    uint32_t r1;

    sd_send_dummy_bytes(spim_base_addr, 1);
    spim_cs_assert(spim_base_addr);
    spim_block_write(spim_base_addr, cmd, SD_CMD_SZB);
    r1 = sd_get_r3(spim_base_addr, 0, rsp);
    spim_cs_deassert(spim_base_addr);
    sd_send_dummy_bytes(spim_base_addr, SD_POSTAMBLE_SZB);

    return r1;
}
static int const sd_init(uint32_t const spim_base_addr, unsigned const cphb) {
    uint32_t r1, r3;
    unsigned i;

    spim_init(spim_base_addr, cphb, 0, 0);

    sd_send_dummy_bytes(spim_base_addr, 20);
    r1 = sd_send__cmd_r1(spim_base_addr, sd__cmd00);
    printf("CMD00: r1=0x%X\n", r1);
    r1 = sd_send__cmd_r3(spim_base_addr, sd__cmd08, &r3);
    printf("CMD08: r1=0x%X, r3=0x%X\n", r1, r3);
    if (r1 == 0x01) { // version 2 card
    } else { // version 1 card ?
    }
    r1 = sd_send__cmd_r3(spim_base_addr, sd__cmd58, &r3);
    printf("CMD58: r1=0x%X, r3=0x%X\n", r1, r3);

/*
    r1 = sd_send_acmd_r3(spim_base_addr, sd_acmd41, &r3);
    printf("CMD58: r1=0x%X, r3=0x%X\n", r1, r3);
*/

    for (r1 = 1; r1 != 0; ) {
        r1 = sd_send__cmd_r1(spim_base_addr, sd__cmd55);
        printf("CMD55: r1=0x%X\n", r1);
        r1 = sd_send__cmd_r3(spim_base_addr, sd_acmd41, &r3);
        printf("ACMD41: r1=0x%X, r3=0x%X\n", r1, r3);
    }

    return 0;
}
static int const sd_block_read(uint32_t const spim_base_addr, void *const buffer, uint32_t const sector) {
    uint32_t r1;
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
    r1 = sd_get_r1(spim_base_addr, 0);

    if (r1) {
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

    error = sd_init(spim_base_addr, clk_freq_hz / 333000);

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
//printf("outer loop\n");
        if (sector != sd_context->buffered_sector) {
//printf("block read\n");
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


#include <soc.h>

uint32_t *sd_buffer;

uint32_t const spim_base_addr = SPIM_BASE_ADDR;

void sd_test() {
    size_t bytes_read;

    sd_context_t *const sd_context = sd_context_create(SPIM_BASE_ADDR, CLK_FREQ);
    printf("sd_context=%X\n", (uint32_t const)sd_context);

    sd_buffer = vmem32_alloc(4024);
    sd_seek(sd_context, 510, SD_SEEK_SET);
    bytes_read = sd_read(sd_buffer, sizeof(uint32_t), 1024, sd_context);
    printf("bytes_read=%u\n", (uint32_t const)bytes_read);

    unsigned i;
    for (i = 0; i < 1024; ++i) {
        printf("%X%s", sd_buffer[i], ((i + 1) & 7 ? " " : "\n"));
    }




/*
    //for (r1 = 1; r1 & 0x01; ) {
        spim_cs_assert(SPIM_BASE_ADDR);
        sd_send_cmd(SPIM_BASE_ADDR, sd_cmd01);
        r1 = sd_get_r1(SPIM_BASE_ADDR, 0);
        spim_cs_deassert(SPIM_BASE_ADDR);
        sd_send_postamble(SPIM_BASE_ADDR);
        printf("CMD01: 0x%X\n", r1);
    //}
    spim_cs_assert(SPIM_BASE_ADDR);
    sd_send_cmd(SPIM_BASE_ADDR, sd_cmd08);
    r1 = sd_get_r1(SPIM_BASE_ADDR, 0);
    spim_cs_deassert(SPIM_BASE_ADDR);
    sd_send_postamble(SPIM_BASE_ADDR);
    printf("CMD08: 0x%X\n", r1);

    for (ocr = 0; !(ocr >> 31); ) {
        spim_cs_assert(SPIM_BASE_ADDR);
        sd_send_cmd(SPIM_BASE_ADDR, sd_cmd58);
        r1 = sd_get_r1(SPIM_BASE_ADDR, 0);
        spim_cs_deassert(SPIM_BASE_ADDR);
        sd_send_postamble(SPIM_BASE_ADDR);
        printf("CMD55: 0x%X\n", r1);

        spim_cs_assert(SPIM_BASE_ADDR);
        sd_send_cmd(SPIM_BASE_ADDR, sd_cmd58);
        ocr = sd_get_r3(SPIM_BASE_ADDR, 0, &r1);
        spim_cs_deassert(SPIM_BASE_ADDR);
        sd_send_postamble(SPIM_BASE_ADDR);
        printf("CMD41: 0x%X, 0x%x\n", r1, ocr);
    }
*/
}
