/******************************************************************************
 * frontendAllegro.c
 * Interfaz gráfica para PC con Allegro 5.
 *
 * La idea de la consigna: Allegro es la interfaz "completa" y la RasPi un caso
 * particular. Acá aprovechamos sprites, animaciones, texto y transparencias.
 *****************************************************************************/
#include "juego.h"
#include "frontend.h"
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

/* ---------------------------------------------------------------------------
 * Medidas
 * CAMBIO: antes la ventana era 800x800 y el fondo de 840x780 se estiraba.
 * Ahora el tablero es exactamente del tamaño del fondo (14x13 celdas de 60 px)
 * y abajo hay una franja de 60 px para el HUD (vidas, puntaje, nivel). Así
 * el texto ya no tapa la fila de las cunas.
 * ------------------------------------------------------------------------- */
#define CELDA_PX        60
#define PX_POR_UNIDAD   (CELDA_PX / UNIDADES_POR_CELDA)
#define TABLERO_ANCHO   (TABLERO_COLS * CELDA_PX)        /* 840 */
#define TABLERO_ALTO    (TABLERO_FILS * CELDA_PX)        /* 780 */
#define HUD_ALTO        60
#define VENTANA_ANCHO   TABLERO_ANCHO
#define VENTANA_ALTO    (TABLERO_ALTO + HUD_ALTO)

/* Cantidad de cuadros en cada hoja de sprites (contados sobre las imágenes) */
#define FRAMES_RANA     8      /* CAMBIO: antes se dividía por 7 y el recorte salía corrido */
#define FRAMES_MUERTE   3
#define FRAMES_TORTUGA  9
#define FRAMES_AUTOS    4

#define COLOR_BLANCO    al_map_rgb(255, 255, 255)
#define COLOR_VERDE     al_map_rgb(80, 220, 40)
#define COLOR_AMARILLO  al_map_rgb(255, 230, 40)
#define COLOR_GRIS      al_map_rgb(150, 150, 150)
#define COLOR_ROJO      al_map_rgb(230, 40, 40)

/* ---------------------------------------------------------------------------
 * Recursos
 * ------------------------------------------------------------------------- */
static ALLEGRO_DISPLAY *display = NULL;
static ALLEGRO_EVENT_QUEUE *cola_eventos = NULL;
/* CAMBIO: cola aparte para el timer. esperar_siguiente_frame() se bloquea
 * hasta el próximo tick del timer, en vez de al_rest(0.1). Con al_rest el
 * tiempo de dibujado se sumaba a la espera y el juego no corría a una
 * velocidad constante. */
static ALLEGRO_EVENT_QUEUE *cola_timer = NULL;
static ALLEGRO_TIMER *timer = NULL;

static ALLEGRO_BITMAP *bmp_fondo = NULL;
static ALLEGRO_BITMAP *bmp_rana = NULL;
static ALLEGRO_BITMAP *bmp_muerte = NULL;
static ALLEGRO_BITMAP *bmp_autos = NULL;
static ALLEGRO_BITMAP *bmp_camion = NULL;
static ALLEGRO_BITMAP *bmp_tortuga = NULL;
static ALLEGRO_BITMAP *bmp_tronco_corto = NULL;
static ALLEGRO_BITMAP *bmp_tronco_medio = NULL;
static ALLEGRO_BITMAP *bmp_tronco_largo = NULL;
static ALLEGRO_BITMAP *bmp_cuna_rana = NULL;
static ALLEGRO_BITMAP *bmp_cuna_mosca = NULL;
static ALLEGRO_BITMAP *bmp_vida = NULL;
static ALLEGRO_BITMAP *bmp_titulo = NULL;
static ALLEGRO_BITMAP *bmp_game_over = NULL;
static ALLEGRO_BITMAP *bmp_boton_retry = NULL;
static ALLEGRO_FONT *fuente_grande = NULL;
static ALLEGRO_FONT *fuente_chica = NULL;

/* Contador propio para animaciones de menú (el backend no cuenta ticks
 * fuera de la partida) */
static unsigned long frames_dibujados = 0;

