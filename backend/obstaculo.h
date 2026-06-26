#ifndef _OBSTACULO_H_
#define _OBSTACULO_H_

typedef enum {
    OBSTACULO_AUTO,
    OBSTACULO_CAMION
} TipoObstaculo_t;

typedef enum {
    DIR_ARRIBA,
    DIR_ABAJO,
    DIR_IZQUIERDA,
    DIR_DERECHA
} Direccion_t;

typedef struct {
    int x;
    int y;  
} Coordenada_t;

typedef struct {
    int ancho;
    int alto;
} DimensionObstaculo_t;

typedef struct {
    double rapidez;
    Direccion_t dir;
} Velocidad_t;

typedef struct obstaculo_t {
    /* Posicion */
    double x;                   // Posicion x (actual)
    double y;                   // Posicion y (actual)

    /* Dimensiones */
    DimensionObstaculo_t dim;

    /* Direccion y rapidez de movimiento */
    Velocidad_t vel;

    /* Tipo de obstaculo */
    TipoObstaculo_t tipo;
}Obstaculo_t;

typedef struct Obstaculo_t * Obstaculo;

Obstaculo nuevo_obstaculo(Coordenada_t coord_inicial, DimensionObstaculo_t dim, Velocidad_t vel, TipoObstaculo_t tipo);

void actualizar_posicion(Obstaculo o);

Coordenada_t obtener_posicion(Obstaculo o);
DimensionObstaculo_t obtener_dimension(Obstaculo o);
TipoObstaculo_t obtener_tipo(Obstaculo o);

void destruir_obstaculo(Obstaculo o);

#endif // _OBSTACULO_H_
