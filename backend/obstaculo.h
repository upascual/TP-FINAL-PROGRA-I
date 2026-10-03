/******************************************************************************
 * obstaculo.h
 * Autos, camiones, troncos y tortugas: cómo se mueven y qué lugar ocupan.
 *
 * CAMBIO: antes había DOS modelos de obstáculo en paralelo:
 *   - obstaculo_t dentro de juego.h (el que se usaba de verdad), y
 *   - este archivo, con un TAD opaco, otro enum de direcciones (DIR_ARRIBA vs
 *     DIR_UP), posiciones double y destruir_obstaculo() declarada pero nunca
 *     definida. El Makefile ni siquiera lo compilaba.
 * Ahora hay un solo modelo y vive acá. juego.c se encarga de las reglas
 * (choques, vidas, puntaje) y este módulo solo del movimiento y la geometría.
 * Así juego.c queda más corto y cada módulo tiene una sola responsabilidad.
 *
 * No usamos malloc: los obstáculos viven en un arreglo fijo dentro de game_t,
 * no hace falta memoria dinámica para algo de tamaño conocido.
 *****************************************************************************/
#ifndef OBSTACULO_H
#define OBSTACULO_H

#include "tablero.h"

/* CAMBIO: antes no había "tipo" y el frontend dibujaba todo como un auto,
 * incluidos troncos y tortugas. */
typedef enum {
    OBS_AUTO,
    OBS_CAMION,
    OBS_TRONCO,
    OBS_TORTUGA
} tipo_obstaculo_t;

/* CAMBIO: estado de buceo de las tortugas con 4 fases en vez de un
 * hundido sí/no. Las fases intermedias avisan al jugador (en Allegro se ven
 * las burbujas) antes de que la tortuga desaparezca. Solo HUNDIDA mata. */
typedef enum {
    BUCEO_SUPERFICIE,
    BUCEO_BAJANDO,
    BUCEO_HUNDIDA,
    BUCEO_SUBIENDO
} estado_buceo_t;

typedef struct {
    tipo_obstaculo_t tipo;
    int variante;        /* Cuál de los sprites de auto usar (lo decide el
                            nivel, el frontend solo lo lee)                 */
    int fila;
    int x;               /* Borde izquierdo en unidades. SIEMPRE dentro de
                            [0, ANCHO_UNIDADES): el wrap-around se resuelve
                            con módulo en un solo lugar (obstaculo.c)       */
    int largo;           /* En celdas */
    int sentido;         /* +1 = derecha, -1 = izquierda */

    /* CAMBIO: velocidad con acumulador en vez de "ticks_para_mover".
     * velocidad = centésimas de unidad por tick. Permite subir la velocidad
     * de a poco en cada nivel (un entero de ticks solo deja saltos bruscos
     * 3 -> 2 -> 1). */
    int velocidad;
    int acumulador;

    /* CAMBIO: contador propio para el buceo. Antes se usaba el mismo
     * contador para moverse y para hundirse, y como en las tortugas nunca se
     * reseteaba, después de unos ticks se movían en TODOS los frames. */
    int es_sumergible;
    int contador_buceo;
    estado_buceo_t buceo;
} obstaculo_t;

/* Avanza el obstáculo un tick. Devuelve cuántas unidades se desplazó
 * (con signo). juego.c lo usa para arrastrar a la rana si va encima. */
int obstaculo_avanzar(obstaculo_t *o);

/* Ancho en unidades */
int obstaculo_ancho(const obstaculo_t *o);

/* ¿El punto x (en unidades) cae sobre el obstáculo? Tiene en cuenta el
 * wrap-around (un tronco que sale por la derecha y asoma por la izquierda). */
int obstaculo_contiene(const obstaculo_t *o, int x);

/* ¿El intervalo [x, x + ancho) se superpone con el obstáculo? */
int obstaculo_solapa(const obstaculo_t *o, int x, int ancho);

/* ¿Se puede pararse encima? (troncos y tortugas que no están hundidas) */
int obstaculo_flota(const obstaculo_t *o);

/* Módulo matemático (siempre positivo). El % de C da negativo con
 * operandos negativos, que era justamente el caso que el código viejo
 * corregía a mano en tres lugares. */
int modulo(int a, int m);

#endif /* OBSTACULO_H */
