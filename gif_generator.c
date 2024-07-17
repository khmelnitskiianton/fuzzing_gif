/*

    My simple realization of Random GIF Generator (include LZW encoding)
    Common Bug: in image data if amount of sub-blocks not one, only first sub-block will be display.
    Thats why only one sub-block will generates! [0,255]
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "gif_generator.h"
#include "gif_lzw_encode.h"

static const size_t   PACKAGE_FIELD_POS  = 10;      //Position of Package Field in LSD
static const uint8_t  END_TRAILER        = 0x3B;    //Gif trailer - end code for image
static const uint16_t SHIFT_POS          = 3;       //shift pos for image descriptor 
static const uint16_t MAX_SIZE_IMAGE     = 0x30;    //max height/width of image

#define SIZE_OF_UNC_DATA 0x2A0   //Size of uncompressed data - fixed, 0x300 after compression <= 250 - ideal for image with one block

static void generate_gif_title                     (gif_object_t *gif);
static void generate_gif_logical_screen_descriptor (gif_object_t *gif);
static void generate_gif_color_table               (gif_object_t *gif, size_t size_of_table);
static void generate_gif_image_descriptor          (gif_object_t *gif, uint32_t uncompressed_size_block);
static void generate_gif_sub_block                 (gif_object_t *gif, uint32_t uncompressed_size_block);

static void generate_gif_graphics_control_extension(gif_object_t *gif);
static void generate_gif_plain_text_extension(gif_object_t *gif);
static void generate_gif_application_extension(gif_object_t *gif);
static void generate_gif_comment_extension(gif_object_t *gif);

void generate_gif(gif_object_t *gif)
{
    //Need:
    //gif->data - pointer on buffer to write
    //gif->pos - shift in buff to start

    generate_gif_title(gif);
    generate_gif_logical_screen_descriptor(gif);

    //Generate global color table
    size_t size_of_global_color_table = 1 << (((gif->data)[PACKAGE_FIELD_POS] & 0b00000111) + 1); //number of colors = 2^(N+1)
    generate_gif_color_table(gif, size_of_global_color_table);

    
    if (rand()%5 == 0)  //1/4 probability because this block crashes file!
    {
        generate_gif_graphics_control_extension(gif);//turn on/off custom animation
    }

    //Blocks of Image Data
    //P.S. I make them only with size [0,255] because sequence in one descriptor gives error(idk why).
    uint32_t size = ((uint32_t)(gif->height)) * ((uint32_t)(gif->width)); 
    uint32_t amount_of_image_descriptors = size / SIZE_OF_UNC_DATA;
    generate_gif_image_descriptor(gif, size % SIZE_OF_UNC_DATA);
    if (rand()%2) amount_of_image_descriptors = 0; //set random: one or more images
    for(uint32_t i = 0; i < amount_of_image_descriptors; i++)
    {
        generate_gif_image_descriptor(gif, SIZE_OF_UNC_DATA-1);
    }

    //Extensions
    if (rand()%5 == 0)  //1/4 probability because this block crashes file!
    {
        generate_gif_plain_text_extension(gif);//turn on/off custom animation
    }

    if (rand()%5 == 0)  //1/4 probability because this block crashes file!
    {
        generate_gif_application_extension(gif);//turn on/off custom animation
    }

    if (rand()%5 == 0)  //1/4 probability because this block crashes file!
    {
        generate_gif_comment_extension(gif);//turn on/off custom animation
    }

    (gif->data)[(gif->pos)++] = END_TRAILER; //end gif image
}

//Set signature of GIF file
static void generate_gif_title(gif_object_t *gif)
{
    char versions[2][7] = {"GIF89a", "GIF87a"};
    if (rand() % 10)  //1/10 case will generate 87a version - old for many extensions
    {
        memcpy(gif->data, versions[0], 6);
    }
    else
    {
        memcpy(gif->data, versions[1], 6);
    }
    gif->pos += 6; 
}

//Set logical screen descriptor of GIF file
static void generate_gif_logical_screen_descriptor(gif_object_t *gif)
{
    //Set width and height - uint16 LE x 2
    uint16_t width  = 1+(uint16_t) (rand() % MAX_SIZE_IMAGE);
    uint16_t height = 0+(uint16_t) (rand() % MAX_SIZE_IMAGE);
    gif->width  = width;
    gif->height = height;
    memcpy((gif->data)+(gif->pos), (uint8_t *)&(width), sizeof(uint16_t));     //writes 2byte number to buffer with Little Endian
    gif->pos += 2;
    memcpy((gif->data)+(gif->pos), (uint8_t *)&(height),sizeof(uint16_t));
    gif->pos += 2;

    //Set Packed Field - uint8 LE with 4 fields!
    uint8_t global_color_table_flag    = 1;  //1 byte 
    uint8_t color_resolution           = (uint8_t) (rand() % 8);  //3 bytes
    uint8_t sort_flag                  = 0;         //1 byte  (doesn't make sense)
    uint8_t size_of_global_color_table = (uint8_t) (rand() % 2);  //N -> 2^(N+1) colors i stop at 0-1
    //merge all fields
    uint8_t packed_field = (uint8_t)((((((global_color_table_flag << 3) +
                                        color_resolution)         << 1) + 
                                        sort_flag)                << 3) + 
                                        size_of_global_color_table);
    (gif->data)[(gif->pos)++] = packed_field;

    //Set Background Color Index & PixelAspect Ratio
    uint8_t background_color_index = (uint8_t) (rand() % (1 << (size_of_global_color_table+1))); //index of background color from global table
    uint8_t pixel_aspect_ratio     = (uint8_t) (rand() % 2);//set 0 or 1, strange thing 
    (gif->data)[(gif->pos)++] = background_color_index;
    (gif->data)[(gif->pos)++] = pixel_aspect_ratio;
}

static void generate_gif_color_table(gif_object_t *gif, size_t size_of_table)
{
    for (size_t i = 0; i < size_of_table; i++) //each color - 3 bytes in RGB
    {
        uint8_t r = (uint8_t) (rand() % 0xFF);
        uint8_t g = (uint8_t) (rand() % 0xFF);
        uint8_t b = (uint8_t) (rand() % 0xFF);
        (gif->data)[(gif->pos)++] = r;
        (gif->data)[(gif->pos)++] = g;
        (gif->data)[(gif->pos)++] = b;
    }
}

static void generate_gif_image_descriptor(gif_object_t *gif, uint32_t uncompressed_size_block)
{
    //Write image separator
    uint8_t image_separator = 0x2C;
    (gif->data)[(gif->pos)++] = image_separator;

    //Write position left & top generate
    uint16_t left = 0;
    uint16_t top  = 0;
    if (rand() % 2) //Turn on/off shift
    {
        left = SHIFT_POS;
        top  = SHIFT_POS;
    }
    memcpy((gif->data)+(gif->pos), (uint8_t *)&(left), sizeof(uint16_t)); //writes 2byte number to buffer with Little Endian
    gif->pos += 2;
    memcpy((gif->data)+(gif->pos), (uint8_t *)&(top), sizeof(uint16_t));
    gif->pos += 2;  

    //Write width & height dont generate
    memcpy((gif->data)+(gif->pos), (uint8_t *)&(gif->width), sizeof(uint16_t)); //writes 2byte number to buffer with Little Endian
    gif->pos += 2;
    memcpy((gif->data)+(gif->pos), (uint8_t *)&(gif->height), sizeof(uint16_t));
    gif->pos += 2;    

    uint8_t local_color_table_flag    = 0;                    //1 byte
    uint8_t interlace_flag            = 0;                    //1 byte 
    uint8_t sort_flag                 = 0;                    //1 byte
    uint8_t reserve                   = 0;                    //2 bytes
    uint8_t size_of_local_color_table = 0;                    //3 bytes

    uint8_t packed_field=(uint8_t)((((((((local_color_table_flag  << 1) | 
                                       interlace_flag)            << 1) | 
                                       sort_flag)                 << 2) |
                                       reserve)                   << 3) | 
                                       size_of_local_color_table);
    (gif->data)[(gif->pos)++] = packed_field;    

    //No local table in my generation

    //Image Data
    // 1 byte lzw minimum code size
    uint8_t lzw_minimum_code_size = ((gif->data)[PACKAGE_FIELD_POS] & 0b00000111) + 1; //because formula 2^(n+1)
    gif->lzw_minimum_code_size = lzw_minimum_code_size;
    
    //Starting sub-blocks
    generate_gif_sub_block(gif, uncompressed_size_block);

    (gif->data)[(gif->pos)++] = 0;//set 00 byte for end of image data
}

static void generate_gif_sub_block(gif_object_t *gif, uint32_t uncompressed_size_block)
{
    if (!uncompressed_size_block) return;
    (gif->data)[(gif->pos)++] = gif->lzw_minimum_code_size;
    //Sub block consist of 1 byte - size of data block of compressed with LZW sequence of indexes in color table
    //Make random sequence of color indexes - dry data
    uint8_t uncompressed_buffer[SIZE_OF_UNC_DATA+100] = {};                      //make buffer for dry data
    int amount_colors = 1 << (gif->lzw_minimum_code_size);         //number of colors from table
    for (size_t i = 0; i < uncompressed_size_block; i++) 
    {
        uncompressed_buffer[i] = (uint8_t) (rand() % amount_colors); //uncompressed data: 1,1,1,1,3,3,3,3,2,2,2,2,...
    }
    size_t compressed_size_block = 0;
    //Algorithm LZW for compression dry data
    size_t pos_for_size_of_block = (gif->pos);
    (gif->pos)++;
    gif_lzw_compress(uncompressed_buffer, uncompressed_size_block, (gif->data) + (gif->pos), &compressed_size_block, gif->lzw_minimum_code_size);
    //writes to buffer compressed image data.
    gif->pos = gif->pos + compressed_size_block; //set to new pos
    (gif->data)[pos_for_size_of_block] = (uint8_t )compressed_size_block; //write size of image data block before data block
}

//Generates sequence for settings of Program Block,Animation Block,Text BLock and Other Extensions of 89a version
//I take them like pattern from https://www.matthewflickinger.com/lab/whatsinagif/bits_and_bytes.asp

static void generate_gif_graphics_control_extension(gif_object_t *gif)
{
    const uint8_t extension[] = {0x21, 0xF9, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00};
    memcpy(gif->data, extension, sizeof(extension));
    gif->pos += sizeof(extension); 
}

static void generate_gif_plain_text_extension(gif_object_t *gif)
{
    const uint8_t extension[] = {0x21, 0x01, 0x0C, 0x00, 0x00, 0x00, 0x00, 
                                 0x64, 0x00, 0x64, 0x00, 0x14, 0x14, 0x01, 
                                 0x00, 0x0B, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 
                                 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x00};
    memcpy(gif->data, extension,  sizeof(extension));
    gif->pos += sizeof(extension); 
}

static void generate_gif_application_extension(gif_object_t *gif)
{
    const uint8_t extension[] = {0x21, 0xFF, 0x0B, 0x4E, 0x45, 0x54, 
                                 0x53, 0x43, 0x41, 0x50, 0x45, 0x32, 
                                 0x2E, 0x30, 0x03, 0x01, 0x05, 0x00, 0x00};
    memcpy(gif->data, extension,  sizeof(extension));
    gif->pos += sizeof(extension); 
}

static void generate_gif_comment_extension(gif_object_t *gif)
{
    const uint8_t extension[] = {0x21, 0xFE, 0x09, 0x62, 0x6C, 0x75, 
                                 0x65, 0x62, 0x65, 0x72, 0x72, 0x79, 0x00};
    memcpy(gif->data, extension,  sizeof(extension));
    gif->pos += sizeof(extension); 
}


