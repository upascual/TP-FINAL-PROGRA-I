/******************************************************************************
 * juego.c
 * Reglas del Frogger y máquina de estados. No sabe nada de Allegro ni de la
 * RasPi: recibe un input_t por tick y modifica game_t. Los frontends solo
 * leen game_t para dibujar.
 *****************************************************************************/
#include "juego.h"
#include "niveles.h"
#include <stdlib.h>
#include <time.h>

/* ---------------------------------------------------------------------------
 * Constantes de juego
 * ------------------------------------------------------------------------- */
#define VIDAS_INICIALES 3

/* Puntaje. La consigna pide que dependa de las veces que la rana cruza y del
 * nivel, así que todo se multiplica por el nivel. */
#define PUNTOS_AVANCE        10    /* Por cada fila nueva alcanzada        */
#define PUNTOS_CUNA         200    /* Por llegar a una cuna (x nivel)       */
#define PUNTOS_MOSCA        200    /* Extra si en la cuna estaba la mosca   */
#define PUNTOS_NIVEL       1000    /* Por completar las 5 cunas (x nivel)   */

#define ANCHO_RANA          UNIDADES_POR_CELDA
/* Hitbox contra autos un poco más chica que el sprite, para que no se sienta
 * injusto cuando apenas se rozan */
#define MARGEN_HITBOX       2
/* Qué tan lejos del centro de una cuna se acepta la llegada */
#define TOLERANCIA_CUNA     (UNIDADES_POR_CELDA / 2)

#define X_INICIAL_RANA      (CENTRO_CUNAS[2] - ANCHO_RANA / 2)

#define TICKS_MOSCA_VISIBLE (5 * FPS)
#define TICKS_MOSCA_OCULTA  (4 * FPS)

const int CENTRO_CUNAS[CANT_CUNAS] = {12, 48, 84, 120, 156};

/* ---------------------------------------------------------------------------
 * Prototipos de funciones internas
 * CAMBIO: mover_rana, actualizar_obstaculos, etc. antes eran públicas y main
 * las llamaba una por una. Ahora son static (privadas del módulo): main ya
 * no necesita conocer el orden en que se aplican las reglas.
 * ------------------------------------------------------------------------- */
static void nueva_partida(game_t *game);
static void cargar_nivel(game_t *game);
static void reaparecer_rana(game_t *game);
static void actualizar_menu(game_t *game, input_t input);
static void actualizar_partida(game_t *game, input_t input);
static void actualizar_pausa(game_t *game, input_t input);
static void actualizar_fin(game_t *game, input_t input);
static int  navegar_opciones(game_t *game, input_t input);
static void mover_rana(game_t *game, input_t input);
static void actualizar_obstaculos(game_t *game);
static void actualizar_mosca(game_t *game);
static causa_muerte_t verificar_colisiones(const game_t *game);
static void procesar_meta(game_t *game);
static void matar_rana(game_t *game, causa_muerte_t causa);
static void terminar_partida(game_t *game);

/* ===========================================================================
 * Interfaz pública
 * ========================================================================= */

void juego_inicializar(game_t *game) {
    srand((unsigned) time(NULL));
    top10_cargar(&game->top10, ARCHIVO_PUNTAJES);
    game->posicion_top10 = -1;

    /* Dejamos una partida armada para que el estado nunca tenga basura,
     * aunque todavía estemos en el menú */
    nueva_partida(game);
    game->estado = GAME_MENU;
    game->opcion = MENU_JUGAR;
}

void juego_actualizar(game_t *game, input_t input) {
    /* Cerrar la ventana corta desde cualquier estado */
    if (input == INPUT_QUIT) {
        game->estado = GAME_EXIT;
    } else {
        switch (game->estado) {
        case GAME_MENU:  actualizar_menu(game, input);    break;
        case GAME_ON:    actualizar_partida(game, input); break;
        case GAME_PAUSE: actualizar_pausa(game, input);   break;
        case GAME_END:   actualizar_fin(game, input);     break;
        default:                                          break;
        }
    }
}

int juego_cant_opciones(const game_t *game) {
    int cant = 0;

    switch (game->estado) {
    case GAME_MENU:  cant = MENU_CANT_OPCIONES;  break;
    case GAME_PAUSE: cant = PAUSA_CANT_OPCIONES; break;
    case GAME_END:   cant = FIN_CANT_OPCIONES;   break;
    default:                                     break;
    }
    return cant;
}

