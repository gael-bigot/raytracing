#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "bitmap.h"

// Permet d'écrire le reste de la division euclidienne d'un entier quelconque par 256 dans un fichier
void write_int(FILE* file, int x, int bytes){
    uint8_t r;
    for (int i = 0; i< bytes; i++){
        r = x%256;
        fwrite(&r, 1, 1, file);
        x /= 256;
    }
}

void bitmap_write(FILE* file, uint8_t** pixels, int width, int height){
    int padding;
    int datasize;
    if (3*width % 4 == 0){
        padding = 0;
    } else {
        padding = 4 - (3*width)%4;
    }
    datasize = height*(3*width+padding);
    
    //On annonce que c'est un bitmap
    fwrite("BM", 2, 1, file);

    // file size
    write_int(file, 54+datasize, 4);
    // unused
    write_int(file, 0, 4);
    // pixel map offset
    write_int(file, 54, 4);
    // size of DIB header
    write_int(file, 40, 4);
    // width
    write_int(file, width, 4);
    // height
    write_int(file, height, 4);
    // layers
    write_int(file, 1, 2);
    // bits per pixel (3*8)
    write_int(file, 24, 2);
    // mode rgb
    write_int(file, 0, 4);
    // size of raw bitmap data (including padding)
    write_int(file, datasize, 4);
    // print resolution (useless)
    write_int(file, 0, 8);
    // palette (useless)
    write_int(file, 0, 8);

    // écriture des données des pixels
    for (int i = height-1; i>=0; i--){
        for (int j = 0; j<width; j++){
            write_int(file, pixels[i][j], 1);
            write_int(file, pixels[i][j], 1);
            write_int(file, 0, 1);
        }
        write_int(file, 0, padding);
    }
}
