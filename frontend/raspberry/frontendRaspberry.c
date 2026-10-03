/******************************************************************************
 * frontendRaspberry.c
 * Interfaz para la Raspberry Pi: matriz de LEDs de 16x16 (un solo color) y
 * joystick con botón.
 *
 * CAMBIO: antes el archivo se llamaba "fontendRaspberry.c" (faltaba la r) y
 * el Makefile buscaba "frontendRaspberry.c", así que "make raspberry" fallaba.
 * Además todo era comentarios: no prendía ningún LED ni leía el joystick, y
 * la consigna exige que TODO lo obligatorio funcione en las dos plataformas.
 *
 * Usa los drivers que da la cátedra (disdrv y joydrv). Los nombres de abajo
 * son los de la versión que circula en el ITBA; si su versión cambia algún
 * nombre, se ajusta solo en este archivo (para eso está el frontend).
 *
 * Cómo se ve el juego en una matriz monocromática:
 *   - El tablero lógico (14x13) ocupa las columnas 1-14 y filas 0-12.
 *   - Autos, camiones, troncos y tortugas: LEDs prendidos.
 *   - Agua, calle y veredas: apagados.
 *   - Rana: titila (así se distingue cuando va arriba de un tronco).
 *   - Fila de meta: arbustos prendidos y cunas libres apagadas (es el
 *     "hueco" al que hay que llegar). Una cuna ocupada se prende, porque ya
 *     no se puede entrar. La mosca de bonus titila rápido en su cuna.
 *   - Tortugas por hundirse: titilan (aviso). Hundidas: apagadas.
 *   - Fila 15: vidas a la izquierda, nivel a la derecha.
 *****************************************************************************/
#include "juego.h"
#include "frontend.h"
#include <stdint.h>
#include <unistd.h>
#include "disdrv.h"
#include "joydrv.h"

/* ---------------------------------------------------------------------------
 * Configuración
 * ------------------------------------------------------------------------- */
#define LEDS_ANCHO        16
#define LEDS_ALTO         16
#define OFFSET_X          1      /* Columna del LED donde empieza el tablero */
#define OFFSET_Y          0
#define FILA_HUD          15

/* Joystick: valores entre -128 y 127 */
#define UMBRAL_MOVIMIENTO 70     /* Más que esto cuenta como dirección      */
#define UMBRAL_CENTRO     30     /* Menos que esto cuenta como "suelto"     */
/* Si en su placa arriba/abajo quedan al revés, cambiar a 1 */
#define JOY_INVERTIR_Y    0

#define MICROSEG_POR_FRAME (1000000 / FPS)

/* Contador de frames para los titileos */
static unsigned long frame = 0;

/* ---------------------------------------------------------------------------
 * Íconos de 8x8 para los menús (cada byte es una fila, bit 7 = izquierda)
 * ------------------------------------------------------------------------- */
static const uint8_t ICONO_PLAY[8]     = {0x10, 0x18, 0x1C, 0x1E, 0x1E, 0x1C, 0x18, 0x10};
static const uint8_t ICONO_SALIR[8]    = {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81};
static const uint8_t ICONO_REINICIO[8] = {0x3A, 0x46, 0x8E, 0x80, 0x81, 0x81, 0x42, 0x3C};
static const uint8_t ICONO_MENU[8]     = {0x18, 0x3C, 0x7E, 0xFF, 0x42, 0x5A, 0x5A, 0x7E};

/* Dígitos de 3x5 para mostrar el puntaje (bit 2 = izquierda) */
static const uint8_t DIGITOS[10][5] = {
    {7, 5, 5, 5, 7}, {2, 6, 2, 2, 7}, {7, 1, 7, 4, 7}, {7, 1, 7, 1, 7}, {5, 5, 7, 1, 1},
    {7, 4, 7, 1, 7}, {7, 4, 7, 5, 7}, {7, 1, 1, 1, 1}, {7, 5, 7, 5, 7}, {7, 5, 7, 1, 7}
};

/* ---------------------------------------------------------------------------
 * Utilidades de dibujo
 * ------------------------------------------------------------------------- */

/* Prende un LED, ignorando coordenadas fuera de la matriz (así el resto del
 * código no tiene que chequear bordes) */
static void prender(int x, int y) {
    if (x >= 0 && x < LEDS_ANCHO && y >= 0 && y < LEDS_ALTO) {
        dcoord_t punto = {(uint8_t) x, (uint8_t) y};
        disp_write(punto, D_ON);
    }
}

/* Titileo: devuelve 1 durante la parte "prendida" del ciclo */
static int titila(int periodo_frames) {
    return (frame / (periodo_frames / 2)) % 2 == 0;
}

