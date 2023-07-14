#ifndef __BITMAP_H
#define __BITMAP_H

/* Ecrit un tableau d'entier 8 bits dans un fichier au format bitmap/
Seul le monochrome est implémenté */
void bitmap_write(FILE* file, uint8_t** pixels, int width, int height);

#endif
