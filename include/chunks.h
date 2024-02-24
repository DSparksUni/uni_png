#ifndef CHUNK_H_INCLUDED_
#define CHUNK_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "types.h"

typedef struct IHDR_t {
    u4 length;
    
    //Data
    u4 width;
    u4 height;
    u1 bit_depth;
    u1 color_type;
    //[TODO] implement interlaced files
    //u1 interlace_method;

    u4 crc;
} IHDR;
void print_IHDR(const IHDR*);
bool validate_IHDR_colors(const IHDR*);

typedef struct PLTE_t {
    u4 length;

    u4 idx;
    u4 entries;
    //Data
    u1* red;
    u1* green;
    u1* blue;

    u4 crc;
} PLTE;
PLTE* create_PLTE(u4);
void print_PLTE(PLTE*);
void destroy_PLTE(PLTE*);

#endif  //CHUNK_H_INCLUDED_
