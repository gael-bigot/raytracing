#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "geometry.h"
#include "raytracing.h"


scene* load_scene(FILE* obj_file){
    char* buffer = malloc(1024*sizeof(char));
    int n_vertices = 0;
    int n_triangles = 0;
    
    while (fgets(buffer, 1024, obj_file)){
        if (buffer[0] == 'v' && buffer[1] == ' '){
            n_vertices++;
        } else if (buffer[0] == 'f'){
            n_triangles++;
        }
    }

    fprintf(stderr, "%d", n_vertices);

    vector* vertices = malloc(n_vertices*sizeof(vector));
    triangle* triangles = malloc(n_triangles*sizeof(triangle));

    

    rewind(obj_file);

    int v_i = 0;
    int t_i = 0;

    char* ptr;

    while (fgets(buffer, 1024, obj_file)){
        // fprintf(stderr, "%s", buffer);
        if (buffer[0] == 'v' && buffer[1] == ' '){
            vertices[v_i].x = strtof(buffer+2, &ptr);
            vertices[v_i].y = strtof(ptr+1, &ptr);
            vertices[v_i].z = strtof(ptr+1, &ptr);
            v_i++;
        } else if (buffer[0] == 'f'){
            triangles[t_i].a = vertices[strtol(buffer+2, &ptr, 10)-1];
            strtol(ptr+1, &ptr, 10);
            strtol(ptr+1, &ptr, 10);
            triangles[t_i].b = vertices[strtol(ptr+1, &ptr, 10)-1];
            strtol(ptr+1, &ptr, 10);
            strtol(ptr+1, &ptr, 10);
            triangles[t_i].c = vertices[strtol(ptr+1, &ptr, 10)-1];
            t_i++;
        }
    }

    scene* s = malloc(sizeof(scene));
    s->n_triangles = n_triangles;
    s->triangles = triangles;

    free(vertices);

    fprintf(stderr, "%s", "yay");

    return s;

}





ray** simulate_ray(ray* r, scene* s, int n){
    ray** path = malloc(n*sizeof(ray));
    for (int i=0; i<n; i++){
        path[i] = NULL;
    }
    path[0] = r;

    ray** collisions = malloc(sizeof(ray*) * s->n_triangles);
    double* distances = malloc(sizeof(double) * s->n_triangles);

    /* On simule n-1 réflexions */
    for (int i = 0; i<n-1; i++){
        /* Pour chaque nouvelle réflexion on teste la collision avec chaque triangle de la scène
        On enregistre les distances des points de réflexions pour ne garder que le plus proche */
        int c = 0;
        for (int j=0; j<s->n_triangles; j++){
            collisions[c] = reflect(r, s->triangles+j);
            if (collisions[c] != NULL){
                distances[c] = distance(path[i]->origin, collisions[c]->origin);
                c++;
            }
        }

        /* Dans le cas où le rayon ne rencontre aucun triangle on met fin à la simulation */
        if (0 == c){
            break;
        }

        /* On trouve le point d'intersection le plus proche */
        int min_i = 0;
        for (int j = 1; j<c; j++){
            if (distances[j] < distances[min_i]){
                min_i = j;
            }
        }

        /* On ajoute au chemin le nouveau rayon */
        path[i+1] = collisions[min_i];

        /* On libère les collisions ignorées */
        for (int j = 0; j<c; j++){
            if (j != min_i && collisions[j] != NULL){
                free(collisions[j]);
            }
        }
    }
    free(distances);
    free(collisions);

    return path;
}


uint8_t** render_scene(scene* s, int width, int height, double horizontal_fov, int max_reflexions){
    ray r;
    ray** path;
    int n = 0;
    double lum;
    double theta;
    double phi;

    uint8_t** pixels = malloc(height * sizeof(uint8_t*));


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
    if (s->camera.direction.x > 0){
        cam_vert_angle = atan(s->camera.direction.z/s->camera.direction.x);
    } else if (s->camera.direction.x < 0){
        cam_vert_angle = M_PI + atan(s->camera.direction.z/s->camera.direction.x);
    } else {
        if (s->camera.direction.z >= 0){
            cam_vert_angle = M_PI/2;
        } else {
            cam_vert_angle = - M_PI/2;
        }
    }



    double window_length = 2*tan(horizontal_fov/2);
    double window_height = (double) height / width * window_length;



    for (int i = 0; i < height; i++){
        pixels[i] = malloc(width * sizeof(uint8_t));
        for (int j = 0; j < width; j++){
            r.origin = s->camera.origin;
            r.direction.x = 1;
            r.direction.y = - ((double) j/width - 0.5) * window_length;
            r.direction.z = - ((double) i/height - 0.5) * window_height;


            // vertical rotation

            double new_x = cos(cam_vert_angle) * r.direction.x - sin(cam_vert_angle) * r.direction.z;
            double new_z = cos(cam_vert_angle) * r.direction.z + sin(cam_vert_angle) * r.direction.x;

            r.direction.x = new_x;
            r.direction.z = new_z;


            // horizontal rotation

            new_x = cos(cam_horiz_angle) * r.direction.x - sin(cam_horiz_angle) * r.direction.y;
            double new_y = cos(cam_horiz_angle) * r.direction.y + sin(cam_horiz_angle) * r.direction.x;

            r.direction.x = new_x;
            r.direction.y = new_y;

            
            path = simulate_ray(&r, s, max_reflexions);

            n = 0;
            while (path[n] != NULL){n++;}

            lum = - scalar_product(normalize(s->lighting_direction), normalize(path[n-1]->direction));
        

            if (lum<0){
                lum = 0;
            }

            pixels[i][j] = (uint8_t) (lum * 255);

            for (int i=1; i<max_reflexions; i++){
                if (path[i] != NULL){
                    free(path[i]);
                }
            }

            free(path);
        }
    }
    return pixels;
}