#include <vmem32.h>

#include <stddef.h>
#include <stdint.h>

#include <vmutex32.h>

#define VMEM32_PAGE_SZWX        5
#define VMEM32_PAGE_SZW         (1 << VMEM32_PAGE_SZWX)
#define VMEM32_PAGE_SZW_MASK    (VMEM32_PAGE_SZW - 1)
#define VMEM32_PAGE_SZBX        (VMEM32_PAGE_SZWX + 2)
#define VMEM32_PAGE_SZB         (1 << VMEM32_PAGE_SZBX)
#define VMEM32_PAGE_SZB_MASK    (VMEM32_PAGE_SZB - 1)

#include <uart.h>

vmutex32_t table_mutex;

uint32_t mpool_start;
uint32_t mpool_szp;

inline void mark_page(uint32_t const page, uint32_t const value) {
    if (value) {
        ((uint32_t *const)mpool_start)[page >> 5] |=  (1 << (page & 0x1f));
    } else {
        ((uint32_t *const)mpool_start)[page >> 5] &= ~(1 << (page & 0x1f));
    }
}
inline uint32_t const get_page(uint32_t const page) {
    return (((uint32_t *const)mpool_start)[page >> 5] >> (page & 0x1f)) & 1;
}
void vmem32_dump_table(void) {
    int i;

    for (i = 0; i < (mpool_szp >> 5) + (mpool_szp & 0x1f ? 1 : 0); ++i) {
        printf("%X%c", ((uint32_t const *const)mpool_start)[i], ((i + 1) & 0x7 ? ' ' : '\n'));
    }
}
void vmem32_init(uint8_t *const first_byte, uint8_t *const last_byte) {
    uint32_t remainder;
    uint32_t table_szw;
    uint32_t table_szp;
    uint32_t i;

    remainder   = (uint32_t const)first_byte & VMEM32_PAGE_SZB_MASK;
    mpool_start = ((uint32_t const)first_byte & ~VMEM32_PAGE_SZB_MASK) + (remainder ? VMEM32_PAGE_SZB : 0);
    mpool_szp   = ((uint32_t const)last_byte + 1 - mpool_start) >> VMEM32_PAGE_SZBX;

    remainder = mpool_szp & 0x1f;
    table_szw = (mpool_szp >> 5) + (remainder ? 1 : 0);
    remainder = table_szw & VMEM32_PAGE_SZW_MASK;
    table_szp = (table_szw >> VMEM32_PAGE_SZWX) + (remainder ? 1 : 0);

    for (i = 0; i < table_szp - 1; ++i) {
        mark_page(i, 1);
    }
    for (; i < mpool_szp; ++i) {
        mark_page(i, 0);
    }

    vmutex32_init(&table_mutex);
}
void *const vmem32_alloc(size_t const n_bytes) {
    uint32_t remainder;
    uint32_t n_pages;
    uint32_t alloc_start_page;
    uint32_t i;
    uint32_t page_count;

    if (n_bytes == 0) {
        return VMEM32_NULL;
    }

    remainder = n_bytes & VMEM32_PAGE_SZB_MASK;
    n_pages   = (n_bytes >> VMEM32_PAGE_SZBX) + (remainder ? 1 : 0);
    if (n_pages == 1) {
        n_pages = 2;
    }

    vmutex32_wait_for_lock(&table_mutex, VMUTEX32_STATE_LOCKED);
    alloc_start_page = 0;
    for (i = 0; i + n_pages <= mpool_szp && alloc_start_page == 0; ) {
        // check for a contigous run of n_pages free pages
        for (page_count = 0; get_page(i + page_count) == 0 && page_count < n_pages + 1; ++page_count);

        if (page_count == n_pages + 1) { // free page run found
            alloc_start_page = i + 1;
            for (page_count = 1; page_count < n_pages; ++page_count) {
                mark_page(i + page_count, 1);
            }
        }

        i += page_count + 1;
    }
    vmutex32_unlock(&table_mutex);

    // generate pointer
    if (alloc_start_page == 0) {
        return VMEM32_NULL;
    }
    return (void *const)(mpool_start + (alloc_start_page << VMEM32_PAGE_SZBX));
}
void vmem32_free(void *const ptr) {
    uint32_t page;
    uint32_t i;

    page = ((uint32_t const)ptr - mpool_start) >> VMEM32_PAGE_SZBX;

    vmutex32_wait_for_lock(&table_mutex, VMUTEX32_STATE_LOCKED);
    for (i = page; get_page(i) != 0; ++i) {
        mark_page(i, 0);
    }
    vmutex32_unlock(&table_mutex);
}
