#include "juego.h"
#include "frontend.h"
// #include <libreria_matriz.h> 

int inicializar_hardware(void) {
    // init_matriz_leds();
    return 0;
}

void apagar_hardware(void) {
    // apagar_matriz_leds();
}

direction_t capturar_entrada(void) {
    // Leer los pines GPIO de los botones de la placa
    // if (boton_arriba_presionado()) return DIR_UP;
    // ...
    return DIR_NONE;
}

void dibujar_estado(const game_t *game) {
    // limpiar_matriz();

    // 1. DIBUJAR LA RANA (Relación 1 a 1, un LED por coordenada)
    // prender_led(game->rana.col, game->rana.fila, COLOR_VERDE);

    // 2. DIBUJAR LOS OBSTÁCULOS
    int i = 0;
    while (i < game->cant_obstaculos) {
        int j = 0;
        // Como no podemos dibujar un "rectángulo", tenemos que prender LED por LED
        // recorriendo el largo del obstáculo
        while (j < game->obstaculos[i].largo) {
            int col_actual = game->obstaculos[i].col + j;
            
            // Lógica de wrap-around visual (si el auto se asoma por el otro lado de la matriz)
            col_actual = col_actual % TABLERO_COLS;
            if (col_actual < 0) {
                col_actual += TABLERO_COLS;
            }

            // prender_led(col_actual, game->obstaculos[i].fila, COLOR_ROJO);
            j++;
        }
        i++;
    }

    // actualizar_matriz();
}

void esperar_siguiente_frame(void) {
    // Función de sleep/delay estándar de Linux para RasPi
    // usleep(100000); 
}