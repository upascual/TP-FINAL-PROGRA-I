/******************************************************************************
 * niveles.c
 *****************************************************************************/
#include "niveles.h"

/* Descripción de un carril en el nivel 1 */
typedef struct {
    int fila;
    tipo_obstaculo_t tipo;
    int variante;     /* Sprite de auto (0 a 3), ignorado en los demás tipos */
    int largo;        /* En celdas                                          */
    int cantidad;     /* Cuántos obstáculos hay en el carril                */
    int sentido;      /* +1 derecha, -1 izquierda                           */
    int velocidad;    /* Centésimas de unidad por tick, en el nivel 1       */
    int sumergibles;  /* Cuántos grupos de tortugas bucean (en el nivel 1)  */
} carril_t;

/* Distribución parecida a la del arcade: en el río troncos hacia la derecha
 * y tortugas hacia la izquierda; en la calle los carriles alternan sentido.
 * Cada carril tiene distinto largo y velocidad (requisito obligatorio). */
static const carril_t CARRILES[] = {
    /* fila  tipo         var largo cant sentido vel sumerg */
    {  1,  OBS_TRONCO,   0,  4,    2,   +1,     35,  0 },
    {  2,  OBS_TORTUGA,  0,  2,    4,   -1,     40,  1 },
    {  3,  OBS_TRONCO,   0,  5,    2,   +1,     55,  0 },
    {  4,  OBS_TRONCO,   0,  3,    3,   +1,     25,  0 },
    {  5,  OBS_TORTUGA,  0,  3,    3,   -1,     30,  1 },

    {  7,  OBS_CAMION,   0,  2,    2,   -1,     30,  0 },
    {  8,  OBS_AUTO,     1,  1,    1,   +1,     90,  0 },
    {  9,  OBS_AUTO,     0,  1,    3,   -1,     35,  0 },
    { 10,  OBS_AUTO,     3,  1,    3,   +1,     25,  0 },
    { 11,  OBS_AUTO,     2,  1,    3,   -1,     30,  0 }
};
#define CANT_CARRILES ((int)(sizeof(CARRILES) / sizeof(CARRILES[0])))

/* Lugar libre mínimo entre obstáculos del mismo carril (en unidades), para
 * que siempre se pueda pasar */
#define HUECO_MINIMO (2 * UNIDADES_POR_CELDA)

/* Velocidad: +20% por nivel, hasta un máximo de 3 veces la original */
#define AUMENTO_VEL_POR_NIVEL 20
#define FACTOR_VEL_MAXIMO     300

static int entra_en_carril(int largo, int cantidad) {
    return cantidad * (largo * UNIDADES_POR_CELDA + HUECO_MINIMO) <= ANCHO_UNIDADES;
}

int niveles_cargar(obstaculo_t obstaculos[], int max, int nivel) {
    int factor_vel = 100 + AUMENTO_VEL_POR_NIVEL * (nivel - 1);
    int cant = 0;
    int c = 0;

    if (factor_vel > FACTOR_VEL_MAXIMO) {
        factor_vel = FACTOR_VEL_MAXIMO;
    }

    while (c < CANT_CARRILES) {
        carril_t carril = CARRILES[c];
        int i = 0;

        /* --- Dificultad extra además de la velocidad --- */

        /* Nivel 3 en adelante: un auto más por carril (si entra) */
        if (nivel >= 3 && (carril.tipo == OBS_AUTO || carril.tipo == OBS_CAMION) &&
            entra_en_carril(carril.largo, carril.cantidad + 1)) {
            carril.cantidad++;
        }
        /* Nivel 4 en adelante: troncos más cortos */
        if (nivel >= 4 && carril.tipo == OBS_TRONCO && carril.largo > 2) {
            carril.largo--;
        }
        /* Cada nivel bucea un grupo de tortugas más */
        if (carril.tipo == OBS_TORTUGA) {
            carril.sumergibles += nivel - 1;
            if (carril.sumergibles > carril.cantidad) {
                carril.sumergibles = carril.cantidad;
            }
        }

        while (i < carril.cantidad && cant < max) {
            obstaculo_t *o = &obstaculos[cant];

            o->tipo = carril.tipo;
            o->variante = carril.variante;
            o->fila = carril.fila;
            o->largo = carril.largo;
            o->sentido = carril.sentido;
            o->velocidad = carril.velocidad * factor_vel / 100;
            o->acumulador = 0;

            /* Repartidos parejo en el carril, con un corrimiento distinto por
             * fila para que no queden todos alineados */
            o->x = modulo(carril.fila * 17 + i * ANCHO_UNIDADES / carril.cantidad,
                          ANCHO_UNIDADES);

            o->es_sumergible = (carril.tipo == OBS_TORTUGA && i < carril.sumergibles);
            /* Cada grupo arranca en otro momento del ciclo para que no se
             * hundan todos a la vez */
            o->contador_buceo = i * 50;
            o->buceo = BUCEO_SUPERFICIE;

            cant++;
            i++;
        }
        c++;
    }
    return cant;
}
