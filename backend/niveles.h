/******************************************************************************
 * niveles.h
 * Diseño de los carriles y cómo se endurecen con cada nivel.
 *
 * CAMBIO: módulo nuevo. Antes había 3 obstáculos "de prueba" cargados a mano
 * en inicializar_juego() y al subir de nivel (game->nivel++) no cambiaba
 * nada del juego. La consigna exige niveles con dificultad creciente
 * (requisito OBLIGATORIO), así que ahora cada nivel arma los carriles a
 * partir de una tabla y les aplica la dificultad.
 *****************************************************************************/
#ifndef NIVELES_H
#define NIVELES_H

#include "obstaculo.h"

/* Llena el arreglo con los obstáculos del nivel pedido.
 * Devuelve cuántos cargó (nunca más de max). */
int niveles_cargar(obstaculo_t obstaculos[], int max, int nivel);

#endif /* NIVELES_H */
