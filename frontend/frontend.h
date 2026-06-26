#ifndef FRONTEND_H
#define FRONTEND_H

#include "juego.h" // Necesitamos conocer game_t y direction_t

// FUNCIONES DE INICIALIZACIÓN *************************************************
// En PC iniciará Allegro, en RasPi iniciará la placa de hardware.
int inicializar_hardware(void); 
void apagar_hardware(void);

// FUNCIONES DE ENTRADA (INPUT) ************************************************
// Debe leer el teclado (PC) o el joystick (RasPi) de forma NO BLOQUEANTE.
// Es decir, si el usuario no toca nada, devuelve DIR_NONE y el juego sigue.
direction_t capturar_entrada(void);

// FUNCIONES DE SALIDA (OUTPUT) ************************************************
// En PC dibujará bitmaps/formas, en RasPi encenderá LEDs.
void dibujar_estado(const game_t *game);

// CONTROL DEL TIEMPO **********************************************************
// Fundamental para que los autos se muevan a velocidad constante (FPS)
void esperar_siguiente_frame(void);

#endif