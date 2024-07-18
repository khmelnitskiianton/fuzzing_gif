#ifndef GIF_GENERATOR_H
#define GIF_GENERATOR_H

//Struct of gif object information
typedef struct gif_object {
    uint8_t  *data;    //ptr to begin of file buffer
    size_t   pos;      //current position in file buffer
    uint16_t width;    //size of image
    uint16_t height;
    uint8_t  lzw_minimum_code_size; //for lzw compression
} gif_object_t;

//Main function of gif generator
void generate_gif(gif_object_t *gif);

#endif