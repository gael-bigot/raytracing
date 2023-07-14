#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "geometry.h"


double length(vector v){
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

vector vector_sum(vector u, vector v){
    vector res;
    res.x = u.x + v.x;
    res.y = u.y + v.y;
    res.z = u.z + v.z;
    return res;
}

vector vector_diff(vector u, vector v){
    vector res;
    res.x = u.x - v.x;
    res.y = u.y - v.y;
    res.z = u.z - v.z;
    return res;
}

double distance(vector u, vector v){
    return length(vector_diff(u, v));
}

vector normalize(vector v){
    double l = length(v);
    vector res;
    res.x = v.x/l;
    res.y = v.y/l;
    res.z = v.z/l;
    return res;
}

vector scal_product(vector v, double a){
    return (vector) {v.x*a, v.y*a, v.z*a};
}

double dot_product(vector u, vector v){
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

vector cross_product(vector u, vector v){
    vector res;
    res.x = u.y * v.z - u.z * v.y;
    res.y = v.x * u.z - v.z * u.x;
    res.z = u.x * v.y - u.y * v.x;
    return res;
}

vector random_vect(){
    vector res;
    /* la génération d'un vecteur aléatoire se fait en choisissant les deux angles
    le caractérisant en coordonnées polaires de manière uniforme */
    double theta = (double) rand() / RAND_MAX * 2 * M_PI;
    double phi = (double) rand() / RAND_MAX * M_PI;
    res.x = sin(phi)*cos(theta);
    res.y = sin(phi)*sin(theta);
    res.z = cos(phi);
    return res;
}

vector* intersect(ray* r, triangle* t){
    /* L'équation du plan est sous la forme (n|v) = cste, cette constante est égale à (a|n)
    En notant la droite associée au rayon A + tu, on souhaite (A + ku | n) = (a|n)
    ie (A|n) + k(u|n) = (a|n)
    ou encore
    k = ( (a|n) - (A|n) ) / (u|n)
    Le cas (u|n) = 0 (rayon tangent au plan) renverra NULL sans autre forme de procès
    */
    double s = dot_product(r->direction, t->n);

    if (0 == s){
        return NULL;
    }

    double k = (dot_product(t->a, t->n) - dot_product(r->origin, t->n)) / s;

    /* Si k est négatif, le point d'intersection se situe du mauvais côté de la demie-droite
    Si k est nul, l'origine du rayon est confondu avec le point d'intersection, on considère qu'il n'y a pas réflexion puisque le rayon part du triangle 
    */
    if (k<=0){
        return NULL;
    }

    /* On obtient enfin le point d'intersection avec le plan contenant le triangle */
    vector* p = malloc(sizeof(vector));
    p->x = r->origin.x + k * r->direction.x;
    p->y = r->origin.y + k * r->direction.y;
    p->z = r->origin.z + k * r->direction.z;
    
    /* Il reste à vérifier que le point si situe à l'intérieur du triangle
    Pour cela, on vérifie que AP se situe entre AB et AC (produits vectoriels de signe opposé) puis que BP se situe entre BA et BC
    */
    vector AP = vector_diff(*p, t->a);
    vector AB = vector_diff(t->b, t->a);
    vector AC = vector_diff(t->c, t->a);

    if (dot_product(cross_product(AP, AB), cross_product(AP, AC)) > 0){
        free(p);
        return NULL;
    }

    vector BP = vector_diff(*p, t->b);
    vector BC = vector_diff(t->c, t->b);

    /* Sachant BA = -AB, la condition est renversée */
    if (dot_product(cross_product(BP, AB), cross_product(BP, BC)) < 0){
        free(p);
        return NULL;
    }

    return p;
}

ray* reflect(ray* r, triangle* t){
    vector* p = intersect(r, t);
    if (NULL == p){
        return NULL;
    }
    
    // on calcule la composante normale du vecteur incident
    double normal_component = dot_product(t->n, r->direction);

    /* on inverse la composante normale pour créer le rayon réfléchi, suivant la formule
    i = r - 2i.n */
    vector new_dir;
    new_dir.x = r->direction.x - 2*(t->n).x*normal_component;
    new_dir.y = r->direction.y - 2*(t->n).y*normal_component;
    new_dir.z = r->direction.z - 2*(t->n).z*normal_component;

    ray* res = malloc(sizeof(ray));

    res->origin = *p;
    res->direction = new_dir;

    free(p);

    return res;
}

ray* diffuse_solid(ray* r, triangle* t){
    vector* p = intersect(r, t);

    if (NULL == p){
        return NULL;
    }

    ray* res = malloc(sizeof(ray));

    res->origin = *p;

    vector e1, e2, e3;

    /* on crée une base adaptée au plan du triangle */
    /* le vecteur e1 est normal au triangle et pointe du cate du rayon incident */
    if (dot_product(r->direction, t->n) < 0){
        e1 = t->n;
    } else {
        e1 = (vector) {-t->n.x, -t->n.y, -t->n.z};
    }
    // on crée un vecteur normal à e1 grâce à un produit vectoriel avec un vecteur aléatoire
    e2 = normalize(cross_product(e1, random_vect()));
    // le troisième vecteur est obtenu par produit vectoriel entre les deux précédents
    e3 = cross_product(e1, e2);

    double phi, theta;

    // phi est choisi uniformément entre 0 et 2Pi
    phi = (double) rand()/RAND_MAX * 2*M_PI;
    // theta est choisi entre 0 et PI/2 suivant la loi de Lambert
    theta = asin((double) rand()/RAND_MAX);

    res->direction = vector_sum(scal_product(e1, cos(theta)), vector_sum(scal_product(e2, sin(theta)*cos(phi)), scal_product(e3, sin(theta)*sin(phi))));    
    free(p);

    return res;
}

ray* diffuse_translucent(ray* r, triangle* t){
    /* Code identique à diffuse_solid, à la différence que le rayon réfléchi
    n'a pas de contrainte pour la direction */
    vector* p = intersect(r, t);

    if (NULL == p){
        return NULL;
    }

    ray* res = malloc(sizeof(ray));

    res->origin = *p;

    vector e1, e2, e3;

    /* on crée une base adaptée au plan du triangle
    le vecteur e1 est normal au triangle, dans une direction aléatoire (multiplication de n par 1 ou -1) */
    e1 = scal_product(t->n, (rand()%2)*2-1);
    // on crée un vecteur normal à e1 grâce à un produit vectoriel avec un vecteur aléatoire
    e2 = normalize(cross_product(e1, random_vect()));
    // le troisième vecteur est obtenu par produit vectoriel entre les deux précédents
    e3 = cross_product(e1, e2);

    double phi, theta;

    // phi est choisi uniformément entre 0 et 2Pi
    phi = (double) rand() / RAND_MAX * 2 * M_PI;
    // theta est choisi entre 0 et PI/2 suivant la loi de Lambert
    theta = asin((double) rand()/RAND_MAX);

    res->direction = vector_sum(scal_product(e1, cos(theta)), vector_sum(scal_product(e2, sin(theta)*cos(phi)), scal_product(e3, sin(theta)*sin(phi))));    
    free(p);

    return res;
}