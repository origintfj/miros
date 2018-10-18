#include <vprocess32.h>

#include <vmem32.h>
#include <vstring.h>

//#include <uart.h> // TODO - remove

static void *const vprocess32_premain(void *const arg) { // TODO - move section init into vprocess32_start
    uint32_t const argc  = ((uint32_t const *)arg)[0];
    uint32_t const argv  = ((uint32_t const *)arg)[1];
    uint32_t const entry = ((uint32_t const *)arg)[2];

    uint32_t const *const got_end = (uint32_t const *const)(((uint32_t const *)arg)[2] + (2 << 2));
    uint32_t const *const bss_end = (uint32_t const *const)(((uint32_t const *)arg)[2] + (4 << 2));

    //printf("\n");
    //printf("argc  : %X\n", argc);
    //printf("argv  : %X\n", argv);
    //printf("entry : %X\n", entry);

    uint32_t *table_index;

    // got start
    table_index = (uint32_t *)(((uint32_t const *)arg)[2] + (1 << 2));
    //printf("got_start : %X\n", table_index);
    //printf("got_end   : %X\n", got_end);
    for (; table_index < got_end; ++table_index) {
        //*table_index = *table_index + entry;
    }

    // bss start
    table_index = (uint32_t *)(((uint32_t const *)arg)[2] + (3 << 2));
    //printf("bss_start : %X\n", table_index);
    //printf("bss_end   : %X\n", bss_end);
    for (; table_index < got_end; ++table_index) {
        //*table_index = 0;
    }

    __asm__ volatile ("mv a0, %0; mv a1, %1; jalr %2;"
                      :: "r"(argc), "r"(argv), "r"(entry) : "ra", "memory");

    void *rtn_val;
    __asm__ volatile ("mv %0, a0;" : "=r"(rtn_val) :: "memory");

    return rtn_val; // enable return value
}
thread_id_t const vprocess32_start(fat32_t *const fat32, char const *const path, unsigned const stack_szw,
                                   int const argc, char const *const *const argv) {
    // open the program file
    fat32_file_t *const ifile = fat32_open(fat32, path);
    if (ifile == NULL) {
        return 0;
    }

    // calculate the size of the argv-buffer
    unsigned i, arg_buffer_szb;
    for (i = 0, arg_buffer_szb = 0; i < argc; ++i) {
        arg_buffer_szb += strlen(argv[i]) + 1;
    }

    // allocate a memory block for the program (stack, argv, program, and argv-buffer)
    fat32_seek(ifile, 0, FAT32_SEEK_END);
    unsigned ifile_len = fat32_tell(ifile);
    uint8_t *const process_container = (uint8_t *const)vmem32_alloc(stack_szw * sizeof(uint32_t)
                                                                    + argc * sizeof(char *)
                                                                    + ifile_len * sizeof(uint8_t)
                                                                    + arg_buffer_szb * sizeof(char));

    // assign the relevant sections of the buffer to appropriate pointers
    uint32_t *stack_base_fd;
    char **arg_vector;
    void *const(*process_entry)(void *const);
    char *arg_buffer;
    stack_base_fd = (uint32_t *const)&process_container[stack_szw * sizeof(uint32_t)];
    arg_vector = (char **const)&process_container[stack_szw * sizeof(uint32_t)];
    process_entry = (void *const(*)(void *const))&process_container[stack_szw * sizeof(uint32_t)
                                                                    + argc * sizeof(char *)];
    arg_buffer = (char *const)&process_container[stack_szw * sizeof(uint32_t)
                                                 + argc * sizeof(char *)
                                                 + ifile_len * sizeof(uint8_t)];

    // copy the program into the container, and close the file
    fat32_seek(ifile, 0, FAT32_SEEK_SET);
    fat32_read(&process_container[stack_szw * sizeof(uint32_t) + argc * sizeof(char *)],
                                  sizeof(uint8_t), ifile_len, ifile);
    fat32_close(ifile);

    // deep-copy argv and the argv-buffer to the process container
    for (i = 0, arg_buffer_szb = 0; i < argc; ++i) {
        arg_vector[i] = &arg_buffer[arg_buffer_szb];
        strcpy(arg_vector[i], argv[i]);
        arg_buffer_szb += strlen(argv[i]) + 1;
    }

    stack_base_fd -= 3;
    stack_base_fd[0] = (uint32_t const)argc;
    stack_base_fd[1] = (uint32_t const)arg_vector;
    stack_base_fd[2] = (uint32_t const)process_entry;

    thread_id_t thread_id;
    thread_id = vthread32_create_raw(vprocess32_premain, stack_base_fd,
                                     process_container, stack_base_fd, 0x0080);

    return thread_id;
}