/* ---------------------------------------------------------------------------
 * Inicialización
 * ------------------------------------------------------------------------- */

/* CAMBIO: cada carga se chequea y, si falla, se avisa CUÁL archivo faltó.
 * Antes solo se devolvía -1 sin decir nada. */
static ALLEGRO_BITMAP *cargar_imagen(const char *ruta, int *todo_ok) {
    ALLEGRO_BITMAP *bmp = al_load_bitmap(ruta);
    if (bmp == NULL) {
        fprintf(stderr, "Error: no se pudo cargar '%s'\n", ruta);
        *todo_ok = 0;
    }
    return bmp;
}

static ALLEGRO_FONT *cargar_fuente(const char *ruta, int tamanio, int *todo_ok) {
    ALLEGRO_FONT *fuente = al_load_ttf_font(ruta, tamanio, 0);
    if (fuente == NULL) {
        fprintf(stderr, "Error: no se pudo cargar '%s'\n", ruta);
        *todo_ok = 0;
    }
    return fuente;
}

/* CAMBIO: nos paramos en la carpeta del ejecutable antes de cargar assets.
 * Las rutas "assets/..." son relativas: si VS Code (o cualquiera) corre el
 * programa desde otra carpeta, antes no encontraba ninguna imagen. */
static void ir_a_carpeta_del_ejecutable(void) {
    ALLEGRO_PATH *ruta = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
    if (ruta != NULL) {
        al_change_directory(al_path_cstr(ruta, ALLEGRO_NATIVE_PATH_SEP));
        al_destroy_path(ruta);
    }
}

int inicializar_hardware(void) {
    int todo_ok = 1;

    /* CAMBIO: antes no se chequeaba ninguna de estas llamadas */
    if (!al_init() || !al_install_keyboard() || !al_init_primitives_addon() ||
        !al_init_image_addon() || !al_init_font_addon() || !al_init_ttf_addon()) {
        fprintf(stderr, "Error: no se pudo inicializar Allegro o sus addons\n");
        return -1;
    }

    display = al_create_display(VENTANA_ANCHO, VENTANA_ALTO);
    cola_eventos = al_create_event_queue();
    cola_timer = al_create_event_queue();
    timer = al_create_timer(1.0 / FPS);
    if (display == NULL || cola_eventos == NULL || cola_timer == NULL || timer == NULL) {
        fprintf(stderr, "Error: no se pudo crear la ventana, las colas o el timer\n");
        return -1;
    }
    al_set_window_title(display, "Frogger");

    al_register_event_source(cola_eventos, al_get_keyboard_event_source());
    al_register_event_source(cola_eventos, al_get_display_event_source(display));
    al_register_event_source(cola_timer, al_get_timer_event_source(timer));

    ir_a_carpeta_del_ejecutable();

    bmp_fondo        = cargar_imagen("assets/background.png", &todo_ok);
    bmp_rana         = cargar_imagen("assets/frog.png", &todo_ok);
    bmp_muerte       = cargar_imagen("assets/death.png", &todo_ok);
    bmp_autos        = cargar_imagen("assets/cars.png", &todo_ok);
    bmp_camion       = cargar_imagen("assets/truck.png", &todo_ok);
    bmp_tortuga      = cargar_imagen("assets/turtle.png", &todo_ok);
    bmp_tronco_corto = cargar_imagen("assets/log1.png", &todo_ok);
    bmp_tronco_medio = cargar_imagen("assets/log2.png", &todo_ok);
    bmp_tronco_largo = cargar_imagen("assets/log3.png", &todo_ok);
    bmp_cuna_rana    = cargar_imagen("assets/goal.png", &todo_ok);
    bmp_cuna_mosca   = cargar_imagen("assets/bug_goal.png", &todo_ok);
    bmp_vida         = cargar_imagen("assets/life.png", &todo_ok);
    bmp_titulo       = cargar_imagen("assets/froggerTitle.png", &todo_ok);
    bmp_game_over    = cargar_imagen("assets/gameOverScreen.png", &todo_ok);
    bmp_boton_retry  = cargar_imagen("assets/RetryButton.png", &todo_ok);
    fuente_grande    = cargar_fuente("assets/Pixellari.ttf", 48, &todo_ok);
    fuente_chica     = cargar_fuente("assets/Pixellari.ttf", 28, &todo_ok);

    if (!todo_ok) {
        return -1;
    }

    al_start_timer(timer);
    return 0;
}

