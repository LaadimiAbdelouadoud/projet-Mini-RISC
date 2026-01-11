#include "platform.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MEMORY_SIZE (32 * 1024 * 1024)  // 32 MiB
#define MEMORY_BASE 0x80000000
#define CHAROUT_BASE 0x10000000

platform_t* platform_new(void)
{
    platform_t *plt = (platform_t *)malloc(sizeof(platform_t));
    if (!plt) {
        fprintf(stderr, "Error: Failed to allocate platform\n");
        return NULL;
    }

    plt->memory = (uint32_t *)malloc(MEMORY_SIZE);
    if (!plt->memory) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        free(plt);
        return NULL;
    }

    memset(plt->memory, 0, MEMORY_SIZE);

    return plt;
}

void platform_free(platform_t *platform)
{
    if (platform) {
        if (platform->memory) {
            free(platform->memory);
        }
        free(platform);
    }
}

static int get_access_size(access_type_t access_type)
{
    switch (access_type) {
        case ACCESS_BYTE: return 1;
        case ACCESS_HALF: return 2;
        case ACCESS_WORD: return 4;
        default: return -1;
    }
}

static int is_valid_address(uint32_t addr, int access_size)
{
    // Check if address is in memory range [0x80000000, 0x80000000 + 32MiB)
    if (addr >= MEMORY_BASE && addr + access_size <= MEMORY_BASE + MEMORY_SIZE) {
        return 1;
    }
    // Check if address is in CharOut range [0x10000000, 0x10000010)
    if (addr >= CHAROUT_BASE && addr + access_size <= CHAROUT_BASE + 0x10) {
        return 1;
    }
    return 0;
}

static int is_aligned(uint32_t addr, access_type_t access_type)
{
    int alignment = get_access_size(access_type);
    return (addr % alignment) == 0;
}

int platform_read(platform_t *plt, access_type_t access_type, uint32_t addr, uint32_t *data)
{
    int access_size = get_access_size(access_type);

    if (!is_valid_address(addr, access_size)) {
        fprintf(stderr, "Error: Invalid memory address 0x%08x\n", addr);
        return -1;
    }

    if (!is_aligned(addr, access_type)) {
        fprintf(stderr, "Error: Misaligned access at 0x%08x\n", addr);
        return -1;
    }

    // GETCHAR at 0x1000000C
    if (addr == CHAROUT_BASE + 0x0C) {
        if (access_type != ACCESS_WORD) {
            fprintf(stderr, "Error: GETCHAR requires 32-bit read\n");
            return -1;
        }
        
        int input = getchar();
        if (input == EOF) {
            fprintf(stderr, "Error: EOF on stdin\n");
            return -1;
        }
        
        *data = (uint32_t)input;
        return 0;
    }

    // CharOut always returns 0 for other I/O addresses
    if (addr >= CHAROUT_BASE && addr < CHAROUT_BASE + 0x10) {
        *data = 0;
        return 0;
    }

    // Read from memory
    uint8_t *memory_bytes = (uint8_t *)plt->memory;
    uint32_t byte_offset = addr - MEMORY_BASE;

    *data = 0;

    if (access_type == ACCESS_BYTE) {
        *data = memory_bytes[byte_offset];
    } else if (access_type == ACCESS_HALF) {
        *data = (memory_bytes[byte_offset] |
                (memory_bytes[byte_offset + 1] << 8));
    } else if (access_type == ACCESS_WORD) {
        *data = (memory_bytes[byte_offset] |
                (memory_bytes[byte_offset + 1] << 8) |
                (memory_bytes[byte_offset + 2] << 16) |
                (memory_bytes[byte_offset + 3] << 24));
    }

    return 0;
}

int platform_write(platform_t *plt, access_type_t access_type, uint32_t addr, uint32_t data)
{
    int access_size = get_access_size(access_type);

    if (!is_valid_address(addr, access_size)) {
        fprintf(stderr, "Error: Invalid memory address 0x%08x\n", addr);
        return -1;
    }

    if (!is_aligned(addr, access_type)) {
        fprintf(stderr, "Error: Misaligned access at 0x%08x\n", addr);
        return -1;
    }

    // Handle CharOut device
    if (addr >= CHAROUT_BASE && addr < CHAROUT_BASE + 0x10) {
        if (addr == CHAROUT_BASE) {
            // Write character (lower 8 bits)
            printf("%c", (char)(data & 0xFF));
            fflush(stdout);
        } else if (addr == CHAROUT_BASE + 4) {
            // Write signed integer
            printf("%d", (int32_t)data);
            fflush(stdout);
        } else if (addr == CHAROUT_BASE + 8) {
            // Write unsigned integer in hex
            printf("%x", data);
            fflush(stdout);
        }
        return 0;
    }

    // Write to memory
    uint8_t *memory_bytes = (uint8_t *)plt->memory;
    uint32_t byte_offset = addr - MEMORY_BASE;

    if (access_type == ACCESS_BYTE) {
        memory_bytes[byte_offset] = (uint8_t)(data & 0xFF);
    } else if (access_type == ACCESS_HALF) {
        memory_bytes[byte_offset] = (uint8_t)(data & 0xFF);
        memory_bytes[byte_offset + 1] = (uint8_t)((data >> 8) & 0xFF);
    } else if (access_type == ACCESS_WORD) {
        memory_bytes[byte_offset] = (uint8_t)(data & 0xFF);
        memory_bytes[byte_offset + 1] = (uint8_t)((data >> 8) & 0xFF);
        memory_bytes[byte_offset + 2] = (uint32_t)((data >> 16) & 0xFF);
        memory_bytes[byte_offset + 3] = (uint8_t)((data >> 24) & 0xFF);
    }

    return 0;
}

void platform_load_program(platform_t *plt, const char *file_name)
{
    FILE *fp = fopen(file_name, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Failed to open file '%s'\n", file_name);
        return;
    }

    uint8_t *memory_bytes = (uint8_t *)plt->memory;
    size_t bytes_read = fread(memory_bytes, 1, MEMORY_SIZE, fp);
    
    printf("Loaded %zu bytes from '%s'\n", bytes_read, file_name);

    fclose(fp);
}