static void dibujar_icono(const uint8_t icono[8], int x0, int y0) {
    int fila = 0;
    while (fila < 8) {
        int col = 0;
        while (col < 8) {
            if (icono[fila] & (0x80 >> col)) {
                prender(x0 + col, y0 + fila);
            }
            col++;
        }
        fila++;
    }
}

/* Dibuja un número con dígitos de 3x5. Devuelve el ancho en LEDs. */
static int dibujar_numero(int numero, int x0, int y0) {
    char cifras[12];
    int cant = 0;
    int i;

    /* Separamos las cifras (al revés) */
    do {
        cifras[cant] = (char) (numero % 10);
        numero /= 10;
        cant++;
    } while (numero > 0 && cant < 12);

    i = 0;
    while (i < cant) {
        int d = cifras[cant - 1 - i];
        int fila = 0;
        while (fila < 5) {
            int col = 0;
            while (col < 3) {
                if (DIGITOS[d][fila] & (4 >> col)) {
                    prender(x0 + i * 4 + col, y0 + fila);
                }
                col++;
            }
            fila++;
        }
        i++;
    }
    return cant * 4 - 1;
}

static int ancho_numero(int numero) {
    int cant = 1;
    while (numero >= 10) {
        numero /= 10;
        cant++;
    }
    return cant * 4 - 1;
}

/* Puntos en la columna 0 que indican cuántas opciones hay y cuál está
 * elegida: la elegida fija, las demás titilando */
static void dibujar_indicador_opciones(int cant, int elegida, int y0) {
    int i = 0;
    while (i < cant) {
        if (i == elegida || titila(40)) {
            prender(0, y0 + i * 2);
        }
        i++;
    }
}

/* ---------------------------------------------------------------------------
 * Inicialización
 * ------------------------------------------------------------------------- */
int inicializar_hardware(void) {
    disp_init();
    disp_clear();
    disp_update();
    joy_init();
    return 0;
}

void apagar_hardware(void) {
    disp_clear();
    disp_update();
}

/* ---------------------------------------------------------------------------
 * Entrada
 * ------------------------------------------------------------------------- */

/* El joystick es analógico y se lee 60 veces por segundo. Si devolviéramos la
 * dirección mientras está inclinado, la rana saltaría 60 veces por segundo.
 * Por eso se detecta el "flanco": solo cuenta cuando pasa de suelto a
 * inclinado. Lo mismo con el botón. */
input_t capturar_entrada(void) {
    static int joystick_suelto = 1;
    static int boton_anterior = 0;
    input_t input = INPUT_NONE;
    jcoord_t pos;
    int boton;
    int x, y, abs_x, abs_y;

    joy_update();
    pos = joy_get_coord();
    boton = (joy_get_switch() == J_PRESS);

    x = pos.x;
    y = JOY_INVERTIR_Y ? -pos.y : pos.y;
    abs_x = (x < 0) ? -x : x;
    abs_y = (y < 0) ? -y : y;

    if (boton && !boton_anterior) {
        input = INPUT_ENTER;
    } else if (abs_x < UMBRAL_CENTRO && abs_y < UMBRAL_CENTRO) {
        joystick_suelto = 1;
    } else if (joystick_suelto && (abs_x > UMBRAL_MOVIMIENTO || abs_y > UMBRAL_MOVIMIENTO)) {
        joystick_suelto = 0;
        /* Gana el eje más inclinado */
        if (abs_y >= abs_x) {
            input = (y > 0) ? INPUT_UP : INPUT_DOWN;
        } else {
            input = (x > 0) ? INPUT_RIGHT : INPUT_LEFT;
        }
    }
    boton_anterior = boton;
    return input;
}

/* ---------------------------------------------------------------------------
 * Partida
 * ------------------------------------------------------------------------- */

static void dibujar_meta(const game_t *game) {
    int col = 0;
    int cuna = 0;

    /* Arbustos: toda la fila prendida... */
    while (col < TABLERO_COLS) {
        prender(OFFSET_X + col, OFFSET_Y + FILA_META);
        col++;
    }
    /* ...y las cunas libres se "apagan" pisándolas con el estado correcto.
     * Como disp_write escribe el valor que le pasemos, usamos D_OFF directo. */
    while (cuna < CANT_CUNAS) {
        int col_cuna = CENTRO_CUNAS[cuna] / UNIDADES_POR_CELDA;
        dcoord_t punto = {(uint8_t) (OFFSET_X + col_cuna), (uint8_t) (OFFSET_Y + FILA_META)};

        if (!game->cunas_ocupadas[cuna]) {
            int prendida = (game->cuna_mosca == cuna) && titila(8);
            disp_write(punto, prendida ? D_ON : D_OFF);
        }
        cuna++;
    }
}

