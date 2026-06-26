#include "juego.h"
#include "frontend.h"

int main(void) {
    game_t juego;
    direction_t input_jugador = DIR_NONE;

    // 1. Inicialización (Lógica + Gráficos)
    inicializar_juego(&juego);
    // inicializar_graficos(); <- Próxima fase (con Allegro/Raspi)

    // 2. El Game Loop (Se ejecuta muchas veces por segundo)
    while(juego.estado == GAME_ON) 
    {
        // A. CAPTURAR ENTRADA (Sin bloquear el programa)
        // input_jugador = leer_teclado_o_joystick(); 

        // B. ACTUALIZAR ESTADO (Back-end)
        if (input_jugador != DIR_NONE) 
        {
            mover_rana(&juego, input_jugador);
        }
        actualizar_obstaculos(&juego); // Los autos se mueven solos aquí
        
        // C. LÓGICA DE REGLAS (Llamamos a las funciones del back-end)
        
        // Regla 1: Colisiones en la calle o río
        if (verificar_colisiones(&juego))
        {
            ranaMuere(&juego);
        }

        // Regla 2: Llegada a la meta superior
        if (verificar_meta(&juego))
        {
            ranaMuere(&juego);
        }

        // Regla 3: Condición de Fin de Juego
        if (juego.vidas <= 0)
        {
            juego.estado = GAME_END;
        }

        // D. RENDERIZAR (Front-end)
        // dibujar_pantalla(&juego);

        // E. CONTROL DE TIEMPO
        // esperar_para_mantener_FPS();
    }

    // finalizar_graficos();
    return 0;
}




/* VIEJO +++++++++++++++++++++++++++++++++++++++++++++++++
int main(void)
{
    // inicializamos el juego
    game_t game;
    char tecla;

    inicializar_juego(&game);

    mostrar_menu();
    tecla = getchar();

    if(tecla == 'j')
    {
        game.estado = GAME_ON;
    }
    else if(tecla == 'q')
    {
        game.estado = GAME_END;
    }

    while(game.estado == GAME_ON)
    {
        tecla = getchar();

        if(tecla == 'w')
        {
            mover_rana(&game, DIR_UP);
        }
        else if(tecla == 's')
        {
            mover_rana(&game, DIR_DOWN);
        }
        else if(tecla == 'a')
        {
            mover_rana(&game, DIR_LEFT);
        }
        else if(tecla == 'd')
        {
            mover_rana(&game, DIR_RIGHT);
        }
        else if(tecla == 'q')
        {
            game.estado = GAME_END;
        }
    }

    return 0;
}
*/