int juego_columna_rana(const game_t *game) {
    int col = (game->rana.x + ANCHO_RANA / 2) / UNIDADES_POR_CELDA;

    if (col < 0) {
        col = 0;
    } else if (col >= TABLERO_COLS) {
        col = TABLERO_COLS - 1;
    }
    return col;
}

/* ===========================================================================
 * Inicialización de partida / nivel
 * ========================================================================= */

static void nueva_partida(game_t *game) {
    game->vidas = VIDAS_INICIALES;
    game->nivel = 1;
    game->puntaje = 0;
    game->ticks = 0;
    game->posicion_top10 = -1;
    cargar_nivel(game);
}

static void cargar_nivel(game_t *game) {
    int i = 0;

    game->cant_obstaculos = niveles_cargar(game->obstaculos, MAX_OBSTACULOS, game->nivel);

    game->ranas_salvadas = 0;
    while (i < CANT_CUNAS) {
        game->cunas_ocupadas[i] = 0;
        i++;
    }
    game->cuna_mosca = -1;
    game->ticks_mosca = TICKS_MOSCA_OCULTA;

    reaparecer_rana(game);
}

static void reaparecer_rana(game_t *game) {
    game->rana.fila = FILA_SALIDA;
    game->rana.x = X_INICIAL_RANA;
    game->rana.orientacion = ORIENT_ARRIBA;
    game->rana.ticks_salto = 0;
    game->rana.ticks_muerte = 0;
    game->rana.causa_muerte = MUERTE_NINGUNA;
    game->rana.fila_record = FILA_SALIDA;
}

/* ===========================================================================
 * Menús
 * ========================================================================= */

/* Mueve la opción resaltada con arriba/abajo (dando la vuelta).
 * Devuelve 1 si el usuario confirmó con ENTER. */
static int navegar_opciones(game_t *game, input_t input) {
    int cant = juego_cant_opciones(game);
    int confirmo = 0;

    if (input == INPUT_UP) {
        game->opcion = modulo(game->opcion - 1, cant);
    } else if (input == INPUT_DOWN) {
        game->opcion = modulo(game->opcion + 1, cant);
    } else if (input == INPUT_ENTER) {
        confirmo = 1;
    }
    return confirmo;
}

static void actualizar_menu(game_t *game, input_t input) {
    if (navegar_opciones(game, input)) {
        if (game->opcion == MENU_JUGAR) {
            nueva_partida(game);
            game->estado = GAME_ON;
        } else {
            game->estado = GAME_EXIT;
        }
    }
}

static void actualizar_pausa(game_t *game, input_t input) {
    if (input == INPUT_PAUSE) {
        /* Apretar pausa de nuevo es atajo de "continuar" */
        game->estado = GAME_ON;
    } else if (navegar_opciones(game, input)) {
        if (game->opcion == PAUSA_CONTINUAR) {
            game->estado = GAME_ON;
        } else if (game->opcion == PAUSA_REINICIAR) {
            nueva_partida(game);
            game->estado = GAME_ON;
        } else {
            game->estado = GAME_MENU;
            game->opcion = MENU_JUGAR;
        }
    }
}

static void actualizar_fin(game_t *game, input_t input) {
    if (navegar_opciones(game, input)) {
        if (game->opcion == FIN_REINTENTAR) {
            nueva_partida(game);
            game->estado = GAME_ON;
        } else {
            game->estado = GAME_MENU;
            game->opcion = MENU_JUGAR;
        }
    }
}

/* ===========================================================================
 * Partida
 * ========================================================================= */

/* CAMBIO: este es el orden que antes estaba desparramado en main.c */
static void actualizar_partida(game_t *game, input_t input) {
    rana_t *rana = &game->rana;

    if (input == INPUT_PAUSE || input == INPUT_ENTER) {
        game->estado = GAME_PAUSE;
        game->opcion = PAUSA_CONTINUAR;
    } else {
        game->ticks++;

        if (rana->ticks_salto > 0) {
            rana->ticks_salto--;
        }

        /* Mientras muere no se puede mover */
        if (rana->ticks_muerte == 0 && input != INPUT_NONE) {
            mover_rana(game, input);
        }

        /* Los obstáculos se mueven siempre (y arrastran a la rana si va
         * arriba de un tronco o tortuga) */
        actualizar_obstaculos(game);
        actualizar_mosca(game);

        if (rana->ticks_muerte > 0) {
            rana->ticks_muerte--;
            if (rana->ticks_muerte == 0) {
                /* CAMBIO: antes, al quedarse sin vidas el programa se cerraba.
                 * Ahora pasa a la pantalla de Game Over. */
                if (game->vidas <= 0) {
                    terminar_partida(game);
                } else {
                    reaparecer_rana(game);
                }
            }
        } else if (game->estado == GAME_ON) {
            causa_muerte_t causa = verificar_colisiones(game);
            if (causa != MUERTE_NINGUNA) {
                matar_rana(game, causa);
            }
        }
    }
}

