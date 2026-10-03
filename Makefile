# =============================================================================
# Makefile del TP Frogger
#
#   make allegro            -> compila la versión de PC     (./frogger-allegro)
#   make allegro DEBUG=1    -> igual pero con AddressSanitizer y sin optimizar
#   make raspberry          -> compila la versión de la RasPi (./frogger-raspberry)
#   make clean              -> borra ejecutables y objetos
#
# CAMBIOS respecto de la versión anterior:
#  - Compila por objetos (.o) en build/. Si tocás un solo archivo, recompila
#    solo ese (y los que incluyen el .h que cambió, gracias a -MMD).
#  - El backend tiene más archivos (obstaculo.c, niveles.c, puntajes.c).
#  - El frontend de la RasPi se llama frontendRaspberry.c (antes el archivo
#    tenía un typo, "fontend", y el Makefile no lo encontraba).
#  - -fsanitize=address solo con DEBUG=1. Con Allegro, LeakSanitizer reporta
#    "fugas" internas de la librería al cerrar y el programa termina con error,
#    aunque nuestro código esté bien. En la RasPi ASan es muy lento.
#  - Se sacó -D RASPBERRY: no hace falta un #ifdef porque el Makefile ya elige
#    qué frontend linkear (el "makefile genérico" que recomienda la consigna).
# =============================================================================

CC     := gcc
CFLAGS := -Wall -Wextra -std=gnu11 -Ibackend -Ifrontend

BACKEND_SRC := main.c \
               backend/juego.c \
               backend/obstaculo.c \
               backend/niveles.c \
               backend/puntajes.c

# ----------------------------------------------------------------- Allegro ---
ALLEGRO_EXEC := frogger-allegro
ALLEGRO_SRC  := $(BACKEND_SRC) frontend/allegro/frontendAllegro.c
ALLEGRO_LIBS := -lallegro -lallegro_primitives -lallegro_image -lallegro_font -lallegro_ttf

ifeq ($(DEBUG),1)
  ALLEGRO_BUILD  := build/allegro-debug
  ALLEGRO_EXTRA  := -g -O0 -fsanitize=address
else
  ALLEGRO_BUILD  := build/allegro
  ALLEGRO_EXTRA  := -O2
endif
ALLEGRO_OBJ := $(patsubst %.c,$(ALLEGRO_BUILD)/%.o,$(ALLEGRO_SRC))

# --------------------------------------------------------------- Raspberry ---
# Copiar acá los archivos que da la cátedra: disdrv.h, joydrv.h y sus .o
# (si en su versión tienen otro nombre, cambiarlo en RASPBERRY_LIBS).
RPI_LIBDIR      := frontend/raspberry/libs
RASPBERRY_EXEC  := frogger-raspberry
RASPBERRY_SRC   := $(BACKEND_SRC) frontend/raspberry/frontendRaspberry.c
RASPBERRY_BUILD := build/raspberry
RASPBERRY_OBJ   := $(patsubst %.c,$(RASPBERRY_BUILD)/%.o,$(RASPBERRY_SRC))
RASPBERRY_LIBS  := $(RPI_LIBDIR)/disdrv.o $(RPI_LIBDIR)/joydrv.o

# ----------------------------------------------------------------- Reglas ---
.PHONY: default allegro raspberry clean

default:
	@echo "Especificar \"make allegro\" o \"make raspberry\""

allegro: $(ALLEGRO_EXEC)

raspberry: $(RASPBERRY_EXEC)

$(ALLEGRO_EXEC): $(ALLEGRO_OBJ)
	$(CC) $(ALLEGRO_EXTRA) $^ -o $@ $(ALLEGRO_LIBS)

$(RASPBERRY_EXEC): $(RASPBERRY_OBJ)
	$(CC) $^ $(RASPBERRY_LIBS) -o $@

$(ALLEGRO_BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(ALLEGRO_EXTRA) -MMD -MP -c $< -o $@

$(RASPBERRY_BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -O2 -I$(RPI_LIBDIR) -MMD -MP -c $< -o $@

clean:
	rm -rf build $(ALLEGRO_EXEC) $(RASPBERRY_EXEC)

# Dependencias de los .h generadas por -MMD
-include $(ALLEGRO_OBJ:.o=.d) $(RASPBERRY_OBJ:.o=.d)
