#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "geometry.h"
#include "raytracing.h"


scene* load_scene(FILE* obj_file){
    /* ne fonctionne qu'avec des triangles (attention le format .obj usuel accepte des quadrilatères) */
    char* buffer = malloc(1024*sizeof(char));
    int n_vertices = 0;
    int n_triangles = 0;
    
    // Comptage du nombre de points et de triangles
    while (fgets(buffer, 1024, obj_file)){
        if (buffer[0] == 'v' && buffer[1] == ' '){
            n_vertices++;
        } else if (buffer[0] == 'f'){
            n_triangles++;
        }
    }

    // Création des tableaux de stockage
    vector* vertices = malloc(n_vertices*sizeof(vector));
    triangle* triangles = malloc(n_triangles*sizeof(triangle));

    rewind(obj_file);

    int v_i = 0;
    int t_i = 0;

    char* ptr;

    double absorbtion_coeff = 0;
    double reflexion_coeff = 1;
    double solid_diff_coeff = 0;
    double trans_diff_coeff = 0;

    while (fgets(buffer, 1024, obj_file)){
        //On parse le fichier pour en tirer les coordonnées des points
        if (buffer[0] == 'v' && buffer[1] == ' '){
            vertices[v_i].x = strtof(buffer+2, &ptr);
            vertices[v_i].y = strtof(ptr+1, &ptr);
            vertices[v_i].z = strtof(ptr+1, &ptr);
            v_i++;
        } else if (buffer[0] == 'f'){
            // On associe les points à leurs triangles respectifs
            triangles[t_i].a = vertices[strtol(buffer+2, &ptr, 10)-1];
            triangles[t_i].b = vertices[strtol(ptr+1, &ptr, 10)-1];
            triangles[t_i].c = vertices[strtol(ptr+1, &ptr, 10)-1];
            triangles[t_i].n = normalize(cross_product(vector_diff(triangles[t_i].a, triangles[t_i].b),vector_diff(triangles[t_i].a, triangles[t_i].c)));
            // on attribue les propriétés optiques au triangle
            triangles[t_i].absorbtion_coeff = absorbtion_coeff;
            triangles[t_i].reflexion_coeff = reflexion_coeff;
            triangles[t_i].solid_diff_coeff = solid_diff_coeff;
            triangles[t_i].trans_diff_coeff = trans_diff_coeff;
            t_i++;
        } else if (buffer[0] == '#' && buffer[1] == 'p' && buffer[2] == 'p'){
            // en cas de balise #pp on modifie les propriétées du matériaux
            absorbtion_coeff = strtof(buffer+4, &ptr);
            reflexion_coeff = strtof(ptr+1, &ptr);
            solid_diff_coeff = strtof(ptr+1, &ptr);
            trans_diff_coeff = strtof(ptr+1, &ptr);
        }
    }
    scene* s = malloc(sizeof(scene));
    s->n_triangles = n_triangles;
    s->triangles = triangles;
    fprintf(stderr, "Fin du chargement d'une scène composée de %d triangles.\n", s->n_triangles);
    free(vertices);
    free(buffer);
    return s;
}


ray** simulate_ray(ray* r, scene* s, int n_max){
    ray** path = malloc((n_max+1)*sizeof(ray));
    for (int i=0; i<n_max+1; i++){
        path[i] = NULL;
    }
    path[0] = r;
    // Tableaux notant les collisions et la distance du rayon à tous les triangles rencontrés
    vector* collision;
    double* distances = malloc(sizeof(double) * s->n_triangles);
    int* collided_triangles = malloc(sizeof(int) * s->n_triangles);
    int last_triangle = -1;

    /* On simule n-1 réflexions */
    for (int i = 0; i<n_max-1; i++){
        /* Pour chaque nouvelle réflexion on teste la collision avec chaque triangle de la scène
        On enregistre les distances des points de réflexions pour ne garder que le plus proche */
        int c = 0;
        for (int j=0; j<s->n_triangles; j++){
            if (j != last_triangle){
                collision = intersect(path[i], s->triangles+j);
                if (collision != NULL){
                    distances[c] = distance(path[i]->origin, *collision);
                    collided_triangles[c] = j;
                    free(collision);
                    c++;
                }
            }
        }


        /* Dans le cas où le rayon ne rencontre aucun triangle on met fin à la simulation */
        if (0 == c){
            // fprintf(stderr, "%s", "no colision\n");
            break;
        }

        /* On trouve le point d'intersection le plus proche mais non nul */
        int min_i = 0;

        for (int j = min_i; j<c; j++){
            if (distances[j] < distances[min_i] && distances[j] > 0){
                min_i = j;
            }
        }

        /* On note le triangle correspondant comme étant le dernier percuté */
        last_triangle = collided_triangles[min_i];

        /* On ajoute au chemin le nouveau rayon absorbé, réfléchi ou diffusé
        Dans le cas de l'absorbtion la simulation est arrếtée */
        double x = (double) rand() / RAND_MAX;
        if (x < s->triangles[last_triangle].absorbtion_coeff){
            // cas de l'absorbtion  
            break;
        } else if (x < s->triangles[last_triangle].reflexion_coeff + s->triangles[last_triangle].absorbtion_coeff){
            // cas de la réfléxion parfaite
            path[i+1] = reflect(path[i], s->triangles+last_triangle);
        } else if (x < s->triangles[last_triangle].reflexion_coeff +
                        s->triangles[last_triangle].absorbtion_coeff +
                        s->triangles[last_triangle].solid_diff_coeff){
            // cas de la diffusion solide
            path[i+1] = diffuse_solid(path[i], s->triangles+last_triangle);
        } else {
            // cas de la diffusion translucide
            path[i+1] = diffuse_translucent(path[i], s->triangles+last_triangle);
        }
    }
    
    free(distances);
    free(collided_triangles);

    return path;
}


