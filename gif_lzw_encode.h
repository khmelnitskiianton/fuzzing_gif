#ifndef GIF_LZW_ENCODE_H
#define GIF_LZW_ENCODE_H

#define MAX_SIZE_CODE_TABLE 4096        //standard of encoding gif
#define MAX_SIZE_UNCOMPRESSED_DATA 100  //max size of uncompressed data

//Record in table from LZW compression
typedef struct table_record {
    size_t  code;
    uint8_t *str;
    size_t  size;
} table_record_t;

//Table from LZW compression
typedef struct code_table {
    table_record_t  records[MAX_SIZE_CODE_TABLE];
    size_t          size;
} code_table_t;

//Main function of LZW encoding
void gif_lzw_compress   (const uint8_t *uncompressed_buffer, const size_t size_unc_buf, 
                        uint8_t *compressed_buffer, size_t *size_comp_buf, 
                        const uint8_t lzw_minimum_code_size);

#endif