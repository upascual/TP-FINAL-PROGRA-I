#ifndef JUEGO_H // GUARDA
#define JUEGO_H

#include "tablero.h"
#include "obstaculo.h"

#define CANT_MENUFARES 5

typedef enum { 
    GAME_ON, 
    GAME_PAUSE, 
    GAME_MENU, 
    GAME_END
} estado_juego_t;

typedef enum {
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} direction_t;

typedef struct {
    int fila;
    int col;
} rana_t;

typedef struct {
    rana_t rana;
    Obstaculo_t obstaculos[MAX_OBSTACULOS];
    estado_juego_t estado;

    int cant_obstaculos;
    int vidas;
    int nivel;
    int puntaje;

    int cunas_ocupadas[CANT_MENUFARES]; // 0 si está libre, 1 si está ocupada por una rana
    int ranas_salvadas;             // Contador de 0 a 5 para saber cuándo pasar de nivel
} game_t;

void inicializar_juego(game_t *game); //inicializa el mundo
void mover_rana(game_t *game, direction_t dir); // ve y actualiza los movimientos de la rana
void actualizar_obstaculos(game_t *game); //actualiza la posicion de los obstaculos
int verificar_colisiones(const game_t *game); // Retorna 1 si muere, 0 si está a salvo
int verificar_meta(game_t *game); // Retorna 1 si muere, 0 si está a salvo o no llegó
void rana_muere(game_t *game); // Decrementar vidas y volver a fil 15 a la rana 

#endif