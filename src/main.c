#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "types.h"
#include "chunks.h"

#define PNG_FILE_SIGNATURE 9894494448401390090ull 

byte* read_entire_binary(const char*, size_t*);

u8 read_bytes(byte*, size_t*, size_t, size_t);
char* read_string(byte*, size_t*, size_t, size_t);
/*
u2 read_u2(byte*, size_t, size_t);
u4 read_u4(byte*, size_t, size_t);
u8 read_u8(byte*, size_t, size_t);
*/
#define read_u1(data, cursor, len) (byte)read_bytes(data, &cursor, len, 1)
#define read_u2(data, cursor, len) (u2)read_bytes(data, &cursor, len, 2)
#define read_u4(data, cursor, len) (u4)read_bytes(data, &cursor, len, 4)
#define read_u8(data, cursor, len) read_bytes(data, &cursor, len, 8)

#define read_string1(data, cursor, len) read_string(data, &cursor, len, 1)
#define read_string2(data, cursor, len) read_string(data, &cursor, len, 2)
#define read_string4(data, cursor, len) read_string(data, &cursor, len, 4)
#define read_string8(data, cursor, len) read_string(data, &cursor, len, 8)

int main(int argc, char** argv) {
    if(argc < 2) {
        fputs("[ERROR] No input file supplied...\n", stderr);
        return -1;
    }
    const char* image_file_path = argv[1];

    byte* idat_data;

    size_t image_size;
    byte* image_data = read_entire_binary(
        image_file_path, &image_size
    );
    if(!image_data) {
        fprintf(
            stderr, "[ERROR] Couldn't read file '%s'\n", image_file_path
        );
        return -1;
    }

    printf(
        "PNG file '%s' of size %zu bytes:\n\n",
        image_file_path, image_size
    );
    size_t cursor = 0;  //Index into file  

    u8 file_sig = read_u8(image_data, cursor, image_size);
    if(file_sig != PNG_FILE_SIGNATURE) {
        fprintf(
            stderr, "[ERROR] Invalid png file signature (%zu)\n",
            (size_t)file_sig
        );
        fprintf(
            stderr, "[INFO] Expected file signature: %llu\n",
            PNG_FILE_SIGNATURE
        );
        goto fail;
    }
    puts("Valid file signature...\n");

    u4 header_len = read_u4(image_data, cursor, image_size);

    char* header_chunk_type = read_string4(
        image_data, cursor, image_size
    );
    if(strcmp(header_chunk_type, "IHDR") != 0) {
        fprintf(
            stderr,
            "[ERROR] Incorrect initial chunk type '%s'\n",
            header_chunk_type
        );
        free(header_chunk_type);
        goto fail;
    }
    free(header_chunk_type);

    IHDR header;
    header.length = header_len;
    header.width = read_u4(
        image_data, cursor, image_size
    );
    header.height = read_u4(
        image_data, cursor, image_size
    );

    header.bit_depth = image_data[cursor++];
    header.color_type = image_data[cursor++];
    if(!validate_IHDR_colors(&header)) {
        fputs(
            "[ERROR] Invalid combination of color type and bit depth",
            stderr
        );
        fprintf(
            stderr, "[INFO] color_type=%u, bit_depth=%u\n",
            header.bit_depth, header.color_type
        );
    }
    printf("Valid color type and bit depth...\n\n");

    u1 compression_method = image_data[cursor++];
    if(compression_method != 0) {
        fputs(
            "[ERROR] Invalid compression method...\n",
            stderr
        );
        goto fail;
    }

    u1 filter_method = image_data[cursor++];
    if(filter_method != 0) {
        fputs(
            "[ERROR] Invalid filter method...\n",
            stderr
        );
        goto fail;
    }

    u1 interlace_method = image_data[cursor++];
    if(interlace_method != 0) {
        fputs(
            "[ERROR] Parsing interlaced files is not implemented\n",
            stderr
        );
    }

    header.crc = read_u4(image_data, cursor, image_size);
    print_IHDR(&header);

    u4 next_len = read_u4(image_data, cursor, image_size);
    char* next_type = read_string4(
        image_data, cursor, image_size
    );
    PLTE* palette;
    if(strcmp(next_type, "PLTE") == 0) {
        palette = create_PLTE(next_len);

        for(size_t i = 0; i < palette->entries; i++) {
            palette->red[i] = image_data[cursor++];
            palette->green[i] = image_data[cursor++];
            palette->blue[i] = image_data[cursor++];
        }

        palette->crc = read_u4(
            image_data, cursor, image_size
        );

        print_PLTE(palette);

        free(next_type);
        destroy_PLTE(palette);
    }

    u4 idat_len = read_u4(image_data, cursor, image_size);
    char* idat_type = read_string4(
        image_data, cursor, image_size
    );
    if(strcmp(idat_type, "IDAT") != 0) {
        fputs(
            "[ERROR] Ancillary chunk parsing is not implemented",
            stderr
        );

        free(idat_type);
        goto fail;
    }

    printf("IDAT Chunk(%u) {}\n\n", idat_len);
    idat_data = malloc(idat_len);
    
    printf(
        "IDAT Samples: [%x, %x, %x, %x, %x, %x]\n\n",
        idat_data[0], idat_data[1], idat_data[2],
        idat_data[idat_len-1], idat_data[idat_len-2],
        idat_data[idat_len-3]
    );

    if(!idat_data) {
        fputs(
            "[ERROR] Failed to allocate IHDR buffer\n",
            stderr
        );
        goto fail;
    }
    memcpy(idat_data, image_data+cursor, idat_len);
    cursor += idat_len;

    printf(
        "IDAT Beginning Samples: [%x, %x, %x, %x]\n",
        idat_data[0], idat_data[1], idat_data[2],
        idat_data[3]
    );
    printf(
        "IDAT End Samples: [%x, %x, %x, %x]\n\n",
        idat_data[idat_len-1], idat_data[idat_len-2],
        idat_data[idat_len-3], idat_data[idat_len-4]
    );

    u4 idat_crc = read_u4(image_data, cursor, image_size);

    u4 iend_len = read_u4(image_data, cursor, image_size);

    char* iend_type = read_string4(image_data, cursor, image_size);
    if(!iend_type) goto fail;

    if(strcmp(iend_type, "IEND") != 0) {
        fputs(
            "[ERROR] Parsing multiple IDAT chunks is not implemented...\n",
            stderr
        );
        fprintf(
            stderr, "[INFO] Chunk type: %s\n",
            iend_type
        );
        free(iend_type);
        goto fail;
    }
    free(iend_type);

    printf("IEND Chunk(%u) {}\n\n", iend_len);

    u4 iend_crc = read_u4(image_data, cursor, image_size);

    free(idat_data);
    free(image_data);
    return 0;
fail:
    if(idat_data) free(idat_data);
    free(image_data);
    return -1;
}

