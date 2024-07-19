#ifndef GIF_GENERATOR_H
#define GIF_GENERATOR_H

#define SIZE_OF_UNC_DATA                                                                                               \
    0x300 // Size of uncompressed data,
          // 0x2A0 after compression <= 250 - ideal for image with one block [0, 255] bytes

// Struct of gif object information
typedef struct gif_object
{
    uint8_t *data;  // ptr to begin of file buffer
    size_t pos;     // current position in file buffer
    uint16_t width; // size of image
    uint16_t height;
    uint8_t lzw_minimum_code_size; // for lzw compression
} gif_object_t;

// Main function of gif generator
void generate_gif(gif_object_t *gif);
void generate_gif_image_descriptor(gif_object_t *gif, uint32_t uncompressed_size_block);
void generate_gif_graphics_control_extension(gif_object_t *gif);
void generate_gif_plain_text_extension(gif_object_t *gif);
void generate_gif_application_extension(gif_object_t *gif);
void generate_gif_comment_extension(gif_object_t *gif);

#endif