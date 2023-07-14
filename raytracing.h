#ifndef __RAYTRACING_H
#define __RAYTRACING_H

typedef struct scene_s{
    ray camera;
    vector lighting_direction;
    int n_triangles;
    triangle* triangles;
} scene;

/* Charge une scène depuis un fichier .obj
Seuls les modèles triangulés et sans normales/textures sont compatibles */
scene* load_scene(FILE* obj_file);

/* Simule n réflexions du rayon r dans la scene s et renvoie le tableau des rayons successifs de la trajectoire, terminé par NULL */
ray** simulate_ray(ray* r, scene* s, int n);

/* Ecrit des trajectoires de rayon dans un fichier .obj */
void paths_to_obj(FILE* f, ray*** paths);

/* Génère le rendu de la scène s sous la forme d’un tableau d’entiers 8 bits de taille height*width */
uint8_t** render_scene(scene* s, int width, int height, double horizontal_fov, int rays_per_pixel, int max_reflexions);

#endif
