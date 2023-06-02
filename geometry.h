#ifndef __GEOMETRY_H
#define __GEOMETRY_H

/* Type représentant un vecteur de R^3 sous la forme de trois coordonnées */
typedef struct vector_s{
    double x;
    double y;
    double z;
} vector;

/* Type représentant un triangle sous la forme de trois points, ainsi que des proprietees optiques */
typedef struct triangle_s{
    vector a;
    vector b;
    vector c;
    vector n;
    double absorbtion_coeff;
    double reflexion_coeff;
    double solid_diff_coeff;
    double trans_diff_coeff;
} triangle;


/* Type représentant un rayon sous la forme d'une demi droite (origine et vecteur drecteur) */
typedef struct ray_s{
    vector origin;
    vector direction;
} ray;

/* Renvoie la norme du vecteur v */
double length(vector v);

/* Renvoie v normalisé */
vector normalize(vector v);

/* Renvoie la distance entre u et v */
double distance(vector u, vector v);

/* Renvoie la somme de deux vecteurs */
vector vector_sum(vector u, vector v);

/* Renvoie la différence entre le vecteur u et le vecteur v */
vector vector_diff(vector u, vector v);

/* Renvoie le vecteur v multiplié par un scalaire a */
vector scal_product(vector v, double a);

/* Renvoie le produit scalaire de u et v */
double dot_product(vector u, vector v);

/* Renvoie le produit vectoriel de u et v */
vector cross_product(vector u, vector v);

/* Renvoie un vecteur unitaire aléatoire */
vector random_vect();

/* Renvoie, s'il existe, le point d'intersection entre une demie droite (rayon) et un triangle
Renvoie NULL si l'intersection n'existe pas */
vector* intersect(ray* r, triangle* t);

/* Renvoie, s'il existe, le rayon réfléchi à partir du rayon incident et d'un triangle 
Renvoie NULL si l'intersection n'existe pas */
ray* reflect(ray* r, triangle* t);

/* Renvoie en cas de collision un rayon réfléchi dans une direction aléatoire du côté du rayon incident */
ray* diffuse_solid(ray* r, triangle* t);

/* Renvoie en cas de collision un rayon réfléchi dans une direction aléatoire */
ray* diffuse_translucent(ray* r, triangle* t);


#endif
