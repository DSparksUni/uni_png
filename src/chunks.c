#include "chunks.h"

void print_IHDR(const IHDR* chunk) {
    printf("IHDR Chunk(%u) {\n", chunk->length);

    printf("    width = %u\n", chunk->width);
    printf("    height = %u\n", chunk->height);
    printf("    bit depth = %u\n", chunk->bit_depth);
    printf("    color type = %u\n", chunk->color_type);
    //[TODO] implement interlaced files
    //printf("    interlace method = %u\n", chunk->interlace_method);

    printf("}\n\n");
}

bool validate_IHDR_colors(const IHDR* chunk) {
    u1 bd = chunk->bit_depth;
    switch(chunk->color_type) {
        case 0: return  bd == 1 || bd == 2 || bd == 4 ||
                        bd == 8 || bd == 16;
        case 2: return  bd == 8 || bd == 16;
        case 3: return  bd == 1 || bd == 2 || bd == 4 ||
                        bd == 8;
        case 4: return  bd == 8 || bd == 16;
        case 6: return  bd == 8 || bd == 16;

        default: return false;
    }
}

PLTE* create_PLTE(u4 length) {
    if(length % 3 != 0) return NULL;
    u4 entries = length / 3;

    PLTE* chunk = malloc(sizeof(PLTE));
    if(!chunk) return NULL;

    chunk->red = calloc(entries, sizeof(u1));
    chunk->green = calloc(entries, sizeof(u1));
    chunk->blue = calloc(entries, sizeof(u1));

    chunk->idx = 0;
    chunk->length = length;
    chunk->entries = entries;
    chunk->crc = 0;

    return chunk;
}

void print_array(const char* name, u1* array, u4 len) {
    printf("    %s = [", name);
    for(u4 i = 0; i < len; i++) {
        if(i != len - 1) printf("%u, ", array[i]);
        else printf("%u", array[i]);
    }
    printf("]\n");
}

void print_PLTE(PLTE* chunk) {
    printf("PLTE Chunk(%u) {\n", chunk->length);

    printf("    red = [%u]\n", chunk->entries);
    printf("    green = [%u]\n", chunk->entries);
    printf("    blue = [%u]\n", chunk->entries);

    printf("}\n\n");
}

void destroy_PLTE(PLTE* chunk) {
    free(chunk->red);
    free(chunk->green);
    free(chunk->blue);

    free(chunk);
}
