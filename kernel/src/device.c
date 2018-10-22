#include <device.h>

#include <vstring.h>
#include <vmem32.h>
#include <vtime32.h>
#include <sd.h>
//#include <uart.h>

extern soc_device_t soc_device_table[];

device_tree_t *const device_enumerate(void) {
    unsigned device_count;
    for (device_count = 0; soc_device_table[device_count].type[0] != '\0'; ++device_count);

    printf("%u devices found.\n", device_count);
    device_tree_t *device_tree;
    device_tree = (device_tree_t *const)vmem32_alloc(device_count * sizeof(device_tree_t));

    unsigned i;
    char device_name[SOC_DEVICE_TYPE_SZB];
    char const *subname;
    for (i = 0; i < device_count; ++i) {
        strcpy(device_name, soc_device_table[i].type);
        unsigned j;
        void *current_context;
        for (j = 0, current_context = NULL; device_name[j] != '\0'; ) {
            for (subname = &device_name[j]; device_name[j] != '.' && device_name[j] != '\0'; ++j);
            if (device_name[j] == '.') {
                device_name[j++] = '\0';
            }

            if (!strcmp(subname, "SPIM_SD")) {
                device_tree[i].device_handle = (uint32_t const)sd_context_create(soc_device_table[i].dev_base_addr, CLK_FREQ);
            } else if (!strcmp(subname, "TIMER")) {
                device_tree[i].device_handle = (uint32_t const)vmem32_alloc(sizeof(vtime32_context_t));
            }
            printf("-%s-", subname);
        }
        printf("\n");
    }
}
