#include "juego.h"

void inicializar_juego(game_t *game) {
    game->estado = GAME_ON;
    game->vidas = 3;
    game->nivel = 1;
    game->puntaje = 0;
    
    game->rana.fila = TABLERO_FILS - 1;
    game->rana.col = TABLERO_COLS / 2;

    // Inicializamos 3 obstáculos de prueba
    game->cant_obstaculos = 3; 

    // 1. Auto
    game->obstaculos[0].fila = 12;
    game->obstaculos[0].col = 0;
    game->obstaculos[0].largo = 2;
    game->obstaculos[0].direccion = DIR_RIGHT;
    game->obstaculos[0].ticks_para_mover = 3; 
    game->obstaculos[0].contador_ticks = 0;
    game->obstaculos[0].es_sumergible = 0;
    game->obstaculos[0].esta_hundido = 0;

    // 2. Tronco
    game->obstaculos[1].fila = 5;
    game->obstaculos[1].col = 8;
    game->obstaculos[1].largo = 3; 
    game->obstaculos[1].direccion = DIR_LEFT;
    game->obstaculos[1].ticks_para_mover = 6; 
    game->obstaculos[1].contador_ticks = 0;
    game->obstaculos[1].es_sumergible = 0;
    game->obstaculos[1].esta_hundido = 0;

    // 3. Tortuga
    game->obstaculos[2].fila = 3;
    game->obstaculos[2].col = 3;
    game->obstaculos[2].largo = 2;
    game->obstaculos[2].direccion = DIR_RIGHT;
    game->obstaculos[2].ticks_para_mover = 5;
    game->obstaculos[2].contador_ticks = 0;
    game->obstaculos[2].es_sumergible = 1; 
    game->obstaculos[2].esta_hundido = 0;

    game->ranas_salvadas = 0;
    
    int i = 0;
    while (i < CANT_MENUFARES) {
        game->cunas_ocupadas[i] = 0;
        i++;
    }
}

void mover_rana(game_t *game, direction_t dir) {
    if(dir == DIR_UP) {
        if (game->rana.fila > 0) game->rana.fila--;
    }
    else if(dir == DIR_DOWN) {
        if(game->rana.fila < TABLERO_FILS - 1) game->rana.fila++;
        else rana_muere(game); 
    }
    else if(dir == DIR_LEFT) {
        if(game->rana.col > 0) game->rana.col--;
        else rana_muere(game); 
    }
    else if(dir == DIR_RIGHT) {
        if(game->rana.col < TABLERO_COLS - 1) game->rana.col++;
        else rana_muere(game); 
    }
}

void actualizar_obstaculos(game_t *game) {
    int i = 0;
    while (i < game->cant_obstaculos) {
        int rana_sobre_este_obstaculo = 0;
        
        if (game->obstaculos[i].fila == game->rana.fila && game->rana.fila >= 2 && game->rana.fila <= 6) {
            int j = 0;
            while (j < game->obstaculos[i].largo) {
                int pos_real = (game->obstaculos[i].col + j) % TABLERO_COLS;
                if (pos_real < 0) pos_real += TABLERO_COLS;
                
                if (game->rana.col == pos_real) {
                    rana_sobre_este_obstaculo = 1; 
                }
                j++;
            }
        }

        if (game->obstaculos[i].es_sumergible) {
            if (game->obstaculos[i].contador_ticks % 40 > 20) {
                game->obstaculos[i].esta_hundido = 1;
            } else {
                game->obstaculos[i].esta_hundido = 0;
            }
        }

        game->obstaculos[i].contador_ticks++;

        if (game->obstaculos[i].contador_ticks >= game->obstaculos[i].ticks_para_mover) {
            if (game->obstaculos[i].direccion == DIR_RIGHT) {
                game->obstaculos[i].col++;
                if (rana_sobre_este_obstaculo == 1) game->rana.col++;
                
                if (game->obstaculos[i].col > TABLERO_COLS) {
                    game->obstaculos[i].col = -game->obstaculos[i].largo; 
                }
            }
            else if (game->obstaculos[i].direccion == DIR_LEFT) {
                game->obstaculos[i].col--;
                if (rana_sobre_este_obstaculo == 1) game->rana.col--;
                
                if (game->obstaculos[i].col + game->obstaculos[i].largo < 0) {
                    game->obstaculos[i].col = TABLERO_COLS; 
                }
            }
            
            if (!game->obstaculos[i].es_sumergible) {
                game->obstaculos[i].contador_ticks = 0;
            }
        }
        i++;
    }

    if (game->rana.col < 0 || game->rana.col >= TABLERO_COLS) {
        rana_muere(game);
    }
}

int verificar_colisiones(const game_t *game) {
    int rana_f = game->rana.fila;
    int rana_c = game->rana.col;

    if (rana_f >= 8 && rana_f <= 14) {
        int i = 0;
        while (i < game->cant_obstaculos) {
            if (game->obstaculos[i].fila == rana_f) {
                int j = 0;
                while (j < game->obstaculos[i].largo) {
                    int pos_real = (game->obstaculos[i].col + j) % TABLERO_COLS;
                    if (pos_real < 0) pos_real += TABLERO_COLS;
                    
                    if (rana_c == pos_real) {
                        return 1; 
                    }
                    j++;
                }
            }
            i++;
        }
        return 0; 
    }
    else if (rana_f >= 2 && rana_f <= 6) {
        int i = 0;
        while (i < game->cant_obstaculos) {
            if (game->obstaculos[i].fila == rana_f) {
                int j = 0;
                while (j < game->obstaculos[i].largo) {
                    int pos_real = (game->obstaculos[i].col + j) % TABLERO_COLS;
                    if (pos_real < 0) pos_real += TABLERO_COLS;
                    
                    if (rana_c == pos_real) {
                        if (game->obstaculos[i].es_sumergible && game->obstaculos[i].esta_hundido) {
                            return 1; 
                        } else {
                            return 0; 
                        }
                    }
                    j++;
                }
            }
            i++;
        }
        return 1; 
    }
    return 0; 
}

int verificar_meta(game_t *game) {
    int resultado = 0; 
    
    if (game->rana.fila == 0) {
        const int columnas_cunas[CANT_MENUFARES] = {1, 4, 7, 10, 13};
        int cuna_encontrada = -1;
        
        int i = 0;
        while (i < CANT_MENUFARES && cuna_encontrada == -1) {
            if (game->rana.col == columnas_cunas[i]) {
                cuna_encontrada = i; 
            }
            i++;
        }
        
        if (cuna_encontrada == -1) {
            resultado = 1; 
        }
        else if (game->cunas_ocupadas[cuna_encontrada] == 1) {
            resultado = 1; 
        }
        else {
            game->cunas_ocupadas[cuna_encontrada] = 1; 
            game->ranas_salvadas++;
            game->puntaje += 200 * game->nivel; 

            if (game->ranas_salvadas == CANT_MENUFARES) {
                game->nivel++;
                game->ranas_salvadas = 0;
                
                int j = 0;
                while (j < CANT_MENUFARES) {
                    game->cunas_ocupadas[j] = 0;
                    j++;
                }
            }
            game->rana.fila = TABLERO_FILS - 1;
            game->rana.col = TABLERO_COLS / 2;
            resultado = 0; 
        }
    }
    return resultado;
}

void rana_muere(game_t *game) {
    game->vidas--;
    game->rana.fila = TABLERO_FILS - 1;   
    game->rana.col = TABLERO_COLS / 2; 
}