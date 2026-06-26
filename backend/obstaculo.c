#include "obstaculo.h"
#include <stdlib.h>

struct Obstaculo_t {
    /* Posicion */
    double x;                   // Posicion x (actual)
    double y;                   // Posicion y (actual)

    /* Dimensiones */
    DimensionObstaculo_t dim;

    /* Direccion y rapidez de movimiento */
    Velocidad_t vel;

    /* Tipo de obstaculo */
    TipoObstaculo_t tipo;
};

Obstaculo nuevo_obstaculo(Coordenada_t coord_inicial, DimensionObstaculo_t dim, Velocidad_t vel, TipoObstaculo_t tipo) {
    Obstaculo o = malloc(sizeof(struct Obstaculo_t));
    if (o != NULL) {
        o->x = (double) coord_inicial.x;
        o->y = (double) coord_inicial.y;
        o->dim = dim;
        o->vel = vel;
        o->tipo = tipo;
    }
    return o;
}

void actualizar_posicion(Obstaculo o) {
    switch (o->vel.dir){
    case DIR_ABAJO:
        o->y += o->vel.rapidez;
        break;
    case DIR_ARRIBA:
        o->y -= o->vel.rapidez;
        break;
    case DIR_DERECHA:
        o->x += o->vel.rapidez;
        break;
    case DIR_IZQUIERDA:
        o->x -= o->vel.rapidez;
        break;
    default:
        break;
    }
}

/*************************************************/
/*** FUNCIONES DE CONSULTA QUE USA EL FRONTEND ***/
/*************************************************/

Coordenada_t obtener_posicion(Obstaculo o) {
    Coordenada_t coord = {
        .x = (int) o->x,
        .y = (int) o->y
    };
    return coord;
}

DimensionObstaculo_t obtener_dimension(Obstaculo o) {
    return o->dim;
}

TipoObstaculo_t obtener_tipo(Obstaculo o) {
    return o->tipo;
}
