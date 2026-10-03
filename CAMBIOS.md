# Cambios respecto de la versión anterior

En el código cada cambio está marcado con un comentario `CAMBIO:` que explica el porqué. Acá está el resumen ordenado por importancia.

## 1. Lo que impedía compilar o correr

| Problema | Qué se hizo | Por qué |
|---|---|---|
| `frontend/raspberry/fontendRaspberry.c` (typo) | Renombrado a `frontendRaspberry.c` | El Makefile buscaba ese nombre y `make raspberry` fallaba. |
| `frogger.c` con otro `main()` | Borrado | Si se compilan todos los `.c` juntos hay dos `main` y no linkea. Era un archivo de prueba. |
| Ejecutable `frogger-allegro` subido al repo | Borrado y agregado a `.gitignore` | Los binarios se generan con `make`; uno viejo confunde (podías estar corriendo una versión que no era la de tu código). |
| `main` ignoraba el valor de `inicializar_hardware()` | Ahora sale con un mensaje si falla | Si faltaba una imagen, el programa seguía y se caía al dibujar con un bitmap `NULL`. |
| Assets con ruta relativa | El frontend Allegro se para en la carpeta del ejecutable antes de cargarlos | Si se corría desde otra carpeta (pasa mucho con VS Code), no encontraba ninguna imagen. |
| Ninguna llamada de Allegro se chequeaba | Se chequean todas y se avisa qué archivo falló | Antes fallaba en silencio y no había forma de saber qué pasó. |
| `-fsanitize=address` siempre activo | Solo con `make allegro DEBUG=1` | LeakSanitizer reporta "fugas" internas de Allegro al cerrar y el programa termina con error aunque el código esté bien. En la RasPi es muy lento. |

## 2. Bugs de lógica

| Bug | Qué se hizo |
|---|---|
| Las tortugas se movían en **todos** los frames después de unos ticks (el contador de movimiento nunca se reseteaba porque se usaba también para el buceo). | Contadores separados: `acumulador` para moverse y `contador_buceo` para hundirse. |
| Chocar contra un borde (abajo en la salida, izquierda o derecha) **mataba** a la rana. | Ahora la rana simplemente no se mueve, como en el original. |
| Wrap-around inconsistente: el auto que se veía y el que mataba estaban en lugares distintos (la posición iba de `-largo` a 16 y las colisiones usaban `%`). | La posición queda siempre en `[0, ancho)` y se usa un único criterio con `modulo()` para colisiones y dibujo. Allegro dibuja el pedazo que asoma del otro lado. |
| La fila 1 no se chequeaba: la rana podía pararse en el agua. | Las zonas del mapa son `#define` en `tablero.h` y todas las funciones las usan. |
| Una tortuga hundida igual arrastraba a la rana. | Solo arrastra lo que flota (`obstaculo_flota()`). |
| El fondo (840x780) se estiraba a 800x800 y no coincidía con las filas. | Tablero de 14x13 celdas de 60 px, el tamaño exacto del fondo. En la RasPi entra igual en la matriz de 16x16. |
| Sprite de la rana dividido por 7 (la hoja tiene 8 cuadros). | Corregido. Además se usa la orientación (arriba/abajo/izq/der) y el cuadro de salto. |
| Troncos y tortugas se dibujaban como autos. | `obstaculo_t` tiene `tipo` y cada uno usa su sprite (troncos según largo, tortugas animadas, camiones). |
| `verificar_meta()` modificaba el estado pese a llamarse "verificar". | Renombrada a `procesar_meta()` y es privada del backend. |
| Movimiento de a celda entera (60 px de golpe). | Posiciones en 12 "unidades" por celda: autos y troncos se deslizan suave. La RasPi divide por 12. |

## 3. Requisitos obligatorios que faltaban

