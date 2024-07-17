/*

    My simple realization of encoding data of color indexes with GIF LZW format
    It bases on https://www.matthewflickinger.com/lab/whatsinagif/lzw_image_data.asp

*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "gif_lzw_encode.h"

const size_t INIT_RECORD_STR_SIZE = 20;
const int    NOT_FIND             = -1;

static void gif_lzw_create_record   (code_table_t *table, size_t size_of_bytes);
static int  gif_lzw_find_record     (const uint8_t *index_buffer, const size_t size_of_buf, code_table_t *table);
static void gif_lzw_add_to_table    (const uint8_t *index_buffer, const size_t len_index_buffer, code_table_t *table);
static bool gif_lzw_custom_compare  (const uint8_t *buff1, const size_t size_buf1, const uint8_t *buff2, const size_t size_buf2);

//This function compresses buffer with GIF LZW. LZW minimum code size default - minimum degree of two bigger than amount of colors
void gif_lzw_compress(const uint8_t *uncompressed_buffer, const size_t size_unc_buf, uint8_t *compressed_buffer, size_t *size_comp_buf, const uint8_t lzw_minimum_code_size)
{
    code_table_t table = {}; //create code table
    table.size = 0;

    //Initialize all colors and special codes
    for (int i = 0; i < (1 << lzw_minimum_code_size); i++)
    {
        gif_lzw_create_record(&table, 1);
        table.records[i].str[0] = (uint8_t) (table.records[i].code);
    }
    //two special records
    gif_lzw_create_record(&table, strlen("Clear Code"));
    int index_cc = (int) table.size-1;
    memcpy(table.records[(table.size)-1].str, "Clear Code", strlen("Clear Code"));
    gif_lzw_create_record(&table, strlen("EOI Code"));
    int index_eoi = (int) table.size-1;
    memcpy(table.records[(table.size)-1].str, "EOI Code", strlen("EOI Code"));

    //Start algorithm
    uint8_t lzw_current_code_size = lzw_minimum_code_size + 1; //start size of bits for code
    //1. Compress to code stream with codes from table.
    uint8_t index_buffer[MAX_SIZE_UNCOMPRESSED_DATA] = {};//support buffer
    uint8_t k = 0;
    size_t  pos_index_buffer = 0;
    *size_comp_buf = 0;

    uint8_t  byte_buf = 0;   // byte buffer write table code and write to compressed data if full
    uint8_t  buf_ptr = 1;    //number = 2^n - set current bit in byte
    int      index_ptr = 1; //number = 2^n - set current bit in number

    //1 byte add Clear Code 
    for (int i = 0; i < lzw_current_code_size; i++)
    {
        byte_buf += buf_ptr * ((uint8_t)(!!(index_cc & index_ptr)));
        index_ptr <<= 1;
        if (buf_ptr == 0x80){
            compressed_buffer[*size_comp_buf] = byte_buf;
            (*size_comp_buf)++;
            byte_buf = 0;
            buf_ptr = 1;
            continue;
        }
        buf_ptr <<= 1;
    }


    index_buffer[pos_index_buffer++] = uncompressed_buffer[0]; //move 1 byte in buffer

    for (size_t i = 1; i < size_unc_buf; i++)
    {
        k = uncompressed_buffer[i];
        index_buffer[pos_index_buffer++] = k; //move previous k in index buffer
        if (gif_lzw_find_record(index_buffer, pos_index_buffer,&table) != -1) //is content of index buffer in table
        {
            //k in index buffer already
        }
        else 
        {
            gif_lzw_add_to_table(index_buffer, pos_index_buffer, &table);//add index buffer + k to table
            pos_index_buffer--;
            index_buffer[pos_index_buffer] = 0;//remove k from index buffer
            int find_record = gif_lzw_find_record(index_buffer, pos_index_buffer, &table);
            if (find_record != -1) 
            {   
                index_ptr = 1; //number = 2^n - set current bit in number
                for (int j = 0; j < lzw_current_code_size; j++)
                {
                    byte_buf += buf_ptr * ((uint8_t)(!!(find_record & index_ptr)));
                    index_ptr <<= 1;
                    if (buf_ptr == 0x80){
                        compressed_buffer[*size_comp_buf] = byte_buf;
                        (*size_comp_buf)++;
                        byte_buf = 0;
                        buf_ptr = 1;
                        continue;
                    }
                    buf_ptr <<= 1;
                }
            }
            for(;pos_index_buffer > 0;pos_index_buffer--){index_buffer[pos_index_buffer] = 0;}//free index buffer
            index_buffer[pos_index_buffer] = k;//new index buffer consist of only k
            pos_index_buffer++;
            k = 0;
            if (((int) table.size-1) == (1 << lzw_current_code_size)) {lzw_current_code_size++;} //check for table overflowing 2^N size
        }
    }
    //Write code of index buffer after loop
    int end_index_index_buffer = gif_lzw_find_record(index_buffer, pos_index_buffer,&table);
    if (end_index_index_buffer != -1)
    { 
        index_ptr = 1; //number = 2^n - set current bit in number
        for (int i = 0; i < lzw_current_code_size; i++)
        {
            byte_buf += buf_ptr * ((uint8_t)(!!(end_index_index_buffer & index_ptr)));
            index_ptr <<= 1;
            if (buf_ptr == 0x80){
                compressed_buffer[*size_comp_buf] = byte_buf;
                (*size_comp_buf)++;
                byte_buf = 0;
                buf_ptr = 1;
                continue;
            }
            buf_ptr <<= 1;
        }
    }
    else 
    {
        gif_lzw_add_to_table(index_buffer, pos_index_buffer, &table);
        index_ptr = 1; //number = 2^n - set current bit in number
        for (int i = 0; i < lzw_current_code_size; i++)
        {
            byte_buf += buf_ptr * ((uint8_t)(!!(((int)table.size-1)& index_ptr)));
            index_ptr <<= 1;
            if (buf_ptr == 0x80){
                compressed_buffer[*size_comp_buf] = byte_buf;
                (*size_comp_buf)++;
                byte_buf = 0;
                buf_ptr = 1;
                continue;
            }
            buf_ptr <<= 1;
        }
        if (((int)table.size-1) == (1 << lzw_current_code_size)) {lzw_current_code_size++;} //check for table overflowing 2^N size
    }
    //EOF Code
    index_ptr = 1; //number = 2^n - set current bit in number
    for (int i = 0; i < lzw_current_code_size; i++)
    {
        byte_buf += buf_ptr * ((uint8_t)(!!(index_eoi & index_ptr)));
        index_ptr <<= 1;
        if (buf_ptr == 0x80){
            compressed_buffer[*size_comp_buf] = byte_buf;
            (*size_comp_buf)++;
            byte_buf = 0;
            buf_ptr = 1;
            continue;
        }
        buf_ptr <<= 1;
    }
    //Clear bit buffer
    if (byte_buf) 
    {
        compressed_buffer[*size_comp_buf] = byte_buf;
        (*size_comp_buf)++;
    }

    //Free table
    for (size_t i = 0; i < table.size; i++)
    {
        if (table.records[i].size != 0)
        {
            free(table.records[i].str);
        }
    }
}

//Creates new records and init it.
static void gif_lzw_create_record(code_table_t *table, size_t size_of_bytes)
{
    table->records[table->size].code = table->size; 
    table->records[table->size].str  = (uint8_t *) calloc(size_of_bytes + 1, sizeof(uint8_t));
    table->records[table->size].size = size_of_bytes;
    //printf("Create: size: %d\n, size bytes: %d\n\n", table->records[table->size].code, table->records[table->size].size);
    (table->size)++;
}

//Compare index_buffer and str in records. returns index of find record in table or -1.
static int gif_lzw_find_record(const uint8_t *index_buffer, const size_t size_of_buf, code_table_t *table)
{
    for (size_t i = 0; i < (table->size);i++)
    {   
        //Index buffer = index buffer + k, compare
        bool flag_in_table = gif_lzw_custom_compare(index_buffer, size_of_buf, table->records[i].str, table->records[i].size);
        if (flag_in_table) return (int)i;
    }
    return NOT_FIND;
}

//Add new record with content of index buffer to table 
static void gif_lzw_add_to_table(const uint8_t *index_buffer, const size_t len_index_buffer, code_table_t *table)
{
    gif_lzw_create_record(table, len_index_buffer);
    memcpy(table->records[(table->size)-1].str, index_buffer, len_index_buffer);
}

//Custom compare two buffers of uint8_t
static bool gif_lzw_custom_compare(const uint8_t *buff1, const size_t size_buf1, const uint8_t *buff2, const size_t size_buf2)
{
    if (size_buf1 != size_buf2) return false;
    for (size_t i = 0; i < size_buf1; i++)
    {
        if (buff1[i] != buff2[i]) return false;
    }
    return true;
}