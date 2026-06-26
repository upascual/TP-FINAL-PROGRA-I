#include "juego.h"
#include "frontend.h"

int main(void) {
    game_t juego;
    direction_t input_jugador = DIR_NONE;

    inicializar_hardware(); 
    inicializar_juego(&juego);
    
    while(juego.estado == GAME_ON) {
        
        input_jugador = capturar_entrada(); 

        // Atrapamos la señal de cierre de ventana
        if (input_jugador == DIR_QUIT) {
            juego.estado = GAME_END; 
        } 
        else if (input_jugador != DIR_NONE) {
            mover_rana(&juego, input_jugador);
        }

        actualizar_obstaculos(&juego); 
        
        if (verificar_colisiones(&juego)) {
            rana_muere(&juego);
        }

        if (verificar_meta(&juego)) {
            rana_muere(&juego);
        }

        if (juego.vidas <= 0) {
            juego.estado = GAME_END;
        }

        dibujar_estado(&juego);
        esperar_siguiente_frame();
    }

    apagar_hardware();
    return 0;
}