static void mover_rana(game_t *game, input_t input) {
    rana_t *rana = &game->rana;
    int se_movio = 1;

    /* CAMBIO: chocar contra un borde ya NO mata a la rana (antes, apretar
     * abajo en la fila de salida o ir contra los costados restaba una vida).
     * Ahora simplemente no se mueve, como en el original. */
    if (input == INPUT_UP) {
        rana->orientacion = ORIENT_ARRIBA;
        if (rana->fila > FILA_META) {
            rana->fila--;
        } else {
            se_movio = 0;
        }
    } else if (input == INPUT_DOWN) {
        rana->orientacion = ORIENT_ABAJO;
        if (rana->fila < FILA_SALIDA) {
            rana->fila++;
        } else {
            se_movio = 0;
        }
    } else if (input == INPUT_LEFT) {
        rana->orientacion = ORIENT_IZQUIERDA;
        rana->x -= UNIDADES_POR_CELDA;
        if (rana->x < 0) {
            rana->x = 0;   /* Si estaba pegada al borde, queda en el borde */
        }
    } else if (input == INPUT_RIGHT) {
        rana->orientacion = ORIENT_DERECHA;
        rana->x += UNIDADES_POR_CELDA;
        if (rana->x > ANCHO_UNIDADES - ANCHO_RANA) {
            rana->x = ANCHO_UNIDADES - ANCHO_RANA;
        }
    } else {
        se_movio = 0;      /* ENTER/PAUSE ya se trataron antes */
    }

    if (se_movio) {
        rana->ticks_salto = DURACION_SALTO;

        /* Puntos solo por filas nuevas (si no, se farmean puntos subiendo y
         * bajando) */
        if (rana->fila < rana->fila_record) {
            rana->fila_record = rana->fila;
            game->puntaje += PUNTOS_AVANCE;
        }

        if (rana->fila == FILA_META) {
            procesar_meta(game);
        }
    }
}

static void actualizar_obstaculos(game_t *game) {
    rana_t *rana = &game->rana;
    int centro_rana = rana->x + ANCHO_RANA / 2;
    int i = 0;

    while (i < game->cant_obstaculos) {
        obstaculo_t *o = &game->obstaculos[i];

        /* Se decide ANTES de mover si la rana va encima, así se mueven juntos.
         * CAMBIO: ahora también exige que el obstáculo flote (antes una
         * tortuga hundida igual arrastraba a la rana). */
        int arrastra = rana->ticks_muerte == 0 &&
                       o->fila == rana->fila &&
                       obstaculo_flota(o) &&
                       obstaculo_contiene(o, centro_rana);

        int desplazamiento = obstaculo_avanzar(o);

        if (arrastra) {
            rana->x += desplazamiento;
        }
        i++;
    }
}

static void actualizar_mosca(game_t *game) {
    game->ticks_mosca--;
    if (game->ticks_mosca <= 0) {
        if (game->cuna_mosca >= 0) {
            game->cuna_mosca = -1;
            game->ticks_mosca = TICKS_MOSCA_OCULTA;
        } else {
            /* Elegimos una cuna libre al azar. Si todas están ocupadas
             * (no debería pasar, se cambia de nivel antes) no aparece. */
            int libres = CANT_CUNAS - game->ranas_salvadas;
            if (libres > 0) {
                int elegida = rand() % libres;
                int i = 0;
                while (i < CANT_CUNAS && game->cuna_mosca == -1) {
                    if (!game->cunas_ocupadas[i]) {
                        if (elegida == 0) {
                            game->cuna_mosca = i;
                        }
                        elegida--;
                    }
                    i++;
                }
            }
            game->ticks_mosca = TICKS_MOSCA_VISIBLE;
        }
    }
}

/* Devuelve por qué murió la rana, o MUERTE_NINGUNA si está a salvo.
 * CAMBIO: antes devolvía 1/0 y había dos bloques casi idénticos (calle y
 * río) copiando el recorrido de casilleros. Ahora la geometría está en
 * obstaculo.c y acá solo quedan las reglas. */
