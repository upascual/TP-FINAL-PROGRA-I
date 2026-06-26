#include "juego.h"
#include "frontend.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h> 
#include <allegro5/allegro_font.h>  
#include <allegro5/allegro_ttf.h>   

#define VENTANA_PIXELES 800
#define FACTOR_ESCALA (VENTANA_PIXELES / TABLERO_COLS)

static ALLEGRO_DISPLAY *display = NULL;
static ALLEGRO_EVENT_QUEUE *event_queue = NULL;

static ALLEGRO_BITMAP *bmp_fondo = NULL;
static ALLEGRO_BITMAP *bmp_rana = NULL;
static ALLEGRO_BITMAP *bmp_autos = NULL;
static ALLEGRO_FONT *fuente_pixellari = NULL;

int inicializar_hardware(void) {
    al_init();
    al_install_keyboard();
    al_init_primitives_addon();
    
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    display = al_create_display(VENTANA_PIXELES, VENTANA_PIXELES);
    event_queue = al_create_event_queue();
    
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_display_event_source(display));

    bmp_fondo = al_load_bitmap("assets/background.png");
    bmp_rana = al_load_bitmap("assets/frog.png");
    bmp_autos = al_load_bitmap("assets/cars.png");
    fuente_pixellari = al_load_ttf_font("assets/Pixellari.ttf", 36, 0); 

    if (!bmp_fondo || !bmp_rana || !bmp_autos || !fuente_pixellari) {
        return -1; 
    }

    return 0;
}

void apagar_hardware(void) {
    al_destroy_bitmap(bmp_fondo);
    al_destroy_bitmap(bmp_rana);
    al_destroy_bitmap(bmp_autos);
    al_destroy_font(fuente_pixellari);

    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
}

direction_t capturar_entrada(void) {
    ALLEGRO_EVENT ev;
    if (al_get_next_event(event_queue, &ev)) {
        
        // Señal para cerrar la ventana
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            return DIR_QUIT;
        }

        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_UP) return DIR_UP;
            if (ev.keyboard.keycode == ALLEGRO_KEY_DOWN) return DIR_DOWN;
            if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT) return DIR_LEFT;
            if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) return DIR_RIGHT;
        }
    }
    return DIR_NONE;
}

void dibujar_estado(const game_t *game) {
    al_draw_scaled_bitmap(bmp_fondo, 
                          0, 0, al_get_bitmap_width(bmp_fondo), al_get_bitmap_height(bmp_fondo), 
                          0, 0, VENTANA_PIXELES, VENTANA_PIXELES, 
                          0);

    int i = 0;
    while (i < game->cant_obstaculos) {
        int px_x = game->obstaculos[i].col * FACTOR_ESCALA;
        int px_y = game->obstaculos[i].fila * FACTOR_ESCALA;
        
        int ancho_sprite_auto = al_get_bitmap_width(bmp_autos) / 4; 
        int alto_sprite_auto = al_get_bitmap_height(bmp_autos);
        
        al_draw_scaled_bitmap(bmp_autos, 
                              0 * ancho_sprite_auto, 0, ancho_sprite_auto, alto_sprite_auto, 
                              px_x, px_y, game->obstaculos[i].largo * FACTOR_ESCALA, FACTOR_ESCALA, 
                              0);
        i++;
    }

    int rana_px_x = game->rana.col * FACTOR_ESCALA;
    int rana_px_y = game->rana.fila * FACTOR_ESCALA;
    
    int ancho_sprite_rana = al_get_bitmap_width(bmp_rana) / 7;
    int alto_sprite_rana = al_get_bitmap_height(bmp_rana);
    
    int frame_actual = 0; 

    al_draw_scaled_bitmap(bmp_rana, 
                          frame_actual * ancho_sprite_rana, 0, ancho_sprite_rana, alto_sprite_rana, 
                          rana_px_x, rana_px_y, FACTOR_ESCALA, FACTOR_ESCALA, 
                          0);

    al_draw_textf(fuente_pixellari, al_map_rgb(255, 255, 255), 10, 10, 0, "SCORE: %d", game->puntaje);
    al_draw_textf(fuente_pixellari, al_map_rgb(255, 255, 255), VENTANA_PIXELES - 150, 10, 0, "LIVES: %d", game->vidas);

    al_flip_display();
}

void esperar_siguiente_frame(void) {
    al_rest(0.1); 
}