static void dibujar_obstaculos(const game_t *game) {
    int i = 0;

    while (i < game->cant_obstaculos) {
        const obstaculo_t *o = &game->obstaculos[i];
        int visible = 1;

        if (o->es_sumergible) {
            if (o->buceo == BUCEO_HUNDIDA) {
                visible = 0;
            } else if (o->buceo == BUCEO_BAJANDO) {
                visible = titila(12);     /* Aviso de que se va a hundir */
            }
        }

        if (visible) {
            /* Un LED por columna cuyo centro cae dentro del obstáculo. Se usa
             * obstaculo_contiene() del backend, así el wrap-around se ve igual
             * que como se calcula la colisión. */
            int col = 0;
            while (col < TABLERO_COLS) {
                int centro_col = col * UNIDADES_POR_CELDA + UNIDADES_POR_CELDA / 2;
                if (obstaculo_contiene(o, centro_col)) {
                    prender(OFFSET_X + col, OFFSET_Y + o->fila);
                }
                col++;
            }
        }
        i++;
    }
}

static void dibujar_rana(const game_t *game) {
    /* Titila más rápido mientras muere */
    int periodo = (game->rana.ticks_muerte > 0) ? 6 : 20;

    if (titila(periodo)) {
        prender(OFFSET_X + juego_columna_rana(game), OFFSET_Y + game->rana.fila);
    }
}

static void dibujar_hud(const game_t *game) {
    int i = 0;

    /* Vidas: un LED cada dos columnas desde la izquierda */
    while (i < game->vidas) {
        prender(i * 2, FILA_HUD);
        i++;
    }
    /* Nivel: un LED por nivel desde la derecha (hasta 6) */
    i = 0;
    while (i < game->nivel && i < 6) {
        prender(LEDS_ANCHO - 1 - i * 2, FILA_HUD);
        i++;
    }
}

static void dibujar_partida(const game_t *game) {
    dibujar_meta(game);
    dibujar_obstaculos(game);
    dibujar_rana(game);
    dibujar_hud(game);
}

/* ---------------------------------------------------------------------------
 * Menús
 * ------------------------------------------------------------------------- */

static void dibujar_menu(const game_t *game) {
    const uint8_t *icono = (game->opcion == MENU_JUGAR) ? ICONO_PLAY : ICONO_SALIR;
    dibujar_icono(icono, 4, 4);
    dibujar_indicador_opciones(MENU_CANT_OPCIONES, game->opcion, 6);
}

static void dibujar_pausa(const game_t *game) {
    const uint8_t *icono = ICONO_PLAY;

    if (game->opcion == PAUSA_REINICIAR) {
        icono = ICONO_REINICIO;
    } else if (game->opcion == PAUSA_MENU) {
        icono = ICONO_MENU;
    }
    /* Dos barras arriba = "pausa" */
    prender(6, 0); prender(6, 1); prender(6, 2);
    prender(9, 0); prender(9, 1); prender(9, 2);
    dibujar_icono(icono, 4, 5);
    dibujar_indicador_opciones(PAUSA_CANT_OPCIONES, game->opcion, 6);
    dibujar_hud(game);
}

/* Game over: el puntaje pasa deslizándose arriba (mensaje deslizante,
 * requisito opcional) y abajo el ícono de la opción elegida. Si entró al
 * top 10, titila un marco en las esquinas. */
static void dibujar_fin(const game_t *game) {
    int ancho = ancho_numero(game->puntaje);
    int recorrido = LEDS_ANCHO + ancho;
    int x = LEDS_ANCHO - (int) ((frame / 6) % (unsigned long) recorrido);
    const uint8_t *icono = (game->opcion == FIN_REINTENTAR) ? ICONO_REINICIO : ICONO_MENU;

    dibujar_numero(game->puntaje, x, 1);
    dibujar_icono(icono, 4, 8);
    dibujar_indicador_opciones(FIN_CANT_OPCIONES, game->opcion, 10);

    if (game->posicion_top10 >= 0 && titila(30)) {
        prender(0, 0); prender(15, 0); prender(0, 15); prender(15, 15);
    }
}

void dibujar_estado(const game_t *game) {
    frame++;
    disp_clear();

    switch (game->estado) {
    case GAME_MENU:  dibujar_menu(game);    break;
    case GAME_ON:    dibujar_partida(game); break;
    case GAME_PAUSE: dibujar_pausa(game);   break;
    case GAME_END:   dibujar_fin(game);     break;
    default:                                break;
    }
    disp_update();
}

void esperar_siguiente_frame(void) {
    /* Aproximado: no descuenta lo que tardó el frame en dibujarse, pero en la
     * RasPi dibujar 256 LEDs es muy rápido comparado con 16 ms */
    usleep(MICROSEG_POR_FRAME);
}