void paths_to_obj(FILE* f, ray*** paths){
    int vertex_count = 1;
    int n;
    vector end;
    /* On itère à travers le tableau de chemins */
    for (int i = 0; paths[i] != NULL; i++){
        int j;
        /* On trouve la longueur du chemin */
        for (n = 0; paths[i][n] != NULL; n++);
        /* On ignore les chemins ne comportant que le rayon d'origine */
        if (n>1){
            /* Ecriture de chaque point de la trajectoire dans le fichier */
            for (j = 0; paths[i][j] != NULL; j++){
                fprintf(f, "v %lf %lf %lf\n", paths[i][j]->origin.x, paths[i][j]->origin.y, paths[i][j]->origin.z);
            }
            /* On ajoute un point supplémentaire pour visualiser la direction du dernier rayon */
            end = vector_sum(paths[i][j-1]->origin, normalize(paths[i][j-1]->direction));
            fprintf(f, "v %lf %lf %lf\n", end.x, end.y, end.z);
            /* On écrit la trajectoire
            Le compteur vertex_count permet de reprendre le numérotation au bon endroit */
            fprintf(f, "%s", "\nl");
            for (int k = 0; k < j+1; k++){
                fprintf(f, " %d", vertex_count + k);
            }
            fprintf(f, "%s", "\n\n");
            vertex_count += j + 1;
        }
    }
}

uint8_t** render_scene(scene* s, int width, int height, double horizontal_fov, int rays_per_pixel, int max_reflexions){
    ray r;
    ray** path;
    double lum;
    uint8_t** pixels = malloc(height * sizeof(uint8_t*));

    // Classique arctan(y/x) pour trouver les angles de la caméra en coordonnées polaires
    double cam_horiz_angle;
    if (s->camera.direction.x > 0){
        cam_horiz_angle = atan(s->camera.direction.y/s->camera.direction.x);
    } else if (s->camera.direction.x < 0){
        cam_horiz_angle = M_PI + atan(s->camera.direction.y/s->camera.direction.x);
    } else {
        if (s->camera.direction.y >= 0){
            cam_horiz_angle = M_PI/2;
        } else {
            cam_horiz_angle = - M_PI/2;
        }
    }
    double cam_vert_angle;
    double h_comp = sqrt(s->camera.direction.x*s->camera.direction.x + s->camera.direction.y*s->camera.direction.y);
    if (h_comp > 0){
        cam_vert_angle = atan(s->camera.direction.z/h_comp);
    } else {
        if (s->camera.direction.z >= 0){
            cam_vert_angle = M_PI/2;
        } else {
            cam_vert_angle = - M_PI/2;
        }
    }

    /* Dimensions du rectangle de projection de la caméra (en unités de l'espace) */
    double window_length = 2*tan(horizontal_fov/2);
    double window_height = (double) height / width * window_length;

    int n;
    
    // Génération du bitmap ligne par ligne
    for (int i = 0; i < height; i++){
        pixels[i] = malloc(width * sizeof(uint8_t));
        // fprintf(stderr, "Ligne %d sur %d\n", i, height);
        // Génération des rayons de manière uniforme sur l'aire donnée
        for (int j = 0; j < width; j++){
            r.origin = s->camera.origin;
            r.direction.x = 1;
            r.direction.y = - ((double) j/width - 0.5) * window_length;
            r.direction.z = - ((double) i/height - 0.5) * window_height;

            // rotation verticale autour de l'axe y

            double new_x = cos(cam_vert_angle) * r.direction.x - sin(cam_vert_angle) * r.direction.z;
            double new_z = cos(cam_vert_angle) * r.direction.z + sin(cam_vert_angle) * r.direction.x;

            r.direction.x = new_x;
            r.direction.z = new_z;

            // rotation horizontale autour de l'axe z

            new_x = cos(cam_horiz_angle) * r.direction.x - sin(cam_horiz_angle) * r.direction.y;
            double new_y = cos(cam_horiz_angle) * r.direction.y + sin(cam_horiz_angle) * r.direction.x;

            r.direction.x = new_x;
            r.direction.y = new_y;

            double total = 0;


            for (int k=0; k<rays_per_pixel; k++){
                path = simulate_ray(&r, s, max_reflexions);
                // On trouve le dernier rayon du chemin
                n = 0;
                while (path[n] != NULL){n++;}
                /* On calcule la luminosité. 
                Si le chemin est de longueur max_reflections, on considère que le rayon n'a pas abouti
                et que la luminosité est nulle. */
                if (n == max_reflexions){
                    lum = 0;
                } else {
                    lum = - dot_product(normalize(s->lighting_direction), normalize(path[n-1]->direction));
                    /* Un plancher nul est appliqué à la luminosité */
                    if(lum < 0){
                        lum = 0;
                    }
                }

                // constante de la distribution de diffusion, on considère que le ciel diffuse en cos^e
                double e = 1;
                total += pow(lum, e);
                
                // Libération de la mémoire
                for (int i=1; i<n; i++){
                    free(path[i]);
                }
                free(path);
            }
            pixels[i][j] = (uint8_t) (total / rays_per_pixel * 255);
        }
    }
    return pixels;
}