/* CAMBIO: se destruye solo lo que se llegó a crear. Si inicializar falló a
 * la mitad, antes se destruían punteros NULL (Allegro no siempre lo tolera). */
#define DESTRUIR(ptr, funcion) do { if ((ptr) != NULL) { funcion(ptr); (ptr) = NULL; } } while (0)

void apagar_hardware(void) {
    DESTRUIR(bmp_fondo, al_destroy_bitmap);
    DESTRUIR(bmp_rana, al_destroy_bitmap);
    DESTRUIR(bmp_muerte, al_destroy_bitmap);
    DESTRUIR(bmp_autos, al_destroy_bitmap);
    DESTRUIR(bmp_camion, al_destroy_bitmap);
    DESTRUIR(bmp_tortuga, al_destroy_bitmap);
    DESTRUIR(bmp_tronco_corto, al_destroy_bitmap);
    DESTRUIR(bmp_tronco_medio, al_destroy_bitmap);
    DESTRUIR(bmp_tronco_largo, al_destroy_bitmap);
    DESTRUIR(bmp_cuna_rana, al_destroy_bitmap);
    DESTRUIR(bmp_cuna_mosca, al_destroy_bitmap);
    DESTRUIR(bmp_vida, al_destroy_bitmap);
    DESTRUIR(bmp_titulo, al_destroy_bitmap);
    DESTRUIR(bmp_game_over, al_destroy_bitmap);
    DESTRUIR(bmp_boton_retry, al_destroy_bitmap);
    DESTRUIR(fuente_grande, al_destroy_font);
    DESTRUIR(fuente_chica, al_destroy_font);
    DESTRUIR(timer, al_destroy_timer);
    DESTRUIR(cola_timer, al_destroy_event_queue);
    DESTRUIR(cola_eventos, al_destroy_event_queue);
    DESTRUIR(display, al_destroy_display);
}

/* ---------------------------------------------------------------------------
 * Entrada
 * ------------------------------------------------------------------------- */

static input_t traducir_tecla(int tecla) {
    input_t input = INPUT_NONE;

    switch (tecla) {
    case ALLEGRO_KEY_UP:    case ALLEGRO_KEY_W: input = INPUT_UP;    break;
    case ALLEGRO_KEY_DOWN:  case ALLEGRO_KEY_S: input = INPUT_DOWN;  break;
    case ALLEGRO_KEY_LEFT:  case ALLEGRO_KEY_A: input = INPUT_LEFT;  break;
    case ALLEGRO_KEY_RIGHT: case ALLEGRO_KEY_D: input = INPUT_RIGHT; break;
    case ALLEGRO_KEY_ENTER: case ALLEGRO_KEY_SPACE:
    case ALLEGRO_KEY_PAD_ENTER:                 input = INPUT_ENTER; break;
    case ALLEGRO_KEY_ESCAPE: case ALLEGRO_KEY_P: input = INPUT_PAUSE; break;
    default:                                                         break;
    }
    return input;
}

/* CAMBIO: antes se leía UN solo evento por frame. Si llegaba primero un
 * evento que no era tecla (mover el mouse sobre la ventana, por ejemplo), esa
 * tecla se atendía recién en el frame siguiente o más tarde. Ahora se
 * descartan eventos hasta encontrar uno útil. Si hay más de una tecla
 * encolada, la siguiente se atiende en el próximo frame (no se pierde). */
input_t capturar_entrada(void) {
    ALLEGRO_EVENT ev;
    input_t input = INPUT_NONE;

    while (input == INPUT_NONE && al_get_next_event(cola_eventos, &ev)) {
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            input = INPUT_QUIT;
        } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            input = traducir_tecla(ev.keyboard.keycode);
        }
    }
    return input;
}

