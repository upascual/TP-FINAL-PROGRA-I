/******************************************************************************
 * puntajes.c
 *****************************************************************************/
#include "puntajes.h"
#include <stdio.h>

void top10_cargar(top10_t *top, const char *archivo) {
    FILE *f = fopen(archivo, "r");
    int valor;

    top->cant = 0;
    if (f != NULL) {
        /* Validamos lo que leemos: si alguien edita el archivo a mano y deja
         * basura o números negativos, se ignora (robustez, punto 1 de la
         * nota). Usamos top10_insertar para que quede ordenado aunque el
         * archivo no lo esté. */
        while (fscanf(f, "%d", &valor) == 1) {
            if (valor > 0) {
                top10_insertar(top, valor);
            }
        }
        fclose(f);
    }
}

int top10_guardar(const top10_t *top, const char *archivo) {
    FILE *f = fopen(archivo, "w");
    int resultado = -1;
    int i = 0;

    if (f != NULL) {
        while (i < top->cant) {
            fprintf(f, "%d\n", top->puntajes[i]);
            i++;
        }
        fclose(f);
        resultado = 0;
    }
    return resultado;
}

int top10_insertar(top10_t *top, int puntaje) {
    int pos = top->cant;

    /* Buscamos desde el final hacia arriba, corriendo los menores un lugar */
    while (pos > 0 && top->puntajes[pos - 1] < puntaje) {
        if (pos < TOP_CANT) {
            top->puntajes[pos] = top->puntajes[pos - 1];
        }
        pos--;
    }

    if (pos < TOP_CANT) {
        top->puntajes[pos] = puntaje;
        if (top->cant < TOP_CANT) {
            top->cant++;
        }
    } else {
        pos = -1;
    }
    return pos;
}

int top10_record(const top10_t *top) {
    return (top->cant > 0) ? top->puntajes[0] : 0;
}