byte* read_entire_binary(
        const char* file_path, size_t* out_file_size
) {
    FILE* input_file = fopen(file_path, "rb");
    if(!input_file) return NULL;

    fseek(input_file, 0, SEEK_END);
    size_t input_file_size = (size_t)ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    byte* file_buffer = malloc(input_file_size);
    if(!file_buffer) {
        fclose(input_file);
        return NULL;
    }

    size_t bytes_read = fread(
        file_buffer, 1, input_file_size, input_file
    );
    if(bytes_read != input_file_size) {
        fclose(input_file);
        free(file_buffer);
        return NULL;
    }

    *out_file_size = input_file_size;
    return file_buffer;
}

u8 read_bytes(
    byte* data, size_t* cursor, size_t data_len, size_t n
) {
    if(*cursor + n >= data_len) return 0;

    u8 result = 0;
    for(size_t i = *cursor; i < (*cursor)+n; i++)
        result = (result << 8) | data[i];

    *cursor += n;
    return result;
}

char* read_string(
    byte* data, size_t* cursor, size_t data_len, size_t n
) {
    if(*cursor + n >= data_len){
        fputs(
            "[ERROR] Attempted string reading exceeds buffer size\n",
            stderr
        );

        return NULL;
    }

    char* string = malloc(n + 1);
    if(!string) return NULL;
   
    for(size_t i = 0; i < n; i++)
        string[i] = (char)(data[(*cursor)+i]);

    *cursor += n;
    string[n] = '\0';
    return string;
}