/* ---------------------------------------------------------------------------
 * Dibujo de la partida
 * ------------------------------------------------------------------------- */

/* Dibuja un recorte de una hoja de sprites escalado a un rectángulo */
static void dibujar_cuadro(ALLEGRO_BITMAP *hoja, int cuadro, int cant_cuadros,
                           float x, float y, float ancho, float alto, int flags) {
    float ancho_cuadro = (float) al_get_bitmap_width(hoja) / cant_cuadros;
    al_draw_scaled_bitmap(hoja, cuadro * ancho_cuadro, 0, ancho_cuadro, al_get_bitmap_height(hoja),
                          x, y, ancho, alto, flags);
}

static int cuadro_tortuga(const obstaculo_t *o, unsigned long ticks) {
    int cuadro = -1;   /* -1 = no se dibuja (hundida) */

    switch (o->buceo) {
    case BUCEO_SUPERFICIE: cuadro = (ticks / 10) % 3;      break;  /* Nadando */
    case BUCEO_BAJANDO:    cuadro = 3 + (ticks / 8) % 2;   break;  /* Burbujas: aviso */
    case BUCEO_SUBIENDO:   cuadro = 5;                     break;
    case BUCEO_HUNDIDA:                                    break;
    }
    return cuadro;
}

/* Dibuja un obstáculo con su borde izquierdo en x_px */
static void dibujar_obstaculo_en(const obstaculo_t *o, float x_px, float y_px, unsigned long ticks) {
    float ancho_px = o->largo * CELDA_PX;
    /* Los sprites de vehículos y tortugas miran a la derecha; los que van
     * para la izquierda se espejan */
    int flags = (o->sentido < 0) ? ALLEGRO_FLIP_HORIZONTAL : 0;
    int i = 0;

    switch (o->tipo) {
    case OBS_AUTO:
        dibujar_cuadro(bmp_autos, o->variante % FRAMES_AUTOS, FRAMES_AUTOS,
                       x_px, y_px, ancho_px, CELDA_PX, flags);
        break;
    case OBS_CAMION:
        dibujar_cuadro(bmp_camion, 0, 1, x_px, y_px, ancho_px, CELDA_PX, flags);
        break;
    case OBS_TRONCO: {
        ALLEGRO_BITMAP *bmp = bmp_tronco_corto;
        if (o->largo >= 5) {
            bmp = bmp_tronco_largo;
        } else if (o->largo == 4) {
            bmp = bmp_tronco_medio;
        }
        dibujar_cuadro(bmp, 0, 1, x_px, y_px, ancho_px, CELDA_PX, 0);
        break;
    }
    case OBS_TORTUGA: {
        int cuadro = cuadro_tortuga(o, ticks);
        if (cuadro >= 0) {
            /* Un grupo de tortugas = una tortuga por celda */
            while (i < o->largo) {
                dibujar_cuadro(bmp_tortuga, cuadro, FRAMES_TORTUGA,
                               x_px + i * CELDA_PX, y_px, CELDA_PX, CELDA_PX, flags);
                i++;
            }
        }
        break;
    }
    }
}

static void dibujar_obstaculo(const obstaculo_t *o, unsigned long ticks) {
    float x_px = o->x * PX_POR_UNIDAD;
    float y_px = o->fila * CELDA_PX;

    dibujar_obstaculo_en(o, x_px, y_px, ticks);

    /* CAMBIO: wrap-around visual. Si el obstáculo se pasa del borde derecho,
     * el pedazo que sobra se dibuja entrando por la izquierda. Es el mismo
     * criterio que usan las colisiones (obstaculo_contiene/solapa). */
    if (o->x + obstaculo_ancho(o) > ANCHO_UNIDADES) {
        dibujar_obstaculo_en(o, x_px - TABLERO_ANCHO, y_px, ticks);
    }
}

