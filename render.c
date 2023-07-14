#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "geometry.h"
#include "raytracing.h"
#include "bitmap.h"


/*
Ce programme permet de générer un rendu bitmap d'un fichier .obj passé en ligne de commande.

On lance test nomFichier.obj resX resY h_fov max_reflexions iterations_per_pixel

Malheureusement la position de la caméra et l'orientation de la lumière sont hardcodés par flemme
*/

int main(int argc, char *argv[]){
    assert (argc==7);
    FILE* f = fopen(argv[1], "r");
    scene* s = load_scene(f);

    fclose(f);
    
    int resX, resY, max_ref, iterations;
    sscanf(argv[2], "%d", &resX);
    sscanf(argv[3], "%d", &resY);
    sscanf(argv[5], "%d", &max_ref);
    sscanf(argv[6], "%d", &iterations);
    double h_fov;
    sscanf(argv[4], "%lf", &h_fov);

    // Mise en place de la caméra et du plan d'illumination

    s->camera.origin = (vector) {4, 0, 0};

    s->camera.direction = (vector) {-1, 0, 0};

    s->lighting_direction = (vector) {-1, 0, -1};

    srand(time(NULL));


    clock_t begin = clock();

    uint8_t** pixels = render_scene(s, resX, resY, h_fov / 180 * M_PI, iterations, max_ref);

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    fprintf(stderr, "Temps : %lf\n", time_spent);


    char* path = malloc(128*sizeof(char));
    time_t t;
    time(&t);
    sprintf(path, "renders/render_%s.bmp", ctime(&t));

    f = fopen(path, "wb");

    bitmap_write(f, pixels, resX, resY);

    fclose(f);

    free(path);

    free(s->triangles);
    free(s);


    for (int i = 0; i < resY; i++){
        free(pixels[i]);
    }

    free(pixels);

    return 0;
}
