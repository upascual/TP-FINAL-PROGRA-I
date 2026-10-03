/******************************************************************************
 * frontend.h
 * Lo que cada plataforma (Allegro o RasPi) tiene que implementar.
 * main.c y el backend solo conocen estas funciones, nunca Allegro ni los
 * drivers de la placa. El Makefile elige qué implementación se linkea.
 *****************************************************************************/
#ifndef FRONTEND_H
#define FRONTEND_H

#include "juego.h"

/* Inicializa la plataforma (Allegro o display+joystick).
 * Devuelve 0 si salió bien, -1 si algo falló.
 * CAMBIO: antes main ignoraba este valor y, si faltaba un asset, el programa
 * seguía y explotaba al dibujar con un bitmap NULL. */
int inicializar_hardware(void);
void apagar_hardware(void);

/* Entrada NO bloqueante: si el usuario no hizo nada devuelve INPUT_NONE */
input_t capturar_entrada(void);

/* Dibuja el estado completo (menú, partida, pausa o game over) */
void dibujar_estado(const game_t *game);

/* Espera hasta el próximo frame, para correr a FPS ticks por segundo */
void esperar_siguiente_frame(void);

#endif /* FRONTEND_H */