static void dibujar_cunas(const game_t *game) {
    int i = 0;

    while (i < CANT_CUNAS) {
        float centro_x = CENTRO_CUNAS[i] * PX_POR_UNIDAD;
        if (game->cunas_ocupadas[i]) {
            float lado = al_get_bitmap_width(bmp_cuna_rana);
            al_draw_bitmap(bmp_cuna_rana, centro_x - lado / 2, (CELDA_PX - lado) / 2, 0);
        } else if (game->cuna_mosca == i) {
            float lado = al_get_bitmap_width(bmp_cuna_mosca);
            al_draw_bitmap(bmp_cuna_mosca, centro_x - lado / 2, (CELDA_PX - lado) / 2, 0);
        }
        i++;
    }
}

static void dibujar_rana(const game_t *game) {
    const rana_t *rana = &game->rana;
    float x_px = rana->x * PX_POR_UNIDAD;
    float y_px = rana->fila * CELDA_PX;

    if (rana->ticks_muerte > 0) {
        /* Animación de muerte: avanza los 3 cuadros a lo largo de la duración */
        int transcurrido = DURACION_MUERTE - rana->ticks_muerte;
        int cuadro = transcurrido * FRAMES_MUERTE / DURACION_MUERTE;
        if (cuadro >= FRAMES_MUERTE) {
            cuadro = FRAMES_MUERTE - 1;
        }
        dibujar_cuadro(bmp_muerte, cuadro, FRAMES_MUERTE, x_px, y_px, CELDA_PX, CELDA_PX, 0);
    } else {
        /* La hoja tiene, en orden: arriba, abajo, izquierda, derecha; cada
         * una en reposo y saltando. Coincide con el orden de orientacion_t. */
        int cuadro = rana->orientacion * 2 + (rana->ticks_salto > 0 ? 1 : 0);
        dibujar_cuadro(bmp_rana, cuadro, FRAMES_RANA, x_px, y_px, CELDA_PX, CELDA_PX, 0);
    }
}

static const char *texto_causa_muerte(causa_muerte_t causa) {
    const char *texto = "";

    switch (causa) {
    case MUERTE_ATROPELLADA: texto = "ATROPELLADA!";      break;
    case MUERTE_AHOGADA:     texto = "AHOGADA!";          break;
    case MUERTE_ARBUSTO:     texto = "CHOCO EL ARBUSTO!"; break;
    case MUERTE_ARRASTRADA:  texto = "SE LA LLEVO EL RIO!"; break;
    case MUERTE_NINGUNA:                                  break;
    }
    return texto;
}

/* Cartel centrado con fondo semitransparente */
static void dibujar_cartel(const char *texto, float y, ALLEGRO_COLOR color) {
    float ancho = al_get_text_width(fuente_grande, texto) + 40;
    float alto = al_get_font_line_height(fuente_grande) + 16;
    float x = (VENTANA_ANCHO - ancho) / 2;

    al_draw_filled_rectangle(x, y, x + ancho, y + alto, al_map_rgba(0, 0, 0, 200));
    al_draw_text(fuente_grande, color, VENTANA_ANCHO / 2, y + 8, ALLEGRO_ALIGN_CENTRE, texto);
}

static void dibujar_hud(const game_t *game) {
    int i = 0;
    float y_texto = TABLERO_ALTO + (HUD_ALTO - al_get_font_line_height(fuente_chica)) / 2;

    al_draw_filled_rectangle(0, TABLERO_ALTO, VENTANA_ANCHO, VENTANA_ALTO, al_map_rgb(0, 0, 0));

    /* Vidas como iconitos de rana en vez de un número */
    while (i < game->vidas) {
        al_draw_bitmap(bmp_vida, 15 + i * 36, TABLERO_ALTO + (HUD_ALTO - 30) / 2, 0);
        i++;
    }

    al_draw_textf(fuente_chica, COLOR_VERDE, 180, y_texto, 0, "NIVEL %d", game->nivel);
    al_draw_textf(fuente_chica, COLOR_BLANCO, 380, y_texto, 0, "SCORE %d", game->puntaje);
    al_draw_textf(fuente_chica, COLOR_AMARILLO, VENTANA_ANCHO - 15, y_texto, ALLEGRO_ALIGN_RIGHT,
                  "HI %d", top10_record(&game->top10) > game->puntaje ?
                           top10_record(&game->top10) : game->puntaje);
}

