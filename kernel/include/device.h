#ifndef DEVICE_H
#define DEVICE_H

#include <stddef.h>

#include <soc.h>

typedef struct device_tree {
    char device_name[SOC_DEVICE_TYPE_SZB];
    unsigned device_number;
    uint32_t device_handle;
} device_tree_t;

device_tree_t *const device_enumerate(void);

#endif
