/******************************************************************************
 * obstaculo.c
 * Movimiento y geometría de los obstáculos. Ver comentarios en obstaculo.h.
 *****************************************************************************/
#include "obstaculo.h"

/* Duración del ciclo de buceo de una tortuga, en ticks, y dónde empieza cada
 * fase dentro del ciclo. 4 segundos en total: 2.5 s arriba, 0.5 s bajando,
 * 0.66 s hundida (la única fase que mata), 0.33 s subiendo. */
#define CICLO_BUCEO        (4 * FPS)
#define INICIO_BAJANDO     (CICLO_BUCEO * 60 / 100)
#define INICIO_HUNDIDA     (CICLO_BUCEO * 75 / 100)
#define INICIO_SUBIENDO    (CICLO_BUCEO * 92 / 100)

int modulo(int a, int m) {
    int r = a % m;
    if (r < 0) {
        r += m;
    }
    return r;
}

static void actualizar_buceo(obstaculo_t *o) {
    int fase;

    o->contador_buceo = (o->contador_buceo + 1) % CICLO_BUCEO;
    fase = o->contador_buceo;

    if (fase < INICIO_BAJANDO) {
        o->buceo = BUCEO_SUPERFICIE;
    } else if (fase < INICIO_HUNDIDA) {
        o->buceo = BUCEO_BAJANDO;
    } else if (fase < INICIO_SUBIENDO) {
        o->buceo = BUCEO_HUNDIDA;
    } else {
        o->buceo = BUCEO_SUBIENDO;
    }
}

int obstaculo_avanzar(obstaculo_t *o) {
    int pasos = 0;

    o->acumulador += o->velocidad;
    while (o->acumulador >= 100) {
        o->acumulador -= 100;
        pasos++;
    }

    /* CAMBIO: wrap-around con módulo. Antes la posición iba de -largo hasta
     * TABLERO_COLS y las colisiones usaban %, así que el auto que se veía y
     * el que mataba no estaban en el mismo lugar. Ahora x siempre está en
     * [0, ANCHO_UNIDADES) y todos (colisiones y dibujo) usan el mismo
     * criterio. */
    o->x = modulo(o->x + pasos * o->sentido, ANCHO_UNIDADES);

    if (o->es_sumergible) {
        actualizar_buceo(o);
    }

    return pasos * o->sentido;
}

int obstaculo_ancho(const obstaculo_t *o) {
    return o->largo * UNIDADES_POR_CELDA;
}

int obstaculo_contiene(const obstaculo_t *o, int x) {
    /* Distancia desde el borde izquierdo del obstáculo hasta x, "dando la
     * vuelta" si hace falta. Si es menor al ancho, x está adentro. */
    return modulo(x - o->x, ANCHO_UNIDADES) < obstaculo_ancho(o);
}

int obstaculo_solapa(const obstaculo_t *o, int x, int ancho) {
    /* Dos intervalos se tocan si el comienzo de alguno cae dentro del otro */
    return modulo(x - o->x, ANCHO_UNIDADES) < obstaculo_ancho(o) ||
           modulo(o->x - x, ANCHO_UNIDADES) < ancho;
}

int obstaculo_flota(const obstaculo_t *o) {
    int flota = 0;

    if (o->tipo == OBS_TRONCO) {
        flota = 1;
    } else if (o->tipo == OBS_TORTUGA) {
        flota = !(o->es_sumergible && o->buceo == BUCEO_HUNDIDA);
    }
    return flota;
}