static void dibujar_partida(const game_t *game) {
    int i = 0;

    /* CAMBIO: el fondo se dibuja 1 a 1, sin estirar */
    al_draw_bitmap(bmp_fondo, 0, 0, 0);
    dibujar_cunas(game);

    while (i < game->cant_obstaculos) {
        dibujar_obstaculo(&game->obstaculos[i], game->ticks);
        i++;
    }

    dibujar_rana(game);
    dibujar_hud(game);

    if (game->rana.ticks_muerte > 0) {
        dibujar_cartel(texto_causa_muerte(game->rana.causa_muerte), TABLERO_ALTO / 2 - 40, COLOR_ROJO);
    }
}

/* ---------------------------------------------------------------------------
 * Menús
 * ------------------------------------------------------------------------- */

/* Lista de opciones centrada; la elegida va en verde con una ranita al lado */
static void dibujar_opciones(const char *opciones[], int cant, int elegida, float y_inicial) {
    float alto_linea = al_get_font_line_height(fuente_grande) + 14;
    int i = 0;

    while (i < cant) {
        float y = y_inicial + i * alto_linea;
        if (i == elegida) {
            float ancho = al_get_text_width(fuente_grande, opciones[i]);
            /* La ranita "salta" un poco para que se note la selección */
            float salto = ((frames_dibujados / 15) % 2) ? 4 : 0;
            al_draw_text(fuente_grande, COLOR_VERDE, VENTANA_ANCHO / 2, y, ALLEGRO_ALIGN_CENTRE, opciones[i]);
            dibujar_cuadro(bmp_rana, 6, FRAMES_RANA, VENTANA_ANCHO / 2 - ancho / 2 - 60, y - salto, 44, 44, 0);
        } else {
            al_draw_text(fuente_grande, COLOR_GRIS, VENTANA_ANCHO / 2, y, ALLEGRO_ALIGN_CENTRE, opciones[i]);
        }
        i++;
    }
}

static void dibujar_top10(const top10_t *top, float y) {
    int i = 0;
    float alto_linea = al_get_font_line_height(fuente_chica) + 4;

    al_draw_text(fuente_chica, COLOR_AMARILLO, VENTANA_ANCHO / 2, y, ALLEGRO_ALIGN_CENTRE, "TOP 10");
    y += alto_linea + 8;

    if (top->cant == 0) {
        al_draw_text(fuente_chica, COLOR_GRIS, VENTANA_ANCHO / 2, y, ALLEGRO_ALIGN_CENTRE, "Todavia no hay puntajes");
    }
    /* Dos columnas de 5 */
    while (i < top->cant) {
        float x = (i < 5) ? VENTANA_ANCHO / 2 - 180 : VENTANA_ANCHO / 2 + 40;
        float yi = y + (i % 5) * alto_linea;
        al_draw_textf(fuente_chica, COLOR_BLANCO, x, yi, 0, "%2d.  %d", i + 1, top->puntajes[i]);
        i++;
    }
}

static void dibujar_menu(const game_t *game) {
    static const char *opciones[MENU_CANT_OPCIONES] = {"JUGAR", "SALIR"};

    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_bitmap(bmp_titulo, (VENTANA_ANCHO - al_get_bitmap_width(bmp_titulo)) / 2, 90, 0);
    dibujar_opciones(opciones, MENU_CANT_OPCIONES, game->opcion, 280);
    dibujar_top10(&game->top10, 480);
    al_draw_text(fuente_chica, COLOR_GRIS, VENTANA_ANCHO / 2, VENTANA_ALTO - 50, ALLEGRO_ALIGN_CENTRE,
                 "Flechas: mover    Enter: elegir    Esc / P: pausa");
}

