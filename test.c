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


/* Ce programme permet de simuler des rayons incidents arrivant sur un modèle.obj
et de générer un second fichier.obj représentant les trajectoires de ces rayons

Une implémentation en ligne de commande serait bien plus ergonomique mais les paramètres sont trop nombreux,
et je n'ai pas la foi d'implémenter un système de fichiers config */


int main(){
    FILE* read_f = fopen("models/horiz.obj", "r");
    scene* s = load_scene(read_f);

    /*
    fprintf(stderr, "%lf\n", s->triangles[0].absorbtion_coeff);
    fprintf(stderr, "%lf\n", s->triangles[0].reflexion_coeff);
    fprintf(stderr, "%lf\n", s->triangles[0].diffusion_coeff);
    */

    fclose(read_f);
    int N = 1000;
    ray* rays = malloc(N*sizeof(ray));
    ray*** paths = malloc((N+1)*sizeof(ray**));
    paths[N] = NULL;
    // les rayons sont générés avec une origine placée aléatoirement dans un rectangle et une direction fixe
    for (int i=0; i<N; i++){
        // mode plan
        //rays[i].origin = (vector) {-2, ((double) rand()/RAND_MAX-0.5)*0.5, 4+((double) rand()/RAND_MAX-0.5) * 0.5};
        //rays[i].direction = (vector) {1, 0, -1};
        rays[i].origin = (vector) {((double) rand()/RAND_MAX-0.5)*0.5, -6, 1.75-((double) rand()/RAND_MAX) * 0.25};
        rays[i].direction = (vector) {0, 2, -1};

        /*
        // mode sphere
        rays[i].origin = (vector) {-2,0,2};
        rays[i].direction = random_vect();
        
        // mode ponctuel
        rays[i].origin = (vector) {-2,0,2};
        rays[i].direction = (vector) {1, 0, -1};

        */

        paths[i] = simulate_ray(rays+i, s, 100);
    }

    FILE* write_f = fopen("renders/trajectories.obj", "w");
    paths_to_obj(write_f, paths);

    // nettoyage

    free(rays);
    for (int i = 0; i<N; i++){
        for (int j = 1; paths[i][j] != NULL; j++){
            free(paths[i][j]);
        }
        free(paths[i]);
    }
    free(paths);

    free(s->triangles);
    free(s);

}