static causa_muerte_t verificar_colisiones(const game_t *game) {
    const rana_t *rana = &game->rana;
    int centro = rana->x + ANCHO_RANA / 2;
    causa_muerte_t causa = MUERTE_NINGUNA;
    int i = 0;

    if (centro < 0 || centro >= ANCHO_UNIDADES) {
        causa = MUERTE_ARRASTRADA;
    } else if (rana->fila >= FILA_CALLE_INICIO && rana->fila <= FILA_CALLE_FIN) {
        /* Calle: muere si toca cualquier vehículo de su fila */
        while (i < game->cant_obstaculos && causa == MUERTE_NINGUNA) {
            const obstaculo_t *o = &game->obstaculos[i];
            if (o->fila == rana->fila &&
                obstaculo_solapa(o, rana->x + MARGEN_HITBOX, ANCHO_RANA - 2 * MARGEN_HITBOX)) {
                causa = MUERTE_ATROPELLADA;
            }
            i++;
        }
    } else if (rana->fila >= FILA_RIO_INICIO && rana->fila <= FILA_RIO_FIN) {
        /* Río: muere si NO está sobre algo que flote */
        int a_salvo = 0;
        while (i < game->cant_obstaculos && !a_salvo) {
            const obstaculo_t *o = &game->obstaculos[i];
            if (o->fila == rana->fila && obstaculo_flota(o) && obstaculo_contiene(o, centro)) {
                a_salvo = 1;
            }
            i++;
        }
        if (!a_salvo) {
            causa = MUERTE_AHOGADA;
        }
    }
    /* Veredas: siempre a salvo. La fila de meta se resuelve en
     * procesar_meta() en el momento en que la rana llega. */
    return causa;
}

/* CAMBIO: antes se llamaba verificar_meta() pero además de verificar
 * modificaba el estado (cunas, puntaje, nivel). El nombre nuevo dice lo que
 * hace. Además ahora mata a la rana directamente con la causa correcta, en
 * vez de devolver 1 para que main llame a rana_muere(). */
static void procesar_meta(game_t *game) {
    rana_t *rana = &game->rana;
    int centro = rana->x + ANCHO_RANA / 2;
    int cuna = -1;
    int i = 0;

    while (i < CANT_CUNAS && cuna == -1) {
        if (abs(centro - CENTRO_CUNAS[i]) <= TOLERANCIA_CUNA) {
            cuna = i;
        }
        i++;
    }

    if (cuna == -1 || game->cunas_ocupadas[cuna]) {
        matar_rana(game, MUERTE_ARBUSTO);
    } else {
        game->cunas_ocupadas[cuna] = 1;
        game->ranas_salvadas++;
        game->puntaje += PUNTOS_CUNA * game->nivel;

        if (cuna == game->cuna_mosca) {
            game->puntaje += PUNTOS_MOSCA * game->nivel;
            game->cuna_mosca = -1;
            game->ticks_mosca = TICKS_MOSCA_OCULTA;
        }

        if (game->ranas_salvadas == CANT_CUNAS) {
            game->puntaje += PUNTOS_NIVEL * game->nivel;
            game->nivel++;
            cargar_nivel(game);      /* CAMBIO: ahora el nivel nuevo es más difícil */
        } else {
            reaparecer_rana(game);
        }
    }
}

static void matar_rana(game_t *game, causa_muerte_t causa) {
    rana_t *rana = &game->rana;

    /* Por si dos cosas la matan en el mismo tick: una sola vida */
    if (rana->ticks_muerte == 0) {
        game->vidas--;
        rana->ticks_muerte = DURACION_MUERTE;
        rana->causa_muerte = causa;

        /* Si la arrastraron afuera, la dejamos en el borde para que la
         * animación de muerte se vea */
        if (rana->x < 0) {
            rana->x = 0;
        } else if (rana->x > ANCHO_UNIDADES - ANCHO_RANA) {
            rana->x = ANCHO_UNIDADES - ANCHO_RANA;
        }
    }
}

static void terminar_partida(game_t *game) {
    /* Un 0 no tiene sentido guardarlo en el top */
    game->posicion_top10 = (game->puntaje > 0) ? top10_insertar(&game->top10, game->puntaje) : -1;
    if (game->posicion_top10 >= 0) {
        /* Si no se puede guardar el archivo el juego sigue igual; el top 10
         * de esta sesión queda en memoria */
        top10_guardar(&game->top10, ARCHIVO_PUNTAJES);
    }
    game->estado = GAME_END;
    game->opcion = FIN_REINTENTAR;
}
