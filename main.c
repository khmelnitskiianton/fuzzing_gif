/*

    CPP FOR DEBUGGING GIF GENERATOR AND GIF ENCODING

*/


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "gif_lzw_encode.h"
#include "gif_generator.h"

const size_t MAX_SIZE = 200000;                 //max size of generated gif-file

int main(int argc, char **argv)
{
    srand(time(0));
    //uint8_t buff[]= 
    //{1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 
    //2,2,2,0,0,0,0,1,1,1,2,2,2,0,0,0,0,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1, 2,2,2,2,2,1,1,1,1,1
    //};
    //uint8_t buff1[200] = {};
    //size_t size_buff1 = 0;
    //gif_lzw_compress(buff, 100, buff1, &size_buff1, 2);
    //for (int i = 0; i < size_buff1; i++)
    //{
    //    printf("%02x ", buff1[i]);
    //}
    //printf("\n");
    //printf("Size %02x\n", size_buff1);
    system("touch generated.gif");
    FILE *file_ptr = fopen("generated.gif", "wb");
    gif_object_t gif = {};
    //init action
    uint8_t buffer[MAX_SIZE] = "";    //big buffer
    gif.data = buffer;
    gif.pos  = 0;
    generate_gif(&gif);
    fwrite(buffer, sizeof(uint8_t), gif.pos, file_ptr);
    fclose(file_ptr);

    return 0;
}