static void dibujar_pausa(const game_t *game) {
    static const char *opciones[PAUSA_CANT_OPCIONES] = {"CONTINUAR", "REINICIAR", "MENU"};

    /* La partida congelada de fondo, oscurecida */
    dibujar_partida(game);
    al_draw_filled_rectangle(0, 0, VENTANA_ANCHO, VENTANA_ALTO, al_map_rgba(0, 0, 0, 180));
    al_draw_text(fuente_grande, COLOR_AMARILLO, VENTANA_ANCHO / 2, 220, ALLEGRO_ALIGN_CENTRE, "PAUSA");
    dibujar_opciones(opciones, PAUSA_CANT_OPCIONES, game->opcion, 330);
}

/* CAMBIO: pantalla de Game Over (antes el programa se cerraba sin mostrar el
 * puntaje, que es requisito obligatorio). Usa gameOverScreen.png y
 * RetryButton.png, que estaban en assets pero no se usaban. */
static void dibujar_fin(const game_t *game) {
    /* La imagen es de 300x400; la mostramos a 1.5x */
    const float escala = 1.5f;
    float ancho = al_get_bitmap_width(bmp_game_over) * escala;
    float alto = al_get_bitmap_height(bmp_game_over) * escala;
    float x0 = (VENTANA_ANCHO - ancho) / 2;
    float y0 = 20;
    /* Posiciones de los textos "Score:" y "High Score:" dentro de la imagen
     * original (medidas sobre el PNG) */
    float x_valor = x0 + 160 * escala;
    float y_score = y0 + 135 * escala;
    float y_record = y0 + 192 * escala;
    float y_botones = y0 + alto - 130;
    float x_boton = (VENTANA_ANCHO - al_get_bitmap_width(bmp_boton_retry)) / 2;
    float y_menu = y_botones + al_get_bitmap_height(bmp_boton_retry) + 20;

    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_scaled_bitmap(bmp_game_over, 0, 0, al_get_bitmap_width(bmp_game_over),
                          al_get_bitmap_height(bmp_game_over), x0, y0, ancho, alto, 0);

    al_draw_textf(fuente_grande, COLOR_BLANCO, x_valor, y_score, 0, "%d", game->puntaje);
    al_draw_textf(fuente_grande, COLOR_BLANCO, x_valor, y_record, 0, "%d", top10_record(&game->top10));

    if (game->posicion_top10 >= 0) {
        al_draw_textf(fuente_chica, COLOR_AMARILLO, VENTANA_ANCHO / 2, y0 + 250 * escala,
                      ALLEGRO_ALIGN_CENTRE, "Entraste al TOP 10! Puesto %d", game->posicion_top10 + 1);
    } else {
        al_draw_textf(fuente_chica, COLOR_GRIS, VENTANA_ANCHO / 2, y0 + 250 * escala,
                      ALLEGRO_ALIGN_CENTRE, "Llegaste al nivel %d", game->nivel);
    }

    /* Botón Retry (imagen) y opción MENU, con borde en la elegida */
    al_draw_bitmap(bmp_boton_retry, x_boton, y_botones, 0);
    al_draw_text(fuente_grande, game->opcion == FIN_MENU ? COLOR_VERDE : COLOR_GRIS,
                 VENTANA_ANCHO / 2, y_menu, ALLEGRO_ALIGN_CENTRE, "MENU");

    if (game->opcion == FIN_REINTENTAR) {
        al_draw_rectangle(x_boton - 6, y_botones - 6,
                          x_boton + al_get_bitmap_width(bmp_boton_retry) + 6,
                          y_botones + al_get_bitmap_height(bmp_boton_retry) + 6, COLOR_VERDE, 4);
    }
}

void dibujar_estado(const game_t *game) {
    frames_dibujados++;

    switch (game->estado) {
    case GAME_MENU:  dibujar_menu(game);    break;
    case GAME_ON:    dibujar_partida(game); break;
    case GAME_PAUSE: dibujar_pausa(game);   break;
    case GAME_END:   dibujar_fin(game);     break;
    default:                                break;
    }
    al_flip_display();
}

void esperar_siguiente_frame(void) {
    ALLEGRO_EVENT ev;
    al_wait_for_event(cola_timer, &ev);
}
