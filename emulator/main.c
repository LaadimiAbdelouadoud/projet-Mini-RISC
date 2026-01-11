#include "platform.h"
#include "minirisc.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program.bin>\n", argv[0]);
        return 1;
    }

    platform_t *platform = platform_new();
    if (!platform) {
        fprintf(stderr, "Error: Failed to create platform\n");
        return 1;
    }

    minirisc_t *minirisc = minirisc_new(0x80000000, platform);
    if (!minirisc) {
        fprintf(stderr, "Error: Failed to create processor\n");
        platform_free(platform);
        return 1;
    }

    platform_load_program(platform, argv[1]);

    printf("Starting Mini-RISC emulator...\n");
    minirisc_run(minirisc);
    printf("\n");

    minirisc_free(minirisc);
    platform_free(platform);

    return 0;
}