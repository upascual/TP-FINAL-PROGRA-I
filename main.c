/******************************************************************************
 * main.c
 * Loop principal. Es el mismo para las dos plataformas: el Makefile decide
 * qué frontend se linkea.
 *
 * CAMBIO: antes main llamaba una por una a las reglas del juego
 * (mover_rana, actualizar_obstaculos, verificar_colisiones, verificar_meta,
 * rana_muere...). Eso es lógica del juego metida en main, y además el loop
 * solo corría mientras estado == GAME_ON, así que perder cerraba el programa.
 * Ahora main solo hace: leer entrada -> actualizar -> dibujar -> esperar,
 * y la máquina de estados vive en el backend.
 *****************************************************************************/
#include <stdio.h>
#include "juego.h"
#include "frontend.h"

int main(void) {
    game_t juego;

    /* CAMBIO: se chequea si el hardware arrancó. Antes, si faltaba una imagen,
     * el programa seguía y se caía al dibujar con un puntero NULL. */
    if (inicializar_hardware() != 0) {
        fprintf(stderr, "No se pudo inicializar. Revisar que la carpeta assets/ "
                        "esté al lado del ejecutable.\n");
        apagar_hardware();
        return 1;
    }

    juego_inicializar(&juego);

    while (juego.estado != GAME_EXIT) {
        juego_actualizar(&juego, capturar_entrada());
        dibujar_estado(&juego);
        esperar_siguiente_frame();
    }

    apagar_hardware();
    return 0;
}
