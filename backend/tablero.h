/******************************************************************************
 * tablero.h
 * Dimensiones y "mapa" lógico del juego. Lo usan el backend y los dos
 * frontends, así todos hablan del mismo tablero.
 *
 * CAMBIO: antes era 16x16 "porque la matriz de LEDs es de 16x16". Pero el
 * fondo de Allegro (background.png, 840x780) está pensado para una grilla de
 * 14x13 celdas de 60 px (la del Frogger original). Con 16x16 el fondo se
 * estiraba y las franjas de río/calle no coincidían con las filas lógicas.
 * Con 14x13 el fondo calza exacto en Allegro y en la RasPi entra igual en la
 * matriz de 16x16 (sobran filas/columnas que usamos para el HUD).
 *
 * CAMBIO: las filas de cada zona ahora son #define. Antes estaban
 * hardcodeadas (2, 6, 8, 14...) en tres funciones distintas y la fila 1
 * había quedado sin chequear (la rana podía pararse en el agua).
 *
 * CAMBIO: el guard era _TABLERO_H_. Los identificadores que empiezan con
 * guion bajo + mayúscula están reservados por el estándar de C.
 *****************************************************************************/
#ifndef TABLERO_H
#define TABLERO_H

/* Grilla lógica */
#define TABLERO_COLS 14
#define TABLERO_FILS 13

/* CAMBIO: posiciones horizontales en "unidades" (subdivisiones de celda).
 * Antes todo se movía de a una celda entera, que con celdas de 60 px se ve a
 * los saltos. Con 12 unidades por celda los autos y troncos se deslizan suave
 * en Allegro, y la RasPi simplemente divide por 12 para saber qué LED prender.
 * Las filas siguen siendo enteras (la rana salta de a una fila). */
#define UNIDADES_POR_CELDA 12
#define ANCHO_UNIDADES (TABLERO_COLS * UNIDADES_POR_CELDA)

/* CAMBIO: el backend avanza un "tick" por frame. Las velocidades están
 * pensadas para esta frecuencia, así que los dos frontends tienen que
 * respetarla en esperar_siguiente_frame(). Antes Allegro iba a 10 FPS fijos
 * con al_rest(0.1) y se sentía trabado. */
#define FPS 60

/* Zonas del mapa (filas) */
#define FILA_META          0   /* Cunas y arbustos                  */
#define FILA_RIO_INICIO    1
#define FILA_RIO_FIN       5
#define FILA_VEREDA        6   /* Vereda del medio (zona segura)    */
#define FILA_CALLE_INICIO  7
#define FILA_CALLE_FIN     11
#define FILA_SALIDA        12  /* Vereda de abajo (zona segura)     */

#define CANT_CUNAS      5
#define MAX_OBSTACULOS  40

#endif /* TABLERO_H */
