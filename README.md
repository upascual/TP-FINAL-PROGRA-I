# progra1-frogger

TP Final de Programación I (25.02, ITBA): Frogger en C para PC (Allegro 5) y Raspberry Pi (display matricial 16x16 + joystick), con el mismo backend.

## Compilar y correr

Desde la carpeta raíz del proyecto, en Linux o WSL:

```bash
make allegro          # versión PC
./frogger-allegro

make allegro DEBUG=1  # con AddressSanitizer
ASAN_OPTIONS=detect_leaks=0 ./frogger-allegro

make raspberry        # versión RasPi (ver frontend/raspberry/libs/LEEME.txt)
./frogger-raspberry
```

No usar el botón "Run" de VS Code: compila solo el archivo abierto y no encuentra el resto de los módulos ni Allegro.

## Controles

| Acción | PC | RasPi |
|---|---|---|
| Mover / navegar menús | Flechas o WASD | Joystick |
| Elegir opción | Enter o Espacio | Botón del joystick |
| Pausa | Esc o P (también Enter) | Botón del joystick |

## Estructura

```
main.c                      loop principal (igual para las dos plataformas)
backend/
  tablero.h                 dimensiones y zonas del mapa
  juego.c/.h                reglas y máquina de estados (menú, partida, pausa, fin)
  obstaculo.c/.h            movimiento y geometría de los obstáculos
  niveles.c/.h              carriles y dificultad por nivel
  puntajes.c/.h             top 10 en puntajes.txt
frontend/
  frontend.h                interfaz que implementa cada plataforma
  allegro/frontendAllegro.c
  raspberry/frontendRaspberry.c
assets/                     imágenes y fuente (tienen que estar al lado del ejecutable)
```

El backend no incluye nada de Allegro ni de la RasPi: recibe un `input_t` por frame (`juego_actualizar`) y los frontends solo leen `game_t` para dibujar.
