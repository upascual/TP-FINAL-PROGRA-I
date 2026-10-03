/******************************************************************************
 * puntajes.h
 * Top 10 de puntajes guardado en un archivo de texto.
 *
 * CAMBIO: módulo nuevo (requisito OPCIONAL "Puntajes máximos"). También sirve
 * para mostrar el récord (High Score) en la pantalla de Game Over, que ya
 * tenía su asset (gameOverScreen.png) pero no se usaba.
 *****************************************************************************/
#ifndef PUNTAJES_H
#define PUNTAJES_H

#define TOP_CANT 10
#define ARCHIVO_PUNTAJES "puntajes.txt"

typedef struct {
    int puntajes[TOP_CANT];   /* Ordenados de mayor a menor */
    int cant;                 /* Cuántos hay cargados (0 a TOP_CANT) */
} top10_t;

/* Carga el top desde el archivo. Si no existe o está roto, queda vacío
 * (no es un error: la primera vez que se juega el archivo no existe). */
void top10_cargar(top10_t *top, const char *archivo);

/* Guarda el top. Devuelve 0 si salió bien, -1 si no se pudo escribir. */
int top10_guardar(const top10_t *top, const char *archivo);

/* Inserta un puntaje manteniendo el orden.
 * Devuelve la posición (0 = primero) o -1 si no entró al top. */
int top10_insertar(top10_t *top, int puntaje);

/* Mejor puntaje (0 si el top está vacío) */
int top10_record(const top10_t *top);

#endif /* PUNTAJES_H */
