#ifndef JUEGO_H
#define JUEGO_H

#include "tablero.h"

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
    DIR_RIGHT,
    DIR_QUIT // Señal para cerrar el juego con la "X"
} direction_t;

typedef struct {
    int fila;
    int col;
    int largo;
    direction_t direccion;
    
    int ticks_para_mover; 
    int contador_ticks;
    int es_sumergible; 
    int esta_hundido;  
} obstaculo_t;

typedef struct {
    int fila;
    int col;
} rana_t;

typedef struct {
    rana_t rana;
    obstaculo_t obstaculos[MAX_OBSTACULOS];
    estado_juego_t estado;

    int cant_obstaculos;
    int vidas;
    int nivel;
    int puntaje;

    int cunas_ocupadas[CANT_MENUFARES]; 
    int ranas_salvadas;             
} game_t;

void inicializar_juego(game_t *game);
void mover_rana(game_t *game, direction_t dir); 
void actualizar_obstaculos(game_t *game); 
int verificar_colisiones(const game_t *game); 
int verificar_meta(game_t *game); 
void rana_muere(game_t *game); 

#endif