- **Niveles con dificultad creciente**: nuevo módulo `niveles.c`. Cada nivel sube la velocidad un 20 %, desde el nivel 3 hay un auto más por carril, desde el 4 los troncos son más cortos y cada nivel bucea un grupo más de tortugas.
- **Pausar, reiniciar o salir sin cerrar el programa**: máquina de estados en el backend (`GAME_MENU`, `GAME_ON`, `GAME_PAUSE`, `GAME_END`, `GAME_EXIT`) con menú de inicio, menú de pausa (continuar / reiniciar / menú) y pantalla de fin (reintentar / menú).
- **Mostrar el puntaje al terminar**: pantalla de Game Over con puntaje y récord, usando `gameOverScreen.png` y `RetryButton.png`.
- **Obstáculos de distinto largo y velocidad**: 10 carriles completos en vez de 3 obstáculos de prueba.
- **Funcionar en la RasPi**: el frontend era todo comentarios. Ahora dibuja el juego y los menús en la matriz y lee el joystick (con detección de flanco para que la rana no salte 60 veces por segundo).
- **Detectar atropellada y ahogada**: se guarda la causa de muerte y se muestra en pantalla.

## 4. Estructura

- **La máquina de estados pasó de `main.c` al backend.** `main` quedó en: leer entrada → `juego_actualizar()` → dibujar → esperar. Las dos plataformas comparten toda la lógica, incluidos los menús.
- **Un solo modelo de obstáculo.** Había dos (`obstaculo_t` en `juego.h` y un TAD en `obstaculo.c` que no se compilaba, con otro enum de direcciones y una función declarada sin definir). Ahora `obstaculo.c` tiene el movimiento y la geometría, y `juego.c` las reglas.
- **Funciones internas `static`.** `mover_rana`, `verificar_colisiones`, etc. ya no son públicas: `main` no necesita saber en qué orden se aplican las reglas.
- **`direction_t` pasó a ser `input_t`** con `INPUT_ENTER` e `INPUT_PAUSE`. `DIR_QUIT` no era una dirección.
- **Timer de Allegro a 60 FPS** en vez de `al_rest(0.1)`: velocidad constante y fluida. La entrada ya no se atrasa si llega un evento que no es tecla.
- **HUD en una franja propia** debajo del tablero (vidas con iconos, nivel, puntaje, récord). Antes el texto tapaba las cunas.
- **Makefile por objetos** con dependencias automáticas de los `.h`. Se sacó `-D RASPBERRY`: el Makefile ya elige qué frontend linkear.
- Guards de headers sin guion bajo inicial (`_TABLERO_H_` es un identificador reservado en C).
- Se mantuvo el estilo de ustedes: nombres en español, `while` en vez de `for`, una variable de resultado en vez de varios `return`.

## 5. Opcionales que se agregaron

- **Top 10** guardado en `puntajes.txt`, mostrado en el menú, y aviso en Game Over si el puntaje entró (con el puesto). El archivo se valida al leerlo.
- **Mosca de bonus**: aparece un rato en una cuna libre y da puntos extra (usa `bug_goal.png`).
- **Animaciones**: salto y muerte de la rana, tortugas nadando y con burbujas antes de hundirse, ranita que salta en el menú.
- **Mensaje deslizante** con el puntaje en la RasPi.
- **Puntaje**: 10 por cada fila nueva, 200 × nivel por cuna, 200 × nivel por mosca, 1000 × nivel por completar el nivel.

## Pendiente para ustedes

- **Drivers de la RasPi**: copiar los de la cátedra en `frontend/raspberry/libs/` (ver `LEEME.txt`). El frontend usa los nombres de la versión que circula en el ITBA (`disp_init`, `disp_write`, `joy_get_coord`, etc.). Si alguno difiere, se cambia solo en `frontendRaspberry.c`. Probar en la placa si arriba/abajo del joystick quedan al revés (`JOY_INVERTIR_Y`).
- **Sonido** (opcional): la cátedra da una librería de audio para la RasPi y Allegro tiene `allegro_audio`.
- `background.jpeg` y `ladybug.png` no se usan; se pueden borrar o aprovechar.
