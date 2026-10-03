/******************************************************************************
 * juego.h
 * Estado completo del juego y su única puerta de entrada: juego_actualizar().
 *
 * CAMBIO GRANDE: la máquina de estados (menú, partida, pausa, game over) ahora
 * vive acá, en el backend, y no en main.c. Antes el while de main corría
 * mientras estado == GAME_ON y al perder el programa se cerraba de golpe:
 * no se podía pausar, reiniciar ni volver al menú (requisito OBLIGATORIO
 * "Reanudar el juego") y no se mostraba el puntaje final (también
 * obligatorio). Al ponerla en el backend, las dos plataformas comparten la
 * misma lógica de menús y cada frontend solo se ocupa de dibujarlos.
 *****************************************************************************/
#ifndef JUEGO_H
#define JUEGO_H

#include "tablero.h"
#include "obstaculo.h"
#include "puntajes.h"

typedef enum {
    GAME_MENU,     /* Menú de inicio                                 */
    GAME_ON,       /* Jugando                                        */
    GAME_PAUSE,    /* Pausa (con opciones continuar/reiniciar/menú)  */
    GAME_END,      /* Game over: muestra puntaje y top 10            */
    GAME_EXIT      /* Pidieron salir: main corta el loop             */
} estado_juego_t;

/* CAMBIO: antes se llamaba direction_t y tenía DIR_QUIT, que no es una
 * dirección. Ahora es la "entrada" del usuario en general, y se agregaron
 * ENTER (elegir opción) y PAUSE, necesarias para los menús.
 * En la RasPi el botón del joystick manda INPUT_ENTER, que durante la
 * partida también pausa, así alcanza con un solo botón. */
typedef enum {
    INPUT_NONE,
    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_ENTER,
    INPUT_PAUSE,
    INPUT_QUIT     /* Cerrar ventana (Allegro) */
} input_t;

/* Opciones de cada menú. El frontend las lee para saber qué resaltar. */
typedef enum { MENU_JUGAR, MENU_SALIR, MENU_CANT_OPCIONES } opcion_menu_t;
typedef enum { PAUSA_CONTINUAR, PAUSA_REINICIAR, PAUSA_MENU, PAUSA_CANT_OPCIONES } opcion_pausa_t;
typedef enum { FIN_REINTENTAR, FIN_MENU, FIN_CANT_OPCIONES } opcion_fin_t;

typedef enum {
    ORIENT_ARRIBA,
    ORIENT_ABAJO,
    ORIENT_IZQUIERDA,
    ORIENT_DERECHA
} orientacion_t;

/* CAMBIO: la causa de muerte queda guardada para que el frontend pueda
 * mostrarla. Detectar "rana atropellada" y "rana ahogada" es obligatorio,
 * y en la defensa conviene que se vea cuál de las dos pasó. */
typedef enum {
    MUERTE_NINGUNA,
    MUERTE_ATROPELLADA,
    MUERTE_AHOGADA,
    MUERTE_ARBUSTO,      /* Saltó a la fila de meta fuera de una cuna, o a
                            una cuna ya ocupada                           */
    MUERTE_ARRASTRADA    /* Un tronco/tortuga la sacó del tablero         */
} causa_muerte_t;

typedef struct {
    int fila;
    int x;                      /* Borde izquierdo, en unidades           */
    orientacion_t orientacion;  /* Para elegir el sprite                  */
    int ticks_salto;            /* > 0 mientras dura la animación de salto */
    int ticks_muerte;           /* > 0 mientras dura la animación de muerte;
                                   la rana no se mueve en ese tiempo       */
    causa_muerte_t causa_muerte;
    int fila_record;            /* Fila más alta alcanzada en este viaje,
                                   para dar puntos solo por avanzar        */
} rana_t;

typedef struct {
    estado_juego_t estado;
    int opcion;                 /* Opción resaltada en el menú actual     */

    rana_t rana;
    obstaculo_t obstaculos[MAX_OBSTACULOS];
    int cant_obstaculos;

    int vidas;
    int nivel;
    int puntaje;

    int cunas_ocupadas[CANT_CUNAS];
    int ranas_salvadas;

    /* Mosca de bonus (requisito opcional / originalidad): aparece un rato en
     * una cuna libre y da puntos extra si la rana llega ahí */
    int cuna_mosca;             /* -1 si no hay mosca                     */
    int ticks_mosca;

    unsigned long ticks;        /* Ticks jugados (para animaciones)       */

    top10_t top10;
    int posicion_top10;         /* Al terminar: puesto alcanzado o -1     */
} game_t;

/* Duraciones (en ticks) que el frontend también usa para animar */
#define DURACION_SALTO   (FPS / 8)
#define DURACION_MUERTE  (FPS * 3 / 2)

/* Centro horizontal (en unidades) de cada cuna. Medido sobre el fondo:
 * los huecos de background.png están corridos media celda respecto de la
 * grilla, por eso los centros son múltiplos de 12 y no de 12 + 6. */
extern const int CENTRO_CUNAS[CANT_CUNAS];

/* --- Interfaz pública del backend --- */

/* Deja el juego en el menú de inicio y carga el top 10 */
void juego_inicializar(game_t *game);

/* Avanza el juego un tick (frame) con la entrada del usuario.
 * Es lo único que main necesita llamar. */
void juego_actualizar(game_t *game, input_t input);

/* Cantidad de opciones del menú del estado actual (0 si no hay menú) */
int juego_cant_opciones(const game_t *game);

/* Columna de la grilla en la que está la rana (para la RasPi) */
int juego_columna_rana(const game_t *game);

#endif /* JUEGO_H */
