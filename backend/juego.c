#include "juego.h"
// ++++++++++++++++++++++++++++++++ERROR (VER OBSTACULOS DIVIDIDOS EN PARTES DE ARIBA Y ABAJO)++++++++++++++++++++++++++++++++++++++++
void inicializar_juego(game_t *game) 
{
    game->estado = GAME_ON;
    game->vidas = 3;
    game->nivel = 1;
    game->puntaje = 0;
    
    // Posición inicial de la rana
    game->rana.fila = TABLERO_FILS - 1;    // Fila 15
    game->rana.col = TABLERO_COLS / 2;  // Columna 8

    game->cant_obstaculos = 0; 

    game->ranas_salvadas = 0;
    for (int i = 0; i < CANT_MENUFARES; i++) 
    {
        game->cunas_ocupadas[i] = 0; // Todas las cunas empiezan vacías
    }
}

void mover_rana(game_t *game, direction_t dir) 
{
    // MOVIMIENTO HACIA ARRIBA
    if(dir == DIR_UP) 
    {
        if (game->rana.fila > 0) 
        {
            game->rana.fila--;
        }
        // Si fila == 0 no muere, se lo evalúa verificar_meta() en el main.c
    }
    // MOVIMIENTO HACIA ABAJO
    else if(dir == DIR_DOWN) 
    {
        if(game->rana.fila < TABLERO_FILS - 1) 
        {
            game->rana.fila++;
        }
        else 
        {
            rana_muere(game); // Intentó salir por el borde inferior (Fila 15)
        }
    }
    // MOVIMIENTO HACIA LA IZQUIERDA
    else if(dir == DIR_LEFT) 
    {
        if(game->rana.col > 0) 
        {
            game->rana.col--;
        }
        else 
        {
            rana_muere(game); // Intentó salir por el borde izquierdo (Columna 0)
        }
    }
    // MOVIMIENTO HACIA LA DERECHA
    else if(dir == DIR_RIGHT) 
    {
        if(game->rana.col < TABLERO_COLS - 1) 
        {
            game->rana.col++;
        }
        else 
        {
            rana_muere(game); // Intentó salir por el borde derecho (Columna 15)
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++ERROR (VELOCIDADES DISTINTAS ES DE OBSTACULOS)++++++++++++++++++++++++++++++++++++++++++++++
void actualizar_obstaculos(game_t *game) 
{
    // Recorremos todos los obstáculos activos en el juego
    for (int i = 0; i < game->cant_obstaculos; i++) 
    {
        // Guardamos si la rana está parada exactamente sobre este obstáculo antes de moverlo
        int rana_sobre_este_obstaculo = 0;
        
        // La rana SOLO se mueve con el obstáculo si están en la misma fila Y es la zona del río
        if (game->obstaculos[i].fila == game->rana.fila && game->rana.fila >= 2 && game->rana.fila <= 6) 
        {
            int obs_inicio = game->obstaculos[i].col;
            int obs_fin = obs_inicio + game->obstaculos[i].largo - 1;

            if (game->rana.col >= obs_inicio && game->rana.col <= obs_fin) 
            {
                rana_sobre_este_obstaculo = 1; // Aquí sí es un tronco
            }
        }

        // MOVIMIENTO HACIA LA DERECHA 
        if (game->obstaculos[i].direccion == DIR_RIGHT) 
        {
            game->obstaculos[i].col++; // Avanza el obstáculo
            
            // Si la rana estaba encima, hereda el movimiento (física móvil)
            if (rana_sobre_este_obstaculo == 1) 
            {
                game->rana.col++;
            }
            
            // Si el inicio del objeto salió por completo de la pantalla derecha
            if (game->obstaculos[i].col > TABLERO_COLS) 
            {
                game->obstaculos[i].col = - game->obstaculos[i].largo; 
            }
        }
        // MOVIMIENTO HACIA LA IZQUIERDA 
        else if (game->obstaculos[i].direccion == DIR_LEFT) 
        {
            game->obstaculos[i].col--; // Retrocede el obstáculo
            
            // Si la rana estaba encima, hereda el movimiento
            if (rana_sobre_este_obstaculo == 1) 
            {
                game->rana.col--;
            }
            
            // Si el final del objeto salió por la izquierda
            if (game->obstaculos[i].col + game->obstaculos[i].largo < 0) 
            {
                game->obstaculos[i].col = TABLERO_COLS; 
            }
        }
    }

    // DETECCIÓN DE BORDES POR ARRASTRE 
    // Si después de mover todos los troncos, la rana fue empujada fuera de la pantalla (0 a 15), muere.
    if (game->rana.col < 0 || game->rana.col >= TABLERO_COLS) 
    {
        ranaMuere(game);
    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ERROR (ALGUNAS TORTUGAS SE HUNDEN) ++++++++++++++++++
int verificar_colisiones(const game_t *game) // Devuelve 1 si la rana perdió una vida (choco o ahogo) y 0 si está a salvo
{
    int rana_f = game->rana.fila;
    int rana_c = game->rana.col;

    // ZONA DE LA CALLE (Filas 8 a 14)
    if (rana_f >= 8 && rana_f <= 14) 
    {
        // Recorremos todos los obstáculos buscando los autos
        for (int i = 0; i < game->cant_obstaculos; i++) 
        {
            //  Si el obstáculo está en la misma fila que la rana
            if (game->obstaculos[i].fila == rana_f) 
            {
                // límites físicos del obstáculo
                int obs_inicio = game->obstaculos[i].col;
                int obs_fin = obs_inicio + game->obstaculos[i].largo - 1;

                // Si la columna de la rana es mayor o igual al inicio del auto y 
                //menor o igual al final del auto CHOQUE
                if (rana_c >= obs_inicio && rana_c <= obs_fin) 
                {
                    return 1; // La rana esta muerta
                }
            }
        }
        return 0; // Si revisó todos los autos y no tocó ninguno, está a viva
    }

    // ZONA DEL RÍO (Filas 2 a 6)
    else if (rana_f >= 2 && rana_f <= 6) 
    {
        for (int i = 0; i < game->cant_obstaculos; i++) 
        {
            if (game->obstaculos[i].fila == rana_f) 
            {
                int obs_inicio = game->obstaculos[i].col;
                int obs_fin = obs_inicio + game->obstaculos[i].largo - 1;

                // Si la rana coincide con la posición de un tronco
                if (rana_c >= obs_inicio && rana_c <= obs_fin) 
                {
                    return 0;   // Esta sobre un tronco => convida
                }
            }
        }
        
        // Finalizada la revisión de la fila del río y la rana no esta sobre un tronco => Se cayó al agua
        return 1; 
    }

    // ZONAS SEGURAS (Veredas inicial, media y final)
    // Si la rana no está ni en el río ni en la calle esta bien.
    return 0; 
}

int verificar_meta(game_t *game) // Retorna 1 si muere, 0 si está a salvo o no llegó
{
    int resultado = 0; // 0 = a salvo o no llegó a la fila, 1 = murió en el intento
    
    // Si llegó a la fila superior
    if (game->rana.fila == 0) 
    {
        // Columnas exactas de tus 5 cunas
        const int columnas_cunas[CANT_MENUFARES] = {1, 4, 7, 10, 13};
        int cuna_encontrada = -1;// Agregamos 'cuna_encontrada == -1' en la condición del bucle para no usar break.
        
        // En cuanto encuentra la cuna, la condición se vuelve falsa y el ciclo termina solo.
        for (int i = 0; i < CANT_MENUFARES && cuna_encontrada == -1; i++) 
        {
            if (game->rana.col == columnas_cunas[i]) 
            {
                cuna_encontrada = i; // Guarda cuál cuna es (0 a 4)
            }
        }

        
        if (cuna_encontrada == -1) // Chocó contra la pared y no embocó en ninguna cuna)
        {
            resultado = 1; 
        }
        else if (game->cunas_ocupadas[cuna_encontrada] == 1) // La cuna ya estaba ocupada por otra rana previamente salvada
        {
            resultado = 1; 
        }
        else // Llegó a una cuna libre
        {
            game->cunas_ocupadas[cuna_encontrada] = 1; // La marcamos como llena
            game->ranas_salvadas ++ ;
            game->puntaje += 200 * game->nivel; // Sumamos puntos

            // Si salvó las 5 ranas, pasa de nivel y limpia las cunas
            if (game->ranas_salvadas == CANT_MENUFARES) 
            {
                game->nivel++;
                game->ranas_salvadas = 0;
                for (int j = 0; j < CANT_MENUFARES; j++) 
                {
                    game->cunas_ocupadas[j] = 0;
                }
            }

            // En caso de éxito, la rana actual vuelve sana y salva abajo para la próxima
            game->rana.fila = TABLERO_FILS - 1;
            game->rana.col = TABLERO_COLS / 2;
            resultado = 0; 
        }
    }
    
    return resultado;
}

void rana_muere(game_t *game) // Decrementar vidas y volver a fil 15 a la rana 
{
    game->vidas--;
    game->rana.fila = TABLERO_FILS - 1;   // Reinicia fila abajo (15)
    game->rana.col = TABLERO_COLS / 2; // Reinicia col al medio (8)
}